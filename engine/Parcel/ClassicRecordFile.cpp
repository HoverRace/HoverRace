
// ClassicRecordFile.cpp
//
// Copyright (c) 2010, 2012, 2015 Michael Imamura.
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

#include "../Util/InspectMapNode.h"
#include "../Util/Log.h"
#include "../Util/Str.h"
#include "ClassicObjStream.h"

#include "ClassicRecordFile.h"

using std::ios;

using namespace HoverRace::Util;

namespace HoverRace {
namespace Parcel {

namespace {

/**
 * Used internally to signal errors in ClassicRecordFileHeader.
 */
class ClassicRecordFileExn : public Exception
{
	using SUPER = Exception;

public:
	ClassicRecordFileExn() : SUPER() { }
	ClassicRecordFileExn(const std::string &msg) : SUPER(msg) { }
	virtual ~ClassicRecordFileExn() noexcept { }
};

}  // namespace

class ClassicRecordFileHeader : public Util::Inspectable /*{{{*/
{
	using SUPER = Util::Inspectable;

public:
	ClassicRecordFileHeader();
	ClassicRecordFileHeader(MR_UInt32 numRecords);
	virtual ~ClassicRecordFileHeader();

	void Serialize(ObjStream &os);

	void Inspect(Util::InspectMapNode &node) const override;

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
	recordsUsed(0), recordsMax(numRecords),
	recordList(new MR_UInt32[numRecords])
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
			throw ClassicRecordFileExn("Missing or corrupt header");
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

//}}} ClassicRecordFileHeader

// ClassicRecordFile

ClassicRecordFile::ClassicRecordFile() :
	SUPER(), constructionMode(false), curRecord(0), header(NULL)
{
}

ClassicRecordFile::~ClassicRecordFile()
{
	if (header != NULL) {
		// Re-write header for construction mode.
		if (constructionMode) {
			fseek(fileStream, 0, SEEK_SET);
			ClassicObjStream objStream(fileStream, filename, true);
			try {
				header->Serialize(objStream);
			}
			catch (ClassicRecordFileExn &ex) {
				HR_LOG(error) << "" << filename << ": " << ex.what();
			}
		}

		fclose(fileStream);
		delete header;
	}
}

bool ClassicRecordFile::CreateForWrite(const Util::OS::path_t &filename,
	MR_UInt32 numRecords, const char *title)
{
	if (header) {
		HR_LOG(error) << "" << filename << ": "
			"Cannot create for writing (already open).";
		return false;
	}

	this->filename = filename;

	fileStream = OS::FOpen(filename, "w+b");
	if (!fileStream) {
		HR_LOG(error) << "" << filename << ": Failed to open: " <<
			OS::StrError(errno);
		return false;
	}

	ClassicObjStream objStream(fileStream, filename, true);
	header = new ClassicRecordFileHeader(numRecords);
	header->title = title;
	try {
		header->Serialize(objStream);
	}
	catch (ClassicRecordFileExn &ex) {
		HR_LOG(error) << "" << filename << ": " << ex.what();
		fclose(fileStream);
		return false;
	}

	constructionMode = true;

	return true;
}

bool ClassicRecordFile::OpenForWrite(const Util::OS::path_t &filename)
{
	if (header) {
		HR_LOG(error) << "" << filename << ": Cannot open (already open).";
		return false;
	}

	this->filename = filename;

	fileStream = OS::FOpen(filename, "r+b");
	if (!fileStream) {
		HR_LOG(error) << "" << filename << ": Failed to open: " <<
			OS::StrError(errno);
		return false;
	}

	ClassicObjStream objStream(fileStream, filename, false);
	header = new ClassicRecordFileHeader();
	try {
		header->Serialize(objStream);
	}
	catch (ClassicRecordFileExn &ex) {
		HR_LOG(error) << "" << filename << ": " << ex.what();
		fclose(fileStream);
		return false;
	}

	if (!header->recordList) {
		HR_LOG(error) << "" << filename << ": Failed to read header or no records.";
		fclose(fileStream);
		return false;
	}

	curRecord = 0;

	constructionMode = true;

	return true;
}

bool ClassicRecordFile::OpenForRead(const Util::OS::path_t &filename, bool)
{
	if (OpenForWrite(filename)) {
		constructionMode = false;

		//TODO: Validate checksum;
		return true;
	}

	return false;
}

DWORD ClassicRecordFile::ComputeSum(const OS::path_t &filename)
{
	DWORD lReturnValue = 0;
	DWORD lBuffer[2048];

	FILE *lFile = OS::FOpen(filename, "rb");

	if(lFile != NULL) {
		size_t lDataLen = fread(lBuffer, 1, sizeof(lBuffer), lFile);

		if(lDataLen > 0) {
			lDataLen /= sizeof(DWORD);

			BOOL lSkippedDo = FALSE;

			for(size_t lCounter = 0; lCounter < lDataLen; lCounter++) {
				if(!lSkippedDo && (lBuffer[lCounter] == 0)) {
					lSkippedDo = TRUE;
					lCounter += 6;
				}
				else {
					lReturnValue += lBuffer[lCounter] + ~(lBuffer[lCounter] >> 12);
					lReturnValue = (lReturnValue << 1) + (lReturnValue >> 31);
				}
			}
		}
		fclose(lFile);
	}
	return lReturnValue;
}

bool ClassicRecordFile::ApplyChecksum(const OS::path_t &filename)
{
	DWORD sum = ComputeSum(filename);

	if (OpenForWrite(filename)) {
		header->sumValid = true;
		header->checksum = sum;
		return true;
	}
	else {
		return false;
	}
}

DWORD ClassicRecordFile::GetAlignMode()
{
	return (header != NULL && header->recordList != NULL) ? header->checksum : 0;
}

MR_UInt32 ClassicRecordFile::GetNbRecords() const
{
	return (header != NULL && header->recordList != NULL) ? header->recordsUsed : 0;
}

void ClassicRecordFile::SelectRecord(MR_UInt32 i)
{
	if (header != NULL) {
		if (i < header->recordsUsed) {
			curRecord = i;
			fseek(fileStream, header->recordList[i], SEEK_SET);
		}
		else {
			HR_LOG(error) << "" << filename << "Invalid record (" << i << ") out of "
				"allocated records (" << header->recordsUsed << ").";
		}
	}
}

bool ClassicRecordFile::BeginANewRecord()
{
	if (header == NULL) {
		HR_LOG(error) << "" << filename << ": Not initialized for writing.";
		return false;
	}

	if (header->recordsUsed >= header->recordsMax) {
		HR_LOG(error) << "" << filename << ": Exceeded preallocated record count: " <<
			header->recordsMax;
		return false;
	}

	fseek(fileStream, 0, SEEK_END);
	curRecord = header->recordsUsed++;

	auto pos = ftell(fileStream);
	if (pos < 0) {
		HR_LOG(error) << "" << filename << ": " << OS::StrError(errno);
		return false;
	}
	if (static_cast<MR_UInt64>(pos) >= std::numeric_limits<MR_UInt32>::max()) {
		HR_LOG(error) << "" << filename << ": Too large!";
		return false;
	}

	header->recordList[curRecord] = static_cast<MR_UInt32>(pos);

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
