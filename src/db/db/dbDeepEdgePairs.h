
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


#ifndef HDR_dbDeepEdgePairs
#define HDR_dbDeepEdgePairs

#include "dbCommon.h"

#include "dbMutableEdgePairs.h"
#include "dbDeepShapeStore.h"
#include "dbEdgePairs.h"

namespace db {

/**
 *  @brief Provides hierarchical edges implementation
 */
class DB_PUBLIC DeepEdgePairs
  : public db::MutableEdgePairs, public db::DeepShapeCollectionDelegateBase
{
public:
  DeepEdgePairs ();
  DeepEdgePairs (const RecursiveShapeIterator &si, DeepShapeStore &dss);
  DeepEdgePairs (const RecursiveShapeIterator &si, DeepShapeStore &dss, const db::ICplxTrans &trans);

  DeepEdgePairs (const DeepEdgePairs &other);
  DeepEdgePairs (const DeepLayer &dl);

  ~DeepEdgePairs () override;

  EdgePairsDelegate *clone () const override;

  void do_insert (const db::EdgePair &edge_pair, db::properties_id_type prop_id) override;

  void do_transform (const db::Trans &t) override;
  void do_transform (const db::ICplxTrans &t) override;
  void do_transform (const db::IMatrix2d &t) override;
  void do_transform (const db::IMatrix3d &t) override;

  void flatten () override;

  void reserve (size_t n) override;

  EdgePairsIteratorDelegate *begin () const override;
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override;

  size_t count () const override;
  size_t hier_count () const override;
  std::string to_string (size_t) const override;
  Box bbox () const override;
  bool empty () const override;
  const db::EdgePair *nth (size_t n) const override;
  db::properties_id_type nth_prop_id (size_t n) const override;
  bool has_valid_edge_pairs () const override;
  const db::RecursiveShapeIterator *iter () const override;
  void apply_property_translator (const db::PropertiesTranslator &pt) override;

  EdgePairsDelegate *filter_in_place (const EdgePairFilterBase &filter) override;
  EdgePairsDelegate *filtered (const EdgePairFilterBase &) const override;
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> filtered_pair (const EdgePairFilterBase &filter) const override;
  EdgePairsDelegate *process_in_place (const EdgePairProcessorBase &) override;
  EdgePairsDelegate *processed (const EdgePairProcessorBase &) const override;
  RegionDelegate *processed_to_polygons (const EdgePairToPolygonProcessorBase &filter) const override;
  EdgesDelegate *processed_to_edges (const EdgePairToEdgeProcessorBase &filter) const override;

  EdgePairsDelegate *add_in_place (const EdgePairs &other) override;
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

  DeepShapeCollectionDelegateBase *deep () override
  {
    return this;
  }

protected:
  EdgesDelegate *pull_generic (const Edges &other) const override;
  RegionDelegate *pull_generic (const Region &other) const override;
  EdgePairsDelegate *selected_interacting_generic (const Edges &other, bool inverse, size_t min_count, size_t max_count) const override;
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> selected_interacting_pair_generic (const Edges &other, size_t min_count, size_t max_count) const override;
  EdgePairsDelegate *selected_interacting_generic (const Region &other, EdgePairInteractionMode mode, bool inverse, size_t min_count, size_t max_count) const override;
  std::pair<EdgePairsDelegate *, EdgePairsDelegate *> selected_interacting_pair_generic (const Region &other, EdgePairInteractionMode mode, size_t min_count, size_t max_count) const override;

private:
  DeepEdgePairs &operator= (const DeepEdgePairs &other);

  void init ();
  EdgesDelegate *generic_edges (bool first, bool second) const;
  std::pair<DeepEdgePairs *, DeepEdgePairs *> apply_filter (const EdgePairFilterBase &filter, bool with_true, bool with_false) const;
};

}

#endif

