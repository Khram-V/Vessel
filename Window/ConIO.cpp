#include <stdio.h>
    if( --y<0 )y=Screen.dwCursorPosition.Y;
    SetConsoleCursorPosition( StdOut,(const COORD){ short( x ),short( y ) } );}
 int wherex(){ return Screen.dwCursorPosition.X+1; }
 int wherey(){ return Screen.dwCursorPosition.Y+1; }
COORD wherexy(){ return (COORD){ short( Screen.dwCursorPosition.X+1 ),
    SetConsoleWindowInfo( StdOut,true,&S );
    SetConsoleScreenBufferSize( StdOut,(COORD){ short(w),short(h>bh?h:bh) } );
#define ArgStr( n ) va_list aV; va_start( aV,fmt ); n=vprintf( fmt,aV )+3; \
static struct _ScreenSave_
                  *stderr=*stdout=*freopen( "CONOUT$","w",stdout ); // == "CON"
                  StdOut=GetStdHandle( STD_OUTPUT_HANDLE );
                  GetConsoleScreenBufferInfo( StdOut,&Screen ); clrscr();
//                setlocale( LC_ALL,".UTF8" );
                }
 ~_ScreenSave_(){ SetConsoleWindowInfo( StdOut,true,&(Screen.srWindow) );
                  SetConsoleCursorPosition( StdOut,Screen.dwCursorPosition );
                  FreeConsole();
#endif
  // __attribute__( (constructor) )
  // __attribute__( (destructor) )
 _Save_; /** статическая инициализация с сохранением исходного состояния экрана
idx xRRGGBB: