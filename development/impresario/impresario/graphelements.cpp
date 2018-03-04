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

#include "graphelements.h"
#include "graphmain.h"
#include <QSet>
#include <QMetaObject>
#include <QMetaProperty>

namespace graph
{
  //-----------------------------------------------------------------------
  // Class BaseElement
  //-----------------------------------------------------------------------
  BaseElement::BaseElement() : QObject(0), Serializer("element",BaseElement::staticMetaObject.propertyOffset(),this),
    elementId(QUuid::createUuid()), graphicsItem(), baseDataPtr()
  {
  }

  void BaseElement::refDataChanged()
  {
    if (hasSceneItem() && sceneItem()->scene() != 0)
    {
      sceneItem()->update();
    }
  }

  BaseItem::Ptr BaseElement::sceneItem(BaseItem* parent)
  {
    if (graphicsItem.isNull())
    {
      if (!baseDataPtr.isNull())
      {
        graphicsItem = baseDataPtr->createVisualization(*this,parent);
        if (graphicsItem.isNull())
        {
          graphicsItem = createSceneItem(parent);
        }
      }
      else
      {
        graphicsItem = createSceneItem(parent);
      }
    }
    return graphicsItem;
  }

  BaseItem::Ptr BaseElement::createSceneItem(BaseItem* /*parent*/)
  {
    return BaseItem::Ptr();
  }

  //-----------------------------------------------------------------------
  // Class GraphElement
  //-----------------------------------------------------------------------
  void GraphElement::setGraphReference(GraphBase* graph)
  {
    if (!graphRef.isNull() && graph != 0) return;
    if (!graphRef.isNull())
    {
      disconnect(graphRef.data(),SIGNAL(statusUpdated(int)),this,SLOT(graphDataChanged(int)));
      if (graphRef->hasScene() && !graphicsItem.isNull() && graphicsItem->scene() != 0)
      {
        graphRef->scene()->removeItem(graphicsItem.data());
      }
    }
    graphRef = graph;
    if (!graphRef.isNull())
    {
      connect(graphRef.data(),SIGNAL(statusUpdated(int)),this,SLOT(graphDataChanged(int)));
      if (graphRef->hasScene() && !graphicsItem.isNull() && graphicsItem->scene() == 0)
      {
        graphRef->scene()->addItem(graphicsItem.data());
      }
    }
    emit statusUpdated(*this,(int)GraphAssignment);
  }

  void GraphElement::setFlagDelete(bool safeToDelete)
  {
    if (flagDelete == safeToDelete) return;
    flagDelete = safeToDelete;
    if (!graphRef.isNull() && graphRef->hasScene())
    {
      if (flagDelete && !graphicsItem.isNull() && graphicsItem->scene() != 0)
      {
        graphRef->scene()->removeItem(sceneItem().data());
      }
      else if (!flagDelete && (graphicsItem.isNull() || (!graphicsItem.isNull() && graphicsItem->scene() == 0)))
      {
        graphRef->scene()->addItem(sceneItem().data());
      }
    }
    emit statusUpdated(*this,(flagDelete) ? (int)RemovedFromGraph : (int)AddedToGraph);
  }

  void GraphElement::graphDataChanged(int change)
  {
    if (change == GraphBase::Name)
    {
      emit statusUpdated(*this,(int)GraphAssignment);
    }
  }

  //-----------------------------------------------------------------------
  // Class Pin
  //-----------------------------------------------------------------------
  void Pin::setDataRef(PinData::Ptr dataPtr)
  {
    if (baseDataPtr == dataPtr) return;
    if (!baseDataPtr.isNull())
    {
      disconnect(baseDataPtr.data(),SIGNAL(dataUpdated()),this,SLOT(refDataChanged()));
      disconnect(this,SIGNAL(statusUpdated(graph::BaseElement&,int)),baseDataPtr.data(),SLOT(elementStatusUpdated(graph::BaseElement&,int)));
      baseDataPtr.clear();
    }
    baseDataPtr = dataPtr;
    if (!baseDataPtr.isNull())
    {
      connect(baseDataPtr.data(),SIGNAL(dataUpdated()),this,SLOT(refDataChanged()));
      connect(this,SIGNAL(statusUpdated(graph::BaseElement&,int)),baseDataPtr.data(),SLOT(elementStatusUpdated(graph::BaseElement&,int)));
    }
    emit statusUpdated(*this,DataRef);
  }

