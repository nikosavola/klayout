
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


#ifndef HDR_dbEmptyTexts
#define HDR_dbEmptyTexts

#include "dbCommon.h"

#include "dbTextsDelegate.h"
#include "dbRecursiveShapeIterator.h"

namespace db {

/**
 *  @brief The delegate for the actual edge set implementation
 */
class DB_PUBLIC EmptyTexts
  : public TextsDelegate
{
public:
  EmptyTexts ();
  EmptyTexts (const EmptyTexts &other);

  TextsDelegate *clone () const override;

  std::string to_string (size_t) const override { return std::string (); }

  TextsIteratorDelegate *begin () const override { return nullptr; }
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override { return std::make_pair (db::RecursiveShapeIterator (), db::ICplxTrans ()); }

  bool empty () const override { return true; }
  size_t count () const override { return 0; }
  size_t hier_count () const override { return 0; }

  Box bbox () const override { return Box (); }

  TextsDelegate *filter_in_place (const TextFilterBase &) override { return this; }
  TextsDelegate *filtered (const TextFilterBase &) const override { return new EmptyTexts (); }
  std::pair<TextsDelegate *, TextsDelegate *> filtered_pair (const TextFilterBase &) const override { return std::make_pair (new EmptyTexts (), new EmptyTexts ()); }

  TextsDelegate *process_in_place (const TextProcessorBase &) override { return this; }
  TextsDelegate *processed (const TextProcessorBase &) const override { return new EmptyTexts (); }
  RegionDelegate *processed_to_polygons (const TextToPolygonProcessorBase &) const override;

  RegionDelegate *polygons (db::Coord e, const tl::Variant &text_prop) const override;
  EdgesDelegate *edges () const override;

  TextsDelegate *add_in_place (const Texts &other) override;
  TextsDelegate *add (const Texts &other) const override;

  TextsDelegate *in (const Texts &, bool) const override { return new EmptyTexts (); }

  const db::Text *nth (size_t) const override { tl_assert (false); }
  db::properties_id_type nth_prop_id (size_t) const override { tl_assert (false); }
  bool has_valid_texts () const override { return true; }

  const db::RecursiveShapeIterator *iter () const override { return nullptr; }
  void apply_property_translator (const db::PropertiesTranslator &) override { }

  bool equals (const Texts &other) const override;
  bool less (const Texts &other) const override;

  void insert_into (Layout *, db::cell_index_type, unsigned int) const override { }
  void insert_into_as_polygons (Layout *, db::cell_index_type, unsigned int, db::Coord) const override { }

  RegionDelegate *pull_interacting (const Region &) const override;
  TextsDelegate *selected_interacting (const Region &) const override;
  TextsDelegate *selected_not_interacting (const Region &) const override;

private:
  EmptyTexts &operator= (const EmptyTexts &other);
};

}

#endif

