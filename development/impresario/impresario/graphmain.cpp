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

#include "graphmain.h"
#include "grapheditor.h"
#include <QMutexLocker>
#include <QMetaObject>
#include <QMetaProperty>
#include <QStringList>

namespace graph
{
  //-----------------------------------------------------------------------
  // Class ElementManager
  //-----------------------------------------------------------------------
  ElementManager::ElementManager() : QObject(0), mutex(QMutex::Recursive), vertexTypes(), edgeTypes(), vertexInstances(), edgeInstances()
  {
  }

  ElementManager::~ElementManager()
  {
    clear();
  }

  void ElementManager::clear()
  {
    QMutexLocker lock(&mutex);
    vertexInstances.clear();
    edgeInstances.clear();
    vertexTypes.clear();
    edgeTypes.clear();
  }

  bool ElementManager::registerVertexDataType(VertexData::Ptr vertexDataType)
  {
    QMutexLocker lock(&mutex);
    if (vertexDataType.isNull() || vertexTypes.contains(vertexDataType->signature()))
    {
      Q_ASSERT_X(false,"graph::ElementManager::registerVertexType","Vertex type pointer is Null or given vertex type with given signature is already registered.");
      return false;
    }
    vertexTypes.insert(vertexDataType->signature(),vertexDataType);
    return true;
  }

  bool ElementManager::registerEdgeDataType(EdgeData::Ptr edgeDataType)
  {
    QMutexLocker lock(&mutex);
    if (!edgeDataType.isNull() && edgeTypes.contains(edgeDataType->signature()))
    {
      Q_ASSERT_X(false,"graph::ElementManager::registerEdgeType","Edge type pointer is Null or given edge type with given signature is already registered.");
      return false;
    }
    edgeTypes.insert(edgeDataType->signature(),edgeDataType);
    return true;
  }

  Vertex::Ptr ElementManager::createVertexInstance(VertexData::Ptr vertexDataType)
  {
    if (!vertexDataType.isNull())
    {
      return createVertexInstance(vertexDataType->signature());
    }
    else
    {
      return Vertex::Ptr();
    }
  }

  Vertex::Ptr ElementManager::createVertexInstance(const QString& typeSignature)
  {
    QMutexLocker lock(&mutex);
    QString signature = matchVertexSignature(vertexTypes,typeSignature);
    if (signature.isEmpty())
    {
      //Q_ASSERT_X(false,"graph::ElementManager::createVertexInstance","Vertex type with given signature is not registered.");
      return Vertex::Ptr();
    }
    VertexData::Ptr vertexDataType = vertexTypes[signature];
    Vertex::Ptr vertexInstance = Vertex::Ptr(new Vertex(vertexDataType->clone()));
    vertexInstances.insert(vertexDataType->signature(),vertexInstance);
    connect(vertexInstance.data(),SIGNAL(statusUpdated(graph::BaseElement&,int)),this,SLOT(vertexDataRefChanged(graph::BaseElement&,int)));
    emit vertexCreated(vertexInstance);
    return vertexInstance;
  }

  QString ElementManager::matchVertexSignature(const VertexDataTypeMap& vertexDataTypes, const QString& signature)
  {
    if (!vertexDataTypes.contains(signature))
    {
      return QString();
    }
    else
    {
      return signature;
    }
  }


  Edge::Ptr ElementManager::createEdgeInstance(Pin::Ref source, Pin::Ref destination, EdgeData::Ptr edgeDataType)
  {
    if (!edgeDataType.isNull())
    {
      return createEdgeInstance(source, destination, edgeDataType->signature());
    }
    else
    {
      return Edge::Ptr();
    }
  }

