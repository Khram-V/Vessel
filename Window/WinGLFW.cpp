//
//!  Контекстно-связанный интерфейс с экранными окнами для графики OpenGL
//   формальные построения и динамические переустановки текстовых страниц
//     и графических фрагментов внутри главного окна Window
//
//  Подборка настроек для пересохранения графической среды и параметров Windows
//      - переключение и сохранение текущего состояния контекстной среды OpenGL
//               с отработкой деструктора для восстановления исходного контента
static Window* getWindow( GLFWwindow* window )   // , bool status=false
{//if( status )glfwMakeContextCurrent( window ); // выбор в списке окон Windows
  return (Window*)glfwGetWindowUserPointer(window);  // к возникшему прерыванию
}
//  Подборка статических процедур обработки прерываний
//
static void glfw_window_pos_callback( GLFWwindow* window, int xpos,int ypos )
{ Window *Win=getWindow( window ); Win->WindowX=xpos;
                                   Win->WindowY=ypos;
}
static void glfw_window_size_callback( GLFWwindow* window,int width,int height )
{ Window *Win=getWindow( window );
   Win->Height=height;
   Win->Width=width;   // glContext Set( Win );
   Win->Rest().Show();
}
/// мышка
static void glfw_cursor_enter_callback( GLFWwindow* window, int entered )
{ getWindow( window )->isCursorInside=entered;
}
static void glfw_mouse_button_callback
( GLFWwindow* window, int button, int action, int mods )
{ int scan=0; Real x,y;
  switch( button )
  { case GLFW_MOUSE_BUTTON_LEFT : scan=action==GLFW_PRESS?WM_LBUTTONDOWN:WM_LBUTTONUP; break;
    case GLFW_MOUSE_BUTTON_RIGHT: scan=action==GLFW_PRESS?WM_RBUTTONDOWN:WM_RBUTTONUP; break;
    case GLFW_MOUSE_BUTTON_3    : scan=action==GLFW_PRESS?WM_MBUTTONDOWN:WM_MBUTTONUP; break;
   default: return;
  }
 Window *Win=getWindow( window ); // glContext Set( Win );
      glfwGetCursorPos( window,&x,&y );
      Win->setMods( mods );
  if( Win->isCursorInside )Win->PutMouse( scan,x,y );
}
static void glfw_cursor_position_callback( GLFWwindow* window, Real x,Real y )
{ Window *Win=getWindow( window ); // glContext Set( Win );
  if( Win->isCursorInside )Win->PutMouse( WM_MOUSEMOVE,x,y );
}
static void glfw_scroll_callback( GLFWwindow* window, Real xoff,Real yoff )
{ Window *Win=getWindow( window ); // glContext Set( Win );
  if( Win->isCursorInside )Win->PutMouse( WM_MOUSEWHEEL,xoff,yoff );
}
static void callbackError( int error, const char* description )
  { Break( "~Ошибка GLFW( %d ):\n %s ",error,description );
  }
