@echo off

set RootDir=.
set BinHost=windows

@rem If we're in the wrong dir, choke with a useful message.
if not exist %RootDir%\install-gui.bat goto enobat

set OPATH=%PATH%
set PATH=%RootDir%\utils\H-%BinHost%\bin;%OPATH%

@rem If we run a cygwin application, ignore dll conflicts
set CYGWIN_MISMATCH_OK=1
set CYGWIN_TESTING=1
set CYGWIN=nontsec

@rem Go go gaget installer
cd %RootDir%
wish84 install.tcl

@rem unset vars
set PATH=%OPATH%
set RootDir=
set BinHost=
goto fini

:enobat
echo You must first cd to the location of this .bat file before running.
pause
goto fini

:fini
