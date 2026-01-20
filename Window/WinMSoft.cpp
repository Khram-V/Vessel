//
// ════════════════════════════════════════════════════════════════════════════
//   Window Procedure - общая для всех процедура обработки Windows прерываний
//     Виртуальные процедуры динамического управления текущим окном Window
//       теряются после переобъявления в охватывающих(производных) классах
//
static bool WinRequest( HWND hWin=NULL )  // текущее состояние запросов Windows
{ MSG WinMsg; if( PeekMessage( &WinMsg,hWin,0,0,PM_REMOVE ) )
              { //if( WinMsg.message==WM_QUIT )
                //{ while( First )First->Close(); exit( WinMsg.wParam ); } else
                { TranslateMessage( &WinMsg );
                   DispatchMessage( &WinMsg );
                } return true;
              } return false;
}
static void WaitEvents( HWND hW=NULL )// с нулём опрос сразу всех активных окон
                      { if( hW || First )while( WinRequest( hW ) ); }
/* сбор
{ if( hW )while( WinRequest( hW ) ); else
  { Window *W=First;   // поиск по списку окон Window для возникшего прерывания
    while( W ){ if( W->hWnd )while( WinRequest( W->hWnd) ); W=W->Next; }
} } */
Window* Place::Ready()             // либо одно активное, либо все окна Windows
{ if( First )if( Site )if( Site->hWnd )
    { WaitEvents( Site->hWnd ); return Site; } return NULL;
}
bool WinReady( Window *Win )       // без указания адреса опрашиваются все окна
{ WaitEvents(); if( Win )return Win->Ready()!=NULL; else //if( First )WaitEvents();
  return First!=NULL;
}
//! Контекстно-связанный интерфейс с экранными окнами для графики OpenGL
//  формальные построения и динамические переустановки текстовых страниц
//           и графических фрагментов внутри главного окна Window
//
//  Подборка настроек для пересохранения графической среды и параметров Windows
//     - переключение и временное сохранение состояния контекстной среды OpenGL
//               с отработкой деструктора для восстановления исходного контента
//
bool glAct( const Window *W ){ return wglMakeCurrent( W->hDC,W->hRC ); }
//{ if( W ){ bool B=wglMakeCurrent( W->hDC,W->hRC );
//            if( B )if( W->hDC ){ WaitEvents(); return B; } } return false;
//}
//             конструктор = пролог с восстановлением через эпилог = деструктор
//
glContext::glContext( const Window* W ): Active( true ),DC( wglGetCurrentDC() )
  { if( W->hDC==DC )DC=0; else   // установка размерений нового растрового поля
    { RC=wglGetCurrentContext(); // glPushAttrib(GL_VIEWPORT_BIT);  // prologue
      Active=glAct( W );
   // if( Active )glViewport( W->pX,W->pY,W->Width,W->Height );
  } }
