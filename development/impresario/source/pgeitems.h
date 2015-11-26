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
**   along with Impresario in subdirectory "licenses", file "LICENSE.GPLv3".
**   If not, see <http://www.gnu.org/licenses/>.
******************************************************************************************/
#ifndef PGEITEMS_H
#define PGEITEMS_H

#include "graphitems.h"
#include "graphelements.h"
#include "pgewndprops.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>

namespace pge
{
  class MacroPinItem : public graph::PinItem
  {
  public:
    MacroPinItem(graph::Pin& pinRef, BaseItem* parent = 0);
    ~MacroPinItem() {}

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

  protected:
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
  };

  class MacroLinkItem : public graph::EdgeItem, public PropUpdateInterface
  {
  public:
    MacroLinkItem(graph::Edge& edgeRef, BaseItem* parent = 0) : graph::EdgeItem(edgeRef,parent) {}
    ~MacroLinkItem() {}

    virtual void setupProperties(WndProperties& propWnd) const;
    virtual void updateProperties(WndProperties& propWnd) const;
    virtual void propertyChanged(QtVariantProperty& prop);

  protected:
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
  };

  class MacroItem : public graph::VertexItem, public PropUpdateInterface
  {
  public:
    MacroItem(graph::Vertex& vertexRef, BaseItem* parent = 0) : graph::VertexItem(vertexRef,parent) {}
    ~MacroItem() {}

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void setupProperties(WndProperties& propWnd) const;
    virtual void updateProperties(WndProperties& propWnd) const;
    virtual void propertyChanged(QtVariantProperty& prop);

  protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
  };
}

#endif // PGEITEMS_H
