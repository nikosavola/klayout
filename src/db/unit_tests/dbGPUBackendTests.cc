
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
#include "dbPolygon.h"
#include "dbPolygonTools.h"
#include "tlUnitTest.h"

#include <vector>
#include <cmath>

// -----------------------------------------------------------------------------------
//  Tests for GPU Backend (CPU fallback path)
//  These tests verify the GPU backend interface using the CPU fallback implementation.

TEST(GPUBackend_1_singleton)
{
  //  Test that the GPU backend singleton is accessible
  db::GPUBackend &backend = db::GPUBackend::instance ();

  //  Without CUDA compiled in, should be CPU fallback
#if !defined(HAVE_CUDA)
  EXPECT_EQ (backend.type (), db::GPU_None);
  EXPECT_EQ (backend.is_available (), false);
  EXPECT_EQ (backend.device_info ().name, "CPU Fallback");
#endif
}

TEST(GPUBackend_2_enable_disable)
{
  //  Test enable/disable mechanism
  EXPECT_EQ (db::GPUBackend::is_enabled (), true);

  db::GPUBackend::set_enabled (false);
  EXPECT_EQ (db::GPUBackend::is_enabled (), false);

  db::GPUBackend::set_enabled (true);
  EXPECT_EQ (db::GPUBackend::is_enabled (), true);
}

TEST(GPUBackend_3_point_in_polygon_basic)
{
  //  Test batch point-in-polygon with a simple square
  db::Polygon square (db::Box (0, 0, 1000, 1000));

  std::vector<db::Point> points;
  points.push_back (db::Point (500, 500));    //  inside
  points.push_back (db::Point (0, 0));        //  on corner
  points.push_back (db::Point (500, 0));      //  on edge
  points.push_back (db::Point (1500, 500));   //  outside
  points.push_back (db::Point (-100, -100));  //  outside

  std::vector<int> results;
  db::GPUBackend::instance ().batch_point_in_polygon (square, points, results);

  EXPECT_EQ (results.size (), size_t (5));
  EXPECT_EQ (results[0], 1);   //  inside
  EXPECT_EQ (results[1], 0);   //  on boundary
  EXPECT_EQ (results[2], 0);   //  on boundary
  EXPECT_EQ (results[3], -1);  //  outside
  EXPECT_EQ (results[4], -1);  //  outside
}

TEST(GPUBackend_4_point_in_polygon_triangle)
{
  //  Test with a triangle
  std::vector<db::Point> hull;
  hull.push_back (db::Point (0, 0));
  hull.push_back (db::Point (1000, 0));
  hull.push_back (db::Point (500, 1000));
  db::Polygon triangle;
  triangle.assign_hull (hull.begin (), hull.end ());

  std::vector<db::Point> points;
  points.push_back (db::Point (500, 300));    //  inside
  points.push_back (db::Point (100, 800));    //  outside (left of edge)
  points.push_back (db::Point (900, 800));    //  outside (right of edge)
  points.push_back (db::Point (500, 0));      //  on edge

  std::vector<int> results;
  db::GPUBackend::instance ().batch_point_in_polygon (triangle, points, results);

  EXPECT_EQ (results.size (), size_t (4));
  EXPECT_EQ (results[0], 1);   //  inside
  EXPECT_EQ (results[1], -1);  //  outside
  EXPECT_EQ (results[2], -1);  //  outside
  EXPECT_EQ (results[3], 0);   //  on boundary
}

TEST(GPUBackend_5_point_in_polygons_multi)
{
  //  Test batch point-in-polygons with multiple polygons
  std::vector<db::Polygon> polygons;
  polygons.push_back (db::Polygon (db::Box (0, 0, 100, 100)));
  polygons.push_back (db::Polygon (db::Box (200, 200, 300, 300)));

  std::vector<db::Point> points;
  points.push_back (db::Point (50, 50));     //  inside polygon 0
  points.push_back (db::Point (250, 250));   //  inside polygon 1
  points.push_back (db::Point (150, 150));   //  outside polygon 0
  points.push_back (db::Point (150, 150));   //  outside polygon 1

  std::vector<size_t> indices;
  indices.push_back (0);
  indices.push_back (1);
  indices.push_back (0);
  indices.push_back (1);

  std::vector<int> results;
  db::GPUBackend::instance ().batch_point_in_polygons (polygons, points, indices, results);

  EXPECT_EQ (results.size (), size_t (4));
  EXPECT_EQ (results[0], 1);   //  inside
  EXPECT_EQ (results[1], 1);   //  inside
  EXPECT_EQ (results[2], -1);  //  outside
  EXPECT_EQ (results[3], -1);  //  outside
}

