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
#ifndef BUILDINFO_H
#define BUILDINFO_H

#include <QObject>
#include <QString>

namespace app {
  /**
   * Class providing build information about 
   * <span class="impresario">Impresario</span>.
   * Build information include among others:
   * Used compiler, version number, etc.
   * These information are visualized in the "About" dialog (\ref dlgAbout::dlgAbout).
   */
  class BuildInfo : public QObject
  {
  public:
    /**
     * Access the build information of this program.
     * \return Constant reference to the only existing instance of this class.
     */
    static const BuildInfo& instance()
    {
      return buildInfo;
    }

    /**
     * Get the date of this build.
     * \return <code>QString</code> containing the date of the build.
     */
    const QString buildDate() const;

    /**
     * Get the used compiler for this build.
     * \return <code>QString</code> containing the compiler name.
     */
    const QString compiler() const;

    /**
     * Get the compiler ID as an integer which is better suited
     * for comparison.
     * \return Compiler ID as positive value. 0 if the compiler is not known.
     */
    unsigned int compilerId() const;

    /**
     * Check if this build includes debug information.
     * \return <code>true</code> if this is a debug build, 
     *         <code>false</code> otherwise.
     */
    bool isDebugVersion() const;

    /**
     * Check if this build is 64bit build.
     * \return <code>true</code> if this is a 64bit build,
     *         <code>false</code> if this is a 32bit build.
     */
    bool is64Bit() const;

    /**
     * Get a string describing the build derived from id.
     * \param buildId Number encoding the build (Architecture, used compiler, compiler version).
     * \param isDebug Boolean indicating whether it is a debug build.
     * \return <code>QString</code> containing the build description.
     */
    const QString buildString(unsigned int buildId, bool isDebug) const;

    /**
     * Get the version number of this program in a string.
     * \return <code>QString</code> containing the program version.
     */
    const QString versionString() const;

  protected:
    /**
     * Standard constructor. This constructor is protected because only one
     * instance of this class may exist at runtime. This instance is created
     * automatically at program start.
     */
    BuildInfo(void)
    {
    }

    /**
     * Standard destructor. This destructor is protected because only one
     * instance of this class may exist at runtime. This instance is created
     * automatically at program start.
     */
    virtual ~BuildInfo(void)
    {
    }

  private:
    /** 
     * The only instance of this class in the entire program, created at
     * program start.
     */
    static const BuildInfo buildInfo;
  };

}

#endif /* BUILDINFO_H */
