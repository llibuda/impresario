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
#ifndef SYSLOGLOGGER_H
#define SYSLOGLOGGER_H

#include <QObject>
#include <QDateTime>
#include <QMap>
#include <QSet>
#include <QVector>

namespace syslog
{

  void info(const QString& msg, const QString& category = QString());
  void warning(const QString& msg, const QString& category = QString());
  void error(const QString& msg, const QString& category = QString());

  class Logger : public QObject
  {
    Q_OBJECT
  public:
    enum MsgType
    {
      Information = 'I',
      Warning = 'W',
      Error = 'E'
    };

    struct LogEntry
    {
      QDateTime timeStamp;
      MsgType   msgType;
      QString   message;
      QString   category;
    };

    void write(MsgType type, const QString& msg, const QString& category = QString());

    int getMessageCount(MsgType type) const
    {
      return stats[type];
    }

    int getMessageCount() const
    {
      return messages.count();
    }

    const LogEntry& getMessage(int pos) const
    {
      Q_ASSERT(pos >= 0 && pos < messages.count());
      return messages[pos];
    }

    static Logger& instance();

  signals:
    void changedMsgCount(Logger::MsgType type, int countType, int countTotal);
    void newLogEntry(Logger::MsgType type, QString msg, QString category);

  public slots:
    void clear();
    void save(const QString& fileName);

  private slots:
    void createNewLogEntry(Logger::MsgType type, const QString msg, const QString category);

  private:
    Q_DISABLE_COPY(Logger)

    Logger(QObject *parent = nullptr);

    static int idMsgType;

    typedef QVector<LogEntry> MsgList;
    typedef QMap<MsgType,int> MsgStats;
    typedef QSet<QString>     MsgCategories;

    MsgList       messages;
    MsgCategories categories;
    MsgStats      stats;
    int           maxCategoryLength;
  };

}
#endif // SYSLOGLOGGER_H
