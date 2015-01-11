
// Container.h
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

#include "BaseContainer.h"

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
		class Display;
	}
}

namespace HoverRace {
namespace Display {

/**
 * An invisible container for other UI widgets.
 * @author Michael Imamura
 */
class MR_DllDeclare Container : public BaseContainer
{
	using SUPER = BaseContainer;

public:
	Container(Display &display, uiLayoutFlags_t layoutFlags = 0) :
		SUPER(display, layoutFlags) { }
	Container(Display &display, const Vec2 &size, bool clip = true,
		uiLayoutFlags_t layoutFlags = 0) :
		SUPER(display, size, clip, layoutFlags) { }
	virtual ~Container() { }

public:
	// Public forwards of container manipulation functions.

	/// @copydoc BaseContainer::NewChild
	template<class T, class... Args>
	typename std::enable_if<std::is_base_of<UiViewModel, T>::value,
		std::shared_ptr<T>>::type
	NewChild(Args&&... args)
	{
		return SUPER::NewChild<T>(std::forward<Args>(args)...);
	}

	/// @copydoc BaseContainer::InsertChild
	template<class T>
	HR_DEPRECATED
	typename std::enable_if<std::is_base_of<UiViewModel, T>::value,
		std::shared_ptr<T>
		>::type
	InsertChild(int pos, T *child)
	{
		return SUPER::InsertChild(pos, child);
	}

	/// @copydoc BaseContainer::RemoveChild
	template<class T>
	typename std::enable_if<std::is_base_of<UiViewModel, T>::value,
		std::shared_ptr<T>
		>::type
	RemoveChild(const std::shared_ptr<T> &child)
	{
		return SUPER::RemoveChild(child);
	}

	/// @copydoc BaseContainer::ReorderChild
	template<class T>
	typename std::enable_if<std::is_base_of<UiViewModel, T>::value,
		std::shared_ptr<T>
		>::type
	ReorderChild(const std::shared_ptr<T> &child, size_t idx)
	{
		return SUPER::ReorderChild(child, idx);
	}

	void Clear() override
	{
		SUPER::Clear();
	}
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