  Edge::Ptr ElementManager::createEdgeInstance(Pin::Ref source, Pin::Ref destination, const QString& typeSignature)
  {
    QMutexLocker lock(&mutex);
    QString signature = matchEdgeSignature(edgeTypes,typeSignature);
    if (signature.isEmpty())
    {
      //Q_ASSERT_X(false,"graph::ElementManager::createEdgeInstance","Edge type with given signature is not registered.");
      return Edge::Ptr();
    }
    EdgeData::Ptr edgeDataType = edgeTypes[signature];
    Edge::Ptr edgeInstance = Edge::Ptr(new Edge(source, destination, edgeDataType->clone()));
    edgeInstances.insert(edgeDataType->signature(),edgeInstance);
    connect(edgeInstance.data(),SIGNAL(statusUpdated(graph::BaseElement&,int)),this,SLOT(edgeDataRefChanged(graph::BaseElement&,int)));
    emit edgeCreated(edgeInstance);
    return edgeInstance;
  }

  QString ElementManager::matchEdgeSignature(const EdgeDataTypeMap& edgeDataTypes, const QString& signature)
  {
    if (!edgeDataTypes.contains(signature))
    {
      return QString();
    }
    else
    {
      return signature;
    }
  }

  bool ElementManager::deleteVertexInstance(Vertex::Ptr vertex)
  {
    if (!vertex.isNull() && vertex->safeToDelete() && vertexInstances.contains(vertex->dataRef()->signature(),vertex))
    {
      disconnect(vertex.data(),SIGNAL(statusUpdated(graph::BaseElement&,int)),this,SLOT(vertexDataRefChanged(graph::BaseElement&,int)));
      QMutexLocker lock(&mutex);
      emit vertexToBeDeleted(vertex);
      return vertexInstances.remove(vertex->dataRef()->signature(),vertex) == 1;
    }
    else
    {
      return false;
    }
  }

  bool ElementManager::deleteEdgeInstance(Edge::Ptr edge)
  {
    if (!edge.isNull() && edge->safeToDelete() && edgeInstances.contains(edge->dataRef()->signature(),edge))
    {
      disconnect(edge.data(),SIGNAL(statusUpdated(graph::BaseElement&,int)),this,SLOT(edgeDataRefChanged(graph::BaseElement&,int)));
      QMutexLocker lock(&mutex);
      emit edgeToBeDeleted(edge);
      return edgeInstances.remove(edge->dataRef()->signature(),edge) == 1;
    }
    else
    {
      return false;
    }
  }

  ElementManager::VertexInstanceList ElementManager::vertices(VertexData::Ptr vertexType) const
  {
    if (!vertexType.isNull())
    {
      return vertices(vertexType->signature());
    }
    else
    {
      return VertexInstanceList();
    }
  }

  ElementManager::VertexInstanceList ElementManager::vertices(const QString& typeSignature) const
  {
    QMutexLocker lock(&mutex);
    return vertexInstances.values(typeSignature);
  }

  ElementManager::EdgeInstanceList ElementManager::edges(EdgeData::Ptr edgeType) const
  {
    if (!edgeType.isNull())
    {
      return edges(edgeType->signature());
    }
    else
    {
      return EdgeInstanceList();
    }
  }

  ElementManager::EdgeInstanceList ElementManager::edges(const QString& typeSignature) const
  {
    QMutexLocker lock(&mutex);
    return edgeInstances.values(typeSignature);
  }

  Vertex::Ptr ElementManager::vertexFromId(const QUuid& id) const
  {
    QMutexLocker lock(&mutex);
    for(VertexInstanceMap::const_iterator it = vertexInstances.begin(); it != vertexInstances.end(); ++it)
    {
      if (it.value()->id() == id)
      {
        return it.value();
      }
    }
    return Vertex::Ptr();
  }

  Edge::Ptr ElementManager::edgeFromId(const QUuid& id) const
  {
    QMutexLocker lock(&mutex);
    for(EdgeInstanceMap::const_iterator it = edgeInstances.begin(); it != edgeInstances.end(); ++it)
    {
      if (it.value()->id() == id)
      {
        return it.value();
      }
    }
    return Edge::Ptr();
  }

