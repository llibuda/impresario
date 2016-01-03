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

#include "appmacrolibrary.h"
#include "appmacromanager.h"
#include "appmacro.h"
#include "appbuildinfo.h"
#include "sysloglogger.h"
#include "stdconsoleinterface.h"
#include <QObject>
#include <QDir>
#include <QFileInfo>

namespace app
{
  //-----------------------------------------------------------------------
  // Class MacroLibrary
  //-----------------------------------------------------------------------
  MacroLibrary::MacroLibrary() : libPath(), libName(), libCreator(), libDescription(), libBuildDate(),
    libCompiler(), libVersionString(QObject::tr("Undefined")), libBuildType(), libQtVersionString(QObject::tr("None")), libAPIVersionString(QObject::tr("Undefined")),
    libVersion(0), libCompilerVersion(0), libQtVersion(0), libAPIVersion(0), libCntMacros(0), libCntViewers(0), libIsDebug(false)
  {
  }

  MacroLibrary::MacroLibrary(const MacroLibrary& /*other*/)
  {
  }

  MacroLibrary::~MacroLibrary()
  {
  }

  //-----------------------------------------------------------------------
  // Class MacroLibraryDLL
  //-----------------------------------------------------------------------
  const QString MacroLibraryDLL::InterfaceFunctionNames[] = {
    "libGetBuildDate",
    "libGetCompiler",
    "libGetCompilerId",
    "libGetQtVersion",
    "libIsDebugVersion",
    "libGetName",
    "libGetVersion",
    "libGetAPIVersion",
    "libGetCreator",
    "libGetDescription",
    "libInitialize",
    "libTerminate",
    "macroClone",
    "macroDelete",
    "macroGetType",
    "macroGetName",
    "macroGetCreator",
    "macroGetGroup",
    "macroGetDescription",
    "macroGetErrorMsg",
    "macroGetPropertyWidgetComponent",
    "macroGetInputs",
    "macroGetOutputs",
    "macroGetParameters",
    "macroStart",
    "macroApply",
    "macroStop",
    "macroSetParameterValue",
    "macroGetParameterValue",
    "macroSetImpresarioDataPtr",
    "macroGetImpresarioDataPtr",
    "macroCreateWidget",
    "macroDestroyWidget",
    "\0"
  };

  MacroLibraryDLL::MacroLibraryDLL() : MacroLibrary()
  {
  }

  MacroLibraryDLL::MacroLibraryDLL(const MacroLibraryDLL& other) : MacroLibrary(other)
  {
  }

  MacroLibraryDLL::~MacroLibraryDLL()
  {
    if (libHandler.isLoaded())
    {
      // Free memory allocated within library
      FunctionMap::const_iterator it = functions.find(libTerminate);
      if (it != functions.end())
      {
        PFN_LIBTERM(it.value())();
      }
      libHandler.unload();
    }
  }

