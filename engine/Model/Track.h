
// Track.h
//
// Copyright (c) 2010, 2014 Michael Imamura.
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

#include "../Display/Res.h"
#include "../Vec.h"

#include "TrackEntry.h"

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
		class Level;
	}
	namespace Parcel {
		class RecordFile;
		typedef std::shared_ptr<RecordFile> RecordFilePtr;
	}
}

namespace HoverRace {
namespace Model {

/**
 * A track level.
 * @author Michael Imamura
 */
class MR_DllDeclare Track : public Util::Inspectable
{
	typedef Util::Inspectable SUPER;
	private:
		Track() = delete;
	public:
		Track(const std::string &name, Parcel::RecordFilePtr recFile);
		virtual ~Track();

		Parcel::RecordFilePtr GetRecordFile() const { return recFile; }
		const TrackEntry &GetHeader() const { return header; }
		Level *GetLevel() const { return level; }

		/**
		 * Retrieve the coordinates of the north-west corner of the track.
		 * @return The coordinates, or (0, 0) if the track hasn't been loaded.
		 */
		const Vec2 &GetOffset() const { return offset; }

		/**
		 * Retrieve the size of the track.
		 * @return The size, or (0, 0) if the track hasn't been loaded.
		 */
		const Vec2 &GetSize() const { return size; }

		std::shared_ptr<Display::Res<Display::Texture>> GetMap() const { return map; }

		double GetGravity() const;
		void SetGravity(double gravity);

		virtual void Inspect(Util::InspectMapNode &node) const;

	private:
		void LoadLevel(bool allowRendering, char gameOpts);
		void LoadMap();

	public:
		void Load(bool allowRendering, char gameOpts);

	private:
		Parcel::RecordFilePtr recFile;
		TrackEntry header;
		Level *level;
		Vec2 offset;
		Vec2 size;
		std::shared_ptr<Display::Res<Display::Texture>> map;
};
typedef std::shared_ptr<Track> TrackPtr;

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
