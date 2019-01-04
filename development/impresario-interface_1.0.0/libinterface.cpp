/****************************************************************************************************
**   Impresario Interface - Image Processing Engineering System applying Reusable Interactive Objects
**   This file is part of the Impresario Interface.
**
**   Copyright (C) 2015  Lars Libuda
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
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
  #define _CRT_SECURE_NO_DEPRECATE
#endif

#include "libinterface.h"
#include "macrobase.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <climits>

wchar_t                     g_szCompiler[256];
wchar_t                     g_szDate[16];
std::vector<MacroHandle>    g_Macros;
std::editstreambuf*         g_coutStream = 0;
std::editstreambuf*         g_cerrStream = 0;
std::basic_streambuf<char>* g_pCoutOld = 0;
std::basic_streambuf<char>* g_pCerrOld = 0;
PFN_MACROPARAM_CHANGED      g_cbMacroParamChanged;

void initConsoleRedirect(PFN_CONSOLE_REDIRECT cbStdCout, PFN_CONSOLE_REDIRECT cbStdCerr) {
  if (cbStdCout != 0) {
    g_coutStream = new std::editstreambuf(cbStdCout);
    g_pCoutOld = std::cout.rdbuf(g_coutStream);
  }
  if (cbStdCerr != 0) {
    g_cerrStream = new std::editstreambuf(cbStdCerr);
    g_pCerrOld = std::cerr.rdbuf(g_cerrStream);
  }
}

// It is important to have the following include exactly at this place!
#include "libconfig.h"
// NOTE: The function
// bool libInitialize(MacroHandle** list, unsigned int* count)
// is defined via preprocessor macro expansion in libconfig.h

const wchar_t* libGetBuildDate() {
  mbstowcs(g_szDate,__DATE__,16);
  return g_szDate;
}

const wchar_t* libGetCompiler() {
  wchar_t szArch[6];
  if (sizeof (void *) * CHAR_BIT == 64) {
    wcsncpy(szArch,L"64bit\0",6);
  }
  else {
    wcsncpy(szArch,L"32bit\0",6);
  }
#if defined(__GNUC__)
  // GCC
  swprintf(g_szCompiler,256,L"GCC %i.%i.%i (%ls)",__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__,szArch);
  return g_szCompiler;
#elif defined(_MSC_VER)
  // Microsoft
  _snwprintf(g_szCompiler,sizeof(g_szCompiler),L"Microsoft C/C++ Compiler %i.%i %ls",_MSC_VER / 100,_MSC_VER % 100,szArch);
#if _MSC_VER == 1300 
  wcsncat(g_szCompiler,L" (Visual C++ 2002)",sizeof(g_szCompiler));
#elif _MSC_VER == 1310 
  wcsncat(g_szCompiler,L" (Visual C++ 2003)",sizeof(g_szCompiler));
#elif _MSC_VER == 1400 
  wcsncat(g_szCompiler,L" (Visual C++ 2005)",sizeof(g_szCompiler));
#elif _MSC_VER == 1500
  wcsncat(g_szCompiler,L" (Visual C++ 2008)",sizeof(g_szCompiler));
#elif _MSC_VER == 1600
  wcsncat(g_szCompiler,L" (Visual C++ 2010)",sizeof(g_szCompiler));
#elif _MSC_VER == 1700
  wcsncat(g_szCompiler,L" (Visual C++ 2012)",sizeof(g_szCompiler));
#elif _MSC_VER == 1800
  wcsncat(g_szCompiler,L" (Visual C++ 2013)",sizeof(g_szCompiler));
#elif _MSC_VER == 1900
  wcsncat(g_szCompiler,L" (Visual C++ 2015)",sizeof(g_szCompiler));
#elif _MSC_VER == 1914
  wcsncat(g_szCompiler,L" (Visual C++ 2017)",sizeof(g_szCompiler));
#endif
  return g_szCompiler;
#else
  // no supported compiler
  wcsncpy(g_szCompiler,L"Not recognized",sizeof(g_szCompiler));
  return g_szCompiler;
#endif
}

unsigned int libGetCompilerId() {
  bool is64bit = (sizeof (void *) * CHAR_BIT == 64) ? true : false;
#if defined(__GNUC__)
  // GCC
  return ((is64bit) ? 641 : 321) * 1000000 + __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__;
#elif defined(_MSC_VER)
  return ((is64bit) ? 642 : 322) * 1000000 + (_MSC_VER / 100) * 10000 + (_MSC_VER % 100) * 100;
#else
  // no supported compiler
  return 0;
#endif
}

unsigned int libGetQtVersion() {
#if defined(QT_VERSION)
  return (unsigned int)QT_VERSION;
#else
  return 0;
#endif
}

bool libIsDebugVersion() {
#if defined(_IMPRESARIO_DEBUG)
  return true;
#else
  return false;
#endif
}

unsigned int libGetVersion() {
  return (LIB_VERSION_MAJOR << 16) + (LIB_VERSION_MINOR << 8) + LIB_VERSION_PATCH;
}

unsigned int libGetAPIVersion() {
  return (INTERFACE_API_MAJOR << 16) + (INTERFACE_API_MINOR << 8) + INTERFACE_API_PATCH;
}

const wchar_t* libGetCreator() {
  return LIB_CREATOR;
}

const wchar_t* libGetName() {
  return LIB_NAME;
}

const wchar_t* libGetDescription() {
  return LIB_DESCRIPTION;
}

void libTerminate() {
  // reset standard stream redirection if necessary
  if (g_coutStream != 0 && g_pCoutOld != 0) {
    std::cout.rdbuf(g_pCoutOld);
    delete g_coutStream;
    g_coutStream = 0;
    g_pCoutOld = 0;
  }
  if (g_cerrStream != 0 && g_pCerrOld != 0) {
    std::cerr.rdbuf(g_pCerrOld);
    delete g_cerrStream;
    g_cerrStream = 0;
    g_pCerrOld = 0;
  }
  // delete macro list
  for(std::size_t i = 0; i < g_Macros.size(); ++i) {
    MacroBase* macro = (MacroBase*)g_Macros[i];
    delete macro;
  }
  g_Macros.clear();
}

MacroHandle macroClone(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  MacroBase* clone = macro->clone();
  return (MacroHandle)clone;
}

void macroSetImpresarioDataPtr(MacroHandle handle, void *dataPtr) {
  MacroBase* macro = (MacroBase*)handle;
  macro->setImpresarioDataPtr(dataPtr);
}

void* macroGetImpresarioDataPtr(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  return macro->getImpresarioDataPtr();
}

bool macroDelete(MacroHandle handle) {
  for(std::size_t i = 0; i < g_Macros.size(); ++i) {
    if (g_Macros[i] == handle) {
      return false;
    }
  }
  MacroBase* macro = (MacroBase*)handle;
  delete macro;
  return true;
}

unsigned int macroGetType(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  return (unsigned int)macro->getType();
}

const wchar_t* macroGetName(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  const std::wstring& str = macro->getName();
  return str.c_str();
}

const wchar_t* macroGetCreator(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  const std::wstring& str = macro->getCreator();
  return str.c_str();
}

const wchar_t* macroGetGroup(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  const std::wstring& str = macro->getGroup();
  return str.c_str();
}

const wchar_t* macroGetDescription(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  const std::wstring& str = macro->getDescription();
  return str.c_str();
}

const wchar_t* macroGetErrorMsg(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  const std::wstring& str = macro->getErrorMsg();
  return str.c_str();
}

const wchar_t* macroGetPropertyWidgetComponent(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  const std::wstring& str = macro->getPropertyWidgetComponent();
  return str.c_str();
}

DataDescriptor* macroGetInputs(MacroHandle handle, unsigned int* count) {
  MacroBase* macro = (MacroBase*)handle;
  const MacroBase::ValueVector& inputs = macro->getInputs();
  *count = static_cast<unsigned int>(inputs.size());
  if (inputs.size() > 0) {
    return inputs[0]->getDescriptorPtr();
  }
  else {
    return 0;
  }
}

DataDescriptor* macroGetOutputs(MacroHandle handle, unsigned int* count) {
  MacroBase* macro = (MacroBase*)handle;
  const MacroBase::ValueVector& outputs = macro->getOutputs();
  *count = static_cast<unsigned int>(outputs.size());
  if (outputs.size() > 0) {
    return outputs[0]->getDescriptorPtr();
  }
  else {
    return 0;
  }
}

DataDescriptor* macroGetParameters(MacroHandle handle, unsigned int* count) {
  MacroBase* macro = (MacroBase*)handle;
  const MacroBase::ValueVector& params = macro->getParameters();
  *count = static_cast<unsigned int>(params.size());
  if (params.size() > 0) {
    return params[0]->getDescriptorPtr();
  }
  else {
    return 0;
  }
}

int macroStart(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  return (int)(macro->init());
}

int macroApply(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  return (int)(macro->apply());
}

int macroStop(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  return (int)(macro->exit());
}

void macroSetParameterValue(MacroHandle handle, unsigned int parameter, const wchar_t* strValue) {
  MacroBase* macro = (MacroBase*)handle;
  const MacroBase::ValueVector& params = macro->getParameters();
  if (parameter < params.size()) {
    std::wstring value(strValue);
    macro->setParameterValueAsString(parameter,value);
  }
}

const wchar_t* macroGetParameterValue(MacroHandle handle, unsigned int parameter) {
  MacroBase* macro = (MacroBase*)handle;
  const MacroBase::ValueVector& params = macro->getParameters();
  if (parameter < params.size()) {
    return macro->getParameterValueAsString(parameter).c_str();
  }
  else
  {
    return 0;
  }
}

#if defined(QT_VERSION)
#include "macroextended.h"

void* macroCreateWidget(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  if (macro->getType() != Macro) {
    MacroExtBase* macroExt = static_cast<MacroExtBase*>(macro);
    return reinterpret_cast<void*>(macroExt->createWidget());
  }
  return 0;
}

void macroDestroyWidget(MacroHandle handle) {
  MacroBase* macro = (MacroBase*)handle;
  if (macro->getType() != Macro) {
    MacroExtBase* macroExt = static_cast<MacroExtBase*>(macro);
    macroExt->destroyWidget();
  }
}

#else
void* macroCreateWidget(MacroHandle /*handle*/) {
  return 0;
}

void macroDestroyWidget(MacroHandle /*handle*/) {
}

#endif

void notifyParameterChanged(MacroHandle handle, unsigned int parameter, void* dataPtr) {
  if (g_cbMacroParamChanged) {
    g_cbMacroParamChanged(handle,parameter,dataPtr);
  }
}