glContext::~glContext()     // деструктор = эпилог с возвратом былого контекста
  { if( DC )wglMakeCurrent( DC,RC ); //,glPopAttrib();              // epilogue
  }
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
#include <wchar.h>
bool Window::InterruptProcedure( UINT message, WPARAM wParam, LPARAM lParam )
{ if( message>=WM_MOUSEFIRST && message<=WM_MOUSELAST )
  { if( message==WM_MOUSEWHEEL )    //! отделение запросов мышки от всех прочих
         PutMouse( message,(short)(LOWORD(wParam)),(short)(HIWORD(wParam)) );
    else PutMouse( message,LOWORD(lParam),HIWORD(lParam) );
  } else
  switch( message )
  { //case WM_CREATE:                           // Настройка пропуска кликов мыши
    // SetWindowLongPtr(hWnd, GWL_EXSTYLE,
    // (LONG_PTR)(GetWindowLongPtr(hWnd, GWL_EXSTYLE)|WS_EX_LAYERED)); // | WS_EX_TRANSPARENT
    // SetLayeredWindowAttributes( hWnd,0,200,LWA_ALPHA ); break; // Установка прозрачности
    //case WM_NCHITTEST:                // Обработка системных операций с окном
    //   return DefWindowProc( hWnd,message,wParam,lParam );
    case WM_MOVE: WindowX=LOWORD( lParam ),         // 03 перемещение по экрану
                  WindowY=HIWORD( lParam ); break;
    case WM_SIZE: Width = LOWORD( lParam ),         // 05 изменение размеров
                  Height= HIWORD( lParam );         //    Refresh(); break;
    case WM_PAINT:                                  // 15 интерполяция и/или
    { PAINTSTRUCT ps; BeginPaint( hWnd,&ps );       //    перепрорисовка
      if( hDC )if( !isDraw ){ glContext Set( this );
                              if( Set.Active ){ isDraw=true;
                                if( Draw() )Refresh(); isDraw=false; } }
      EndPaint( hWnd,&ps ); ValidateRect( hWnd,NULL );
    }
    case WM_KEYUP     : break;                  // 257
    case WM_SYSKEYUP  : break;                  // 261
    case WM_KEYDOWN   :                         // 256
    case WM_SYSKEYDOWN:                         // 260
    { fixed Key=0;
//    WaitEvents(); // hWnd );                  // на входе чистый ключ
      switch( wParam )                          // командные перекодировки
      {//ase VK_RETURN: Key=_Enter;      break; // 13≡13 => так будет повтор
        case VK_PRIOR : Key=_North_East; break; // 33⇒ 9 - 8+1
        case VK_NEXT  : Key=_South_East; break; // 34⇒12 - 8+4
        case VK_END   : Key=_South_West; break; // 35⇒ 6 - 2+4
        case VK_HOME  : Key=_North_West; break; // 36⇒ 3 - 2+1
        case VK_LEFT  : Key=_West;       break; // 37⇒ 2
        case VK_UP    : Key=_North;      break; // 38⇒ 1
        case VK_RIGHT : Key=_East;       break; // 39⇒ 8
        case VK_DOWN  : Key=_South;      break; // 40⇒ 4
        case VK_INSERT: Key=_Ins;        break; // 45⇒28
        case VK_DELETE: Key=_Del;        break; // 46⇒29
//      case VK_ESCAPE: Key=_Esc;        break; // 27⇒27 - двойной прострел
       default:
        if( !(HIWORD( lParam )&KF_REPEAT) )    // повторение F-команд исключено
          { if( wParam>=VK_F1 && wParam<=VK_F12 )Key=_F1+wParam-VK_F1; }
      } if( Key )PutChar( Key );                   // запись в буфер клавиатуры
    } break;
//  case WM_TIMER: if( idEvent==wParam )         // 275 -> внутренняя процедура
//                 { PutTimer(); return true; } break;
    case WM_CHAR:                              // 258 = WM_CHAR message handler
    { WPARAM Key;
//    WaitEvents(); // hWnd );                          // на входе чистый ключ
      switch( Key=wParam )
      { case VK_BACK  : Key=_BkSp;  break;              // 8 -> _BkSp(14)
        case VK_TAB   : Key=_Tab;   break;              // 9 -> _Tab (30)
        case VK_CANCEL: while( First )First->Close();   // 3 -> просто на выход
                     PostQuitMessage( VK_CANCEL ); return false;   // с обходом
      } if( Key )PutChar( Key );                   // запись в буфер UniCode-16
    } break;
    case WM_CLOSE: Close(); // break;//=16 - сигнал о возможности закрытия окна
     // DestroyWindow(hWnd); break; // внутри идёт запрос закрытия окна Windows
    case WM_DESTROY:     // =2 здесь должны быть закрыты все внутренние объекты
      if( !First )PostQuitMessage(0); break; // с кодом 0—нормальное завершение
    case WM_QUIT:        // безусловно (вторично) срабатывает деструктор Window
         while( First )First->Close();
    default: return false; // DefWindowProc( hWnd,message,wParam,lParam );
  }          return true;  // освобождение очереди от нераспознанных на выход
}
//!   Конструктор создает окно OpenGL, и ... не образует цикла прерываний ...
//     площадка Place в основании окна ортогонализуется и пересохраняется
//
//static const UINT_PTR tId=11;      // идентификатор таймера общего прерывания

