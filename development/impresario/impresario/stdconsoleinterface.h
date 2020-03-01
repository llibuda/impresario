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
#ifndef STDCONSOLEINTERFACE_H
#define STDCONSOLEINTERFACE_H

#include <QPlainTextEdit>
#include <QMutex>
#include <QSet>
#include <QColor>

namespace std
{
  class ConsoleOutEdit;

  class ConsoleInterface
  {
  public:
    static ConsoleInterface& instance();
    static void receivedStdOut(char* text);
    static void receivedStdErr(char* text);

    void registerEditor(ConsoleOutEdit* editor);

  private:
    ConsoleInterface();
    virtual ~ConsoleInterface();

    QSet<ConsoleOutEdit*> editors;
    QMutex                mutex;
  };

  class ConsoleOutEdit : public QPlainTextEdit
  {
    Q_OBJECT
  public:
    explicit ConsoleOutEdit(QWidget *parent = 0);

    void addTextThreadSafe(QString text, QColor color);

  signals:
    void addText(QString text, QColor color);

  private slots:
    void displayText(QString text, QColor color);
  };

}
#endif // STDCONSOLEINTERFACE_H
