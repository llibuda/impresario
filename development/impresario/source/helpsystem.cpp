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
#include "framemainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

namespace help
{
  //-----------------------------------------------------------------------
  // Class MainWindow
  //-----------------------------------------------------------------------
  MainWindow* MainWindow::wndInstance = 0;

  MainWindow& MainWindow::instance()
  {
    if (!wndInstance)
    {
      wndInstance = new MainWindow();
    }
    return *wndInstance;
  }

  void MainWindow::release()
  {
    if (wndInstance)
    {
      delete wndInstance;
      wndInstance = 0;
    }
  }

  MainWindow::MainWindow() : QMainWindow(), ptrBrowser(0)
  {
    Q_ASSERT(!System::helpEngine().isNull());
    setWindowTitle(QApplication::applicationName() + " - " + tr("Help"));

    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks);
    setCorner(Qt::TopLeftCorner,Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner,Qt::LeftDockWidgetArea);
    setTabPosition(Qt::AllDockWidgetAreas,QTabWidget::North);

    // setup status bar
    QStatusBar* statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    ptrBrowser = new ContentWindow(this);
    setCentralWidget(ptrBrowser);

    // create dock widgets
    QDockWidget* dockWidgetContents = new QDockWidget(tr("Contents"),this);
    dockWidgetContents->setWidget(System::helpEngine()->contentWidget());
    addDockWidget(Qt::LeftDockWidgetArea,dockWidgetContents);

    QDockWidget* dockWidgetIndex = new QDockWidget(tr("Index"),this);
    dockWidgetIndex->setWidget(System::helpEngine()->indexWidget());
    addDockWidget(Qt::LeftDockWidgetArea,dockWidgetIndex);

    QDockWidget* dockWidgetSearch = new QDockWidget(tr("Search"),this);
    QWidget* widgetSearch = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(widgetSearch);
    layout->addWidget(System::helpEngine()->searchEngine()->queryWidget());
    layout->addWidget(System::helpEngine()->searchEngine()->resultWidget(),1);
    layout->setMargin(5);
    widgetSearch->setLayout(layout);
    dockWidgetSearch->setWidget(widgetSearch);
    addDockWidget(Qt::LeftDockWidgetArea,dockWidgetSearch);

    tabifyDockWidget(dockWidgetContents,dockWidgetIndex);
    tabifyDockWidget(dockWidgetIndex,dockWidgetSearch);
    dockWidgetContents->raise();

    // connect signals
    connect(System::helpEngine()->contentWidget(),SIGNAL(linkActivated(QUrl)),this,SLOT(showPage(QUrl)));
    connect(System::helpEngine()->searchEngine()->queryWidget(),SIGNAL(search()),this,SLOT(runSearch()));
    connect(System::helpEngine()->searchEngine()->resultWidget(),SIGNAL(requestShowLink(QUrl)),this,SLOT(showPage(QUrl)));
  }


  MainWindow::~MainWindow()
  {
  }

  void MainWindow::runSearch()
  {
    System::helpEngine()->searchEngine()->search(System::helpEngine()->searchEngine()->queryWidget()->query());
  }

  void MainWindow::showPage(const QUrl &url)
  {
    statusBar()->showMessage(QString(tr("Url: %1")).arg(url.toString()));
    //ptrBrowser->setHtml(ptrHelpEngine->fileData(url),QUrl("qthelp://impresario.2.0/doc/"));
    ptrBrowser->setUrl(url);
  }

  //-----------------------------------------------------------------------
  // Class System
  //-----------------------------------------------------------------------
  PtrHelpEngine System::ptrHelpEngine;

  System::System() : QObject(0)
  {
  }

  System::~System()
  {
    closeHelp();
  }

  void System::initHelp(const QString &helpCollectionFilePath, const QString& mainHelpCheckExpression)
  {
    PtrHelpEngine helpEngineInstance = PtrHelpEngine(new QHelpEngine(helpCollectionFilePath));
    helpEngineInstance->setupData();
    QString errorMsg = helpEngineInstance->error();
    if (errorMsg.count() > 0)
    {
      syslog::error(QString(tr("Help system: Online help is not available. %1")).arg(errorMsg));
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
      if (helpEngineInstance->registerDocumentation(helpDir.absoluteFilePath(helpFile)))
      {
        syslog::info(QString(tr("Help system: Registered help file '%1'.")).arg(QDir::toNativeSeparators(helpDir.absoluteFilePath(helpFile))));
      }
    }
    // check whether we have any help at all
    QStringList registeredHelpFiles = helpEngineInstance->registeredDocumentations();
    if (registeredHelpFiles.count() == 0)
    {
      syslog::error(QString(tr("Help system: Online help is not available. No help files registered in path '%1'. %2")).arg(QDir::toNativeSeparators(helpDir.absolutePath())).arg(helpEngineInstance->error()));
      return;
    }

    // check whether we have Impresario help
    QRegularExpression regEx(mainHelpCheckExpression,QRegularExpression::CaseInsensitiveOption);
    if (registeredHelpFiles.indexOf(regEx,0) < 0)
    {
      syslog::warning(QString(tr("Help system: Main help for application is not available due to missing help file.")));
    }
    syslog::info(QString(tr("Help system: Online help initialized. Number of referenced help files: %1")).arg(registeredHelpFiles.count()));

    // exchange help engine
    MainWindow::release();
    ptrHelpEngine = helpEngineInstance;
    ptrHelpEngine->searchEngine()->reindexDocumentation();
  }

  void System::showHelpContents()
  {
    if (!ptrHelpEngine.isNull())
    {
      MainWindow& helpWndInstance = MainWindow::instance();
      helpWndInstance.show();
    }
    else
    {
      QMessageBox msgBox(QMessageBox::Critical,QApplication::applicationName(),tr("Help system was not correctly initialized. Online help is not available."),QMessageBox::Ok,&frame::MainWindow::instance());
      msgBox.exec();
    }
  }

  void System::showHelpIndex()
  {
    if (!ptrHelpEngine.isNull())
    {
      MainWindow& helpWndInstance = MainWindow::instance();
      helpWndInstance.show();
    }
    else
    {
      QMessageBox msgBox(QMessageBox::Critical,QApplication::applicationName(),tr("Help system was not correctly initialized. Online help is not available."),QMessageBox::Ok,&frame::MainWindow::instance());
      msgBox.exec();
    }
  }

  void System::closeHelp()
  {
    MainWindow::release();
    ptrHelpEngine.clear();
  }

}
