@echo off

mkdir build
copy ".\external\lib\*.dll"  ".\build\"

:: set enviroment vars and requred stuff for the msvc compiler
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

:: Set paths for include directories, libraries, and output directories
set INCLUDE_DIRS=/I..\external\inc\ /I..\external\inc\IMGUI\ /I..\external\inc\freetype\ /I..\inc\
set LIBRARY_DIRS=/LIBPATH:..\external\lib\
set LIBRARIES=opengl32.lib glfw3.lib glew32.lib freetype.lib libpng16.lib zlib.lib brotlidec.lib brotlicommon.lib bz2.lib user32.lib gdi32.lib shell32.lib kernel32.lib
set SRC_FILES=..\main.cpp ..\external\src\glad.c ..\external\src\IMGUI\*.cpp ..\src\Shaders.cpp
set C_FLAGS=/Zi /EHsc /O2 /W4 /MD /nologo /std:c17 /D_WIN64
set L_FLAGS=/SUBSYSTEM:WINDOWS

pushd .\build
cl  %C_FLAGS% %INCLUDE_DIRS% %SRC_FILES% /link %LIBRARY_DIRS% %LIBRARIES% %LFLAGS%
.\main.exe
popd
