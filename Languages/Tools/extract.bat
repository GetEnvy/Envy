@echo off
SkinExport.exe ..\..\..\Envy\Resource.h "$(VCInstallDir)\atlmfc\include\afxres.h" ..\..\..\Envy\Envy.rc Skin.xml
pause