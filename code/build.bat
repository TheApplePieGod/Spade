@echo off
cls
REM Compiler Options: https://msdn.microsoft.com/en-us/library/fwkeyyhe.aspx

REM setting SPADE_3D to 0 disables 3d only features, drastically improving performance
set ConstantsAndMacros=-DSPADE_DEBUG=1 -DSPADE_WIN32=1 -DSPADE_3D=1
set DisabledWarnings=-wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -wd4996 -Wv:18
set TempDisabledWarnings=-wd4244 -wd4267 -wd4701 -wd4702
REM Include -WX to turn warnings back on.
set CommonCompilerFlags=-Od -Oi -MTd -FC -Z7 -Zo -fp:fast -fp:except- -Gm- -GR -EHa- -EHsc -W4 -nologo %DisabledWarnings% %ConstantsAndMacros%
set CommonLinkerFlags= -incremental:no -opt:ref 
REM set LibsToLink=user32.lib gdi32.lib Advapi32.lib winmm.lib dsound.lib Ws2_32.lib dxgi.lib d3d11.lib D3dcompiler.lib  
set LibsToLink=user32.lib gdi32.lib Advapi32.lib winmm.lib dsound.lib dxgi.lib d3d11.lib D3dcompiler.lib Xaudio2.lib Ole32.lib
set Box2DLibsToLink=m:\code\lib\Box2D\obj\*.obj
REM set SpineLibsToLink=m:\code\lib\spine-c\obj\*.obj
set FBXLibsToLink= M:\code\lib\fbxsdk\lib\vs2015\x64\release\*.lib
REM set PHYSXLibsToLink= M:\code\lib\physx\Lib\vc15win64\DEBUG\*.lib

REM set DLLFunctionExports= -EXPORT:EngineUpdate -EXPORT:EngineRender -EXPORT:EngineInitializationAndSetup -EXPORT:OutputDebugInfo
REM set AssetPackerIncludeDirs= /I M:\code\lib\fbxsdk\include /I m:\code\src\platform /I m:\code\lib\json /I m:\code\lib\stb
set IncludeDirs= /I m:\code\lib\spine-c\include /I m:\code\lib /I m:\code\lib\Box2D /I m:\code\lib\stb /I m:\code\src /I m:\code\lib\physx\Include /I m:\code\lib\physx\Lib
REM set Win32IncludeDirs= /I m:\code\src\platform /I m:\code\lib
set Box2DCPPFiles= m:\code\lib\Box2D\Collision\*.cpp m:\code\lib\Box2D\Collision\Shapes\*.cpp m:\code\lib\Box2D\Common\*.cpp m:\code\lib\Box2D\Dynamics\*.cpp m:\code\lib\Box2D\Dynamics\Contacts\*.cpp m:\code\lib\Box2D\Dynamics\Joints\*.cpp m:\code\lib\Box2D\Rope\*.cpp

REM ---------
REM Manage build directory and make sure we are in it before proceeding.
IF NOT EXIST m:\build mkdir m:\build
pushd build

REM ---------
REM Remove all .pdb files
del *.pdb > NUL 2> NUL

REM Build the asset packer
REM cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS /link %CommonLinkerFlags% %FBXLibsToLink% REM M:\code\src\asset_packer\asset_packer.cpp -Fmasset_packer.map 

REM ---------
REM Create a lock file in order to support hot-reload of the game DLL
REM echo WAITING FOR PDB > lock.tmp

REM ---------
REM Build the Game DLL - can be hot-reloaded.
REM This version of cl assumes the obj files for both Spine and Box2D have already been created and are in the lib folder
REM cl %CommonCompilerFlags% %IncludeDirs% M:\code\src\engine\catalina.cpp -Fmcatalina.map -LD /link %CommonLinkerFlags%  %Box2DLibsToLink% %SpineLibsToLink% -PDB:catalina_%random%.pdb %DLLFunctionExports%
REM cl %CommonCompilerFlags% %IncludeDirs% M:\code\src\engine\catalina.cpp -Fmcatalina.map -LD /link %CommonLinkerFlags%  %PHYSXLibsToLink% -PDB:catalina_%random%.pdb %DLLFunctionExports%

REM Run this CL if we need to recompile all of the obj files for the Spine and Box2d Libraries
REM cl %CommonCompilerFlags% %TempDisabledWarnings% %IncludeDirs% %Win32IncludeDirs% /I M:\code\lib\fbxsdk\include M:\code\src\windows\spade.cpp %Box2DCPPFiles% -Fmtest.map -LD /link %CommonLinkerFlags%

REM --------
REM Remove the lock file to the game is clear to reload the game DLL.
REM del lock.tmp

REM --------
REM Build the WIN32 GAME EXE
REM During hot-reload, we will always get a compile fail because the exe is in use. This is fine because
REM ...hot-reload only supports reloading the game DLL.

REM Full rebuild
REM cl %CommonCompilerFlags% /I M:\code\lib\fbxsdk\include /I M:\code\lib\Box2D %IncludeDirs% %Box2DCPPFiles% %Win32IncludeDirs% M:\code\src\windows\spade.cpp -Fmtest.map /link %CommonLinkerFlags% %LibsToLink% %FBXLibsToLink%
cl %CommonCompilerFlags% /I M:\code\lib\fbxsdk\include /I M:\code\lib\Box2D %IncludeDirs% %Win32IncludeDirs% M:\code\src\windows\spade.cpp -Fmtest.map /link %CommonLinkerFlags% %LibsToLink% %FBXLibsToLink% %Box2DLibsToLink%
popd