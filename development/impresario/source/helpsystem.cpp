#include "helpsystem.h"
#include "sysloglogger.h"
#include "framemainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

namespace help {


  System::System() : QObject(0), ptrHelpEngine(0)
  {
  }

  System::~System()
  {
    delete ptrHelpEngine;
    ptrHelpEngine = 0;
  }

  void System::initHelp(const QString &helpCollectionFilePath, const QString& mainHelpCheckExpression)
  {
    QHelpEngine* helpEngineInstance = new QHelpEngine(helpCollectionFilePath);
    helpEngineInstance->setupData();
    QString errorMsg = helpEngineInstance->error();
    if (errorMsg.count() > 0)
    {
      syslog::error(QString(tr("Help system: Online help is not available. %1")).arg(errorMsg));
      delete helpEngineInstance;
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
      delete helpEngineInstance;
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
    // TODO: close open help windows
    delete ptrHelpEngine;
    ptrHelpEngine = helpEngineInstance;
  }

  void System::showHelpContents()
  {
    if (ptrHelpEngine)
    {

    }
    else
    {
      QMessageBox msgBox(QMessageBox::Critical,QApplication::applicationName(),tr("Help system was not correctly initialized. Online help is not available."),QMessageBox::Ok,&frame::MainWindow::instance());
      msgBox.exec();
    }
  }

  void System::showHelpIndex()
  {
    if (ptrHelpEngine)
    {

    }
    else
    {
      QMessageBox msgBox(QMessageBox::Critical,QApplication::applicationName(),tr("Help system was not correctly initialized. Online help is not available."),QMessageBox::Ok,&frame::MainWindow::instance());
      msgBox.exec();
    }
  }

}
