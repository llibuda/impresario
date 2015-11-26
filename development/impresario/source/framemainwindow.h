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
#ifndef FRAMEMAINWINDOW_H
#define FRAMEMAINWINDOW_H

#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMap>
#include <QDockWidget>
#include <QUndoGroup>
#include <QSignalMapper>
#include "pgecomponents.h"
#include "appmacro.h"
#include "qthelper.h"

namespace frame
{

  class MainWindow : public QMainWindow
  {
    Q_OBJECT

  public:
    enum AppState
    {
      None,
      Standard,
      ProcessGraphEdit
    };

    static MainWindow& instance();
    static void release();

    AppState state() const
    {
      return currentState;
    }

    QWidget* mdiGetActive();

  signals:
    void changedState(MainWindow::AppState oldState, MainWindow::AppState newState);

  public slots:
    void mdiChildDestroy(pge::ProcessGraphEditor* child);

  private slots:
    void fileNew();
    void fileOpen();
    void extrasSettings();
    void helpAbout();
    void mdiUpdateUI();
    void mdiNavigateMacro();

  protected:
    virtual void closeEvent(QCloseEvent* event);

  private:
    MainWindow(QWidget* parent = 0);
    MainWindow& operator=(const MainWindow&) { return *this; }
    ~MainWindow();

    static MainWindow* wndInstance;

    pge::ProcessGraphEditor* mdiChildCreate(const QString& fileName = QString());
    QMdiSubWindow* mdiChildFind(const QString &fileName);

    QMdiArea*         mdiArea;
    QUndoGroup*       undoGroup;
    AppState          currentState;
    SignalMultiplexer multiplexer;
    QSignalMapper     mapper;
  };

}

#endif // FRAMEMAINWINDOW_H