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
#ifndef GRAPHITEMS_H
#define GRAPHITEMS_H

#include "graphdefines.h"
#include "graphresources.h"
#include "graphserializer.h"
#include "libavoid/shape.h"
#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QFocusEvent>
#include <QRectF>
#include <QPointF>
#include <QSizeF>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

namespace graph
{
  class BaseElement;
  class Pin;
  class Edge;
  class Vertex;

  class BaseItem : public QGraphicsObject, public Serializer
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<BaseItem> Ptr;

    enum ItemType
    {
      PinItemType = UserType + 1,
      EdgeItemType,
      VertexItemType
    };

    BaseItem(BaseElement& ref, BaseItem* parent = 0);
    BaseItem();

    BaseElement& baseElement() const
    {
      return elementRef;
    }

    void setPalette(Palette::Ptr palette)
    {
      elementPal = palette;
    }

    const Palette& palette() const;

    void setState(Defines::ItemStateType state)
    {
      if (elementState != state)
      {
        elementState = state;
        update();
      }
    }

    Defines::ItemStateType state() const
    {
      return elementState;
    }

    const Cursors& cursors() const;

    void setBlocked(bool blockOn)
    {
      blocked = blockOn;
      notifyBlock(blocked);
    }

    bool isBlocked() const
    {
      return blocked;
    }

  protected:
    virtual void notifyBlock(bool /*blockOn*/) {}

  protected:
    BaseElement&  elementRef;
    Defines::ItemStateType elementState;

  private:
    Palette::Ptr  elementPal;
    bool          blocked;
  };

  class PinItem : public BaseItem
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<PinItem> Ptr;

    PinItem(Pin& pinRef, BaseItem* parent = 0);
    ~PinItem();

    virtual int type() const
    {
      return BaseItem::PinItemType;
    }

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void paintToPixmap(QPainter* painter);

    Pin& pin() const;

    void setConnectionAllowed(bool allowed);

    bool connectionAllowed() const
    {
      return connAllowed;
    }

    virtual QPointF edgeAnchor() const;

  protected:
    virtual void notifyBlock(bool blockOn);

  private:
    bool connAllowed;
  };

  class EdgeItemUpdateInterface
  {
    friend class VertexItem;
  private:
    virtual void refresh() = 0;
  };

  class EdgeItem : public BaseItem, public EdgeItemUpdateInterface
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<EdgeItem> Ptr;

    EdgeItem(Edge& edgeRef, BaseItem* parent = 0);
    EdgeItem(PinItem* startPin, QPointF endPoint);
    ~EdgeItem();

    virtual int type() const
    {
      return BaseItem::EdgeItemType;
    }

    Edge& edge() const;

    virtual QRectF boundingRect() const;
    QPainterPath shape() const;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    const PinItem* source() const
    {
      return sourcePin;
    }

    const PinItem* destination() const
    {
      return destPin;
    }

    void setEndPoint(QPointF end);

  protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

  private:
    static void routerCallback(void *ptr);
    void setupLinkPath();
    Avoid::ConnDirFlag connectorDirection(PinItem* pinItem, bool invert = false);
    virtual void refresh();

    Avoid::ConnRef* routerConnector;

    PinItem*        sourcePin;
    PinItem*        destPin;
    QPainterPath    linkPath;
    QPainterPath    linkArrowHead;
    QPointF         ptEnd;
    QRectF          rectBound;
    QMatrix         arrowHeadTransform;
  };

  class VertexItem : public BaseItem
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<VertexItem> Ptr;

    VertexItem(Vertex& vertexRef, BaseItem* parent = 0);
    ~VertexItem();

    virtual int type() const
    {
      return BaseItem::VertexItemType;
    }

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    QPixmap paintToPixmap();

    Vertex& vertex() const;

    QSizeF minItemSize() const
    {
      return minRectSize;
    }

    void setMinItemSize(QSizeF minItemSize)
    {
      minRectSize = minItemSize;
    }

    bool resizingEnabled() const
    {
      return resizeEnabled;
    }

    void setResizingEnabled(bool enable = true)
    {
      resizeEnabled = enable;
    }

    qreal minEdgeDistance() const
    {
      return minEdgeDist;
    }

    void setMinEdgeDistance(qreal distance)
    {
      minEdgeDist = distance;
    }

    qreal pinSpacing() const
    {
      return pinSpace;
    }

    void setPinSpacing(qreal space)
    {
      pinSpace = space;
    }

    QRectF itemRect() const
    {
      return rect;
    }

    void setItemRect(QRectF itemRect);

    void setItemLayout(Defines::LayoutDirectionType direction);

    Defines::LayoutDirectionType itemLayout() const;

    virtual void save(QXmlStreamWriter& stream) const;
    virtual bool load(QXmlStreamReader &stream);

  protected:
    virtual void notifyBlock(bool blockOn);
    virtual QSizeF calcItemMinSize(const QSizeF& minAllowedSize, Defines::LayoutDirectionType direction);
    virtual void pinsCollect(int& cntIn, QSizeF& maxInSize, int& cntOut, QSizeF& maxOutSize);
    virtual void pinsSetPosition(const int& cntIn, const QSizeF& maxInSize, const int& cntOut, const QSizeF& maxOutSize);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event);

    QRectF innerRect() const
    {
      return rcInterior;
    }

  private:
    void updateEdgeItems();

    QRectF                       rect;
    QRectF                       rcInterior;
    QSizeF                       minRectSize;
    QSizeF                       minSize;
    qreal                        minEdgeDist;
    bool                         resizeEnabled;
    bool                         resizeMode;
    int                          resizeCorner;
    QRectF                       resizeArea[4];
    QRectF                       resizeStartRect;
    bool                         moveMode;
    QPointF                      moveStartPos;
    int                          countInputPins;
    int                          countOutputPins;
    QSizeF                       maxInputPinSize;
    QSizeF                       maxOutputPinSize;
    qreal                        pinSpace;
    Defines::LayoutDirectionType layoutDir;
    Avoid::ShapeRef*             routerShape;

  };
}

Q_DECLARE_METATYPE(graph::BaseItem::Ptr)

#endif // GRAPHITEMS_H
