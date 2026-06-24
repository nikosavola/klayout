
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


#ifndef HDR_dbEmptyEdgePairs
#define HDR_dbEmptyEdgePairs

#include "dbCommon.h"

#include "dbEdgePairsDelegate.h"
#include "dbRecursiveShapeIterator.h"

namespace db {

/**
 *  @brief The delegate for the actual edge set implementation
 */
class DB_PUBLIC EmptyEdgePairs
  : public EdgePairsDelegate
{
public:
  EmptyEdgePairs ();
  EmptyEdgePairs (const EmptyEdgePairs &other);

  EdgePairsDelegate *clone () const override;

  std::string to_string (size_t) const override { return std::string (); }

  EdgePairsIteratorDelegate *begin () const override { return nullptr; }
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override { return std::make_pair (db::RecursiveShapeIterator (), db::ICplxTrans ()); }

  bool empty () const override { return true; }
  size_t count () const override { return 0; }
  size_t hier_count () const override { return 0; }

  Box bbox () const override { return Box (); }

  EdgePairsDelegate *filter_in_place (const EdgePairFilterBase &) override { return this; }
  EdgePairsDelegate *filtered (const EdgePairFilterBase &) const override { return new EmptyEdgePairs (); }
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> filtered_pair (const EdgePairFilterBase &) const override { return std::make_pair (new EmptyEdgePairs (), new EmptyEdgePairs ()); }
  EdgePairsDelegate *process_in_place (const EdgePairProcessorBase &) override { return this; }
  EdgePairsDelegate *processed (const EdgePairProcessorBase &) const override { return new EmptyEdgePairs (); }
  RegionDelegate *processed_to_polygons (const EdgePairToPolygonProcessorBase &filter) const override;
  EdgesDelegate *processed_to_edges (const EdgePairToEdgeProcessorBase &filter) const override;

  RegionDelegate *pull_interacting (const Region &) const override;
  EdgesDelegate *pull_interacting (const Edges &) const override;
  EdgePairsDelegate *selected_interacting (const Region &, size_t, size_t) const override { return new EmptyEdgePairs (); }
  EdgePairsDelegate *selected_not_interacting (const Region &, size_t, size_t) const override { return new EmptyEdgePairs (); }
  EdgePairsDelegate *selected_interacting (const Edges &, size_t, size_t) const override { return new EmptyEdgePairs (); }
  EdgePairsDelegate *selected_not_interacting (const Edges &, size_t, size_t) const override { return new EmptyEdgePairs (); }
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> selected_interacting_pair (const Region &, size_t, size_t) const override { return std::make_pair (new EmptyEdgePairs (), new EmptyEdgePairs ()); }
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> selected_interacting_pair (const Edges &, size_t, size_t) const override { return std::make_pair (new EmptyEdgePairs (), new EmptyEdgePairs ()); }

  EdgePairsDelegate *selected_outside (const Region &) const override { return new EmptyEdgePairs (); }
  EdgePairsDelegate *selected_not_outside (const Region &) const override { return new EmptyEdgePairs (); }
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> selected_outside_pair (const Region &) const override { return std::make_pair (new EmptyEdgePairs (), new EmptyEdgePairs ()); }
  EdgePairsDelegate *selected_inside (const Region &) const override { return new EmptyEdgePairs (); }
  EdgePairsDelegate *selected_not_inside (const Region &) const override { return new EmptyEdgePairs (); }
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> selected_inside_pair (const Region &) const override { return std::make_pair (new EmptyEdgePairs (), new EmptyEdgePairs ()); }

  RegionDelegate *polygons (db::Coord e) const override;
  EdgesDelegate *edges () const override;
  EdgesDelegate *first_edges () const override;
  EdgesDelegate *second_edges () const override;

  EdgePairsDelegate *add_in_place (const EdgePairs &other) override;
  EdgePairsDelegate *add (const EdgePairs &other) const override;

  EdgePairsDelegate *in (const EdgePairs &, bool) const override { return new EmptyEdgePairs (); }

  const db::EdgePair *nth (size_t) const override { tl_assert (false); }
  db::properties_id_type nth_prop_id (size_t) const override { tl_assert (false); }
  bool has_valid_edge_pairs () const override { return true; }

  const db::RecursiveShapeIterator *iter () const override { return nullptr; }
  void apply_property_translator (const db::PropertiesTranslator &) override { }

  bool equals (const EdgePairs &other) const override;
  bool less (const EdgePairs &other) const override;

  void insert_into (Layout *, db::cell_index_type, unsigned int) const override { }
  void insert_into_as_polygons (Layout *, db::cell_index_type, unsigned int, db::Coord) const override { }

private:
  EmptyEdgePairs &operator= (const EmptyEdgePairs &other);
};

}

#endif

