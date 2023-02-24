//
//!  Контекстно-связанный интерфейс с экранными окнами для графики OpenGL
//   формальные построения и динамические переустановки текстовых страниц
//     и графических фрагментов внутри главного окна Window
//
#include "Window.h"

static Window *First=NULL; // первое окно в последовательном статическом списке
static const GLsizei ListsGroup=256; // ?длина группы TrueType шрифтов Windows
//
//  Подборка настроек для пересохранения графической среды и параметров Windows
//      - переключение и сохранение текущего состояния контекстной среды OpenGL
//               с отработкой деструктора для восстановления исходного контента
inline Window* getWindow( GLFWwindow* window ) //, bool status=false )
{ //if(status)glfwMakeContextCurrent( window );// выбор в списке окон Windows к
  return (Window*)glfwGetWindowUserPointer( window ); // возникшему прерыванию
}
struct glEnv{ GLFWwindow *was;
 explicit glEnv( Window &Win ): was( glfwGetCurrentContext() ) //,W( &Win )
  { if( Win.glfwWindow==was )was=NULL;
    else glfwMakeContextCurrent( Win.glfwWindow ); }
 ~glEnv()
  { if( was ){ glfwMakeContextCurrent( was ); getWindow( was )->ScanKey(); } }
};
bool WinReady( Window *W )         // без указания адреса опрашиваются все окна
{ if( W )return W->Ready()!=NULL; return First!=NULL;
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
   Win->Width=width; glEnv Set( *Win ); Win->Rest().Show();
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
 Window *Win=getWindow( window ); glEnv Set( *Win );
      glfwGetCursorPos( window,&x,&y );
      Win->setMods( mods );
  if( Win->isCursorInside )Win->PutMouse( scan,x,y );
}
static void glfw_cursor_position_callback( GLFWwindow* window, Real x,Real y )
{ Window *Win=getWindow( window ); glEnv Set( *Win );
  if( Win->isCursorInside )Win->PutMouse( WM_MOUSEMOVE,x,y );
}
static void glfw_scroll_callback( GLFWwindow* window, Real xoff,Real yoff )
{ Window *Win=getWindow( window ); glEnv Set( *Win );
  if( Win->isCursorInside )Win->PutMouse( WM_MOUSEWHEEL,xoff,yoff );
}
/// Клавиатура
static void glfw_charmods_callback
( GLFWwindow* window, unsigned int codepoint, int mods )
{ Window *Win=getWindow( window ); glEnv Set( *Win );
          Win->setMods( mods );               // static_cast<byte>( codepoint )
          Win->PutChar( (byte)codepoint );    // !! забавно
}
static void glfw_input_key_callback
( GLFWwindow* window, int key, int scancode, int action, int mods )
{ Window *Win=getWindow( window ); glEnv Set( *Win );
          Win->setMods( mods );
  if( action!=GLFW_RELEASE )
  { byte Key=0;                             // чистый ключ на входе в процедуру
    switch( key )
    { case GLFW_KEY_UP        : Key=_North;      break;
      case GLFW_KEY_LEFT      : Key=_West;       break;
      case GLFW_KEY_DOWN      : Key=_South;      break;
      case GLFW_KEY_RIGHT     : Key=_East;       break;
      case GLFW_KEY_HOME      : Key=_North_West; break;
      case GLFW_KEY_PAGE_DOWN : Key=_South_East; break;
      case GLFW_KEY_END       : Key=_South_West; break;
      case GLFW_KEY_PAGE_UP   : Key=_North_East; break;
      case GLFW_KEY_INSERT    : Key=_Ins;        break;
      case GLFW_KEY_DELETE    : Key=_Del;        break;
      case GLFW_KEY_BACKSPACE : Key=_BkSp;       break;
      case GLFW_KEY_TAB       : Key=_Tab;        break;
      case GLFW_KEY_ESCAPE    : Key=_Esc;        break;
     default:
      if( action != GLFW_REPEAT ) // повторение F-команд исключено
      { if( key>=GLFW_KEY_F1 && key<=GLFW_KEY_F12 )Key=_F1+key-GLFW_KEY_F1;
        //                                  else   Key=wParam&0xFF;
      }
    } if( Key )Win->PutChar( Key );
} }
#if 0
static void glfw_draw_callback( GLFWwindow* window )
{ getWindow( window )->Activate().Save().Show(); }
static void glfw_focus_callback( GLFWwindow* window, int focus )
{ getWindow( window )->Show(); }
static void glfw_window_close_callback( GLFWwindow* window )
   { getWindow( window )->~Window(); }
#endif
static void callbackError( int error, const char* description )
     { Break( "glfw ошибка( %d ):\n %s ",error,description );
     }
