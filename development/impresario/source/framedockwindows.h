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
#ifndef FRAMEDOCKWINDOWS_H
#define FRAMEDOCKWINDOWS_H

#include "framemainwindow.h"
#include "qthelper.h"
#include <QDockWidget>
#include <QUndoGroup>

namespace frame
{
  class DockWndBase : public QDockWidget
  {
    Q_OBJECT
  public:
    DockWndBase(const QString& objName, const QString& title, QWidget* parent = 0);
    virtual ~DockWndBase();

  public slots:
    virtual void changeState(MainWindow::AppState oldState, MainWindow::AppState newState);

  protected:
    void loadVisibility();
    void saveVisibility();

    bool visibleWhenAvailable;

  };

  class DockWndSysLog : public DockWndBase
  {
    Q_OBJECT
  public:
    DockWndSysLog(QWidget *parent = 0);
  };

  class DockWndMacroDb : public DockWndBase
  {
    Q_OBJECT
  public:
    DockWndMacroDb(QWidget *parent = 0);
    virtual ~DockWndMacroDb();

  private:
    Splitter* splitWnd;
  };

  class DockWndChangeLog : public DockWndBase
  {
    Q_OBJECT
  public:
    DockWndChangeLog(QUndoGroup* undoGroup, QWidget *parent = 0);

  public slots:
    virtual void changeState(MainWindow::AppState oldState, MainWindow::AppState newState);
  };

  class DockWndProperties : public DockWndBase
  {
    Q_OBJECT
  public:
    DockWndProperties(QWidget *parent = 0);

  public slots:
    virtual void changeState(MainWindow::AppState oldState, MainWindow::AppState newState);
  };

  class DockWndConsole : public DockWndBase
  {
    Q_OBJECT
  public:
    DockWndConsole(QWidget *parent = 0);
    ~DockWndConsole();

  public slots:
    virtual void changeState(MainWindow::AppState oldState, MainWindow::AppState newState);
  };

}
#endif // FRAMEDOCKWINDOWS_H
