
// InspectMapNode.h
// Inspection state for an object.
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

#include <boost/lexical_cast.hpp>

#include "InspectNode.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace yaml {
	class Emitter;
}

namespace HoverRace {
namespace Util {

class Inspectable;

class MR_DllDeclare InspectMapNode : public InspectNode
{
	typedef InspectNode SUPER;
	public:
		InspectMapNode();
		virtual ~InspectMapNode();

	protected:
		void AddStringField(const std::string &name, const std::string &value);

	public:
		void RenderToStream(std::ostream &os);
		void RenderToString(std::string &s);

	public:
		virtual void RenderToYaml(yaml::Emitter &emitter);

	public:
		template<typename T>
		InspectMapNode &AddField(const std::string &name, const T &value)
		{
			AddStringField(name, boost::lexical_cast<std::string>(value));
			return *this;
		}

		InspectMapNode &AddField(const std::string &name, const char *value)
		{
			AddStringField(name, value);
			return *this;
		}

		InspectMapNode &AddField(const std::string &name, const std::string &value)
		{
			AddStringField(name, value);
			return *this;
		}

		InspectMapNode &AddField(const std::string &name, bool value)
		{
			AddStringField(name, value ? "true" : "false");
			return *this;
		}

		InspectMapNode &AddSubobject(const std::string &name, const Inspectable *obj);

	private:
		typedef std::map<std::string,InspectNodePtr> fields_t;
		fields_t fields;
};
typedef boost::shared_ptr<InspectMapNode> InspectMapNodePtr;

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
