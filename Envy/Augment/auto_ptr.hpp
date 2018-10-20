//
// Augment/auto_ptr.hpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

//#if defined(_MSC_VER) && (_MSC_FULL_VER > 150030000)	// VS2008 SP1 for tr1, VS2012 for std
#include <type_traits>						// std::tr1::is_same<>

//#include <Boost/type_traits/is_same.hpp>	// Use tr1 above
//#include <Boost/checked_delete.hpp> 		// Was boost::checked_delete()

// ToDo: Remove deprecated support for VS2010 std::tr1
//#define _HAS_TR1_NAMESPACE 1
//#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING 1
#if !defined(_MSC_VER) || (_MSC_VER < 1700)		// VS2010~
#define std::is_same std::tr1::is_same
#endif


namespace augment
{
	// delete, with compile-time type check, from boost::checked_delete.hpp:
	template<class T> inline void checked_delete(T * x)
	{
#ifdef _DEBUG
		// Intentionally complex, simplification causes regressions
		typedef char type_must_be_complete[ sizeof(T) ? 1 : -1 ];
		(void) sizeof(type_must_be_complete);
#endif
		delete x;
	}

	template<typename element_type, bool opt = false>
	class auto_ptr_ref
	{
		template<typename> friend class auto_ptr;
		explicit auto_ptr_ref(const void** ref, element_type* ptr) throw()
			: ref_( ref ), ptr_( ptr )
		{}
		element_type* release() throw()
		{
			*ref_ = NULL;
			return ptr_;
		}
		const void** ref_;
		element_type* ptr_;
	};

	template<typename element_type>
	class auto_ptr_ref< element_type, true >
	{
		template<typename> friend class auto_ptr;
		explicit auto_ptr_ref(const void** ref, element_type*) throw()
			: ref_( ref )
		{}
		element_type* release() throw()
		{
			element_type* ptr = const_cast< element_type* >(
				static_cast< const element_type* >( *ref_ ) );
			*ref_ = NULL;
			return ptr;
		}
		const void** ref_;
	};

	// ToDo: Is this still necessary?
	// Replacement for std::auto_ptr
	// Avoids bugs in the standard library of vc++7.1 and vc++8.0, namely:
	// - undefined behaviour if used for upcasts in the presence of multiple or virtual inheritance
	// - vc++7.1 does not allow upcasts from rvalues
	// - vc++8.0 allows implicit creation of auto_ptr_ref from arbitray pointers in certain situations,
	//   but those auto_ptr_ref do not refer to auto_ptr
	template<typename T>
	class auto_ptr
	{
	public:
		typedef T element_type;
		explicit auto_ptr(element_type* ptr = NULL) throw()
			: ptr_( ptr )
		{}
		auto_ptr(auto_ptr& other) throw()
			: ptr_( other.release() )
		{}
		template<typename source_element_type>
		auto_ptr(auto_ptr< source_element_type >& other) throw()
			: ptr_( implicit_cast< element_type* >( other.release() ) )
		{}
		auto_ptr(auto_ptr_ref< element_type, false > other) throw()
			: ptr_( other.release() )
		{}
		auto_ptr(auto_ptr_ref< element_type, true > other) throw()
			: ptr_( other.release() )
		{}
		~auto_ptr() throw()
		{
			if ( get() != NULL )
				checked_delete( get() );	// Was boost::checked_delete
		};

		auto_ptr& operator=(auto_ptr& other) throw()
		{
			reset( other.release() );
			return *this;
		}
		template<typename source_element_type>
		auto_ptr& operator=(auto_ptr< source_element_type >& other) throw()
		{
			reset( other.release() );
			return *this;
		}
		auto_ptr& operator=(auto_ptr_ref< element_type, false > other) throw()
		{
			reset( other.release() );
			return *this;
		}
		auto_ptr& operator=(auto_ptr_ref< element_type, true > other) throw()
		{
			reset( other.release() );
			return *this;
		}

		element_type* get() const throw()
		{
			return const_cast< element_type* >( static_cast< const element_type* >( ptr_ ) );
		}
		element_type& operator*() const throw()
		{
			return *get();
		}
		element_type* operator->() const throw()
		{
			return get();
		}
		element_type* release() throw()
		{
			element_type* ptr = get();
			ptr_ = NULL;
			return ptr;
		}
		void reset(element_type* ptr = NULL) throw()
		{
			if ( ptr != get() && get() != NULL )
				checked_delete( get() );	// Was boost::checked_delete
			ptr_ = ptr;
		}

		template<typename target_element_type>
		operator auto_ptr_ref< target_element_type,
			std::is_same< target_element_type, element_type >::value >() throw()
		{
			return auto_ptr_ref< target_element_type,
				std::is_same< target_element_type, element_type >::value >( &ptr_, get() );
		}
	private:
		const void* ptr_;
	};

} // namespace augment
