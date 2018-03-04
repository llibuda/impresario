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

#include "grapheditor.h"
#include "graphmain.h"
#include "graphdefines.h"
#include "graphresources.h"
#include <QApplication>
#include <QClipboard>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QMimeData>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

namespace graph
{
  //-----------------------------------------------------------------------
  // Class Scene
  //-----------------------------------------------------------------------
  Scene::Scene(QObject *parent) : QGraphicsScene(parent), Serializer("visualization",Scene::staticMetaObject.propertyOffset(),this),
    edgeRouter(Avoid::OrthogonalRouting), dropPosition(), edgeCreationMode(false),
    edgeValid(false), edgeStartItem(0), edgeEndItem(0), edgeItem(0), layoutDir(Defines::TopToBottom), scenePalette(), blocked(false)
  {
    edgeRouter.setRoutingParameter(Avoid::idealNudgingDistance,10.0);
    edgeRouter.setRoutingParameter(Avoid::segmentPenalty,1.0);
    //edgeRouter.setRoutingParameter(Avoid::fixedSharedPathPenalty,5.0);
    //edgeRouter.setRoutingOption(Avoid::nudgeOrthogonalSegmentsConnectedToShapes,true);
    //edgeRouter.setRoutingOption(Avoid::penaliseOrthogonalSharedPathsAtConnEnds,true);
  }

  Scene::~Scene()
  {
    QList<QGraphicsItem*> graphItems = items();
    foreach(QGraphicsItem* item, graphItems)
    {
      if (item->scene() != 0) removeItem(item);
    }
  }

  const Palette& Scene::palette() const
  {
    if (!scenePalette.isNull())
    {
      return *scenePalette;
    }
    return *Palette::graphDefault();
  }

  void Scene::onGraphChanged(int reason)
  {
    switch(reason)
    {
      case GraphBase::EditingLocked:
      case GraphBase::EditingUnlocked:
      {
        blocked = (reason == GraphBase::EditingLocked);
        QList<QGraphicsItem*> graphItems = items();
        foreach(QGraphicsItem* item, graphItems)
        {
          BaseItem* bItem = qgraphicsitem_cast<BaseItem*>(item);
          if (bItem != 0)
          {
            bItem->setBlocked(blocked);
          }
        }
        break;
      }
    }
  }

  void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
  {
    if (event->mimeData()->hasFormat("text/graph-vertex-type-signature"))
    {
      event->accept();
    }
    else
    {
      event->ignore();
    }
  }

  void Scene::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
  {
    event->accept();
  }

  void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
  {
    if (event->mimeData()->hasFormat("text/graph-vertex-type-signature"))
    {
      event->setDropAction(Qt::LinkAction);
      event->accept();
    }
    else
    {
      event->ignore();
    }
  }

  void Scene::dropEvent(QGraphicsSceneDragDropEvent* event)
  {
    if (event->mimeData()->hasFormat("text/graph-vertex-type-signature"))
    {
      Qt::MouseButtons mouseButtons = event->buttons();
      // Bug on Linux: mouseButtons seems to be Qt::NoButton during drag and drop
      // So we put in the condition mouseButtons == Qt::NoButton as well to have
      // at least some result. Hopefully this will be fixed in Qt
      if (mouseButtons == Qt::LeftButton || mouseButtons == Qt::NoButton)
      {
        QByteArray signatureData = event->mimeData()->data("text/graph-vertex-type-signature");
        QString signatureString(signatureData);
        emit leftClickDrop(*this,event->scenePos(),signatureString);
        event->setDropAction(Qt::LinkAction);
        event->accept();
      }
      else if (mouseButtons == Qt::RightButton)
      {
        QGraphicsPixmapItem* pixItem = new QGraphicsPixmapItem(QPixmap::fromImage(qvariant_cast<QImage>(event->mimeData()->imageData())));
        QSizeF pixSize = pixItem->boundingRect().size();
        QPointF pixPos = event->scenePos() - QPointF(pixSize.width() / 2.0,pixSize.height() / 2.0);
        if (pixPos.x() < 0.0)
        {
          pixPos.setX(0.0);
        }
        if (pixPos.y() < 0.0)
        {
          pixPos.setY(0.0);
        }
        pixItem->setPos(pixPos);
        addItem(pixItem);
        dropPosition = event->scenePos();
        QByteArray signatureData = event->mimeData()->data("text/graph-vertex-type-signature");
        QString signatureString(signatureData);
        emit rightClickDrop(*this,event->scenePos(),signatureString);
        removeItem(pixItem);
        delete pixItem;
        event->setDropAction(Qt::LinkAction);
        event->accept();
      }
      else
      {
        event->ignore();
      }
    }
    else
    {
      event->ignore();
    }
  }

