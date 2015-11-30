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
#ifndef APPIMPRESARIO_H
#define APPIMPRESARIO_H

#include "resources.h"
#include "singleapplication.h"
#include <QString>
#include <QIODevice>
#include <QSettings>
#include <QQmlEngine>
#include <QWidget>

namespace app
{
  class Impresario : public SingleApplication
  {
    Q_OBJECT
  public:
    static Impresario& instance(int & argc, char ** argv);
    static Impresario& instance();
    static void release();

    bool initCritical();
    bool initNonCritical();

    void setActivationWindow(QWidget* actWin)
    {
      wndActWindow = actWin;
    }

    QQmlEngine& qmlEngine()
    {
      return qmlEngineInstance;
    }

  public slots:
    void settingChanged(Resource::SettingsIDs id);
    void initMacroLibraries();

  signals:
    void initCriticalSuccessful();
    void initNonCriticalSuccessful();
    void raiseMainWindow();

  private slots:
    void activatedByAnotherInstance();

  private:
    static bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map);
    static bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map);

    Impresario(int & argc, char ** argv);
    //Impresario(const Impresario& other) {}
    Impresario& operator=(const Impresario&) { return *this; }
    virtual ~Impresario();

    bool initResourcePath();
    bool initProcessGraphPath();
    bool initDepLibPaths();
    bool initMacroLibPaths();

    static Impresario* appInstance;

    QQmlEngine qmlEngineInstance;
    QWidget*   wndActWindow;
  };
}
#endif // APPIMPRESARIO_H