//!  Конструкторы и процедуры сохранения и перерисовки фонового изображения
//!    - все странички всегда связаны с одним из графических окон Window,
//!    - однако отсутствие ссылки Prev - показывает,
//!      что окно исключено из всех операций со списками окон
//     ( это площадка базового окна или фантомное отображение вне Window )
//
Place::Place( Window *Win, byte Mode ): Site( Win ),Up( NULL ),Signs( Mode ),
      pX( 0 ),pY( 0 ),Width( Win->Width ),Height( Win->Height ),
      hFont( 0 ),BitFont( NULL ),                // DispСССР: _8x16,_8x14,_8x08
      FontWidth( 0 ),FontHeight( 0 ),chX( 0 ),chY( 0 ),      // буквенное место
      MouseState( 0 ),       // беспросветная внутренняя унылость мышки/курсора
      extDraw( NULL ),       // свободное рисование без дополнительных настроек
      extPass( NULL ),       // адреса двух внешних\свободных и бесконтрольных
      extPush( NULL ),       // процедур для параллельного контроля хода мышки
          Img( NULL )        // ~~~~~~~ //
{ if( this!=(Place*)Win )               // обходится неприкасаемое базовое окно
  { Place *S=(Place*)Win;
    while( S->Up )S=S->Up; S->Up=this;  // новая площадка набрасывается сверху
    glfwMakeContextCurrent( Win->glfwWindow );     // временная привязка к окну
    Alfabet().Activate();               // Courier New \ Bit( _8x16 )
    chY=Height-FontHeight;   // буквенная позиция изначально будет сверху/слева
} }
Place::~Place()  // освобождение площадки, шрифтов, картинки и всех точек входа
{ if( Site ){ glEnv Gl( *Site );
    for( Place *S=(Place*)Site; S; S=S->Up )if( S->Up==this )S->Up=Up;
    if( Img )free( Img ); Img=NULL;  extDraw=NULL; extPass=NULL; extPush=NULL;
    if( FontBase )glDeleteLists( FontBase,ListsGroup ); FontBase=0;
    Site->Refresh();
} }
//!   блок контекстно-зависимых (дружественных) процедур управления фрагментами
//                  возможно нужна единая ссылка к чистому выходу из прерываний
//
Place& Place::Activate(bool act) // активизация графического контекста площадки
{ glfwMakeContextCurrent( Site->glfwWindow );     // предустановленной страницы
  glViewport( pX,pY,Width,Height );               // установка растрового поля
//glScissor( pX,pY,Width,Height );                // обрезка внешнего окружения
  if( act )                                       // обход масштабирования
  { glMatrixMode(GL_PROJECTION),glLoadIdentity(); // на размер фрагмента экрана
    if( Signs&PlaceOrtho )                        // ортогональная отметка под
    { Real rat=Real( Width )/Height;              // исходное ортонормированное
      if( rat>1 )glOrtho(-rat,rat,-1,1,    -1,1); // масштабирование экрана во
          else   glOrtho(-1,1,-1/rat,1/rat,-1,1); // внутренний 3D-размер[-1:1]
    }     else   glOrtho(0,Width,0,Height, -1,1); // +отсчет растра снизу\слева
    glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // иначе просто по экрану
  } return *this;                                 // единичная матрица модели
}
//!    Window Procedure - общая для всех процедура запросов состояния GLFW
//!
Window* Place::Ready(){ if(Site){Site->WaitEvents(); } return Site; }
//
//!   Конструктор создает окно OpenGL, и ... не образует цикла прерываний ...
//      площадка Place в основании окна ортогонализуется и пересохраняется
//
Window::Window( const char *_title,int x,int y,int w,int h )
      : Place( this,PlaceAbove|PlaceOrtho ),         // ортогонализуется [-1:1]
        Title( _title ), Next( NULL ),               //    и сохраняется
        isCursorInside( true ),     // Flag if cursor inside this window or not
        glfwWindow( NULL ),
        ScreenWidth( GetSystemMetrics( SM_CXSCREEN ) ),
        ScreenHeight( GetSystemMetrics( SM_CYSCREEN ) ),
        WindowX( 0 ),WindowY( 0 ),KeyPos( 0 ),KeyPas( 0 ),
        isTimer( 0 ),isMouse( 0 ),KeyInterrupt( false ),
        extKey( NULL ),extTime( NULL ),mods( 0 )
{   //
    //  Изначальная инициализация glfw библиотеки ++ включение нового объекта
    //                                   в конец простого списка - перечисления
    Locate( x,y,w,h );  // -- без glfwWindow - просто исходные размерности окна
    if( !First )
    { glfwSetErrorCallback( callbackError ); glfwInit(); First=this;
    } else
    { Window *W=First; while( W->Next )W=W->Next; W->Next=this;
    }
    if( !_title )
    glfwWindowHint( GLFW_DECORATED,false ); // случай без оконтуривающих рамок
 // glfwWindowHint( GLFW_FLOATING,true ); // активное окно всегда поверх других
    glfwWindowHint( GLFW_VISIBLE,true );  // изначально окно становится видимым
    glfwWindowHint( GLFW_RESIZABLE,true );// разрешение изменения размеров окна
    glfwWindowHint( GLFW_DOUBLEBUFFER,true ); // двойной буфер памяти для окна
 // glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR,4 );
 // glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR,3 );
 // glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE );
    glfwWindow=glfwCreateWindow                 //   Создание независимого окна
             ( w,h,_title?_title:"Window-Place",NULL,NULL );
    if( !glfwWindow )glfwTerminate(),Break( "ошибка GLFW" );
    glfwMakeContextCurrent( glfwWindow );       // добавление callback процедур
    glfwSetWindowUserPointer( glfwWindow,this );
    dTime=0.0; nextTime=glfwGetTime();          // начальная установка таймера
    glfwSetWindowPosCallback      ( glfwWindow,glfw_window_pos_callback );
    glfwSetFramebufferSizeCallback( glfwWindow,glfw_window_size_callback );
    glfwSetKeyCallback            ( glfwWindow,glfw_input_key_callback );
    glfwSetCharModsCallback       ( glfwWindow,glfw_charmods_callback );
    glfwSetCursorPosCallback      ( glfwWindow,glfw_cursor_position_callback );
    glfwSetCursorEnterCallback    ( glfwWindow,glfw_cursor_enter_callback );
    glfwSetMouseButtonCallback    ( glfwWindow,glfw_mouse_button_callback );
    glfwSetScrollCallback         ( glfwWindow,glfw_scroll_callback );
//  glfwSetWindowFocusCallback    ( glfwWindow,glfw_focus_callback );
//  glfwSetWindowRefreshCallback  ( glfwWindow,glfw_draw_callback );
//         can add processing of quit message( previously was turned off )
//  glfwSetWindowCloseCallback( glfwWindow,glfw_window_close_callback );
//        GLFWmonitor *monitor=glfwGetPrimaryMonitor();
//  const GLFWvidmode *mode  = glfwGetVideoMode( monitor );
//
//  Установка выполнена, теперь прописка размерностей, шрифтов и вложенных окон
//
    Up=NULL;                        // верхний фрагмент в списке наложений
    Site=this;                      // связанный Place ссылается на Window
    Locate( x,y,w,h );              // установка размерений нового окна
    Alfabet();                      // исходный шрифт участвует в размерностях
    Activate().Clear();             // включение всего окна в качестве площадки
    chY=Height-FontHeight;          // позиция текстового курсора сверху/слева
}
Window::~Window()                   // Разрушение окна в обработке прерываний
{ if( Site )                                     // не без предосторожностей
  { Window *Cur=First;                           // обработка/очистка списка
    while( Up )Up->~Place();                     // сброс наложенных фрагментов
        KillTimer();                             // отключение таймера
    //  PutChar( 0 );                            // отмена запроса клавиатуры
    //  ScanStatus();                            // очистка запросов Windows
    //  Site->~Place();                          // обрушение графического поля
        Site=NULL;                               //! указание нуля деструктору
    if( Cur==this )Cur=First=Next; else          // первое Window-вхождение
    while( Cur->Next )                           // и надо особо уважить поиски
       if( Cur->Next!=this )Cur=Cur->Next; else  // себя самого с удалением
         { Cur->Next=Next; break; }              // самого первого из найденных
    if( glfwWindow )
      { glfwDestroyWindow( glfwWindow );         // закрытие окна с переходом
        glfwWindow = NULL;
      }
    if( Cur )Cur->Activate(); //else exit( 4 );  // - на смежный нижний уровень
} }
//   восстановление картинки для всех фрагментов с опцией PlaceAbove+(Img)
//
Window& Window::Refresh()// прорисовка изображения с копий в оперативной памяти
{ glEnv Set( *this );                                               // Clear();
  for( Place *S=(Place*)this; S; S=S->Up )S->Rest();                // Show();
 unsigned *I=Img; Img=NULL; Show(); Img=I;  // ...визуализация картинки в целом
//glfwSwapBuffers( glfwWindow ); glFlush(); // возможно так значительно быстрее
  return *this;
}
Place& Place::Refresh(){ Site->Refresh(); return *this;  }
//!
//!   Позиционирование окон по правилам Windows
//!
Window& Window::Locate( int X,int Y, int W,int H )     // по правилам Windows
{ W = ( Width =min( W>0?W:1024,ScreenWidth ) );       // позиционирование идет
  H = ( Height=min( H>0?H:768,ScreenHeight ) );      // от левого верхнего угла
  X = X>0 ? X:( X<0 ? X + ScreenWidth -W : 2*( ScreenWidth-W )/3 );
  Y = Y>0 ? Y:( Y<0 ? Y + ScreenHeight-H : ( ScreenHeight-H )/4 );
  WindowX = max( 0,min( X,ScreenWidth -W ) );
  WindowY = max( 0,min( Y,ScreenHeight-H ) );
  if( glfwWindow )
  { glfwSetWindowPos( glfwWindow,WindowX,WindowY );
    glfwSetWindowSize( glfwWindow,W,H );
    WaitEvents();
  } return Refresh();
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
byte Window::ModifyKeyWithState( byte code )
{ ///     простой опрос состояния клавиатуры с установкой командных аккордов
  if( glfwGetKey( glfwWindow,GLFW_KEY_LEFT_SHIFT   )==GLFW_PRESS )code|=LEFT;
  if( glfwGetKey( glfwWindow,GLFW_KEY_RIGHT_SHIFT  )==GLFW_PRESS )code|=RIGHT;
  if( glfwGetKey( glfwWindow,GLFW_KEY_LEFT_CONTROL )==GLFW_PRESS )code|=LCTRL;
  if( glfwGetKey( glfwWindow,GLFW_KEY_RIGHT_CONTROL)==GLFW_PRESS )code|=RCTRL;
  if( glfwGetKey( glfwWindow,GLFW_KEY_LEFT_ALT     )==GLFW_PRESS )code|=L_ALT;
  if( glfwGetKey( glfwWindow,GLFW_KEY_RIGHT_ALT    )==GLFW_PRESS )code|=R_ALT;
  return code;
}
#endif
void Window::PutChar( byte Key )                    // занесение одного символа
{  ++KeyPas; KeyPas&=0x3F;                          //  и его кода в кольцевой
     KeyBuffer[KeyPas].Key=Key;                     //  буфер для букв и кодов
     KeyBuffer[KeyPas].Code=ModifyKeyWithState();
 if( KeyPas==KeyPos ){ MessageBeep( MB_OK ); ++KeyPos; KeyPos&=0x3F; }
 if( !KeyInterrupt )
   { glEnv( *this ); KeyInterrupt=true;  // предустановка графической среды
     KeyBoard( Key ); KeyInterrupt=false; glFinish();
}  }
//!  обращение к клавиатуре через активное и контекстно настроенное окно Window
//                  ! осторожно, здесь предполагается отсутствие вызовов OpenGL
byte Window::WaitKey()                          // цикл ожидания нового символа
   { while( KeyPos==KeyPas )WaitEvents( true ); // обновление перед остановкой
     return KeyBuffer[++KeyPos&=0x3F].Key;      //   в том же окне
   }
byte Window::ScanKey()         // запрос появления нового символа на клавиатуре
   { WaitEvents(); return KeyPas==KeyPos ? 0 : KeyBuffer[KeyPos].Key; }
byte Window::GetKey()
   { WaitEvents(); if( KeyPas==KeyPos )return 0; return WaitKey(); }
byte Window::ScanStatus()
   { WaitEvents(); if( KeyPas==KeyPos )return ModifyKeyWithState();
                                 else  return KeyBuffer[KeyPos].Code; }
bool Place::Draw()       // в виртуальной среде Draw доступен внутренний пролог
{ if( extDraw )          // настройки OpenGL с контекстным эпилогом прорисовки
  { glEnv S(*Site);       // подготовка среды к внешнему исполнению с рекурсией
    if( extDraw() )
    { if( this==Site )Save().Refresh(); else Show(); return false; }
  } return true;
}
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
  { if( y||x ){ glEnv( *this ); Mouse( _MouseWheel,x,y ); } MouseState=0;
  } else
  { Place *P=this; int px=x,py=y;         //!  поиск последней/верхней площадки
    for( Place *S=P; S; S=S->Up )         //!     по общему списку их наложений
    { int _x=x-S->pX,_y=y+S->pY-Height+S->Height; // выход должен быть успешным
      if( unsigned( _y )<=unsigned( S->Height )
       && unsigned( _x )<=unsigned( S->Width ) ){ P=S; py=_y,px=_x; }
    }
   static volatile int xo=0,yo=-1;        //! рекурсия мышиных прерываний c
    if( !MouseState )                     //! координатами предыдущих вхождений
    { glEnv(*this); yo=-1; P->Mouse( px,py );
    } else                                          //! yo=-1 окно не захвачено
    { if( ModifyKeyWithState()==L_ALT && MouseState==_MouseLeft )
      { if( yo<0 )xo=x,yo=y; else     // перемещение окна left<Alt> и left<Btn>
        if( x!=xo || y!=yo )Locate( WindowX+x-xo,WindowY+y-yo,Width,Height );
      } else
      { glEnv(*this); yo=-1; P->Mouse( MouseState,px,py );
      }
    }
  } isMouse=0; // MouseState &= ~_MouseWheel;
}
//!  Прямое и параллельное обращение к таймеру с соблюдением очередей Windows
//!       (все расчеты в миллисекундах, опрокидывание через 49,7 суток)
//!
DWORD volatile                                          // тики [мс] от времени
      StartTime=GetTickCount();                         //     запуска Windows
