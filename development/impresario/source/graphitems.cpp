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

#include "graphitems.h"
#include "graphelements.h"
#include "grapheditor.h"
#include "graphresources.h"
#include <qmath.h>
#include <QApplication>
#include <QRegExp>

namespace graph
{
  //-----------------------------------------------------------------------
  // Class BaseItem
  //-----------------------------------------------------------------------
  static BaseElement InvalidElement;

  BaseItem::BaseItem(BaseElement& ref, BaseItem* parent) : QGraphicsObject(parent), Serializer("visualization",BaseItem::staticMetaObject.propertyOffset(),this),
    elementRef(ref), elementState(Defines::Normal), elementPal(), blocked(false)
  {
  }

  BaseItem::BaseItem() : QGraphicsObject(0), Serializer("visualization",BaseItem::staticMetaObject.propertyOffset(),this),
    elementRef(InvalidElement), elementState(Defines::Normal), elementPal(), blocked(false)
  {
  }

  const Palette& BaseItem::palette() const
  {
    if (!elementPal.isNull())
    {
      return *elementPal;
    }
    Scene* graphScene = qobject_cast<Scene*>(scene());
    if (graphScene != 0)
    {
      return graphScene->palette();
    }
    return *Palette::graphDefault();
  }

  const Cursors& BaseItem::cursors() const
  {
    Scene* graphScene = qobject_cast<Scene*>(scene());
    if (graphScene != 0 && !graphScene->views().isEmpty())
    {
      SceneEditor* editor = static_cast<SceneEditor*>(graphScene->views().first());
      if (editor != 0)
      {
        return editor->cursorSet();
      }
    }
    return *Cursors::graphDefault();
  }

  //-----------------------------------------------------------------------
  // Class PinItem
  //-----------------------------------------------------------------------
  PinItem::PinItem(Pin& pinRef, BaseItem* parent) : BaseItem(pinRef,parent),
    connAllowed(true)
  {
    setCursor(cursors().cursor(Cursors::CursorConnectValid));
  }

  PinItem::~PinItem()
  {
  }

  QPointF PinItem::edgeAnchor() const
  {
    VertexItem* vertexItem = static_cast<VertexItem*>(parentItem());
    Q_ASSERT(vertexItem != 0);
    QPointF ptAnchor;
    QRectF itemRect = boundingRect();
    switch(vertexItem->itemLayout())
    {
      case Defines::TopToBottom:
        ptAnchor = QPointF(itemRect.center().x(),(pin().direction() == Defines::Incoming) ? itemRect.top() : itemRect.bottom());
        break;
      case Defines::LeftToRight:
        ptAnchor = QPointF((pin().direction() == Defines::Incoming) ? itemRect.left() : itemRect.right(),itemRect.center().y());
        break;
      case Defines::RightToLeft:
        ptAnchor = QPointF((pin().direction() == Defines::Incoming) ? itemRect.right() : itemRect.left(),itemRect.center().y());
        break;
    }
    return mapToScene(ptAnchor);
  }

  void PinItem::setConnectionAllowed(bool allowed)
  {
    if (pin().direction() != Defines::Incoming)
    {
      return;
    }
    connAllowed = allowed;
    setCursor((connAllowed) ? cursors().cursor(Cursors::CursorConnectValid) : cursors().cursor(Cursors::CursorConnectInvalid));
  }

  QRectF PinItem::boundingRect() const
  {
    return QRectF(-3.0, -3.0, 6.0, 6.0);
  }

  void PinItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
  {
    const Palette& pal = palette();
    switch(pin().direction())
    {
      case Defines::Incoming:
        painter->setPen(pal.pen(Palette::PinIncomingBorder));
        painter->setBrush(pal.brush(Palette::PinIncomingFill));
        break;
      case Defines::Outgoing:
        painter->setPen(pal.pen(Palette::PinOutgoingBorder));
        painter->setBrush(pal.brush(Palette::PinOutgoingFill));
        break;
      case Defines::NonDirectional:
        painter->setPen(pal.pen(Palette::PinUndirectionalBorder));
        painter->setBrush(pal.brush(Palette::PinUndirectionalFill));
        break;
      case Defines::Undefined:
        painter->setPen(pal.pen(Palette::PinUndefinedBorder));
        painter->setBrush(pal.brush(Palette::PinUndefinedFill));
        break;
    }
    QRectF itemRect = boundingRect();
    painter->drawEllipse(QPointF(0.0,0.0),itemRect.width() / 2.0,itemRect.height() / 2.0);
  }

  void PinItem::paintToPixmap(QPainter* painter)
  {
    QTransform transform = QTransform::fromTranslate(pos().x(),pos().y());
    painter->setTransform(transform);
    this->paint(painter,0,0);
    painter->resetTransform();
  }