Window::Window( const char *_title, int x,int y, int w,int h )
: Place( this,PlaceOrtho ), // ортогонализуется [-1:1] | PlaceAbove-сохраняется
  hDC( 0 ), hWnd( 0 ), hRC( 0 ), Caption( _title ), Next( NULL ),
//ScreenWidth( GetSystemMetrics( SM_CXSCREEN ) ),
//ScreenHeight( GetSystemMetrics( SM_CYSCREEN ) ),
  WindowX( CW_USEDEFAULT ),WindowY( CW_USEDEFAULT ),
  isTimer( 0 ),isMouse( false ),mSec( 0 ),idEvent( 12 ),    // tId + номер окна
  KeyPos( 0 ),KeyPas( 0 ),onKey( false ),extKey( NULL ),extTime( NULL )
{ //ATOM atom;
  WNDCLASSW wc={ sizeof( WNDCLASSW ) };
  HINSTANCE hInstance=GetModuleHandle( NULL );
  PIXELFORMATDESCRIPTOR pfd; //={ sizeof(PIXELFORMATDESCRIPTOR) };
  RECT rectWorkArea;
   SystemParametersInfoW( SPI_GETWORKAREA,0,&rectWorkArea,0 );
   ScreenHeight=rectWorkArea.bottom-rectWorkArea.top;
   ScreenWidth=rectWorkArea.right-rectWorkArea.left;
// ZeroMemory( &wc,sizeof( WNDCLASSEXW ) );
// memset( (void*)KeyBuffer,0,sizeof( KeyBuffer ) );
//
// включение нового объекта в конец простого списка - перечисления
//
  static WCHAR ws[16]=L"WGL";
   if( !First )First=this; else
   { Window *Win=First; while( Win->Next )Win=Win->Next; Win->Next=this;
     static int wn=0; swprintf( ws+3,L"_%d",++wn ); idEvent+=wn;      // tId+wn
   }
//  регистрация класса с особыми определениями для Microsoft Windows
//
// wc.cbSize = sizeof(WNDCLASSEX);
   wc.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC; // тип окна
   wc.lpfnWndProc = WindowInterruptProcedure; // адрес оконной функции
// wc.cbClsExtra = 0;                       // Ex: дополнительные данные класса
// wc.cbWndExtra = 0;                       // ~ дополнительные данные для окна
   wc.hInstance = hInstance;                // дескриптор экземпляра приложения
   wc.hIcon=LoadIcon( hInstance,"Icon" );   // IDI_APPLICATION иконки для окна
   wc.hCursor=LoadCursor( NULL,IDC_ARROW ); // дескриптор курсора для окна
// wc.hbrBackground=NULL; // GetStockObject( BLACK_BRUSH ) цвет заполнения окна
// wc.lpszMenuName =NULL;                  // имя главного меню
   wc.lpszClassName=ws;                    // имя класса окна
 /*atom=*/ RegisterClassW( &wc );          // Ex:==0 => "\n!\7RegisterClass\n "
   Locate( x,y,w,h );                      // -- без hWnd - только размерности
   hWnd = CreateWindowW                    // Create main window
   ( //WS_EX_LAYERED | WS_EX_TRANSPARENT,  // Прозрачное, проницаемое для мышки
     wc.lpszClassName,                     // имя класса окна
     _title ? U2W(_title):L"Window-Place", //   и заголовок окна
     ( _title ? WS_OVERLAPPEDWINDOW:0 )    // ? если без заголовка - нет рамки
              | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
              | WS_POPUPWINDOW | WS_VISIBLE, // стиль окна ? WS_CAPTION
     WindowX,                              // координаты верхнего левого угла
     WindowY,                              // CW_USEDEFAULT
     Width +( !_title ? 2 : GetSystemMetrics( SM_CXSIZEFRAME )*2 ),
     Height+( !_title ? 2 : GetSystemMetrics( SM_CYSIZEFRAME )*2
                          + GetSystemMetrics( SM_CYCAPTION ) ),
     HWND_DESKTOP,                         // дескриптор родительского окна
     NULL,                                 // дескриптор главного меню
     hInstance,                            // дескриптор приложения
     NULL                                  // указатель доп.информации
   );
   ZeroMemory( &pfd,sizeof( PIXELFORMATDESCRIPTOR ) );
   pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );
   pfd.nVersion = 1;                 // set the pixel format for the DC
   pfd.dwFlags = PFD_DRAW_TO_WINDOW  // Draw to Window (not to bitmap)
               | PFD_SUPPORT_OPENGL  // Support OpenGL calls in window
               | PFD_DOUBLEBUFFER;   // Double buffered mode ~ PFD_SWAP_COPY | PFD_SWAP_LAYER_BUFFERS | PFD_GENERIC_FORMAT
   pfd.iPixelType = PFD_TYPE_RGBA;   // RGBA Color mode
   pfd.cColorBits = 24;   // 32
   pfd.cDepthBits = 16;   // 32
   pfd.iLayerType = PFD_MAIN_PLANE;
   hDC = GetDC( hWnd );              // get the device context ( DC )
   SetPixelFormat( hDC,ChoosePixelFormat( hDC,&pfd ),&pfd );
   hRC=wglCreateContext( hDC );      // create
       wglMakeCurrent( hDC,hRC );    // and enable render context ( RC )
