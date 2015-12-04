/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015  Lars Libuda
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
#ifndef APPMACRO_H
#define APPMACRO_H

#include "graphdata.h"
#include "graphelements.h"
#include "appmacrolibrary.h"
#include <QString>
#include <QTime>
#include <QSharedPointer>
#include <QMutex>
#include <QMutexLocker>
#include <QVariant>
#include <QVariantList>
#include <QMap>
#include <QList>
#include <QSet>
#include <QWidget>

namespace app
{
  class ProcessGraph;
  class Macro;
  class MacroViewer;

  class MacroParameter : public QObject
  {
    Q_OBJECT
    Q_DISABLE_COPY(MacroParameter)
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString type READ getType CONSTANT)
    Q_PROPERTY(QString component READ getUIComponent CONSTANT STORED false)
    Q_PROPERTY(QString properties READ getUIProperties CONSTANT STORED false)
    Q_PROPERTY(QString description READ getDescription CONSTANT STORED false)
    Q_PROPERTY(QVariant value READ getValue WRITE setValue NOTIFY valueChangedByUser)
  public:
    typedef QSharedPointer<MacroParameter> Ptr;

    MacroParameter(const Macro& macro, const QString& paramName, const QString& descr, const QString& paramType, const QString& paramConfig, int idx);
    ~MacroParameter();

    const QString& getName() const
    {
      return name;
    }

    const QString& getDescription() const
    {
      return description;
    }

    const QString& getType() const
    {
      return type;
    }

    const QString& getUIComponent() const
    {
      return qmlUIComponent;
    }

    const QString& getUIProperties() const
    {
      return qmlUIProperties;
    }

    int getIndex() const
    {
      return index;
    }

    void setValue(const QVariant& val)
    {
      QMutexLocker lock(&mutex);
      if (val != propValue)
      {
        propValue = val;
        emit valueChangedByUser();
      }
    }

    QVariant getValue() const
    {
      QMutexLocker lock(&mutex);
      return propValue;
    }

    void setDefaultValue(const QVariant& defaultValue)
    {
      QMutexLocker lock(&mutex);
      if (defaultValue != propDefaultValue)
      {
        propDefaultValue = defaultValue;
        propValue = defaultValue;
        emit valueChangedByUser();
      }
    }

    void updateValueByMacro(const QVariant& val)
    {
      QMutexLocker lock(&mutex);
      if (val != propValue)
      {
        propValue = val;
        emit valueChangedByMacro();
      }
    }

  signals:
    void valueChangedByUser();
    void valueChangedByMacro();

