#******************************************************************************************
#   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
#   Copyright (C) 2015-2017  Lars Libuda
#
#   This file is part of Impresario.
#
#   Impresario is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   Impresario is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with Impresario in subdirectory "licenses", file "LICENSE_Impresario.GPLv3".
#   If not, see <http://www.gnu.org/licenses/>.
#*****************************************************************************************
QT += core gui widgets network xmlpatterns qml quick quickwidgets concurrent help webkitwidgets
TARGET = Impresario
TEMPLATE = app
VERSION = 2.1.0
QMAKE_TARGET_DESCRIPTION = "Image Processing Engineering System applying Reusable Interactive Objects"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2015-2017  Lars Libuda"

INCLUDEPATH += $$quote($$_PRO_FILE_PWD_/../components/qtpropertybrowser/src)
INCLUDEPATH += $$quote($$_PRO_FILE_PWD_/../components/libavoid/source)
INCLUDEPATH += $$quote($$_PRO_FILE_PWD_/../components/singleapplication)

SOURCES += main.cpp \
    appimpresario.cpp \
    framemainwindow.cpp \
    framestatusbar.cpp \
    frametoolbars.cpp \
    configdlgsettings.cpp \
    configdlgpages.cpp \
    syslogwndlogger.cpp \
    sysloglogger.cpp \
    aboutdlgabout.cpp \
    appbuildinfo.cpp \
    resources.cpp \
    qthelper.cpp \
    dbwndmacros.cpp \
    appdlgterminate.cpp \
    appmacrolibrary.cpp \
    appmacro.cpp \
    dbmodel.cpp \
    dbviewconfig.cpp \
    framemenubar.cpp \
    framedockwindows.cpp \
    syslogwndconsole.cpp \
    dbwndprop.cpp \
    graphelements.cpp \
    graphmain.cpp \
    appmacromanager.cpp \
    appprocessgraph.cpp \
    graphitems.cpp \
    grapheditor.cpp \
    pgecomponents.cpp \
    pgecommands.cpp \
    pgeitems.cpp \
    graphdata.cpp \
    pgewndprops.cpp \
    graphresources.cpp \
    graphserializer.cpp \
    stdconsoleinterface.cpp \
    ../components/singleapplication/singleapplication.cpp \
    helpsystem.cpp \
    helpcontentwindow.cpp

HEADERS  += appimpresario.h \
    framemainwindow.h \
    framestatusbar.h \
    frametoolbars.h \
    configdlgsettings.h \
    configdlgpages.h \
    syslogwndlogger.h \
    sysloglogger.h \
    aboutdlgabout.h \
    version.h \
    appbuildinfo.h \
    resources.h \
    qthelper.h \
    dbwndmacros.h \
    appdlgterminate.h \
    appmacrolibrary.h \
    appmacro.h \
    dbmodel.h \
    dbviewconfig.h \
    framemenubar.h \
    framedockwindows.h \
    syslogwndconsole.h \
    dbwndprop.h \
    graphelements.h \
    graphmain.h \
    appmacromanager.h \
    appprocessgraph.h \
    graphitems.h \
    grapheditor.h \
    pgecomponents.h \
    pgecommands.h \
    pgeitems.h \
    graphdata.h \
    graphdefines.h \
    pgewndprops.h \
    graphresources.h \
    graphserializer.h \
    stdconsoleinterface.h \
    ../components/singleapplication/singleapplication.h \
    helpsystem.h \
    helpcontentwindow.h

RESOURCES += \
    resources.qrc

win32 {
  RC_ICONS += ../misc/impresario.ico
  DEFINES += QT_QTPROPERTYBROWSER_IMPORT
  contains(QT_ARCH, i386) {
    CONFIG(release, release|debug) {
      DESTDIR = $$_PRO_FILE_PWD_/../../../bin-win32-release
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/qtpropertybrowser/lib/win32) -lQt5PropertyBrowser
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/libavoid/lib/win32) -llibavoid
    }
    CONFIG(debug, release|debug) {
      DESTDIR = $$_PRO_FILE_PWD_/../../../bin-win32-debug
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/qtpropertybrowser/lib/win32) -lQt5PropertyBrowserd
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/libavoid/lib/win32) -llibavoidd
    }
  }
  else {
    CONFIG(release, release|debug) {
      DESTDIR = $$_PRO_FILE_PWD_/../../../bin-win64-release
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/qtpropertybrowser/lib/win64) -lQt5PropertyBrowser
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/libavoid/lib/win64) -llibavoid
    }
    CONFIG(debug, release|debug) {
      DESTDIR = $$_PRO_FILE_PWD_/../../../bin-win64-debug
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/qtpropertybrowser/lib/win64) -lQt5PropertyBrowserd
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/libavoid/lib/win64) -llibavoidd
    }
  }
}

unix {
  contains(QT_ARCH, i386) {
    CONFIG(release, release|debug) {
      DESTDIR = $$_PRO_FILE_PWD_/../../../bin-unix32-release
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/qtpropertybrowser/lib/unix32) -lQt5PropertyBrowser
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/libavoid/lib/unix32) -lavoid
    }
    CONFIG(debug, release|debug) {
      DESTDIR = $$_PRO_FILE_PWD_/../../../bin-unix32-debug
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/qtpropertybrowser/lib/unix32) -lQt5PropertyBrowserd
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/libavoid/lib/unix32) -lavoidd
    }
  }
  else {
    CONFIG(release, release|debug) {
      DESTDIR = $$_PRO_FILE_PWD_/../../../bin-unix64-release
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/qtpropertybrowser/lib/unix64) -lQt5PropertyBrowser
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/libavoid/lib/unix64) -lavoid
    }
    CONFIG(debug, release|debug) {
      DESTDIR = $$_PRO_FILE_PWD_/../../../bin-unix64-debug
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/qtpropertybrowser/lib/unix64) -lQt5PropertyBrowserd
      LIBS += $$quote(-L$$_PRO_FILE_PWD_/../components/libavoid/lib/unix64) -lavoidd
    }
  }
}

OTHER_FILES += \
    ../../../resources/PropertyWidget.qml \
    ../../../resources/IntSpinBox.qml \
    ../../../resources/StringLineEdit.qml \
    ../../../resources/BoolComboBox.qml \
    ../../../resources/StringDirSelector.qml \
    ../../../resources/IntComboBox.qml \
    ../../../resources/RealSpinBox.qml \
    ../../../resources/StringFileSelector.qml \












