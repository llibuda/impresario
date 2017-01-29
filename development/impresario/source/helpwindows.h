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
#ifndef HELPCONTENTWINDOW_H
#define HELPCONTENTWINDOW_H

#include <QtWebKitWidgets>
#include <QtHelp>
#include <QMainWindow>

namespace help
{
  class ContentWindow : public QWebView
  {
  public:
    ContentWindow(QHelpEngine& helpEngine, QWidget* parent = 0);
    ~ContentWindow();

    QFont viewerFont() const;
    void setViewerFont(const QFont &font);    
  };

  class MainWindow : public QMainWindow
  {
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)
  public:
    MainWindow(QHelpEngine& helpEngine);
    ~MainWindow();

  private slots:
    void runSearch();
    void showPage(const QUrl& url);

  private:
    QHelpEngine&   helpEngineInstance;
    ContentWindow* ptrBrowser;
  };

}
#endif // HELPCONTENTWINDOW_H
