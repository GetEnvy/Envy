@echo off
@setLocal EnableExtensions EnableDelayedExpansion
@echo off

set "header1=Microsoft Visual Studio Solution File, Format Version 12.00"
set "header2=# Visual Studio 2012"
set "version=v110"
set "versionxp=v110_xp"

(
echo %header1%
echo %header2%

for /f "skip=2 delims=*" %%a in (Envy.sln) do (
echo %%a
)
)>Envy.sln.temp

xcopy Envy.sln.temp Envy.sln /y
del Envy.sln.temp /f /q

cd ..\

set counter=0

for /r %%n in (*.vcxproj) do (
  echo %%n
  set update=0
  set newversion=%versionxp%
  if exist %%n.temp del /f %%n.temp

(
  for /f "delims=" %%l in (%%n) do (
    set "linetest=%%l"

    if "!linetest:~2,24!"=="<PropertyGroup Condition" (
      if "!linetest:~61,13!"=="Release|Win32" (set newversion=%versionxp%
      ) else if "!linetest:~61,11!"=="Debug|Win32" (set newversion=%versionxp%
      ) else if "!linetest:~61,11!"=="Release|x64" (set newversion=%version%
      ) else if "!linetest:~61,9!"=="Debug|x64" (set newversion=%version%)
    )

    if "!linetest:~5,15!"=="PlatformToolset" (
      if not "!linetest:~21,4!"=="%version%" (set /a update+=1 && set /a counter+=1) && echo     ^<PlatformToolset^>!newversion!^</PlatformToolset^>
    ) else (setLocal DisableDelayedExpansion && echo %%l&& endlocal)
  )
)>%%n.temp

  if !update! neq 0 (move /y %%n.temp %%n
  ) else (del %%n.temp && echo         No change.)
)

echo.
echo %counter% changes to PlatformToolset %version%.
echo.
endlocal
pause
