# define correct file name post fixes for various builds
BUILD_POSTFIX = qt$${QT_MAJOR_VERSION}$${QT_MINOR_VERSION}$${QT_PATCH_VERSION}
win32 {
  if(win32-g++*) {
    BUILD_POSTFIX = win32_$${BUILD_POSTIFX}_mingw$${system( g++ -dumpversion )}
  }
  else {
    if (win32-msvc*) {
      contains(QT_ARCH, i386) {
        BUILD_POSTFIX = win32_$${BUILD_POSTFIX}_vc$${MSVC_VER}
      }
      else {
        BUILD_POSTFIX = win64_$${BUILD_POSTFIX}_vc$${MSVC_VER}
      }
    }
    else {
      error(The current compiler is not supported.)
    }
  }
}

unix {
  BUILD_POSTFIX = $${BUILD_POSTFIX}_gcc$${system( g++ -dumpversion )}
  contains(QT_ARCH, i386) {
    BUILD_POSTFIX = unix32_$${BUILD_POSTFIX}
  }
  else {
    BUILD_POSTFIX = unix64_$${BUILD_POSTFIX}
  }
}

CONFIG(debug, release|debug) {
  BUILD_POSTFIX = $${BUILD_POSTFIX}_debug
  BUILD_TYPE = debug
}
CONFIG(release, release|debug) {
  BUILD_POSTFIX = $${BUILD_POSTFIX}_release
  BUILD_TYPE = release
}

IMPRESARIO_BIN_PATH = $$clean_path($${PWD}/../../bin_$${BUILD_POSTFIX})
