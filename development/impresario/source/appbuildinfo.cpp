/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2017  Lars Libuda
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

#include "appbuildinfo.h"
#include "version.h"

namespace app
{
  const BuildInfo BuildInfo::buildInfo;

  const QString BuildInfo::buildDate() const
  {
    return QString(__DATE__);
  }

  const QString BuildInfo::compiler() const
  {
#if defined(__GNUC__)
    // GCC
    return QString("GCC %1.%2.%3 (%4)").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__).arg((is64Bit() ? "64bit" : "32bit"));
#elif defined(_MSC_VER)
    // Microsoft
    QString compiler = QString("Microsoft C/C++ %1.%2 %3").arg(_MSC_VER / 100).arg(_MSC_VER % 100).arg((is64Bit() ? "64bit" : "32bit"));
#if _MSC_VER == 1300
  compiler.append(" (Visual C++ 2002)");
#elif _MSC_VER == 1310
  compiler.append(" (Visual C++ 2003)");
#elif _MSC_VER == 1400
  compiler.append(" (Visual C++ 2005)");
#elif _MSC_VER == 1500
  compiler.append(" (Visual C++ 2008)");
#elif _MSC_VER == 1600
  compiler.append(" (Visual C++ 2010)");
#elif _MSC_VER == 1700
  compiler.append(" (Visual C++ 2012)");
#elif _MSC_VER == 1800
  compiler.append(" (Visual C++ 2013)");
#elif _MSC_VER == 1900
  compiler.append(" (Visual C++ 2015)");
#endif
    return compiler;
#else
    // no supported compiler
    return QString("Not recognized");
#endif
  }

  unsigned int BuildInfo::compilerId() const
  {
#if defined(__GNUC__)
    // GCC
    return ((is64Bit() ? 641 : 321)) * 1000000 + __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__;
#elif defined(_MSC_VER)
    return ((is64Bit() ? 642 : 322)) * 1000000 + (_MSC_VER / 100) * 10000 + (_MSC_VER % 100) * 100;
#else
    // no supported compiler
    return 0;
#endif
  }

  bool BuildInfo::isDebugVersion() const
  {
#if defined(QT_NO_DEBUG)
    return false;
#else
    return true;
#endif
  }

  bool BuildInfo::is64Bit() const
  {
    if(sizeof (void *) * CHAR_BIT == 64)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  const QString BuildInfo::buildString(unsigned int buildId, bool isDebug) const
  {
    if (buildId == 0)
    {
      if (isDebug)
      {
        return tr("Unsupported (Debug)");
      }
      else
      {
        return tr("Unsupported");
      }
    }
    QString architectureInfo;
    unsigned int architecture = buildId / 1000000;
    if (architecture > 600)
    {
      architectureInfo = "64bit";
    }
    else
    {
      architectureInfo = "32bit";
    }
    unsigned int compiler = architecture % 10;
    QString buildInfo;
    unsigned int major = buildId % 1000000 / 10000;
    unsigned int minor = buildId % 10000 / 100;
    unsigned int patch = buildId % 100;
    switch(compiler)
    {
      case 1:
        buildInfo = QString("GCC %1.%2.%3 %4 ").arg(major).arg(minor).arg(patch).arg(architectureInfo);
        break;
      case 2:
        buildInfo = QString("MSC %1.%2 %3 ").arg(major).arg(minor).arg(architectureInfo);
        break;
      default:
        break;
    }
    if (isDebug)
    {
      buildInfo.append(tr("Debug"));
    }
    else
    {
      buildInfo.append(tr("Release"));
    }
    return buildInfo;
  }

  const QString BuildInfo::versionString() const
  {
    return QString("%1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_PATCH);
  }
}

