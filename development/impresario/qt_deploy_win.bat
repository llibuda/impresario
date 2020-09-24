@echo off
@echo Qt Windows Deployment script

setlocal EnableDelayedExpansion

REM check script path
For %%A in ("%0") do (
  Set SCRIPTDIR=%%~dpA
  Set SCRIPTNAME=%%~nxA
)
@echo Script directory is %SCRIPTDIR%.

set WINDEPLOYTOOL=%QTDIR%\bin\windeployqt.exe

REM check Qt path and windeploy tool
if not exist "%WINDEPLOYTOOL%" (
  @echo Cannot find path to windeployqt.exe. Looked in %WINDEPLOYTOOL%.
  exit 1
)

set TARGETFILE=%1
set TARGETNAME=""
set TARGETDIR=""

REM check target file
if not exist "%TARGETFILE%" (
  @echo Target file %TARGETFILE% does not exist.
  exit 1
)
For %%A in ("%TARGETFILE%") do (
  Set TARGETDIR=%%~dpA
  Set TARGETNAME=%%~nxA
)
REM @echo Folder is: %TARGETDIR%
REM @echo Name is: %TARGETNAME%

REM create standard directories for process graphs and macro libraries
If not exist "%TARGETDIR%..\lib" (
  mkdir %TARGETDIR%..\lib
)
If not exist "%TARGETDIR%..\processgraphs" (
  mkdir %TARGETDIR%..\processgraphs
)

REM delete old subdirectories in case they exist
If exist "%TARGETDIR%qtweb" (
  rmdir %TARGETDIR%qtweb /S /Q
)
If exist "%TARGETDIR%qtqml" (
  rmdir %TARGETDIR%qtqml /S /Q
)
If exist "%TARGETDIR%qtplugins" (
  rmdir %TARGETDIR%qtplugins /S /Q
)

REM call windeployqt to get required Qt components
REM "%WINDEPLOYTOOL%" "%TARGETFILE%" --plugindir="%TARGETDIR%qtplugins" --no-quick-import
"%WINDEPLOYTOOL%" "%TARGETFILE%" --plugindir="%TARGETDIR%plugins" --qmldir="%TARGETDIR%..\resources"

REM move all QML directories to subdirectory qml
mkdir %TARGETDIR%qml
FOR /f %%d in ('dir %TARGETDIR%Qt* /A:D /B') DO (
  move %TARGETDIR%%%d %TARGETDIR%qml
)

REM rename and reorder some files and directories
rename %TARGETDIR%qml qtqml
rename %TARGETDIR%plugins qtplugins

move %TARGETDIR%QtWebEngineProcess*.exe %TARGETDIR%resources
move %TARGETDIR%translations\qtwebengine_locales %TARGETDIR%resources
rename %TARGETDIR%resources qtweb
rmdir %TARGETDIR%translations /S /Q

REM copy files from misc directory
copy %SCRIPTDIR%misc\Impresario.xml %TARGETDIR% /Y

REM copy QML components
@echo Copying QML Components...
xcopy %QTDIR%\qml\* %TARGETDIR%qtqml /S /Y /I /Q
REM in release build, remove debug files from QML directory
if not x%TARGETDIR:release=%==x%TARGETDIR% (
  @echo Release version. Cleaning up debug files of QML Components...
  del %TARGETDIR%qtqml\*.pdb /S /Q
  set n=2
  set i=0
  FOR /f %%a IN ('dir %TARGETDIR%qtqml\*.dll /S /B') DO (
    set /A i+=1, iMODn=i %% n
    if !iMODn! equ 0 del "%%a"
  )
)