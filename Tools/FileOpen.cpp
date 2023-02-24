//
//  Простейшие процедуры для запроса имени c открытием нового файла FileOpen
//  со считыванием строк произвольной длины и разбором табуляторов getString
//
//      В фильтре выбора файлов вместо \0 необходимо использовать \1
//      Для принудительного диалога выбора файлов
//          в первом символе Title должен быть символ '?'
//
//#define _UNICODE      -- снято для явного указания операций с Windows-Unicode
#include <stdio.h>
#include <windows.h>
#include "../Type.h"

FILE *FileOpen
(       char *FName,  // = буфер полного пути имени файла с длиной = MAX_PATH*2
  const char *Type,   // = "rt"
  const char *Ext,    // = "vsl",
  const char *Choice, // = "Ship Hull Form (*.vsl)\0*.vsl\0All Files\0*.*\0\0"
  const char *Title ) // = "? выбрать корпус или - Esc - для модели МИДВ"
{ WCHAR wType[32],*wName=(WCHAR*)FName; wcscpy( wName,U2W( FName ) );
  FILE *F=NULL;                         wcscpy( wType,U2W( Type ) );
  if( *Title!='?' && *wName!=L'*' )F=_wfopen( wName,wType );
  if( !F )
  { OPENFILENAMEW W={ sizeof( OPENFILENAMEW ),0 };
                  W.lpstrFile   = wName; // L"Аврора.vsl";
    if( Title  )  W.lpstrTitle  = wcsdup( U2W( Title ) );
    if( Ext    )  W.lpstrDefExt = wcsdup( U2W( Ext ) );
    if( Choice ){ WCHAR *C=wcsdup( U2W( Choice ) ); W.lpstrFilter=C;
                  while( C=wcschr( C,L'\1' ) )*C++=0;
                } W.nMaxFile = MAX_PATH*2;
    if( Type[0]=='w' )
    { W.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
      if( GetSaveFileNameW( &W ) )F=_wfopen( W.lpstrFile,wType );
    } else
    { W.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST
                   | OFN_EXPLORER | OFN_HIDEREADONLY; // OFN_ALLOWMULTISELECT
      if( GetOpenFileNameW( &W ) )F=_wfopen( W.lpstrFile,wType );
    } wcscpy( (WCHAR*)FName,W.lpstrFile );
    free( (void*)W.lpstrTitle );
    free( (void*)W.lpstrDefExt );
    free( (void*)W.lpstrFilter );
  } strcpy( FName,W2U(wName) ); return F; // должно быть место для имени во вне
//} strcpy( FName,fname( W2U( wName ) ) ); return F; // место для имени во-вне.
}
//      по случаю - чтение файловых строчек с тем же (единым!) буфером в памяти
//
static string LS;        //! другая рабочая строчка текста неограниченной длины

char *getString( FILE *F )        // Чтение текстовой строки произвольной длины
{ int k=0;                        //  с проверкой наличия <cr> перед <lf>
  if( F )                         //
  while( !feof( F ) ){ if( (LS[k]=fgetc( F ))!='\n' )k++; else
                       { if( k>1 )if( LS[k-1]=='\r' )--k; break; }
                     } LS[k]=0; return LS;
}
char *getString( FILE *F, int t ) // Чтение строчки произвольной длины и
{ int c,k=0; t=minmax( 2,t,8 );   // с разбором табуляторов, конец строки может
  while( !feof( F ) && (c=fgetc( F ))!='\n' ) // быть до и после выборки строки
     if( c!='\r' ){ if( c!='\t' )LS[k++]=c; else do LS[k++]=' '; while( k%t );
                  } LS[k]=0; return LS;
}
