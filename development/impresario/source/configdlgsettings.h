/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2016  Lars Libuda
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
#ifndef CONFIGDLGSETTINGS_H
#define CONFIGDLGSETTINGS_H

#include <QDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStackedWidget>
#include <QWidget>
#include <QMap>
#include <QPair>

namespace config
{
  // forward declaration to prevent circular includes
  class DlgPageBase;

  class DlgSettings : public QDialog
  {
    Q_OBJECT
  public:
    enum DlgPage
    {
      DirGeneral = 0,
      DirProcessGraphs,
      DirMacroLibs,
      DirDependencies,
      DirResources,
      MacroDBGeneral,
      MacroDBView,
      MacroDBFilter,
      EditorPropertyWnd
    };

    explicit DlgSettings(QWidget *parent = 0, DlgPage startPage = DirGeneral);

  private slots:
    void changePage(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void changePage(DlgSettings::DlgPage page);
    void saveSettings();

  private:
    typedef QMap<DlgPage, QPair<QTreeWidgetItem*, DlgPageBase*> > PageMap;

    PageMap         pageMap;
    QTreeWidget*    selectionPane;
    QStackedWidget* contentPane;
  };
}
#endif // CONFIGDLGSETTINGS_H
