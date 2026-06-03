
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


#ifndef HDR_dbGPUBackend
#define HDR_dbGPUBackend

#include "dbCommon.h"
#include "dbPolygon.h"
#include "dbBox.h"
#include "dbPoint.h"

#include <vector>
#include <string>
#include <memory>

namespace db
{

// -----------------------------------------------------------------------------------
//  GPU Device Information

/**
 *  @brief Information about a detected GPU device
 */
struct DB_PUBLIC GPUDeviceInfo
{
  std::string name;
  size_t total_memory;
  int compute_capability_major;
  int compute_capability_minor;
  int max_threads_per_block;
  int multiprocessor_count;

  GPUDeviceInfo ()
    : total_memory (0), compute_capability_major (0), compute_capability_minor (0),
      max_threads_per_block (0), multiprocessor_count (0)
  { }
};

// -----------------------------------------------------------------------------------
//  GPU Backend Type

/**
 *  @brief Enumeration of supported GPU backend types
 */
enum GPUBackendType
{
  GPU_None = 0,     //  No GPU available or disabled
  GPU_CUDA = 1,     //  NVIDIA CUDA
  GPU_OpenCL = 2    //  OpenCL (vendor-neutral)
};

// -----------------------------------------------------------------------------------
//  GPU Backend Base Class

/**
 *  @brief Abstract base class for GPU computation backends
 *
 *  This class provides the interface for GPU-accelerated polygon operations.
 *  Concrete implementations exist for CUDA and OpenCL backends.
 *  When no GPU is available, a CPU fallback implementation is used transparently.
 */
class DB_PUBLIC GPUBackend
{
public:
  GPUBackend ();
  virtual ~GPUBackend ();

  /**
   *  @brief Returns the singleton GPU backend instance
   *
   *  The backend is initialized on first access. If no GPU is available,
   *  returns a fallback backend that uses CPU implementations.
   */
  static GPUBackend &instance ();

  /**
   *  @brief Returns the type of the active backend
   */
  virtual GPUBackendType type () const = 0;

  /**
   *  @brief Returns true if GPU acceleration is available
   */
  virtual bool is_available () const = 0;

  /**
   *  @brief Returns information about the active GPU device
   */
  virtual GPUDeviceInfo device_info () const = 0;

  /**
   *  @brief Returns the minimum number of polygons to justify GPU dispatch
   *
   *  Below this threshold, the CPU path is faster due to data transfer overhead.
   */
  virtual size_t min_polygon_count_for_gpu () const;

  /**
   *  @brief Returns the minimum number of points for GPU point-in-polygon tests
   */
  virtual size_t min_point_count_for_gpu () const;

  // -----------------------------------------------------------------------------------
  //  Polygon Sizing (Offset/Inflate/Deflate)

  /**
   *  @brief GPU-accelerated polygon sizing (offset) operation
   *
   *  Applies an offset (dx, dy) to each polygon independently using the specified mode.
   *  This is an embarrassingly parallel operation ideal for GPU acceleration.
   *
   *  @param polygons Input polygons to size
   *  @param dx X-direction offset (positive = grow, negative = shrink)
   *  @param dy Y-direction offset (positive = grow, negative = shrink)
   *  @param mode Sizing mode (0 = round corners, 1 = diamond, 2 = octagon, 3 = square)
   *  @param results Output sized polygons (one or more per input polygon)
   */
  virtual void polygon_sizing (
    const std::vector<db::Polygon> &polygons,
    db::Coord dx, db::Coord dy,
    unsigned int mode,
    std::vector<db::Polygon> &results) = 0;

  // -----------------------------------------------------------------------------------
  //  Point-in-Polygon Batch Tests

  /**
   *  @brief GPU-accelerated batch point-in-polygon test
   *
   *  Tests multiple points against a single polygon simultaneously.
   *  Returns 1 for inside, 0 for on boundary, -1 for outside.
   *
   *  @param polygon The polygon to test against
   *  @param points The points to test
   *  @param results Output: test results for each point (1=inside, 0=on, -1=outside)
   */
  virtual void batch_point_in_polygon (
    const db::Polygon &polygon,
    const std::vector<db::Point> &points,
    std::vector<int> &results) = 0;

