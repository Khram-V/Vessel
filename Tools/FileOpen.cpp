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
static string LS;     //! ~файловая рабочая строчка текста неограниченной длины
#if 0
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
}
#endif
FILE *FileOpen
( char *wName,// = буфер полного пути имени файла в UTF-8 с длиной = MAX_PATH*2
  const WCHAR *wType,  // = "rt"
  const WCHAR *Ext,    // = "vsl",
  const WCHAR *Choice, // = "Ship Hull Form (*.vsl)\0*.vsl\0All Files\0*.*\0\0"
  const WCHAR *Title ) // = "? выбрать корпус или - Esc - для модели МИДВ"
{ FILE *F=NULL; WCHAR *C=NULL,*Fname=U2W( wName ); // буфер не менее 2К статики
  if( *Title!=L'?' && *wName!='*' )F=_wfopen( Fname,wType ); ///'?' не забывать
  if( !F )
  { OPENFILENAMEW W={ sizeof( OPENFILENAMEW ),0 };
                  W.lpstrInitialDir =
                  W.lpstrFile   = Fname; // L"Аврора.vsl";
    if( Title  )  W.lpstrTitle  = Title;
    if( Ext    )  W.lpstrDefExt = Ext;
    if( Choice ){ W.lpstrFilter =( C=wcsdup( Choice ) ); // копия для константы
                  while( C=wcschr( C,L'\1' ) )*C++=0;
                } W.nMaxFile = MAX_PATH*2;
    if( wType[0]==L'w' )
    { W.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST; //| OFN_FILEMUSTEXIST;
      if( GetSaveFileNameW( &W ) )F=_wfopen( W.lpstrFile,wType );
    } else
    { W.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST
                   | OFN_EXPLORER | OFN_HIDEREADONLY; // OFN_ALLOWMULTISELECT
      if( GetOpenFileNameW( &W ) )F=_wfopen( W.lpstrFile,wType );
    } if( C )free( C );
    strcpy( wName,W2U( wcscpy( (WCHAR*)( (char*)LS ),W.lpstrFile) ) ); //?Fname
  } return F;
}
//      по случаю - чтение файловых строчек со своим (другим!) буфером в памяти
//

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
/*#if 1
#include <WChar.h>
char* W2U( const wchar_t *str ) /// входная str не может быть унаследованной LS
{ int i,l,iL=wcslen( str ),n; LS[0]=0; // в пределах инициированной длины на 2К
   for( i=l=0; i<iL; i++ ){ strcpy( &LS[l],CtU(str[i],&n) ); l+=n; } return LS;
}
wchar_t* U2W( const char* str )   // и здесь только в пределах имеющейся LS.len
{ if( str )if( *str ){ LS[0]=0;   // LS[Usize( str )*2]=0;
    char *s=(char*)str; unsigned u=0; wchar_t *w=(wchar_t*)( (char*)LS );
    while( *s ){ s=UtC( u,s ); *w++=wchar_t( u ); } *w=0;
  } return (wchar_t*)( (char*)LS );
}
#else
#include <windows.h>
//
//    всё будет на UTF-8, и только открытие файлов в Unicode-Windows (UTF-16le)
//
char* W2U( const wchar_t *str )
{ int iL=::WideCharToMultiByte( CP_UTF8,0,str,-1,NULL,0,NULL,NULL ); LS[iL]=0;
         ::WideCharToMultiByte( CP_UTF8,0,str,-1,LS,iL,NULL,NULL ); return LS;
}
wchar_t* U2W( const char* str )
{ int uL=::MultiByteToWideChar( CP_UTF8,0,str,-1,NULL,0 ); LS[uL*2]=0;
         ::MultiByteToWideChar( CP_UTF8,0,str,-1,(LPWSTR)((char*)LS),uL );
  return (wchar_t*)((char*)LS);
}
#endif */
/*
void Break( const char Msg[],... )    // Случай аварийного завершения программы
{ va_list V; va_start( V,Msg );       // или приостановка с первым символом "~"
 char *str=(char*)malloc( vsprintf( 0,Msg,V )+4 );
                          vsprintf( str,Msg,V ); va_end( V );
  MessageBoxW( NULL,U2W(str),*Msg=='~'?L"Info":L"Break",MB_ICONASTERISK|MB_OK );
  if( *Msg!='~' )exit( MB_OK ); free( str );
}
*/
