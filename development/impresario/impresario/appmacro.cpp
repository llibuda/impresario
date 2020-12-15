/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2020  Lars Libuda
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

#include "appmacro.h"
#include "appmacromanager.h"
#include "pgeitems.h"
#include "sysloglogger.h"
#include <QMetaProperty>
#include <QCoreApplication>
#include <QMetaObject>
#include <QStringList>
#include <QElapsedTimer>

namespace app
{
  //-----------------------------------------------------------------------
  // Class MacroParameter
  //-----------------------------------------------------------------------
  MacroParameter::MacroParameter(const Macro& macro, const QString& paramName, const QString& descr, const QString& paramType, const QString& paramConfig, int idx) : QObject(0),
    macroRef(macro), name(paramName), description(descr), type(paramType), qmlUIComponent(), qmlUIProperties(), propValue(), propDefaultValue(), index(idx), mutex(QMutex::Recursive)
  {
    QStringList config = paramConfig.split('|',Qt::SkipEmptyParts);
    if (config.count() > 0)
    {
      qmlUIComponent = config[0];
    }
    if (config.count() > 1)
    {
      qmlUIProperties = config[1];
    }
  }

  MacroParameter::~MacroParameter()
  {
  }

  //-----------------------------------------------------------------------
  // Class MacroPin
  //-----------------------------------------------------------------------
  MacroPin::MacroPin(const Macro& macro, const QString& itemName, const QString& itemDescr, const QString& itemType, void* itemData, graph::Defines::PinDirectionType direction) : graph::PinData(itemName + ": " + itemType,direction),
    dataPtr(itemData), macroRef(macro), description(itemDescr), type(itemType), pinName(itemName)
  {
    setPropertyStartIndex(app::MacroPin::staticMetaObject.propertyOffset());
  }

  MacroPin::~MacroPin()
  {
  }

  bool MacroPin::allowConnectionTo(const PinData& other) const
  {
    const MacroPin& otherPin = static_cast<const MacroPin&>(other);
    return &macroRef != &(otherPin.macroRef) && type == otherPin.type;
  }

  QSharedPointer<graph::BaseItem> MacroPin::createVisualization(graph::BaseElement& elementRef, graph::BaseItem* parent)
  {
    return QSharedPointer<graph::BaseItem>(new pge::MacroPinItem(static_cast<graph::Pin&>(elementRef),parent));
  }

  //-----------------------------------------------------------------------
  // Class MacroInput
  //-----------------------------------------------------------------------
  MacroInput::MacroInput(const Macro& macro, const QString& itemName, const QString& itemDescr, const QString& itemType, void* itemData) : MacroPin(macro,itemName,itemDescr,itemType,itemData,graph::Defines::Incoming)
  {
  }

  MacroInput::~MacroInput()
  {
  }

  //-----------------------------------------------------------------------
  // Class MacroOutput
  //-----------------------------------------------------------------------
  MacroOutput::MacroOutput(const Macro& macro, const QString& itemName, const QString& itemDescr, const QString& itemType, void* itemData) : MacroPin(macro,itemName,itemDescr,itemType,itemData,graph::Defines::Outgoing)
  {
  }

  MacroOutput::~MacroOutput()
  {
  }

  //-----------------------------------------------------------------------
  // Class MacroLink
  //-----------------------------------------------------------------------
  MacroLink::MacroLink() : graph::EdgeData()
  {
    // Initially, prototype links are loaded in separate thread. The following makes sure, that all link instances live in the application's main thread
    moveToThread(QCoreApplication::instance()->thread());
  }

  QSharedPointer<graph::BaseItem> MacroLink::createVisualization(graph::BaseElement& elementRef, graph::BaseItem* parent)
  {
    return QSharedPointer<graph::BaseItem>(new pge::MacroLinkItem(static_cast<graph::Edge&>(elementRef),parent));
  }

