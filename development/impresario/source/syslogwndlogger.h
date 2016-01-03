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
#ifndef SYSLOGWNDLOGGER_H
#define SYSLOGWNDLOGGER_H

#include "sysloglogger.h"
#include <QWidget>
#include <QToolBar>
#include <QTreeView>
#include <QMenu>

namespace syslog
{

  class WndLogger : public QWidget
  {
    Q_OBJECT
  public:
    explicit WndLogger(QWidget *parent = 0);
    virtual ~WndLogger();

  protected:
    void contextMenuEvent(QContextMenuEvent* event);

  private slots:
    void updateUI(Logger::MsgType type, unsigned int count);
    void toggleFilterError(bool checked);
    void toggleFilterWarning(bool checked);
    void toggleFilterMessage(bool checked);

  private:
    QToolBar*  tbFilter;
    QToolBar*  tbActions;
    QTreeView* logView;
    QMenu*     menu;
  };

}

#endif // SYSLOGWNDLOGGER_H
