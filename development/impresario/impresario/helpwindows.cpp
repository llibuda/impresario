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
#include "helpwindows.h"
#include "helpsystem.h"
#include <QApplication>
#include <QDebug>

namespace help
{
  struct ExtensionMap {
      const char *extension;
      const char *mimeType;
  } extensionMap[] = {
      { ".bmp", "image/bmp" },
      { ".css", "text/css" },
      { ".gif", "image/gif" },
      { ".html", "text/html" },
      { ".htm", "text/html" },
      { ".ico", "image/x-icon" },
      { ".jpeg", "image/jpeg" },
      { ".jpg", "image/jpeg" },
      { ".js", "application/x-javascript" },
      { ".mng", "video/x-mng" },
      { ".pbm", "image/x-portable-bitmap" },
      { ".pgm", "image/x-portable-graymap" },
      { ".pdf", "application/pdf" },
      { ".png", "image/png" },
      { ".ppm", "image/x-portable-pixmap" },
      { ".rss", "application/rss+xml" },
      { ".svg", "image/svg+xml" },
      { ".svgz", "image/svg+xml" },
      { ".text", "text/plain" },
      { ".tif", "image/tiff" },
      { ".tiff", "image/tiff" },
      { ".txt", "text/plain" },
      { ".xbm", "image/x-xbitmap" },
      { ".xml", "text/xml" },
      { ".xpm", "image/x-xpm" },
      { ".xsl", "text/xsl" },
      { ".xhtml", "application/xhtml+xml" },
      { ".wml", "text/vnd.wap.wml" },
      { ".wmlc", "application/vnd.wap.wmlc" },
      { "about:blank", 0 },
      { 0, 0 }
  };

  QByteArray mimeFromUrl(const QUrl &url)
  {
    const QString &path = url.path();
    const int index = path.lastIndexOf(QLatin1Char('.'));
    const QByteArray &ext = path.mid(index).toUtf8().toLower();

    const ExtensionMap *e = extensionMap;
    while (e->extension) {
      if (ext == e->extension)
        return QByteArray(e->mimeType);
      ++e;
    }
    return QByteArray("application/octet-stream");
  }

  //-----------------------------------------------------------------------
  // Class HelpUrlSchemeHandler Definition (INTERNAL)
  // Handing of qthelp URLs
  //-----------------------------------------------------------------------
  class HelpUrlSchemeHandler : public QWebEngineUrlSchemeHandler
  {
  public:
    HelpUrlSchemeHandler(QHelpEngineCore& helpEngine, QObject *parent = Q_NULLPTR);
    ~HelpUrlSchemeHandler();

    virtual void requestStarted(QWebEngineUrlRequestJob *request);

  private:
    QHelpEngineCore& helpEngineInstance;
  };

  HelpUrlSchemeHandler::HelpUrlSchemeHandler(QHelpEngineCore& helpEngine, QObject *parent) : QWebEngineUrlSchemeHandler(parent),
    helpEngineInstance(helpEngine)
  {
  }

  HelpUrlSchemeHandler::~HelpUrlSchemeHandler()
  {
  }

