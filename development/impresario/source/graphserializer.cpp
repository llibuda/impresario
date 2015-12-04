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

#include "graphserializer.h"
#include "grapheditor.h"
#include "graphitems.h"
#include "graphdata.h"
#include "graphmain.h"
#include <QMetaProperty>
#include <QMetaObject>

namespace graph
{

  Serializer::Serializer(const QString& element, int startIndex, QObject* objPtr) : elementName(element), propertyOffset(startIndex), obj(objPtr)
  {
  }

  Serializer::Serializer() : elementName("default"), propertyOffset(1)
  {
  }

  Serializer::~Serializer()
  {
  }

  void Serializer::save(QXmlStreamWriter& stream) const
  {
    writeElementStart(stream);
    writeProperties(stream);
    writeElementEnd(stream);
  }

  bool Serializer::load(QXmlStreamReader& stream)
  {
    if (!readElementStart(stream)) return false;
    if (!readProperties(stream)) return false;
    if (!readElementEnd(stream)) return false;
    return true;
  }

  void Serializer::writeElementStart(QXmlStreamWriter& stream) const
  {
    stream.writeStartElement(elementName);
    const QMetaObject *metaobject = obj->metaObject();
    QString className = QString::fromLatin1(metaobject->className());
    stream.writeAttribute("class",className);
    const BaseElement* baseElement = qobject_cast<const BaseElement*>(obj);
    if (baseElement)
    {
      stream.writeAttribute("id",baseElement->id().toString());
    }
    const GraphElement* graphElement = qobject_cast<const GraphElement*>(obj);
    if (graphElement)
    {
      GraphData::Ptr ptr = graphElement->baseDataRef().dynamicCast<GraphData>();
      if (!ptr.isNull())
      {
        stream.writeAttribute("dataTypeSignature",ptr->signature());
      }
      else
      {
        stream.writeAttribute("dataTypeSignature","");
      }
    }
    const Edge* edgeElement = qobject_cast<const Edge*>(obj);
    if (edgeElement)
    {
      stream.writeAttribute("srcPinId",edgeElement->srcPin().data()->id().toString());
      stream.writeAttribute("destPinId",edgeElement->destPin().data()->id().toString());
    }
  }

  void Serializer::writeElementEnd(QXmlStreamWriter& stream) const
  {
    stream.writeEndElement();
  }

  void Serializer::writeProperties(QXmlStreamWriter& stream) const
  {
    const QMetaObject *metaobject = obj->metaObject();
    int total = metaobject->propertyCount();
    for (int i = propertyOffset; i < total; ++i)
    {
      QMetaProperty metaproperty = metaobject->property(i);
      const char* name = metaproperty.name();
      QVariant propValue = obj->property(name);
      //qDebug() << name << " " << metaproperty.typeName() << " " << metaproperty.userType() << " " << metaproperty.type();
      if (metaproperty.isEnumType())
      {
        QMetaEnum metaEnum = metaproperty.enumerator();
        stream.writeTextElement(QString(name),QString(metaEnum.valueToKey(propValue.toInt())));
      }
      else if (propValue.canConvert<graph::Scene::Ptr>())
      {
        Scene::Ptr ptr = propValue.value<graph::Scene::Ptr>();
        if (!ptr.isNull())
        {
          ptr->save(stream);
        }
        else
        {
          stream.writeStartElement(QString(name));
          stream.writeAttribute("class","");
          stream.writeEndElement();
        }
      }
      else if (propValue.canConvert<graph::BaseItem::Ptr>())
      {
        BaseItem::Ptr ptr = propValue.value<graph::BaseItem::Ptr>();
        if (!ptr.isNull())
        {
          ptr->save(stream);
        }
        else
        {
          stream.writeStartElement(QString(name));
          stream.writeAttribute("class","");
          stream.writeEndElement();
        }
      }
      else if (propValue.canConvert<graph::BaseData::Ptr>())
      {
        BaseData::Ptr ptr = propValue.value<graph::BaseData::Ptr>();
        if (!ptr.isNull())
        {
          ptr->save(stream);
        }
        else
        {
          stream.writeStartElement(QString(name));
          stream.writeAttribute("class","");
          stream.writeEndElement();
        }
      }
      else if (propValue.canConvert<QString>())
      {
        stream.writeTextElement(QString(name),propValue.toString());
      }
      else
      {
        stream.writeTextElement(QString(name),"");
      }
    }
  }

