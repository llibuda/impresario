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
#ifndef MACROBASE_H_
#define MACROBASE_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
  #define _CRT_SECURE_NO_DEPRECATE
#endif

#include "libinterface.h"
#include <assert.h>
#include <stdlib.h>
#include <typeinfo>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#if defined(__GNUC__)
  #include <cxxabi.h>
#endif

//------------------------------------------
// Helper classes for determining type names
//------------------------------------------
template <typename T>
struct TypeName {
  static std::string get() {
    std::string strTypeName;
#if defined(__GNUC__)
    int status;
    char* szTypeName = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    strTypeName = std::string(szTypeName);
    free(szTypeName);
#else
    strTypeName = std::string(typeid(T).name());
    // if the type name contains "class ", we will erase this to make this type
    // name compatible with those created by GCC
    std::size_t pos = strTypeName.find("class ",0);
    while(pos != std::string::npos)
    {
      strTypeName.erase(pos,6);
      pos = strTypeName.find("class ",0);
    }
    // if the type name contains an asterix with leading white space, we'll erase the
    // white space to make this type name compatible with those created by GCC
    pos = strTypeName.find(" *",0);
    while(pos != std::string::npos)
    {
      strTypeName.erase(pos,1);
      pos = strTypeName.find(" *",0);
    }
    // if the type name contains an asterix with leading white space, we'll erase the
    // white space to make this type name compatible with those created by GCC
    pos = strTypeName.find(" __ptr64",0);
    while(pos != std::string::npos)
    {
      strTypeName.erase(pos,8);
      pos = strTypeName.find(" __ptr64",0);
    }
#endif
    return strTypeName;
  }
};

template <>
struct TypeName<std::string> {
  static std::string get() {
    return std::string("std::string");
  }
};

template <>
struct TypeName<std::wstring> {
  static std::string get() {
    return std::string("std::wstring");
  }
};

//------------------------------------------
// Class ValueBase
//------------------------------------------
class ValueBase {
public:
  virtual ~ValueBase() {
  }

  const std::wstring& getName() const        { return m_strName; }
  const std::wstring& getDescription() const { return m_strDescription; }
  DataDescriptor* getDescriptorPtr()         { return &m_dataDescriptor; }

protected:
  ValueBase(const std::wstring& strName, const std::wstring& strDescription, void* dataPtr, const std::string& typeName) : m_strName(strName), m_strDescription(strDescription), m_strTypeName(typeName) {
    m_dataDescriptor.name = m_strName.c_str();
    m_dataDescriptor.description = m_strDescription.c_str();
    m_dataDescriptor.valuePtr = dataPtr;
    m_dataDescriptor.next = 0;
    m_dataDescriptor.type = m_strTypeName.c_str();
  }

private:
  std::wstring   m_strName;
  std::wstring   m_strDescription;
  std::string    m_strTypeName;
  DataDescriptor m_dataDescriptor;
};

//------------------------------------------
// Class MacroInput
//------------------------------------------
template <class T>
class MacroInput : public ValueBase {
public:
  MacroInput(const std::wstring& strName, const std::wstring& strDescription) : ValueBase(strName,strDescription,&m_ptValue,TypeName<T>::get()), m_ptValue(0) {}
  virtual ~MacroInput() {}
  const T* readAccess() const { return m_ptValue; }

private:
  T* m_ptValue;
};

//------------------------------------------
// Class MacroOuput
//------------------------------------------
template <class T>
class MacroOutput : public ValueBase {
public:
  MacroOutput(const std::wstring& strName, const std::wstring& strDescription) : ValueBase(strName,strDescription,&m_tValue,TypeName<T>::get()), m_tValue(T()) {}
  virtual ~MacroOutput()  {}
  T& writeAccess() { return m_tValue; }

private:
  T m_tValue;
};

//------------------------------------------
// Class ValueParameter
//------------------------------------------
class ValueParameter : public ValueBase {
public:
  ValueParameter(const std::wstring& strName, const std::wstring& strDescription, void* dataPtr, const std::string& typeName) : ValueBase(strName,strDescription,dataPtr,typeName) {
  }

  // abstract methods to set and get a parameter as string
  virtual void setValueAsString(const std::wstring& strValue) = 0;
  virtual const std::wstring& getValueAsString() const = 0;

};

//------------------------------------------
// Class ParameterValueConverter and specializations
//------------------------------------------
template <class T>
class ParameterValueConverter {
public:
  virtual T fromString(const std::wstring& strValue) const {
    std::basic_istringstream< wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t> > iss(strValue);
    T value;
    iss >> value;
    return value;
  }

  virtual std::wstring toString(T value) const {
    std::basic_ostringstream< wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t> > oss;
    oss << value;
    return std::wstring(oss.str());
  }
};

template <>
class ParameterValueConverter<std::wstring> {
public:
  virtual std::wstring fromString(const std::wstring& strValue) const {
    return strValue;
  }

  virtual std::wstring toString(std::wstring value) const {
    return value;
  }
};

