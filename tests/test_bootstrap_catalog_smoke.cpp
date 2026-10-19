//
// test_bootstrap_catalog_smoke.cpp
//
// Offline parser tests for Data/DefaultServices.dat and DefaultServers.dat.
// No live P2P or HTTP.
//
// This file is part of Envy (getenvy.com) (C) 2016-2026
//

#include "test_framework.h"
#include "../Envy/BootstrapCatalog.h"

#include <cstdio>
#include <string>
#include <cwchar>
#include <algorithm>
#include <vector>

namespace {

struct ServiceRow
{
	wchar_t type;
	std::wstring endpoint;
};

struct ServerRow
{
	wchar_t type;
	bool priority;
	std::wstring host;
};

BootstrapParseStatus ParseService(const wchar_t* line, ServiceRow* row)
{
	wchar_t cType = 0;
	const wchar_t* psz = nullptr;
	size_t n = 0;
	const BootstrapParseStatus st = BootstrapParseServiceLine(
		line, line ? wcslen( line ) : 0, &cType, &psz, &n );
	if ( st == BootstrapParseStatus::Ok && row )
	{
		row->type = cType;
		row->endpoint.assign( psz, n );
	}
	return st;
}

BootstrapParseStatus ParseServer(const wchar_t* line, ServerRow* row)
{
	wchar_t cType = 0;
	bool bPri = false;
	const wchar_t* psz = nullptr;
	size_t n = 0;
	const BootstrapParseStatus st = BootstrapParseServerLine(
		line, line ? wcslen( line ) : 0, &cType, &bPri, &psz, &n );
	if ( st == BootstrapParseStatus::Ok && row )
	{
		row->type = cType;
		row->priority = bPri;
		row->host.assign( psz, n );
	}
	return st;
}

const wchar_t* kShippedServices =
	L"M http://midian.jayl.de/g2/bazooka.php\n"
	L"M http://bj.ddns.net/beacon/gwc.php\n"
	L"M http://gweb3.4octets.co.uk/gwc.php\n"
	L"2 http://dkac.trillinux.org/dkac/dkac.php\n"
	L"U uhc:useast.gnutella.dyslexicfish.net:3558\n"
	L"U uhc:uswest.gnutella.dyslexicfish.net:3558\n"
	L"U uhc:uk.gnutella.dyslexicfish.net:3558\n"
	L"U uhc:au.gnutella.dyslexicfish.net:3558\n"
	L"U uhc:1.uhc.gtk-gnutella.nl:19104\n"
	L"U uhc:2.uhc.gtk-gnutella.nl:4876\n"
	L"D https://upd.emule-security.org/server.met\n"
	L"D https://shortypower.org/server.met\n"
	L"H https://dchublist.org/hublist.xml.bz2\n"
	L"H https://dchublist.ru/hublist.xml.bz2\n"
	L"H https://hublist.pwiam.com/hublist.xml.bz2\n"
	L"H https://www.te-home.net/?do=hublist&get=hublist.xml.bz2\n"
	L"K https://upd.emule-security.org/nodes.dat\n"
	L"X http://gwebcache.spearforensics.com/\n"
	L"X http://www.myfoxy.net/gwc/cgi-bin/fc\n"
	L"X http://gwc.gofoxy.net:2108/gwc/cgi-bin/fc\n";

void CountServices(const wchar_t* blob, int* nWeb, int* nG2, int* nG1, int* nG1Udp, int* nMet, int* nHub, int* nNodesDat)
{
	*nWeb = *nG2 = *nG1 = *nG1Udp = *nMet = *nHub = *nNodesDat = 0;
	const wchar_t* p = blob;
	while ( p && *p )
	{
		const wchar_t* eol = wcschr( p, L'\n' );
		const size_t nLine = eol ? static_cast< size_t >( eol - p ) : wcslen( p );
		std::wstring line( p, nLine );
		wchar_t cType = 0;
		const wchar_t* psz = nullptr;
		size_t n = 0;
		if ( BootstrapParseServiceLine( line.c_str(), line.size(), &cType, &psz, &n )
			== BootstrapParseStatus::Ok )
		{
			const BootstrapServiceClass cls = BootstrapClassifyServiceType( cType );
			if ( cls == BootstrapServiceClass::MultiGwc ||
				 cls == BootstrapServiceClass::G2Gwc ||
				 cls == BootstrapServiceClass::G1Gwc )
				++(*nWeb);
			if ( cls == BootstrapServiceClass::MultiGwc ||
				 cls == BootstrapServiceClass::G2Gwc )
				++(*nG2);
			if ( cls == BootstrapServiceClass::MultiGwc ||
				 cls == BootstrapServiceClass::G1Gwc )
				++(*nG1);
			if ( cls == BootstrapServiceClass::GnutellaUdp )
			{
				// UHC counts as G1 in EnoughServices.
				if ( n >= 4 && wcsncmp( psz, L"uhc:", 4 ) == 0 )
				{
					++(*nG1);
					++(*nG1Udp);
				}
				if ( n >= 5 && wcsncmp( psz, L"ukhl:", 5 ) == 0 )
					++(*nG2);
			}
			if ( cls == BootstrapServiceClass::Ed2kMet )
				++(*nMet);
			if ( cls == BootstrapServiceClass::DcHublist )
				++(*nHub);
			if ( cls == BootstrapServiceClass::KadNodesDat )
				++(*nNodesDat);
		}
		p = eol ? eol + 1 : p + nLine;
		if ( !eol )
			break;
	}
}

} // namespace

