
// Track.h
//
// Copyright (c) 2010, 2014-2015 Michael Imamura.
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
		class FreeElementBase;
		class GameOptions;
		class Level;
	}
	namespace Parcel {
		class RecordFile;
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
	using SUPER = Util::Inspectable;

private:
	Track() = delete;
public:
	Track(const std::string &name,
		std::shared_ptr<Parcel::RecordFile> recFile =
			std::shared_ptr<Parcel::RecordFile>());
	virtual ~Track();

	Parcel::RecordFile *GetRecordFile() const { return recFile.get(); }
	std::shared_ptr<Parcel::RecordFile> ShareRecordFile() const { return recFile; }
	const TrackEntry &GetHeader() const { return header; }
	Level *GetLevel() const { return level.get(); }

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

	/**
	 * Gets the gravity multiplier.
	 * @return The gravity multiplier (1.0 is normal gravity).
	 */
	double GetGravity() const { return physics.gravity; }

	/**
	 * Set the current gravity multiplier.
	 * @param gravity The gravity multiplier (1.0 is normal gravity).
	 */
	void SetGravity(double gravity) { this->physics.gravity = gravity; }

	/**
	 * Add a FreeElement to be managed by this Track.
	 * @param elem The element.
	 */
	void AddFreeElement(std::shared_ptr<Model::FreeElementBase> elem)
	{
		freeElements.emplace_back(std::move(elem));
	}

public:
	virtual void Inspect(Util::InspectMapNode &node) const;

private:
	void LoadHeader();
	void LoadLevel(bool allowRendering, const GameOptions &gameOpts);
	void LoadMap();

public:
	void Load(bool allowRendering, const GameOptions &gameOpts);

private:
	std::shared_ptr<Parcel::RecordFile> recFile;
	TrackEntry header;
	std::unique_ptr<Level> level;
	Vec2 offset;
	Vec2 size;
	std::shared_ptr<Display::Res<Display::Texture>> map;
	std::list<std::shared_ptr<Model::FreeElementBase>> freeElements;

	struct Physics
	{
		Physics();

		double gravity;
	} physics;
};

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