  /**
   *  @brief GPU-accelerated batch point-in-polygon test for multiple polygons
   *
   *  Tests each point against its corresponding polygon.
   *  polygon_indices maps each point to its polygon index.
   *
   *  @param polygons The polygons to test against
   *  @param points The points to test
   *  @param polygon_indices Index into polygons array for each point
   *  @param results Output: test results for each point (1=inside, 0=on, -1=outside)
   */
  virtual void batch_point_in_polygons (
    const std::vector<db::Polygon> &polygons,
    const std::vector<db::Point> &points,
    const std::vector<size_t> &polygon_indices,
    std::vector<int> &results) = 0;

  // -----------------------------------------------------------------------------------
  //  Bounding Box Computation

  /**
   *  @brief GPU-accelerated batch bounding box computation
   *
   *  Computes the bounding box of each polygon in parallel.
   *
   *  @param polygons Input polygons
   *  @param boxes Output: bounding box for each polygon
   */
  virtual void batch_bounding_boxes (
    const std::vector<db::Polygon> &polygons,
    std::vector<db::Box> &boxes) = 0;

  // -----------------------------------------------------------------------------------
  //  Configuration

  /**
   *  @brief Enable or disable GPU acceleration globally
   */
  static void set_enabled (bool enabled);

  /**
   *  @brief Returns true if GPU acceleration is globally enabled
   */
  static bool is_enabled ();

private:
  static bool s_enabled;
  static std::unique_ptr<GPUBackend> sp_instance;
};

// -----------------------------------------------------------------------------------
//  CPU Fallback Backend

/**
 *  @brief CPU fallback implementation of the GPU backend interface
 *
 *  This backend is used when no GPU is available or GPU support is disabled.
 *  It provides equivalent functionality using CPU-based algorithms.
 */
class DB_PUBLIC GPUBackendCPUFallback : public GPUBackend
{
public:
  GPUBackendCPUFallback ();
  virtual ~GPUBackendCPUFallback ();

  virtual GPUBackendType type () const;
  virtual bool is_available () const;
  virtual GPUDeviceInfo device_info () const;

  virtual void polygon_sizing (
    const std::vector<db::Polygon> &polygons,
    db::Coord dx, db::Coord dy,
    unsigned int mode,
    std::vector<db::Polygon> &results);

  virtual void batch_point_in_polygon (
    const db::Polygon &polygon,
    const std::vector<db::Point> &points,
    std::vector<int> &results);

  virtual void batch_point_in_polygons (
    const std::vector<db::Polygon> &polygons,
    const std::vector<db::Point> &points,
    const std::vector<size_t> &polygon_indices,
    std::vector<int> &results);

  virtual void batch_bounding_boxes (
    const std::vector<db::Polygon> &polygons,
    std::vector<db::Box> &boxes);
};

#if defined(HAVE_CUDA)

// -----------------------------------------------------------------------------------
//  CUDA Backend

/**
 *  @brief CUDA-based GPU backend implementation
 *
 *  Uses NVIDIA CUDA for GPU-accelerated polygon operations.
 *  Requires CUDA toolkit and NVIDIA GPU with compute capability >= 3.5.
 */
class DB_PUBLIC GPUBackendCUDA : public GPUBackend
{
public:
  GPUBackendCUDA ();
  virtual ~GPUBackendCUDA ();

  virtual GPUBackendType type () const;
  virtual bool is_available () const;
  virtual GPUDeviceInfo device_info () const;

  virtual void polygon_sizing (
    const std::vector<db::Polygon> &polygons,
    db::Coord dx, db::Coord dy,
    unsigned int mode,
    std::vector<db::Polygon> &results);

  virtual void batch_point_in_polygon (
    const db::Polygon &polygon,
    const std::vector<db::Point> &points,
    std::vector<int> &results);

  virtual void batch_point_in_polygons (
    const std::vector<db::Polygon> &polygons,
    const std::vector<db::Point> &points,
    const std::vector<size_t> &polygon_indices,
    std::vector<int> &results);

  virtual void batch_bounding_boxes (
    const std::vector<db::Polygon> &polygons,
    std::vector<db::Box> &boxes);

private:
  bool m_initialized;
  GPUDeviceInfo m_device_info;

  bool initialize ();
};

#endif // HAVE_CUDA

}  // namespace db

#endif // HDR_dbGPUBackend