static bool test_service_valid_https()
{
	ServiceRow row{};
	return ParseService( L"D https://upd.emule-security.org/server.met", &row )
		== BootstrapParseStatus::Ok
		&& row.type == L'D'
		&& row.endpoint == L"https://upd.emule-security.org/server.met";
}

static bool test_service_comment()
{
	return ParseService( L"# M http://example.invalid/gwc", nullptr )
		== BootstrapParseStatus::Skip;
}

static bool test_service_blank()
{
	return ParseService( L"   \t", nullptr ) == BootstrapParseStatus::Skip;
}

static bool test_service_unknown_type()
{
	return ParseService( L"Q http://example.invalid/gwc", nullptr )
		== BootstrapParseStatus::Invalid;
}

static bool test_service_invalid_url()
{
	return ParseService( L"D ftp://upd.emule-security.org/server.met", nullptr )
		== BootstrapParseStatus::Invalid;
}

static bool test_service_whitespace_endpoint()
{
	ServiceRow row{};
	return ParseService( L"D   https://upd.emule-security.org/server.met", &row )
		== BootstrapParseStatus::Ok
		&& row.endpoint == L"https://upd.emule-security.org/server.met";
}

static bool test_service_uhc()
{
	ServiceRow row{};
	return ParseService( L"U uhc:1.uhc.gtk-gnutella.nl:19104", &row )
		== BootstrapParseStatus::Ok
		&& BootstrapClassifyServiceType( row.type ) == BootstrapServiceClass::GnutellaUdp;
}

static bool test_server_valid_dht()
{
	ServerRow row{};
	return ParseServer( L"B dht.libtorrent.org:25401", &row )
		== BootstrapParseStatus::Ok
		&& row.type == L'B'
		&& !row.priority
		&& row.host == L"dht.libtorrent.org:25401";
}

static bool test_server_priority_and_comment()
{
	ServerRow row{};
	return ParseServer( L"B* dht.transmissionbt.com:6881\t# Transmission", &row )
		== BootstrapParseStatus::Ok
		&& row.priority
		&& row.host == L"dht.transmissionbt.com:6881";
}

static bool test_server_comment_line()
{
	return ParseServer( L"# B router.bitcomet.com:6881", nullptr )
		== BootstrapParseStatus::Skip;
}

static bool test_server_missing_port()
{
	return ParseServer( L"B router.bittorrent.com", nullptr )
		== BootstrapParseStatus::Invalid;
}

static bool test_server_unknown_type()
{
	return ParseServer( L"Z 1.2.3.4:1234", nullptr )
		== BootstrapParseStatus::Invalid;
}

