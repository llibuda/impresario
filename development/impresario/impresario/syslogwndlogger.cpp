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

#include "syslogwndlogger.h"
#include "resources.h"
#include <QApplication>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QFileDialog>
#include <QDate>
#include <QTextOption>
#include <QTextLayout>

namespace syslog
{
  //-----------------------------------------------------------------------
  // Class LoggerModel
  //-----------------------------------------------------------------------
  LoggerModel::LoggerModel(Logger* loggerInstance, QObject* parent) : QAbstractItemModel (parent), logger(0), countRows(0),
    icoError(":/icons/resources/error.png"),
    icoWarning(":/icons/resources/warning.png"),
    icoInfo(":/icons/resources/information.png")
  {
    attachLogger(loggerInstance);
  }

  LoggerModel::~LoggerModel()
  {
  }

  void LoggerModel::attachLogger(Logger* loggerInstance)
  {
    beginResetModel();
    if (logger != 0)
    {
      disconnect(logger,&Logger::changedMsgCount,this,&LoggerModel::update);
      logger = 0;
      countRows = 0;
    }
    logger = loggerInstance;
    if (logger != 0)
    {
      countRows = logger->getMessageCount();
      connect(logger,&Logger::changedMsgCount,this,&LoggerModel::update);
    }
    endResetModel();
  }

  QVariant LoggerModel::data(const QModelIndex &index, int role) const
  {
    Q_ASSERT(logger != 0);
    if (role == Qt::DisplayRole)
    {
      const Logger::LogEntry& entry = logger->getMessage(index.row());
      switch(index.column())
      {
        case 0: return entry.timeStamp.toString("hh:mm:ss.zzz");
        case 1: return QChar(entry.msgType);
        case 2: return entry.category;
        case 3: return entry.message;
      }
    }
    else if (role == Qt::DecorationRole && index.column() == 0)
    {
      const Logger::LogEntry& entry = logger->getMessage(index.row());
      switch(entry.msgType)
      {
        case Logger::Information: return icoInfo;
        case Logger::Warning: return icoWarning;
        case Logger::Error: return icoError;
      }
    }
    return QVariant();
  }

