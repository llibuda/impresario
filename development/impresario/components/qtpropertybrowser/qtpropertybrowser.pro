QT += core gui widgets
TEMPLATE = lib
CONFIG += dll

win32 {
  DEFINES += QT_QTPROPERTYBROWSER_EXPORT
  CONFIG(release, release|debug) {
    TARGET = Qt5PropertyBrowser
  }
  CONFIG(debug, release|debug) {
    TARGET = Qt5PropertyBrowserd
  }
  contains(QT_ARCH, i386) {
    DESTDIR = $$_PRO_FILE_PWD_/lib/win32
  }
  else {
    DESTDIR = $$_PRO_FILE_PWD_/lib/win64
  }
}

unix {
  CONFIG(release, release|debug) {
    TARGET = Qt5PropertyBrowser
  }
  CONFIG(debug, release|debug) {
    TARGET = Qt5PropertyBrowserd
  }
  contains(QT_ARCH, i386) {
    DESTDIR = $$_PRO_FILE_PWD_/lib/unix32
  }
  else {
    DESTDIR = $$_PRO_FILE_PWD_/lib/unix64
  }
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

