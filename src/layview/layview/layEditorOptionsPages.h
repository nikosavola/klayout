
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

#ifndef HDR_layEditorOptionsPages
#define HDR_layEditorOptionsPages

#if defined(HAVE_QT)

#include "layviewCommon.h"
#include "layEditorOptionsPage.h"

#include "tlObjectCollection.h"

#include <QFrame>
#include <QDialog>

#include <vector>
#include <string>

class QTabWidget;
class QLabel;
class QDialogButtonBox;
class QAbstractButton;

namespace lay
{

class PluginDeclaration;
class Dispatcher;
class Plugin;
class EditorOptionsModalPages;

/**
 *  @brief The object properties tab widget
 */
class LAYVIEW_PUBLIC EditorOptionsPages
  : public QFrame, public lay::EditorOptionsPageCollection
{
Q_OBJECT

public:
  EditorOptionsPages (QWidget *parent, lay::LayoutViewBase *view, const std::vector<lay::EditorOptionsPage *> &pages);
  ~EditorOptionsPages () override;

  void unregister_page (lay::EditorOptionsPage *page) override;
  bool has_content () const override;
  bool has_modal_content () const override;
  void activate_page (lay::EditorOptionsPage *page) override;
  void make_page_current (lay::EditorOptionsPage *page) override;
  bool exec_modal (lay::EditorOptionsPage *page) override;
  std::vector<lay::EditorOptionsPage *> editor_options_pages (const lay::PluginDeclaration *plugin_declaration) override;
  std::vector<lay::EditorOptionsPage *> editor_options_pages () override;
  void activate (const lay::Plugin *plugin) override;
  lay::EditorOptionsPage *page_with_name (const std::string &name) override;

  void do_apply (bool modal);

public slots:
  void apply ();
  void setup ();

private:
  tl::weak_collection <lay::EditorOptionsPage> m_pages;
  lay::LayoutViewBase *mp_view;
  QTabWidget *mp_pages;
  EditorOptionsModalPages *mp_modal_pages;
  bool m_update_enabled;

  void update (lay::EditorOptionsPage *page);
  void focusInEvent (QFocusEvent *event) override;
};

/**
 *  @brief The object properties modal page dialog
 */
class LAYVIEW_PUBLIC EditorOptionsModalPages
  : public QDialog
{
Q_OBJECT

public:
  EditorOptionsModalPages (EditorOptionsPages *parent);
  ~EditorOptionsModalPages () override;

  int count ();
  int current_index ();
  void set_current_index (int index);
  void add_page (EditorOptionsPageWidget *page);
  void remove_page (int index);
  EditorOptionsPage *widget (int index);

private slots:
  void accept () override;
  void reject () override;
  void clicked (QAbstractButton *button);

private:
  EditorOptionsPages *mp_parent;
  QTabWidget *mp_pages;
  QFrame *mp_single_page_frame;
  EditorOptionsPageWidget *mp_single_page;
  QDialogButtonBox *mp_button_box;

  void update_title ();
};

}

#endif

#endif

