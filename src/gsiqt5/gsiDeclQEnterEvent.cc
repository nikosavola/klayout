
/*

  KLayout Layout Viewer
  Copyright (C) 2006-2018 Matthias Koefferlein

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

/**
*  @file gsiDeclQEnterEvent.cc 
*
*  DO NOT EDIT THIS FILE. 
*  This file has been created automatically
*/

#include <QEnterEvent>
#include <QEvent>
#include <QPoint>
#include <QPointF>
#include "gsiQt.h"
#include "gsiQtCommon.h"
#include "gsiDeclQtTypeTraits.h"
#include <memory>

// -----------------------------------------------------------------------
// class QEnterEvent

// QPoint QEnterEvent::globalPos()


static void _init_f_globalPos_c0 (qt_gsi::GenericMethod *decl)
{
  decl->set_return<QPoint > ();
}

static void _call_f_globalPos_c0 (const qt_gsi::GenericMethod * /*decl*/, void *cls, gsi::SerialArgs &args, gsi::SerialArgs &ret) 
{
  __SUPPRESS_UNUSED_WARNING(args);
  ret.write<QPoint > ((QPoint)((QEnterEvent *)cls)->globalPos ());
}


// int QEnterEvent::globalX()


static void _init_f_globalX_c0 (qt_gsi::GenericMethod *decl)
{
  decl->set_return<int > ();
}

static void _call_f_globalX_c0 (const qt_gsi::GenericMethod * /*decl*/, void *cls, gsi::SerialArgs &args, gsi::SerialArgs &ret) 
{
  __SUPPRESS_UNUSED_WARNING(args);
  ret.write<int > ((int)((QEnterEvent *)cls)->globalX ());
}


// int QEnterEvent::globalY()


static void _init_f_globalY_c0 (qt_gsi::GenericMethod *decl)
{
  decl->set_return<int > ();
}

static void _call_f_globalY_c0 (const qt_gsi::GenericMethod * /*decl*/, void *cls, gsi::SerialArgs &args, gsi::SerialArgs &ret) 
{
  __SUPPRESS_UNUSED_WARNING(args);
  ret.write<int > ((int)((QEnterEvent *)cls)->globalY ());
}


// const QPointF &QEnterEvent::localPos()


static void _init_f_localPos_c0 (qt_gsi::GenericMethod *decl)
{
  decl->set_return<const QPointF & > ();
}

static void _call_f_localPos_c0 (const qt_gsi::GenericMethod * /*decl*/, void *cls, gsi::SerialArgs &args, gsi::SerialArgs &ret) 
{
  __SUPPRESS_UNUSED_WARNING(args);
  ret.write<const QPointF & > ((const QPointF &)((QEnterEvent *)cls)->localPos ());
}


// QPoint QEnterEvent::pos()


static void _init_f_pos_c0 (qt_gsi::GenericMethod *decl)
{
  decl->set_return<QPoint > ();
}

static void _call_f_pos_c0 (const qt_gsi::GenericMethod * /*decl*/, void *cls, gsi::SerialArgs &args, gsi::SerialArgs &ret) 
{
  __SUPPRESS_UNUSED_WARNING(args);
  ret.write<QPoint > ((QPoint)((QEnterEvent *)cls)->pos ());
}


// const QPointF &QEnterEvent::screenPos()


static void _init_f_screenPos_c0 (qt_gsi::GenericMethod *decl)
{
  decl->set_return<const QPointF & > ();
}

static void _call_f_screenPos_c0 (const qt_gsi::GenericMethod * /*decl*/, void *cls, gsi::SerialArgs &args, gsi::SerialArgs &ret) 
{
  __SUPPRESS_UNUSED_WARNING(args);
  ret.write<const QPointF & > ((const QPointF &)((QEnterEvent *)cls)->screenPos ());
}


// const QPointF &QEnterEvent::windowPos()


static void _init_f_windowPos_c0 (qt_gsi::GenericMethod *decl)
{
  decl->set_return<const QPointF & > ();
}

static void _call_f_windowPos_c0 (const qt_gsi::GenericMethod * /*decl*/, void *cls, gsi::SerialArgs &args, gsi::SerialArgs &ret) 
{
  __SUPPRESS_UNUSED_WARNING(args);
  ret.write<const QPointF & > ((const QPointF &)((QEnterEvent *)cls)->windowPos ());
}


// int QEnterEvent::x()


static void _init_f_x_c0 (qt_gsi::GenericMethod *decl)
{
  decl->set_return<int > ();
}

static void _call_f_x_c0 (const qt_gsi::GenericMethod * /*decl*/, void *cls, gsi::SerialArgs &args, gsi::SerialArgs &ret) 
{
  __SUPPRESS_UNUSED_WARNING(args);
  ret.write<int > ((int)((QEnterEvent *)cls)->x ());
}


// int QEnterEvent::y()


static void _init_f_y_c0 (qt_gsi::GenericMethod *decl)
{
  decl->set_return<int > ();
}

static void _call_f_y_c0 (const qt_gsi::GenericMethod * /*decl*/, void *cls, gsi::SerialArgs &args, gsi::SerialArgs &ret) 
{
  __SUPPRESS_UNUSED_WARNING(args);
  ret.write<int > ((int)((QEnterEvent *)cls)->y ());
}


