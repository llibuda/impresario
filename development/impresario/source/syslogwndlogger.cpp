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
**   along with Impresario in subdirectory "licenses", file "LICENSE.GPLv3".
**   If not, see <http://www.gnu.org/licenses/>.
******************************************************************************************/

#include "syslogwndlogger.h"
#include "resources.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QContextMenuEvent>
#include <QHeaderView>

namespace syslog
{

  WndLogger::WndLogger(QWidget *parent) : QWidget(parent), tbFilter(0), tbActions(0), logView(0), menu(0)
  {
    tbFilter = new QToolBar(this);
    tbFilter->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    tbFilter->addAction(Resource::action(Resource::SYSLOG_FILTERERRORS));
    tbFilter->addAction(Resource::action(Resource::SYSLOG_FILTERWARNINGS));
    tbFilter->addAction(Resource::action(Resource::SYSLOG_FILTERMESSAGES));
    tbActions = new QToolBar(this);
    tbActions->addAction(Resource::action(Resource::SYSLOG_SAVE));
    tbActions->addAction(Resource::action(Resource::SYSLOG_CLEAR));

    // set up actual widget for log message visualization
    logView = new QTreeView(this);
    logView->setRootIsDecorated(false);
    logView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QSortFilterProxyModel* model = new QSortFilterProxyModel(this);
    model->setSourceModel(&Logger::instance());
    model->setFilterRegExp("[IEW]");
    model->setFilterKeyColumn(1);
    model->setDynamicSortFilter(true);
    logView->setModel(model);
    logView->hideColumn(1);
    logView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    logView->header()->hide();
    logView->setSelectionMode(QAbstractItemView::NoSelection);

    // create layout of WndLogger
    QHBoxLayout* tbLayout = new QHBoxLayout();
    tbLayout->setContentsMargins(0,0,0,0);
    tbLayout->setSpacing(0);
    tbLayout->addWidget(tbFilter);
    tbLayout->addWidget(tbActions,1);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addLayout(tbLayout);
    layout->addWidget(logView,1);
    this->setLayout(layout);

    // create context menu
    menu = new QMenu(this);
    menu->addAction(Resource::action(Resource::SYSLOG_FILTERERRORS));
    menu->addAction(Resource::action(Resource::SYSLOG_FILTERWARNINGS));
    menu->addAction(Resource::action(Resource::SYSLOG_FILTERMESSAGES));
    menu->addSeparator();
    menu->addAction(Resource::action(Resource::SYSLOG_SAVE));
    menu->addAction(Resource::action(Resource::SYSLOG_CLEAR));

    // connect actions
    connect(Resource::action(Resource::SYSLOG_SAVE),SIGNAL(triggered()),&Logger::instance(),SLOT(saveLog()));
    connect(Resource::action(Resource::SYSLOG_CLEAR),SIGNAL(triggered()),&Logger::instance(),SLOT(clearLog()));
    connect(Resource::action(Resource::SYSLOG_FILTERERRORS),SIGNAL(toggled(bool)),this,SLOT(toggleFilterError(bool)));
    connect(Resource::action(Resource::SYSLOG_FILTERWARNINGS),SIGNAL(toggled(bool)),this,SLOT(toggleFilterWarning(bool)));
    connect(Resource::action(Resource::SYSLOG_FILTERMESSAGES),SIGNAL(toggled(bool)),this,SLOT(toggleFilterMessage(bool)));
    connect(&Logger::instance(),SIGNAL(changedMsgCount(Logger::MsgType,uint)),this,SLOT(updateUI(Logger::MsgType,uint)));

    updateUI(Logger::Information,Logger::instance().getMessageCount(Logger::Information));
    updateUI(Logger::Warning,Logger::instance().getMessageCount(Logger::Warning));
    updateUI(Logger::Error,Logger::instance().getMessageCount(Logger::Error));
  }

  WndLogger::~WndLogger()
  {
    delete menu;
    menu = 0;
  }

  void WndLogger::contextMenuEvent(QContextMenuEvent* event)
  {
    menu->popup(event->globalPos());
  }

  void WndLogger::updateUI(Logger::MsgType type, unsigned int count)
  {
    QString text;
    switch(type)
    {
    case Logger::Information:
      if (count == 1)
      {
        Resource::action(Resource::SYSLOG_FILTERMESSAGES)->setIconText(tr("1 Message"));
      }
      else
      {
        text = QString(tr("%1 Messages")).arg(count);
        Resource::action(Resource::SYSLOG_FILTERMESSAGES)->setIconText(text);
      }
      break;
    case Logger::Warning:
      if (count == 1)
      {
        Resource::action(Resource::SYSLOG_FILTERWARNINGS)->setIconText(tr("1 Warning"));
      }
      else
      {
        text = QString(tr("%1 Warnings")).arg(count);
        Resource::action(Resource::SYSLOG_FILTERWARNINGS)->setIconText(text);
      }
      break;
    case Logger::Error:
      if (count == 1)
      {
        Resource::action(Resource::SYSLOG_FILTERERRORS)->setIconText(tr("1 Error"));
      }
      else
      {
        text = QString(tr("%1 Errors")).arg(count);
       Resource::action(Resource::SYSLOG_FILTERERRORS)->setIconText(text);
      }
      break;
    }
    Resource::action(Resource::SYSLOG_CLEAR)->setEnabled(Logger::instance().rowCount()>0);
    Resource::action(Resource::SYSLOG_SAVE)->setEnabled(Logger::instance().rowCount()>0);
    logView->scrollToBottom();
  }

  void WndLogger::toggleFilterError(bool checked)
  {
    QSortFilterProxyModel* model = static_cast<QSortFilterProxyModel*>(logView->model());
    QString filter = model->filterRegExp().pattern();
    QChar id = QChar(Logger::Error);
    if (checked)
    {
      if (!filter.contains(id))
      {
        filter.insert(1,id);
      }
    }
    else
    {
      filter.remove(id);
    }
    model->setFilterRegExp(filter);
  }

  void WndLogger::toggleFilterWarning(bool checked)
  {
    QSortFilterProxyModel* model = static_cast<QSortFilterProxyModel*>(logView->model());
    QString filter = model->filterRegExp().pattern();
    QChar id = QChar(Logger::Warning);
    if (checked)
    {
      if (!filter.contains(id))
      {
        filter.insert(1,id);
      }
    }
    else
    {
      filter.remove(id);
    }
    model->setFilterRegExp(filter);
  }

  void WndLogger::toggleFilterMessage(bool checked)
  {
    QSortFilterProxyModel* model = static_cast<QSortFilterProxyModel*>(logView->model());
    QString filter = model->filterRegExp().pattern();
    QChar id = QChar(Logger::Information);
    if (checked)
    {
      if (!filter.contains(id))
      {
        filter.insert(1,id);
      }
    }
    else
    {
      filter.remove(id);
    }
    model->setFilterRegExp(filter);
  }

}
