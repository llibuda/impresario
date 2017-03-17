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
#ifndef GRAPHMAIN_H
#define GRAPHMAIN_H

#include "graphserializer.h"
#include "graphdata.h"
#include "graphelements.h"
#include "grapheditor.h"
#include <QObject>
#include <QUuid>
#include <QList>
#include <QMap>
#include <QStack>
#include <QMultiMap>
#include <QMutex>
#include <QSharedPointer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

namespace graph
{
  class ElementManager : public QObject
  {
    Q_OBJECT
  public:
    typedef QList<Vertex::Ptr> VertexInstanceList;
    typedef QList<Edge::Ptr>   EdgeInstanceList;

    struct VertexDataTypeIterator
    {
      virtual bool handleVertexDataType(VertexData::Ptr vertexDataType, va_list args) = 0;
    };

    ElementManager();
    virtual ~ElementManager();

    void clear();

    bool registerVertexDataType(VertexData::Ptr vertexDataType);
    bool registerEdgeDataType(EdgeData::Ptr edgeDataType);

    Vertex::Ptr createVertexInstance(VertexData::Ptr vertexDataType);
    Vertex::Ptr createVertexInstance(const QString& typeSignature);
    Edge::Ptr createEdgeInstance(Pin::Ref source, Pin::Ref destination, EdgeData::Ptr edgeDataType);
    Edge::Ptr createEdgeInstance(Pin::Ref source, Pin::Ref destination, const QString& typeSignature);

    bool deleteVertexInstance(Vertex::Ptr vertex);
    bool deleteEdgeInstance(Edge::Ptr edge);

    VertexInstanceList vertices(VertexData::Ptr vertexType) const;
    VertexInstanceList vertices(const QString& typeSignature) const;
    EdgeInstanceList edges(EdgeData::Ptr edgeType) const;
    EdgeInstanceList edges(const QString& typeSignature) const;

    Vertex::Ptr vertexFromId(const QUuid& id) const;
    Edge::Ptr edgeFromId(const QUuid& id) const;

    void iterateVertexDataTypes(VertexDataTypeIterator* iterator, ... ) const;

  signals:
    void vertexCreated(graph::Vertex::Ptr vertexInstance);
    void vertexToBeDeleted(graph::Vertex::Ptr vertexInstance);
    void edgeCreated(graph::Edge::Ptr edgeInstance);
    void edgeToBeDeleted(graph::Edge::Ptr edgeInstance);

  protected slots:
    void vertexDataRefChanged(graph::BaseElement& element,int reason);
    void edgeDataRefChanged(graph::BaseElement& element,int reason);

  protected:
    typedef QMap<QString,VertexData::Ptr>   VertexDataTypeMap;
    typedef QMap<QString,EdgeData::Ptr>     EdgeDataTypeMap;

    virtual QString matchVertexSignature(const VertexDataTypeMap& vertexDataTypes, const QString& signature);
    virtual QString matchEdgeSignature(const EdgeDataTypeMap& edgeDataTypes, const QString& signature);

    mutable QMutex    mutex;

  private:
    Q_DISABLE_COPY(ElementManager)
    typedef QMultiMap<QString, Vertex::Ptr> VertexInstanceMap;
    typedef QMultiMap<QString, Edge::Ptr>   EdgeInstanceMap;

    VertexDataTypeMap vertexTypes;
    EdgeDataTypeMap   edgeTypes;
    VertexInstanceMap vertexInstances;
    EdgeInstanceMap   edgeInstances;
  };

  class GraphBase : public QObject, public Serializer
  {
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(graph::Scene::Ptr visualization READ visualization)
  public:
    GraphBase();

    QString id() const
    {
      return graphId.toString();
    }

    Scene::Ptr visualization() const
    {
      return graphScene;
    }

    bool hasScene() const
    {
      return !graphScene.isNull();
    }

    Scene::Ptr scene();

    const QString& name() const
    {
      return graphName;
    }

    void setName(const QString& name)
    {
      if (graphName == name) return;
      graphName = name;
      emit statusUpdated((int)Name);
    }

    int countVertices() const
    {
      return vertices.size();
    }

    int countEdges() const
    {
      return edges.size();
    }

    QList<Vertex::Ptr> vertexList() const
    {
      return vertices.values();
    }

    QList<Edge::Ptr> edgeList() const
    {
      return edges.values();
    }

    QUuid lockEditing()
    {
      if (lockId.isNull())
      {
        lockId = QUuid::createUuid();
        emit statusUpdated(EditingLocked);
        return lockId;
      }
      return QUuid();
    }

