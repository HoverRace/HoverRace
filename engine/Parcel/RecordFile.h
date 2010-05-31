
// RecordFile.h
// Base class for parcel implementations.
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

#include "../Util/Inspectable.h"
#include "../Util/InspectNode.h"

#include "ObjStream.h"

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

/**
 * Base class for parcel implementations.
 * @author Michael Imamura
 */
class MR_DllDeclare RecordFile : public Util::Inspectable {
	typedef Util::Inspectable SUPER;
	public:
		RecordFile() : SUPER() { }
		virtual ~RecordFile() { }

		virtual bool CreateForWrite(const char *filename, int numRecords, const char *title=NULL) = 0;
		virtual bool OpenForWrite(const char *filename) = 0;
		virtual bool OpenForRead(const char *filename, bool validateChecksum=false) = 0;

		virtual DWORD GetAlignMode() = 0;

		virtual int GetNbRecords() const = 0;
		virtual void SelectRecord(int i) = 0;
		virtual bool BeginANewRecord() = 0;

		/**
		 * Open an object stream for reading at the current record.
		 * It is the caller's responsibility to ensure that only one stream
		 * (input or output) exists at a time.
		 * @return A shared pointer to the new input stream (never @c NULL).
		 */
		virtual ObjStreamPtr StreamIn() = 0;

		/**
		 * Open an object stream for writing at the current record.
		 * It is the caller's responsibility to ensure that only one stream
		 * (input or output) exists at a time.
		 * @return A shared pointer to the new output stream (never @c NULL).
		 */
		virtual ObjStreamPtr StreamOut() = 0;
};
typedef boost::shared_ptr<RecordFile> RecordFilePtr;

}  // namespace Parcel
}  // namespace HoverRace

#undef MR_DllDeclare
