/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2018  Lars Libuda
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
#ifndef APPMACROLIBRARY_H
#define APPMACROLIBRARY_H

#include <QString>
#include <QMap>
#include <QLibrary>

namespace app
{
  // forward declaration to prevent circular reference of header files
  class MacroDLL;
  class MacroManager;

  class MacroLibrary
  {
    friend class MacroManager;
  public:

    const QString& getPath() const
    {
      return libPath;
    }

    const QString& getName() const
    {
      return libName;
    }

    const QString& getCreator() const
    {
      return libCreator;
    }

    const QString& getDescription() const
    {
      return libDescription;
    }

    const QString& getBuildDate() const
    {
      return libBuildDate;
    }

    const QString& getVersionString() const
    {
      return libVersionString;
    }

    unsigned int getVersion() const
    {
      return libVersion;
    }

    const QString& getAPIVersionString() const
    {
      return libAPIVersionString;
    }

    unsigned int getAPIVersion() const
    {
      return libAPIVersion;
    }

    const QString& getBuildType() const
    {
      return libBuildType;
    }

    unsigned int getCompilerVersion() const
    {
      return libCompilerVersion;
    }

    const QString& getQtVersionString() const
    {
      return libQtVersionString;
    }

    unsigned int getQtVersion() const
    {
      return libQtVersion;
    }

    unsigned int countMacros() const
    {
      return libCntMacros;
    }

    unsigned int countViewers() const
    {
      return libCntViewers;
    }

    bool isDebugVersion() const
    {
      return libIsDebug;
    }

  protected:
    MacroLibrary();
    MacroLibrary(const MacroLibrary& other);
    virtual ~MacroLibrary();

    virtual bool load(const QString& path, MacroManager& manager) = 0;

    QString libPath;
    QString libName;
    QString libCreator;
    QString libDescription;
    QString libBuildDate;
    QString libCompiler;
    QString libVersionString;
    QString libBuildType;
    QString libQtVersionString;
    QString libAPIVersionString;
    unsigned int libVersion;
    unsigned int libCompilerVersion;
    unsigned int libQtVersion;
    unsigned int libAPIVersion;
    unsigned int libCntMacros;
    unsigned int libCntViewers;
    bool         libIsDebug;
  };

  class MacroLibraryDLL : public MacroLibrary
  {
    friend class MacroManager;
    friend class MacroDLL;
    friend class MacroViewer;

  protected:
    MacroLibraryDLL();
    MacroLibraryDLL(const MacroLibraryDLL& other);
    virtual ~MacroLibraryDLL();

    virtual bool load(const QString& path, MacroManager& manager);
    bool isMacroCompatible(const MacroDLL& macro);

  private:
    /**
     * Type definition for a macro handle as it is used by the
     * macro library interface
     */
    typedef void* MacroHandle;

    /**
     * Type definition for a macro data descriptor as it is used by the
     * macro library interface to handle macro inputs, outputs, and parameters
     */
    struct DataDescriptor
    {
      const wchar_t*  name;
      const wchar_t*  description;
      const char*     type;
      void*           valuePtr;
      DataDescriptor* next;
    };

    /**
     * Callback function called by DLL in case a parameter of a macro changed.
     */
    static void cbMacroParameterChanged(MacroHandle handle, unsigned int paramIndex, void* dataPtr);

