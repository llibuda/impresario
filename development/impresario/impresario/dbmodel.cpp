/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2018  Lars Libuda
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

#include "dbmodel.h"
#include "appmacro.h"
#include "appmacromanager.h"
#include "resources.h"
#include <QMutexLocker>
#include <QIcon>
#include <QByteArray>
#include <QtVariantProperty>
#include <QSettings>
#include <QFileInfo>

namespace db
{
  //-----------------------------------------------------------------------
  // Class ModelItem
  //-----------------------------------------------------------------------
  ModelItem::ModelItem(ModelItem* parent) : searchKey(), itemType(MODELITEMTYPE_GENERIC), parentItem(parent)
  {
  }

  ModelItem::~ModelItem()
  {
    qDeleteAll(childItem);
  }

  void ModelItem::appendChild(ModelItem *item)
  {
    childItem.append(item);
  }

  int ModelItem::row() const
  {
    if (parentItem)
    {
      return parentItem->childItem.indexOf(const_cast<ModelItem*>(this));
    }
    return 0;
  }

  ModelItem* ModelItem::find(const QString& key) const
  {
    for(QList<ModelItem*>::const_iterator it = childItem.begin(); it != childItem.end(); ++it)
    {
      if (key == (*it)->searchKey)
      {
        return *it;
      }
    }
    return 0;
  }

  bool ModelItem::matchMacro(const QRegExp pattern) const
  {
    if (type() == MODELITEMTYPE_MACRO)
    {
      return searchKey.contains(pattern);
    }
    else
    {
      for(QList<ModelItem*>::const_iterator it = childItem.begin(); it != childItem.end(); ++it)
      {
        if ((*it)->matchMacro(pattern))
        {
          return true;
        }
      }
      return false;
    }
  }

  QList<QtVariantProperty*> ModelItem::properties(QtVariantPropertyManager& propManager) const
  {
    QList<QtVariantProperty*> propList;
    QtVariantProperty* group;
    group = propManager.addProperty(QtVariantPropertyManager::groupTypeId(), QObject::tr("Category"));
    QtVariantProperty* item;
    item = propManager.addProperty(QVariant::String, QObject::tr("Type"));
    item->setValue(QObject::tr("ModelItem"));
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Value"));
    item->setValue(searchKey);
    group->addSubProperty(item);
    propList.append(group);
    return propList;
  }

  //-----------------------------------------------------------------------
  // Class ModelItemRoot
  //-----------------------------------------------------------------------
  ModelItemRoot::ModelItemRoot(const QString& viewTableFormat, ModelItem* parent) : ModelItem(parent)
  {
    itemType = MODELITEMTYPE_ROOT;
    searchKey = viewTableFormat;
  }

  QVariant ModelItemRoot::data(int column, int role) const
  {
    switch(role)
    {
    case Qt::DisplayRole:
      {
        QChar colId = searchKey.at(column);
        return ViewFormat::columnName(colId);
      }
    case TypeRole:
      return type();
    default:
      return QVariant();
    }
  }

  QList<QtVariantProperty*> ModelItemRoot::properties(QtVariantPropertyManager& /*propManager*/) const
  {
    return QList<QtVariantProperty*>();
  }

  int ModelItemRoot::columnCount() const
  {
    return searchKey.length();
  }

  int ModelItemRoot::levelCount() const
  {
    const ModelItem* parent = this;
    int count = 0;
    while(parent->childCount() > 0)
    {
      ++count;
      parent = parent->child(0);
    }
    return count;
  }

  //-----------------------------------------------------------------------
  // Class ModelItemCreator
  //-----------------------------------------------------------------------
  ModelItemCreator::ModelItemCreator(const QString& creator, ModelItem* parent) : ModelItem(parent)
  {
    itemType = MODELITEMTYPE_CREATOR;
    searchKey = creator;
  }

  QVariant ModelItemCreator::data(int column, int role) const
  {
    if (column == 0)
    {
      switch(role)
      {
      case Qt::DisplayRole:
        return searchKey;
      case Qt::DecorationRole:
        return QIcon(":/icons/resources/macrocreator.png");
      case TypeRole:
        return type();
      default:
        return QVariant();
      }
    }
    else
    {
      return QVariant();
    }
  }