  bool MacroLibraryDLL::load(const QString& path, MacroManager& manager)
  {
    MacroHandle* macroList = NULL;
    libHandler.setFileName(path);
    if (!libHandler.load())
    {
      syslog::error(QString(QObject::tr("Libraries: Operating system cannot load '%1'. Maybe dependencies are missing?")).arg(QDir::toNativeSeparators(path)));
      return false;
    }
    // load symbols from library
    int index = 0;
    while(InterfaceFunctionNames[index].length() > 0) {
      void* funcPtr = (void*)libHandler.resolve(InterfaceFunctionNames[index].toLatin1().constData());
      if (funcPtr == NULL) {
        // symbol not found -> invalid library
        functions.clear();
        libHandler.unload();
        syslog::error(QString(QObject::tr("Libraries: Missing symbol '%1' in library '%2'. Library not loaded.")).arg(InterfaceFunctionNames[index]).arg(QDir::toNativeSeparators(path)));
        return false;
      }
      else {
        // insert function pointer
        functions.insert((LibFunctions)index,funcPtr);
      }
      ++index;
    }
    // initialize the library
    unsigned int cntElements;
    bool init = PFN_LIBINIT(functions[libInitialize])(&macroList,&cntElements,&std::ConsoleInterface::receivedStdOut,&std::ConsoleInterface::receivedStdErr,&cbMacroParameterChanged);
    if (!init || cntElements == 0) {
      functions.clear();
      libHandler.unload();
      if (!init) {
        syslog::error(QString(QObject::tr("Libraries: '%1' could not be initialized. Library not loaded.")).arg(QDir::toNativeSeparators(path)));
      }
      else {
        syslog::warning(QString(QObject::tr("Libraries: '%1' does not contain any macros. Library not loaded.")).arg(QDir::toNativeSeparators(path)));
      }
      return false;
    }
    // initialize remaining attributes
    libName = QString::fromWCharArray(PFN_LIBSTRING(functions[libGetName])());
    libPath = QDir::toNativeSeparators(path);
    libCreator = QString::fromWCharArray(PFN_LIBSTRING(functions[libGetCreator])());
    libDescription = QString::fromWCharArray(PFN_LIBSTRING(functions[libGetDescription])());
    libBuildDate = QString::fromWCharArray(PFN_LIBSTRING(functions[libGetBuildDate])());
    libCompiler = QString::fromWCharArray(PFN_LIBSTRING(functions[libGetCompiler])());
    libCompilerVersion = PFN_LIBUINT(functions[libGetCompilerId])();
    libVersion = PFN_LIBUINT(functions[libGetVersion])();
    libQtVersion = PFN_LIBUINT(functions[libGetQtVersion])();
    libAPIVersion = PFN_LIBUINT(functions[libGetAPIVersion])();
    if (libQtVersion > 0)
    {
      unsigned int patch = libQtVersion & 0xFF;
      unsigned int minor = (libQtVersion & 0xFF00) >> 8;
      unsigned int major = (libQtVersion & 0xFF0000) >> 16;
      libQtVersionString = QString("%1.%2.%3").arg(major,0,16).arg(minor,0,16).arg(patch,0,16);
    }
    if (libAPIVersion > 0)
    {
      unsigned int minor = (libAPIVersion & 0xFF00) >> 8;
      unsigned int major = (libAPIVersion & 0xFF0000) >> 16;
      libAPIVersionString = QString("%1.%2").arg(major).arg(minor);
    }
    if (libVersion > 0)
    {
      unsigned int patch = libVersion & 0xFF;
      unsigned int minor = (libVersion & 0xFF00) >> 8;
      unsigned int major = (libVersion & 0xFF0000) >> 16;
      libVersionString = QString("%1.%2.%3").arg(major,0,16).arg(minor).arg(patch);
    }
    libIsDebug = PFN_LIBBOOL(functions[libIsDebugVersion])();
    libBuildType = app::BuildInfo::instance().buildString(libCompilerVersion,libIsDebug);
    // try to add all macros of the library to the macro map
    for(unsigned int index = 0; index < cntElements; ++index)
    {
      unsigned int macroType = getMacroType(macroList[index]);
      switch(macroType)
      {
        case 0: // load a normal macro not depending on Qt
        {
          graph::VertexData::Ptr macro = graph::VertexData::Ptr(new MacroDLL(*this,macroList[index]));
          if (!macro.isNull())
          {
            if (!manager.registerVertexDataType(macro))
            {
              syslog::warning(QString(QObject::tr("Libraries: Macro '%1' from library '%2' is already contained in database.")).arg(static_cast<app::Macro*>(macro.data())->getName()).arg(QDir::toNativeSeparators(path)));
              macro = graph::VertexData::Ptr();
            }
            else
            {
              libCntMacros++;
            }
          }
          else
          {
            syslog::error(QString(QObject::tr("Libraries: Failed to allocate memory for a macro in '%1'.")).arg(QDir::toNativeSeparators(path)));
          }
          break;
        }
        case 1: // load an extended macro depending on Qt -> check for runtime compatibility!
        {
          graph::VertexData::Ptr macro = graph::VertexData::Ptr(new MacroDLL(*this,macroList[index]));
          if (!macro.isNull())
          {
            if (isMacroCompatible(*static_cast<app::MacroDLL*>(macro.data())))
            {
              if (!manager.registerVertexDataType(macro))
              {
                syslog::warning(QString(QObject::tr("Libraries: Macro '%1' from library '%2' is already contained in database.")).arg(static_cast<app::Macro*>(macro.data())->getName()).arg(QDir::toNativeSeparators(path)));
                macro = graph::VertexData::Ptr();
              }
              else
              {
                libCntMacros++;
              }
            }
          }
          else
          {
            syslog::error(QString(QObject::tr("Libraries: Failed to allocate memory for a macro in '%1'.")).arg(QDir::toNativeSeparators(path)));
          }
          break;
        }
        case 2: // load a viewer depending on Qt
        {
          app::MacroViewer::Ptr viewer = app::MacroViewer::Ptr(new MacroViewer(*this,macroList[index]));
          if (!viewer.isNull())
          {
            if (isMacroCompatible(*viewer.data()))
            {
              if (!manager.registerMacroViewer(viewer))
              {
                syslog::warning(QString(QObject::tr("Libraries: Viewer '%1' from library '%2' could not be registered.")).arg(static_cast<app::Macro*>(viewer.data())->getName()).arg(QDir::toNativeSeparators(path)));
                viewer = app::MacroViewer::Ptr();
              }
              else
              {
                libCntViewers++;
              }
            }
          }
          else
          {
            syslog::error(QString(QObject::tr("Libraries: Failed to allocate memory for a viewer in '%1'.")).arg(QDir::toNativeSeparators(path)));
          }
          break;
        }
        default:
          syslog::warning(QString(QObject::tr("Libraries: '%1' contains macros of invalid type. Skipping.")).arg(QDir::toNativeSeparators(path)));
          break;
      }
    }
    if (libCntMacros + libCntViewers == 0)
    {
      syslog::warning(QString(QObject::tr("Libraries: '%1' skipped. Neither macros nor viewers loaded.")).arg(QDir::toNativeSeparators(path)));
      return false;
    }
    syslog::info(QString(QObject::tr("Libraries: '%1' loaded. Registered macros: %2. Registered viewers: %3.")).arg(QDir::toNativeSeparators(path)).arg(libCntMacros).arg(libCntViewers));
    return true;
  }

