
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


#ifndef HDR_dbOriginalLayerRegion
#define HDR_dbOriginalLayerRegion

#include "dbCommon.h"

#include "dbAsIfFlatRegion.h"

namespace db {

class EdgesDelegate;
class RegionDelegate;
class DeepShapeStore;

/**
 *  @brief An original layerregion based on a RecursiveShapeIterator
 */
class DB_PUBLIC OriginalLayerRegion
  : public AsIfFlatRegion
{
public:
  OriginalLayerRegion ();
  OriginalLayerRegion (const OriginalLayerRegion &other);
  OriginalLayerRegion (const RecursiveShapeIterator &si, bool is_merged = false);
  OriginalLayerRegion (const RecursiveShapeIterator &si, const db::ICplxTrans &trans, bool merged_semantics, bool is_merged = false);
  ~OriginalLayerRegion () override;

  RegionDelegate *clone () const override;

  RegionIteratorDelegate *begin () const override;
  RegionIteratorDelegate *begin_merged () const override;
  RegionIteratorDelegate *begin_unmerged () const override;

  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override;
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_merged_iter () const override;
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_unmerged_iter () const override;

  bool empty () const override;

  bool is_merged () const override;
  size_t count () const override;
  size_t hier_count () const override;

  const db::Polygon *nth (size_t n) const override;
  db::properties_id_type nth_prop_id (size_t) const override;
  bool has_valid_polygons () const override;
  bool has_valid_merged_polygons () const override;

  const db::RecursiveShapeIterator *iter () const override;
  void apply_property_translator (const db::PropertiesTranslator &pt) override;

  bool equals (const Region &other) const override;
  bool less (const Region &other) const override;

  void insert_into (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer) const override;

protected:
  void merged_semantics_changed () override;
  void join_properties_on_merge_changed () override;
  void min_coherence_changed () override;

private:
  OriginalLayerRegion &operator= (const OriginalLayerRegion &other);

  bool m_is_merged{};
  mutable db::Shapes m_merged_polygons;
  mutable bool m_merged_polygons_valid{};
  mutable db::RecursiveShapeIterator m_iter;
  db::ICplxTrans m_iter_trans;

  void init ();
  void ensure_merged_polygons_valid () const;
};

}

#endif