//
//  Установка выполнена, теперь прописка размерностей, шрифтов и вложенных окон
//
   Up=NULL;                          // верхний фрагмент в списке наложений
   Site=this;                        // связанный Place ссылается на Window
   Activate().AlfaVector().Clear();  // исходный шрифт и настройка площадки
   chY=Height-AlfaHeight();          // позиция текстового курсора сверху/слева
}
Window::~Window(){ if(this)Close(); } // Разрушение окна в обработке прерываний
#if 0
{ if( Site && hWnd )                             // не без предосторожностей
  { KillTimer();                                 // отключение таймера вручную
    while( GetKey() );                           // очистка запросов клавиатуры
    while( Up )Up->~Place();                     // сброс наложенных фрагментов
    //       Site->~Place();                     // обрушение графического поля
    //   WaitEvents( hWnd );                     // выборка запросов по Windows
    extPush=0; extPass=0; extDraw=0; extKey=0;   // все транзакции отключаются
   Window *Cur=First;                            // на обработку/очистку списка
    if( First==this )First=Next,Cur=Next; else   // первое Window - вхождение
    while( Cur->Next )                           // или последовательный поиск
     { if( Cur->Next!=this )Cur=Cur->Next; else  // себя самого с исключением
         { Cur->Next=Next; break; }              // при самом первом совпадении
     } Site=NULL;                                // сброс повторов деструктора
    wglMakeCurrent( NULL,NULL );                 // - закрытие OpenGL
    wglDeleteContext( hRC ); hRC=0;              // - без очистки страниц?
    ReleaseDC( hWnd,hDC );   hDC=0;              // освобождение всех ресурсов
    DestroyWindow( hWnd );  hWnd=0;              // - запрос на закрытие окна
    if( Cur )/*glAct( Cur ),*/ Cur->Above();     // - на смежный нижний уровень
  //    else PostQuitMessage( WM_QUIT );         // ~~ закрытие последнего окна
  //WaitEvents();     // ожидание завершения всех операция по программе в целом
  }
}
#endif
void Window::Close()
{ if( First )if( Site && hWnd )// не без предосторожностей this->~Window();
  { KillTimer();                                 // отключение таймера вручную
    while( GetKey() );                           // очистка запросов клавиатуры
    while( Up )Up->~Place();                     // сброс наложенных фрагментов
    //       Site->~Place();                     // обрушение графического поля
    //   WaitEvents( hWnd );                     // выборка запросов по Windows
    extPush=0; extPass=0; extDraw=0; extKey=0;   // все транзакции отключаются
   Window *Cur=First;                            // на обработку/очистку списка
    if( Cur==this )First=Cur=Next; else          // первое Window-вхождение
    while( Cur->Next )                           // и надо особо уважить поиски
     { if( Cur->Next==this ){ Cur->Next=Next; break; } Cur=Cur->Next; }
    Site=NULL;                                   // сброс повторов деструктора
    wglMakeCurrent( NULL,NULL );                 // - закрытие OpenGL
    wglDeleteContext( hRC ); hRC=0;              // - без очистки страниц?
    ReleaseDC( hWnd,hDC );   hDC=0;              // освобождение всех ресурсов
    DestroyWindow( hWnd );   hWnd=0;             // - запрос на закрытие окна
    if( Cur )glAct( Cur ),Cur->Above();          // - на смежный нижний уровень
  //    else PostQuitMessage( WM_QUIT );         // ~~ закрытие последнего окна
  //WaitEvents();     // ожидание завершения всех операция по программе в целом
  }
}
//
//!   Позиционирование окон по правилам Windows
//!
Window& Window::Locate( int X,int Y, int W,int H )     // по правилам Windows
{ int BX=1,BY=1,BC=0;  //const int x=X,y=Y,w=W,h=H;    // позиционирование идет
  if( Caption ){ BC=GetSystemMetrics( SM_CYCAPTION );  // от левого верхнего угла
                 BY=GetSystemMetrics( SM_CYSIZEFRAME );
                 BX=GetSystemMetrics( SM_CXSIZEFRAME ); }
  W = ( Width =min( W>0?W:800,ScreenWidth-BX*2 ) )+BX*2;
  H = ( Height=min( H>0?H:600,ScreenHeight-BY*2-BC ) )+BY*2+BC;
  X = X>0 ? X:( X<0 ? X + ScreenWidth-W : 2*( ScreenWidth-W )/3 );
  Y = Y>0 ? Y:( Y<0 ? Y + ScreenHeight-H : ( ScreenHeight-H )/4 );
  WindowX = minmax( 0,X,ScreenWidth-W );  pX=0;
  WindowY = minmax( 0,Y,ScreenHeight-H ); pY=0;
  if( hWnd )
  { //glAct( this );          //  wglMakeCurrent( NULL,NULL ) - закрытие OpenGL
    ReleaseDC( hWnd,hDC ); hDC=0;
//  SetWindowPos( hWnd,HWND_TOP,WindowX,WindowY,W,H,SWP_SHOWWINDOW );
//  MoveWindow( hWnd,x,y,w,h,true );
    MoveWindow( hWnd,WindowX,WindowY,W,H,true );
    hDC=GetDC( hWnd );
//  UpdateWindow( hWnd );
    Activate(); //.AlfaVector(); // размерения окна и переадаптированный шрифт
//  chY=Height-Th;             // позиция текстового курсора сверху/слева
    Refresh();                 // на выходе сброс привязки к текущему контексту
  } return *this;
}
//! Блок накопления данных клавиатуры в текущем кольцевом буфере класса Window
//!      (надо бы его спрятать, конечно, но в С++ ? - странно)
//
static fixed KeyStates( fixed code=0 )                  // простой опрос
{ if( GetAsyncKeyState( VK_LSHIFT   ) )code|=LEFT;      // состояния клавиатуры
  if( GetAsyncKeyState( VK_RSHIFT   ) )code|=RIGHT;     // с установкой
  if( GetAsyncKeyState( VK_LCONTROL ) )code|=LCTRL;     // командных аккордов
  if( GetAsyncKeyState( VK_RCONTROL ) )code|=RCTRL;
  if( GetAsyncKeyState( VK_LMENU    ) )code|=L_ALT;
  if( GetAsyncKeyState( VK_RMENU    ) )code|=R_ALT; return code;
}
//!  Обращение к клавиатуре через активное и контекстно настроенное окно Window
//                  ! осторожно, здесь предполагается отсутствие вызовов OpenGL
fixed Window::WaitKey()   // стандартный цикл ожидания нового символа в Windows
{                //  HWND FWin=GetFocus(); SetActiveWindow( hWnd ); ~~ Above();
  if( onKey )return false; else onKey=true;     // isTimer=0; SetFocus( hWnd );
//glAct( this );
  while( Site && KeyPos==KeyPas )WinRequest();           // hWnd // ||isTimer>0
  WaitEvents( hWnd );
//  if( hWnd==GetFocus() ) // WaitEvents(); else
//  { if( !WinRequest( hWnd ) )                          // ожидание символа כל
//    { WaitEvents(); if( !Site )return onKey=false; } } //   в том же окне:
  onKey=false;                                           // SetFocus( FWin );
  return KeyBuffer[++KeyPos&=lKey].Key;                  //   wctob( key )=>
}                                                        //   Uni16=>Win1251
//
// мышка работает на площадках, однако прерывания организуются из главного окна
//                                           предотвращение рекурсии прерываний
#define call if( !isMouse){ isMouse=true; glContext S(this); bool ret=P->Mouse(
#define end ); if( ret ){ if( P==this )Save().Refresh(); else P->Show(); } \
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
  } } else
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
    { if( KeyStates()==L_ALT && MouseState==_MouseLeft )
      { if( xo<-1 )ClipCursor( NULL );
        if( yo<0 )xo=x,yo=y; else     // перемещение окна left<Alt> и left<Btn>
        if( x!=xo || y!=yo )Locate
        ( WindowX+x-xo-(Caption?GetSystemMetrics(SM_CXSIZEFRAME):1 ),
          WindowY+y-yo-(Caption?GetSystemMetrics(SM_CYSIZEFRAME)
                              + GetSystemMetrics(SM_CYCAPTION):1),Width,Height);
      } else if( yo>=0 )yo=-1; else
      { POINT LU={ P->pX,Height-P->pY-P->Height }; ClientToScreen( hWnd,&LU );
        if( xo!=-2 )              // xo=-2 - установлено ограничение хода мышки
        { RECT RC={LU.x,LU.y,LU.x+P->Width,LU.y+P->Height}; ClipCursor(&RC);
        } call MouseState,px,py end xo=-2;
      }
    } // MouseState &= ~_MouseWheel;
  }   // WaitEvents( hWnd  );
}

