/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2020  Lars Libuda
**
**   This file is part of Impresario.
**
**   Impresario is free software: you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation, either version 3 of the License, or
**   (at your option) any later version.
**
**   Impresario is distributed in the hope that it will be useful,
**   but WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**   GNU General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with Impresario in subdirectory "licenses", file "LICENSE_Impresario.GPLv3".
**   If not, see <http://www.gnu.org/licenses/>.
******************************************************************************************/
#ifndef DBWNDMACROS_H
#define DBWNDMACROS_H

#include "dbmodel.h"
#include "dbviewconfig.h"
#include "resources.h"
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QKeyEvent>
#include <QTreeView>
#include <QValidator>

namespace db
{
  class MacroTreeView;

  class MacroSearchValidator : public QValidator
  {
    Q_OBJECT
  public:
    MacroSearchValidator(QObject* parent = 0);
    virtual State	validate ( QString & input, int & pos ) const;
  };

  class MacroSearch : public QLineEdit
  {
    Q_OBJECT
  public:
    MacroSearch(QWidget* parent = 0);
    ~MacroSearch();

    void setBuddy(MacroTreeView* treeView);

    MacroTreeView* buddy() const
    {
      return treeViewBuddy;
    }

  protected:
    virtual void keyPressEvent(QKeyEvent* event);

  private:
    MacroSearchValidator inputValidator;
    MacroTreeView*       treeViewBuddy;
  };

  class MacroTreeView : public QTreeView
  {
    friend class MacroSearch;
    Q_OBJECT
  public:
    MacroTreeView(QWidget* parent = 0);
    ~MacroTreeView();

  protected:
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void startDrag(Qt::DropActions supportedActions);
  };

  class WndMacros : public QWidget
  {
    Q_OBJECT
  public:
    explicit WndMacros(QWidget *parent = 0);
    virtual ~WndMacros();

  signals:
    void addMacroInstance(int countInstances, const QString& typeSignature);
    void selectionChanged(const db::ModelItem* current, const db::ModelItem* previous);

  public slots:
    void viewConfigChanged(Resource::SettingsIDs id);

  private slots:
    void initDBView(bool appInitOk);
    void updateView(int level);
    void manageViews();
    void manageFilters();
    void changeView(const QString& item);
    void changeFilter(const QString& item);
    void rebuildViewModel();
    void searchMacro(const QString& pattern);
    void resetSearchAndAddMacro();
    void resetSearch();
    void macroSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void macroContextMenu(const QPoint& pos);
    void macroDatabaseLoadStarted();
    void macroDatabaseLoadFinished();

  private:
    QPushButton*   btnSearchReset;
    QPushButton*   btnFilter;
    QPushButton*   btnView;
    MacroSearch*   edtSearch;
    QComboBox*     cbFilter;
    QComboBox*     cbView;
    MacroTreeView* vwMacros;

    ViewConfigItemModel modelFilters;
    ViewConfigItemModel modelViews;
    SortFilterModel     sortModel;
    Model               modelMacros;
  };

}
#endif // DBWNDMACROS_H
