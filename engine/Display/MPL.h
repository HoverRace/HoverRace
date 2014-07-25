
// MPL.h
//
// Copyright (c) 2014 Michael Imamura.
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

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Display {
/// Display-oriented metaprogramming utilities.
namespace MPL {

/// Determine if a class as a SetSize(Vec2).
template<class C>
struct HasSetSize
{
	private:
		template<class T>
		static auto check(T*) ->
			decltype(std::declval<T>().SetSize(std::declval<Vec2>()),
			std::true_type());

		template<class>
		static std::false_type check(...);

		typedef decltype(check<C>(nullptr)) type;

	public:
		static const bool value = type::value;
};

template<class T>
typename std::enable_if<MPL::HasSetSize<T>::value, void>::type
inline SetSize(T &widget, double w, double h)
{
	widget.SetSize(Vec2(w, h));
}

template<class T>
typename std::enable_if<MPL::HasSetSize<T>::value, void>::type
inline SetSize(T &widget, Vec2 &&vec)
{
	widget.SetSize(std::forward<Vec2>(vec));
}

template<class T>
typename std::enable_if<!MPL::HasSetSize<T>::value, void>::type
inline SetSize(T&, double, double)
{
	// Do nothing.
}

template<class T>
typename std::enable_if<!MPL::HasSetSize<T>::value, void>::type
inline SetSize(T&, Vec2&&)
{
	// Do nothing.
}

}  // namespace MPL
}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