  bool MacroLibraryDLL::isMacroCompatible(const MacroDLL& macro)
  {
    unsigned int impresarioCompiler = app::BuildInfo::instance().compilerId();
    unsigned int libraryCompiler = libCompilerVersion;
    if (impresarioCompiler / 1000000 != libraryCompiler / 1000000)
    {
      syslog::error(QString(QObject::tr("Libraries: %3 '%1' from library '%2' uses run time libraries incompatible to Impresario. %3 is skipped.")).arg(macro.getName()).arg(QDir::toNativeSeparators(macro.getLibrary().getPath())).arg(macro.getClass()));
      return false;
    }
    //unsigned int libQtPatch = libQtVersion & 0xFF;
    //unsigned int libQtMinor = (libQtVersion & 0xFF00) >> 8;
    unsigned int libQtMajor = (libQtVersion & 0xFF0000) >> 16;
    //unsigned int impresarioQtPatch = QT_VERSION & 0xFF;
    //unsigned int impresarioQtMinor = (QT_VERSION & 0xFF00) >> 8;
    unsigned int impresarioQtMajor = (QT_VERSION & 0xFF0000) >> 16;
    if (libQtMajor != impresarioQtMajor)
    {
      syslog::error(QString(QObject::tr("Libraries: %3 '%1' from library '%2' uses Qt version incompatible to Impresario. %3 is skipped.")).arg(macro.getName()).arg(QDir::toNativeSeparators(macro.getLibrary().getPath())).arg(macro.getClass()));
      return false;
    }
    if (app::BuildInfo::instance().isDebugVersion() != isDebugVersion())
    {
      syslog::error(QString(QObject::tr("Libraries: %3 '%1' from library '%2' has incompatible build type to Impresario. %3 is skipped.")).arg(macro.getName()).arg(QDir::toNativeSeparators(macro.getLibrary().getPath())).arg(macro.getClass()));
      return false;
    }
    if (impresarioCompiler / 10000 != libraryCompiler / 10000)
    {
      syslog::warning(QString(QObject::tr("Libraries: %3 '%1' from library '%2' uses different compiler compared to Impresario.\nMaybe incompatible run time libraries will leed to unexpected faults. You have been warned!")).arg(macro.getName()).arg(QDir::toNativeSeparators(macro.getLibrary().getPath())).arg(macro.getClass()));
    }
    return true;
  }

  void MacroLibraryDLL::cbMacroParameterChanged(MacroHandle /*handle*/, unsigned int paramIndex, void* dataPtr)
  {
    MacroDLL* macroPtr = reinterpret_cast<MacroDLL*>(dataPtr);
    if (macroPtr)
    {
      macroPtr->parameterChangedByMacro(paramIndex);
    }
  }