template <>
class ParameterValueConverter<std::string> {
public:
  virtual std::string fromString(const std::wstring& strValue) const {
    char* str = new char[strValue.length() + 1];
    wcstombs(str,strValue.c_str(),strValue.length() + 1);
    std::string string(str);
    delete [] str;
    return string;
  }

  virtual std::wstring toString(std::string value) const {
    wchar_t* wstr = new wchar_t[value.length() + 1];
    mbstowcs(wstr,value.c_str(),value.length() + 1);
    std::wstring wstring(wstr);
    delete [] wstr;
    return wstring;
  }
};

//------------------------------------------
// Class MacroParameter
//------------------------------------------
template <class T>
class MacroParameter : public ValueParameter {
public:
  MacroParameter(const std::wstring& strName, const std::wstring strDescription, const T& tDefaultValue, const std::wstring& qmlUIComponent, const std::wstring& qmlUIProperties, const ParameterValueConverter<T>& converter) : ValueParameter(strName,strDescription,0,TypeName<T>::get()),
    m_converter(converter), m_tValue(tDefaultValue), m_tDefaultValue(tDefaultValue), m_strValue(), m_strAttributes() {
    m_strValue = m_converter.toString(m_tValue);
    m_strAttributes = qmlUIComponent + L'|' + qmlUIProperties;
    DataDescriptor* data = getDescriptorPtr();
    assert(data != 0);
    data->valuePtr = reinterpret_cast<void*>(const_cast<wchar_t*>(m_strAttributes.c_str()));
  }

  virtual ~MacroParameter() {}

  void     setValue(const T& tValue) { m_tValue = tValue; m_strValue = m_converter.toString(tValue); }
  const T& getValue() const          { return m_tValue; }
  const T& getDefault() const        { return m_tDefaultValue; }
  
  virtual void setValueAsString(const std::wstring& strValue)  { 
    m_tValue = m_converter.fromString(strValue); 
    m_strValue = strValue;
  }

  virtual const std::wstring& getValueAsString() const {
    return m_strValue;
  }

private:
  ParameterValueConverter<T> m_converter;
  T                          m_tValue;
  T                          m_tDefaultValue;
  std::wstring               m_strValue;
  std::wstring               m_strAttributes;
};

//------------------------------------------
// Class MacroBase
//------------------------------------------
class MacroBase {
public:
  // data type to hold inputs, outputs, and parameters
  typedef std::vector<ValueBase*> ValueVector;

  // standard constructor
  MacroBase(void) : m_strMacroName(), m_strMacroGroup(), m_strMacroCreator(), m_strMacroDescription(), m_strMacroMsg(), m_strPropWidgetFile(),
                    m_vecInput(), m_vecOutput(), m_vecParams(), m_setChangedParams(), m_ptrImpresarioData(0) {
  }

  // standard destructor
  virtual ~MacroBase(void) {
    while(!m_vecInput.empty()) {
      delete m_vecInput.front();
      m_vecInput.erase(m_vecInput.begin());
    }
    while(!m_vecOutput.empty()) {
      delete m_vecOutput.front();
      m_vecOutput.erase(m_vecOutput.begin());
    }
    while(!m_vecParams.empty()) {
      delete m_vecParams.front();
      m_vecParams.erase(m_vecParams.begin());
    }
  }

  // clone is called every time this macro is selected by the user and added to the editor
  // override to provide a copy of the original macro, which is created when the DLL is loaded
  virtual MacroBase* clone() const { return 0; }

  // methods to access private attributes, neccessary for the main application
  const   std::wstring& getName() const                    { return m_strMacroName; }
  const   std::wstring& getGroup() const                   { return m_strMacroGroup; }
  const   std::wstring& getCreator() const                 { return m_strMacroCreator; }
  const   std::wstring& getDescription() const             { return m_strMacroDescription; }
  const   std::wstring& getErrorMsg() const                { return m_strMacroMsg; }
  const   std::wstring& getPropertyWidgetComponent() const { return m_strPropWidgetFile; }
  const   ValueVector&  getInputs() const                  { return m_vecInput; }
  const   ValueVector&  getOutputs() const                 { return m_vecOutput; }
  const   ValueVector&  getParameters() const              { return m_vecParams; }
  virtual MacroType     getType() const                    { return Macro; }

  // methods to read and write parameters
  const std::wstring& getParameterValueAsString(unsigned int parameterIndex) const {
    assert(parameterIndex < m_vecParams.size());
    return static_cast<ValueParameter*>(m_vecParams[parameterIndex])->getValueAsString();
  }

  void setParameterValueAsString(unsigned int parameterIndex, std::wstring& value) {
    assert(parameterIndex < m_vecParams.size());
    m_setChangedParams.insert(parameterIndex);
    static_cast<ValueParameter*>(m_vecParams[parameterIndex])->setValueAsString(value);
  }

  // methods to set and read the data ptr set by Impresario
  void* getImpresarioDataPtr() const {
    return m_ptrImpresarioData;
  }

  void setImpresarioDataPtr(void* dataPtr) {
    m_ptrImpresarioData = dataPtr;
  }

  // methods for executing macro
  enum Status {
    Ok,
    Stop,
    Error
  };

