/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2019  Lars Libuda
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

#include "dbwndprop.h"
#include "appmacromanager.h"
#include "resources.h"
#include <QList>
#include <QSet>
#include <QtVariantProperty>
#include <QStandardItem>
#include <QMenu>
#include <QIcon>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QAction>
#include <QSettings>

namespace db
{

  //-----------------------------------------------------------------------
  // Class WndPropBrowser
  //-----------------------------------------------------------------------
  WndPropBrowser::WndPropBrowser(QWidget *parent) : QtTreePropertyBrowser(parent), propManager(0)
  {
    this->setPropertiesWithoutValueMarked(true);
    this->setRootIsDecorated(false);
    this->setResizeMode(QtTreePropertyBrowser::Interactive);
  }

  WndPropBrowser::~WndPropBrowser()
  {
    delete this->propManager;
    this->propManager = 0;
  }

  void WndPropBrowser::updateProps(const db::ModelItem* current, const db::ModelItem* previous)
  {
    if (current == 0 && previous != 0)
    {
      return;
    }
    if (current == 0 && previous == 0)
    {
      this->clear();
      delete this->propManager;
      this->propManager = 0;
    }
    if (current != 0)
    {
      QtVariantPropertyManager* newPropManager = new QtVariantPropertyManager();
      QList<QtVariantProperty*> propList = current->properties(*newPropManager);
      this->clear();
      foreach(QtProperty* prop,propList)
      {
        this->addProperty(prop);
      }
      delete this->propManager;
      this->propManager = newPropManager;
    }
  }

  //-----------------------------------------------------------------------
  // Class WndDescription
  //-----------------------------------------------------------------------
  WndDescription::WndDescription(QWidget *parent) : QWidget(parent), edtDescription(0), lblDescription(0)
  {
    setAutoFillBackground(true);

    lblDescription = new QLabel(tr("Description"));
    edtDescription = new QTextEdit();
    edtDescription->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(lblDescription);
    layout->addWidget(edtDescription,1);
    setLayout(layout);
  }

  WndDescription::~WndDescription()
  {
  }

  void WndDescription::updateProps(const app::Macro &macro, int paramIndex)
  {
    if (paramIndex < 0 || paramIndex >= macro.parameters().count())
    {
      edtDescription->setText(macro.getDescription());
      lblDescription->setText(QString(tr("Description of '%1'")).arg(macro.getName()));
    }
    else
    {
      app::MacroParameter* param = macro.parameters().at(paramIndex).value<app::MacroParameter*>();
      Q_ASSERT(param != 0);
      edtDescription->setText(param->getDescription());
      lblDescription->setText(QString(tr("Description of '%1.%2'")).arg(macro.getName()).arg(param->getName()));
    }
  }

  void WndDescription::updateProps(const app::MacroLibrary &lib)
  {
    edtDescription->setText(lib.getDescription());
    lblDescription->setText(QString(tr("Description of '%1'")).arg(lib.getName()));
  }

