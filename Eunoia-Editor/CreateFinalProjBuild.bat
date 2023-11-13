@echo off
set ProjectName=%1%
set ProjectDir=../EunoiaProjects/%ProjectName%
set BuildPath=../EunoiaProjects/%ProjectName%/FinalBuild
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
robocopy ../Eunoia-Engine/Res %BuildPath%/Res /E
robocopy ../EunoiaProjects/%ProjectName%/Assets ../EunoiaProjects/%ProjectName%/FinalBuild/Assets /E
call "../Bin/Dist-windows-x86_64/Eunoia-Introspection/Eunoia-Introspection.exe" ../EunoiaProjects/%ProjectName%/Src/%ProjectName%.h ../EunoiaProjects/%ProjectName%/Src/%ProjectName%_Generated.h
mkdir "%BuildPath%/Bin"
cl /Fo%BuildPath%/Bin/ /Fe%BuildPath%/%ProjectName% /O2 /Ob3 /Ot /GL /MT /EHsc /DEU_PROJ="" /DEU_DIST /DEU_PROJECT /DEU_FINAL_PROJECT_BUILD /DEU_PLATFORM_WINDOWS /I"../Eunoia-Engine/Src" /I"../Eunoia-Engine/Libs/Vulkan/Include" /I"../Eunoia-Engine/Libs/Bullet/Include" %ProjectDir%/Src/*.cpp %ProjectDir%/Src/Systems/*.cpp %BuildPath%/TempMain.cpp gdi32.lib user32.lib ../Bin/Dist-windows-x86_64/Eunoia-Engine/Eunoia-Engine.lib 