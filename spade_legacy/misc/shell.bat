@echo off
subst M: /d
subst M: "E:\Spade"
call "E:\VS2019\VC\Auxiliary\Build\vcvars64" x64
set path=M:\misc;M:\code\lib;M:\code\lib\fbxsdk\lib;%path%
M:
cd code
call devenv
START "VSCode" "E:\VSCode\Microsoft VS Code\Code.exe" "M:\misc\spade.code-workspace"
START "GitKraken" "C:\Users\Evan\AppData\Local\gitkraken\app-6.4.1\GitKraken.exe"
REM START "Explorer" "M:\build\assets"