  void Scene::mousePressEvent(QGraphicsSceneMouseEvent *event)
  {
    if (event->button() == Qt::LeftButton)
    {
      QTransform transform;
      QGraphicsItem* item = itemAt(event->scenePos(),transform);
      if (item != 0 && !blocked)
      {
        edgeStartItem = dynamic_cast<PinItem*>(item);
        if (edgeStartItem && edgeStartItem->connectionAllowed())
        {
          edgeItem = new EdgeItem(edgeStartItem,event->scenePos());
          addItem(edgeItem);
          edgeCreationMode = true;
        }
      }
    }
    if (!edgeCreationMode)
    {
      if (event->button() == Qt::RightButton)
      {
        event->accept();
      }
      else
      {
        QGraphicsScene::mousePressEvent(event);
      }
    }
    else
    {
      event->accept();
    }
  }

  void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
  {
    if (edgeCreationMode)
    {
      edgeValid = false;
      QPointF endPos = event->scenePos();
      QTransform transform;
      QGraphicsItem* item = itemAt(event->scenePos(),transform);
      if (item != 0)
      {
        edgeEndItem = dynamic_cast<PinItem*>(item);
        if (edgeEndItem)
        {
          endPos = edgeEndItem->edgeAnchor();
          edgeValid = (edgeStartItem != edgeEndItem &&
                       edgeStartItem->pin().allowsEdgeTo(edgeEndItem->pin()));
        }
      }
      edgeItem->setEndPoint(endPos);
      if (edgeValid)
      {
        edgeItem->setState(Defines::Valid);
        SceneEditor* editor = static_cast<SceneEditor*>(views().first());
        editor->viewport()->setCursor(editor->cursorSet().cursor(Cursors::CursorConnectValid));
      }
      else
      {
        edgeItem->setState(Defines::Invalid);
        SceneEditor* editor = static_cast<SceneEditor*>(views().first());
        editor->viewport()->setCursor(editor->cursorSet().cursor(Cursors::CursorConnectInvalid));
      }
      event->accept();
    }
    else
    {
      QGraphicsScene::mouseMoveEvent(event);
    }
  }

  void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
  {
    if (edgeCreationMode)
    {
      if (edgeItem)
      {
        removeItem(edgeItem);
        delete edgeItem;
        edgeItem = 0;
      }
      if (edgeValid && edgeStartItem && edgeEndItem)
      {
        PinItem* src = edgeStartItem;
        PinItem* dst = edgeEndItem;
        if (edgeEndItem->pin().direction() == Defines::Outgoing)
        {
          src = edgeEndItem;
          dst = edgeStartItem;
        }
        emit requestEdge(*this,*src,*dst);
      }
      edgeCreationMode = false;
      edgeValid = false;
      edgeStartItem = 0;
      edgeEndItem = 0;
      views().first()->viewport()->unsetCursor();
      event->accept();
    }
    else
    {
      QGraphicsScene::mouseReleaseEvent(event);
    }
  }

  //-----------------------------------------------------------------------
  // Class SceneEditor
  //-----------------------------------------------------------------------
  int SceneEditor::SetSceneEvent = QEvent::registerEventType(QEvent::User + 1);

  SceneEditor::SceneEditor(GraphBase& graph, ElementManager& manager, QWidget *parent) : QGraphicsView(parent),
    elementManager(manager), graphBase(graph), graphElements(),
    zoomFactor(1.0), zoomMin(0.2), zoomMax(3.0), zoomAreaSelected(QRectF()), zoomToSelectionMode(false),
    posVertexOffH(15.0), posVertexOffV(15.0), posVertexOvMax(15), posVertexOvCnt(0), cursors(Cursors::graphDefault())
  {
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    setRubberBandSelectionMode(Qt::ContainsItemShape);
    QApplication::postEvent(this,new QEvent((QEvent::Type)SetSceneEvent));
  }

  SceneEditor::~SceneEditor()
  {
    clearElements();
  }