/*
static void glfw_draw_callback( GLFWwindow* window )
  { getWindow( window )->Activate().Save().Show();
  }
static void glfw_focus_callback( GLFWwindow* window, int focus )
  { glfwFocusWindow( window ); getWindow( window )->Show();
  }
*/
static bool DoNotClose=false;
/// Клавиатура
static void glfw_charmods_callback
( GLFWwindow* window, unsigned int codepoint, int mods ){ //if( codepoint ){
     Window *Win=getWindow( window ); //glContext Set( Win );
             Win->setMods( mods );             // static_cast<byte>( codepoint )
             Win->PutChar( (fixed)codepoint ); //  !! забавно
}
static void glfw_input_key_callback
( GLFWwindow* window, int _key, int scancode, int action, int mods )
{ Window *Win=getWindow( window ); // glContext Set( Win );
          Win->setMods( mods );
  if( action!=GLFW_RELEASE )                // !=0
//if( action==GLFW_PRESS )                  //  =1
  { fixed Key=0;                            // чистый ключ на входе в процедуру
    switch( _key )
    { case GLFW_KEY_UP       : Key=_North;      break;
      case GLFW_KEY_LEFT     : Key=_West;       break;
      case GLFW_KEY_DOWN     : Key=_South;      break;
      case GLFW_KEY_RIGHT    : Key=_East;       break;
      case GLFW_KEY_HOME     : Key=_North_West; break;
      case GLFW_KEY_PAGE_DOWN: Key=_South_East; break;
      case GLFW_KEY_END      : Key=_South_West; break;
      case GLFW_KEY_PAGE_UP  : Key=_North_East; break;
      case GLFW_KEY_ESCAPE   : Key=_Esc;        break;
      case GLFW_KEY_INSERT   : Key=_Ins;        break;
      case GLFW_KEY_TAB      : Key=_Tab;        break;
      case GLFW_KEY_DELETE   : Key=_Del;        break;
      case GLFW_KEY_BACKSPACE: Key=_BkSp;       break;
      case GLFW_KEY_ENTER    : Key=_Enter;      break;
     default:
      if( action!=GLFW_REPEAT )                // повторение F-команд исключено
      { if( _key>=GLFW_KEY_F1 && _key<=GLFW_KEY_F12 )Key=_F1+_key-GLFW_KEY_F1;
        DoNotClose=true;                                 // else Key=_key&0xFF;
      }
    } if( Key )Win->PutChar( Key );
} }
static void glfw_window_close_callback( GLFWwindow* window )
{ if( DoNotClose )DoNotClose=false; else      // дабы избежать закрытия по <F4>
  { getWindow( window )->Close();
//  if( First )//if( window )
//  { Window *W; if( (W=getWindow( window ))!=NULL )W->Close();
//               for( W=First; W->Next; W=W->Next )W->KillTimer();
//  }
  }
}
//!  Window Procedure - общая для всех процедура запросов состояния GLFW
//!
Window* Place::Ready()
{ if( First )if( Site ){ Site->WaitEvents(); return Site; } return NULL;
  // else WinReady();
}
bool WinReady( Window *W )    // без указания адреса опрашиваются корень списка
{ if( W )return W->Ready()!=NULL; else
  if( First )for( W=First; W->Next; W=W->Next )W->WaitEvents();
  return First!=NULL;
}
//!  Конструктор создает окно OpenGL, и ... не образует цикла прерываний ...
//      площадка Place в основании окна ортогонализуется и пересохраняется
//
Window::Window( const char *_title, int x,int y,int w,int h )
      : Place( this,PlaceAbove|PlaceOrtho ),         // ортогонализуется [-1:1]
        Next( NULL ),glfwWindow( NULL ),Caption( _title ), // и сохраняется
        ScreenWidth( GetSystemMetrics( SM_CXSCREEN ) ),
        ScreenHeight( GetSystemMetrics( SM_CYSCREEN ) ),
        WindowX( 0 ),WindowY( 0 ),KeyPos( 0 ),KeyPas( 0 ),
        isCursorInside( true ),     // Flag if cursor inside this window or not
        dTime( 0 ),nextTime( 0 ),   // шаг и ожидаемое прерывание таймера [µсек]
        isTimer( 0 ),isMouse( 0 ),onKey( false ),mods( 0 ),
        extKey( NULL ),extTime( NULL )
{
 // Изначальная инициализация glfw библиотеки
 //       ++ включение нового объекта в конец простого списка - перечисления

    Locate( x,y,w,h );  // -- без glfwWindow - просто исходные размерности окна
    if( !First )
    { glfwSetErrorCallback( callbackError );
      glfwInit(); First=this; Next=NULL;
    }
    else{ Window *W=First; while( W->Next )W=W->Next; W->Next=this; Next=NULL; }  // ++ ячейка
 // glfwInitHint( GLFW_PLATFORM,GLFW_PLATFORM_WIN32 );
    if( !_title )
    glfwWindowHint( GLFW_DECORATED,false );  // случай без оконтуривающих рамок
    glfwWindowHint( GLFW_VISIBLE,true );  // изначально окно становится видимым
    glfwWindowHint( GLFW_RESIZABLE,true );// разрешение изменения размеров окна
    glfwWindowHint( GLFW_DOUBLEBUFFER,true );  // двойной буфер памяти для окна
 // glfwWindowHint( GLFW_FLOATING,true ); // активное окно всегда поверх других
 // glfwWindowHint( GLFW_DEPTH_BITS,true );// расслоение по глубине - дальности
 //!glfwWindowHint( GLFW_TRANSPARENT_FRAMEBUFFER,GLFW_TRUE );
 // glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR,4 );
 // glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR,3 );
 //?glfwWindowHint( GLFW_HOVERED,true ); // контроль присутствия курсора в окне
 //?glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT,GLFW_TRUE );

    glfwWindow=glfwCreateWindow                   // Создание независимого окна
      ( Width,Height,_title?_title:"Window-Place",   // glfwGetPrimaryMonitor()
                                      NULL,NULL );
    if( !glfwWindow ){ glfwTerminate(); Break( "GLFW-error" ); }
    glfwMakeContextCurrent( glfwWindow );       // добавление callback процедур
    glfwSetWindowUserPointer( glfwWindow,this );
    dTime=0; nextTime=glfwGetTime()*1e3;      // начальная установка таймера
//
//  Установка выполнена, теперь прописка размерностей, шрифтов и вложенных окон
//
    Up=NULL;                        // верхний фрагмент в списке наложений
    Site=this;                      // связанный Place ссылается на Window
    Locate( x,y,Width,Height );     // установка размерений нового окна
    AlfaVector();                   // исходный шрифт участвует в размерностях
    Activate().Clear();             // включение всего окна в качестве площадки
    chY=Height-AlfaHeight();        // позиция текстового курсора сверху/слева
//
//  Блок прерываний от внешних устройств и внутренних процессов
//
    glfwSetKeyCallback            ( glfwWindow,glfw_input_key_callback );//Ctrl
    glfwSetCharModsCallback       ( glfwWindow,glfw_charmods_callback ); //Char
    glfwSetWindowPosCallback      ( glfwWindow,glfw_window_pos_callback );
    glfwSetFramebufferSizeCallback( glfwWindow,glfw_window_size_callback );
    glfwSetCursorPosCallback      ( glfwWindow,glfw_cursor_position_callback );
    glfwSetCursorEnterCallback    ( glfwWindow,glfw_cursor_enter_callback );
    glfwSetMouseButtonCallback    ( glfwWindow,glfw_mouse_button_callback );
    glfwSetScrollCallback         ( glfwWindow,glfw_scroll_callback );
    glfwSetWindowCloseCallback    ( glfwWindow,glfw_window_close_callback );
//  glfwSetWindowFocusCallback    ( glfwWindow,glfw_focus_callback );
//  glfwSetWindowRefreshCallback  ( glfwWindow,glfw_draw_callback );
//            can add processing of quit message( previously was turned off )
    Icon( "Icon" );
/*
       GLFWmonitor *monitor=glfwGetPrimaryMonitor();     // для целого экрана
 const GLFWvidmode *mode  = glfwGetVideoMode( monitor );
       ScreenWidth=mode->width;
       ScreenHeight=mode->height;
 //или glfwGetWindowSize( glfwWindow,&ScreenWidth,&ScreenHeight );
       Break( "~ Locate: %d·%d => %d x %d",pX,pY,ScreenWidth,ScreenHeight ); */
}
Window::~Window()                     // Разрушение окна в обработке прерываний
{ if( glfwWindow )                               // не без предосторожностей
//if( Site )
  { KillTimer();                                 // отключение таймера
    while( Up )Up->~Place();                     // сброс наложенных фрагментов
    //       Site->~Place();                     // обрушение графического поля
    //  PutChar( 0 );                            // отмена запросов клавиатуры
    //  ScanStatus();                            // очистка запросов Windows
    //  while( GetKey() );
    //  WaitEvents();
    Site = NULL;                                 //! сброс повторов деструктора
   Window *Cur=First;                            // обработка/очистка списка
    if( Cur==this )First=Cur=Next; else          // первое Window-вхождение
    while( Cur->Next )                           // и надо особо уважить поиски
     { if( Cur->Next==this ){ Cur->Next=Next; break; } Cur=Cur->Next; }

//     if( Cur->Next!=this )Cur=Cur->Next; else  // себя самого с удалением
//       { Cur->Next=Next; break; }    // самого первого из найденных
//  if( glfwWindow ){
        glfwDestroyWindow( glfwWindow );         // закрытие окна с переходом
        glfwWindow = NULL;
//    }
    if( Cur )glAct( Cur ); //else First=NULL;    // - на смежный нижний уровень
    else{ First=NULL; glfwTerminate(); _exit( 22 );} // WinReady(),exit(3); // Cur->Activate();
  }
}
void Window::Close(){ if( glfwWindow )this->~Window(); } //delete this; }; //~Window(); }
//!
//!   Позиционирование окон по правилам Windows
//!
Window& Window::Locate( int X,int Y, int W,int H )   // по правилам Windows
{ W = ( Width =min( W>0?W:800,ScreenWidth ) );       // позиционирование идет
  H = ( Height=min( H>0?H:600,ScreenHeight ) );      // от левого верхнего угла
  X = X>0 ? X:( X<0 ? X + ScreenWidth -W : 2*( ScreenWidth-W )/3 );
  Y = Y>0 ? Y:( Y<0 ? Y + ScreenHeight-H : ( ScreenHeight-H )/4 );
  WindowX = minmax( 0,X,ScreenWidth-W );  pX=0;
  WindowY = minmax( 0,Y,ScreenHeight-H ); pY=0;
  if( glAct( this ) )                                             // glfwWindow
  { glfwSetWindowPos( glfwWindow,WindowX,WindowY );
    glfwSetWindowSize( glfwWindow,W,H );
    //Refresh();
    //WaitEvents();
  } return *this; //Refresh();
}
//!  Блок накопления данных клавиатуры в текущем кольцевом буфере класса Window
//!      (надо бы его спрятать, конечно, но в С++ ? - странно)
#if 0
byte Window::ModifyKeyWithState( byte code )
{ //glfwMakeContextCurrent( glfwWindow );
  if(glfwGetKey(glfwWindow,GLFW_KEY_LEFT_SHIFT   )==GLFW_PRESS && (mods&GLFW_MOD_SHIFT ) )code|=LEFT;
  if(glfwGetKey(glfwWindow,GLFW_KEY_RIGHT_SHIFT  )==GLFW_PRESS && (mods&GLFW_MOD_SHIFT ) )code|=RIGHT;
  if(glfwGetKey(glfwWindow,GLFW_KEY_LEFT_CONTROL )==GLFW_PRESS && (mods&GLFW_MOD_CONTROL))code|=LCTRL;
  if(glfwGetKey(glfwWindow,GLFW_KEY_RIGHT_CONTROL)==GLFW_PRESS && (mods&GLFW_MOD_CONTROL))code|=RCTRL;
  if(glfwGetKey(glfwWindow,GLFW_KEY_RIGHT_ALT    )==GLFW_PRESS && (mods&GLFW_MOD_ALT   ) )code|=R_ALT;
  if(glfwGetKey(glfwWindow,GLFW_KEY_LEFT_ALT     )==GLFW_PRESS && (mods&GLFW_MOD_ALT   ) )code|=L_ALT;
  return code;
}
#else
fixed Window::KeyStates( fixed code )
{ //  glfwMakeContextCurrent( glfwWindow );
  ///     простой опрос состояния клавиатуры с установкой командных аккордов
  if( Site )
  { if( glfwGetKey( glfwWindow,GLFW_KEY_LEFT_SHIFT   )==GLFW_PRESS )code|=LEFT;
    if( glfwGetKey( glfwWindow,GLFW_KEY_RIGHT_SHIFT  )==GLFW_PRESS )code|=RIGHT;
    if( glfwGetKey( glfwWindow,GLFW_KEY_LEFT_CONTROL )==GLFW_PRESS )code|=LCTRL;
    if( glfwGetKey( glfwWindow,GLFW_KEY_RIGHT_CONTROL)==GLFW_PRESS )code|=RCTRL;
    if( glfwGetKey( glfwWindow,GLFW_KEY_LEFT_ALT     )==GLFW_PRESS )code|=L_ALT;
    if( glfwGetKey( glfwWindow,GLFW_KEY_RIGHT_ALT    )==GLFW_PRESS )code|=R_ALT;
  } return code;
}
#endif
//
//!  обращение к клавиатуре через активное и контекстно настроенное окно Window
//                  ! осторожно, здесь предполагается отсутствие вызовов OpenGL
fixed Window::WaitKey()                         // цикл ожидания нового символа
{ onKey=true;
  while( Site && KeyPos==KeyPas )WaitEvents( true ); // обновление перед
  onKey=false; return KeyBuffer[++KeyPos&=0x3F].Key;// остановкой в том же окне
}                                                   //glfwPollEvents();
//!
//!  Внутренние процедуры для реализации виртуальных обращений с мышкой
//!
void Window::PutMouse( UINT State, int x,int y )
{ switch( State )                     // ?( и как теперь с виртуальностью )
  { case WM_MOUSEMOVE    : break;     // ?  контекст OpenGL не сверяется
    case WM_LBUTTONDBLCLK:            //   - мышка ставит его сама
    case WM_LBUTTONDOWN  : MouseState |=  _MouseLeft;   break;
    case WM_LBUTTONUP    : MouseState &= ~_MouseLeft;   break;
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN  : MouseState |=  _MouseRight;  break;
    case WM_RBUTTONUP    : MouseState &= ~_MouseRight;  break;
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN  : MouseState |=  _MouseMiddle; break;
    case WM_MBUTTONUP    : MouseState &= ~_MouseMiddle; break;
    case WM_MOUSEWHEEL   : MouseState =   _MouseWheel;  break; // сдвиг
    default: MouseState=0; return; // isMouse=0; return;
  }
  if( isMouse )return; isMouse++;         // предотвращение рекурсии прерываний
  if( MouseState==_MouseWheel )           // прокрутка проходит сама по себе
  { if( y||x ){ glContext S( this ); Mouse( _MouseWheel,x,y*32 ); } MouseState=0;
  } else
  { Place *P=this; int px=x,py=y;         //!  поиск последней/верхней площадки
    for( Place *S=P; S; S=S->Up )         //!     по общему списку их наложений
    { int _x=x-S->pX,_y=y+S->pY-Height+S->Height; // выход должен быть успешным
      if( unsigned( _y )<=unsigned( S->Height )
       && unsigned( _x )<=unsigned( S->Width ) ){ P=S; py=_y,px=_x; }
    }
   static volatile int xo=0,yo=-1;        //! рекурсия мышиных прерываний c
    if( !MouseState )                     //! координатами предыдущих вхождений
    { glContext S( this ); yo=-1; P->Mouse( px,py );
    } else                                          //! yo=-1 окно не захвачено
    { if( KeyStates()==L_ALT && MouseState==_MouseLeft )
      { if( yo<0 )xo=x,yo=y; else     // перемещение окна left<Alt> и left<Btn>
        if( x!=xo || y!=yo )Locate( WindowX+x-xo,WindowY+y-yo,Width,Height );
      } else
      { glContext S( this ); yo=-1; P->Mouse( MouseState,px,py );
      }
    }
  } isMouse=0; // MouseState &= ~_MouseWheel;
}
//!  Прямое и параллельное обращение к таймеру с соблюдением очередей Windows
//!       (все расчеты в миллисекундах, опрокидывание через 49,7 суток)
//!
#if 1
DWORD volatile RealTime=0,                              // тики [мс] от времени
      StartTime=GetTickCount();                         //     запуска Windows
