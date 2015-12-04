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
#ifndef GRAPHELEMENTS_H
#define GRAPHELEMENTS_H

#include "graphdata.h"
#include "graphitems.h"
#include "graphserializer.h"
#include <QSharedPointer>
#include <QWeakPointer>
#include <QPointer>
#include <QUuid>
#include <QObject>
#include <QList>
#include <QMultiMap>
#include <QMutex>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

namespace graph
{
  class Vertex;
  class GraphBase;

  class BaseElement : public QObject, public Serializer
  {
    Q_OBJECT
    Q_PROPERTY(graph::BaseData::Ptr data READ baseDataRef)
    Q_PROPERTY(graph::BaseItem::Ptr visualization READ visualization)

  public:
    typedef QSharedPointer<BaseElement> Ptr;
    typedef QWeakPointer<BaseElement>   Ref;

    BaseElement();

    virtual ~BaseElement()
    {
      baseDataPtr.clear();
    }

    QUuid id() const
    {
      return elementId;
    }

    void setId(const QUuid& uuid)
    {
      elementId = uuid;
      emit statusUpdated(*this,(int)ElementId);
    }

    BaseItem::Ptr visualization() const
    {
      return graphicsItem;
    }

    bool hasSceneItem() const
    {
      return !graphicsItem.isNull();
    }

    BaseItem::Ptr sceneItem(BaseItem* parent = 0);

    BaseData::Ptr baseDataRef() const
    {
      return baseDataPtr;
    }

    enum StatusChange
    {
      ElementId,
      GraphAssignment,
      AddedToGraph,
      RemovedFromGraph,
      DataRef,
      ConnectionAdded,
      ConnectionRemoved
    };

  signals:
    void statusUpdated(graph::BaseElement& element, int change = 0);

  protected slots:
    virtual void refDataChanged();

  protected:
    virtual BaseItem::Ptr createSceneItem(BaseItem* parent = 0);

    QUuid         elementId;
    BaseItem::Ptr graphicsItem;
    BaseData::Ptr baseDataPtr;
  };

  class GraphElementHandler
  {
    friend class GraphBase;
  protected:
    virtual void setGraphReference(GraphBase* graph) = 0;
    virtual void setFlagDelete(bool safeToDelete) = 0;
  };

  class GraphElement : public BaseElement, public GraphElementHandler
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<GraphElement> Ptr;
    typedef QWeakPointer<GraphElement>   Ref;

    GraphElement() : BaseElement(), graphRef(), flagDelete(true)
    {
    }

    QPointer<GraphBase> graph() const
    {
      return graphRef;
    }

    bool safeToDelete() const
    {
      return (flagDelete || (!flagDelete && graphRef.isNull()));
    }

  protected:
    virtual void setGraphReference(GraphBase* graph);
    virtual void setFlagDelete(bool safeToDelete);

  protected slots:
    virtual void graphDataChanged(int change);

