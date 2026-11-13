//
//! «Контекстная графика» – (Window-Place)
//!  Контекстно-зависимая среда построения трехмерной графики OpenGL
//!       с виртуальными и свободными транзакциями прерываний в C++,
//!       со стековым наложением графических и текстовых фрагментов
//!       и многооконного интерфейса OS-Windows/Linux-GLFW
//!  Обобщенный класс Window::Place для OpenGL (и GLFW)
//
//                        ©2010-май, В.Н.Храмушин, СахГУ №2010615850/2010-09-08
#include <StdIO.h>
#include "Window.h"
//
//   Простенький интерфейс с экранными окнами для графики OpenGL-GLFW-MsWindows
//
static Window *First=NULL; // первое окно в последовательном статическом списке
//
// ════════════════════════════════════════════════════════════════════════════
//   Window Procedure - общая для всех процедура обработки Windows прерываний
//        Виртуальные процедуры динамического управления текущим окном Window
//          теряются после переобъявления в охватывающих(производных) классах
#ifdef GLFW
#include "WinGLFW.cpp"                      // GLFW OpenGL
#else
#include "WinMSoft.cpp"                     // MicroSoft Windows (MSDK)
#endif
//      восстановление картинки для всех фрагментов с опцией PlaceAbove + (Img)
//
Window& Window::Refresh()    // сборка изображения с копий в оперативной памяти
{ if( glAct( this ) )
  { glContext Set( this );     // Clear();      // glAdjust( this ) => навсегда
    if( Set.Active )
    { for( Place *S=(Place*)this; S; S=S->Up )S->Rest();  // если есть Show()
      Show();                               // только обновление новой картинки
//    SwapBuffers( hDC );                   // возможно так значительно быстрее
      glFlush();                            // картинка исполнена и - сохранена
    } WaitEvents();                         // и больше ничего рисовать не надо
  } return *this;                           // и опять туда же на исходное окно
}
Place& Place::Refresh(){ if( Site )Site->Refresh(); return *this; }
//
//   и лишь по внешнему виду две общие процедуры для клавиатуры
//
void Window::PutChar( fixed Key )
{    KeyBuffer[++KeyPas&=lKey].Key=Key;  // занесение одного символа и его кода
     KeyBuffer[KeyPas].Code=KeyStates(); // в кольцевой буфер для букв и кодов
//while( isTimer && WinRequest() );     // ожидание выхода таймерных транзакций
  if( KeyPas==KeyPos )
    { MessageBeep( MB_OK ); ++KeyPos&=lKey; } else             // сброс-перебор
  if( !onKey )                 // блок рекурсивных прерываний от активного окна
  while( KeyPos!=KeyPas )      //  нагромождение очереди запросов от клавиатуры
  { int oK=KeyPos;             // Фиксированная предустановка графической среды
    { //glContext Act( this ); // со сбоем других внешних транзакций над OpenGL
      //if( Act.Active )
      { if( !KeyBoard( KeyBuffer[++KeyPos&=lKey].Key ) ){ KeyPos=oK; break; }
        // glFinish(); // ~~~ до/при/пере/пред/установка фона графической среды
        WaitEvents();  // на отказе символ возвращается в цикл ожидания очереди
    } }
  } WaitEvents(); // hWnd );   // освобождение от всех запросов в Windows
  //waitKey=false;
}
bool Window::KeyBoard( fixed key )// виртуальная процедура обработки прерываний
{ if( Ready() )                // здесь Ready делает выборку очереди прерываний
  if( extKey ){ glContext S( this ); // установка графического контента OpenGL
              return extKey( key ); // true - символ принят, false - к возврату
  } return onlyVirtualKeybord;  // вариант отмены привычных ожиданий клавиатуры
}
fixed Window::GetKey()         // запрос появления нового символа на клавиатуре
{ if( Ready() )
  if( KeyPas!=KeyPos )return KeyBuffer[++KeyPos&=lKey].Key; return 0;
}
fixed Window::ScanKey()        // просто проверка текущей активности клавиатуры
{ if( Ready() )
  if( KeyPas!=KeyPos )return KeyBuffer[KeyPos].Key; return 0;
}
fixed Window::ScanStatus()     // обновление в случае отсутствия новых запросов
{ if( Ready() )
  if( KeyPas!=KeyPos )return KeyBuffer[KeyPos].Code;
                      return KeyStates();
}
//  ...  все согласованные процедуры объединяются в единый модуль интерактивной
//  графической среды Window::Place в/исключая независимые операции с Юлианским
//        календарем и перекодировками Unicode/UTF-8 для Windows-1251 и OEM-866
//
#include "Window-Place.cpp"
#include "Window-Free.cpp"
// #include "UniCode.cpp"
// #include "Julian.cpp"
// #include "Sym_CCCP.c"
//
// Случай аварийного завершения программы или приостановка первым символом ="~"
//
fixed Break( const char *Msg, ... ) // _Esc - отмена или _Enter - подтверждение
{ char str[256]=""; bool msg=*Msg=='~'; fixed ans; va_list V;
  va_start( V,Msg ); vsnprintf( str,255,Msg,V ); va_end( V );
  ans=Message( msg?"\x1...к сведению":"\x2Завершение",str );
  if( !msg )exit( MB_OK );                      // ! со всеми деструкторами ...
  return ans;
}
fixed Message( const char *Title,const char *Msg,... ) //?! остановка программы
{ int i,k,n,w,l; va_list V; va_start( V,Msg );
 char str[512]=""; vsnprintf( str,511,Msg,V ); va_end( V ); l=strlen( str );
  for( i=k=n=w=0; i<=l; i++ )
   if( str[i]=='\n' || i==l )
     { str[i]=0; if( Ulen( str+k )>w )w=Ulen( str+k ); k=i;
                 if( i<l )str[i]='\n',++n; else break;
     } w = max( max( 12,w+5 ),Ulen( Title )+4 ); n+=2;
 Window B( 0,0,0,(w+1.8)*9,(n+2.6)*18.0 );
 Place P( &B,PlaceAbove ); TextContext Ts; P.AlfaBit( _8x16 ); glColor3f( 255,0,0 );
  P.Frame( 0,0,w,n ); glColor3f( 0,0,0.5 );
  P.Print( 4,2.5,str );                                            // 63,127,96
  if( *Title==1 )glColor3f( 0,1,0 ); else
  if( *Title==2 )glColor3f( 1,0,0 ); else
                 glColor3f( 0,.3,.3 );
  P.AlfaBit( _8x08 ).Print( 2,2.2,*Title>2?Title:Title+1 );        // 63,96,127
#ifdef GLFW
#if defined( GLFW_VERSION_MINOR ) && ( GLFW_VERSION_MINOR == 5 )                // B.glfwWindow->focusOnShow = true
   glfwSetWindowAttrib( B.glfwWindow,GLFW_FOCUS_ON_SHOW,GLFW_TRUE );            //~FOCUSED~FLOATING
#endif
#else
  SetWindowPos( B.hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE ); //! захват
#endif
  return B.Above().WaitKey();                     // | SWP_NOACTIVATE
}
