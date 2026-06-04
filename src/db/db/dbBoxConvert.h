
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



#ifndef HDR_dbBoxConvert
#define HDR_dbBoxConvert

#include "dbText.h"
#include "dbPolygon.h"
#include "dbEdge.h"
#include "dbEdgePair.h"
#include "dbUserObject.h"
#include "dbBox.h"
#include "dbPath.h"
#include "dbArray.h"
#include "dbObjectWithProperties.h"

namespace db
{

class Cell;
class CellInst;
class Layout;

/**
 *  @brief A tag class to indicate "simple" box computation
 *
 *  Using this tag as a typedef for box_convert<T>::complexity
 *  enables algorithms to simply use the box without caching it.
 */

struct DB_PUBLIC simple_bbox_tag { };

/**
 *  @brief A tag class to indicate "complex" box computation
 *
 *  Using this tag as a typedef for box_convert<T>::complexity
 *  enables algorithms to use cached bboxes, i.e. for the dbBoxTree.
 */

struct DB_PUBLIC complex_bbox_tag { };

/**
 *  @brief The generic box converter for the shapes
 *
 *  The box converter is supposed to convert a given
 *  shape (i.e. edge, polygon etc) into a box covering
 *  the shape as close as possible).
 *  This is just the template declaration. The specializations
 *  provide the correct implementation.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class Sh, bool AllowEmpty>
struct box_convert 
{
};

/**
 *  @brief The shape-with-properties box converter
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class Sh, bool AllowEmpty>
struct box_convert< db::object_with_properties<Sh>, AllowEmpty >
{
  using base_convert = db::box_convert<Sh>;

  using complexity = typename base_convert::complexity;
  using box_type = typename base_convert::box_type;

  box_type operator() (const db::object_with_properties<Sh> &s) const
  {
    return bconvert (s);
  }

  base_convert bconvert;
};

/**
 *  @brief The text box converter
 *
 *  The text is just a point, so the box returned is
 *  degenerated, but usable.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, bool AllowEmpty>
struct box_convert< db::text<C>, AllowEmpty >
{
  using text_type = db::text<C>;
  using box_type = db::box<C>;

  using complexity = simple_bbox_tag;

  box_type operator() (const text_type &t) const
  {
    return t.box ();
  }
};

/**
 *  @brief The text reference box converter
 *
 *  The text is just a point, so the box returned is
 *  degenerated, but usable.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class Text, class Trans, bool AllowEmpty>
struct box_convert< db::text_ref<Text, Trans>, AllowEmpty >
{
  using text_ref_type = db::text_ref<Text, Trans>;
  using coord_type = typename Text::coord_type;
  using box_type = db::box<coord_type>;

  using complexity = simple_bbox_tag;

  box_type operator() (const text_ref_type &t) const
  {
    return t.box ();
  }
};

/**
 *  @brief The text reference array box converter
 *
 *  The text is just a point, so the box returned is
 *  degenerated, but usable.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class Text, class Trans, class ArrayTrans, bool AllowEmpty>
struct box_convert< db::array< db::text_ref<Text, Trans>, ArrayTrans>, AllowEmpty >
{
  using text_ref_type = db::text_ref<Text, Trans>;
  using text_ref_array_type = db::array<text_ref_type, ArrayTrans>;
  using coord_type = typename Text::coord_type;
  using box_type = db::box<coord_type>;

  using complexity = simple_bbox_tag;

  box_type operator() (const text_ref_array_type &t) const
  {
    box_convert< db::text_ref<Text, Trans> > bc;
    return t.bbox (bc);
  }
};

/**
 *  @brief The path box converter
 *
 *  This maps the box to the bounding box of the polygon.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, bool AllowEmpty>
struct box_convert < db::path<C>, AllowEmpty >
{
  using path_type = db::path <C>;
  using box_type = db::box <C>;

  using complexity = complex_bbox_tag;

  box_type operator() (const path_type &p) const
  {
    return p.box ();
  }
};

/**
 *  @brief The path reference box converter
 *
 *  This maps the box to the bounding box of the polygon.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class Path, class Trans, bool AllowEmpty>
struct box_convert < db::path_ref<Path, Trans>, AllowEmpty >
{
  using path_ref_type = db::path_ref <Path, Trans>;
  using coord_type = typename Path::coord_type;
  using box_type = db::box <coord_type>;

  using complexity = complex_bbox_tag;

  box_type operator() (const path_ref_type &p) const
  {
    return p.box ();
  }
};

/**
 *  @brief The path reference array box converter
 *
 *  This maps the box to the bounding box of the polygon.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class Path, class Trans, class ArrayTrans, bool AllowEmpty>
struct box_convert < db::array< db::path_ref<Path, Trans>, ArrayTrans >, AllowEmpty >
{
  using path_ref_type = db::path_ref <Path, Trans>;
  using path_ref_array_type = db::array<path_ref_type, ArrayTrans>;
  using coord_type = typename Path::coord_type;
  using box_type = db::box <coord_type>;

  using complexity = complex_bbox_tag;

  box_type operator() (const path_ref_array_type &p) const
  {
    box_convert< db::path_ref<Path, Trans> > bc;
    return p.bbox (bc);
  }
};

/**
 *  @brief The polygon box converter
 *
 *  This maps the box to the bounding box of the polygon.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, bool AllowEmpty>
struct box_convert< db::polygon<C>, AllowEmpty >
{
  using polygon_type = db::polygon<C>;
  using box_type = db::box<C>;

  using complexity = simple_bbox_tag;

  const box_type &operator() (const polygon_type &p) const
  {
    return p.box ();
  }
};

/**
 *  @brief The simple polygon box converter
 *
 *  This maps the box to the bounding box of the polygon.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, bool AllowEmpty>
struct box_convert< db::simple_polygon<C>, AllowEmpty >
{
  using simple_polygon_type = db::simple_polygon<C>;
  using box_type = db::box<C>;

  using complexity = simple_bbox_tag;

  const box_type &operator() (const simple_polygon_type &p) const
  {
    return p.box ();
  }
};

/**
 *  @brief The polygon reference box converter
 *
 *  This maps the box to the bounding box of the polygon.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class Poly, class Trans, bool AllowEmpty>
struct box_convert< db::polygon_ref<Poly, Trans>, AllowEmpty >
{
  using polygon_ref_type = db::polygon_ref<Poly, Trans>;
  using coord_type = typename Poly::coord_type;
  using box_type = db::box<coord_type>;

  using complexity = simple_bbox_tag;

  box_type operator() (const polygon_ref_type &p) const
  {
    return p.box ();
  }
};

/**
 *  @brief The polygon reference box converter
 *
 *  This maps the box to the bounding box of the polygon.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class Poly, class Trans, class ArrayTrans, bool AllowEmpty>
struct box_convert< db::array< db::polygon_ref<Poly, Trans>, ArrayTrans >, AllowEmpty >
{
  using polygon_ref_type = db::polygon_ref<Poly, Trans>;
  using polygon_ref_array_type = db::array<polygon_ref_type, ArrayTrans>;
  using coord_type = typename Poly::coord_type;
  using box_type = db::box<coord_type>;

  using complexity = simple_bbox_tag;

  box_type operator() (const polygon_ref_array_type &p) const
  {
    box_convert< db::polygon_ref<Poly, Trans> > bc;
    return p.bbox (bc);
  }
};

/**
 *  @brief The point box converter
 *
 *  This maps the box to a degenerated box with just one point.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, bool AllowEmpty>
struct box_convert< db::point<C>, AllowEmpty >
{
  using point_type = db::point<C>;
  using box_type = db::box<C>;

  using complexity = simple_bbox_tag;

  box_type operator() (const point_type &p) const
  {
    return box_type (p, p);
  }
};

/**
 *  @brief The vector box converter
 *
 *  This maps the box to a degenerated box with just one point.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, bool AllowEmpty>
struct box_convert< db::vector<C>, AllowEmpty >
{
  using vector_type = db::vector<C>;
  using point_type = db::point<C>;
  using box_type = db::box<C>;

  using complexity = simple_bbox_tag;

  box_type operator() (const vector_type &p) const
  {
    return box_type (point_type () + p, point_type () + p);
  }
};

/**
 *  @brief The edge pair box converter
 *
 *  This maps the box to the bounding box of the edge pair, which may
 *  be degenerate, i.e. for horizontal edges.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, bool AllowEmpty>
struct box_convert< db::edge_pair<C>, AllowEmpty >
{
  using edge_pair_type = db::edge_pair<C>;
  using box_type = db::box<C>;

  using complexity = simple_bbox_tag;

  box_type operator() (const edge_pair_type &e) const
  {
    return e.bbox ();
  }
};

/**
 *  @brief The edge box converter
 *
 *  This maps the box to the bounding box of the edge, which may
 *  be degenerate, i.e. for horizontal edges.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, bool AllowEmpty>
struct box_convert< db::edge<C>, AllowEmpty >
{
  using edge_type = db::edge<C>;
  using box_type = db::box<C>;

  using complexity = simple_bbox_tag;

  box_type operator() (const edge_type &e) const
  {
    return box_type (e.p1 (), e.p2 ());
  }
};

/**
 *  @brief The user object box converter
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, bool AllowEmpty>
struct box_convert< db::user_object<C>, AllowEmpty >
{
  using user_object_type = db::user_object<C>;
  using box_type = db::box<C>;

  using complexity = complex_bbox_tag;

  box_type operator() (const user_object_type &r) const
  {
    return r.box ();
  }
};

/**
 *  @brief The box-to-box converter
 *
 *  This is the trivial case where a box becomes a box.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, bool AllowEmpty>
struct box_convert <db::box <C>, AllowEmpty >
{
  using box_type = db::box <C>;

  using complexity = simple_bbox_tag;

  const box_type &operator() (const box_type &b) const
  {
    return b;
  }
};

/**
 *  @brief The box array box converter
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, class ArrayTrans, bool AllowEmpty>
struct box_convert< db::array< db::box<C>, ArrayTrans >, AllowEmpty >
{
  using coord_type = C;
  using box_type = db::box<coord_type>;
  using box_array_type = db::array<box_type, ArrayTrans>;

  using complexity = simple_bbox_tag;

  box_type operator() (const box_array_type &a) const
  {
    box_convert<box_type> bc;
    return a.bbox (bc);
  }
};

/**
 *  @brief The box-to-box converter for the case of boxes with special internal representation
 *
 *  This is the trivial case where a box becomes a box.
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, class R, bool AllowEmpty>
struct box_convert <db::box <C, R>, AllowEmpty >
{
  using box_type = db::box <C>;

  using complexity = simple_bbox_tag;

  box_type operator() (const db::box<C, R> &b) const
  {
    return box_type (b);
  }
};

/**
 *  @brief The box array box converter for boxes with special internal representation
 *
 *  The AllowEmpty flag is ignored currently.
 */
template <class C, class R, class ArrayTrans, bool AllowEmpty>
struct box_convert< db::array< db::box<C, R>, ArrayTrans >, AllowEmpty >
{
  using box_type = db::box<C>;
  using box_array_type = db::array< db::box<C, R>, ArrayTrans>;

