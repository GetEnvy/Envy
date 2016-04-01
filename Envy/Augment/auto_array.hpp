//
// Augment/Auto_array.hpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010,2016 and Shareaza 2002-2007
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//
//

#pragma once

namespace augment
{

// delete, with compile-time type check, from boost::checked_delete.hpp:
template<class T> inline void checked_array_delete(T * x)
{
	typedef char type_must_be_complete[ sizeof(T)? 1: -1 ];
	(void) sizeof(type_must_be_complete);
	delete [] x;
}

template<typename T>
class auto_array
{
public:
	typedef T element_type;

private:
	element_type* ptr_;

private:
	struct auto_array_ref
	{
		explicit auto_array_ref(element_type** ref) throw()
			: ref_( ref )
		{}
		element_type* release() throw()
		{
			element_type* ptr = *ref_;
			*ref_ = NULL;
			return ptr;
		}
		element_type** ref_;
	};

public:
	explicit auto_array(element_type* ptr = NULL) throw()
		: ptr_( ptr )
	{}
	auto_array(auto_array& other) throw()
		: ptr_( other.release() )
	{}
	auto_array(auto_array_ref other) throw()
		: ptr_( other.release() )
	{}
	~auto_array() throw()
	{
		if ( get() != NULL )
			checked_array_delete( get() );		// Was boost::
	};

	auto_array& operator=(auto_array& other) throw()
	{
		reset( other.release() );
		return *this;
	}
	auto_array& operator=(auto_array_ref other) throw()
	{
		reset( other.release() );
		return *this;
	}

	element_type* get() const throw()
	{
		return ptr_;
	}
	element_type& operator[](std::size_t index) const throw()
	{
		return get()[ index ];
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
			checked_array_delete( get() );		// Was boost::
		ptr_ = ptr;
	}

	operator auto_array_ref() throw()
	{
		return auto_array_ref( &ptr_ );
	}
};

} // namespace augment
