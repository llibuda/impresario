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
#ifndef HELPSYSTEM_H
#define HELPSYSTEM_H

#include "helpwindows.h"
#include <QObject>
#include <QtHelp>

namespace help
{
  class System : public QObject
  {
    Q_OBJECT
  public:
    System();
    virtual ~System();

    void initialize(const QString& helpCollectionFilePath, const QString& mainHelpCheckExpression);

    bool helpAvailable() const
    {
      return (ptrHelpEngine != 0);
    }

  signals:

  public slots:
    void showHelpContents();
    void showHelpIndex();
    void closeHelp();

  private slots:
    void helpSetupStarted();
    void helpSetupFinished();
    void helpIndexingStarted();
    void helpIndexingFinished();
    void helpWarning(const QString& msg);

  private:
    void showHelpMainWindow();
    void destroyHelpMainWindow();
    void destroyHelpEngine();
    QWidget* findApplicationMainWindow() const;

    QHelpEngine* ptrHelpEngine;
    MainWindow*  ptrHelpMainWnd;
  };

}
#endif // HELPSYSTEM_H