  using complexity = simple_bbox_tag;

  box_type operator() (const box_array_type &a) const
  {
    box_convert< box<C, R> > bc;
    return a.bbox (bc);
  }
};

//  Note: the implementation is delegated, so we don't need to make box_convert dllexport.
DB_PUBLIC db::Box cell_box_convert_impl (const db::Cell &c, int layer, bool allow_empty);

/**
 *  @brief The cell box converter
 *
 *  This class is used as a function to obtain the bounding box of
 *  a cell, whether layer-wise or global.
 *
 *  If AllowEmpty is false, the box converter treats empty cells as single points at 0, 0.
 */

template <bool AllowEmpty>
struct DB_PUBLIC box_convert <db::Cell, AllowEmpty>
{
  using cell_type = db::Cell;
  using box_type = db::Box;

  using complexity = complex_bbox_tag;

  box_convert ()
    : m_layer (-1)
  { }
  
  box_convert (unsigned int l)
    : m_layer (l)
  { }
  
  box_type operator() (const cell_type &c) const
  {
    return cell_box_convert_impl (c, m_layer, AllowEmpty);
  }

private:
  int m_layer;
};

//  Note: the implementation is delegated, so we don't need to make box_convert dllexport.
DB_PUBLIC db::Box cellinst_box_convert_impl (const db::CellInst &inst, const db::Layout *layout, int layer, bool allow_empty);

/**
 *  @brief The cell inst box converter
 *
 *  This class is used as a function to convert a cell instance
 *  to a box for a given layer. This requires that the per-layer 
 *  bounding boxes of the cell have been computed already.
 *
 *  If AllowEmpty is false, the box converter treats empty cells as single points at 0, 0.
 */

template <bool AllowEmpty>
struct box_convert <db::CellInst, AllowEmpty>
{
  using cell_inst_type = db::CellInst;
  using layout_type = db::Layout;
  using box_type = db::Box;

