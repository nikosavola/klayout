
/*

  KLayout Layout Viewer
  Copyright (C) 2006-2026 Matthias Koefferlein

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

/**
 *  @file dbGPUKernels.cu
 *  @brief CUDA kernel implementations for GPU-accelerated polygon operations
 *
 *  This file contains CUDA kernels for:
 *  - Batch point-in-polygon testing
 *  - Polygon sizing (offset/inflate/deflate)
 *  - Batch bounding box computation
 *
 *  Compilation requires nvcc (NVIDIA CUDA Compiler) and a compatible GPU.
 *  These kernels are called from dbGPUBackend.cc via the GPUBackendCUDA class.
 */

#if defined(HAVE_CUDA)

#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#include <cstdint>
#include <cstdio>

// -----------------------------------------------------------------------------------
//  Device-side data structures (mirrors of host-side db::Point, db::Edge etc.)

struct GPUPoint
{
  int32_t x;
  int32_t y;
};

struct GPUEdge
{
  GPUPoint p1;
  GPUPoint p2;
};

// -----------------------------------------------------------------------------------
//  Point-in-Polygon Kernel
//
//  Uses the winding number (ray-casting) algorithm.
//  Each thread tests one point against the polygon edges.

__device__ int gpu_side_of (const GPUEdge &edge, const GPUPoint &pt)
{
  //  Cross product of (p2-p1) x (pt-p1) to determine side
  int64_t dx = (int64_t)edge.p2.x - (int64_t)edge.p1.x;
  int64_t dy = (int64_t)edge.p2.y - (int64_t)edge.p1.y;
  int64_t px = (int64_t)pt.x - (int64_t)edge.p1.x;
  int64_t py = (int64_t)pt.y - (int64_t)edge.p1.y;

  int64_t cross = dx * py - dy * px;

  if (cross > 0) return 1;
  if (cross < 0) return -1;
  return 0;
}

__global__ void kernel_point_in_polygon (
  const GPUPoint *points,
  int num_points,
  const GPUEdge *edges,
  int num_edges,
  int *results)
{
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx >= num_points) return;

  GPUPoint pt = points[idx];
  int wrapcount = 0;
  int result = -1;  //  Default: outside

  for (int e = 0; e < num_edges; ++e) {
    GPUEdge edge = edges[e];

    int32_t y1 = edge.p1.y;
    int32_t y2 = edge.p2.y;

    if (y1 <= pt.y && y2 > pt.y) {
      //  Upward crossing
      int side = gpu_side_of (edge, pt);
      if (side < 0) {
        ++wrapcount;
      } else if (side == 0) {
        results[idx] = 0;  //  On boundary
        return;
      }
    } else if (y2 <= pt.y && y1 > pt.y) {
      //  Downward crossing
      int side = gpu_side_of (edge, pt);
      if (side > 0) {
        --wrapcount;
      } else if (side == 0) {
        results[idx] = 0;  //  On boundary
        return;
      }
    } else if (y1 == pt.y && y2 == pt.y) {
      //  Horizontal edge - check if point is on it
      int32_t min_x = min (edge.p1.x, edge.p2.x);
      int32_t max_x = max (edge.p1.x, edge.p2.x);
      if (pt.x >= min_x && pt.x <= max_x) {
        results[idx] = 0;  //  On boundary
        return;
      }
    }
  }

  results[idx] = (wrapcount != 0) ? 1 : -1;
}

// -----------------------------------------------------------------------------------
//  Bounding Box Kernel
//
//  Each thread computes the bounding box of one polygon.
//  Polygons are stored as a flat array of vertices with an offset array.

