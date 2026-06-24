
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


#ifndef HDR_dbDeepEdges
#define HDR_dbDeepEdges

#include "dbCommon.h"

#include "dbMutableEdges.h"
#include "dbDeepShapeStore.h"
#include "dbEdgeBoolean.h"
#include "dbEdgePairs.h"

namespace db {

class Edges;
class DeepRegion;

/**
 *  @brief Provides hierarchical edges implementation
 */
class DB_PUBLIC DeepEdges
  : public db::MutableEdges, public db::DeepShapeCollectionDelegateBase
{
public:
  DeepEdges ();
  DeepEdges (const db::Edges &other, DeepShapeStore &dss);
  DeepEdges (const RecursiveShapeIterator &si, DeepShapeStore &dss, bool as_edges = true);
  DeepEdges (const RecursiveShapeIterator &si, DeepShapeStore &dss, const db::ICplxTrans &trans, bool as_edges = true, bool merged_semantics = true);

  DeepEdges (const DeepEdges &other);
  DeepEdges (const DeepLayer &dl);

  ~DeepEdges () override;

  void do_transform (const db::Trans &t) override;
  void do_transform (const db::ICplxTrans &t) override;
  void do_transform (const db::IMatrix2d &t) override;
  void do_transform (const db::IMatrix3d &t) override;

  void flatten () override;

  void reserve (size_t n) override;

  void do_insert (const db::Edge &edge, properties_id_type prop_id) override;

  EdgesDelegate *clone () const override;

  EdgesIteratorDelegate *begin () const override;
  EdgesIteratorDelegate *begin_merged () const override;

  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override;
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_merged_iter () const override;

  bool empty () const override;
  bool is_merged () const override;

  const db::Edge *nth (size_t n) const override;
  db::properties_id_type nth_prop_id (size_t n) const override;
  bool has_valid_edges () const override;
  bool has_valid_merged_edges () const override;

  const db::RecursiveShapeIterator *iter () const override;
  void apply_property_translator (const db::PropertiesTranslator &pt) override;

  bool equals (const Edges &other) const override;
  bool less (const Edges &other) const override;

  size_t count () const override;
  size_t hier_count () const override;
  Box bbox () const override;

  DeepEdges::length_type length (const db::Box &) const override;

  std::string to_string (size_t nmax) const override;

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

  EdgesDelegate *filter_in_place (const EdgeFilterBase &filter) override;
  EdgesDelegate *filtered (const EdgeFilterBase &) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> filtered_pair (const EdgeFilterBase &filter) const override;
  EdgesDelegate *process_in_place (const EdgeProcessorBase &) override;
  EdgesDelegate *processed (const EdgeProcessorBase &) const override;
  EdgePairsDelegate *processed_to_edge_pairs (const EdgeToEdgePairProcessorBase &filter) const override;
  RegionDelegate *processed_to_polygons (const EdgeToPolygonProcessorBase &filter) const override;

  EdgesDelegate *merged_in_place () override;
  EdgesDelegate *merged () const override;

  EdgesDelegate *and_with (const Edges &other) const override;
  EdgesDelegate *not_with (const Edges &other) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> andnot_with (const Edges &) const override;

  EdgesDelegate *and_with (const Region &other) const override;
  EdgesDelegate *not_with (const Region &other) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> andnot_with (const Region &) const override;

  EdgesDelegate *xor_with (const Edges &other) const override;

  EdgesDelegate *or_with (const Edges &other) const override;

  EdgesDelegate *add_in_place (const Edges &other) override;
  EdgesDelegate *add (const Edges &other) const override;

  EdgesDelegate *intersections (const Edges &other) const override;

  EdgesDelegate *inside_part (const Region &other) const override;
  EdgesDelegate *outside_part (const Region &other) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> inside_outside_part_pair (const Region &) const override;

  RegionDelegate *extended (coord_type ext_b, coord_type ext_e, coord_type ext_o, coord_type ext_i, bool join) const override;

  EdgesDelegate *in (const Edges &, bool) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> in_and_out (const Edges &) const override;

  void insert_into (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer) const override;

  DeepShapeCollectionDelegateBase *deep () override
  {
    return this;
  }

  void set_is_merged (bool f);

  bool merged_edges_available () const;
  const DeepLayer &merged_deep_layer () const;

protected:
  void merged_semantics_changed () override;

private:
  friend class DeepRegion;

  DeepEdges &operator= (const DeepEdges &other);

  mutable DeepLayer m_merged_edges;
  mutable bool m_merged_edges_valid;
  mutable size_t m_merged_edges_boc_hash;
  bool m_is_merged;

  void init ();
  void ensure_merged_edges_valid () const;
  std::pair<DeepLayer, DeepLayer> and_or_not_with (const DeepEdges *other, EdgeBoolOp op) const;
  std::pair<DeepLayer, DeepLayer> edge_region_op (const DeepRegion *other, EdgePolygonOp::mode_t op, bool include_borders) const;
  EdgePairsDelegate *run_check (db::edge_relation_type rel, const Edges *other, db::Coord d, const db::EdgesCheckOptions &options) const;
  EdgesDelegate *pull_generic (const Edges &edges) const override;
  RegionDelegate *pull_generic (const Region &region) const override;
  EdgesDelegate *selected_interacting_generic (const Edges &edges, EdgeInteractionMode mode, bool inverse, size_t min_count, size_t max_count) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_interacting_pair_generic (const Edges &edges, EdgeInteractionMode mode, size_t min_count, size_t max_count) const override;
  EdgesDelegate *selected_interacting_generic (const Region &region, EdgeInteractionMode mode, bool inverse, size_t min_count, size_t max_count) const override;
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_interacting_pair_generic (const Region &region, EdgeInteractionMode mode, size_t min_count, size_t max_count) const override;
  EdgesDelegate *selected_interacting_generic_impl (const DeepRegion *other_deep, EdgeInteractionMode mode, bool inverse, size_t min_count, size_t max_count) const;
  std::pair<EdgesDelegate *, EdgesDelegate *> selected_interacting_pair_generic_impl (const DeepRegion *other_deep, EdgeInteractionMode mode, size_t min_count, size_t max_count) const;
  std::pair<DeepEdges *, DeepEdges *> apply_filter (const EdgeFilterBase &filter, bool with_true, bool with_false) const;

  template <class Result, class OutputContainer> OutputContainer *processed_impl (const edge_processor<Result> &filter) const;
};

}

#endif

