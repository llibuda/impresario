/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2019  Lars Libuda
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
#ifndef GRAPHDEFINES_H
#define GRAPHDEFINES_H
#include <QObject>

namespace graph
{
  class Defines : public QObject
  {
    Q_OBJECT
    Q_ENUMS(LayoutDirectionType)
    Q_ENUMS(PinDirectionType)
    Q_ENUMS(ItemStateType)
  public:

    enum LayoutDirectionType
    {
      TopToBottom,
      LeftToRight,
      RightToLeft
    };

    enum PinDirectionType
    {
      Incoming,
      NonDirectional,
      Outgoing,
      Undefined
    };

    enum ItemStateType
    {
      Normal,
      Valid,
      Invalid,
      Selected,
      Focused
    };
  };

}

#endif // GRAPHDEFINES_H
