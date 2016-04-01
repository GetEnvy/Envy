@echo off
md ..\New
SkinTranslate.exe ..\en.xml ..\New\en.pot
for %%i in ( ..\*.xml ) do SkinTranslate.exe ..\en.xml %%i ..\New\#.po
pause
for %%i in (..\New\*.po) do msgmerge.exe --no-escape --quiet --no-wrap --update --backup=off "%%i" ..\New\en.pot
for %%i in (..\New\*.po) do SkinTranslate.exe en.xml "%%i" ..\New\#.xml
:: Set Visual Studio path:
:: SkinExport.exe ..\..\..\Envy\Resource.h "$(VCInstallDir)\atlmfc\include\afxres.h" ..\..\..\Envy\Envy.rc ..\New\Skin.xml
pause