  QList<QtVariantProperty*> ModelItemCreator::properties(QtVariantPropertyManager& propManager) const
  {
    QList<QtVariantProperty*> propList = ModelItem::properties(propManager);
    static_cast<QtVariantProperty*>(propList.at(0)->subProperties().at(0))->setValue(QObject::tr("Creator"));
    return propList;
  }

  //-----------------------------------------------------------------------
  // Class ModelItemGroup
  //-----------------------------------------------------------------------
  ModelItemGroup::ModelItemGroup(const QString& group, ModelItem* parent) : ModelItem(parent)
  {
    itemType = MODELITEMTYPE_GROUP;
    searchKey = group;
  }

  QVariant ModelItemGroup::data(int column, int role) const
  {
    if (column == 0)
    {
      switch(role)
      {
      case Qt::DisplayRole:
        return searchKey;
      case Qt::DecorationRole:
        return QIcon(":/icons/resources/macrogroup.png");
      case TypeRole:
        return type();
      default:
        return QVariant();
      }
    }
    else
    {
      return QVariant();
    }
  }

  QList<QtVariantProperty*> ModelItemGroup::properties(QtVariantPropertyManager& propManager) const
  {
    QList<QtVariantProperty*> propList = ModelItem::properties(propManager);
    static_cast<QtVariantProperty*>(propList.at(0)->subProperties().at(0))->setValue(QObject::tr("Group"));
    return propList;
  }

  //-----------------------------------------------------------------------
  // Class ModelItemBuild
  //-----------------------------------------------------------------------
  ModelItemBuild::ModelItemBuild(const QString& build, ModelItem* parent) : ModelItem(parent)
  {
    itemType = MODELITEMTYPE_BUILD;
    searchKey = build;
  }

  QVariant ModelItemBuild::data(int column, int role) const
  {
    if (column == 0)
    {
      switch(role)
      {
      case Qt::DisplayRole:
        return searchKey;
      case Qt::DecorationRole:
        return QIcon(":/icons/resources/macrobuild.png");
      case TypeRole:
        return type();
      default:
        return QVariant();
      }
    }
    else
    {
      return QVariant();
    }
  }

  QList<QtVariantProperty*> ModelItemBuild::properties(QtVariantPropertyManager& propManager) const
  {
    QList<QtVariantProperty*> propList = ModelItem::properties(propManager);
    static_cast<QtVariantProperty*>(propList.at(0)->subProperties().at(0))->setValue(QObject::tr("Build"));
    return propList;
  }

  //-----------------------------------------------------------------------
  // Class ModelItemType
  //-----------------------------------------------------------------------
  ModelItemType::ModelItemType(const QString& type, ModelItem* parent) : ModelItem(parent)
  {
    itemType = MODELITEMTYPE_TYPE;
    searchKey = type;
  }

  QVariant ModelItemType::data(int column, int role) const
  {
    if (column == 0)
    {
      switch(role)
      {
      case Qt::DisplayRole:
        return searchKey;
      case Qt::DecorationRole:
        return QIcon(":/icons/resources/macrotype.png");
      case TypeRole:
        return type();
      default:
        return QVariant();
      }
    }
    else
    {
      return QVariant();
    }
  }

  QList<QtVariantProperty*> ModelItemType::properties(QtVariantPropertyManager& propManager) const
  {
    QList<QtVariantProperty*> propList = ModelItem::properties(propManager);
    static_cast<QtVariantProperty*>(propList.at(0)->subProperties().at(0))->setValue(QObject::tr("Macro Type"));
    return propList;
  }

  //-----------------------------------------------------------------------
  // Class ModelItemLibrary
  //-----------------------------------------------------------------------
  ModelItemLibrary::ModelItemLibrary(const QString& libraryName, ModelItem* parent) : ModelItem(parent)
  {
    itemType = MODELITEMTYPE_LIBRARY;
    searchKey = libraryName;
  }

  QVariant ModelItemLibrary::data(int column, int role) const
  {
    if (column == 0)
    {
      switch(role)
      {
      case Qt::DisplayRole:
        return searchKey;
      case Qt::DecorationRole:
        return QIcon(":/icons/resources/macrodb.png");
      case TypeRole:
        return type();
      default:
        return QVariant();
      }
    }
    else
    {
      return QVariant();
    }
  }

  QList<QtVariantProperty*> ModelItemLibrary::properties(QtVariantPropertyManager& propManager) const
  {
    QList<QtVariantProperty*> propList = ModelItem::properties(propManager);
    static_cast<QtVariantProperty*>(propList.at(0)->subProperties().at(0))->setValue(QObject::tr("Library"));
    return propList;
  }

