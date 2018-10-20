//
// Firewall.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

// CFirewall wraps Windows COM components to change Windows Firewall settings, and talk UPnP to a NAT router
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CFirewall


#include "StdAfx.h"
#include "Firewall.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Make/Delete the WindowsFirewall object
CFirewall::CFirewall()
{
}

CFirewall::~CFirewall()
{
}

BOOL CFirewall::Init()
{
	// Create an instance of the firewall settings manager
	HRESULT hr = FwManager.CoCreateInstance( __uuidof( NetFwMgr ) );
	if ( SUCCEEDED( hr ) && FwManager )
	{
		// Retrieve the local firewall policy
		hr = FwManager->get_LocalPolicy( &Policy );
		if ( SUCCEEDED( hr ) && Policy )
		{
			// Retrieve the firewall profile currently in effect
			hr = Policy->get_CurrentProfile( &Profile );
			if ( SUCCEEDED( hr ) && Profile )
			{
				// Retrieve the allowed services collection
				/*hr =*/ Profile->get_Services( &ServiceList );
				// Retrieve the authorized application collection
				/*hr =*/ Profile->get_AuthorizedApplications( &ProgramList );
				// Retrieve the globally open ports collection
				/*hr =*/ Profile->get_GloballyOpenPorts( &PortList );
			}
		}
	}

	return ServiceList && ProgramList && PortList;
}

// Takes a service type, like NET_FW_SERVICE_UPNP, which is listed in Windows Firewall and can't be removed
// Makes sure it is checked in Windows Firewall, checking it if necessary
// Returns true if the service is listed and checked, false if we weren't able to check it
BOOL CFirewall::SetupService( NET_FW_SERVICE_TYPE service )
{
	// Make sure the COM interfaces have been accessed
	//if ( ! FwManager ) return FALSE;

	// If the service isn't enabled on the Windows Firewall exceptions list
	BOOL bEnabled = FALSE;
	if ( ! IsServiceEnabled( service, &bEnabled ) ) return FALSE;
	if ( ! bEnabled )
	{
		// Check its checkbox
		if ( ! EnableService( service ) ) return FALSE;

		// Wait for discovery to complete
		Sleep( 3000 );
	}

	// The service is listed and checked
	return TRUE;
}

// Takes a path and file name like "C:\Folder\Program.exe" and a name like "My Program"
// Makes sure the program is listed in Windows Firewall and its listing is checked, adding and checking it as necessary
// Returns true if the program is listed and checked, false if we weren't able to do it
// When bRemove is TRUE, it removes the application from the exception list
BOOL CFirewall::SetupProgram( const CString& path, const CString& name, BOOL bRemove )
{
	// Make sure the COM interfaces have been accessed
	//if ( ! FwManager ) return FALSE;

	// If the program isn't on the Windows Firewall exceptions list
	BOOL bListed = FALSE;
	if ( ! IsProgramListed( path, &bListed ) ) return FALSE;
	if ( ! bListed && ! bRemove )
	{
		// Add it to the list with a checked checkbox
		if ( ! AddProgram( path, name ) ) return FALSE;
	}
	else if ( bListed && bRemove )
	{
		if ( ! RemoveProgram( path ) ) return FALSE;
		return TRUE;
	}

	// If the program is on the list, but its checkbox isn't checked
	BOOL bEnabled = FALSE;
	if ( ! IsProgramEnabled( path, &bEnabled ) ) return FALSE;
	if ( ! bEnabled )
	{
		// Check the checkbox
		if ( ! EnableProgram( path ) ) return FALSE;
	}

	// The program is listed and checked
	return TRUE;
}

// Takes a program path and file name, like "C:\Folder\Program.exe"
// Determines if it's listed in Windows Firewall
// Returns true if it works, and writes the answer in listed
BOOL CFirewall::IsProgramListed( const CString& path, BOOL* listed )
{
	if ( ! ProgramList ) return FALSE;	// COM not initialized

	// Look for the program in the list
	// Try to get the interface for the program with the given name
	Program.Release();
	HRESULT hr = ProgramList->Item( CComBSTR( path ), &Program );
	if ( SUCCEEDED( hr ) && Program )
	{
		// The program is in the list
		*listed = TRUE;
		return TRUE;
	}

	// ProgramList->Item call failed,
	// The error is not found
	if ( hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) )
	{
		// The program is not in the list
		*listed = FALSE;
		return TRUE;
	}

	// Some other error occurred, report it
	return FALSE;
}

