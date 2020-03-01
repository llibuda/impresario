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

#include "aboutdlgabout.h"
#include "appbuildinfo.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QTextStream>
#include <QFile>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QDate>
#include <QRect>
#include <QFont>

namespace about
{

  DlgAbout::DlgAbout(QWidget *parent) : QDialog(parent), pageStack(0), pageSelector(0)
  {
    setWindowModality(Qt::WindowModal);
    // create pixmap from splash screen in resources
    // and add version, build date, and build tool information
    QPixmap pixmap = QPixmap(":information/resources/splash.png");
    QString version = tr("Version ") + app::BuildInfo::instance().versionString();
    if (app::BuildInfo::instance().isDebugVersion())
    {
      version += tr(" Debug");
    }
    QString copyRight = QString("Copyright (C) 2015-%1  Lars Libuda").arg(QDate::currentDate().year());
    QString buildDate = tr("Date: ");
    QString buildTool = tr("Tool: ");
    QPainter painter(&pixmap);
    QRect rect = pixmap.rect();
    painter.setFont(QFont("HELVETICA",10,QFont::Bold));
    int txtWidth = painter.fontMetrics().width(version);
    int x = qMax(0,(rect.width() - txtWidth) / 2);
    int y = 2 * rect.height() / 3;
    painter.drawText(x,y,version);
    txtWidth = painter.fontMetrics().width(copyRight);
    x = qMax(0,(rect.width() - txtWidth) / 2);
    y += painter.fontMetrics().height();
    painter.drawText(x,y,copyRight);
    painter.setFont(QFont("HELVETICA",10,QFont::Normal));
    int xoff = qMax(painter.fontMetrics().width(buildDate),painter.fontMetrics().width(buildTool));
    int yoff = painter.fontMetrics().height();
    x = 5;
    y = rect.height() - yoff - 2;
    painter.drawText(x,y,buildDate);
    painter.drawText(x + xoff,y,app::BuildInfo::instance().buildDate());
    y = rect.height() - 2;
    painter.drawText(x,y,buildTool);
    painter.drawText(x + xoff,y,app::BuildInfo::instance().compiler());

    // create label with pixmap for display in dialog
    QLabel* information = new QLabel();
    information->setPixmap(pixmap);
    information->setAlignment(Qt::AlignCenter);

    // create license information page for display in dialog
    QFile* file = new QFile(":information/resources/license.txt");
    file->open(QFile::ReadOnly);
    QTextStream* stream = new QTextStream(file);
    QString txtLicense = stream->readAll();
    file->close();
    delete file;
    delete stream;
    QTextEdit* license = new QTextEdit(txtLicense);
    license->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);

    // create credits information page for display in dialog
    file = new QFile(":information/resources/credits.txt");
    file->open(QFile::ReadOnly);
    stream = new QTextStream(file);
    QString txtCredits = stream->readAll();
    file->close();
    delete file;
    delete stream;
    QTextEdit* credits = new QTextEdit(txtCredits);
    credits->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);

    // create combo box for page selection
    pageSelector = new QComboBox(this);
    pageSelector->setEditable(false);
    pageSelector->addItem(tr("Information"));
    pageSelector->addItem(tr("Impresario license"));
    pageSelector->addItem(tr("Credits"));

    // add pages to stack widget
    pageStack = new QStackedWidget(this);
    pageStack->addWidget(information);
    pageStack->addWidget(license);
    pageStack->addWidget(credits);

    // create layout of dialog
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok,Qt::Horizontal,this);
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(pageSelector);
    layout->addWidget(pageStack,1);
    layout->addWidget(buttons);
    setLayout(layout);
    setWindowTitle(tr("About Impresario"));
    setFixedSize(sizeHint());

    // add signal processing
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(pageSelector,SIGNAL(currentIndexChanged(int)),pageStack,SLOT(setCurrentIndex(int)));
  }

}
