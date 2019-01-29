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

#include "pgecomponents.h"
#include "pgecommands.h"
#include "framemainwindow.h"
#include "appmacromanager.h"
#include "appimpresario.h"
#include "sysloglogger.h"
#include "resources.h"
#include <QByteArray>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QStringList>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QCursor>
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>
#include <QtXmlPatterns/QAbstractMessageHandler>
#include <QSaveFile>

namespace pge
{

  //-----------------------------------------------------------------------
  // Class MessageHandler - Used for schema validation
  //-----------------------------------------------------------------------
  class MessageHandler : public QAbstractMessageHandler
  {
  public:
    MessageHandler() : QAbstractMessageHandler(0)
    {
    }

  protected:
    virtual void handleMessage(QtMsgType type, const QString &description,
                               const QUrl& /*identifier*/, const QSourceLocation& sourceLocation)
    {
      QXmlStreamReader reader(description);
      QString result;
      result.reserve(description.size());

      while(!reader.atEnd())
      {
        reader.readNext();
        switch(reader.tokenType())
        {
          case QXmlStreamReader::StartElement:
          {
            continue;
          }
          case QXmlStreamReader::Characters:
          {
            result.append(reader.text().toString());
            continue;
          }
          case QXmlStreamReader::EndElement:
          {
            continue;
          }
          case QXmlStreamReader::StartDocument:
          case QXmlStreamReader::EndDocument:
            continue;
          default:
            Q_ASSERT_X(false, Q_FUNC_INFO, "Unexpected node.");
        }
      }
      QString fileName = sourceLocation.uri().toString().split('/').last();
      switch(type)
      {
        case QtWarningMsg:
          syslog::warning(QString(tr("%1: In '%2' at line %3, column %4:\n%5"))
                          .arg(fileName).arg(sourceLocation.uri().toString()).arg(sourceLocation.line()).arg(sourceLocation.column()).arg(result),tr("Process Graph"));
          break;
        case QtFatalMsg:
          syslog::error(QString(tr("%1: In '%2' at line %3, column %4:\n%5"))
                          .arg(fileName).arg(sourceLocation.uri().toString()).arg(sourceLocation.line()).arg(sourceLocation.column()).arg(result),tr("Process Graph"));
          break;
        default:
          syslog::info(QString(tr("%1: In '%2' at line %3, column %4:\n%5"))
                          .arg(fileName).arg(sourceLocation.uri().toString()).arg(sourceLocation.line()).arg(sourceLocation.column()).arg(result),tr("Process Graph"));
          break;
      }
    }
  };

  //-----------------------------------------------------------------------
  // Class ProcessGraphEditor
  //-----------------------------------------------------------------------
  ProcessGraphEditor::ProcessGraphEditor(QWidget* parent) : graph::SceneEditor(processGraph,app::MacroManager::instance(),parent),
    pgControl(processGraph), pgThread(), pgRunnable(false), pgRunning(false), pgPaused(false), pgSnapped(false), pgUnlockId(),
    docFileName(), editUndoStack(), dropPos(-1.0,-1.0), viewers()
  {
    setFileName(QString());
  }

  ProcessGraphEditor::~ProcessGraphEditor()
  {
  }

