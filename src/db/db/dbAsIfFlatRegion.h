
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


#ifndef HDR_dbAsIfFlatRegion
#define HDR_dbAsIfFlatRegion

#include "dbCommon.h"

#include "dbRegionDelegate.h"
#include "dbPolygon.h"
#include "dbEdge.h"
#include "dbBoxScanner.h"
#include "dbEdgePairRelations.h"

#include <set>

namespace db {

/**
 *  @brief Provides default flat implementations
 */
class DB_PUBLIC AsIfFlatRegion
  : public RegionDelegate
{
public:
  AsIfFlatRegion ();
  ~AsIfFlatRegion () override;

  bool is_box () const override;

  area_type area (const db::Box &box) const override;
  perimeter_type perimeter (const db::Box &box) const override;
  Box bbox () const override;

  std::string to_string (size_t nmax) const override;

  EdgePairsDelegate *cop_to_edge_pairs (db::CompoundRegionOperationNode &node, PropertyConstraint prop_constraint) override;
  RegionDelegate *cop_to_region (db::CompoundRegionOperationNode &node, db::PropertyConstraint prop_constraint) override;
  EdgesDelegate *cop_to_edges (db::CompoundRegionOperationNode &node, db::PropertyConstraint prop_constraint) override;

  EdgePairsDelegate *width_check (db::Coord d, const RegionCheckOptions &options) const override;
  EdgePairsDelegate *space_check (db::Coord d, const RegionCheckOptions &options) const override;
  EdgePairsDelegate *isolated_check (db::Coord d, const RegionCheckOptions &options) const override;
  EdgePairsDelegate *notch_check (db::Coord d, const RegionCheckOptions &options) const override;
  EdgePairsDelegate *enclosing_check (const Region &other, db::Coord d, const RegionCheckOptions &options) const override;
  EdgePairsDelegate *overlap_check (const Region &other, db::Coord d, const RegionCheckOptions &options) const override;
  EdgePairsDelegate *separation_check (const Region &other, db::Coord d, const RegionCheckOptions &options) const override;
  EdgePairsDelegate *inside_check (const Region &other, db::Coord d, const RegionCheckOptions &options) const override;

  EdgePairsDelegate *grid_check (db::Coord gx, db::Coord gy) const override;
  EdgePairsDelegate *angle_check (double min, double max, bool inverse) const override;

  RegionDelegate *snapped_in_place (db::Coord gx, db::Coord gy) override
  {
    return snapped (gx, gy);
  }

  RegionDelegate *snapped (db::Coord gx, db::Coord gy) override;

  RegionDelegate *scaled_and_snapped_in_place (db::Coord gx, db::Coord mx, db::Coord dx, db::Coord gy, db::Coord my, db::Coord dy) override
  {
    return scaled_and_snapped (gx, mx, dx, gy, my, dy);
  }

  RegionDelegate *scaled_and_snapped (db::Coord gx, db::Coord mx, db::Coord dx, db::Coord gy, db::Coord my, db::Coord dy) override;

  EdgesDelegate *edges (const EdgeFilterBase *filter, const db::PolygonToEdgeProcessorBase *proc) const override;

  RegionDelegate *process_in_place (const PolygonProcessorBase &filter) override
  {
    return processed (filter);
  }

  RegionDelegate *processed (const PolygonProcessorBase &filter) const override;
  EdgesDelegate *processed_to_edges (const PolygonToEdgeProcessorBase &filter) const override;
  EdgePairsDelegate *processed_to_edge_pairs (const PolygonToEdgePairProcessorBase &filter) const override;

  RegionDelegate *filter_in_place (const PolygonFilterBase &filter) override
  {
    return filtered (filter);
  }

  RegionDelegate *filtered (const PolygonFilterBase &filter) const override;
  std::pair<RegionDelegate *, RegionDelegate *> filtered_pair (const PolygonFilterBase &filter) const override;

  RegionDelegate *merged_in_place () override
  {
    return merged ();
  }

  RegionDelegate *merged_in_place (bool min_coherence, unsigned int min_wc, bool join_properties_on_merge) override
  {
    return merged (min_coherence, min_wc, join_properties_on_merge);
  }

  RegionDelegate *merged () const override
  {
    return merged (min_coherence (), 0, join_properties_on_merge ());
  }

  RegionDelegate *merged (bool min_coherence, unsigned int min_wc, bool join_properties_on_merge) const override;

  RegionDelegate *sized (coord_type d, unsigned int mode) const override;
  RegionDelegate *sized (coord_type dx, coord_type dy, unsigned int mode) const override;
  RegionDelegate *sized_inside (const Region &inside, bool outside, coord_type d, int steps, unsigned int mode) const override;
  RegionDelegate *sized_inside (const Region &inside, bool outside, coord_type dx, coord_type dy, int steps, unsigned int mode) const override;

  RegionDelegate *and_with (const Region &other, PropertyConstraint property_constraint) const override;
  RegionDelegate *not_with (const Region &other, PropertyConstraint property_constraint) const override;
  RegionDelegate *xor_with (const Region &other, PropertyConstraint prop_constraint) const override;
  RegionDelegate *or_with (const Region &other, PropertyConstraint prop_constraint) const override;
  std::pair<RegionDelegate *, RegionDelegate *> andnot_with (const Region &other, PropertyConstraint property_constraint) const override;

  RegionDelegate *add_in_place (const Region &other) override
  {
    return add (other);
  }

  RegionDelegate *add (const Region &other) const override;

  RegionDelegate *peel (double /*complexity_factor*/) const override
  {
    return const_cast<AsIfFlatRegion *> (this);
  }

  RegionDelegate *selected_outside (const Region &other) const override
  {
    return selected_interacting_generic (other, 1, false, Positive, size_t (1), std::numeric_limits<size_t>::max ()).first;
  }

  RegionDelegate *selected_not_outside (const Region &other) const override
  {
    return selected_interacting_generic (other, 1, false, Negative, size_t (1), std::numeric_limits<size_t>::max ()).first;
  }

  std::pair<RegionDelegate *, RegionDelegate *> selected_outside_pair (const Region &other) const override
  {
    return selected_interacting_generic (other, 1, false, PositiveAndNegative, size_t (1), std::numeric_limits<size_t>::max ());
  }

  RegionDelegate *selected_inside (const Region &other) const override
  {
    return selected_interacting_generic (other, -1, true, Positive, size_t (1), std::numeric_limits<size_t>::max ()).first;
  }

  RegionDelegate *selected_not_inside (const Region &other) const override
  {
    return selected_interacting_generic (other, -1, true, Negative, size_t (1), std::numeric_limits<size_t>::max ()).first;
  }

  std::pair<RegionDelegate *, RegionDelegate *> selected_inside_pair (const Region &other) const override
  {
    return selected_interacting_generic (other, -1, true, PositiveAndNegative, size_t (1), std::numeric_limits<size_t>::max ());
  }

  RegionDelegate *selected_enclosing (const Region &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, -2, false, Positive, min_count, max_count).first;
  }

