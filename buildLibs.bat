echo off
if not defined VCINSTALLDIR (
  echo Setting up MSVC environment...
  call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
    echo MSVC environment already set.
)

set ProjectNameR=imguidll_r
set ProjectNameD=imguidll_d
set FilesToBuild=src\*.cpp
set OutDir=build
set OutDirR=%OutDir%\\release
set OutDirD=%OutDir%\\debug

pushd "%cd%\imguidll"

:: Release
if not exist "%OutDirR%" mkdir "%OutDirR%"
cl.exe /c /MP /EHsc /MD /O2 /Fo"%OutDirR%\\" %FilesToBuild%
link /DLL /OUT:"%OutDirR%\\%ProjectNameR%.dll" /IMPLIB:"%OutDirR%\\%ProjectNameR%.lib" "%OutDirR%\\*.obj"
if errorlevel 1 goto error
copy %OutDirR%\%ProjectNameR%.lib ..\TextEditor\deps\imgui\
copy %OutDirR%\%ProjectNameR%.dll ..\TextEditor\deps\imgui\

:: Debug
if not exist "%OutDirD%" mkdir "%OutDirD%"
cl.exe /c /MP /EHsc /MDd /Zo /Od /Oi /Fo"%OutDirD%\\" %FilesToBuild%
link /DLL /OUT:"%OutDirD%\\%ProjectNameD%.dll" /IMPLIB:"%OutDirD%\\%ProjectNameD%.lib" "%OutDirD%\\*.obj"
if errorlevel 1 goto error
copy %OutDirD%\%ProjectNameD%.lib ..\TextEditor\deps\imgui\
copy %OutDirD%\%ProjectNameD%.dll ..\TextEditor\deps\imgui\

goto end

:error
echo Error on compile
popd
exit /b 1

:end
popd