  void SceneEditor::setScene(QGraphicsScene* scene)
  {
    Scene* oldScene = qobject_cast<Scene*>(this->scene());
    if (oldScene != 0)
    {
      disconnect(oldScene,SIGNAL(leftClickDrop(Scene&,QPointF,QString)),this,SLOT(baseTypeLeftClickDrop(Scene&,QPointF,QString)));
      disconnect(oldScene,SIGNAL(rightClickDrop(Scene&,QPointF,QString)),this,SLOT(baseTypeRightClickDrop(Scene&,QPointF,QString)));
      disconnect(oldScene,SIGNAL(itemMoved(Scene&,BaseItem&,QPointF)),this,SLOT(baseItemMoved(Scene&,BaseItem&,QPointF)));
      disconnect(oldScene,SIGNAL(itemResized(Scene&,BaseItem&,QRectF)),this,SLOT(baseItemResized(Scene&,BaseItem&,QRectF)));
      disconnect(oldScene,SIGNAL(requestEdge(Scene&,PinItem&,PinItem&)),this,SLOT(edgeItemToBeCreated(Scene&,PinItem&,PinItem&)));
      disconnect(&graphBase,SIGNAL(statusUpdated(int)),oldScene,SLOT(onGraphChanged(int)));
    }
    QGraphicsView::setScene(scene);
    Scene* newScene = qobject_cast<Scene*>(this->scene());
    if (newScene != 0)
    {
      connect(newScene,SIGNAL(leftClickDrop(Scene&,QPointF,QString)),this,SLOT(baseTypeLeftClickDrop(Scene&,QPointF,QString)));
      connect(newScene,SIGNAL(rightClickDrop(Scene&,QPointF,QString)),this,SLOT(baseTypeRightClickDrop(Scene&,QPointF,QString)));
      connect(newScene,SIGNAL(itemMoved(Scene&,BaseItem&,QPointF)),this,SLOT(baseItemMoved(Scene&,BaseItem&,QPointF)));
      connect(newScene,SIGNAL(itemResized(Scene&,BaseItem&,QRectF)),this,SLOT(baseItemResized(Scene&,BaseItem&,QRectF)));
      connect(newScene,SIGNAL(requestEdge(Scene&,PinItem&,PinItem&)),this,SLOT(edgeItemToBeCreated(Scene&,PinItem&,PinItem&)));
      connect(&graphBase,SIGNAL(statusUpdated(int)),newScene,SLOT(onGraphChanged(int)));
    }
  }

  void SceneEditor::setZoomFactorMin(qreal min)
  {
    if (min > zoomMax || min == zoomMin) return;
    zoomMin = min;
    if (zoomMin > zoomFactor)
    {
      zoomFactor = zoomMin;
      int zf = (int)zoomFactor * 100;
      viewZoomChange(zf);
      emit zoomFactorChanged(zf);
    }
  }

  void SceneEditor::setZoomFactorMax(qreal max)
  {
    if (max < zoomMin || max == zoomMax) return;
    zoomMax = max;
    if (zoomMax < zoomFactor)
    {
      zoomFactor = zoomMax;
      int zf = (int)zoomFactor * 100;
      viewZoomChange(zf);
      emit zoomFactorChanged(zf);
    }
  }

  void SceneEditor::setPositioningBehavior(int maxOverlappingCnt, qreal offsetItemHorz, qreal offsetItemVert)
  {
    posVertexOvMax = maxOverlappingCnt;
    posVertexOffH = offsetItemHorz;
    posVertexOffV = offsetItemVert;
  }

  void SceneEditor::addElementItemsToGraph(const BaseItemList& elementItems, bool reverseOrder)
  {
    if (graphBase.editLockActive())
    {
      return;
    }
    if (!reverseOrder)
    {
      for(BaseItemList::const_iterator it = elementItems.begin(); it != elementItems.end(); ++it)
      {
        switch((*it)->type())
        {
          case BaseItem::VertexItemType:
          {
            VertexItem::Ptr vertexItem = (*it).staticCast<VertexItem>();
            graphBase.addVertex(graphElements[vertexItem->vertex().id()].toStrongRef().staticCast<Vertex>());
            break;
          }
          case BaseItem::EdgeItemType:
          {
            EdgeItem::Ptr edgeItem = (*it).staticCast<EdgeItem>();
            graphBase.addEdge(graphElements[edgeItem->edge().id()].toStrongRef().staticCast<Edge>());
            break;
          }
          default:
            break;
        }
      }
    }
    else
    {
      for(int i = elementItems.count() - 1; i != -1; --i)
      {
        BaseItem::Ptr item = elementItems[i];
        switch(item->type())
        {
          case BaseItem::VertexItemType:
          {
            VertexItem::Ptr vertexItem = item.staticCast<VertexItem>();
            graphBase.addVertex(graphElements[vertexItem->vertex().id()].toStrongRef().staticCast<Vertex>());
            break;
          }
          case BaseItem::EdgeItemType:
          {
            EdgeItem::Ptr edgeItem = item.staticCast<EdgeItem>();
            graphBase.addEdge(graphElements[edgeItem->edge().id()].toStrongRef().staticCast<Edge>());
            break;
          }
          default:
            break;
        }
      }
    }
  }

