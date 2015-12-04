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

#include "pgeitems.h"
#include "graphresources.h"
#include "resources.h"
#include "appmacro.h"
#include <QPainter>
#include <QMenu>
#include <QGraphicsScene>
#include <QGraphicsView>

namespace pge
{
  //-----------------------------------------------------------------------
  // Class MacroPinItem
  //-----------------------------------------------------------------------
  MacroPinItem::MacroPinItem(graph::Pin& pinRef, BaseItem* parent) : graph::PinItem(pinRef,parent)
  {
    setToolTip(pinRef.dataRef()->id());
  }

  void MacroPinItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
  {
    graph::VertexItem* vertexItem = static_cast<graph::VertexItem*>(parentItem());
    Q_ASSERT(vertexItem != 0);
    const graph::Palette& pal = palette();
    QRectF rect = boundingRect();
    switch(vertexItem->itemLayout())
    {
      case graph::Defines::TopToBottom:
      {
        QPainterPath pinPath;
        if (pin().direction() == graph::Defines::Incoming)
        {
          painter->setPen(pal.pen(graph::Palette::PinIncomingBorder));
          pinPath.moveTo(rect.right(),rect.top() + rect.height() / 2.0);
          pinPath.lineTo(rect.topRight());
          pinPath.lineTo(rect.topLeft());
          pinPath.lineTo(rect.left(),rect.top() + rect.height() / 2.0);
          pinPath.arcTo(rect,180.0,180.0);
          pinPath.closeSubpath();
          painter->setBrush(pal.brush(graph::Palette::PinIncomingFill));
          painter->drawPath(pinPath);
        }
        else
        {
          painter->setPen(pal.pen(graph::Palette::PinOutgoingBorder));
          pinPath.moveTo(rect.right(),rect.top() + rect.height() / 2.0);
          pinPath.lineTo(rect.bottomRight());
          pinPath.lineTo(rect.bottomLeft());
          pinPath.lineTo(rect.left(),rect.top() + rect.height() / 2.0);
          pinPath.arcTo(rect,180.0,-180.0);
          pinPath.closeSubpath();
          painter->setBrush(pal.brush(graph::Palette::PinOutgoingFill));
          painter->drawPath(pinPath);
        }
        break;
      }
      case graph::Defines::LeftToRight:
      {
        QPainterPath pinPath;
        if (pin().direction() == graph::Defines::Incoming)
        {
          painter->setPen(pal.pen(graph::Palette::PinIncomingBorder));
          pinPath.moveTo(rect.left() + rect.width() / 2.0,rect.top());
          pinPath.lineTo(rect.topLeft());
          pinPath.lineTo(rect.bottomLeft());
          pinPath.lineTo(rect.left() + rect.width() / 2.0,rect.bottom());
          pinPath.arcTo(rect,270.0,180.0);
          pinPath.closeSubpath();
          painter->setBrush(pal.brush(graph::Palette::PinIncomingFill));
          painter->drawPath(pinPath);
        }
        else
        {
          painter->setPen(pal.pen(graph::Palette::PinOutgoingBorder));
          pinPath.moveTo(rect.left() + rect.width() / 2.0,rect.top());
          pinPath.lineTo(rect.topRight());
          pinPath.lineTo(rect.bottomRight());
          pinPath.lineTo(rect.left() + rect.width() / 2.0,rect.bottom());
          pinPath.arcTo(rect,270.0,-180.0);
          pinPath.closeSubpath();
          painter->setBrush(pal.brush(graph::Palette::PinOutgoingFill));
          painter->drawPath(pinPath);
        }
        break;
      }
      case graph::Defines::RightToLeft:
      {
        QPainterPath pinPath;
        if (pin().direction() == graph::Defines::Incoming)
        {
          painter->setPen(pal.pen(graph::Palette::PinIncomingBorder));
          pinPath.moveTo(rect.left() + rect.width() / 2.0,rect.top());
          pinPath.lineTo(rect.topRight());
          pinPath.lineTo(rect.bottomRight());
          pinPath.lineTo(rect.left() + rect.width() / 2.0,rect.bottom());
          pinPath.arcTo(rect,270.0,-180.0);
          pinPath.closeSubpath();
          painter->setBrush(pal.brush(graph::Palette::PinIncomingFill));
          painter->drawPath(pinPath);
        }
        else
        {
          painter->setPen(pal.pen(graph::Palette::PinOutgoingBorder));
          pinPath.moveTo(rect.left() + rect.width() / 2.0,rect.top());
          pinPath.lineTo(rect.topLeft());
          pinPath.lineTo(rect.bottomLeft());
          pinPath.lineTo(rect.left() + rect.width() / 2.0,rect.bottom());
          pinPath.arcTo(rect,270.0,180.0);
          pinPath.closeSubpath();
          painter->setBrush(pal.brush(graph::Palette::PinOutgoingFill));
          painter->drawPath(pinPath);
        }
        break;
      }
    }
  }

