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

#include "resources.h"
#include <QObject>
#include <QIcon>
#include <QKeySequence>
#include <QBitmap>

Resource Resource::instance;

QMap<Resource::SettingsIDs,QString> Resource::paths;
QMap<Resource::ActionIDs,QPointer<QAction> >* Resource::actions = 0;
QMap<Resource::CursorIDs,QCursor*>* Resource::cursors = 0;
QMap<Resource::FontIDs,QFont*>* Resource::fonts = 0;

Resource::Resource()
{
  actions = new QMap<Resource::ActionIDs,QPointer<QAction> >;
  cursors = new QMap<Resource::CursorIDs,QCursor*>;
  fonts = new QMap<Resource::FontIDs,QFont*>;
}

Resource::~Resource()
{
  // delete action resources
  for(QMap<Resource::ActionIDs,QPointer<QAction> >::iterator it = actions->begin(); it != actions->end(); ++it)
  {
    delete it.value();
  }
  actions->clear();
  delete actions;
  actions = 0;
  // delete cursor resources
  for(QMap<Resource::CursorIDs,QCursor*>::iterator it = cursors->begin(); it != cursors->end(); ++it)
  {
    delete it.value();
  }
  cursors->clear();
  delete cursors;
  cursors = 0;
  // delete font resources
  for(QMap<Resource::FontIDs,QFont*>::iterator it = fonts->begin(); it != fonts->end(); ++it)
  {
    delete it.value();
  }
  fonts->clear();
  delete fonts;
  fonts = 0;
}

void Resource::initPaths()
{
  if (paths.count() > 0)
  {
    return;
  }
  paths[SETTINGS_PATH_PROCESSGRAPH] = "/Data/Paths/ProcessGraphs";
  paths[SETTINGS_PATH_MACROS] = "/Data/Paths/Macros";
  paths[SETTINGS_PATH_DEPLIBS] = "/Data/Paths/DepLibs";
  paths[SETTINGS_PATH_RESOURCES] = "/Data/Paths/Resources";
  paths[SETTINGS_DB_VIEWFORMATS] = "/GUI/DB/ViewFormats";
  paths[SETTINGS_DB_VIEWFILTERS] = "/GUI/DB/ViewFilters";
  paths[SETTINGS_DB_AUTORESETSEARCH] = "/GUI/DB/AutoResetSearch";
  paths[SETTINGS_DB_SHOWVIEWERS] = "/GUI/DB/ShowViewers";
  paths[SETTINGS_DB_DEFAULTPROPS_MACRO] = "/GUI/DB/DefaultProps/Macro";
  paths[SETTINGS_DB_DEFAULTPROPS_VIEWER] = "/GUI/DB/DefaultProps/Viewer";
  paths[SETTINGS_DB_DEFAULTPROPS_LIB] = "/GUI/DB/DefaultProps/Library";
  paths[SETTINGS_DB_DEFAULTPROPS_OTHERS] = "/GUI/DB/DefaultProps/Others";
  paths[SETTINGS_GUI_WINDOWSTATE] = "/GUI/MainWindow/State";
  paths[SETTINGS_GUI_WINDOWGEOMETRY] = "/GUI/MainWindow/Geometry";
  paths[SETTINGS_GUI_TOOLBAR_VISIBILITY] = "/GUI/MainWindow/ToolBarVisibility";
  paths[SETTINGS_GUI_DOCKWND_VISIBILITY] = "/GUI/MainWindow/DockWndVisibility";
  paths[SETTINGS_GUI_MACROWND_SPLITTER] = "/GUI/MainWindow/MacroWndSplitter";
  paths[SETTINGS_GUI_PROPWND_SPLITTER] = "/GUI/MainWindow/PropertyWndSplitter";
  paths[SETTINGS_PROP_DEFAULTWIDGET] = "/GUI/PropertyWindow/DefaultWidget";
  paths[SETTINGS_PROP_DEFAULTHELP_MACRO] = "/GUI/PropertyWindow/DefaultHelp/Macro";
  paths[SETTINGS_PROP_DEFAULTHELP_OTHERS] = "/GUI/PropertyWindow/DefaultHelp/Others";
}