  Qt::ItemFlags LoggerModel::flags(const QModelIndex & /*index*/) const
  {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  QVariant LoggerModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
  {
    if (role == Qt::DisplayRole)
    {
      switch(section)
      {
        case 0: return QObject::tr("Time");
        case 1: return QObject::tr("Type");
        case 2: return QObject::tr("Category");
        case 3: return QObject::tr("Message");
      }
    }
    return QVariant();
  }

  QModelIndex LoggerModel::index(int row, int column, const QModelIndex &parent) const
  {
    if (!hasIndex(row,column,parent)) return QModelIndex();
    return createIndex(row,column);
  }

  QModelIndex LoggerModel::parent(const QModelIndex & /*index*/) const
  {
    return QModelIndex();
  }

  int LoggerModel::rowCount(const QModelIndex & /*parent*/) const
  {
    return countRows;
  }

  int LoggerModel::columnCount(const QModelIndex & /*parent*/) const
  {
    return 4;
  }

  void LoggerModel::update(Logger::MsgType /*type*/, int /*typeCount*/, int totalCount)
  {
    if (totalCount == 0 && countRows > 0)
    {
      beginResetModel();
      countRows = 0;
      endResetModel();
    }
    else
    {
      beginInsertRows(QModelIndex(),countRows,totalCount - 1);
      countRows = totalCount;
      endInsertRows();
    }
  }

  //-----------------------------------------------------------------------
  // Class LoggerItemDelegate
  //-----------------------------------------------------------------------

  LoggerItemDelegate::LoggerItemDelegate(QObject *parent) : QStyledItemDelegate(parent), sectionSizes(), wrappingEnabled(false)
  {
    QTreeView* view = qobject_cast<QTreeView*>(parent);
    if (view != nullptr)
    {
      QHeaderView* header = view->header();
      if (header != nullptr)
      {
        connect(header,&QHeaderView::sectionResized,this,&LoggerItemDelegate::sectionResized);
        wrappingEnabled = true;
      }
    }
  }

  LoggerItemDelegate::~LoggerItemDelegate()
  {
  }

  QSize LoggerItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
    if (wrappingEnabled && index.isValid() && index.column() == index.model()->columnCount() - 1)
    {
      QStyleOptionViewItem opt = option;
      initStyleOption(&opt, index);
      QTextOption textOption;
      textOption.setWrapMode(QTextOption::WordWrap);
      textOption.setTextDirection(opt.direction);
      textOption.setAlignment(QStyle::visualAlignment(opt.direction, opt.displayAlignment));
      QTextLayout textLayout(opt.text, opt.font);
      textLayout.setTextOption(textOption);
      qreal height = 0;
      qreal widthUsed = 0;
      textLayout.beginLayout();
      while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
          break;
        line.setLineWidth(sectionSizes[index.column()]);
        line.setPosition(QPointF(0, height));
        height += line.height();
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
      }
      textLayout.endLayout();
      return QSize(int(widthUsed),int(height));
      /*
      QSize baseSize(sectionSizes[index.column()],10000);
      QRect outRect = option.fontMetrics.boundingRect(QRect(QPoint(0, 0), baseSize), option.displayAlignment | Qt::TextWordWrap, index.data().toString());
      baseSize.setHeight(outRect.height() + 2);
      return baseSize;
      */
    }
    else
    {
      return QStyledItemDelegate::sizeHint(option, index);
    }
  }

  void LoggerItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    if (wrappingEnabled)
    {
      opt.decorationAlignment = (option.decorationAlignment & Qt::AlignHorizontal_Mask) | Qt::AlignTop;
      opt.displayAlignment = (option.displayAlignment & Qt::AlignHorizontal_Mask) | Qt::AlignTop;
    }
    QStyle *style = QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, nullptr);
  }

  void LoggerItemDelegate::sectionResized(int logicalIndex, int /*oldSize*/, int newSize)
  {
    sectionSizes[logicalIndex] = newSize;
    emit sizeHintChanged(QModelIndex());
  }

  //-----------------------------------------------------------------------
  // Class WndLogger
  //-----------------------------------------------------------------------

  WndLogger::WndLogger(Logger* loggerInstance, QWidget *parent) : QWidget(parent), logView(0), menu(0)
  {
    Q_ASSERT(loggerInstance != 0);
    QToolBar* tbFilter = new QToolBar(this);
    tbFilter->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    tbFilter->addAction(Resource::action(Resource::SYSLOG_FILTERERRORS));
    tbFilter->addAction(Resource::action(Resource::SYSLOG_FILTERWARNINGS));
    tbFilter->addAction(Resource::action(Resource::SYSLOG_FILTERMESSAGES));
    QToolBar* tbActions = new QToolBar(this);
    tbActions->addAction(Resource::action(Resource::SYSLOG_SAVE));
    tbActions->addAction(Resource::action(Resource::SYSLOG_CLEAR));

    // set up model for logger
    LoggerModel* loggerModel = new LoggerModel(loggerInstance,this);

    // set up actual widget for log message visualization
    QSortFilterProxyModel* model = new QSortFilterProxyModel(this);
    model->setFilterRegExp("[IEW]");
    model->setFilterKeyColumn(1);
    model->setDynamicSortFilter(true);
    model->setSourceModel(loggerModel);

    logView = new QTreeView(this);
    logView->setRootIsDecorated(false);
    logView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logView->setAllColumnsShowFocus(true);
    logView->setWordWrap(true);
    logView->setModel(model);
    logView->hideColumn(1);
    logView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    logView->header()->hide();
    logView->setSelectionMode(QAbstractItemView::NoSelection);
    logView->setItemDelegate(new LoggerItemDelegate(logView));

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
    connect(this,&WndLogger::destroyed,menu,&QMenu::deleteLater);

    // connect actions
    connect(Resource::action(Resource::SYSLOG_SAVE),&QAction::triggered,this,&WndLogger::saveLog);
    connect(Resource::action(Resource::SYSLOG_CLEAR),&QAction::triggered,this,&WndLogger::clearLog);
    connect(Resource::action(Resource::SYSLOG_FILTERERRORS),&QAction::toggled,this,&WndLogger::toggleFilterError);
    connect(Resource::action(Resource::SYSLOG_FILTERWARNINGS),&QAction::toggled,this,&WndLogger::toggleFilterWarning);
    connect(Resource::action(Resource::SYSLOG_FILTERMESSAGES),&QAction::toggled,this,&WndLogger::toggleFilterMessage);
    connect(&Logger::instance(),&Logger::changedMsgCount,this,&WndLogger::updateUI);

    updateUI(Logger::Error,0,0);
  }

  WndLogger::~WndLogger()
  {
  }

  void WndLogger::contextMenuEvent(QContextMenuEvent* event)
  {
    menu->popup(event->globalPos());
  }

  void WndLogger::updateUI(Logger::MsgType type, int typeCount, int totalCount)
  {
    QString text;
    Resource::action(Resource::SYSLOG_CLEAR)->setEnabled(totalCount>0);
    Resource::action(Resource::SYSLOG_SAVE)->setEnabled(totalCount>0);
    if (totalCount == 0 && typeCount == 0)
    {
      text = QString(tr("%1 Messages")).arg(0);
      Resource::action(Resource::SYSLOG_FILTERMESSAGES)->setIconText(text);
      text = QString(tr("%1 Warnings")).arg(0);
      Resource::action(Resource::SYSLOG_FILTERWARNINGS)->setIconText(text);
      text = QString(tr("%1 Errors")).arg(0);
      Resource::action(Resource::SYSLOG_FILTERERRORS)->setIconText(text);
      return;
    }
    switch(type)
    {
    case Logger::Information:
      if (typeCount == 1)
      {
        Resource::action(Resource::SYSLOG_FILTERMESSAGES)->setIconText(tr("1 Message"));
      }
      else
      {
        text = QString(tr("%1 Messages")).arg(typeCount);
        Resource::action(Resource::SYSLOG_FILTERMESSAGES)->setIconText(text);
      }
      break;
    case Logger::Warning:
      if (typeCount == 1)
      {
        Resource::action(Resource::SYSLOG_FILTERWARNINGS)->setIconText(tr("1 Warning"));
      }
      else
      {
        text = QString(tr("%1 Warnings")).arg(typeCount);
        Resource::action(Resource::SYSLOG_FILTERWARNINGS)->setIconText(text);
      }
      break;
    case Logger::Error:
      if (typeCount == 1)
      {
        Resource::action(Resource::SYSLOG_FILTERERRORS)->setIconText(tr("1 Error"));
      }
      else
      {
        text = QString(tr("%1 Errors")).arg(typeCount);
        Resource::action(Resource::SYSLOG_FILTERERRORS)->setIconText(text);
      }
      break;
    }
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

  void WndLogger::saveLog()
  {
    QString filters = tr("Log files (*.log);;All files (*.*)");
    QString defFileName = QCoreApplication::applicationName() + QDate::currentDate().toString("_yyyy-MM-dd") + ".log";
    QString filename = QFileDialog::getSaveFileName(0, tr("Save log file"),defFileName,filters,0,QFileDialog::DontConfirmOverwrite);
    if (filename.length() == 0)
    {
      return;
    }
    Logger::instance().save(filename);
  }

  void WndLogger::clearLog()
  {
    Logger::instance().clear();
  }
}
