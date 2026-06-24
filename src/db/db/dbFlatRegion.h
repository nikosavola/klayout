
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


#ifndef HDR_dbFlatRegion
#define HDR_dbFlatRegion

#include "dbCommon.h"

#include "dbMutableRegion.h"
#include "dbShapes.h"
#include "dbShapes2.h"
#include "tlCopyOnWrite.h"

namespace db {

/**
 *  @brief An iterator delegate for the flat region
 */
typedef generic_shapes_iterator_delegate<db::Polygon> FlatRegionIterator;

/**
 *  @brief A flat, polygon-set delegate
 */
class DB_PUBLIC FlatRegion
  : public MutableRegion
{
public:
  typedef db::Polygon value_type;
  typedef db::layer<db::Polygon, db::unstable_layer_tag> polygon_layer_type;
  typedef polygon_layer_type::iterator polygon_iterator_type;
  typedef db::layer<db::PolygonWithProperties, db::unstable_layer_tag> polygon_layer_wp_type;
  typedef polygon_layer_wp_type::iterator polygon_iterator_wp_type;

  FlatRegion (double area_ratio = 0.0, size_t max_vertex_count = 0);
  FlatRegion (const db::Shapes &polygons, bool is_merged = false, double area_ratio = 0.0, size_t max_vertex_count = 0);
  FlatRegion (const db::Shapes &polygons, const db::ICplxTrans &trans, bool merged_semantics, bool is_merged = false, double area_ratio = 0.0, size_t max_vertex_count = 0);
  FlatRegion (bool is_merged, double area_ratio = 0.0, size_t max_vertex_count = 0);

  FlatRegion (const FlatRegion &other);

  ~FlatRegion () override;

  RegionDelegate *clone () const override
  {
    return new FlatRegion (*this);
  }

  void reserve (size_t) override;

  RegionIteratorDelegate *begin () const override;
  RegionIteratorDelegate *begin_merged () const override;
  RegionIteratorDelegate *begin_unmerged () const override;

  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override;
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_merged_iter () const override;
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_unmerged_iter () const override;

  bool empty () const override;
  size_t count () const override;
  size_t hier_count () const override;
  bool is_merged () const override;

  void insert_into (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer) const override;

  RegionDelegate *merged_in_place () override;
  RegionDelegate *merged_in_place (bool min_coherence, unsigned int min_wc, bool join_properties_on_merge) override;
  RegionDelegate *merged () const override;
  RegionDelegate *merged (bool min_coherence, unsigned int min_wc, bool join_properties_on_merge) const override
  {
    return db::AsIfFlatRegion::merged (min_coherence, min_wc, join_properties_on_merge);
  }

  RegionDelegate *process_in_place (const PolygonProcessorBase &filter) override;
  RegionDelegate *filter_in_place (const PolygonFilterBase &filter) override;

  RegionDelegate *add_in_place (const Region &other) override;
  RegionDelegate *add (const Region &other) const override;

  const db::Polygon *nth (size_t n) const override;
  db::properties_id_type nth_prop_id (size_t) const override;
  bool has_valid_polygons () const override;
  bool has_valid_merged_polygons () const override;

  const db::RecursiveShapeIterator *iter () const override;
  void apply_property_translator (const db::PropertiesTranslator &pt) override;

  void do_insert (const db::Polygon &polygon, db::properties_id_type prop_id) override;

  void do_transform (const db::Trans &t) override
  {
    transform_generic (t);
  }

  void do_transform (const db::ICplxTrans &t) override
  {
    transform_generic (t);
  }

  void do_transform (const db::IMatrix2d &t) override
  {
    transform_generic (t);
  }

  void do_transform (const db::IMatrix3d &t) override
  {
    transform_generic (t);
  }

  void flatten () override { }

  db::Shapes &raw_polygons () { return *mp_polygons; }
  const db::Shapes &raw_polygons () const { return *mp_polygons; }

protected:
  void merged_semantics_changed () override;
  void join_properties_on_merge_changed () override;
  void min_coherence_changed () override;
  Box compute_bbox () const override;
  void invalidate_cache ();
  void set_is_merged (bool m);

private:
  friend class AsIfFlatRegion;
  friend class Region;

  FlatRegion &operator= (const FlatRegion &other);

  mutable bool m_is_merged;
  mutable tl::copy_on_write_ptr<db::Shapes> mp_polygons;
  mutable tl::copy_on_write_ptr<db::Shapes> mp_merged_polygons;
  mutable bool m_merged_polygons_valid;
  double m_area_ratio;
  size_t m_max_vertex_count;

  void init ();
  void ensure_merged_polygons_valid () const;
  void ensure_unmerged_polygons_valid () const;

  template <class Trans>
  void transform_generic (const Trans &trans)
  {
    if (! trans.is_unity ()) {
      db::Shapes &polygons = *mp_polygons;
      for (polygon_iterator_type p = polygons.get_layer<db::Polygon, db::unstable_layer_tag> ().begin (); p != polygons.get_layer<db::Polygon, db::unstable_layer_tag> ().end (); ++p) {
        polygons.get_layer<db::Polygon, db::unstable_layer_tag> ().replace (p, p->transformed (trans));
      }
      for (polygon_iterator_wp_type p = polygons.get_layer<db::PolygonWithProperties, db::unstable_layer_tag> ().begin (); p != polygons.get_layer<db::PolygonWithProperties, db::unstable_layer_tag> ().end (); ++p) {
        polygons.get_layer<db::PolygonWithProperties, db::unstable_layer_tag> ().replace (p, p->transformed (trans));
      }
      invalidate_cache ();
    }
  }
};

}

#endif

