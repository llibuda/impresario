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
#ifndef PGECOMPONENTS_H
#define PGECOMPONENTS_H

#include "appprocessgraph.h"
#include "grapheditor.h"
#include "pgewndprops.h"
#include "appmacro.h"
#include "qthelper.h"
#include <QUndoStack>
#include <QCloseEvent>
#include <QEvent>
#include <QString>
#include <QDialog>
#include <QSpinBox>
#include <QWidget>
#include <QMap>
#include <QGraphicsItem>
#include <QPointF>
#include <QUuid>
#include <QContextMenuEvent>
#include <QDockWidget>
#include <QThread>

namespace pge
{
  class Viewer;

  class ProcessGraphEditor : public graph::SceneEditor, public MdiUpdateInterface, public PropUpdateInterface
  {
    Q_OBJECT
  public:
    ProcessGraphEditor(QWidget* parent = 0);
    ~ProcessGraphEditor();

    QUndoStack* undoStack()
    {
      return &editUndoStack;
    }

    const QString& fileName() const
    {
      return docFileName;
    }

    bool load(const QString& fileName);

    virtual void emitAllSignals();

    virtual void setupProperties(WndProperties& propWnd) const;
    virtual void updateProperties(WndProperties& propWnd) const;
    virtual void propertyChanged(QtVariantProperty& prop);

  signals:
    void subWindowAboutToClose(pge::ProcessGraphEditor*);
    void updateMacroCommands(bool);
    void updateEditCommands(bool);
    void updateCopyCommand(bool);
    void updatePasteCommand(bool);
    void updateUndoCommand(bool);
    void updateRedoCommand(bool);
    void updatePropWnd(pge::PropUpdateInterface* item, bool createProps = false);
    void updateStartCommand(bool);
    void updatePauseCommand(bool);
    void updateStopCommand(bool);
    void updateSnapCommand(bool);
    void updateCheckStartCommand(bool);
    void updateCheckPauseCommand(bool);
    void updateCheckStopCommand(bool);
    void updateCheckSnapCommand(bool);
    void pauseProcessing();
    void stopProcessing();
    void snapProcessing();
    void viewersHide();
    void viewersShow();
    void viewersClose();

  public slots:
    bool fileSave();
    bool fileSaveAs();
    void editCopy();
    void editCut();
    void editPaste();
    void editDelete();
    void editSetAnchor();
    void editAddMacro(int countInstances, const QString& typeSignatures = "");
    void ctrlStart();
    void ctrlPause();
    void ctrlStop();
    void ctrlSnap();
    void macroWatchOutput();


  private slots:
    void viewerClosed(QUuid pinId);
    void processGraphModified(bool clean);
    void sceneSelectionChanged();
    void ctrlStarted();
    void ctrlPaused(bool pauseOn);
    void ctrlStopped();
    virtual void onGraphModified(int status);

  protected:
    virtual void initialize();
    virtual void onVertexTypeLeftClickDrop(const QString& typeSignature, QPointF pos);
    virtual void onVertexTypeRightClickDrop(const QString& typeSignature, QPointF pos);
    virtual void onEdgeToBeCreated(graph::Pin::Ref srcPin,graph::Pin::Ref destPin);
    virtual void onItemMoved(BaseItemList elementItems, QPointF offset);
    virtual void onItemResized(graph::BaseItem::Ptr element, QRectF initialSize);
    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void closeEvent(QCloseEvent* event);
    virtual void changeEvent(QEvent* event);

  private:
    bool save(const QString& fileName);
    bool maybeSave();
    void setFileName(const QString &fileName);

    typedef QMap<QUuid,Viewer*> ViewerMap;

    app::ProcessGraph     processGraph;
    app::ProcessGraphCtrl pgControl;
    QThread               pgThread;
    bool                  pgRunnable;
    bool                  pgRunning;
    bool                  pgPaused;
    bool                  pgSnapped;
    QUuid                 pgUnlockId;
    QString               docFileName;
    QUndoStack            editUndoStack;
    QPointF               dropPos;
    ViewerMap             viewers;
  };

  class DlgCreateMacroInstance : public QDialog
  {
    Q_OBJECT
  public:
    DlgCreateMacroInstance(QWidget* parent = 0);

  private slots:
    void ok();

  private:
    QSpinBox* spinBox;
  };

  class Viewer : public QDockWidget
  {
    Q_OBJECT
  public:
    Viewer(graph::Pin* pinPtr, const QString& pgName, QWidget* parent = 0);
    virtual ~Viewer();

    bool init();

  public slots:
    void show();
    void hide();

  signals:
    void viewerAboutToClose(QUuid);

  protected:
    virtual void closeEvent(QCloseEvent* event);

  private slots:
    void vertexStateChanged(graph::BaseElement& element, int change);
    void vertexToBeDeleted(graph::Vertex::Ptr vertexInstance);
    void updateViewer();

  private:
    graph::Pin*           outputPin;
    app::MacroViewer::Ptr viewerMacro;
    bool                  stayHidden;
    const QString&        processGraphName;
    QSize                 lastSize;
  };

}
#endif // PGECOMPONENTS_H
