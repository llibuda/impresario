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

#include "stdconsoleinterface.h"
#include <QTextCharFormat>
#include <QBrush>

namespace std
{
  //-----------------------------------------------------------------------
  // Class ConsoleInterface
  //-----------------------------------------------------------------------
  ConsoleInterface& ConsoleInterface::instance()
  {
    static ConsoleInterface interface;
    return interface;
  }

  ConsoleInterface::ConsoleInterface() : editors()
  {
  }

  ConsoleInterface::~ConsoleInterface()
  {
    editors.clear();
  }

  void ConsoleInterface::receivedStdOut(char *text)
  {
    QMutexLocker lock(&(instance().mutex));
    for(QSet<ConsoleOutEdit*>::iterator it = instance().editors.begin(); it != instance().editors.end(); ++it)
    {
      (*it)->addTextThreadSafe(QString(text),Qt::black);
    }
  }

  void ConsoleInterface::receivedStdErr(char *text)
  {
    QMutexLocker lock(&(instance().mutex));
    for(QSet<ConsoleOutEdit*>::iterator it = instance().editors.begin(); it != instance().editors.end(); ++it)
    {
      (*it)->addTextThreadSafe(QString(text),Qt::red);
    }
  }

  void ConsoleInterface::registerEditor(ConsoleOutEdit* editor)
  {
    if (editor != 0)
    {
      QMutexLocker lock(&mutex);
      editors.insert(editor);
    }
  }

  //-----------------------------------------------------------------------
  // Class ConsoleOutEdit
  //-----------------------------------------------------------------------
  ConsoleOutEdit::ConsoleOutEdit(QWidget *parent) : QPlainTextEdit(parent)
  {
    connect(this,SIGNAL(addText(QString,QColor)),this,SLOT(displayText(QString,QColor)),Qt::QueuedConnection);
    ConsoleInterface::instance().registerEditor(this);
  }

  void ConsoleOutEdit::addTextThreadSafe(QString text, QColor color)
  {
    emit addText(text,color);
  }

  void ConsoleOutEdit::displayText(QString text, QColor color)
  {
    QTextCharFormat tf;
    tf = this->currentCharFormat();
    tf.setForeground(QBrush(color));
    this->setCurrentCharFormat(tf);
    this->insertPlainText(text);
    this->ensureCursorVisible();
  }
}
