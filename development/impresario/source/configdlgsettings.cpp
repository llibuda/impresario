/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2017  Lars Libuda
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

#include "configdlgsettings.h"
#include "configdlgpages.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QHeaderView>
#include <QSplitter>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>

namespace config
{
  DlgSettings::DlgSettings(QWidget *parent, DlgPage startPage) : QDialog(parent)
  {
    // create layout
    selectionPane = new QTreeWidget;
    selectionPane->header()->hide();
    contentPane = new QStackedWidget;

    QSplitter* horizontalLayout = new QSplitter(this);
    horizontalLayout->setChildrenCollapsible(false);
    horizontalLayout->addWidget(selectionPane);
    horizontalLayout->addWidget(contentPane);
    horizontalLayout->setStretchFactor(0,30);
    horizontalLayout->setStretchFactor(1,70);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(horizontalLayout,1);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Impresario Settings"));
    setWindowIcon(QIcon(":/icons/resources/settings.png"));

    // set signal handlers
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect(selectionPane,SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),this, SLOT(changePage(QTreeWidgetItem*,QTreeWidgetItem*)));

    // create dialog pages
    QTreeWidgetItem* dlgItemRoot;
    QTreeWidgetItem* dlgItem;
    DlgPageBase* dlgPage;

    dlgItemRoot = new QTreeWidgetItem(selectionPane,DirGeneral);
    dlgItemRoot->setIcon(0,QIcon(":/icons/resources/cfgfolders.png"));
    dlgItemRoot->setText(0,tr("Directories"));
    dlgItemRoot->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    dlgPage = new DlgPageDirectories(this);
    contentPane->addWidget(dlgPage);
    pageMap[DirGeneral] = qMakePair(dlgItemRoot,dlgPage);

    dlgItem = new QTreeWidgetItem(dlgItemRoot,DirProcessGraphs);
    dlgItem->setIcon(0,QIcon(":/icons/resources/cfgfolderpg.png"));
    dlgItem->setText(0,tr("Process Graphs"));
    dlgItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    dlgPage = new DlgPageDirProcessGraphs(this);
    contentPane->addWidget(dlgPage);
    pageMap[DirProcessGraphs] = qMakePair(dlgItem,dlgPage);

    dlgItem = new QTreeWidgetItem(dlgItemRoot,DirMacroLibs);
    dlgItem->setIcon(0,QIcon(":/icons/resources/cfgfoldermacros.png"));
    dlgItem->setText(0,tr("Macro Libraries"));
    dlgItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    dlgPage = new DlgPageDirMacroLibs(this);
    contentPane->addWidget(dlgPage);
    pageMap[DirMacroLibs] = qMakePair(dlgItem,dlgPage);

    dlgItem = new QTreeWidgetItem(dlgItemRoot,DirDependencies);
    dlgItem->setIcon(0,QIcon(":/icons/resources/cfgfolderdep.png"));
    dlgItem->setText(0,tr("Dependent Libraries"));
    dlgItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    dlgPage = new DlgPageDirDependencies(this);
    contentPane->addWidget(dlgPage);
    pageMap[DirDependencies] = qMakePair(dlgItem,dlgPage);

    dlgItem = new QTreeWidgetItem(dlgItemRoot,DirDocumentation);
    dlgItem->setIcon(0,QIcon(":/icons/resources/cfgfolderdoc.png"));
    dlgItem->setText(0,tr("Documentation"));
    dlgItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    dlgPage = new DlgPageDirDocumentation(this);
    contentPane->addWidget(dlgPage);
    pageMap[DirDocumentation] = qMakePair(dlgItem,dlgPage);

    dlgItem = new QTreeWidgetItem(dlgItemRoot,DirResources);
    dlgItem->setIcon(0,QIcon(":/icons/resources/cfgfolderres.png"));
    dlgItem->setText(0,tr("Resources"));
    dlgItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    dlgPage = new DlgPageDirResources(this);
    contentPane->addWidget(dlgPage);
    pageMap[DirResources] = qMakePair(dlgItem,dlgPage);