  private:
    QPointer<GraphBase> graphRef;
    bool                flagDelete;
  };

  class PinHandler
  {
    friend class GraphBase;
  protected:
    virtual void incConnections() = 0;
    virtual void decConnections() = 0;
  };

  class Pin : public BaseElement, public PinHandler
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<Pin> Ptr;
    typedef QWeakPointer<Pin>   Ref;

    Pin(Vertex& vertex, PinData::Ptr dataPtr) : BaseElement(),
      vertexRef(vertex), cntConnections(0)
    {
      setElement("pin");
      setDataRef(dataPtr);
    }

    Vertex& vertex() const
    {
      return vertexRef;
    }

    Defines::PinDirectionType direction() const
    {
      if (baseDataPtr.isNull())
      {
        return Defines::Undefined;
      }
      else
      {
        return dataRef()->direction();
      }
    }

    PinData::Ptr dataRef() const
    {
      return baseDataPtr.staticCast<PinData>();
    }

    int connections() const
    {
      return cntConnections;
    }

    void setDataRef(PinData::Ptr dataPtr);

    bool allowsEdgeTo(const Pin& other) const;

  protected:
    virtual BaseItem::Ptr createSceneItem(BaseItem* parent = 0);

    virtual void incConnections()
    {
      ++cntConnections;
      if (direction() == Defines::Incoming && hasSceneItem())
      {
        sceneItem().staticCast<PinItem>()->setConnectionAllowed(false);
      }
      emit statusUpdated(*this,(int)ConnectionAdded);
    }

    virtual void decConnections()
    {
      if (cntConnections == 0) return;
      --cntConnections;
      if (cntConnections == 0 && direction() == Defines::Incoming && hasSceneItem())
      {
        sceneItem().staticCast<PinItem>()->setConnectionAllowed(true);
      }
      emit statusUpdated(*this,(int)ConnectionRemoved);
    }

  private:
    Q_DISABLE_COPY(Pin)

    Vertex& vertexRef;
    int     cntConnections;
  };

  class Edge : public GraphElement
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<Edge> Ptr;
    typedef QWeakPointer<Edge>   Ref;

    Edge(Pin::Ref source, Pin::Ref destination, EdgeData::Ptr dataPtr);

    const Pin::Ref srcPin() const
    {
      return pinSrc;
    }

    const Pin::Ref destPin() const
    {
      return pinDest;
    }

    bool isValid() const
    {
      return !pinSrc.isNull() && !pinDest.isNull() && pinSrc.data()->allowsEdgeTo(*pinDest.data()) && pinDest.data()->allowsEdgeTo(*pinSrc.data());
    }

    EdgeData::Ptr dataRef() const
    {
      return baseDataPtr.staticCast<EdgeData>();
    }

    void setDataRef(EdgeData::Ptr dataPtr);

  protected:
    virtual BaseItem::Ptr createSceneItem(BaseItem* parent = 0);

  private:
    Q_DISABLE_COPY(Edge)

    Pin::Ref pinSrc;
    Pin::Ref pinDest;
  };

  class VertexHandler
  {
    friend class GraphBase;
  protected:
    virtual void addEdgeReference(Edge::Ref edgeRef, Defines::PinDirectionType direction) = 0;
    virtual void removeEdgeReference(Edge::Ref edgeRef, Defines::PinDirectionType direction) = 0;
  };

  class Vertex : public GraphElement, public VertexHandler
  {
    Q_OBJECT
    Q_PROPERTY(bool topologyForced READ topologicalOrderForced WRITE forceTopologicalOrder)
  public:
    typedef QSharedPointer<Vertex> Ptr;
    typedef QWeakPointer<Vertex>   Ref;

    typedef QMultiMap<Defines::PinDirectionType,Pin::Ptr> PinMap;
    typedef QList<Pin::Ptr> PinList;
    typedef QMultiMap<Defines::PinDirectionType,Edge::Ref> EdgeRefMap;
    typedef QList<Edge::Ref> EdgeRefList;

    Vertex(VertexData::Ptr dataPtr);
    virtual ~Vertex();

    const PinMap& pins() const
    {
      return vertexPins;
    }

    const PinList pins(Defines::PinDirectionType type) const
    {
      return vertexPins.values(type);
    }

    const EdgeRefMap& edges() const
    {
      return edgeRefs;
    }

    const EdgeRefList edges(Defines::PinDirectionType type) const
    {
      return edgeRefs.values(type);
    }

    VertexData::Ptr dataRef() const
    {
      return baseDataPtr.staticCast<VertexData>();
    }

    void setDataRef(VertexData::Ptr dataPtr);

    int topologicalOrder() const;

    void forceTopologicalOrder(bool force)
    {
      QMutexLocker lock(&mutex);
      if (forceOrder != force)
      {
        forceOrder = force;
        emit statusUpdated(*this,TopologicalOrderForced);
      }
    }

    bool topologicalOrderForced() const
    {
      QMutexLocker lock(&mutex);
      return forceOrder;
    }

    bool isInCycle() const;

    virtual void save(QXmlStreamWriter& stream) const;
    virtual bool load(QXmlStreamReader& stream);

    enum StatusChange
    {
      TopologicalOrderForced = ConnectionRemoved + 1
    };

  protected slots:
    void pinDataAdded(PinData::Ptr pinType);
    void pinDataRemoved(PinData::Ptr pinType);

  protected:
    virtual BaseItem::Ptr createSceneItem(BaseItem* parent = 0);
    virtual void addEdgeReference(Edge::Ref edgeRef, Defines::PinDirectionType direction);
    virtual void removeEdgeReference(Edge::Ref edgeRef, Defines::PinDirectionType direction);

  private:
    PinMap         vertexPins;
    EdgeRefMap     edgeRefs;
    bool           forceOrder;
    mutable QMutex mutex;
  };

}
#endif // GRAPHELEMENTS_H
