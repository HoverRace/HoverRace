
// SpriteTextureRes.cpp
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

#include "StdAfx.h"

#include "../engine/Display/Texture.h"
#include "../engine/Parcel/ObjStream.h"
#include "../engine/Util/Log.h"

#include "SpriteTextureRes.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param recordName The name of the record being loaded.
 * @param archive The input object stream.
 */
SpriteTextureRes::SpriteTextureRes(const std::string &recordName,
	Parcel::ObjStream &archive) :
	SUPER(),
	id("spriteTexture:" + recordName)
{
	imageData.pixels = nullptr;

	MR_UInt32 numItems, itemHeight, totalHeight, width;

	archive >> numItems >> itemHeight >> totalHeight >> width;

	imageData.pixels = malloc(width * totalHeight);
	imageData.width = width;
	imageData.height = totalHeight;
	imageData.depth = 8;
	imageData.pitch = width;
	imageData.rMask = 0;
	imageData.gMask = 0;
	imageData.bMask = 0;
	imageData.aMask = 0;

	try {
		// Sprites are stored upside-down.
		MR_UInt8 *buf = static_cast<MR_UInt8*>(imageData.pixels);
		buf += (totalHeight - 1) * width;
		for (unsigned row = 0; row < totalHeight; ++row) {
			archive.Read(buf, width);
			buf -= width;
		}
	}
	catch (Parcel::ObjStreamExn&) {
		free(imageData.pixels);
		throw;
	}

	Log::Debug("Loaded sprite resource (%ux%u, %u items): %s",
		width, totalHeight, numItems, recordName.c_str());
}

SpriteTextureRes::~SpriteTextureRes()
{
	if (imageData.pixels) {
		free(imageData.pixels);
	}
}

}  // namespace Display
}  // namespace HoverRace
