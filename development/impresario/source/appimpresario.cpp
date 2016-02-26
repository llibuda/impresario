/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2016  Lars Libuda
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

#include "appimpresario.h"
#include "sysloglogger.h"
#include "appmacromanager.h"
#include "resources.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDir>
#include <QStringList>
#include <stdlib.h>
#include <QClipboard>
#include <QDebug>

namespace app
{
  Impresario* Impresario::appInstance = 0;

  bool Impresario::readXmlFile(QIODevice &device, QSettings::SettingsMap &map)
  {
    QXmlStreamReader xmlReader(&device);
    QStringList elements;
    while (!xmlReader.atEnd() && !xmlReader.hasError())
    {
      xmlReader.readNext();

      if (xmlReader.isStartElement() && xmlReader.name() != "Settings")
      {
        elements.append(xmlReader.name().toString());
      }
      else if (xmlReader.isEndElement())
      {
        if(!elements.isEmpty()) elements.removeLast();
      }
      else if (xmlReader.isCharacters() && !xmlReader.isWhitespace())
      {
        QString key;
        for(int i = 0; i < elements.size(); i++)
        {
          if(i != 0) key += "/";
          key += elements.at(i);
        }
        map[key] = xmlReader.text().toString();
      }
    }

    if (xmlReader.hasError())
    {
      qWarning() << xmlReader.errorString();
      return false;
    }

    return true;
  }

