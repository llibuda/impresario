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
  System::System() : QObject(0), ptrHelpEngine(0), ptrHelpMainWnd(0), helpInitialized(false), urlMainPage()
  {
  }

  System::~System()
  {
    destroyHelpEngine();
  }

  void System::initialize(const QString &helpCollectionFilePath, const QString& mainPageID)
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

    // scan for help files in given directory recursively
    bool updateRequired = false;
    QFileInfo fileInfo(helpCollectionFilePath);
    QDir helpDir(fileInfo.absolutePath());
    QStringList helpFiles;
    scanForHelpFiles(helpDir,helpFiles);
    // get registered documentations
    QStringList registeredNamespaces = ptrHelpEngine->registeredDocumentations();
    // run through registered namespaces and check whether help file is actually existing.
    // if it does not exist, remove it from help.
    foreach(QString nameSpace, registeredNamespaces)
    {
      QString helpFile = ptrHelpEngine->documentationFileName(nameSpace);
      int fileIndex = helpFiles.indexOf(helpFile);
      if (fileIndex < 0)
      {
        if (ptrHelpEngine->unregisterDocumentation(nameSpace))
        {
          syslog::warning(QString(tr("Help system: Removed reference to help file '%1'. File does not exist.")).arg(QDir::toNativeSeparators(helpFile)));
          updateRequired = true;
        }
        else
        {
          syslog::error(QString(tr("Help system: Failed to unregister non-existing help file '%1'. %2")).arg(QDir::toNativeSeparators(helpFile)).arg(ptrHelpEngine->error()));
        }
      }
      else
      {
        // file exists and is registered -> remove it from file list
        helpFiles.removeAt(fileIndex);
      }
    }
    // all remaining files in list must now be registered.
    foreach(QString helpFile, helpFiles)
    {
      if (ptrHelpEngine->registerDocumentation(helpFile))
      {
        syslog::info(QString(tr("Help system: Registered help file '%1'.")).arg(QDir::toNativeSeparators(helpFile)));
        updateRequired = true;
      }
      else
      {
        syslog::error(QString(tr("Help system: Failed to register help file '%1'. %2")).arg(QDir::toNativeSeparators(helpFile)).arg(ptrHelpEngine->error()));
      }
    }
    // connect signals and slots
    connect(ptrHelpEngine,SIGNAL(setupStarted()),this,SLOT(helpSetupStarted()));
    connect(ptrHelpEngine,SIGNAL(setupFinished()),this,SLOT(helpSetupFinished()));
    connect(ptrHelpEngine,SIGNAL(warning(QString)),this,SLOT(helpWarning(QString)));
    connect(ptrHelpEngine->searchEngine(),SIGNAL(indexingStarted()),this,SLOT(helpIndexingStarted()));
    connect(ptrHelpEngine->searchEngine(),SIGNAL(indexingFinished()),this,SLOT(helpIndexingFinished()));

    connect(ptrHelpEngine,SIGNAL(setupFinished()),ptrHelpEngine->searchEngine(),SLOT(indexDocumentation()));

    if (updateRequired)
    {
      ptrHelpEngine->setupData();
    }

    // check whether we have any help at all
    QStringList registeredHelpFiles = ptrHelpEngine->registeredDocumentations();
    if (registeredHelpFiles.count() == 0)
    {
      syslog::error(QString(tr("Help system: Online help is not available. No help files registered in path '%1'. %2")).arg(QDir::toNativeSeparators(helpDir.absolutePath())).arg(ptrHelpEngine->error()));
      destroyHelpEngine();
      return;
    }

    // check whether we have main help
    QMap<QString,QUrl> mapHits = ptrHelpEngine->linksForIdentifier(mainPageID);
    if (mapHits.count() == 0)
    {
      syslog::warning(QString(tr("Help system: Main help for application is not available due to missing help file.")));
    }
    else
    {
      urlMainPage = mapHits.first();
    }
    syslog::info(QString(tr("Help system: Online help initialized. Number of referenced help files: %1")).arg(registeredHelpFiles.count()));
    helpInitialized = true;
  }

  void System::showHelpContents(const QString& helpID)
  {
    if (helpInitialized)
    {
      QUrl url = urlMainPage;
      if (!helpID.isEmpty())
      {
        QMap<QString,QUrl> mapHits = ptrHelpEngine->linksForIdentifier(helpID);
        if (mapHits.count() > 0)
        {
          url = mapHits.first();
        }
      }
      showHelpMainWindow(url);
    }
    else
    {
      QMessageBox msgBox(QMessageBox::Critical,QApplication::applicationName(),tr("Help system was not correctly initialized. Online help is not available."),QMessageBox::Ok,findApplicationMainWindow());
      msgBox.exec();
    }
  }

  void System::showHelpIndex()
  {
    if (helpInitialized)
    {
      showHelpMainWindow(urlMainPage);
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

  void System::showHelpMainWindow(const QUrl url)
  {
    if (!ptrHelpMainWnd && ptrHelpEngine)
    {
      ptrHelpMainWnd = new MainWindow(*ptrHelpEngine);
    }
    if (ptrHelpMainWnd)
    {
      if (url.isValid())
      {
        ptrHelpMainWnd->showPage(url);
      }
      if (ptrHelpMainWnd->isHidden())
      {
        ptrHelpMainWnd->show();
      }
      else
      {
        ptrHelpMainWnd->raise();
      }
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
    helpInitialized = false;
    urlMainPage = QUrl();
  }

  QWidget* System::findApplicationMainWindow()
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

  void System::scanForHelpFiles(const QDir& directory, QStringList& helpFileList)
  {
    if (!directory.exists()) return;
    // scan for *.qch files
    QStringList pattern;
    pattern.append("*.qch");
    QFileInfoList helpFileInfos = directory.entryInfoList(pattern, QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name | QDir::IgnoreCase | QDir::DirsLast);
    foreach(QFileInfo fileInfo, helpFileInfos)
    {
      if (fileInfo.isFile())
      {
        helpFileList.append(fileInfo.absoluteFilePath());
      }
      else if (fileInfo.isDir())
      {
        // recursive step down
        scanForHelpFiles(QDir(fileInfo.absoluteFilePath()),helpFileList);
      }
    }
  }

}