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

#include "tlUnitTest.h"
#include "tlLog.h"
#include "dbHierProcessor.h"
#include "dbRegionLocalOperations.h"

#include <algorithm>
#include <chrono>
#include <mutex>
#include <set>
#include <thread>

namespace {

const int benchmark_cells = 64;

class ObservedAndOperation : public db::BoolAndOrNotLocalOperation
{
public:
  ObservedAndOperation () : db::BoolAndOrNotLocalOperation (true) { }

  void do_compute_local (db::Layout *layout, db::Cell *cell, const db::shape_interactions<db::PolygonRef, db::PolygonRef> &interactions, std::vector<std::unordered_set<db::PolygonRef> > &results, const db::LocalProcessorBase *proc) const override
  {
    // Keep the tally lock out of the geometry work.
    {
      std::lock_guard<std::mutex> lock (m_mutex);
      m_workers.insert (std::this_thread::get_id ());
    }
    db::BoolAndOrNotLocalOperation::do_compute_local (layout, cell, interactions, results, proc);
  }

  size_t workers () const
  {
    return m_workers.size ();
  }

private:
  mutable std::mutex m_mutex;
  mutable std::set<std::thread::id> m_workers;
};

struct BenchmarkResult
{
  std::vector<std::pair<std::string, std::string> > shapes;
  std::vector<std::pair<std::string, std::string> > expected;
  double seconds;
  size_t workers;
};

BenchmarkResult run_hierarchical_and (unsigned int threads, int boxes_per_cell)
{
  db::Layout layout;
  const unsigned int subject = layout.insert_layer (db::LayerProperties (1, 0));
  const unsigned int intruder = layout.insert_layer (db::LayerProperties (2, 0));
  const unsigned int output = layout.insert_layer (db::LayerProperties (3, 0));
  db::Cell &top = layout.cell (layout.add_cell ("TOP"));
  BenchmarkResult result;

  for (int c = 0; c < benchmark_cells; ++c) {
    const std::string name = tl::sprintf ("LEAF_%d", c);
    db::Cell &leaf = layout.cell (layout.add_cell (name.c_str ()));
    top.insert (db::CellInstArray (db::CellInst (leaf.cell_index ()), db::Trans (0, false, db::Vector (c * 2000, 0))));
    for (int b = 0; b < boxes_per_cell; ++b) {
      const int x = (b % 10) * 20;
      const int y = (b / 10) * 20;
      db::Polygon expected (db::Box (x, y, x + 10, y + 10));
      db::PolygonRef polygon (expected, layout.shape_repository ());
      leaf.shapes (subject).insert (polygon);
      leaf.shapes (intruder).insert (polygon);
      result.expected.push_back (std::make_pair (name, expected.to_string ()));
    }
  }

  ObservedAndOperation op;
  db::local_processor<db::PolygonRef, db::PolygonRef, db::PolygonRef> proc (&layout, &top);
  proc.set_threads (threads);
  proc.set_report_progress (false);
  std::vector<unsigned int> intruders (1, intruder);
  std::vector<unsigned int> outputs (1, output);

  const auto start = std::chrono::steady_clock::now ();
  proc.run (&op, subject, intruders, outputs);
  const double seconds = std::chrono::duration<double> (std::chrono::steady_clock::now () - start).count ();

  result.seconds = seconds;
  result.workers = op.workers ();
  for (db::Layout::iterator cell = layout.begin (); cell != layout.end (); ++cell) {
    for (db::Shapes::shape_iterator shape = cell->shapes (output).begin (db::ShapeIterator::Polygons); ! shape.at_end (); ++shape) {
      db::Polygon polygon;
      shape->polygon (polygon);
      result.shapes.push_back (std::make_pair (layout.cell_name (cell->cell_index ()), polygon.to_string ()));
    }
  }
  std::sort (result.shapes.begin (), result.shapes.end ());
  std::sort (result.expected.begin (), result.expected.end ());
  return result;
}

void check_hierarchical_and (tl::TestBase *_this, int boxes_per_cell, bool report)
{
  BenchmarkResult serial = run_hierarchical_and (0, boxes_per_cell);
  BenchmarkResult parallel = run_hierarchical_and (4, boxes_per_cell);

  EXPECT_EQ (serial.shapes.size (), size_t (benchmark_cells * boxes_per_cell));
  EXPECT (serial.shapes == serial.expected);
  EXPECT (serial.shapes == parallel.shapes);
  EXPECT_EQ (serial.workers, size_t (1));
  if (std::thread::hardware_concurrency () > 1) {
    EXPECT (parallel.workers > 1);
  }
  if (report) {
    tl::info << tl::sprintf ("Hierarchical AND: serial %.3fs, four threads %.3fs, %.2fx, workers %d", serial.seconds, parallel.seconds, serial.seconds / parallel.seconds, int (parallel.workers));
  }
}

}

TEST(ParallelHierarchicalAndCorrectness)
{
  check_hierarchical_and (_this, 800, false);
}

TEST(ParallelHierarchicalAndBenchmark)
{
  test_is_long_runner ();
  check_hierarchical_and (_this, 3200, true);
}
