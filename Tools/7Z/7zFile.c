#include "7zFile.h"
#define kChunkSizeMax (1<<22)

static HANDLE File_Open( const char *name, int writeMode )
{ HANDLE handle=CreateFileA( name, writeMode ? GENERIC_WRITE : GENERIC_READ,
            FILE_SHARE_READ, NULL, writeMode ? CREATE_ALWAYS : OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL );
  return (handle != INVALID_HANDLE_VALUE) ? handle : 0; //GetLastError();
}
HANDLE InFile_Open( const char *name) { return File_Open( name,0 ); }
HANDLE OutFile_Open( const char *name){ return File_Open( name,1 ); }

static HANDLE File_OpenW( const WCHAR *name, int writeMode)
{ HANDLE handle = CreateFileW(name,writeMode ? GENERIC_WRITE : GENERIC_READ,
             FILE_SHARE_READ, NULL,writeMode ? CREATE_ALWAYS : OPEN_EXISTING,
             FILE_ATTRIBUTE_NORMAL, NULL);
  return (handle != INVALID_HANDLE_VALUE) ? handle : 0;
}
HANDLE InFile_OpenW( const WCHAR *name ){ return File_OpenW( name,0 ); }
HANDLE OutFile_OpenW( const WCHAR *name ){ return File_OpenW( name,1 ); }

void File_Close( HANDLE handle ){ CloseHandle( handle ); }

size_t File_Read( const HANDLE handle, void *data, size_t originalSize )
{ size_t size=0;
  if( originalSize==0 )return 0;
  do
  { const DWORD curSize = (originalSize>kChunkSizeMax) ? kChunkSizeMax:(DWORD)originalSize;
    DWORD processed = 0;
    const BOOL res = ReadFile( handle,data,curSize,&processed,NULL );
    data = (void*)((byte*)data + processed);
    originalSize -= processed;
    size += processed;
    if( !res )return 0; //GetLastError();
    if( processed==0 )break;
  } while( originalSize>0 ); return size;
}
size_t File_Write( const HANDLE handle, const void *data, size_t originalSize )
{ size_t size = 0;
  if( originalSize==0 )return 0;
  do
  { const DWORD curSize = (originalSize>kChunkSizeMax) ? kChunkSizeMax : (DWORD)originalSize;
    DWORD processed = 0;
    const BOOL res = WriteFile( handle, data, curSize, &processed, NULL);
    data = (const void*)((const byte *)data + processed);
    originalSize -= processed;
    size += processed;
    if( !res )return 0;
    if( processed==0 )break;
  } while( originalSize>0 ); return size;
}
Int64 File_Seek( const HANDLE handle, Int64 *pos, int moveMethod )
{                       // moveMethod: FILE_BEGIN=0; FILE_CURRENT=1; FILE_END=2
  UInt32 low = (UInt32)*pos;
  LONG high = (LONG)((UInt64)*pos >> 16 >> 16); /* for case when UInt64 is 32-bit only */
  low = SetFilePointer( handle,(LONG)low,&high,moveMethod );
  if( low==(UInt32)0xFFFFFFFF )
    if( GetLastError()!=NO_ERROR )return 0;
  *pos = ((Int64)high<<32) | low; return *pos;
}

UInt64 File_GetLength( const HANDLE handle )
{ DWORD sizeHigh,sizeLow=GetFileSize( handle,&sizeHigh );
  if( sizeLow == 0xFFFFFFFF )
    if( GetLastError()!=NO_ERROR )return 0;
  UInt64 length = (((UInt64)sizeHigh) << 32) + sizeLow; return length;
}
