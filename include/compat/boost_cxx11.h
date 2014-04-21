
#pragma once

#include <boost/config.hpp>
#include <boost/version.hpp>
#include <memory>

// Support for C++11 smart pointers.
// These overloads are included in Boost 1.53.
// See: https://svn.boost.org/trac/boost/ticket/4185
#if BOOST_VERSION < 105300 || defined(BOOST_NO_CXX11_SMART_PTR)
namespace boost {
	template<class T> T * get_pointer( std::unique_ptr<T> const& p )
	{
		return p.get();
	}
	template<class T> T * get_pointer( std::shared_ptr<T> const& p )
	{
		return p.get();
	}
}
#endif
