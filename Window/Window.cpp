//
//      Простенький интерфейс с экранными окнами для графики OpenGL-MsWindows
//
#include <stdio.h>
#include "Window.h"
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
const GLsizei ListsGroup=256; // вектор группы TrueType шрифтов
static Window *First=NULL; // первое окно в последовательном статическом списке
#define AF (*(Fnt?Fnt:Site->Fnt))
//
// ════════════════════════════════════════════════════════════════════════════
//   Window Procedure - общая для всех процедура обработки Windows прерываний
//     Виртуальные процедуры динамического управления текущим окном Window
//       теряются после переобъявления в охватывающих(производных) классах
//
static bool WinRequest( HWND hWin=NULL )  // текущее состояние запросов Windows
{ MSG WinMsg; if( PeekMessage( &WinMsg,hWin,0,0,PM_REMOVE ) )
              { if( WinMsg.message==WM_QUIT )exit( WinMsg.wParam ); else
                { TranslateMessage( &WinMsg );
                   DispatchMessage( &WinMsg );
                } return true;
              } return false;
}
static void WinExecute( HWND hW=NULL )
                      { if( hW || First )while( WinRequest( hW ) ); }
/* сбор
{ if( hW )while( WinRequest( hW ) ); else
  { Window *W=First;   // поиск по списку окон Window для возникшего прерывания
    while( W ){ if( W->hWnd )while( WinRequest( W->hWnd) ); W=W->Next; }
} } */
Window* Place::Ready()             // либо одно активное, либо все окна Windows
{ if( Site )WinExecute( Site->hWnd ); //else if( First )WinExecute();
  return Site;
}
bool WinReady( Window *Win )       // без указания адреса опрашиваются все окна
{ WinExecute(); if( Win )return Win->Ready()!=NULL; return First!=NULL;
}
//  Подборка настроек для пересохранения графической среды и параметров Windows
//
static void PushMatrix()     //! сброс в стек координатных систем OpenGL-Window
{ glPushAttrib( GL_VIEWPORT_BIT | GL_POLYGON_BIT | GL_ENABLE_BIT ); // что надо
  glPushMatrix(),glMatrixMode( GL_PROJECTION ),glPushMatrix(),
                 glMatrixMode( GL_MODELVIEW );
}
static void PopMatrix()          //! и затем восстановление исходного состояния
{ glMatrixMode( GL_PROJECTION ),glPopMatrix(),
  glMatrixMode( GL_MODELVIEW ),glPopMatrix(),glPopAttrib(); // обратный порядок
}
//     - переключение и временное сохранение состояния контекстной среды OpenGL
//               с отработкой деструктора для восстановления исходного контента
//
bool glAct( const Window *Win ){ return wglMakeCurrent( Win->hDC,Win->hRC ); }
//             конструктор = пролог с восстановлением через эпилог = деструктор
glContext::glContext( const Window* W ) : DC( wglGetCurrentDC() )   // prologue
  { if( W->hDC==DC )DC=0; else { RC=wglGetCurrentContext(); glAct( W ); } }
glContext::~glContext()     // деструктор = эпилог с возвратом былого контекста
  { if( DC )wglMakeCurrent( DC,RC );                                // epilogue
  }
