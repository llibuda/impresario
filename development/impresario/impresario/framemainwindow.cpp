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

#include "appimpresario.h"
#include "appmacromanager.h"
#include "framemainwindow.h"
#include "framestatusbar.h"
#include "framemenubar.h"
#include "frametoolbars.h"
#include "framedockwindows.h"
#include "configdlgsettings.h"
#include "aboutdlgabout.h"
#include "dbwndmacros.h"
#include "sysloglogger.h"
#include "resources.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QDir>

namespace frame
{
  MainWindow* MainWindow::wndInstance = 0;

  MainWindow& MainWindow::instance()
  {
    if (!wndInstance)
    {
      wndInstance = new MainWindow();
    }
    return *wndInstance;
  }

  void MainWindow::release()
  {
    if (wndInstance)
    {
      delete wndInstance;
      wndInstance = 0;
    }
  }

  MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), mdiArea(0), undoGroup(0), currentState(None)
  {
    setWindowIcon(QIcon(":/icons/resources/impresario.png"));
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks);
    setCorner(Qt::TopLeftCorner,Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner,Qt::LeftDockWidgetArea);

    // initialize the status bar
    StatusBar* statBar = new StatusBar(this);
    setStatusBar(statBar);
    statusBar()->showMessage(tr("Welcome"));

    // create undo group and corresponding actions
    undoGroup = new QUndoGroup(this);
    QAction* undoAction = undoGroup->createUndoAction(this);
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setIcon(QIcon(":/icons/resources/undo.png"));
    undoAction->setStatusTip(QObject::tr("Undo the last edit action"));
    QAction* redoAction = undoGroup->createRedoAction(this);
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setIcon(QIcon(":/icons/resources/redo.png"));
    redoAction->setStatusTip(QObject::tr("Redo the last edit action"));
    Resource::addAction(Resource::EDIT_UNDO,undoAction);
    Resource::addAction(Resource::EDIT_REDO,redoAction);

    // set up pge area for the process graphs
    mdiArea = new QMdiArea;
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setViewMode(QMdiArea::TabbedView);
    mdiArea->setTabShape(QTabWidget::Rounded);
    mdiArea->setDocumentMode(true);
    mdiArea->setTabsClosable(true);

    setCentralWidget(mdiArea);
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(mdiUpdateUI()));

    // create macro window dock
    DockWndMacroDb* dockMacroDb = new DockWndMacroDb(this);
    Resource::addAction(Resource::VIEW_DOCK_MACROS,dockMacroDb->toggleViewAction());
    addDockWidget(Qt::LeftDockWidgetArea,dockMacroDb);
    // create syslog dock
    DockWndSysLog* dockSysLog = new DockWndSysLog(this);
    Resource::addAction(Resource::VIEW_DOCK_SYSMSG,dockSysLog->toggleViewAction());
    addDockWidget(Qt::BottomDockWidgetArea,dockSysLog);
    // create undo stack view dock
    DockWndChangeLog* dockUndo = new DockWndChangeLog(undoGroup,this);
    Resource::addAction(Resource::VIEW_DOCK_UNDOSTACK,dockUndo->toggleViewAction());
    addDockWidget(Qt::RightDockWidgetArea,dockUndo);
    // create console output dock
    DockWndConsole* dockConsole = new DockWndConsole(this);
    Resource::addAction(Resource::VIEW_DOCK_CONSOLE,dockConsole->toggleViewAction());
    addDockWidget(Qt::BottomDockWidgetArea,dockConsole);
    // create property browser dock
    DockWndProperties* dockProps = new DockWndProperties(this);
    Resource::addAction(Resource::VIEW_DOCK_PROPS,dockProps->toggleViewAction());
    addDockWidget(Qt::RightDockWidgetArea,dockProps);

    // create tool bars
    ToolBarFile* tbFile = new ToolBarFile(this);
    Resource::addAction(Resource::VIEW_TB_FILE,tbFile->toggleViewAction());
    addToolBar(tbFile);
    ToolBarEdit* tbEdit = new ToolBarEdit(this);
    Resource::addAction(Resource::VIEW_TB_EDIT,tbEdit->toggleViewAction());
    addToolBar(tbEdit);
    ToolBarView* tbView = new ToolBarView(this);
    Resource::addAction(Resource::VIEW_TB_VIEW,tbView->toggleViewAction());
    addToolBar(tbView);
    ToolBarCtrl* tbCtrl = new ToolBarCtrl(this);
    Resource::addAction(Resource::VIEW_TB_CONTROL,tbCtrl->toggleViewAction());
    addToolBar(tbCtrl);

    // create menu bar
    MenuBarImpresario* menuBar = new MenuBarImpresario(this);
    setMenuBar(menuBar);

    // connect fixed actions to be handled in the main window to corresponding slots
    connect(this,SIGNAL(changedState(MainWindow::AppState,MainWindow::AppState)),dockMacroDb,SLOT(changeState(MainWindow::AppState,MainWindow::AppState)));
    connect(this,SIGNAL(changedState(MainWindow::AppState,MainWindow::AppState)),dockSysLog,SLOT(changeState(MainWindow::AppState,MainWindow::AppState)));
    connect(this,SIGNAL(changedState(MainWindow::AppState,MainWindow::AppState)),dockUndo,SLOT(changeState(MainWindow::AppState,MainWindow::AppState)));
    connect(this,SIGNAL(changedState(MainWindow::AppState,MainWindow::AppState)),dockProps,SLOT(changeState(MainWindow::AppState,MainWindow::AppState)));
    connect(this,SIGNAL(changedState(MainWindow::AppState,MainWindow::AppState)),dockConsole,SLOT(changeState(MainWindow::AppState,MainWindow::AppState)));
    connect(this,SIGNAL(changedState(MainWindow::AppState,MainWindow::AppState)),tbFile,SLOT(changeState(MainWindow::AppState,MainWindow::AppState)));
    connect(this,SIGNAL(changedState(MainWindow::AppState,MainWindow::AppState)),tbEdit,SLOT(changeState(MainWindow::AppState,MainWindow::AppState)));
    connect(this,SIGNAL(changedState(MainWindow::AppState,MainWindow::AppState)),tbView,SLOT(changeState(MainWindow::AppState,MainWindow::AppState)));
    connect(this,SIGNAL(changedState(MainWindow::AppState,MainWindow::AppState)),tbCtrl,SLOT(changeState(MainWindow::AppState,MainWindow::AppState)));
    connect(this,SIGNAL(changedState(MainWindow::AppState,MainWindow::AppState)),menuBar,SLOT(changeState(MainWindow::AppState,MainWindow::AppState)));
    connect(Resource::action(Resource::FILE_NEW), SIGNAL(triggered()), this, SLOT(fileNew()));
    connect(Resource::action(Resource::FILE_LOAD), SIGNAL(triggered()), this, SLOT(fileOpen()));
    connect(Resource::action(Resource::FILE_CLOSE), SIGNAL(triggered()), mdiArea, SLOT(closeActiveSubWindow()));
    connect(Resource::action(Resource::FILE_CLOSEALL), SIGNAL(triggered()), mdiArea, SLOT(closeAllSubWindows()));
    connect(Resource::action(Resource::FILE_QUIT), SIGNAL(triggered()), this, SLOT(close()));
    connect(Resource::action(Resource::EXTRAS_SETTINGS), SIGNAL(triggered()), this, SLOT(extrasSettings()));
    connect(Resource::action(Resource::HELP_CONTENT), SIGNAL(triggered()), &app::Impresario::instance().helpEngine(), SLOT(showHelpContents()));
    connect(Resource::action(Resource::HELP_IDX), SIGNAL(triggered()), &app::Impresario::instance().helpEngine(), SLOT(showHelpIndex()));
    connect(Resource::action(Resource::HELP_ABOUT), SIGNAL(triggered()), this, SLOT(helpAbout()));
    connect(this,SIGNAL(closeHelpSystem()),&app::Impresario::instance().helpEngine(), SLOT(closeHelp()));
    connect(Resource::action(Resource::MACRO_FINDINSTANCE),SIGNAL(triggered()),this,SLOT(mdiNavigateMacro()));

    connect(&app::MacroManager::instance(),&app::MacroManager::loadPrototypesStarted,statBar,&StatusBar::showProgressBar,Qt::QueuedConnection);
    connect(&app::MacroManager::instance(),&app::MacroManager::loadPrototypesFinished,statBar,&StatusBar::hideProgressBar,Qt::QueuedConnection);
    connect(&app::MacroManager::instance(),&app::MacroManager::loadPrototypesProgress,statBar,&StatusBar::updateProgress,Qt::QueuedConnection);
    QAction* fileLoadAction = Resource::action(Resource::FILE_LOAD);
    connect(&app::MacroManager::instance(), &app::MacroManager::loadPrototypesStarted, this, [=] () { fileLoadAction->setEnabled(false); },Qt::QueuedConnection);
    connect(&app::MacroManager::instance(), &app::MacroManager::loadPrototypesFinished, this, [=] () { fileLoadAction->setEnabled(true); },Qt::QueuedConnection);

    // connect variable actions to be handled in the different MDI windows
    db::WndMacros* wndMacros = findChild<db::WndMacros*>("WndMacros");
    connect(Resource::action(Resource::MACRO_CREATEINSTANCE_1), SIGNAL(triggered()),&mapper,SLOT(map()));
    connect(Resource::action(Resource::MACRO_CREATEINSTANCE_2), SIGNAL(triggered()),&mapper,SLOT(map()));
    connect(Resource::action(Resource::MACRO_CREATEINSTANCE_3), SIGNAL(triggered()),&mapper,SLOT(map()));
    connect(Resource::action(Resource::MACRO_CREATEINSTANCE_MULT), SIGNAL(triggered()),&mapper,SLOT(map()));
    mapper.setMapping(Resource::action(Resource::MACRO_CREATEINSTANCE_1),1);
    mapper.setMapping(Resource::action(Resource::MACRO_CREATEINSTANCE_2),2);
    mapper.setMapping(Resource::action(Resource::MACRO_CREATEINSTANCE_3),3);
    mapper.setMapping(Resource::action(Resource::MACRO_CREATEINSTANCE_MULT),0);
    multiplexer.connect(&mapper,SIGNAL(mapped(int)),SLOT(editAddMacro(int)));
    multiplexer.connect(SIGNAL(updateMacroCommands(bool)),Resource::action(Resource::MACRO_CREATEINSTANCE_1),SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateMacroCommands(bool)),Resource::action(Resource::MACRO_CREATEINSTANCE_2),SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateMacroCommands(bool)),Resource::action(Resource::MACRO_CREATEINSTANCE_3),SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateMacroCommands(bool)),Resource::action(Resource::MACRO_CREATEINSTANCE_MULT),SLOT(setEnabled(bool)));
    multiplexer.connect(wndMacros,SIGNAL(addMacroInstance(int, const QString&)),SLOT(editAddMacro(int, const QString&)));
    multiplexer.connect(Resource::action(Resource::FILE_SAVE), SIGNAL(triggered()), SLOT(fileSave()));
    multiplexer.connect(Resource::action(Resource::FILE_SAVEAS), SIGNAL(triggered()), SLOT(fileSaveAs()));
    multiplexer.connect(Resource::action(Resource::EDIT_COPY), SIGNAL(triggered()), SLOT(editCopy()));
    multiplexer.connect(Resource::action(Resource::EDIT_CUT), SIGNAL(triggered()), SLOT(editCut()));
    multiplexer.connect(Resource::action(Resource::EDIT_PASTE), SIGNAL(triggered()), SLOT(editPaste()));
    multiplexer.connect(Resource::action(Resource::EDIT_DELETE), SIGNAL(triggered()), SLOT(editDelete()));
    multiplexer.connect(Resource::action(Resource::EDIT_SELECTALL), SIGNAL(triggered()), SLOT(editSelectAll()));
    multiplexer.connect(Resource::action(Resource::EDIT_SETANCHOR), SIGNAL(triggered()), SLOT(editSetAnchor()));
    multiplexer.connect(Resource::action(Resource::MACRO_WATCHOUTPUT), SIGNAL(triggered()), SLOT(macroWatchOutput()));
    multiplexer.connect(SIGNAL(updateEditCommands(bool)),Resource::action(Resource::EDIT_CUT),SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateCopyCommand(bool)),Resource::action(Resource::EDIT_COPY),SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updatePasteCommand(bool)),Resource::action(Resource::EDIT_PASTE),SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateEditCommands(bool)),Resource::action(Resource::EDIT_DELETE),SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateMacroCommands(bool)),Resource::action(Resource::EDIT_SETANCHOR),SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateUndoCommand(bool)),Resource::action(Resource::EDIT_UNDO),SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateRedoCommand(bool)),Resource::action(Resource::EDIT_REDO),SLOT(setEnabled(bool)));
    // Commands from view tool bar
    multiplexer.connect(Resource::action(Resource::VIEW_ZOOM_IN), SIGNAL(triggered()), SLOT(viewZoomIn()));
    multiplexer.connect(Resource::action(Resource::VIEW_ZOOM_OUT), SIGNAL(triggered()), SLOT(viewZoomOut()));
    multiplexer.connect(Resource::action(Resource::VIEW_ZOOM_SEL), SIGNAL(triggered(bool)), SLOT(viewZoomSelection(bool)));
    multiplexer.connect(SIGNAL(updateZoomInCommand(bool)),Resource::action(Resource::VIEW_ZOOM_IN), SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateZoomOutCommand(bool)),Resource::action(Resource::VIEW_ZOOM_OUT), SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateZoomSelCommand(bool)),Resource::action(Resource::VIEW_ZOOM_SEL), SLOT(setChecked(bool)));
    multiplexer.connect(Resource::action(Resource::VIEW_ZOOM_100), SIGNAL(triggered()), SLOT(viewZoom100()));
    multiplexer.connect(Resource::action(Resource::VIEW_ZOOM_PAGE), SIGNAL(triggered()), SLOT(viewZoomPage()));
    multiplexer.connect(tbView->zoomBox(),SIGNAL(valueChanged(int)),SLOT(viewZoomChange(int)));
    multiplexer.connect(SIGNAL(zoomFactorChanged(int)),tbView->zoomBox(),SLOT(setValue(int)));
    // Commands from control tool bar
    multiplexer.connect(Resource::action(Resource::CTRL_START), SIGNAL(triggered()), SLOT(ctrlStart()));
    multiplexer.connect(Resource::action(Resource::CTRL_PAUSE), SIGNAL(triggered()), SLOT(ctrlPause()));
    multiplexer.connect(Resource::action(Resource::CTRL_STOP), SIGNAL(triggered()), SLOT(ctrlStop()));
    multiplexer.connect(Resource::action(Resource::CTRL_SNAP), SIGNAL(triggered()), SLOT(ctrlSnap()));
    multiplexer.connect(SIGNAL(updateStartCommand(bool)),Resource::action(Resource::CTRL_START), SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updatePauseCommand(bool)),Resource::action(Resource::CTRL_PAUSE), SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateStopCommand(bool)),Resource::action(Resource::CTRL_STOP), SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateSnapCommand(bool)),Resource::action(Resource::CTRL_SNAP), SLOT(setEnabled(bool)));
    multiplexer.connect(SIGNAL(updateCheckStartCommand(bool)),Resource::action(Resource::CTRL_START), SLOT(setChecked(bool)));
    multiplexer.connect(SIGNAL(updateCheckPauseCommand(bool)),Resource::action(Resource::CTRL_PAUSE), SLOT(setChecked(bool)));
    multiplexer.connect(SIGNAL(updateCheckStopCommand(bool)),Resource::action(Resource::CTRL_STOP), SLOT(setChecked(bool)));
    multiplexer.connect(SIGNAL(updateCheckSnapCommand(bool)),Resource::action(Resource::CTRL_SNAP), SLOT(setChecked(bool)));
    // Command for updating property window
    multiplexer.connect(SIGNAL(updatePropWnd(pge::PropUpdateInterface*,bool)),static_cast<pge::WndProperties*>(dockProps->widget()),SLOT(updateProps(pge::PropUpdateInterface*,bool)));

    // restore last state and geometry if possible
    QSettings settings;
    restoreGeometry(QByteArray::fromBase64(settings.value(Resource::path(Resource::SETTINGS_GUI_WINDOWGEOMETRY)).toByteArray()));
    restoreState(QByteArray::fromBase64(settings.value(Resource::path(Resource::SETTINGS_GUI_WINDOWSTATE)).toByteArray()));

    // call mdiUpdateUI to initialize menubar and toolbars depending on application state
    mdiUpdateUI();
  }

  MainWindow::~MainWindow()
  {
    delete undoGroup;
    undoGroup = 0;
  }

  void MainWindow::fileNew()
  {
    pge::ProcessGraphEditor *child = mdiChildCreate();
    child->show();
  }

  void MainWindow::fileOpen()
  {
    QString pgPath = Resource::getPath(Resource::SETTINGS_PATH_PROCESSGRAPH);
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open ProcessGraph"),
                                                    pgPath,
                                                    tr("Impresario Process Graphs (*.ipg);; All files (*.*)"));
    if (!fileName.isEmpty())
    {
      QMdiSubWindow* existing = mdiChildFind(fileName);
      if (existing)
      {
        mdiArea->setActiveSubWindow(existing);
        return;
      }
      pge::ProcessGraphEditor* child = mdiChildCreate(fileName);
      if (child != 0)
      {
        child->show();
        statusBar()->showMessage(QString(tr("Loaded Process Graph from %1.")).arg(QDir::toNativeSeparators(fileName)), 2000);
      }
    }
  }

  void MainWindow::extrasSettings()
  {
    config::DlgSettings dlgConfig(this);
    dlgConfig.exec();
  }

  void MainWindow::helpAbout()
  {
    about::DlgAbout dlgAbout(this);
    dlgAbout.exec();
  }

  void MainWindow::closeEvent(QCloseEvent *event)
  {
    mdiArea->closeAllSubWindows();
    if (mdiArea->currentSubWindow())
    {
      event->ignore();
    }
    else
    {
      QSettings settings;
      settings.setValue(Resource::path(Resource::SETTINGS_GUI_WINDOWGEOMETRY), saveGeometry().toBase64());
      settings.setValue(Resource::path(Resource::SETTINGS_GUI_WINDOWSTATE), saveState().toBase64());
      emit closeHelpSystem();
      event->accept();
    }
  }

  pge::ProcessGraphEditor* MainWindow::mdiChildCreate(const QString& fileName)
  {
    // create a new pge child
    pge::ProcessGraphEditor *child = new pge::ProcessGraphEditor;
    if (fileName.isEmpty() || (!fileName.isEmpty() && child->load(fileName)))
    {
      undoGroup->addStack(child->undoStack());
      // add it to pge area
      QMdiSubWindow* subWnd = mdiArea->addSubWindow(child);
      subWnd->setSystemMenu(0);
      multiplexer.setCurrentObject(child);
      return child;
    }
    return 0;
  }

  void MainWindow::mdiChildDestroy(pge::ProcessGraphEditor* child)
  {
    undoGroup->removeStack(child->undoStack());
  }

  void MainWindow::mdiUpdateUI()
  {
    QWidget* mdiChild = mdiGetActive();
    bool hasChild = (mdiChild != 0);
    AppState oldState = currentState;
    if (hasChild)
    {
      pge::ProcessGraphEditor* editor = static_cast<pge::ProcessGraphEditor*>(mdiChild);
      currentState = ProcessGraphEdit;
      multiplexer.setCurrentObject(editor);
      if (!editor->graph().editLockActive())
      {
        undoGroup->setActiveStack(editor->undoStack());
      }
      else
      {
        undoGroup->setActiveStack(0);
      }
    }
    else
    {
      currentState = Standard;
      multiplexer.setCurrentObject(0);
      undoGroup->setActiveStack(0);
    }
    emit changedState(oldState,currentState);
  }

  void MainWindow::mdiNavigateMacro()
  {
    QAction* navAction = Resource::action(Resource::MACRO_FINDINSTANCE);
    graph::Vertex* macroInstance = reinterpret_cast<graph::Vertex*>(navAction->data().toULongLong());
    if (!macroInstance)
    {
      return;
    }
    // find process graph view where macro is located in
    if (macroInstance->graph().isNull())
    {
      syslog::error(QString(tr("Impresario: Macro instance with UUID '%1' is not assigned to any graph.")).arg(macroInstance->id().toString()));
      return;
    }
    pge::ProcessGraphEditor* pgEdit = frame::MainWindow::instance().findChild<pge::ProcessGraphEditor*>(macroInstance->graph()->id());
    // activate found process graph view
    QMdiSubWindow* mdiSubWnd = static_cast<QMdiSubWindow*>(pgEdit->parent());
    mdiArea->setActiveSubWindow(mdiSubWnd);
    // if macro is not marked for deletion find macro instance in scene and give it focus
    if (!macroInstance->safeToDelete())
    {
      pgEdit->scene()->clearSelection();
      graph::BaseItem* macroItem = macroInstance->sceneItem().data();
      macroItem->setSelected(true);
      macroItem->setFocus();
      pgEdit->ensureVisible(macroItem);
    }
  }

  QWidget* MainWindow::mdiGetActive()
  {
    if (QMdiSubWindow *activeSubWindow = mdiArea->currentSubWindow())
    {
      return activeSubWindow->widget();
    }
    return 0;
  }

  QMdiSubWindow* MainWindow::mdiChildFind(const QString &fileName)
  {
    QString canonicalFilePath = QDir::toNativeSeparators(QFileInfo(fileName).canonicalFilePath());
    foreach(QMdiSubWindow* window, mdiArea->subWindowList())
    {
      pge::ProcessGraphEditor* mdiChild = qobject_cast<pge::ProcessGraphEditor*>(window->widget());
      if (mdiChild->fileName() == canonicalFilePath)
      {
        return window;
      }
    }
    return 0;
  }
}
