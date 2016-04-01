@echo off
md ..\New
for %%i in ( ..\*.xml ) do SkinTranslate.exe ..\en.xml %%i ..\New\#.po
pause