  using complexity = complex_bbox_tag;

  box_convert ()
    : mp_layout (0), m_layer (-1)
  { }
  
  box_convert (const layout_type &ly, unsigned int l)
    : mp_layout (&ly), m_layer (l)
  { }
  
  box_convert (const layout_type &ly)
    : mp_layout (&ly), m_layer (-1)
  { }
  
  box_type operator() (const cell_inst_type &t) const
  {
    return cellinst_box_convert_impl (t, mp_layout, m_layer, AllowEmpty);
  }

private:
  const layout_type *mp_layout;
  int m_layer;
};

/**
 *  @brief The cell inst array box converter
 *
 *  This class is used as a function to convert a cell instance array
 *  to a box for a given layer or for all layers. This requires that 
 *  the per-layer or overall bounding boxes of the cell have been 
 *  computed already.
 *
 *  If AllowEmpty is false, the box converter treats empty cells as single points at 0, 0.
 */

template <class ArrayTrans, bool AllowEmpty>
struct box_convert <db::array <db::CellInst, ArrayTrans>, AllowEmpty>
{
  using cell_inst_type = db::CellInst;
  using layout_type = db::Layout;
  using box_type = db::Box;

  using complexity = complex_bbox_tag;

  box_convert ()
    : m_bc ()
  { }
  
