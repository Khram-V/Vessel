//
//  Простейшие процедуры для запроса имени c открытием нового файла FileOpen
//  со считыванием строк произвольной длины и разбором табуляторов getString
//
#include <stdio.h>
#include <windows.h>
#include "../Type.h"

FILE *FileOpen
(       char *FName,  // = буфер полного пути имени файла с длиной = MAX_PATH
  const char *Type,   // = "rt"
  const char *Ext,    // = "vsl",
  const char *Choice, // = "Ship Hull Form (*.vsl)\0*.vsl\0All Files\0*.*\0\0"
  const char *Title ) // = "? выбрать корпус или - Esc - для модели МИДВ"
{ OPENFILENAME F; ZeroMemory( &F,sizeof( OPENFILENAME ) );
    F.lStructSize = sizeof( OPENFILENAME );
    F.hwndOwner   = NULL;
    F.lpstrFilter = Choice;
    F.lpstrFile   = FName;
    F.nMaxFile    = MAX_PATH;
    F.lpstrDefExt = Ext;
    F.lpstrTitle  = Title;
    if( Type[0]=='w' )
    { F.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
      if( GetSaveFileName( &F ) )return fopen( FName,Type ); } else
    { F.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
      if( GetOpenFileName( &F ) )return fopen( FName,Type ); } return NULL;
}