  //-----------------------------------------------------------------------
  // Class ModelItemLibraryFile
  //-----------------------------------------------------------------------
  ModelItemLibraryFile::ModelItemLibraryFile(const app::MacroLibrary& library, ModelItem* parent) : ModelItem(parent), libRef(&library)
  {
    itemType = MODELITEMTYPE_LIBRARYFILE;
    QFileInfo fileInfo(libRef->getPath());
    searchKey = fileInfo.completeBaseName();
  }

  QVariant ModelItemLibraryFile::data(int column, int role) const
  {
    if (column == 0)
    {
      switch(role)
      {
      case Qt::DisplayRole:
        return searchKey;
      case Qt::DecorationRole:
        return QIcon(":/icons/resources/macrolib.png");
      case TypeRole:
        return type();
      default:
        return QVariant();
      }
    }
    else
    {
      return QVariant();
    }
  }

  QList<QtVariantProperty*> ModelItemLibraryFile::properties(QtVariantPropertyManager& propManager) const
  {
    QList<QtVariantProperty*> propList = ModelItem::properties(propManager);
    static_cast<QtVariantProperty*>(propList.at(0)->subProperties().at(0))->setValue(QObject::tr("Library file"));

    QtVariantProperty* group;
    group = propManager.addProperty(QtVariantPropertyManager::groupTypeId(), QObject::tr("Library ") + libRef->getName());
    QtVariantProperty* item;
    item = propManager.addProperty(QVariant::String, QObject::tr("Library file"));
    item->setValue(libRef->getPath());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Version"));
    item->setValue(libRef->getVersionString());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Build"));
    item->setValue(libRef->getBuildType());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Build date"));
    item->setValue(libRef->getBuildDate());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Qt version"));
    item->setValue(libRef->getQtVersionString());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::Int, QObject::tr("Loaded macros"));
    item->setValue(libRef->countMacros());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::Int, QObject::tr("Loaded viewers"));
    item->setValue(libRef->countViewers());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Creator"));
    item->setValue(libRef->getCreator());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("API Version"));
    item->setValue(libRef->getAPIVersionString());
    group->addSubProperty(item);
    propList.append(group);

    return propList;
  }

  //-----------------------------------------------------------------------
  // Class ModelItemMacro
  //-----------------------------------------------------------------------
  ModelItemMacro::ModelItemMacro(const app::Macro& macro, const QString& colDef, ModelItem* parent) : ModelItem(parent), macroRef(macro), columnDef(colDef)
  {
    itemType = MODELITEMTYPE_MACRO;
    searchKey = macro.getName();
  }

  QVariant ModelItemMacro::data(int column, int role) const
  {
    if (role == Qt::DisplayRole)
    {
      switch(columnDef[column].toLatin1())
      {
      case 'M':
        return macroRef.getName();
      case 'G':
        return macroRef.getGroup();
      case 'C':
        return macroRef.getCreator();
      case 'L':
        return macroRef.getLibrary().getName();
      case 'F':
        {
          QFileInfo info(macroRef.getLibrary().getPath());
          return info.completeBaseName();
        }
      case 'T':
        return macroRef.getClass();
      case 'B':
        return macroRef.getBuild();
      default:
        return QVariant();
      }
    }
    else if (role == Qt::DecorationRole)
    {
      switch(columnDef[column].toLatin1())
      {
      case 'M':
        return QIcon(":/icons/resources/macro.png");
      case 'G':
        return QIcon(":/icons/resources/macrogroup.png");
      case 'C':
        return QIcon(":/icons/resources/macrocreator.png");
      case 'L':
        return QIcon(":/icons/resources/macrodb.png");
      case 'F':
          return QIcon(":/icons/resources/macrolib.png");
      case 'T':
        return QIcon(":/icons/resources/macrotype.png");
      case 'B':
        return QIcon(":/icons/resources/macrobuild.png");
      default:
        return QVariant();
      }
    }
    else if (role == TypeRole)
    {
      return type();
    }
    else
    {
      return QVariant();
    }
  }

  QList<QtVariantProperty*> ModelItemMacro::properties(QtVariantPropertyManager& propManager) const
  {
    QList<QtVariantProperty*> propList = ModelItem::properties(propManager);
    static_cast<QtVariantProperty*>(propList.at(0)->subProperties().at(0))->setValue(QObject::tr("Macro"));

    QtVariantProperty* group;
    QtVariantProperty* libGroup;
    QtVariantProperty* inputGroup;
    QtVariantProperty* outputGroup;
    QtVariantProperty* paramGroup;
    group = propManager.addProperty(QtVariantPropertyManager::groupTypeId(), QObject::tr("Macro ") + macroRef.getName());
    QtVariantProperty* item;
    // General properties
    item = propManager.addProperty(QVariant::String, QObject::tr("Creator"));
    item->setValue(macroRef.getCreator());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Group"));
    item->setValue(macroRef.getGroup());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Type"));
    item->setValue(macroRef.getClass());
    group->addSubProperty(item);
    // Inputs and Outputs
    inputGroup = propManager.addProperty(QVariant::String, QObject::tr("Inputs"));
    outputGroup = propManager.addProperty(QVariant::String, QObject::tr("Outputs"));
    int cntInputs = 0;
    int cntOutputs = 0;
    const app::Macro::PinDataMap& pins = macroRef.pinData();
    for(app::Macro::PinDataMap::const_iterator it = pins.begin(); it != pins.end(); ++it)
    {
      item = propManager.addProperty(QVariant::String, static_cast<app::MacroPin*>(it.value().data())->getName());
      item->setValue((static_cast<app::MacroPin*>(it.value().data()))->getType());
      switch(it.value()->direction())
      {
      case graph::Defines::Incoming:
        inputGroup->addSubProperty(item);
        cntInputs++;
        break;
      case graph::Defines::Outgoing:
        outputGroup->addSubProperty(item);
        cntOutputs++;
        break;
      default:
        break;
      }
    }
    inputGroup->setValue(cntInputs);
    outputGroup->setValue(cntOutputs);
    group->addSubProperty(inputGroup);
    group->addSubProperty(outputGroup);
    // Parameters
    paramGroup = propManager.addProperty(QVariant::String, QObject::tr("Parameters"));
    const QVariantList params = macroRef.parameters();
    foreach(QVariant variant,params)
    {
      app::MacroParameter* param = variant.value<app::MacroParameter*>();
      item = propManager.addProperty(QVariant::String, param->getName());
      item->setValue(param->getType());
      paramGroup->addSubProperty(item);
    }
    paramGroup->setValue(params.size());
    group->addSubProperty(paramGroup);
    // Library properties
    libGroup = propManager.addProperty(QVariant::String, QObject::tr("Library"));
    libGroup->setValue(macroRef.getLibrary().getName());
    group->addSubProperty(libGroup);
    item = propManager.addProperty(QVariant::String, QObject::tr("Libray file"));
    item->setValue(macroRef.getLibrary().getPath());
    libGroup->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Version"));
    item->setValue(macroRef.getLibrary().getVersionString());
    libGroup->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Build"));
    item->setValue(macroRef.getBuild());
    libGroup->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Build date"));
    item->setValue(macroRef.getLibrary().getBuildDate());
    libGroup->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Qt version"));
    item->setValue(macroRef.getLibrary().getQtVersionString());
    libGroup->addSubProperty(item);
    item = propManager.addProperty(QVariant::Int, QObject::tr("Loaded macros"));
    item->setValue(macroRef.getLibrary().countMacros());
    libGroup->addSubProperty(item);
    item = propManager.addProperty(QVariant::Int, QObject::tr("Loaded viewers"));
    item->setValue(macroRef.getLibrary().countViewers());
    libGroup->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Creator"));
    item->setValue(macroRef.getLibrary().getCreator());
    libGroup->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("API Version"));
    item->setValue(macroRef.getLibrary().getAPIVersionString());
    libGroup->addSubProperty(item);

    propList.append(group);
    return propList;
  }

  int ModelItemMacro::columnCount() const
  {
    return columnDef.length();
  }

  //-----------------------------------------------------------------------
  // Class ModelItemViewer
  //-----------------------------------------------------------------------
  ModelItemViewer::ModelItemViewer(const app::Macro& macro, const QString& colDef, ModelItem* parent) : ModelItemMacro(macro,colDef,parent)
  {
    itemType = MODELITEMTYPE_VIEWER;
  }

  QVariant ModelItemViewer::data(int column, int role) const
  {
    if (role == Qt::ForegroundRole)
    {
      return QVariant(QColor(Qt::lightGray));
    }
    else if (role == Qt::DecorationRole)
    {
      switch(columnDef[column].toLatin1())
      {
      case 'M':
        return QIcon(":/icons/resources/eye.png");
      case 'G':
        return QIcon(":/icons/resources/macrogroup.png");
      case 'C':
        return QIcon(":/icons/resources/macrocreator.png");
      case 'L':
        return QIcon(":/icons/resources/macrodb.png");
      case 'F':
        return QIcon(":/icons/resources/macrolib.png");
      case 'T':
        return QIcon(":/icons/resources/macrotype.png");
      case 'B':
        return QIcon(":/icons/resources/macrobuild.png");
      default:
        return QVariant();
      }
    }
    else
    {
      return ModelItemMacro::data(column,role);
    }
  }

  QList<QtVariantProperty*> ModelItemViewer::properties(QtVariantPropertyManager& propManager) const
  {
    QList<QtVariantProperty*> propList = ModelItem::properties(propManager);
    static_cast<QtVariantProperty*>(propList.at(0)->subProperties().at(0))->setValue(QObject::tr("Viewer"));

    QtVariantProperty* group;
    QtVariantProperty* libGroup;
    QtVariantProperty* inputGroup;
    group = propManager.addProperty(QtVariantPropertyManager::groupTypeId(), QObject::tr("Viewer ") + macroRef.getName());
    QtVariantProperty* item;
    // General properties
    item = propManager.addProperty(QVariant::String, QObject::tr("Creator"));
    item->setValue(macroRef.getCreator());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Group"));
    item->setValue(macroRef.getGroup());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Type"));
    item->setValue(macroRef.getClass());
    group->addSubProperty(item);
    // Data types
    inputGroup = propManager.addProperty(QVariant::String, QObject::tr("Supported data types"));
    int cntInputs = 0;
    const app::Macro::PinDataMap& pins = macroRef.pinData();
    for(app::Macro::PinDataMap::const_iterator it = pins.begin(); it != pins.end(); ++it)
    {
      item = propManager.addProperty(QVariant::String, static_cast<app::MacroPin*>(it.value().data())->getName());
      item->setValue((static_cast<app::MacroPin*>(it.value().data()))->getType());
      switch(it.value()->direction())
      {
      case graph::Defines::Incoming:
        inputGroup->addSubProperty(item);
        cntInputs++;
        break;
      default:
        break;
      }
    }
    inputGroup->setValue(cntInputs);
    group->addSubProperty(inputGroup);
    // Library properties
    libGroup = propManager.addProperty(QVariant::String, QObject::tr("Library"));
    libGroup->setValue(macroRef.getLibrary().getName());
    group->addSubProperty(libGroup);
    item = propManager.addProperty(QVariant::String, QObject::tr("Libray file"));
    item->setValue(macroRef.getLibrary().getPath());
    libGroup->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Version"));
    item->setValue(macroRef.getLibrary().getVersionString());
    libGroup->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Build"));
    item->setValue(macroRef.getBuild());
    libGroup->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Build date"));
    item->setValue(macroRef.getLibrary().getBuildDate());
    libGroup->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Qt version"));
    item->setValue(macroRef.getLibrary().getQtVersionString());
    libGroup->addSubProperty(item);
    propList.append(group);

    return propList;
  }

  //-----------------------------------------------------------------------
  // Class ModelCreator
  //-----------------------------------------------------------------------
  ModelCreator::ModelCreator(QObject* parent) : QObject(parent), watcher()
  {
    connect(&watcher,SIGNAL(finished()),this,SLOT(creatingViewModelFinished()));
  }

  ModelCreator::~ModelCreator()
  {
  }

  void ModelCreator::createViewModel(const ViewFormat& format, const ViewFilter& filter)
  {
    QFuture<ModelItemRoot*> future = QtConcurrent::run(ModelCreator::doCreateModel,format,filter);
    watcher.setFuture(future);
  }

  void ModelCreator::creatingViewModelFinished()
  {
    emit modelCreated(watcher.result());
  }

  ModelItemRoot* ModelCreator::doCreateModel(const ViewFormat& format, const ViewFilter& filter)
  {
    QString fmtTree;
    QString fmtTable;
    if (format.valid())
    {
      fmtTree = format.getTreePart();
      fmtTable = format.getTablePart();
    }
    else
    {
      return 0;
    }
    // create new model tree
    ModelItemRoot* root = new ModelItemRoot(fmtTable,0);
    app::MacroManager::instance().iterateVertexDataTypes(ModelCreator::handleVertexDataType,&fmtTree,&fmtTable,&filter,root);
    QSettings settings;
    if (settings.value(Resource::path(Resource::SETTINGS_DB_SHOWVIEWERS),false).toBool())
    {
      app::MacroManager::instance().iterateViewerTypes(ModelCreator::handleVertexDataType,&fmtTree,&fmtTable,&filter,root);
    }
    return root;
  }

  bool ModelCreator::handleVertexDataType(graph::VertexData::Ptr macroPtr, va_list args)
  {
    QString*       ptrFmtTree = va_arg(args,QString*);
    QString*       ptrFmtTable = va_arg(args,QString*);
    ViewFilter*    flt = va_arg(args,ViewFilter*);
    ModelItemRoot* modelRoot = va_arg(args,ModelItemRoot*);
    if (flt->valid())
    {
      const app::Macro& macro = *(static_cast<const app::Macro*>(macroPtr.data()));
      if (flt->apply(macro))
      {
        addModelItem(macro,*ptrFmtTree,*ptrFmtTable,0,modelRoot);
      }
    }
    else
    {
      addModelItem(*(static_cast<const app::Macro*>(macroPtr.data())),*ptrFmtTree,*ptrFmtTable,0,modelRoot);
    }
    return true;
  }

  void ModelCreator::addModelItem(const app::Macro& macro, const QString& treeFormat, const QString& tableFormat, int pos, ModelItem* parent)
  {
    if (pos >= treeFormat.length())
    {
      return;
    }
    ModelItem* childItem = 0;
    switch(treeFormat[pos].toLatin1())
    {
    case 'M':
      if (macro.getType() != app::Macro::Viewer)
      {
        childItem = new ModelItemMacro(macro, tableFormat, parent);
      }
      else
      {
        childItem = new ModelItemViewer(macro, tableFormat, parent);
      }
      break;
    case 'G':
      childItem = new ModelItemGroup(macro.getGroup(), parent);
      break;
    case 'C':
      childItem = new ModelItemCreator(macro.getCreator(), parent);
      break;
    case 'L':
      childItem = new ModelItemLibrary(macro.getLibrary().getName(), parent);
      break;
    case 'F':
      childItem = new ModelItemLibraryFile(macro.getLibrary(), parent);
      break;
    case 'T':
      childItem = new ModelItemType(macro.getClass(), parent);
      break;
    case 'B':
      childItem = new ModelItemBuild(macro.getBuild(), parent);
      break;
    }
    ModelItem* item = parent->find(childItem->key());
    if (!item || (treeFormat[pos] == 'M'))
    {
      parent->appendChild(childItem);
    }
    else
    {
      delete childItem;
      childItem = 0;
    }
    addModelItem(macro, treeFormat, tableFormat, pos + 1, (childItem == 0) ? item : childItem);
  }

  //-----------------------------------------------------------------------
  // Class Model
  //-----------------------------------------------------------------------
  Model::Model(QObject *parent) : QAbstractItemModel(parent), rootItem(0), modelCreator()
  {
    connect(&modelCreator,SIGNAL(modelCreated(ModelItemRoot*)),this,SLOT(updateInternalModel(ModelItemRoot*)));
  }

  Model::~Model()
  {
    delete rootItem;
    rootItem = 0;
  }

  void Model::updateModel(ModelUpdateReason reason, const ViewFormat* format, const ViewFilter* filter)
  {
    if (format && filter)
    {
      modelCreator.createViewModel(*format,*filter);
      emit modelUpdateStart(reason);
    }
  }

  void Model::updateInternalModel(ModelItemRoot *root)
  {
    if (root)
    {
      beginResetModel();
      delete rootItem;
      rootItem = root;
      endResetModel();
    }
    emit modelUpdateDone((rootItem != 0) ? rootItem->levelCount() : 0);
  }

  int Model::columnCount(const QModelIndex& /*parent*/) const
  {
    if (rootItem)
    {
      return rootItem->columnCount();
    }
    else
    {
      return 0;
    }
  }

  QVariant Model::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
    {
      return QVariant();
    }
    if ((role != Qt::DisplayRole) && (role != Qt::DecorationRole) && (role != ModelItem::TypeRole) && (role != Qt::ForegroundRole))
    {
      return QVariant();
    }
    ModelItem *item = static_cast<ModelItem*>(index.internalPointer());
    return item->data(index.column(),role);
  }

  Qt::ItemFlags Model::flags(const QModelIndex &index) const
  {
    if (!index.isValid())
    {
      return 0;
    }
    Qt::ItemFlags fl =  Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    const ModelItem* item = static_cast<const ModelItem*>(index.internalPointer());
    if (item->type() == ModelItem::MODELITEMTYPE_MACRO)
    {
      fl |= Qt::ItemIsDragEnabled;
    }
    return fl;
  }

  QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
      if (rootItem != 0)
      {
        return rootItem->data(section);
      }
      else
      {
        return QVariant();
      }
    }
    return QVariant();
  }

  QModelIndex Model::index(int row, int column, const QModelIndex &parent) const
  {
    if (!hasIndex(row, column, parent))
    {
      return QModelIndex();
    }
    ModelItem* parentItem;
    if (!parent.isValid())
    {
      parentItem = rootItem;
    }
    else
    {
      parentItem = static_cast<ModelItem*>(parent.internalPointer());
    }
    ModelItem* childItem = parentItem->child(row);
    if (childItem)
    {
      return createIndex(row, column, childItem);
    }
    else
    {
      return QModelIndex();
    }
  }

  QModelIndex Model::parent(const QModelIndex &index) const
  {
    if (!index.isValid())
    {
      return QModelIndex();
    }
    ModelItem* childItem = static_cast<ModelItem*>(index.internalPointer());
    ModelItem* parentItem = childItem->parent();

    if (parentItem == rootItem)
    {
      return QModelIndex();
    }
    return createIndex(parentItem->row(), 0, parentItem);
  }

  int Model::rowCount(const QModelIndex &parent) const
  {
    ModelItem* parentItem;
    if (parent.column() > 0)
    {
      return 0;
    }
    if (!parent.isValid())
    {
      parentItem = rootItem;
    }
    else
    {
      parentItem = static_cast<ModelItem*>(parent.internalPointer());
    }
    if (parentItem != 0)
    {
      return parentItem->childCount();
    }
    else
    {
      return 0;
    }
  }

  QMimeData* Model::mimeData(const QModelIndexList& indexes) const
  {
    if (indexes.isEmpty())
    {
      return 0;
    }
    QStringList typeList;
    foreach (QModelIndex index, indexes)
    {
      if (index.isValid())
      {
        const ModelItemMacro* item = static_cast<const ModelItemMacro*>(index.internalPointer());
        if (!typeList.contains(item->macro().getTypeSignature()))
        {
          typeList.append(item->macro().getTypeSignature());
        }
      }
    }
    QByteArray encodedData;
    encodedData.append(typeList.join("\n"));
    QMimeData *mimeData = new QMimeData();
    mimeData->setData("text/graph-vertex-type-signature",encodedData);
    return mimeData;
  }

  QStringList Model::mimeTypes() const
  {
    QStringList types;
    types << "text/impresario-macro-type-signature";
    return types;
  }

  //-----------------------------------------------------------------------
  // Class SortFilterModel
  //-----------------------------------------------------------------------
  SortFilterModel::SortFilterModel(QObject* parent) : QSortFilterProxyModel(parent)
  {
  }

  SortFilterModel::~SortFilterModel()
  {
  }

  bool SortFilterModel::filterAcceptsRow(int source_row, const QModelIndex & source_parent ) const
  {
    if (filterRegExp().isEmpty() || filterKeyColumn() < 0)
    {
      return true;
    }
    QModelIndex itemIndex = this->sourceModel()->index(source_row,filterKeyColumn(),source_parent);
    Q_ASSERT_X(itemIndex.isValid(),"SortFilterModel::filterAcceptsRow","Invalid item index");
    ModelItem* item = static_cast<ModelItem*>(itemIndex.internalPointer());
    Q_ASSERT_X(item != 0,"SortFilterModel::filterAcceptsRow","Null item");
    if (item->type() == db::ModelItem::MODELITEMTYPE_MACRO)
    {
      return item->key().contains(filterRegExp());
    }
    else
    {
      return item->matchMacro(filterRegExp());
    }
  }

}
