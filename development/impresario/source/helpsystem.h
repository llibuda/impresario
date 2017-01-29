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
#include <QMainWindow>
#include <QtHelp>
#include <QSharedPointer>

namespace help
{
  typedef QSharedPointer<QHelpEngine> PtrHelpEngine;

  class MainWindow : public QMainWindow
  {
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)
  public:

    static MainWindow& instance();
    static void release();

  private slots:
    void runSearch();
    void showPage(const QUrl& url);

  private:
    MainWindow();
    ~MainWindow();

    static MainWindow* wndInstance;

    ContentWindow* ptrBrowser;
  };

  class System : public QObject
  {
    Q_OBJECT
  public:
    System();
    virtual ~System();

    void initHelp(const QString& helpCollectionFilePath, const QString& mainHelpCheckExpression);

    bool helpAvailable() const
    {
      return (ptrHelpEngine != 0);
    }

    static const PtrHelpEngine helpEngine()
    {
      return ptrHelpEngine;
    }

  signals:

  public slots:
    void showHelpContents();
    void showHelpIndex();
    void closeHelp();

  private:
    static PtrHelpEngine ptrHelpEngine;
  };

}
#endif // HELPSYSTEM_H
