
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


#ifndef HDR_dbAsIfFlatEdges
#define HDR_dbAsIfFlatEdges

#include "dbCommon.h"
#include "dbBoxScanner.h"
#include "dbEdgesDelegate.h"
#include "dbEdgeBoolean.h"
#include "dbEdgeProcessor.h"
#include "dbEdgesUtils.h"
#include "dbBoxScanner.h"
#include "dbPolygonTools.h"

#include <map>
#include <vector>

namespace db {

class PolygonSink;

/**
 *  @brief Provides default flat implementations
 */
class DB_PUBLIC AsIfFlatEdges
  : public EdgesDelegate
{
public:
  AsIfFlatEdges ();
  ~AsIfFlatEdges () override;

  size_t count () const override;
  size_t hier_count () const override;
  std::string to_string (size_t) const override;
  distance_type length (const db::Box &) const override;
  Box bbox () const override;

  EdgePairsDelegate *width_check (db::Coord d, const db::EdgesCheckOptions &options) const override
  {
    return run_check (db::WidthRelation, nullptr, d, options);
  }
    
  EdgePairsDelegate *space_check (db::Coord d, const db::EdgesCheckOptions &options) const override
  {
    return run_check (db::SpaceRelation, nullptr, d, options);
  }

  EdgePairsDelegate *enclosing_check (const Edges &other, db::Coord d, const db::EdgesCheckOptions &options) const override
  {
    return run_check (db::OverlapRelation, &other, d, options);
  }

  EdgePairsDelegate *overlap_check (const Edges &other, db::Coord d, const db::EdgesCheckOptions &options) const override
  {
    return run_check (db::WidthRelation, &other, d, options);
  }

  EdgePairsDelegate *separation_check (const Edges &other, db::Coord d, const db::EdgesCheckOptions &options) const override
  {
    return run_check (db::SpaceRelation, &other, d, options);
  }

  EdgePairsDelegate *inside_check (const Edges &other, db::Coord d, const db::EdgesCheckOptions &options) const override
  {
    return run_check (db::InsideRelation, &other, d, options);
  }

  EdgesDelegate *process_in_place (const EdgeProcessorBase &filter) override
  {
    return processed (filter);
  }

  EdgesDelegate *processed (const EdgeProcessorBase &filter) const override;
  EdgePairsDelegate *processed_to_edge_pairs (const EdgeToEdgePairProcessorBase &) const override;
  RegionDelegate *processed_to_polygons (const EdgeToPolygonProcessorBase &) const override;

  EdgesDelegate *filter_in_place (const EdgeFilterBase &filter) override
  {
    return filtered (filter);
  }

  EdgesDelegate *filtered (const EdgeFilterBase &) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> filtered_pair (const EdgeFilterBase &filter) const override;

  EdgesDelegate *merged_in_place () override
  {
    return merged ();
  }

  EdgesDelegate *merged () const override;

  EdgesDelegate *and_with (const Edges &other) const override;

  EdgesDelegate *not_with (const Edges &other) const override;

  std::pair<EdgesDelegate *, EdgesDelegate *> andnot_with (const Edges &other) const override;

  EdgesDelegate *and_with (const Region &other) const override;

  EdgesDelegate *not_with (const Region &other) const override;

  std::pair<EdgesDelegate *, EdgesDelegate *> andnot_with (const Region &other) const override;

  EdgesDelegate *xor_with (const Edges &other) const override;

  EdgesDelegate *or_with (const Edges &other) const override;

  EdgesDelegate *intersections (const Edges &other) const override;

  EdgesDelegate *add_in_place (const Edges &other) override
  {
    return add (other);
  }

  EdgesDelegate *add (const Edges &other) const override;

  EdgesDelegate *inside_part (const Region &other) const override
  {
    return edge_region_op (other, db::EdgePolygonOp::Inside, false /*don't include borders*/).first;
  }

  EdgesDelegate *outside_part (const Region &other) const override
  {
    return edge_region_op (other, db::EdgePolygonOp::Outside, false /*don't include borders*/).first;
  }

  std::pair<EdgesDelegate *, EdgesDelegate *> inside_outside_part_pair (const Region &other) const override
  {
    return edge_region_op (other, db::EdgePolygonOp::Both, false /*don't include borders*/);
  }

  RegionDelegate *extended (coord_type ext_b, coord_type ext_e, coord_type ext_o, coord_type ext_i, bool join) const override;

  EdgesDelegate *pull_interacting (const Edges &) const override;
  RegionDelegate *pull_interacting (const Region &) const override;
  EdgesDelegate *selected_interacting (const Edges &, size_t min_count, size_t max_count) const override;
  EdgesDelegate *selected_not_interacting (const Edges &, size_t min_count, size_t max_count) const override;
  EdgesDelegate *selected_interacting (const Region &, size_t min_count, size_t max_count) const override;
  EdgesDelegate *selected_not_interacting (const Region &, size_t min_count, size_t max_count) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_interacting_pair (const Region &other, size_t min_count, size_t max_count) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_interacting_pair (const Edges &other, size_t min_count, size_t max_count) const override;

  EdgesDelegate *selected_outside (const Edges &other) const override;
  EdgesDelegate *selected_not_outside (const Edges &other) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_outside_pair (const Edges &other) const override;
  EdgesDelegate *selected_inside (const Edges &other) const override;
  EdgesDelegate *selected_not_inside (const Edges &other) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_inside_pair (const Edges &other) const override;
  EdgesDelegate *selected_outside (const Region &other) const override;
  EdgesDelegate *selected_not_outside (const Region &other) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_outside_pair (const Region &other) const override;
  EdgesDelegate *selected_inside (const Region &other) const override;
  EdgesDelegate *selected_not_inside (const Region &other) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_inside_pair (const Region &other) const override;

  EdgesDelegate *in (const Edges &, bool) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> in_and_out (const Edges &) const override;

  bool equals (const Edges &other) const override;
  bool less (const Edges &other) const override;

  void insert_into (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer) const override;

protected:
  void update_bbox (const db::Box &box);
  void invalidate_bbox ();
  EdgePairsDelegate *run_check (db::edge_relation_type rel, const Edges *other, db::Coord d, const EdgesCheckOptions &options) const;
  virtual EdgesDelegate *pull_generic (const Edges &edges) const;
  virtual RegionDelegate *pull_generic (const Region &region) const;
  virtual EdgesDelegate *selected_interacting_generic (const Edges &edges, EdgeInteractionMode mode, bool inverse, size_t min_count, size_t max_count) const;
  virtual std::pair<EdgesDelegate *, EdgesDelegate *> selected_interacting_pair_generic (const Edges &edges, EdgeInteractionMode mode, size_t min_count, size_t max_count) const;
  virtual EdgesDelegate *selected_interacting_generic (const Region &region, EdgeInteractionMode mode, bool inverse, size_t min_count, size_t max_count) const;
  virtual std::pair<EdgesDelegate *, EdgesDelegate *> selected_interacting_pair_generic (const Region &region, EdgeInteractionMode mode, size_t min_count, size_t max_count) const;
  AsIfFlatEdges &operator= (const AsIfFlatEdges &other);
  AsIfFlatEdges (const AsIfFlatEdges &other);

private:
  mutable bool m_bbox_valid;
  mutable db::Box m_bbox;

  virtual db::Box compute_bbox () const;
  EdgesDelegate *boolean (const Edges *other, EdgeBoolOp op) const;
  std::pair<EdgesDelegate *, EdgesDelegate *> boolean_andnot (const Edges *other) const;
  std::pair<EdgesDelegate *, EdgesDelegate *> edge_region_op(const Region &other, db::EdgePolygonOp::mode_t mode, bool include_borders) const;
};

}

#endif

