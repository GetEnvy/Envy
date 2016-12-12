@echo on
@setLocal EnableExtensions EnableDelayedExpansion
@echo off

set "version=1.0"
set "commaver=1,0"
set "internalver=1001"

set "productvertext=			VALUE "ProductVersion","
set "filevertext=			VALUE "FileVersion","
set counter=0

cd ..\
echo Envy.h ...

(
  rem Empty Lines Support:
  for /f "delims=] tokens=1*" %%a in ('find /v /n "" ^<Envy\Envy.h') do (
    set linetesta=%%b
    if "!linetesta:~0,26!"=="#define INTERNAL_VERSION		" (
      if not "!linetesta:~26!"==%internalver% (set /a counter+=1)
      echo #define INTERNAL_VERSION		%internalver%
    ) else (setLocal DisableDelayedExpansion && echo.%%b&& endlocal)
  )
)>Envy.h.temp

if %counter% neq 0 (move /y Envy.h.temp Envy\Envy.h
) else (del Envy.h.temp && echo         No change.)


for /r %%n in (*.rc) do (
  echo %%n
  if exist %%n.temp del /f %%n.temp
  set update=0

(
  for /f "delims=] tokens=1*" %%a in ('find /n /v "" ^<""%%n""') do (
    set "linetest=%%b"

	if "%%b"=="" (echo.
    ) else if "!linetest:~1,15!"=="PRODUCTVERSION " (
      if not "!linetest:~15!"=="%commaver%" (set /a update+=1 && set /a counter+=1 && echo  PRODUCTVERSION !commaver!
      ) else (setLocal DisableDelayedExpansion && echo %%b&& endlocal)
    ) else if "!linetest:~0,26!"=="%productvertext%" (
      if not "!linetest:~28,-1!"=="%version%" (set /a update+=1 && set /a counter+=1 && echo 			VALUE "ProductVersion", "!version!"
      ) else (setLocal DisableDelayedExpansion && echo %%b&& endlocal)
    ) else if "%%~nn"=="Envy" (
      if "!linetest:~1,12!"=="FILEVERSION " (
        if not "!linetest:~12!"=="%commaver%" (set /a update+=1 && set /a counter+=1 && echo  FILEVERSION !commaver!
        ) else (setLocal DisableDelayedExpansion && echo %%b&& endlocal)
      ) else if "!linetest:~0,23!"=="%filevertext%" (
        if not "!linetest:~25,-1!"=="%version%" (set /a update+=1 && set /a counter+=1 && echo 			VALUE "FileVersion", "!version!"
        ) else (setLocal DisableDelayedExpansion && echo %%b&& endlocal)
      ) else (setLocal DisableDelayedExpansion && echo %%b&& endlocal)
    ) else (setLocal DisableDelayedExpansion && echo %%b&& endlocal)
  )
)>%%n.temp

  if !update! neq 0 (move /y %%n.temp %%n
  ) else (del %%n.temp && echo         No change.)

)

echo.
echo %counter% changes to Release Version %version%.
echo.
endlocal
pause
