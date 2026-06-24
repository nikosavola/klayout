
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


#ifndef HDR_dbEmptyEdges
#define HDR_dbEmptyEdges

#include "dbCommon.h"
#include "dbEdgesDelegate.h"
#include "dbRecursiveShapeIterator.h"

namespace db {

/**
 *  @brief An empty Edges
 */
class DB_PUBLIC EmptyEdges
  : public EdgesDelegate
{
public:
  EmptyEdges ();
  ~EmptyEdges () override;

  EmptyEdges (const EmptyEdges &other);
  EdgesDelegate *clone () const override;

  EdgesIteratorDelegate *begin () const override { return nullptr; }
  EdgesIteratorDelegate *begin_merged () const override { return nullptr; }

  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override { return std::make_pair (db::RecursiveShapeIterator (), db::ICplxTrans ()); }
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_merged_iter () const override { return std::make_pair (db::RecursiveShapeIterator (), db::ICplxTrans ()); }

  bool empty () const override { return true; }
  size_t count () const override { return 0; }
  size_t hier_count () const override { return 0; }
  std::string to_string (size_t) const override { return std::string (); }
  bool is_merged () const override { return true; }
  distance_type length (const db::Box &) const override { return 0; }
  Box bbox () const override { return db::Box (); }

  EdgePairsDelegate *width_check (db::Coord, const db::EdgesCheckOptions &) const override;
  EdgePairsDelegate *space_check (db::Coord, const db::EdgesCheckOptions &) const override;
  EdgePairsDelegate *enclosing_check (const Edges &, db::Coord, const db::EdgesCheckOptions &) const override;
  EdgePairsDelegate *overlap_check (const Edges &, db::Coord, const db::EdgesCheckOptions &) const override;
  EdgePairsDelegate *separation_check (const Edges &, db::Coord, const db::EdgesCheckOptions &) const override;
  EdgePairsDelegate *inside_check (const Edges &, db::Coord, const db::EdgesCheckOptions &) const override;

  EdgesDelegate *filter_in_place (const EdgeFilterBase &) override { return this; }
  EdgesDelegate *filtered (const EdgeFilterBase &) const override { return new EmptyEdges (); }
  std::pair<EdgesDelegate *, EdgesDelegate *> filtered_pair (const EdgeFilterBase &) const override { return std::make_pair (new EmptyEdges (), new EmptyEdges ()); }
  EdgesDelegate *process_in_place (const EdgeProcessorBase &) override { return this; }
  EdgesDelegate *processed (const EdgeProcessorBase &) const override { return new EmptyEdges (); }
  EdgePairsDelegate *processed_to_edge_pairs (const EdgeToEdgePairProcessorBase &) const override;
  RegionDelegate *processed_to_polygons (const EdgeToPolygonProcessorBase &) const override;

  EdgesDelegate *merged_in_place () override { return this; }
  EdgesDelegate *merged () const override { return new EmptyEdges (); }

  EdgesDelegate *and_with (const Edges &) const override { return new EmptyEdges (); }
  EdgesDelegate *not_with (const Edges &) const override { return new EmptyEdges (); }
  std::pair<EdgesDelegate *, EdgesDelegate *> andnot_with (const Edges &) const override { return std::make_pair (new EmptyEdges (), new EmptyEdges ()); }
  EdgesDelegate *and_with (const Region &) const override { return new EmptyEdges (); }
  EdgesDelegate *not_with (const Region &) const override { return new EmptyEdges (); }
  std::pair<EdgesDelegate *, EdgesDelegate *> andnot_with (const Region &) const override { return std::make_pair (new EmptyEdges (), new EmptyEdges ()); }
  EdgesDelegate *xor_with (const Edges &other) const override;
  EdgesDelegate *or_with (const Edges &other) const override;
  EdgesDelegate *add_in_place (const Edges &other) override;
  EdgesDelegate *add (const Edges &other) const override;
  EdgesDelegate *intersections (const Edges &) const override { return new EmptyEdges (); }

  RegionDelegate *extended (coord_type, coord_type, coord_type, coord_type, bool) const override;

  EdgesDelegate *inside_part (const Region &) const override { return new EmptyEdges (); }
  EdgesDelegate *outside_part (const Region &) const override { return new EmptyEdges (); }
  std::pair<EdgesDelegate *, EdgesDelegate *> inside_outside_part_pair (const Region &) const override { return std::make_pair (new EmptyEdges (), new EmptyEdges ()); }

  RegionDelegate *pull_interacting (const Region &) const override;
  EdgesDelegate *pull_interacting (const Edges &) const override { return new EmptyEdges (); }
  EdgesDelegate *selected_interacting (const Edges &, size_t, size_t) const override { return new EmptyEdges (); }
  EdgesDelegate *selected_not_interacting (const Edges &, size_t, size_t) const override { return new EmptyEdges (); }
  EdgesDelegate *selected_interacting (const Region &, size_t, size_t) const override { return new EmptyEdges (); }
  EdgesDelegate *selected_not_interacting (const Region &, size_t, size_t) const override { return new EmptyEdges (); }
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_interacting_pair (const Region &, size_t, size_t) const override { return std::make_pair (new EmptyEdges (), new EmptyEdges ()); }
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_interacting_pair (const Edges &, size_t, size_t) const override { return std::make_pair (new EmptyEdges (), new EmptyEdges ()); }

  EdgesDelegate *selected_outside (const Region &) const override { return new EmptyEdges (); }
  EdgesDelegate *selected_not_outside (const Region &) const override { return new EmptyEdges (); }
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_outside_pair (const Region &) const override { return std::make_pair (new EmptyEdges (), new EmptyEdges ()); }
  EdgesDelegate *selected_inside (const Region &) const override { return new EmptyEdges (); }
  EdgesDelegate *selected_not_inside (const Region &) const override { return new EmptyEdges (); }
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_inside_pair (const Region &) const override { return std::make_pair (new EmptyEdges (), new EmptyEdges ()); }
  EdgesDelegate *selected_outside (const Edges &) const override { return new EmptyEdges (); }
  EdgesDelegate *selected_not_outside (const Edges &) const override { return new EmptyEdges (); }
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_outside_pair (const Edges &) const override { return std::make_pair (new EmptyEdges (), new EmptyEdges ()); }
  EdgesDelegate *selected_inside (const Edges &) const override { return new EmptyEdges (); }
  EdgesDelegate *selected_not_inside (const Edges &) const override { return new EmptyEdges (); }
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_inside_pair (const Edges &) const override { return std::make_pair (new EmptyEdges (), new EmptyEdges ()); }

  EdgesDelegate *in (const Edges &, bool) const override { return new EmptyEdges (); }
  std::pair<EdgesDelegate *, EdgesDelegate *> in_and_out (const Edges &) const override { return std::make_pair (new EmptyEdges (), new EmptyEdges ()); }

  const db::Edge *nth (size_t) const override { tl_assert (false); }
  db::properties_id_type nth_prop_id (size_t) const override { tl_assert (false); }
  bool has_valid_edges () const override { return true; }
  bool has_valid_merged_edges () const override { return true; }

  const db::RecursiveShapeIterator *iter () const override { return nullptr; }
  void apply_property_translator (const db::PropertiesTranslator &) override { }

  bool equals (const Edges &other) const override;
  bool less (const Edges &other) const override;

  void insert_into (Layout *, db::cell_index_type, unsigned int) const override { }

private:
  EmptyEdges &operator= (const EmptyEdges &other);
};

}  // namespace db

#endif

