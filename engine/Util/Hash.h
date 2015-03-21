
// Hash.h
//
// Copyright (c) 2015 Michael Imamura.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which
// you have taken this file. If you can not find the license you can not use
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions
// and limitations under the License.

#pragma once

namespace HoverRace {
namespace Util {

namespace Hash {

/**
 * Mix in a value into a hash.
 *
 * This can be used to build a hash value from multiple values.
 *
 * The algorithm is based on
 * <a href="http://www.boost.org/doc/libs/1_57_0/doc/html/hash/combine.html">boost::hash_combine</a>,
 * but uses std::hash instead of boost::hash.
 *
 * @param [in,out] seed The accumulating hash value.
 * @param v The value to mix in.
 */
template<class T>
inline void Combine(std::size_t &seed, const T &v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}  // namespace Hash

}  // namespace Util
}  // namespace HoverRace
