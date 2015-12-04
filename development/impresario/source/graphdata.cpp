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

#include "graphdata.h"
#include "graphelements.h"
#include "graphitems.h"
#include <QMetaObject>
#include <QMetaProperty>

namespace graph
{
  //-----------------------------------------------------------------------
  // Class BaseData
  //-----------------------------------------------------------------------
  BaseData::BaseData() : QObject(0), Serializer("data",BaseData::staticMetaObject.propertyOffset(),this)
  {
  }

  QSharedPointer<BaseItem> BaseData::createVisualization(BaseElement& /*elementRef*/, BaseItem* /*parent*/)
  {
    return BaseItem::Ptr();
  }

  //-----------------------------------------------------------------------
  // Class VertexData
  //-----------------------------------------------------------------------

  PinData::Ptr VertexData::addPinData(const QMetaObject& pinTypeInfo, QString id, Defines::PinDirectionType direction)
  {
    if (pins.contains(id))
    {
      return PinData::Ptr();
    }
    QObject* obj = pinTypeInfo.newInstance(Q_ARG(QString,id),Q_ARG(Defines::PinDirectionType,direction));
    if (obj != 0)
    {
      PinData::Ptr pinPtr = PinData::Ptr(qobject_cast<PinData*>(obj));
      pins.insert(id,pinPtr);
      emit pinDataAdded(pinPtr);
      return pinPtr;
    }
    else
    {
      delete obj;
      return PinData::Ptr();
    }
  }

  bool VertexData::addPinData(PinData::Ptr pinType)
  {
    if (pinType.isNull() || pins.contains(pinType->id()))
    {
      return false;
    }
    pins.insert(pinType->id(),pinType);
    emit pinDataAdded(pinType);
    return true;
  }

  bool VertexData::removePinData(PinData::Ptr pinType)
  {
    if (pinType.isNull())
    {
      return false;
    }
    if (pins.contains(pinType->id()) && pins.remove(pinType->id()))
    {
      emit pinDataRemoved(pinType);
      return true;
    }
    return false;
  }

}
