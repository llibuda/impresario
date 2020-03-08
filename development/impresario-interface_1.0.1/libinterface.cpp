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

#include "libinterface.h"
#include "macrobase.h"
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <map>
#include <iostream>
#include <climits>
#include <streambuf>
#include <string>
#include <algorithm>
#include <cassert>

//----------------------------------------------------------------------------------------------
//--------------------- class for redirection of std::cout and std::cerr -----------------------
//----------------------------------------------------------------------------------------------
namespace std
{
  template <class E, class T = std::char_traits<E>, int BUF_SIZE = 512 >
  class basic_editstreambuf : public std::basic_streambuf< E, T >
  {
  public:
      basic_editstreambuf(PFN_CONSOLE_REDIRECT callback) : std::basic_streambuf<E,T>{}, cbStream{callback}
    {
      psz = new typename T::char_type[ BUF_SIZE ];
      this->pubsetbuf( psz, BUF_SIZE );
      // leave place for single char + 0 terminator
      this->setp( psz, psz + BUF_SIZE - 2 );
    }

      basic_editstreambuf() : std::basic_streambuf<E,T>{}
    {
      psz = new typename T::char_type[ BUF_SIZE ];
      this->pubsetbuf( psz, BUF_SIZE );
      // leave place for single char + 0 terminator
      this->setp( psz, psz + BUF_SIZE - 2 );
    }

    ~basic_editstreambuf() override
    {
      delete psz;
      psz = nullptr;
    }

  protected:
    typename T::int_type overflow(typename T::int_type c = T::eof()) override
    {
      // maybe mutex lock required here?
      typename T::char_type* plast = std::basic_streambuf<E,T>::pptr();
      if (c != T::eof())
      {
        // add c to buffer
        *plast++ = static_cast<typename T::char_type>(c);
      }
      *plast = typename T::char_type();

      // Pass text to the edit control
      if (cbStream != nullptr)
      {
        cbStream(std::basic_streambuf<E,T>::pbase());
      }
      this->setp(std::basic_streambuf<E,T>::pbase(),std::basic_streambuf<E,T>::epptr());

      // mutex lock to be released here
      return c != T::eof() ? T::not_eof( c ) : T::eof();
    }

    int sync() override
    {
      overflow();
      return 0;
    }

    std::streamsize xsputn(const typename T::char_type* pch, std::streamsize n) override
    {
      std::streamsize nMax, nPut;
      // maybe mutex lock required here?
      for(nPut = 0; 0 < n;)
      {
        if (std::basic_streambuf<E,T>::pptr() != nullptr && 0 < (nMax = static_cast<std::streamsize>(std::basic_streambuf<E,T>::epptr() - std::basic_streambuf<E,T>::pptr())))
        {
          if(n < nMax)
          {
            nMax = n;
          }
          T::copy(std::basic_streambuf<E,T>::pptr(), pch, static_cast<size_t>(nMax));

          // Sync if string contains LF
          bool bSync = T::find( pch, size_t(nMax), T::to_char_type( '\n' ) ) != nullptr;
          pch += nMax, nPut += nMax, n -= nMax, std::basic_streambuf<E,T>::pbump(static_cast<int>(nMax));
          if (bSync)
          {
            sync();
          }
        }
        else if (T::eq_int_type(T::eof(),overflow(T::to_int_type(*pch))))
        {
          break;
        }
        else
        {
          ++pch, ++nPut, --n;
        }
      }
      // mutex lock to be released here
      return nPut;
    }

  private:
    typename T::char_type* psz;
    PFN_CONSOLE_REDIRECT   cbStream;
  };

  using editstreambuf = basic_editstreambuf<char>;
}

//----------------------------------------------------------------------------------------------
//------------- global variables and functions for this module in anonymous namespace ----------
//----------------------------------------------------------------------------------------------

namespace {

