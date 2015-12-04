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
#ifndef GRAPHSERIALIZER_H
#define GRAPHSERIALIZER_H

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace graph
{

  class Serializer
  {
  public:
    Serializer(const QString& element, int startIndex, QObject* objPtr);
    virtual ~Serializer();

    virtual void save(QXmlStreamWriter& stream) const;
    virtual bool load(QXmlStreamReader& stream);

    const QString& element() const
    {
      return elementName;
    }

    void setElement(const QString& element)
    {
      elementName = element;
    }

    int propertyStartIndex() const
    {
      return propertyOffset;
    }

    void setPropertyStartIndex(int startIndex)
    {
      propertyOffset = startIndex;
    }

  protected:
    void writeElementStart(QXmlStreamWriter& stream) const;
    void writeElementEnd(QXmlStreamWriter& stream) const;
    void writeProperties(QXmlStreamWriter& stream) const;
    bool readElementStart(QXmlStreamReader& stream);
    bool readElementEnd(QXmlStreamReader& stream);
    bool readProperties(QXmlStreamReader& stream);

  private:
    Serializer();

    QString  elementName;
    int      propertyOffset;
    QObject* obj;
  };
}
#endif // GRAPHSERIALIZER_H
