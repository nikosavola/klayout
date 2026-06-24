
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


#ifndef HDR_dbEmptyRegion
#define HDR_dbEmptyRegion

#include "dbCommon.h"
#include "dbRegionDelegate.h"
#include "dbEmptyEdges.h"
#include "dbEmptyTexts.h"

namespace db {

/**
 *  @brief An empty Region
 */
class DB_PUBLIC EmptyRegion
  : public RegionDelegate
{
public:
  EmptyRegion ();
  ~EmptyRegion () override;

  EmptyRegion (const EmptyRegion &other);
  RegionDelegate *clone () const override;

  RegionIteratorDelegate *begin () const override { return nullptr; }
  RegionIteratorDelegate *begin_merged () const override { return nullptr; }
  RegionIteratorDelegate *begin_unmerged () const override { return nullptr; }

  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override { return std::make_pair (db::RecursiveShapeIterator (), db::ICplxTrans ()); }
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_merged_iter () const override { return std::make_pair (db::RecursiveShapeIterator (), db::ICplxTrans ()); }
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_unmerged_iter () const override { return std::make_pair (db::RecursiveShapeIterator (), db::ICplxTrans ()); }

  bool empty () const override { return true; }
  size_t count () const override { return 0; }
  size_t hier_count () const override { return 0; }
  std::string to_string (size_t) const override { return std::string (); }

  bool is_box () const override { return false; }
  bool is_merged () const override { return true; }
  area_type area (const db::Box &) const override { return 0; }
  perimeter_type perimeter (const db::Box &) const override { return 0; }

  Box bbox () const override { return Box (); }

  EdgePairsDelegate *cop_to_edge_pairs (db::CompoundRegionOperationNode &node, PropertyConstraint) override;
  RegionDelegate *cop_to_region (db::CompoundRegionOperationNode &node, PropertyConstraint) override;
  EdgesDelegate *cop_to_edges (db::CompoundRegionOperationNode &node, PropertyConstraint) override;

  EdgePairsDelegate *width_check (db::Coord, const RegionCheckOptions &) const override;
  EdgePairsDelegate *space_check (db::Coord, const RegionCheckOptions &) const override;
  EdgePairsDelegate *isolated_check (db::Coord, const RegionCheckOptions &) const override;
  EdgePairsDelegate *notch_check (db::Coord, const RegionCheckOptions &) const override;
  EdgePairsDelegate *enclosing_check (const Region &, db::Coord, const RegionCheckOptions &) const override;
  EdgePairsDelegate *overlap_check (const Region &, db::Coord, const RegionCheckOptions &) const override;
  EdgePairsDelegate *separation_check (const Region &, db::Coord, const RegionCheckOptions &) const override;
  EdgePairsDelegate *inside_check (const Region &, db::Coord, const RegionCheckOptions &) const override;
  EdgePairsDelegate *grid_check (db::Coord, db::Coord) const override;
  EdgePairsDelegate *angle_check (double, double, bool) const override;

  RegionDelegate *snapped_in_place (db::Coord, db::Coord) override { return this; }
  RegionDelegate *snapped (db::Coord, db::Coord) override { return new EmptyRegion (); }
  RegionDelegate *scaled_and_snapped_in_place (db::Coord, db::Coord, db::Coord, db::Coord, db::Coord, db::Coord) override { return this; }
  RegionDelegate *scaled_and_snapped (db::Coord, db::Coord, db::Coord, db::Coord, db::Coord, db::Coord) override { return new EmptyRegion (); }

  EdgesDelegate *edges (const EdgeFilterBase *, const PolygonToEdgeProcessorBase *) const override;
  RegionDelegate *filter_in_place (const PolygonFilterBase &) override { return this; }
  RegionDelegate *filtered (const PolygonFilterBase &) const override { return new EmptyRegion (); }
  std::pair<RegionDelegate *, RegionDelegate *> filtered_pair (const PolygonFilterBase &) const override { return std::make_pair (new EmptyRegion (), new EmptyRegion ()); }
  RegionDelegate *process_in_place (const PolygonProcessorBase &) override { return this; }
  RegionDelegate *processed (const PolygonProcessorBase &) const override { return new EmptyRegion (); }
  EdgesDelegate *processed_to_edges (const PolygonToEdgeProcessorBase &) const override;
  EdgePairsDelegate *processed_to_edge_pairs (const PolygonToEdgePairProcessorBase &) const override;

  RegionDelegate *merged_in_place () override { return this; }
  RegionDelegate *merged_in_place (bool, unsigned int, bool) override { return this; }
  RegionDelegate *merged () const override { return new EmptyRegion (); }
  RegionDelegate *merged (bool, unsigned int, bool) const override { return new EmptyRegion (); }

  RegionDelegate *sized (coord_type, unsigned int) const override { return new EmptyRegion (); }
  RegionDelegate *sized (coord_type, coord_type, unsigned int) const override { return new EmptyRegion (); }
  RegionDelegate *sized_inside (const Region &, bool, coord_type, int, unsigned int) const override { return new EmptyRegion (); }
  RegionDelegate *sized_inside (const Region &, bool, coord_type, coord_type, int, unsigned int) const override { return new EmptyRegion (); }

  RegionDelegate *and_with (const Region &, db::PropertyConstraint) const override { return new EmptyRegion (); }
  RegionDelegate *not_with (const Region &, db::PropertyConstraint) const override { return new EmptyRegion (); }
  std::pair<RegionDelegate *, RegionDelegate *> andnot_with (const Region &, db::PropertyConstraint) const override { return std::make_pair (new EmptyRegion (), new EmptyRegion ()); }
  RegionDelegate *xor_with (const Region &other, db::PropertyConstraint prop_constraint) const override;
  RegionDelegate *or_with (const Region &other, db::PropertyConstraint prop_constraint) const override;
  RegionDelegate *add_in_place (const Region &other) override;
  RegionDelegate *add (const Region &other) const override;

  RegionDelegate *peel (double /*complexity_factor*/) const override { return new EmptyRegion (); }

  RegionDelegate *selected_outside (const Region &) const override { return new EmptyRegion (); }
  RegionDelegate *selected_not_outside (const Region &) const override { return new EmptyRegion (); }
  std::pair<RegionDelegate *, RegionDelegate *> selected_outside_pair (const Region &) const override { return std::make_pair (new EmptyRegion (), new EmptyRegion ()); }
  RegionDelegate *selected_inside (const Region &) const override { return new EmptyRegion (); }
  RegionDelegate *selected_not_inside (const Region &) const override { return new EmptyRegion (); }
  std::pair<RegionDelegate *, RegionDelegate *> selected_inside_pair (const Region &) const override { return std::make_pair (new EmptyRegion (), new EmptyRegion ()); }
  RegionDelegate *selected_enclosing (const Region &, size_t, size_t) const override { return new EmptyRegion (); }
  RegionDelegate *selected_not_enclosing (const Region &, size_t, size_t) const override { return new EmptyRegion (); }
  std::pair<RegionDelegate *, RegionDelegate *> selected_enclosing_pair (const Region &, size_t, size_t) const override { return std::make_pair (new EmptyRegion (), new EmptyRegion ()); }
  RegionDelegate *selected_interacting (const Region &, size_t, size_t) const override { return new EmptyRegion (); }
  RegionDelegate *selected_not_interacting (const Region &, size_t, size_t) const override { return new EmptyRegion (); }
  std::pair<RegionDelegate *, RegionDelegate *> selected_interacting_pair (const Region &, size_t, size_t) const override { return std::make_pair (new EmptyRegion (), new EmptyRegion ()); }
  RegionDelegate *selected_interacting (const Edges &, size_t, size_t) const override { return new EmptyRegion (); }
  RegionDelegate *selected_not_interacting (const Edges &, size_t, size_t) const override { return new EmptyRegion (); }
  std::pair<RegionDelegate *, RegionDelegate *> selected_interacting_pair (const Edges &, size_t, size_t) const override { return std::make_pair (new EmptyRegion (), new EmptyRegion ()); }
  RegionDelegate *selected_interacting (const Texts &, size_t, size_t) const override { return new EmptyRegion (); }
  RegionDelegate *selected_not_interacting (const Texts &, size_t, size_t) const override { return new EmptyRegion (); }
  std::pair<RegionDelegate *, RegionDelegate *> selected_interacting_pair (const Texts &, size_t, size_t) const override { return std::make_pair (new EmptyRegion (), new EmptyRegion ()); }
  RegionDelegate *selected_overlapping (const Region &, size_t, size_t) const override { return new EmptyRegion (); }
  RegionDelegate *selected_not_overlapping (const Region &, size_t, size_t) const override { return new EmptyRegion (); }
  std::pair<RegionDelegate *, RegionDelegate *> selected_overlapping_pair (const Region &, size_t, size_t) const override { return std::make_pair (new EmptyRegion (), new EmptyRegion ()); }
  RegionDelegate *pull_inside (const Region &) const override  { return new EmptyRegion (); }
  RegionDelegate *pull_interacting (const Region &) const override  { return new EmptyRegion (); }
  EdgesDelegate *pull_interacting (const Edges &) const override  { return new EmptyEdges (); }
  TextsDelegate *pull_interacting (const Texts &) const override  { return new EmptyTexts (); }
  RegionDelegate *pull_overlapping (const Region &) const override  { return new EmptyRegion (); }
  RegionDelegate *in (const Region &, bool) const override { return new EmptyRegion (); }
  std::pair<RegionDelegate *, RegionDelegate *> in_and_out (const Region &) const override { return std::make_pair (new EmptyRegion (), new EmptyRegion ()); }

  bool has_valid_polygons () const override { return true; }
  bool has_valid_merged_polygons () const override { return true; }
  const db::Polygon *nth (size_t) const override { tl_assert (false); }
  db::properties_id_type nth_prop_id (size_t) const override { tl_assert (false); }

  const db::RecursiveShapeIterator *iter () const override { return nullptr; }
  void apply_property_translator (const db::PropertiesTranslator &) override { }

  bool equals (const Region &other) const override;
  bool less (const Region &other) const override;

  void insert_into (Layout *, db::cell_index_type, unsigned int) const override { }

  RegionDelegate *nets (LayoutToNetlist *, NetPropertyMode, const tl::Variant &, const std::vector<const db::Net *> *) const override { return new EmptyRegion (); }

private:
  EmptyRegion &operator= (const EmptyRegion &other);
};

}  // namespace db

#endif

