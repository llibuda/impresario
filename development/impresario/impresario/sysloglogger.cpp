/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2018  Lars Libuda
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
  void info(const QString& msg)
  {
    Logger::instance().write(Logger::Information,msg);
  }

  void warning(const QString& msg)
  {
    Logger::instance().write(Logger::Warning,msg);
  }

  void error(const QString& msg)
  {
    Logger::instance().write(Logger::Error,msg);
  }

  //-----------------------------------------------------------------------
  // Class Logger
  //-----------------------------------------------------------------------
  int Logger::idMsgType = qRegisterMetaType<Logger::MsgType>("Logger::MsgType");
  int Logger::idVectorInt = qRegisterMetaType< QVector<int> >("Logger::Vector<int>");

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

  void Logger::write(MsgType type, const QString& msg)
  {
    QStandardItem* item = new QStandardItem();
    switch(type)
    {
    case Information:
      item->setIcon(icoInfo);
      break;
    case Warning:
      item->setIcon(icoWarning);
      break;
    case Error:
      item->setIcon(icoError);
      break;
    }

    QMutexLocker locker(&mutex);
    appendRow(item);
    QDateTime timeStamp = QDateTime::currentDateTime();
    setData(index(rowCount()-1,0),timeStamp.toString("hh:mm:ss.zzz"));
    setData(index(rowCount()-1,0),timeStamp.toString("yyyy-MM-dd hh:mm:ss.zzz"),SaveTimeStampRole);
    setData(index(rowCount()-1,1),QChar(type));
    setData(index(rowCount()-1,2),msg);
    stat[type]++;
    emit changedMsgCount(type,stat[type],stat[Information] + stat[Warning] + stat[Error]);
  }

  void Logger::clear()
  {
    QMutexLocker locker(&mutex);
    removeRows(0,rowCount());
    emit changedMsgCount(Error,0,0);
    stat.clear();
  }

  void Logger::save()
  {
    QString filters = tr("Log files (*.log);;All files (*.*)");
    QString defFileName = QCoreApplication::applicationName() + QDate::currentDate().toString("_yyyy-MM-dd") + ".log";
    QString filename = QFileDialog::getSaveFileName(0, tr("Save log file"),defFileName,filters,0,QFileDialog::DontConfirmOverwrite);
    if (filename.length() == 0)
    {
      return;
    }
    // check whether file exists
    QFile file(filename);
    QFile::OpenMode mode = QFile::Text | QFile::WriteOnly;
    mode |= (file.exists()) ? QFile::Append : QFile::Truncate;

    // save log to file
    if (file.open(mode))
    {
      QTextStream out(&file);
      out.setFieldAlignment(QTextStream::AlignLeft);
      out << "------- " << app::Impresario::instance().applicationName();
      out << " log written " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
      out << " -------" << endl;

      const int timeFieldWidth = 24;
      const int typeFieldWidth = 2;
      for(int i = 0; i < rowCount(); ++i)
      {
        out.setFieldWidth(timeFieldWidth);
        out << data(index(i,0),SaveTimeStampRole).toString();
        out.setFieldWidth(typeFieldWidth);
        out << data(index(i,1)).toChar();
        out.setFieldWidth(0);
        QStringList textLines = data(index(i,2)).toString().split('\n');
        for(int index = 0; index < textLines.count(); ++index)
        {
          if (index > 0)
          {
            out << QString(timeFieldWidth + typeFieldWidth,' ');
          }
          out << textLines.at(index) << endl;
        }
      }

      file.close();
    }
  }
}
