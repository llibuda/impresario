/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2020  Lars Libuda
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
#include <QCoreApplication>
#include <QTextStream>
#include <QFile>
#include <QStringList>

namespace syslog
{
  void info(const QString& msg, const QString& category)
  {
    Logger::instance().write(Logger::Information,msg,category);
  }

  void warning(const QString& msg, const QString& category)
  {
    Logger::instance().write(Logger::Warning,msg,category);
  }

  void error(const QString& msg, const QString& category)
  {
    Logger::instance().write(Logger::Error,msg,category);
  }

  //-----------------------------------------------------------------------
  // Class Logger
  //-----------------------------------------------------------------------
  int Logger::idMsgType = qRegisterMetaType<Logger::MsgType>("Logger::MsgType");

  Logger& Logger::instance()
  {
    static Logger log;
    return log;
  }

  Logger::Logger(QObject *parent) :  QObject(parent), messages(), categories(), stats(), maxCategoryLength(0)
  {
    QObject::connect(this,SIGNAL(newLogEntry(Logger::MsgType,QString,QString)),this,SLOT(createNewLogEntry(Logger::MsgType,QString,QString)),Qt::QueuedConnection);
  }

  void Logger::write(MsgType type, const QString& msg,const QString& category)
  {
    emit newLogEntry(type,msg,category);
  }

  void Logger::createNewLogEntry(Logger::MsgType type, const QString msg, const QString category)
  {
    if (msg.length() == 0 && category.length() == 0) return;
    LogEntry logEntry;
    logEntry.timeStamp = QDateTime::currentDateTime();
    logEntry.msgType = type;
    logEntry.message = msg;
    logEntry.category = category;
    messages.append(logEntry);
    stats[type]++;
    categories.insert(category);
    if (category.length() > maxCategoryLength) maxCategoryLength = category.length();
    emit changedMsgCount(type,stats[type],messages.count());
  }

  void Logger::clear()
  {
    messages.clear();
    categories.clear();
    stats.clear();
    maxCategoryLength = 0;
    emit changedMsgCount(Error,0,0);
  }

  void Logger::save(const QString& fileName)
  {
    // check whether file exists
    QFile file(fileName);
    QFile::OpenMode mode = QFile::Text | QFile::WriteOnly;
    mode |= (file.exists()) ? QFile::Append : QFile::Truncate;

    // save log to file
    if (file.open(mode))
    {
      QTextStream out(&file);
      out.setFieldAlignment(QTextStream::AlignLeft);
      out << "------- " << QCoreApplication::applicationName();
      out << " log written " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
      out << " -------" << Qt::endl;

      const int timeFieldWidth = 24;
      const int typeFieldWidth = 2;
      int categoryFieldWidth = 0;
      if (maxCategoryLength > 0) categoryFieldWidth = maxCategoryLength + 1;
      for(int i = 0; i < messages.count(); ++i)
      {
        out.setFieldWidth(timeFieldWidth);
        out << messages[i].timeStamp.toString("yyyy-MM-dd hh:mm:ss.zzz");
        out.setFieldWidth(typeFieldWidth);
        out << QChar(messages[i].msgType);
        if (categoryFieldWidth > 0)
        {
          out.setFieldWidth(categoryFieldWidth);
          out << messages[i].category;
        }
        out.setFieldWidth(0);
        QStringList textLines = messages[i].message.split('\n');
        for(int index = 0; index < textLines.count(); ++index)
        {
          if (index > 0)
          {
            out << QString(timeFieldWidth + typeFieldWidth + categoryFieldWidth,' ');
          }
          out << textLines.at(index) << Qt::endl;
        }
      }

      file.close();
    }
  }
}
