//
// MiniUPnP.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2014-2016 and Shareaza 2014
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "MiniUPnP.h"
#include "Network.h"

// MiniUPnPc library
// Copyright (c) 2005-2015 Thomas Bernard
#include <MiniUPnP\miniupnpc.h>
#include <MiniUPnP\upnpcommands.h>
#include <MiniUPnP\upnpdev.h>
#pragma comment( lib, "miniupnpc" )

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


CMiniUPnP::CMiniUPnP()
	: m_nExternalTCPPort	( 0 )
	, m_nExternalUDPPort	( 0 )
{
}

CMiniUPnP::~CMiniUPnP()
{
	StopAsyncFind();
}

void CMiniUPnP::StartDiscovery()
{
	BeginThread( "MiniUPnP" );
}

void CMiniUPnP::StopAsyncFind()
{
	CloseThread();
}

void CMiniUPnP::DeletePorts()
{
	CStringA sPort;

	if ( m_nExternalTCPPort )
	{
		sPort.Format( "%u", m_nExternalTCPPort );
		int result = UPNP_DeletePortMapping( m_sControlURL, m_sServiceType, sPort, "TCP", 0 );
		if ( result == UPNPCOMMAND_SUCCESS )
			theApp.Message( MSG_DEBUG, L"UPnP successfully unmapped TCP port %u.", m_nExternalTCPPort );
		else
			theApp.Message( MSG_DEBUG, L"UPnP failed to unmap TCP port %u, error %d.", m_nExternalTCPPort, result );
		m_nExternalTCPPort = 0;
	}

	if ( m_nExternalUDPPort )
	{
		sPort.Format( "%u", m_nExternalUDPPort );
		int result = UPNP_DeletePortMapping( m_sControlURL, m_sServiceType, sPort, "UDP", 0 );
		if ( result == UPNPCOMMAND_SUCCESS )
			theApp.Message( MSG_DEBUG, L"UPnP successfully unmapped UDP port %u.", m_nExternalUDPPort );
		else
			theApp.Message( MSG_DEBUG, L"UPnP failed to unmap UDP port %u, error %d.", m_nExternalUDPPort, result );
		m_nExternalUDPPort = 0;
	}
}

bool CMiniUPnP::IsAsyncFindRunning()
{
	return IsThreadAlive();
}