  RegionDelegate *selected_not_enclosing (const Region &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, -2, false, Negative, min_count, max_count).first;
  }

  std::pair<RegionDelegate *, RegionDelegate *> selected_enclosing_pair (const Region &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, -2, false, PositiveAndNegative, min_count, max_count);
  }

  RegionDelegate *selected_interacting (const Region &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, 0, true, Positive, min_count, max_count).first;
  }

  RegionDelegate *selected_not_interacting (const Region &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, 0, true, Negative, min_count, max_count).first;
  }

  std::pair<RegionDelegate *, RegionDelegate *> selected_interacting_pair (const Region &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, 0, true, PositiveAndNegative, min_count, max_count);
  }

  RegionDelegate *selected_interacting (const Edges &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, Positive, min_count, max_count).first;
  }

  RegionDelegate *selected_not_interacting (const Edges &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, Negative, min_count, max_count).first;
  }

  std::pair<RegionDelegate *, RegionDelegate *> selected_interacting_pair (const Edges &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, PositiveAndNegative, min_count, max_count);
  }

  RegionDelegate *selected_interacting (const Texts &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, Positive, min_count, max_count).first;
  }

  RegionDelegate *selected_not_interacting (const Texts &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, Negative, min_count, max_count).first;
  }

  std::pair<RegionDelegate *, RegionDelegate *> selected_interacting_pair (const Texts &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, PositiveAndNegative, min_count, max_count);
  }

  RegionDelegate *selected_overlapping (const Region &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, 0, false, Positive, min_count, max_count).first;
  }

  RegionDelegate *selected_not_overlapping (const Region &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, 0, false, Negative, min_count, max_count).first;
  }

  std::pair<RegionDelegate *, RegionDelegate *> selected_overlapping_pair (const Region &other, size_t min_count, size_t max_count) const override
  {
    return selected_interacting_generic (other, 0, false, PositiveAndNegative, min_count, max_count);
  }

  RegionDelegate *pull_inside (const Region &other) const override
  {
    return pull_generic (other, -1, true);
  }

  RegionDelegate *pull_interacting (const Region &other) const override
  {
    return pull_generic (other, 0, true);
  }

  EdgesDelegate *pull_interacting (const Edges &other) const override
  {
    return pull_generic (other);
  }

  TextsDelegate *pull_interacting (const Texts &other) const override
  {
    return pull_generic (other);
  }

  RegionDelegate *pull_overlapping (const Region &other) const override
  {
    return pull_generic (other, 0, false);
  }

  RegionDelegate *in (const Region &other, bool invert) const override
  {
    return in_and_out_generic (other, invert ? Negative : Positive).first;
  }

  std::pair<RegionDelegate *, RegionDelegate *> in_and_out (const Region &other) const override
  {
    return in_and_out_generic (other, PositiveAndNegative);
  }

  bool equals (const Region &other) const override;
  bool less (const Region &other) const override;

  RegionDelegate *nets (LayoutToNetlist *l2n, NetPropertyMode prop_mode, const tl::Variant &net_prop_name, const std::vector<const db::Net *> *net_filter) const override;

  void insert_into (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer) const override;

