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
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QSortFilterProxyModel>

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

  class DlgTopicSelection : public QDialog
  {
    Q_OBJECT
  public:
    DlgTopicSelection(const QMap<QString,QUrl>& helpTopics, const QString& keyword, QWidget* parent);

    QUrl link() const
    {
      return url;
    }

  private slots:
    void setTopicFilter(const QString& filter);
    void topicActivated(const QModelIndex& index);
    void selectTopic();

  protected:
    virtual bool eventFilter(QObject *object, QEvent *event);

  private:
    QUrl                   url;
    QLabel*                lblTopic;
    QLineEdit*             edtFilter;
    QListView*             lvTopics;
    QSortFilterProxyModel* mdlFilter;
  };

  class HelpIndexWidget : public QWidget
  {
    Q_OBJECT

  public:
    HelpIndexWidget(QHelpEngine& helpEngine, QWidget *parent = 0);
    ~HelpIndexWidget();

    void setSearchLineEditText(const QString &text);
    QString searchLineEditText() const
    {
      return edtSearch->text();
    }

  signals:
    void linkActivated(const QUrl &link, const QString &keyword);
    void linksActivated(const QMap<QString, QUrl> &links, const QString &keyword);
    void escapePressed();

  private slots:
    void filterIndices(const QString &filter);
    void enableSearchLineEdit();
    void disableSearchLineEdit();

  private:
    bool eventFilter(QObject *obj, QEvent *e);
    void focusInEvent(QFocusEvent *e);
    void open(QHelpIndexWidget *indexWidget, const QModelIndex &index);

    QLineEdit *edtSearch;
    QHelpIndexWidget *lvIndex;
  };

  class MainWindow : public QMainWindow
  {
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)
  public:
    MainWindow(QHelpEngine& helpEngine);
    ~MainWindow();

  public slots:
    void runSearch();
    void showPage(const QUrl& url);
    void showPage(const QUrl& url, const QString& keyword);
    void selectTopic(const QMap<QString,QUrl>& topicList, const QString& keyword);

  protected:
    virtual void closeEvent(QCloseEvent* event);

  private:
    static const QString keyHelpWndGeometry;
    static const QString keyHelpWndState;

    QHelpEngine&   helpEngineInstance;
    ContentWindow* ptrBrowser;
  };

}
#endif // HELPCONTENTWINDOW_H
