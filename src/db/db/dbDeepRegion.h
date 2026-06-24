
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


#ifndef HDR_dbDeepRegion
#define HDR_dbDeepRegion

#include "dbCommon.h"

#include "dbMutableRegion.h"
#include "dbDeepShapeStore.h"

namespace db {

/**
 *  @brief A deep, polygon-set delegate
 */
class DB_PUBLIC DeepRegion
  : public MutableRegion, public DeepShapeCollectionDelegateBase
{
public:
  typedef db::layer<db::Polygon, db::unstable_layer_tag> polygon_layer_type;
  typedef polygon_layer_type::iterator polygon_iterator_type;

  DeepRegion ();
  DeepRegion (const db::Region &other, DeepShapeStore &dss);
  DeepRegion (const RecursiveShapeIterator &si, DeepShapeStore &dss, double area_ratio = 0.0, size_t max_vertex_count = 0);
  DeepRegion (const RecursiveShapeIterator &si, DeepShapeStore &dss, const db::ICplxTrans &trans, bool merged_semantics = true, double area_ratio = 0.0, size_t max_vertex_count = 0);

  DeepRegion (const DeepRegion &other);
  DeepRegion (const DeepLayer &dl);

  ~DeepRegion () override;

  RegionDelegate *clone () const override;

  void do_insert (const db::Polygon &polygon, db::properties_id_type prop_id) override;

  void do_transform (const db::Trans &t) override;
  void do_transform (const db::ICplxTrans &t) override;
  void do_transform (const db::IMatrix2d &t) override;
  void do_transform (const db::IMatrix3d &t) override;

  void flatten () override;

  void reserve (size_t) override;

  RegionIteratorDelegate *begin () const override;
  RegionIteratorDelegate *begin_merged () const override;
  RegionIteratorDelegate *begin_unmerged () const override;

  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override;
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_merged_iter () const override;
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_unmerged_iter () const override;

  bool empty () const override;
  bool is_merged () const override;

  const db::Polygon *nth (size_t n) const override;
  db::properties_id_type nth_prop_id (size_t n) const override;
  bool has_valid_polygons () const override;
  bool has_valid_merged_polygons () const override;

  const db::RecursiveShapeIterator *iter () const override;
  void apply_property_translator (const db::PropertiesTranslator &pt) override;

  bool equals (const Region &other) const override;
  bool less (const Region &other) const override;

  bool is_box () const override;
  size_t count () const override;
  size_t hier_count () const override;

  area_type area (const db::Box &box) const override;
  perimeter_type perimeter (const db::Box &box) const override;
  Box bbox () const override;

  std::string to_string (size_t nmax) const override;

  EdgePairsDelegate *cop_to_edge_pairs (db::CompoundRegionOperationNode &node, db::PropertyConstraint prop_constraint) override;
  RegionDelegate *cop_to_region (db::CompoundRegionOperationNode &node, db::PropertyConstraint prop_constraint) override;
  EdgesDelegate *cop_to_edges (db::CompoundRegionOperationNode &node, db::PropertyConstraint prop_constraint) override;

  RegionDelegate *and_with (const Region &other, db::PropertyConstraint property_constraint) const override;
  RegionDelegate *not_with (const Region &other, db::PropertyConstraint property_constraint) const override;
  RegionDelegate *xor_with (const Region &other, db::PropertyConstraint property_constraint) const override;
  RegionDelegate *or_with (const Region &other, db::PropertyConstraint property_constraint) const override;
  std::pair<RegionDelegate *, RegionDelegate *> andnot_with (const Region &, db::PropertyConstraint property_constraint) const override;

  RegionDelegate *add_in_place (const Region &other) override;
  RegionDelegate *add (const Region &other) const override;

  EdgePairsDelegate *grid_check (db::Coord gx, db::Coord gy) const override;
  EdgePairsDelegate *angle_check (double min, double max, bool inverse) const override;

  RegionDelegate *snapped_in_place (db::Coord gx, db::Coord gy) override
  {
    return snapped (gx, gy);
  }

  RegionDelegate *snapped (db::Coord gx, db::Coord gy) override;

  EdgesDelegate *edges (const EdgeFilterBase *filter, const db::PolygonToEdgeProcessorBase *proc) const override;

  RegionDelegate *process_in_place (const PolygonProcessorBase &filter) override;
  RegionDelegate *processed (const PolygonProcessorBase &filter) const override;
  EdgesDelegate *processed_to_edges (const PolygonToEdgeProcessorBase &filter) const override;
  EdgePairsDelegate *processed_to_edge_pairs (const PolygonToEdgePairProcessorBase &filter) const override;
  RegionDelegate *filter_in_place (const PolygonFilterBase &filter) override;
  RegionDelegate *filtered (const PolygonFilterBase &filter) const override;
  std::pair<RegionDelegate *, RegionDelegate *> filtered_pair (const PolygonFilterBase &filter) const override;

  RegionDelegate *merged_in_place () override;
  RegionDelegate *merged_in_place (bool min_coherence, unsigned int min_wc, bool join_properties_on_merge) override;

  RegionDelegate *merged () const override;
  RegionDelegate *merged (bool min_coherence, unsigned int min_wc, bool join_properties_on_merge) const override;

  RegionDelegate *sized (coord_type d, unsigned int mode) const override;
  RegionDelegate *sized (coord_type dx, coord_type dy, unsigned int mode) const override;
  RegionDelegate *sized_inside (const Region &inside, bool outside, coord_type d, int steps, unsigned int mode) const override;
  RegionDelegate *sized_inside (const Region &inside, bool outside, coord_type dx, coord_type dy, int steps, unsigned int mode) const override;

  RegionDelegate *peel (double complexity_factor) const override;

  void insert_into (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer) const override;

  RegionDelegate *nets (LayoutToNetlist *l2n, NetPropertyMode prop_mode, const tl::Variant &net_prop_name, const std::vector<const Net *> *nets) const override;

  DeepShapeCollectionDelegateBase *deep () override
  {
    return this;
  }

  void set_is_merged (bool f);

  bool merged_polygons_available () const;
  const DeepLayer &merged_deep_layer () const;

protected:
  void merged_semantics_changed () override;
  void min_coherence_changed () override;
  void join_properties_on_merge_changed () override;

  EdgePairsDelegate *run_check (db::edge_relation_type rel, bool different_polygons, const Region *other, db::Coord d, const RegionCheckOptions &options) const override;
  EdgePairsDelegate *run_single_polygon_check (db::edge_relation_type rel, db::Coord d, const RegionCheckOptions &options) const override;
  std::pair<RegionDelegate *, RegionDelegate *> selected_interacting_generic (const Region &other, int mode, bool touching, InteractingOutputMode output_mode, size_t min_count, size_t max_count) const override;
  std::pair<RegionDelegate *, RegionDelegate *> selected_interacting_generic (const Edges &other, InteractingOutputMode output_mode, size_t min_count, size_t max_count) const override;
  std::pair<RegionDelegate *, RegionDelegate *> selected_interacting_generic (const Texts &other, InteractingOutputMode output_mode, size_t min_count, size_t max_count) const override;
  RegionDelegate *pull_generic (const Region &other, int mode, bool touching) const override;
  EdgesDelegate *pull_generic (const Edges &other) const override;
  TextsDelegate *pull_generic (const Texts &other) const override;
  std::pair<RegionDelegate *, RegionDelegate *> in_and_out_generic (const Region &other, InteractingOutputMode output_mode) const override;

private:
  friend class DeepEdges;
  friend class DeepTexts;

  DeepRegion &operator= (const DeepRegion &other);

  mutable DeepLayer m_merged_polygons;
  mutable bool m_merged_polygons_valid;
  mutable size_t m_merged_polygons_boc_hash;
  mutable bool m_is_merged;

  void init ();
  void ensure_merged_polygons_valid () const;
  void ensure_unmerged_polygons_valid () const;
  DeepLayer not_with_impl (const DeepRegion *other, PropertyConstraint property_constraint) const;
  DeepLayer and_with_impl (const DeepRegion *other, PropertyConstraint property_constraint) const;
  std::pair<DeepLayer, DeepLayer> and_and_not_with (const DeepRegion *other, PropertyConstraint property_constraint) const;
  std::pair<DeepRegion *, DeepRegion *> apply_filter (const PolygonFilterBase &filter, bool with_true, bool with_false) const;
  template <class Proc>
  void configure_proc (Proc &proc) const
  {
    proc.set_description (progress_desc ());
    proc.set_report_progress (report_progress ());
    proc.set_base_verbosity (base_verbosity ());
  }
};

}

#endif

