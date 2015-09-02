
// Bundle.cpp
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

#include "../Util/Str.h"
#include "ClassicRecordFile.h"

#include "Bundle.h"

namespace fs = boost::filesystem;

using namespace HoverRace::Util;

namespace HoverRace {
namespace Parcel {

const Bundle::iterator Bundle::END;

/**
 * Constructor.
 * @param dir The directory (does not need to exist).
 * @param subBundle Optional fall-back bundle.
 */
Bundle::Bundle(const OS::path_t &dir, std::shared_ptr<Bundle> subBundle) :
	dir(dir), subBundle(std::move(subBundle))
{
}

/**
 * Open an existing parcel.
 * All parcels, including sub-bundles, will be searched.
 * @param name The name of the parcel.
 * @param writing @c true if the parcel will be written to,
 *                @c false if read-only.
 * @return The parcel (may be @c nullptr if parcel does not exist).
 */
std::shared_ptr<RecordFile> Bundle::OpenParcel(
	const std::string &name, bool writing) const
{
	OS::path_t pt = dir / Str::UP(name.c_str());

	if (fs::exists(pt)) {
		RecordFile *rec = new ClassicRecordFile();
		if (writing) {
			rec->OpenForWrite(pt);
		}
		else {
			rec->OpenForRead(pt);
		}
		return std::shared_ptr<RecordFile>(rec);
	}
	else {
		if (!subBundle) {
			return std::shared_ptr<RecordFile>();
		}
		else {
			return subBundle->OpenParcel(name, writing);
		}
	}
}

// class Iterator

const OS::dirIter_t Bundle::Iterator::END;

Bundle::Iterator::Iterator() :
	bundle(), iter(END)
{
}

Bundle::Iterator::Iterator(const Bundle *bundle) :
	bundle(bundle), iter(END)
{
	FindNextValidBundle();
	if (this->bundle)
		iter = OS::dirIter_t(this->bundle->dir);
}

Bundle::Iterator &Bundle::Iterator::operator++()
{
	if (iter != END) {
		++iter;
		if (iter == END) {
			bundle = bundle->subBundle.get();
			FindNextValidBundle();
			iter = (!bundle) ? END : OS::dirIter_t(bundle->dir);
		}
	}
	return *this;
}

Bundle::Iterator Bundle::Iterator::operator++(int)
{
	Bundle::Iterator retv = *this;
	++(*this);
	return retv;
}

void Bundle::Iterator::FindNextValidBundle()
{
	if (!bundle) return;

	while (!(fs::exists(bundle->dir) &&
		fs::is_directory(bundle->dir) &&
		!fs::is_empty(bundle->dir)))
	{
		bundle = bundle->subBundle.get();
		if (!bundle) return;
	}
}

}  // namespace Parcel
}  // namespace HoverRace
