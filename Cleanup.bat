@echo off
//del /q "Builds\*.exe" 2>nul:
//del /q "Builds\*.txt" 2>nul:
//del /q "Visual Studio\Envy.sdf" 2>nul:
//rd /s /q "Visual Studio\ipch\" 2>nul:
//for /r %%i in (*.aps) do del /q "%%i" && echo Cleaning %%i
//for /r %%i in (*.ncb) do del /q "%%i" && echo Cleaning %%i
//for /r %%i in (*.sdf) do del /q "%%i" && echo Cleaning %%i
for /r %%i in (*.user) do del /q "%%i" && echo Cleaning %%i
for /r %%i in (*_i.c) do del /q "%%i" && echo Cleaning %%i
for /d /r %%i in (*.*) do if exist "%%i\Release Win32\" rd /s /q "%%i\Release Win32\" && echo Cleaning %%i\Release Win32\
for /d /r %%i in (*.*) do if exist "%%i\Release x64\" rd /s /q "%%i\Release x64\" && echo Cleaning %%i\Release x64\
for /d /r %%i in (*.*) do if exist "%%i\Debug Win32\" rd /s /q "%%i\Debug Win32\" && echo Cleaning %%i\Debug Win32\
for /d /r %%i in (*.*) do if exist "%%i\Debug x64\" rd /s /q "%%i\Debug x64\" && echo Cleaning %%i\Debug x64\
pause