    unsigned int getMacroType(const MacroHandle handle) const;
    QString getMacroName(const MacroHandle handle) const;
    QString getMacroCreator(const MacroHandle handle) const;
    QString getMacroGroup(const MacroHandle handle) const;
    QString getMacroDescription(const MacroHandle handle) const;
    QString getMacroErrorMsg(const MacroHandle handle) const;
    QString getMacroPropertyWidgetComponent(const MacroHandle handle) const;
    void setMacroImpresarioDataPtr(const MacroHandle handle, void* dataPtr) const;
    void* getMacroImpresarioDataPtr(const MacroHandle handle) const;
    DataDescriptor* getMacroInputs(const MacroHandle handle, unsigned int& count) const;
    DataDescriptor* getMacroOutputs(const MacroHandle handle, unsigned int& count) const;
    DataDescriptor* getMacroParameters(const MacroHandle handle, unsigned int& count) const;
    MacroHandle cloneMacro(const MacroHandle handle) const;
    bool deleteMacro(const MacroHandle handle) const;
    int startMacro(const MacroHandle handle) const;
    int applyMacro(const MacroHandle handle) const;
    int stopMacro(const MacroHandle handle) const;
    void setMacroParameter(const MacroHandle handle, unsigned int paramIndex, const QString& value) const;
    QString getMacroParameter(const MacroHandle handle, unsigned int paramIndex) const;
    void* createMacroWidget(const MacroHandle handle) const;
    void destroyMacroWidget(const MacroHandle handle) const;

    // Function type definitions for Impresario interface
    typedef const wchar_t*  (* PFN_LIBSTRING)  ();
    typedef unsigned int    (* PFN_LIBUINT)    ();
    typedef bool            (* PFN_LIBBOOL)    ();
    typedef bool            (* PFN_LIBINIT)    (MacroHandle**,unsigned int*,void (*cbCout)(char*),void (*cbCerr)(char*),void (*cbMacroParamChanged)(MacroHandle,unsigned int,void*));
    typedef void            (* PFN_LIBTERM)    ();
    typedef const wchar_t*  (* PFN_MACSTRING)  (MacroHandle);
    typedef unsigned int    (* PFN_MACUINT)    (MacroHandle);
    typedef bool            (* PFN_MACBOOL)    (MacroHandle);
    typedef int             (* PFN_MACINT)     (MacroHandle);
    typedef MacroHandle     (* PFN_MACCLONE)   (MacroHandle);
    typedef DataDescriptor* (* PFN_MACDATA)    (MacroHandle,unsigned int*);
    typedef void            (* PFN_MACSETPARAM)(MacroHandle,unsigned int,const wchar_t*);
    typedef const wchar_t*  (* PFN_MACGETPARAM)(MacroHandle,unsigned int);
    typedef void*           (* PFN_MACVOIDPTR) (MacroHandle);
    typedef void            (* PFN_MACVOID)    (MacroHandle);
    typedef void            (* PFN_MACSETPTR)  (MacroHandle,void*);

    /**
     * Enumeration of all functions imported from loaded DLL which deals with
     * library organization. This enumeration is private because these functions
     * are just accessed by this class.
     */
    enum LibFunctions {
      libGetBuildDate,
      libGetCompiler,
      libGetCompilerId,
      libGetQtVersion,
      libIsDebugVersion,
      libGetName,
      libGetVersion,
      libGetAPIVersion,
      libGetCreator,
      libGetDescription,
      libInitialize,
      libTerminate,
      macroClone,
      macroDelete,
      macroGetType,
      macroGetName,
      macroGetCreator,
      macroGetGroup,
      macroGetDescription,
      macroGetErrorMsg,
      macroGetPropertyWidgetComponent,
      macroGetInputs,
      macroGetOutputs,
      macroGetParameters,
      macroStart,
      macroApply,
      macroStop,
      macroSetParameterValue,
      macroGetParameterValue,
      macroSetImpresarioDataPtr,
      macroGetImpresarioDataPtr,
      macroCreateWidget,
      macroDestroyWidget
    };

    /**
     * Map type for holding all pointers to functions located in the loaded DLL.
     */
    typedef QMap<LibFunctions,void*> FunctionMap;

    /**
     * Array of function names to be imported from DLL
     */
    static const QString InterfaceFunctionNames[];

    /**
     * Map containing all function pointers into macro library.
     */
    FunctionMap functions;

    /**
     * Handle to the dynamic link library.
     */
    QLibrary libHandler;
  };

}
#endif // DBMACROLIBRARY_H
