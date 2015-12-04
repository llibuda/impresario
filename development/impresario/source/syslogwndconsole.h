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
#ifndef SYSLOGWNDCONSOLE_H
#define SYSLOGWNDCONSOLE_H

#include "stdconsoleinterface.h"
#include <QWidget>
#include <QMenu>

namespace syslog
{
  class WndConsole : public QWidget
  {
    Q_OBJECT
  public:
    explicit WndConsole(QWidget *parent = 0);
    ~WndConsole();

  protected:
    void contextMenuEvent(QContextMenuEvent* event);

  private slots:
    void updateUI(int blockCount);
    void saveConsoleOutput();

  private:
    std::ConsoleOutEdit consoleLog;
    QMenu               menu;
  };

}

#endif // SYSLOGWNDCONSOLE_H