DWORD ElapsedTime(){ return GetTickCount()-StartTime; } //  от старта программы
DWORD GetTime()
    { DWORD T=GetTickCount(); if( StartTime>T )StartTime=T; return T; }
#else
DWORD volatile                                          // тики [мс] от времени
      StartTime=glfwGetTime()*1e3;                         //     запуска Windows
DWORD ElapsedTime(){ return glfwGetTime()*1e3-StartTime; } //  от старта программы
DWORD GetTime()
    { DWORD T=glfwGetTime()*1e3; if( StartTime>T )StartTime=T; return T; }
#endif
bool Window::Timer()
{ if( glfwWindow )
  { //WaitEvents();
    if( extTime )        // настройка OpenGL с контекстным эпилогом перерисовки
//  if( !isTimer )       // подготовка среды, вызов процедуры внешнего исполнения
    { bool Res; /*isTimer=true;*/ Res=extTime(); /*isTimer=false;*/ return Res;
    }
  } return false;
}
Window& Window::SetTimer( DWORD mSec,bool(*inTm)() )// время и адрес исполнения
{ if( glfwWindow )
  if( !mSec )                                      // пока только таймер №12
  { dTime=0; extTime=NULL; } else                  // идентификатор не привязан
  { dTime=mSec;                              // glfwWaitEventsTimeout( dTime );
    nextTime=glfwGetTime()*1e3+dTime; extTime=inTm;
  } return *this;
}
Window& Window::KillTimer()                           // сброс таймера - если 0
{ if( glfwWindow )SetTimer( 0 ); return *this;
}
void Window::PutTimer()             //? внутренняя обработка допускает пропуски
{ if( glfwWindow )
  if( !isTimer )                    //! потрясающе ! вдруг обнаружились повторы
  //if( !KeyInterrupt )             // внешних прерываний без исполнения первых
  //if( !isMouse )                  // ! еще и с клавиатурой - проблемы
  if( Ready() )
  { ++isTimer;                        // на повторах - сбрасываются расчеты
    { glContext( this );
//    if( Timer() )Save().Refresh();  // с восстановлением наложенных страничек
//    if( Timer() )Show();            // с перерисовкой картинки
      if( Timer() )Refresh();         // с перерисовкой картинки
//    Timer();
//    Refresh(); WaitEvents();
    } isTimer=0;                      // разрешение повторных вхождений таймера
  }// WaitEvents()                    // после завершения всех операций OpenGL
}

