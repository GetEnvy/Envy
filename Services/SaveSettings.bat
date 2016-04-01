@echo off
rem    This batch file saves Envy settings from the registry
rem    to files EnvySave1.reg and EnvySave2.reg

regedit /ea EnvySave1.reg HKEY_CURRENT_USER\Software\Envy
regedit /ea EnvySave2.reg HKEY_LOCAL_MACHINE\Software\Envy
