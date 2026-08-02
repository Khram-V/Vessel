//
//! «Контекстная графика» – (Window-Place)
//!  Контекстно-зависимая среда построения трехмерной графики OpenGL
//!       с виртуальными и свободными транзакциями прерываний в C++,
//!       со стековым наложением графических и текстовых фрагментов
//!       и многооконного интерфейса OS-Windows/Linux-GLFW
//!  Обобщенный класс Window::Place для OpenGL (и GLFW)
//
//                        ©2010-май, В.Н.Храмушин, СахГУ №2010615850/2010-09-08
//
#include <StdIO.h>
#include "Window.h"
//
//   Простенький интерфейс с экранными окнами для графики OpenGL-GLFW-MsWindows
//
static Window *First=NULL; // первое окно в последовательном статическом списке
//
// ════════════════════════════════════════════════════════════════════════════
//   Window Procedure - общая для всех процедура обработки Windows прерываний
//     Виртуальные процедуры динамического управления текущим окном Window
//       теряются после переобъявления в охватывающих(производных) классах
//
#ifdef GLFW
#include "WinGLFW.cpp"                      // GLFW OpenGL
#else
#include "WinMSoft.cpp"                     // MicroSoft Windows (MSDK)
#endif
//      восстановление картинки для всех фрагментов с опцией PlaceAbove + (Img)
//
Window& Window::Refresh()    // сборка изображения с копий в оперативной памяти
{ if( Site )
  { glContext Set( this );     // Clear();      // glAdjust( this ) => навсегда
    if( Set.Active )
    { for( Place *S=(Place*)this; S; S=S->Up )S->Rest();  // если есть Show()
      Show();                               // только обновление новой картинки
//    SwapBuffers( hDC );                   // возможно так значительно быстрее
      glFlush();                            // картинка исполнена и - сохранена
  } } return *this;                         // и больше ничего рисовать не надо
}                                           //   опять туда же на всё окно
Place& Place::Refresh(){ if( Site )Site->Refresh(); return *this; }
//
//   и лишь по внешнему виду две общие процедуры для клавиатуры
//
//static volatile bool waitKey=false;
//#include "ConIO.h"
//#include <windows.h>

void Window::PutChar( fixed Key )
{
//while( waitKey && WinReady() );       // как-то избавиться от повторов-рекурсии
//if( waitKey )return;
//if( waitKey )WaitTime( 100 ); //delay( 100 );
//waitKey=true;
//                                unsigned K=Key;
//print( 1,38,"\n Key=%X '%s'...    ",Key,&K );

      KeyBuffer[++KeyPas&=lKey].Key=Key; // занесение одного символа и его кода
      KeyBuffer[KeyPas].Code=KeyStates(); // в кольцевой буфер для букв и кодов
//    while( isTimer && WinRequest() ); // ожидание выхода таймерных транзакций
  if( KeyPas==KeyPos )
    { MessageBeep( MB_OK ); ++KeyPos&=lKey; } else             // сброс-перебор
  if( !onKey )                 // блок рекурсивных прерываний от активного окна
  while( KeyPos!=KeyPas )      //  нагромождение очереди запросов от клавиатуры
  { int oK=KeyPos;             // Фиксированная предустановка графической среды
    { //glContext Act( this ); // со сбоем других внешних транзакций над OpenGL
      //glAct( this );
      //if( Act.Active )
      { if( !KeyBoard( KeyBuffer[++KeyPos&=lKey].Key ) ){ KeyPos=oK; break; }
        WaitEvents(); // при отказе символ возвращается в цикл ожидания очереди
      // glFinish();       ~~ до/при/пере/пред/установка фона графической среды
    } }
  }
  WaitEvents(); // hWnd );   // освобождение от всех запросов в Windows
  //waitKey=false;
}
#if 0
bool Window::KeyBoard( fixed key )// виртуальная процедура обработки прерываний
{ if( extKey ){ glContext S( this );  // установка графического контента OpenGL
              return extKey( key ); // true - символ принят, false - к возврату
  } return false; //!KeyPas!=KeyPos; либо все недочитанные символы сбрасываются
}
fixed Window::GetKey()         // запрос появления нового символа на клавиатуре
{ WaitEvents(); if( KeyPas==KeyPos )return 0; return KeyBuffer[ ++KeyPos&=lKey ].Key; } // WaitKey(); }
fixed Window::ScanKey()        // просто проверка текущей активности клавиатуры
{ WaitEvents(); return KeyPas==KeyPos ? 0 : KeyBuffer[KeyPos].Key; }
fixed Window::ScanStatus()      // обновление в случае отсутствия новых запросов
{ WaitEvents(); if( KeyPas==KeyPos )return KeyStates();
                                    return KeyBuffer[KeyPos].Code;
}
#else
bool Window::KeyBoard( fixed key )// виртуальная процедура обработки прерываний
{ if( Ready() )
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
fixed Window::ScanStatus()      // обновление в случае отсутствия новых запросов
{ if( Ready() )
  if( KeyPas!=KeyPos )return KeyBuffer[KeyPos].Code;
                      return KeyStates();
}
#endif
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
{ char str[256]; bool msg=*Msg=='~'; fixed ans; va_list V;
  va_start( V,Msg ); vsnprintf( str,255,Msg,V ); va_end( V );
//#pragma omp barrier
  Window B( msg?"...к сведению":"Завершение",0,0,max(24,Ulen(str))*9+64,60 );
  glColor3b( msg?63:127,127,63 ); B.AlfaBit(_8x16).Print( 4,-2,str );
  ans=B.WaitKey(); if( !msg )exit( MB_OK+4 ); // ! со всеми деструкторами ...
  return ans;
}
fixed Message( const char *Title,const char *Msg,... ) //?! остановка программы
{ va_list V; va_start(V,Msg); char *str=(char*)malloc( vsprintf( 0,Msg,V )*2+4 );
                          vsprintf( str,Msg,V ); va_end( V );
  Window B( Title,0,0,max(24,Ulen(str))*9+64,60 );
  glColor3b( 63,127,63 ); B.AlfaBit(_8x16).Print( 4,-2,str ); free( str );
//#pragma omp barrier
  return B.WaitKey();
}


