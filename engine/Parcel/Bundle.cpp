
// Bundle.cpp
// A source of parcels.
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
#include "ClassicRecordFile.h"
#ifdef _WIN32
#	include "MfcRecordFile.h"
#endif

#include "Bundle.h"

namespace fs = boost::filesystem;

using namespace HoverRace::Util;

namespace HoverRace {
namespace Parcel {

/**
 * Constructor.
 * @param dir The directory (does not need to exist).
 * @param subBundle Optional fall-back bundle.
 */
Bundle::Bundle(const OS::path_t &dir, BundlePtr subBundle) :
	dir(dir), subBundle(subBundle)
{
}

/**
 * Open an existing parcel.
 * All parcels, including sub-bundles, will be searched.
 * @param name The name of the parcel.
 * @param writing @c true if the parcel will be written to, @c false if read-only.
 * @return The parcel (may be @c NULL if parcel does not exist).
 */
RecordFilePtr Bundle::OpenParcel(const std::string &name, bool writing) const
{
	OS::path_t pt = dir / Str::UP(name.c_str());

	if (fs::exists(pt)) {
#		ifdef _WIN32
			RecordFile *rec = MfcRecordFile::New();
#		else
			RecordFile *rec = new ClassicRecordFile();
#		endif
		if (writing) {
			rec->OpenForWrite(Str::PU(pt.file_string().c_str()));
		}
		else {
			rec->OpenForRead(Str::PU(pt.file_string().c_str()));
		}
		return RecordFilePtr(rec);
	}
	else {
		if (subBundle.get() == NULL) {
			return RecordFilePtr();
		}
		else {
			return subBundle->OpenParcel(name, writing);
		}
	}
}

}  // namespace Parcel
}  // namespace HoverRace