  void SceneEditor::removeElementItemsFromGraph(const BaseItemList& elementItems, bool reverseOrder)
  {
    if (graphBase.editLockActive())
    {
      return;
    }
    if (!reverseOrder)
    {
      for(BaseItemList::const_iterator it = elementItems.begin(); it != elementItems.end(); ++it)
      {
        switch((*it)->type())
        {
          case BaseItem::VertexItemType:
          {
            VertexItem::Ptr vertexItem = (*it).staticCast<VertexItem>();
            graphBase.removeVertex(graphElements[vertexItem->vertex().id()].toStrongRef().staticCast<Vertex>());
            break;
          }
          case BaseItem::EdgeItemType:
          {
            EdgeItem::Ptr edgeItem = (*it).staticCast<EdgeItem>();
            graphBase.removeEdge(graphElements[edgeItem->edge().id()].toStrongRef().staticCast<Edge>());
            break;
          }
          default:
            break;
        }
      }
    }
    else
    {
      for(int i = elementItems.count() - 1; i != -1; --i)
      {
        BaseItem::Ptr item = elementItems[i];
        switch(item->type())
        {
          case BaseItem::VertexItemType:
          {
            VertexItem::Ptr vertexItem = item.staticCast<VertexItem>();
            graphBase.removeVertex(graphElements[vertexItem->vertex().id()].toStrongRef().staticCast<Vertex>());
            break;
          }
          case BaseItem::EdgeItemType:
          {
            EdgeItem::Ptr edgeItem = item.staticCast<EdgeItem>();
            graphBase.removeEdge(graphElements[edgeItem->edge().id()].toStrongRef().staticCast<Edge>());
            break;
          }
          default:
            break;
        }
      }
    }
  }

  void SceneEditor::copyElementItemsToClipboard(const BaseItemList& elementItems) const
  {
    if (elementItems.isEmpty()) return;
    QByteArray xmlString;
    QXmlStreamWriter stream(&xmlString);
    stream.writeStartDocument();
    stream.writeStartElement("SceneEditorClipboard");
    for(BaseItemList::const_iterator it = elementItems.begin(); it != elementItems.end(); ++it)
    {
      (*it)->baseElement().save(stream);
    }
    stream.writeEndElement();
    stream.writeEndDocument();
    QClipboard* clipboard = QApplication::clipboard();
    QMimeData* clpData = new QMimeData();
    clpData->setData("SceneEditor/xml",xmlString);
    clipboard->setMimeData(clpData);
  }

  void SceneEditor::editCopy()
  {
    BaseItemList list = collectElementItemsForClipboard();
    copyElementItemsToClipboard(list);
  }

  void SceneEditor::editSelectAll()
  {
    QPainterPath path;
    path.addRect(scene()->sceneRect());
    scene()->setSelectionArea(path);
  }

  void SceneEditor::viewZoomIn()
  {
    if (zoomFactor < zoomMax)
    {
      QTransform trans = transform();
      qreal scaleX = 1.0 / trans.m11();
      qreal scaleY = 1.0 / trans.m22();
      zoomFactor += 0.1;
      if (zoomFactor > zoomMax)
      {
        zoomFactor = zoomMax;
      }
      else if (zoomFactor < zoomMin)
      {
        zoomFactor = zoomMin;
      }
      scaleX *= zoomFactor;
      scaleY *= zoomFactor;
      scale(scaleX,scaleY);
      emit zoomFactorChanged((int)(zoomFactor * 100));
    }
    else
    {
      emit updateZoomInCommand(false);
      emit updateZoomOutCommand(true);
    }
  }

  void SceneEditor::viewZoomOut()
  {
    if (zoomFactor > zoomMin)
    {
      QTransform trans = transform();
      qreal scaleX = 1.0 / trans.m11();
      qreal scaleY = 1.0 / trans.m22();
      zoomFactor -= 0.1;
      if (zoomFactor < zoomMin)
      {
        zoomFactor = zoomMin;
      }
      else if (zoomFactor > zoomMax)
      {
        zoomFactor = zoomMax;
      }
      scaleX *= zoomFactor;
      scaleY *= zoomFactor;
      scale(scaleX,scaleY);
      emit zoomFactorChanged((int)(zoomFactor * 100));
    }
    else
    {
      emit updateZoomInCommand(true);
      emit updateZoomOutCommand(false);
    }
  }