  QRectF MacroPinItem::boundingRect() const
  {
    graph::VertexItem* vertexItem = static_cast<graph::VertexItem*>(parentItem());
    Q_ASSERT(vertexItem != 0);
    switch(vertexItem->itemLayout())
    {
      case graph::Defines::TopToBottom:
        return QRectF(-3.0, -5.0, 6.0, 10.0);
      case graph::Defines::LeftToRight:
      case graph::Defines::RightToLeft:
        return QRectF(-5.0, -3.0, 10.0, 6.0);
      default:
        return QRectF(-3.0, -5.0, 6.0, 10.0);
    }
  }

  void MacroPinItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
  {
    /*
    QAction* macroWatch = Resource::action(Resource::MACRO_WATCHOUTPUT);
    if (this->pin().direction() == graph::Defines::Outgoing)
    {
      macroWatch->setData(reinterpret_cast<qulonglong>(&(this->pin())));
      macroWatch->trigger();
      event->accept();
    }
    else
    {
      event->ignore();
    }
    */
    event->ignore();
  }

  void MacroPinItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
  {
    MacroItem* macroItem = dynamic_cast<MacroItem*>(this->parentItem());
    Q_ASSERT(macroItem != 0);
    bool selected = macroItem->isSelected();
    event->accept();
    if (!selected)
    {
      macroItem->setSelected(true);
    }
    QMenu popup;
    QAction* edtCut = Resource::action(Resource::EDIT_CUT);
    QAction* edtCopy = Resource::action(Resource::EDIT_COPY);
    QAction* edtDel = Resource::action(Resource::EDIT_DELETE);
    QAction* macroWatch = Resource::action(Resource::MACRO_WATCHOUTPUT);
    if (this->pin().direction() == graph::Defines::Outgoing)
    {
      macroWatch->setData(reinterpret_cast<qulonglong>(&(this->pin())));
      popup.addAction(macroWatch);
      popup.addSeparator();
    }
    if ((macroItem->vertex().isInCycle() && !macroItem->vertex().topologicalOrderForced() &&
         macroItem->vertex().topologicalOrder() < 0) || macroItem->vertex().topologicalOrderForced())
    {
      QAction* edtAnchor = Resource::action(Resource::EDIT_SETANCHOR);
      void* ptrMacro = reinterpret_cast<void*>(&(macroItem->vertex()));
      edtAnchor->setData(QVariant::fromValue(ptrMacro));
      edtAnchor->setChecked(macroItem->vertex().topologicalOrderForced());
      popup.addAction(edtAnchor);
      popup.addSeparator();
    }
    popup.addAction(edtCut);
    popup.addAction(edtCopy);
    popup.addAction(edtDel);
    popup.exec(event->screenPos(),popup.actions().first());
    if (!selected)
    {
      macroItem->setSelected(false);
    }
  }

  //-----------------------------------------------------------------------
  // Class MacroLinkItem
  //-----------------------------------------------------------------------
  void MacroLinkItem::setupProperties(WndProperties& propWnd) const
  {
    QtVariantPropertyManager& propManager = propWnd.infoPropertyManager();
    QtVariantProperty* item;
    QtVariantProperty* group;
    group = propManager.addProperty(QtVariantPropertyManager::groupTypeId(), QObject::tr("General"));
    item = propManager.addProperty(QVariant::String, QObject::tr("Link type"));
    item->setValue(edge().srcPin().data()->dataRef().staticCast<app::MacroPin>()->getType());
    group->addSubProperty(item);
    group = propManager.addProperty(QtVariantPropertyManager::groupTypeId(), QObject::tr("Source"));
    item = propManager.addProperty(QVariant::String, QObject::tr("Macro"));
    item->setValue(edge().srcPin().data()->vertex().dataRef().staticCast<app::Macro>()->getName());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Output pin"));
    item->setValue(edge().srcPin().data()->dataRef().staticCast<app::MacroPin>()->getName());
    group->addSubProperty(item);
    group = propManager.addProperty(QtVariantPropertyManager::groupTypeId(), QObject::tr("Destination"));
    item = propManager.addProperty(QVariant::String, QObject::tr("Macro"));
    item->setValue(edge().destPin().data()->vertex().dataRef().staticCast<app::Macro>()->getName());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Input pin"));
    item->setValue(edge().destPin().data()->dataRef().staticCast<app::MacroPin>()->getName());
    group->addSubProperty(item);
  }