TEST(GPUBackend_6_bounding_boxes)
{
  //  Test batch bounding box computation
  std::vector<db::Polygon> polygons;

  //  Simple box
  polygons.push_back (db::Polygon (db::Box (10, 20, 30, 40)));

  //  Triangle
  std::vector<db::Point> hull;
  hull.push_back (db::Point (100, 100));
  hull.push_back (db::Point (200, 100));
  hull.push_back (db::Point (150, 300));
  db::Polygon triangle;
  triangle.assign_hull (hull.begin (), hull.end ());
  polygons.push_back (triangle);

  //  L-shaped polygon
  hull.clear ();
  hull.push_back (db::Point (0, 0));
  hull.push_back (db::Point (100, 0));
  hull.push_back (db::Point (100, 50));
  hull.push_back (db::Point (50, 50));
  hull.push_back (db::Point (50, 100));
  hull.push_back (db::Point (0, 100));
  db::Polygon lshape;
  lshape.assign_hull (hull.begin (), hull.end ());
  polygons.push_back (lshape);

  std::vector<db::Box> boxes;
  db::GPUBackend::instance ().batch_bounding_boxes (polygons, boxes);

  EXPECT_EQ (boxes.size (), size_t (3));
  EXPECT_EQ (boxes[0], db::Box (10, 20, 30, 40));
  EXPECT_EQ (boxes[1], db::Box (100, 100, 200, 300));
  EXPECT_EQ (boxes[2], db::Box (0, 0, 100, 100));
}

TEST(GPUBackend_7_polygon_sizing)
{
  //  Test polygon sizing (CPU fallback)
  std::vector<db::Polygon> polygons;
  polygons.push_back (db::Polygon (db::Box (0, 0, 1000, 1000)));

  std::vector<db::Polygon> results;

  //  Grow by 100 in all directions (square mode = 2)
  db::GPUBackend::instance ().polygon_sizing (polygons, 100, 100, 2, results);

  //  Result should be larger than the original
  EXPECT_EQ (results.size () > size_t (0), true);
  if (results.size () > 0) {
    db::Box result_box = results[0].box ();
    //  The sized polygon should encompass the grown area
    EXPECT_EQ (result_box.left () <= -100, true);
    EXPECT_EQ (result_box.bottom () <= -100, true);
    EXPECT_EQ (result_box.right () >= 1100, true);
    EXPECT_EQ (result_box.top () >= 1100, true);
  }
}

TEST(GPUBackend_8_polygon_sizing_shrink)
{
  //  Test polygon sizing with negative offset (shrink)
  std::vector<db::Polygon> polygons;
  polygons.push_back (db::Polygon (db::Box (0, 0, 1000, 1000)));

  std::vector<db::Polygon> results;

  //  Shrink by 100 in all directions (square mode = 2)
  db::GPUBackend::instance ().polygon_sizing (polygons, -100, -100, 2, results);

  //  Result should be smaller than the original
  EXPECT_EQ (results.size () > size_t (0), true);
  if (results.size () > 0) {
    db::Box result_box = results[0].box ();
    EXPECT_EQ (result_box.left () >= 100, true);
    EXPECT_EQ (result_box.bottom () >= 100, true);
    EXPECT_EQ (result_box.right () <= 900, true);
    EXPECT_EQ (result_box.top () <= 900, true);
  }
}

TEST(GPUBackend_9_thresholds)
{
  //  Test threshold values
  db::GPUBackend &backend = db::GPUBackend::instance ();

  EXPECT_EQ (backend.min_polygon_count_for_gpu () > size_t (0), true);
  EXPECT_EQ (backend.min_point_count_for_gpu () > size_t (0), true);
}

TEST(GPUBackend_10_empty_inputs)
{
  //  Test handling of empty inputs
  db::GPUBackend &backend = db::GPUBackend::instance ();

  //  Empty polygon list for sizing
  std::vector<db::Polygon> empty_polygons;
  std::vector<db::Polygon> sizing_results;
  backend.polygon_sizing (empty_polygons, 100, 100, 2, sizing_results);
  EXPECT_EQ (sizing_results.size (), size_t (0));

  //  Empty point list for point-in-polygon
  db::Polygon square (db::Box (0, 0, 100, 100));
  std::vector<db::Point> empty_points;
  std::vector<int> pip_results;
  backend.batch_point_in_polygon (square, empty_points, pip_results);
  EXPECT_EQ (pip_results.size (), size_t (0));

  //  Empty polygon list for bounding boxes
  std::vector<db::Box> bbox_results;
  backend.batch_bounding_boxes (empty_polygons, bbox_results);
  EXPECT_EQ (bbox_results.size (), size_t (0));
}