  void MacroLink::elementStatusUpdated(graph::BaseElement &element, int change)
  {
    switch(change)
    {
      case graph::BaseElement::AddedToGraph:
      {
        graph::Edge& edge = static_cast<graph::Edge&>(element);
        MacroInput::Ptr pinIn = edge.destPin()->dataRef().staticCast<MacroInput>();
        MacroOutput::Ptr pinOut = edge.srcPin()->dataRef().staticCast<MacroOutput>();
        if (!pinIn.isNull() && !pinOut.isNull())
        {
          pinIn->setDataPtr(*pinOut.data());
        }
        break;
      }
      case graph::BaseElement::RemovedFromGraph:
      {
        graph::Edge& edge = static_cast<graph::Edge&>(element);
        MacroInput::Ptr pinIn = edge.destPin()->dataRef().staticCast<MacroInput>();
        if (!pinIn.isNull())
        {
          pinIn->resetDataPtr();
        }
        break;
      }
    }
  }

  //-----------------------------------------------------------------------
  // Class Macro
  //-----------------------------------------------------------------------
  Macro::Macro(const MacroLibrary& lib) : graph::VertexData(), library(lib), name(), creator(), group(), description(), errorMsg(), propertyWidgetComponent(),
    macroClass(), type(Undefined), params(), prototype(0), mutex(QMutex::Recursive), runTime(0), state(Idle), viewers()
  {
    // Initially, prototype macros are loaded in separate thread. The following makes sure, that all macro instances live in the application's main thread
    moveToThread(QCoreApplication::instance()->thread());
  }

  Macro::~Macro()
  {
    for(int index = 0; index < params.count(); ++index)
    {
      delete params[index].value<MacroParameter*>();
    }
    params.clear();
    viewers.clear();
  }

  void Macro::save(QXmlStreamWriter& stream) const
  {
    writeElementStart(stream);
    writeProperties(stream);

    stream.writeStartElement("parameters");
    foreach(QVariant variant, params)
    {
      MacroParameter* param = variant.value<MacroParameter*>();
      stream.writeStartElement("parameter");
      stream.writeAttribute("name",param->getName());
      stream.writeAttribute("type",param->getType());
      stream.writeCharacters(param->getValue().toString());
      stream.writeEndElement(); // parameter
    }
    stream.writeEndElement(); // parameters

    writeElementEnd(stream);
  }

  bool Macro::load(QXmlStreamReader &stream)
  {
    if (!readElementStart(stream)) return false;
    if (stream.readNextStartElement() && stream.name() == "parameters")
    {
      // build internal map
      QMap<QString, MacroParameter*> paramMap;
      foreach(QVariant paramVariant,params)
      {
        MacroParameter* param = paramVariant.value<MacroParameter*>();
        paramMap.insert(param->getName() + param->getType(),param);
      }
      // read parameters
      while(stream.readNextStartElement() && stream.name() == "parameter")
      {
        QString paramName = stream.attributes().value("name").toString();
        QString paramType = stream.attributes().value("type").toString();
        QString paramValue = stream.readElementText();
        if (paramMap.contains(paramName + paramType))
        {
          MacroParameter* param = paramMap[paramName + paramType];
          // TODO: What happens if this value was manipulated in XML file?
          param->setValue(QVariant(paramValue));
        }
        else
        {
          QFile* file = qobject_cast<QFile*>(stream.device());
          Q_ASSERT(file != 0);
          QString filename = file->fileName().split('/').last();
          syslog::warning(QString(tr("%5: Parameter '%1' of type '%2' is not supported by macro '%3' with signature '%4'. Ignored.")).arg(paramName).arg(paramType).arg(getName()).arg(getTypeSignature()).arg(filename),tr("Process Graph"));
        }
      }
    }
    if (!readElementEnd(stream)) return false;
    return true;
  }

  bool Macro::registerViewer(QSharedPointer<MacroViewer> viewer)
  {
    if (!viewer.isNull())
    {
      QMutexLocker locker(&mutex);
      if (viewer->start() == 0)
      {
        viewers.insert(viewer.data());
        return true;
      }
    }
    return false;
  }