static bool test_dedup_case_insensitive()
{
	const wchar_t* a = L"https://ShortyPower.org/server.met";
	const wchar_t* b = L"https://shortypower.org/server.met";
	const wchar_t* other = L"https://other.example/server.met";
	return BootstrapWideEqualsNoCase( a, wcslen( a ), b, wcslen( b ) )
		&& ! BootstrapWideEqualsNoCase( a, wcslen( a ), other, wcslen( other ) );
}

static bool test_fold_ascii_only()
{
	const wchar_t upperA = L'A';
	const wchar_t lowerA = L'a';
	const wchar_t angstrom = 0x00C5;	// Latin capital A with ring
	const wchar_t angstromSmall = 0x00E5;
	return BootstrapFoldAscii( upperA ) == lowerA
		&& BootstrapFoldAscii( angstrom ) == angstrom
		&& ! BootstrapWideEqualsNoCase( &angstrom, 1, &angstromSmall, 1 );
}

static bool test_kad_nodes_dat_rejects_http_url()
{
	return ParseService( L"K http://upd.emule-security.org/nodes.dat", nullptr )
		== BootstrapParseStatus::Invalid;
}

static bool test_kad_nodes_dat_accepts_https_url()
{
	ServiceRow row = {};
	return ParseService( L"K https://upd.emule-security.org/nodes.dat", &row )
		== BootstrapParseStatus::Ok && row.type == L'K';
}

static bool test_shipped_catalogue_meets_minima()
{
	int nWeb = 0, nG2 = 0, nG1 = 0, nG1Udp = 0, nMet = 0, nHub = 0, nNodesDat = 0;
	CountServices( kShippedServices, &nWeb, &nG2, &nG1, &nG1Udp, &nMet, &nHub, &nNodesDat );
	return nWeb >= BootstrapMinWebCaches
		&& nG2 >= BootstrapMinG2Services
		&& nG1 >= BootstrapMinG1Services
		&& nG1Udp >= BootstrapMinG1UdpServices
		&& nMet >= BootstrapMinEd2kMet
		&& nHub >= BootstrapMinDcHublists
		&& nNodesDat >= BootstrapMinKadNodesDat;
}

static bool test_shipped_catalogue_no_static_ed2k_ip()
{
	return wcsstr( kShippedServices, L"176.103." ) == nullptr
		&& wcsstr( kShippedServices, L"91.226.212.11" ) == nullptr;
}

static bool test_shipped_catalogue_no_getenvy_gwc()
{
	return wcsstr( kShippedServices, L"cache.getenvy.com" ) == nullptr
		&& wcsstr( kShippedServices, L"dchublist.com/" ) == nullptr
		&& wcsstr( kShippedServices, L"tankafett.biz" ) == nullptr;
}

static std::string ParentDir( const std::string& path )
{
	const size_t n = path.find_last_of( "/\\" );
	return n == std::string::npos ? std::string() : path.substr( 0, n );
}

static bool LoadRepoDataFile( const char* name, std::wstring* wide )
{
	if ( ! name || ! wide )
		return false;
	const std::string srcDir = ParentDir( __FILE__ );
	const std::string paths[] = {
		std::string( "Data/" ) + name,
		std::string( "../Data/" ) + name,
		std::string( "../../Data/" ) + name,
		std::string( "../../../Data/" ) + name,
		srcDir + "/../Data/" + name
	};
	FILE* fp = nullptr;
	for ( const std::string& path : paths )
	{
		fp = fopen( path.c_str(), "rb" );
		if ( fp )
			break;
	}
	if ( ! fp )
		return false;
	std::string bytes;
	char buf[ 4096 ];
	size_t nRead;
	while ( ( nRead = fread( buf, 1, sizeof( buf ), fp ) ) > 0 )
		bytes.append( buf, nRead );
	fclose( fp );
	wide->clear();
	wide->reserve( bytes.size() );
	for ( unsigned char ch : bytes )
		wide->push_back( static_cast< wchar_t >( ch ) );
	return true;
}

