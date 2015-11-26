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
#ifndef GRAPHRESOURCES_H
#define GRAPHRESOURCES_H

#include "graphdefines.h"
#include <QObject>
#include <QBrush>
#include <QPen>
#include <QCursor>
#include <QFont>
#include <QMap>
#include <QSharedPointer>

namespace graph
{
  class Palette : public QObject
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<Palette> Ptr;

    static Palette::Ptr graphDefault();
    static void setGraphDefault(Palette::Ptr palette);

    explicit Palette(QObject *parent = 0);

    virtual ~Palette();

    enum BrushID
    {
      PinUndirectionalFill,
      PinIncomingFill,
      PinOutgoingFill,
      PinUndefinedFill,
      VertexBackgroundFill,
      VertexInteriorFill,
      VertexTextBackgroundFill,
      VertexResizeHandleFill,
      UserDefinedBrush
    };

    enum PenID
    {
      PinUndirectionalBorder,
      PinIncomingBorder,
      PinOutgoingBorder,
      PinUndefinedBorder,
      EdgeBorder,
      VertexBorder,
      VertexResizeHandleBorder,
      UserDefinedPen
    };

    const QBrush& brush(int id, Defines::ItemStateType = Defines::Normal) const;

    const QPen& pen(int id, Defines::ItemStateType = Defines::Normal) const;

    void setBrush(const QBrush& brush, int id, Defines::ItemStateType state = Defines::Normal)
    {
      brushes[state][id] = BrushPtr(new QBrush(brush));
    }

    void setPen(const QPen& pen, int id, Defines::ItemStateType state = Defines::Normal)
    {
      pens[state][id] = PenPtr(new QPen(pen));
    }

  private:
    typedef QSharedPointer<QBrush> BrushPtr;
    typedef QSharedPointer<QPen>   PenPtr;
    typedef QSharedPointer<QFont>  FontPtr;

    typedef QMap<Defines::ItemStateType, QMap<int,BrushPtr> > BrushMap;
    typedef QMap<Defines::ItemStateType, QMap<int,PenPtr> >   PenMap;

    static Palette::Ptr defaultPalette;
    static const QBrush InvalidBrush;
    static const QPen   InvalidPen;

    BrushMap brushes;
    PenMap   pens;
  };

  class Cursors : public QObject
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<Cursors> Ptr;

    static Cursors::Ptr graphDefault();
    static void setGraphDefault(Cursors::Ptr cursorCollection);

    explicit Cursors(QObject *parent = 0);

    virtual ~Cursors();

    enum CursorID
    {
      CursorConnectValid,
      CursorConnectInvalid,
      CursorZoom,
      UserDefinedCursor
    };

    const QCursor& cursor(int id) const
    {
      if (cursors.contains(id))
      {
        return *(cursors[id]);
      }
      else
      {
        return InvalidCursor;
      }
    }

    void setCursor(const QCursor& cursor, int id)
    {
      cursors[id] = CursorPtr(new QCursor(cursor));
    }

  private:
    typedef QSharedPointer<QCursor> CursorPtr;
    typedef QMap<int,CursorPtr>     CursorMap;

    static Cursors::Ptr defaultCursors;
    static const QCursor InvalidCursor;

    CursorMap cursors;
  };
}
#endif // GRAPHRESOURCES_H