bool Window::Timer()// контроль транзакций, вызов процедуры внешнего исполнения
   { if( extTime )return extTime(); return false;
   }
//!  Прямое и параллельное обращение к таймеру с соблюдением очередей Windows
//!      (все расчеты в миллисекундах, опрокидывание через 49,7 суток)
//!
DWORD volatile RealTime=0,           // время исполнения параллельной процедуры
               StartTime=GetTickCount(); // тики[мс] от времени запуска Windows
DWORD GetTime(){ DWORD T=GetTickCount(); if(StartTime>T)StartTime=T; return T; }
DWORD ElapsedTime(){ return GetTickCount()-StartTime; } //  от старта программы
//
//!   Общий таймер не привязан к Window::Place, задействуется лишь единожды
//    и служит квотированию чистого времени вычислений mCon/mWork
//    с задержками на визуализацию и команды внешнего управления mSec/mWait
//    статика определений не полагает возможности рекурсии-реентерабельности
//
static bool (*extFree)()=NULL;      // процедура главного вычислительного цикла
static DWORD mWait=0,mWork=0;       // время задержки и циклов по исполнению
#if 1
static UINT_PTR IdT=0;               // базовый идентификатор общего прерывания

static void CALLBACK TimerProc( HWND hWind,UINT uMsg,UINT_PTR timerId,DWORD St)
{ if( hWind )                                 // при достижении очереди таймера
  { Window *Win=Find( hWind );                // исполнение в контекстной среде
    if( Win )if( timerId==Win->idEvent )
    { if( !Win->mSec )Win->isTimer=0; else    // при завершении всех транзакций
      if( !Win->isTimer )  // настройка OpenGL контекстным эпилогом перерисовки
      { // glContext S( Win );
        Win->isTimer++;
        ::KillTimer( hWind,timerId );
        { // glContext S( Win );
          // if( S.Active )
          if( glAct( Win ) )
          if( Win->Timer() )Win->Save().Refresh();  // на виртуальную процедуру
          WaitEvents( Win->hWnd );   // и для верности подождать исполнения ...
        }
        ::SetTimer( hWind,timerId,Win->mSec,TimerProc );   // ...заведомо старт
        Win->isTimer=0;              //  isTimer--;   с проблемами незавершёнки
    } } return;                      // фиксируется фоновая подложка всего окна
  }
  if( IdT!=timerId )return;              // всякие Sleep и т.п. пусть идут мимо
    ::KillTimer( 0,timerId );            // отключаем таймер, пока не изменился
  if( extFree )                          // запуск вычислений на заданное время
  { DWORD Rt,T;    //, St=GetTickCount() -- отсчет начала приоритетных расчётов
    do
    { T=GetTickCount();                  //= отметка реального времени расчётов
      if( !extFree() )mWait=0;           //! исполнение или полный выход =false
      RealTime+=(Rt=GetTickCount())-T;   //  использованный интервал времени #0
      if( mWait && Rt-St>=mWork )        //- перезапуск по истечению указанного
      { IdT=::SetTimer( 0,0,mWait,TimerProc );   // рабочего кванта времени и
        break;                                   // тогда к повтору безвременья
    } } while( mWait );
  } else mWait=0;                  //if( IdT ){ ::KillTimer( 0,IdT ); IdT=0; }
}
DWORD WaitTime( DWORD Wait,        // активная задержка для внешнего управления
                bool( *inStay )(), // собственно сам вычислительный эксперимент
                DWORD Work )       // время исполнения рабочего процесса [мСек]
{ extFree=inStay,mWork=Work,mWait=Wait;               // инициализация таймеров
  if( Wait )IdT=::SetTimer( 0,0,Wait,TimerProc );     // כל = (со всеми окнами)
  while( First && mWait )WaitEvents();                // ожидание чистки mWait
  return RealTime;                                    // выход в особом случае
}
                    //while( isTimer>1 )if( !WinRequest( hWnd ) )WaitMessage();
