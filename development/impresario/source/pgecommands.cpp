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

#include "pgecommands.h"
#include "graphitems.h"
#include <QObject>

namespace pge
{
  //-----------------------------------------------------------------------
  // Class CmdAddMacro
  //-----------------------------------------------------------------------
  CmdAddMacro::CmdAddMacro(ProcessGraphEditor* processGraphEditor, const ProcessGraphEditor::BaseItemList& itemList, QUndoCommand *parent) : QUndoCommand(parent), editor(processGraphEditor), items(itemList)
  {
    Q_ASSERT(items.count() > 0);
    Q_ASSERT(editor != 0);

    if (items.count() == 1)
    {
      app::Macro* macro = static_cast<app::Macro*>(items[0].staticCast<graph::VertexItem>()->vertex().dataRef().data());
      setText(QObject::tr("add 1 instance of macro '%1'").arg(macro->getName()));
    }
    else
    {
      setText(QObject::tr("add group of %1 macro instances").arg(items.count()));
    }
  }

  CmdAddMacro::~CmdAddMacro()
  {
    items.clear();
  }

  void CmdAddMacro::undo()
  {
    editor->removeElementItemsFromGraph(items);
    editor->scene()->update();
  }

  void CmdAddMacro::redo()
  {
    editor->addElementItemsToGraph(items);
    editor->scene()->clearSelection();
    editor->scene()->update();
  }

  //-----------------------------------------------------------------------
  // Class CmdDeleteElements
  //-----------------------------------------------------------------------
  CmdDeleteElements::CmdDeleteElements(ProcessGraphEditor* processGraphEditor, const ProcessGraphEditor::BaseItemList& itemList, QUndoCommand *parent) : QUndoCommand(parent), editor(processGraphEditor), items(itemList)
  {
    Q_ASSERT(items.count() > 0);
    Q_ASSERT(editor != 0);

    app::Macro* macro = 0;
    graph::Edge* link = 0;
    int cntMacros = 0;
    int cntLinks = 0;
    foreach(graph::BaseItem::Ptr item, items)
    {
      switch(item->type())
      {
        case graph::BaseItem::VertexItemType:
          macro = static_cast<app::Macro*>(item.staticCast<graph::VertexItem>()->vertex().dataRef().data());
          ++cntMacros;
          break;
        case graph::BaseItem::EdgeItemType:
          link = &item.staticCast<graph::EdgeItem>()->edge();
          ++cntLinks;
          break;
        default:
          break;
      }
    }

    if (cntMacros == 1)
    {
      if (cntLinks == 0)
      {
        setText(QObject::tr("delete macro '%1'").arg(macro->getName()));
      }
      else
      {
        setText(QObject::tr("delete macro '%1' and links").arg(macro->getName()));
      }
    }
    else if (cntMacros > 1)
    {
      if (cntLinks == 0)
      {
        setText(QObject::tr("delete set of macros"));
      }
      else
      {
        setText(QObject::tr("delete set of macros and links"));
      }
    }
    else
    {
      if (cntLinks == 1)
      {
        setText(QObject::tr("delete link from '%1:%2' to '%3:%4'")
                .arg(static_cast<app::Macro*>(link->srcPin().data()->vertex().dataRef().data())->getName())
                .arg(link->srcPin().data()->dataRef()->id())
                .arg(static_cast<app::Macro*>(link->destPin().data()->vertex().dataRef().data())->getName())
                .arg(link->destPin().data()->dataRef()->id()));
      }
      else
      {
        setText(QObject::tr("delete set of links"));
      }
    }
  }

  CmdDeleteElements::~CmdDeleteElements()
  {
    items.clear();
  }

  void CmdDeleteElements::undo()
  {
    // add back all items to process graph and scene
    editor->addElementItemsToGraph(items);
    // update scene
    editor->scene()->update();
  }

  void CmdDeleteElements::redo()
  {
    // remove all selected links from process graph and scene
    editor->removeElementItemsFromGraph(items,true);
    // update scene
    editor->scene()->update();
  }

