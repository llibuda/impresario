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

namespace help
{
  //-----------------------------------------------------------------------
  // Class WebKitBrowserSupport Declaration (INTERNAL)
  //-----------------------------------------------------------------------
  class WebkitBrowserSupport
  {
  public:
    enum ResolveUrlResult {
      UrlRedirect,
      UrlLocalData,
      UrlResolveError
    };

    static QString msgError404();
    static QString msgPageNotFound();
    static QString msgAllDocumentationSets();
    static QString msgLoadError(const QUrl &url);
    static QString msgHtmlErrorPage(const QUrl &url);
    static QString mimeFromUrl(const QUrl &url);

    static ResolveUrlResult resolveUrl(const QUrl &url, const QHelpEngineCore& helpEngineInstance, QUrl *targetUrl, QByteArray *data);

    // Create an instance of QNetworkAccessManager for WebKit-type browsers.
    static QNetworkAccessManager *createNetworkAccessManager(QHelpEngine& helpEngine, QObject *parent = 0);
  };

  //-----------------------------------------------------------------------
  // Class HelpNetworkReply (INTERNAL)
  //-----------------------------------------------------------------------

  class HelpNetworkReply : public QNetworkReply
  {
  public:
    HelpNetworkReply(const QNetworkRequest &request, const QByteArray &fileData, const QString &mimeType);

    virtual void abort();

    virtual qint64 bytesAvailable() const
    { return data.length() + QNetworkReply::bytesAvailable(); }

  protected:
    virtual qint64 readData(char *data, qint64 maxlen);

  private:
    QByteArray   data;
    const qint64 origLen;
  };

