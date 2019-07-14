/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2019  Lars Libuda
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

#include "appmacromanager.h"
#include "appmacro.h"
#include "sysloglogger.h"
#include <QDir>
#include <QRegularExpression>
#include <QtConcurrent>

namespace app
{
  MacroManager MacroManager::macroManager;

  MacroManager::MacroManager() : graph::ElementManager(), libList()
  {
  }

  MacroManager::~MacroManager()
  {
    QMutexLocker lock(&mutex);
    viewers.clear();
  }

  void MacroManager::loadPrototypes(const QStringList &dirs)
  {
    QtConcurrent::run(this,&MacroManager::doLoadPrototypes,dirs);
  }

  void MacroManager::doLoadPrototypes(const QStringList &dirs)
  {
    emit loadPrototypesStarted();

    unloadPrototypes();

    graph::EdgeData::Ptr impresarioEdge = graph::EdgeData::Ptr(new app::MacroLink());
    impresarioEdge->setSignature("Impresario::DataFlowEdge");
    registerEdgeDataType(impresarioEdge);

    QStringList patterns;
#ifdef Q_OS_WIN
    patterns.append("*.dll");
#else
    patterns.append("*.so");
    patterns.append("*.so.*");
    patterns.append("*.so.*.*");
    patterns.append("*.so.*.*.*");
#endif
    unsigned int cntMacros = 0;
    unsigned int cntViewers = 0;
    QStringList files;
    for(int dindex = 0; dindex < dirs.count(); ++dindex)
    {
      QDir dir(dirs[dindex]);
      QStringList dirFileList = dir.entryList(patterns, QDir::Files | QDir::NoSymLinks, QDir::Name | QDir::IgnoreCase);
      for(int findex = 0; findex < dirFileList.count(); ++findex)
      {
        files.append(dirs[dindex] + '/' + dirFileList[findex]);
      }
    }
    emit loadPrototypesProgress(0,files.count(),tr("Loading macro library file %v of %m."));
    for(int findex = 0; findex < files.count(); ++findex)
    {
#ifdef Q_OS_WIN
      MacroLibrary* lib = new MacroLibraryDLL();
      if (lib->load(files[findex],*this))
      {
        cntMacros += lib->countMacros();
        cntViewers += lib->countViewers();
        libList.append(lib);
      }
      else
      {
        delete lib;
      }
#else
      QRegularExpression regExp("(.*\\.so)(\\.\\d+){0,3}");
      if (regExp.match(files[findex]).hasMatch())
      {
        MacroLibrary* lib = new MacroLibraryDLL();
        if (lib->load(files[findex],*this))
        {
          cntMacros += lib->countMacros();
          cntViewers += lib->countViewers();
          libList.append(lib);
        }
        else
        {
          delete lib;
        }
      }
#endif
      emit loadPrototypesProgress(findex + 1,files.count());
    }
    if (libList.size() > 0)
    {
      if (libList.size() == 1)
      {
        switch(cntMacros + cntViewers)
        {
        case 0:
          syslog::warning(QString(QObject::tr("Loaded 1 library but no elements registered.")),QObject::tr("Libraries"));
          break;
        default:
          syslog::info(QString(QObject::tr("Loaded 1 library. Registered macros: %1. Registered viewers: %2")).arg(cntMacros).arg(cntViewers),QObject::tr("Libraries"));
          break;
        }
      }
      else
      {
        switch(cntMacros + cntViewers)
        {
        case 0:
          syslog::warning(QString(QObject::tr("Loaded %1 libraries but no elements registered.")).arg(libList.size()),QObject::tr("Libraries"));
          break;
        default:
          syslog::info(QString(QObject::tr("Loaded %1 libraries. Registered macros: %2. Registered viewers: %3")).arg(libList.size()).arg(cntMacros).arg(cntViewers),QObject::tr("Libraries"));
          break;
        }
      }
    }
    else
    {
      syslog::warning(QObject::tr("No libraries loaded. Please check the search paths in settings."),QObject::tr("Libraries"));
    }

    emit loadPrototypesFinished();
  }

  void MacroManager::unloadPrototypes()
  {
    QMutexLocker lock(&mutex);
    viewers.clear();
    clear();
    // It is crucial to release loaded libraries in reverse order because libraries redirect std:cout and std::cerr.
    // Their previous addresses have to be restored in correct order.
    while(!libList.empty())
    {
      delete libList.last();
      libList.pop_back();
    }
  }

  bool MacroManager::registerMacroViewer(MacroViewer::Ptr viewer)
  {
    QMutexLocker lock(&mutex);
    if (!viewer.isNull())
    {
      foreach(QString typeSignature,viewer->dataTypes())
      {
        viewers.insert(typeSignature,viewer);
      }
      return true;
    }
    return false;
  }

  MacroViewer::Ptr MacroManager::createMacroViewerInstance(const QString &dataType)
  {
    QMutexLocker lock(&mutex);
    if (!viewers.contains(dataType))
    {
      return MacroViewer::Ptr();
    }
    MacroViewer::Ptr viewerPtr = viewers.value(dataType);
    return viewerPtr->clone().staticCast<MacroViewer>();
  }

  void MacroManager::iterateViewerTypes(IteratorFunction iterator, ... ) const
  {
    Q_ASSERT(iterator != 0);
    QMutexLocker lock(&mutex);
    QSet<app::MacroViewer*> visitedViewers;
    for(ViewerMap::const_iterator it = viewers.begin(); it != viewers.end(); ++it)
    {
      if (!visitedViewers.contains(it.value().data()))
      {
        visitedViewers.insert(it.value().data());
        va_list args;
        va_start(args,iterator);
        if (!(*iterator)(it.value(),args))
        {
          va_end(args);
          return;
        }
        va_end(args);
      }
    }
  }

  QString MacroManager::matchVertexSignature(const VertexDataTypeMap& vertexDataTypes, const QString& signature)
  {
    // if we have an exact match of signature, return it and we're done
    if (vertexDataTypes.contains(signature)) return signature;
    // no exact match found, search for best matching alternative
    QChar buildTypeChar = signature[signature.length()-1];
    QString regMacroLibrary = signature.left(signature.lastIndexOf('_'));
    QString regStrMatchFullLibrary = QString("%1_\\d\\d+%2$").arg(QRegularExpression::escape(regMacroLibrary)).arg(buildTypeChar);
    QRegularExpression regExpMatchFullLibrary(regStrMatchFullLibrary);
    for(VertexDataTypeMap::const_iterator it = vertexDataTypes.begin(); it != vertexDataTypes.end(); ++it)
    {
      if (regExpMatchFullLibrary.match(it.key()).hasMatch())
      {
        return it.key();
      }
    }
    return QString();
  }

}
