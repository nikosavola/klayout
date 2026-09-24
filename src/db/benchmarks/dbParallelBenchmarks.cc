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
#include "dbHierProcessor.h"
#include "dbRegionLocalOperations.h"

#include <benchmark/benchmark.h>

#include <cstdint>
#include <string>
#include <vector>

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

BENCHMARK (hierarchical_and)->Args ({0, 800})->Args ({1, 800})->Args ({2, 800})->Args ({4, 800})->Args ({0, 3200})->Args ({1, 3200})->Args ({2, 3200})->Args ({4, 3200})->UseRealTime ()->Unit (benchmark::kMillisecond);
BENCHMARK (edge_merge)->Arg (4096)->Arg (16384)->UseRealTime ()->Unit (benchmark::kMillisecond);

}

BENCHMARK_MAIN ();