  void ElementManager::vertexDataRefChanged(graph::BaseElement& element,int reason)
  {
    if (reason != BaseElement::DataRef) return;
    QMutexLocker lock(&mutex);
    Vertex& vertex = static_cast<Vertex&>(element);
    for(VertexInstanceMap::iterator it = vertexInstances.begin(); it != vertexInstances.end(); ++it)
    {
      if (it.value()->id() == vertex.id())
      {
        Vertex::Ptr vertexPtr = it.value();
        vertexInstances.erase(it);
        vertexInstances.insert(vertexPtr->dataRef()->signature(),vertexPtr);
        return;
      }
    }
  }

  void ElementManager::edgeDataRefChanged(graph::BaseElement& element,int reason)
  {
    if (reason != BaseElement::DataRef) return;
    QMutexLocker lock(&mutex);
    Edge& edge = static_cast<Edge&>(element);
    for(EdgeInstanceMap::iterator it = edgeInstances.begin(); it != edgeInstances.end(); ++it)
    {
      if (it.value()->id() == edge.id())
      {
        Edge::Ptr edgePtr = it.value();
        edgeInstances.erase(it);
        edgeInstances.insert(edgePtr->dataRef()->signature(),edgePtr);
        return;
      }
    }
  }

  void ElementManager::iterateVertexDataTypes(IteratorFunction iterator, ... ) const
  {
    Q_ASSERT(iterator != 0);
    QMutexLocker lock(&mutex);
    for(VertexDataTypeMap::const_iterator it = vertexTypes.begin(); it != vertexTypes.end(); ++it)
    {
      va_list args;
      va_start(args,iterator);
      if (!(*iterator)(it.value(),args))
      {
        va_end(args);
        return;
      }
      va_end(args);
    }
  }

  //-----------------------------------------------------------------------
  // Class GraphBase
  //-----------------------------------------------------------------------
  GraphBase::GraphBase() : QObject(0), Serializer("graph",GraphBase::staticMetaObject.propertyOffset(),this), vertices(), edges(), graphName(), graphId(QUuid::createUuid()), lockId(),
    graphScene(), mutex(QMutex::Recursive)
  {
  }

  Scene::Ptr GraphBase::scene()
  {
    if (graphScene.isNull())
    {
      graphScene = Scene::Ptr(new Scene());
    }
    return graphScene;
  }

  bool GraphBase::addVertex(Vertex::Ptr vertex)
  {
    QMutexLocker lock(&mutex);
    if (!lockId.isNull()) return false;
    Q_ASSERT_X(!vertex.isNull(),"graph::GraphBase::addVertex","Vertex pointer is 0!");
    if (!vertex.isNull() && vertex->edges().count() == 0 && !vertices.contains(vertex->id()) && vertex->safeToDelete())
    {
      GraphElementHandler* handler = static_cast<GraphElementHandler*>(vertex.data());
      handler->setGraphReference(this);
      handler->setFlagDelete(false);
      vertices.insert(vertex->id(), vertex);
      connect(vertex.data(),SIGNAL(statusUpdated(graph::BaseElement&,int)),this,SLOT(vertexChanged(graph::BaseElement&,int)));
      emit vertexAdded(vertex);
      emit statusUpdated((int)CountVertices);
      return true;
    }
    return false;
  }

  bool GraphBase::removeVertex(Vertex::Ptr vertex)
  {
    QMutexLocker lock(&mutex);
    if (!lockId.isNull()) return false;
    Q_ASSERT_X(!vertex.isNull(),"graph::GraphBase::removeVertex","Vertex pointer is 0!");
    if (!vertex.isNull() && vertex->edges().count() == 0 && vertices.contains(vertex->id()) && vertex->graph() == this)
    {
      GraphElementHandler* handler = static_cast<GraphElementHandler*>(vertex.data());
      handler->setFlagDelete(true);
      emit vertexToBeRemoved(vertex);
      vertices.remove(vertex->id());
      disconnect(vertex.data(),SIGNAL(statusUpdated(graph::BaseElement&,int)),this,SLOT(vertexChanged(graph::BaseElement&,int)));
      emit statusUpdated((int)CountVertices);
      return true;
    }
    return false;
  }

