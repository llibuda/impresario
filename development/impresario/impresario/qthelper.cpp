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

#include "qthelper.h"
#include <QPainter>
#include <QTransform>
#include <QDebug>

//-----------------------------------------------------------------------
// Additional operators
//-----------------------------------------------------------------------
bool operator==(const QStringList& l1, const QStringList& l2)
{
  if (l1.size() != l2.size())
  {
    return false;
  }
  for(int i = 0; i < l1.size(); ++i)
  {
    if (l1.at(i) != l2.at(i))
    {
      return false;
    }
  }
  return true;
}

bool operator!=(const QStringList& l1, const QStringList& l2)
{
  return !(l1 == l2);
}

//-----------------------------------------------------------------------
// Class Splitter
//-----------------------------------------------------------------------
Splitter::Splitter(Qt::Orientation orientation, QWidget* parent) : QSplitter(orientation,parent)
{

}

QSplitterHandle *Splitter::createHandle()
{
  return new SplitterHandle(orientation(), this);
}

//-----------------------------------------------------------------------
// Class SplitterHandle
//-----------------------------------------------------------------------
SplitterHandle::SplitterHandle(Qt::Orientation orientation, QSplitter *parent) : QSplitterHandle(orientation,parent),
                                                                                 areaCloseOpen(),
                                                                                 imgArrowUp(":/icons/resources/splitterarrowup.png"),
                                                                                 imgArrowDown(":/icons/resources/splitterarrowdown.png"),
                                                                                 imgArrowLeft(imgArrowDown.transformed(QTransform().rotate(90.0))),
                                                                                 imgArrowRight(imgArrowUp.transformed(QTransform().rotate(90.0))),
                                                                                 hover(false), pressed(false), splitterIndex(-1),
                                                                                 widgetIndex(-1), lastSize(-1)
{
  setAttribute(Qt::WA_Hover,true);
}

void SplitterHandle::mouseMoveEvent(QMouseEvent* event)
{
  if (hover)
  {
    if (areaCloseOpen.contains(event->pos()))
    {
      setCursor(Qt::PointingHandCursor);
    }
    else if (rect().contains(event->pos()))
    {
      setCursor((orientation() == Qt::Vertical) ? Qt::SplitVCursor : Qt::SplitHCursor);
    }
  }
  if (!pressed)
  {
    QSplitterHandle::mouseMoveEvent(event);
  }
}

void SplitterHandle::mousePressEvent(QMouseEvent* event)
{
  if (areaCloseOpen.contains(event->pos()))
  {
    pressed = true;
  }
  else
  {
    QSplitterHandle::mousePressEvent(event);
  }
}

void SplitterHandle::mouseReleaseEvent(QMouseEvent* event)
{
  if (pressed)
  {
    if (areaCloseOpen.contains(event->pos()))
    {
      int min;
      int max;
      splitter()->getRange(splitterIndex,&min,&max);
      if (splitter()->sizes()[widgetIndex] > 0)
      {
        lastSize = splitter()->sizes()[widgetIndex];
        moveSplitter((widgetIndex < splitterIndex) ? min : max);
      }
      else
      {
        if (lastSize <= 0)
        {
          lastSize = (max - min) / 2;
        }
        moveSplitter((widgetIndex < splitterIndex) ? (min + lastSize) : (max - lastSize));
      }
    }
    pressed = false;
  }
  else
  {
    QSplitterHandle::mouseReleaseEvent(event);
  }
}

bool SplitterHandle::event(QEvent *event)
{
  switch(event->type())
  {
  case QEvent::HoverEnter:
    hover = true;
    setMouseTracking(true);
    break;
  case QEvent::HoverLeave:
    hover = false;
    setMouseTracking(false);
    break;
  default:
    break;
  }
  return QSplitterHandle::event(event);
}

void SplitterHandle::paintEvent(QPaintEvent *event)
{
  QSplitterHandle::paintEvent(event);
  QPainter painter(this);
  int currentSize = splitter()->sizes()[widgetIndex];
  if (orientation() == Qt::Horizontal)
  {
    if (widgetIndex < splitterIndex)
    {
      painter.drawImage(areaCloseOpen,(currentSize > 0) ? imgArrowLeft : imgArrowRight);
    }
    else
    {
      painter.drawImage(areaCloseOpen,(currentSize > 0) ? imgArrowRight : imgArrowLeft);
    }
  }
  else
  {
    if (widgetIndex < splitterIndex)
    {
      painter.drawImage(areaCloseOpen,(currentSize > 0) ? imgArrowUp : imgArrowDown);
    }
    else
    {
      painter.drawImage(areaCloseOpen,(currentSize > 0) ? imgArrowDown : imgArrowUp);
    }
  }
}