  //-----------------------------------------------------------------------
  // Class CmdCutElements
  //-----------------------------------------------------------------------
  CmdCutElements::CmdCutElements(ProcessGraphEditor* processGraphEditor, const ProcessGraphEditor::BaseItemList& itemList, QUndoCommand *parent) : QUndoCommand(parent), editor(processGraphEditor), items(itemList)
  {
    Q_ASSERT(items.count() > 0);
    Q_ASSERT(editor != 0);

    app::Macro* macro = 0;
    graph::Edge* link = 0;
    int cntMacros = 0;
    int cntLinks = 0;
    foreach(graph::BaseItem::Ptr item, items)
    {
      switch(item->type())
      {
        case graph::BaseItem::VertexItemType:
          macro = static_cast<app::Macro*>(item.staticCast<graph::VertexItem>()->vertex().dataRef().data());
          ++cntMacros;
          break;
        case graph::BaseItem::EdgeItemType:
          link = &item.staticCast<graph::EdgeItem>()->edge();
          ++cntLinks;
          break;
        default:
          break;
      }
    }

    if (cntMacros == 1)
    {
      if (cntLinks == 0)
      {
        setText(QObject::tr("cut macro '%1'").arg(macro->getName()));
      }
      else
      {
        setText(QObject::tr("cut macro '%1' (removing loose links)").arg(macro->getName()));
      }
    }
    else if (cntMacros > 1)
    {
      if (cntLinks == 0)
      {
        setText(QObject::tr("cut set of macros"));
      }
      else
      {
        setText(QObject::tr("cut set of macros and links (removing loose links)"));
      }
    }
    else
    {
      if (cntLinks == 1)
      {
        setText(QObject::tr("cut link from '%1:%2' to '%3:%4'")
                .arg(static_cast<app::Macro*>(link->srcPin().data()->vertex().dataRef().data())->getName())
                .arg(link->srcPin().data()->dataRef()->id())
                .arg(static_cast<app::Macro*>(link->destPin().data()->vertex().dataRef().data())->getName())
                .arg(link->destPin().data()->dataRef()->id()));
      }
      else
      {
        setText(QObject::tr("cut set of links"));
      }
    }
  }

  CmdCutElements::~CmdCutElements()
  {
    items.clear();
  }

  void CmdCutElements::undo()
  {
    // add back all items to process graph and scene
    editor->addElementItemsToGraph(items);
    // update scene
    editor->scene()->update();
  }

  void CmdCutElements::redo()
  {
    // remove all selected links from process graph and scene
    editor->removeElementItemsFromGraph(items,true);
    // update scene
    editor->scene()->update();
  }

  //-----------------------------------------------------------------------
  // Class CmdPasteElements
  //-----------------------------------------------------------------------
  CmdPasteElements::CmdPasteElements(ProcessGraphEditor* processGraphEditor, const ProcessGraphEditor::BaseItemList& itemList, QUndoCommand *parent) : QUndoCommand(parent), editor(processGraphEditor), items(itemList)
  {
    Q_ASSERT(items.count() > 0);
    Q_ASSERT(editor != 0);

    app::Macro* macro = 0;
    graph::Edge* link = 0;
    int cntMacros = 0;
    int cntLinks = 0;
    foreach(graph::BaseItem::Ptr item, items)
    {
      switch(item->type())
      {
        case graph::BaseItem::VertexItemType:
          macro = static_cast<app::Macro*>(item.staticCast<graph::VertexItem>()->vertex().dataRef().data());
          ++cntMacros;
          break;
        case graph::BaseItem::EdgeItemType:
          link = &item.staticCast<graph::EdgeItem>()->edge();
          ++cntLinks;
          break;
        default:
          break;
      }
    }

    if (cntMacros == 1)
    {
      if (cntLinks == 0)
      {
        setText(QObject::tr("paste macro '%1'").arg(macro->getName()));
      }
      else
      {
        setText(QObject::tr("paste macro '%1'").arg(macro->getName()));
      }
    }
    else if (cntMacros > 1)
    {
      if (cntLinks == 0)
      {
        setText(QObject::tr("paste set of macros"));
      }
      else
      {
        setText(QObject::tr("paste set of macros and links"));
      }
    }
    else
    {
      if (cntLinks == 1)
      {
        setText(QObject::tr("paste link from '%1:%2' to '%3:%4'")
                .arg(static_cast<app::Macro*>(link->srcPin().data()->vertex().dataRef().data())->getName())
                .arg(link->srcPin().data()->dataRef()->id())
                .arg(static_cast<app::Macro*>(link->destPin().data()->vertex().dataRef().data())->getName())
                .arg(link->destPin().data()->dataRef()->id()));
      }
      else
      {
        setText(QObject::tr("paste set of links"));
      }
    }
  }

