
// TrackEntry.h
// Track metadata.
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

#include "../Util/Inspectable.h"
#include "../Util/MR_Types.h"
#include "../Util/OS.h"

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
	namespace Parcel {
		class ObjStream;
	}
	namespace Util {
		class InspectMapNode;
	}
}

namespace HoverRace {
namespace Model {

/**
 * The metadata for a track.
 * @author Michael Imamura
 */
class MR_DllDeclare TrackEntry : public Util::Inspectable
{
	typedef Util::Inspectable SUPER;
	public:
		TrackEntry() : SUPER() { }
		virtual ~TrackEntry() { }

		void Serialize(Parcel::ObjStream &os);

		virtual void Inspect(Util::InspectMapNode &node) const;

		bool operator<(const TrackEntry &elem2) const
		{
			int diff = sortingIndex - elem2.sortingIndex;
			if (diff == 0) {
				return name < elem2.name;
			}
			else {
				return (diff < 0);
			}
		}

		bool operator==(const TrackEntry &elem2) const
		{
			return ((name == elem2.name) && 
					(description == elem2.description));
		}

	public:
		std::string name;
#		ifdef _DEBUG
			Util::OS::path_t path;  // For parcel debugging.
#		endif
		std::string description;
		MR_Int32 regMinor;
		MR_Int32 regMajor;
		MR_Int32 registrationMode;
		MR_Int32 sortingIndex;
};
typedef boost::shared_ptr<TrackEntry> TrackEntryPtr;

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
