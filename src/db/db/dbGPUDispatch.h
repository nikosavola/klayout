
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


#ifndef HDR_dbGPUDispatch
#define HDR_dbGPUDispatch

#include "dbCommon.h"
#include "dbGPUBackend.h"

namespace db
{

/**
 *  @brief Helper to determine whether to use GPU for a given operation
 *
 *  This class encapsulates the decision logic for dispatching work to the GPU
 *  vs. keeping it on the CPU. The decision is based on:
 *  - Whether GPU support is compiled in and enabled
 *  - Whether a GPU device is available
 *  - Whether the workload exceeds the minimum threshold for GPU dispatch
 */
class DB_PUBLIC GPUDispatch
{
public:
  /**
   *  @brief Returns true if GPU should be used for polygon sizing with the given count
   */
  static bool use_gpu_for_sizing (size_t polygon_count)
  {
    if (! GPUBackend::is_enabled ()) {
      return false;
    }
    GPUBackend &backend = GPUBackend::instance ();
    return backend.is_available () && polygon_count >= backend.min_polygon_count_for_gpu ();
  }

  /**
   *  @brief Returns true if GPU should be used for point-in-polygon tests with the given count
   */
  static bool use_gpu_for_point_in_polygon (size_t point_count)
  {
    if (! GPUBackend::is_enabled ()) {
      return false;
    }
    GPUBackend &backend = GPUBackend::instance ();
    return backend.is_available () && point_count >= backend.min_point_count_for_gpu ();
  }

  /**
   *  @brief Returns true if GPU should be used for bounding box computation with the given count
   */
  static bool use_gpu_for_bounding_boxes (size_t polygon_count)
  {
    if (! GPUBackend::is_enabled ()) {
      return false;
    }
    GPUBackend &backend = GPUBackend::instance ();
    //  Bounding boxes are very cheap on CPU, so require a higher threshold
    return backend.is_available () && polygon_count >= backend.min_polygon_count_for_gpu () * 10;
  }
};

}  // namespace db

#endif // HDR_dbGPUDispatch