  Pin& PinItem::pin() const
  {
    return static_cast<Pin&>(elementRef);
  }

  void PinItem::notifyBlock(bool blockOn)
  {
    if (blockOn)
    {
      unsetCursor();
    }
    else
    {
      setCursor((connAllowed) ? cursors().cursor(Cursors::CursorConnectValid) : cursors().cursor(Cursors::CursorConnectInvalid));
    }
  }

  //-----------------------------------------------------------------------
  // Class EdgeItem
  //-----------------------------------------------------------------------

  EdgeItem::EdgeItem(Edge& edgeRef, BaseItem* parent) : BaseItem(edgeRef,parent),
    routerConnector(0), linkPath(), linkArrowHead(), ptEnd(), rectBound(), arrowHeadTransform()
  {
    sourcePin = edge().srcPin().toStrongRef()->sceneItem().staticCast<PinItem>().data();
    destPin = edge().destPin().toStrongRef()->sceneItem().staticCast<PinItem>().data();
    setFlag(QGraphicsItem::ItemIsSelectable,true);
    setFlag(QGraphicsItem::ItemIsMovable,false);

    linkArrowHead.moveTo(-4.0,-10.0);
    linkArrowHead.lineTo(+4.0,-10.0);
    linkArrowHead.lineTo(0.0,0.0);
    linkArrowHead.closeSubpath();
  }

  EdgeItem::EdgeItem(PinItem* startPin, QPointF endPoint) : BaseItem(),
    routerConnector(0), sourcePin(startPin), destPin(), linkPath(), linkArrowHead(), ptEnd(endPoint), rectBound(), arrowHeadTransform()
  {
    elementState = Defines::Invalid;
    linkArrowHead.moveTo(-4.0,-10.0);
    linkArrowHead.lineTo(+4.0,-10.0);
    linkArrowHead.lineTo(0.0,0.0);
    linkArrowHead.closeSubpath();
  }

  EdgeItem::~EdgeItem()
  {
    routerConnector = 0;
  }

  Edge& EdgeItem::edge() const
  {
    return static_cast<Edge&>(elementRef);
  }

  void EdgeItem::setEndPoint(QPointF end)
  {
    ptEnd = end;

    Avoid::ConnEnd dstPt(Avoid::Point(ptEnd.x(),ptEnd.y()),connectorDirection(sourcePin,true));
    routerConnector->setDestEndpoint(dstPt);
    routerConnector->router()->processTransaction();
  }

  QRectF EdgeItem::boundingRect() const
  {
    return rectBound;
  }

  QPainterPath EdgeItem::shape() const
  {
    if (destPin)
    {
      QPainterPathStroker stroker;
      stroker.setWidth(4.0);
      return stroker.createStroke(linkPath);
    }
    else
    {
      return linkPath;
    }
  }

  void EdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
  {
    const Palette& pal = palette();
    if (isSelected())
    {
      elementState = Defines::Selected;
    }
    else if (destPin)
    {
      elementState = Defines::Normal;
    }
    painter->setPen(pal.pen(Palette::EdgeBorder,state()));
    painter->drawPath(linkPath);
    if (destPin && destPin->pin().direction() == Defines::Incoming)
    {
      painter->setBrush(painter->pen().color());
      QPainterPath arrowHead = arrowHeadTransform.map(linkArrowHead);
      painter->drawPath(arrowHead);
    }
  }

  QVariant EdgeItem::itemChange(GraphicsItemChange change, const QVariant &value)
  {
    switch(change)
    {
      case QGraphicsItem::ItemSceneChange:
      {
        QGraphicsScene* scenePtr = value.value<QGraphicsScene*>();
        if (scenePtr != 0)
        {
          Scene* pgScene = dynamic_cast<Scene*>(scenePtr);
          QPointF ptSource = sourcePin->edgeAnchor();
          QPointF ptDest = (destPin != 0) ? destPin->edgeAnchor() : ptEnd;
          Avoid::ConnEnd srcPt(Avoid::Point(ptSource.x(),ptSource.y()),connectorDirection(sourcePin,false));
          Avoid::ConnEnd dstPt(Avoid::Point(ptDest.x(),ptDest.y()),connectorDirection((destPin != 0) ? destPin : sourcePin,destPin == 0));
          routerConnector = new Avoid::ConnRef(&(pgScene->router()), srcPt,dstPt);
          routerConnector->setCallback(routerCallback,this);
          pgScene->router().processTransaction();
        }
        else
        {
          Scene* pgScene = dynamic_cast<Scene*>(scene());
          pgScene->router().deleteConnector(routerConnector);
          pgScene->router().processTransaction();
          routerConnector = 0;
          linkPath = QPainterPath();
        }
        break;
      }
      default:
        break;
    }
    return value;
  }

