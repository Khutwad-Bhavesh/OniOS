@echo off
REM DOS Games Archive DOSBox launch script
REM 
REM Game title: DOOM (Shareware 1.9)
REM Configuration: standard
REM 
config -set cpu "cycles=auto 6075 98%% limit 33445"
config -set render aspect=true
cd DOOMS
loadfix -25
DOOM.EXE
PAUSE >NUL
echo.
loadfix -f
cd..
config -set cpu "cycles=auto 6075 100%% limit 23880"
cls
echo Thank you for playing DOOM Shareware!
echo.
echo To play again, type DOOM and press Enter
echo.
echo Your DOS Games Archive http://www.dosgamesarchive.com/
@echo on