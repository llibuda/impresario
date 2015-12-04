/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015  Lars Libuda
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
#ifndef DBWNDPROP_H
#define DBWNDPROP_H

#include "dbmodel.h"
#include "appmacro.h"
#include "appmacrolibrary.h"
#include "graphelements.h"
#include <QTabWidget>
#include <QTextEdit>
#include <QtTreePropertyBrowser>
#include <QtVariantPropertyManager>
#include <QModelIndex>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSortFilterProxyModel>
#include <QPoint>
#include <QLabel>

namespace db
{
  class WndPropBrowser : public QtTreePropertyBrowser
  {
    Q_OBJECT
  public:
    WndPropBrowser(QWidget* parent);
    virtual ~WndPropBrowser();

  public slots:
    void updateProps(const db::ModelItem* current, const db::ModelItem* previous);

  private:
    QtVariantPropertyManager* propManager;
  };

  class WndDescription : public QWidget
  {
    Q_OBJECT
  public:
    WndDescription(QWidget* parent = 0);
    virtual ~WndDescription();

  public slots:
    void updateProps(const app::Macro& macro, int paramIndex = -1);
    void updateProps(const app::MacroLibrary& lib);

  private:
    QTextEdit* edtDescription;
    QLabel*    lblDescription;
  };

  class WndMacroInstances : public QWidget
  {
    Q_OBJECT
  public:
    WndMacroInstances(QWidget* parent = 0);
    virtual ~WndMacroInstances();

  public slots:
    void updateProps(const app::Macro& macro);

  private slots:
    void macroInstanceAdd(graph::Vertex::Ptr instance);
    void macroInstanceDel(graph::Vertex::Ptr instance);
    void macroInstanceDataUpdated(graph::Vertex& instance, app::Macro::UpdateReason reason);
    void showContextMenu(const QPoint& pos);

  private:
    QStandardItem* findItem(graph::Vertex* vertexPtr) const;

    QStandardItemModel    model;
    QSortFilterProxyModel sortModel;
    const app::Macro*     currentMacroPrototype;
    QTreeView*            vwInstances;
    QLabel*               lblInstances;
  };

  class WndTypeProps : public QTabWidget
  {
    Q_OBJECT
  public:
    WndTypeProps(QWidget *parent = 0);

  public slots:
    void updateProps(const db::ModelItem* current, const db::ModelItem* previous);

  private:
    WndPropBrowser    tabProps;
    WndDescription    tabDescription;
    WndMacroInstances tabInstances;
  };

}
#endif // DBWNDPROP_H