  void EdgeItem::routerCallback(void *ptr)
  {
    EdgeItem* edgeItem = reinterpret_cast<EdgeItem*>(ptr);
    if (edgeItem) edgeItem->setupLinkPath();
  }

  void EdgeItem::setupLinkPath()
  {
    QPainterPath newPath;
    const Avoid::PolyLine route = routerConnector->displayRoute();
    newPath.moveTo(route.at(0).x,route.at(0).y);
    for (size_t i = 1; i < route.size(); ++i)
    {
      newPath.lineTo(route.at(i).x,route.at(i).y);
    }
    QPointF pt1 = QPointF(route.at(route.size() - 1).x,route.at(route.size() - 1).y);
    QPointF pt2 = QPointF(route.at(route.size() - 2).x,route.at(route.size() - 2).y);
    QPointF diff = pt1 - pt2;
    qreal angle = qAtan2(diff.x(),diff.y()) * -180.0 / 3.14159265;
    prepareGeometryChange();
    linkPath = newPath;
    rectBound = linkPath.boundingRect().adjusted(-4.0,-4.0,4.0,4.0).normalized();
    arrowHeadTransform.reset();
    arrowHeadTransform.translate(pt1.x(),pt1.y());
    arrowHeadTransform.rotate(angle);
  }

  Avoid::ConnDirFlag EdgeItem::connectorDirection(PinItem* pinItem, bool invert)
  {
    Avoid::ConnDirFlag dir = Avoid::ConnDirNone;
    VertexItem* vertexItem = dynamic_cast<VertexItem*>(pinItem->parentItem());
    Q_ASSERT(vertexItem != 0);
    switch(vertexItem->itemLayout())
    {
      case Defines::TopToBottom:
        dir = Avoid::ConnDirDown;
        break;
      case Defines::LeftToRight:
        dir = Avoid::ConnDirRight;
        break;
      case Defines::RightToLeft:
        dir = Avoid::ConnDirLeft;
        break;
    }
    if ((pinItem->pin().direction() == Defines::Incoming) != invert)
    {
      switch(dir)
      {
        case Avoid::ConnDirDown:
          dir = Avoid::ConnDirUp;
          break;
        case Avoid::ConnDirRight:
          dir = Avoid::ConnDirLeft;
          break;
        case Avoid::ConnDirLeft:
          dir = Avoid::ConnDirRight;
          break;
        default:
          break;
      }
    }
    return dir;
  }

  void EdgeItem::refresh()
  {
    QPointF ptSource = sourcePin->edgeAnchor();
    QPointF ptDest = (destPin != 0) ? destPin->edgeAnchor() : ptEnd;
    Avoid::ConnEnd srcPt(Avoid::Point(ptSource.x(),ptSource.y()),connectorDirection(sourcePin,false));
    Avoid::ConnEnd dstPt(Avoid::Point(ptDest.x(),ptDest.y()),connectorDirection((destPin != 0) ? destPin : sourcePin,destPin == 0));
    routerConnector->setEndpoints(srcPt,dstPt);
    //routerConnector->router()->processTransaction();
  }

  //-----------------------------------------------------------------------
  // Class VertexItem
  //-----------------------------------------------------------------------
  VertexItem::VertexItem(Vertex& vertexRef, BaseItem* parent) : BaseItem(vertexRef,parent),
    rect(), rcInterior(), minRectSize(100.0,70.0), minSize(minRectSize), minEdgeDist(15.0),
    resizeEnabled(true), resizeMode(false), resizeCorner(-1), resizeArea(), resizeStartRect(),
    moveMode(false), moveStartPos(),
    countInputPins(0), countOutputPins(0), maxInputPinSize(0.0,0.0), maxOutputPinSize(0.0,0.0), pinSpace(5.0),
    layoutDir(Defines::TopToBottom), routerShape(0)
  {
    setFlag(QGraphicsItem::ItemIsMovable,true);
    setFlag(QGraphicsItem::ItemIsSelectable,true);
    setFlag(QGraphicsItem::ItemIsFocusable,true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges,true);
    setAcceptHoverEvents(true);

    pinsCollect(countInputPins,maxInputPinSize,countOutputPins,maxOutputPinSize);

    layoutDir = itemLayout();
    setItemLayout(layoutDir);
  }

  VertexItem::~VertexItem()
  {
    routerShape = 0;
  }

  QRectF VertexItem::boundingRect() const
  {
    const Palette& pal = palette();
    qreal penWidth = pal.pen(Palette::VertexBorder).widthF();
    return rect.adjusted(-penWidth/2,-penWidth/2,+penWidth/2,+penWidth/2);
  }