// Takes a service type, like NET_FW_SERVICE_UPNP
// Determines if the listing for that service in Windows Firewall is checked or unchecked
// Returns true if it works, and writes the answer in enabled
BOOL CFirewall::IsServiceEnabled( NET_FW_SERVICE_TYPE service, BOOL* enabled )
{
	if ( ! ServiceList ) return FALSE;

	// Look for the service in the list
	Service.Release();
	HRESULT hr = ServiceList->Item( service, &Service );
	if ( FAILED( hr ) || ! Service ) return FALSE;	// Services can't be removed from the list

	// Find out if the service is enabled
	VARIANT_BOOL v = VARIANT_FALSE;
	hr = Service->get_Enabled( &v );
	if ( FAILED( hr ) ) return FALSE;

	if ( v == VARIANT_FALSE )
		*enabled = FALSE;	// The service is on the list, but the checkbox next to it is cleared
	else
		*enabled = TRUE;	// The service is on the list and the checkbox is checked

	return TRUE;
}

// Takes a program path and file name like "C:\Folder\Program.exe"
// Determines if the listing for that program in Windows Firewall is checked or unchecked
// Returns true if it works, and writes the answer in enabled
BOOL CFirewall::IsProgramEnabled( const CString& path, BOOL* enabled )
{
	// First, make sure the program is listed
	BOOL bListed = FALSE;
	if ( ! IsProgramListed( path, &bListed ) ) return FALSE;	// This sets the Program interface we can use here
	if ( ! bListed ) return FALSE;		// The program isn't in the list at all

	// Find out if the program is enabled
	VARIANT_BOOL v = VARIANT_FALSE;
	HRESULT hr = Program->get_Enabled( &v );
	if ( FAILED( hr ) ) return FALSE;

	if ( v == VARIANT_FALSE )
		*enabled = FALSE;	// The program is on the list, but the checkbox next to it is cleared
	else
		*enabled = TRUE;	// The program is on the list and the checkbox is checked

	return TRUE;
}

// This means that all the exceptions such as GloballyOpenPorts, Applications, or Services,
// which are specified in the profile, are ignored and only locally initiated traffic is allowed

BOOL CFirewall::AreExceptionsAllowed() const
{
	if ( ! Profile ) return FALSE;		// COM not initialized

	VARIANT_BOOL vbNotAllowed = VARIANT_FALSE;
	HRESULT hr = Profile->get_ExceptionsNotAllowed( &vbNotAllowed );
	if ( SUCCEEDED( hr ) && vbNotAllowed != VARIANT_FALSE ) return FALSE;

	return TRUE;
}

// Takes a path and file name like "C:\Folder\Program.exe" and a name like "My Program"
// Lists and checks the program on Windows Firewall, so now it can listed on a socket without a warning popping up
// Returns false on error
BOOL CFirewall::AddProgram( const CString& path, const CString& name )
{
	HRESULT hr;

	// Create an instance of an authorized application, we'll use this to add our new application
	Program.Release();
	hr = Program.CoCreateInstance( __uuidof( NetFwAuthorizedApplication ) );
	if ( FAILED( hr ) ) return FALSE;

	hr = Program->put_ProcessImageFileName( CComBSTR( path ) );	// Set the process image file name
	if ( FAILED( hr ) ) return FALSE;

	hr = Program->put_Name( CComBSTR( name ) );					// Set the program name
	if ( FAILED( hr ) ) return FALSE;

	// Get the program on the Windows Firewall accept list
	hr = ProgramList->Add( Program );							// Add the application to the collection
	if ( FAILED( hr ) ) return FALSE;

	return TRUE;
}

BOOL CFirewall::RemoveProgram( const CString& path )
{
	if ( ! ProgramList ) return FALSE;	// COM not initialized

	HRESULT hr = ProgramList->Remove( CComBSTR( path ) );		// Remove the application to the collection
	if ( FAILED( hr ) ) return FALSE;

	return TRUE;
}

// Takes a service type, like NET_FW_SERVICE_UPNP
// Checks the checkbox next to its listing in Windows Firewall
// Returns false on error
BOOL CFirewall::EnableService( NET_FW_SERVICE_TYPE service )
{
	if ( ! ServiceList ) return FALSE;	// COM not initialized

	// Look for the service in the list
	Service.Release();
	HRESULT hr = ServiceList->Item( service, &Service );
	if ( FAILED( hr ) || ! Service ) return FALSE;				// Services can't be removed from the list

	// Check the box next to the service
	hr = Service->put_Enabled( VARIANT_TRUE );
	if ( FAILED( hr ) ) return FALSE;

	return TRUE;
}

// Takes a program path and file name like "C:\Folder\Program.exe"
// Checks the checkbox next to its listing in Windows Firewall
// Returns false on error
BOOL CFirewall::EnableProgram( const CString& path )
{
	// First, make sure the program is listed
	BOOL bListed;
	if ( ! IsProgramListed( path, &bListed ) ) return FALSE;	// This sets the Program interface we can use here
	if ( ! bListed ) return FALSE;		// The program isn't on the list at all
	if ( ! Program ) return FALSE;		// COM not initialized

	// Check the box next to the program
	HRESULT hr = Program->put_Enabled( VARIANT_TRUE );
	if ( FAILED( hr ) ) return FALSE;

	return TRUE;
}