protected:
  void update_bbox (const db::Box &box);
  void invalidate_bbox ();
  void merge_polygons_to (db::Shapes &output, bool min_coherence, unsigned int min_wc, bool join_properties_on_merge) const;
  RegionDelegate *and_or_not_with (bool is_and, const Region &other, PropertyConstraint property_constraint) const;

  virtual EdgePairsDelegate *run_check (db::edge_relation_type rel, bool different_polygons, const Region *other, db::Coord d, const RegionCheckOptions &options) const;
  virtual EdgePairsDelegate *run_single_polygon_check (db::edge_relation_type rel, db::Coord d, const RegionCheckOptions &options) const;
  virtual std::pair<RegionDelegate *, RegionDelegate *> selected_interacting_generic (const Region &other, int mode, bool touching, InteractingOutputMode output_mode, size_t min_count, size_t max_count) const;
  virtual std::pair<RegionDelegate *, RegionDelegate *> selected_interacting_generic (const Edges &other, InteractingOutputMode output_mode, size_t min_count, size_t max_count) const;
  virtual std::pair<RegionDelegate *, RegionDelegate *> selected_interacting_generic (const Texts &other, InteractingOutputMode output_mode, size_t min_count, size_t max_count) const;
  virtual RegionDelegate *pull_generic (const Region &other, int mode, bool touching) const;
  virtual EdgesDelegate *pull_generic (const Edges &other) const;
  virtual TextsDelegate *pull_generic (const Texts &other) const;
  virtual std::pair<RegionDelegate *, RegionDelegate *> in_and_out_generic (const Region &other, InteractingOutputMode output_mode) const;

  template <class Trans>
  static void produce_markers_for_grid_check (const db::Polygon &poly, const Trans &tr, db::Coord gx, db::Coord gy, db::Shapes &shapes);
  template <class Trans>
  static void produce_markers_for_angle_check (const db::Polygon &poly, const Trans &tr, double min, double max, bool inverse, db::Shapes &shapes);

  AsIfFlatRegion &operator= (const AsIfFlatRegion &other);
  AsIfFlatRegion (const AsIfFlatRegion &other);

private:

  mutable bool m_bbox_valid;
  mutable db::Box m_bbox;

  virtual db::Box compute_bbox () const;
  EdgePairsDelegate *space_or_isolated_check (db::Coord d, const RegionCheckOptions &options, bool isolated) const;
};

}

#endif

