@echo off
subst M: /d
subst M: "E:\Spade"
call "E:\VS2019\VC\Auxiliary\Build\vcvars64" x64
set path=M:\misc;M:\code\lib;%path%
M:
REM cd code
call devenv
REM START "VSCode" "E:\VSCode\Microsoft VS Code\Code.exe" "M:\misc\spade.code-workspace"
REM START "GitKraken" "C:\Users\Evan\AppData\Local\gitkraken\app-6.4.1\GitKraken.exe"
REM START "Explorer" "M:\build\assets"

