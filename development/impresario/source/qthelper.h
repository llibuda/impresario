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
#ifndef QTHELPER_H
#define QTHELPER_H

#include <QStringList>
#include <QSplitter>
#include <QSplitterHandle>
#include <QEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QImage>
#include <QRect>
#include <QPointer>
#include <QList>

bool operator==(const QStringList& l1, const QStringList& l2);
bool operator!=(const QStringList& l1, const QStringList& l2);

class Splitter : public QSplitter
{
  Q_OBJECT
public:
  Splitter(Qt::Orientation orientation, QWidget *parent = 0);

protected:
  QSplitterHandle *createHandle();
};

class SplitterHandle : public QSplitterHandle
{
  Q_OBJECT
public:
  SplitterHandle(Qt::Orientation orientation, QSplitter* parent = 0);

protected:
  virtual bool event(QEvent* event);
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void paintEvent(QPaintEvent* event);
  virtual void resizeEvent(QResizeEvent* event);

private:
  void determineIndexes();

  QRect  areaCloseOpen;
  QImage imgArrowUp;
  QImage imgArrowDown;
  QImage imgArrowLeft;
  QImage imgArrowRight;
  bool   hover;
  bool   pressed;
  int    splitterIndex;
  int    widgetIndex;
  int    lastSize;
};

struct MdiUpdateInterface
{
  virtual void emitAllSignals() = 0;
};

class SignalMultiplexer : public QObject
{
  Q_OBJECT
public:
  SignalMultiplexer(QObject *parent = 0);

  void connect(QObject *sender, const char *signal, const char *slot);
  bool disconnect(QObject *sender, const char *signal, const char *slot);
  void connect(const char *signal, QObject *receiver, const char *slot);
  bool disconnect(const char *signal, QObject *receiver, const char *slot);

  QObject *currentObject() const
  {
    return object;
  }

public slots:
  void setCurrentObject(QObject *newObject);

private:
  struct Connection
  {
    QPointer<QObject> sender;
    QPointer<QObject> receiver;
    const char *signal;
    const char *slot;
  };

  void connect(const Connection &conn);
  void disconnect(const Connection &conn);

  QPointer<QObject> object;
  QList<Connection> connections;

};

#endif // QTHELPER_H
