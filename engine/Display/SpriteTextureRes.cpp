
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

#include "../StdAfx.h"

#include "../Display/Texture.h"
#include "../Parcel/ObjStream.h"
#include "../Util/Log.h"

#include "SpriteTextureRes.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

namespace {
const MR_UInt32 MAX_TEXTURE_WIDTH = 4096;
const MR_UInt32 MAX_TEXTURE_HEIGHT = 4096;
}

/**
 * Constructor.
 * @param recordName The name of the record being loaded.
 * @param archive The input object stream.
 * @param flipped @c true if the sprite data is stored upside-down.
 */
SpriteTextureRes::SpriteTextureRes(const std::string &recordName,
	Parcel::ObjStream &archive, bool flipped) :
	SUPER(),
	id("spriteTexture:" + recordName), flipped(flipped)
{
	imageData.pixels = nullptr;

	MR_UInt32 numItems, itemHeight, totalHeight, width;

	archive >> numItems >> itemHeight >> totalHeight >> width;

	if (numItems == 0) {
		throw Parcel::ObjStreamExn(recordName + ": No items in sprite.");
	}
	else if (numItems > 1) {
		Log::Warn("%s: More than one item in sprite texture "
			"(entire sprite will be used): %u", recordName.c_str(), numItems);
	}

	if (width > MAX_TEXTURE_WIDTH || totalHeight > MAX_TEXTURE_HEIGHT) {
		throw Parcel::ObjStreamExn(boost::str(boost::format(
			"%s: Texture size (%ux%u) exceeds maximum size (%ux%u)") %
			recordName % width % totalHeight %
			MAX_TEXTURE_WIDTH % MAX_TEXTURE_HEIGHT));
	}

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
		if (flipped) {
			// Sprites are stored upside-down.
			MR_UInt8 *buf = static_cast<MR_UInt8*>(imageData.pixels);
			buf += (totalHeight - 1) * width;
			for (unsigned row = 0; row < totalHeight; ++row) {
				archive.Read(buf, width);
				buf -= width;
			}
		}
		else {
			// Sprites are not upside-down.
			archive.Read(imageData.pixels, width * totalHeight);
		}
	}
	catch (Parcel::ObjStreamExn&) {
		free(imageData.pixels);
		imageData.pixels = nullptr;
		throw;
	}

	HR_LOG(debug) << "Loaded sprite resource "
		"(" << width << "x" << totalHeight << ", " <<
		numItems << " items): " << recordName;
}

SpriteTextureRes::~SpriteTextureRes()
{
	if (imageData.pixels) {
		free(imageData.pixels);
	}
}

}  // namespace Display
}  // namespace HoverRace
