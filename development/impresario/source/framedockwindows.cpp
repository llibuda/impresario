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

#include "framedockwindows.h"
#include "resources.h"
#include "syslogwndlogger.h"
#include "syslogwndconsole.h"
#include "dbwndmacros.h"
#include "pgewndprops.h"
#include "dbwndprop.h"
#include <QSettings>
#include <QUndoView>

namespace frame
{
  //-----------------------------------------------------------------------
  // Class DockWndBase
  //-----------------------------------------------------------------------
  DockWndBase::DockWndBase(const QString& objName, const QString& title, QWidget* parent) : QDockWidget(title,parent), visibleWhenAvailable(true)
  {
    this->setObjectName(objName);
    this->loadVisibility();
  }

  DockWndBase::~DockWndBase()
  {
    saveVisibility();
  }

  void DockWndBase::changeState(MainWindow::AppState /*oldState*/, MainWindow::AppState /*newState*/)
  {
  }

  void DockWndBase::loadVisibility()
  {
    Q_ASSERT(!objectName().isEmpty());
    QSettings settings;
    QString path = Resource::path(Resource::SETTINGS_GUI_DOCKWND_VISIBILITY) + '/' + objectName();
    visibleWhenAvailable = settings.value(path,true).toBool();
  }

  void DockWndBase::saveVisibility()
  {
    Q_ASSERT(!objectName().isEmpty());
    QSettings settings;
    QString path = Resource::path(Resource::SETTINGS_GUI_DOCKWND_VISIBILITY) + '/' + objectName();
    settings.setValue(path,visibleWhenAvailable);
  }

  //-----------------------------------------------------------------------
  // Class DockWndSysLog
  //-----------------------------------------------------------------------
  DockWndSysLog::DockWndSysLog(QWidget *parent) : DockWndBase("DockSystemLog",tr("System Messages"),parent)
  {
    syslog::WndLogger* logWnd = new syslog::WndLogger(this);
    this->setWidget(logWnd);
  }

  //-----------------------------------------------------------------------
  // Class DockWndMacroDb
  //-----------------------------------------------------------------------
  DockWndMacroDb::DockWndMacroDb(QWidget *parent) : DockWndBase("DockMacroDB",tr("Macros"),parent), splitWnd(0)
  {
    splitWnd = new Splitter(Qt::Vertical);
    db::WndTypeProps* props = new db::WndTypeProps(splitWnd);
    db::WndMacros* macroWnd = new db::WndMacros(splitWnd);
    splitWnd->addWidget(macroWnd);
    splitWnd->addWidget(props);
    splitWnd->setCollapsible(0,false);
    splitWnd->setCollapsible(1,true);
    QSettings settings;
    splitWnd->restoreState(QByteArray::fromBase64(settings.value(Resource::path(Resource::SETTINGS_GUI_MACROWND_SPLITTER)).toByteArray()));
    this->setWidget(splitWnd);
    connect(macroWnd,SIGNAL(selectionChanged(const db::ModelItem*,const db::ModelItem*)),props,SLOT(updateProps(const db::ModelItem*,const db::ModelItem*)));
  }

  DockWndMacroDb::~DockWndMacroDb()
  {
    QSettings settings;
    settings.setValue(Resource::path(Resource::SETTINGS_GUI_MACROWND_SPLITTER),splitWnd->saveState().toBase64());
  }

  //-----------------------------------------------------------------------
  // Class DockWndChangeLog
  //-----------------------------------------------------------------------
  DockWndChangeLog::DockWndChangeLog(QUndoGroup* undoGroup, QWidget *parent) : DockWndBase("DockChangeLog",tr("Change log"),parent)
  {
    QUndoView* undoView = new QUndoView(undoGroup);
    undoView->setCleanIcon(QIcon(":/icons/resources/save.png"));
    undoView->setEmptyLabel(tr("<saved>"));
    this->setWidget(undoView);
  }

  void DockWndChangeLog::changeState(MainWindow::AppState oldState, MainWindow::AppState newState)
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
  // Class DockWndProperties
  //-----------------------------------------------------------------------
  DockWndProperties::DockWndProperties(QWidget *parent) : DockWndBase("DockMacroProperties",tr("Properties"),parent)
  {
    pge::WndProperties* propWnd = new pge::WndProperties(this);
    this->setWidget(propWnd);
  }

  void DockWndProperties::changeState(MainWindow::AppState oldState, MainWindow::AppState newState)
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
  // Class DockWndConsole
  //-----------------------------------------------------------------------
  DockWndConsole::DockWndConsole(QWidget *parent) : DockWndBase("DockConsoleOut",tr("Console output"),parent)
  {
    this->setWidget(new syslog::WndConsole(this));
  }

  DockWndConsole::~DockWndConsole()
  {
  }

  void DockWndConsole::changeState(MainWindow::AppState oldState, MainWindow::AppState newState)
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