  Status init()  {
    m_strMacroMsg.clear();
    for(std::size_t index = 0; index < m_vecParams.size(); ++index) {
      m_setChangedParams.insert(static_cast<unsigned int>(index));
    }
    onParametersChanged(m_setChangedParams);
    m_setChangedParams.clear();
    return onInit();
  }

  Status apply() {
    if (!m_setChangedParams.empty()) {
      onParametersChanged(m_setChangedParams);
      m_setChangedParams.clear();
    }
    return onApply();
  }

  Status exit()  {
    if (!m_setChangedParams.empty()) {
      onParametersChanged(m_setChangedParams);
      m_setChangedParams.clear();
    }
    return onExit();
  }

protected:
  typedef std::set<unsigned int> ParameterSet;

  // methods to be overwritten in derived classes to perfrom required actions
  virtual Status onInit()  { return Ok; }
  virtual Status onApply() { return Ok; }
  virtual Status onExit()  { return Ok; }
  virtual void   onParametersChanged(ParameterSet&) {}

  // API methods to set up derived classes
  void setName(const std::wstring& strName)                              { m_strMacroName = strName; }
  void setGroup(const std::wstring& strGroup)                            { m_strMacroGroup = strGroup; }
  void setCreator(const std::wstring& strCreator)                        { m_strMacroCreator = strCreator; }
  void setDescription(const std::wstring& strDescription)                { m_strMacroDescription = strDescription; }
  void setErrorMsg(const std::wstring& strErrorMsg)                      { m_strMacroMsg = strErrorMsg; }
  void setPropertyWidgetComponent(const std::wstring& strPropWidgetFile) { m_strPropWidgetFile = strPropWidgetFile; }

  // API methods to set up and access macro inputs
  template<class T>
  bool addInput(const std::wstring& name, const std::wstring& description) {
    if (name.empty()) {
      return false;
    }
    MacroInput<T>* input = new MacroInput<T>(name,description);
    if (m_vecInput.size() > 0) {
      DataDescriptor* descr = m_vecInput.back()->getDescriptorPtr();
      descr->next = input->getDescriptorPtr();
    }
    m_vecInput.push_back(input);
    return true;
  }

  template<class T>
  const T* accessInput(std::size_t index) {
    assert(index >= 0 && index < m_vecInput.size());
    MacroInput<T>* input = static_cast<MacroInput<T>*>(m_vecInput.at(index));
    const T* value = input->readAccess();
    return value;
  }

  // API methods to set up and access macro outputs
  template<class T>
  bool addOutput(const std::wstring& name, const std::wstring& description) {
    if (name.empty()) {
      return false;
    }
    MacroOutput<T>* output = new MacroOutput<T>(name,description);
    if (m_vecOutput.size() > 0) {
      DataDescriptor* descr = m_vecOutput.back()->getDescriptorPtr();
      descr->next = output->getDescriptorPtr();
    }
    m_vecOutput.push_back(output);
    return true;
  }

  template<class T>
  T& accessOutput(std::size_t index) {
    assert(index >= 0 && index < m_vecOutput.size());
    MacroOutput<T>* output = static_cast<MacroOutput<T>*>(m_vecOutput.at(index));
    return output->writeAccess();
  }

    // API methods to set up and access macro parameters
  template<class T>
  bool addParameter(const std::wstring& name, const std::wstring& description, const T& tDefaultValue, const std::wstring& qmlUIComponent = L"", const std::wstring& qmlUIProperties = L"", const ParameterValueConverter<T>& converter = ParameterValueConverter<T>()) {
    if (name.empty()) {
      return false;
    }
    MacroParameter<T>* param = new MacroParameter<T>(name,description,tDefaultValue,qmlUIComponent,qmlUIProperties,converter);
    if (m_vecParams.size() > 0) {
      DataDescriptor* descr = m_vecParams.back()->getDescriptorPtr();
      descr->next = param->getDescriptorPtr();
    }
    m_vecParams.push_back(param);
    return true;
  }

  template<class T>
  const T& getParameterValue(std::size_t index) {
    assert(index >= 0 && index < m_vecParams.size());
    MacroParameter<T>* param = static_cast<MacroParameter<T>*>(m_vecParams.at(index));
    return param->getValue();
  }

  template<class T>
  void setParameterValue(std::size_t index, const T& value) {
    assert(index >= 0 && index < m_vecParams.size());
    MacroParameter<T>* param = static_cast<MacroParameter<T>*>(m_vecParams.at(index));
    if (value != param->getValue()) {
      param->setValue(value);
      notifyParameterChanged((MacroHandle)this,static_cast<unsigned int>(index),m_ptrImpresarioData);
    }
  }

private:
  // attributes describing the macro class by its name, group, creator, and additional text
  std::wstring m_strMacroName;
  std::wstring m_strMacroGroup;
  std::wstring m_strMacroCreator;
  std::wstring m_strMacroDescription;
  std::wstring m_strMacroMsg;
  std::wstring m_strPropWidgetFile;
  ValueVector  m_vecInput;
  ValueVector  m_vecOutput;
  ValueVector  m_vecParams;
  ParameterSet m_setChangedParams;
  void*        m_ptrImpresarioData;
};

#endif /* MACROBASE_H_ */