//!  Конструкторы и процедуры сохранения и перерисовки фонового изображения
//!    - все странички всегда связаны с одним из графических окон Window,
//!    - однако отсутствие ссылки Prev - показывает,
//!      что окно исключено из всех операций со списками окон
//     ( это площадка базового окна или фантомное отображение вне Window )
//
Place::Place( Window *Win, byte Mode ): Site( Win ),Signs( Mode ),Up( NULL ),
       pX( 0 ),pY( 0 ),Width( Win->Width ),Height( Win->Height ),
       MouseState(0),xo(0),yo(-1),      // беспросветная унылость мышки/курсора
       chX(0),chY(0),cbX(1), // место и отступ строки текста от боковой границы
       extDraw( NULL ),      // свободное рисование без дополнительных настроек
       extPass( NULL ),      // адреса двух внешних\свободных и бесконтрольных
       extPush( NULL ),      // процедур для параллельного контроля хода мышки
           Fnt( NULL ),      // шрифтовой блок при нуле будет ссылкой на Window
           Img( NULL ),      // фоновый растр с наложенной площадкой PlaceAbove
        isDraw( false ),     //  прорисовка сбрасывается по случаю незавершения
       isMouse( false )      //     предыдущей операции (во избежание рекурсии)
{ if( glAct( Win ) )         // связь контекста OpenGL с требуемым окном Window
  if( this!=(Place*)Win )    // обход неприкасаемого базового окна Window.Place
  { Place *S=(Place*)Win;
    while( S->Up )S=S->Up; S->Up=this;   // новая площадка набрасывается сверху
//  if( Site->Up==this )Site->Save();    // первый фрагмент сохраняет фон окна
    chY=Height-AF.H;         // буквенная позиция изначально будет сверху/слева
} }
Place::~Place()  // освобождение площадки, шрифтов, картинки и всех точек входа
{ if( Site )if( glAct( Site ) )              // +++ средняя площадка вышибается
  { if( Img ){ free( Img ); Img=NULL; }      // все связные объекты расчищаются
    if( Fnt )
    { if( Fnt->Base )glDeleteLists( Fnt->Base,ListsGroup ); Fnt->Base=0;
      if( Fnt->hF )DeleteObject( Fnt->hF ); Fnt->Bit=NULL; free( Fnt ); Fnt=0;
    }
    for( Place *S=(Place*)Site; S; S=S->Up )
     if( S->Up==this ){ S->Up=Up; break; }
    if( Site==this )Site=NULL; else;        // возврат к моменту создания Place
    if( Signs&PlaceAbove )Site->Refresh();  // обновление после удаления Place
} }
//!    главная процедура обработки прерываний Windows
//!
static Window* Find( HWND hWind )
{ Window *Win=First;  // поиск по списку окна Windows для возникшего прерывания
  while( Win )if( hWind==Win->hWnd )break; else Win=Win->Next; return Win;
}
static LRESULT CALLBACK WindowInterruptProcedure
( HWND hWind, UINT message, WPARAM wParam,LPARAM lParam )
{ Window *Win=First;  // поиск по списку окна Windows для возникшего прерывания
//if( message==WM_QUIT )exit( WM_QUIT ); else
  if( (Win=Find( hWind ))!=NULL )         // WM_CREATE=1 - только создание окна
  { if( Win->InterruptProcedure( message,wParam,lParam ) )return 0;
  } return DefWindowProcW( hWind,message,wParam,lParam );
}
bool Window::InterruptProcedure( UINT message, WPARAM wParam, LPARAM lParam )
{ if( message>=WM_MOUSEFIRST && message<=WM_MOUSELAST )
  { if( message==WM_MOUSEWHEEL )    //! отделение запросов мышки от всех прочих
         PutMouse( message,(short)(LOWORD(wParam)),(short)(HIWORD(wParam)) );
    else PutMouse( message,LOWORD(lParam),HIWORD(lParam) );
  } else
  switch( message )
  { case WM_MOVE: WindowX=LOWORD( lParam ),         // 03 перемещение по экрану
                  WindowY=HIWORD( lParam ); break;
    case WM_SIZE: Width = LOWORD( lParam ),         // 05 изменение размеров
                  Height= HIWORD( lParam );         //    Refresh(); break;
    case WM_PAINT:                                  // 15 интерполяция и/или
    { PAINTSTRUCT ps; BeginPaint( hWnd,&ps );       //    перепрорисовка
      if( hDC )if( !isDraw ){ glContext Set( this ); isDraw=true;
                             if( Draw() )Refresh(); isDraw=false; }
      EndPaint( hWnd,&ps ); ValidateRect( hWnd,NULL );
    }
    case WM_KEYUP     : break;                  // 257
    case WM_SYSKEYUP  : break;                  // 261
    case WM_KEYDOWN   :                         // 256
    case WM_SYSKEYDOWN:                         // 260
    { byte Key=0;                               // на входе чистый ключ
      switch( wParam )
      { case VK_PRIOR : Key=_North_East; break; // 33
        case VK_NEXT  : Key=_South_East; break; // 34
        case VK_END   : Key=_South_West; break; // 35
        case VK_HOME  : Key=_North_West; break; // 36
        case VK_LEFT  : Key=_West;       break; // 37
        case VK_UP    : Key=_North;      break; // 38
        case VK_RIGHT : Key=_East;       break; // 39
        case VK_DOWN  : Key=_South;      break; // 40
        case VK_INSERT: Key=_Ins;        break; // 45
        case VK_DELETE: Key=_Del;        break; // 46
       default:
        if( !(HIWORD( lParam )&KF_REPEAT) )    // повторение F-команд исключено
          { if( wParam>=VK_F1 && wParam<=VK_F12 )Key=_F1+wParam-VK_F1; }
      } if( Key )PutChar( Key );               // запись в буфер клавиатуры
    }   break;
    case WM_CHAR:                              // 258 = WM_CHAR message handler
    { byte Key;
      switch( Key=wParam )
      { case VK_BACK  : Key=_BkSp; break;      // 8 -> _BkSp(14)
        case VK_TAB   : Key=_Tab;  break;      // 9 -> _Tab(30)
        case VK_CANCEL: //Close();
                        PostQuitMessage( VK_CANCEL );
                     // exit( VK_CANCEL );     // 3 -> просто на выход
                        return true;
      } PutChar( Key );                        // и ещё одна запись в буфер
    }   break;
//  case WM_TIMER: if( idEvent==wParam )       // 275 -> внутренняя процедура
//                 { PutTimer(); return true; } break;
    case WM_CLOSE:                  // =16 - сигнал о возможности закрытия окна
      DestroyWindow( hWnd ); break; // внутри идёт запрос закрытия окна Windows
    case WM_DESTROY:     // =2 здесь должны быть закрыты все внутренние объекты
         Close(); break; // безусловно (вторично) срабатывает деструктор Window
    default: return false;                     //  => DefWindowProc
  }          return true;                      // return 0 на выход
}
//!   Конструктор создает окно OpenGL, и ... не образует цикла прерываний ...
//     площадка Place в основании окна ортогонализуется и пересохраняется
//
Window::Window( const char *_title, int x,int y, int w,int h )
: Place( this,PlaceOrtho ), // ортогонализуется [-1:1] | PlaceAbove-сохраняется
  hDC( 0 ), hWnd( 0 ), hRC( 0 ), Title( _title ),Next( NULL ),
