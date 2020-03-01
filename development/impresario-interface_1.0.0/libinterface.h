/****************************************************************************************************
**   Impresario Interface - Image Processing Engineering System applying Reusable Interactive Objects
**   This file is part of the Impresario Interface.
**
**   Copyright (C) 2015, 2020  Lars Libuda
**   All rights reserved.
**
**   Redistribution and use in source and binary forms, with or without
**   modification, are permitted provided that the following conditions are met:
**       * Redistributions of source code must retain the above copyright
**         notice, this list of conditions and the following disclaimer.
**       * Redistributions in binary form must reproduce the above copyright
**         notice, this list of conditions and the following disclaimer in the
**         documentation and/or other materials provided with the distribution.
**       * Neither the name of the copyright holder nor the
**         names of its contributors may be used to endorse or promote products
**         derived from this software without specific prior written permission.
**
**   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
**   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
**   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
**   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
**   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
**   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
*****************************************************************************************************/
#ifndef LIBINTERFACE_H_
#define LIBINTERFACE_H_

#define INTERFACE_API_MAJOR 1
#define INTERFACE_API_MINOR 0
#define INTERFACE_API_PATCH 0

#ifdef _IMPRESARIO_WIN
  // definition for Windows platform
  #define MACRO_API __declspec(dllexport)
#else
  // empty definition for Linux platform
  #define MACRO_API
#endif

// definition of Impresario C interface
#ifdef __cplusplus
extern "C" {
#endif

  typedef void* MacroHandle;
  typedef void (*PFN_CONSOLE_REDIRECT) (char*);
  typedef void (*PFN_MACROPARAM_CHANGED) (MacroHandle, unsigned int, void*);

  struct DataDescriptor {
    const wchar_t*  name;
    const wchar_t*  description;
    const char*     type;
    void*           valuePtr;
    DataDescriptor* next;
  };

  enum MacroType {
    Macro = 0,
    ExtendedMacro,
    Viewer
  };

  MACRO_API const wchar_t*  libGetBuildDate();
  MACRO_API const wchar_t*  libGetCompiler();
  MACRO_API unsigned int    libGetCompilerId();
  MACRO_API unsigned int    libGetQtVersion();
  MACRO_API bool            libIsDebugVersion();
  MACRO_API const wchar_t*  libGetName();
  MACRO_API unsigned int    libGetVersion();
  MACRO_API unsigned int    libGetAPIVersion();
  MACRO_API const wchar_t*  libGetCreator();
  MACRO_API const wchar_t*  libGetDescription();
  MACRO_API bool            libInitialize(MacroHandle** list, unsigned int* count, PFN_CONSOLE_REDIRECT cbStdCout, PFN_CONSOLE_REDIRECT cbStdCerr, PFN_MACROPARAM_CHANGED cbMacroParamChanged);
  MACRO_API void            libTerminate();

  MACRO_API MacroHandle     macroClone(MacroHandle handle);
  MACRO_API void            macroSetImpresarioDataPtr(MacroHandle handle, void* dataPtr);
  MACRO_API void*           macroGetImpresarioDataPtr(MacroHandle handle);
  MACRO_API bool            macroDelete(MacroHandle handle);
  MACRO_API unsigned int    macroGetType(MacroHandle handle);
  MACRO_API const wchar_t*  macroGetName(MacroHandle handle);
  MACRO_API const wchar_t*  macroGetCreator(MacroHandle handle);
  MACRO_API const wchar_t*  macroGetGroup(MacroHandle handle);
  MACRO_API const wchar_t*  macroGetDescription(MacroHandle handle);
  MACRO_API const wchar_t*  macroGetErrorMsg(MacroHandle handle);
  MACRO_API const wchar_t*  macroGetPropertyWidgetComponent(MacroHandle handle);
  MACRO_API DataDescriptor* macroGetInputs(MacroHandle handle, unsigned int* count);
  MACRO_API DataDescriptor* macroGetOutputs(MacroHandle handle, unsigned int* count);
  MACRO_API DataDescriptor* macroGetParameters(MacroHandle handle, unsigned int* count);
  MACRO_API int             macroStart(MacroHandle handle);
  MACRO_API int             macroApply(MacroHandle handle);
  MACRO_API int             macroStop(MacroHandle handle);
  MACRO_API void            macroSetParameterValue(MacroHandle handle, unsigned int parameter, const wchar_t* strValue);
  MACRO_API const wchar_t*  macroGetParameterValue(MacroHandle handle, unsigned int parameter);
  MACRO_API void*           macroCreateWidget(MacroHandle handle);
  MACRO_API void            macroDestroyWidget(MacroHandle handle);

#ifdef __cplusplus
} /* extern C */
#endif
// end of definition of Impresario interface

// definitions for internal use
#define MACRO_REGISTRATION_BEGIN
#define MACRO_ADD(class_name)
#define MACRO_REGISTRATION_END
void notifyParameterChanged(MacroHandle handle, unsigned int parameter);

#endif /* LIBINTERFACE_H_ */
