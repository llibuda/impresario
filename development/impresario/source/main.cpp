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

#include "appimpresario.h"
#include "framemainwindow.h"
#include "appdlgterminate.h"

int main(int argc, char *argv[])
{
  int result = 1;
  app::Impresario& a = app::Impresario::instance(argc, argv);
  if (a.initCritical())
  {
    frame::MainWindow& mw = frame::MainWindow::instance();
    a.setActivationWindow(&mw);
    mw.show();
    a.initNonCritical();
    result = a.exec();
    frame::MainWindow::release();
  }
  else
  {
    app::DlgTerminate dlg;
    dlg.exec();
  }
  app::Impresario::release();
  return result;
}
