/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2016  Lars Libuda
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

#include "dbwndmacros.h"
#include "configdlgsettings.h"
#include "appimpresario.h"
#include "framemainwindow.h"
#include "sysloglogger.h"
#include "framemainwindow.h"
#include "framestatusbar.h"
#include "appmacromanager.h"
#include "graphitems.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QModelIndexList>
#include <QModelIndex>
#include <QSettings>
#include <QRectF>
#include <QPainter>
#include <QMenu>
#include <QPixmap>
#include <QDrag>

namespace db
{
  //-----------------------------------------------------------------------
  // Class MacroSearchValidator
  //-----------------------------------------------------------------------
  MacroSearchValidator::MacroSearchValidator(QObject* parent) : QValidator(parent)
  {
  }

  QValidator::State	MacroSearchValidator::validate (QString & input, int & pos) const
  {
    if ((pos > 0) && input[0].isSpace())
    {
      return QValidator::Invalid;
    }
    return QValidator::Acceptable;
  }

  //-----------------------------------------------------------------------
  // Class MacroSearch
  //-----------------------------------------------------------------------
  MacroSearch::MacroSearch(QWidget* parent) : QLineEdit(parent), inputValidator(), treeViewBuddy(0)
  {
    this->setValidator(&inputValidator);
  }

  MacroSearch::~MacroSearch()
  {
    setBuddy(0);
  }

  void MacroSearch::setBuddy(MacroTreeView* treeView)
  {
    if (treeViewBuddy)
    {
      treeViewBuddy->setFocusProxy(0);
      treeViewBuddy = 0;
    }
    if (treeView)
    {
      treeViewBuddy = treeView;
      treeViewBuddy->setFocusProxy(this);
    }
  }

  void MacroSearch::keyPressEvent(QKeyEvent* event)
  {
    if (treeViewBuddy)
    {
      switch(event->key())
      {
      case Qt::Key_Up:
      case Qt::Key_Down:
        {
          treeViewBuddy->keyPressEvent(event);
          break;
        }
      default:
        QLineEdit::keyPressEvent(event);
      }
    }
    else
    {
      QLineEdit::keyPressEvent(event);
    }
  }

