SkinExtract 1.0

Test resources for validity.  Extracts string table from .RC file and saves result as .XML file.

Usage:
	SkinExtract.exe input.h input.rc output.xml

Example for Envy resources:
	SkinExtract.exe ..\..\..\Envy\Resource.h "$(VCInstallDir)\atlmfc\include\afxres.h" ..\..\..\Envy\Envy.rc strings.xml
