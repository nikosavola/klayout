
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


#ifndef HDR_dbFlatEdgePairs
#define HDR_dbFlatEdgePairs

#include "dbCommon.h"

#include "dbMutableEdgePairs.h"
#include "dbShapes.h"
#include "dbGenericShapeIterator.h"
#include "tlCopyOnWrite.h"

namespace db {

/**
 *  @brief An iterator delegate for the flat edge pair set
 */
typedef generic_shapes_iterator_delegate<db::EdgePair> FlatEdgePairsIterator;

/**
 *  @brief The delegate for the actual edge pair set implementation
 */
class DB_PUBLIC FlatEdgePairs
  : public MutableEdgePairs
{
public:
  typedef db::layer<db::EdgePair, db::unstable_layer_tag> edge_pair_layer_type;
  typedef edge_pair_layer_type::iterator edge_pair_iterator_type;

  FlatEdgePairs ();
  FlatEdgePairs (const db::Shapes &edges);

  FlatEdgePairs (const FlatEdgePairs &other);

  ~FlatEdgePairs () override;

  EdgePairsDelegate *clone () const override
  {
    return new FlatEdgePairs (*this);
  }

  void reserve (size_t) override;

  EdgePairsIteratorDelegate *begin () const override;
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override;

  bool empty () const override;
  size_t count () const override;
  size_t hier_count () const override;

  EdgePairsDelegate *filter_in_place (const EdgePairFilterBase &filter) override;

  EdgePairsDelegate *add_in_place (const EdgePairs &other) override;
  EdgePairsDelegate *add (const EdgePairs &other) const override;

  const db::EdgePair *nth (size_t n) const override;
  db::properties_id_type nth_prop_id (size_t n) const override;
  bool has_valid_edge_pairs () const override;

  const db::RecursiveShapeIterator *iter () const override;
  void apply_property_translator (const db::PropertiesTranslator &pt) override;

  void insert_into (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer) const override;
  void insert_into_as_polygons (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer, db::Coord enl) const override;

  void do_insert (const db::EdgePair &edge_pair, db::properties_id_type prop_id) override;

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

  db::Shapes &raw_edge_pairs () { return *mp_edge_pairs; }
  const db::Shapes &raw_edge_pairs () const { return *mp_edge_pairs; }

protected:
  Box compute_bbox () const override;
  void invalidate_cache ();

private:
  friend class AsIfFlatEdgePairs;

  FlatEdgePairs &operator= (const FlatEdgePairs &other);

  mutable tl::copy_on_write_ptr<db::Shapes> mp_edge_pairs;

  template <class Trans>
  void transform_generic (const Trans &trans)
  {
    if (! trans.is_unity ()) {
      db::Shapes &ep = *mp_edge_pairs;
      for (edge_pair_iterator_type p = ep.template get_layer<db::EdgePair, db::unstable_layer_tag> ().begin (); p != ep.template get_layer<db::EdgePair, db::unstable_layer_tag> ().end (); ++p) {
        ep.get_layer<db::EdgePair, db::unstable_layer_tag> ().replace (p, p->transformed (trans));
      }
      invalidate_cache ();
    }
  }
};

}

#endif

