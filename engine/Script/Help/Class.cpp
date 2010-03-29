
// Class.cpp
// Help text for a class in the scripting API.
//
// Copyright (c) 2010 Michael Imamura.
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

#include "StdAfx.h"

#include "Class.h"

namespace HoverRace {
namespace Script {
namespace Help {

/**
 * Constructor.
 * @param name The name of the class.
 */
Class::Class(const std::string &name) :
	name(name)
{
}

Class::~Class()
{
}

const std::string &Class::GetName() const
{
	return name;
}

const Class::methods_t &Class::GetMethods() const
{
	return methods;
}

void Class::AddMethod(MethodPtr method)
{
	if (method == NULL) return;

	methods.insert(methods_t::value_type(method->GetName(), method));
}

MethodPtr Class::GetMethod(const std::string &methodName) const
{
	methods_t::const_iterator iter = methods.find(methodName);
	return (iter == methods.end()) ? MethodPtr() : iter->second;
}


}  // namespace Help
}  // namespace Script
}  // namespace HoverRace