  box_convert (const layout_type &g, unsigned int l)
    : m_bc (g, l)
  { }
  
  box_convert (const layout_type &g)
    : m_bc (g)
  { }
  
  box_type operator() (const db::array <cell_inst_type, ArrayTrans> &t) const
  {
    return t.bbox (m_bc);
  }

private:
  box_convert <cell_inst_type, AllowEmpty> m_bc;
};

/**
 *  @brief The cell-inst-array-with-properties box converter
 *
 *  If AllowEmpty is false, the box converter treats empty cells as single points at 0, 0.
 */
template <class ArrayTrans, bool AllowEmpty>
struct box_convert< db::object_with_properties< db::array <db::CellInst, ArrayTrans> >, AllowEmpty>
{
  using cell_inst_array = db::array <db::CellInst, ArrayTrans>;
  using base_convert = db::box_convert<cell_inst_array, AllowEmpty>;
  using layout_type = db::Layout;

  using complexity = typename base_convert::complexity;
  using box_type = typename base_convert::box_type;

  box_convert ()
    : bconvert ()
  { }

  box_convert (const layout_type &g, unsigned int l)
    : bconvert (g, l)
  { }

  box_convert (const layout_type &g)
    : bconvert (g)
  { }

  box_type operator() (const db::object_with_properties<cell_inst_array> &s) const
  {
    return bconvert (s);
  }

  base_convert bconvert;
};

}

#endif

