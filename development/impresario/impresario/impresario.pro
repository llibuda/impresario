#******************************************************************************************
#   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
#   Copyright (C) 2015-2020  Lars Libuda
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
QT += core gui widgets network xmlpatterns qml quick quickwidgets concurrent help webenginewidgets
CONFIG += c++11
TARGET = Impresario
TEMPLATE = app
VERSION = 2.1.1
QMAKE_TARGET_DESCRIPTION = "Image Processing Engineering System applying Reusable Interactive Objects"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2015-2020  Lars Libuda"

!include(../impresario_bin_path.pri) {
  error(Failed to include impresario_bin_path.pri)
}
DESTDIR = $${IMPRESARIO_BIN_PATH}

win32 {
  QMAKE_POST_LINK = $$quote($${_PRO_FILE_PWD_}/../qt_deploy_win.bat) "$${DESTDIR}/$${TARGET}.exe"
  RC_ICONS += ../misc/impresario.ico
  DEFINES += QT_QTPROPERTYBROWSER_IMPORT

  CONFIG(release, release|debug) {
    LIBS += $$quote(-L../components/libavoid/release) -llibavoid
    LIBS += $$quote(-L../components/qtpropertybrowser/release) -lQt5PropertyBrowser
  }
  CONFIG(debug, release|debug) {
    LIBS += $$quote(-L../components/libavoid/debug) -llibavoidd
    LIBS += $$quote(-L../components/qtpropertybrowser/debug) -lQt5PropertyBrowserd
    # Add support for Visual Leak Detector 2.5.1 for debug version on Windows.
    # You also have to add "C:\Program Files (x86)\Visual Leak Detector\bin\Win64" to
    # the PATH environment variable.
    # Comment out the following two lines if Visual Leak Detector is not installed
    #INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include/"
    #LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
  }
}

unix {
  QMAKE_POST_LINK = $$quote($${_PRO_FILE_PWD_}/../qt_deploy_unix.sh) "$${DESTDIR}/$${TARGET}"
  CONFIG(release, release|debug) {
    LIBS += $$quote(-L../components/libavoid) -lavoid
    LIBS += $$quote(-L../components/qtpropertybrowser) -lQt5PropertyBrowser
  }
  CONFIG(debug, release|debug) {
    LIBS += $$quote(-L../components/libavoid) -lavoidd
    LIBS += $$quote(-L../components/qtpropertybrowser) -lQt5PropertyBrowserd
  }
}

INCLUDEPATH += $$quote($${_PRO_FILE_PWD_}/../components/qtpropertybrowser/src)
INCLUDEPATH += $$quote($${_PRO_FILE_PWD_}/../components/libavoid/source)
INCLUDEPATH += $$quote($${_PRO_FILE_PWD_}/../components/singleapplication)

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
    helpwindows.cpp

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
    helpwindows.h

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    ../../../resources/PropertyWidget.qml \
    ../../../resources/IntSpinBox.qml \
    ../../../resources/StringLineEdit.qml \
    ../../../resources/BoolComboBox.qml \
    ../../../resources/StringDirSelector.qml \
    ../../../resources/IntComboBox.qml \
    ../../../resources/RealSpinBox.qml \
    ../../../resources/StringFileSelector.qml

DISTFILES +=