  bool Impresario::writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map)
  {
    QXmlStreamWriter xmlWriter(&device);

    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("Settings");

    QStringList prev_elements;
    QSettings::SettingsMap::ConstIterator map_i;

    for (map_i = map.begin(); map_i != map.end(); map_i++)
    {
      QStringList elements = map_i.key().split("/");
      int x = 0;
      while(x < prev_elements.size() && elements.at(x) == prev_elements.at(x))
      {
        x++;
      }

      for(int i = prev_elements.size() - 1; i >= x; i--)
      {
        //qDebug() << "</" << prev_elements.at(i) << ">";
        xmlWriter.writeEndElement();
      }

      for (int i = x; i < elements.size(); i++)
      {
        //qDebug() << "<" << elements.at(i) << ">";
        xmlWriter.writeStartElement(elements.at(i));
      }

      xmlWriter.writeCharacters(map_i.value().toString().toUtf8());
      //qDebug() << map_i.value();

      prev_elements = elements;
    }

    for(int i = 0; i < prev_elements.size(); i++)
    {
      //qDebug() << "</" << prev_elements.at(i) << ">";
      xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    return true;
  }


  Impresario& Impresario::instance(int & argc, char ** argv)
  {
    if (!appInstance)
    {
      appInstance = new Impresario(argc,argv);
    }
    return *appInstance;
  }

  Impresario& Impresario::instance()
  {
    if (!appInstance)
    {
      int   argc = 0;
      char* argv = 0;
      appInstance = new Impresario(argc,&argv);
    }
    return *appInstance;
  }

  void Impresario::release()
  {
    if (appInstance)
    {
      QApplication::clipboard()->clear();
      delete appInstance;
      appInstance = 0;
    }
  }

  Impresario::Impresario(int & argc, char ** argv) : SingleApplication(argc,argv), wndActWindow(0)
  {
    // set application name, version, and organization for use
    // in QSettings instance
    setApplicationName("Impresario");
    setApplicationVersion("2.0");
    setOrganizationName("Impresario");

    // register new format for application settings
    QSettings::Format XmlFormat = QSettings::registerFormat("xml", readXmlFile, writeXmlFile);
    QSettings::setPath(XmlFormat,QSettings::SystemScope,applicationDirPath());
    QSettings::setDefaultFormat(XmlFormat);

    connect(this,SIGNAL(showUp()),this,SLOT(activatedByAnotherInstance()));
  }

  Impresario::~Impresario()
  {
    MacroManager::instance().unloadPrototypes();
  }

  bool Impresario::initCritical()
  {
    bool result = initResourcePath();
    if (result)
    {
      emit initCriticalSuccessful();
    }
    return result;
  }

  bool Impresario::initNonCritical()
  {
    bool result = true;
    result = initProcessGraphPath() && result;
    result = initDepLibPaths() && result;
    result = initMacroLibPaths() && result;
    initMacroLibraries();
    if (result)
    {
      emit initNonCriticalSuccessful();
    }
    return result;
  }

  void Impresario::activatedByAnotherInstance()
  {
    syslog::info(tr("Activated by another instance."));
    if (wndActWindow)
    {
      wndActWindow->setWindowState(wndActWindow->windowState() & ~Qt::WindowMinimized);
      wndActWindow->raise();
      wndActWindow->activateWindow();
    }
  }

  void Impresario::initMacroLibraries()
  {
    QStringList dirs = Resource::getPaths(Resource::SETTINGS_PATH_MACROS);
    MacroManager::instance().loadPrototypes(dirs);
  }

  void Impresario::settingChanged(Resource::SettingsIDs id)
  {
    switch(id)
    {
    case Resource::SETTINGS_PATH_MACROS:
      initMacroLibPaths();
      break;
    case Resource::SETTINGS_PATH_DEPLIBS:
      initDepLibPaths();
      break;
    case Resource::SETTINGS_PATH_PROCESSGRAPH:
      initProcessGraphPath();
      break;
    case Resource::SETTINGS_PATH_RESOURCES:
      initResourcePath();
      break;
    default:
      break;
    }
  }

  bool Impresario::initProcessGraphPath()
  {
    // check path for process graphs
    QStringList pgPaths;
    pgPaths.append(Resource::getPath(Resource::SETTINGS_PATH_PROCESSGRAPH));
    pgPaths.append(Resource::getPath(Resource::SETTINGS_PATH_PROCESSGRAPH,QSettings::SystemScope));
    pgPaths.append(QDir::toNativeSeparators(QDir(applicationDirPath() + "/../processgraphs").absolutePath()));
    pgPaths.append(QDir::toNativeSeparators(applicationDirPath()));
    QString path = QString();
    for(int i = 0; i < pgPaths.count(); ++i)
    {
      if (pgPaths[i].length() > 0)
      {
        QDir dir(pgPaths[i]);
        if (dir.exists()) // path exists -> all ok
        {
          path = pgPaths[i];
          if (i > 0)
          {
            Resource::setPath(Resource::SETTINGS_PATH_PROCESSGRAPH,path);
          }
          syslog::info(QString(tr("Configuration: Path to process graphs is '%1'.")).arg(path));
          break; // quit loop
        }
        else // path does not exist
        {
          syslog::warning(QString(tr("Configuration: Specified path '%1' to process graphs does not exist.")).arg(pgPaths[i]));
        }
      }
    }
    return true;
  }

  bool Impresario::initResourcePath()
  {
    // check for QML path
    QDir qmlPath(applicationDirPath() + "/qml");
    if (qmlPath.exists())
    {
      qmlEngineInstance.addImportPath(qmlPath.absolutePath());
    }
    else
    {
      syslog::error(QString(tr("Configuration: Path to QML resources '%1' does not exists. Please reinstall Impresario.")).arg(QDir::toNativeSeparators(qmlPath.absolutePath())));
      return false;
    }
    // check resource path and required files
    QStringList resPaths;
    resPaths.append(Resource::getPath(Resource::SETTINGS_PATH_RESOURCES));
    resPaths.append(Resource::getPath(Resource::SETTINGS_PATH_RESOURCES,QSettings::SystemScope));
    resPaths.append(QDir::toNativeSeparators(QDir(applicationDirPath() + "/../resources").absolutePath()));
    resPaths.append(QDir::toNativeSeparators(applicationDirPath()));
    QString path = QString();
    for(int i = 0; i < resPaths.count(); ++i)
    {
      if (resPaths[i].length() > 0)
      {
        QDir dir(resPaths[i]);
        if (dir.exists()) // path exists -> all ok
        {
          path = resPaths[i];
          if (i > 0)
          {
            Resource::setPath(Resource::SETTINGS_PATH_RESOURCES,path);
          }
          break; // quit loop
        }
        else // path does not exist
        {
          syslog::warning(QString(tr("Configuration: Specified path '%1' to resources does not exist.")).arg(resPaths[i]));
        }
      }
    }
    if (path.isEmpty())
    {
      syslog::error(QString(tr("Configuration: No valid path to resources found. Please reinstall Impresario.")));
      return false;
    }
    else
    {
      syslog::info(QString(tr("Configuration: Path to resources is '%1'.")).arg(path));
    }

    // check for process graph schema file
    QFileInfo xsdFile(path + "/processgraph.xsd");
    if (!xsdFile.exists() || !xsdFile.isFile())
    {
      syslog::error(QString(tr("Configuration: Schema file '%1' for process graphs does not exists. Please reinstall Impresario.")).arg(QDir::toNativeSeparators(xsdFile.fileName())));
      return false;
    }
    // check for default property widget
    QSettings settings;
    QFileInfo defPropFile(Resource::getPath(Resource::SETTINGS_PATH_RESOURCES) + '/' + settings.value(Resource::path((Resource::SETTINGS_PROP_DEFAULTWIDGET))).toString());
    if (!defPropFile.exists() || !defPropFile.isFile())
    {
      syslog::error(QString(tr("Configuration: Default QML property widget '%1' does not exists. Please reinstall Impresario.")).arg(QDir::toNativeSeparators(defPropFile.absoluteFilePath())));
      return false;
    }
    return true;
  }

  bool Impresario::initDepLibPaths()
  {
    QList<QSettings::Scope> settingsScopes;
    settingsScopes << QSettings::UserScope << QSettings::SystemScope;
    int index = 0;
    QStringList libPaths;
    do
    {
      QStringList paths = Resource::getPaths(Resource::SETTINGS_PATH_DEPLIBS,settingsScopes[index++]);
      for(int i = 0; i < paths.size(); ++i)
      {
        if (paths[i].length() == 0)
        {
          syslog::warning(tr("Configuration: Empty path for dependend libraries specified. Ignored."));
        }
        else
        {
          QDir dir(paths[i]);
          if (dir.exists())
          {
            libPaths.append(paths[i]);
          }
          else
          {
            syslog::warning(QString(tr("Configuration: Specified path '%1' for dependend libraries does not exist. Ignored.")).arg(paths[i]));
          }
        }
      }
      if ((index > 0 || paths != libPaths) && libPaths.size() > 0)
      {
        Resource::setPaths(Resource::SETTINGS_PATH_DEPLIBS,libPaths);
      }
    } while(libPaths.size() == 0 && index < settingsScopes.size());
    if (libPaths.size() > 0)
    {
      syslog::info(QString(tr("Configuration: Using the following paths for dependend libraries\n%1")).arg(libPaths.join("\n")));
      //foreach(QString path, libPaths)
      //{
      //  addLibraryPath(path);
      //}

      // Extend OS search path for dynamic linkable libraries
      // This solution works on Windows only. Linux does not allow to modify
      // LD_LIBRARY_PATH from an already started process. On Linux this tasks
      // has to be coded in a script which starts the Impresario process after
      // setting LD_LIBRARY_PATH
#ifdef Q_OS_WIN
      QString pathVariable(getenv("PATH"));
      pathVariable = libPaths.join(";") + ";" + pathVariable;
      pathVariable.prepend("PATH=");
      putenv(pathVariable.toLatin1().data());
#else
      Resource::setPaths(Resource::SETTINGS_PATH_DEPLIBS,libPaths);
#endif
    }
    else
    {
      syslog::info(tr("Configuration: No valid paths for dependend libraries specified."));
    }
    return true;
  }

  bool Impresario::initMacroLibPaths()
  {
    QList<QSettings::Scope> settingsScopes;
    settingsScopes << QSettings::UserScope << QSettings::SystemScope;
    int index = 0;
    QStringList libPaths;
    do
    {
      QStringList paths = Resource::getPaths(Resource::SETTINGS_PATH_MACROS,settingsScopes[index++]);
      for(int i = 0; i < paths.size(); ++i)
      {
        if (paths[i].length() == 0)
        {
          syslog::warning(tr("Configuration: Empty path for macro libraries specified. Ignored."));
        }
        else
        {
          QDir dir(paths[i]);
          if (dir.exists())
          {
            libPaths.append(paths[i]);
          }
          else
          {
            syslog::warning(QString(tr("Configuration: Specified path '%1' for macro libraries does not exist. Ignored.")).arg(paths[i]));
          }
        }
      }
      if ((index > 0 || paths != libPaths) && libPaths.size() > 0)
      {
        Resource::setPaths(Resource::SETTINGS_PATH_MACROS,libPaths);
      }
    } while(libPaths.size() == 0 && index < settingsScopes.size());
    if (libPaths.size() == 0)
    {
      syslog::warning(QString(tr("Configuration: No valid paths for macro libraries specified. Added '%1' as default.")).arg(QDir::toNativeSeparators(applicationDirPath())));
      libPaths.append(QDir::toNativeSeparators(applicationDirPath()));
      Resource::setPaths(Resource::SETTINGS_PATH_MACROS,libPaths);
    }
    syslog::info(QString(tr("Configuration: Using the following paths for macro libraries\n%1")).arg(libPaths.join("\n")));
    return true;
  }

}