DWORD ElapsedTime(){ return GetTickCount()-StartTime; } //  от старта программы
DWORD GetTime()
    { DWORD T=GetTickCount(); if( StartTime>T )StartTime=T; return T; }
bool Window::Timer()
{ if( extTime )        // настройка OpenGL с контекстным эпилогом перерисовки
  if( !isTimer )       // подготовка среды, вызов процедуры внешнего исполнения
  { bool Res; isTimer=true; Res=extTime(); isTimer=false; return Res;
  } return false;
}
Window& Window::SetTimer( DWORD mSec,bool(*inTm)() )// время и адрес исполнения
{ if( !mSec )                                      // пока только таймер №12
  { dTime=0.0; extTime=NULL; } else                // идентификатор не привязан
  { dTime=Real( mSec )/1000.0;               // glfwWaitEventsTimeout( dTime );
    nextTime=glfwGetTime()+dTime; extTime=inTm;
  } return *this;
}
void Window::PutTimer()             //? внутренняя обработка допускает пропуски
{ //if( !isTimer )                  //! потрясающе ! вдруг обнаружились повторы
  //if( !KeyInterrupt )             // внешних прерываний без исполнения первых
  //if( !isMouse )                  // ! еще и с клавиатурой - проблемы
  { //++isTimer;                    // на повторах - сбрасываются расчеты
    { glEnv(*this );
//    if( Timer() )Save().Refresh(); // с восстановлением наложенных страничек
      if( Timer() )Show();          // с перерисовкой картинки
//    if( Timer() )Refresh();       // с перерисовкой картинки
    } //isTimer=0;                  // разрешение повторных вхождений таймера
  } ScanKey();                      // после завершения всех операций OpenGL
}
#if 0
static DWORD mWait=20;
static bool (*extWait)()=NULL;  // процедура для главного вычислительного цикла
static void CALLBACK TimerProc( HWND,UINT,UINT_PTR timerId,DWORD )
{ KillTimer( 0,timerId );  // сразу отключаем таймер, пока не изменился timerId
  if( extWait )
  { if( extWait() )SetTimer( 0,0,mWait,TimerProc ); else mWait=0;
  } else mWait=0;  WinExecute();
}
void WaitTime( DWORD mSec,bool( *inStay )() ) // общая активная задержка потока
{ extWait=inStay; mWait=mSec;  // простой и бесконтрольный вариант без рекурсии
  WinExecute(); SetTimer( 0,0,mSec,TimerProc );         // כל в работе все окна
  while( mWait )if( !WinRequest() )WaitMessage();       // задержка для mWait=0
}
#else
void WaitTime( DWORD mSec, bool( *inStay )(), DWORD W ){ Sleep( mSec ); }
#endif
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Window::WaitEvents( bool stop )
{ for( Window *aW=First; aW; aW=aW->Next )
   if( aW->dTime )
   if( glfwGetTime()>=aW->nextTime )
     { aW->nextTime=glfwGetTime()+aW->dTime; aW->PutTimer(); }
  if( stop )
  { // Show();
    if( dTime )glfwWaitEventsTimeout( dTime );
    else glfwWaitEvents();
  } else glfwPollEvents();
}
///    Начальная установка базового шрифта, связанного с текущим окном OpenGL
///
Place& Place::AlfaBit( unsigned char *DispCCCP )     /// старый советский растр
{ if( !( BitFont=DispCCCP ) )BitFont=_8x16;           // в GLFW линкуется сходу
         FontWidth=BitFont[0]+1,FontHeight=BitFont[1]+2; return *this;
}
//!  Windows TrueType шрифт Courier New-18 устанавливается по умолчанию
//
Place& Place::Alfabet( int h, const char *fnt, int weight, bool italic )
{ if( FontBase )glDeleteLists( FontBase,ListsGroup ); FontBase=0;
  if( hFont )DeleteObject( hFont );                   BitFont=NULL;
   glfwMakeContextCurrent( Site->glfwWindow );
  HDC hDC = wglGetCurrentDC();
  LOGFONT logfont;                                // Setup Font characteristics
   logfont.lfHeight       = h<2?18:h;            //  высота шрифта = 18
   logfont.lfWidth        = 0;
   logfont.lfEscapement   = GM_COMPATIBLE;       //   GM_ADVANCED - 0
   logfont.lfOrientation  = 0;
   logfont.lfWeight       = weight;  //FW_DONTCARE+THIN,ULTRALIGHT,LIGHT,NORMAL
   logfont.lfItalic       = italic;  // MEDIUM,SEMIBOLD,BOLD,EXTRABOLD,HEAVY
   logfont.lfUnderline    = false;
   logfont.lfStrikeOut    = false;
   logfont.lfCharSet      = RUSSIAN_CHARSET; // FONT_CHARSET_UTF8; 65001; CP_UTF8; DEFAULT_CHARSET; ANSI_CHARSET
   logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;          // OUT_TT_PRECIS
   logfont.lfClipPrecision= CLIP_DEFAULT_PRECIS;         // OUT_STROKE_PRECIS
   logfont.lfQuality      = PROOF_QUALITY;               // DEFAULT_QUALITY
   logfont.lfPitchAndFamily=DEFAULT_PITCH;               //
   strcpy( logfont.lfFaceName,fnt?fnt:"Courier New" );   // имя шрифта 32 байта
        hFont = CreateFontIndirect( &logfont );          // Create the font
  HFONT oFont=(HFONT)SelectObject( hDC,hFont );          // and display list
   FontBase=glGenLists( ListsGroup );
   if( !wglUseFontBitmaps( hDC,0,ListsGroup,FontBase ) ) //! очень странная ??
        wglUseFontBitmaps( hDC,0,ListsGroup,FontBase );  //!       проблема ??
// { glDeleteLists( FontBase,ListsGroup==ListsGroup );
//   FontBase=0; FontWidth=FontHeight=1 } else
   { TEXTMETRIC T; GetTextMetrics( hDC,&T );
      FontWidth=T.tmAveCharWidth;  // (AlfaRect( "_Ё°y" ).cx+3)/4;
      FontHeight=logfont.lfHeight; // T.tmAscent; //tmHeight+tmExternalLeading;
   }
   if( oFont )SelectObject( hDC,oFont ); return *this;
}
//
//   Определение размеров изображения текст !!! необходим многострочный вариант
//
SIZE Place::AlfaRect( const char *s, bool ANSI ) // предварительнo формируется
{ SIZE F={ ANSI?strlen( s ):Ulen( s ),0 };      // в буфере UtA или ANSI-строки
   if( BitFont )F.cx*=FontWidth,F.cy=FontHeight; else
     { glfwMakeContextCurrent( Site->glfwWindow );
      HDC hDC=wglGetCurrentDC(); glListBase( FontBase );
      HFONT oFont=(HFONT)SelectObject( hDC,hFont ); char *str=(char*)s;
       if( !ANSI )str=UtA( s,false );          // осторожно с временным буфером
       GetTextExtentPoint32( hDC,str,F.cx,&F );
       if( oFont )SelectObject( hDC,oFont );
     } return F;
}
//! Подборка независимых от среды программирования транзакций в OpenGL
//  (пере)Установка размеров рабочей площадки/фрагмента внутри окна Window
//   если X,Y > 0 - отсчеты от левого верхнего угла, иначе - правого нижнего
//   если Width,Height > 0 - отсчеты в символах, если 0 - до самой границы окна
//                если < 0 - в пикселях и нормальных отсчетах, Y - снизу вверх
//     отсутствие шрифта соответствует размеру точки [ 1x1 ].
//
Place& Place::Area( int X,int Y, int W,int H )    // достаточно сделать шаг
{ int fw=FontWidth,fh=FontHeight;                // единичным и далее по Place,
   if( !W )W=Site->Width;  else if( W<0 )W=-W,fw=1; else W=W*fw; // связанной с
   if( !H )H=Site->Height; else if( H<0 )H=-H,fh=1; else H=H*fh; ++H; // Window
   if( X>0 )X=(X-1)*fw; else X = Site->Width - W + X*fw;
   if( fh>1 ){ if( Y>0 )Y = Site->Height - H - fh*(Y-1); else Y = -Y*fh; }
       else { if( Y<=0 )Y = Site->Height - H + Y; }
   if( W>Site->Width  )W=Site->Width;              // перерасчет с проверкой
   if( H>Site->Height )H=Site->Height;            // относительно главного окна
   if( X+W>Site->Width  )X=Site->Width-W;  if( X<0 )X=0;
   if( Y+H>Site->Height )Y=Site->Height-H; if( Y<0 )Y=0;
// if( Signs & PlaceAbove )
   if( Img )   // если ранее сохранялась фоновая подложка, то восстанавливается
   if( X!=pX || Y!=pY || Width!=W || Height!=H )Rest();  // растянутая картинка
       pX=X,pY=Y, Width=W,Height=H;
   if( chY>Height-FontHeight )chY=Height-FontHeight;// аппликата верхней строки
   if( Signs & PlaceAbove )Save();
   return *this;
}
//      Комплекс встроенных и внутренних процедур
//      Подготовка площадки для растровых манипуляций
//
Rastr_Sector::Rastr_Sector( int X,int Y, int W,int H )
             :dp( glIsEnabled( GL_DEPTH_TEST ) )
  { glPushAttrib( GL_VIEWPORT_BIT|GL_DEPTH_BUFFER_BIT );
    glPushMatrix(); glLoadIdentity(); glMatrixMode( GL_PROJECTION );
    glPushMatrix(); glLoadIdentity(); glViewport( X,Y,W,H );
    glOrtho( 0,W,0,H,-1,1 ); glMatrixMode( GL_MODELVIEW );
    if( dp )glDisable( GL_DEPTH_TEST );
  }