  void VertexItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
  {
    prepareGeometryChange();
    const Palette& pal = palette();
    // check layout direction and change it if necessary
    Defines::LayoutDirectionType dir = itemLayout();
    if (dir != layoutDir)
    {
      setItemLayout(dir);
    }
    // draw background rectangle
    if (hasFocus())
    {
      painter->setPen(pal.pen(Palette::VertexBorder,Defines::Focused));
    }
    else if (isSelected())
    {
      painter->setPen(pal.pen(Palette::VertexBorder,Defines::Selected));
    }
    else
    {
      painter->setPen(pal.pen(Palette::VertexBorder));
    }
    // draw macro item
    painter->setBrush(pal.brush(Palette::VertexBackgroundFill));
    painter->drawRoundedRect(rect, 10.0, 10.0, Qt::AbsoluteSize);
    // add inner rectangle
    QRectF innerRect = rect.adjusted(+3,+3,-2,-2);
    painter->setPen(QPen(Qt::NoPen));
    painter->setBrush(pal.brush(Palette::VertexInteriorFill));
    painter->drawRoundedRect(innerRect, 10.0, 10.0, Qt::AbsoluteSize);
    // paint macro interior dependent on layout
    switch(layoutDir)
    {
      case Defines::TopToBottom:
      {
        // paint lines and background to separate pins from text and status symbols
        qreal offsetTop = (countInputPins > 0) ? maxInputPinSize.height() - 1.0 : 2.0;
        qreal offsetBottom = (countOutputPins > 0) ? -maxOutputPinSize.height() : -2.0;
        rcInterior = innerRect.adjusted(1.0,offsetTop,-1.0,offsetBottom);
        painter->setPen(QPen(Qt::NoPen));
        painter->setBrush(pal.brush(Palette::VertexTextBackgroundFill));
        painter->drawRect(rcInterior);
        painter->setPen(pal.pen(Palette::VertexBorder));
        if (countInputPins > 0) painter->drawLine(rcInterior.topLeft(),rcInterior.topRight());
        if (countOutputPins > 0) painter->drawLine(rcInterior.bottomLeft(),rcInterior.bottomRight());
        break;
      }
      case Defines::LeftToRight:
      {
        // paint lines and background to separate macro name and macro time from in- and outputs
        qreal offsetLeft = (countInputPins > 0) ? maxInputPinSize.height() - 1.0 : 2.0;
        qreal offsetRight = (countOutputPins > 0) ? -maxOutputPinSize.height() : -2.0;
        rcInterior = innerRect.adjusted(offsetLeft,1.0,offsetRight,-1.0);
        painter->setPen(QPen(Qt::NoPen));
        painter->setBrush(pal.brush(Palette::VertexTextBackgroundFill));
        painter->drawRect(rcInterior);
        painter->setPen(pal.pen(Palette::VertexBorder));
        if (countInputPins > 0) painter->drawLine(rcInterior.topLeft(),rcInterior.bottomLeft());
        if (countOutputPins > 0) painter->drawLine(rcInterior.topRight(),rcInterior.bottomRight());
        break;
      }
      case Defines::RightToLeft:
      {
        // paint lines and background to separate macro name and macro time from in- and outputs
        qreal offsetLeft = (countOutputPins > 0) ? maxOutputPinSize.height() : 2.0;
        qreal offsetRight = (countInputPins > 0) ? -maxInputPinSize.height() : -2.0;
        rcInterior = innerRect.adjusted(offsetLeft,1.0,offsetRight,-1.0);
        painter->setPen(QPen(Qt::NoPen));
        painter->setBrush(pal.brush(Palette::VertexTextBackgroundFill));
        painter->drawRect(rcInterior);
        painter->setPen(pal.pen(Palette::VertexBorder));
        if (countOutputPins > 0) painter->drawLine(rcInterior.topLeft(),rcInterior.bottomLeft());
        if (countInputPins > 0) painter->drawLine(rcInterior.topRight(),rcInterior.bottomRight());
        break;
      }
    }
    // paint resize handles in case item has focus
    if (hasFocus() && resizeEnabled && !isBlocked())
    {
      painter->setPen(pal.pen(Palette::VertexResizeHandleBorder,Defines::Focused));
      painter->setBrush(pal.brush(Palette::VertexResizeHandleFill,Defines::Focused));
      painter->setOpacity(0.7);
      for(int i = 0; i < 4; ++i)
      {
        painter->drawEllipse(resizeArea[i].center(),3,3);
      }
    }
  }

  Vertex& VertexItem::vertex() const
  {
    return static_cast<Vertex&>(elementRef);
  }

