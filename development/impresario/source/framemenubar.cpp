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

#include "framemenubar.h"
#include "resources.h"
#include <QMenu>
#include <QMenuBar>

namespace frame
{

  MenuBarImpresario::MenuBarImpresario(QWidget *parent) :  QMenuBar(parent)
  {
    // build file menu
    menuFile = this->addMenu(tr("&File"));
    menuFile->addAction(Resource::action(Resource::FILE_NEW));
    menuFile->addAction(Resource::action(Resource::FILE_LOAD));
    menuFile->addAction(Resource::action(Resource::FILE_SAVE));
    menuFile->addAction(Resource::action(Resource::FILE_SAVEAS));
    menuFile->addSeparator();
    menuFile->addAction(Resource::action(Resource::FILE_CLOSE));
    menuFile->addAction(Resource::action(Resource::FILE_CLOSEALL));
    menuFile->addSeparator();
    menuFile->addAction(Resource::action(Resource::FILE_QUIT));

    // build edit menu
    menuEdit = this->addMenu(tr("&Edit"));
    menuEdit->addAction(Resource::action(Resource::EDIT_UNDO));
    menuEdit->addAction(Resource::action(Resource::EDIT_REDO));
    menuEdit->addSeparator();
    menuEdit->addAction(Resource::action(Resource::EDIT_CUT));
    menuEdit->addAction(Resource::action(Resource::EDIT_COPY));
    menuEdit->addAction(Resource::action(Resource::EDIT_PASTE));
    menuEdit->addAction(Resource::action(Resource::EDIT_DELETE));
    menuEdit->addSeparator();
    menuEdit->addAction(Resource::action(Resource::EDIT_SELECTALL));

    // build view menu
    menuView = this->addMenu(tr("&View"));
    QMenu* tbMenu = menuView->addMenu(tr("&Toolbars"));
    tbMenu->addAction(Resource::action(Resource::VIEW_TB_FILE));
    tbMenu->addAction(Resource::action(Resource::VIEW_TB_EDIT));
    tbMenu->addAction(Resource::action(Resource::VIEW_TB_VIEW));
    tbMenu->addAction(Resource::action(Resource::VIEW_TB_CONTROL));
    QMenu* dockMenu = menuView->addMenu(tr("&Docks"));
    dockMenu->addAction(Resource::action(Resource::VIEW_DOCK_MACROS));
    dockMenu->addAction(Resource::action(Resource::VIEW_DOCK_UNDOSTACK));
    dockMenu->addAction(Resource::action(Resource::VIEW_DOCK_CONSOLE));
    dockMenu->addAction(Resource::action(Resource::VIEW_DOCK_PROPS));
    dockMenu->addAction(Resource::action(Resource::VIEW_DOCK_SYSMSG));
    menuView->addSeparator();
    menuView->addAction(Resource::action(Resource::VIEW_ZOOM_100));
    menuView->addAction(Resource::action(Resource::VIEW_ZOOM_IN));
    menuView->addAction(Resource::action(Resource::VIEW_ZOOM_OUT));
    menuView->addAction(Resource::action(Resource::VIEW_ZOOM_SEL));
    menuView->addAction(Resource::action(Resource::VIEW_ZOOM_PAGE));

    // build control menu
    menuControl = this->addMenu(tr("&Control"));
    menuControl->addAction(Resource::action(Resource::CTRL_START));
    menuControl->addAction(Resource::action(Resource::CTRL_PAUSE));
    menuControl->addAction(Resource::action(Resource::CTRL_STOP));
    menuControl->addSeparator();
    menuControl->addAction(Resource::action(Resource::CTRL_SNAP));

    // build extras menu
    menuExtras = this->addMenu(tr("E&xtras"));
    menuExtras->addAction(Resource::action(Resource::EXTRAS_SETTINGS));

    // build help menu
    menuHelp = this->addMenu(tr("&Help"));
    menuHelp->addAction(Resource::action(Resource::HELP_ABOUT));
  }

  void MenuBarImpresario::changeState(MainWindow::AppState /*oldState*/, MainWindow::AppState newState)
  {
    switch(newState)
    {
    case MainWindow::None:
      break;
    case MainWindow::Standard:
      Resource::action(Resource::FILE_SAVE)->setVisible(false);
      Resource::action(Resource::FILE_SAVEAS)->setVisible(false);
      Resource::action(Resource::FILE_CLOSE)->setVisible(false);
      Resource::action(Resource::FILE_CLOSEALL)->setVisible(false);

      Resource::action(Resource::VIEW_TB_EDIT)->setVisible(false);
      Resource::action(Resource::VIEW_TB_VIEW)->setVisible(false);
      Resource::action(Resource::VIEW_TB_CONTROL)->setVisible(false);
      Resource::action(Resource::VIEW_DOCK_UNDOSTACK)->setVisible(false);
      Resource::action(Resource::VIEW_DOCK_PROPS)->setVisible(false);
      Resource::action(Resource::VIEW_DOCK_CONSOLE)->setVisible(false);
      Resource::action(Resource::VIEW_ZOOM_IN)->setVisible(false);
      Resource::action(Resource::VIEW_ZOOM_OUT)->setVisible(false);
      Resource::action(Resource::VIEW_ZOOM_SEL)->setVisible(false);
      Resource::action(Resource::VIEW_ZOOM_PAGE)->setVisible(false);
      Resource::action(Resource::VIEW_ZOOM_100)->setVisible(false);

      menuEdit->menuAction()->setVisible(false);
      menuControl->menuAction()->setVisible(false);

      break;
    case MainWindow::ProcessGraphEdit:
      Resource::action(Resource::FILE_SAVE)->setVisible(true);
      Resource::action(Resource::FILE_SAVEAS)->setVisible(true);
      Resource::action(Resource::FILE_CLOSE)->setVisible(true);
      Resource::action(Resource::FILE_CLOSEALL)->setVisible(true);

      Resource::action(Resource::VIEW_TB_EDIT)->setVisible(true);
      Resource::action(Resource::VIEW_TB_VIEW)->setVisible(true);
      Resource::action(Resource::VIEW_TB_CONTROL)->setVisible(true);
      Resource::action(Resource::VIEW_DOCK_UNDOSTACK)->setVisible(true);
      Resource::action(Resource::VIEW_DOCK_PROPS)->setVisible(true);
      Resource::action(Resource::VIEW_DOCK_CONSOLE)->setVisible(true);
      Resource::action(Resource::VIEW_ZOOM_IN)->setVisible(true);
      Resource::action(Resource::VIEW_ZOOM_OUT)->setVisible(true);
      Resource::action(Resource::VIEW_ZOOM_SEL)->setVisible(true);
      Resource::action(Resource::VIEW_ZOOM_PAGE)->setVisible(true);
      Resource::action(Resource::VIEW_ZOOM_100)->setVisible(true);

      menuEdit->menuAction()->setVisible(true);
      menuControl->menuAction()->setVisible(true);

      break;
    }
  }
}