  //-----------------------------------------------------------------------
  // Class MacroTreeView
  //-----------------------------------------------------------------------
  MacroTreeView::MacroTreeView(QWidget *parent) : QTreeView(parent)
  {
    setSortingEnabled(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragDropMode(DragOnly);
    setDragEnabled(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
  }

  MacroTreeView::~MacroTreeView()
  {
  }

  void MacroTreeView::keyPressEvent(QKeyEvent* event)
  {
    if (event->key() == Qt::Key_Down)
    {
      QModelIndex curIndex = selectionModel()->currentIndex();
      if (curIndex.isValid())
      {
        expand(curIndex);
      }
    }
    QTreeView::keyPressEvent(event);
  }

  void MacroTreeView::startDrag(Qt::DropActions supportedActions)
  {
    QModelIndexList indexes = selectionModel()->selectedRows();
    for(int i = indexes.count() - 1 ; i >= 0; --i)
    {
      if (!(indexes.at(i).isValid()) || !(model()->flags(indexes.at(i)) & Qt::ItemIsDragEnabled))
      {
        indexes.removeAt(i);
      }
    }
    if (indexes.count() > 0)
    {
      QMimeData *data = model()->mimeData(indexes);
      if (!data)
      {
        return;
      }
      QPixmap dragPixmap(":/icons/resources/macro.png");
      QModelIndex srcIndex = dynamic_cast<QSortFilterProxyModel*>(model())->mapToSource(indexes[0]);
      const ModelItemMacro* item = static_cast<const ModelItemMacro*>(srcIndex.internalPointer());
      graph::Vertex::Ptr dummyVertex = app::MacroManager::instance().createVertexInstance(item->macro().signature());
      if (!dummyVertex.isNull())
      {
        graph::VertexItem::Ptr vertexItem = dummyVertex->sceneItem().staticCast<graph::VertexItem>();
        if (!vertexItem.isNull())
        {
          dragPixmap = vertexItem->paintToPixmap();
        }
        app::MacroManager::instance().deleteVertexInstance(dummyVertex);
      }
      data->setImageData(dragPixmap.toImage());
      QDrag* dragObjectPtr = new QDrag(this);
      dragObjectPtr->setPixmap(dragPixmap);
      dragObjectPtr->setMimeData(data);
      dragObjectPtr->setHotSpot(dragPixmap.rect().center());
      Qt::DropAction defDropAction = Qt::IgnoreAction;
      if (defaultDropAction() != Qt::IgnoreAction && (supportedActions & defaultDropAction()))
      {
        defDropAction = defaultDropAction();
      }
      else if (supportedActions & Qt::CopyAction && dragDropMode() != QAbstractItemView::InternalMove)
      {
        defDropAction = Qt::CopyAction;
      }
      if (dragObjectPtr->exec(supportedActions, defDropAction) == Qt::MoveAction)
      {
        // clear or remove dragged items
        const QItemSelection selection = selectionModel()->selection();
        QList<QItemSelectionRange>::const_iterator it = selection.constBegin();
        if (!dragDropOverwriteMode())
        {
          for (; it != selection.constEnd(); ++it)
          {
            QModelIndex parent = (*it).parent();
            if ((*it).left() != 0)
              continue;
            if ((*it).right() != (model()->columnCount(parent) - 1))
              continue;
            int count = (*it).bottom() - (*it).top() + 1;
            model()->removeRows((*it).top(), count, parent);
          }
        }
        else
        {
          QModelIndexList list = selection.indexes();
          for (int i=0; i < list.size(); ++i)
          {
            QModelIndex index = list.at(i);
            QMap<int, QVariant> roles = model()->itemData(index);
            for (QMap<int, QVariant>::Iterator it = roles.begin(); it != roles.end(); ++it)
            {
              it.value() = QVariant();
            }
            model()->setItemData(index, roles);
          }
        }
      }
    }
  }

  //-----------------------------------------------------------------------
  // Class WndMacros
  //-----------------------------------------------------------------------
  WndMacros::WndMacros(QWidget *parent) : QWidget(parent), modelFilters(new ViewFilter()), modelViews(new ViewFormat())
  {
    this->setObjectName("WndMacros");

    // setup layout of window
    QLabel* lblSearch = new QLabel(tr("Search:"));
    QLabel* lblFilter = new QLabel(tr("Filter:"));
    QLabel* lblView = new QLabel(tr("View:"));
    btnSearchReset = new QPushButton(QIcon(":/icons/resources/reset.png"),"");
    btnFilter = new QPushButton(QIcon(":/icons/resources/dbfilter.png"),"");
    btnView = new QPushButton(QIcon(":/icons/resources/dbview.png"),"");
    edtSearch = new MacroSearch();
    cbFilter = new QComboBox();
    cbView = new QComboBox();
    vwMacros = new MacroTreeView();

    // three column layout for controls
    QGridLayout* layout = new QGridLayout();
    layout->addWidget(lblView,0,0);
    layout->addWidget(cbView,0,1);
    layout->addWidget(btnView,0,2);
    layout->addWidget(lblFilter,1,0);
    layout->addWidget(cbFilter,1,1);
    layout->addWidget(btnFilter,1,2);
    layout->addWidget(lblSearch,2,0);
    layout->addWidget(edtSearch,2,1);
    layout->addWidget(btnSearchReset,2,2);
    layout->setColumnStretch(1,1);
    layout->setRowStretch(0,0);
    layout->setRowStretch(1,0);
    layout->setRowStretch(2,0);
    layout->setContentsMargins(5,0,5,0);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addLayout(layout,0);
    mainLayout->addWidget(vwMacros,1);
    this->setLayout(mainLayout);

    // configure controls
    cbView->setModel(&modelViews);
    cbView->setModelColumn(1);
    cbFilter->setModel(&modelFilters);
    cbFilter->setModelColumn(1);
    btnSearchReset->setToolTip(tr("Reset search"));
    btnSearchReset->setEnabled(false);
    btnFilter->setToolTip(tr("Manage filter"));
    btnView->setToolTip(tr("Manage views"));
    sortModel.setSortCaseSensitivity(Qt::CaseInsensitive);
    sortModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    sortModel.setDynamicSortFilter(true);
    sortModel.setSourceModel(&modelMacros);
    vwMacros->setModel(&sortModel);
    vwMacros->sortByColumn(0,Qt::AscendingOrder);
    edtSearch->setBuddy(vwMacros);

    // connect signals
    connect(&app::Impresario::instance(),SIGNAL(initNonCriticalSuccessful()),this,SLOT(initDBView()));
    connect(btnSearchReset,SIGNAL(clicked()),this,SLOT(resetSearch()));
    connect(btnView,SIGNAL(clicked()),this,SLOT(manageViews()));
    connect(btnFilter,SIGNAL(clicked()),this,SLOT(manageFilters()));
    connect(cbView,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(changeView(const QString&)));
    connect(cbFilter,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(changeFilter(const QString&)));
    connect(edtSearch,SIGNAL(textEdited(QString)),this,SLOT(searchMacro(QString)));
    connect(edtSearch,SIGNAL(returnPressed()),this,SLOT(resetSearchAndAddMacro()));
    connect(&modelMacros,SIGNAL(modelUpdateDone(int)),this,SLOT(updateView(int)));
    connect(&modelViews,SIGNAL(activeViewConfigItemChanged()),this,SLOT(rebuildViewModel()));
    connect(&modelFilters,SIGNAL(activeViewConfigItemChanged()),this,SLOT(rebuildViewModel()));
    connect(vwMacros,SIGNAL(activated(QModelIndex)),this,SLOT(resetSearchAndAddMacro()));
    connect(vwMacros->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(macroSelectionChanged(QModelIndex,QModelIndex)));
    connect(vwMacros,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(macroContextMenu(QPoint)));
  }

  WndMacros::~WndMacros()
  {
    modelViews.save(Resource::path(Resource::SETTINGS_DB_VIEWFORMATS));
    modelFilters.save(Resource::path(Resource::SETTINGS_DB_VIEWFILTERS));
  }

  void WndMacros::initDBView()
  {
    // connect status bar for subsequent indications of model changes
    connect(&modelMacros,SIGNAL(modelUpdateStart(db::Model::ModelUpdateReason)),static_cast<frame::StatusBar*>(frame::MainWindow::instance().statusBar()),SLOT(indicateViewUpdate(db::Model::ModelUpdateReason)));
    connect(&modelMacros,SIGNAL(modelUpdateDone(int)),static_cast<frame::StatusBar*>(frame::MainWindow::instance().statusBar()),SLOT(unindicateViewUpdate(int)));

    // block model signals to avoid emitting signal "activeViewConfigItemChanged" in load method called in viewConfigChanged
    modelViews.blockSignals(true);
    modelFilters.blockSignals(true);
    // load data base views
    viewConfigChanged(Resource::SETTINGS_DB_VIEWFORMATS);
    // load data base views
    viewConfigChanged(Resource::SETTINGS_DB_VIEWFILTERS);
    // activate model signals again
    modelViews.blockSignals(false);
    modelFilters.blockSignals(false);
    // build inital view model
    modelMacros.updateModel(Model::UPDATE_INITIAL,static_cast<const ViewFormat*>(modelViews.getActiveViewConfig()),static_cast<const ViewFilter*>(modelFilters.getActiveViewConfig()));
  }

  void WndMacros::viewConfigChanged(Resource::SettingsIDs id)
  {
    switch(id)
    {
    case Resource::SETTINGS_DB_VIEWFORMATS:
      cbView->blockSignals(true);
      syslog::info(tr("Database View: Loading views..."));
      modelViews.load(Resource::path(Resource::SETTINGS_DB_VIEWFORMATS),false);
      cbView->setCurrentIndex(modelViews.getActive().row());
      cbView->setModelColumn(1);
      syslog::info(QString(tr("Database View: Loaded %1 views.")).arg(cbView->count()));
      cbView->blockSignals(false);
      break;
    case Resource::SETTINGS_DB_VIEWFILTERS:
      cbFilter->blockSignals(true);
      syslog::info(tr("Database Filter: Loading filters..."));
      modelFilters.load(Resource::path(Resource::SETTINGS_DB_VIEWFILTERS),true);
      cbFilter->setCurrentIndex(modelFilters.getActive().row());
      cbFilter->setModelColumn(1);
      syslog::info(QString(tr("Database Filter: Loaded %1 filters.")).arg(cbFilter->count()));
      cbFilter->blockSignals(false);
      break;
    case Resource::SETTINGS_DB_SHOWVIEWERS:
      modelMacros.updateModel(Model::UPDATE_SHOWVIEWERS,static_cast<const ViewFormat*>(modelViews.getActiveViewConfig()),static_cast<const ViewFilter*>(modelFilters.getActiveViewConfig()));
      break;
    default:
      return;
    }
  }

  void WndMacros::rebuildViewModel()
  {
    modelMacros.updateModel(Model::UPDATE_INITIAL,static_cast<const ViewFormat*>(modelViews.getActiveViewConfig()),static_cast<const ViewFilter*>(modelFilters.getActiveViewConfig()));
  }

  void WndMacros::updateView(int level)
  {
    if (level == 0)
    {
      return;
    }
    const ViewFormat* fmt = static_cast<const ViewFormat*>(modelViews.getActiveViewConfig());
    int col = (fmt != 0 && fmt->valid()) ? fmt->getTablePart().indexOf('M') : 0;
    sortModel.setFilterKeyColumn(col);
    vwMacros->setRootIsDecorated(level > 1);
    int depth = (fmt != 0 && fmt->valid()) ? fmt->getTreePart().indexOf('M') - 1 : 0;
    vwMacros->expandToDepth(depth);
    vwMacros->resizeColumnToContents(col);
    emit selectionChanged(0,0);
  }

  void WndMacros::manageViews()
  {
    config::DlgSettings dlgConfig(&frame::MainWindow::instance(),config::DlgSettings::MacroDBView);
    dlgConfig.exec();
  }

  void WndMacros::manageFilters()
  {
    config::DlgSettings dlgConfig(&frame::MainWindow::instance(),config::DlgSettings::MacroDBFilter);
    dlgConfig.exec();
  }

  void WndMacros::changeView(const QString &item)
  {
    if (modelViews.setActive(item))
    {
      modelMacros.updateModel(Model::UPDATE_VIEWCHANGED,static_cast<const ViewFormat*>(modelViews.getActiveViewConfig()),static_cast<const ViewFilter*>(modelFilters.getActiveViewConfig()));
    }
  }

  void WndMacros::changeFilter(const QString &item)
  {
    if (modelFilters.setActive(item))
    {
      modelMacros.updateModel(Model::UPDATE_FILTERCHANGED,static_cast<const ViewFormat*>(modelViews.getActiveViewConfig()),static_cast<const ViewFilter*>(modelFilters.getActiveViewConfig()));
    }
  }

  void WndMacros::searchMacro(const QString& pattern)
  {
    btnSearchReset->setEnabled(!pattern.isEmpty());
    sortModel.setFilterRegExp(pattern);
    QModelIndexList matchedMacro = sortModel.match(sortModel.index(0,0),Qt::ModelItemTypeRole,db::ModelItem::MODELITEMTYPE_MACRO,1,Qt::MatchExactly | Qt::MatchRecursive);
    if (matchedMacro.count() > 0 && !pattern.isEmpty())
    {
      vwMacros->selectionModel()->setCurrentIndex(matchedMacro[0],QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
    else
    {
      vwMacros->selectionModel()->reset();
    }
  }

  void WndMacros::macroSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
  {
    const ModelItem* currentItem = 0;
    const ModelItem* oldItem = 0;
    if (current.isValid())
    {
      QModelIndex currentItemIndex = sortModel.mapToSource(current);
      currentItem = static_cast<const ModelItem*>(currentItemIndex.internalPointer());
    }
    if (previous.isValid())
    {
      QModelIndex previousItemIndex = sortModel.mapToSource(previous);
      oldItem = static_cast<const ModelItem*>(previousItemIndex.internalPointer());
    }
    emit selectionChanged(currentItem,oldItem);
  }

  void WndMacros::macroContextMenu(const QPoint &pos)
  {
    if (frame::MainWindow::instance().state() != frame::MainWindow::ProcessGraphEdit)
    {
      return;
    }
    QModelIndex index = vwMacros->indexAt(pos);
    if (index.isValid())
    {
      QModelIndex srcIndex = sortModel.mapToSource(index);
      const ModelItem* item = static_cast<const ModelItem*>(srcIndex.internalPointer());
      if (item->type() == ModelItem::MODELITEMTYPE_MACRO)
      {
        const app::Macro* macroType = &(static_cast<const ModelItemMacro*>(item)->macro());
        QMenu popup;
        QAction* createInstance1 = Resource::action(Resource::MACRO_CREATEINSTANCE_1);
        QAction* createInstance2 = Resource::action(Resource::MACRO_CREATEINSTANCE_2);
        QAction* createInstance3 = Resource::action(Resource::MACRO_CREATEINSTANCE_3);
        QAction* createInstanceMult = Resource::action(Resource::MACRO_CREATEINSTANCE_MULT);
        createInstanceMult->setData(macroType->getTypeSignature());
        popup.addAction(createInstance1);
        popup.addAction(createInstance2);
        popup.addAction(createInstance3);
        popup.addSeparator();
        popup.addAction(createInstanceMult);
        popup.exec(vwMacros->mapToGlobal(pos),createInstance1);
      }
    }
  }

  void WndMacros::resetSearchAndAddMacro()
  {
    // reset search if option set
    QSettings settings;
    if (settings.value(Resource::path(Resource::SETTINGS_DB_AUTORESETSEARCH),false).toBool())
    {
      resetSearch();
    }
    // add macro instance to current process graph
    QModelIndex itemIndex = sortModel.mapToSource(vwMacros->currentIndex());
    const ModelItem* item = static_cast<const ModelItem*>(itemIndex.internalPointer());
    if (itemIndex.isValid() && item && item->type() == ModelItem::MODELITEMTYPE_MACRO)
    {
      const app::Macro& macroType = static_cast<const ModelItemMacro*>(item)->macro();
      emit addMacroInstance(1,macroType.getTypeSignature());
    }
  }

  void WndMacros::resetSearch()
  {
     edtSearch->clear();
     sortModel.setFilterRegExp("");
     btnSearchReset->setEnabled(false);
  }

}