  bool Macro::unregisterViewer(QSharedPointer<MacroViewer> viewer)
  {
    QMutexLocker locker(&mutex);
    if (viewers.contains(viewer.data()))
    {
      viewers.remove(viewer.data());
      return viewer->stop() == 0;
    }
    return false;
  }

  QString Macro::getRuntimeString() const
  {
    mutex.lock();
    qint64 musecs = runTime;
    mutex.unlock();
    qint64 msecs = musecs / 1000;
    qint64 secs = msecs / 1000;
    if (secs > 0)
    {
      return QString("%1.%2 s").arg(secs).arg(msecs % 1000);
    }
    else if (musecs > 0)
    {
      return QString("%1.%2 ms").arg(msecs).arg(musecs % 1000);
    }
    else
    {
      return QString("0.000 ms");
    }
  }

  //-----------------------------------------------------------------------
  // Class MacroDLL
  //-----------------------------------------------------------------------
  MacroDLL::MacroDLL(const MacroLibraryDLL& lib, const MacroLibraryDLL::MacroHandle& handle) : Macro(lib), macroHandle(handle)
  {
    // fill general attributes
    name = lib.getMacroName(macroHandle);
    creator = lib.getMacroCreator(macroHandle);
    group = lib.getMacroGroup(macroHandle);
    description = lib.getMacroDescription(macroHandle);
    propertyWidgetComponent = lib.getMacroPropertyWidgetComponent(macroHandle);
    switch(lib.getMacroType(macroHandle))
    {
      case 0:
        macroClass = tr("Macro");
        type = NormalMacro;
        break;
      case 1:
        macroClass = tr("Macro");
        type = ExtendedMacro;
        break;
      case 2:
        macroClass = tr("Viewer");
        type = Viewer;
        break;
    }
    setSignature(name + '_' + lib.getName() + '_' + lib.getVersionString() + '_' +
                 QString("%1").arg(lib.getCompilerVersion()) + ((lib.isDebugVersion()) ? "d" : "r"));
    // create macro inputs
    unsigned int count = 0;
    MacroLibraryDLL::DataDescriptor* dataDescr = lib.getMacroInputs(macroHandle,count);
    while(dataDescr)
    {
      QString itemName = QString::fromWCharArray(dataDescr->name);
      QString itemDescription = QString::fromWCharArray(dataDescr->description);
      QString itemType = QString::fromLatin1(dataDescr->type);
      graph::PinData::Ptr item = graph::PinData::Ptr(new MacroInput(*this,itemName,itemDescription,itemType,dataDescr->valuePtr));
      addPinData(item);
      dataDescr = dataDescr->next;
    }
    // create macro outputs
    count = 0;
    dataDescr = lib.getMacroOutputs(macroHandle,count);
    while(dataDescr)
    {
      QString itemName = QString::fromWCharArray(dataDescr->name);
      QString itemDescription = QString::fromWCharArray(dataDescr->description);
      QString itemType = QString::fromLatin1(dataDescr->type);
      graph::PinData::Ptr item = graph::PinData::Ptr(new MacroOutput(*this,itemName,itemDescription,itemType,dataDescr->valuePtr));
      addPinData(item);
      dataDescr = dataDescr->next;
    }
    // create macro parameters
    dataDescr = lib.getMacroParameters(macroHandle,count);
    count = 0;
    while(dataDescr)
    {
      QString itemName = QString::fromWCharArray(dataDescr->name);
      QString itemDescription = QString::fromWCharArray(dataDescr->description);
      QString itemType = QString::fromLatin1(dataDescr->type);
      QString itemConfig;
      if (dataDescr->valuePtr != 0)
      {
        itemConfig = QString::fromWCharArray(reinterpret_cast<const wchar_t*>(dataDescr->valuePtr));
      }
      MacroParameter* item = new MacroParameter(*this,itemName,itemDescription,itemType,itemConfig,count);
      QString itemValue = lib.getMacroParameter(macroHandle,count++);
      QVariant value(itemValue);
      item->setDefaultValue(value);
      params.append(QVariant::fromValue(item));
      connect(item,SIGNAL(valueChangedByUser()),this,SLOT(parameterChangedByUser()));
      dataDescr = dataDescr->next;
    }
  }

