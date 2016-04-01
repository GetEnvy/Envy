//
// Revision.js
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2009-2012 and Shareaza 2009
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version at your option.
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 (AGPLv3) for details:
// (http://www.gnu.org/licenses/agpl.html)
//

// This script uses .SVN hidden folder data to update current directory revision
// in Revision.h file if needed.  (For About dialog, etc.)


var fso = WScript.CreateObject( "Scripting.FileSystemObject" );
var fname = fso.GetAbsolutePathName( "Revision.h" );
var fsvn;
var svnfile;

var revision;
try
{
	// Old SVN style first
	fsvn = fso.GetAbsolutePathName( ".svn\\all-wcprops" );
	svnfile = fso.OpenTextFile( fsvn, 1, false );
	revision = svnfile.Read( 69 ).substr( 66 );		// Parsing:  "!svn/ver/XXX/..."
	WScript.Echo( "Current SVN Revision is " + revision );
	svnfile.Close();
}
catch(e)
{
	try
	{
		// New SVN style (1.7+)
		fsvn = fso.GetAbsolutePathName( "..\\.svn\\wc.db" );
		svnfile = fso.OpenTextFile( fsvn, 1, false );
		revision = svnfile.Read( 4 );		// Test
	}
	catch(e)
	{
		try
		{
			fsvn = fso.GetAbsolutePathName( "..\\..\\.svn\\wc.db" );
			svnfile = fso.OpenTextFile( fsvn, 1, false );
			revision = svnfile.Read( 4 );	// Test
		}
		catch(e)
		{
			WScript.Echo( "No SVN Revision Data Available." );
			WScript.Quit( 0 );
		}
	}

	revision = null;

	//svnfile.Skip( 1000 );
	var text = svnfile.ReadAll();
	svnfile.Close();

	var re = /normaldir.{60,90}?\/!svn\/ver\/(\d+)\/(?:trunk\/)?Envy\)/gi;
	var result = re.exec( text );

	while ( result != null )
	{
		revision = result[1];
		result = re.exec( text );
	}

	if ( revision == null )
	{
		WScript.Echo( "No Proper SVN Revision Detected." );
		WScript.Quit( 0 );
	}

	WScript.Echo( "Current SVN Revision is r" + revision );
}

var modified;
try
{
	var tsr = fso.OpenTextFile( fname, 1, false );
	modified = tsr.ReadLine().substr( 4 );			// Parsing:  "// rXXX"
	tsr.Close();
}
catch(e)
{
	WScript.Echo( "Revision.h does not exist." );
}

if ( revision != modified )
{
	WScript.Echo( "Updating Revision from \"" + modified + "\" to \"" + revision + "\"...");
	try
	{
		var tsw = fso.OpenTextFile( fname, 2, true );
		tsw.WriteLine( "// r" + revision );
		tsw.WriteLine( "// Auto-generated" );
		tsw.WriteLine( "" );
		tsw.WriteLine( "#pragma once" );
		tsw.WriteLine( "" );
		tsw.WriteLine( "#define __REVISION__ \t\"" + revision + "\"" );
		tsw.WriteLine( "//#define __MODAUTHOR__\t\"YOUR NAME HERE\"" );
		tsw.Close();
	}
	catch(e)
	{
		WScript.Echo( "Revision Update Failed: \"" + fname + "\"" );
		WScript.Quit( 1 );
	}
}
else
	WScript.Echo( "Revision is up to date." );

WScript.Quit( 0 );