  wchar_t                     g_szCompiler[256];
  wchar_t                     g_szBuildDate[16];
  std::vector<MacroHandle>    g_Macros;
  std::editstreambuf*         g_coutStream = nullptr;
  std::editstreambuf*         g_cerrStream = nullptr;
  std::basic_streambuf<char>* g_pCoutOld = nullptr;
  std::basic_streambuf<char>* g_pCerrOld = nullptr;
  std::map<MacroHandle,void*> g_MacroBackReference;
  PFN_MACROPARAM_CHANGED      g_cbMacroParamChanged;

  void initCompiler()
  {
    wchar_t szArch[6];
    if (sizeof (void *) * CHAR_BIT == 64) {
    #ifdef _MSC_VER
      wcsncpy_s(szArch, 6, L"64bit\0", 6);
    #else
      wcsncpy(szArch,L"64bit\0",6);
    #endif
    }
    else {
    #ifdef _MSC_VER
      wcsncpy_s(szArch, 6, L"64bit\0", 6);
    #else
      wcsncpy(szArch,L"64bit\0",6);
    #endif
    }
  #if defined(__GNUC__)
    // GCC
    #ifdef __STDC_LIB_EXT1__
      swprintf_s(g_szCompiler,sizeof(g_szCompiler) / sizeof(wchar_t),L"GCC %i.%i.%i (%ls)",__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__,szArch);
    #else
      swprintf(g_szCompiler,256,L"GCC %i.%i.%i (%ls)",__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__,szArch);
    #endif
  #elif defined(_MSC_VER)
    // Microsoft
    swprintf_s(g_szCompiler,sizeof(g_szCompiler) / sizeof(wchar_t),L"Microsoft C/C++ Compiler %i.%i %ls",_MSC_VER / 100,_MSC_VER % 100,szArch);
  #if _MSC_VER == 1400
    wcsncat_s(g_szCompiler,sizeof(g_szCompiler) / sizeof(wchar_t),L" (Visual Studio 2005)",21);
  #elif _MSC_VER == 1500
    wcsncat_s(g_szCompiler,sizeof(g_szCompiler) / sizeof(wchar_t),L" (Visual Studio 2008)",21);
  #elif _MSC_VER == 1600
    wcsncat_s(g_szCompiler,sizeof(g_szCompiler) / sizeof(wchar_t),L" (Visual Studio 2010)",21);
  #elif _MSC_VER == 1700
    wcsncat_s(g_szCompiler,sizeof(g_szCompiler) / sizeof(wchar_t),L" (Visual Studio 2012)",21);
  #elif _MSC_VER == 1800
    wcsncat_s(g_szCompiler,sizeof(g_szCompiler) / sizeof(wchar_t),L" (Visual Studio 2013)",21);
  #elif _MSC_VER == 1900
    wcsncat_s(g_szCompiler,sizeof(g_szCompiler) / sizeof(wchar_t),L" (Visual Studio 2015)",21);
  #elif _MSC_VER >= 1910 && _MSC_VER < 1920
    wcsncat_s(g_szCompiler,sizeof(g_szCompiler) / sizeof(wchar_t),L" (Visual Studio 2017)",21);
  #elif _MSC_VER >= 1920
    wcsncat_s(g_szCompiler,sizeof(g_szCompiler) / sizeof(wchar_t),L" (Visual Studio 2019)",21);
  #endif
  #else
    // no supported compiler
    wcsncpy_s(g_szCompiler,sizeof(g_szCompiler) / sizeof(wchar_t),L"Not recognized",14);
  #endif
  }

  void initBuildDate()
  {
  #ifdef _MSC_VER
    const char* buildDate{ __DATE__ };
    size_t      inSize { sizeof(buildDate) };
    size_t      outSize;
    mbstate_t   conversionState;
    memset(&conversionState,0,sizeof(conversionState));
    mbsrtowcs_s(&outSize, g_szBuildDate, 16, &buildDate, inSize, &conversionState);
  #else
    mbstowcs(g_szBuildDate,__DATE__,16);
  #endif
  }

