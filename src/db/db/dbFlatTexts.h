
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


#ifndef HDR_dbFlatTexts
#define HDR_dbFlatTexts

#include "dbCommon.h"

#include "dbMutableTexts.h"
#include "dbShapes.h"
#include "tlCopyOnWrite.h"

namespace db {

/**
 *  @brief An iterator delegate for the flat text set
 */
typedef generic_shapes_iterator_delegate<db::Text> FlatTextsIterator;

/**
 *  @brief The delegate for the actual text set implementation
 */
class DB_PUBLIC FlatTexts
  : public MutableTexts
{
public:
  typedef db::Text value_type;

  typedef db::layer<db::Text, db::unstable_layer_tag> text_layer_type;
  typedef text_layer_type::iterator text_iterator_type;

  FlatTexts ();
  FlatTexts (const db::Shapes &texts);

  FlatTexts (const FlatTexts &other);

  ~FlatTexts () override;

  TextsDelegate *clone () const override
  {
    return new FlatTexts (*this);
  }

  void reserve (size_t) override;

  TextsIteratorDelegate *begin () const override;
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override;

  bool empty () const override;
  size_t count () const override;
  size_t hier_count () const override;

  TextsDelegate *filter_in_place (const TextFilterBase &filter) override;

  TextsDelegate *add_in_place (const Texts &other) override;
  TextsDelegate *add (const Texts &other) const override;

  const db::Text *nth (size_t n) const override;
  db::properties_id_type nth_prop_id (size_t n) const override;
  bool has_valid_texts () const override;

  const db::RecursiveShapeIterator *iter () const override;
  void apply_property_translator (const db::PropertiesTranslator &pt) override;

  void insert_into (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer) const override;
  void insert_into_as_polygons (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer, db::Coord enl) const override;

  void flatten () override { }

  void do_insert (const db::Text &text, properties_id_type prop_id) override;

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

  db::Shapes &raw_texts () { return *mp_texts; }
  const db::Shapes &raw_texts () const { return *mp_texts; }

protected:
  Box compute_bbox () const override;
  void invalidate_cache ();

private:
  friend class AsIfFlatTexts;

  FlatTexts &operator= (const FlatTexts &other);

  mutable tl::copy_on_write_ptr<db::Shapes> mp_texts;

  template <class Trans>
  void transform_generic (const Trans &trans)
  {
    if (! trans.is_unity ()) {
      db::Shapes &texts = *mp_texts;
      for (text_iterator_type p = texts.template get_layer<db::Text, db::unstable_layer_tag> ().begin (); p != texts.template get_layer<db::Text, db::unstable_layer_tag> ().end (); ++p) {
        texts.get_layer<db::Text, db::unstable_layer_tag> ().replace (p, p->transformed (trans));
      }
      invalidate_cache ();
    }
  }
};

}

#endif

