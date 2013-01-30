
// ClassicRecordFile.cpp
// Standard HoverRace 1.x parcel format.
//
// Copyright (c) 2010, 2012 Michael Imamura.
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

#include "../Util/InspectMapNode.h"
#include "../Util/Str.h"
#include "ClassicObjStream.h"

#include "ClassicRecordFile.h"

using std::ios;

using namespace HoverRace::Util;

namespace HoverRace {
namespace Parcel {

// ClassicRecordFileHeader

class ClassicRecordFileHeader : public Util::Inspectable
{
	typedef Util::Inspectable SUPER;
	public:
		ClassicRecordFileHeader();
		ClassicRecordFileHeader(MR_UInt32 numRecords);
		virtual ~ClassicRecordFileHeader();

		void Serialize(ObjStream &os);

		virtual void Inspect(Util::InspectMapNode &node) const;

	public:
		std::string title;
		bool sumValid;
		MR_UInt32 checksum;
		MR_UInt32 recordsUsed;
		MR_UInt32 recordsMax;
		MR_UInt32 *recordList;
};

ClassicRecordFileHeader::ClassicRecordFileHeader() :
	SUPER(), sumValid(false), checksum(0),
	recordsUsed(0), recordsMax(0), recordList(NULL)
{
}

ClassicRecordFileHeader::ClassicRecordFileHeader(MR_UInt32 numRecords) :
	SUPER(), sumValid(false), checksum(0),
	recordsUsed(0), recordsMax(numRecords), recordList(new MR_UInt32[numRecords])
{
	ASSERT(numRecords > 0);

	for (MR_UInt32 i = 0; i < numRecords; ++i) {
		recordList[i] = 0;
	}
}

ClassicRecordFileHeader::~ClassicRecordFileHeader()
{
	delete[] recordList;
}

void ClassicRecordFileHeader::Serialize(ObjStream &os)
{
	if (os.IsWriting()) {
		BOOL sumValidLoad = FALSE;  //TODO

		os << title <<
			(MR_Int32) 0 << (MR_Int32) 0 <<
			sumValidLoad << checksum << recordsUsed << recordsMax <<
			(MR_Int32) 0 << (MR_Int32) 0;

		if (recordsMax > 0) {
			for (unsigned int i = 0; i < recordsMax; ++i) {
				os << recordList[i];
			}
		}
	}
	else {
		delete[] recordList;
		recordList = NULL;

		MR_UInt32 dummy;
		BOOL sumValidLoad;

		os >> title >>
			dummy >> dummy >>
			sumValidLoad >> checksum >> recordsUsed >> recordsMax >>
			dummy >> dummy;
		sumValid = sumValidLoad != FALSE;

		// Check title for validity.
		if (title.find("HoverRace track file") == std::string::npos &&
			title.find("Fireball object factory resource file") == std::string::npos)
		{
			//TODO: Log error.
			ASSERT(FALSE);
		}
		else {
			if (recordsMax > 0) {
				recordList = new MR_UInt32[recordsMax];
				for (unsigned int i = 0; i < recordsMax; ++i) {
					os >> recordList[i];
				}
			}
		}
	}
}

void ClassicRecordFileHeader::Inspect(Util::InspectMapNode &node) const
{
	node.
		AddField("title", title).
		AddField("sumValid", sumValid).
		AddField("checksum", checksum).
		AddField("recordsUsed", recordsUsed).
		AddField("recordsMax", recordsMax).
		AddArray("recordList", recordList, 0, recordsMax);
}

// ClassicRecordFile

ClassicRecordFile::ClassicRecordFile() :
	SUPER(), constructionMode(false), curRecord(-1), header(NULL)
{
}

ClassicRecordFile::~ClassicRecordFile()
{
	if (header != NULL) {
		// Re-write header for construction mode.
		if (constructionMode) {
			fseek(fileStream, 0, SEEK_SET);
			ClassicObjStream objStream(fileStream, filename, true);
			header->Serialize(objStream);
		}

		fclose(fileStream);
		delete header;
	}
}

bool ClassicRecordFile::CreateForWrite(const Util::OS::path_t &filename, int numRecords, const char *title)
{
	if (header != NULL) return false;

	this->filename = filename;

	fileStream = OS::FOpen(filename, "w+b");
	if (fileStream == NULL) return false;

	ClassicObjStream objStream(fileStream, filename, true);
	header = new ClassicRecordFileHeader(numRecords);
	header->title = title;
	header->Serialize(objStream);

	constructionMode = true;

	return true;
}

bool ClassicRecordFile::OpenForWrite(const Util::OS::path_t &filename)
{
	if (header != NULL) return false;

	this->filename = filename;

	fileStream = OS::FOpen(filename, "r+b");
	if (fileStream == NULL) return false;

	ClassicObjStream objStream(fileStream, filename, false);
	header = new ClassicRecordFileHeader();
	header->Serialize(objStream);

	if (header->recordList == NULL) { fclose(fileStream); return false; }

	curRecord = 0;

	constructionMode = true;

	return true;
}

bool ClassicRecordFile::OpenForRead(const Util::OS::path_t &filename, bool validateChecksum)
{
	if (OpenForWrite(filename)) {
		constructionMode = false;

		//TODO: Validate checksum;
		return true;
	}

	return false;
}

DWORD ClassicRecordFile::GetAlignMode()
{
	return (header != NULL && header->recordList != NULL) ? header->checksum : 0;
}

int ClassicRecordFile::GetNbRecords() const
{
	return (header != NULL && header->recordList != NULL) ? header->recordsUsed : 0;
}

void ClassicRecordFile::SelectRecord(int i)
{
	if (header != NULL) {
		if ((unsigned)i < header->recordsUsed) {
			curRecord = i;
			fseek(fileStream, header->recordList[i], SEEK_SET);
		}
		else {
			ASSERT(FALSE);
		}
	}
}

bool ClassicRecordFile::BeginANewRecord()
{
	if (header == NULL) {
		ASSERT(FALSE);
		return false;
	}

	if (header->recordsUsed >= header->recordsMax) {
		// Exceeded preallocated record count.
		ASSERT(FALSE);
		return false;
	}

	fseek(fileStream, 0, SEEK_END);
	curRecord = header->recordsUsed++;
	header->recordList[curRecord] = ftell(fileStream);

	return true;
}

void ClassicRecordFile::Inspect(Util::InspectMapNode &node) const
{
	node.
		AddField("curRecord", curRecord).
		AddSubobject("header", header);
}

ObjStreamPtr ClassicRecordFile::StreamIn()
{
	return ObjStreamPtr(new ClassicObjStream(fileStream, filename, false));
}

ObjStreamPtr ClassicRecordFile::StreamOut()
{
	return ObjStreamPtr(new ClassicObjStream(fileStream, filename, true));
}

}  // namespace Parcel
}  // namespace HoverRace