  void initConsoleRedirect(PFN_CONSOLE_REDIRECT cbStdCout, PFN_CONSOLE_REDIRECT cbStdCerr) {
    if (cbStdCout != nullptr) {
      try {
        g_coutStream = new std::editstreambuf(cbStdCout);
        g_pCoutOld = std::cout.rdbuf(g_coutStream);
      }
      catch(...) {
        g_coutStream = nullptr;
      }
    }
    if (cbStdCerr != nullptr) {
      try {
        g_cerrStream = new std::editstreambuf(cbStdCerr);
        g_pCerrOld = std::cerr.rdbuf(g_cerrStream);
      }
      catch(...) {
        g_cerrStream = nullptr;
      }
    }
  }

  void undoConsoleRedirect() {
    // reset standard stream redirection if necessary
    if (g_coutStream != nullptr && g_pCoutOld != nullptr) {
      std::cout.rdbuf(g_pCoutOld);
      delete g_coutStream;
      g_coutStream = nullptr;
      g_pCoutOld = nullptr;
    }
    if (g_cerrStream != nullptr && g_pCerrOld != nullptr) {
      std::cerr.rdbuf(g_pCerrOld);
      delete g_cerrStream;
      g_cerrStream = nullptr;
      g_pCerrOld = nullptr;
    }
  }
}

//----------------------------------------------------------------------------------------------
//------------- API Wrapper class for accessing individial MacroBase instances -----------------
//----------------------------------------------------------------------------------------------
class MacroAPIWrapper
{
public:
  MacroAPIWrapper(const MacroAPIWrapper&) = delete;
  MacroAPIWrapper& operator=(const MacroAPIWrapper&) = delete;
  MacroAPIWrapper(MacroAPIWrapper&&) = delete;
  MacroAPIWrapper& operator=(MacroAPIWrapper&&) = delete;

  MacroAPIWrapper(MacroBase* macro) : m_macroPtr{macro} {
    assert(macro != nullptr);
  }

  virtual ~MacroAPIWrapper() = default;

  virtual MacroAPIWrapper* clone() = 0;

  MacroBase*     data() const                       { return m_macroPtr.get(); }
  const wchar_t* getName() const                    { return m_macroPtr->getName().c_str(); }
  const wchar_t* getGroup() const                   { return m_macroPtr->getGroup().c_str(); }
  const wchar_t* getCreator() const                 { return m_macroPtr->getCreator().c_str(); }
  const wchar_t* getDescription() const             { return m_macroPtr->getDescription().c_str(); }
  const wchar_t* getErrorMsg() const                { return m_macroPtr->getErrorMsg().c_str(); }
  const wchar_t* getPropertyWidgetComponent() const { return m_macroPtr->getPropertyWidgetComponent().c_str(); }
  MacroType      getType() const                    { return m_macroPtr->getType(); }

  // C-Interface for API to access inputs, outputs, and parameters
  DataDescriptor* getInputsCInterface(unsigned int* count) const {
    auto& inputs = m_macroPtr->getInputs();
    *count = static_cast<unsigned int>(inputs.size());
    return (inputs.empty()) ? nullptr : inputs[0]->getDescriptorPtr();
  }

  DataDescriptor* getOutputsCInterface(unsigned int* count) const {
    auto& outputs = m_macroPtr->getOutputs();
    *count = static_cast<unsigned int>(outputs.size());
    return (outputs.empty()) ? nullptr : outputs[0]->getDescriptorPtr();
  }

  DataDescriptor* getParametersCInterface(unsigned int* count) const {
    auto& params = m_macroPtr->getParameters();
    *count = static_cast<unsigned int>(params.size());
    return (params.empty()) ? nullptr : params[0]->getDescriptorPtr();
  }

  const wchar_t* getParameterValueAsString(unsigned int parameterIndex) const {
    auto& params = m_macroPtr->getParameters();
    if (parameterIndex < params.size()) {
      return dynamic_cast<ValueParameter*>(params[parameterIndex].get())->getValueAsString().c_str();
    }
    else
    {
      return nullptr;
    }
  }

