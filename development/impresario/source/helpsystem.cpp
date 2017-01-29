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
#include "helpsystem.h"
#include "sysloglogger.h"
#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>

namespace help
{
  //-----------------------------------------------------------------------
  // Class System
  //-----------------------------------------------------------------------
  System::System() : QObject(0), ptrHelpEngine(0), ptrHelpMainWnd(0)
  {
  }

  System::~System()
  {
    destroyHelpEngine();
  }

  void System::initialize(const QString &helpCollectionFilePath, const QString& mainHelpCheckExpression)
  {
    // if help is already initialized, we close it first
    destroyHelpEngine();
    // create new help engine
    ptrHelpEngine = new QHelpEngine(helpCollectionFilePath);
    if (!ptrHelpEngine)
    {
      syslog::error(QString(tr("Help system: Online help is not available. Could not allocate memory.")));
      return;
    }

    // we need to call setupData() first, otherwise no qch file will be registered successfully
    disconnect(ptrHelpEngine,SIGNAL(setupFinished()),ptrHelpEngine->searchEngine(),SLOT(indexDocumentation()));
    ptrHelpEngine->setupData();

    QString errorMsg = ptrHelpEngine->error();
    if (errorMsg.count() > 0)
    {
      syslog::error(QString(tr("Help system: Online help is not available. %1")).arg(errorMsg));
      destroyHelpEngine();
      return;
    }
    QFileInfo fileInfo(helpCollectionFilePath);
    QDir helpDir(fileInfo.absolutePath());
    // register all help files in directory in our collection
    QStringList helpFilePattern;
    helpFilePattern.append("*.qch");
    QStringList helpFiles = helpDir.entryList(helpFilePattern, QDir::Files | QDir::NoSymLinks, QDir::Name | QDir::IgnoreCase);
    foreach(QString helpFile, helpFiles)
    {
      if (ptrHelpEngine->registerDocumentation(helpDir.absoluteFilePath(helpFile)))
      {
        syslog::info(QString(tr("Help system: Registered help file '%1'.")).arg(QDir::toNativeSeparators(helpDir.absoluteFilePath(helpFile))));
      }
    }
    // connect signals and slots
    connect(ptrHelpEngine,SIGNAL(setupStarted()),this,SLOT(helpSetupStarted()));
    connect(ptrHelpEngine,SIGNAL(setupFinished()),this,SLOT(helpSetupFinished()));
    connect(ptrHelpEngine,SIGNAL(warning(QString)),this,SLOT(helpWarning(QString)));
    connect(ptrHelpEngine->searchEngine(),SIGNAL(indexingStarted()),this,SLOT(helpIndexingStarted()));
    connect(ptrHelpEngine->searchEngine(),SIGNAL(indexingFinished()),this,SLOT(helpIndexingFinished()));

    connect(ptrHelpEngine,SIGNAL(setupFinished()),ptrHelpEngine->searchEngine(),SLOT(indexDocumentation()));

    // check whether we have any help at all
    QStringList registeredHelpFiles = ptrHelpEngine->registeredDocumentations();
    if (registeredHelpFiles.count() == 0)
    {
      syslog::error(QString(tr("Help system: Online help is not available. No help files registered in path '%1'. %2")).arg(QDir::toNativeSeparators(helpDir.absolutePath())).arg(ptrHelpEngine->error()));
      destroyHelpEngine();
      return;
    }

    // check whether we have main help
    QRegularExpression regEx(mainHelpCheckExpression,QRegularExpression::CaseInsensitiveOption);
    if (registeredHelpFiles.indexOf(regEx,0) < 0)
    {
      syslog::warning(QString(tr("Help system: Main help for application is not available due to missing help file.")));
    }
    syslog::info(QString(tr("Help system: Online help initialized. Number of referenced help files: %1")).arg(registeredHelpFiles.count()));
  }

  void System::showHelpContents()
  {
    if (ptrHelpEngine)
    {
      showHelpMainWindow();
    }
    else
    {
      QMessageBox msgBox(QMessageBox::Critical,QApplication::applicationName(),tr("Help system was not correctly initialized. Online help is not available."),QMessageBox::Ok,findApplicationMainWindow());
      msgBox.exec();
    }
  }

  void System::showHelpIndex()
  {
    if (ptrHelpEngine)
    {
      showHelpMainWindow();
    }
    else
    {
      QMessageBox msgBox(QMessageBox::Critical,QApplication::applicationName(),tr("Help system was not correctly initialized. Online help is not available."),QMessageBox::Ok,findApplicationMainWindow());
      msgBox.exec();
    }
  }

  void System::closeHelp()
  {
    destroyHelpMainWindow();
  }

  void System::helpSetupStarted()
  {
    // syslog::info(QString(tr("Help system: Setup started.")));
  }

  void System::helpSetupFinished()
  {
    syslog::info(QString(tr("Help system: Setup finished.")));
  }

  void System::helpIndexingStarted()
  {
    syslog::info(QString(tr("Help system: Indexing started.")));
  }

  void System::helpIndexingFinished()
  {
    syslog::info(QString(tr("Help system: Indexing finished.")));
  }

  void System::helpWarning(const QString& msg)
  {
    syslog::warning(QString(tr("Help system: %1")).arg(msg));
  }

  void System::showHelpMainWindow()
  {
    if (!ptrHelpMainWnd && ptrHelpEngine)
    {
      ptrHelpMainWnd = new MainWindow(*ptrHelpEngine);
    }
    if (ptrHelpMainWnd && ptrHelpMainWnd->isHidden())
    {
      ptrHelpMainWnd->show();
    }
  }

  void System::destroyHelpMainWindow()
  {
    if (ptrHelpMainWnd)
    {
      ptrHelpMainWnd->close();
      delete ptrHelpMainWnd;
      ptrHelpMainWnd = 0;
    }
  }

  void System::destroyHelpEngine()
  {
    destroyHelpMainWindow();
    delete ptrHelpEngine;
    ptrHelpEngine = 0;
  }

  QWidget* System::findApplicationMainWindow() const
  {
    QWidgetList widgetList = QApplication::topLevelWidgets();
    foreach(QWidget* widget, widgetList)
    {
      QMainWindow* mainWindow = qobject_cast<QMainWindow*>(widget);
      if (mainWindow)
      {
        return mainWindow;
      }
    }
    return 0;
  }

}
