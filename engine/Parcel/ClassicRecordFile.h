
// ClassicRecordFile.h
// Standard HoverRace 1.x parcel format.
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

#pragma once

#include <fstream>

#include "RecordFile.h"

#ifdef _WIN32
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
	typedef RecordFile SUPER;
	public:
		ClassicRecordFile();
		virtual ~ClassicRecordFile();

		virtual bool CreateForWrite(const char *filename, int numRecords, const char *title=NULL);
		virtual bool OpenForWrite(const char *filename);
		virtual bool OpenForRead(const char *filename, bool validateChecksum=false);

		virtual DWORD GetAlignMode();

		virtual int GetNbRecords() const;
		virtual void SelectRecord(int i);
		virtual bool BeginANewRecord();

		virtual void Inspect(Util::InspectMapNode &node) const;

		virtual ObjStreamPtr StreamIn();
		virtual ObjStreamPtr StreamOut();

	private:
		int curRecord;
		ClassicRecordFileHeader *header;
		FILE *fileStream;
		std::string filename;
};

}  // namespace Parcel
}  // namespace HoverRace

#undef MR_DllDeclare
