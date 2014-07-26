
// Res.h
//
// Copyright (c) 2014 Michael Imamura.
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

#include "../Util/MR_Types.h"
#include "../Exception.h"

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
		class Texture;
	}
}


namespace HoverRace {
namespace Display {

class ResLoadExn;

class BaseRes
{
	public:
		virtual ~BaseRes() { }

	public:
		/**
		 * Retrieve the ID for this resource.
		 *
		 * The ID should be unique for every resource being pointed at,
		 * so two instances that point to the same resource (e.g. a file)
		 * should return the same ID.
		 *
		 * This allows loaded resources to be cached, so if two instances
		 * point to, say, the same file, then the resource will be loaded once
		 * (although this is left as an exercise for the Display).
		 *
		 * @return The identifier.
		 */
		virtual std::string GetId() const = 0;

		/**
		 * Open the stream for reading.
		 * @return The input stream.
		 * @throw ResourceLoadExn
		 */
		virtual std::unique_ptr<std::istream> Open() const = 0;
};

/**
 * Base class for loadable resources.
 * @author Michael Imamura
 */
template<class T>
class Res : public BaseRes
{
	public:
		virtual ~Res() { }
};

/**
 * Base class for loadable textures.
 * @author Michael Imamura
 */
template<>
class Res<Texture> : public BaseRes
{
	public:
		virtual ~Res() { }

	public:
		struct ImageData
		{
			void *pixels;
			int width;
			int height;
			int depth;
			int pitch;
			MR_UInt32 rMask;
			MR_UInt32 gMask;
			MR_UInt32 bMask;
			MR_UInt32 aMask;
		};

	public:
		virtual bool IsGenerated() const
		{
			return false;
		}

		virtual const ImageData *GetImageData()
		{
			return nullptr;
		}
};

/**
 * A resource failed to load.
 * @author Michael Imamura
 */
class ResLoadExn : Exception
{
	typedef Exception SUPER;

	public:
		ResLoadExn() : SUPER() { }
		ResLoadExn(const std::string &msg) : SUPER(msg) { }
		ResLoadExn(const char *msg) : SUPER(msg) { }
		virtual ~ResLoadExn() noexcept { }
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
