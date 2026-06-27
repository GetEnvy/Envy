//
// Hashes.hpp
//
// Hash type definitions for Envy
// Provides typedefs for commonly used hash types
//

#pragma once

#include "Hashes/Hash.hpp"
#include "Hashes/HashDescriptors.hpp"
#include "Hashes/StoragePolicies.hpp"
#include "Hashes/CheckingPolicies.hpp"
#include "Hashes/ValidationPolicies.hpp"
#include "Hashes/Compatibility.hpp"

namespace Hashes
{
	//! \brief The default SHA1 hash type.
	typedef Hash< Policies::Sha1Descriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::BasicValidation > Sha1Hash;
	//! \brief The SHA1 hash type suitable to represent conditions in (partial) file objects.
	typedef Hash< Policies::Sha1Descriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::ExtendedValidation > Sha1ManagedHash;

	//! \brief The default Tiger hash type.
	typedef Hash< Policies::TigerDescriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::BasicValidation > TigerHash;
	//! \brief The Tiger hash type suitable to represent conditions in (partial) file objects.
	typedef Hash< Policies::TigerDescriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::ExtendedValidation > TigerManagedHash;

	//! \brief The default ED2K hash type.
	typedef Hash< Policies::Ed2kDescriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::BasicValidation > Ed2kHash;
	//! \brief The ED2K hash type suitable to represent conditions in (partial) file objects.
	typedef Hash< Policies::Ed2kDescriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::ExtendedValidation > Ed2kManagedHash;

	//! \brief The default MD5 hash type.
	typedef Hash< Policies::Md5Descriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::BasicValidation > Md5Hash;
	//! \brief The MD5 hash type suitable to represent conditions in (partial) file objects.
	typedef Hash< Policies::Md5Descriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::ExtendedValidation > Md5ManagedHash;

	//! \brief The default Bittorrent info hash type.
	typedef Hash< Policies::BthDescriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::BasicValidation > BtHash;
	//! \brief The Bittorent info hash type suitable to represent conditions in (partial) file objects.
	typedef Hash< Policies::BthDescriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::ExtendedValidation > BtManagedHash;
	//! \brief This Bittorrent hash type is useful to represent the hash of
	//!		a verification block (technically it's SHA1), no overhead
	typedef Hash< Policies::BthDescriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::NoValidation > BtPureHash;

	//! \brief The default SHA256 hash type (BitTorrent v2 support).
	typedef Hash< Policies::Sha256Descriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::BasicValidation > Sha256Hash;
	//! \brief The SHA256 hash type suitable to represent conditions in (partial) file objects.
	typedef Hash< Policies::Sha256Descriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::ExtendedValidation > Sha256ManagedHash;

	//! \brief The Guid hash type.
	typedef Hash< Policies::GuidDescriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::BasicValidation > Guid;

	//! \brief The Bittorrent Guid hash type.
	typedef Hash< Policies::BtGuidDescriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::BasicValidation > BtGuid;

	//! \brief Generates Guid from extended Bittorrent Guid
	inline Guid transformGuid(const BtGuid& other)
	{
		Guid result;
		memcpy( &result[ 0 ], &other[ 0 ], result.byteCount );
		result.validate();
		return result;
	}

	//! \brief Generates Bittorrent Guid from Guid, using Envy's special signature.
	inline BtGuid transformGuid(const Guid& other)
	{
		BtGuid result;
		memcpy( &result[ 0 ], &other[ 0 ], result.byteCount );
		*( result.end() - 1 ) = *other.begin() ^ swapEndianess( *( other.end() - 1 ) );
		result.validate();
		return result;
	}

	//! \brief Predicate checking if a given Bittorrent Guid uses a
	//!		special signature to signal it belongs to Envy/Shareaza client.
	inline bool isExtendedBtGuid(const BtGuid& hash)
	{
		return hash.isValid() &&
			*( hash.end() - 1 ) == ( *hash.begin() ^ swapEndianess( *( hash.end() - 2 ) ) );
	}

} // namespace Hashes

//! Enumeration to select a specific hash type
enum
{
	HASH_NULL = 0,      //!< Unknown (any) hash type
	HASH_SHA1 = 1,      //!< use SHA1
	HASH_MD5 = 2,       //!< use MD5
	HASH_TIGERTREE = 3, //!< use Tiger tree hash
	HASH_ED2K = 4,      //!< use ED2K
	HASH_TORRENT = 5    //!< use Bittorrent info hash
};

// Make compatibility functions available in global namespace
using Hashes::SerializeOut;
using Hashes::SerializeIn;
using Hashes::transformGuid;
using Hashes::isExtendedBtGuid;

// MFC CMap HashKey specializations for Hashes::Guid
// Required for using Guid as a key type in CMap containers
template<> AFX_INLINE UINT AFXAPI HashKey(Hashes::Guid key)
{
	// Use first 4 bytes of the GUID as hash value
	return *reinterpret_cast<const UINT*>(&key[0]);
}

template<> AFX_INLINE UINT AFXAPI HashKey(Hashes::Guid& key)
{
	return *reinterpret_cast<const UINT*>(&key[0]);
}

template<> AFX_INLINE UINT AFXAPI HashKey(const Hashes::Guid& key)
{
	return *reinterpret_cast<const UINT*>(&key[0]);
}
