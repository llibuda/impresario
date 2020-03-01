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

#include "framestatusbar.h"
#include <QPixmap>

namespace frame
{

  StatusBar::StatusBar(QWidget *parent) : QStatusBar(parent), lblUpdateView(0), progressBar(0)
  {
    lblUpdateView = new QLabel(this);
    lblUpdateView->setPixmap(QPixmap(":/icons/resources/dbview_grey.png"));
    lblUpdateView->setToolTip(tr("Indicator for database view update"));
    addPermanentWidget(lblUpdateView);
    progressBar = new QProgressBar(this);
  }

  void StatusBar::indicateViewUpdate(db::Model::ModelUpdateReason /*reason*/)
  {
    lblUpdateView->setPixmap(QPixmap(":/icons/resources/dbview.png"));
    lblUpdateView->setToolTip(tr("Macro database view is updated..."));
  }

  void StatusBar::unindicateViewUpdate(int /*successful*/)
  {
    lblUpdateView->setPixmap(QPixmap(":/icons/resources/dbview_grey.png"));
    lblUpdateView->setToolTip(tr("Indicator for database view update"));
  }

  void StatusBar::showProgressBar()
  {
    insertPermanentWidget(0,progressBar,1);
    progressBar->setMaximumHeight(lblUpdateView->height());
  }

  void StatusBar::hideProgressBar()
  {
    removeWidget(progressBar);
  }

  void StatusBar::updateProgress(int current, int total, const QString& format)
  {
    if (current == 0)
    {
      progressBar->setRange(current,total);
      if (!format.isEmpty())
      {
        progressBar->setFormat(format);
        progressBar->setTextVisible(true);
      }
    }
    else
    {
      progressBar->setValue(current);
    }
  }

}