  void setParameterValueAsString(unsigned int parameterIndex, const wchar_t* cstrValue) {
    auto& params = m_macroPtr->getParameters();
    if (parameterIndex < params.size()) {
      std::wstring value(cstrValue);
      dynamic_cast<ValueParameter*>(params[parameterIndex].get())->setValueAsString(value);
      m_setChangedParams.insert(parameterIndex);
    }
  }

  MacroBase::Status init()  {
    m_macroPtr->setErrorMsg();
    for(std::size_t index = 0; index < m_macroPtr->m_vecParams.size(); ++index) {
      m_setChangedParams.insert(static_cast<unsigned int>(index));
    }
    m_macroPtr->onParametersChanged(m_setChangedParams);
    m_setChangedParams.clear();
    return m_macroPtr->onInit();
  }

  MacroBase::Status apply() {
    if (!m_setChangedParams.empty()) {
      m_macroPtr->onParametersChanged(m_setChangedParams);
      m_setChangedParams.clear();
    }
    return m_macroPtr->onApply();
  }

  MacroBase::Status exit()  {
    if (!m_setChangedParams.empty()) {
      m_macroPtr->onParametersChanged(m_setChangedParams);
      m_setChangedParams.clear();
    }
    return m_macroPtr->onExit();
  }

protected:
  using MacroPtr = std::unique_ptr<MacroBase>;
  using ParameterSet = MacroBase::ParameterSet;

  MacroPtr     m_macroPtr;
  ParameterSet m_setChangedParams;
};

template<typename T>
class MacroAPITypeWrapper : public MacroAPIWrapper {
public:
  MacroAPITypeWrapper() : MacroAPIWrapper{ new T{} } {
  }
  ~MacroAPITypeWrapper() override = default;

  MacroAPIWrapper* clone() override { return new MacroAPITypeWrapper<T>{}; }
};

//----------------------------------------------------------------------------------------------
//--------------------- include library configuration from libconfig.h -------------------------
//----------------------------------------------------------------------------------------------
// NOTE: The function
// libInitialize(MacroHandle** list, unsigned int* count, PFN_CONSOLE_REDIRECT cbStdCout, PFN_CONSOLE_REDIRECT cbStdCerr, PFN_MACROPARAM_CHANGED cbMacroParamChanged);
// is defined via preprocessor expansion in libconfig.h with the following macros
#undef MACRO_REGISTRATION_BEGIN
#define MACRO_REGISTRATION_BEGIN bool libInitialize(MacroHandle** list, unsigned int* count, PFN_CONSOLE_REDIRECT cbStdCout, \
                                                    PFN_CONSOLE_REDIRECT cbStdCerr, PFN_MACROPARAM_CHANGED cbMacroParamChanged) { \
                                      initCompiler();                              \
                                      initBuildDate();                             \
                                      if (g_Macros.size() > 0) {                   \
                                        return false;                              \
                                      }                                            \
                                      g_cbMacroParamChanged = cbMacroParamChanged; \
                                      initConsoleRedirect(cbStdCout,cbStdCerr);    \
                                      try {

#undef MACRO_ADD
#define MACRO_ADD(class_name) g_Macros.push_back(new MacroAPITypeWrapper<class_name>{});

#undef MACRO_REGISTRATION_END
#define MACRO_REGISTRATION_END   }                                       \
                                 catch(...) {                            \
                                   libTerminate();                       \
                                   *count = 0;                           \
                                   *list = nullptr;                      \
                                   return false;                         \
                                 }                                       \
                                 *count = (unsigned int)g_Macros.size(); \
                                 *list = g_Macros.data();                \
                                 return true;                            \
                               }

// It is important to have the following include exactly at this place!
#include "libconfig.h"

//----------------------------------------------------------------------------------------------
//--------------------- implementation of interface functions ----------------------------------
//----------------------------------------------------------------------------------------------
const wchar_t* libGetBuildDate() {
  return g_szBuildDate;
}

const wchar_t* libGetCompiler() {
  return g_szCompiler;
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
  return static_cast<unsigned int>(QT_VERSION);
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
  undoConsoleRedirect();
  // delete macro list
  for(MacroHandle gMacroHandle : g_Macros) {
    auto macroWrapper = static_cast<MacroAPIWrapper*>(gMacroHandle);
    delete macroWrapper;
  }
  g_Macros.clear();
}

MacroHandle macroClone(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  try {
    auto clone = macroWrapper->clone();
    return static_cast<MacroHandle>(clone);
  }
  catch(...) {
    return nullptr;
  }
}

void macroSetImpresarioDataPtr(MacroHandle handle, void *dataPtr) {
  if (handle != nullptr) {
    auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
    assert(macroWrapper != nullptr);
    g_MacroBackReference[macroWrapper->data()] = dataPtr;
  }
}

void* macroGetImpresarioDataPtr(MacroHandle handle) {
  if (handle != nullptr) {
    auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
    assert(macroWrapper != nullptr);
    return g_MacroBackReference[macroWrapper->data()];
  }
  else {
    return nullptr;
  }
}

bool macroDelete(MacroHandle handle) {
  if (handle == nullptr || std::find(g_Macros.cbegin(),g_Macros.cend(),handle) != g_Macros.cend()) {
    return false;
  }
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  delete macroWrapper;
  return true;
}

unsigned int macroGetType(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return static_cast<unsigned int>(macroWrapper->getType());
}

const wchar_t* macroGetName(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return macroWrapper->getName();
}

const wchar_t* macroGetCreator(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return macroWrapper->getCreator();
}

const wchar_t* macroGetGroup(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return macroWrapper->getGroup();
}

const wchar_t* macroGetDescription(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return macroWrapper->getDescription();
}

const wchar_t* macroGetErrorMsg(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return macroWrapper->getErrorMsg();
}

const wchar_t* macroGetPropertyWidgetComponent(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return macroWrapper->getPropertyWidgetComponent();
}

DataDescriptor* macroGetInputs(MacroHandle handle, unsigned int* count) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return macroWrapper->getInputsCInterface(count);
}