  void MacroLinkItem::updateProperties(WndProperties& /*propWnd*/) const
  {
  }

  void MacroLinkItem::propertyChanged(QtVariantProperty& /*prop*/)
  {
  }

  void MacroLinkItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
  {
    QAction* macroWatch = Resource::action(Resource::MACRO_WATCHOUTPUT);
    macroWatch->setData(reinterpret_cast<qulonglong>(edge().srcPin().data()));
    macroWatch->trigger();
    event->accept();
  }

  void MacroLinkItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
  {
    bool selected = this->isSelected();
    event->accept();
    if (!selected)
    {
      this->setSelected(true);
    }
    QMenu popup;
    QAction* edtCut = Resource::action(Resource::EDIT_CUT);
    QAction* edtCopy = Resource::action(Resource::EDIT_COPY);
    QAction* edtDel = Resource::action(Resource::EDIT_DELETE);
    QAction* macroWatch = Resource::action(Resource::MACRO_WATCHOUTPUT);
    macroWatch->setData(reinterpret_cast<qulonglong>(edge().srcPin().data()));
    popup.addAction(macroWatch);
    popup.addSeparator();
    if (scene()->selectedItems().count() > 1)
    {
      popup.addAction(edtCut);
      popup.addAction(edtCopy);
    }
    popup.addAction(edtDel);
    popup.exec(event->screenPos(),popup.actions().first());
    if (!selected)
    {
      this->setSelected(false);
    }
  }

  //-----------------------------------------------------------------------
  // Class MacroItem
  //-----------------------------------------------------------------------
  void MacroItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
  {
    app::Macro::Ptr macroInstance = vertex().dataRef().staticCast<app::Macro>();
    Q_ASSERT(!macroInstance.isNull());

    graph::VertexItem::paint(painter,option,widget);
    QRectF rcInner = innerRect();

    // calculate text sizes
    QString macroName = macroInstance->getName();
    QFontMetrics fmName(*Resource::font(Resource::FONT_MACRONAME));
    QRect rectName = fmName.boundingRect(macroName);
    QString macroRuntime = macroInstance->getRuntimeString();
    QFontMetrics fmStatus(*Resource::font(Resource::FONT_MACROSTATUS));
    QRect rectRuntime = fmStatus.boundingRect("000.000 ms");
    QPixmap statusIcon;
    switch(macroInstance->getState())
    {
      case app::Macro::Idle:
        statusIcon = QPixmap(":/icons/resources/bullet_black.png");
        break;
      case app::Macro::Running:
        statusIcon = QPixmap(":/icons/resources/bullet_orange.png");
        break;
      case app::Macro::Ok:
        if (!toolTip().isEmpty()) setToolTip(QString());
        statusIcon = QPixmap(":/icons/resources/bullet_green.png");
        break;
      case app::Macro::Failure:
        setToolTip(macroInstance->getErrorMsg());
        statusIcon = QPixmap(":/icons/resources/bullet_red.png");
        break;
    }
    QString macroOrder = QString(QObject::tr("Order No.: "));
    if (vertex().topologicalOrder() >= 0)
    {
      macroOrder += QString("%1").arg(vertex().topologicalOrder());
    }
    else
    {
      macroOrder += '-';
    }
    QPixmap orderStatus;
    if (vertex().isInCycle() && vertex().topologicalOrder() == 0 && vertex().topologicalOrderForced())
    {
      orderStatus = QPixmap(":/icons/resources/anchor.png");
    }
    else if (vertex().isInCycle() && vertex().topologicalOrder() < 0 && !vertex().topologicalOrderForced())
    {
      orderStatus = QPixmap(":/icons/resources/cycle.png");
    }
    else if (!vertex().isInCycle() && vertex().topologicalOrder() < 0)
    {
      orderStatus = QPixmap(":/icons/resources/error.png");
    }
    QRect rectOrder = fmStatus.boundingRect(macroOrder);
    rectOrder.adjust(0,0,orderStatus.width(),16 - rectOrder.height());

    // paint macro name
    QRectF rcTextName(rcInner.left(),rcInner.top(),rcInner.width(),(qreal)rectName.height());
    painter->setPen(QPen(Qt::black));
    painter->setFont(*Resource::font(Resource::FONT_MACRONAME));
    painter->drawText(rcTextName,Qt::AlignCenter,macroName);
    // paint runtime including indicator
    QRectF rcTextRuntime(rcInner.left() + rcInner.width() * 0.02 + statusIcon.width(),rcInner.top() + rectName.height() + 1.0,rcInner.width() - 2 * rcInner.width() * 0.02 - statusIcon.width(),(qreal)rectRuntime.height());
    painter->setPen(QPen(Qt::blue));
    painter->setFont(*Resource::font(Resource::FONT_MACROSTATUS));
    painter->setClipRect(rcTextRuntime);
    painter->drawText(rcTextRuntime,Qt::AlignRight,macroRuntime);
    painter->setClipRect(rcInner);
    painter->drawPixmap(rcInner.left() + rcInner.width() * 0.02,rcTextRuntime.top(),statusIcon.rect().width(),statusIcon.rect().height(),statusIcon);
    // paint topological order
    QRectF rcTextOrder(rcInner.left() + orderStatus.rect().width() + (rcInner.width() - rectOrder.width()) / 2.0,rcInner.bottom() - (qreal)rectOrder.height(),rcInner.width(),(qreal)rectOrder.height());
    painter->setPen(QPen(Qt::black));
    painter->drawText(rcTextOrder,Qt::AlignLeft,macroOrder);
    if (!orderStatus.isNull())
    {
      painter->drawPixmap(rcTextOrder.left() - orderStatus.rect().width() - 1.0,rcTextOrder.top(),orderStatus.rect().width(),orderStatus.rect().height(),orderStatus);
    }
    painter->setClipRect(itemRect());
  }

