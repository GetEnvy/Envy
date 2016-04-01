
Envy Language Skin Translation Tools.
___________________________________________


Localization is supported through the theme engine with .XML skin files.

An always-updated English sample skin is the basis for all other languages:
http://svn.code.sf.net/p/getenvy/code/trunk/Languages/en.xml


There are over 4000 availble strings of varying priority, uniquely formatted for Envy.
Unfortunately there is no very simple way to maintain updates in such files.
The basic method is using WinMerge (winmerge.org) or similar tool to compare default-en and new default-XX,
while tracking updates from http://sourceforge.net/p/getenvy/code/HEAD/log/?path=/trunk/Languages/en.xml

- The SkinExtract commandline tool will export all isolated strings/tips resources to an .XML file for reference. (To update default-en)

- The SkinTranslate commandline conversion tool will assist those who prefer to work with common .po files in Poedit (poedit.net)

- The SkinUpdater GUI tool may help identify simple XML updates if you have both the older equivalent and updated versions of default-en.xml available.

Typically: Convert an .XML to .PO, make modifications easily in Poedit, then convert back to a new .XML for comparison with the original in WinMerge.





SkinTranslate Commandline Utility.
_________________________________


Basic usage from the DOS prompt (Run: cmd) in the proper directory:


1. Create .POT-file from default English .XML-file:

	SkinTranslate.exe default-en.xml default-en.pot

2. Create .PO-file from target language .XML-file:

	SkinTranslate.exe default-en.xml default-XX.xml default-XX.po

3. Recreate translated .XML-file from edited .PO-file:

	SkinTranslate.exe default-en.xml default-XX.po default-XX.xml


Note third parameter can use meta symbol "#" to repeat filename from second parameter:
	"SkinTranslate.exe ..\default-en.xml ..\default-XX.xml ..\Poedit\#.po"
	"SkinTranslate.exe ..\default-en.xml ..\Poedit\default-XX.po ..\New\#.xml"

One-click scripts are provided for mass conversions.

SkinTranslate.exe was created for Shareaza by Nikolay Raspopov <ryo-oh-ki@narod.ru>





Example Translation Guide.
__________________________


PoEdit utility can be downloaded from:	http://www.poedit.net/download.php
WinMerge can be downloaded from:	http://winmerge.org/downloads/


How to Update existing translation in Poedit:

1. Create default-en.pot from updated default-en.xml (see Usage #1 above).
2. Convert existing default-XX.xml to default-XX.po if needed  (see Usage #2 above).
3. Run Poedit.
4. Open default-XX.po and then select menu: Catalog -> Update from POT file -> Select default-en.pot.
5. Translate new strings.
6. Convert default-XX.po back to default-XX.xml (see Usage #3 above).
7. Run WinMerge.
8. Drag'n'Drop pre-existing XX.xml, then Drag'n'Drop new XX.xml, and make any highlighted changes.
9. Test final .XML in installed Envy/skins/languages folder.
10.Upload both updated files to Translation forum. (New .XML and .PO)


How to Create new translation in Poedit:

1. Create en.pot from en.xml (see Usage #1 above).
2. Run Poedit.
3. Select menu: File -> New catalog from POT file... -> Select en.pot.
4. Fill translation properties (codepage MUST be UTF-8). Press OK.
5. Save as XX.po, where XX is a two letter language code.
6. Translate.
7. Convert XX.po back to XX.xml (see Usage #3 above).
8. Make language flag (XX.ico file) or request one.
9. Upload all files to Translation forum.


How to Create new translation natively:

0. Find your new XX 2-letter code ( http://en.wikipedia.org/wiki/List_of_ISO_639-1_codes )

1. Copy en.xml to installed \Envy\Skins\Languages\XX.xml
2. Copy another flag temporarily as \Envy\Skins\Languages\XX.ico
3. Open .XML in Notepad, etc., or WinMerge with original en.xml.
4. Look through the file to get familiar before making any changes.
5. First thing DELETE <commandTips> section from the middle of the file, (<tip id="") are least important.
6. Now update the Manifest at the beginning of the file to reflect the new language.
7. Begin by converting English text in Toolbars at top (text="") or ListColumns at bottom (to="") -Most important or quickest.
8. Then continue with the top Menus, Strings, Dialogs, and Documents you may recognize as important.
9. Note special XML characters in various sections: &#160; = &nbsp; for an extra space, &amp; for every "&", specifically \n or {n} for new lines, etc.
10.Test often. Select your language once from the Tools menu, and simply press Ctrl+R or Ctrl+Shift+Z to refresh your changes.


Note the 30 languages inherited from Shareaza have all been extensively updated structurally, but still contain outdated text and flaws.
