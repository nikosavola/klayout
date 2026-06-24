
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


#ifndef HDR_dbDeepTexts
#define HDR_dbDeepTexts

#include "dbCommon.h"

#include "dbMutableTexts.h"
#include "dbDeepShapeStore.h"
#include "dbTexts.h"

namespace db {

/**
 *  @brief Provides hierarchical edges implementation
 */
class DB_PUBLIC DeepTexts
  : public db::MutableTexts, public db::DeepShapeCollectionDelegateBase
{
public:
  DeepTexts ();
  DeepTexts (const db::Texts &other, DeepShapeStore &dss);
  DeepTexts (const RecursiveShapeIterator &si, DeepShapeStore &dss);
  DeepTexts (const RecursiveShapeIterator &si, DeepShapeStore &dss, const db::ICplxTrans &trans);

  DeepTexts (const DeepTexts &other);
  DeepTexts (const DeepLayer &dl);

  ~DeepTexts () override;

  TextsDelegate *clone () const override;

  void do_insert (const db::Text &text, properties_id_type prop_id) override;

  void do_transform (const db::Trans &t) override;
  void do_transform (const db::ICplxTrans &t) override;
  void do_transform (const db::IMatrix2d &t) override;
  void do_transform (const db::IMatrix3d &t) override;

  void flatten () override;

  void reserve (size_t n) override;

  TextsIteratorDelegate *begin () const override;
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override;

  size_t count () const override;
  size_t hier_count () const override;
  std::string to_string (size_t) const override;
  Box bbox () const override;
  bool empty () const override;
  const db::Text *nth (size_t n) const override;
  db::properties_id_type nth_prop_id (size_t n) const override;
  bool has_valid_texts () const override;
  const db::RecursiveShapeIterator *iter () const override;
  void apply_property_translator (const db::PropertiesTranslator &pt) override;

  TextsDelegate *filter_in_place (const TextFilterBase &filter) override;
  TextsDelegate *filtered (const TextFilterBase &) const override;
  std::pair<TextsDelegate *, TextsDelegate *> filtered_pair (const TextFilterBase &filter) const override;

  TextsDelegate *process_in_place (const TextProcessorBase &) override;
  TextsDelegate *processed (const TextProcessorBase &) const override;
  RegionDelegate *processed_to_polygons (const TextToPolygonProcessorBase &filter) const override;

  TextsDelegate *add_in_place (const Texts &other) override;
  TextsDelegate *add (const Texts &other) const override;

  RegionDelegate *polygons (db::Coord e, const tl::Variant &text_prop) const override;
  EdgesDelegate *edges () const override;

  TextsDelegate *in (const Texts &, bool) const override;

  bool equals (const Texts &other) const override;
  bool less (const Texts &other) const override;

  void insert_into (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer) const override;
  void insert_into_as_polygons (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer, db::Coord enl) const override;

  DeepShapeCollectionDelegateBase *deep () override
  {
    return this;
  }

private:
  DeepTexts &operator= (const DeepTexts &other);

  void init ();
  std::pair<DeepTexts *, DeepTexts *> apply_filter (const TextFilterBase &filter, bool with_true, bool with_false) const;

  TextsDelegate *selected_interacting_generic (const Region &other, bool inverse) const override;
  RegionDelegate *pull_generic (const Region &other) const override;
};

}

#endif

