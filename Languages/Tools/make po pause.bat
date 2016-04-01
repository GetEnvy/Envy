@echo off
for %%i in ( ..\*.xml ) do (
	SkinTranslate.exe ..\en.xml %%i ..\Poedit\#.po
	pause )