  HelpNetworkReply::HelpNetworkReply(const QNetworkRequest &request, const QByteArray &fileData, const QString& mimeType)
    : data(fileData), origLen(fileData.length())
  {
    setRequest(request);
    setUrl(request.url());
    setOpenMode(QIODevice::ReadOnly);

    setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(origLen));
    QTimer::singleShot(0, this, &QNetworkReply::metaDataChanged);
    QTimer::singleShot(0, this, &QNetworkReply::readyRead);
    QTimer::singleShot(0, this, &QNetworkReply::finished);
  }

  void HelpNetworkReply::abort()
  {
  }

  qint64 HelpNetworkReply::readData(char *buffer, qint64 maxlen)
  {
    qint64 len = qMin(qint64(data.length()), maxlen);
    if (len)
    {
      memcpy(buffer, data.constData(), len);
      data.remove(0, len);
    }
    if (!data.length()) QTimer::singleShot(0, this, &QNetworkReply::finished);
    return len;
  }

  //-----------------------------------------------------------------------
  // Class HelpRedirectNetworkReply (INTERNAL)
  //-----------------------------------------------------------------------

  class HelpRedirectNetworkReply : public QNetworkReply
  {
  public:
    HelpRedirectNetworkReply(const QNetworkRequest &request, const QUrl &newUrl)
    {
      setRequest(request);
      setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 301);
      setAttribute(QNetworkRequest::RedirectionTargetAttribute, newUrl);
      QTimer::singleShot(0, this, &QNetworkReply::finished);
    }

  protected:
    void abort() { }
    qint64 readData(char*, qint64) { return qint64(-1); }
  };

  //-----------------------------------------------------------------------
  // Class HelpNetworkAccessManager (INTERNAL)
  //-----------------------------------------------------------------------

  class HelpNetworkAccessManager : public QNetworkAccessManager
  {
  public:
    HelpNetworkAccessManager(QHelpEngineCore& helpEngine, QObject *parent);

  protected:
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice* outgoingData = 0);

  private:
    QHelpEngineCore& helpEngineInstance;
  };

  HelpNetworkAccessManager::HelpNetworkAccessManager(QHelpEngineCore& helpEngine, QObject *parent) : QNetworkAccessManager(parent), helpEngineInstance(helpEngine)
  {
  }

  QNetworkReply* HelpNetworkAccessManager::createRequest(Operation, const QNetworkRequest &request, QIODevice*)
  {
    QByteArray data;
    const QUrl url = request.url();
    QUrl redirectedUrl;
    switch (WebkitBrowserSupport::resolveUrl(url, helpEngineInstance, &redirectedUrl, &data)) {
      case WebkitBrowserSupport::UrlRedirect:
        return new HelpRedirectNetworkReply(request, redirectedUrl);
      case WebkitBrowserSupport::UrlLocalData: {
        const QString mimeType = WebkitBrowserSupport::mimeFromUrl(url);
        return new HelpNetworkReply(request, data, mimeType);
      }
      case WebkitBrowserSupport::UrlResolveError:
        break;
    }
    return new HelpNetworkReply(request, WebkitBrowserSupport::msgHtmlErrorPage(request.url()).toUtf8(), QStringLiteral("text/html"));
  }

  //-----------------------------------------------------------------------
  // Class WebKitBrowserSupport Definition (INTERNAL)
  //-----------------------------------------------------------------------
  static const char g_htmlPage[] = "<html><head><meta http-equiv=\"content-type\" content=\"text/html; "
                                   "charset=UTF-8\"><title>%1</title><style>body{padding: 3em 0em;background: #eeeeee;}"
                                   "hr{color: lightgray;width: 100%;}img{float: left;opacity: .8;}#box{background: white;border: 1px solid "
                                   "lightgray;width: 600px;padding: 60px;margin: auto;}h1{font-size: 130%;font-weight: bold;border-bottom: "
                                   "1px solid lightgray;margin-left: 48px;}h2{font-size: 100%;font-weight: normal;border-bottom: 1px solid "
                                   "lightgray;margin-left: 48px;}ul{font-size: 80%;padding-left: 48px;margin: 0;}#reloadButton{padding-left:"
                                   "48px;}</style></head><body><div id=\"box\"><h1>%2</h1><h2>%3</h2><h2><b>%4</b></h2></div></body></html>";

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

  QString WebkitBrowserSupport::msgError404()
  {
    return QCoreApplication::translate("Help System", "Error 404...");
  }

  QString WebkitBrowserSupport::msgPageNotFound()
  {
    return QCoreApplication::translate("Help System", "The page could not be found!");
  }

  QString WebkitBrowserSupport::msgAllDocumentationSets()
  {
    return QCoreApplication::translate("Help System","Please make sure that you have all documentation sets installed.");
  }

  QString WebkitBrowserSupport::msgLoadError(const QUrl &url)
  {
    return QObject::tr("Error loading: %1").arg(url.toString());
  }

  QString WebkitBrowserSupport::msgHtmlErrorPage(const QUrl &url)
  {
    return QString::fromLatin1(g_htmlPage)
        .arg(WebkitBrowserSupport::msgError404(), WebkitBrowserSupport::msgPageNotFound(),
             WebkitBrowserSupport::msgLoadError(url), WebkitBrowserSupport::msgAllDocumentationSets());
  }

  QString WebkitBrowserSupport::mimeFromUrl(const QUrl &url)
  {
    const QString &path = url.path();
    const int index = path.lastIndexOf(QLatin1Char('.'));
    const QByteArray &ext = path.mid(index).toUtf8().toLower();

    const ExtensionMap *e = extensionMap;
    while (e->extension) {
      if (ext == e->extension)
        return QLatin1String(e->mimeType);
      ++e;
    }
    return QLatin1String("application/octet-stream");
  }

  WebkitBrowserSupport::ResolveUrlResult WebkitBrowserSupport::resolveUrl(const QUrl &url, const QHelpEngineCore& helpEngineInstance, QUrl *targetUrlP, QByteArray *dataP)
  {
    const QUrl targetUrl = helpEngineInstance.findFile(url);
    if (!targetUrl.isValid())
      return UrlResolveError;

    if (targetUrl != url)
    {
      if (targetUrlP) *targetUrlP = targetUrl;
      return UrlRedirect;
    }

    if (dataP) *dataP = helpEngineInstance.fileData(targetUrl);

    return UrlLocalData;
  }

  QNetworkAccessManager *WebkitBrowserSupport::createNetworkAccessManager(QHelpEngine& helpEngine, QObject *parent)
  {
    return new HelpNetworkAccessManager(helpEngine, parent);
  }

  //-----------------------------------------------------------------------
  // Class ContentWindow
  //-----------------------------------------------------------------------

  ContentWindow::ContentWindow(QHelpEngine& helpEngine, QWidget* parent) : QWebView(parent)
  {
    setAcceptDrops(false);
    settings()->setAttribute(QWebSettings::JavaEnabled, false);
    settings()->setAttribute(QWebSettings::PluginsEnabled, false);

    page()->setNetworkAccessManager(WebkitBrowserSupport::createNetworkAccessManager(helpEngine,this));
    setViewerFont(viewerFont());
  }

  ContentWindow::~ContentWindow()
  {
    QWebSettings::clearMemoryCaches();
  }

  QFont ContentWindow::viewerFont() const
  {
  //  if (HelpEngineWrapper::instance().usesBrowserFont())
  //    return HelpEngineWrapper::instance().browserFont();
    QWebSettings *webSettings = QWebSettings::globalSettings();
    return QFont(webSettings->fontFamily(QWebSettings::SansSerifFont),
                 webSettings->fontSize(QWebSettings::DefaultFontSize));
  }

  void ContentWindow::setViewerFont(const QFont &font)
  {
    QWebSettings *webSettings = settings();
    webSettings->setFontFamily(QWebSettings::StandardFont, font.family());
    webSettings->setFontSize(QWebSettings::DefaultFontSize, font.pointSize());
  }

  //-----------------------------------------------------------------------
  // Class DlgTopicSelection
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
    addDockWidget(Qt::LeftDockWidgetArea,dockWidgetContents);

    QDockWidget* dockWidgetIndex = new QDockWidget(tr("Index"),this);
    dockWidgetIndex->setWidget(helpEngineInstance.indexWidget());
    dockWidgetIndex->setObjectName(dockWidgetIndex->windowTitle());
    addDockWidget(Qt::LeftDockWidgetArea,dockWidgetIndex);

    QDockWidget* dockWidgetSearch = new QDockWidget(tr("Search"),this);
    QWidget* widgetSearch = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(widgetSearch);
    layout->addWidget(helpEngineInstance.searchEngine()->queryWidget());
    layout->addWidget(helpEngineInstance.searchEngine()->resultWidget(),1);
    layout->setMargin(5);
    widgetSearch->setLayout(layout);
    dockWidgetSearch->setWidget(widgetSearch);
    dockWidgetSearch->setObjectName(dockWidgetSearch->windowTitle());
    addDockWidget(Qt::LeftDockWidgetArea,dockWidgetSearch);

    tabifyDockWidget(dockWidgetContents,dockWidgetIndex);
    tabifyDockWidget(dockWidgetIndex,dockWidgetSearch);
    dockWidgetContents->raise();

    // create menu bar
    QMenu* menuFile = new QMenu(tr("&File"),this);
    QAction* action = menuFile->addAction(QIcon(":/icons/resources/quit.png"),tr("&Close"),this,SLOT(close()),QKeySequence::Close);
    action->setStatusTip(tr("Close help window."));

    QMenu* menuView = new QMenu(tr("&View"),this);
    menuView->addAction(dockWidgetContents->toggleViewAction());
    dockWidgetContents->toggleViewAction()->setStatusTip(tr("Toggle visibility of contents window"));
    menuView->addAction(dockWidgetIndex->toggleViewAction());
    dockWidgetIndex->toggleViewAction()->setStatusTip(tr("Toggle visibility of index window"));
    menuView->addAction(dockWidgetSearch->toggleViewAction());
    dockWidgetSearch->toggleViewAction()->setStatusTip(tr("Toggle visibility of search window"));

    QMenuBar* menuBar = new QMenuBar(this);
    menuBar->addMenu(menuFile);
    menuBar->addMenu(menuView);
    setMenuBar(menuBar);

    // connect signals
    connect(helpEngineInstance.contentWidget(),SIGNAL(linkActivated(QUrl)),this,SLOT(showPage(QUrl)));
    connect(helpEngineInstance.indexWidget(),SIGNAL(linkActivated(QUrl,QString)),this,SLOT(showPage(QUrl)));
    connect(helpEngineInstance.indexWidget(),SIGNAL(linksActivated(QMap<QString,QUrl>,QString)),this,SLOT(selectTopic(QMap<QString,QUrl>,QString)));
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

  void MainWindow::closeEvent(QCloseEvent* event)
  {
    helpEngineInstance.setCustomValue(keyHelpWndGeometry,saveGeometry());
    helpEngineInstance.setCustomValue(keyHelpWndState,saveState());
    QMainWindow::closeEvent(event);
  }

}
