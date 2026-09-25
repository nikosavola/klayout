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

#include "dbEdgeProcessor.h"
#include "dbCompoundOperation.h"
#include "dbHierNetworkProcessor.h"
#include "dbHierProcessor.h"
#include "dbNetlistCompare.h"
#include "dbNetlistDeviceClasses.h"
#include "dbPLCConvexDecomposition.h"
#include "dbPolygonTools.h"
#include "dbRegion.h"
#include "dbRegionLocalOperations.h"
#include "dbShapes.h"

#include <benchmark/benchmark.h>

#include <cstdint>
#include <string>
#include <vector>

#if defined(_OPENMP)
#include <omp.h>
#endif

namespace {

class HierarchicalAndFixture
{
public:
  explicit HierarchicalAndFixture (int boxes_per_cell)
    : m_subject (m_layout.insert_layer (db::LayerProperties (1, 0))),
      m_intruder (m_layout.insert_layer (db::LayerProperties (2, 0))),
      m_output (m_layout.insert_layer (db::LayerProperties (3, 0))),
      m_top (m_layout.add_cell ("TOP")),
      m_boxes_per_cell (boxes_per_cell)
  {
    for (int c = 0; c < 64; ++c) {
      const std::string name = "LEAF_" + std::to_string (c);
      db::Cell &leaf = m_layout.cell (m_layout.add_cell (name.c_str ()));
      m_layout.cell (m_top).insert (db::CellInstArray (db::CellInst (leaf.cell_index ()), db::Trans (0, false, db::Vector (c * 2000, 0))));
      for (int b = 0; b < m_boxes_per_cell; ++b) {
        const int x = (b % 10) * 20;
        const int y = (b / 10) * 20;
        const db::Polygon polygon (db::Box (x, y, x + 10, y + 10));
        const db::PolygonRef ref (polygon, m_layout.shape_repository ());
        leaf.shapes (m_subject).insert (ref);
        leaf.shapes (m_intruder).insert (ref);
      }
    }
  }

  void reset ()
  {
    m_layout.clear_layer (m_output);
  }

  void run (unsigned int threads)
  {
    db::BoolAndOrNotLocalOperation op (true);
    db::local_processor<db::PolygonRef, db::PolygonRef, db::PolygonRef> proc (&m_layout, &m_layout.cell (m_top));
    proc.set_threads (threads);
    proc.set_report_progress (false);
    proc.run (&op, m_subject, std::vector<unsigned int> (1, m_intruder), std::vector<unsigned int> (1, m_output));
  }

  size_t output_count ()
  {
    size_t count = 0;
    for (db::Layout::iterator cell = m_layout.begin (); cell != m_layout.end (); ++cell) {
      count += cell->shapes (m_output).size ();
    }
    return count;
  }

