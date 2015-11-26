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
#ifndef FRAMESTATUSBAR_H
#define FRAMESTATUSBAR_H

#include "dbmodel.h"
#include <QStatusBar>
#include <QLabel>

namespace frame
{

  class StatusBar : public QStatusBar
  {
    Q_OBJECT
  public:
    explicit StatusBar(QWidget *parent = 0);

  signals:

  public slots:
    void indicateViewUpdate(db::Model::ModelUpdateReason reason);
    void unindicateViewUpdate(int level);

  private:
    QLabel* lblUpdateView;

  };

}
#endif // FRAMESTATUSBAR_H