#else
void Window::PutTimer() // контекстная транзакция конкретного окна как процесса
   { if( !mSec )isTimer=0; else // при завершении всех транзакций - ненадёжная
     if( !isTimer )       // настройка OpenGL контекстным эпилогом перерисовки
     { glContext S( this );
       if( S.Active )
       {  isTimer++;
//       ::KillTimer( hWnd,idEvent );
         if( Timer() )Save().Refresh();                    // запрос транзакции
//       ::SetTimer( hWnd,idEvent,mSec,0 );                // ...заведомо старт
         WaitEvents( hWnd );                               // и для верности...
         isTimer=0;
   } } }

/// !!! -- не проходит назначение таймерного  индекса ???
static void CALLBACK TimerProc( HWND hWind,UINT uMsg,UINT_PTR timerId,DWORD St)
{ if( hWind )                                 // при достижении очереди таймера
  { Window *Win=Find( hWind );                // исполнение в контекстной среде
    if( Win )if( timerId==Win->idEvent )
    { if( !Win->mSec )Win->isTimer=0; else    // при завершении всех транзакций
      if( !Win->isTimer )  // настройка OpenGL контекстным эпилогом перерисовки
      { glContext S( Win );
        if( S.Active )
        { Win->isTimer++;
//        ::KillTimer( hWind,timerId );
          if( Win->Timer() )Win->Save().Refresh();         // запрос транзакции
//        ::SetTimer( hWind,timerId,Win->mSec,TimerProc ); // ...заведомо старт
          WaitEvents();                                    // и для верности...
          Win->isTimer=0;
    } } } return;                    // фиксируется фоновая подложка всего окна
  }
  if( tId!=timerId )return;              // всякие Sleep и т.п. пусть идут мимо
  ::KillTimer( 0,tId );              // отключаем таймер, пока не изменился
  if( extFree )                          // запуск вычислений на заданное время
  { DWORD Rt,T;    //, St=GetTickCount() -- отсчет начала приоритетных расчётов
    do
    { T=GetTickCount();                  //= отметка реального времени расчётов
      if( !extFree() )mWait=0;           //! исполнение или полный выход =false
      RealTime+=(Rt=GetTickCount())-T;   //  использованный интервал времени #0
      if( mWait && Rt-St>=mWork )        //- перезапуск по истечению указанного
      { ::SetTimer( 0,tId,mWait,TimerProc );   // рабочего кванта времени и
        break;                                   // тогда к повтору безвременья
      }
    } while( mWait );
  } else mWait=0;
}