  size_t input_count () const
  {
    return size_t (64 * m_boxes_per_cell);
  }

private:
  db::Layout m_layout;
  unsigned int m_subject, m_intruder, m_output;
  db::cell_index_type m_top;
  int m_boxes_per_cell;
};

void hierarchical_and (benchmark::State &state)
{
  const unsigned int threads = unsigned (state.range (0));
  HierarchicalAndFixture fixture (int (state.range (1)));

  for (auto _ : state) {
    state.PauseTiming ();
    fixture.reset ();
    state.ResumeTiming ();
    fixture.run (threads);
  }

  if (fixture.output_count () != fixture.input_count ()) {
    state.SkipWithError ("hierarchical AND produced the wrong number of polygons");
  }
  state.SetItemsProcessed (int64_t (state.iterations ()) * int64_t (fixture.input_count ()));
}

void edge_merge (benchmark::State &state)
{
  const int boxes = int (state.range (0));
  std::vector<db::Polygon> input;
  input.reserve (boxes);
  for (int b = 0; b < boxes; ++b) {
    const int x = (b % 128) * 20;
    const int y = (b / 128) * 20;
    input.push_back (db::Polygon (db::Box (x, y, x + 10, y + 10)));
  }

  size_t output_count = 0;
  for (auto _ : state) {
    db::EdgeProcessor proc;
    for (const db::Polygon &polygon : input) {
      proc.insert (polygon);
    }
    db::EdgeContainer output;
    db::MergeOp op;
    proc.process (output, op);
    output_count = output.edges ().size ();
    benchmark::DoNotOptimize (output_count);
  }

  if (output_count != size_t (4 * boxes)) {
    state.SkipWithError ("edge merge produced the wrong number of edges");
  }
  state.SetItemsProcessed (int64_t (state.iterations ()) * boxes);
}

void compound_region (benchmark::State &state, bool boolean_or)
{
  const unsigned int threads = unsigned (state.range (0));
  const int boxes_per_cell = int (state.range (1));
  db::Layout layout;
  const unsigned int subject = layout.insert_layer (db::LayerProperties (1, 0));
  const unsigned int intruder = layout.insert_layer (db::LayerProperties (2, 0));
  const db::cell_index_type top = layout.add_cell ("TOP");
  for (int c = 0; c < 32; ++c) {
    db::Cell &leaf = layout.cell (layout.add_cell (("LEAF_" + std::to_string (c)).c_str ()));
    layout.cell (top).insert (db::CellInstArray (db::CellInst (leaf.cell_index ()), db::Trans (0, false, db::Vector (c * 2000, 0))));
    for (int b = 0; b < boxes_per_cell; ++b) {
      const int x = (b % 16) * 20;
      const int y = (b / 16) * 20;
      const db::PolygonRef shape (db::Polygon (db::Box (x, y, x + 10, y + 10)), layout.shape_repository ());
      leaf.shapes (subject).insert (shape);
      leaf.shapes (intruder).insert (shape);
    }
  }

  db::DeepShapeStore store;
  store.set_threads (int (threads));
  db::Region left (db::RecursiveShapeIterator (layout, layout.cell (top), subject), store);
  db::Region right (db::RecursiveShapeIterator (layout, layout.cell (top), intruder), store);
  const size_t expected = size_t (32 * boxes_per_cell);

  for (auto _ : state) {
    db::CompoundRegionOperationPrimaryNode *primary = new db::CompoundRegionOperationPrimaryNode ();
    db::CompoundRegionOperationSecondaryNode *secondary = new db::CompoundRegionOperationSecondaryNode (&right);
    size_t count = 0;
    if (boolean_or) {
      db::CompoundRegionGeometricalBoolOperationNode op (db::CompoundRegionGeometricalBoolOperationNode::Or, primary, secondary);
      db::Region result = left.cop_to_region (op);
      count = result.count ();
    } else {
      db::CompoundRegionInteractOperationNode op (primary, secondary, 0, true, false);
      db::Region result = left.cop_to_region (op);
      count = result.count ();
    }
    benchmark::DoNotOptimize (count);
    if (count != expected) {
      state.SkipWithError ("compound region operation produced the wrong number of polygons");
      break;
    }
  }
  state.SetItemsProcessed (int64_t (state.iterations ()) * int64_t (expected));
}

void compound_bool_or (benchmark::State &state)
{
  compound_region (state, true);
}

void compound_interact (benchmark::State &state)
{
  compound_region (state, false);
}

void hierarchical_connectivity (benchmark::State &state)
{
  const int threads = int (state.range (0));
  const int cells = int (state.range (1));
  db::Layout layout;
  const unsigned int layer = layout.insert_layer (db::LayerProperties (1, 0));
  const db::cell_index_type top = layout.add_cell ("TOP");
  std::vector<db::cell_index_type> leaves;
  for (int c = 0; c < cells; ++c) {
    db::Cell &leaf = layout.cell (layout.add_cell (("LEAF_" + std::to_string (c)).c_str ()));
    leaves.push_back (leaf.cell_index ());
    for (int b = 0; b < 16; ++b) {
      const int x = (b % 4) * 20;
      const int y = (b / 4) * 20;
      leaf.shapes (layer).insert (db::PolygonRef (db::Polygon (db::Box (x, y, x + 10, y + 10)), layout.shape_repository ()));
    }
    layout.cell (top).insert (db::CellInstArray (db::CellInst (leaf.cell_index ()), db::Trans (0, false, db::Vector (c * 200, 0))));
  }
  db::Connectivity conn;
  conn.connect (layer, layer);

#if defined(_OPENMP)
  omp_set_dynamic (0);
  omp_set_num_threads (threads);
#endif

  for (auto _ : state) {
    db::hier_clusters<db::PolygonRef> clusters;
    clusters.build (layout, layout.cell (top), conn);
    size_t count = 0;
    for (db::cell_index_type leaf : leaves) {
      count += clusters.clusters_per_cell (leaf).size ();
    }
    benchmark::DoNotOptimize (count);
    if (count != size_t (cells * 16)) {
      state.SkipWithError ("hierarchical connectivity produced the wrong number of clusters");
      break;
    }
  }
  state.SetItemsProcessed (int64_t (state.iterations ()) * cells * 16);
}

void netlist_compare (benchmark::State &state)
{
  const int devices = int (state.range (0));
  std::string source = "circuit ARRAY (CENTER=C);\n";
  for (int i = 0; i < devices; ++i) {
    source += "  device RES $" + std::to_string (i + 1) + " (A=C,B=N" + std::to_string (i + 1) + ") (R=" + std::to_string (i + 1) + ");\n";
  }
  source += "end;\n";
  db::Netlist left, right;
  db::DeviceClass *left_resistor = new db::DeviceClassResistor ();
  db::DeviceClass *right_resistor = new db::DeviceClassResistor ();
  left_resistor->set_name ("RES");
  right_resistor->set_name ("RES");
  left.add_device_class (left_resistor);
  right.add_device_class (right_resistor);
  left.from_string (source.c_str ());
  right.from_string (source.c_str ());

  for (auto _ : state) {
    db::NetlistComparer comparer;
    comparer.set_dont_consider_net_names (true);
    const bool equal = comparer.compare (&left, &right);
    benchmark::DoNotOptimize (equal);
    if (! equal) {
      state.SkipWithError ("identical netlists did not compare equal");
      break;
    }
  }
  state.SetItemsProcessed (int64_t (state.iterations ()) * devices);
}

void polygon_rasterize (benchmark::State &state)
{
  const int vertices = int (state.range (0));
  db::Polygon polygon;
  std::vector<db::Point> points;
  points.reserve (size_t (vertices));
  for (int i = 0; i < vertices / 2; ++i) {
    points.push_back (db::Point (i * 2, i % 2 == 0 ? 0 : 1));
  }
  for (int i = vertices / 2 - 1; i >= 0; --i) {
    points.push_back (db::Point (i * 2, 200 + (i % 2)));
  }
  polygon.assign_hull (points.begin (), points.end ());

  for (auto _ : state) {
    db::AreaMap map (db::Point (-10, -10), db::Vector (10, 10), size_t (vertices / 10 + 3), 24);
    const bool changed = db::rasterize (polygon, map);
    benchmark::DoNotOptimize (changed);
    if (! changed) {
      state.SkipWithError ("polygon rasterization produced no area");
      break;
    }
  }
  state.SetItemsProcessed (int64_t (state.iterations ()) * vertices);
}

void region_merge_properties (benchmark::State &state)
{
  const int boxes = int (state.range (0));
  db::PropertiesSet properties;
  std::vector<db::properties_id_type> property_ids;
  for (int group = 0; group < 8; ++group) {
    properties.clear ();
    properties.insert (tl::Variant ("group"), group);
    property_ids.push_back (db::properties_id (properties));
  }

  db::Region region;
  for (int b = 0; b < boxes; ++b) {
    const int x = (b % 128) * 20;
    const int y = (b / 128) * 20;
    region.insert (db::BoxWithProperties (db::Box (x, y, x + 10, y + 10), property_ids[size_t (b % 8)]));
  }

  for (auto _ : state) {
    db::Region result = region.merged (false, 0, false);
    const size_t count = result.count ();
    benchmark::DoNotOptimize (count);
    if (count != size_t (boxes)) {
      state.SkipWithError ("property-aware region merge produced the wrong number of polygons");
      break;
    }
  }
  state.SetItemsProcessed (int64_t (state.iterations ()) * boxes);
}

void shapes_erase (benchmark::State &state)
{
  const int boxes = int (state.range (0));
  std::vector<db::Polygon> all, to_remove;
  all.reserve (size_t (boxes));
  to_remove.reserve (size_t (boxes / 2));
  for (int b = 0; b < boxes; ++b) {
    const int x = (b % 128) * 20;
    const int y = (b / 128) * 20;
    db::Polygon polygon (db::Box (x, y, x + 10, y + 10));
    all.push_back (polygon);
    if (b % 2 == 0) {
      to_remove.push_back (polygon);
    }
  }

  for (auto _ : state) {
    state.PauseTiming ();
    db::Shapes shapes (true);
    for (const db::Polygon &polygon : all) {
      shapes.insert (polygon);
    }
    state.ResumeTiming ();

    db::layer_op<db::Polygon, db::stable_layer_tag> operation (false, to_remove.begin (), to_remove.end ());
    operation.redo (&shapes);
    const size_t count = shapes.size ();
    benchmark::DoNotOptimize (count);
    if (count != size_t (boxes / 2)) {
      state.SkipWithError ("shape erasure produced the wrong number of polygons");
      break;
    }
  }
  state.SetItemsProcessed (int64_t (state.iterations ()) * boxes);
}

#if defined(__linux__)
void plc_decomposition (benchmark::State &state)
{
  const int contours = int (state.range (0));
  const db::Point points[] = {
    db::Point (0, 0), db::Point (0, 100), db::Point (1000, 100),
    db::Point (1000, 500), db::Point (1100, 500),
    db::Point (1100, 100), db::Point (2100, 100), db::Point (2100, 0)
  };
  db::Polygon polygon;
  polygon.assign_hull (points, points + sizeof (points) / sizeof (points[0]));
  db::plc::ConvexDecompositionParameters parameters;
  parameters.with_segments = true;

  for (auto _ : state) {
    for (int n = 0; n < contours; ++n) {
      db::plc::Graph graph;
      db::plc::ConvexDecomposition decomposition (&graph);
      decomposition.decompose (polygon, parameters, 0.001);
      const size_t count = graph.num_polygons ();
      benchmark::DoNotOptimize (count);
      if (count == 0) {
        state.SkipWithError ("PLC decomposition produced no polygons");
        break;
      }
    }
  }
  state.SetItemsProcessed (int64_t (state.iterations ()) * contours);
}
#endif

BENCHMARK (hierarchical_and)->ArgsProduct ({{0, 1, 2, 3, 4, 6, 8, 10, 12}, {800, 3200}})->UseRealTime ()->Unit (benchmark::kMillisecond);
BENCHMARK (edge_merge)->Arg (4096)->Arg (16384)->UseRealTime ()->Unit (benchmark::kMillisecond);
BENCHMARK (compound_bool_or)->ArgsProduct ({{0, 1, 2, 3, 4, 6, 8, 10, 12}, {128, 512}})->UseRealTime ()->Unit (benchmark::kMillisecond);
BENCHMARK (compound_interact)->ArgsProduct ({{0, 1, 2, 3, 4, 6, 8, 10, 12}, {128, 512}})->UseRealTime ()->Unit (benchmark::kMillisecond);
BENCHMARK (hierarchical_connectivity)->ArgsProduct ({{1, 2, 3, 4, 6, 8, 10, 12}, {64, 256}})->UseRealTime ()->Unit (benchmark::kMillisecond);
BENCHMARK (netlist_compare)->Arg (512)->Arg (2048)->UseRealTime ()->Unit (benchmark::kMillisecond);
BENCHMARK (polygon_rasterize)->Arg (2048)->Arg (8192)->UseRealTime ()->Unit (benchmark::kMillisecond);
BENCHMARK (region_merge_properties)->Arg (4096)->Arg (16384)->UseRealTime ()->Unit (benchmark::kMillisecond);
BENCHMARK (shapes_erase)->Arg (4096)->Arg (16384)->UseRealTime ()->Unit (benchmark::kMillisecond);
#if defined(__linux__)
BENCHMARK (plc_decomposition)->Arg (16)->Arg (64)->UseRealTime ()->Unit (benchmark::kMillisecond);
#endif

}

BENCHMARK_MAIN ();