  bool ProcessGraphEditor::load(const QString &fileName)
  {
    QString pgFileName = fileName.split('/').last();
    // get path to schema file
    QString schemaPath = Resource::getPath(Resource::SETTINGS_PATH_RESOURCES);
    schemaPath += "/processgraph.xsd";
    QFile schemaFile(schemaPath);
    if (!schemaFile.exists())
    {
      syslog::error(QString(tr("%1: Used XML schema file '%2' does not exist. Cannot validate process graph files.")).arg(pgFileName).arg(QDir::toNativeSeparators(schemaPath)),tr("Configuration"));
      return false;
    }
    // validate source file
    schemaFile.open(QIODevice::ReadOnly);
    MessageHandler msgHandler;
    QXmlSchema schema;
    schema.setMessageHandler(&msgHandler);
    schema.load(&schemaFile, QUrl::fromLocalFile(schemaFile.fileName()));
    schemaFile.close();
    if (!schema.isValid())
    {
      syslog::error(QString(tr("%1: Used XML schema in file '%2' is invalid. Cannot validate process graph files.")).arg(pgFileName).arg(QDir::toNativeSeparators(schemaPath)),tr("Configuration"));
      return false;
    }
    QXmlSchemaValidator validator(schema);
    QFile xmlFile(fileName);
    xmlFile.open(QIODevice::ReadOnly);
    if (!validator.validate(&xmlFile, QUrl::fromLocalFile(xmlFile.fileName())))
    {
      xmlFile.close();
      syslog::error(QString(tr("%1: File '%2' does not contain a valid process graph.")).arg(pgFileName).arg(QDir::toNativeSeparators(fileName)),tr("Process Graph"));
      return false;
    }
    // now load data from file
    syslog::info(QString(tr("%1: Reading process graph from '%2'...")).arg(pgFileName).arg(QDir::toNativeSeparators(fileName)),tr("Process Graph"));
    xmlFile.seek(0);
    QXmlStreamReader stream(&xmlFile);
    if (!graph::SceneEditor::load(stream))
    {
      xmlFile.close();
      syslog::error(pgFileName + ": " + stream.errorString(),tr("Process Graph"));
      syslog::error(QString(tr("%1: File '%2' not loaded.")).arg(pgFileName).arg(QDir::toNativeSeparators(fileName)),tr("Process Graph"));
      return false;
    }
    if (stream.hasError())
    {
      syslog::warning(pgFileName + ": " + stream.errorString(),tr("Process Graph"));
    }
    xmlFile.close();
    setFileName(fileName);
    syslog::info(QString(tr("%1: Opened process graph.")).arg(processGraph.name()),tr("Process Graph"));
    return true;
  }

  void ProcessGraphEditor::emitAllSignals()
  {
    sceneSelectionChanged();
    emit updateZoomInCommand(zoomFactorCurrent() < zoomFactorMax());
    emit updateZoomOutCommand(zoomFactorCurrent() > zoomFactorMin());
    emit zoomFactorChanged((int)(zoomFactorCurrent() * 100));
    emit updateZoomSelCommand(zoomCursorActive());
    emit updateCheckStartCommand(pgRunning && !pgSnapped);
    emit updateCheckPauseCommand(pgPaused);
    emit updateCheckStopCommand(false);
    emit updateCheckSnapCommand(pgRunning && pgSnapped);
    emit updateStartCommand(!pgRunning && pgRunnable);
    emit updatePauseCommand(pgRunning);
    emit updateStopCommand(pgRunning);
    emit updateSnapCommand(!pgRunning && pgRunnable);
    emit updateMacroCommands(!processGraph.editLockActive());
    emit updateEditCommands(!processGraph.editLockActive() && scene() && !scene()->selectedItems().isEmpty());
    emit updateCopyCommand(scene() && !scene()->selectedItems().isEmpty());
    emit updateUndoCommand(!processGraph.editLockActive() && undoStack()->canUndo());
    emit updateRedoCommand(!processGraph.editLockActive() && undoStack()->canRedo());
    emit updatePasteCommand(!processGraph.editLockActive() && QApplication::clipboard()->mimeData() && QApplication::clipboard()->mimeData()->hasFormat("SceneEditor/xml"));
    if (scene() != 0) emit updatePropWnd(this,true);
  }

  void ProcessGraphEditor::setupProperties(WndProperties& propWnd) const
  {
    QtVariantPropertyManager& infoManager = propWnd.infoPropertyManager();
    QtVariantProperty* item;
    QtVariantProperty* group;
    group = infoManager.addProperty(QtVariantPropertyManager::groupTypeId(), QObject::tr("General"));
    item = infoManager.addProperty(QVariant::String, QObject::tr("Process Graph"));
    QString name = graph().name();
    item->setValue(name);
    group->addSubProperty(item);
    item = infoManager.addProperty(QVariant::String, QObject::tr("Instance UUID"));
    item->setValue(graph().id());
    group->addSubProperty(item);
    group = infoManager.addProperty(QtVariantPropertyManager::groupTypeId(), QObject::tr("Object count"));
    item = infoManager.addProperty(QVariant::Int, QObject::tr("Macros"));
    item->setValue(graph().countVertices());
    group->addSubProperty(item);
    item = infoManager.addProperty(QVariant::Int, QObject::tr("Links"));
    item->setValue(graph().countEdges());
    group->addSubProperty(item);

    QtVariantPropertyManager& propManager = propWnd.stdPropertyManager();
    group = propManager.addProperty(QtVariantPropertyManager::groupTypeId(), QObject::tr("Visualization"));
    item = propManager.addProperty(QtVariantPropertyManager::enumTypeId(), QObject::tr("Layout"));
    QStringList enumLayoutNames;
    enumLayoutNames << QObject::tr("Top to Bottom") << QObject::tr("Left to Right") << QObject::tr("Right to Left");
    item->setAttribute(QLatin1String("enumNames"), enumLayoutNames);
    item->setValue(static_cast<graph::Scene*>(scene())->graphLayout());
    group->addSubProperty(item);
  }