//ScreenWidth( GetSystemMetrics( SM_CXSCREEN ) ),
//ScreenHeight( GetSystemMetrics( SM_CYSCREEN ) ),
  WindowX( CW_USEDEFAULT ),WindowY( CW_USEDEFAULT ),
  isTimer( false ),isMouse( false ), mSec( 0 ),idEvent( 0x12 ),
  KeyPos( 0 ),KeyPas( 0 ),onKey( false ),extKey( NULL ),extTime( NULL )
{ ATOM atom;
  WNDCLASSW wc;
  HINSTANCE hInstance=GetModuleHandle( NULL );
  PIXELFORMATDESCRIPTOR pfd; //={ sizeof(PIXELFORMATDESCRIPTOR) };
  RECT rectWorkArea;
   SystemParametersInfoW( SPI_GETWORKAREA,0,&rectWorkArea,0 );
   ScreenHeight=rectWorkArea.bottom-rectWorkArea.top;
   ScreenWidth=rectWorkArea.right-rectWorkArea.left;
   ZeroMemory( &wc,sizeof( WNDCLASSW ) );
// memset( (void*)KeyBuffer,0,sizeof( KeyBuffer ) );
//
//  включение нового объекта в конец простого списка - перечисления
//
  static WCHAR ws[16]=L"WGL";
   if( !First )First=this; else
   { Window *Win=First; while( Win->Next )Win=Win->Next; Win->Next=this;
     static int wn=0; swprintf( ws+3,L"_%d",++wn ); idEvent+=wn;
   }
//  регистрация класса с особыми определениями для Microsoft Windows
//
   wc.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC; // тип окна
   wc.lpfnWndProc = WindowInterruptProcedure; // адрес оконной функции
// wc.cbClsExtra = 0;                        // дополнительные данные класса
// wc.cbWndExtra = 0;                        // дополнительные данные для окна
   wc.hInstance = hInstance;                // дескриптор экземпляра приложения
   wc.hIcon=LoadIcon( hInstance,"Icon" );   // IDI_APPLICATION иконки для окна
   wc.hCursor=LoadCursor( NULL,IDC_ARROW ); // дескриптор курсора для окна
// wc.hbrBackground=NULL; // GetStockObject( BLACK_BRUSH ) цвет заполнения окна
// wc.lpszMenuName =NULL;                   // имя главного меню
   wc.lpszClassName=ws;                     // имя класса окна
   if( (atom=RegisterClassW( &wc ))==0 )printf( "\n!\7RegisterClass\n " );
   Locate( x,y,w,h );                       // -- без hWnd - только размерности
   hWnd = CreateWindowW                     // Create main window
   ( wc.lpszClassName,                      // имя класса окна
     _title ? U2W(_title):L"Window-Place",  //   и заголовок окна
     ( _title ? WS_OVERLAPPEDWINDOW:0 )     // ? если без заголовка - нет рамки
              | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
              | WS_POPUPWINDOW | WS_VISIBLE,// стиль окна ? WS_CAPTION
     WindowX,                               // координаты верхнего левого угла
     WindowY,                               // CW_USEDEFAULT
     Width +( !_title ? 2 : GetSystemMetrics( SM_CXSIZEFRAME )*2 ),
     Height+( !_title ? 2 : GetSystemMetrics( SM_CYSIZEFRAME )*2
                          + GetSystemMetrics( SM_CYCAPTION ) ),
     HWND_DESKTOP,                          // дескриптор родительского окна
     NULL,                                  // дескриптор главного меню
     hInstance,                             // дескриптор приложения
     NULL                                   // указатель доп. информации
   );
   ZeroMemory( &pfd,sizeof( PIXELFORMATDESCRIPTOR ) );
   pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );
   pfd.nVersion = 1;                        // set the pixel format for the DC
   pfd.dwFlags = PFD_DRAW_TO_WINDOW         // Draw to Window (not to bitmap)
               | PFD_SUPPORT_OPENGL         // Support OpenGL calls in window
               | PFD_DOUBLEBUFFER;          // Double buffered mode           ~ PFD_SWAP_COPY | PFD_SWAP_LAYER_BUFFERS | PFD_GENERIC_FORMAT
   pfd.iPixelType = PFD_TYPE_RGBA;          // RGBA Color mode
   pfd.cColorBits = 24;      //32
   pfd.cDepthBits = 16;      //32
   pfd.iLayerType = PFD_MAIN_PLANE;
   hDC = GetDC( hWnd );                    // get the device context ( DC )
   SetPixelFormat( hDC,ChoosePixelFormat( hDC,&pfd ),&pfd );
   hRC=wglCreateContext( hDC );            // create
       wglMakeCurrent( hDC,hRC );          // and enable render context ( RC )
