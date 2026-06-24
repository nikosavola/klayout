
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

#if defined(HAVE_QT)

#ifndef HDR_layDialogs
#define HDR_layDialogs

#include "dbLayout.h"
#include "layuiCommon.h"

#include <QDialog>
#include <memory>

class QTreeWidgetItem;
class QModelIndex;

namespace lay
{
  class GenericSyntaxHighlighterAttributes;
}

namespace Ui
{
  class LayerSourceDialog;
  class NewLayoutPropertiesDialog;
  class NewLayerPropertiesDialog;
  class NewCellPropertiesDialog;
  class LayoutViewFunctionDialog;
  class MoveOptionsDialog;
  class MoveToOptionsDialog;
  class DeleteCellModeDialog;
  class CopyCellModeDialog;
  class ReplaceCellOptionsDialog;
  class ClearLayerModeDialog;
  class OpenLayoutModeDialog;
  class RenameCellDialog;
  class DuplicateLayerDialog;
  class AlignCellOptionsDialog;
  class FlattenInstOptionsDialog;
  class UserPropertiesForm;
  class UserPropertiesEditForm;
  class UndoRedoListForm;
}

namespace lay
{

class CellView;
class LayoutViewBase;

/**
 *  @brief The layer source dialog
 */
class LAYUI_PUBLIC LayerSourceDialog
  : public QDialog
{
Q_OBJECT

public:
  LayerSourceDialog (QWidget *parent);
  ~LayerSourceDialog () override;

  bool exec_dialog (std::string &s);

private:
  Ui::LayerSourceDialog *mp_ui;
};

/**
 *  @brief The new cell properties dialog
 */
class LAYUI_PUBLIC NewCellPropertiesDialog 
  : public QDialog
{
Q_OBJECT

public:
  NewCellPropertiesDialog (QWidget *parent);
  ~NewCellPropertiesDialog () override;

  bool exec_dialog (const db::Layout *layout, std::string &cell_name, double &size);

private:
  void accept () override;

  Ui::NewCellPropertiesDialog *mp_ui;
  const db::Layout *mp_layout;
};

/**
 *  @brief The new layer properties dialog
 */
class LAYUI_PUBLIC NewLayerPropertiesDialog 
  : public QDialog
{
Q_OBJECT

public:
  NewLayerPropertiesDialog (QWidget *parent);
  ~NewLayerPropertiesDialog () override;

  bool exec_dialog (const lay::CellView &cv, db::LayerProperties &src);
  bool exec_dialog (db::LayerProperties &src);

private:
  void accept () override;
  void get (db::LayerProperties &src);

  Ui::NewLayerPropertiesDialog *mp_ui;
};

/**
 *  @brief The generic layout view functions dialog
 *
 *  This dialog replaces QInputDialog and offers
 *  an "apply" callback.
 */
class LAYUI_PUBLIC LayoutViewFunctionDialog
  : public QDialog, public tl::Object
{
Q_OBJECT

public:
  LayoutViewFunctionDialog (QWidget *parent, const QString &title, const QString &label);
  ~LayoutViewFunctionDialog () override;

  bool exec_dialog (QString &value);

  tl::event<std::string> apply_event;
  tl::event<std::string> accept_event;

private slots:
  void apply_clicked ();

private:
  void accept () override;

  Ui::LayoutViewFunctionDialog *mp_ui;
};


/**
 *  @brief The move options dialog
 */
class LAYUI_PUBLIC MoveOptionsDialog 
  : public QDialog, public tl::Object
{
Q_OBJECT

public:
  MoveOptionsDialog (QWidget *parent);
  ~MoveOptionsDialog () override;

  bool exec_dialog (db::DVector &disp);

  tl::event<db::DVector> apply_event;
  tl::event<db::DVector> accept_event;

private slots:
  void apply_clicked ();

private:
  void accept () override;
  db::DVector vector ();

  Ui::MoveOptionsDialog *mp_ui;
};

/**
 *  @brief The move "to" options dialog
 */
class LAYUI_PUBLIC MoveToOptionsDialog 
  : public QDialog
{
Q_OBJECT

public:
  MoveToOptionsDialog (QWidget *parent);
  ~MoveToOptionsDialog () override;

  bool exec_dialog (int &mode_x, int &mode_y, db::DPoint &target);

private slots:
  void button_clicked ();

private:
  void accept () override;

  Ui::MoveToOptionsDialog *mp_ui;
};

/**
 *  @brief The rename cell options dialog
 */
class LAYUI_PUBLIC RenameCellDialog 
  : public QDialog
{
Q_OBJECT

public:
  RenameCellDialog (QWidget *parent);
  ~RenameCellDialog () override;

  bool exec_dialog (const db::Layout &layout, std::string &name);

private:
  void accept () override;

  Ui::RenameCellDialog *mp_ui;
  const db::Layout *mp_layout;
};

/**
 *  @brief The replace cell options dialog
 */
class LAYUI_PUBLIC ReplaceCellOptionsDialog
  : public QDialog
{
Q_OBJECT

public:
  ReplaceCellOptionsDialog (QWidget *parent);
  ~ReplaceCellOptionsDialog () override;

  /** 
   *  @brief Execute the dialog
   *
   *  The mode is either 0 (for shallow), 1 (for deep) and 2 (for complete)
   */
  bool exec_dialog (const lay::CellView &cv, int &replace_mode, db::cell_index_type &cell);

protected:
  void accept () override;

private:
  Ui::ReplaceCellOptionsDialog *mp_ui;
};

/**
 *  @brief The copy cell options dialog
 */
class LAYUI_PUBLIC CopyCellModeDialog 
  : public QDialog
{
Q_OBJECT

public:
  CopyCellModeDialog (QWidget *parent);
  ~CopyCellModeDialog () override;

  /** 
   *  @brief Execute the dialog
   *
   *  The mode is either 0 (for shallow), 1 (for deep)
   */
  bool exec_dialog (int &copy_mode, bool &dont_ask_again);

private:
  Ui::CopyCellModeDialog *mp_ui;
};

/**
 *  @brief The delete cell options dialog
 */
class LAYUI_PUBLIC DeleteCellModeDialog 
  : public QDialog
{
Q_OBJECT

public:
  DeleteCellModeDialog (QWidget *parent);
  ~DeleteCellModeDialog () override;

  /** 
   *  @brief Execute the dialog
   *
   *  The mode is either 0 (for shallow), 1 (for deep) and 2 (for complete)
   */
  bool exec_dialog (int &delete_mode);

private:
  Ui::DeleteCellModeDialog *mp_ui;
};

/**
 *  @brief The delete cell options dialog
 */
class LAYUI_PUBLIC ClearLayerModeDialog 
  : public QDialog
{
Q_OBJECT

public:
  ClearLayerModeDialog (QWidget *parent);
  ~ClearLayerModeDialog () override;

  /** 
   *  @brief Execute the dialog
   *
   *  The mode is either 0 (for locally), 1 (for hierarchically) and 2 (for all)
   */
  bool exec_dialog (int &clear_mode);

private:
  Ui::ClearLayerModeDialog *mp_ui;
};

/**
 *  @brief The open layout mode dialog
 */
class LAYUI_PUBLIC OpenLayoutModeDialog 
  : public QDialog
{
Q_OBJECT

public:
  OpenLayoutModeDialog (QWidget *parent);
  ~OpenLayoutModeDialog () override;

  /** 
   *  @brief Execute the dialog
   *
   *  The mode is either 0 (to replace current view), 1 (to create new view) and 2 (add to current view)
   */
  bool exec_dialog (int &open_mode);

private:
  Ui::OpenLayoutModeDialog *mp_ui;
};

/**
 *  @brief The new layout properties dialog
 */
class LAYUI_PUBLIC NewLayoutPropertiesDialog 
  : public QDialog
{
Q_OBJECT

public:
  NewLayoutPropertiesDialog (QWidget *parent);
  ~NewLayoutPropertiesDialog () override;

  bool exec_dialog (std::string &tech_name, std::string &cell_name, double &dbu, double &window_size, std::vector<db::LayerProperties> &layers, bool &current_panel);

private slots:
  void tech_changed ();

private:
  void accept () override;

  Ui::NewLayoutPropertiesDialog *mp_ui;
  double m_default_dbu;
};

/**
 *  @brief The duplicate layer operation options
 */
class LAYUI_PUBLIC DuplicateLayerDialog 
  : public QDialog
{
Q_OBJECT

public:
  DuplicateLayerDialog (QWidget *parent);
  ~DuplicateLayerDialog () override;

  bool exec_dialog (lay::LayoutViewBase *view, int &cv, int &layer, int &cv_res, int &layer_res, int &hier_mode, bool &clear_before);

public slots:
  void cv_changed (int);

private:
  void accept () override;

  Ui::DuplicateLayerDialog *mp_ui;
  lay::LayoutViewBase *mp_view;
};

/**
 *  @brief A data structure holding the options for the "align cell" dialog
 */
struct LAYUI_PUBLIC AlignCellOptions
{
  AlignCellOptions ()
    : mode_x (-1), mode_y (-1), xpos (0.0), ypos (0.0), visible_only (false), adjust_parents (true)
  { }

