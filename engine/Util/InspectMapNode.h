
// InspectMapNode.h
//
// Copyright (c) 2010, 2015 Michael Imamura.
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
namespace Util {

class Inspectable;

class InspectScalarNode : public InspectNode
{
	using SUPER = InspectNode;

public:
	InspectScalarNode(const std::string &value) :
		SUPER(), value(value) { }
	virtual ~InspectScalarNode() { }

public:
	void RenderToYaml(yaml::Emitter &emitter) override;

private:
	std::string value;
};

class InspectSeqNode : public InspectNode
{
	using SUPER = InspectNode;

public:
	InspectSeqNode(size_t sz) : SUPER()
	{
		fields.reserve(sz);
	}
	virtual ~InspectSeqNode() { }

public:
	void Add(std::shared_ptr<InspectNode> s)
	{
		fields.emplace_back(std::move(s));
	}

public:
	void RenderToYaml(yaml::Emitter &emitter) override;

private:
	std::vector<std::shared_ptr<InspectNode>> fields;
};

/**
 * An inspection node which maps field names to values
 * (either strings or inspectable subobjects).
 * @author Michael Imamura
 */
class MR_DllDeclare InspectMapNode : public InspectNode
{
	using SUPER = InspectNode;

public:
	InspectMapNode() : SUPER() { }
	virtual ~InspectMapNode() { }

protected:
	void AddStringField(const std::string &name, const std::string &value);

public:
	void RenderToStream(std::ostream &os);
	void RenderToString(std::string &s);

public:
	void RenderToYaml(yaml::Emitter &emitter) override;

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

	template<typename T>
	InspectMapNode &AddArray(const std::string &name, T *elems,
		size_t startIndex, size_t size)
	{
		auto node = std::make_shared<InspectSeqNode>(size);
		for (size_t i = startIndex; i < size; ++i)
			node->Add(std::make_shared<InspectScalarNode>(
				boost::lexical_cast<std::string>(elems[i])));
		fields.emplace_back(name, node);
		return *this;
	}

	InspectMapNode &AddSubobject(const std::string &name, const Inspectable *obj);

private:
	std::vector<std::pair<std::string, std::shared_ptr<InspectNode>>> fields;
};

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