  void SceneEditor::viewZoomSelection(bool checked)
  {
    if (checked)
    {
      viewport()->setCursor(cursors->cursor(Cursors::CursorZoom));
      zoomToSelectionMode = true;
    }
    else
    {
      viewport()->unsetCursor();
      zoomToSelectionMode = false;
    }
  }

  void SceneEditor::viewZoom100()
  {
    QTransform trans = transform();
    qreal scaleX = 1.0 / trans.m11();
    qreal scaleY = 1.0 / trans.m22();
    scale(scaleX,scaleY);
    zoomFactor = 1.0;
    emit zoomFactorChanged(100);
    emit updateZoomInCommand(true);
    emit updateZoomOutCommand(true);
  }

  void SceneEditor::viewZoomPage()
  {
    fitInView(scene()->itemsBoundingRect(),Qt::KeepAspectRatio);
    zoomFactor = transform().m11();
    emit zoomFactorChanged((int)(zoomFactor * 100));
    emit updateZoomInCommand(zoomFactor < zoomMax);
    emit updateZoomOutCommand(zoomFactor > zoomMin);
  }

  void SceneEditor::viewZoomChange(int factor)
  {
    zoomFactor = factor / 100.0;
    QTransform trans = transform();
    qreal scaleX = (1.0 / trans.m11()) * zoomFactor;
    qreal scaleY = (1.0 / trans.m22()) * zoomFactor;
    scale(scaleX,scaleY);
    emit updateZoomInCommand(zoomFactor < zoomMax);
    emit updateZoomOutCommand(zoomFactor > zoomMin);
  }

  void SceneEditor::updateSceneRect(const QRectF &rect)
  {
    QRectF newRect = rect | QRectF(0.0,0.0,1.0,1.0);
    setSceneRect(newRect);
  }

  void SceneEditor::save(QXmlStreamWriter &stream) const
  {
    graphBase.save(stream);
  }

  bool SceneEditor::load(QXmlStreamReader &stream)
  {
    connect(&graphBase,SIGNAL(statusUpdated(int)),this,SLOT(onGraphModified(int)));

    // load graph
    if (!graphBase.load(stream,elementManager)) return false;
    // update references
    foreach(Vertex::Ptr vertex,graphBase.vertexList())
    {
      graphElements.insert(vertex->id(),vertex.toWeakRef());
      //vertex->sceneItem().dynamicCast<VertexItem>()->setItemLayout(graphBase.scene()->graphLayout());
    }
    foreach(Edge::Ptr edge,graphBase.edgeList())
    {
      graphElements.insert(edge->id(),edge.toWeakRef());
    }
    return true;
  }

  void SceneEditor::onGraphModified(int reason)
  {
    switch(reason)
    {
      case graph::GraphBase::EditingLocked:
      {
        this->setAcceptDrops(false);
        break;
      }
      case graph::GraphBase::EditingUnlocked:
      {
        this->setAcceptDrops(true);
        break;
      }
    }
  }

  SceneEditor::BaseItemList SceneEditor::createVertexItem(int countInstances, const QStringList& signatures, QPointF startPos)
  {
    Q_ASSERT(signatures.count() > 0);
    BaseItemList list;
    if (graphBase.editLockActive())
    {
      return list;
    }
    Vertex::Ptr vertex = Vertex::Ptr();
    QPointF insertPos = startPos;
    int posItemOvCnt = 0;
    foreach(QString signature, signatures)
    {
      for(int i = 0; i < countInstances; ++i)
      {
        // create vertex
        vertex = elementManager.createVertexInstance(signature);
        Q_ASSERT_X(!vertex.isNull(),"SceneView::createVertexItems","Failed to create vertex instance for given signature.");
        if (!vertex.isNull())
        {
          // set position of corresponding graphics item
          VertexItem::Ptr vertexItem = vertex->sceneItem().staticCast<VertexItem>();
          if (insertPos == QPointF(0.0,0.0))
          {
            QSizeF itemSize = vertexItem->boundingRect().size();
            QPointF pos;
            pos.setX(posVertexOvCnt * posVertexOffH + itemSize.width() / 2.0);
            pos.setY(posVertexOvCnt * posVertexOffV + itemSize.height() / 2.0);
            if (++posVertexOvCnt >= posVertexOvMax)
            {
              posVertexOvCnt = 0;
            }
            vertexItem->setPos(pos);
          }
          else
          {
            QSizeF itemSize = vertexItem->boundingRect().size();
            QPointF pos;
            pos.setX(posItemOvCnt * posVertexOffH + insertPos.x());
            if (pos.x() - itemSize.width() / 2.0 < 0.0)
            {
              pos.setX(itemSize.width() / 2.0);
            }
            pos.setY(posItemOvCnt * posVertexOffV + insertPos.y());
            if (pos.y() - itemSize.height() / 2.0 < 0.0)
            {
              pos.setY(itemSize.height() / 2.0);
            }
            if (++posItemOvCnt >= posVertexOvMax)
            {
              posItemOvCnt = 0;
            }
            vertexItem->setPos(pos);
          }
          list.append(vertexItem);
          graphElements.insert(vertex->id(),vertex.toWeakRef());
        }
      }
    }
    return list;
  }

