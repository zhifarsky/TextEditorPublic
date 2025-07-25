echo off
if not defined VCINSTALLDIR (
  echo Setting up MSVC environment...
  call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
    echo MSVC environment already set.
)

if "%1"=="RELEASE" (
  set LINKER_LIBS="..\deps\imgui\imguidll_r.lib"
)
if not "%1"=="RELEASE" (
  set LINKER_LIBS="..\deps\imgui\imguidll_d.lib"
)
set INCLUDE_PATH="..\deps"

set FilesToBuild=..\src\*.cpp

set EXE_NAME=editor.exe

::flags for compilation process
:: /Gm - minimal rebuild. Если часть файлов не рекомпилируется в нужный момент -  поставить /Gm-
set CompilerFlags=/nologo /Gm- /GR- /GS- /Zo /Od /Oi /Gs9999999 /EHa-
set CompilerFlagsRelease=/nologo /Gm- /GR- /GS- /O2 /Oi /Gs9999999 /EHa-

set LinkerFlags=/subsystem:windows /nodefaultlib %LINKER_LIBS% kernel32.lib User32.lib Gdi32.lib opengl32.lib Comdlg32.lib /stack:0x100000,0x100000

::create directories
mkdir "%cd%\TextEditor\build"
pushd "%cd%\TextEditor\build"

::run compiler
:: RELEASE build
if "%1"=="RELEASE" (
  mkdir "%cd%\release"
  cl.exe /I%INCLUDE_PATH% /Ferelease\%EXE_NAME% /Forelease\ %CompilerFlagsRelease% %IncludeDirs% %FilesToBuild% -link %LinkerFlags%

  if errorlevel 1 goto error

  robocopy ..\res .\release\res /MIR
  copy ..\deps\imgui\imguidll_r.dll .\release\
)
:: DEBUG build
if not "%1"=="RELEASE" (
  mkdir "%cd%\debug"
  cl.exe /I%INCLUDE_PATH% /Fedebug\%EXE_NAME% /Fodebug\ %CompilerFlags% %IncludeDirs% /Zi %FilesToBuild% -link %LinkerFlags% /DEBUG
  @REM cl.exe /Feeditor.exe %CompilerFlags% %IncludeDirs% /Zi %FilesToBuild% /link /LIBPATH:lib %LinkerFlags% imgui.lib /DEBUG  
  
  if errorlevel 1 goto error

  robocopy ..\res .\debug\res /MIR
  copy ..\deps\imgui\imguidll_d.dll .\debug\
)
goto end

:error
echo Error on compile
popd
exit /b 1

:end
popd
@REM endlocal