Rastr_Sector::~Rastr_Sector()
  { if( dp )glEnable( GL_DEPTH_TEST );
    glMatrixMode( GL_PROJECTION ); glPopMatrix();
    glMatrixMode( GL_MODELVIEW ); glPopMatrix(); glPopAttrib();
  }
TextContext::TextContext( bool b ): Base( b ),
  dp( glIsEnabled( GL_DEPTH_TEST ) ),
  cf( glIsEnabled( GL_CULL_FACE ) ),
  lh( glIsEnabled( GL_LIGHTING ) )
{ //if( Base )PushMatrix();
  if( dp )glDisable( GL_DEPTH_TEST );
  if( cf )glDisable( GL_CULL_FACE );
  if( lh )glDisable( GL_LIGHTING );
  glGetIntegerv( GL_POLYGON_MODE,(GLint*)pm );
  glPolygonMode( GL_BACK,GL_FILL );
//glListBase( FontBase );
}
TextContext::~TextContext()
{ glPolygonMode( pm[0],pm[1] );
  if( lh )glEnable( GL_LIGHTING );
  if( cf )glEnable( GL_CULL_FACE );
  if( dp )glEnable( GL_DEPTH_TEST );
  //if( Base )PopMatrix();
}
Place& Place::Clear( bool back ) // очистка фоном/true или текущим/false цветом
{ glScissor( pX,pY,Width,Height );  // Режим обрезки внешнего окружения и его
  glEnable ( GL_SCISSOR_TEST );     // временное включение для очистки площадки
  if( back )glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); else
  { float c[8]; glGetFloatv( GL_COLOR_CLEAR_VALUE,c );
                glGetFloatv( GL_CURRENT_COLOR,c+4 );
                glClearColor( c[4],c[5],c[6],c[7] );
                glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                glClearColor( *c,c[1],c[2],c[3] );
  } glDisable( GL_SCISSOR_TEST );     // разблокирование графических фрагментов
    glScissor( 0,0,0,0 ); return *this; //   и отключение обрезки
}
Place& Place::Show()                     //! прорисовка растрового фрагмента из
{ Rastr_Sector Sv( pX,pY,Width,Height ); //  фонового буфера в видимый рисунок
  glRasterPos2i( 0,0 );                        glDrawBuffer( GL_FRONT );
  glCopyPixels( pX,pY,Width,Height,GL_COLOR ); glDrawBuffer( GL_BACK );
  if( Signs&PlaceAbove )Save(); glFlush(); return *this;
}
//!    сохранение текущего изображения/фрагмента в буфере оперативной памяти
//        Img[0] - общая, контрольная длина массива
//        Img[1] x Img[2] x Img[3] - x·y·z - трехмерные размерности изображения
Place& Place::Save()                   // фрагментация здесь вполне естественна
{ if( Width>0 && Height>0 )            // и не требует особой работы со списком
  { unsigned Size=sizeof( int )*( Width*Height+4 );
    glPushAttrib( GL_VIEWPORT_BIT );
    glViewport( 0,0,Site->Width,Site->Height );
    if( !Img        )Img=(unsigned*)malloc( Size ); else
    if( Size>Img[0] )Img=(unsigned*)realloc( Img,Size );
    if( Img )
    { Img[0]=Size,Img[3]=0; glReadPixels
      ( pX,pY,Img[1]=Width,Img[2]=Height,GL_RGBA,GL_UNSIGNED_BYTE,Img+4 );
    }
  //glViewport( pX,pY,Width,Height );
    glPopAttrib();
  } glFlush(); return *this;
}                        //
Place& Place::Rest()     //! прямое восстановление растра из собственной памяти
{ if( Img )              //  в фоновый буфер изображения с перемасштабированием
  { Rastr_Sector Sv( pX,pY,Width,Height ); glRasterPos2i( 0,0 );
    if( Width!=int( Img[1] ) || Height!=int( Img[2] ) )
    glPixelZoom ( float( Width )/Img[1],float( Height )/Img[2] );
    glDrawPixels( Img[1],Img[2],GL_RGBA,GL_UNSIGNED_BYTE,Img+4 );
    glPixelZoom ( 1.0,1.0 );
  } glFlush(); return *this;
}
//
//!   Работа с текстами в стиле Microsoft-Windows
//
static		               // Растровый шрифт DOS-OEM(866)alt из эпохи CCCP
void _OutBitText( const char *s, const byte *font, int& X,int& Y )
{ int w=font[0]+1,h=font[1]+2;               // в разборе текста контролируется
  char c,*bst=UtA( s,true );                 //   переход на новую строку
   glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );
   glPixelStorei( GL_UNPACK_SWAP_BYTES,false ); // Step through the string,
   glPixelStorei( GL_UNPACK_LSB_FIRST, false ); //  drawing each character.
   glPixelStorei( GL_UNPACK_ROW_LENGTH, 0    ); // A newline will simply
   glPixelStorei( GL_UNPACK_SKIP_ROWS,  0    ); // translate the next
   glPixelStorei( GL_UNPACK_SKIP_PIXELS,0    ); // character's insertion
   glPixelStorei( GL_UNPACK_ALIGNMENT,  1    ); // point back to the start of
   while( ( c = *bst++ )!=0 )                   // the line and down one line
    if( c=='\n' ){ glBitmap( 0,0,0,0,-X,-h,NULL ); X=0; Y-=h; } else
                 { glBitmap( w-1,h-2,         // Bitmap's width and height
                              -1,1,           // The origin in the font glyph
                               w,0,           // смещение до новой позиции
                             font+2+c*(h-2)   //  + 2 байта и номер Х на высоту
                           ); X+=w; }         // сдвиг позиции и растра символа
   glPopClientAttrib();
}
//!   Работа с текстами в стиле Microsoft-Windows
static
void _OutText( const char *T, int base, int& X,int& Y,int h,int hP=0 )
{ char *s,*str=UtA( T,false );              // собственно печать текста связана
   glPushAttrib( GL_LIST_BIT );             // с разборкой переносов строк
   glListBase( base );
Rp:if( hP )
   if( (s=strchr( str,'\n' ) )!=NULL )         // ... смещение X делается вовне
     { *s=0; if( str[0] )glCallLists( strlen( str ),GL_UNSIGNED_BYTE,str );
       glRasterPos2i( X=0,Y-=h );
       if( s[1] ){ memmove( str,s+1,strlen( s+1 )+1 ); goto Rp; } str[0]=0; }
   if( str[0] )glCallLists( h=strlen( str ),GL_UNSIGNED_BYTE,str );
   glPopAttrib();
}
Place& Place::String( const char *str )
{ static float P[4]={ 0.0,0.0,0.0,0.0 }; Rastr_Sector Sv( pX,pY,Width,Height );
  glRasterPos2i( chX,chY );
  if( BitFont )_OutBitText( str,BitFont,chX,chY );  // chX - удлиняется изнутри
  else{ _OutText( str,FontBase,chX,chY,FontHeight,Height );
        glGetFloatv( GL_CURRENT_RASTER_POSITION,P ); chX=P[0]-pX;
      } glFinish(); return *this;
}
//!  Прорисовка плоско/горизонтального текста по предварительному определению
//!       размеров букв и смещением надписи относительно точки в плоскости XY
//
Place& Place::String( Course Dir, const Real *P, const char* str )
{ int dx,dy;                                    // плоская надпись со смещением
   glRasterPos3d( P[0],P[1],P[2] );
  SIZE sz=AlfaRect( str );  dx = -sz.cx/2;                           // _Center
   if( Dir&_East  )dx = 4;  dy = -sz.cy/6;
   if( Dir&_West  )dx = -sz.cx - 4;
   if( Dir&_North )dy =  sz.cy/5 + 3;
   if( Dir&_South )dy = -sz.cy + 3; glBitmap( 0,0,0,0,dx,dy,NULL );  // сдвиг и
   if( BitFont )_OutBitText( str,BitFont,dx,dy );             // переносы строк
          else  _OutText( str,FontBase,dx,dy,0 );            // здесь отключены
   return *this;
}
#include <StdIO.h>                            // блок разных текстовых надписей
#include <StdArg.h>
#define ArgStr int len; va_list aV; va_start( aV,fmt );                    \
  if( ( len=vsnprintf( 0,0,fmt,aV ) )>0 ){ va_start( aV,fmt ); wl[len+2]=0; \
  vsnprintf( wl,len+1,fmt,aV ); } va_end( aV ); if( len<=0 )return *this;