  CmdPasteElements::~CmdPasteElements()
  {
    items.clear();
  }

  void CmdPasteElements::undo()
  {
    editor->removeElementItemsFromGraph(items,true);
    // update scene
    editor->scene()->update();
  }

  void CmdPasteElements::redo()
  {
    editor->scene()->clearSelection();
    editor->addElementItemsToGraph(items);
    foreach(graph::BaseItem::Ptr item,items)
    {
      item->setSelected(true);
    }

    // update scene
    editor->scene()->update();
  }


  //-----------------------------------------------------------------------
  // Class CmdMoveElements
  //-----------------------------------------------------------------------
  const int CmdMoveElements::moveCmdId = 1;

  CmdMoveElements::CmdMoveElements(ProcessGraphEditor* processGraphEditor, const ProcessGraphEditor::BaseItemList& itemList, QPointF offset, QUndoCommand *parent) : QUndoCommand(parent), editor(processGraphEditor), items(), moveOffset(offset)
  {
    Q_ASSERT(editor != 0);
    for(ProcessGraphEditor::BaseItemList::const_iterator it = itemList.begin(); it != itemList.end(); ++it)
    {
      items.insert((*it).data(),(*it).data()->pos());
    }
    if (items.count() == 1)
    {
      app::Macro* macro = static_cast<app::Macro*>(static_cast<graph::VertexItem*>(items.begin().key())->vertex().dataRef().data());
      setText(QObject::tr("move macro '%1'").arg(macro->getName()));
    }
    else
    {
      setText(QObject::tr("move macro group"));
    }
  }

  CmdMoveElements::~CmdMoveElements()
  {
  }

  bool CmdMoveElements::mergeWith(const QUndoCommand *command)
  {
    const CmdMoveElements* moveCmd = static_cast<const CmdMoveElements*>(command);
    if (items.count() != moveCmd->items.count())
    {
      return false;
    }
    QMap<QGraphicsItem*,QPointF> itemsUpdate;
    for(QMap<QGraphicsItem*,QPointF>::iterator it = items.begin(); it != items.end(); ++it)
    {
      QMap<QGraphicsItem*,QPointF>::const_iterator newItemIt = moveCmd->items.find(it.key());
      if (newItemIt != moveCmd->items.end())
      {
        itemsUpdate.insert(it.key(),newItemIt.value());
      }
      else
      {
        return false;
      }
    }
    QPointF lastPos = items.begin().value();
    QPointF actPos = moveCmd->items[items.begin().key()];
    QPointF origPos = lastPos - moveOffset;
    moveOffset = actPos - origPos;
    items = itemsUpdate;
    return true;
  }

  void CmdMoveElements::undo()
  {
    for(QMap<QGraphicsItem*,QPointF>::iterator it = items.begin(); it != items.end(); ++it)
    {
      it.key()->setPos(it.value() - moveOffset);
    }
    // update scene
    editor->scene()->update();
  }

  void CmdMoveElements::redo()
  {
    for(QMap<QGraphicsItem*,QPointF>::iterator it = items.begin(); it != items.end(); ++it)
    {
      it.key()->setPos(it.value());
    }
    // update scene
    editor->scene()->update();
  }