/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void Window::WaitEvents( bool stop )
{ if( First )if( glfwWindow )//if( glAct( this ) )
  { for( Window *aW=First; aW; aW=aW->Next ) // перебор активированных окон
    if( aW->dTime )                          // если таймер установлен и
    if( glfwGetTime()*1e3>=aW->nextTime )    // текущее время больше заданного
      { aW->nextTime=glfwGetTime()*1e3+aW->dTime; aW->PutTimer(); } // новый шаг-такт
    if( stop )
    { Show();
      if( dTime )glfwWaitEventsTimeout( Real( dTime )/1e3 );
      else glfwWaitEvents();
    } else glfwPollEvents();
  }
}
//#include <StdArg.h>                           // блок разных текстовых надписей
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw3native.h"

Window& Window::Title( const char* A )
{ if( glfwWindow )if( Caption ){ char S[strlen(Caption)+strlen(A)+8];
    glfwSetWindowTitle                        // GLFWwindow*, const char* title
    ( glfwWindow,strcat( strcat(strcpy(S,Caption),"  ↔  " ),A ) );
//  SetWindowTextW( hWnd,U2W( strcat(strcat(strcpy(S,Caption),"  ↔  " ),A) ) );
  } return *this;
}

// Функция для конвертации HICON в формат GLFW (согласовано с Алисой в Яндексе)