  QPixmap VertexItem::paintToPixmap()
  {
    // shift item rect as pixmaps cannot handle negative coordinates
    QRectF pixRect = rect;
    rect.moveTopLeft(QPointF(0.0,0.0));
    pinsSetPosition(countInputPins,maxInputPinSize,countOutputPins,maxOutputPinSize);
    // draw item including pins
    QPixmap pixmap(rect.size().toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    this->paint(&painter,0,0);
    foreach(QGraphicsItem* item, childItems())
    {
      PinItem* pin = static_cast<PinItem*>(item);
      pin->paintToPixmap(&painter);
    }
    // undo transform on item rect and pins
    rect = pixRect;
    pinsSetPosition(countInputPins,maxInputPinSize,countOutputPins,maxOutputPinSize);
    // make pixmap transparent
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.fillRect(pixmap.rect(), QColor(0, 0, 0, 150));
    return pixmap;
  }

  void VertexItem::setItemRect(QRectF itemRect)
  {
    prepareGeometryChange();
    rect = itemRect;
    resizeArea[0] = QRectF(rect.topLeft().x(),rect.topLeft().y(),5.0,5.0);
    resizeArea[1] = QRectF(rect.bottomLeft().x(),rect.bottomLeft().y() - 5.0,5.0,5.0);
    resizeArea[2] = QRectF(rect.bottomRight().x() - 5.0,rect.bottomRight().y() - 5.0,5.0,5.0);
    resizeArea[3] = QRectF(rect.topRight().x() - 5.0,rect.topRight().y(),5.0,5.0);
    pinsSetPosition(countInputPins,maxInputPinSize,countOutputPins,maxOutputPinSize);
    updateEdgeItems();
  }

  void VertexItem::setItemLayout(Defines::LayoutDirectionType direction)
  {
    prepareGeometryChange();
    layoutDir = direction;
    minSize = calcItemMinSize(minRectSize,layoutDir);
    rect = QRectF(QPointF(-minSize.width() / 2, -minSize.height() / 2), minSize);
    resizeArea[0] = QRectF(rect.topLeft().x(),rect.topLeft().y(),5.0,5.0);
    resizeArea[1] = QRectF(rect.bottomLeft().x(),rect.bottomLeft().y() - 5.0,5.0,5.0);
    resizeArea[2] = QRectF(rect.bottomRight().x() - 5.0,rect.bottomRight().y() - 5.0,5.0,5.0);
    resizeArea[3] = QRectF(rect.topRight().x() - 5.0,rect.topRight().y(),5.0,5.0);

    pinsSetPosition(countInputPins,maxInputPinSize,countOutputPins,maxOutputPinSize);
    updateEdgeItems();
  }

  Defines::LayoutDirectionType VertexItem::itemLayout() const
  {
    Scene* graphScene = 0;
    if (!scene())
    {
      // TODO: Find better solution for detecting active editor
      QWidget* topWindow = QApplication::activeWindow();
      if (topWindow)
      {
        QList<SceneEditor*> editors = topWindow->findChildren<SceneEditor*>();
        foreach(SceneEditor* editor, editors)
        {
          if (editor->isActiveWindow())
          {
            graphScene = qobject_cast<Scene*>(editor->scene());
            break;
          }
        }
      }
    }
    else
    {
      graphScene = qobject_cast<Scene*>(scene());
    }
    if (graphScene)
    {
      return graphScene->graphLayout();
    }
    else
    {
      return layoutDir;
    }
  }

  void VertexItem::save(QXmlStreamWriter& stream) const
  {
    QString strPos = QString("{%1;%2}").arg(pos().x()).arg(pos().y());
    QString strSize = QString("{%1;%2}").arg(rect.width()).arg(rect.height());
    writeElementStart(stream);
    stream.writeTextElement("position",strPos);
    stream.writeTextElement("size",strSize);
    writeElementEnd(stream);
  }

  bool VertexItem::load(QXmlStreamReader &stream)
  {
    if (!readElementStart(stream)) return false;
    QString className = QString(metaObject()->className());
    // read item position
    if (stream.readNextStartElement() && stream.name() == "position")
    {
      QString content = stream.readElementText();
      QRegExp regExp("\\{(-?\\d+(\\.\\d+)?);(-?\\d+(\\.\\d+)?)\\}");
      if (regExp.exactMatch(content))
      {
        setPos(QPointF(regExp.cap(1).toDouble(),regExp.cap(3).toDouble()));
      }
      else
      {
        stream.raiseError(QString(QObject::tr("At line %1, column %2: Invalid format for property 'position' of class instance '%3' (element '%4').")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(className).arg(element()));
        return false;
      }
    }
    else
    {
      stream.raiseError(QString(QObject::tr("At line %1, column %2: Failed to read element for property 'position' of class instance '%3' (element '%4').")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(className).arg(element()));
      return false;
    }

    // read item size
    if (stream.readNextStartElement() && stream.name() == "size")
    {
      QString content = stream.readElementText();
      QRegExp regExp("\\{(\\d+(\\.\\d+)?);(\\d+(\\.\\d+)?)\\}");
      if (regExp.exactMatch(content))
      {
        QSizeF size(regExp.cap(1).toDouble(),regExp.cap(3).toDouble());
        QSizeF min = calcItemMinSize(minSize,layoutDir);
        if (size.width() < min.width())
        {
          size.setWidth(min.width());
        }
        if (size.height() < min.height())
        {
          size.setHeight(min.height());
        }
        setItemRect(QRectF(QPointF(-size.width()/2.0,-size.height()/2.0),size));
      }
      else
      {
        stream.raiseError(QString(QObject::tr("At line %1, column %2: Invalid format for property 'size' of class instance '%3' (element '%4').")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(className).arg(element()));
        return false;
      }

    }
    else
    {
      stream.raiseError(QString(QObject::tr("At line %1, column %2: Failed to read element for property 'size' of class instance '%3' (element '%4').")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(className).arg(element()));
      return false;
    }
    if (!readElementEnd(stream)) return false;
    return true;
  }

  void VertexItem::notifyBlock(bool blockOn)
  {
    setFlag(ItemIsMovable,!blockOn);
  }

  QSizeF VertexItem::calcItemMinSize(const QSizeF& minAllowedSize, Defines::LayoutDirectionType direction)
  {
    QSizeF min = minAllowedSize;
    qreal requiredHeight = 0.0;
    qreal requiredWidth = 0.0;
    switch(direction)
    {
      case Defines::TopToBottom:
      {
        requiredWidth = qMax(countInputPins * maxInputPinSize.width() + (countInputPins + 1) * pinSpace,countOutputPins * maxOutputPinSize.width() + (countOutputPins + 1) * pinSpace);
        requiredHeight = maxInputPinSize.height() + maxOutputPinSize.height() + 2.0;
        break;
      }
      case Defines::LeftToRight:
      case Defines::RightToLeft:
      {
        requiredHeight = qMax(countInputPins * maxInputPinSize.width() + (countInputPins + 1) * pinSpace,countOutputPins * maxOutputPinSize.width() + (countOutputPins + 1) * pinSpace);
        requiredWidth = maxInputPinSize.height() + maxOutputPinSize.height() + 2.0;
        break;
      }
    }
    if (min.width() < requiredWidth)
    {
      min.setWidth(requiredWidth);
    }
    if (min.height() < requiredHeight)
    {
      min.setHeight(requiredHeight);
    }
    return min;
  }

  void VertexItem::pinsCollect(int& cntIn, QSizeF& maxInSize, int& cntOut, QSizeF& maxOutSize)
  {
    cntIn = cntOut = 0;
    maxInSize = maxOutSize = QSizeF(0.0,0.0);
    const Vertex::PinMap& pinMap = vertex().pins();
    for(Vertex::PinMap::const_iterator it = pinMap.begin(); it != pinMap.end(); ++it)
    {
      BaseItem::Ptr itemPtr = it.value()->sceneItem(this);
      switch(it.key())
      {
        case Defines::Incoming:
        case Defines::NonDirectional:
          cntIn++;
          maxInSize.setWidth(qMax<qreal>(maxInSize.width(),itemPtr->boundingRect().width()));
          maxInSize.setHeight(qMax<qreal>(maxInSize.height(),itemPtr->boundingRect().height()));
          break;
        case Defines::Outgoing:
        case Defines::Undefined:
          cntOut++;
          maxOutSize.setWidth(qMax<qreal>(maxOutSize.width(),itemPtr->boundingRect().width()));
          maxOutSize.setHeight(qMax<qreal>(maxOutSize.height(),itemPtr->boundingRect().height()));
          break;
      }
    }
    if (itemLayout() != Defines::TopToBottom)
    {
      maxInSize.transpose();
      maxOutSize.transpose();
    }
  }

  void VertexItem::pinsSetPosition(const int& cntIn, const QSizeF& maxInSize, const int& cntOut, const QSizeF& maxOutSize)
  {
    switch(layoutDir)
    {
      case Defines::TopToBottom:
        if (cntIn > 0)
        {
          qreal pinSize = cntIn * maxInSize.width() + (cntIn - 1) * pinSpace;
          qreal x = rect.left() + (rect.width() - pinSize) / 2.0 + maxInSize.width() / 2.0;
          qreal y = rect.top() + maxInSize.height() / 2.0;
          for(int i = 0; i < cntIn; ++i)
          {
            childItems().at(i)->setPos(x,y);
            x += pinSpace + maxInSize.width();
          }
        }
        if (cntOut > 0)
        {
          qreal pinSize = cntOut * maxOutSize.width() + (cntOut - 1) * pinSpace;
          qreal x = rect.left() + (rect.width() - pinSize) / 2.0 + maxOutSize.width() / 2.0;
          qreal y = rect.bottom() - maxOutSize.height() / 2.0;
          for(int i = cntIn; i < cntIn + cntOut; ++i)
          {
            childItems().at(i)->setPos(x,y);
            x += pinSpace + maxOutSize.width();
          }
        }
        break;
      case Defines::LeftToRight:
        if (cntIn > 0)
        {
          qreal pinSize = cntIn * maxInSize.width() + (cntIn - 1) * pinSpace;
          qreal x = rect.left() + maxInSize.height() / 2.0;
          qreal y = rect.top() + (rect.height() - pinSize) / 2.0 + maxInSize.width() / 2.0;
          for(int i = 0; i < cntIn; ++i)
          {
            childItems().at(i)->setPos(x,y);
            y += pinSpace + maxInSize.width();
          }
        }
        if (cntOut > 0)
        {
          qreal pinSize = cntOut * maxOutSize.width() + (cntOut - 1) * pinSpace;
          qreal x = rect.right() - maxOutSize.height() / 2.0;
          qreal y = rect.top() + (rect.height() - pinSize) / 2.0 + maxOutSize.width() / 2.0;
          for(int i = cntIn; i < cntIn + cntOut; ++i)
          {
            childItems().at(i)->setPos(x,y);
            y += pinSpace + maxOutSize.width();
          }
        }
        break;
      case Defines::RightToLeft:
        if (cntIn > 0)
        {
          qreal pinSize = cntIn * maxInSize.width() + (cntIn - 1) * pinSpace;
          qreal x = rect.right() - maxInSize.height() / 2.0;
          qreal y = rect.top() + (rect.height() - pinSize) / 2.0 + maxInSize.width() / 2.0;
          for(int i = 0; i < cntIn; ++i)
          {
            childItems().at(i)->setPos(x,y);
            y += pinSpace + maxInSize.width();
          }
        }
        if (cntOut > 0)
        {
          qreal pinSize = cntOut * maxOutSize.width() + (cntOut - 1) * pinSpace;
          qreal x = rect.left() + maxOutSize.height() / 2.0;
          qreal y = rect.top() + (rect.height() - pinSize) / 2.0 + maxOutSize.width() / 2.0;
          for(int i = cntIn; i < cntIn + cntOut; ++i)
          {
            childItems().at(i)->setPos(x,y);
            y += pinSpace + maxOutSize.width();
          }
        }
        break;
    }
  }

  QVariant VertexItem::itemChange(GraphicsItemChange change, const QVariant &value)
  {
    switch(change)
    {
      case QGraphicsItem::ItemPositionHasChanged:
      {
        updateEdgeItems();
        break;
      }
      case QGraphicsItem::ItemSceneChange:
      {
        QGraphicsScene* scenePtr = value.value<QGraphicsScene*>();
        if (scenePtr != 0)
        {
          Scene* graphScene = dynamic_cast<Scene*>(scenePtr);
          QPointF ptTopLeft = mapToScene(rect.topLeft()) - QPointF(minEdgeDist,minEdgeDist);
          QPointF ptBtmRight = mapToScene(rect.bottomRight()) + QPointF(minEdgeDist,minEdgeDist);
          Avoid::Rectangle rectangle(Avoid::Point(ptTopLeft.x(),ptTopLeft.y()), Avoid::Point(ptBtmRight.x(),ptBtmRight.y()));
          routerShape = new Avoid::ShapeRef(&graphScene->router(), rectangle);
          graphScene->router().processTransaction();
        }
        else
        {
          Scene* graphScene = dynamic_cast<Scene*>(scene());
          graphScene->router().deleteShape(routerShape);
          graphScene->router().processTransaction();
          routerShape = 0;
        }
        break;
      }
      default:
        break;
    }
    return value;
  }

  void VertexItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
  {
    if (resizeMode)
    {
      QPointF delta = event->pos() - event->lastPos();
      switch(resizeCorner)
      {
        case 0:
          if (rect.width() - delta.x() < minSize.width())
          {
            delta.setX(0.0);
          }
          if (rect.height() - delta.y() < minSize.height())
          {
            delta.setY(0.0);
          }
          prepareGeometryChange();
          rect.adjust(delta.x(),delta.y(),0.0,0.0);
          break;
        case 1:
          if (rect.width() - delta.x() < minSize.width())
          {
            delta.setX(0.0);
          }
          if (rect.height() + delta.y() < minSize.height())
          {
            delta.setY(0.0);
          }
          prepareGeometryChange();
          rect.adjust(delta.x(),0.0,0.0,delta.y());
          break;
        case 2:
          if (rect.width() + delta.x() < minSize.width())
          {
            delta.setX(0.0);
          }
          if (rect.height() + delta.y() < minSize.height())
          {
            delta.setY(0.0);
          }
          prepareGeometryChange();
          rect.adjust(0.0,0.0,delta.x(),delta.y());
          break;
        case 3:
          if (rect.width() + delta.x() < minSize.width())
          {
            delta.setX(0.0);
          }
          if (rect.height() - delta.y() < minSize.height())
          {
            delta.setY(0.0);
          }
          prepareGeometryChange();
          rect.adjust(0.0,delta.y(),delta.x(),0.0);
          break;
      }
      resizeArea[0] = QRectF(rect.topLeft().x(),rect.topLeft().y(),5.0,5.0);
      resizeArea[1] = QRectF(rect.bottomLeft().x(),rect.bottomLeft().y() - 5.0,5.0,5.0);
      resizeArea[2] = QRectF(rect.bottomRight().x() - 5.0,rect.bottomRight().y() - 5.0,5.0,5.0);
      resizeArea[3] = QRectF(rect.topRight().x() - 5.0,rect.topRight().y(),5.0,5.0);
      pinsSetPosition(countInputPins,maxInputPinSize,countOutputPins,maxOutputPinSize);
      updateEdgeItems();
    }
    else
    {
      QGraphicsItem::mouseMoveEvent(event);
    }
  }

  void VertexItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
  {
    if (event->button() == Qt::LeftButton && !isBlocked())
    {
      if (resizeCorner >= 0 && resizeEnabled)
      {
        resizeStartRect = rect;
        resizeMode = true;
      }
      else
      {
        moveMode = true;
        moveStartPos = event->scenePos();
      }
    }
    QGraphicsItem::mousePressEvent(event);
  }

  void VertexItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
  {
    if (moveMode && scene() != 0)
    {
      if (moveStartPos != event->scenePos())
      {
        Scene* graphScene = dynamic_cast<Scene*>(scene());
        if (graphScene != 0)
        {
          graphScene->onItemMoved(*this,event->scenePos() - moveStartPos);
        }
      }
      moveMode = false;
    }
    if (resizeMode && scene() != 0)
    {
      if (resizeStartRect != rect)
      {
        Scene* graphScene = dynamic_cast<Scene*>(scene());
        if (graphScene != 0)
        {
          graphScene->onItemResized(*this,resizeStartRect);
        }
      }
      resizeMode = false;
    }
    QGraphicsItem::mouseReleaseEvent(event);
  }

  void VertexItem::focusOutEvent(QFocusEvent* event)
  {
    moveMode = false;
    resizeMode = false;
    resizeCorner = -1;
    unsetCursor();
    QGraphicsItem::focusOutEvent(event);
  }

  void VertexItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
  {
    event->accept();
    if (!hasFocus() || resizeMode || !resizeEnabled || isBlocked())
    {
      return;
    }
    if (resizeArea[0].contains(event->pos()))
    {
      resizeCorner = 0;
      setCursor(Qt::SizeFDiagCursor);
    }
    else if (resizeArea[1].contains(event->pos()))
    {
      resizeCorner = 1;
      setCursor(Qt::SizeBDiagCursor);
    }
    else if (resizeArea[2].contains(event->pos()))
    {
      resizeCorner = 2;
      setCursor(Qt::SizeFDiagCursor);
    }
    else if (resizeArea[3].contains(event->pos()))
    {
      resizeCorner = 3;
      setCursor(Qt::SizeBDiagCursor);
    }
    else
    {
      resizeCorner = -1;
      unsetCursor();
    }
  }

  void VertexItem::updateEdgeItems()
  {
    Scene* graphScene = dynamic_cast<Scene*>(scene());
    if (routerShape != 0 && graphScene != 0)
    {
      QPointF ptTopLeft = mapToScene(rect.topLeft()) - QPointF(minEdgeDist,minEdgeDist);
      QPointF ptBtmRight = mapToScene(rect.bottomRight()) + QPointF(minEdgeDist,minEdgeDist);
      Avoid::Rectangle rectangle(Avoid::Point(ptTopLeft.x(),ptTopLeft.y()), Avoid::Point(ptBtmRight.x(),ptBtmRight.y()));
      graphScene->router().moveShape(routerShape,rectangle);
    }
    const Vertex::EdgeRefMap& edgeMap = vertex().edges();
    for(Vertex::EdgeRefMap::const_iterator it = edgeMap.begin(); it != edgeMap.end(); ++it)
    {
      EdgeItem::Ptr edgeItem = it.value().toStrongRef()->sceneItem().staticCast<EdgeItem>();
      static_cast<EdgeItemUpdateInterface*>(edgeItem.data())->refresh();
    }
    if (graphScene != 0)
    {
      graphScene->router().processTransaction();
    }
  }
}