  unsigned int MacroLibraryDLL::getMacroType(const MacroHandle handle) const
  {
    return (PFN_MACUINT(functions[macroGetType])(handle));
  }

  QString MacroLibraryDLL::getMacroName(const MacroHandle handle) const
  {
    return QString::fromWCharArray(PFN_MACSTRING(functions[macroGetName])(handle));
  }

  QString MacroLibraryDLL::getMacroCreator(const MacroHandle handle) const
  {
    return QString::fromWCharArray(PFN_MACSTRING(functions[macroGetCreator])(handle));
  }

  QString MacroLibraryDLL::getMacroGroup(const MacroHandle handle) const
  {
    return QString::fromWCharArray(PFN_MACSTRING(functions[macroGetGroup])(handle));
  }

  QString MacroLibraryDLL::getMacroDescription(const MacroHandle handle) const
  {
    return QString::fromWCharArray(PFN_MACSTRING(functions[macroGetDescription])(handle));
  }

  QString MacroLibraryDLL::getMacroErrorMsg(const MacroHandle handle) const
  {
    return QString::fromWCharArray(PFN_MACSTRING(functions[macroGetErrorMsg])(handle));
  }

  QString MacroLibraryDLL::getMacroPropertyWidgetComponent(const MacroHandle handle) const
  {
    return QString::fromWCharArray(PFN_MACSTRING(functions[macroGetPropertyWidgetComponent])(handle));
  }

  void MacroLibraryDLL::setMacroImpresarioDataPtr(const MacroHandle handle, void* dataPtr) const
  {
    return PFN_MACSETPTR(functions[macroSetImpresarioDataPtr])(handle,dataPtr);
  }

  void* MacroLibraryDLL::getMacroImpresarioDataPtr(const MacroHandle handle) const
  {
    return PFN_MACVOIDPTR(functions[macroGetImpresarioDataPtr])(handle);
  }

  MacroLibraryDLL::DataDescriptor* MacroLibraryDLL::getMacroInputs(const MacroHandle handle, unsigned int& count) const
  {
    return PFN_MACDATA(functions[macroGetInputs])(handle,&count);
  }

  MacroLibraryDLL::DataDescriptor* MacroLibraryDLL::getMacroOutputs(const MacroHandle handle, unsigned int& count) const
  {
    return PFN_MACDATA(functions[macroGetOutputs])(handle,&count);
  }

  MacroLibraryDLL::DataDescriptor* MacroLibraryDLL::getMacroParameters(const MacroHandle handle, unsigned int& count) const
  {
    return PFN_MACDATA(functions[macroGetParameters])(handle,&count);
  }

  MacroLibraryDLL::MacroHandle MacroLibraryDLL::cloneMacro(const MacroHandle handle) const
  {
    return PFN_MACCLONE(functions[macroClone])(handle);
  }

  bool MacroLibraryDLL::deleteMacro(const MacroHandle handle) const
  {
    return PFN_MACBOOL(functions[macroDelete])(handle);
  }

  int MacroLibraryDLL::startMacro(const MacroHandle handle) const
  {
    return PFN_MACINT(functions[macroStart])(handle);
  }

  int MacroLibraryDLL::applyMacro(const MacroHandle handle) const
  {
    return PFN_MACINT(functions[macroApply])(handle);
  }

  int MacroLibraryDLL::stopMacro(const MacroHandle handle) const
  {
    return PFN_MACINT(functions[macroStop])(handle);
  }

  void MacroLibraryDLL::setMacroParameter(const MacroHandle handle, unsigned int paramIndex, const QString& value) const
  {
    return PFN_MACSETPARAM(functions[macroSetParameterValue])(handle,paramIndex,value.toStdWString().c_str());
  }

  QString MacroLibraryDLL::getMacroParameter(const MacroHandle handle, unsigned int paramIndex) const
  {
    return QString::fromWCharArray(PFN_MACGETPARAM(functions[macroGetParameterValue])(handle,paramIndex));
  }

  void* MacroLibraryDLL::createMacroWidget(const MacroHandle handle) const
  {
    return PFN_MACVOIDPTR(functions[macroCreateWidget])(handle);
  }

  void MacroLibraryDLL::destroyMacroWidget(const MacroHandle handle) const
  {
    return PFN_MACVOID(functions[macroDestroyWidget])(handle);
  }
}