  bool Serializer::readElementStart(QXmlStreamReader &stream)
  {
    // compare to expected name
    if (stream.name() != elementName)
    {
      stream.raiseError(QString(QObject::tr("At line %1, column %2: Read element '%3' does not match expected name '%4'.")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(stream.name().toString()).arg(elementName));
      return false;
    }
    else
    {
      // read class attribute
      QString classNameXML = stream.attributes().value("class").toString();
      QString classNameObj = QString(obj->metaObject()->className());
      if (classNameXML != classNameObj)
      {
        stream.raiseError(QString(QObject::tr("At line %1, column %2: Read class name '%3' does not match instantiated class '%4' for element '%5'.")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(classNameXML).arg(classNameObj).arg(elementName));
        return false;
      }
      else
      {
        BaseElement* baseElement = qobject_cast<BaseElement*>(obj);
        if (baseElement)
        {
          QUuid id = QUuid(stream.attributes().value("id").toString());
          if (id.isNull())
          {
            stream.raiseError(QString(QObject::tr("At line %1, column %2: Element '%3' with class '%4' has an invalid id '%5'.")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(elementName).arg(classNameObj).arg(id.toString()));
            return false;
          }
          baseElement->setId(id);
        }
        return true;
      }
    }
  }

  bool Serializer::readElementEnd(QXmlStreamReader &stream)
  {
    stream.skipCurrentElement();
    return true;
  }

  bool Serializer::readProperties(QXmlStreamReader &stream)
  {
    const QMetaObject *metaobject = obj->metaObject();
    QString className = QString(metaobject->className());
    int total = metaobject->propertyCount();
    for (int i = propertyOffset; i < total; ++i)
    {
      QMetaProperty metaproperty = metaobject->property(i);
      const char* name = metaproperty.name();
      // read element matching property
      if (stream.readNextStartElement() && stream.name() == QString(name))
      {
        QVariant propValue = obj->property(name);
        if (propValue.canConvert<graph::Scene::Ptr>())
        {
          QString sceneClass = stream.attributes().value("class").toString();
          GraphBase* graphBase = qobject_cast<GraphBase*>(obj);
          if (!sceneClass.isEmpty() && graphBase)
          {
            if (!graphBase->scene()->load(stream)) return false;
          }
        }
        else if (propValue.canConvert<graph::BaseItem::Ptr>())
        {
          QString itemClass = stream.attributes().value("class").toString();
          BaseElement* baseElement = qobject_cast<BaseElement*>(obj);
          if (!itemClass.isEmpty() && baseElement)
          {
            if (!baseElement->sceneItem()->load(stream)) return false;
          }
        }
        else if (propValue.canConvert<graph::BaseData::Ptr>())
        {
          QString dataClass = stream.attributes().value("class").toString();
          BaseElement* baseElement = qobject_cast<BaseElement*>(obj);
          if (!dataClass.isEmpty() && baseElement)
          {
            if (!baseElement->baseDataRef()->load(stream)) return false;
          }
        }
        else if (propValue.canConvert<QString>())
        {
          QString content = stream.readElementText();
          if (metaproperty.isWritable())
          {
            bool propSet = true;
            bool valueSet = true;
            if (metaproperty.isEnumType() || metaproperty.isFlagType())
            {
              QMetaEnum metaEnum = metaproperty.enumerator();
              propSet = obj->setProperty(name,QVariant(metaEnum.keyToValue(content.toLatin1().data(),&valueSet)));
            }
            else
            {
              propSet = obj->setProperty(name,QVariant(content));
            }
            if (!propSet || !valueSet)
            {
              stream.raiseError(QString(QObject::tr("At line %1, column %2: Failed to set value '%3' for property '%4' of class instance '%5' (element '%6').")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(content).arg(QString(name)).arg(className).arg(elementName));
              return false;
            }
          }
          else
          {
            QString value = obj->property(name).toString();
            if (content != value)
            {
              stream.raiseError(QString(QObject::tr("At line %1, column %2: Constant value '%3' for read-only property '%4' of class instance '%5' does not match read content '%6' of element '%7'.")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(value).arg(QString(name)).arg(className).arg(content).arg(elementName));
              return false;
            }
          }
        }
        else
        {
          stream.raiseError(QString(QObject::tr("At line %1, column %2: Unsupported value type '%3' for property '%4' of class instance '%5' (element '%6').")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(QString(metaproperty.typeName())).arg(QString(name)).arg(className).arg(elementName));
          return false;
        }
      }
      else
      {
        stream.raiseError(QString(QObject::tr("At line %1, column %2: Failed to read element for property '%3' of class instance '%4' (element '%5').")).arg(stream.lineNumber()).arg(stream.columnNumber()).arg(QString(name)).arg(className).arg(elementName));
        return false;
      }
    }
    return true;
  }

}
