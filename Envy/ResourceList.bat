@echo off
for /f "tokens=2 skip=12 eol=/" %%a in (Resource.h) do ( echo %%a && echo:>>ResourceList.txt && echo %%a:>>ResourceList.txt && findstr /S /L /M /C:"%%a" "*.cpp" >> "ResourceList.txt" )
echo For Unused Do RegExp Replace ".+?^(ID[^:]+): \r\n "  "$1\r\n"
pause