    dlgItemRoot = new QTreeWidgetItem(selectionPane,MacroDBGeneral);
    dlgItemRoot->setIcon(0,QIcon(":/icons/resources/db.png"));
    dlgItemRoot->setText(0,tr("Macro Database"));
    dlgItemRoot->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    dlgPage = new DlgPageViewMacroDB(this);
    contentPane->addWidget(dlgPage);
    pageMap[MacroDBGeneral] = qMakePair(dlgItemRoot,dlgPage);

    dlgItem = new QTreeWidgetItem(dlgItemRoot,MacroDBView);
    dlgItem->setIcon(0,QIcon(":/icons/resources/dbview.png"));
    dlgItem->setText(0,tr("View"));
    dlgItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    dlgPage = new DlgPageViewFormat(this);
    contentPane->addWidget(dlgPage);
    pageMap[MacroDBView] = qMakePair(dlgItem,dlgPage);

    dlgItem = new QTreeWidgetItem(dlgItemRoot,MacroDBFilter);
    dlgItem->setIcon(0,QIcon(":/icons/resources/dbfilter.png"));
    dlgItem->setText(0,tr("Filter"));
    dlgItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    dlgPage = new DlgPageViewFilter(this);
    contentPane->addWidget(dlgPage);
    pageMap[MacroDBFilter] = qMakePair(dlgItem,dlgPage);

    dlgItemRoot = new QTreeWidgetItem(selectionPane,EditorPropertyWnd);
    dlgItemRoot->setIcon(0,QIcon(":/icons/resources/property_wnd.png"));
    dlgItemRoot->setText(0,tr("Property Window"));
    dlgItemRoot->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    dlgPage = new DlgPagePropertyWnd(this);
    contentPane->addWidget(dlgPage);
    pageMap[EditorPropertyWnd] = qMakePair(dlgItemRoot,dlgPage);

    // load settings for dialog pages
    for(PageMap::Iterator it = pageMap.begin(); it != pageMap.end(); ++it)
    {
      (*it).second->loadSettings();
    }

    // set initial page to show
    selectionPane->expandAll();
    changePage(startPage);
  }

  void DlgSettings::changePage(QTreeWidgetItem *current, QTreeWidgetItem *previous)
  {
    if (!current)
    {
      current = previous;
    }
    contentPane->setCurrentWidget(pageMap[static_cast<DlgPage>(current->type())].second);
  }

  void DlgSettings::changePage(DlgSettings::DlgPage page)
  {
    // the next call will select the appropriate item in the selectionPane.
    // the method will then emit a signal which leads to the call of
    // changePage(QTreeWidgetItem *current, QTreeWidgetItem *previous) which in turn
    // activates the corresponding dialog page
    selectionPane->setCurrentItem(pageMap[page].first);
  }

  void DlgSettings::saveSettings()
  {
    // save settings for dialog pages
    bool validatedSuccessfully = true;
    bool closeAndSave = false;
    QStringList details;
    for(PageMap::Iterator it = pageMap.begin(); it != pageMap.end(); ++it)
    {
      validatedSuccessfully = (*it).second->validateSettings(details) && validatedSuccessfully;
    }
    if (!validatedSuccessfully)
    {
      QMessageBox msgBox(this);
      msgBox.setWindowTitle(tr("Impresario Settings"));
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setText(tr("Some settings have invalid values and will not be stored. See details."));
      msgBox.setInformativeText(tr("Click Ok to ignore invalid settings and close the dialog. Click Cancel to change the settings."));
      msgBox.setDetailedText(details.join("\n"));
      msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Ok);
      if (msgBox.exec() == QMessageBox::Ok)
      {
        closeAndSave = true;
      }
    }
    else
    {
      closeAndSave = true;
    }
    if (closeAndSave)
    {
      this->close();
      for(PageMap::Iterator it = pageMap.begin(); it != pageMap.end(); ++it)
      {
        (*it).second->saveSettings();
      }
    }
  }
}