__global__ void kernel_bounding_boxes (
  const GPUPoint *vertices,
  const int *polygon_offsets,
  const int *polygon_sizes,
  int num_polygons,
  int32_t *boxes)  //  [min_x, min_y, max_x, max_y] per polygon
{
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx >= num_polygons) return;

  int offset = polygon_offsets[idx];
  int size = polygon_sizes[idx];

  if (size == 0) {
    boxes[idx * 4 + 0] = 0;
    boxes[idx * 4 + 1] = 0;
    boxes[idx * 4 + 2] = 0;
    boxes[idx * 4 + 3] = 0;
    return;
  }

  int32_t min_x = vertices[offset].x;
  int32_t min_y = vertices[offset].y;
  int32_t max_x = min_x;
  int32_t max_y = min_y;

  for (int i = 1; i < size; ++i) {
    GPUPoint p = vertices[offset + i];
    min_x = min (min_x, p.x);
    min_y = min (min_y, p.y);
    max_x = max (max_x, p.x);
    max_y = max (max_y, p.y);
  }

  boxes[idx * 4 + 0] = min_x;
  boxes[idx * 4 + 1] = min_y;
  boxes[idx * 4 + 2] = max_x;
  boxes[idx * 4 + 3] = max_y;
}

// -----------------------------------------------------------------------------------
//  Polygon Sizing Kernel (Square mode only - mode 3)
//
//  For square-mode sizing, each edge is offset by (dx, dy) perpendicular to the edge
//  direction. This is the simplest sizing mode and most GPU-friendly.
//  Each thread processes one edge of one polygon to produce the offset edge.

__global__ void kernel_polygon_sizing_square (
  const GPUPoint *vertices,
  const int *polygon_offsets,
  const int *polygon_sizes,
  int num_polygons,
  int32_t dx,
  int32_t dy,
  GPUPoint *output_vertices)
{
  int poly_idx = blockIdx.x;
  if (poly_idx >= num_polygons) return;

  int offset = polygon_offsets[poly_idx];
  int size = polygon_sizes[poly_idx];
  int vert_idx = threadIdx.x;

  if (vert_idx >= size) return;

  //  For square sizing: offset each vertex by (dx, dy) based on the
  //  orientation of its adjacent edges.
  //  Simplified approach: move each vertex outward by (dx, dy) in the
  //  direction determined by the angle bisector of its two adjacent edges.

  int prev_idx = (vert_idx + size - 1) % size;
  int next_idx = (vert_idx + 1) % size;

  GPUPoint prev = vertices[offset + prev_idx];
  GPUPoint curr = vertices[offset + vert_idx];
  GPUPoint next = vertices[offset + next_idx];

  //  Compute edge normals (outward-pointing for CCW polygon)
  //  Edge from prev to curr
  int32_t e1_dx = curr.x - prev.x;
  int32_t e1_dy = curr.y - prev.y;
  //  Normal: (dy, -dx) for left-side normal in CCW
  //  Edge from curr to next
  int32_t e2_dx = next.x - curr.x;
  int32_t e2_dy = next.y - curr.y;

  //  Simplified square sizing: offset vertex by (sign_x * dx, sign_y * dy)
  //  where sign is determined by the average normal direction
  float n1x = (float)e1_dy;
  float n1y = (float)(-e1_dx);
  float n2x = (float)e2_dy;
  float n2y = (float)(-e2_dx);

  //  Normalize
  float len1 = sqrtf (n1x * n1x + n1y * n1y);
  float len2 = sqrtf (n2x * n2x + n2y * n2y);

  if (len1 > 0.0f) { n1x /= len1; n1y /= len1; }
  if (len2 > 0.0f) { n2x /= len2; n2y /= len2; }

  //  Average normal (bisector direction)
  float nx = (n1x + n2x) * 0.5f;
  float ny = (n1y + n2y) * 0.5f;
  float nlen = sqrtf (nx * nx + ny * ny);

  GPUPoint result;
  if (nlen > 1e-6f) {
    //  Scale the offset to maintain correct distance
    float scale = 1.0f / nlen;
    result.x = curr.x + (int32_t)(nx * scale * (float)dx);
    result.y = curr.y + (int32_t)(ny * scale * (float)dy);
  } else {
    result.x = curr.x;
    result.y = curr.y;
  }

  output_vertices[offset + vert_idx] = result;
}

// -----------------------------------------------------------------------------------
//  Host-callable wrapper functions