    bool unlockEditing(QUuid unlockId)
    {
      if (lockId.isNull())
      {
        return true;
      }
      if (unlockId == lockId)
      {
        lockId = QUuid();
        emit statusUpdated(EditingUnlocked);
        return true;
      }
      return false;
    }

    bool editLockActive() const
    {
      return !lockId.isNull();
    }

    bool addVertex(Vertex::Ptr vertex);
    bool removeVertex(Vertex::Ptr vertex);
    bool addEdge(Edge::Ptr edge);
    bool removeEdge(Edge::Ptr edge);

    virtual void save(QXmlStreamWriter& stream) const;

    virtual bool load(QXmlStreamReader& stream, ElementManager& manager);

    enum StatusChange
    {
      Name,
      CountVertices,
      CountEdges,
      EditingLocked,
      EditingUnlocked,
      StatusChange_End
    };

    typedef QMultiMap<int,Vertex::Ptr> ComponentMap;

    const ComponentMap components() const;

  signals:
    void vertexAdded(Vertex::Ptr vertex);
    void vertexToBeRemoved(Vertex::Ptr vertex);
    void edgeAdded(Edge::Ptr edge);
    void edgeToBeRemoved(Edge::Ptr edge);
    void statusUpdated(int change);

  protected slots:
    virtual void vertexChanged(graph::BaseElement& element, int reason);

  protected:
    typedef QMap<QUuid, Vertex::Ptr> VertexMap;
    typedef QMap<QUuid, Edge::Ptr>   EdgeMap;

    void visitVertex(Vertex* vertex, int compNr, QMap<QUuid,int>& components) const;

    VertexMap      vertices;
    EdgeMap        edges;
    QString        graphName;
    QUuid          graphId;
    QUuid          lockId;
    Scene::Ptr     graphScene;
    mutable QMutex mutex;
  };

  class DirectedGraph : public GraphBase
  {
    Q_OBJECT
    Q_PROPERTY(bool autoUpdateTopologicalOrder READ autoUpdateEnabledTopologicalOrder WRITE enableAutoUpdateTopologicalOrder)
    Q_PROPERTY(bool autoUpdateStrongComponents READ autoUpdateEnabledStrongComponents WRITE enableAutoUpdateStrongComponents)
  public:
    DirectedGraph();
    ~DirectedGraph();

    void enableAutoUpdateTopologicalOrder(bool enable = true)
    {
      topologicalOrderAutoUpdate = enable;
    }

    bool autoUpdateEnabledTopologicalOrder() const
    {
      return topologicalOrderAutoUpdate;
    }

    void enableAutoUpdateStrongComponents(bool enable = true)
    {
      strongComponentsAutoUpdate = enable;
    }

    bool autoUpdateEnabledStrongComponents() const
    {
      return strongComponentsAutoUpdate;
    }

    int vertexOrder(QUuid id) const
    {
      QMutexLocker lock(&mutex);
      return vertexInfo[id].order;
    }

    bool vertexInCycle(QUuid id) const
    {
      QMutexLocker lock(&mutex);
      return vertexInfo[id].inCycle;
    }

    int strongComponentsCount();

    bool strongComponent(int index, GraphBase::ComponentMap& result);

    const ComponentMap& topologicalOrder();
    const ComponentMap& strongComponents();

    enum StatusChange
    {
      TopologicalOrder = GraphBase::StatusChange_End + 1,
      StrongComponents,
      StatusChange_End
    };

  protected slots:
    virtual void vertexChanged(graph::BaseElement& element, int reason);
    virtual void graphChanged(int reason);

  protected:
    class DirectedVertexInfo
    {
    public:
      DirectedVertexInfo() : order(-1), inCycle(false) {}
      int  order;
      bool inCycle;
    };

    typedef QMap<QUuid,DirectedVertexInfo> VertexInfoMap;

    void visitVertex(Vertex* vertex, QSet<Vertex*>& verticesVisited, int order, VertexInfoMap& vertexMap);
    int visitVertex(Vertex* vertex, QMap<QUuid,int>& verticesVisited, int id, VertexMap& vertices, QStack<Vertex*>& stack, ComponentMap& components);

    VertexInfoMap vertexInfo;
    ComponentMap  topologicalOrderMap;
    bool          topologicalOrderUpdatedRequired;
    bool          topologicalOrderAutoUpdate;
    ComponentMap  strongComponentMap;
    bool          strongComponentsUpdatedRequired;
    bool          strongComponentsAutoUpdate;
  };

}
#endif // GRAPHMAIN_H