  bool GraphBase::addEdge(Edge::Ptr edge)
  {
    QMutexLocker lock(&mutex);
    if (!lockId.isNull()) return false;
    Q_ASSERT_X(!edge.isNull(),"graph::GraphBase::addEdge","Edge pointer is 0!");
    if (!edge.isNull() && !edges.contains(edge->id()) && edge->safeToDelete() && edge->isValid() &&
        vertices.contains(edge->destPin().data()->vertex().id()) && vertices.contains(edge->srcPin().data()->vertex().id()))
    {
      GraphElementHandler* handler = static_cast<GraphElementHandler*>(edge.data());
      handler->setFlagDelete(false);
      handler->setGraphReference(this);
      VertexHandler* handlerSrc = static_cast<VertexHandler*>(&edge->srcPin().data()->vertex());
      VertexHandler* handlerDest = static_cast<VertexHandler*>(&edge->destPin().data()->vertex());
      handlerSrc->addEdgeReference(edge.toWeakRef(),edge->srcPin().data()->direction());
      handlerDest->addEdgeReference(edge.toWeakRef(),edge->destPin().data()->direction());
      PinHandler* handlerSrcPin = static_cast<PinHandler*>(edge->srcPin().data());
      PinHandler* handlerDestPin = static_cast<PinHandler*>(edge->destPin().data());
      handlerSrcPin->incConnections();
      handlerDestPin->incConnections();
      edges.insert(edge->id(), edge);
      emit edgeAdded(edge);
      emit statusUpdated((int)CountEdges);
      return true;
    }
    return false;
  }

  bool GraphBase::removeEdge(Edge::Ptr edge)
  {
    QMutexLocker lock(&mutex);
    if (!lockId.isNull()) return false;
    Q_ASSERT_X(!edge.isNull(),"graph::GraphBase::removeEdge","Edge pointer is 0!");
    if (!edge.isNull() && edges.contains(edge->id()) && edge->graph() == this)
    {
      GraphElementHandler* handler = static_cast<GraphElementHandler*>(edge.data());
      handler->setFlagDelete(true);
      VertexHandler* handlerSrc = static_cast<VertexHandler*>(&edge->srcPin().data()->vertex());
      VertexHandler* handlerDest = static_cast<VertexHandler*>(&edge->destPin().data()->vertex());
      handlerSrc->removeEdgeReference(edge.toWeakRef(),edge->srcPin().data()->direction());
      handlerDest->removeEdgeReference(edge.toWeakRef(),edge->destPin().data()->direction());
      PinHandler* handlerSrcPin = static_cast<PinHandler*>(edge->srcPin().data());
      PinHandler* handlerDestPin = static_cast<PinHandler*>(edge->destPin().data());
      handlerSrcPin->decConnections();
      handlerDestPin->decConnections();
      emit edgeToBeRemoved(edge);
      edges.remove(edge->id());
      emit statusUpdated((int)CountEdges);
      return true;
    }
    return false;
  }

  void GraphBase::save(QXmlStreamWriter& stream) const
  {
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    writeElementStart(stream);
    stream.writeAttribute("id",graphId.toString());
    writeProperties(stream);

    stream.writeStartElement("vertices");
    for(VertexMap::const_iterator it = vertices.begin(); it != vertices.end(); ++it)
    {
      it.value()->save(stream);
    }
    stream.writeEndElement(); // vertices
    stream.writeStartElement("edges");
    for(EdgeMap::const_iterator it = edges.begin(); it != edges.end(); ++it)
    {
      it.value()->save(stream);
    }
    stream.writeEndElement(); // edges

    writeElementEnd(stream); // graph
    stream.writeEndDocument();
  }

