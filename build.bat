echo off

::
:: Init MSVC environment
::

if not defined VCINSTALLDIR (
  echo Setting up MSVC environment...
  call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
    echo MSVC environment already set.
)

::
:: Set Variables
::

set ReleaseBuild="%1"=="RELEASE"
set DebugBuild=not "%1"=="RELEASE"

set IncludePath="..\deps"
set FilesToBuild=..\src\win32_editor.cpp
set ExeName=editor.exe

::
:: Compiler flags
::

:: /fixed нужно убрать для .DLL
@REM set CommonFlags=/nologo /Gm- /GR- /GS- /Oi /Gs9999999 /EHa-
@REM set CommonLinkerFlags=/incremental:no /fixed /opt:ref /opt:icf /stack:0x100000,0x100000 /nodefaultlib kernel32.lib User32.lib Gdi32.lib opengl32.lib Comdlg32.lib

:: компилируем с libvcruntime.lib, так как imgui требует memcpy и тд. если убрать imgui, можно компилировать без него (и вернуться к /nodefaultlib?)
set CommonFlags=/nologo /Gm- /GR- /GS- /Oi /EHa-
set CommonLinkerFlags=/ENTRY:main /incremental:no /fixed /opt:ref /opt:icf kernel32.lib User32.lib Gdi32.lib opengl32.lib Comdlg32.lib libvcruntime.lib 

if %ReleaseBuild% (
  echo Release build
  set CompilerFlags=%CommonFlags% /O2
  set LinkerFlags=%CommonLinkerFlags% /subsystem:windows "..\deps\imgui\imguidll_r.lib"
) 
if %DebugBuild% (
  echo Debug build
  set CompilerFlags=%CommonFlags% /Z7 /Zo /Od /D_DEBUG 
  set LinkerFlags=%CommonLinkerFlags% /subsystem:console "..\deps\imgui\imguidll_d.lib"
)


set RobocopyFlags=/MIR /nfl /ndl /njh /njs

::
:: Create Directories
::

if not exist "%cd%\TextEditor\build" mkdir "%cd%\TextEditor\build"
pushd "%cd%\TextEditor\build"

echo =========== Compilation Started ===========

::
:: Compile
::

if %ReleaseBuild% (  
  if not exist "%cd%\release" mkdir "%cd%\release"
  
  cl.exe /I%IncludePath% /Ferelease\%ExeName% /Forelease\ %CompilerFlags% %IncludeDirs% %FilesToBuild% -link %LinkerFlags%

  if errorlevel 1 goto error

  echo =========== Compilation Done ===========

  robocopy ..\res .\release\res %RobocopyFlags%
  copy ..\deps\imgui\imguidll_r.dll .\release\
)
if %DebugBuild% (  
  if not exist "%cd%\debug" mkdir "%cd%\debug"
  
  cl.exe /I%IncludePath% /Fedebug\%ExeName% /Fodebug\ %CompilerFlags% %IncludeDirs% %FilesToBuild% -link %LinkerFlags% /DEBUG
  
  if errorlevel 1 goto error

  echo =========== Compilation Done ===========

  robocopy ..\res .\debug\res %RobocopyFlags%
  copy ..\deps\imgui\imguidll_d.dll .\debug\
)
goto end

:error
echo Error on compile
popd
exit /b 1

:end
popd
:: RUN
@REM echo ==================================
@REM call "%cd%\TextEditor\build\debug\editor.exe"
@REM endlocal