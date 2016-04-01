@echo off
md ..\New
for %%i in ( ..\Poedit\*.po ) do SkinTranslate.exe ..\en.xml %%i ..\New\#.xml
@pause