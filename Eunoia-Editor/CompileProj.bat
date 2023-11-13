@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" %1 -p:Platform=x64 -p:Configuration=%2%