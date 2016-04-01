Schema XSD files provide for search selection, metadata fields, download group types, etc.
Schema XML files provide for translation and presentation of XSD.

SchemaDescriptor URIs (schemas.getenvy.com) are universal names, and cannot easily be changed.
Schema reconciliation is in SchemaMappings.xml and Schema.cpp, etc.

The exact filename from .xsd is used to detect related resources: (.xml/.ico)
Library icons may be re-specified in xml.  Simply overwrite existing icons to skin, not upgrade-safe.
".Safe" variant icons are used when original cannot be loaded. (Was Windows2000)

"Database" folder was used for convenience in xml translation. (Outdated)
"typeFilter" extensions in xml are used for download group default filters.

Library Organizer file/folder inclusion itself remains hard-coded.
Filetype .ext handling is in core LibraryBuilderInternals.cpp, and various Library Builder Plugins.
Default folders are in LibraryFolder, AlbumFolder, etc.
Folders translated by Schema language when missing (at first run or deletion).

Note many aspects are mutually dependant in various places,
any functional change must often be mirrored elsewhere.  (xsd/xml/code)






Translation Credits:  (From Shareaza source)

af	Afrikaans schemas by Johann van der Walt (Thesage)
ca	Catalan schemas by Daniel Olivares Giménez, Equip de traduccions de FeshoCAT
cz	Czech schemas by Odin
de	German schemas by Spooky, jlh, OldDeath
ee	Estonian schemas by Janza
es	Spanish schemas by Help, others
fr	French schemas by Antoine Martin-Lalande (Sensi), zigotozor
gr	Greek schemas by erimitis
he	Hebrew schemas by _peer_ (H.L)
hr	Croatian schemas by Biza
hu	Hungarian schemas by Yuri
it	Italian schemas by ale5000
ja	Japanese schemas by DukeDog, CyberBob
ko	Korean schemas machine translated
lt	Lithuanian schemas by Rolandas Rudomanskis
nl	Dutch schemas by Jan-Willem Koornwinder (thurstydog), Jonne, Neglacio
no	Norwegian schemas by Tom Seland (seli), vnmartinsen
pt	Portuguese (Brasil) schemas by Lukas Taves (LightHeaven), Felipe
ru	Russian schemas by dAbserver, Rolandas Rudomanskis, Ryo-oh-ki
sl	Slovenian schemas by Martin McDowell
sq	Albanian schemas by Besmir Godole
sr	Serbian schemas by Woodstoock
sv	Swedish schemas by Marcus Ehrenlans, Printz, DanielB
tr	Turkish schemas by F. Doğru, F.A. Saraçoğulları, zigotozor
zhs	Chinese Simplified schemas by Sanley, John
zht	Chinese Traditional schemas by James Bond

<!-- Document:Document XML Framework; 1.4; Jonathan C. Nilson; originals by Higgy -->