  MacroDLL::~MacroDLL()
  {
    if (prototype)
    {
      disconnect(this,SIGNAL(instanceDataUpdated(graph::Vertex&,app::Macro::UpdateReason)),prototype,SIGNAL(instanceDataUpdated(graph::Vertex&,app::Macro::UpdateReason)));
      prototype = 0;
    }
    const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
    lib.deleteMacro(macroHandle);
  }

  graph::VertexData::Ptr MacroDLL::clone()
  {
    const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
    const MacroLibraryDLL::MacroHandle handle = lib.cloneMacro(this->macroHandle);
    if (!handle)
    {
      return graph::VertexData::Ptr();
    }
    graph::VertexData::Ptr instance = graph::VertexData::Ptr(new MacroDLL(lib,handle));
    if (instance.isNull())
    {
      lib.deleteMacro(handle);
    }
    Macro* macroInstance = static_cast<Macro*>(instance.data());
    lib.setMacroImpresarioDataPtr(handle,macroInstance);
    macroInstance->prototype = (this->prototype == 0) ? this : this->prototype;
    connect(macroInstance,SIGNAL(instanceDataUpdated(graph::Vertex&,app::Macro::UpdateReason)),macroInstance->prototype,SIGNAL(instanceDataUpdated(graph::Vertex&,app::Macro::UpdateReason)));
    return instance;
  }

  QSharedPointer<graph::BaseItem> MacroDLL::createVisualization(graph::BaseElement& elementRef, graph::BaseItem* parent)
  {
    return QSharedPointer<graph::BaseItem>(new pge::MacroItem(static_cast<graph::Vertex&>(elementRef),parent));
  }

  int MacroDLL::start()
  {
    const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
    int result = lib.startMacro(macroHandle);
    mutex.lock();
    if (result > 1)
    {
      state = Failure;
      errorMsg = lib.getMacroErrorMsg(macroHandle);
    }
    else
    {
      state = Ok;
    }
    mutex.unlock();
    return result;
  }

  int MacroDLL::apply()
  {
    QElapsedTimer time;
    mutex.lock();
    state = Running;
    mutex.unlock();
    emit dataUpdated();
    const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
    time.start();
    int result = lib.applyMacro(macroHandle);
    runTime = time.nsecsElapsed() / 1000; // runtime in micro seconds
    mutex.lock();
    if (result > 1)
    {
      errorMsg = lib.getMacroErrorMsg(macroHandle);
    }
    if (viewers.count() > 0 && result < 1)
    {
      for(ViewerSet::iterator it = viewers.begin(); it != viewers.end(); ++it)
      {
        if ((*it)->apply() > 1)
        {
          errorMsg = QString(tr("Attached viewer '%1': %2").arg((*it)->getName())).arg((*it)->getErrorMsg());
          result = 2;
          break;
        }
      }
      emit updateViewers();
    }
    state = (result > 1) ? Failure : Ok;
    mutex.unlock();
    emit dataUpdated();
    return result;
  }

  int MacroDLL::stop()
  {
    const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
    int result = lib.stopMacro(macroHandle);
    mutex.lock();
    if (state != Failure)
    {
      if (result > 1)
      {
        errorMsg = lib.getMacroErrorMsg(macroHandle);
      }
      else
      {
        state = Idle;
      }
    }
    mutex.unlock();
    return result;
  }

  QWidget* MacroDLL::createWidget()
  {
    const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
    if (lib.getMacroType(macroHandle) == 0) return 0;
    QWidget* widget = 0;
    try
    {
      widget = reinterpret_cast<QWidget*>(lib.createMacroWidget(macroHandle));
      if (widget != 0) widget->update();
      return widget;
    }
    catch(...)
    {
      if (widget != 0) lib.destroyMacroWidget(macroHandle);
      return 0;
    }

  }

  void MacroDLL::destroyWidget()
  {
    const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
    if (lib.getMacroType(macroHandle) > 0)
    {
      lib.destroyMacroWidget(macroHandle);
    }
  }

