QT += core gui widgets
TEMPLATE = lib
CONFIG += shared

!include(../../impresario_bin_path.pri) {
  error(Failed to include impresario_bin_path.pri)
}

win32 {
  DEFINES += QT_QTPROPERTYBROWSER_EXPORT
  CONFIG(release, release|debug) {
    TARGET = Qt5PropertyBrowser
  }
  CONFIG(debug, release|debug) {
    TARGET = Qt5PropertyBrowserd
  }
  QMAKE_POST_LINK = copy /Y $$shell_path($${OUT_PWD})\\$${BUILD_TYPE}\*.dll $$shell_path($${IMPRESARIO_BIN_PATH})
}

unix {
  CONFIG(release, release|debug) {
    TARGET = Qt5PropertyBrowser
  }
  CONFIG(debug, release|debug) {
    TARGET = Qt5PropertyBrowserd
  }
  QMAKE_POST_LINK = cp $$shell_path($${OUT_PWD})/$${BUILD_TYPE}/*.so* $$shell_path($${IMPRESARIO_BIN_PATH})
}

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
SOURCES += src/qtpropertybrowser.cpp \
        src/qtpropertymanager.cpp \
        src/qteditorfactory.cpp \
        src/qtvariantproperty.cpp \
        src/qttreepropertybrowser.cpp \
        src/qtbuttonpropertybrowser.cpp \
        src/qtgroupboxpropertybrowser.cpp \
        src/qtpropertybrowserutils.cpp
HEADERS += src/qtpropertybrowser.h \
        src/qtpropertymanager.h \
        src/qteditorfactory.h \
        src/qtvariantproperty.h \
        src/qttreepropertybrowser.h \
        src/qtbuttonpropertybrowser.h \
        src/qtgroupboxpropertybrowser.h \
        src/qtpropertybrowserutils_p.h
RESOURCES += src/qtpropertybrowser.qrc

