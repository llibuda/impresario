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

#include "sysloglogger.h"
#include "appimpresario.h"
#include <QDateTime>
#include <QStandardItem>
#include <QFileDialog>
#include <QDate>
#include <QDateTime>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include <QStringList>
#include <QVector>
#include <QMutexLocker>

namespace syslog
{
  int Logger::idMsgType = qRegisterMetaType<Logger::MsgType>("Logger::MsgType");
  int Logger::idVectorInt = qRegisterMetaType< QVector<int> >("Logger::Vector<int>");

  void info(const QString& msg)
  {
    Logger::instance().addMessage(Logger::Information,msg);
  }

  void warning(const QString& msg)
  {
    Logger::instance().addMessage(Logger::Warning,msg);
  }

  void error(const QString& msg)
  {
    Logger::instance().addMessage(Logger::Error,msg);
  }

  Logger& Logger::instance()
  {
    static Logger log;
    return log;
  }

  Logger::Logger(QObject *parent) :  QStandardItemModel(0,3,parent),
    icoError(":/icons/resources/error.png"),
    icoWarning(":/icons/resources/warning.png"),
    icoInfo(":/icons/resources/information.png")
  {
    setHeaderData(0, Qt::Horizontal, QObject::tr("Time"));
    setHeaderData(1, Qt::Horizontal, QObject::tr("Type"));
    setHeaderData(2, Qt::Horizontal, QObject::tr("Message"));
  }

  void Logger::addMessage(MsgType type, const QString& msg)
  {
    QStringList list = msg.split('\n',QString::SkipEmptyParts);
    QStandardItem* item = new QStandardItem();
    switch(type)
    {
    case Information:
      item->setIcon(icoInfo);
      emit changedMsgCount(Information,++stat[Information]);
      break;
    case Warning:
      item->setIcon(icoWarning);
      emit changedMsgCount(Warning,++stat[Warning]);
      break;
    case Error:
      item->setIcon(icoError);
      emit changedMsgCount(Error,++stat[Error]);
      break;
    }
    QMutexLocker locker(&mutex);
    appendRow(item);
    setData(index(rowCount()-1,0),QDateTime::currentDateTime());
    setData(index(rowCount()-1,1),QChar(type));
    if (!list.isEmpty())
    {
      setData(index(rowCount()-1,2),list[0]);

      list.pop_front();
      for(QStringList::Iterator it = list.begin(); it != list.end(); ++it)
      {
        item = new QStandardItem();
        appendRow(item);
        setData(index(rowCount()-1,1),QChar(type));
        setData(index(rowCount()-1,2),*it);
      }
    }
    else
    {
      setData(index(rowCount()-1,2),"");
    }
  }

  void Logger::clearLog()
  {
    QMutexLocker locker(&mutex);
    removeRows(0,rowCount());
    for(MsgStats::iterator iter = stat.begin(); iter != stat.end(); ++iter)
    {
      emit changedMsgCount(iter.key(),0);
    }
    stat.clear();
  }

  void Logger::saveLog()
  {
    // ask for file
    QString filters = tr("Log files (*.log);;All files (*.*)");
    QString defFileName = app::Impresario::instance().applicationName() + tr("_Log_") + QDate::currentDate().toString("yyyy-MM-dd") + ".log";
    QString filename = QFileDialog::getSaveFileName(0, tr("Save log file"),defFileName,filters,0,QFileDialog::DontConfirmOverwrite);
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
      msgBox.setWindowTitle(tr("Save log file"));
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
    // save log to file
    if (file.open(mode))
    {
      QTextStream out(&file);
      out << "------- " << app::Impresario::instance().applicationName();
      out << " log written " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
      out << " -------" << endl;

      for(int i = 0; i < rowCount(); ++i)
      {
        out.setFieldWidth(19);
        out << data(index(i,0)).toDateTime().toString("yyyy-MM-dd hh:mm:ss");
        out.setFieldWidth(0);
        out << "  " << data(index(i,1)).toChar() << "  ";
        out << data(index(i,2)).toString() << endl;
      }

      file.close();
    }
  }

}
