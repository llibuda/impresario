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

namespace help
{
  // Provide helper functions for feeding the QtHelp data stored in the help database
  // into various browsers.
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

    static ResolveUrlResult resolveUrl(const QUrl &url, QUrl *targetUrl,
                                       QByteArray *data);
    static QByteArray fileDataForLocalUrl(const QUrl &url);

    // Create an instance of QNetworkAccessManager for WebKit-type browsers.
    static QNetworkAccessManager *createNetworkAccessManager(QObject *parent = 0);
  };

  class ContentWindow : public QWebView
  {
  public:
    ContentWindow(QWidget* parent = 0);
    ~ContentWindow();

    QFont viewerFont() const;
    void setViewerFont(const QFont &font);
  };

}
#endif // HELPCONTENTWINDOW_H
