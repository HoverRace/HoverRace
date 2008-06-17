// RecordFile.h
//
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

#ifndef RECORD_FILE_H
#define RECORD_FILE_H

#ifdef MR_UTIL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif


class MR_RecordFileTable;

class MR_RecordFile: public CFile
{
   private:

      MR_RecordFileTable* mTable;
      int                 mCurrentRecord; // for read and write, -1 = not specified
      BOOL                mConstructionMode;

   public:
      // Constructors
	   MR_DllDeclare MR_RecordFile();
	   MR_DllDeclare ~MR_RecordFile();

     
      // Creation operations
	   MR_DllDeclare BOOL CreateForWrite( const char* pFileName, int pNbRecords, const char* lTitle = NULL );
	   MR_DllDeclare BOOL OpenForWrite( const char* pFileName );

	   MR_DllDeclare BOOL BeginANewRecord();

      // Read operation
	   MR_DllDeclare BOOL OpenForRead( const char* pFileName, BOOL pValidateChkSum = FALSE );
      MR_DllDeclare void SelectRecord( int pRecordNumber );

      // Checksum stuff (Renamed to Reopen for security purpose
      #define ApplyChecksum ReOpen
	   MR_DllDeclare BOOL ReOpen( const char* pFileName );

      #define GetCheckSum GetAlignMode
	   MR_DllDeclare DWORD GetAlignMode( );


      // File information functions
      MR_DllDeclare int  GetNbRecords()const;
      MR_DllDeclare int  GetNbRecordsMax()const;
      MR_DllDeclare int  GetCurrentRecordNumber()const;
      
      // Overrided CFile operations
	   MR_DllDeclare ULONGLONG GetPosition() const;
	   MR_DllDeclare CString GetFileTitle() const;

      MR_DllDeclare BOOL   Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL);
      MR_DllDeclare CFile* Duplicate() const;

	   MR_DllDeclare LONG Seek(LONG lOff, UINT nFrom);
	   MR_DllDeclare void SetLength(DWORD dwNewLen);
	   MR_DllDeclare ULONGLONG GetLength() const;

	   MR_DllDeclare UINT Read(void* lpBuf, UINT nCount);
	   MR_DllDeclare void Write( const void* lpBuf, UINT nCount);

	   MR_DllDeclare void LockRange(DWORD dwPos, DWORD dwCount);
	   MR_DllDeclare void UnlockRange(DWORD dwPos, DWORD dwCount);

	   MR_DllDeclare void Abort();
	   MR_DllDeclare void Close();

      #ifdef _DEBUG
	      MR_DllDeclare void AssertValid() const;
         MR_DllDeclare void Dump(CDumpContext& dc) const;
      #endif

};

#undef MR_DllDeclare

#endif