  //-----------------------------------------------------------------------
  // Class WndMacroInstances
  //-----------------------------------------------------------------------
  WndMacroInstances::WndMacroInstances(QWidget *parent) : QWidget(parent), model(), sortModel(), currentMacroPrototype(0), vwInstances(0), lblInstances(0)
  {
    setAutoFillBackground(true);

    model.setHorizontalHeaderItem(0,new QStandardItem(tr("Instance UUID")));
    model.setHorizontalHeaderItem(1,new QStandardItem(tr("Location")));
    sortModel.setSourceModel(&model);

    vwInstances = new QTreeView();
    vwInstances->setSelectionMode(QAbstractItemView::SingleSelection);
    vwInstances->setRootIsDecorated(false);
    vwInstances->setSortingEnabled(true);
    vwInstances->setContextMenuPolicy(Qt::CustomContextMenu);
    vwInstances->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vwInstances->setModel(&sortModel);

    lblInstances = new QLabel(tr("Instances"));

    QGridLayout* layout = new QGridLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(lblInstances,0,0);
    layout->addWidget(vwInstances,1,0);
    layout->setRowStretch(1,1);
    layout->setColumnStretch(0,1);
    setLayout(layout);

    connect(vwInstances,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showContextMenu(QPoint)));
    connect(&app::MacroManager::instance(),SIGNAL(vertexCreated(graph::Vertex::Ptr)),this,SLOT(macroInstanceAdd(graph::Vertex::Ptr)));
    connect(&app::MacroManager::instance(),SIGNAL(vertexToBeDeleted(graph::Vertex::Ptr)),this,SLOT(macroInstanceDel(graph::Vertex::Ptr)));
  }

  WndMacroInstances::~WndMacroInstances()
  {
    if (currentMacroPrototype)
    {
      disconnect(currentMacroPrototype,SIGNAL(instanceDataUpdated(graph::Vertex&,app::Macro::UpdateReason)),this,SLOT(macroInstanceDataUpdated(graph::Vertex&,app::Macro::UpdateReason)));
      currentMacroPrototype = 0;
    }
  }

  void WndMacroInstances::updateProps(const app::Macro &macro)
  {
    if (currentMacroPrototype)
    {
      model.removeRows(0,model.rowCount());
      disconnect(currentMacroPrototype,SIGNAL(instanceDataUpdated(graph::Vertex&,app::Macro::UpdateReason)),this,SLOT(macroInstanceDataUpdated(graph::Vertex&,app::Macro::UpdateReason)));
      currentMacroPrototype = 0;
    }
    app::MacroManager::VertexInstanceList instList = app::MacroManager::instance().vertices(macro.signature());
    int row = 0;
    foreach(graph::Vertex::Ptr inst, instList)
    {
      QStandardItem* item = new QStandardItem(QIcon(":/icons/resources/macro.png"),inst->id().toString());
      item->setData(reinterpret_cast<qulonglong>(inst.data()),MacroVertexPtrRole);
      model.setItem(row,0,item);
      QIcon icoState;
      if (inst->safeToDelete())
      {
        icoState = QIcon(":/icons/resources/macrounlinked.png");
      }
      else
      {
        icoState = QIcon(":/icons/resources/macrolinked.png");
      }
      if (!inst->graph().isNull())
      {
        model.setItem(row,1,new QStandardItem(icoState,inst->graph()->name()));
      }
      else
      {
        model.setItem(row,1,new QStandardItem(icoState,tr("<UNASSIGNED>")));
      }
      ++row;
    }
    lblInstances->setText(QString(tr("Instances of '%1'")).arg(macro.getName()));
    currentMacroPrototype = &macro;
    connect(currentMacroPrototype,SIGNAL(instanceDataUpdated(graph::Vertex&,app::Macro::UpdateReason)),this,SLOT(macroInstanceDataUpdated(graph::Vertex&,app::Macro::UpdateReason)));
  }

  void WndMacroInstances::macroInstanceAdd(graph::Vertex::Ptr instance)
  {
    if (!currentMacroPrototype || instance->dataRef()->signature() != currentMacroPrototype->signature()) return;
    int row = model.rowCount();
    QStandardItem* item = new QStandardItem(QIcon(":/icons/resources/macro.png"),instance->id().toString());
    item->setData(reinterpret_cast<qulonglong>(instance.data()),MacroVertexPtrRole);
    model.setItem(row,0,item);
    QIcon icoState;
    if (instance->safeToDelete())
    {
      icoState = QIcon(":/icons/resources/macrounlinked.png");
    }
    else
    {
      icoState = QIcon(":/icons/resources/macrolinked.png");
    }
    if (!instance->graph().isNull())
    {
      model.setItem(row,1,new QStandardItem(icoState,instance->graph()->name()));
    }
    else
    {
      model.setItem(row,1,new QStandardItem(icoState,tr("<UNASSIGNED>")));
    }
  }

  void WndMacroInstances::macroInstanceDel(graph::Vertex::Ptr instance)
  {
    if (!currentMacroPrototype || instance->dataRef()->signature() != currentMacroPrototype->signature()) return;
    QStandardItem* macroItem = findItem(instance.data());
    if (macroItem)
    {
      model.removeRow(macroItem->row());
    }
  }

  void WndMacroInstances::macroInstanceDataUpdated(graph::Vertex& instance, app::Macro::UpdateReason reason)
  {
    if (reason != app::Macro::GraphAssignment) return;
    QStandardItem* macroItem = findItem(&instance);
    if (macroItem)
    {
      model.item(macroItem->row(),0)->setText(instance.id().toString());
      QStandardItem* item = model.item(macroItem->row(),1);
      QIcon icoState;
      if (instance.safeToDelete())
      {
        icoState = QIcon(":/icons/resources/macrounlinked.png");
      }
      else
      {
        icoState = QIcon(":/icons/resources/macrolinked.png");
      }
      if (instance.graph())
      {
        item->setData(instance.graph()->name(),Qt::DisplayRole);
        item->setIcon(icoState);
      }
      else
      {
        item->setData(tr("<UNASSIGNED>"),Qt::DisplayRole);
        item->setIcon(icoState);
      }

    }
  }

  void WndMacroInstances::showContextMenu(const QPoint& pos)
  {
    QModelIndex index = vwInstances->indexAt(pos);
    if (index.isValid())
    {
      QModelIndex srcIndex = sortModel.mapToSource(index);
      QStandardItem* item = model.item(srcIndex.row());
      QAction* navAction = Resource::action(Resource::MACRO_FINDINSTANCE);
      graph::Vertex* macroInstance = reinterpret_cast<graph::Vertex*>(item->data(MacroVertexPtrRole).toULongLong());
      if (macroInstance->safeToDelete())
      {
        navAction->setText(tr("Navigate to Process Graph"));
      }
      else
      {
        navAction->setText(tr("Navigate to Macro instance"));
      }
      navAction->setData(item->data(MacroVertexPtrRole));
      QList<QAction*> actions;
      actions.append(navAction);
      QMenu::exec(actions,vwInstances->mapToGlobal(pos),actions[0],this);
    }
  }

  QStandardItem* WndMacroInstances::findItem(graph::Vertex* vertexPtr) const
  {
    for(int i = 0; i < model.rowCount(); ++i)
    {
      QStandardItem* item = model.item(i);
      graph::Vertex* itemData = reinterpret_cast<graph::Vertex*>(item->data(MacroVertexPtrRole).toULongLong());
      if (vertexPtr == itemData)
      {
        return item;
      }
    }
    return 0;
  }

  //-----------------------------------------------------------------------
  // Class WndTypeProps
  //-----------------------------------------------------------------------
  WndTypeProps::WndTypeProps(QWidget *parent) : QTabWidget(parent), tabProps(this), tabDescription(this), tabInstances(this)
  {
    this->setTabShape(QTabWidget::Triangular);
    this->setTabPosition(QTabWidget::South);

    this->tabProps.setObjectName("MacroTypeProps");
    this->addTab(&tabProps,tr("General"));

    this->addTab(&tabDescription,tr("Description"));
    this->removeTab(1);

    this->addTab(&tabInstances,tr("Instances"));
    this->removeTab(1);
  }

  void WndTypeProps::updateProps(const db::ModelItem* current, const db::ModelItem* previous)
  {
    tabProps.updateProps(current,previous);
    if (current)
    {
      QSettings settings;
      int activeTab;
      switch(current->type())
      {
        case db::ModelItem::MODELITEMTYPE_MACRO:
        {
          const db::ModelItemMacro* itemMacro = static_cast<const db::ModelItemMacro*>(current);
          this->tabDescription.updateProps(itemMacro->macro());
          if (this->count() == 1)
          {
            this->addTab(&tabDescription,tr("Description"));
          }
          this->tabInstances.updateProps(itemMacro->macro());
          if (this->count() == 2)
          {
            this->addTab(&tabInstances,tr("Instances"));
          }
          activeTab = settings.value(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_MACRO),0).toInt();
          break;
        }
        case db::ModelItem::MODELITEMTYPE_VIEWER:
        {
          const db::ModelItemMacro* itemMacro = static_cast<const db::ModelItemMacro*>(current);
          this->tabDescription.updateProps(itemMacro->macro());
          if (this->count() == 1)
          {
            this->addTab(&tabDescription,tr("Description"));
          }
          if (this->count() == 3)
          {
            this->removeTab(2);
          }
          activeTab = settings.value(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_VIEWER),0).toInt();
          break;
        }
        case db::ModelItem::MODELITEMTYPE_LIBRARYFILE:
        {
          const db::ModelItemLibraryFile* itemLib = static_cast<const db::ModelItemLibraryFile*>(current);
          this->tabDescription.updateProps(itemLib->library());
          if (this->count() == 1)
          {
            this->addTab(&tabDescription,tr("Description"));
          }
          if (this->count() == 3)
          {
            this->removeTab(2);
          }
          activeTab = settings.value(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_LIB),0).toInt();
          break;
        }
        default:
          activeTab = settings.value(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_OTHERS),0).toInt();
          if (this->count() > 1)
          {
            this->removeTab(2);
            this->removeTab(1);
          }
          break;
      }
      if (!previous || (current->type() != previous->type()))
      {
        if (activeTab >= this->count() || activeTab < 0)
        {
          this->setCurrentIndex(0);
        }
        else
        {
          this->setCurrentIndex(activeTab);
        }
      }
    }
    else
    {
      if (this->count() > 1)
      {
        this->removeTab(2);
        this->removeTab(1);
      }
    }
  }

}
