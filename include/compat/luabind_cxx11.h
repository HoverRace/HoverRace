
#pragma once

#include <memory>

// See boost_cxx11.h.
// Luabind's has_get_pointer_ doesn't notice the definitions in the boost
// namespace, even though it'll use them.  We don't actually need to
// define these, just declare them so it passes has_get_pointer_.
namespace luabind {
namespace detail {
namespace has_get_pointer_ {

template<class T> T * get_pointer( std::unique_ptr<T> const& p );
template<class T> T * get_pointer( std::shared_ptr<T> const& p );

}  // namespace has_get_pointer_
}  // namespace detail
}  // namespace luabind
