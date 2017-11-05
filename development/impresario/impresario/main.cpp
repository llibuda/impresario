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

#include "appimpresario.h"
#include "framemainwindow.h"
#include "appdlgterminate.h"
#include <vector>

#ifdef _MSC_VER
/* The following line includes the header of Visual Leak Detection (see
 * https://kinddragon.github.io/vld/ for more details).
 * This tool can be used in Visual Studio on Windows systems to detect
 * memory leaks in your code quite comfortably.
 */
//#include <vld.h>
#endif // _MSC_VER

int main(int argc, char *argv[])
{
  /* We add command line parameter --disable-seccomp-filter-sandbox here
   * This parameter is passed to QtWebEngineProcess which is started by the help system.
   * In general, this parameter speeds up loading contents significantly.
   * On Manjaro Linux it prevents the process from crashing
   */
  const char* webEngineParameter = "--disable-seccomp-filter-sandbox";
  int argumentCount = argc + 1;
  std::vector<char*> arguments(argumentCount);
  for(int i = 0; i < argc; ++i)
    arguments[i] = argv[i];
  arguments[argumentCount - 1] = const_cast<char*>(webEngineParameter);

  int result = 1;
  app::Impresario& a = app::Impresario::instance(argumentCount, arguments.data());
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
