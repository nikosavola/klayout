
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


#ifndef HDR_antPlugin
#define HDR_antPlugin

#include "antCommon.h"

#include "layPlugin.h"

namespace ant
{

class Template;

class PluginDeclaration
  : public lay::PluginDeclaration
{
public:
  PluginDeclaration ();
  ~PluginDeclaration () override;

  void get_options (std::vector < std::pair<std::string, std::string> > &options) const override;
  void get_menu_entries (std::vector<lay::MenuEntry> &menu_entries) const override;
  lay::Plugin *create_plugin (db::Manager *manager, lay::Dispatcher *, lay::LayoutViewBase *view) const override;
  bool implements_editable (std::string &title) const override;
  bool implements_mouse_mode (std::string &title) const override;
  bool configure (const std::string &name, const std::string &value) override;
#if defined(HAVE_QT)
  std::vector<std::pair <std::string, lay::ConfigPage *> > config_pages (QWidget *parent) const override;
#endif
  void config_finalize () override;
  void initialized (lay::Dispatcher *) override;
  void uninitialize (lay::Dispatcher *) override;
  bool menu_activated (const std::string &symbol) const override;

  std::vector<std::string> additional_editor_options_pages (lay::LayoutViewBase *view) const override;

  void register_annotation_template (const ant::Template &t, lay::Plugin *plugin = nullptr);
  void unregister_annotation_template (const std::string &category, lay::Plugin *plugin = nullptr);

  static PluginDeclaration *instance ();

private:
  void update_current_template ();
  void update_menu ();
  
  std::vector<ant::Template> m_templates;
  int m_current_template;
  tl::weak_collection<lay::ConfigureAction> m_actions;
  bool m_current_template_updated;
  bool m_templates_updated;
};

}

#endif

