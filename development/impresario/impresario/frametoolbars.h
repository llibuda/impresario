/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2019  Lars Libuda
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
#ifndef FRAMETOOLBARS_H
#define FRAMETOOLBARS_H

#include "framemainwindow.h"
#include <QToolBar>
#include <QString>
#include <QSpinBox>

namespace frame
{
  class ToolBarBase : public QToolBar
  {
    Q_OBJECT
  public:
    ToolBarBase(const QString& objName, const QString& title, QWidget* parent = 0);
    virtual ~ToolBarBase();

  public slots:
    virtual void changeState(MainWindow::AppState oldState, MainWindow::AppState newState);

  protected:
    void loadVisibility();
    void saveVisibility();

    bool visibleWhenAvailable;
  };

  class ToolBarFile : public ToolBarBase
  {
    Q_OBJECT
  public:
    ToolBarFile(QWidget *parent = 0);

  };

  class ToolBarEdit : public ToolBarBase
  {
    Q_OBJECT
  public:
    ToolBarEdit(QWidget *parent = 0);

  public slots:
    virtual void changeState(MainWindow::AppState oldState, MainWindow::AppState newState);
  };

  class ToolBarView : public ToolBarBase
  {
    Q_OBJECT
  public:
    ToolBarView(QWidget *parent = 0);

    QSpinBox* zoomBox()
    {
      return &sbZoom;
    }

  public slots:
    virtual void changeState(MainWindow::AppState oldState, MainWindow::AppState newState);

  private:
    QSpinBox sbZoom;
  };

  class ToolBarCtrl : public ToolBarBase
  {
    Q_OBJECT
  public:
    ToolBarCtrl(QWidget *parent = 0);

  public slots:
    virtual void changeState(MainWindow::AppState oldState, MainWindow::AppState newState);
  };
}
#endif // FRAMETOOLBARS_H
