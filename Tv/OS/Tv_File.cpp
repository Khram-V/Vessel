//
//       GetName.Cpp                         Ver. 0.1  /90.12.26/
//                                           Ver. 2.2  /92.10.20/
//                                           Программа поиска файлов
#include <IO.h>
#include "Tv_graph.h"

char *Tfile_Filter=(char*)"Unique data array (*.dat)\0*.dat\0"
                          "All files (*.*)\0*.*\0";
                                           //
long long Tfile( FILE *Fi )                // Вычисление длины открытого файла
#if __MINGW_GNUC_PREREQ( 4,9 )             // == !! 4,5
{ if( Fi )
  { LARGE_INTEGER l;
    if( GetFileSizeEx((HANDLE)_get_osfhandle(fileno(Fi)),&l))return l.QuadPart;
  } return 0;
}
#else
{ unsigned long len=0; if( Fi )len=filelength( fileno( Fi ) ); return len;
}
#endif
//
//    Наконец-то в 93.11.11 осознана необходимость Open/Read/Write
//
static char* NExt( char* Name, const char* Ext )
{ if( Ext )fext( Name,Ext ); return Name;
}
static int FindSim( const char* S, const char* C )
{ char* B=strpbrk( (char*)S,C ); if( B )return int( B-(char*)S ); else return 0;
}
//      Полная замена для Tv_File
//
static char GetMode='x';  // Простейшее обращение
static OPENFILENAME       //
       GetFile={ sizeof( OPENFILENAME ),0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

long long Tfile( char* Path )  // Простое контрольное открытие файла
{ long long L;
  int  l=0,First=true;                         //
  char S,Dir[MAX_PATH]="";                     // здесь корректируется
  if( Path[0]<=' ' )strcpy( Path,"*.*" ); else //   исходное имя файла
  { for( l=strlen( Path ); l>0; l-- )          //
     { S=Path[l-1]; if( S=='\\' || S==':' )break;
  }  }
Repeat: L=0L;
  if( GetMode!='w' )
  { FILE *F;
    if( ( F=fopen( Path,"r" ) )!=0 )
    { L=Tfile( F ); _ac_=max( _ac_,2 );
      fclose( F ); _av_[1]=fname( Path ); F=NULL;
  } }                                  // подготовка предложения к поиску файла
  if( L || !First )return L;           //   в заданной библиотеке
  if( l>0 ){ strncpy( Dir,Path,l ); Dir[l]=0; strcpy( Path,Path+l ); }
  GetFile.lpstrFilter
        = Tfile_Filter[0]>' '?Tfile_Filter:"All Files (*.*)\0*.*\0";
  GetFile.lpstrFile=Path;                    // здесь остается только имя файла
  GetFile.nMaxFile=MAX_PATH;
  GetFile.lpstrInitialDir=Dir[0]>' '?Dir:NULL;
  if( GetMode=='w' )
  { GetFile.Flags=OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    GetSaveFileName( &GetFile );
  } else
  { GetFile.Flags=OFN_FILEMUSTEXIST | OFN_READONLY;
    GetOpenFileName( &GetFile );
  } First=false; goto Repeat;
}
long long Tfile( char* Path, const char* Com )  // С надписью на шапке
{    GetFile.lpstrTitle=Com; long long L=Tfile( Path );
     GetFile.lpstrTitle=0;   return L;
}
FILE *Topen( char* Name, const char* Mode,      // Имя и режим открытия файла
                         const char* Ext,       // Расширение имени файла
                         const char* Com )      // Комментарий к открытию
{
 FILE *F=NULL;
 char Nm[MAX_PATH];
 int l=FindSim( NExt( strcpy( Nm,Name ),Ext ),"*?" );
  GetMode=*Mode|0x20;
  GetFile.lpstrDefExt=Ext;
  if( GetMode=='r' || ( l && GetMode!='w' ) )
  { if( !l )
    if( !(F=fopen( Nm,Mode )) )
    if( !FindSim( Nm,":\\" ) )
    { NExt( strcpy( fname( strcpy( Nm,_av_[0] ) ),Name ),Ext );
      F=fopen( Nm,Mode );
    } if( F )fclose( F ); else Tfile( NExt( strcpy(Nm,Name),Ext ),Com );
  }
  if( ( F=fopen( Nm,Mode ) )!=NULL )strcpy( Name,Nm );
  GetFile.lpstrDefExt=0;
  GetMode='x';
  return F;
}
#if 0
//      Процедура обработки прерывания при ошибке устройства
//
static handlerr( int,int,int,int )
{ enum{ IGNORE,RETRY,ABORT }; Tv_bell(); hardresume( _HARDERR_IGNORE );
  return IGNORE;
}
  for(;;)
  { Tv_bell();
    Ghelp( "?%s error: A)bort R)etry",ax<0?"Device":"Disk" );
    switch( Tv_getc()|0x20 )
    { case 'r':hardretn( RETRY );
      case 'a': return ABORT;
    }
  } //char* Tfile_Filter="Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0";

bool OpenFileName(  char* Title, char* Name,  char* Ext="txt" )
{ OPENFILENAME ofn;
  char szFileName[MAX_PATH]=""; ZeroMemory( &ofn,sizeof(ofn) );
   ofn.lStructSize = sizeof(ofn);
   ofn.lpstrFilter = Tfile_Filter;
   ofn.lpstrFile  = szFileName;
   ofn.hwndOwner = NULL;
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrDefExt = Ext;
   ofn.lpstrTitle = Title;
   ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
   if( !GetOpenFileName( &ofn ) )return false;
      strcpy( Name,szFileName ); return true;
}
bool SaveFileName(  char* Title, char* Name,  char* Ext="txt" )
{ OPENFILENAME ofn;
  char szFileName[MAX_PATH]=""; ZeroMemory( &ofn,sizeof(ofn) );
   ofn.lStructSize = sizeof(ofn);
   ofn.lpstrFilter = Tfile_Filter;
   ofn.lpstrFile  = szFileName;
   ofn.hwndOwner = NULL;
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrDefExt = Ext;
   ofn.lpstrTitle = Title;
   ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
   if( !GetSaveFileName( &ofn ) )return false;
      strcpy( Name,szFileName ); return true;
}
#endif
