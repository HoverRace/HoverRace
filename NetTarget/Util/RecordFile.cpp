// RecordFile.cpp
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

#include "stdafx.h"								  // standar include at the beginning of each cpp file

#include "RecordFile.h"

#define new DEBUG_NEW

// MR_RecordFileTableStuff

class MR_RecordFileTable
{
	public:
		CString mFileTitle;
		BOOL mSumValid;
		DWORD mChkSum;							  // Check sum of the control record
		int mRecordUsed;						  // Nb of record used
		int mRecordMax;							  // Ne of record allowed
		DWORD *mRecordList;

		MR_RecordFileTable();
		MR_RecordFileTable(int pNbRecords);
		~MR_RecordFileTable();

		void Serialize(CArchive & pArchive);

};

static DWORD ComputeSum(const char *pFileName);

MR_RecordFileTable::MR_RecordFileTable()
{
	mRecordUsed = 0;
	mRecordMax = 0;
	mSumValid = FALSE;
	mChkSum = 0;

	mRecordList = NULL;
}

MR_RecordFileTable::MR_RecordFileTable(int pNbRecords)
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

MR_RecordFileTable::~MR_RecordFileTable()
{
	delete[]mRecordList;
}

void MR_RecordFileTable::Serialize(CArchive & pArchive)
{

	if(pArchive.IsStoring()) {
		pArchive << mFileTitle << (int) 0		  // Padding for checksum purpose
			<< (int) 0 << mSumValid << mChkSum << mRecordUsed << mRecordMax << (int) 0 << (int) 0;

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

		if(mRecordMax > 0) {
			mRecordList = new DWORD[mRecordMax];

			pArchive.Read(mRecordList, sizeof(mRecordList[0]) * mRecordMax);
		}
	}
}

// MR_RecordFile
MR_RecordFile::MR_RecordFile()
{
	mConstructionMode = FALSE;
	mTable = NULL;
	mCurrentRecord = -1;
}

MR_RecordFile::~MR_RecordFile()
{
	if(mTable != NULL) {
		Close();
		delete mTable;
	}
}

int MR_RecordFile::GetNbRecords() const
{
	int lReturnValue = 0;

	if(mTable != NULL) {
		lReturnValue = mTable->mRecordUsed;
	}
	return lReturnValue;
}

int MR_RecordFile::GetNbRecordsMax() const
{
	int lReturnValue = 0;

	if(mTable != NULL) {
		lReturnValue = mTable->mRecordMax;
	}
	return lReturnValue;
}

int MR_RecordFile::GetCurrentRecordNumber() const
{
	return mCurrentRecord;
} BOOL MR_RecordFile::CreateForWrite(const char *pFileName, int pNbRecords, const char *pTitle)
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
			mTable = new MR_RecordFileTable(pNbRecords);
			mTable->mFileTitle = pTitle;

			// Write the mTable to reserve some space and position the file on the first record
			{
				CArchive lArchive(this, CArchive::store);

				mTable->Serialize(lArchive);
			}
		}
	}
	return lReturnValue;

}

BOOL MR_RecordFile::OpenForWrite(const char *pFileName)
{
	BOOL lReturnValue = FALSE;
	ASSERT(mTable == NULL);

	if(mTable == NULL) {
		mConstructionMode = TRUE;
		mCurrentRecord = -1;

		// Try to open the file
		lReturnValue = CFile::Open(pFileName, modeReadWrite | typeBinary | shareExclusive);

		if(lReturnValue) {
			CArchive lArchive(this, CArchive::load);
			mTable = new MR_RecordFileTable;

			mTable->Serialize(lArchive);

			if(mTable == NULL)
				lReturnValue = FALSE;

			if(!lReturnValue)
				CFile::Close();
		}
	}
	return lReturnValue;
}

BOOL MR_RecordFile::BeginANewRecord()
{
	BOOL lReturnValue = FALSE;

	ASSERT(mConstructionMode);

	if((mTable != NULL) && mConstructionMode) {
		if(mTable->mRecordUsed < mTable->mRecordMax) {
			mCurrentRecord = mTable->mRecordUsed;
			CFile::Seek(0, end);
			mTable->mRecordList[mCurrentRecord] = CFile::GetPosition();
			mTable->mRecordUsed = mCurrentRecord + 1;

			lReturnValue = TRUE;
		}
	}
	return lReturnValue;
}

