
// ObjStream.cpp
// Base class for parcel serializers.
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

#include "StdAfx.h"

#include "../Util/Str.h"

#include "ObjStream.h"

namespace Str = HoverRace::Util::Str;

namespace HoverRace {
namespace Parcel {

ObjStreamExn::ObjStreamExn(const Util::OS::path_t &path, const std::string &details) :
	SUPER((const char*)Str::PU(path.file_string().c_str()))
{
	std::string &msg = GetMessage();
	msg += ": ";
	msg += details;
}

}  // namespace Parcel
}  // namespace HoverRace