void SplitterHandle::resizeEvent(QResizeEvent* event)
{
  QSplitterHandle::resizeEvent(event);
  // get indexes of splitter handle and widget to operate on
  determineIndexes();
  // calculate area for direct close
  if (orientation() == Qt::Vertical)
  {
    areaCloseOpen.setLeft(rect().left() + (rect().width() - imgArrowUp.width()) / 2);
    areaCloseOpen.setRight(areaCloseOpen.left() + imgArrowUp.width());
    areaCloseOpen.setTop(rect().top());
    areaCloseOpen.setBottom(rect().bottom());
  }
  else
  {
    areaCloseOpen.setTop(rect().top() + (rect().height() - imgArrowLeft.height()) / 2);
    areaCloseOpen.setBottom(areaCloseOpen.top() + imgArrowLeft.height());
    areaCloseOpen.setLeft(rect().left());
    areaCloseOpen.setRight(rect().right());
  }
}

void SplitterHandle::determineIndexes()
{
  for(int i = 0; i < splitter()->count(); ++i)
  {
    if (splitter()->handle(i) == this)
    {
      splitterIndex = i;
      break;
    }
  }
  Q_ASSERT(splitterIndex > 0);
  if (splitter()->isCollapsible(splitterIndex - 1))
  {
    widgetIndex = splitterIndex - 1;
  }
  else
  {
    widgetIndex = splitterIndex;
  }
}

//-----------------------------------------------------------------------
// Class SignalMultiplexer
//-----------------------------------------------------------------------
SignalMultiplexer::SignalMultiplexer(QObject *parent) : QObject(parent)
{
}

void SignalMultiplexer::connect(QObject *sender, const char *signal, const char *slot)
{
  Connection conn;
  conn.sender = sender;
  conn.signal = signal;
  conn.slot = slot;

  connections << conn;
  connect(conn);
}

void SignalMultiplexer::connect(const char *signal, QObject *receiver, const char *slot)
{
  Connection conn;
  conn.receiver = receiver;
  conn.signal = signal;
  conn.slot = slot;

  connections << conn;
  connect(conn);
}

bool SignalMultiplexer::disconnect(QObject *sender, const char *signal, const char *slot)
{
  QList<Connection>::Iterator it = connections.begin();
  for(; it != connections.end(); ++it)
  {
    Connection conn = *it;
    if ((QObject*)conn.sender == sender && qstrcmp(conn.signal, signal) == 0 && qstrcmp(conn.slot, slot) == 0)
    {
      disconnect(conn);
      connections.erase(it);
      return true;
    }
  }
  return false;
}

bool SignalMultiplexer::disconnect(const char *signal, QObject *receiver, const char *slot)
{
  QList<Connection>::Iterator it = connections.begin();
  for(; it != connections.end(); ++it)
  {
    Connection conn = *it;
    if ((QObject*)conn.receiver == receiver && qstrcmp(conn.signal, signal) == 0 && qstrcmp(conn.slot, slot) == 0)
    {
      disconnect(conn);
      connections.erase(it);
      return true;
    }
  }
  return false;
}

void SignalMultiplexer::setCurrentObject(QObject *newObject)
{
  if (newObject == object)
  {
    return;
  }
  QList<Connection>::ConstIterator it;
  for (it = connections.begin(); it != connections.end(); ++it)
  {
    disconnect(*it);
  }
  object = newObject;
  for (it = connections.begin(); it != connections.end(); ++it)
  {
    connect(*it);
  }
  MdiUpdateInterface* document = dynamic_cast<MdiUpdateInterface*>(newObject);
  if (document)
  {
    document->emitAllSignals();
  }
}

void SignalMultiplexer::connect(const Connection &conn)
{
  if (!object)
  {
    return;
  }
  if (!conn.sender && !conn.receiver)
  {
    return;
  }
  if (conn.sender)
  {
    QObject::connect((QObject*)conn.sender, conn.signal, (QObject*)object, conn.slot);
  }
  else
  {
    QObject::connect((QObject*)object, conn.signal, (QObject*)conn.receiver, conn.slot);
  }
}

void SignalMultiplexer::disconnect(const Connection &conn)
{
  if (!object)
  {
    return;
  }
  if (!conn.sender && !conn.receiver)
  {
    return;
  }
  if (conn.sender)
  {
    QObject::disconnect((QObject*)conn.sender, conn.signal, (QObject*)object, conn.slot);
  }
  else
  {
    QObject::disconnect((QObject*)object, conn.signal, (QObject*)conn.receiver, conn.slot);
  }
}
