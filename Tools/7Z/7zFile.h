/* 7zFile.h -- File IO
2021-02-15 : Igor Pavlov : Public domain */

#ifndef __7Z_FILE_H
#define __7Z_FILE_H

#include <windows.h>

typedef          __int32 Int32;
typedef unsigned __int32 UInt32;
typedef          __int64 Int64;
typedef unsigned __int64 UInt64;

HANDLE InFile_Open( const char* );
HANDLE OutFile_Open( const char* );
HANDLE InFile_OpenW( const WCHAR* );
HANDLE OutFile_OpenW( const WCHAR* );

size_t File_Read( const HANDLE, void *data, size_t size );
size_t File_Write( const HANDLE, const void *data, size_t size );

Int64 File_Seek( const HANDLE handle, Int64 *pos, int moveMethod );
UInt64 File_GetLength( const HANDLE );      //FILE_BEGIN=0 ._CURRENT=1 ._END=2

void File_Close( const HANDLE );
#endif
