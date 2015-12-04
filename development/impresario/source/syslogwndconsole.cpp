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

#include "syslogwndconsole.h"
#include "appimpresario.h"
#include "resources.h"
#include <QToolBar>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QMessageBox>
#include <QPushButton>

namespace syslog
{
  //-----------------------------------------------------------------------
  // Class WndConsole
  //-----------------------------------------------------------------------
  WndConsole::WndConsole(QWidget *parent) :  QWidget(parent), consoleLog(this), menu(this)
  {
    QToolBar* tbActions = new QToolBar(this);
    tbActions->addAction(Resource::action(Resource::CONSOLE_SAVE));
    tbActions->addAction(Resource::action(Resource::CONSOLE_CLEAR));

    consoleLog.setTextInteractionFlags(Qt::NoTextInteraction);

    // create layout of WndConsole
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addWidget(tbActions);
    layout->addWidget(&consoleLog,1);
    this->setLayout(layout);

    // create context menu
    menu.addAction(Resource::action(Resource::CONSOLE_SAVE));
    menu.addAction(Resource::action(Resource::CONSOLE_CLEAR));

    // connect actions
    Resource::action(Resource::CONSOLE_CLEAR)->setEnabled(false);
    Resource::action(Resource::CONSOLE_SAVE)->setEnabled(false);
    connect(Resource::action(Resource::CONSOLE_SAVE),SIGNAL(triggered()),this,SLOT(saveConsoleOutput()));
    connect(Resource::action(Resource::CONSOLE_CLEAR),SIGNAL(triggered()),&consoleLog,SLOT(clear()));
    connect(&consoleLog,SIGNAL(blockCountChanged(int)),this,SLOT(updateUI(int)));
  }

  WndConsole::~WndConsole()
  {
  }

  void WndConsole::contextMenuEvent(QContextMenuEvent* event)
  {
    menu.popup(event->globalPos());
  }

  void WndConsole::updateUI(int blockCount)
  {
    Resource::action(Resource::CONSOLE_CLEAR)->setEnabled(blockCount > 1);
    Resource::action(Resource::CONSOLE_SAVE)->setEnabled(blockCount > 1);
  }

  void WndConsole::saveConsoleOutput()
  {
    // ask for file
    QString filters = tr("Console output files (*.txt);;All files (*.*)");
    QString defFileName = app::Impresario::instance().applicationName() + tr("_ConsoleOutput_") + QDate::currentDate().toString("yyyy-MM-dd") + ".txt";
    QString filename = QFileDialog::getSaveFileName(0, tr("Save console output"),defFileName,filters,0,QFileDialog::DontConfirmOverwrite);
    if (filename.length() == 0)
    {
      return;
    }
    // check whether file exists
    QFile file(filename);
    QFile::OpenMode mode = QFile::Text | QFile::WriteOnly;
    if (file.exists())
    {
      QString msg = QString(tr("File %1 already exists. What would you like to do?")).arg(filename);
      QMessageBox msgBox;
      QPushButton *appendButton = msgBox.addButton(tr("Append"), QMessageBox::ActionRole);
      QPushButton *overwriteButton = msgBox.addButton(tr("Overwrite"), QMessageBox::ActionRole);
      QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);
      msgBox.setText(msg);
      msgBox.setWindowTitle(tr("Save console output"));
      msgBox.exec();
      if (msgBox.clickedButton() == appendButton)
      {
        mode |= QFile::Append;
      }
      else if (msgBox.clickedButton() == overwriteButton)
      {
        mode |= QFile::Truncate;
      }
      else if (msgBox.clickedButton() == abortButton)
      {
        return;
      }
    }
    // save console output to file
    if (file.open(mode))
    {
      QTextStream out(&file);
      out << "------- " << app::Impresario::instance().applicationName();
      out << " console output written " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
      out << " -------" << endl;
      out << consoleLog.toPlainText();
      file.close();
    }
  }

}
