
// ClassicRecordFile.h
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

#pragma once

#include <fstream>

#include "RecordFile.h"

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
namespace Parcel {

class ClassicRecordFileHeader;

/**
 * Standard HoverRace 1.x parcel format.
 * @author Michael Imamura
 */
class MR_DllDeclare ClassicRecordFile : public RecordFile
{
	using SUPER = RecordFile;

public:
	ClassicRecordFile();
	virtual ~ClassicRecordFile();

	bool CreateForWrite(const Util::OS::path_t &filename, MR_UInt32 numRecords,
		const char *title = nullptr) override;
	bool OpenForWrite(const Util::OS::path_t &filename) override;
	bool OpenForRead(const Util::OS::path_t &filename,
		bool validateChecksum = false) override;

protected:
	static DWORD ComputeSum(const Util::OS::path_t &filename);
public:
	bool ApplyChecksum(const Util::OS::path_t &filename) override;

	DWORD GetAlignMode() override;

	MR_UInt32 GetNbRecords() const override;
	void SelectRecord(MR_UInt32 i) override;
	bool BeginANewRecord() override;

	void Inspect(Util::InspectMapNode &node) const override;

	ObjStreamPtr StreamIn() override;
	ObjStreamPtr StreamOut() override;

private:
	bool constructionMode;
	MR_UInt32 curRecord;
	ClassicRecordFileHeader *header;
	FILE *fileStream;
	Util::OS::path_t filename;
};

}  // namespace Parcel
}  // namespace HoverRace

#undef MR_DllDeclare
