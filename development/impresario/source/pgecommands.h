/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2016  Lars Libuda
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
#ifndef PGECOMMANDS_H
#define PGECOMMANDS_H

#include "pgecomponents.h"
#include "appmacro.h"
#include <QUndoCommand>
#include <QList>
#include <QMap>
#include <QSet>
#include <QPair>
#include <QGraphicsItem>
#include <QPointF>
#include <QRectF>
#include <QStringList>

namespace pge
{
  class CmdAddMacro : public QUndoCommand
  {
  public:
    CmdAddMacro(ProcessGraphEditor* processGraphEditor, const ProcessGraphEditor::BaseItemList& itemList, QUndoCommand *parent = 0);
    virtual ~CmdAddMacro();

    virtual void undo();
    virtual void redo();

  private:
    ProcessGraphEditor*              editor;
    ProcessGraphEditor::BaseItemList items;
  };

  class CmdDeleteElements : public QUndoCommand
  {
  public:
    CmdDeleteElements(ProcessGraphEditor* processGraphEditor, const ProcessGraphEditor::BaseItemList& itemList, QUndoCommand *parent = 0);
    virtual ~CmdDeleteElements();

    virtual void undo();
    virtual void redo();

  private:
    ProcessGraphEditor*              editor;
    ProcessGraphEditor::BaseItemList items;
  };

  class CmdCutElements : public QUndoCommand
  {
  public:
    CmdCutElements(ProcessGraphEditor* processGraphEditor, const ProcessGraphEditor::BaseItemList& itemList, QUndoCommand *parent = 0);
    virtual ~CmdCutElements();

    virtual void undo();
    virtual void redo();

  private:
    ProcessGraphEditor*              editor;
    ProcessGraphEditor::BaseItemList items;
  };

  class CmdPasteElements : public QUndoCommand
  {
  public:
    CmdPasteElements(ProcessGraphEditor* processGraphEditor, const ProcessGraphEditor::BaseItemList& itemList, QUndoCommand *parent = 0);
    virtual ~CmdPasteElements();

    virtual void undo();
    virtual void redo();

  private:
    ProcessGraphEditor*              editor;
    ProcessGraphEditor::BaseItemList items;
  };

  class CmdMoveElements : public QUndoCommand
  {
  public:
    CmdMoveElements(ProcessGraphEditor* processGraphEditor, const ProcessGraphEditor::BaseItemList& itemList, QPointF offset, QUndoCommand *parent = 0);
    virtual ~CmdMoveElements();

    virtual bool mergeWith(const QUndoCommand *command);
    virtual void undo();
    virtual void redo();

    virtual int id() const
    {
      return moveCmdId;
    }

  private:
    static const int              moveCmdId;
    ProcessGraphEditor*           editor;
    QMap<QGraphicsItem*, QPointF> items;
    QPointF                       moveOffset;
  };

  class CmdResizeMacro : public QUndoCommand
  {
  public:
    CmdResizeMacro(graph::BaseItem::Ptr itemPtr, QRectF size, QUndoCommand *parent = 0);
    virtual ~CmdResizeMacro();

    virtual bool mergeWith(const QUndoCommand *command);
    virtual void undo();
    virtual void redo();

    virtual int id() const
    {
      return resizeCmdId;
    }

  private:
    static const int   resizeCmdId;
    graph::VertexItem* item;
    QRectF             oldSize;
    QRectF             newSize;
  };

  class CmdAddLink : public QUndoCommand
  {
  public:
    CmdAddLink(ProcessGraphEditor* processGraphEditor, const ProcessGraphEditor::BaseItemList& itemList, QUndoCommand *parent = 0);
    virtual ~CmdAddLink();

    virtual void undo();
    virtual void redo();

  private:
    ProcessGraphEditor*              editor;
    ProcessGraphEditor::BaseItemList items;
  };

  class CmdForceTopologicalOrder : public QUndoCommand
  {
  public:
    CmdForceTopologicalOrder(graph::Vertex& graphVertex, bool on, QUndoCommand *parent = 0);
    virtual ~CmdForceTopologicalOrder();

    virtual void undo();
    virtual void redo();

  private:
    graph::Vertex& node;
    bool forceOn;
  };
}
#endif // PGECOMMANDS_H
