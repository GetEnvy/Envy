@echo off
rem    This batch file restores Envy settings from previously saved
rem    files EnvySave1.reg and EnvySave2.reg to the registry

if not exist EnvySave?.reg exit

echo REGEDIT4> "%TEMP%\EnvyDelOld.reg"
echo [-HKEY_CURRENT_USER\Software\Envy]>> "%TEMP%\EnvyDelOld.reg"
echo [-HKEY_LOCAL_MACHINE\Software\Envy]>> "%TEMP%\EnvyDelOld.reg"

start/wait regedit -s "%TEMP%\EnvyDelOld.reg"
del "%TEMP%\EnvyDelOld.reg" > nul

:import
start/wait regedit -s EnvySave1.reg EnvySave2.reg
