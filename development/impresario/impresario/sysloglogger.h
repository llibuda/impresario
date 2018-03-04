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

#include <QStandardItemModel>
#include <QMap>
#include <QIcon>
#include <QMutex>
#include <QFile>

namespace syslog
{

  void info(const QString& msg);
  void warning(const QString& msg);
  void error(const QString& msg);

  class Logger : public QStandardItemModel
  {
    Q_OBJECT
  public:
    enum MsgType
    {
      Information = 'I',
      Warning = 'W',
      Error = 'E'
    };

    void write(MsgType type, const QString& msg);

    int getMessageCount(MsgType type)
    {
      return stat[type];
    }

    static Logger& instance();

  signals:
    void changedMsgCount(Logger::MsgType type, int countType, int countTotal);

  public slots:
    void clear();
    void save();

  private:
    Logger(QObject *parent = 0);
    Logger& operator=(const Logger&) { return *this; }
    virtual ~Logger() {}

    static int idMsgType;
    static int idVectorInt;

    static const int SaveTimeStampRole = Qt::UserRole + 3;

    typedef QMap<MsgType,int> MsgStats;

    MsgStats       stat;
    QIcon          icoError;
    QIcon          icoWarning;
    QIcon          icoInfo;
    mutable QMutex mutex;

  };

}
#endif // SYSLOGLOGGER_H
