/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2017  Lars Libuda
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
#ifndef GRAPHDATA_H
#define GRAPHDATA_H

#include "graphdefines.h"
#include "graphserializer.h"
#include <QObject>
#include <QMultiMap>
#include <QSharedPointer>
#include <QString>

namespace graph
{
  class BaseElement;
  class BaseItem;

  class BaseData : public QObject, public Serializer
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<BaseData> Ptr;

    BaseData();

    virtual QSharedPointer<BaseItem> createVisualization(BaseElement& elementRef, BaseItem* parent = 0);

  public slots:
    virtual void elementStatusUpdated(graph::BaseElement& /*element*/, int /*change*/)
    {
    }

  signals:
    void dataUpdated();

  };

  class GraphData : public BaseData
  {
    Q_OBJECT
    //Q_PROPERTY(QString signature READ signature WRITE setSignature)
  public:
    typedef QSharedPointer<GraphData> Ptr;

    GraphData(const QString& signature = "") : BaseData(), typeSignature(signature)
    {
    }

    void setSignature(const QString& signature)
    {
      typeSignature = signature;
    }

    const QString& signature() const
    {
      return typeSignature;
    }

  private:
    QString typeSignature;
  };

  class PinData : public BaseData
  {
    Q_OBJECT
    Q_PROPERTY(QString name READ id)
  public:
    typedef QSharedPointer<PinData> Ptr;

    virtual bool allowConnectionTo(const PinData& /*other*/) const
    {
      return true;
    }

    Defines::PinDirectionType direction() const
    {
      return dir;
    }

    const QString& id() const
    {
      return uniqueName;
    }

    bool isCompatibleTo(const PinData::Ptr other) const
    {
      return (dir == other->dir) && (uniqueName == other->uniqueName);
    }

    bool isCompatibleTo(const PinData& other) const
    {
      return *this == other;
    }

    bool operator==(const PinData& other) const
    {
      return (dir == other.dir) && (uniqueName == other.uniqueName);
    }

    bool operator !=(const PinData& other) const
    {
      return (dir != other.dir) || (uniqueName != other.uniqueName);
    }

  protected:
    Q_INVOKABLE PinData(const QString id, Defines::PinDirectionType direction) :
      BaseData(), dir(direction), uniqueName(id)
    {
    }

  private:
    Q_DISABLE_COPY(PinData)

    Defines::PinDirectionType dir;
    QString                   uniqueName;
  };

  class EdgeData : public GraphData
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<EdgeData> Ptr;

    EdgeData() : GraphData()
    {
    }

    virtual Ptr clone()
    {
      Ptr ptr = Ptr(new EdgeData());
      ptr->setSignature(this->signature());
      return ptr;
    }
  };

  class VertexData : public GraphData
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<VertexData> Ptr;
    typedef QMap<QString, PinData::Ptr> PinDataMap;

    VertexData() : GraphData(), pins()
    {
    }

    virtual ~VertexData()
    {
      pins.clear();
    }

    virtual Ptr clone() = 0;

    PinData::Ptr addPinData(const QMetaObject& pinTypeInfo, QString id, Defines::PinDirectionType direction);
    bool addPinData(PinData::Ptr pinType);

    bool removePinData(PinData::Ptr pinType);

    const PinDataMap& pinData() const
    {
      return pins;
    }

  signals:
    void pinDataAdded(PinData::Ptr);
    void pinDataRemoved(PinData::Ptr);

  private:
    PinDataMap pins;
  };

}

Q_DECLARE_METATYPE(graph::BaseData::Ptr)

#endif // GRAPHDATA_H