  void ProcessGraphEditor::updateProperties(WndProperties& propWnd) const
  {
    QMap<QString,QtVariantProperty*>& props = propWnd.infoProperties();
    props[QObject::tr("Process Graph")]->setValue(graph().name());
    props[QObject::tr("Instance UUID")]->setValue(graph().id());
    props[QObject::tr("Macros")]->setValue(graph().countVertices());
    props[QObject::tr("Links")]->setValue(graph().countEdges());
  }

  void ProcessGraphEditor::propertyChanged(QtVariantProperty& prop)
  {
    QString name = prop.propertyName();
    if (name == QObject::tr("Layout"))
    {
      static_cast<graph::Scene*>(scene())->setGraphLayout(static_cast<graph::Defines::LayoutDirectionType>(prop.value().toInt()));
      scene()->update();
    }
  }

  bool ProcessGraphEditor::fileSave()
  {
    if (docFileName.isEmpty())
    {
      return fileSaveAs();
    }
    else
    {
      return save(docFileName);
    }
  }

  bool ProcessGraphEditor::fileSaveAs()
  {
    QString pgPath = Resource::getPath(Resource::SETTINGS_PATH_PROCESSGRAPH);
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save ProcessGraph"),
                                                    pgPath,
                                                    tr("Impresario Process Graphs (*.ipg);; All files (*.*)"));
    fileName = QDir::toNativeSeparators(fileName);
    if (fileName.isEmpty())
    {
      return false;
    }
    return save(fileName);
  }

  void ProcessGraphEditor::editCopy()
  {
    BaseItemList list = collectElementItemsForClipboard();
    if (!list.isEmpty())
    {
      copyElementItemsToClipboard(list);
      emit updatePasteCommand(!processGraph.editLockActive() && QApplication::clipboard()->mimeData() && QApplication::clipboard()->mimeData()->hasFormat("SceneEditor/xml"));
    }
    else
    {
      syslog::error(QString(tr("%1: Nothing to copy. Links cannot be copied without connected macros.")).arg(processGraph.name()),tr("Process Graph"));
    }
  }

  void ProcessGraphEditor::editCut()
  {
    BaseItemList list = collectElementItemsForClipboard();
    if (!list.isEmpty())
    {
      copyElementItemsToClipboard(list);
      list = collectElementItemsForDeletion();
      editUndoStack.push(new pge::CmdCutElements(this,list));
    }
    else
    {
      syslog::error(QString(tr("%1: Nothing to cut. Links cannot be cut without connected macros.")).arg(processGraph.name()),tr("Process Graph"));
    }
  }

  void ProcessGraphEditor::editPaste()
  {
    BaseItemList list = pasteElementItemsFromClipboard();
    if (!list.isEmpty())
    {
      editUndoStack.push(new pge::CmdPasteElements(this,list));
    }
    else
    {
      syslog::error(QString(tr("%1: Nothing to paste.")).arg(processGraph.name()),tr("Process Graph"));
    }
  }

  void ProcessGraphEditor::editDelete()
  {
    if (scene()->selectedItems().count() > 0)
    {
      BaseItemList list = collectElementItemsForDeletion();
      editUndoStack.push(new pge::CmdDeleteElements(this,list));
    }
  }

  void ProcessGraphEditor::editSetAnchor()
  {
    void* ptrVertex = Resource::action(Resource::EDIT_SETANCHOR)->data().value<void*>();
    graph::Vertex* vertex = reinterpret_cast<graph::Vertex*>(ptrVertex);
    Q_ASSERT(vertex != 0);
    editUndoStack.push(new pge::CmdForceTopologicalOrder(*vertex,!vertex->topologicalOrderForced()));
  }

  void ProcessGraphEditor::editAddMacro(int countInstances, const QString& typeSignatures)
  {
    // get number of instances to create
    if (countInstances == 0)
    {
      DlgCreateMacroInstance dlg(this);
      dlg.move(QCursor::pos().x() - dlg.size().width() / 2,QCursor::pos().y() - dlg.size().height() / 2);
      countInstances = dlg.exec();
      if (countInstances == 0)
      {
        return;
      }
    }
    // get list of macro type signatures to be created
    QStringList signatureList = typeSignatures.split("\n",QString::SkipEmptyParts);
    if (signatureList.isEmpty())
    {
      signatureList = Resource::action(Resource::MACRO_CREATEINSTANCE_MULT)->data().toString().split("\n",QString::SkipEmptyParts);
    }
    Q_ASSERT(signatureList.count() > 0);
    QPointF insertPos = QPointF(0.0,0.0);
    if (dropPos != QPointF(-1.0,-1.0))
    {
      insertPos = dropPos;
      dropPos = QPointF(-1.0,-1.0);
    }
    // create new macro instances
    BaseItemList list = createVertexItem(countInstances,signatureList,insertPos);
    // create command which actually displays the macros in the scene
    if (!list.empty())
    {
      editUndoStack.push(new pge::CmdAddMacro(this,list));
    }
  }

  void ProcessGraphEditor::ctrlStart()
  {
    pgUnlockId = processGraph.lockEditing();
    if (pgUnlockId.isNull())
    {
      syslog::error(QString(tr("%1: Failed to lock graph execution.")).arg(processGraph.name()),tr("Process Graph"));
      return;
    }
    pgThread.start();
  }

  void ProcessGraphEditor::ctrlPause()
  {
    emit pauseProcessing();
  }

  void ProcessGraphEditor::ctrlStop()
  {
    emit updateCheckStopCommand(true);
    emit stopProcessing();
  }

  void ProcessGraphEditor::ctrlSnap()
  {
    pgSnapped = true;
    emit snapProcessing();
    ctrlStart();
  }

  void ProcessGraphEditor::macroWatchOutput()
  {
    graph::Pin* pinPtr = reinterpret_cast<graph::Pin*>(Resource::action(Resource::MACRO_WATCHOUTPUT)->data().toULongLong());
    if (pinPtr)
    {
      app::MacroOutput::Ptr pinData = pinPtr->dataRef().staticCast<app::MacroOutput>();
      if (!pinData.isNull() && !viewers.contains(pinPtr->id()))
      {
        Viewer* viewer = new Viewer(pinPtr,processGraph.name(),&frame::MainWindow::instance());
        if (viewer && viewer->init())
        {
          connect(viewer,SIGNAL(viewerAboutToClose(QUuid)),this,SLOT(viewerClosed(QUuid)));
          connect(this,SIGNAL(viewersClose()),viewer,SLOT(close()));
          connect(this,SIGNAL(viewersShow()),viewer,SLOT(show()));
          connect(this,SIGNAL(viewersHide()),viewer,SLOT(hide()));
          viewers.insert(pinPtr->id(),viewer);
          viewer->move(QCursor::pos());
          viewer->show();
        }
        else
        {
          delete viewer;
        }
      }
    }
  }

  void ProcessGraphEditor::viewerClosed(QUuid pinId)
  {
    viewers.remove(pinId);
  }

  void ProcessGraphEditor::ctrlStarted()
  {
    pgRunning = true;
    emit updateCheckStartCommand(pgRunning && !pgSnapped);
    emit updateCheckPauseCommand(pgPaused);
    emit updateCheckStopCommand(false);
    emit updateCheckSnapCommand(pgRunning && pgSnapped);
    emit updateStartCommand(!pgRunning);
    emit updatePauseCommand(pgRunning);
    emit updateStopCommand(pgRunning);
    emit updateSnapCommand(!pgRunning);
    undoStack()->setActive(false);
  }

  void ProcessGraphEditor::ctrlPaused(bool pauseOn)
  {
    pgPaused = pauseOn;
    emit updateCheckPauseCommand(pgPaused);
  }

  void ProcessGraphEditor::ctrlStopped()
  {
    if (!processGraph.unlockEditing(pgUnlockId))
    {
      syslog::error(QString(tr("%1: Failed to unlock graph for editing.")).arg(processGraph.name()),tr("Process Graph"));
    }
    undoStack()->setActive(true);
    pgRunning = false;
    pgPaused = false;
    pgSnapped = false;
    emit updateCheckStartCommand(pgRunning && !pgSnapped);
    emit updateCheckPauseCommand(pgPaused);
    emit updateCheckStopCommand(false);
    emit updateCheckSnapCommand(pgRunning && pgSnapped);
    emit updateStartCommand(!pgRunning);
    emit updatePauseCommand(pgRunning);
    emit updateStopCommand(pgRunning);
    emit updateSnapCommand(!pgRunning);
  }

  void ProcessGraphEditor::processGraphModified(bool clean)
  {
    setWindowModified(!clean);
  }

  void ProcessGraphEditor::onGraphModified(int status)
  {
    graph::SceneEditor::onGraphModified(status);
    switch(status)
    {
      case graph::GraphBase::EditingLocked:
        emit updateMacroCommands(false);
        emit updateEditCommands(false);
        emit updatePasteCommand(false);
        emit updateUndoCommand(false);
        emit updateRedoCommand(false);
        break;
      case graph::GraphBase::EditingUnlocked:
        emit updateMacroCommands(true);
        emit updateEditCommands(scene() && !scene()->selectedItems().isEmpty());
        emit updatePasteCommand(QApplication::clipboard()->mimeData() && QApplication::clipboard()->mimeData()->hasFormat("SceneEditor/xml"));
        emit updateUndoCommand(undoStack()->canUndo());
        emit updateRedoCommand(undoStack()->canRedo());
        break;
      case graph::GraphBase::CountVertices:
        pgRunnable = (processGraph.countVertices() > 0 && !processGraph.topologicalOrder().uniqueKeys().contains(-1));
        emit updateStartCommand(!pgRunning && pgRunnable);
        emit updateSnapCommand(!pgRunning && pgRunnable);
        emit updatePropWnd(this);
        break;
      case graph::GraphBase::CountEdges:
        emit updatePropWnd(this);
        break;
      case graph::DirectedGraph::TopologicalOrder:
        pgRunnable = (processGraph.countVertices() > 0 && !processGraph.topologicalOrder().uniqueKeys().contains(-1));
        emit updateStartCommand(!pgRunning && pgRunnable);
        emit updateSnapCommand(!pgRunning && pgRunnable);
        break;
      default:
        break;
    }
  }

  void ProcessGraphEditor::sceneSelectionChanged()
  {
    if (!scene()) return;
    bool enable = scene()->selectedItems().count() > 0;
    if (!processGraph.editLockActive())
    {
      emit updateEditCommands(enable);
    }
    emit updateCopyCommand(enable);
    if (scene()->selectedItems().count() == 1)
    {
      QGraphicsItem* item = scene()->selectedItems().first();
      PropUpdateInterface* propItem = dynamic_cast<PropUpdateInterface*>(item);
      if (propItem)
      {
        emit updatePropWnd(propItem,true);
        return;
      }
    }
    emit updatePropWnd(this,true);
  }

  void ProcessGraphEditor::initialize()
  {
    setObjectName(graph().id());
    setAcceptDrops(true);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(undoStack(),SIGNAL(cleanChanged(bool)),this,SLOT(processGraphModified(bool)));
    connect(this, SIGNAL(subWindowAboutToClose(pge::ProcessGraphEditor*)), &frame::MainWindow::instance(), SLOT(mdiChildDestroy(pge::ProcessGraphEditor*)));
    connect(scene(),SIGNAL(selectionChanged()),this,SLOT(sceneSelectionChanged()));

    pgControl.moveToThread(&pgThread);
    connect(&pgThread,SIGNAL(started()),this,SLOT(ctrlStarted()));
    connect(&pgThread,SIGNAL(finished()),this,SLOT(ctrlStopped()));

    connect(&pgControl,SIGNAL(paused(bool)),this,SLOT(ctrlPaused(bool)));
    connect(this,SIGNAL(pauseProcessing()),&pgControl,SLOT(pause()));
    connect(this,SIGNAL(stopProcessing()),&pgControl,SLOT(stop()));
    connect(this,SIGNAL(snapProcessing()),&pgControl,SLOT(snap()));

    emit updatePropWnd(this,true);
  }

  void ProcessGraphEditor::onVertexTypeLeftClickDrop(const QString& typeSignature, QPointF pos)
  {
    BaseItemList list = createVertexItem(1,QStringList(typeSignature),pos);
    if (!list.empty())
    {
      editUndoStack.push(new pge::CmdAddMacro(this,list));
    }
  }

  void ProcessGraphEditor::onVertexTypeRightClickDrop(const QString& typeSignature, QPointF pos)
  {
    dropPos = pos;
    QMenu popup;
    QAction* createInstance1 = Resource::action(Resource::MACRO_CREATEINSTANCE_1);
    QAction* createInstance2 = Resource::action(Resource::MACRO_CREATEINSTANCE_2);
    QAction* createInstance3 = Resource::action(Resource::MACRO_CREATEINSTANCE_3);
    QAction* createInstanceMult = Resource::action(Resource::MACRO_CREATEINSTANCE_MULT);
    createInstanceMult->setData(typeSignature);
    popup.addAction(createInstance1);
    popup.addAction(createInstance2);
    popup.addAction(createInstance3);
    popup.addSeparator();
    popup.addAction(createInstanceMult);
    popup.exec(QCursor::pos(),createInstance1);
  }

  void ProcessGraphEditor::onEdgeToBeCreated(graph::Pin::Ref srcPin, graph::Pin::Ref destPin)
  {
    BaseItemList edges = createEdgeItem(srcPin,destPin,"Impresario::DataFlowEdge");
    if (!edges.empty())
    {
      undoStack()->push(new CmdAddLink(this,edges));
    }
  }

  void ProcessGraphEditor::onItemMoved(BaseItemList elementItems, QPointF offset)
  {
    undoStack()->push(new CmdMoveElements(this,elementItems,offset));
  }

  void ProcessGraphEditor::onItemResized(graph::BaseItem::Ptr element, QRectF initialSize)
  {
    undoStack()->push(new CmdResizeMacro(element,initialSize));
  }

  void ProcessGraphEditor::contextMenuEvent(QContextMenuEvent *event)
  {
    if (itemAt(event->pos()) == 0)
    {
      event->accept();
      QMenu popup;
      QAction* edtPaste = Resource::action(Resource::EDIT_PASTE);
      QAction* ctrlStop  = Resource::action(Resource::CTRL_STOP);
      QAction* ctrlPause  = Resource::action(Resource::CTRL_PAUSE);
      QAction* ctrlStart = Resource::action(Resource::CTRL_START);
      QAction* ctrlSnap = Resource::action(Resource::CTRL_SNAP);
      if (!pgRunning)
      {
        popup.addAction(edtPaste);
        popup.addSeparator();
        popup.addAction(ctrlStart);
        popup.addAction(ctrlSnap);
      }
      else
      {
        popup.addAction(ctrlPause);
        popup.addAction(ctrlStop);
      }
      popup.exec(event->globalPos(),popup.actions().first());
    }
    else
    {
      QGraphicsView::contextMenuEvent(event);
    }
  }

  void ProcessGraphEditor::closeEvent(QCloseEvent *event)
  {
    if (pgRunning)
    {
      QMessageBox::warning(this, tr("Impresario"),
                           tr("'%1' is still running.\n Please stop the graph before closing it.").arg(processGraph.name()),
                           QMessageBox::Ok);
      event->ignore();
    }
    else if (maybeSave())
    {
      event->accept();
    }
    else
    {
      event->ignore();
    }
    if (event->isAccepted())
    {
      emit viewersClose();
      viewers.clear();
      emit updatePropWnd(0);
      emit subWindowAboutToClose(this);
      disconnect(this, SIGNAL(subWindowAboutToClose(pge::ProcessGraphEditor*)), &frame::MainWindow::instance(), SLOT(mdiChildDestroy(pge::ProcessGraphEditor*)));
    }
  }

  void ProcessGraphEditor::changeEvent(QEvent *event)
  {
    graph::SceneEditor::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange)
    {
      if (windowState() <= Qt::WindowMinimized)
      {
        emit viewersHide();
      }
      else
      {
        emit viewersShow();
      }
    }
  }

  bool ProcessGraphEditor::save(const QString& fileName)
  {
    QSaveFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
      syslog::error(QString(tr("%1: Failed to save graph to file '%2'. %3")).arg(processGraph.name()).arg(fileName).arg(file.errorString()),tr("Process Graph"));
      return false;
    }
    setFileName(fileName);
    QXmlStreamWriter stream(&file);
    graph::SceneEditor::save(stream);
    if (stream.hasError())
    {
      syslog::error(QString(tr("%1: Failed to save graph to file '%2'. %3")).arg(processGraph.name()).arg(fileName).arg(file.errorString()),tr("Process Graph"));
      return false;
    }
    bool isSaved = file.commit();
    if (isSaved)
    {
      //undoStack()->setClean();
      undoStack()->clear();
      clearElements();
      syslog::info(QString(tr("%1: Saved graph to file '%2'.")).arg(processGraph.name()).arg(fileName),tr("Process Graph"));
    }
    else
    {
      syslog::error(QString(tr("%1: Failed to save graph to file '%2'. %3")).arg(processGraph.name()).arg(fileName).arg(file.errorString()),tr("Process Graph"));
    }
    return isSaved;
  }

  bool ProcessGraphEditor::maybeSave()
  {
    if (isWindowModified())
    {
      QString docTitle;
      if (docFileName.isEmpty())
      {
        docTitle = tr("(Untitled)");
      }
      else
      {
        docTitle = QFileInfo(docFileName).fileName();
      }
      QMessageBox::StandardButton ret;
      ret = QMessageBox::warning(this, tr("Impresario"),
                                 tr("'%1' has been modified.\n Do you want to save your changes?").arg(docTitle),
                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
      if (ret == QMessageBox::Save)
      {
        return fileSave();
      }
      else if (ret == QMessageBox::Cancel)
      {
        return false;
      }
    }
    return true;
  }

  void ProcessGraphEditor::setFileName(const QString &fileName)
  {
    QString docTitle = tr("(Untitled)");
    if (!fileName.isEmpty())
    {
      QFileInfo info(fileName);
      docFileName = QDir::toNativeSeparators((info.canonicalFilePath().isEmpty()) ? info.absoluteFilePath() : info.canonicalFilePath());
      docTitle = info.fileName();
    }
    processGraph.setName(docTitle);
    setWindowTitle(docTitle + "[*]");
    setWindowModified(false);
    emit updatePropWnd(this);
  }

  //-----------------------------------------------------------------------
  // Class DlgCreateMacroInstance
  //-----------------------------------------------------------------------
  DlgCreateMacroInstance::DlgCreateMacroInstance(QWidget *parent) : QDialog(parent), spinBox(0)
  {
    setWindowTitle(tr("Create multiple macro instances"));
    setWindowIcon(QIcon(":/icons/resources/macrocreate.png"));
    setWindowModality(Qt::WindowModal);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    spinBox = new QSpinBox();
    spinBox->setRange(1,10);
    QLabel* label = new QLabel(tr("&Number of instances:"));
    label->setBuddy(spinBox);
    QHBoxLayout* layoutCtrl = new QHBoxLayout();
    layoutCtrl->addWidget(label);
    layoutCtrl->addWidget(spinBox,1);
    QVBoxLayout* layoutMain = new QVBoxLayout();
    layoutMain->addLayout(layoutCtrl);
    layoutMain->addWidget(buttonBox);
    setLayout(layoutMain);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(ok()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
  }

  void DlgCreateMacroInstance::ok()
  {
    done(spinBox->value());
  }

  //-----------------------------------------------------------------------
  // Class Viewer
  //-----------------------------------------------------------------------
  Viewer::Viewer(graph::Pin* pinPtr, const QString& pgName, QWidget *parent) : QDockWidget(parent), outputPin(pinPtr), viewerMacro(), stayHidden(false), processGraphName(pgName), lastSize(320,240)
  {
    setAllowedAreas(Qt::NoDockWidgetArea);
    setFloating(true);
    setAttribute(Qt::WA_DeleteOnClose);
  }

  Viewer::~Viewer()
  {
  }

  bool Viewer::init()
  {
    app::MacroOutput::Ptr macroOutput;
    QString pinId;
    if (outputPin != 0)
    {
      macroOutput = outputPin->dataRef().staticCast<app::MacroOutput>();
      pinId = macroOutput->getMacro().getName() + '.' + macroOutput->getName() + ": " + macroOutput->getType();
      setWindowTitle(pinId);
    }
    else
    {
      syslog::error(QString(tr("%1: No valid data reference provided to viewer!")).arg(processGraphName),tr("Process Graph"));
      return false;
    }
    app::MacroManager& manager = app::MacroManager::instance();
    if (!manager.hasMacroViewer(macroOutput->getType()))
    {
      syslog::error(QString(tr("%1: No viewer registered for data type '%2'.")).arg(processGraphName).arg(macroOutput->getType()),tr("Process Graph"));
      return false;
    }
    viewerMacro = manager.createMacroViewerInstance(macroOutput->getType());
    if (viewerMacro.isNull())
    {
      syslog::error(QString(tr("%1: Failed to create viewer for data type '%2'.")).arg(processGraphName).arg(macroOutput->getType()),tr("Process Graph"));
      return false;
    }
    if (!viewerMacro->setData(macroOutput))
    {
      syslog::error(QString(tr("%1: Failed to set input data for viewer of pin '%2'.")).arg(processGraphName).arg(pinId),tr("Process Graph"));
      return false;
    }
    QWidget* viewPort = viewerMacro->createWidget();
    if (viewPort == 0)
    {
      syslog::error(QString(tr("%1: Failed to create widget for viewer of pin '%2'.")).arg(processGraphName).arg(pinId),tr("Process Graph"));
      viewerMacro = app::MacroViewer::Ptr();
      return false;
    }
    app::Macro* macro = outputPin->vertex().dataRef().staticCast<app::Macro>().data();
    if (macro->registerViewer(viewerMacro))
    {
      setWidget(viewPort);
      connect(macro,SIGNAL(updateViewers()),this,SLOT(updateViewer()));
      connect(&(outputPin->vertex()),SIGNAL(statusUpdated(graph::BaseElement&,int)),this,SLOT(vertexStateChanged(graph::BaseElement&,int)));
      connect(&app::MacroManager::instance(),SIGNAL(vertexToBeDeleted(graph::Vertex::Ptr)),this,SLOT(vertexToBeDeleted(graph::Vertex::Ptr)));
      return true;
    }
    else
    {
      QString msg = QString(tr("%1: Error returned in method 'init' of viewer for pin '%2'.")).arg(processGraphName).arg(pinId);
      QString viewerMsg = viewerMacro->getErrorMsg();
      if (!viewerMsg.isEmpty()) msg += '\n' + viewerMsg;
      syslog::error(msg,tr("Process Graph"));
      viewerMacro->destroyWidget();
      viewerMacro = app::MacroViewer::Ptr();
      return false;
    }
  }

  void Viewer::show()
  {
    if (!stayHidden)
    {
      QDockWidget::show();
      resize(lastSize);
    }
  }

  void Viewer::hide()
  {
    lastSize = size();
    QDockWidget::hide();
  }

  void Viewer::closeEvent(QCloseEvent *event)
  {
    stayHidden = true;
    QDockWidget::closeEvent(event);
    if (event->isAccepted())
    {
      emit viewerAboutToClose(outputPin->id());
      if (!viewerMacro.isNull())
      {
        app::Macro* macro = outputPin->vertex().dataRef().staticCast<app::Macro>().data();
        disconnect(macro,SIGNAL(updateViewers()),this,SLOT(updateViewer()));
        if (!macro->unregisterViewer(viewerMacro))
        {
          QString msg = QString(tr("%1: Error returned in method 'exit' of viewer '%2'.")).arg(processGraphName).arg(this->windowTitle());
          QString viewerMsg = viewerMacro->getErrorMsg();
          if (!viewerMsg.isEmpty()) msg += '\n' + viewerMsg;
          syslog::error(msg,tr("Process Graph"));
        }
        disconnect(&(outputPin->vertex()),SIGNAL(statusUpdated(graph::BaseElement&,int)),this,SLOT(vertexStateChanged(graph::BaseElement&,int)));
        disconnect(&app::MacroManager::instance(),SIGNAL(vertexToBeDeleted(graph::Vertex::Ptr)),this,SLOT(vertexToBeDeleted(graph::Vertex::Ptr)));
        viewerMacro->destroyWidget();
      }
    }
    else
    {
      stayHidden = false;
    }
  }

  void Viewer::vertexStateChanged(graph::BaseElement& element, int change)
  {
    if (&element == &(outputPin->vertex()))
    {
      if (change == graph::BaseElement::AddedToGraph)
      {
        app::Macro* macro = outputPin->vertex().dataRef().staticCast<app::Macro>().data();
        connect(macro,SIGNAL(updateViewers()),this,SLOT(updateViewer()));
        stayHidden = false;
        show();
      }
      else if (change == graph::BaseElement::RemovedFromGraph)
      {
        app::Macro* macro = outputPin->vertex().dataRef().staticCast<app::Macro>().data();
        disconnect(macro,SIGNAL(updateViewers()),this,SLOT(updateViewer()));
        stayHidden = true;
        hide();
      }
    }
  }

  void Viewer::vertexToBeDeleted(graph::Vertex::Ptr vertexInstance)
  {
    if (vertexInstance.data() == &(outputPin->vertex()))
    {
      close();
    }
  }

  void Viewer::updateViewer()
  {
    if (!stayHidden) widget()->update();
  }
}
