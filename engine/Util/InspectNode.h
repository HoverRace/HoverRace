
// InspectNode.h
// Inspection state..
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

#pragma once

#include "Inspectable.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Util {

class InspectNode;
typedef boost::shared_ptr<InspectNode> InspectNodePtr;

class MR_DllDeclare InspectNode
{
	public:
		InspectNode();
		virtual ~InspectNode();

		void AddField(const std::string &name, const std::string &value);

		void AddSubobject(const std::string &name, const Inspectable *obj);

	private:
		typedef std::map<std::string,std::string> fields_t;
		fields_t fields;

		typedef std::map<std::string,InspectNodePtr> subobjects_t;
		subobjects_t subobjects;
};

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