static string wl;                // просто "временная" строчка на всё и про всё
Place& Place::Print( const char *fmt,... ){ ArgStr String(wl); return *this; }
Place& Place::Print( int x, int y, const char *fmt, ... )
{ ArgStr char *s,*w=wl; --x;
  while( s=w )
  { if( (w=strchr( s,'\n' ))!=NULL )*w++=0;
              chY=y>0 ? Height-FontHeight*y++ : 3-FontHeight*y--;
    if( x>=0 )chX=x*FontWidth;
         else chX=max( 0L,Width+FontWidth*x-AlfaRect( s ).cx ); String( s );
  } return *this;
}
Place& Place::Text( Course Dir, const Real *P, const char* fmt, ... )
     { ArgStr String( Dir,P,wl ); return *this; }
Place& Place::Text( Course Dir, _Real X,_Real Y,_Real Z, const char* fmt, ... )
     { Real P[3]={ X,Y,Z }; ArgStr String( Dir,P,wl ); return *this; }
#if 0
/**
 *\brief       Function for converting utf-8 string to cp1251 using winapi.
 *\warning     Result string must be free after using.
 *\param[in] s C-string in utf-8 for converting.
 *\return      String in cp1251 encoding.
 */
char* win_from_utf8_to_cp1251(const char * s)
{
  int wchars_num=MultiByteToWideChar( CP_UTF8,0,s,-1,NULL,0 );
  wchar_t* wstr =new wchar_t[wchars_num];
                 MultiByteToWideChar( CP_UTF8,0,s,-1,wstr,wchars_num );
  int chars_num =WideCharToMultiByte( 1251,0,wstr,wchars_num,NULL,0,NULL,NULL );
  char * str = new char[chars_num];
  WideCharToMultiByte(1251, 0, wstr, wchars_num, str, chars_num, NULL, NULL);
  delete[] wstr;
  return str;
}                                    //             win_from_utf8_to_cp1251(s);
#endif
void Break( const char Msg[], ... )   // Случай аварийного завершения программы
{ va_list V; va_start( V,Msg );
  char str[vsprintf( 0,Msg,V )+1]; vsprintf( str,Msg,V ); va_end( V );
  MessageBox( NULL,UtA( str ),"Break",MB_ICONASTERISK|MB_OK ); exit( 1 );
}