  SceneEditor::BaseItemList SceneEditor::createEdgeItem(Pin::Ref srcPin,Pin::Ref destPin,const QString& edgeSignature)
  {
    BaseItemList list;
    if (graphBase.editLockActive())
    {
      return list;
    }
    Edge::Ptr edge = elementManager.createEdgeInstance(srcPin,destPin,edgeSignature);
    Q_ASSERT_X(!edge.isNull(),"SceneView::createEdgeItems","Failed to create edge instance for given signature.");
    if (!edge.isNull())
    {
      EdgeItem::Ptr edgeItem = edge->sceneItem().staticCast<EdgeItem>();
      if (!edgeItem.isNull())
      {
        graphElements.insert(edge->id(),edge.toWeakRef());
        list.append(edgeItem);
      }
      else
      {
        elementManager.deleteEdgeInstance(edge);
      }
    }
    return list;
  }

  SceneEditor::BaseItemList SceneEditor::collectElementItemsForDeletion()
  {
    BaseItemList list;
    if (graphBase.editLockActive())
    {
      return list;
    }
    QSet<BaseItem::Ptr> edges;
    foreach(QGraphicsItem* item, scene()->selectedItems())
    {
      switch(item->type())
      {
      case BaseItem::VertexItemType: // Vertex
        {
          Vertex& vertex = static_cast<VertexItem*>(item)->vertex();
          list.append(vertex.sceneItem());
          const Vertex::EdgeRefMap& edgeRefs = vertex.edges();
          for(Vertex::EdgeRefMap::const_iterator it = edgeRefs.begin(); it != edgeRefs.end(); ++it)
          {
            edges.insert(it.value().toStrongRef()->sceneItem());
          }
          break;
        }
      case BaseItem::EdgeItemType: // Edge
        {
          Edge& edge = static_cast<EdgeItem*>(item)->edge();
          edges.insert(edge.sceneItem());
          break;
        }
      default:
        break;
      }
    }
    list.append(edges.toList());
    return list;
  }

  SceneEditor::BaseItemList SceneEditor::collectElementItemsForClipboard()
  {
    BaseItemList list;
    QList<BaseItem::Ptr> edges;
    foreach(QGraphicsItem* item, scene()->selectedItems())
    {
      switch(item->type())
      {
      case BaseItem::VertexItemType: // Vertex
        {
          Vertex& vertex = static_cast<VertexItem*>(item)->vertex();
          list.append(vertex.sceneItem());
          break;
        }
      case BaseItem::EdgeItemType: // Edge
        {
          Edge& edge = static_cast<EdgeItem*>(item)->edge();
          if (!edge.srcPin().isNull() && ! edge.destPin().isNull() &&
              edge.srcPin().toStrongRef()->vertex().sceneItem()->isSelected() &&
              edge.destPin().toStrongRef()->vertex().sceneItem()->isSelected())
          {
            edges.append(edge.sceneItem());
          }
          else
          {
            item->setSelected(false);
          }
          break;
        }
      default:
        break;
      }
    }
    list.append(edges);
    return list;
  }