  bool GraphBase::load(QXmlStreamReader& stream, ElementManager& manager)
  {
    if (stream.readNextStartElement())
    {
      if (!readElementStart(stream)) return false;
      graphId = QUuid(stream.attributes().value("id").toString());
      if (graphId.isNull())
      {
        stream.raiseError(QString(QObject::tr("At line %1, column %2: Graph has an invalid id '%3'.")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(graphId.toString()));
        return false;
      }
      if (!readProperties(stream)) return false;

      // all properties should have been processed here, now try to read vertices and edges
      QMap<QUuid,Pin::Ptr> pinMap;
      QStringList msgWarningsVertices;
      if (stream.readNextStartElement() && stream.name() == "vertices") // if element is "vertices" then read vertices
      {
        while(stream.readNextStartElement())
        {
          QString signature = stream.attributes().value("dataTypeSignature").toString();
          if (!signature.isEmpty())
          {
            Vertex::Ptr vertexPtr = manager.createVertexInstance(signature);
            if (!vertexPtr.isNull())
            {
              if (signature != vertexPtr->dataRef()->signature())
              {
                msgWarningsVertices.append(QString(tr("at line %1, column %2: Vertex with signature \"%3\" was substituted by vertex with signature \"%4\".")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(signature).arg(vertexPtr->dataRef()->signature()));
              }
              if (!(vertexPtr->load(stream) && addVertex(vertexPtr)))
              {
                manager.deleteVertexInstance(vertexPtr);
                return false;
              }
              // we need to keep track of all pins created for the edges to load
              const Vertex::PinMap& pins = vertexPtr->pins();
              for(Vertex::PinMap::const_iterator it = pins.begin(); it != pins.end(); ++it)
              {
                pinMap.insert(it.value()->id(),it.value());
              }
            }
            else
            {
              msgWarningsVertices.append(QString(tr("at line %1, column %2: Vertex with unknown signature \"%3\" not created.")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(signature));
              stream.skipCurrentElement();
            }
          }
          else
          {
            stream.raiseError(QString(tr("At line %1, column %2: Missing or undefined attribute 'dataTypeSignature' for element '%3'.")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(stream.name().toString()));
            return false;
          }
        }
      }
      // load edges
      QStringList msgWarningsEdges;
      if (stream.readNextStartElement() && stream.name() == "edges") // if element is "edges" then read edges
      {
        while(stream.readNextStartElement())
        {
          QString signature = stream.attributes().value("dataTypeSignature").toString();
          if (!signature.isEmpty())
          {
            QUuid pinSrcId = QUuid(stream.attributes().value("srcPinId").toString());
            QUuid pinDstId = QUuid(stream.attributes().value("destPinId").toString());
            if (pinMap.contains(pinSrcId) && pinMap.contains(pinDstId))
            {
              Edge::Ptr edgePtr = manager.createEdgeInstance(pinMap[pinSrcId].toWeakRef(),pinMap[pinDstId].toWeakRef(),signature);
              if (!edgePtr.isNull())
              {
                if (edgePtr->load(stream))
                {
                  if (!addEdge(edgePtr))
                  {
                    msgWarningsEdges.append(QString(tr("at line %1, column %2: Edge with signature \"%3\" trying to connect incompatible pins.")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(signature));
                    manager.deleteEdgeInstance(edgePtr);
                  }
                }
                else
                {
                  manager.deleteEdgeInstance(edgePtr);
                  return false;
                }
              }
              else
              {
                msgWarningsEdges.append(QString(tr("at line %1, column %2: Edge with unknown signature \"%3\" not created.")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(signature));
                stream.skipCurrentElement();
              }
            }
            else
            {
              msgWarningsEdges.append(QString(tr("at line %1, column %2: Source or destination pin not defined for edge with signature \"%3\".")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(signature));
              stream.skipCurrentElement();
            }
          }
          else
          {
            stream.raiseError(QString(tr("At line %1, column %2: Missing or undefined attribute 'dataTypeSignature' for element '%3'.")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(stream.name().toString()));
            return false;
          }
        }
      }
      if (!readElementEnd(stream)) return false;

      QString warnings;
      if (msgWarningsVertices.count() > 0)
      {
        if (msgWarningsVertices.count() == 1)
        {
          warnings += QString(tr("%1 warning occurred while loading vertices:\n")).arg(msgWarningsVertices.count());
        }
        else
        {
          warnings += QString(tr("%1 warnings occurred while loading vertices:\n")).arg(msgWarningsVertices.count());
        }
        warnings += msgWarningsVertices.join('\n');
      }
      if (msgWarningsEdges.count() > 0)
      {
        if (!warnings.isEmpty()) warnings += '\n';

        if (msgWarningsEdges.count() == 1)
        {
          warnings += QString(tr("%1 warning occurred while loading edges:\n")).arg(msgWarningsEdges.count());
        }
        else
        {
          warnings += QString(tr("%1 warnings occurred while loading edges:\n")).arg(msgWarningsEdges.count());
        }
        warnings += msgWarningsEdges.join('\n');
      }
      if (!warnings.isEmpty())
      {
        if (countVertices() > 0)
        {
          stream.raiseError(warnings);
        }
        else
        {
          warnings += '\n' + tr("Graph does not contain any vertices.");
          stream.raiseError(warnings);
          return false;
        }
      }
    }
    else
    {
      stream.raiseError(QString(QObject::tr("At line %1, column %2: Failed to read root element. Is document empty?")).arg(stream.lineNumber()).arg(stream.columnNumber()));
      return false;
    }
    return true;
  }

  void GraphBase::vertexChanged(graph::BaseElement& element, int reason)
  {
    QMutexLocker lock(&mutex);
    if (!lockId.isNull()) return;
    if (reason == BaseElement::DataRef)
    {
      Vertex& vertex = static_cast<Vertex&>(element);
      // check whether all edges referenced by changed vertex are still valid
      QList<Edge::Ptr> invalidEdges;
      const Vertex::EdgeRefMap& edgeMap = vertex.edges();
      for(Vertex::EdgeRefMap::const_iterator it = edgeMap.begin(); it != edgeMap.end(); ++it)
      {
        Edge::Ptr edgePtr = it.value().toStrongRef();
        if (edges.contains(edgePtr->id()) && !edgePtr->isValid())
        {
          invalidEdges.append(edgePtr);
        }
      }
      // remove invalid egdes from graph
      foreach(Edge::Ptr ptr,invalidEdges)
      {
        removeEdge(ptr);
      }
    }
  }

  const GraphBase::ComponentMap GraphBase::components() const
  {
    QMutexLocker    lock(&mutex);
    QMap<QUuid,int> componentIdMap;
    int             compNr = 0;
    foreach(Vertex::Ptr vertex, vertices)
    {
      if (!componentIdMap.contains(vertex->id()))
      {
        visitVertex(vertex.data(),compNr++,componentIdMap);
      }
    }
    ComponentMap compMap;
    foreach(Vertex::Ptr vertex, vertices)
    {
      compMap.insert(componentIdMap[vertex->id()],vertex);
    }
    return compMap;
  }

  void GraphBase::visitVertex(Vertex* vertex, int compNr, QMap<QUuid,int>& components) const
  {
    components.insert(vertex->id(),compNr);
    Vertex::EdgeRefList edgeList = vertex->edges(Defines::Outgoing);
    foreach(Edge::Ref edgeRef,edgeList)
    {
      Vertex& v = edgeRef.toStrongRef()->destPin().toStrongRef()->vertex();
      if (!components.contains(v.id()))
      {
        visitVertex(&v,compNr,components);
      }
    }
    edgeList = vertex->edges(Defines::Incoming);
    foreach(Edge::Ref edgeRef,edgeList)
    {
      Vertex& v = edgeRef.toStrongRef()->srcPin().toStrongRef()->vertex();
      if (!components.contains(v.id()))
      {
        visitVertex(&v,compNr,components);
      }
    }
  }

  //-----------------------------------------------------------------------
  // Class DirectedGraph
  //-----------------------------------------------------------------------
  DirectedGraph::DirectedGraph() : GraphBase(), vertexInfo(),
    topologicalOrderMap(), topologicalOrderUpdatedRequired(false), topologicalOrderAutoUpdate(true),
    strongComponentMap(), strongComponentsUpdatedRequired(false), strongComponentsAutoUpdate(false)
  {
    connect(this,SIGNAL(statusUpdated(int)),this,SLOT(graphChanged(int)));
  }

  DirectedGraph::~DirectedGraph()
  {
    topologicalOrderMap.clear();
    strongComponentMap.clear();
  }

  int DirectedGraph::strongComponentsCount()
  {
    const ComponentMap& comps = strongComponents();
    return comps.uniqueKeys().size();
  }

  bool DirectedGraph::strongComponent(int index, GraphBase::ComponentMap& result)
  {
    result.clear();
    const ComponentMap& comps = strongComponents();
    QMutexLocker lock(&mutex);
    if (index >= comps.uniqueKeys().size())
    {
      return false;
    }
    QList<Vertex::Ptr> compVertices = comps.values(index);
    foreach(Vertex::Ptr vertex, compVertices)
    {
      result.insert(vertexInfo[vertex->id()].order,vertex);
    }
    return true;
  }

  const GraphBase::ComponentMap& DirectedGraph::topologicalOrder()
  {
    QMutexLocker lock(&mutex);
    if (!topologicalOrderUpdatedRequired)
    {
      return topologicalOrderMap;
    }
    topologicalOrderUpdatedRequired = false;
    topologicalOrderMap.clear();
    // get all vertices without predecessor -> starting vertices
    QList<Vertex*> startVertices;
    for(VertexMap::iterator it = vertices.begin(); it != vertices.end(); ++it)
    {
      if (it.value()->edges().count(Defines::Incoming) == 0)
      {
        startVertices.append(it.value().data());
      }
    }
    // now start labeling the vertices starting at startVertices
    vertexInfo.clear();
    QSet<Vertex*> visitedVertices;
    foreach(Vertex* v,startVertices)
    {
      visitVertex(v,visitedVertices,0,vertexInfo);
    }
    // if we generated the same number of vertex information as the number of vertices in the graph
    // we are done, otherwise we'll have to investigate the strong components
    if (vertexInfo.size() != vertices.size())
    {
      // look for strongly connected components
      const ComponentMap& components = strongComponents();
      startVertices.clear();
      // get all vertices which have forced topological order and are in a cycle
      for(ComponentMap::const_iterator it = components.begin(); it != components.end(); ++it)
      {
        vertexInfo[it.value()->id()].inCycle = true;
        if (it.value()->topologicalOrderForced())
        {
          startVertices.append(it.value().data());
        }
      }
      // now label new set of vertices
      visitedVertices.clear();
      foreach(Vertex* v,startVertices)
      {
        visitVertex(v,visitedVertices,0,vertexInfo);
      }
    }
    // finally write result into map
    for(VertexMap::iterator it = vertices.begin(); it != vertices.end(); ++it)
    {
      topologicalOrderMap.insert(vertexInfo[it.value()->id()].order,it.value());
    }
    // trigger scene update if there is a scene
    if (hasScene()) scene()->update();
    emit statusUpdated(TopologicalOrder);
    return topologicalOrderMap;
  }

  const GraphBase::ComponentMap& DirectedGraph::strongComponents()
  {
    QMutexLocker lock(&mutex);
    if (!strongComponentsUpdatedRequired)
    {
      return strongComponentMap;
    }
    strongComponentsUpdatedRequired = false;
    strongComponentMap.clear();
    QStack<Vertex*> vertexStack;
    QMap<QUuid,int> visitedVertices;
    int id = 0;
    for(VertexMap::iterator it = vertices.begin(); it != vertices.end(); ++it)
    {
      if (!visitedVertices.contains(it.value()->id()))
      {
        visitVertex(it.value().data(),visitedVertices,id,vertices,vertexStack,strongComponentMap);
      }
    }
    emit statusUpdated(StrongComponents);
    return strongComponentMap;
  }

  void DirectedGraph::vertexChanged(BaseElement &element, int reason)
  {
    QMutexLocker lock(&mutex);
    GraphBase::vertexChanged(element,reason);
    if (reason == Vertex::TopologicalOrderForced)
    {
      topologicalOrderUpdatedRequired = true;
      if (topologicalOrderAutoUpdate)
      {
        topologicalOrder();
      }
    }
  }

  void DirectedGraph::graphChanged(int reason)
  {
    QMutexLocker lock(&mutex);
    switch(reason)
    {
      case GraphBase::CountVertices:
        topologicalOrderUpdatedRequired = true;
        if (topologicalOrderAutoUpdate)
        {
          topologicalOrder();
        }
        break;
      case GraphBase::CountEdges:
        topologicalOrderUpdatedRequired = true;
        strongComponentsUpdatedRequired = true;
        if (topologicalOrderAutoUpdate)
        {
          topologicalOrder();
        }
        if (strongComponentsAutoUpdate && !topologicalOrderAutoUpdate)
        {
          strongComponents();
        }
        break;
      default:
        break;
    }
  }

  void DirectedGraph::visitVertex(Vertex* vertex, QSet<Vertex*>& verticesVisited, int order, VertexInfoMap& vertexMap)
  {
    if (vertexMap[vertex->id()].order <= order && !verticesVisited.contains(vertex))
    {
      vertexMap[vertex->id()].order = order;
      verticesVisited.insert(vertex);
      Vertex::EdgeRefList edgeList = vertex->edges(Defines::Outgoing);
      foreach(Edge::Ref edgeRef,edgeList)
      {
        Vertex& v = edgeRef.toStrongRef()->destPin().toStrongRef()->vertex();
        visitVertex(&v,verticesVisited,order + 1,vertexMap);
      }
      verticesVisited.remove(vertex);
    }
  }

  int DirectedGraph::visitVertex(Vertex* vertex, QMap<QUuid,int>& verticesVisited, int id, VertexMap& vertices, QStack<Vertex*>& stack, GraphBase::ComponentMap& components)
  {
    int m, min;
    verticesVisited[vertex->id()] = ++id;
    min = id;
    stack.push(vertex);
    Vertex::EdgeRefList edgeList = vertex->edges(Defines::Outgoing);
    foreach(Edge::Ref edgeRef,edgeList)
    {
      Vertex& v = edgeRef.toStrongRef()->destPin().toStrongRef()->vertex();
      m = (!verticesVisited.contains(v.id())) ? visitVertex(&v,verticesVisited,id,vertices,stack,components) : verticesVisited[v.id()];
      if (m < min) min = m;
    }
    if (min == verticesVisited[vertex->id()])
    {
      int k = verticesVisited[vertex->id()];
      // we are only interested in components consisting of more than one vertex
      if (k != verticesVisited[stack.top()->id()])
      {
        int componentId = components.uniqueKeys().size();
        do
        {
          Vertex* vout = stack.pop();
          components.insert(componentId,vertices[vout->id()]);
          m = verticesVisited[vout->id()];
          verticesVisited[vout->id()] = vertices.size() + 1;
        } while(m != k);
      }
      else
      {
        Vertex* vout = stack.pop();
        verticesVisited[vout->id()] = vertices.size() + 1;
      }
    }
    return min;
  }

}