//
//  Установка выполнена, теперь прописка размерностей, шрифтов и вложенных окон
//
   Up=NULL;                          // верхний фрагмент в списке наложений
   Site=this;                        // связанный Place ссылается на Window
   Activate().Alfabet().Clear();     // исходный шрифт и настройка площадки
   chY=Height-Fnt->H;                // позиция текстового курсора сверху/слева
}
Window::~Window(){ Close(); }        // закрытие окна может не разрушать Window

void Window::Close()                 // Разрушение окна в обработке прерываний
{ if( Site )                                     // не без предосторожностей
  { KillTimer();                                 // отключение таймера вручную
    while( GetKey() );                           // очистка запросов клавиатуры
    extPush=0; extPass=0; extDraw=0; extKey=0;   // все транзакции отключаются
    while( Up )Up->~Place();                     // сброс наложенных фрагментов
//           Site->~Place();                     // обрушение графического поля
    WinExecute( hWnd );                          // выборка запросов по Windows
   Window *Cur=First;                            // на обработку/очистку списка
    if( Cur==this )First=Cur=Next; else          // первое Window - вхождение
    while( Cur->Next )                           // или последовательный поиск
     { if( Cur->Next!=this )Cur=Cur->Next; else  // себя самого с исключением
         { Cur->Next=Next; break; } }            // при самом первом совпадении
    Site=NULL;                                   // сброс повторов деструктора
    wglMakeCurrent( NULL,NULL );                 // - закрытие OpenGL
    wglDeleteContext( hRC ); hRC=0;              // - без очистки страниц?
    ReleaseDC( hWnd,hDC );   hDC=0;              // освобождение всех ресурсов
    DestroyWindow( hWnd );  hWnd=0;              // - запрос на закрытие окна
    if( Cur )glAct( Cur ); else                  // - на смежный нижний уровень
             PostQuitMessage( WM_QUIT );         // ~~ закрытие последнего окна
} }
//!   Позиционирование окон по правилам Windows
//!
Window& Window::Locate( int X,int Y, int W,int H )     // по правилам Windows
{ int BX=1,BY=1,BC=0;                                 // позиционирование идет
  if( Title ){ BC=GetSystemMetrics( SM_CYCAPTION );  // от левого верхнего угла
               BY=GetSystemMetrics( SM_CYSIZEFRAME );
               BX=GetSystemMetrics( SM_CXSIZEFRAME ); }
  W = ( Width =min( W>0?W:800,ScreenWidth-BX*2 ) )+BX*2;
  H = ( Height=min( H>0?H:600,ScreenHeight-BY*2-BC ) )+BY*2+BC;
  X = X>0 ? X:( X<0 ? X + ScreenWidth-W : 2*( ScreenWidth-W )/3 );
  Y = Y>0 ? Y:( Y<0 ? Y + ScreenHeight-H : ( ScreenHeight-H )/4 );
  WindowX = minmax( 0,X,ScreenWidth-W );  pX=0;
  WindowY = minmax( 0,Y,ScreenHeight-H ); pY=0;
  if( hWnd )
  { // glAct( this );
    // wglMakeCurrent( NULL,NULL );                  // - закрытие OpenGL
    ReleaseDC( hWnd,hDC ); hDC=0;
    // SetWindowPos( hWnd,HWND_TOP,WindowX,WindowY,W,H,SWP_SHOWWINDOW );
    MoveWindow( hWnd,WindowX,WindowY,W,H,true );
    hDC=GetDC( hWnd );
    // UpdateWindow( hWnd );
    Activate().Alfabet();      // размерения окна и переадаптированный шрифт
    chY=Height-Fnt->H;         // позиция текстового курсора сверху/слева
    Refresh();                 // на выходе сброс привязки к текущему контексту
  } return *this;
}
bool Place::Draw()       // в виртуальной среде Draw доступен внутренний пролог
{ if( Site && extDraw )  // настройки OpenGL с контекстным эпилогом прорисовки
  { glContext S( Site ); // подготовка среды к внешнему исполнению с рекурсией
    if( extDraw() )      // -- исполнение внешней независимой транзакции
      { if( this==Site )Save().Refresh(); else Show(); return false; }
  } return Site!=NULL;
}
//!  Блок накопления данных клавиатуры в текущем кольцевом буфере класса Window
//!      (надо бы его спрятать, конечно, но в С++ ? - странно)
//
static byte WinAsyncKeyStates( byte code=0 )            // простой опрос
{ if( GetAsyncKeyState( VK_LSHIFT   ) )code|=LEFT;      // состояния клавиатуры
  if( GetAsyncKeyState( VK_RSHIFT   ) )code|=RIGHT;     // с установкой
  if( GetAsyncKeyState( VK_LCONTROL ) )code|=LCTRL;     // командных аккордов
  if( GetAsyncKeyState( VK_RCONTROL ) )code|=RCTRL;
  if( GetAsyncKeyState( VK_LMENU    ) )code|=L_ALT;
  if( GetAsyncKeyState( VK_RMENU    ) )code|=R_ALT; return code;
}
static const byte lKey=0x3F;    // маска(длина) клавиатурного буфера=64 символа
void Window::PutChar( byte Key )         // занесение одного символа и его кода
{ KeyBuffer[++KeyPas&=lKey].Key=Key;          // в кольцевой буфер для символов
  KeyBuffer[KeyPas].Code=WinAsyncKeyStates();  // и клавиш управляющих аккордов
  if( KeyPas==KeyPos ){ MessageBeep(MB_OK); ++KeyPos&=lKey; }  // сброс-перебор
  if( !onKey )                 // блок рекурсивных прерываний от активного окна
  while( KeyPos!=KeyPas )      // нагромождение очереди запросов от клавиатуры
  { int oK=KeyPos;             // Фиксированная предустановка графической среды
    { // glAct( this );        // со сбоем других внешних транзакций над OpenGL
      if( !KeyBoard( KeyBuffer[ ++KeyPos&=lKey ].Key ) ){ KeyPos=oK; break; }
  } } WinExecute();  // при отказе возвращается обратно в цикл ожидания очереди
}                    // новый символ единожды опробовается к считыванию (=true)
bool Window::KeyBoard( byte key ) // виртуальная процедура обработки прерываний
{ if( extKey ){ glContext S(this);// предустановка графического контента OpenGL
    return extKey( key );         // true - символ принят, false - к возврату
  } return false; //!KeyPas!=KeyPos; либо все недочитанные символы сбрасываются
}
//!  Обращение к клавиатуре через активное и контекстно настроенное окно Window
//                  ! осторожно, здесь предполагается отсутствие вызовов OpenGL
byte Window::WaitKey()    // стандартный цикл ожидания нового символа в Windows
{ onKey=true; while( KeyPos==KeyPas ) //|| isTimer )
               { if( !Site )return onKey=false; else WinExecute();
                 if( !WinRequest() )WaitMessage(); }
  onKey=false; return KeyBuffer[ ++KeyPos&=lKey ].Key;   // ожидание символа כל
}                                                        //    в том же окне
byte Window::GetKey()          // запрос появления нового символа на клавиатуре
   { WinExecute(); if( KeyPas==KeyPos )return 0;
                   else return KeyBuffer[ ++KeyPos&=lKey ].Key; }