  SceneEditor::BaseItemList SceneEditor::pasteElementItemsFromClipboard()
  {
    BaseItemList list;
    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* clpData = clipboard->mimeData();
    if (!clpData->hasFormat("SceneEditor/xml")) return list;

    QXmlStreamReader stream(clpData->data("SceneEditor/xml"));
    if (!stream.readNextStartElement() || stream.name() != "SceneEditorClipboard") return list;

    // read vertices and edges copied to clipboard
    QList<Vertex::Ptr>   vertexList;
    QList<Edge::Ptr>     edgeList;
    QMap<QUuid,Pin::Ptr> pinMap;
    while(stream.readNextStartElement())
    {
      if (stream.name() == "vertex")
      {
        QString signature = stream.attributes().value("dataTypeSignature").toString();
        if (!signature.isEmpty())
        {
          Vertex::Ptr vertexPtr = elementManager.createVertexInstance(signature);
          if (!vertexPtr.isNull())
          {
            if (vertexPtr->load(stream))
            {
              vertexList.append(vertexPtr);
              // we need to keep track of all pins created for the edges to load
              const Vertex::PinMap& pins = vertexPtr->pins();
              for(Vertex::PinMap::const_iterator it = pins.begin(); it != pins.end(); ++it)
              {
                pinMap.insert(it.value()->id(),it.value());
              }
            }
            else
            {
              elementManager.deleteVertexInstance(vertexPtr);
            }
          }
          else
          {
            stream.skipCurrentElement();
          }
        }
      }
      else if (stream.name() == "edge")
      {
        QString signature = stream.attributes().value("dataTypeSignature").toString();
        if (!signature.isEmpty())
        {
          QUuid pinSrcId = QUuid(stream.attributes().value("srcPinId").toString());
          QUuid pinDstId = QUuid(stream.attributes().value("destPinId").toString());
          if (pinMap.contains(pinSrcId) && pinMap.contains(pinDstId))
          {
            Edge::Ptr edgePtr = elementManager.createEdgeInstance(pinMap[pinSrcId].toWeakRef(),pinMap[pinDstId].toWeakRef(),signature);
            if (!edgePtr.isNull())
            {
              if (edgePtr->load(stream))
                edgeList.append(edgePtr);
              else
                elementManager.deleteEdgeInstance(edgePtr);
            }
            else
            {
              stream.skipCurrentElement();
            }
          }
          else
          {
            stream.skipCurrentElement();
          }
        }
      }
    }

    // change element ids and add items to graph
    for(QMap<QUuid,Pin::Ptr>::iterator it = pinMap.begin(); it != pinMap.end(); ++it)
    {
      it.value()->setId(QUuid::createUuid());
    }
    foreach(Vertex::Ptr vertex, vertexList)
    {
      vertex->setId(QUuid::createUuid());
      list.append(vertex->sceneItem());
      graphElements.insert(vertex->id(),vertex.toWeakRef());
    }
    foreach(Edge::Ptr edge, edgeList)
    {
      edge->setId(QUuid::createUuid());
      list.append(edge->sceneItem());
      graphElements.insert(edge->id(),edge.toWeakRef());
    }

    return list;
  }

