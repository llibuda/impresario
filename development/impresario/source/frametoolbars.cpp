/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2017  Lars Libuda
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

#include "frametoolbars.h"
#include "resources.h"
#include <QSettings>

namespace frame
{
  //-----------------------------------------------------------------------
  // Class ToolBarBase
  //-----------------------------------------------------------------------
  ToolBarBase::ToolBarBase(const QString& objName, const QString &title, QWidget *parent) : QToolBar(title,parent), visibleWhenAvailable(true)
  {
    this->setObjectName(objName);
    this->loadVisibility();
  }

  ToolBarBase::~ToolBarBase()
  {
    saveVisibility();
  }

  void ToolBarBase::changeState(MainWindow::AppState /*oldState*/, MainWindow::AppState /*newState*/)
  {
  }

  void ToolBarBase::loadVisibility()
  {
    Q_ASSERT(!objectName().isEmpty());
    QSettings settings;
    QString path = Resource::path(Resource::SETTINGS_GUI_TOOLBAR_VISIBILITY) + '/' + objectName();
    visibleWhenAvailable = settings.value(path,true).toBool();
  }

  void ToolBarBase::saveVisibility()
  {
    Q_ASSERT(!objectName().isEmpty());
    QSettings settings;
    QString path = Resource::path(Resource::SETTINGS_GUI_TOOLBAR_VISIBILITY) + '/' + objectName();
    settings.setValue(path,visibleWhenAvailable);
  }

  //-----------------------------------------------------------------------
  // Class ToolBarFile
  //-----------------------------------------------------------------------
  ToolBarFile::ToolBarFile(QWidget *parent) : ToolBarBase("ToolBarFile",tr("&File"),parent)
  {
    // add tools to toolbar
    this->addAction(Resource::action(Resource::FILE_NEW));
    this->addAction(Resource::action(Resource::FILE_LOAD));
    this->addAction(Resource::action(Resource::FILE_SAVE));
    this->addSeparator();
    this->addAction(Resource::action(Resource::FILE_QUIT));
  }

  //-----------------------------------------------------------------------
  // Class ToolBarEdit
  //-----------------------------------------------------------------------
  ToolBarEdit::ToolBarEdit(QWidget *parent) : ToolBarBase("ToolBarEdit",tr("&Edit"),parent)
  {
    // add tools to toolbar
    this->addAction(Resource::action(Resource::EDIT_CUT));
    this->addAction(Resource::action(Resource::EDIT_COPY));
    this->addAction(Resource::action(Resource::EDIT_PASTE));
    this->addSeparator();
    this->addAction(Resource::action(Resource::EDIT_UNDO));
    this->addAction(Resource::action(Resource::EDIT_REDO));
  }

  void ToolBarEdit::changeState(MainWindow::AppState oldState, MainWindow::AppState newState)
  {
    switch(oldState)
    {
    case MainWindow::None:
      break;
    case MainWindow::Standard:
      break;
    case MainWindow::ProcessGraphEdit:
      visibleWhenAvailable = this->isVisible();
      break;
    }
    switch(newState)
    {
    case MainWindow::None:
      break;
    case MainWindow::Standard:
      this->setVisible(false);
      break;
    case MainWindow::ProcessGraphEdit:
      this->setVisible(visibleWhenAvailable);
      break;
    }
  }

  //-----------------------------------------------------------------------
  // Class ToolBarView
  //-----------------------------------------------------------------------
  ToolBarView::ToolBarView(QWidget *parent) : ToolBarBase("ToolBarView",tr("&View"),parent), sbZoom(this)
  {
    // add tools to toolbar
    this->addAction(Resource::action(Resource::VIEW_ZOOM_SEL));
    this->addAction(Resource::action(Resource::VIEW_ZOOM_100));
    this->addWidget(&sbZoom);
    this->addAction(Resource::action(Resource::VIEW_ZOOM_PAGE));
    sbZoom.setMinimum(20);
    sbZoom.setMaximum(300);
    sbZoom.setSingleStep(10);
    sbZoom.setSuffix("%");
    sbZoom.setButtonSymbols(QAbstractSpinBox::PlusMinus);
  }

  void ToolBarView::changeState(MainWindow::AppState oldState, MainWindow::AppState newState)
  {
    switch(oldState)
    {
    case MainWindow::None:
      break;
    case MainWindow::Standard:
      break;
    case MainWindow::ProcessGraphEdit:
      visibleWhenAvailable = this->isVisible();
      break;
    }
    switch(newState)
    {
    case MainWindow::None:
      break;
    case MainWindow::Standard:
      this->setVisible(false);
      break;
    case MainWindow::ProcessGraphEdit:
      this->setVisible(visibleWhenAvailable);
      break;
    }
  }
  //-----------------------------------------------------------------------
  // Class ToolBarCtrl
  //-----------------------------------------------------------------------
  ToolBarCtrl::ToolBarCtrl(QWidget *parent) : ToolBarBase("ToolBarControl",tr("&Control"),parent)
  {
    // add tools to toolbar
    this->addAction(Resource::action(Resource::CTRL_START));
    this->addAction(Resource::action(Resource::CTRL_PAUSE));
    this->addAction(Resource::action(Resource::CTRL_STOP));
    this->addSeparator();
    this->addAction(Resource::action(Resource::CTRL_SNAP));
  }

  void ToolBarCtrl::changeState(MainWindow::AppState oldState, MainWindow::AppState newState)
  {
    switch(oldState)
    {
    case MainWindow::None:
      break;
    case MainWindow::Standard:
      break;
    case MainWindow::ProcessGraphEdit:
      visibleWhenAvailable = this->isVisible();
      break;
    }
    switch(newState)
    {
    case MainWindow::None:
      break;
    case MainWindow::Standard:
      this->setVisible(false);
      break;
    case MainWindow::ProcessGraphEdit:
      this->setVisible(visibleWhenAvailable);
      break;
    }
  }

}