byte Window::ScanKey()         // просто проверка текущей активности клавиатуры
   { WinExecute(); return KeyPas==KeyPos ? 0 : KeyBuffer[KeyPos].Key; }
byte Window::ScanStatus()      // обновление в случае отсутствия новых запросов
   { WinExecute(); if( KeyPas==KeyPos )return WinAsyncKeyStates();
                                  else return KeyBuffer[KeyPos].Code; }
//
//!  Внутренние процедуры для реализации виртуальных обращений с мышкой
//!
bool Place::Mouse( int x,int y ) // виртуальная функция динамически подменяется
{ if( extPass )return extPass( x,y ); else// локальная среда внешней транзакции
  if( Site->extPass )return Site->extPass( x+pX,y-pY-Height+Site->Height );
  return false;               // без внешних транзакций считать всё завершённым
}
bool Place::Mouse( int b, int x,int y )
{ if( extPush )return extPush( b,x,y ); else
  if( Site->extPush )return Site->extPush( b,x+pX,y-pY-Height+Site->Height );
  return false;
} //                                      предотвращение рекурсии прерываний
#define call if( !isMouse){ isMouse=true; glContext S(this); bool ret=P->Mouse(
#define end ); if( ret ){ if( P==this )Save().Refresh(); else P->Show(); }   \
                                                            isMouse=false; }