  bool Pin::allowsEdgeTo(const Pin &other) const
  {
    if ((this->direction() == Defines::NonDirectional && other.direction() == Defines::NonDirectional) ||
        (this->direction() == Defines::Incoming && this->connections() == 0 && other.direction() == Defines::Outgoing) ||
        (this->direction() == Defines::Outgoing && other.direction() == Defines::Incoming && other.connections() == 0))
    {
      if (dataRef() != 0 && other.dataRef() != 0)
      {
        return dataRef()->allowConnectionTo(*other.dataRef());
      }
      else
      {
        return true;
      }
    }
    else
    {
      return false;
    }
  }

  BaseItem::Ptr Pin::createSceneItem(BaseItem* parent)
  {
    return PinItem::Ptr(new PinItem(*this,parent));
  }

  //-----------------------------------------------------------------------
  // Class Edge
  //-----------------------------------------------------------------------
  Edge::Edge(Pin::Ref source, Pin::Ref destination, EdgeData::Ptr dataPtr) : GraphElement(),
    pinSrc(source), pinDest(destination)
  {
    setDataRef(dataPtr);
    setElement("edge");
  }

  void Edge::setDataRef(EdgeData::Ptr dataPtr)
  {
    if (baseDataPtr == dataPtr || (!graph().isNull() && graph()->editLockActive())) return;
    if (!baseDataPtr.isNull())
    {
      disconnect(baseDataPtr.data(),SIGNAL(dataUpdated()),this,SLOT(refDataChanged()));
      disconnect(this,SIGNAL(statusUpdated(graph::BaseElement&,int)),baseDataPtr.data(),SLOT(elementStatusUpdated(graph::BaseElement&,int)));
      baseDataPtr.clear();
    }
    baseDataPtr = dataPtr;
    if (!baseDataPtr.isNull())
    {
      connect(baseDataPtr.data(),SIGNAL(dataUpdated()),this,SLOT(refDataChanged()));
      connect(this,SIGNAL(statusUpdated(graph::BaseElement&,int)),baseDataPtr.data(),SLOT(elementStatusUpdated(graph::BaseElement&,int)));
    }
    emit statusUpdated(*this,DataRef);
  }

  BaseItem::Ptr Edge::createSceneItem(BaseItem* parent)
  {
    return graphicsItem = EdgeItem::Ptr(new EdgeItem(*this,parent));
  }

  //-----------------------------------------------------------------------
  // Class Vertex
  //-----------------------------------------------------------------------
  Vertex::Vertex(VertexData::Ptr dataPtr) : GraphElement(), vertexPins(), edgeRefs(), forceOrder(false), mutex(QMutex::Recursive)
  {
    setDataRef(dataPtr);
    setElement("vertex");
  }

  Vertex::~Vertex()
  {
    blockSignals(true);
    setDataRef(VertexData::Ptr());
    blockSignals(false);
    vertexPins.clear();
  }

  // TODO: Update graphics item with new pins
  void Vertex::setDataRef(VertexData::Ptr dataPtr)
  {
    if (dataPtr == baseDataPtr || (!graph().isNull() && graph()->editLockActive()))
    {
      return;
    }
    VertexData::Ptr oldVertexType = dataRef();
    if (!oldVertexType.isNull())
    {
      // disconnect signales from old type
      disconnect(oldVertexType.data(),SIGNAL(dataUpdated()),this,SLOT(refDataChanged()));
      disconnect(this,SIGNAL(statusUpdated(graph::BaseElement&,int)),oldVertexType.data(),SLOT(elementStatusUpdated(graph::BaseElement&,int)));
      disconnect(oldVertexType.data(),SIGNAL(pinDataAdded(PinData::Ptr)),this,SLOT(pinDataAdded(PinData::Ptr)));
      disconnect(oldVertexType.data(),SIGNAL(pinDataRemoved(PinData::Ptr)),this,SLOT(pinDataRemoved(PinData::Ptr)));
      // clear reference
      baseDataPtr.clear();
    }
    baseDataPtr = dataPtr;
    VertexData::Ptr newVertexType = dataRef();
    if (!newVertexType.isNull())
    {
      // connect signals from new type
      connect(newVertexType.data(),SIGNAL(dataUpdated()),this,SLOT(refDataChanged()));
      connect(this,SIGNAL(statusUpdated(graph::BaseElement&,int)),newVertexType.data(),SLOT(elementStatusUpdated(graph::BaseElement&,int)));
      connect(newVertexType.data(),SIGNAL(pinDataAdded(PinData::Ptr)),this,SLOT(pinDataAdded(PinData::Ptr)));
      connect(newVertexType.data(),SIGNAL(pinDataRemoved(PinData::Ptr)),this,SLOT(pinDataRemoved(PinData::Ptr)));
      // reassign compatible pins and delete incompatible pins
      const VertexData::PinDataMap& typeMap = dataPtr->pinData();
      QSet<QString> assignedTypes;
      PinMap::iterator it_pin = vertexPins.begin();
      while(it_pin != vertexPins.end())
      {
        PinData::Ptr oldType = it_pin.value()->dataRef();
        if (typeMap.contains(oldType->id()) && oldType->isCompatibleTo(typeMap[oldType->id()]))
        {
          it_pin.value()->setDataRef(typeMap[oldType->id()]);
          assignedTypes.insert(it_pin.value()->dataRef()->id());
          ++it_pin;
        }
        else
        {
          it_pin = vertexPins.erase(it_pin);
        }
      }
      // create new pins for new not yet assigned pin types
      for(VertexData::PinDataMap::const_iterator it_type = typeMap.begin(); it_type != typeMap.end(); ++it_type)
      {
        if (!assignedTypes.contains(it_type.key()))
        {
          vertexPins.insert(it_type.value()->direction(),Pin::Ptr(new Pin(*this,it_type.value())));
        }
      }
    }
    else
    {
      // clear all pins in case the new type reference is null
      vertexPins.clear();
    }
    emit statusUpdated(*this,DataRef);
  }

