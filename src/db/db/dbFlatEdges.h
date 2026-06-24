
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


#ifndef HDR_dbFlatEdges
#define HDR_dbFlatEdges

#include "dbCommon.h"

#include "dbMutableEdges.h"
#include "dbShapes.h"
#include "dbShapes2.h"
#include "dbGenericShapeIterator.h"
#include "tlCopyOnWrite.h"

namespace db {

/**
 *  @brief An iterator delegate for the flat edge set
 */
typedef generic_shapes_iterator_delegate<db::Edge> FlatEdgesIterator;

/**
 *  @brief A flat, edge-set delegate
 */
class DB_PUBLIC FlatEdges
  : public MutableEdges
{
public:
  typedef db::Edge value_type;

  typedef db::layer<db::Edge, db::unstable_layer_tag> edge_layer_type;
  typedef edge_layer_type::iterator edge_iterator_type;
  typedef db::layer<db::EdgeWithProperties, db::unstable_layer_tag> edge_layer_wp_type;
  typedef edge_layer_wp_type::iterator edge_iterator_wp_type;

  FlatEdges ();
  FlatEdges (const db::Shapes &edges, bool is_merged);
  FlatEdges (bool is_merged);

  FlatEdges (const FlatEdges &other);

  ~FlatEdges () override;

  EdgesDelegate *clone () const override
  {
    return new FlatEdges (*this);
  }

  void reserve (size_t) override;
  void flatten () override { }

  EdgesIteratorDelegate *begin () const override;
  EdgesIteratorDelegate *begin_merged () const override;

  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_iter () const override;
  std::pair<db::RecursiveShapeIterator, db::ICplxTrans> begin_merged_iter () const override;

  bool empty () const override;
  size_t count () const override;
  size_t hier_count () const override;
  bool is_merged () const override;

  void insert_into (Layout *layout, db::cell_index_type into_cell, unsigned int into_layer) const override;

  virtual EdgesDelegate *processed_in_place (const EdgeProcessorBase &filter);
  EdgesDelegate *filter_in_place (const EdgeFilterBase &filter) override;

  EdgesDelegate *add_in_place (const Edges &other) override;
  EdgesDelegate *add (const Edges &other) const override;

  const db::Edge *nth (size_t n) const override;
  db::properties_id_type nth_prop_id (size_t n) const override;
  bool has_valid_edges () const override;
  bool has_valid_merged_edges () const override;

  const db::RecursiveShapeIterator *iter () const override;
  void apply_property_translator (const db::PropertiesTranslator &pt) override;

  void do_insert (const db::Edge &edge, properties_id_type prop_id) override;

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

  db::Shapes &raw_edges () { return *mp_edges; }
  const db::Shapes &raw_edges () const { return *mp_edges; }

protected:
  void merged_semantics_changed () override;
  Box compute_bbox () const override;
  void invalidate_cache ();
  void set_is_merged (bool m);

private:
  friend class AsIfFlatEdges;

  FlatEdges &operator= (const FlatEdges &other);

  bool m_is_merged;
  mutable tl::copy_on_write_ptr<db::Shapes> mp_edges;
  mutable tl::copy_on_write_ptr<db::Shapes> mp_merged_edges;
  mutable bool m_merged_edges_valid;

  void init ();
  void ensure_merged_edges_valid () const;

  template <class Trans>
  void transform_generic (const Trans &trans)
  {
    if (! trans.is_unity ()) {
      db::Shapes &e = *mp_edges;
      for (edge_iterator_type p = e.template get_layer<db::Edge, db::unstable_layer_tag> ().begin (); p != e.get_layer<db::Edge, db::unstable_layer_tag> ().end (); ++p) {
        e.get_layer<db::Edge, db::unstable_layer_tag> ().replace (p, p->transformed (trans));
      }
      for (edge_iterator_wp_type p = e.template get_layer<db::EdgeWithProperties, db::unstable_layer_tag> ().begin (); p != e.get_layer<db::EdgeWithProperties, db::unstable_layer_tag> ().end (); ++p) {
        e.get_layer<db::EdgeWithProperties, db::unstable_layer_tag> ().replace (p, p->transformed (trans));
      }
      invalidate_cache ();
    }
  }
};

}

#endif