void Window::PutMouse( UINT State, int x,int y )
{ switch( State )                        // ? ( и как теперь с виртуальностью )
  { case WM_MOUSEMOVE    : break;        // ? контекст OpenGL не сверяется
    case WM_LBUTTONDBLCLK:               // - мышка ставит его сама
    case WM_LBUTTONDOWN  : MouseState |=  _MouseLeft;   break;
    case WM_LBUTTONUP    : MouseState &= ~_MouseLeft;   break;
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN  : MouseState |=  _MouseRight;  break;
    case WM_RBUTTONUP    : MouseState &= ~_MouseRight;  break;
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN  : MouseState |=  _MouseMiddle; break;
    case WM_MBUTTONUP    : MouseState &= ~_MouseMiddle; break;
    case WM_MOUSEWHEEL   : MouseState =   _MouseWheel;  break; // сдвиг
   default: MouseState=0; return;
  }
  if( MouseState==_MouseWheel )           // прокрутка проходит сама по себе
  { if( y||x )                            // колёсико не привязано к площадке??
    if( !isMouse )
    { isMouse=true; glContext S( this ); bool ret=Mouse( _MouseWheel,x,y );
      if( ret )Save().Refresh(); isMouse=false; MouseState=0;
    }
  } else
  { Place *P=this; int px=x,py=y;         //   поиск последней/верхней площадки
    for( Place *S=P; S; S=S->Up )         //      по общему списку их наложений
    { int _x=x-S->pX,_y=y+S->pY-Height+S->Height; //       выход будет успешным
      if( unsigned( _y )<=unsigned( S->Height )
       && unsigned( _x )<=unsigned( S->Width ) ){ P=S; py=_y,px=_x; }
    }                                     //  рекурсия мышиных прерываний c
    if( !MouseState )                     //  координатами предыдущих вхождений
    { if( xo<-1 )ClipCursor( NULL ); xo=yo=-1;      //  yo=-1 окно не захвачено
      call px,py end
    } else
    { if( WinAsyncKeyStates()==L_ALT && MouseState==_MouseLeft )
      { if( xo<-1 )ClipCursor( NULL );
        if( yo<0 )xo=x,yo=y; else     // перемещение окна left<Alt> и left<Btn>
        if( x!=xo || y!=yo )Locate
        ( WindowX+x-xo-(Title?GetSystemMetrics(SM_CXSIZEFRAME):1 ),
          WindowY+y-yo-(Title?GetSystemMetrics(SM_CYSIZEFRAME)
                            + GetSystemMetrics(SM_CYCAPTION):1),Width,Height);
      } else if( yo>=0 )yo=-1; else
      { POINT LU={ P->pX,Height-P->pY-P->Height }; ClientToScreen( hWnd,&LU );
        if( xo!=-2 )              // xo=-2 - установлено ограничение хода мышки
        { RECT RC={LU.x,LU.y,LU.x+P->Width,LU.y+P->Height}; ClipCursor(&RC);
        } call MouseState,px,py end xo=-2;
      }
    } // MouseState &= ~_MouseWheel;
  } WinExecute( hWnd  );
}
//!  Прямое и параллельное обращение к таймеру с соблюдением очередей Windows
//!      (все расчеты в миллисекундах, опрокидывание через 49,7 суток)
//!
DWORD volatile RealTime=0,           // время исполнения параллельной процедуры
               StartTime=GetTickCount(); // тики[мс] от времени запуска Windows