  BaseItem::Ptr Vertex::createSceneItem(BaseItem* parent)
  {
    return VertexItem::Ptr(new VertexItem(*this,parent));
  }

  int Vertex::topologicalOrder() const
  {
    DirectedGraph* graph = dynamic_cast<DirectedGraph*>(this->graph().data());
    if (graph != 0)
    {
      return graph->vertexOrder(id());
    }
    else
    {
      return -1;
    }
  }

  bool Vertex::isInCycle() const
  {
    DirectedGraph* graph = dynamic_cast<DirectedGraph*>(this->graph().data());
    if (graph != 0)
    {
      return graph->vertexInCycle(id());
    }
    else
    {
      return false;
    }
  }

  void Vertex::save(QXmlStreamWriter& stream) const
  {
    writeElementStart(stream); // vertex
    writeProperties(stream);

    stream.writeStartElement("pins");
    for(PinMap::const_iterator it = vertexPins.begin(); it != vertexPins.end(); ++it)
    {
      it.value()->save(stream);
    }
    stream.writeEndElement(); // pins

    writeElementEnd(stream); // vertex
  }

  bool Vertex::load(QXmlStreamReader &stream)
  {
    if (!readElementStart(stream)) return false;
    if (!readProperties(stream)) return false;

    if (stream.readNextStartElement() && stream.name() == "pins")
    {
      for(PinMap::const_iterator it = vertexPins.begin(); it != vertexPins.end(); ++it)
      {
        if (stream.readNextStartElement())
        {
          if (!it.value()->load(stream)) return false;
        }
        else
        {
          stream.raiseError(QString(QObject::tr("At line %1, column %2: Failed to read pin element of class instance '%3' (element '%4').")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(QString(metaObject()->className())).arg(element()));
          return false;
        }
      }
      stream.skipCurrentElement();
    }
    else
    {
      stream.raiseError(QString(QObject::tr("At line %1, column %2: Failed to read pin collection of class instance '%3' (element '%4').")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(QString(metaObject()->className())).arg(element()));
      return false;
    }
    if (!readElementEnd(stream)) return false;
    return true;
  }

  void Vertex::pinDataAdded(PinData::Ptr pinType)
  {
    if (!pinType.isNull())
    {
      vertexPins.insert(pinType->direction(),Pin::Ptr(new Pin(*this,pinType)));
    }
  }

  void Vertex::pinDataRemoved(PinData::Ptr pinType)
  {
    if(!pinType.isNull())
    {
      const PinList pinList = pins(pinType->direction());
      foreach(Pin::Ptr pin, pinList)
      {
        if (pin->dataRef() == pinType)
        {
          vertexPins.remove(pinType->direction(),pin);
          emit statusUpdated(*this,DataRef);
          break;
        }
      }
    }
  }

  void Vertex::addEdgeReference(Edge::Ref edgeRef, Defines::PinDirectionType direction)
  {
    Q_ASSERT_X(!edgeRefs.contains(direction,edgeRef),"graph::Vertex::addEdgeReference","Given edge is already in reference list.");
    if (!edgeRefs.contains(direction,edgeRef))
    {
      edgeRefs.insert(direction,edgeRef);
    }
  }

  void Vertex::removeEdgeReference(Edge::Ref edgeRef, Defines::PinDirectionType direction)
  {
    Q_ASSERT_X(edgeRefs.contains(direction,edgeRef),"graph::Vertex::removeEdgeReference","Given edge is not in reference list.");
    if (edgeRefs.contains(direction,edgeRef))
    {
      edgeRefs.remove(direction,edgeRef);
    }
  }

}