  void MacroDLL::parameterChangedByMacro(unsigned int parameterIndex)
  {
    if (parameterIndex < (unsigned int)params.count())
    {
      MacroParameter* param = params.at(parameterIndex).value<MacroParameter*>();
      if (param)
      {
        const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
        mutex.lock();
        QString value = lib.getMacroParameter(macroHandle,parameterIndex);
        mutex.unlock();
        param->updateValueByMacro(QVariant(value));
        emit parameterUpdated(param->getIndex());
      }
    }
  }

  void MacroDLL::parameterChangedByUser()
  {
    MacroParameter* param = qobject_cast<MacroParameter*>(QObject::sender());
    if (param != 0)
    {
      const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
      mutex.lock();
      lib.setMacroParameter(macroHandle,param->getIndex(),param->getValue().toString());
      mutex.unlock();
    }
  }

  //-----------------------------------------------------------------------
  // Class MacroViewer
  //-----------------------------------------------------------------------
  MacroViewer::MacroViewer(const MacroLibraryDLL &lib, const MacroLibraryDLL::MacroHandle &handle) : MacroDLL(lib,handle), dataTypeMap()
  {
    const graph::VertexData::PinDataMap& pins = pinData();
    for(graph::VertexData::PinDataMap::const_iterator it = pins.begin(); it != pins.end(); ++it)
    {
      if (it.value()->direction() == graph::Defines::Incoming)
      {
        MacroInput::Ptr input = it.value().staticCast<MacroInput>();
        dataTypeMap.insert(input->getType(),input.toWeakRef());
      }
    }
  }

  MacroViewer::~MacroViewer()
  {
  }

  graph::VertexData::Ptr MacroViewer::clone()
  {
    const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
    const MacroLibraryDLL::MacroHandle handle = lib.cloneMacro(this->macroHandle);
    if (!handle)
    {
      return graph::VertexData::Ptr();
    }
    graph::VertexData::Ptr instance = graph::VertexData::Ptr(new MacroViewer(lib,handle));
    if (instance.isNull())
    {
      lib.deleteMacro(handle);
    }
    Macro* macroInstance = static_cast<Macro*>(instance.data());
    lib.setMacroImpresarioDataPtr(handle,macroInstance);
    macroInstance->prototype = (this->prototype == 0) ? this : this->prototype;
    return instance;
  }

  QSharedPointer<graph::BaseItem> MacroViewer::createVisualization(graph::BaseElement& /*elementRef*/, graph::BaseItem* /*parent*/)
  {
    return QSharedPointer<graph::BaseItem>();
  }

  bool MacroViewer::setData(MacroOutput::Ptr data)
  {
    if (!data.isNull() && dataTypeMap.contains(data->getType()))
    {
      MacroInput::Ref inputRef = dataTypeMap[data->getType()];
      if (!inputRef.isNull())
      {
        return inputRef->setDataPtr(*data.data());
      }
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }

  int MacroViewer::start()
  {
    const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
    int result = lib.startMacro(macroHandle);
    mutex.lock();
    if (result > 1)
    {
      state = Failure;
      errorMsg = lib.getMacroErrorMsg(macroHandle);
    }
    else
    {
      state = Ok;
    }
    mutex.unlock();
    return result;
  }

  int MacroViewer::apply()
  {
    const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
    int result = lib.applyMacro(macroHandle);
    mutex.lock();
    if (result > 1)
    {
      state = Failure;
      errorMsg = lib.getMacroErrorMsg(macroHandle);
    }
    else
    {
      state = Ok;
    }
    mutex.unlock();
    return result;
  }

  int MacroViewer::stop()
  {
    const MacroLibraryDLL& lib = static_cast<const MacroLibraryDLL&>(library);
    int result = lib.stopMacro(macroHandle);
    mutex.lock();
    if (result > 1)
    {
      errorMsg = lib.getMacroErrorMsg(macroHandle);
    }
    if (state != Failure)
    {
      state = (result > 1) ? Failure : Idle;
    }
    mutex.unlock();
    return result;
  }

}
