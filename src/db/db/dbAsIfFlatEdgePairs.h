
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


#ifndef HDR_dbAsIfFlatEdgePairs
#define HDR_dbAsIfFlatEdgePairs

#include "dbCommon.h"

#include "dbEdgePairsDelegate.h"
#include "dbEdgePairsUtils.h"

namespace db {

/**
 *  @brief Provides default flat implementations
 */
class DB_PUBLIC AsIfFlatEdgePairs
  : public EdgePairsDelegate
{
public:
  AsIfFlatEdgePairs ();
  AsIfFlatEdgePairs (const AsIfFlatEdgePairs &other);
  ~AsIfFlatEdgePairs () override;

  size_t count () const override;
  size_t hier_count () const override;
  std::string to_string (size_t) const override;
  Box bbox () const override;

  EdgePairsDelegate *filter_in_place (const EdgePairFilterBase &filter) override
  {
    return filtered (filter);
  }

  EdgePairsDelegate *filtered (const EdgePairFilterBase &) const override;
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> filtered_pair (const EdgePairFilterBase &filter) const override;

  EdgePairsDelegate *process_in_place (const EdgePairProcessorBase &proc) override
  {
    return processed (proc);
  }

  EdgePairsDelegate *processed (const EdgePairProcessorBase &proc) const override;
  RegionDelegate *processed_to_polygons (const EdgePairToPolygonProcessorBase &proc) const override;
  EdgesDelegate *processed_to_edges (const EdgePairToEdgeProcessorBase &proc) const override;

  RegionDelegate *pull_interacting (const Region &) const override;
  EdgesDelegate *pull_interacting (const Edges &) const override;
  EdgePairsDelegate *selected_interacting (const Region &other, size_t min_count, size_t max_count) const override;
  EdgePairsDelegate *selected_not_interacting (const Region &other, size_t min_count, size_t max_count) const override;
  EdgePairsDelegate *selected_interacting (const Edges &other, size_t min_count, size_t max_count) const override;
  EdgePairsDelegate *selected_not_interacting (const Edges &other, size_t min_count, size_t max_count) const override;
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> selected_interacting_pair (const Region &other, size_t min_count, size_t max_count) const override;
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> selected_interacting_pair (const Edges &other, size_t min_count, size_t max_count) const override;

  EdgePairsDelegate *selected_outside (const Region &other) const override;
  EdgePairsDelegate *selected_not_outside (const Region &other) const override;
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> selected_outside_pair (const Region &other) const override;
  EdgePairsDelegate *selected_inside (const Region &other) const override;
  EdgePairsDelegate *selected_not_inside (const Region &other) const override;
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> selected_inside_pair (const Region &other) const override;

  EdgePairsDelegate *add_in_place (const EdgePairs &other) override
  {
    return add (other);
  }

  EdgePairsDelegate *add (const EdgePairs &other) const override;

  RegionDelegate *polygons (db::Coord e) const override;
  EdgesDelegate *edges () const override;
  EdgesDelegate *first_edges () const override;
  EdgesDelegate *second_edges () const override;

  EdgePairsDelegate *in (const EdgePairs &, bool) const override;

  bool equals (const EdgePairs &other) const override;
  bool less (const EdgePairs &other) const override;

  void insert_into (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer) const override;
  void insert_into_as_polygons (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer, db::Coord enl) const override;

protected:
  void update_bbox (const db::Box &box);
  void invalidate_bbox ();
  virtual EdgesDelegate *pull_generic (const Edges &other) const;
  virtual RegionDelegate *pull_generic (const Region &other) const;
  virtual EdgePairsDelegate *selected_interacting_generic (const Edges &other, bool inverse, size_t min_count, size_t max_count) const;
  virtual std::pair<EdgePairsDelegate *, EdgePairsDelegate *> selected_interacting_pair_generic (const Edges &other, size_t min_count, size_t max_count) const;
  virtual EdgePairsDelegate *selected_interacting_generic (const Region &other, EdgePairInteractionMode mode, bool inverse, size_t min_count, size_t max_count) const;
  virtual std::pair<EdgePairsDelegate *, EdgePairsDelegate *> selected_interacting_pair_generic (const Region &other, EdgePairInteractionMode mode, size_t min_count, size_t max_count) const;

private:
  friend class DeepEdgePairs;

  AsIfFlatEdgePairs &operator= (const AsIfFlatEdgePairs &other);

  mutable bool m_bbox_valid;
  mutable db::Box m_bbox;

  virtual db::Box compute_bbox () const;
};

}

#endif