Window& Window::Icon( const char* A ) // по resource файлу здесь есть вопросы...
{ if( Caption )
  { HICON hIcon=LoadIcon( GetModuleHandle(NULL),A );      // ~~ hInstance
    SendMessage( glfwGetWin32Window( glfwWindow ),        // GetActiveWindow(),
                 WM_SETICON,ICON_BIG,(LPARAM)hIcon );     //? ICON_SMALL
/*
    glfwSetWindowIcon// ( GLFWwindow* window, int count, const GLFWimage* images);
    ( glfwWindow,1,(GLFWimage*)hIcon+?? );
      SendMessage( glfwGetWin32Window( glfwWindow ),      // GetActiveWindow(),
                   WM_SETICON,ICON_BIG,(LPARAM)hIcon );         //   ICON_SMALL
*/  }
  return *this;
}
Window& Window::Above()
{ if( glAct( this ) )glfwFocusWindow( glfwWindow ); return *this;
}
//     - переключение и временное сохранение состояния контекстной среды OpenGL
//               с отработкой деструктора для восстановления исходного контента
//
bool glAct( const Window *W )
{ if( W )if( W->glfwWindow )
    { glfwMakeContextCurrent( W->glfwWindow ); return true; } return false;
}
//             конструктор = пролог с восстановлением через эпилог = деструктор
glContext::glContext( const Window *W ) :                           // prologue
  was( glfwGetCurrentContext() )
  { if( W->glfwWindow==was )was=0,Active=true; // W->glfwWindow!=0;
    else Active=glAct( W );
//  else glfwMakeContextCurrent( W->glfwWindow );
  }
glContext::~glContext()     // деструктор = эпилог с возвратом былого контекста
  { if( was ){ glfwMakeContextCurrent( was );                       // epilogue
//             getWindow( was )->WaitEvents();
             }
  }                                /// доработать !!!
DWORD WaitTime( DWORD mWait,       // активная задержка для внешнего управления
                bool(*inStay)(),   // собственно сам вычислительный эксперимент
                DWORD mWork )      // время на исполнение рабочего цикла [мСек]
{
 //Sleep( mWait );
   First->ScanKey();
   glfwWaitEventsTimeout( 0.03*Real( mWait )/1e3 ); //! [ 0.03 ]
   if( inStay )(*inStay)();
   return ElapsedTime();
}
//              { Sleep( mWait ); }

//  ...  все согласованные процедуры объединяются в единый модуль интерактивной
//  графической среды Window::Place в/исключая независимые операции с Юлианским
//        календарем и перекодировками Unicode/UTF-8 для Windows-1251 и OEM-866
//

