
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

#include "dbGPUBackend.h"
#include "dbPolygonTools.h"
#include "dbEdgeProcessor.h"
#include "dbPolygonGenerators.h"
#include "tlLog.h"

#include <algorithm>
#include <cmath>

namespace db
{

// -----------------------------------------------------------------------------------
//  GPUBackend implementation

bool GPUBackend::s_enabled = true;
std::unique_ptr<GPUBackend> GPUBackend::sp_instance;

GPUBackend::GPUBackend ()
{
  //  .. nothing yet ..
}

GPUBackend::~GPUBackend ()
{
  //  .. nothing yet ..
}

GPUBackend &GPUBackend::instance ()
{
  if (! sp_instance) {
#if defined(HAVE_CUDA)
    //  Try CUDA first
    std::unique_ptr<GPUBackendCUDA> cuda_backend (new GPUBackendCUDA ());
    if (cuda_backend->is_available ()) {
      tl::info << "GPU acceleration: CUDA backend initialized ("
               << cuda_backend->device_info ().name << ")";
      sp_instance = std::move (cuda_backend);
    } else {
      tl::info << "GPU acceleration: CUDA not available, using CPU fallback";
      sp_instance.reset (new GPUBackendCPUFallback ());
    }
#else
    tl::info << "GPU acceleration: built without CUDA support, using CPU fallback";
    sp_instance.reset (new GPUBackendCPUFallback ());
#endif
  }
  return *sp_instance;
}

size_t GPUBackend::min_polygon_count_for_gpu () const
{
  //  Default threshold: 10K polygons to justify GPU dispatch overhead
  return 10000;
}

size_t GPUBackend::min_point_count_for_gpu () const
{
  //  Default threshold: 50K points to justify GPU dispatch overhead
  return 50000;
}

void GPUBackend::set_enabled (bool enabled)
{
  s_enabled = enabled;
}

bool GPUBackend::is_enabled ()
{
  return s_enabled;
}

// -----------------------------------------------------------------------------------
//  GPUBackendCPUFallback implementation

GPUBackendCPUFallback::GPUBackendCPUFallback ()
{
  //  .. nothing yet ..
}

GPUBackendCPUFallback::~GPUBackendCPUFallback ()
{
  //  .. nothing yet ..
}

GPUBackendType GPUBackendCPUFallback::type () const
{
  return GPU_None;
}

bool GPUBackendCPUFallback::is_available () const
{
  return false;
}

GPUDeviceInfo GPUBackendCPUFallback::device_info () const
{
  GPUDeviceInfo info;
  info.name = "CPU Fallback";
  return info;
}

void GPUBackendCPUFallback::polygon_sizing (
  const std::vector<db::Polygon> &polygons,
  db::Coord dx, db::Coord dy,
  unsigned int mode,
  std::vector<db::Polygon> &results)
{
  results.clear ();
  results.reserve (polygons.size ());

  for (size_t i = 0; i < polygons.size (); ++i) {

    db::PolygonContainer pc (results);
    db::PolygonGenerator pg (pc, false, true);
    db::SizingPolygonFilter sf (pg, dx, dy, mode);
    sf.put (polygons[i]);

  }
}

void GPUBackendCPUFallback::batch_point_in_polygon (
  const db::Polygon &polygon,
  const std::vector<db::Point> &points,
  std::vector<int> &results)
{
  results.resize (points.size ());

  inside_poly_test<db::Polygon> test (polygon);

  for (size_t i = 0; i < points.size (); ++i) {
    results[i] = test (points[i]);
  }
}

void GPUBackendCPUFallback::batch_point_in_polygons (
  const std::vector<db::Polygon> &polygons,
  const std::vector<db::Point> &points,
  const std::vector<size_t> &polygon_indices,
  std::vector<int> &results)
{
  results.resize (points.size ());

  //  Cache inside_poly_test objects for each polygon
  std::vector<std::unique_ptr<inside_poly_test<db::Polygon>>> tests;
  tests.reserve (polygons.size ());
  for (size_t i = 0; i < polygons.size (); ++i) {
    tests.emplace_back (new inside_poly_test<db::Polygon> (polygons[i]));
  }

  for (size_t i = 0; i < points.size (); ++i) {
    size_t pi = polygon_indices[i];
    if (pi < tests.size ()) {
      results[i] = (*tests[pi]) (points[i]);
    } else {
      results[i] = -1;
    }
  }
}

void GPUBackendCPUFallback::batch_bounding_boxes (
  const std::vector<db::Polygon> &polygons,
  std::vector<db::Box> &boxes)
{
  boxes.resize (polygons.size ());

  for (size_t i = 0; i < polygons.size (); ++i) {
    boxes[i] = polygons[i].box ();
  }
}

// -----------------------------------------------------------------------------------
//  CUDA Backend implementation (stub - actual CUDA calls in gpu/dbGPUKernels.cu)

#if defined(HAVE_CUDA)

GPUBackendCUDA::GPUBackendCUDA ()
  : m_initialized (false)
{
  m_initialized = initialize ();
}

GPUBackendCUDA::~GPUBackendCUDA ()
{
  //  .. cleanup CUDA resources ..
}

bool GPUBackendCUDA::initialize ()
{
  //  NOTE: This is a stub. Actual CUDA initialization would call:
  //    cudaGetDeviceCount()
  //    cudaGetDeviceProperties()
  //    cudaSetDevice()
  //  For now, report as not available until CUDA kernels are compiled.
  return false;
}

GPUBackendType GPUBackendCUDA::type () const
{
  return GPU_CUDA;
}

bool GPUBackendCUDA::is_available () const
{
  return m_initialized;
}

GPUDeviceInfo GPUBackendCUDA::device_info () const
{
  return m_device_info;
}

void GPUBackendCUDA::polygon_sizing (
  const std::vector<db::Polygon> &polygons,
  db::Coord dx, db::Coord dy,
  unsigned int mode,
  std::vector<db::Polygon> &results)
{
  //  Delegate to CUDA kernel (gpu/dbGPUKernels.cu)
  //  For now, fall back to CPU implementation
  GPUBackendCPUFallback fallback;
  fallback.polygon_sizing (polygons, dx, dy, mode, results);
}

void GPUBackendCUDA::batch_point_in_polygon (
  const db::Polygon &polygon,
  const std::vector<db::Point> &points,
  std::vector<int> &results)
{
  //  Delegate to CUDA kernel (gpu/dbGPUKernels.cu)
  //  For now, fall back to CPU implementation
  GPUBackendCPUFallback fallback;
  fallback.batch_point_in_polygon (polygon, points, results);
}

void GPUBackendCUDA::batch_point_in_polygons (
  const std::vector<db::Polygon> &polygons,
  const std::vector<db::Point> &points,
  const std::vector<size_t> &polygon_indices,
  std::vector<int> &results)
{
  //  Delegate to CUDA kernel (gpu/dbGPUKernels.cu)
  //  For now, fall back to CPU implementation
  GPUBackendCPUFallback fallback;
  fallback.batch_point_in_polygons (polygons, points, polygon_indices, results);
}

void GPUBackendCUDA::batch_bounding_boxes (
  const std::vector<db::Polygon> &polygons,
  std::vector<db::Box> &boxes)
{
  //  Delegate to CUDA kernel (gpu/dbGPUKernels.cu)
  //  For now, fall back to CPU implementation
  GPUBackendCPUFallback fallback;
  fallback.batch_bounding_boxes (polygons, boxes);
}

#endif // HAVE_CUDA

}  // namespace db