  int mode_x, mode_y;
  double xpos, ypos;
  bool visible_only;
  bool adjust_parents;
};

/**
 *  @brief The merge operation options
 */
class LAYUI_PUBLIC AlignCellOptionsDialog 
  : public QDialog
{
Q_OBJECT

public:
  AlignCellOptionsDialog (QWidget *parent);
  ~AlignCellOptionsDialog () override;

  bool exec_dialog (AlignCellOptions &data);

private slots:
  void button_clicked ();
  void accept () override;

private:
  Ui::AlignCellOptionsDialog *mp_ui;
};

/**
 *  @brief Options dialog for the "flatten instances" function
 */
class LAYUI_PUBLIC FlattenInstOptionsDialog
  : public QDialog
{
public:
  FlattenInstOptionsDialog (QWidget *parent, bool enable_pruning = true);
  ~FlattenInstOptionsDialog () override;

  bool exec_dialog (int &levels, bool &prune); 

private:
  Ui::FlattenInstOptionsDialog *mp_ui;
};

/**
 *  @brief The user properties report form
 */
class LAYUI_PUBLIC UserPropertiesForm 
  : public QDialog
{
Q_OBJECT

public:
  UserPropertiesForm (QWidget *parent);
  ~UserPropertiesForm () override;

  bool show (lay::LayoutViewBase *view, unsigned int cv_index, db::properties_id_type &prop_id);
  bool show (lay::LayoutViewBase *view, unsigned int cv_index, db::properties_id_type &prop_id, db::Layout::meta_info_iterator begin_meta, db::Layout::meta_info_iterator end_meta);

public slots:
  void add ();
  void remove ();
  void edit ();
  void dbl_clicked (QTreeWidgetItem *, int);
  void tab_changed (int);

private:
  db::PropertiesSet get_properties (int tab);
  void set_properties (const db::PropertiesSet &props);
  void set_meta_info (db::Layout::meta_info_iterator begin_meta, db::Layout::meta_info_iterator end_meta, const db::Layout &layout);
  void accept () override;

  bool m_editable;
  Ui::UserPropertiesForm *mp_ui;
  db::Layout::meta_info_iterator m_begin_meta, m_end_meta;
  std::unique_ptr<lay::GenericSyntaxHighlighterAttributes> mp_hl_attributes, mp_hl_basic_attributes;
};

/**
 *  @brief The user properties report form
 */
class LAYUI_PUBLIC UserPropertiesEditForm 
  : public QDialog
{
public:
  UserPropertiesEditForm (QWidget *parent);
  ~UserPropertiesEditForm () override;

  bool show (QString &key, QString &value);
  void accept () override;

  Ui::UserPropertiesEditForm *mp_ui;
};

/**
 *  @brief The undo/redo list form
 */
class LAYUI_PUBLIC UndoRedoListForm
  : public QDialog
{
Q_OBJECT

public:
  UndoRedoListForm (QWidget *parent, db::Manager *manager, bool for_undo);
  ~UndoRedoListForm () override;

  bool exec (int &steps);

private slots:
  void selection_changed (const QModelIndex &current);

private:
  Ui::UndoRedoListForm *mp_ui;
  bool m_for_undo;
  db::Manager *mp_manager;
  int m_steps{};
};

}

#endif

#endif  //  defined(HAVE_QT)
