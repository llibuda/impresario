#! /bin/bash

scriptPath=`which "$0"`
scriptFileName="${scriptPath##*/}"
scriptDirectory="${scriptPath%/*}"

echo "Deploy Qt on Linux"

# check number of arguments
if [ $# -ne 1 ]; then
  echo "Invalid number of arguments. Usage: qt_deploy_unix <QT_BINARY_APP>"
  exit 1
fi

# check whether file given in first argument exists
if [ ! -f "$1" ]; then
  echo "$1 does not exist."
  exit 1
fi

# check whether file is executable
if [ ! -x "$1" ]; then
  echo "$1 is not executable."
  exit 1
fi

# check for environment variable QTDIR
if [ -z "$QTDIR" ]; then
    echo "Environment variable QTDIR is not set."
    exit 1
fi

# check whether environment variable QTDIR points to existing directory
if [ ! -d "$QTDIR" ]; then
    echo "Environment variable QTDIR points to non-existing directory $QTDIR."
    exit 1
fi

echo "Deploy Qt for $1 using $QTDIR..."

# split first argument into directory and filename
targetFilePath=$1
targetFileName="${targetFilePath##*/}"
targetDirectory="${targetFilePath%/*}"

#echo "File name: $targetFileName"
#echo "Path: $targetDirectory"

# deploy Qt Libraries
echo "Copying Qt libraries..."
targetDeps=$(ldd "$targetFilePath" | awk 'BEGIN{ORS=" "}$1~/^\//{print $1}$3~/^\//{print $3}' | sed 's/,$/\n/')
for dep in $targetDeps
do
  if [[ "$dep" = *"$QTDIR"* ]]; then
    cp "$dep" "$targetDirectory/qtlib"
  fi
done

# copy Qt plugins to target directory
if [ -d "$targetDirectory/qtplugins" ]; then
  echo "Cleaning up existing Qt plugins..."
  rm -r "$targetDirectory/qtplugins"
fi
echo "Copying Qt plugins..."
cp -r "$QTDIR/plugins" "$targetDirectory/qtplugins"

# copy Qt libraries for some plugins, especially for X11 support
cp "$QTDIR/lib/libQt5DBus.so.5" "$targetDirectory/qtlib"
cp "$QTDIR/lib/libQt5XcbQpa.so.5" "$targetDirectory/qtlib"

# deploy Qt Webengine
if [ -d "$targetDirectory/qtweb" ]; then
  echo "Cleaning up existing Qt Webengine..."
  rm -r "$targetDirectory/qtweb"
fi
echo "Copying Qt Webengine..."
cp -r "$QTDIR/resources" "$targetDirectory/qtweb"
cp -r "$QTDIR/libexec/QtWebEngineProcess" "$targetDirectory/qtweb"
mkdir "$targetDirectory/qtweb/qtwebengine_locales"
cp "$QTDIR/translations/qtwebengine_locales/en-US.pak" "$targetDirectory/qtweb/qtwebengine_locales"

# copy QML components to target directory
if [ -d "$targetDirectory/qtqml" ]; then
  echo "Cleaning up existing QML components..."
  rm -r "$targetDirectory/qtqml"
fi
echo "Copying QML components..."
cp -r "$QTDIR/qml" "$targetDirectory/qtqml"

# copy Impresario specific files
cp "$scriptDirectory/misc/impresario.sh" "$targetDirectory"
cp "$scriptDirectory/misc/impresario.svg" "$targetDirectory"
cp "$scriptDirectory/misc/Impresario.xml" "$targetDirectory"

# create standard paths for macro libraries and process graphs
if [ ! -d "$targetDirectory/../lib" ]; then
  echo "Creating standard directory for macro libraries..."
  mkdir "$targetDirectory/../lib"
fi
if [ ! -d "$targetDirectory/../processgraphs" ]; then
  echo "Creating standard directory for process graphs..."
  mkdir "$targetDirectory/../processgraphs"
fi