  void SceneEditor::clearElements()
  {
    ElementMap::iterator it = graphElements.begin();
    while(it != graphElements.end())
    {
      GraphElement::Ptr elementPtr = it.value().toStrongRef();
      BaseItem::Ptr item = elementPtr->sceneItem();
      if (item->scene() == 0)
      {
        switch(item->type())
        {
          case BaseItem::EdgeItemType:
          {
            Edge::Ptr edge = elementPtr.staticCast<Edge>();
            elementManager.deleteEdgeInstance(edge);
            break;
          }
          case BaseItem::VertexItemType:
          {
            Vertex::Ptr vertex = elementPtr.staticCast<Vertex>();
            elementManager.deleteVertexInstance(vertex);
            break;
          }
          default:
            Q_ASSERT_X(false,"SceneView::clear()","Unknown graphics item type found. Don't know how to handle.");
            break;
        }
        it = graphElements.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  void SceneEditor::initialize()
  {
  }

  void SceneEditor::onVertexTypeLeftClickDrop(const QString& typeSignature, QPointF pos)
  {
    BaseItemList list = createVertexItem(1,QStringList(typeSignature),pos);
    addElementItemsToGraph(list);
  }

  void SceneEditor::onVertexTypeRightClickDrop(const QString& typeSignature, QPointF pos)
  {
    BaseItemList list = createVertexItem(1,QStringList(typeSignature),pos);
    addElementItemsToGraph(list);
  }

  void SceneEditor::onEdgeToBeCreated(Pin::Ref /*srcPin*/, Pin::Ref /*destPin*/)
  {
  }

  void SceneEditor::onItemMoved(BaseItemList /*elementItems*/, QPointF /*offset*/)
  {
  }

  void SceneEditor::onItemResized(BaseItem::Ptr /*element*/, QRectF /*initialSize*/)
  {
  }

  bool SceneEditor::event(QEvent *event)
  {
    if (event->type() == SetSceneEvent)
    {
      setScene(graphBase.scene().data());
      scene()->installEventFilter(this);
      connect(&graphBase,SIGNAL(statusUpdated(int)),this,SLOT(onGraphModified(int)));
      updateSceneRect(scene()->sceneRect());
      initialize();
      return true;
    }
    else
    {
      return QGraphicsView::event(event);
    }
  }

  void SceneEditor::wheelEvent(QWheelEvent* event)
  {
    if (event->modifiers() & Qt::ControlModifier)
    {
      int numDegrees = event->delta() / 8;
      int numSteps = numDegrees / 15;
      if (numSteps > 0)
      {
        viewZoomIn();
      }
      else
      {
        viewZoomOut();
      }
      event->accept();
    }
    else
    {
      QGraphicsView::wheelEvent(event);
    }
  }

  void SceneEditor::mousePressEvent(QMouseEvent *event)
  {
    if (zoomToSelectionMode)
    {
      zoomAreaSelected = scene()->selectionArea().boundingRect();
    }
    QGraphicsView::mousePressEvent(event);
  }

  void SceneEditor::mouseReleaseEvent(QMouseEvent *event)
  {
    QGraphicsView::mouseReleaseEvent(event);
    if (zoomToSelectionMode)
    {
      QRectF zoomArea = scene()->selectionArea().boundingRect();
      if (zoomArea != zoomAreaSelected)
      {
        fitInView(zoomArea,Qt::KeepAspectRatio);
        zoomFactor = transform().m11();
        emit zoomFactorChanged((int)(zoomFactor * 100));
        emit updateZoomInCommand(zoomFactor < zoomMax);
        emit updateZoomOutCommand(zoomFactor > zoomMin);

      }
      zoomToSelectionMode = false;
      viewport()->unsetCursor();
    }
    emit updateZoomSelCommand(zoomToSelectionMode);
  }

  bool SceneEditor::eventFilter(QObject* watched, QEvent* event)
  {
    if (watched == scene() && zoomToSelectionMode && event->type() == QEvent::GraphicsSceneMousePress)
    {
      return true;
    }
    return false;
  }

  void SceneEditor::baseTypeLeftClickDrop(Scene& /*scene*/, QPointF pos, const QString& typeSignature)
  {
    onVertexTypeLeftClickDrop(typeSignature,pos);
  }

  void SceneEditor::baseTypeRightClickDrop(Scene& /*scene*/, QPointF pos, const QString& typeSignature)
  {
    onVertexTypeRightClickDrop(typeSignature,pos);
  }

  void SceneEditor::baseItemMoved(Scene& scene, BaseItem& /*item*/, QPointF offset)
  {
    BaseItemList list;
    QList<QGraphicsItem*> itemList = scene.selectedItems();
    for(QList<QGraphicsItem*>::iterator it = itemList.begin(); it != itemList.end(); ++it)
    {
      VertexItem* vertexItem = dynamic_cast<VertexItem*>(*it);
      if (vertexItem != 0)
      {
        list.append(vertexItem->vertex().sceneItem());
      }
    }
    onItemMoved(list,offset);
  }

  void SceneEditor::baseItemResized(Scene& /*scene*/, BaseItem& item, QRectF initialSize)
  {
    VertexItem* vertexItem = static_cast<VertexItem*>(&item);
    if (vertexItem)
    {
      onItemResized(vertexItem->vertex().sceneItem(),initialSize);
    }
  }

  void SceneEditor::edgeItemToBeCreated(Scene& /*scene*/, PinItem& source, PinItem& dest)
  {
    // find correct Pin::Ref for source
    Pin::Ref srcRef = Pin::Ref();
    const Vertex::PinMap& pinsSrc = source.pin().vertex().pins();
    for(Vertex::PinMap::const_iterator it = pinsSrc.begin(); it != pinsSrc.end(); ++it)
    {
      if (it.value().data() == &source.pin())
      {
        srcRef = it.value().toWeakRef();
        break;
      }
    }
    // find correct Pin::Ref for destination
    Pin::Ref destRef = Pin::Ref();
    const Vertex::PinMap& pinsDest = dest.pin().vertex().pins();
    for(Vertex::PinMap::const_iterator it = pinsDest.begin(); it != pinsDest.end(); ++it)
    {
      if (it.value().data() == &dest.pin())
      {
        destRef = it.value().toWeakRef();
      }
    }
    onEdgeToBeCreated(srcRef,destRef);
  }

}
