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
void Window::PutChar( fixed Key )
{    WaitEvents();                    // как-то избавиться от повторов-рекурсии
     KeyBuffer[++KeyPas&=lKey].Key=Key;  // занесение одного символа и его кода
     KeyBuffer[KeyPas].Code=KeyStates();  // в кольцевой буфер для букв и кодов
  if( KeyPas==KeyPos )
    { MessageBeep( MB_OK ); ++KeyPos&=lKey; return; }          // сброс-перебор
  if( !onKey )                 // блок рекурсивных прерываний от активного окна
  while( KeyPos!=KeyPas )      //  нагромождение очереди запросов от клавиатуры
  { int oK=KeyPos;             // Фиксированная предустановка графической среды
    { glContext Act( this );   // со сбоем других внешних транзакций над OpenGL
      if( Act.Active )
      { if( !KeyBoard( KeyBuffer[++KeyPos&=lKey].Key ) ){ KeyPos=oK; break; }
        WaitEvents();        // при отказе возвращается в цикл ожидания очереди
        glFinish();          // при/пере/пред/установка фона графической среды
  } } }
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
{ if( extKey ){ glContext S( this );  // установка графического контента OpenGL
              return extKey( key ); // true - символ принят, false - к возврату
  } return false; //!KeyPas!=KeyPos; либо все недочитанные символы сбрасываются
}
fixed Window::GetKey()         // запрос появления нового символа на клавиатуре
{ //if( Ready() )
  if( KeyPas!=KeyPos )return KeyBuffer[++KeyPos&=lKey].Key; return 0;
}
fixed Window::ScanKey()        // просто проверка текущей активности клавиатуры
{ //if( Ready() )
  if( KeyPas!=KeyPos )return KeyBuffer[KeyPos].Key; return 0;
}
fixed Window::ScanStatus()      // обновление в случае отсутствия новых запросов
{ //if( Ready() )
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
void Break( const char Msg[],... )    // Случай аварийного завершения программы
{ va_list V; va_start( V,Msg );       // или приостановка с первым символом "~"
 int Len=vsprintf( 0,Msg,V ); char *str=(char*)malloc( Len+16 );
         vsprintf( str,Msg,V ); va_end( V );       //! с длиной что-то не то ??
//if( First )glFinish();
  MessageBoxW( NULL,U2W(str),*Msg=='~'?L"Info":L"Break",MB_ICONASTERISK|MB_OK );
  free( str ); if( *Msg!='~' )exit( MB_OK );
}
// char str[vsprintf( 0,Msg,V )+4+100];

