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

#include "appdlgterminate.h"
#include "aboutdlgabout.h"
#include "syslogwndlogger.h"
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

namespace app
{

  DlgTerminate::DlgTerminate(QWidget *parent) : QDialog(parent)
  {
    QLabel* lblError = new QLabel(tr("A fatal error ocurred during initialization of <b>Impresario</b>. The application needs to be terminated."));
    lblError->setWordWrap(true);
    lblError->setTextFormat(Qt::RichText);
    QVBoxLayout* groupTextLayout = new QVBoxLayout();
    groupTextLayout->addWidget(lblError);
    QGroupBox* groupText = new QGroupBox();
    groupText->setLayout(groupTextLayout);

    QDialogButtonBox* buttonBox = new QDialogButtonBox();
    QPushButton* btnTerminate = new QPushButton(QIcon(":/icons/resources/quit.png"),tr("Quit"));
    QPushButton* btnAbout = new QPushButton(tr("About..."));
    buttonBox->addButton(btnAbout,QDialogButtonBox::HelpRole);
    buttonBox->addButton(btnTerminate,QDialogButtonBox::RejectRole);

    syslog::WndLogger::setIcon(syslog::WndLogger::ICO_INFO, QIcon(":/icons/resources/information.png"));
    syslog::WndLogger::setIcon(syslog::WndLogger::ICO_WARNING, QIcon(":/icons/resources/warning.png"));
    syslog::WndLogger::setIcon(syslog::WndLogger::ICO_ERROR, QIcon(":/icons/resources/error.png"));
    syslog::WndLogger::setIcon(syslog::WndLogger::ICO_SAVE, QIcon(":/icons/resources/save.png"));
    syslog::WndLogger::setIcon(syslog::WndLogger::ICO_CLEAR, QIcon(":/icons/resources/delete.png"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(groupText);
    mainLayout->addWidget(new syslog::WndLogger(this),1);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Impresario"));
    setWindowIcon(QIcon(":/icons/resources/impresario.png"));

    // set signal handlers
    connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(onAbout()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
  }

  void DlgTerminate::onAbout()
  {
    about::DlgAbout dlg(this);
    dlg.exec();
  }

}
