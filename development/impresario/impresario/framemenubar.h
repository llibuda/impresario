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
#ifndef FRAMEMENUBARS_H
#define FRAMEMENUBARS_H

#include "framemainwindow.h"
#include <QMenuBar>
#include <QMenu>

namespace frame
{
  class MenuBarImpresario : public QMenuBar
  {
    Q_OBJECT

  public:
    explicit MenuBarImpresario(QWidget *parent);

  public slots:
    void changeState(MainWindow::AppState oldState, MainWindow::AppState newState);

  private:
    QMenu* menuFile;
    QMenu* menuEdit;
    QMenu* menuView;
    QMenu* menuControl;
    QMenu* menuExtras;
    QMenu* menuHelp;
  };
}
#endif // FRAMEMENUBAR_H