static bool test_shipped_default_services_dat()
{
	std::wstring wide;
	if ( ! LoadRepoDataFile( "DefaultServices.dat", &wide ) )
		return false;
	int nWeb = 0, nG2 = 0, nG1 = 0, nG1Udp = 0, nMet = 0, nHub = 0, nNodesDat = 0;
	CountServices( wide.c_str(), &nWeb, &nG2, &nG1, &nG1Udp, &nMet, &nHub, &nNodesDat );
	// Compare parsed identities, so comments cannot satisfy the check
	// and an accidental extra retired source cannot hide behind minimum counts.
	// Blocked identities must also survive catalogue maintenance.
	std::vector< std::wstring > actual, expected;
	const std::wstring blobs[] = { wide, std::wstring( kShippedServices ) };
	for ( size_t i = 0; i < 2; ++i )
	{
		const std::wstring& blob = blobs[ i ];
		std::vector< std::wstring >& rows = i == 0 ? actual : expected;
		size_t start = 0;
		while ( start < blob.size() )
		{
			const size_t end = blob.find( L'\n', start );
			const std::wstring line = blob.substr( start, end - start );
			ServiceRow row{};
			const BootstrapParseStatus status = ParseService( line.c_str(), &row );
			if ( status == BootstrapParseStatus::Invalid )
				return false;
			const size_t first = line.find_first_not_of( L" \t\r" );
			if ( status == BootstrapParseStatus::Skip && first != std::wstring::npos
				&& line[ first ] != L'#' )
				return false;
			if ( status == BootstrapParseStatus::Ok )
				rows.push_back( std::wstring( 1, row.type ) + L" " + row.endpoint );
			if ( end == std::wstring::npos )
				break;
			start = end + 1;
		}
	}
	std::sort( actual.begin(), actual.end() );
	std::sort( expected.begin(), expected.end() );
	if ( actual != expected )
		return false;

	return nWeb >= BootstrapMinWebCaches
		&& nG2 >= BootstrapMinG2Services
		&& nG1 >= BootstrapMinG1Services
		&& nG1Udp >= BootstrapMinG1UdpServices
		&& nMet >= BootstrapMinEd2kMet
		&& nHub >= BootstrapMinDcHublists
		&& nNodesDat >= BootstrapMinKadNodesDat
		&& wide.find( L"https://upd.emule-security.org/server.met" ) != std::wstring::npos
		&& wide.find( L"https://upd.emule-security.org/nodes.dat" ) != std::wstring::npos
		&& wide.find( L"176.103.48.36" ) == std::wstring::npos;
}

static bool test_shipped_default_servers_dat()
{
	std::wstring wide;
	if ( ! LoadRepoDataFile( "DefaultServers.dat", &wide ) )
		return false;
	int nBt = 0;
	std::vector< std::wstring > hosts;
	const wchar_t* p = wide.c_str();
	while ( p && *p )
	{
		const wchar_t* eol = wcschr( p, L'\n' );
		const size_t nLine = eol ? static_cast< size_t >( eol - p ) : wcslen( p );
		std::wstring line( p, nLine );
		if ( ! line.empty() && line.back() == L'\r' )
			line.pop_back();
		wchar_t cType = 0;
		bool bPri = false;
		const wchar_t* psz = nullptr;
		size_t n = 0;
		const BootstrapParseStatus status = BootstrapParseServerLine(
			line.c_str(), line.size(), &cType, &bPri, &psz, &n );
		if ( status == BootstrapParseStatus::Invalid )
			return false;
		if ( status == BootstrapParseStatus::Ok )
		{
			if ( cType != L'B' || bPri )
				return false;
			const std::wstring host( psz, n );
			if ( host != L"dht.transmissionbt.com:6881" &&
				 host != L"router.bittorrent.com:6881" &&
				 host != L"dht.libtorrent.org:25401" )
				return false;
			if ( std::find( hosts.begin(), hosts.end(), host ) != hosts.end() )
				return false;
			hosts.push_back( host );
			++nBt;
		}
		p = eol ? eol + 1 : p + nLine;
		if ( ! eol )
			break;
	}
	return nBt == 3
		&& wide.find( L"dht.transmissionbt.com:6881" ) != std::wstring::npos
		&& wide.find( L"dht.libtorrent.org:25401" ) != std::wstring::npos
		&& wide.find( L"router.bittorrent.com:6881" ) != std::wstring::npos;
}