BOOL MR_RecordFile::OpenForRead(const char *pFileName, BOOL pValidateChkSum)
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
			CArchive lArchive(this, CArchive::load | CArchive::bNoFlushOnDelete);

			mTable = new MR_RecordFileTable;
			mTable->Serialize(lArchive);
		}

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

	return lReturnValue;
}

// Checksum stuff (Renamed to Reopen for security purpose
//      #define ApplyChecksum ReOpen
BOOL MR_RecordFile::ReOpen(const char *pFileName)
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
DWORD MR_RecordFile::GetAlignMode()
{
	if((mTable != NULL) && (mTable->mSumValid)) {
		return mTable->mChkSum;
	}
	else {
		return 0;
	}
}

void MR_RecordFile::SelectRecord(int pRecordNumber)
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

ULONGLONG MR_RecordFile::GetPosition() const
{
	ULONGLONG lReturnValue = CFile::GetPosition();

	if((mTable != NULL) && (mCurrentRecord >= 0)) {
		lReturnValue -= mTable->mRecordList[mCurrentRecord];
	}
	return lReturnValue;
}

CString MR_RecordFile::GetFileTitle() const
{
	if(mTable != NULL) {
		return mTable->mFileTitle;
	}
	else {
		ASSERT(FALSE);
		return "";
	}
}

BOOL MR_RecordFile::Open(LPCTSTR, UINT, CFileException *)
{
	ASSERT(FALSE);
	AfxThrowNotSupportedException();

	return FALSE;
}

CFile *MR_RecordFile::Duplicate() const
{
	ASSERT(FALSE);
	AfxThrowNotSupportedException();
	return NULL;
} LONG MR_RecordFile::Seek(LONG pOff, UINT pFrom)
{
	// BUG This function do not check for record overflow
	LONG lLocalOffset = 0;

	ASSERT_VALID(this);

	if(mCurrentRecord >= 0) {
		lLocalOffset = mTable->mRecordList[mCurrentRecord];
	}

	return CFile::Seek(pOff + lLocalOffset, pFrom) - lLocalOffset;
}

void MR_RecordFile::SetLength(DWORD)
{
	ASSERT(FALSE);
	AfxThrowNotSupportedException();
}

ULONGLONG MR_RecordFile::GetLength() const
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

UINT MR_RecordFile::Read(void *pBuf, UINT pCount)
{

	// Simply cut nCount if it overflow
	ASSERT_VALID(this);

	if(mCurrentRecord >= 0) {
		if(mCurrentRecord + 1 < mTable->mRecordUsed) {
			UINT lRecordLen = mTable->mRecordList[mCurrentRecord + 1]
				- mTable->mRecordList[mCurrentRecord];

			if(GetPosition() + pCount > lRecordLen) {
				pCount = lRecordLen - GetPosition();
				ASSERT((int) pCount >= 0);
			}
		}
	}

	return CFile::Read(pBuf, pCount);
}

void MR_RecordFile::Write(const void *pBuf, UINT pCount)
{
	ASSERT(mConstructionMode);

	CFile::Write(pBuf, pCount);
}

void MR_RecordFile::LockRange(DWORD, DWORD)
{
	ASSERT(FALSE);
	AfxThrowNotSupportedException();
}

void MR_RecordFile::UnlockRange(DWORD, DWORD)
{
	ASSERT(FALSE);
	AfxThrowNotSupportedException();
}

void MR_RecordFile::Abort()
{
	ASSERT(FALSE);
	CFile(Abort);
}

void MR_RecordFile::Close()
{
	mCurrentRecord = -1;
	if(mConstructionMode && (mTable != NULL)) {
		// Write the Record table

		Seek(0, begin);

		CArchive lArchive(this, CArchive::store);

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

void MR_RecordFile::AssertValid() const
{
	CFile::AssertValid;

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

void MR_RecordFile::Dump(CDumpContext & dc) const
{
	CFile::Dump(dc);

	// TODO
}
#endif
