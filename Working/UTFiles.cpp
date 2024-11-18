#define _UNICODE
#include <StdIO.h>
#include <String.h>
#include <StdLib.h>
#include <Windows.h>

#include <locale.h>

//typedef struct
//{ byte *data;
//  size_t size;
//} CBuf;
//typedef          __int16 Int16;
//typedef unsigned __int16 UInt16;
typedef unsigned __int32 UInt32;

#define _UTF8_START( n )(0x100-(1<<(7-(n))))
#define _UTF8_RANGE( n )(((UInt32)1)<<((n)*5+6))
#define _UTF8_HEAD( n,val )((byte)(_UTF8_START(n)+(val>>(6*(n)))))
#define _UTF8_CHAR( n,val )((byte)(0x80+(((val)>>(6*(n)))&0x3F)))

static size_t Utf16_To_Utf8_Calc( const WCHAR *src, const WCHAR *srcLim )
{ size_t size=0;
  for( ;; )
  { if( src==srcLim )return size; size++; UInt32 val = *src++;
    if( val<0x80 )continue;
    if( val<_UTF8_RANGE(1) ){ size++; continue; }
    if (val>=0xD800 && val<0xDC00 && src!=srcLim )
    { UInt32 c2 = *src;
      if( c2>=0xDC00 && c2<0xE000 ){ src++; size+=3; continue; }
    } size+=2;
  }
}
static char *Utf16_To_Utf8( char *dest, const WCHAR *src, const WCHAR *srcLim )
{ for( ;; )
  { if( src==srcLim )return dest; UInt32 val=*src++;
    if( val<0x80 ){ *dest++ = (char)val; continue; }
    if( val<_UTF8_RANGE(1) )
    { dest[0] = _UTF8_HEAD( 1,val );
      dest[1] = _UTF8_CHAR( 0,val ); dest+=2; continue;
    }
    if( val>=0xD800 && val<0xDC00 && src!=srcLim )
    { UInt32 c2 = *src;
      if( c2>=0xDC00 && c2<0xE000 )
      { src++;
        val = (((val-0xD800)<<10) | (c2-0xDC00)) + 0x10000;
        dest[0] = _UTF8_HEAD( 3,val );
        dest[1] = _UTF8_CHAR( 2,val );
        dest[2] = _UTF8_CHAR( 1,val );
        dest[3] = _UTF8_CHAR( 0,val ); dest+=4; continue;
    } }
    dest[0] = _UTF8_HEAD( 2,val );
    dest[1] = _UTF8_CHAR( 1,val );
    dest[2] = _UTF8_CHAR( 0,val ); dest+=3;
  }
}
static int Utf16_To_Utf8Buf( char *dest, const WCHAR *src, size_t srcLen )
{
  size_t destLen = Utf16_To_Utf8_Calc( src,src+srcLen ); destLen+=1;

//  if( !Buf_EnsureSize( dest,destLen ) )return SZ_ERROR_MEM;

  *Utf16_To_Utf8( dest,src,src+srcLen )=0;
  return 0; //SZ_OK;
}
/// ///////////////////////////////////////////////////////////////////////////

char* UnicodeToUTF8( const WCHAR *str )
{
 static char* pElementText=NULL;
 int iTextLen = WideCharToMultiByte( CP_UTF8,0,str,-1,NULL,0,NULL,NULL );
  pElementText=(char*)realloc( pElementText,iTextLen+1 );
  memset( (void*)pElementText,0,sizeof(char)*(iTextLen+1) );
  ::WideCharToMultiByte( CP_UTF8,0,str,-1,pElementText,iTextLen,NULL,NULL );
 return pElementText;
}
/// ///////////////////////////////////////////////////////////////////////////
static class Locale
{ public: Locale()
  { setlocale( LC_ALL,".UTF8" );
    setlocale( LC_CTYPE,".UTF8" );
    SetConsoleCP( CP_UTF8 );
    SetConsoleOutputCP( CP_UTF8 ); printf( "\n:0 ‏יְרוּשָׁלַיִם \n" );
} } __attribute__((constructor))
    __attribute__((init_priority(500))) Local;
//static void
// __attribute__((constructor))_bar() { printf( "Что-то не то (-) да: ‏יְרוּשָׁלַיִם \n" ); }

int __cdecl main( int Ac, char *Av[] )
//int _tmain( int Argc, LPTSTR Argv[] )
//int wmain( int Argc, LPTSTR Argv[] )
//int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int)
//int WINAPI WinMain
//(   HINSTANCE hInstance,	// указатель на текущий экземпляр
//    HINSTANCE hPrevInstance,	// указатель на предыдущий запущенный экземпляр
//    LPSTR lpCmdLine,		// командная строка
//    int nCmdShow		// тип отображения окна программы
//)
{
//int Ac=0; char *Av[]={ "1","2" };
//  setlocale( LC_ALL,"en_GB.UTF-8" );
//  setlocale( LC_ALL,".65001" );
//  setlocale( LC_ALL,".UTF8" );
//  _wsetlocale( LC_ALL,L".UTF-16" );
//  setlocale( LC_CTYPE,".65001" );
//  setlocale( LC_ALL, "C-UTF-8" );
//  SetConsoleCP( CP_UTF8 );
//  SetConsoleOutputCP( CP_UTF8 );
//  strcpy( Av[1],"V:\\גכעגכע.vsl" );      //   предусматривается "Аврора"

  printf( "\n\r:1 - " ); printf( "%s",GetCommandLineA() );
  printf( "\n\r:2 - " ); printf( "%s",UnicodeToUTF8( GetCommandLineW() ) );
  printf( "\n\r:3 - " ); printf( "%s + %s\n",Av[0],Av[1] );

 int Argc;
  WCHAR **Argv=CommandLineToArgvW( GetCommandLineW(),&Argc );

 char a0[500]="",a1[500]=""; //wchar_t TT;

  Utf16_To_Utf8Buf( a0,Argv[0],100 );
  Utf16_To_Utf8Buf( a1,Argv[1],100 );
//  wcstombs( a0,Argv[0],100 );
//  wcstombs( a1,Argv[1],100 );
  printf( "\n\r:4 - " ); wprintf( L"%s + %s ",Argv[0],Argv[1] );
  printf( "\n\r:5 - " ); printf( "%s + %s ",a0,a1 );
  printf( "\n\r:6 - " ); printf( "%s + ",UnicodeToUTF8( Argv[0] ) );
                         printf(         UnicodeToUTF8( Argv[1] ) );

  printf( "\n\r:7 - %d/%d УТФ-8 =  ©75 Сахалин-‏יְרוּשָׁלַיִם ---------\n",Ac,Argc ); //getchar(); //strcpy( av[1],a1 );
  return 0;
}


#if 0

// Example Live Demo

#include <stdio.h>
void before_main() __attribute__((constructor));
void after_main() __attribute__((destructor));

void before_main() { printf("This is executed before main.\n"); }
void after_main() { printf("This is executed after main."); }

main() { printf("Inside main\n"); }

Output:
This is executed before main.
Inside main
This is executed after main.

#endif

