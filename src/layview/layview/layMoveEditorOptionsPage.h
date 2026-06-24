
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

#ifndef HDR_layMoveEditorOptionsPage
#define HDR_layMoveEditorOptionsPage

#include "layviewCommon.h"
#include "layEditorOptionsPageWidget.h"
#include "layMove.h"

class QHBoxLayout;

namespace lay {
  class DecoratedLineEdit;
}

namespace lay {

class MoveEditorOptionsPage
  : public lay::EditorOptionsPageWidget
{
public:
  MoveEditorOptionsPage (lay::LayoutViewBase *view, lay::Dispatcher *dispatcher);

  std::string title () const override;
  const char *name () const override;
  int order () const override;
  void deactivated () override;
  void commit (lay::Dispatcher *dispatcher) override;
  void configure (const std::string &name, const std::string &value) override;

private:
  QHBoxLayout *mp_layout;
  lay::DecoratedLineEdit *mp_x_le, *mp_y_le;
};

}

#endif