  void HelpUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob* request)
  {
    Q_ASSERT(request != 0);
    //qDebug() << request->requestMethod() << request->requestUrl();

    QUrl url = request->requestUrl();
    if (!url.isValid())
    {
      request->fail(QWebEngineUrlRequestJob::UrlInvalid);
    }
    QUrl helpUrl = helpEngineInstance.findFile(url);
    if (!helpUrl.isValid())
    {
      request->fail(QWebEngineUrlRequestJob::UrlNotFound);
    }
    QByteArray data = helpEngineInstance.fileData(helpUrl);
    QBuffer* buffer = new QBuffer();
    connect(request, SIGNAL(destroyed()), buffer, SLOT(deleteLater()));
    buffer->setData(data);
    request->reply(mimeFromUrl(helpUrl),buffer);
  }

  //-----------------------------------------------------------------------
  // Class ContentWindow
  //-----------------------------------------------------------------------

  ContentWindow::ContentWindow(QHelpEngine& helpEngine, QWidget* parent) : QWebEngineView(parent)
  {
    // install URL scheme handler for qthelp
    QWebEnginePage* webPage = page();
    const char qtHelpScheme[] = "qthelp";
    const QWebEngineUrlSchemeHandler* helpSchemeHandler = webPage->profile()->urlSchemeHandler(qtHelpScheme);
    if (!helpSchemeHandler)
    {
      HelpUrlSchemeHandler* handler = new HelpUrlSchemeHandler(helpEngine,this);
      connect(webPage->profile(), SIGNAL(destroyed()), handler, SLOT(deleteLater()));
      webPage->profile()->installUrlSchemeHandler(qtHelpScheme, handler);
    }

    setAcceptDrops(false);
    settings()->setAttribute(QWebEngineSettings::PluginsEnabled, false);
    settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, false);

    setViewerFont(viewerFont());
    connect(webPage,SIGNAL(linkHovered(QString)),this,SIGNAL(linkHovered(QString)));
  }

  ContentWindow::~ContentWindow()
  {
  }

  QFont ContentWindow::viewerFont() const
  {
    QWebEngineSettings *webSettings = QWebEngineSettings::globalSettings();
    return QFont(webSettings->fontFamily(QWebEngineSettings::SansSerifFont),
                 webSettings->fontSize(QWebEngineSettings::DefaultFontSize));
  }

  void ContentWindow::setViewerFont(const QFont &font)
  {
    QWebEngineSettings *webSettings = settings();
    webSettings->setFontFamily(QWebEngineSettings::StandardFont, font.family());
    webSettings->setFontSize(QWebEngineSettings::DefaultFontSize, font.pointSize());
  }

  //-----------------------------------------------------------------------
  // Class DlgTopicSelection
  // Adapted from Qt Assistant source code
  //-----------------------------------------------------------------------
  DlgTopicSelection::DlgTopicSelection(const QMap<QString,QUrl>& helpTopics, const QString& keyword, QWidget *parent) : QDialog(parent),
    url(), lblTopic(0), edtFilter(0), lvTopics(0), mdlFilter(0)
  {
    setWindowTitle(tr("Select Topic"));
    setSizeGripEnabled(true);
    setWindowModality(Qt::WindowModal);

    lblTopic = new QLabel(this);
    lblTopic->setTextFormat(Qt::RichText);
    lblTopic->setText(QString(tr("Select help topic for <b>%1</b>")).arg(keyword));

    edtFilter = new QLineEdit(this);
    edtFilter->setPlaceholderText(tr("Filter"));
    setFocusProxy(edtFilter);
    edtFilter->installEventFilter(this);
    connect(edtFilter,SIGNAL(textChanged(QString)),this,SLOT(setTopicFilter(QString)));

    QStandardItemModel* mdlItems = new QStandardItemModel();
    QMap<QString,QUrl>::ConstIterator it;
    for(it = helpTopics.begin(); it != helpTopics.end(); ++it)
    {
      QStandardItem* item = new QStandardItem(it.key());
      item->setToolTip(it.value().toString());
      item->setData(it.value());
      mdlItems->appendRow(item);
    }

    mdlFilter = new QSortFilterProxyModel();
    mdlFilter->setSourceModel(mdlItems);
    mdlFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);
    lvTopics = new QListView(this);
    lvTopics->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lvTopics->setUniformItemSizes(true);
    lvTopics->setModel(mdlFilter);
    if (mdlFilter->rowCount() != 0)
    {
      lvTopics->setCurrentIndex(mdlFilter->index(0, 0));
    }
    connect(lvTopics,SIGNAL(activated(QModelIndex)),this,SLOT(topicActivated(QModelIndex)));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,this);
    connect(buttonBox,SIGNAL(accepted()),this,SLOT(selectTopic()));
    connect(buttonBox,SIGNAL(rejected()),this,SLOT(reject()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(lblTopic);
    layout->addWidget(edtFilter);
    layout->addWidget(lvTopics,1);
    layout->addWidget(buttonBox);
    setLayout(layout);
  }

  void DlgTopicSelection::setTopicFilter(const QString& filter)
  {
    mdlFilter->setFilterFixedString(filter);
    if (mdlFilter->rowCount() != 0 && !lvTopics->currentIndex().isValid())
    {
      lvTopics->setCurrentIndex(mdlFilter->index(0, 0));
    }
  }

  void DlgTopicSelection::topicActivated(const QModelIndex& index)
  {
    if (index.isValid())
    {
      url = index.data(Qt::UserRole+1).toUrl();
    }
    accept();
  }

  void DlgTopicSelection::selectTopic()
  {
    topicActivated(lvTopics->currentIndex());
  }

  bool DlgTopicSelection::eventFilter(QObject *object, QEvent *event)
  {
    if (object == edtFilter && event->type() == QEvent::KeyPress)
    {
      QModelIndex idx = lvTopics->currentIndex();
      switch ((static_cast<QKeyEvent*>(event)->key()))
      {
        case Qt::Key_Up:
          idx = mdlFilter->index(idx.row() - 1,idx.column(),idx.parent());
          if (idx.isValid())
          {
            lvTopics->setCurrentIndex(idx);
          }
          break;
        case Qt::Key_Down:
          idx = mdlFilter->index(idx.row() + 1,idx.column(),idx.parent());
          if (idx.isValid())
          {
            lvTopics->setCurrentIndex(idx);
          }
          break;
        default: ;
      }
    }
    else if (edtFilter && event->type() == QEvent::FocusIn && static_cast<QFocusEvent *>(event)->reason() != Qt::MouseFocusReason)
    {
      edtFilter->selectAll();
      edtFilter->setFocus();
    }
    return QDialog::eventFilter(object, event);
  }

  //-----------------------------------------------------------------------
  // Class HelpIndexWidget
  // Adapted from Qt Assistant source code
  //-----------------------------------------------------------------------
  HelpIndexWidget::HelpIndexWidget(QHelpEngine& helpEngine, QWidget *parent) : QWidget(parent), edtSearch(0), lvIndex(0)
  {
    QVBoxLayout* layout = new QVBoxLayout(this);
    edtSearch = new QLineEdit(this);
    edtSearch->setPlaceholderText(tr("Look for"));
    connect(edtSearch, SIGNAL(textChanged(QString)), this, SLOT(filterIndices(QString)));
    edtSearch->installEventFilter(this);
    layout->setMargin(4);
    layout->addWidget(edtSearch);

    lvIndex = helpEngine.indexWidget();
    lvIndex->installEventFilter(this);
    connect(helpEngine.indexModel(), SIGNAL(indexCreationStarted()), this, SLOT(disableSearchLineEdit()));
    connect(helpEngine.indexModel(), SIGNAL(indexCreated()), this, SLOT(enableSearchLineEdit()));
    connect(lvIndex, SIGNAL(linkActivated(QUrl,QString)), this, SIGNAL(linkActivated(QUrl,QString)));
    connect(lvIndex, SIGNAL(linksActivated(QMap<QString,QUrl>,QString)), this, SIGNAL(linksActivated(QMap<QString,QUrl>,QString)));
    connect(edtSearch, SIGNAL(returnPressed()), lvIndex, SLOT(activateCurrentItem()));
    layout->addWidget(lvIndex);

    lvIndex->viewport()->installEventFilter(this);
  }

  HelpIndexWidget::~HelpIndexWidget()
  {
  }

  void HelpIndexWidget::filterIndices(const QString &filter)
  {
    if (filter.contains(QLatin1Char('*')))
    {
      lvIndex->filterIndices(filter, filter);
    }
    else
    {
      lvIndex->filterIndices(filter, QString());
    }
  }

  bool HelpIndexWidget::eventFilter(QObject *obj, QEvent *e)
  {
    if (obj == edtSearch && e->type() == QEvent::KeyPress)
    {
      QKeyEvent *ke = static_cast<QKeyEvent*>(e);
      QModelIndex idx = lvIndex->currentIndex();
      switch (ke->key())
      {
        case Qt::Key_Up:
          idx = lvIndex->model()->index(idx.row()-1, idx.column(), idx.parent());
          if (idx.isValid())
          {
            lvIndex->setCurrentIndex(idx);
            return true;
          }
          break;
        case Qt::Key_Down:
          idx = lvIndex->model()->index(idx.row()+1, idx.column(), idx.parent());
          if (idx.isValid())
          {
            lvIndex->setCurrentIndex(idx);
            return true;
          }
          break;
        case Qt::Key_Escape:
          emit escapePressed();
          return true;
        default: ; // stop complaining
      }
    }
    /*
    else if (obj == lvIndex && e->type() == QEvent::ContextMenu)
    {
      QContextMenuEvent *ctxtEvent = static_cast<QContextMenuEvent*>(e);
      QModelIndex idx = lvIndex->indexAt(ctxtEvent->pos());
      if (idx.isValid())
      {
        QMenu menu;
        QAction *curTab = menu.addAction(tr("Open Link"));
        QAction *newTab = menu.addAction(tr("Open Link in New Tab"));
        menu.move(lvIndex->mapToGlobal(ctxtEvent->pos()));
        QAction *action = menu.exec();
        if (curTab == action)
        {
          lvIndex->activateCurrentItem();
        }
        else if (newTab == action)
        {
          open(lvIndex, idx);
        }
      }
    }
    */
    else if (lvIndex && obj == lvIndex->viewport() && e->type() == QEvent::MouseButtonRelease)
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
      QModelIndex idx = lvIndex->indexAt(mouseEvent->pos());
      if (idx.isValid())
      {
        Qt::MouseButtons button = mouseEvent->button();
        if (((button == Qt::LeftButton) && (mouseEvent->modifiers() & Qt::ControlModifier)) || (button == Qt::MidButton))
        {
          open(lvIndex, idx);
        }
      }
    }
#ifdef Q_OS_MAC
    else if (obj == lvIndex && e->type() == QEvent::KeyPress)
    {
      QKeyEvent *ke = static_cast<QKeyEvent*>(e);
      if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
        lvIndex->activateCurrentItem();
    }
#endif
    return QWidget::eventFilter(obj, e);
  }

  void HelpIndexWidget::enableSearchLineEdit()
  {
    edtSearch->setDisabled(false);
    filterIndices(edtSearch->text());
  }

  void HelpIndexWidget::disableSearchLineEdit()
  {
    edtSearch->setDisabled(true);
  }

  void HelpIndexWidget::setSearchLineEditText(const QString &text)
  {
    edtSearch->setText(text);
  }

  void HelpIndexWidget::focusInEvent(QFocusEvent *e)
  {
    if (e->reason() != Qt::MouseFocusReason)
    {
      edtSearch->selectAll();
      edtSearch->setFocus();
    }
  }

  void HelpIndexWidget::open(QHelpIndexWidget* indexWidget, const QModelIndex &index)
  {
    QHelpIndexModel *model = qobject_cast<QHelpIndexModel*>(indexWidget->model());
    if (model)
    {
      QString keyword = model->data(index, Qt::DisplayRole).toString();
      QMap<QString, QUrl> links = model->linksForKeyword(keyword);
      QUrl url;
      if (links.count() > 1)
      {
        emit linksActivated(links,keyword);
      }
      else if (links.count() == 1)
      {
        emit linkActivated(url,keyword);
      }
    }
  }

  //-----------------------------------------------------------------------
  // Class MainWindow
  //-----------------------------------------------------------------------
  const QString MainWindow::keyHelpWndGeometry = QLatin1String("HelpWndGeometry");
  const QString MainWindow::keyHelpWndState = QLatin1String("HelpWndState");

  MainWindow::MainWindow(QHelpEngine& helpEngine) : QMainWindow(), helpEngineInstance(helpEngine), ptrBrowser(0)
  {
    setWindowTitle(QApplication::applicationName() + " - " + tr("Help"));

    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks);
    setCorner(Qt::TopLeftCorner,Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner,Qt::LeftDockWidgetArea);
    setTabPosition(Qt::AllDockWidgetAreas,QTabWidget::North);

    // setup status bar
    QStatusBar* statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    // setup content browser
    ptrBrowser = new ContentWindow(helpEngineInstance, this);
    setCentralWidget(ptrBrowser);

    // create dock widgets
    QDockWidget* dockWidgetContents = new QDockWidget(tr("Contents"),this);
    dockWidgetContents->setWidget(helpEngineInstance.contentWidget());
    dockWidgetContents->setObjectName(dockWidgetContents->windowTitle());
    dockWidgetContents->toggleViewAction()->setStatusTip(tr("Toggle visibility of contents window"));
    addDockWidget(Qt::LeftDockWidgetArea,dockWidgetContents);

    QDockWidget* dockWidgetIndex = new QDockWidget(tr("Index"),this);
    HelpIndexWidget* indexWidget = new HelpIndexWidget(helpEngineInstance,this);
    dockWidgetIndex->setWidget(indexWidget);
    dockWidgetIndex->setObjectName(dockWidgetIndex->windowTitle());
    dockWidgetIndex->toggleViewAction()->setStatusTip(tr("Toggle visibility of index window"));
    addDockWidget(Qt::LeftDockWidgetArea,dockWidgetIndex);

    QDockWidget* dockWidgetSearch = new QDockWidget(tr("Search"),this);
    QWidget* widgetSearch = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(widgetSearch);
    layout->addWidget(helpEngineInstance.searchEngine()->queryWidget());
    layout->addWidget(helpEngineInstance.searchEngine()->resultWidget(),1);
    layout->setMargin(4);
    widgetSearch->setLayout(layout);
    dockWidgetSearch->setWidget(widgetSearch);
    dockWidgetSearch->setObjectName(dockWidgetSearch->windowTitle());
    dockWidgetSearch->toggleViewAction()->setStatusTip(tr("Toggle visibility of search window"));
    addDockWidget(Qt::LeftDockWidgetArea,dockWidgetSearch);

    tabifyDockWidget(dockWidgetContents,dockWidgetIndex);
    tabifyDockWidget(dockWidgetIndex,dockWidgetSearch);
    dockWidgetContents->raise();

    // create tool bars
    QToolBar* tbNavigation = new QToolBar(tr("&Navigation"),this);
    QAction* navBack = ptrBrowser->pageAction(QWebEnginePage::Back);
    QAction* navForward = ptrBrowser->pageAction(QWebEnginePage::Forward);
    QAction* navReload = ptrBrowser->pageAction(QWebEnginePage::Reload);
    navBack->setIcon(QIcon(":/icons/resources/arrowleft.png"));
    navBack->setStatusTip(tr("Navigate one step back"));
    navBack->setShortcut(QKeySequence::Back);
    navForward->setIcon(QIcon(":/icons/resources/arrowright.png"));
    navForward->setStatusTip(tr("Navigate one step forward"));
    navForward->setShortcut(QKeySequence::Forward);
    navReload->setIcon(QIcon(":/icons/resources/arrowrefresh.png"));
    navReload->setStatusTip(tr("Reload the current page"));
    navReload->setShortcut(QKeySequence::Refresh);
    tbNavigation->addAction(navBack);
    tbNavigation->addAction(navForward);
    tbNavigation->addAction(navReload);
    tbNavigation->setObjectName(tbNavigation->windowTitle());
    tbNavigation->toggleViewAction()->setStatusTip(tr("Toggle visibility of navigation toolbar"));
    addToolBar(tbNavigation);

    QToolBar* tbView = new QToolBar(tr("&View"),this);
    QAction* viewZoomIn = tbView->addAction(QIcon(":/icons/resources/zoom_in.png"),tr("Increase size"),this,SLOT(zoomIn()));
    viewZoomIn->setShortcut(QKeySequence::ZoomIn);
    viewZoomIn->setStatusTip(tr("Increase size of contents"));
    QAction* viewZoomOut = tbView->addAction(QIcon(":/icons/resources/zoom_out.png"),tr("Decrease size"),this,SLOT(zoomOut()));
    viewZoomOut->setShortcut(QKeySequence::ZoomOut);
    viewZoomOut->setStatusTip(tr("Decrease size of contents"));
    QAction* viewZoom100 = tbView->addAction(QIcon(":/icons/resources/zoom_100.png"),tr("Default size"),this,SLOT(zoom100()));
    viewZoom100->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0));
    viewZoom100->setStatusTip(tr("Set default size of contents"));
    connect(this,SIGNAL(destroyed()),viewZoomIn,SLOT(deleteLater()));
    connect(this,SIGNAL(destroyed()),viewZoomOut,SLOT(deleteLater()));
    connect(this,SIGNAL(destroyed()),viewZoom100,SLOT(deleteLater()));
    tbView->setObjectName(tbView->windowTitle());
    tbView->toggleViewAction()->setStatusTip(tr("Toggle visibility of view toolbar"));
    addToolBar(tbView);

    // create menu bar
    QMenu* menuFile = new QMenu(tr("&File"),this);
    QAction* closeHelp = menuFile->addAction(QIcon(":/icons/resources/quit.png"),tr("&Close"),this,SLOT(close()),QKeySequence::Close);
    closeHelp->setStatusTip(tr("Close help window."));
    connect(this,SIGNAL(destroyed()),closeHelp,SLOT(deleteLater()));

    QMenu* menuToolbars = new QMenu(tr("Toolbars"),this);
    menuToolbars->addAction(tbNavigation->toggleViewAction());
    menuToolbars->addAction(tbView->toggleViewAction());

    QMenu* menuView = new QMenu(tr("&View"),this);
    menuView->addAction(dockWidgetContents->toggleViewAction());
    menuView->addAction(dockWidgetIndex->toggleViewAction());
    menuView->addAction(dockWidgetSearch->toggleViewAction());
    menuView->addSeparator();
    menuView->addMenu(menuToolbars);
    menuView->addSeparator();
    menuView->addAction(viewZoomIn);
    menuView->addAction(viewZoomOut);
    menuView->addAction(viewZoom100);

    QMenu* menuGoTo = new QMenu(tr("&Go to"),this);
    menuGoTo->addAction(navBack);
    menuGoTo->addAction(navForward);
    menuGoTo->addSeparator();
    menuGoTo->addAction(navReload);

    QMenuBar* menuBar = new QMenuBar(this);
    menuBar->addMenu(menuFile);
    menuBar->addMenu(menuView);
    menuBar->addMenu(menuGoTo);
    setMenuBar(menuBar);

    // connect signals
    connect(ptrBrowser,SIGNAL(linkHovered(QString)),statusBar,SLOT(showMessage(QString)));
    connect(indexWidget,SIGNAL(linkActivated(QUrl,QString)),this,SLOT(showPage(QUrl,QString)));
    connect(indexWidget,SIGNAL(linksActivated(QMap<QString,QUrl>,QString)),this,SLOT(selectTopic(QMap<QString,QUrl>,QString)));
    connect(helpEngineInstance.contentWidget(),SIGNAL(linkActivated(QUrl)),this,SLOT(showPage(QUrl)));
    connect(helpEngineInstance.searchEngine()->queryWidget(),SIGNAL(search()),this,SLOT(runSearch()));
    connect(helpEngineInstance.searchEngine()->resultWidget(),SIGNAL(requestShowLink(QUrl)),this,SLOT(showPage(QUrl)));

    restoreGeometry(helpEngineInstance.customValue(keyHelpWndGeometry).toByteArray());
    restoreState(helpEngineInstance.customValue(keyHelpWndState).toByteArray());
  }


  MainWindow::~MainWindow()
  {
  }

  void MainWindow::runSearch()
  {
    helpEngineInstance.searchEngine()->search(helpEngineInstance.searchEngine()->queryWidget()->query());
  }

  void MainWindow::showPage(const QUrl &url)
  {
    ptrBrowser->setUrl(url);
  }

  void MainWindow::showPage(const QUrl &url, const QString& /*keyword*/)
  {
    ptrBrowser->setUrl(url);
  }

  void MainWindow::selectTopic(const QMap<QString,QUrl>& topicList, const QString& keyword)
  {
    DlgTopicSelection dlg(topicList,keyword,this);
    if (dlg.exec() > 0)
    {
      ptrBrowser->setUrl(dlg.link());
    }
  }

  void MainWindow::zoomIn()
  {
    if (ptrBrowser)
    {
      qreal zoomFactor = ptrBrowser->zoomFactor() + qreal(0.1);
      if (zoomFactor > qreal(5.0)) zoomFactor = qreal(5.0);
      ptrBrowser->setZoomFactor(zoomFactor);
    }
  }

  void MainWindow::zoomOut()
  {
    if (ptrBrowser)
    {
      qreal zoomFactor = ptrBrowser->zoomFactor() - qreal(0.1);
      if (zoomFactor < qreal(0.25)) zoomFactor = qreal(0.25);
      ptrBrowser->setZoomFactor(zoomFactor);
    }
  }

  void MainWindow::zoom100()
  {
    if (ptrBrowser)
    {
      ptrBrowser->setZoomFactor(qreal(1.0));
    }
  }

  void MainWindow::closeEvent(QCloseEvent* event)
  {
    helpEngineInstance.setCustomValue(keyHelpWndGeometry,saveGeometry());
    helpEngineInstance.setCustomValue(keyHelpWndState,saveState());
    QMainWindow::closeEvent(event);
  }

}