static bool test_service_https_scheme_case()
{
	ServiceRow row{};
	return ParseService( L"D HTTPS://upd.emule-security.org/server.met", &row )
		== BootstrapParseStatus::Ok
		&& row.endpoint == L"HTTPS://upd.emule-security.org/server.met";
}

static bool test_server_tab_separator()
{
	ServerRow row{};
	return ParseServer( L"B\tdht.libtorrent.org:25401", &row )
		== BootstrapParseStatus::Ok
		&& row.type == L'B'
		&& row.host == L"dht.libtorrent.org:25401";
}

static bool test_server_trailing_note()
{
	ServerRow row{};
	return ParseServer( L"B dht.libtorrent.org:25401 seen-time", &row )
		== BootstrapParseStatus::Ok
		&& row.host == L"dht.libtorrent.org:25401";
}

static bool test_server_port_range()
{
	return ParseServer( L"B router.bittorrent.com:0", nullptr ) == BootstrapParseStatus::Invalid
		&& ParseServer( L"B router.bittorrent.com:65536", nullptr ) == BootstrapParseStatus::Invalid
		&& ParseServer( L"B router.bittorrent.com:6881", nullptr ) == BootstrapParseStatus::Ok;
}

static bool test_server_extra_colon_rejected()
{
	return ParseServer( L"B host:extra:6881", nullptr ) == BootstrapParseStatus::Invalid
		&& ParseServer( L"B [::1]:6881", nullptr ) == BootstrapParseStatus::Invalid;
}

static bool test_service_uhc_port_range()
{
	return ParseService( L"U uhc:1.uhc.gtk-gnutella.nl:0", nullptr ) == BootstrapParseStatus::Invalid
		&& ParseService( L"U uhc:1.uhc.gtk-gnutella.nl:65536", nullptr ) == BootstrapParseStatus::Invalid
		&& ParseService( L"U uhc:1.uhc.gtk-gnutella.nl:19104:99", nullptr ) == BootstrapParseStatus::Invalid
		&& ParseService( L"U uhc:1.uhc.gtk-gnutella.nl:19104", nullptr ) == BootstrapParseStatus::Ok
		&& ParseService( L"U ukhl:cache.example.net:6346", nullptr ) == BootstrapParseStatus::Ok
		&& ParseService( L"U ftp:example.net:3558", nullptr ) == BootstrapParseStatus::Invalid;
}

static bool test_dht_boot_slot_cap_and_dns_budget()
{
	BootstrapDhtBootSlot oSlots[ BootstrapDhtRouterPingCap ] = {};
	int nBoot = 0;
	for ( int i = 0; i < BootstrapDhtRouterPingCap; ++i )
	{
		if ( ! BootstrapDhtAppendBootSlot( oSlots, BootstrapDhtRouterPingCap, &nBoot,
			0ul, 6881, true ) )
			return false;
	}
	if ( BootstrapDhtAppendBootSlot( oSlots, BootstrapDhtRouterPingCap, &nBoot, 0ul, 6881, true ) )
		return false;
	if ( nBoot != BootstrapDhtRouterPingCap )
		return false;
	if ( BootstrapDhtUseStoredAddr( oSlots[ 0 ].nAddr ) )
		return false;
	if ( ! BootstrapDhtTryBlockingResolve( 0ul, true, 0 ) )
		return false;
	if ( BootstrapDhtTryBlockingResolve( 0ul, true, BootstrapDhtBlockingResolveCap ) )
		return false;
	if ( ! BootstrapDhtUseStoredAddr( 0x7f000001ul ) )
		return false;
	if ( BootstrapDhtTryBlockingResolve( 0x7f000001ul, true, 0 ) )
		return false;
	return true;
}

static bool test_catalogue_truncated_rows()
{
	// The legacy loader skips short service rows; none may become an endpoint.
	return ParseService( L"D", nullptr ) != BootstrapParseStatus::Ok
		&& ParseService( L"K ", nullptr ) != BootstrapParseStatus::Ok
		&& ParseService( L"U uhc:", nullptr ) != BootstrapParseStatus::Ok
		&& ParseService( L"D https:/", nullptr ) == BootstrapParseStatus::Invalid
		&& ParseServer( L"B ", nullptr ) == BootstrapParseStatus::Invalid
		&& ParseServer( L"B router.bittorrent.com:", nullptr ) == BootstrapParseStatus::Invalid;
}

