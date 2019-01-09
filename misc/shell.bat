@echo off
subst M: "E:\Spade"
call "D:\Visual Studio\VC\Auxiliary\Build\vcvars64" x64
set path=M:\misc;M:\code\lib;M:\code\lib\fbxsdk\lib;%path%
M:
cd code
call devenv
START "VSCode" "E:\VSCode\Microsoft VS Code\Code.exe" "M:\misc\spade.code-workspace"
