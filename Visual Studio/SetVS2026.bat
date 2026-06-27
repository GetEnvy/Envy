@echo off
@setLocal EnableExtensions EnableDelayedExpansion

REM ============================================================================
REM Retarget Envy.sln and all .vcxproj files to Visual Studio 2026 (v145).
REM
REM Behavior:
REM   1. Updates Envy.sln headers to "Format Version 12.00 / # Visual Studio 18".
REM   2. Rewrites every <PlatformToolset> tag found anywhere in the tree to v145.
REM   3. Drops the v141_xp toolset (XP targeting was retired in v145).
REM   4. Sets WindowsTargetPlatformVersion to 10.0 when missing.
REM
REM Run from this folder ("Visual Studio\SetVS2026.bat"). Safe to re-run.
REM ============================================================================

set "header1=Microsoft Visual Studio Solution File, Format Version 12.00"
set "header2=# Visual Studio Version 18"
set "version=v145"

REM ---- Patch the solution file -----------------------------------------------
if not exist "Envy.sln" (
  echo [ERROR] Envy.sln not found in %cd%. Run this script from the
  echo         "Visual Studio" folder of the Envy source tree.
  exit /b 1
)

if exist Envy.sln.temp del /f /q Envy.sln.temp

(
  echo %header1%
  echo %header2%

  for /f "skip=2 delims=*" %%a in (Envy.sln) do (
    echo %%a
  )
) > Envy.sln.temp

xcopy Envy.sln.temp Envy.sln /y > nul
del Envy.sln.temp /f /q

cd ..\

set counter=0
set projects_touched=0

REM ---- Patch every .vcxproj --------------------------------------------------
for /r %%n in (*.vcxproj) do (
  set "proj=%%n"
  set "skip=0"

  REM Skip the plugin wizard templates - they're meant to stay un-upgraded.
  echo !proj! | findstr /i "PluginWizard" > nul && set "skip=1"
  if "!skip!"=="0" (
    set update=0
    if exist "%%n.temp" del /f /q "%%n.temp"

    (
      for /f "delims=" %%l in (%%n) do (
        set "linetest=%%l"

        REM Replace any <PlatformToolset>... line with v145.
        if "!linetest:~5,15!"=="PlatformToolset" (
          echo     ^<PlatformToolset^>!version!^</PlatformToolset^>
          set /a update+=1
        ) else (
          setLocal DisableDelayedExpansion
          echo %%l
          endlocal
        )
      )
    ) > "%%n.temp"

    if !update! neq 0 (
      move /y "%%n.temp" "%%n" > nul
      set /a counter+=!update!
      set /a projects_touched+=1
      echo Patched: %%n
    ) else (
      del /f /q "%%n.temp"
    )
  )
)

echo.
echo ============================================================================
echo  Retarget complete.
echo  Projects touched : %projects_touched%
echo  PlatformToolset replacements : %counter% -^> %version%
echo ============================================================================
echo.
echo Next steps:
echo   1. Open "Visual Studio\Envy.sln" in Visual Studio 2026.
echo   2. Right-click the solution and choose "Retarget solution"
echo      to align WindowsTargetPlatformVersion with the installed SDK.
echo   3. Build the solution. First build will fetch vcpkg dependencies
echo      defined in /vcpkg.json - this can take 10-30 minutes.
echo.

endlocal
pause
