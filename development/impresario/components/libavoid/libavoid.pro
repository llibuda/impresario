#-------------------------------------------------
#
# Project created by QtCreator 2015-11-29T18:28:33
#
#-------------------------------------------------

TEMPLATE = lib
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ./source/

win32 {
  CONFIG += dll
  DEFINES += LIBAVOID_EXPORTS
  CONFIG(release, release|debug) {
    TARGET = libavoid
  }
  CONFIG(debug, release|debug) {
    TARGET = libavoidd
  }
  contains(QT_ARCH, i386) {
    DESTDIR = $$_PRO_FILE_PWD_/lib/win32
  }
  else {
    DESTDIR = $$_PRO_FILE_PWD_/lib/win64
  }
}

SOURCES += \
    source/actioninfo.cpp \
    source/connectionpin.cpp \
    source/connector.cpp \
    source/connend.cpp \
    source/geometry.cpp \
    source/geomtypes.cpp \
    source/graph.cpp \
    source/hyperedge.cpp \
    source/hyperedgeimprover.cpp \
    source/hyperedgetree.cpp \
    source/junction.cpp \
    source/makepath.cpp \
    source/mtst.cpp \
    source/obstacle.cpp \
    source/orthogonal.cpp \
    source/router.cpp \
    source/scanline.cpp \
    source/shape.cpp \
    source/timer.cpp \
    source/vertices.cpp \
    source/viscluster.cpp \
    source/visibility.cpp \
    source/vpsc.cpp

HEADERS += \
    source/libavoid/actioninfo.h \
    source/libavoid/assertions.h \
    source/libavoid/connectionpin.h \
    source/libavoid/connector.h \
    source/libavoid/connend.h \
    source/libavoid/debug.h \
    source/libavoid/debughandler.h \
    source/libavoid/dllexport.h \
    source/libavoid/geometry.h \
    source/libavoid/geomtypes.h \
    source/libavoid/graph.h \
    source/libavoid/hyperedge.h \
    source/libavoid/hyperedgeimprover.h \
    source/libavoid/hyperedgetree.h \
    source/libavoid/junction.h \
    source/libavoid/libavoid.h \
    source/libavoid/makepath.h \
    source/libavoid/mtst.h \
    source/libavoid/obstacle.h \
    source/libavoid/orthogonal.h \
    source/libavoid/router.h \
    source/libavoid/scanline.h \
    source/libavoid/shape.h \
    source/libavoid/timer.h \
    source/libavoid/vertices.h \
    source/libavoid/viscluster.h \
    source/libavoid/visibility.h \
    source/libavoid/vpsc.h