  //-----------------------------------------------------------------------
  // Class CmdResizeMacro
  //-----------------------------------------------------------------------
  const int CmdResizeMacro::resizeCmdId = 2;

  CmdResizeMacro::CmdResizeMacro(graph::BaseItem::Ptr itemPtr, QRectF size, QUndoCommand *parent) : QUndoCommand(parent), item(itemPtr.staticCast<graph::VertexItem>().data()), oldSize(size)
  {
    Q_ASSERT(item != 0);
    app::Macro* macro = static_cast<app::Macro*>(item->vertex().dataRef().data());
    setText(QObject::tr("resize macro '%1'").arg(macro->getName()));
    newSize = item->itemRect();
  }

  CmdResizeMacro::~CmdResizeMacro()
  {
  }

  bool CmdResizeMacro::mergeWith(const QUndoCommand *command)
  {
    const CmdResizeMacro* resizeCmd = static_cast<const CmdResizeMacro*>(command);
    if (item != resizeCmd->item)
    {
      return false;
    }
    newSize = resizeCmd->newSize;
    return true;
  }

  void CmdResizeMacro::undo()
  {
    item->setItemRect(oldSize);
  }

  void CmdResizeMacro::redo()
  {
    item->setItemRect(newSize);
  }

  //-----------------------------------------------------------------------
  // Class CmdAddLink
  //-----------------------------------------------------------------------
  CmdAddLink::CmdAddLink(ProcessGraphEditor* processGraphEditor, const ProcessGraphEditor::BaseItemList& itemList, QUndoCommand *parent) : QUndoCommand(parent),
    editor(processGraphEditor), items(itemList)
  {
    Q_ASSERT(processGraphEditor != 0 && items.count() == 1);
    graph::Edge& edge = items[0].staticCast<graph::EdgeItem>()->edge();
    app::Macro* macroSrc = static_cast<app::Macro*>(edge.srcPin().toStrongRef()->vertex().dataRef().data());
    app::Macro* macroDst = static_cast<app::Macro*>(edge.destPin().toStrongRef()->vertex().dataRef().data());
    setText(QString(QObject::tr("add link from '%1:%2' to '%3:%4'"))
            .arg(macroSrc->getName())
            .arg(edge.srcPin().toStrongRef()->dataRef()->id())
            .arg(macroDst->getName())
            .arg(edge.destPin().toStrongRef()->dataRef()->id()));
  }

  CmdAddLink::~CmdAddLink()
  {
    items.clear();
  }

  void CmdAddLink::undo()
  {
    editor->removeElementItemsFromGraph(items);
    editor->scene()->update();
  }

  void CmdAddLink::redo()
  {
    editor->addElementItemsToGraph(items);
    editor->scene()->clearSelection();
    editor->scene()->update();
  }

  //-----------------------------------------------------------------------
  // Class CmdSetLoopAnchor
  //-----------------------------------------------------------------------
  CmdForceTopologicalOrder::CmdForceTopologicalOrder(graph::Vertex& graphVertex, bool on, QUndoCommand *parent) : QUndoCommand(parent),
    node(graphVertex), forceOn(on)
  {
    app::Macro* macro = static_cast<app::Macro*>(node.dataRef().data());
    if (forceOn)
    {
      setText(QString(QObject::tr("Set cycle anchor for macro '%1'")).arg(macro->getName()));
    }
    else
    {
      setText(QString(QObject::tr("Reset cylce anchor for macro '%1'")).arg(macro->getName()));
    }
  }

  CmdForceTopologicalOrder::~CmdForceTopologicalOrder()
  {
  }

  void CmdForceTopologicalOrder::undo()
  {
    node.forceTopologicalOrder(!forceOn);
  }

  void CmdForceTopologicalOrder::redo()
  {
    node.forceTopologicalOrder(forceOn);
  }
}
