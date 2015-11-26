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
#ifndef RESOURCES_H
#define RESOURCES_H

#include <QMap>
#include <QPointer>
#include <QString>
#include <QAction>
#include <QCursor>
#include <QFont>

namespace Qt
{
  const int ModelItemTypeRole = UserRole + 1;
}

class Resource
{
public:
  enum SettingsIDs
  {
    SETTINGS_PATH_PROCESSGRAPH,
    SETTINGS_PATH_MACROS,
    SETTINGS_PATH_DEPLIBS,
    SETTINGS_PATH_RESOURCES,
    SETTINGS_DB_VIEWFORMATS,
    SETTINGS_DB_VIEWFILTERS,
    SETTINGS_DB_AUTORESETSEARCH,
    SETTINGS_DB_SHOWVIEWERS,
    SETTINGS_DB_DEFAULTPROPS_MACRO,
    SETTINGS_DB_DEFAULTPROPS_VIEWER,
    SETTINGS_DB_DEFAULTPROPS_LIB,
    SETTINGS_DB_DEFAULTPROPS_OTHERS,
    SETTINGS_GUI_WINDOWSTATE,
    SETTINGS_GUI_WINDOWGEOMETRY,
    SETTINGS_GUI_TOOLBAR_VISIBILITY,
    SETTINGS_GUI_DOCKWND_VISIBILITY,
    SETTINGS_GUI_MACROWND_SPLITTER,
    SETTINGS_GUI_PROPWND_SPLITTER,
    SETTINGS_PROP_DEFAULTWIDGET,
    SETTINGS_PROP_DEFAULTHELP_MACRO,
    SETTINGS_PROP_DEFAULTHELP_OTHERS
  };

  enum ActionIDs
  {
    FILE_NEW,
    FILE_LOAD,
    FILE_SAVE,
    FILE_SAVEAS,
    FILE_CLOSE,
    FILE_CLOSEALL,
    FILE_QUIT,
    EDIT_UNDO,
    EDIT_REDO,
    EDIT_CUT,
    EDIT_COPY,
    EDIT_PASTE,
    EDIT_DELETE,
    EDIT_SELECTALL,
    EDIT_SETANCHOR,
    VIEW_TB_FILE,
    VIEW_TB_EDIT,
    VIEW_TB_VIEW,
    VIEW_TB_CONTROL,
    VIEW_DOCK_MACROS,
    VIEW_DOCK_SYSMSG,
    VIEW_DOCK_UNDOSTACK,
    VIEW_DOCK_CONSOLE,
    VIEW_DOCK_PROPS,
    VIEW_ZOOM_IN,
    VIEW_ZOOM_100,
    VIEW_ZOOM_OUT,
    VIEW_ZOOM_SEL,
    VIEW_ZOOM_PAGE,
    CTRL_START,
    CTRL_PAUSE,
    CTRL_STOP,
    CTRL_SNAP,
    EXTRAS_SETTINGS,
    HELP_ABOUT,
    SYSLOG_FILTERERRORS,
    SYSLOG_FILTERWARNINGS,
    SYSLOG_FILTERMESSAGES,
    SYSLOG_CLEAR,
    SYSLOG_SAVE,
    CONSOLE_CLEAR,
    CONSOLE_SAVE,
    MACRO_FINDINSTANCE,
    MACRO_CREATEINSTANCE_1,
    MACRO_CREATEINSTANCE_2,
    MACRO_CREATEINSTANCE_3,
    MACRO_CREATEINSTANCE_MULT,
    MACRO_WATCHOUTPUT
  };

  enum CursorIDs
  {
  };

  enum FontIDs
  {
    FONT_MACRONAME,
    FONT_MACROSTATUS
  };

  static const QString& path(SettingsIDs id)
  {
    initPaths();
    return paths[id];
  }

  static QAction* action(ActionIDs id)
  {
    initActions();
    return (*actions)[id];
  }

  static QCursor* cursor(CursorIDs id)
  {
    initCursors();
    return (*cursors)[id];
  }

  static QFont* font(FontIDs id)
  {
    initFonts();
    return (*fonts)[id];
  }

  static void addAction(ActionIDs id, QAction* action)
  {
    initActions();
    if (actions->find(id) != actions->end())
    {
      if ((*actions)[id] != 0)
      {
        delete (*actions)[id];
      }
      actions->remove(id);
    }
    actions->insert(id,action);
  }

private:
  Resource();
  Resource(const Resource&) {}
  Resource& operator=(const Resource&) { return *this; }
  virtual ~Resource();

  static void initPaths();
  static void initActions();
  static void initCursors();
  static void initFonts();

  static Resource instance;
  static QMap<SettingsIDs,QString>           paths;
  static QMap<ActionIDs,QPointer<QAction> >* actions;
  static QMap<CursorIDs,QCursor*>*           cursors;
  static QMap<FontIDs,QFont*>*               fonts;
};

#endif // RESOURCES_H