DataDescriptor* macroGetOutputs(MacroHandle handle, unsigned int* count) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return macroWrapper->getOutputsCInterface(count);
}

DataDescriptor* macroGetParameters(MacroHandle handle, unsigned int* count) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return macroWrapper->getParametersCInterface(count);
}

int macroStart(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return static_cast<int>(macroWrapper->init());
}

int macroApply(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return static_cast<int>(macroWrapper->apply());
}

int macroStop(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return static_cast<int>(macroWrapper->exit());
}

void macroSetParameterValue(MacroHandle handle, unsigned int parameter, const wchar_t* cstrValue) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  macroWrapper->setParameterValueAsString(parameter,cstrValue);
}

const wchar_t* macroGetParameterValue(MacroHandle handle, unsigned int parameter) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  return macroWrapper->getParameterValueAsString(parameter);
}

#if defined(QT_VERSION)
#include "macroextended.h"

void* macroCreateWidget(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  if (macroWrapper->getType() != Macro) {
    auto macroExt = dynamic_cast<MacroExtBase*>(macroWrapper->data());
    assert(macroExt != nullptr);
    return reinterpret_cast<void*>(macroExt->createWidget());
  }
  return nullptr;
}

void macroDestroyWidget(MacroHandle handle) {
  auto macroWrapper = static_cast<MacroAPIWrapper*>(handle);
  assert(macroWrapper != nullptr);
  if (macroWrapper->getType() != Macro) {
    auto macroExt = dynamic_cast<MacroExtBase*>(macroWrapper->data());
    assert(macroExt != nullptr);
    macroExt->destroyWidget();
  }
}

#else
void* macroCreateWidget(MacroHandle /*handle*/) {
  return nullptr;
}

void macroDestroyWidget(MacroHandle /*handle*/) {
}

#endif

void notifyParameterChanged(MacroHandle handle, unsigned int parameter) {
  if (g_cbMacroParamChanged) {
    g_cbMacroParamChanged(handle,parameter,g_MacroBackReference[handle]);
  }
}

