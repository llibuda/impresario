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
#include "helpcontentwindow.h"
#include "helpsystem.h"

namespace help
{

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
    HelpNetworkAccessManager(QObject *parent);

  protected:
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice* outgoingData = 0);
  };

  HelpNetworkAccessManager::HelpNetworkAccessManager(QObject *parent) : QNetworkAccessManager(parent)
  {
  }

  QNetworkReply* HelpNetworkAccessManager::createRequest(Operation, const QNetworkRequest &request, QIODevice*)
  {
    QByteArray data;
    const QUrl url = request.url();
    QUrl redirectedUrl;
    switch (WebkitBrowserSupport::resolveUrl(url, &redirectedUrl, &data)) {
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
  // Class WebKitBrowserSupport
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

  QByteArray WebkitBrowserSupport::fileDataForLocalUrl(const QUrl &url)
  {
      return System::helpEngine()->fileData(url);
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

  WebkitBrowserSupport::ResolveUrlResult WebkitBrowserSupport::resolveUrl(const QUrl &url, QUrl *targetUrlP, QByteArray *dataP)
  {
    const PtrHelpEngine engine = System::helpEngine();

    const QUrl targetUrl = engine->findFile(url);
    if (!targetUrl.isValid())
      return UrlResolveError;

    if (targetUrl != url)
    {
      if (targetUrlP) *targetUrlP = targetUrl;
      return UrlRedirect;
    }

    if (dataP) *dataP = WebkitBrowserSupport::fileDataForLocalUrl(targetUrl);

    return UrlLocalData;
  }

  QNetworkAccessManager *WebkitBrowserSupport::createNetworkAccessManager(QObject *parent)
  {
    return new HelpNetworkAccessManager(parent);
  }

  //-----------------------------------------------------------------------
  // Class ContentWindow
  //-----------------------------------------------------------------------

  ContentWindow::ContentWindow(QWidget* parent) : QWebView(parent)
  {
    setAcceptDrops(false);
    settings()->setAttribute(QWebSettings::JavaEnabled, false);
    settings()->setAttribute(QWebSettings::PluginsEnabled, false);

    page()->setNetworkAccessManager(WebkitBrowserSupport::createNetworkAccessManager(this));
    setFont(viewerFont());
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
    return QFont(webSettings->fontFamily(QWebSettings::StandardFont),
                 webSettings->fontSize(QWebSettings::DefaultFontSize));
  }

  void ContentWindow::setViewerFont(const QFont &font)
  {
    QWebSettings *webSettings = settings();
    webSettings->setFontFamily(QWebSettings::StandardFont, font.family());
    webSettings->setFontFamily(QWebSettings::StandardFont, "Helvetica");
    webSettings->setFontSize(QWebSettings::DefaultFontSize, font.pointSize());
    webSettings->setFontSize(QWebSettings::DefaultFontSize, 24);
  }

}
