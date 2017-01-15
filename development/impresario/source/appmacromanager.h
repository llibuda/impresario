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
#ifndef APPMACROMANAGER_H
#define APPMACROMANAGER_H

#include "graphmain.h"
#include "appmacrolibrary.h"
#include "appmacro.h"
#include <QList>
#include <QStringList>
#include <QMultiMap>

namespace app
{

  class MacroManager : public graph::ElementManager
  {
    Q_OBJECT
  public:
    static MacroManager& instance()
    {
      return macroManager;
    }

    void loadPrototypes(const QStringList& dirs);
    void unloadPrototypes();

    bool registerMacroViewer(MacroViewer::Ptr viewer);
    MacroViewer::Ptr createMacroViewerInstance(const QString& dataType);

    bool hasMacroViewer(const QString& dataType) const
    {
      return viewers.contains(dataType);
    }

    void iterateViewerTypes(VertexDataTypeIterator* iterator, ... ) const;

  protected:
    virtual QString matchVertexSignature(const VertexDataTypeMap& vertexDataTypes, const QString& signature);

  private:
    Q_DISABLE_COPY(MacroManager)

    MacroManager();
    virtual ~MacroManager();

    static MacroManager macroManager;

    typedef QList<MacroLibrary*> LibraryList;
    typedef QMultiMap<QString, MacroViewer::Ptr> ViewerMap;

    LibraryList libList;
    ViewerMap   viewers;
  };

}
#endif // APPMACROMANAGER_H