extern "C" {

/**
 *  @brief Launch point-in-polygon kernel
 *
 *  @return 0 on success, non-zero on CUDA error
 */
int gpu_batch_point_in_polygon (
  const int32_t *h_points_xy,  //  Interleaved [x0,y0,x1,y1,...] on host
  int num_points,
  const int32_t *h_edges_xy,   //  Interleaved [x1,y1,x2,y2,...] on host
  int num_edges,
  int *h_results)              //  Output on host
{
  GPUPoint *d_points = nullptr;
  GPUEdge *d_edges = nullptr;
  int *d_results = nullptr;

  size_t points_size = num_points * sizeof(GPUPoint);
  size_t edges_size = num_edges * sizeof(GPUEdge);
  size_t results_size = num_points * sizeof(int);

  //  Allocate device memory
  if (cudaMalloc (&d_points, points_size) != cudaSuccess) return -1;
  if (cudaMalloc (&d_edges, edges_size) != cudaSuccess) { cudaFree (d_points); return -1; }
  if (cudaMalloc (&d_results, results_size) != cudaSuccess) { cudaFree (d_points); cudaFree (d_edges); return -1; }

  //  Copy data to device
  cudaMemcpy (d_points, h_points_xy, points_size, cudaMemcpyHostToDevice);
  cudaMemcpy (d_edges, h_edges_xy, edges_size, cudaMemcpyHostToDevice);

  //  Launch kernel
  int threads_per_block = 256;
  int num_blocks = (num_points + threads_per_block - 1) / threads_per_block;
  kernel_point_in_polygon<<<num_blocks, threads_per_block>>> (
    d_points, num_points, d_edges, num_edges, d_results);

  //  Copy results back
  cudaMemcpy (h_results, d_results, results_size, cudaMemcpyDeviceToHost);

  //  Cleanup
  cudaFree (d_points);
  cudaFree (d_edges);
  cudaFree (d_results);

  return (cudaGetLastError () == cudaSuccess) ? 0 : -1;
}

/**
 *  @brief Launch bounding box kernel
 *
 *  @return 0 on success, non-zero on CUDA error
 */
int gpu_batch_bounding_boxes (
  const int32_t *h_vertices_xy,  //  Interleaved vertex coords on host
  const int *h_offsets,          //  Polygon vertex offsets
  const int *h_sizes,            //  Polygon vertex counts
  int num_polygons,
  int total_vertices,
  int32_t *h_boxes)              //  Output [min_x, min_y, max_x, max_y] per polygon
{
  GPUPoint *d_vertices = nullptr;
  int *d_offsets = nullptr;
  int *d_sizes = nullptr;
  int32_t *d_boxes = nullptr;

  size_t verts_size = total_vertices * sizeof(GPUPoint);
  size_t offsets_size = num_polygons * sizeof(int);
  size_t boxes_size = num_polygons * 4 * sizeof(int32_t);

  if (cudaMalloc (&d_vertices, verts_size) != cudaSuccess) return -1;
  if (cudaMalloc (&d_offsets, offsets_size) != cudaSuccess) { cudaFree (d_vertices); return -1; }
  if (cudaMalloc (&d_sizes, offsets_size) != cudaSuccess) { cudaFree (d_vertices); cudaFree (d_offsets); return -1; }
  if (cudaMalloc (&d_boxes, boxes_size) != cudaSuccess) { cudaFree (d_vertices); cudaFree (d_offsets); cudaFree (d_sizes); return -1; }

  cudaMemcpy (d_vertices, h_vertices_xy, verts_size, cudaMemcpyHostToDevice);
  cudaMemcpy (d_offsets, h_offsets, offsets_size, cudaMemcpyHostToDevice);
  cudaMemcpy (d_sizes, h_sizes, offsets_size, cudaMemcpyHostToDevice);

  int threads_per_block = 256;
  int num_blocks = (num_polygons + threads_per_block - 1) / threads_per_block;
  kernel_bounding_boxes<<<num_blocks, threads_per_block>>> (
    d_vertices, d_offsets, d_sizes, num_polygons, d_boxes);

  cudaMemcpy (h_boxes, d_boxes, boxes_size, cudaMemcpyDeviceToHost);

  cudaFree (d_vertices);
  cudaFree (d_offsets);
  cudaFree (d_sizes);
  cudaFree (d_boxes);

  return (cudaGetLastError () == cudaSuccess) ? 0 : -1;
}

}  // extern "C"

#endif // HAVE_CUDA
