// ControlAction.h
// Define an action to be performed by a control.  This is essentially a functor,
// and should be bound to a control.
//
// Copyright (c) 2010 Ryan Curtin
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
//

#pragma once

#include <string>

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
namespace Control {

/***
 * Abstract base class which allows us a simple reference to arbitrary functors
 * of type ControlActionImpl (which bind a class via a template parameter).
 * Unfortunately, this must use virtual functions to work correctly, which is
 * not optimal, but is the best the language can do.
 *
 * To create a functor, derive the ControlAction class.
 *
 * @author Ryan Curtin
 */
template<class T>
class ControlAction
{
public:
	ControlAction(const std::string &name, int listOrder) :
		name(name), listOrder(listOrder) { }
	virtual ~ControlAction() { }

	virtual void operator()(T eventValue) = 0;

	const std::string &GetName() { return name; }
	int GetListOrder() { return listOrder; }

protected:
	std::string name;
	int listOrder;
};
using ControlActionPtr = std::shared_ptr<ControlAction<int>>;

/***
 * An action performer that does absolutely nothing.
 * Provides a template for other classes that will be used as an
 * ActionPerformer.
 */
class MR_DllDeclare BlankAction : public ControlAction<int>
{
public:
	BlankAction(std::string name, int listOrder) :
		ControlAction(name, listOrder) { }

	virtual void operator()(int) { }
};

} // namespace Control
} // namespace HoverRace

#undef MR_DllDeclare