namespace gsi
{

static gsi::Methods methods_QEnterEvent () {
  gsi::Methods methods;
  methods += new qt_gsi::GenericMethod ("globalPos", "@brief Method QPoint QEnterEvent::globalPos()\n", true, &_init_f_globalPos_c0, &_call_f_globalPos_c0);
  methods += new qt_gsi::GenericMethod ("globalX", "@brief Method int QEnterEvent::globalX()\n", true, &_init_f_globalX_c0, &_call_f_globalX_c0);
  methods += new qt_gsi::GenericMethod ("globalY", "@brief Method int QEnterEvent::globalY()\n", true, &_init_f_globalY_c0, &_call_f_globalY_c0);
  methods += new qt_gsi::GenericMethod ("localPos", "@brief Method const QPointF &QEnterEvent::localPos()\n", true, &_init_f_localPos_c0, &_call_f_localPos_c0);
  methods += new qt_gsi::GenericMethod ("pos", "@brief Method QPoint QEnterEvent::pos()\n", true, &_init_f_pos_c0, &_call_f_pos_c0);
  methods += new qt_gsi::GenericMethod ("screenPos", "@brief Method const QPointF &QEnterEvent::screenPos()\n", true, &_init_f_screenPos_c0, &_call_f_screenPos_c0);
  methods += new qt_gsi::GenericMethod ("windowPos", "@brief Method const QPointF &QEnterEvent::windowPos()\n", true, &_init_f_windowPos_c0, &_call_f_windowPos_c0);
  methods += new qt_gsi::GenericMethod ("x", "@brief Method int QEnterEvent::x()\n", true, &_init_f_x_c0, &_call_f_x_c0);
  methods += new qt_gsi::GenericMethod ("y", "@brief Method int QEnterEvent::y()\n", true, &_init_f_y_c0, &_call_f_y_c0);
  return methods;
}

gsi::Class<QEvent> &qtdecl_QEvent ();

gsi::Class<QEnterEvent> decl_QEnterEvent (qtdecl_QEvent (), "QEnterEvent_Native",
  methods_QEnterEvent (),
  "@hide\n@alias QEnterEvent");

GSIQT_PUBLIC gsi::Class<QEnterEvent> &qtdecl_QEnterEvent () { return decl_QEnterEvent; }

}


class QEnterEvent_Adaptor : public QEnterEvent, public qt_gsi::QtObjectBase
{
public:

  virtual ~QEnterEvent_Adaptor();

  //  [adaptor ctor] QEnterEvent::QEnterEvent(const QPointF &localPos, const QPointF &windowPos, const QPointF &screenPos)
  QEnterEvent_Adaptor(const QPointF &localPos, const QPointF &windowPos, const QPointF &screenPos) : QEnterEvent(localPos, windowPos, screenPos)
  {
    qt_gsi::QtObjectBase::init (this);
  }

  
};

QEnterEvent_Adaptor::~QEnterEvent_Adaptor() { }

//  Constructor QEnterEvent::QEnterEvent(const QPointF &localPos, const QPointF &windowPos, const QPointF &screenPos) (adaptor class)

static void _init_ctor_QEnterEvent_Adaptor_5742 (qt_gsi::GenericStaticMethod *decl)
{
  static gsi::ArgSpecBase argspec_0 ("localPos");
  decl->add_arg<const QPointF & > (argspec_0);
  static gsi::ArgSpecBase argspec_1 ("windowPos");
  decl->add_arg<const QPointF & > (argspec_1);
  static gsi::ArgSpecBase argspec_2 ("screenPos");
  decl->add_arg<const QPointF & > (argspec_2);
  decl->set_return_new<QEnterEvent_Adaptor> ();
}

static void _call_ctor_QEnterEvent_Adaptor_5742 (const qt_gsi::GenericStaticMethod * /*decl*/, gsi::SerialArgs &args, gsi::SerialArgs &ret) 
{
  __SUPPRESS_UNUSED_WARNING(args);
  tl::Heap heap;
  const QPointF &arg1 = args.read<const QPointF & > (heap);
  const QPointF &arg2 = args.read<const QPointF & > (heap);
  const QPointF &arg3 = args.read<const QPointF & > (heap);
  ret.write<QEnterEvent_Adaptor *> (new QEnterEvent_Adaptor (arg1, arg2, arg3));
}


namespace gsi
{

gsi::Class<QEnterEvent> &qtdecl_QEnterEvent ();

static gsi::Methods methods_QEnterEvent_Adaptor () {
  gsi::Methods methods;
  methods += new qt_gsi::GenericStaticMethod ("new", "@brief Constructor QEnterEvent::QEnterEvent(const QPointF &localPos, const QPointF &windowPos, const QPointF &screenPos)\nThis method creates an object of class QEnterEvent.", &_init_ctor_QEnterEvent_Adaptor_5742, &_call_ctor_QEnterEvent_Adaptor_5742);
  return methods;
}

gsi::Class<QEnterEvent_Adaptor> decl_QEnterEvent_Adaptor (qtdecl_QEnterEvent (), "QEnterEvent",
  methods_QEnterEvent_Adaptor (),
  "@qt\n@brief Binding of QEnterEvent");

}