DWORD WaitTime( DWORD Wait,        // активная задержка для внешнего управления
                bool( *inStay )(), // собственно сам вычислительный эксперимент
                DWORD Work )       // время исполнения рабочего процесса [мСек]
{ extFree=inStay,mWork=Work,mWait=Wait;               // инициализация таймеров
  if( Wait )::SetTimer( 0,tId,Wait,TimerProc );       // כל = tId всеми окнами
  while( mWait )if( !WinRequest() )WaitMessage();     // ожидание чистки mWait
  return RealTime;                                    // выход в особом случае
}
                    //while( isTimer>1 )if( !WinRequest( hWnd ) )WaitMessage();
#endif

Window& Window::SetTimer( DWORD mS,bool(*inTm)() )    // время+адрес исполнения
{ if( this )
  { WaitEvents( hWnd );                             // исполнение проходящего
    if( !mS )KillTimer(); else                        // включается таймер №12+
    { ::SetTimer( hWnd,idEvent,mSec=mS,TimerProc );   // внутренняя отработка
      extTime=inTm; // выбор адреса для прицепа чужого кода - с OpenGL контекстом
  } } return *this;
}
Window& Window::KillTimer()
{ if( this )if( mSec )          // полная остановка без ожидания ранее начатого
  { mSec=0; WaitEvents(); //while( isTimer>0 )if( !WinRequest() )WaitMessage();
    extTime=NULL; ::KillTimer( hWnd,idEvent );        // теряется внешняя связь
  } return *this;
}
Window& Window::Above()
{ if( Site ) // if( glAct( this ) )
    { SetForegroundWindow( hWnd ); return Refresh(); } return *this;
}
Window& Window::Title( const char* A )
{ if( Caption )if( Site ){ char S[strlen(Caption)+strlen(A)+8];
    SetWindowTextW( hWnd,U2W( strcat(strcat(strcpy(S,Caption),"  ↔  " ),A) ) );
  } return *this;
}
Window& Window::Icon( const char* A )
{ if( Caption ){ HICON hIcon=LoadIcon(GetModuleHandle(NULL),A); // ~~ hInstance
    SendMessage( hWnd,WM_SETICON,ICON_BIG,(LPARAM)hIcon );      //   ICON_SMALL
  } return *this;
}
/*
Window& Above(){ SetForegroundWindow( hWnd ); SetFocus( hWnd );
  SetActiveWindow( hWnd ); ShowWindow( hWnd,SW_SHOWNA ); return Refresh(); }
  while( isTimer>0 )if( !WinRequest() )WaitMessage(); glFinish(); WinExecute();
  while( isTimer )if( !WinRequest() )WaitMessage(); // до перезапуска таймера

static void CALLBACK TimerProc(HWND hWind,UINT uMsg,UINT_PTR timerId,DWORD St);
  if( mSec )::SetTimer( hWnd,idEvent,mSec,TimerProc ); //... заведомо старт ...

#define retkey return wctob( KeyBuffer[KeyPos].Key );
#define retkey { fixed r=KeyBuffer[KeyPos].Key; return r<128?r:wctob( r ); }

#if 0
Window& Window::KillTimer()           //! сверить ещё раз...
{ if( mSec ){ mSec=0; extTime=NULL;   // полная остановка без ожидания начатого
    if( Site )if( hWnd ){::KillTimer( hWnd,idEvent ); // теряется внешняя связь
                          WaitEvents( hWnd );
  } } return *this;
}
#else
Window& Window::KillTimer()
{ if( mSec )                    // полная остановка без ожидания ранее начатого
  { mSec=0; WaitEvents(); //while( isTimer>0 )if( !WinRequest() )WaitMessage();
    extTime=NULL; ::KillTimer( hWnd,idEvent );        // теряется внешняя связь
  } return *this;
}
#endif
*/
