// MfcRecordFile.cpp
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
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
//

#include "StdAfx.h"

#include "MfcObjStream.h"

#include "MfcRecordFile.h"

#define new DEBUG_NEW

namespace HoverRace {
namespace Parcel {

// MfcRecordFileTable

class MfcRecordFileTable : public Util::Inspectable
{
	public:
		CString mFileTitle;
		BOOL mSumValid;
		MR_UInt32 mChkSum;						  // Check sum of the control record
		int mRecordUsed;						  // Nb of record used
		int mRecordMax;							  // Ne of record allowed
		DWORD *mRecordList;

		MfcRecordFileTable();
		MfcRecordFileTable(int pNbRecords);
		virtual ~MfcRecordFileTable();

		void Serialize(ObjStream &pArchive);

		virtual void Inspect(Util::InspectMapNode &node) const;
};

static DWORD ComputeSum(const char *pFileName);

MfcRecordFileTable::MfcRecordFileTable() :
	Util::Inspectable()
{
	mRecordUsed = 0;
	mRecordMax = 0;
	mSumValid = FALSE;
	mChkSum = 0;

	mRecordList = NULL;
}

MfcRecordFileTable::MfcRecordFileTable(int pNbRecords)
{
	ASSERT(pNbRecords > 0);

	mRecordMax = pNbRecords;
	mRecordUsed = 0;
	mRecordList = new DWORD[pNbRecords];

	// Initialize the vector
	for(int lCounter = 0; lCounter < pNbRecords; lCounter++) {
		mRecordList[lCounter] = NULL;
	}
}

MfcRecordFileTable::~MfcRecordFileTable()
{
	delete[]mRecordList;
}

void MfcRecordFileTable::Serialize(ObjStream &pArchive)
{

	if(pArchive.IsWriting()) {
		pArchive << mFileTitle << (MR_Int32) 0		  // Padding for checksum purpose
			<< (MR_Int32) 0 << mSumValid << mChkSum << mRecordUsed << mRecordMax << (MR_Int32) 0 << (MR_Int32) 0;

		if(mRecordMax > 0) {
			ASSERT(mRecordList != NULL);

			pArchive.Write(mRecordList, sizeof(mRecordList[0]) * mRecordMax);
		}
	}
	else {
		delete[]mRecordList;
		mRecordList = NULL;

		int lDummy;

		pArchive >> mFileTitle >> lDummy >> lDummy >> mSumValid >> mChkSum >> mRecordUsed >> mRecordMax >> lDummy >> lDummy;

		// check that file type is correct
		// the file title may contain either "HoverRace track file, (c)GrokkSoft 1997" or
		// "HoverRace track file designed by <REGISTERED NAME>(reg key)" or
		// "Fireball object factory resource file, (c)GrokkSoft 1996" (since this method also opens
		// .dats as well as .trks
		if((mFileTitle.Find("HoverRace track file") == -1) &&
		   (mFileTitle.Find("Fireball object factory resource file") == -1)) {
			char error[200];
			sprintf(error, "Corrupt file: %s\n", pArchive.GetName().c_str());
			OutputDebugString(error);
		} else {
			if(mRecordMax > 0) {
				mRecordList = new DWORD[mRecordMax];

				pArchive.Read(mRecordList, sizeof(mRecordList[0]) * mRecordMax);
			}
		}
	}
}

void MfcRecordFileTable::Inspect(Util::InspectMapNode &node) const
{
	node.
		AddField("title", mFileTitle).
		AddField("sumValid", mSumValid != FALSE).
		AddField("checksum", mChkSum).
		AddField("recordsUsed", mRecordUsed).
		AddField("recordsMax", mRecordMax).
		AddArray("recordList", mRecordList, 0, mRecordMax);
}

// MfcRecordFile

MfcRecordFile::MfcRecordFile() :
	RecordFile(), CFile()
{
	mConstructionMode = FALSE;
	mTable = NULL;
	mCurrentRecord = -1;
}

MfcRecordFile::~MfcRecordFile()
{
	if(mTable != NULL) {
		Close();
		delete mTable;
	}
}

int MfcRecordFile::GetNbRecords() const
{
	int lReturnValue = 0;

	if(mTable != NULL) {
		lReturnValue = mTable->mRecordUsed;
	}
	return lReturnValue;
}

int MfcRecordFile::GetNbRecordsMax() const
{
	int lReturnValue = 0;

	if(mTable != NULL) {
		lReturnValue = mTable->mRecordMax;
	}
	return lReturnValue;
}

int MfcRecordFile::GetCurrentRecordNumber() const
{
	return mCurrentRecord;
}

bool MfcRecordFile::CreateForWrite(const char *pFileName, int pNbRecords, const char *pTitle)
{
	BOOL lReturnValue = FALSE;
	ASSERT(mTable == NULL);						  // Open function must be called only once

	if(mTable == NULL) {
		mConstructionMode = TRUE;
		mCurrentRecord = -1;

		// Try yo open the file
		lReturnValue = CFile::Open(pFileName, modeCreate | modeWrite | typeBinary | shareExclusive);

		if(lReturnValue) {
			// Create the mTable
			mTable = new MfcRecordFileTable(pNbRecords);
			mTable->mFileTitle = pTitle;

			// Write the mTable to reserve some space and position the file on the first record
			{
				MfcObjStream lArchive(this, true);

				mTable->Serialize(lArchive);
			}
		}
	}
	return lReturnValue != FALSE;

}

bool MfcRecordFile::OpenForWrite(const char *pFileName)
{
	BOOL lReturnValue = FALSE;
	ASSERT(mTable == NULL);

	if(mTable == NULL) {
		mConstructionMode = TRUE;
		mCurrentRecord = -1;

		// Try to open the file
		lReturnValue = CFile::Open(pFileName, modeReadWrite | typeBinary | shareExclusive);

		if(lReturnValue) {
			MfcObjStream lArchive(this, false);
			mTable = new MfcRecordFileTable;

			mTable->Serialize(lArchive);

			if(mTable == NULL)
				lReturnValue = FALSE;

			if(!lReturnValue)
				CFile::Close();
		}
	}
	return lReturnValue != FALSE;
}

bool MfcRecordFile::BeginANewRecord()
{
	BOOL lReturnValue = FALSE;

	ASSERT(mConstructionMode);

	if((mTable != NULL) && mConstructionMode) {
		if(mTable->mRecordUsed < mTable->mRecordMax) {
			mCurrentRecord = mTable->mRecordUsed;
			CFile::Seek(0, end);
			mTable->mRecordList[mCurrentRecord] = static_cast<DWORD>(CFile::GetPosition());
			mTable->mRecordUsed = mCurrentRecord + 1;

			lReturnValue = TRUE;
		}
	}
	return lReturnValue != FALSE;
}

bool MfcRecordFile::OpenForRead(const char *pFileName, bool pValidateChkSum)
{
	BOOL lReturnValue = FALSE;
	ASSERT(mTable == NULL);
	CFileException e;

	DWORD lSum = 0;

	if(pValidateChkSum)
		lSum = ComputeSum(pFileName);

	if(mTable == NULL) {
		mConstructionMode = FALSE;
		mCurrentRecord = -1;

		// Try to open the file
		lReturnValue = CFile::Open(pFileName, modeRead | typeBinary | shareDenyWrite, &e);

		if(lReturnValue) {
			MfcObjStream lArchive(this, false);

			mTable = new MfcRecordFileTable;
			mTable->Serialize(lArchive);

			if(mTable->mRecordList == NULL) // file did not read correctly
				lReturnValue = FALSE;
		}

		if(lReturnValue) {
			if(mTable == NULL) {
				lReturnValue = FALSE;
				CFile::Close();
			}
			else if(pValidateChkSum && (lSum != mTable->mChkSum)) {
				// Wrong file sum
				lReturnValue = FALSE;
				CFile::Close();
			}
			else {
				// Select the first record if availlable
				if(mTable->mRecordList != NULL) {
					mCurrentRecord = 0;
					Seek(0, begin);
				}
			}
		}
	}

	return lReturnValue != FALSE;
}

// Checksum stuff (Renamed to Reopen for security purpose
//      #define ApplyChecksum ReOpen
BOOL MfcRecordFile::ReOpen(const char *pFileName)
{
	BOOL lReturnValue = FALSE;

	DWORD lSum = 0;

	lSum = ComputeSum(pFileName);

	lReturnValue = OpenForWrite(pFileName);

	if(lReturnValue) {
		mTable->mSumValid = TRUE;
		mTable->mChkSum = lSum;
	}

	return lReturnValue;
}

// #define GetCheckSum GetAlignMode
DWORD MfcRecordFile::GetAlignMode()
{
	if((mTable != NULL) && (mTable->mSumValid)) {
		return mTable->mChkSum;
	}
	else {
		return 0;
	}
}

void MfcRecordFile::SelectRecord(int pRecordNumber)
{
	ASSERT(!mConstructionMode);

	if((mTable != NULL) && (!mConstructionMode)) {
		if(pRecordNumber < mTable->mRecordUsed) {
			mCurrentRecord = pRecordNumber;
			Seek(0, begin);
		}
		else {
			ASSERT(FALSE);
		}
	}
}

ULONGLONG MfcRecordFile::GetPosition() const
{
	ULONGLONG lReturnValue = CFile::GetPosition();

	if((mTable != NULL) && (mCurrentRecord >= 0)) {
		lReturnValue -= mTable->mRecordList[mCurrentRecord];
	}
	return lReturnValue;
}

CString MfcRecordFile::GetFileTitle() const
{
	if(mTable != NULL) {
		return mTable->mFileTitle;
	}
	else {
		ASSERT(FALSE);
		return "";
	}
}

BOOL MfcRecordFile::Open(LPCTSTR, UINT, CFileException *)
{
	ASSERT(FALSE);
	AfxThrowNotSupportedException();

	return FALSE;
}

CFile *MfcRecordFile::Duplicate() const
{
	ASSERT(FALSE);
	AfxThrowNotSupportedException();
	return NULL;
}

LONG MfcRecordFile::Seek(LONG pOff, UINT pFrom)
{
	// BUG This function do not check for record overflow
	LONG lLocalOffset = 0;

	ASSERT_VALID(this);

	if(mCurrentRecord >= 0) {
		lLocalOffset = mTable->mRecordList[mCurrentRecord];
	}

	return static_cast<LONG>(CFile::Seek(pOff + lLocalOffset, pFrom)) - lLocalOffset;
}

void MfcRecordFile::SetLength(DWORD)
{
	ASSERT(FALSE);
	AfxThrowNotSupportedException();
}

ULONGLONG MfcRecordFile::GetLength() const
{
	ASSERT_VALID(this);
	ASSERT(!mConstructionMode);

	ULONGLONG lReturnValue = CFile::GetLength();

	if(mCurrentRecord >= 0) {
		if(mCurrentRecord + 1 < mTable->mRecordUsed) {
			lReturnValue = mTable->mRecordList[mCurrentRecord + 1]
				- mTable->mRecordList[mCurrentRecord];
		}
		else {
			lReturnValue -= mTable->mRecordList[mCurrentRecord];
		}
	}
	return lReturnValue;
}

UINT MfcRecordFile::Read(void *pBuf, UINT pCount)
{

	// Simply cut nCount if it overflow
	ASSERT_VALID(this);

	if(mCurrentRecord >= 0) {
		if(mCurrentRecord + 1 < mTable->mRecordUsed) {
			UINT lRecordLen = mTable->mRecordList[mCurrentRecord + 1]
				- mTable->mRecordList[mCurrentRecord];

			if(GetPosition() + pCount > lRecordLen) {
				pCount = lRecordLen - static_cast<UINT>(GetPosition());
				ASSERT((int) pCount >= 0);
			}
		}
	}

	return CFile::Read(pBuf, pCount);
}

void MfcRecordFile::Write(const void *pBuf, UINT pCount)
{
	ASSERT(mConstructionMode);

	CFile::Write(pBuf, pCount);
}

void MfcRecordFile::LockRange(DWORD, DWORD)
{
	ASSERT(FALSE);
	AfxThrowNotSupportedException();
}

void MfcRecordFile::UnlockRange(DWORD, DWORD)
{
	ASSERT(FALSE);
	AfxThrowNotSupportedException();
}

void MfcRecordFile::Abort()
{
	ASSERT(FALSE);
	CFile(Abort);
}

void MfcRecordFile::Close()
{
	mCurrentRecord = -1;
	if(mConstructionMode && (mTable != NULL)) {
		// Write the Record table

		Seek(0, begin);

		MfcObjStream lArchive(this, true);

		mTable->Serialize(lArchive);
	}
	mCurrentRecord = -1;
	delete mTable;
	mTable = NULL;

	CFile::Close();
}

DWORD ComputeSum(const char *pFileName)
{
	DWORD lReturnValue = 0;
	DWORD lBuffer[2048];						  // 8 K of data

	FILE *lFile = fopen(pFileName, "rb");

	if(lFile != NULL) {
		int lDataLen = fread(lBuffer, 1, sizeof(lBuffer), lFile);

		if(lDataLen > 0) {
			lDataLen /= sizeof(DWORD);

			BOOL lSkippedDo = FALSE;

			for(int lCounter = 0; lCounter < lDataLen; lCounter++) {
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

#ifdef _DEBUG

void MfcRecordFile::AssertValid() const
{
	CFile::AssertValid();

	ASSERT(mCurrentRecord >= -1);
	if(mTable != NULL) {
		ASSERT(mCurrentRecord >= -1);

		if(!mConstructionMode) {
			ASSERT(mCurrentRecord < mTable->mRecordUsed);
		}
	}
	else {
		ASSERT(mCurrentRecord == -1);
	}
}

void MfcRecordFile::Dump(CDumpContext & dc) const
{
	CFile::Dump(dc);

	// TODO
}
#endif

void MfcRecordFile::Inspect(Util::InspectMapNode &node) const
{
	node.
		AddField("curRecord", mCurrentRecord).
		AddSubobject("header", mTable);
}

/**
 * Open an object stream for reading at the current record.
 * It is the caller's responsibility to ensure that only one stream
 * (input or output) exists at a time.
 * @return A shared pointer to the new input stream (never @c NULL).
 */
ObjStreamPtr MfcRecordFile::StreamIn()
{
	return ObjStreamPtr(new MfcObjStream(this, false));
}

/**
 * Open an object stream for writing at the current record.
 * It is the caller's responsibility to ensure that only one stream
 * (input or output) exists at a time.
 * @return A shared pointer to the new output stream (never @c NULL).
 */
ObjStreamPtr MfcRecordFile::StreamOut()
{
	return ObjStreamPtr(new MfcObjStream(this, true));
}


}  // namespace Parcel
}  // namespace HoverRace