void CMiniUPnP::OnRun()
{
	BOOL bSuccess = FALSE;
	int error = 0;

	if ( UPNPDev* pDevList = upnpDiscover( Settings.Connection.UPnPTimeout, NULL, NULL, UPNP_LOCAL_PORT_ANY, FALSE, 2, &error ) )
	{
		for ( UPNPDev* pDevice = pDevList ; ! bSuccess && pDevice && IsThreadEnabled() ; pDevice = pDevice->pNext )
		{
			theApp.Message( MSG_DEBUG, L"UPnP device: %s : %s", (LPCTSTR)CA2T( pDevice->descURL ), (LPCTSTR)CA2T( pDevice->st ) );

			UPNPUrls urls = {};
			IGDdatas data = {};
			char internalIPAddress[ 16 ] = {};
			int result = UPNP_GetValidIGD( pDevice, &urls, &data, internalIPAddress, sizeof( internalIPAddress ) );
			if ( result )
			{
				m_sServiceType = data.first.servicetype;
				m_sControlURL = urls.controlURL;
				FreeUPNPUrls( &urls );

				switch ( result )
				{
				case 1:
					theApp.Message( MSG_DEBUG, L"UPnP IGD found (valid and connected) : %s", (LPCTSTR)CA2T( m_sControlURL ) );
					break;
				case 2:
					theApp.Message( MSG_DEBUG, L"UPnP IGD found (valid but not connected) . Trying to continue anyway... : %s", (LPCTSTR)CA2T( m_sControlURL ) );
					break;
				default:
					theApp.Message( MSG_DEBUG, L"UPnP IGD found (not valid). Trying to continue anyway... : %s", (LPCTSTR)CA2T( m_sControlURL ) );
				}

				result = UPNP_GetExternalIPAddress( m_sControlURL, m_sServiceType, m_sExternalAddress.GetBuffer( 16 ) );
				m_sExternalAddress.ReleaseBuffer();
				if ( result == UPNPCOMMAND_SUCCESS && ! m_sExternalAddress.IsEmpty() )
				{
					WORD nPort = (WORD)Settings.Connection.InPort;
					bool bRandomPort = Settings.Connection.RandomPort;

					if ( nPort == 0 )	// Random port
						nPort = Network.RandomPort();

					// Try to map both ports
					for ( int i = 0 ; i < 5 && IsThreadEnabled() ; ++i )
					{
						CStringA sPort;
						sPort.Format( "%u", nPort );

						CString strInfo;
						strInfo.Format( L"%s at %s:%u", CLIENT_NAME L" TCP", (LPCTSTR)CA2T( internalIPAddress ), nPort );

						result = UPNP_AddPortMapping( m_sControlURL, m_sServiceType, sPort, sPort, internalIPAddress, (LPCSTR)CT2A( strInfo ), "TCP", NULL, NULL );
						if ( result == UPNPCOMMAND_SUCCESS )
						{
							char sRealPort[ 6 ] = {};
							result = UPNP_GetSpecificPortMappingEntry( m_sControlURL, m_sServiceType, sPort, "TCP", NULL, internalIPAddress, sRealPort, NULL, NULL, NULL );
							if ( result == UPNPCOMMAND_SUCCESS )
							{
								m_nExternalTCPPort = (WORD)atoi( sRealPort );
								theApp.Message( MSG_DEBUG, L"UPnP successfully mapped TCP port %u.", m_nExternalTCPPort );

								strInfo.Format( L"%s at %s:%u", CLIENT_NAME L" UDP", (LPCTSTR)CA2T( internalIPAddress ), nPort );

								result = UPNP_AddPortMapping( m_sControlURL, m_sServiceType, sPort, sPort, internalIPAddress, (LPCSTR)CT2A( strInfo ), "UDP", NULL, NULL );
								if ( result == UPNPCOMMAND_SUCCESS )
								{
									*sRealPort = '\0';
									result = UPNP_GetSpecificPortMappingEntry( m_sControlURL, m_sServiceType, sPort, "UDP", NULL, internalIPAddress, sRealPort, NULL, NULL, NULL );
									if ( result == UPNPCOMMAND_SUCCESS )
									{
										m_nExternalUDPPort = (WORD)atoi( sRealPort );
										theApp.Message( MSG_DEBUG, L"UPnP successfully mapped UDP port %u.", m_nExternalUDPPort );

										bSuccess =  ( m_nExternalTCPPort != 0 ) &&
													( m_nExternalTCPPort == m_nExternalUDPPort ) &&
													( m_nExternalTCPPort == nPort );
										if ( bSuccess )
										{
											Network.AcquireLocalAddress( (LPCTSTR)CA2T( m_sExternalAddress ), nPort );

											Settings.Connection.InPort = nPort;
											Settings.Connection.RandomPort = bRandomPort;
											break;
										}
									}
									else
										theApp.Message( MSG_DEBUG, L"UPnP failed to get mapped UDP port %u, error %d.", nPort, result );
								}
								else
									theApp.Message( MSG_DEBUG, L"UPnP failed to map UDP port %u, error %d.", nPort, result );

								DeletePorts();
							}
							else
								theApp.Message( MSG_DEBUG, L"UPnP failed to get mapped TCP port %u, error %d.", nPort, result );
						}
						else
							theApp.Message( MSG_DEBUG, L"UPnP failed to map TCP port %u, error %d.", nPort, result );

						Sleep( 200 );

						if ( ! bRandomPort && ( nPort < 1024 || nPort >= 65535 ) )
							bRandomPort = true;

						// Change port
						if ( ! bRandomPort )
							nPort++;	// Try increment
						else
							nPort = Network.RandomPort();
					}
				}
				else
					theApp.Message( MSG_DEBUG, L"UPnP failed to get external IP address, error %d.", result );
			}
			else
				theApp.Message( MSG_DEBUG, L"UPnP bad device." );
		}
		freeUPNPDevlist( pDevList );
	}
	else
		theApp.Message( MSG_DEBUG, L"UPnP found no devices." );

	if ( bSuccess )
		Network.OnMapSuccess();
	else
		Network.OnMapFailed();
}