DWORD GetTime(){ DWORD T=GetTickCount(); if(StartTime>T)StartTime=T; return T; }
DWORD ElapsedTime(){ return GetTickCount()-StartTime; } //  от старта программы

bool Window::Timer()// контроль транзакций, вызов процедуры внешнего исполнения
   { if( extTime )return extTime(); return false;
   }
//!   Общий таймер не привязан к Window::Place, задействуется лишь единожды
//    и служит квотированию чистого времени вычислений mCon/mWork
//    с задержками на визуализацию и команды внешнего управления mSec/mWait
//    статика определений не полагает возможности рекурсии-реентерабельности
//
static bool (*extFree)()=NULL;      // процедура главного вычислительного цикла
static DWORD mWait=0,mWork=0;       // время задержки и циклов по исполнению
static UINT_PTR tId=16;             // идентификатор нового прерывания

static void CALLBACK TimerProc( HWND hWind,UINT uMsg,UINT_PTR timerId,DWORD St)
{ if( hWind )
  { Window *Win=Find( hWind );       // исполнение в контекстной среде
    if( Win )
    { if( !Win->mSec )Win->isTimer=false; else
      if( !Win->isTimer )
      { Win->isTimer=true; // настройка OpenGL с контекстным эпилогом перерисовки
        { glContext S( Win ); if( Win->Timer() )Win->Save().Refresh(); }
        Win->isTimer=false;
    } } return;                        // фиксируется фоновая подложка всего окна
  }
  if( tId!=timerId )return;              // всякие Sleep и т.п. пусть идут мимо
  ::KillTimer( 0,timerId );              // отключаем таймер, пока не изменился
  if( extFree )                          // запуск вычислений на заданное время
  { DWORD Rt,T;     //,St=GetTickCount() -- отсчет начала приоритетных расчётов
    do{ WinExecute();                    // выборка предыдущих операций Windows
      T=GetTickCount();                  // отметка реального времени счёта
      if( !extFree() )mWait=0;           //! исполнение или полный выход =false
      RealTime+=(Rt=GetTickCount())-T;   // использованный интервал времени #0
      if( mWait && Rt-St>=mWork )        //- перезапуск по истечению свободного
        { tId=::SetTimer( 0,0,mWait,TimerProc ); break; } // рабочего кванта к
    } while( mWait );                                    // повтору безвременья
  } else mWait=0;
}
Window& Window::SetTimer( DWORD mS,bool(*inTm)() )    // время+адрес исполнения
{ if( !mS )KillTimer(); else                          // включается таймер №12
  { ::SetTimer( hWnd,idEvent,mSec=mS,TimerProc ); extTime=inTm; // выбор адреса
  } WinExecute( hWnd ); return *this;                // очистка очереди Windows
}
Window& Window::KillTimer()
{ if( mSec )                             // приостановка с ожиданием завершения
  { mSec=0; while( isTimer )if( !WinRequest() )WaitMessage();
    extTime=NULL; ::KillTimer( hWnd,idEvent );  // теряется внешняя связь
  } return *this;
}
DWORD WaitTime( DWORD Wait,        // активная задержка для внешнего управления
                bool( *inFree )(), // собственно сам вычислительный эксперимент
                DWORD Work )       // время исполнения рабочего процесса [мСек]
{ extFree=inFree,mWork=Work,mWait=Wait;               // инициализация таймеров
  if( Wait )tId=::SetTimer( 0,0,Wait,TimerProc );     // כל = (со всеми окнами)
  while( mWait )if( !WinRequest() )WaitMessage();
  WinExecute(); return RealTime;                      // ожидание чистки mWait
}
// ... все согласованные процедуры объединяются в единый модуль интерактивной
//  графической среды Window::Place в/исключая независимые операции с Юлианским
//   календарем и перекодировками Unicode/UTF-8 для Windows-1251 и OEM-866
//
#include "Window-Place.cpp"
#include "Window-Free.cpp"
// #include "UniCode.cpp"
// #include "Julian.cpp"
// #include "Sym_CCCP.c"
void Break( const char Msg[],... )    // Случай аварийного завершения программы
{ va_list V; va_start( V,Msg );       // или приостановка с первым символом "~"
 char str[vsprintf( 0,UtA(Msg),V )+1]; vsprintf( str,UtA(Msg),V ); va_end( V );
  MessageBox( NULL,str,*Msg=='~'?"Info":"Break",MB_ICONASTERISK|MB_OK );
  if( *Msg!='~' )exit( MB_OK );
}
//Window& Above(){ SetForegroundWindow( hWnd ); SetFocus( hWnd );
// SetActiveWindow( hWnd ); ShowWindow( hWnd,SW_SHOWNA ); return Refresh(); }