static bool test_catalogue_network_limits()
{
	return BootstrapMinWebCaches == 1 && BootstrapMinG2Services == 3
		&& BootstrapMinG1Services == 2 && BootstrapMinG1UdpServices == 1
		&& BootstrapMinEd2kMet == 2
		&& BootstrapMinDcHublists == 2 && BootstrapMinKadNodesDat == 1
		&& BootstrapDhtRouterPingCap == 8 && BootstrapDhtBlockingResolveCap == 3;
}

static bool test_catalogue_path_falls_back_to_packaged_defaults()
{
	const wchar_t* const writablePath = L"C:\\UserData\\Data\\DefaultServices.dat";
	const wchar_t* const packagedPath = L"C:\\Program Files\\Envy\\Data\\DefaultServices.dat";

	// Fresh output and existing Discovery.dat / HostCache.dat caches both leave
	// the writable catalogue absent, so packaged defaults must still be selected.
	return BootstrapSelectCataloguePath( writablePath, false, packagedPath, true ) == packagedPath
		&& BootstrapSelectCataloguePath( writablePath, true, packagedPath, true ) == writablePath
		&& BootstrapSelectCataloguePath( writablePath, false, packagedPath, false ) == nullptr;
}

static bool test_packaged_catalogue_directory_is_beside_executable()
{
	// The module path names the executable, so packaged Data is a sibling
	// of Envy.exe rather than a child of it.
	const wchar_t* const modulePath = L"C:\\Program Files\\Envy\\Envy.exe";
	const std::wstring directory( modulePath, BootstrapModuleDirectoryLength( modulePath ) );
	return directory == L"C:\\Program Files\\Envy"
		&& directory + L"\\Data\\DefaultServices.dat" == L"C:\\Program Files\\Envy\\Data\\DefaultServices.dat"
		&& BootstrapModuleDirectoryLength( L"Envy.exe" ) == 0
		&& BootstrapModuleDirectoryLength( nullptr ) == 0;
}

static bool test_unrelated_gwcs_do_not_replace_g1_udp_bootstrap()
{
	const int nWebCaches = BootstrapMinWebCaches;
	const int nG2 = BootstrapMinG2Services;
	const int nG1 = BootstrapMinG1Services;
	const int nEd2k = BootstrapMinEd2kMet;
	const int nDc = BootstrapMinDcHublists;
	const int nKad = BootstrapMinKadNodesDat;

	return ! BootstrapDiscoveryMinimumsSatisfied(
		nWebCaches, nG2, nG1, 0, nEd2k, nDc, nKad, true, true, true, true )
		&& BootstrapDiscoveryMinimumsSatisfied(
			nWebCaches, nG2, nG1, BootstrapMinG1UdpServices,
			nEd2k, nDc, nKad, true, true, true, true )
		&& BootstrapDiscoveryMinimumsSatisfied(
			nWebCaches, nG2, 0, 0, nEd2k, nDc, nKad, false, true, true, true );
}

static bool test_legacy_g1_cache_needs_udp_top_up()
{
	std::wstring shipped;
	if ( ! LoadRepoDataFile( "DefaultServices.dat", &shipped ) )
		return false;

	int nWeb = 0, nG2 = 0, nG1 = 0, nG1Udp = 0, nMet = 0, nHub = 0, nNodesDat = 0;
	CountServices( shipped.c_str(), &nWeb, &nG2, &nG1, &nG1Udp, &nMet, &nHub, &nNodesDat );
	if (nG1Udp < BootstrapMinG1UdpServices)
		return false;

	const int nHistoricG1Gwc = BootstrapMinG1Services;
	const bool bHistoricCacheNeedsRepair = !BootstrapDiscoveryMinimumsSatisfied(
	    BootstrapMinWebCaches, BootstrapMinG2Services, nHistoricG1Gwc, 0,
	    BootstrapMinEd2kMet, BootstrapMinDcHublists, BootstrapMinKadNodesDat,
	    true, true, true, true);

	// Load() adds missing defaults and saves the augmented Discovery.dat. On the
	// next startup the persisted UHC row satisfies the separate UDP minimum.
	const int nPersistedG1 = nHistoricG1Gwc + nG1Udp;
	const bool bReloadIsComplete = BootstrapDiscoveryMinimumsSatisfied(
	    BootstrapMinWebCaches, BootstrapMinG2Services, nPersistedG1, nG1Udp,
	    BootstrapMinEd2kMet, BootstrapMinDcHublists, BootstrapMinKadNodesDat,
	    true, true, true, true);

	return bHistoricCacheNeedsRepair && bReloadIsComplete;
}