  private:
    const Macro&   macroRef;
    QString        name;
    QString        description;
    QString        type;
    QString        qmlUIComponent;
    QString        qmlUIProperties;
    QVariant       propValue;
    QVariant       propDefaultValue;
    int            index;
    mutable QMutex mutex;
  };

  class MacroPin : public graph::PinData
  {
    Q_OBJECT
    Q_PROPERTY(QString name READ getName)
  public:
    typedef QSharedPointer<MacroPin> Ptr;

    ~MacroPin();

    const QString& getName() const
    {
      return pinName;
    }

    const QString& getDescription() const
    {
      return description;
    }

    const QString& getType() const
    {
      return type;
    }

    void* getDataPtr() const
    {
      return dataPtr;
    }

    const Macro& getMacro() const
    {
      return macroRef;
    }

    virtual bool allowConnectionTo(const PinData& other) const;
    virtual QSharedPointer<graph::BaseItem> createVisualization(graph::BaseElement& elementRef, graph::BaseItem* parent = 0);

  protected:
    Q_DISABLE_COPY(MacroPin)
    MacroPin(const Macro& macro, const QString& itemName, const QString& itemDescr, const QString& itemType, void* itemData, graph::Defines::PinDirectionType direction);

    void* dataPtr;

  private:
    const Macro& macroRef;
    QString      description;
    QString      type;
    QString      pinName;
  };

  class MacroOutput : public MacroPin
  {
    Q_OBJECT
    Q_DISABLE_COPY(MacroOutput)
    Q_PROPERTY(QString output READ getType)
  public:
    typedef QSharedPointer<MacroOutput> Ptr;
    typedef QWeakPointer<MacroOutput>   Ref;

    MacroOutput(const Macro& macro, const QString& itemName, const QString& itemDescr, const QString& itemType, void* itemData);
    ~MacroOutput();
  };

  class MacroInput : public MacroPin
  {
    Q_OBJECT
    Q_DISABLE_COPY(MacroInput)
    Q_PROPERTY(QString input READ getType)
  public:
    typedef QSharedPointer<MacroInput> Ptr;
    typedef QWeakPointer<MacroInput>   Ref;

    MacroInput(const Macro& macro, const QString& itemName, const QString& itemDescr, const QString& itemType, void* itemData);
    ~MacroInput();

    bool setDataPtr(const MacroOutput& output)
    {
      if (output.getType() != this->getType())
      {
        Q_ASSERT(false);
        return false;
      }
      void** ppData = reinterpret_cast<void**>(dataPtr);
      *ppData = output.getDataPtr();
      return true;
    }

    void resetDataPtr()
    {
      void** ppData = reinterpret_cast<void**>(dataPtr);
      *ppData = 0;
    }

  };

  class MacroLink : public graph::EdgeData
  {
    Q_OBJECT
  public:
    typedef QSharedPointer<MacroLink> Ptr;

    MacroLink() : graph::EdgeData()
    {
    }

    virtual graph::EdgeData::Ptr clone()
    {
      Ptr ptr = Ptr(new MacroLink());
      ptr->setSignature(this->signature());
      return ptr;
    }

    virtual QSharedPointer<graph::BaseItem> createVisualization(graph::BaseElement& elementRef, graph::BaseItem* parent = 0);

  public slots:
    virtual void elementStatusUpdated(graph::BaseElement& element, int change);
  };

  class Macro : public graph::VertexData
  {
    friend class MacroDLL;
    friend class MacroViewer;
    Q_OBJECT
    Q_DISABLE_COPY(Macro)
  public:
    typedef QSharedPointer<Macro> Ptr;
    typedef QWeakPointer<Macro>   Ref;

    virtual ~Macro();

    enum MacroType
    {
      Undefined = -1,
      NormalMacro = 0,
      ExtendedMacro = 1,
      Viewer = 2
    };

    MacroType getType() const
    {
      return type;
    }

    const QString& getName() const
    {
      return name;
    }

    const QString& getCreator() const
    {
      return creator;
    }

    const QString& getGroup() const
    {
      return group;
    }

    const QString& getDescription() const
    {
      return description;
    }

    const QString& getErrorMsg() const
    {
      return errorMsg;
    }

    const QString& getPropertyWidgetComponent() const
    {
      return propertyWidgetComponent;
    }

    const QString& getClass() const
    {
      return macroClass;
    }

    const QString& getBuild() const
    {
      return library.getBuildType();
    }

    const MacroLibrary& getLibrary() const
    {
      return library;
    }

    const QString& getTypeSignature() const
    {
      return signature();
    }

    qint64 getRuntime() const
    {
      QMutexLocker lock(&mutex);
      return runTime;
    }

    QString getRuntimeString() const;

    enum MacroState
    {
      Idle,
      Running,
      Ok,
      Failure
    };

    MacroState getState() const
    {
      QMutexLocker lock(&mutex);
      return state;
    }

    Q_INVOKABLE const QVariantList parameters() const
    {
      return params;
    }

    virtual int start() = 0;
    virtual int apply() = 0;
    virtual int stop() = 0;
    virtual QWidget* createWidget() = 0;
    virtual void destroyWidget() = 0;
    virtual void save(QXmlStreamWriter &stream) const;
    virtual bool load(QXmlStreamReader &stream);

    bool registerViewer(QSharedPointer<MacroViewer> viewer);
    bool unregisterViewer(QSharedPointer<MacroViewer> viewer);

    enum UpdateReason
    {
      GraphAssignment
    };

  signals:
    void instanceDataUpdated(graph::Vertex& vertex, app::Macro::UpdateReason reason);
    void updateViewers();
    void parameterUpdated(int index);

  public slots:
    virtual void elementStatusUpdated(graph::BaseElement& element, int change)
    {
      if (change == graph::BaseElement::GraphAssignment ||
          change == graph::BaseElement::AddedToGraph ||
          change == graph::BaseElement::RemovedFromGraph)
      {
        emit instanceDataUpdated(static_cast<graph::Vertex&>(element), GraphAssignment);
      }
    }

  protected slots:
    virtual void parameterChangedByUser() = 0;

  protected:
    Macro(const MacroLibrary& lib);

    typedef QSet<MacroViewer*> ViewerSet;

    // general attributes for all types of macros (no thread safe access)
    const MacroLibrary& library;
    QString             name;
    QString             creator;
    QString             group;
    QString             description;
    QString             errorMsg;
    QString             propertyWidgetComponent;
    QString             macroClass;
    MacroType           type;
    QVariantList        params;
    Macro*              prototype;
    // thread safe attributes for all types of macros
    mutable QMutex      mutex;
    qint64              runTime;
    MacroState          state;
    ViewerSet           viewers;
  };

  class MacroDLL : public Macro
  {
    friend class MacroLibraryDLL;
    Q_OBJECT
    Q_DISABLE_COPY(MacroDLL)
  public:
    virtual ~MacroDLL();
    virtual int start();
    virtual int apply();
    virtual int stop();
    virtual QWidget* createWidget();
    virtual void destroyWidget();
    virtual graph::VertexData::Ptr clone();
    virtual QSharedPointer<graph::BaseItem> createVisualization(graph::BaseElement& elementRef, graph::BaseItem* parent = 0);

    virtual void parameterChangedByMacro(unsigned int parameterIndex);

  protected slots:
    virtual void parameterChangedByUser();

  protected:
    MacroDLL(const MacroLibraryDLL& lib, const MacroLibraryDLL::MacroHandle& handle);

    MacroLibraryDLL::MacroHandle macroHandle;
  };

  class MacroViewer : public MacroDLL
  {
    friend class MacroLibraryDLL;
    Q_OBJECT
    Q_DISABLE_COPY(MacroViewer)
  public:
    typedef QSharedPointer<MacroViewer> Ptr;

    virtual ~MacroViewer();
    virtual int start();
    virtual int apply();
    virtual int stop();
    virtual graph::VertexData::Ptr clone();
    virtual QSharedPointer<graph::BaseItem> createVisualization(graph::BaseElement& elementRef, graph::BaseItem* parent = 0);

    QList<QString> dataTypes() const
    {
      return dataTypeMap.keys();
    }

    bool setData(MacroOutput::Ptr data);

  private:
    MacroViewer(const MacroLibraryDLL& lib, const MacroLibraryDLL::MacroHandle& handle);

    typedef QMap<QString,MacroInput::Ref> DataTypeMap;

    DataTypeMap dataTypeMap;
  };

}

#endif // APPMACRO_H