void Resource::initActions()
{
  if (actions->count() > 0)
  {
    return;
  }
  QAction* action = new QAction(QIcon(":/icons/resources/new.png"), QObject::tr("&New"), 0);
  action->setShortcuts(QKeySequence::New);
  action->setStatusTip(QObject::tr("Create a new process graph"));
  (*actions)[FILE_NEW] = action;
  action = new QAction(QIcon(":/icons/resources/open.png"), QObject::tr("&Open..."), 0);
  action->setShortcuts(QKeySequence::Open);
  action->setStatusTip(QObject::tr("Open an existing process graph"));
  (*actions)[FILE_LOAD] = action;
  action = new QAction(QIcon(":/icons/resources/save.png"), QObject::tr("&Save"), 0);
  action->setShortcuts(QKeySequence::Save);
  action->setStatusTip(QObject::tr("Save the current process graph"));
  (*actions)[FILE_SAVE] = action;
  action = new QAction(QObject::tr("Save &As..."), 0);
  action->setStatusTip(QObject::tr("Save the current process graph with a different file name"));
  (*actions)[FILE_SAVEAS] = action;
  action = new QAction(QObject::tr("&Close"), 0);
  action->setStatusTip(QObject::tr("Close the current process graph"));
  (*actions)[FILE_CLOSE] = action;
  action = new QAction(QObject::tr("C&lose All"), 0);
  action->setStatusTip(QObject::tr("Close all open process graphs"));
  (*actions)[FILE_CLOSEALL] = action;
  action = new QAction(QIcon(":/icons/resources/quit.png"), QObject::tr("E&xit"), 0);
  action->setShortcuts(QKeySequence::Quit);
  action->setStatusTip(QObject::tr("Quit Impresario"));
  (*actions)[FILE_QUIT] = action;

  action = new QAction(QIcon(":/icons/resources/cut.png"), QObject::tr("Cu&t"), 0);
  action->setShortcuts(QKeySequence::Cut);
  action->setStatusTip(QObject::tr("Cut selection from current process graph to clipboard"));
  (*actions)[EDIT_CUT] = action;
  action = new QAction(QIcon(":/icons/resources/copy.png"), QObject::tr("&Copy"), 0);
  action->setShortcuts(QKeySequence::Copy);
  action->setStatusTip(QObject::tr("Copy selection from current process graph to clipboard"));
  (*actions)[EDIT_COPY] = action;
  action = new QAction(QIcon(":/icons/resources/paste.png"), QObject::tr("&Paste"), 0);
  action->setShortcuts(QKeySequence::Paste);
  action->setStatusTip(QObject::tr("Paste selection in clipboard into current process graph"));
  (*actions)[EDIT_PASTE] = action;
  action = new QAction(QObject::tr("&Delete"), 0);
  action->setShortcuts(QKeySequence::Delete);
  action->setStatusTip(QObject::tr("Delete selection from current process graph"));
  (*actions)[EDIT_DELETE] = action;
  action = new QAction(QObject::tr("Select &all"), 0);
  action->setShortcuts(QKeySequence::SelectAll);
  action->setStatusTip(QObject::tr("Select all items in current process graph"));
  (*actions)[EDIT_SELECTALL] = action;
  action = new QAction(QIcon(":/icons/resources/anchor.png"),QObject::tr("&Toggle cycle anchor"), 0);
  action->setCheckable(true);
  action->setStatusTip(QObject::tr("Set/Reset a closed cycle anchor on this macro"));
  (*actions)[EDIT_SETANCHOR] = action;

  action = new QAction(QIcon(":/icons/resources/zoom_in.png"),QObject::tr("Zoom &In"), 0);
  action->setShortcuts(QKeySequence::ZoomIn);
  action->setStatusTip(QObject::tr("Zoom into current process graph"));
  (*actions)[VIEW_ZOOM_IN] = action;
  action = new QAction(QIcon(":/icons/resources/zoom_out.png"),QObject::tr("Zoom &Out"), 0);
  action->setShortcuts(QKeySequence::ZoomOut);
  action->setStatusTip(QObject::tr("Zoom out of current process graph"));
  (*actions)[VIEW_ZOOM_OUT] = action;
  action = new QAction(QIcon(":/icons/resources/zoom_selection.png"),QObject::tr("Fit to &Selection"), 0);
  action->setStatusTip(QObject::tr("Fit selection of current process graph into current view"));
  action->setCheckable(true);
  (*actions)[VIEW_ZOOM_SEL] = action;
  action = new QAction(QIcon(":/icons/resources/zoom_page.png"),QObject::tr("Fit to &Page"), 0);
  action->setStatusTip(QObject::tr("Fit complete process graph into current view"));
  (*actions)[VIEW_ZOOM_PAGE] = action;
  action = new QAction(QIcon(":/icons/resources/zoom_100.png"),QObject::tr("&Zoom 100%"), 0);
  action->setStatusTip(QObject::tr("Zoom to 100%"));
  (*actions)[VIEW_ZOOM_100] = action;

  action = new QAction(QIcon(":/icons/resources/control_play.png"), QObject::tr("&Start"), 0);
  action->setCheckable(true);
  action->setShortcut(QKeySequence("F1"));
  action->setStatusTip(QObject::tr("Start processing the current graph"));
  (*actions)[CTRL_START] = action;
  action = new QAction(QIcon(":/icons/resources/control_pause.png"), QObject::tr("&Pause"), 0);
  action->setCheckable(true);
  action->setShortcut(QKeySequence("F2"));
  action->setStatusTip(QObject::tr("Pause processing the current graph"));
  (*actions)[CTRL_PAUSE] = action;
  action = new QAction(QIcon(":/icons/resources/control_stop.png"), QObject::tr("S&top"), 0);
  action->setCheckable(true);
  action->setShortcut(QKeySequence("F3"));
  action->setStatusTip(QObject::tr("Stop processing the current graph"));
  (*actions)[CTRL_STOP] = action;
  action = new QAction(QIcon(":/icons/resources/control_end.png"), QObject::tr("S&nap"), 0);
  action->setCheckable(true);
  action->setShortcut(QKeySequence("F4"));
  action->setStatusTip(QObject::tr("Process the current graph for one cycle"));
  (*actions)[CTRL_SNAP] = action;

  action = new QAction(QIcon(":/icons/resources/settings.png"), QObject::tr("&Settings..."), 0);
  action->setStatusTip(QObject::tr("Edit Impresario's settings"));
  (*actions)[EXTRAS_SETTINGS] = action;

  action = new QAction(QObject::tr("&About Impresario..."), 0);
  action->setStatusTip(QObject::tr("View information about Impresario"));
  (*actions)[HELP_ABOUT] = action;

  action = new QAction(QIcon(":/icons/resources/error.png"), QObject::tr("&Errors"), 0);
  action->setCheckable(true);
  action->setChecked(true);
  action->setIconText(QObject::tr("0 Errors"));
  action->setStatusTip(QObject::tr("Toggle error message filter"));
  (*actions)[SYSLOG_FILTERERRORS] = action;
  action = new QAction(QIcon(":/icons/resources/warning.png"), QObject::tr("&Warnings"), 0);
  action->setCheckable(true);
  action->setChecked(true);
  action->setIconText(QObject::tr("0 Warnings"));
  action->setStatusTip(QObject::tr("Toggle warning message filter"));
  (*actions)[SYSLOG_FILTERWARNINGS] = action;
  action = new QAction(QIcon(":/icons/resources/information.png"), QObject::tr("&Messages"), 0);
  action->setCheckable(true);
  action->setChecked(true);
  action->setIconText(QObject::tr("0 Messages"));
  action->setStatusTip(QObject::tr("Toggle information message filter"));
  (*actions)[SYSLOG_FILTERMESSAGES] = action;
  action = new QAction(QIcon(":/icons/resources/delete.png"), QObject::tr("&Clear"), 0);
  action->setStatusTip(QObject::tr("Clear system messages"));
  (*actions)[SYSLOG_CLEAR] = action;
  action = new QAction(QIcon(":/icons/resources/save.png"), QObject::tr("&Save..."), 0);
  action->setStatusTip(QObject::tr("Save system messages to file"));
  (*actions)[SYSLOG_SAVE] = action;
  action = new QAction(QIcon(":/icons/resources/delete.png"), QObject::tr("&Clear"), 0);
  action->setStatusTip(QObject::tr("Clear console output"));
  (*actions)[CONSOLE_CLEAR] = action;
  action = new QAction(QIcon(":/icons/resources/save.png"), QObject::tr("&Save..."), 0);
  action->setStatusTip(QObject::tr("Save console output to file"));
  (*actions)[CONSOLE_SAVE] = action;

  action = new QAction(QObject::tr("&Navigate"), 0);
  action->setStatusTip(QObject::tr("Navigate to macro instance"));
  (*actions)[MACRO_FINDINSTANCE] = action;
  action = new QAction(QObject::tr("Create &1 instance"), 0);
  action->setStatusTip(QObject::tr("Create instance of selected macro"));
  (*actions)[MACRO_CREATEINSTANCE_1] = action;
  action = new QAction(QObject::tr("Create &2 instances"), 0);
  action->setStatusTip(QObject::tr("Create two instances of selected macro"));
  (*actions)[MACRO_CREATEINSTANCE_2] = action;
  action = new QAction(QObject::tr("Create &3 instances"), 0);
  action->setStatusTip(QObject::tr("Create three instances of selected macro"));
  (*actions)[MACRO_CREATEINSTANCE_3] = action;
  action = new QAction(QIcon(":/icons/resources/macrocreate.png"),QObject::tr("&Create multiple instances..."), 0);
  action->setStatusTip(QObject::tr("Create multiple instances of selected macro"));
  (*actions)[MACRO_CREATEINSTANCE_MULT] = action;
  action = new QAction(QIcon(":/icons/resources/eye.png"),QObject::tr("&Watch"), 0);
  action->setStatusTip(QObject::tr("Watch data content"));
  (*actions)[MACRO_WATCHOUTPUT] = action;
}

void Resource::initCursors()
{
  if (cursors->count() > 0)
  {
    return;
  }
}

void Resource::initFonts()
{
  if (fonts->count() > 0)
  {
    return;
  }
  QFont* font;
  font = new QFont("Helvetica",8);
  (*fonts)[FONT_MACRONAME] = font;
  font = new QFont("Helvetica",8);
  (*fonts)[FONT_MACROSTATUS] = font;
}
