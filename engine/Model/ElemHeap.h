
// ElemHeap.h
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
	namespace Model {
		class FreeElement;
	}
	namespace Parcel {
		class ObjStream;
	}
}

namespace HoverRace {
namespace Model {

/**
 * A managed pile of FreeElement instances.
 *
 * For now, we manage the instances in a similar way to Level::FreeElementList.
 * This abstraction is to allow for future changes in memory management.
 *
 * @author Michael Imamura
 */
class ElemHeap
{
public:
	ElemHeap() { }
	ElemHeap(const ElemHeap&) = default;
	ElemHeap(ElemHeap&&) = default;
	~ElemHeap() { }

	ElemHeap &operator=(const ElemHeap&) = default;
	ElemHeap &operator=(ElemHeap&&) = default;

private:
	using elems_t = std::list<std::shared_ptr<FreeElement>>;
public:
	using Handle = elems_t::const_iterator;

public:
	Handle Add(std::shared_ptr<FreeElement> elem)
	{
		elems.emplace_back(std::move(elem));
		return --elems.cend();
	}

public:
	static std::unique_ptr<ElemHeap> StreamIn(Parcel::ObjStream &pArchive);
	void StreamOut(Parcel::ObjStream &pArchive) const;

private:
	elems_t elems;
};

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