void register_bootstrap_catalog_smoke_tests( TestSuite& suite )
{
	suite.add_test( "bootstrap_service_valid_https", test_service_valid_https );
	suite.add_test( "bootstrap_service_comment", test_service_comment );
	suite.add_test( "bootstrap_service_blank", test_service_blank );
	suite.add_test( "bootstrap_service_unknown_type", test_service_unknown_type );
	suite.add_test( "bootstrap_service_invalid_url", test_service_invalid_url );
	suite.add_test( "bootstrap_service_whitespace_endpoint", test_service_whitespace_endpoint );
	suite.add_test( "bootstrap_service_uhc", test_service_uhc );
	suite.add_test( "bootstrap_server_valid_dht", test_server_valid_dht );
	suite.add_test( "bootstrap_server_priority_and_comment", test_server_priority_and_comment );
	suite.add_test( "bootstrap_server_comment_line", test_server_comment_line );
	suite.add_test( "bootstrap_server_missing_port", test_server_missing_port );
	suite.add_test( "bootstrap_server_unknown_type", test_server_unknown_type );
	suite.add_test( "bootstrap_dedup_case_insensitive", test_dedup_case_insensitive );
	suite.add_test( "bootstrap_fold_ascii_only", test_fold_ascii_only );
	suite.add_test( "bootstrap_kad_nodes_dat_rejects_http", test_kad_nodes_dat_rejects_http_url );
	suite.add_test( "bootstrap_kad_nodes_dat_accepts_https", test_kad_nodes_dat_accepts_https_url );
	suite.add_test( "bootstrap_shipped_catalogue_meets_minima", test_shipped_catalogue_meets_minima );
	suite.add_test( "bootstrap_shipped_catalogue_no_static_ed2k_ip", test_shipped_catalogue_no_static_ed2k_ip );
	suite.add_test( "bootstrap_shipped_catalogue_no_getenvy_gwc", test_shipped_catalogue_no_getenvy_gwc );
	suite.add_test( "bootstrap_shipped_default_services_dat", test_shipped_default_services_dat );
	suite.add_test( "bootstrap_shipped_default_servers_dat", test_shipped_default_servers_dat );
	suite.add_test( "bootstrap_service_https_scheme_case", test_service_https_scheme_case );
	suite.add_test( "bootstrap_server_tab_separator", test_server_tab_separator );
	suite.add_test( "bootstrap_server_trailing_note", test_server_trailing_note );
	suite.add_test( "bootstrap_server_port_range", test_server_port_range );
	suite.add_test( "bootstrap_server_extra_colon_rejected", test_server_extra_colon_rejected );
	suite.add_test( "bootstrap_service_uhc_port_range", test_service_uhc_port_range );
	suite.add_test( "bootstrap_dht_boot_slot_cap_and_dns_budget", test_dht_boot_slot_cap_and_dns_budget );
	suite.add_test( "bootstrap_catalogue_truncated_rows", test_catalogue_truncated_rows );
	suite.add_test( "bootstrap_catalogue_network_limits", test_catalogue_network_limits );
	suite.add_test( "bootstrap_catalogue_path_fallback", test_catalogue_path_falls_back_to_packaged_defaults );
	suite.add_test( "bootstrap_packaged_catalogue_beside_executable", test_packaged_catalogue_directory_is_beside_executable );
	suite.add_test( "bootstrap_required_g1_udp_category", test_unrelated_gwcs_do_not_replace_g1_udp_bootstrap );
	suite.add_test( "bootstrap_legacy_g1_cache_needs_udp_top_up", test_legacy_g1_cache_needs_udp_top_up );
}
