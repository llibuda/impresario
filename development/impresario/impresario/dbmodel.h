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
#ifndef DBMODEL_H
#define DBMODEL_H

#include "appmacro.h"
#include "dbviewconfig.h"
#include "appmacromanager.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QVariant>
#include <QMutex>
#include <QObject>
#include <QMimeData>
#include <QStringList>
#include <QtVariantPropertyManager>
#include <QList>
#include <QtVariantProperty>
#include <QtConcurrent>

namespace db
{
  class ModelItem
  {
  public:
    static const int TypeRole = Qt::UserRole + 1;

    enum ModelItemType
    {
      MODELITEMTYPE_GENERIC,
      MODELITEMTYPE_ROOT,
      MODELITEMTYPE_CREATOR,
      MODELITEMTYPE_GROUP,
      MODELITEMTYPE_LIBRARY,
      MODELITEMTYPE_LIBRARYFILE,
      MODELITEMTYPE_TYPE,
      MODELITEMTYPE_BUILD,
      MODELITEMTYPE_MACRO,
      MODELITEMTYPE_VIEWER
    };

    ModelItem(ModelItem* parent = 0);
    virtual ~ModelItem();

    void appendChild(ModelItem *child);
    int row() const;
    ModelItem* find(const QString& key) const;
    bool matchMacro(const QRegExp pattern) const;

    ModelItem* child(int row) const
    {
      return childItem.value(row);
    }

    int childCount() const
    {
      return childItem.count();
    }

    ModelItem* parent() const
    {
      return parentItem;
    }

    const QString& key() const
    {
      return searchKey;
    }

    ModelItemType type() const
    {
      return itemType;
    }

    virtual int columnCount() const
    {
      return 1;
    }

    virtual QVariant data(int column, int role = Qt::DisplayRole) const = 0;
    virtual QList<QtVariantProperty*> properties(QtVariantPropertyManager& propManager) const;

  protected:
    QString       searchKey;
    ModelItemType itemType;

  private:
    QList<ModelItem*> childItem;
    ModelItem* parentItem;

  };

  class ModelItemRoot : public ModelItem
  {
  public:
    ModelItemRoot(const QString& viewTableFormat, ModelItem* parent = 0);
    virtual QVariant data(int column, int role = Qt::DisplayRole) const;
    virtual QList<QtVariantProperty*> properties(QtVariantPropertyManager& propManager) const;
    virtual int columnCount() const;
    int levelCount() const;
  };

  class ModelItemCreator : public ModelItem
  {
  public:
    ModelItemCreator(const QString& creator, ModelItem* parent = 0);
    virtual QVariant data(int column, int role = Qt::DisplayRole) const;
    virtual QList<QtVariantProperty*> properties(QtVariantPropertyManager& propManager) const;
  };

  class ModelItemGroup : public ModelItem
  {
  public:
    ModelItemGroup(const QString& group, ModelItem* parent = 0);
    virtual QVariant data(int column, int role = Qt::DisplayRole) const;
    virtual QList<QtVariantProperty*> properties(QtVariantPropertyManager& propManager) const;
  };

  class ModelItemBuild : public ModelItem
  {
  public:
    ModelItemBuild(const QString& build, ModelItem* parent = 0);
    virtual QVariant data(int column, int role = Qt::DisplayRole) const;
    virtual QList<QtVariantProperty*> properties(QtVariantPropertyManager& propManager) const;
  };

  class ModelItemType : public ModelItem
  {
  public:
    ModelItemType(const QString& type, ModelItem* parent = 0);
    virtual QVariant data(int column, int role = Qt::DisplayRole) const;
    virtual QList<QtVariantProperty*> properties(QtVariantPropertyManager& propManager) const;
  };

  class ModelItemLibrary : public ModelItem
  {
  public:
    ModelItemLibrary(const QString& libraryName, ModelItem* parent = 0);
    virtual QVariant data(int column, int role = Qt::DisplayRole) const;
    virtual QList<QtVariantProperty*> properties(QtVariantPropertyManager& propManager) const;
  };

  class ModelItemLibraryFile : public ModelItem
  {
  public:
    ModelItemLibraryFile(const app::MacroLibrary& library, ModelItem* parent = 0);
    virtual QVariant data(int column, int role = Qt::DisplayRole) const;
    virtual QList<QtVariantProperty*> properties(QtVariantPropertyManager& propManager) const;

    const app::MacroLibrary& library() const
    {
      return *libRef;
    }

  private:
    const app::MacroLibrary* libRef;

  };

  class ModelItemMacro : public ModelItem
  {
  public:
    ModelItemMacro(const app::Macro& macro, const QString& colDef, ModelItem* parent = 0);
    virtual QVariant data(int column, int role = Qt::DisplayRole) const;
    virtual QList<QtVariantProperty*> properties(QtVariantPropertyManager& propManager) const;
    virtual int columnCount() const;

    const app::Macro& macro() const
    {
      return macroRef;
    }

  protected:
    const app::Macro& macroRef;
    const QString columnDef;
  };

  class ModelItemViewer : public ModelItemMacro
  {
  public:
    ModelItemViewer(const app::Macro& macro, const QString& colDef, ModelItem* parent = 0);
    virtual QVariant data(int column, int role = Qt::DisplayRole) const;
    virtual QList<QtVariantProperty*> properties(QtVariantPropertyManager& propManager) const;
  };

  class ModelCreator : public QObject
  {
    Q_OBJECT
  public:
    ModelCreator(QObject* parent = 0);
    ~ModelCreator();

    void createViewModel(const ViewFormat& format, const ViewFilter& filter);

  signals:
    void modelCreated(ModelItemRoot* rootItem);

  private slots:
    void creatingViewModelFinished();

  private:
    static ModelItemRoot* doCreateModel(const ViewFormat& format, const ViewFilter& filter);
    static bool handleVertexDataType(graph::VertexData::Ptr macroPtr, va_list args);
    static void addModelItem(const app::Macro& macro, const QString& treeFormat, const QString& tableFormat, int pos, ModelItem* parent);

    QFutureWatcher<ModelItemRoot*> watcher;
  };

  class Model : public QAbstractItemModel
  {
    Q_OBJECT
  public:

    enum ModelUpdateReason
    {
      UPDATE_INITIAL,
      UPDATE_VIEWCHANGED,
      UPDATE_FILTERCHANGED,
      UPDATE_SHOWVIEWERS
    };

    Model(QObject *parent = 0);
    virtual ~Model();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
    virtual QStringList mimeTypes() const;

  signals:
    void modelUpdateStart(db::Model::ModelUpdateReason reason);
    void modelUpdateDone(int level);

  public slots:
    void updateModel(ModelUpdateReason reason, const ViewFormat* format, const ViewFilter* filter);

  private slots:
    void updateInternalModel(ModelItemRoot* root);

  private:
    ModelItemRoot* rootItem;
    ModelCreator   modelCreator;
  };

  class SortFilterModel : public QSortFilterProxyModel
  {
    Q_OBJECT
  public:
    SortFilterModel(QObject *parent = 0);
    ~SortFilterModel();

  protected:
    virtual bool filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const;

  };

}
#endif // DBMODEL_H