  void MacroItem::setupProperties(WndProperties& propWnd) const
  {
    QtVariantPropertyManager& propManager = propWnd.infoPropertyManager();
    QtVariantProperty* item;
    QtVariantProperty* group;
    app::Macro::Ptr macro = vertex().dataRef().staticCast<app::Macro>();
    group = propManager.addProperty(QtVariantPropertyManager::groupTypeId(), QObject::tr("General"));
    item = propManager.addProperty(QVariant::String, QObject::tr("Macro"));
    item->setValue(macro->getName());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Instance UUID"));
    item->setValue(vertex().id().toString());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Creator"));
    item->setValue(macro->getCreator());
    group->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Group"));
    item->setValue(macro->getGroup());
    group->addSubProperty(item);
    QtVariantProperty* libItem = propManager.addProperty(QVariant::String, QObject::tr("Library"));
    libItem->setValue(macro->getLibrary().getName());
    group->addSubProperty(libItem);
    item = propManager.addProperty(QVariant::String, QObject::tr("Libray file"));
    item->setValue(macro->getLibrary().getPath());
    libItem->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Version"));
    item->setValue(macro->getLibrary().getVersionString());
    libItem->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Build"));
    item->setValue(macro->getLibrary().getBuildType());
    libItem->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Build date"));
    item->setValue(macro->getLibrary().getBuildDate());
    libItem->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Qt version"));
    item->setValue(macro->getLibrary().getQtVersionString());
    libItem->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("Creator"));
    item->setValue(macro->getLibrary().getCreator());
    libItem->addSubProperty(item);
    item = propManager.addProperty(QVariant::String, QObject::tr("API Version"));
    item->setValue(macro->getLibrary().getAPIVersionString());
    libItem->addSubProperty(item);
    // setup QML
    propWnd.setQMLProperties(macro.toWeakRef());
  }

  void MacroItem::updateProperties(WndProperties& /*propWnd*/) const
  {
  }

  void MacroItem::propertyChanged(QtVariantProperty& /*prop*/)
  {
  }

  void MacroItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
  {
    bool selected = this->isSelected();
    event->accept();
    if (!selected)
    {
      this->setSelected(true);
    }
    QMenu popup;
    QAction* edtCut = Resource::action(Resource::EDIT_CUT);
    QAction* edtCopy = Resource::action(Resource::EDIT_COPY);
    QAction* edtDel = Resource::action(Resource::EDIT_DELETE);
    if ((vertex().isInCycle() && !vertex().topologicalOrderForced() && vertex().topologicalOrder() < 0) || vertex().topologicalOrderForced())
    {
      QAction* edtAnchor = Resource::action(Resource::EDIT_SETANCHOR);
      void* ptrMacro = reinterpret_cast<void*>(&vertex());
      edtAnchor->setData(QVariant::fromValue(ptrMacro));
      edtAnchor->setChecked(vertex().topologicalOrderForced());
      popup.addAction(edtAnchor);
      popup.addSeparator();
    }
    popup.addAction(edtCut);
    popup.addAction(edtCopy);
    popup.addAction(edtDel);
    popup.exec(event->screenPos(),popup.actions().first());
    if (!selected)
    {
      this->setSelected(false);
    }
  }
}
