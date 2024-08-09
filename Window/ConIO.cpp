#include <stdio.h>#include "ConIO.h"extern "C"
{static COLORS __BACK = BLACK, __FORE = LIGHTGRAY;static HANDLE StdOut = NULL;static CONSOLE_SCREEN_BUFFER_INFO Screen;//
//       COORD dwSize;            -- of the screen buffer//       COORD dwCursorPosition;//       WORD  wAttributes;//       SMALL_RECT srWindow      { SHORT Left,Top,Right,Bottom }//       COORD dwMaximumWindowSize{ SHORT X,Y } maximum size of console window//void gotoxy( int x,int y )            // при x или y<=0 их отсчеты сохраняются  { if( --x<0 )x=Screen.dwCursorPosition.X;
    if( --y<0 )y=Screen.dwCursorPosition.Y;
    SetConsoleCursorPosition( StdOut,(const COORD){ short( x ),short( y ) } );
  }void textattr( unsigned atr )  { SetConsoleTextAttribute( StdOut,atr ); __BACK=COLORS( ( atr>>4 )&15 );                                           __FORE=COLORS( atr&15 ); }void textbackground( COLORS cbk ){ textattr( __FORE|(cbk<<4) ); }void textcolor     ( COLORS clr ){ textattr( clr|(__BACK<<4) ); }
 int wherex(){ return Screen.dwCursorPosition.X+1; }
 int wherey(){ return Screen.dwCursorPosition.Y+1; }
COORD wherexy(){ return (COORD){ short( Screen.dwCursorPosition.X+1 ),                                 short( Screen.dwCursorPosition.Y+1 ) }; }static void Fill( TCHAR c, WORD b, int len, COORD pos )  { DWORD written; FillConsoleOutputCharacter( StdOut,c,len,pos,&written );                   FillConsoleOutputAttribute( StdOut,b,len,pos,&written ); }void clrscr(){ Fill( ' ',__FORE+( __BACK<<4 ),     Screen.dwSize.X * Screen.dwSize.Y, (COORD){ 0,0 } ); gotoxy( 1,1 ); }void clreol(){ Fill( ' ',__FORE+( __BACK<<4 ),     Screen.dwSize.X - Screen.dwCursorPosition.X, Screen.dwCursorPosition ); }////     при изменении размеров окна крайне желательно восстановление//     исходной консоли (если, конечно, на выходе нет _exit( abn ) )//void textsize( int w,int h,int bh )  { const SMALL_RECT S={ 0,0,short( w-1 ),short( h-1 ) };
    SetConsoleWindowInfo( StdOut,true,&S );
    SetConsoleScreenBufferSize( StdOut,(COORD){ short(w),short(h>bh?h:bh) } );  }
void texttitle( const char* title ){ SetConsoleTitle( title ); }
}
// extern "C" to "C++"
#define ArgStr( n ) va_list aV; va_start( aV,fmt ); n=vprintf( fmt,aV ); \
                    va_end( aV ); return n;
int print( int x,int y, const char *fmt,... ){ gotoxy( x,y ); ArgStr( x ) }int print( const char *fmt,... ){ int n; ArgStr( n ) } // CharToOem( str,str )////   Неявная и не особо управляемая инициализация нового окна текстовой консоли//                              Rus-Windows/Cyr(1251) DOS/OEM(866) UTF-8(65001)//#include <locale.h>
static struct _ScreenSave_{ _ScreenSave_(){ FreeConsole(),AllocConsole(); // отсоединение с пересозданием
                 *stderr=*stdout=*freopen( "CONOUT$","w",stdout );  // == "CON"
                  StdOut=GetStdHandle( STD_OUTPUT_HANDLE );                  SetConsoleMode( StdOut,ENABLE_PROCESSED_OUTPUT );
                  GetConsoleScreenBufferInfo( StdOut,&Screen ); clrscr();                  SetConsoleCP( CP_UTF8 );                  SetConsoleOutputCP( CP_UTF8 );//                setlocale( LC_CTYPE,"" );
//                setlocale( LC_ALL,".UTF8" );
                }#if 0
 ~_ScreenSave_(){ SetConsoleWindowInfo( StdOut,true,&(Screen.srWindow) );                  SetConsoleScreenBufferSize( StdOut,Screen.dwSize );                  SetConsoleTextAttribute( StdOut,Screen.wAttributes );                  Screen.dwCursorPosition.Y=Screen.srWindow.Bottom-2;
                  SetConsoleCursorPosition( StdOut,Screen.dwCursorPosition );
                  FreeConsole();
                  _exit( 12 );                }
#endif} // __attribute__( (init_priority(100)) )
  // __attribute__( (constructor) )
  // __attribute__( (destructor) )
 _Save_;  /* статическая инициализация с сохранением исходного состояния экрана
idx xRRGGBB:  0 x000000 BLACK  1 x0000AA BLUE          1 BLUE  2 x00AA00 GREEN         2 GREEN  3 x00AAAA CYAN          3 GREEN + BLUE  4 xAA0000 RED           4 RED  5 xAA00AA MAGENTA       5 RED + BLUE  6 xAA5500 BROWN         6 RED + GREEN // EGA/VGA (brown)  6 xAAAA00                             // Windows (dark yellow)  7 xAAAAAA LIGHTGRAY     7 RED + GREEN + BLUE  8 x555555 DARKGRAY      8 INTENSITY  9 x5555FF LIGHTBLUE     9 INTENSITY + BLUE 10 x55FF55 LIGHTGREEN   10 INTENSITY + GREEN 11 x55FFFF LIGHTCYAN    11 INTENSITY + GREEN + BLUE 12 xFF5555 LIGHTRED     12 INTENSITY + RED 13 xFF55FF LIGHTMAGENTA 13 INTENSITY + RED + BLUE 14 xFFFF55 YELLOW       14 INTENSITY + RED + GREEN 15 xFFFFFF WHITE        15 INTENSITY + RED + GREEN + BLUE*/