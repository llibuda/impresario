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

#include "graphresources.h"
#include <QLinearGradient>
#include <QColor>
#include <QPixmap>
#include <QBitmap>

namespace graph
{
  //-----------------------------------------------------------------------
  // Class Palette
  //-----------------------------------------------------------------------
  Palette::Ptr Palette::defaultPalette = Palette::Ptr();
  const QBrush Palette::InvalidBrush = QBrush();
  const QPen   Palette::InvalidPen = QPen();

  Palette::Ptr Palette::graphDefault()
  {
    if (defaultPalette.isNull())
    {
      defaultPalette = Ptr(new Palette());
    }
    return defaultPalette;
  }

  void Palette::setGraphDefault(Palette::Ptr palette)
  {
    if (!palette.isNull())
    {
      defaultPalette = palette;
    }
  }

  Palette::Palette(QObject *parent) : QObject(parent), brushes(), pens()
  {
    brushes[Defines::Normal][PinUndirectionalFill] = BrushPtr(new QBrush(Qt::blue));
    brushes[Defines::Normal][PinIncomingFill] = BrushPtr(new QBrush(Qt::yellow));
    brushes[Defines::Normal][PinOutgoingFill] = BrushPtr(new QBrush(Qt::red));
    brushes[Defines::Normal][PinUndefinedFill] = BrushPtr(new QBrush(Qt::gray));

    static QLinearGradient background;
    background.setCoordinateMode(QGradient::ObjectBoundingMode);
    background.setColorAt(0,Qt::white);
    background.setColorAt(1,Qt::gray);
    brushes[Defines::Normal][VertexBackgroundFill] = BrushPtr(new QBrush(background));
    brushes[Defines::Normal][VertexInteriorFill] = BrushPtr(new QBrush(QColor(235,235,193)));
    brushes[Defines::Normal][VertexTextBackgroundFill] = BrushPtr(new QBrush(QColor(235,235,173)));

    brushes[Defines::Normal][VertexResizeHandleFill] = BrushPtr(new QBrush(Qt::white));

    pens[Defines::Normal][PinUndirectionalBorder] = PenPtr(new QPen(Qt::gray));
    pens[Defines::Normal][PinIncomingBorder] = pens[Defines::Normal][PinUndirectionalBorder];
    pens[Defines::Normal][PinOutgoingBorder] = pens[Defines::Normal][PinUndirectionalBorder];
    pens[Defines::Normal][PinUndefinedBorder] = pens[Defines::Normal][PinUndirectionalBorder];

    pens[Defines::Normal][EdgeBorder] = PenPtr(new QPen(Qt::black));
    pens[Defines::Valid][EdgeBorder] = PenPtr(new QPen(Qt::green));
    pens[Defines::Invalid][EdgeBorder] = PenPtr(new QPen(Qt::red));
    pens[Defines::Selected][EdgeBorder] = PenPtr(new QPen(Qt::darkBlue));

    pens[Defines::Normal][VertexBorder] = PenPtr(new QPen(Qt::gray));
    pens[Defines::Selected][VertexBorder] = pens[Defines::Selected][EdgeBorder];
    pens[Defines::Focused][VertexBorder] = PenPtr(new QPen(Qt::blue));

    pens[Defines::Normal][VertexResizeHandleBorder] = PenPtr(new QPen(Qt::black));
  }

  Palette::~Palette()
  {
    brushes.clear();
    pens.clear();
  }

  const QBrush& Palette::brush(int id, Defines::ItemStateType state) const
  {
    if (brushes.contains(state) && brushes[state].contains(id))
    {
      return *(brushes.value(state).value(id));
    }
    else if (brushes.contains(Defines::Normal) && brushes[Defines::Normal].contains(id))
    {
      return *(brushes.value(Defines::Normal).value(id));
    }
    else
      return InvalidBrush;
  }

  const QPen& Palette::pen(int id, Defines::ItemStateType state) const
  {
    if (pens.contains(state) && pens[state].contains(id))
    {
      return *(pens.value(state).value(id));
    }
    else if (pens.contains(Defines::Normal) && pens[Defines::Normal].contains(id))
    {
      return *(pens.value(Defines::Normal).value(id));
    }
    else
      return InvalidPen;
  }

  //-----------------------------------------------------------------------
  // Class Cursors
  //-----------------------------------------------------------------------
  #include "resources/cursor_connect1.xpm"
  #include "resources/cursor_connect1_mask.xpm"
  #include "resources/cursor_connect2.xpm"
  #include "resources/cursor_connect2_mask.xpm"
  #include "resources/cursor_zoom.xpm"
  #include "resources/cursor_zoom_mask.xpm"

  Cursors::Ptr Cursors::defaultCursors = Cursors::Ptr();
  const QCursor Cursors::InvalidCursor = QCursor();

  Cursors::Ptr Cursors::graphDefault()
  {
    if (defaultCursors.isNull())
    {
      defaultCursors = Ptr(new Cursors());
    }
    return defaultCursors;
  }

  void Cursors::setGraphDefault(Cursors::Ptr cursorCollection)
  {
    if (!cursorCollection.isNull())
    {
      defaultCursors = cursorCollection;
    }
  }

  Cursors::Cursors(QObject *parent) : QObject(parent), cursors()
  {
    QPixmap cur = QPixmap(cursor_connect1);
    QPixmap mask = QPixmap(cursor_connect1_mask);
    cursors[CursorConnectValid] = CursorPtr(new QCursor(QBitmap(cur),QBitmap(mask),8,8));
    cur = QPixmap(cursor_connect2);
    mask = QPixmap(cursor_connect2_mask);
    cursors[CursorConnectInvalid] = CursorPtr(new QCursor(QBitmap(cur),QBitmap(mask),8,8));
    cur = QPixmap(cursor_zoom);
    mask = QPixmap(cursor_zoom_mask);
    cursors[CursorZoom] = CursorPtr(new QCursor(QBitmap(cur),QBitmap(mask),8,8));
  }

  Cursors::~Cursors()
  {
    cursors.clear();
  }
}
