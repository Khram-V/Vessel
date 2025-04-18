//
//      Tv_getc : запрос команды от терминала
//         key: @D - Рекурсивная процедура дооформления рисунка
//              @X - быстрый выход из программы
//              ^C - exit
//      return(cr) - <Enter>    - обычный выход из командного режима
//            (14) - <Insert>   - выход по команде включения точки
//            (15) - <Delete>   - выход по команде исключения точки
//            (16) - <BakSpace> - для текстовых режимов
//            (17) - <Tab>      - табуляция
//            (27) - <Escape>   - выход с отменой команды
//       (0xF1..С) - <Function>
//
//      Tv_getk : запрос состояния блокировок клавиатуры
//            1 - RightShift    16 - ScrollLock
//            2 - LeftShift     32 - NumLock
//            4 - Ctrl          64 - CapsLock
//            8 - Alt          128 - Insert
//
#include "Tv_Graph.h"
                                //
       int  Tv_MouseState=0;    // Признак доступности мышиного интерфейса
static int  Tv_isMouse=NoMouse; //
static BYTE CKey,               // Код управляющих клавиш
            VKey;               // Код принятого символа
static int KeyPos=0, KeyPas=0;  // Count of keys in KeyBuffer
static struct{ BYTE Code,Key; } //
               KeyBuffer[0x40]; // Keyboard type-ahead buffer
static void Kput()              //
{ KeyBuffer[KeyPas].Code=CKey;  // беззнаковый кольцевой буфер для букв и кодов
  KeyBuffer[KeyPas].Key=VKey;        VKey=0;  ++KeyPas; KeyPas&=0x3F;
  if( KeyPas==KeyPos ){ MessageBeep( MB_OK ); ++KeyPos; KeyPos&=0x3F; }
}
static bool Kget()
{ if( KeyPas==KeyPos ){ VKey=CKey=0; return false; }
  CKey=KeyBuffer[KeyPos].Code;
  VKey=KeyBuffer[KeyPos].Key;  ++KeyPos; KeyPos&=0x3F; return true;
}                                    //
bool Tv_isKey()                      //! Return keyboard status
{ MSG Msg;                           //
   while( PeekMessage( &Msg,0,0,0,PM_NOREMOVE ) )
   {      GetMessage ( &Msg,0,0,0 ); //
//    switch( Msg.message )          // Блокировка системных аккордов
//    { case WM_SYSKEYUP:            //   кроме <altSpace>,<atlTab>
//      case WM_SYSKEYDOWN:          //
//       if( Msg.wParam!=VK_SPACE
//        && Msg.wParam!=VK_TAB )Msg.message&=WM_KEYDOWN;
//    }
      TranslateMessage( &Msg );
      DispatchMessage ( &Msg );
   }  return KeyPas!=KeyPos;
}
static BYTE WinAsyncKeyStates( BYTE code=0 )            // простой опрос
{ if( GetAsyncKeyState( VK_LSHIFT   ) )code|=LEFT;      // состояния клавиатуры
  if( GetAsyncKeyState( VK_RSHIFT   ) )code|=RIGHT;     // с установкой
  if( GetAsyncKeyState( VK_LCONTROL ) )code|=CTRL;     // командных аккордов
  if( GetAsyncKeyState( VK_RCONTROL ) )code|=CTRL;
  if( GetAsyncKeyState( VK_LMENU    ) )code|=ALT;
  if( GetAsyncKeyState( VK_RMENU    ) )code|=ALT; return code;
}
// WM_CHAR message handler
//
void WindowChar( TCHAR Char )
{ if( Char==VK_CANCEL )_exit( 3 );     // 3
  if( Char==VK_BACK )VKey=_BkSp; else  // 8 -> 16
  if( Char==VK_TAB  )VKey=_Tab; else   // 9 -> 17
                     VKey=W2D( Char );
                     CKey=WinAsyncKeyStates(); Kput();
// printf( "Char=%d, CKey=%d \n",Char,CKey );
}
//#define VK_ALT 0x12 //=18
//   WM_KEYDOWN \ WM_KEYUP
//
void WindowKeyDown( WPARAM wVkey, LONG dwKeyData )
{ struct            // Данные для поддержки Tv_Graphics
  { WORD Count;     // Счетчик нажатий
    BYTE SCode;     // Scan-код нажатой клавиши
    BYTE ECode;     // Расширенный код Windows
  }      WKey;      // Расширение от последней клавиши
  memcpy( &WKey,&dwKeyData,sizeof( WKey ) ); VKey=0; //WKey.SCode;
  if( WKey.ECode&0x80 )               // Клавиша отпущена
  { if( wVkey==VK_SHIFT   )CKey&=~SHIFT; else
    if( wVkey==VK_CONTROL )CKey&=~CTRL; else
    if( wVkey==VK_MENU    )CKey&=~ALT; return;
  }
//CKey=WinAsyncKeyStates();
  switch( wVkey )                  // 01 - команды, 02 - alt-клавиша
  { case VK_CONTROL: CKey|=CTRL;      break;
    case VK_SHIFT  : CKey|=SHIFT;     break;
    case VK_MENU   : CKey|=ALT;       break;
    case VK_UP     : VKey=North;      break;
    case VK_LEFT   : VKey=West;       break;
    case VK_DOWN   : VKey=South;      break;
    case VK_RIGHT  : VKey=East;       break;
    case VK_HOME   : VKey=North_West; break;
    case VK_NEXT   : VKey=South_East; break;
    case VK_END    : VKey=South_West; break;
    case VK_PRIOR  : VKey=North_East; break;
    case VK_INSERT : VKey=_Ins;       break;
    case VK_DELETE : VKey=_Del;       break;
//  case VK_TAB    : VKey=_Tab;       break;
    default        :
     if( !(WKey.ECode&0x40) )
     { if( wVkey>=VK_F1 && wVkey<=VK_F12 )VKey=wVkey+(0xF1-VK_F1); else
       if( CKey&ALT )VKey=wVkey;
     }
  }
  if( VKey ){
//  printf( "CKey=%d, VKey=%d, wVkey=%d, dwKeyDate={%d,x%X,x%X}  \n",CKey,VKey,wVkey,WKey.Count,WKey.SCode,WKey.ECode );
    Kput();
  }
}
//  Процедуры обращения к клавиатуре связаны с EasyWin средой    1994-August-26
//
  void Tv_bell(){ MessageBeep( MB_OK );  } // Простой звонок
Course Tv_getk(){ return Course( CKey ); } // Состояние клавиатуры
Course Tv_getc( const int mSec )           // Последняя команда с клавиатуры
{ while( !Tv_getw() ){ if( mSec ){ Sleep( mSec ); break; } Sleep( 24 ); }
  return (Course)VKey;                     // ~42 посылки в секунду
}                                          //
Course Tv_getw()                           // Один запрос и возврат результата
{ A:  VKey=0;                              //
  if( Tv_isKey() )
  { Kget();

//if( GetAsyncKeyState( VK_LMENU ) )CKey|=ALT;
//if( GetAsyncKeyState( VK_RMENU ) )CKey|=ALT;
//printf( "CKey=%d, VKey=%d  \n",CKey,VKey );

    if( CKey==ALT && VKey>' ' )
    { switch( VKey|0x20 )
      { case 'c': case 225: Tv_revert(); break;             // 'c'
        case 'd': case 162: Tv_draw();   break;             // 'в'
        case 'x': case 231:                                 // 'ч'
        { int h=Tv.Th; Tv.Height( 8 ); Thelp( "> Завершить. Вы уверены?" );
          Tv_getc(); if( VKey==_Enter || (VKey|0x20)=='y' )exit( 0 );
          Ghelp();   Tv.Height( h ); // рекурсия на <alt> !!
      } } goto A;
  } } return Course( VKey );
}
Course Tv_getw( Course Simb ){ if( Tv_getw()==Simb )return Simb; return Center; }
//
//!  <<  Блок процедур обычного (параллельного) доступа к мышке  >>
//
static int X=0,Y=0;                     // Активизация параллельной мышки
 int Tv_Stop_Mouse(){ int i=Tv_isMouse; Tv_isMouse=NoMouse; return i; }
 int Tv_Start_Mouse(){ int i=Tv_isMouse; Tv_isMouse=MouseInstalled; return i; }
 int Tv_MouseStatus(){ return Tv_MouseState; } // { int St=Tv_MouseState; Tv_MouseState=0; return St; }
void Tv_CursorEnable(){  ShowCursor( true ); }    // Визуализация и
void Tv_CursorDisable(){ ShowCursor( false ); }   // скрытие мышиного курсора
//void WindowMouse( int x, int y );               //
void WindowMouse( UINT messg, int x, int y )
{       Y=y; X=x;
  switch( messg )
  { case WM_MOUSEMOVE     : Tv_MouseState=1;                   break;
    case WM_LBUTTONDOWN   : Tv_MouseState=_MouseLeftPress;     break; //  2
    case WM_LBUTTONUP     : Tv_MouseState=_MouseLeftRelease;   break; // 32
    case WM_LBUTTONDBLCLK : Tv_MouseState=_MouseLeftDblClick;  break; //  8
    case WM_RBUTTONDOWN   : Tv_MouseState=_MouseRightPress;    break; //  4
    case WM_RBUTTONUP     : Tv_MouseState=_MouseRightRelease;  break; // 64
    case WM_RBUTTONDBLCLK : Tv_MouseState=_MouseRightDblClick; break; // 16
    case WM_MBUTTONDOWN   :
    case WM_MBUTTONUP     :      //
    case WM_MBUTTONDBLCLK :      // Прямое считывание
    case WM_MOUSEWHEEL    :      //  и установка координат курсора
   default: Tv_MouseState=0;     //
  }
  if( Tv_isMouse==MouseInstalled )
  {                       VKey=0;
    if( Tv_MouseState&30 )VKey=_MousePressed;  else // простой и двойной клик
    if( Tv_MouseState&96 )VKey=_MouseReleased; else
    if( Tv_MouseState&1  )VKey=_MouseMoved;
    if( VKey )Kput();
} }
 int Tv_GetCursor( int &x, int &y ){ x=X; y=Y; return Tv_MouseState; }
void Tv_SetCursor( int x, int y  ){ setwindowmouse( X=x,Y=y ); }
 int Tv_MouseX(){ return X; }    // Текущие координаты
 int Tv_MouseY(){ return Y; }    //  мышиного курсора
//
//!      Дополнительный набор всяких полу-системных функций
//
Real Tv_step( Real Di )              // Вычисление недробного шага из
{ Real iPart=floor( Di=log10( Di ) ); // процедуры стандартной разметки осей
       Di = exp( (Di-iPart)*M_LN10 );
 return pow( 10.0,iPart )*( Di>6.0 ?2.0 : Di>3.0 ?1.0 : Di>1.5 ?0.5:0.2 );
}
unsigned coreleft()
{ static MEMORYSTATUS Ms={ sizeof(MEMORYSTATUS),0,0,0,0,0,0,0 };
  GlobalMemoryStatus( &Ms ); return Ms.dwAvailPhys;
}
//unsigned long farcoreleft(){ return coreleft(); }  // GetFreeSpace( 0 );
//                                                  // GlobalCompact( 0 )/1024;
//!      Аварийное завершение программы
//
void Break( const char Msg[],... )
{ va_list aV;
  va_start( aV,Msg ); va_end( aV ); char Str[120]; vsprintf( Str,Msg,aV );
  MessageBox( NULL,Str,"Break",MB_ICONASTERISK|MB_OK ); _exit( 1 );
}
//     Присоединенные здесь модули могут быть отключенны
//                  и затем транслированы отдельно
//
//  Tv_draw.cpp : Встроенная процедура дооформления рисунка
//
static const
  char Hp[]="F1 Help  F2 Text  F3 Object <Enter>Fix <Ins>New <Del>Old <Tab>%s";
static point p,*V;    // Местоположение, адрес интерполируемой кривой
static Point          //
       Ry={ 0,-100 },Rx={ 100,0 }, // Базисные оси
       As={ 0.6,.6 },Qs={ .4,.4 }; // Эллипс и прямоугольник
static int            // 0 - Базисные оси;  1 - Эллипс
       Pic=0,         // 2 - Прямоугольник; 3 - Кривая линия
       Nv=0,          // Длина массива в памяти       ¦ Enter  - Fix
       Mv=0,          // Контрольное количество точек ¦ Insert - Add
       Kv=0,          // Номер подвижной точки        ¦ Delete - Sub
       Pv=0,          // Степень интерполяции         ¦ '+','-'  Change
       Cl=LIGHTBLUE,  // Цвет                         ¦ '`'Ў'0'  Power
       Tl=SOLID_LINE, // Сохраняемый тип линии        ¦ ' '      Shift
       Sh=0;          // Признаки SHIFT
static Basis Bas;     //
                      //
static point value( point *V, int N, float Ar, int Pw )
{
// Интерполяция по Лагранжу на равномерном интервале
//                 V  - вектор значений функции
//                 N  - текушая длина вектора V
//                 Ar - интерполяционный аргумент
//                 Pw - степень интерполяции
 float yo=0,xo=0,p;           //
 int   Nb,Nf,j=(int)Ar,k=N-1; // Поиск ближайшего левого индекса
  if( j>k )j=k; else          //
  if( j<0 )j=0;
  if( Pw>k )Pw=k;
  if( Pw<=0 )return V[j];
  if( (Nf=++j+Pw/2)>k )Nf=k,Nb=k-Pw; else
  if(  ( Nb=Nf-Pw )<0 )Nb=0,Nf=Pw;
  for( j=Nb;j<=Nf;j++ )
  { for( p=1,k=Nb; k<=Nf; k++ )if( j!=k )p*=(Ar-k)/(j-k);
    yo+=V[j].y*p;
    xo+=V[j].x*p;
  } point q; q.y=int(yo+.5); q.x=int(xo+.5); return q;
}                       //
static point R2p( const float& X, const float& Y ){
 Point Q; Q.x=X;
 point q; Q.y=Y; Q=Bas.Goext( Q ); q.y=(int)Q.y;
                                   q.x=(int)Q.x; return q;
}
static void Axis()
{ point r = R2p( 0,0 ),u;
  line( r,u=R2p( 0,0.8 ) ); needle( u,R2p( 0,1.1 ) );
  line( r,u=R2p( 0.8,0 ) ); needle( u,R2p( 1.1,0 ) );
}
static void Ellipse(){ const float dt = ( M_PI_2/12 );
 float angle=0;
  for( int k=0; k<=48; k++,angle+=dt )
  { point q=R2p( As.x*cos( angle ),As.y*sin( angle ) );
    if( k )lineto( q );
      else moveto( q );
} }
static void RectAngle()
{ point q; moveto( q=R2p(  Qs.x, Qs.y ) );
           lineto(   R2p( -Qs.x, Qs.y ) );
           lineto(   R2p( -Qs.x,-Qs.y ) );
           lineto(   R2p(  Qs.x,-Qs.y ) );
           lineto( q );
}                                 //
static int Not_First_Curve=FALSE; // Используется без выхода на рекурсию
                                  //
static void Curve( int Sg )
{ if( !Mv )line( Tpoint,p ); else
  { float w=1e0/Sg,W=w;
    if( Not_First_Curve )p=V[Kv],Not_First_Curve=FALSE; else V[Kv]=p;
                Sg*=Mv-1;         moveto( V[0] );
    for( int j=0; j<Sg; W+=w,j++ )lineto( value( V,Mv,W,Pv ) );
} }
static void Building_Curve( int ans ) //     ¦ Enter  - Fix
{      //   Nv  Длина массива в памяти       ¦ Insert - Add
       //   My  Контрольное количество точек ¦ Delete - Sub
       //   Ky  Номер подвижной точки        ¦ '+','-'  Change
       //   Pv  Степень интерполяции         ¦ '0'-'9'  Power
 point q; float w; int k=0;
  switch( ans )
  { case _Ins:
    if( Nv<=Mv )V=(point*)Allocate( sizeof(point)*(Nv+=12),V );
    if( !Mv )
    { V[1]=(V[0]=Tpoint)+(V[2]=p); V[1].x/=2;
                   Mv=3,Pv=2,Kv=1; V[1].y/=2;
    } else
    { if( Kv==Mv-1 )w=0.1+(float)Kv,Kv++;
      else if( Kv  )w=0.5+(float)Kv,Kv++; else w=-.1;
      q=value( V,Mv,w,Pv ); for( k=Mv; k>Kv; k-- )V[k]=V[k-1]; V[Kv]=q;
      Pv++;
      Mv++;
    } k=1; break;               // k - Признак корректировки
    case _BkSp: p=Tpoint;
    case _Del :if( Mv<=3 )Mv=0;
     else{ --Mv; for( k=Kv; k<Mv; k++ )V[k]=V[k+1]; if( Kv==Mv )--Kv; }
             k=1; break;
    case ' ':if( Mv>0 ){ q=p-V[Kv]; for( k=0; k<Mv; k++ )V[k]+=q; }
                 k=0;  Not_First_Curve=TRUE; break;
    case '=':
    case '+':if( Mv>2 ){ if( Kv<Mv-1 )Kv++; else Kv=0; k=1; }break;
    case '-':
    case '_':if( Mv>2 ){ if( Kv>0 )Kv--; else Kv=Mv-1; k=1; }break;
    case '`': Pv=Mv-1;
   default  :if( ans>='0'&&ans<='9' )Pv=ans-'0';
  }
  if( k ){ Not_First_Curve=TRUE; Tpoint=p; }
}
static const float Rels=1.02;

static void Deform( Point& R, int C )
{ if( C&North )R.y*=Rels; if( C&East )R.x*=Rels;
  if( C&South )R.y/=Rels; if( C&West )R.x/=Rels;
}
static void Draw( int Wmod=1, int ans=0 ) // Wmod=0 - Закрепление
{                                       //      1 - Текущий объект
 point w=Tpoint;                        //      2 - С обработкой ANS
  setwritemode( Wmod?XOR_PUT:COPY_PUT );
  switch( Pic )
  { case 0: Axis();      break;
    case 1: Ellipse();   break;
    case 2: RectAngle(); break;
    case 3: Curve( Wmod?6:12 );
  } Tpoint=w; setwritemode( COPY_PUT ); // Закрепление рисунка
  if( !Wmod )
  { if( Pic==3 && (Sh&SHIFT)==0 )Mv=0; Tpoint=p;
  } else
  if( Wmod==2 )                         // Обработка команды
  { if( Pic==3 )Building_Curve( ans ); else
    if( ans<_Enter && (Sh&SHIFT)==0 )
    switch( Pic )
    { case 0:{  float S1=0.0174524; const float C1=0.9998477;
        switch( ans )
        { case North: Ry*=Rels; break; case East: Rx*=Rels; break;
          case South: Ry/=Rels; break; case West: Rx/=Rels; break;
         default:
          if( ans&East  )S1=-S1;
          if( ans&South )Rx.x=Rx.x*C1+Rx.y*S1,Rx.y=Rx.y*C1-Rx.x*S1;
                         Ry.x=Ry.x*C1+Ry.y*S1,Ry.y=Ry.y*C1-Ry.x*S1;
      } }                       break;
      case 1: Deform( As,ans ); break;
      case 2: Deform( Qs,ans ); break;
  } }
}
static void Init(){ Point Re;
  Re.x=p.x;
  Re.y=p.y; Bas.Axis( Re,Ry,Rx );
  setlinestyle( Tl&3,0,Tl&4?THICK_WIDTH:NORM_WIDTH ); Draw();
}
static byte *Svm;
static void Mpix( int k, int x, int y, int j ){ byte c=Cl;
  do
  { if( !k )Svm[0]=getpixel(x+j,y); else c=Svm[0]; putpixel( x+j,y,c ); ++Svm;
  } while( (j=-j)<0 );
}
static int Marker()
{ byte Save[60];             // 60 = 32+16+12
 point q=Tpoint;
 int A=0,k,j;
  for( k=0;;k++ ){ Svm=Save;
    for( j=1; j<=16;j++ )Mpix( k,p.x,p.y,  j );
    for( j=-3;j<=8; j++ )Mpix( k,p.x,p.y+j,0 ); if( k )break;
    for( j=1; j<=8; j++ )Mpix( 0,q.x,q.y-j,j ); A=Tv_getc(); Svm=Save+44;
    for( j=1; j<=8; j++ )Mpix( 1,q.x,q.y-j,j ); Sh=Tv_getk();
  } return A;
}
void Tv_draw()                 //
{ static int Level=-1,         // Контролируемый уровень рекурсии
             Mode=0;           // Текущий режим изображения
  if( Level>0 )return;         //
 int ans,c=color( Cl ),bc,k,Hl=0;
  if( Level<0 ){ Cl=(bc=Tv.BkColor)==BLACK ? YELLOW:BLACK; p.x=p.y=24; }
 point K=Tpoint;       Tpoint=p;
 linesettingstype lin; getlinesettings( &lin ); // Тип линии в главной программе
//pattern( WHITE );            //
  Level=1;
Repeat:
  if( Mode )Init(); ans=Marker();
  if( Mode )Draw( 2,ans );  if( Hl ){ Ghelp(); Hl=0; };
  if( ans<_Enter )              // Режим позиционирования и подписей
  { k=!Mode || Pic==3;          //
    if( k || Sh&SHIFT )
    { k=k && Sh&SHIFT?(Mode?12:3):1; if( Mode==0 )k *= Tv.Th+2;
      if( ans&North )p.y-=k;
      if( ans&South )p.y+=k; if( Mode==0 )k=(k*Tv.Tw)/(Tv.Th+2);
      if( ans&West  )p.x-=k;
      if( ans&East  )p.x+=k;
    }
  } else switch( ans )
  { case 0xF3:  if( Mode )++Pic&=3; else Mode=1;
                if( Mode && Pic==3 )Building_Curve( ' ' ); break;
    case 0xF2:  Mode=0; break;
    case 0xF1:{ point q=Tpoint;
         Ghelp( Hp,Mode?(Pic==3?"Line <+|->Change":"Style"):"Color" );
                Hl=1; Tpoint=q;
              } break;
    case _Ins: Tpoint=p; break;
    case _Del: p=Tpoint; break;
    case _Tab:
      if( !Mode || Sh&SHIFT )color( ++Cl&=15 ); else ++Tl&=7; break;
    case _Esc: color( c ); // pattern( bc );
         setlinestyle( lin.linestyle,lin.upattern,lin.thickness );
         p=Tpoint;
         Tpoint=K;
         Level=0;
         return;
   default:
    if( !Mode )
    switch( ans )
    { case _BkSp :Tpoint.x-=Tv.Tw; Tputc(' ');
                  Tpoint.x-=Tv.Tw;   break;
      case _Enter:Tpoint.x=p.x;
                  Tpoint.y+=Tv.Th+2; break;
     default: if( ans>=' ' && ans<0xF1 )Tputc( ans );
    }
    else if( ans==_Enter )Draw( 0 );
  } goto Repeat;
}
//
//  Display/Tmenu                                   \91.06.18\.
//    Пакет диалога с терминалом с помощью таблиц запросов
//      Y,X   - координаты левого верхнего угла для окна запросов
//      Mlist - список параметров для запросов на терминал
//      Nm    - количество запросов в списке Mlist
//     <PgUp> - переход в начало меню
//     <PgDn> - переход в конец меню
//     return - номер последнего активного запроса
//
//  Mlist - Список параметров одного запроса на терминал
//          INPUT                         ACTIVE
//      Y   : skip string                 : номер сроки
//      X   : !0 - готовность             : номер позиции для сообщения
//      lf  :  0 - запрос не производится : длина входного сообщения
//      S   :  ~ строящийся формат чтения данных
//      Msg : NULL - чистое входное поле  : выходной формат
//      dat : NULL и lf<>0 - меню-запрос  : адрес изменяемого объекта
//
Display::Display( Mlist *__M, int n, int x, int y )
{ X=x; M=(mlist*)__M; Nm=n; Hl=0; Y=y; K=0; Active=false;
}
Display::Display( void HL(int), Mlist *__M, int n, int x, int y )
{ X=x; M=(mlist*)__M; Nm=n; Hl=HL; Y=y; K=0; Active=false;
}
#define setC { fc=Tv.FColor; c=color( DARKGRAY ); bc=bkcolor( LIGHTCYAN ); }
#define retC { color( c ); pattern( fc ); bkcolor( bc ); }
#define _s( S ) ( *(S)!='\t'?(S):W2D( (S)+1 ) )

static mlist *Ls;                               //
static char St[244]="";                         // Утроенная рабочая строка
static int Mestr()                              // Разбор текстовой строчки
{                                               //
 char *S=(char*)(Ls->Msg); void *dat=Ls->dat; int lc=0,k=0,l=Ls->lf; Ls->S='s';
  if( !S )*St=0; else
  { lc=strlen( strcpy( St,_s( S ) ) ); while( k<lc && St[k]!='%' )k++; }
  if( dat!=NULL )                               //
  { if( k==lc )strcat( St,_s( (char*)dat ) );   // Here is S==NULL
    else                                        //
    { char ch=St[lc-1];
      short lh=St[lc-2];                        //
      if( ch=='°' || ch==0xF8 )                 // Degrees
      { lh-='0'; Ls->S=0xF8;                    //  '°';
        if( fixed( lh )>3 )lh=0; else if( St[k+1]=='-' )lh=-lh;
        strcpy( St+k,DtoA( *(double*)dat,lh,0xF8 ) );
      } else                                    // and another formats
      { ch|=0x20;                               // tolower( S )
        lh=lh=='l';                             //
        Ls->S=lh ? ch|0x80:ch;                  // (long)|(double)
        if( ch>'d' && ch<'h' )                  //
        { if( lh )          sprintf( St,_s( S ),*(double*)dat );
             else           sprintf( St,_s( S ),*(float*)dat );
        } else if( ch!='s' )sprintf( St,_s( S ),*(long*)dat );
          else              sprintf( St,_s( S ),dat );
    } }
  } else if( l ){ if( l<k )l=k; k=0; }
       Ls -> lf=l;                  St[l+=k]=0;
  for( lc=strlen( St ); lc<l; lc++ )St[lc]=' '; return k;
}
static int Message()
{ int k=Mestr(); Tgo( Ls->X,Ls->Y );   Tpoint.y+=2; Tprintf( St );
                 Tgo( Ls->X+k,Ls->Y ); Tpoint.y+=2; return k;
}
int Display::Answer( int _K )
{ const int IO=81,            //  81-161 - строка обмена с терминалом
            BF=162;           // 162-242 - сохранение для регистрации изменений
 static int lf=0;             //       Начальная инициализация списка сообщений
 int  ls,Lx=0,Ly=1,bc,fc,c,k,kw=_K>=0 && _K<Nm;
 bool key=( M->X==0 );
  setC
  for( Ls=M,k=ls=0; k<Nm; Ls++,k++ )    //
  { if( key )Ls->Y+=Ly;                 // новый номер строки
    if( Ls->Y!=Ly )ls=0,Ly=Ls->Y;       // поиск новой строки
    if( key )Ls->X=ls+1;                // позиция начала сообщения
    ls += Mestr();                      // новая длина строки
    ls += Ls->lf;                       //  + длина входного поля +1
    if( Lx<ls )Lx=ls;                   // выбор ширины окна      -1
  }                                     //
  key |= !Active | !kw;                 // заполнение окна сообщений
  if( !Active )Twindow( X,Y,Lx,Ly,1 );  //
  if( key )for( Ls=M,lf=k=0; k<Nm; Ls++,k++ ){ Message(); lf+=Ls->lf; }
 int ans=0; k=1; kw=kw?_K:K;
  if( !lf ){ Tv_bell(); Tv_getc(); }
  else do
  { if( kw<0 )kw=Nm-1; else if( kw>=Nm )kw=0;
        K=kw;
        Ls=M+kw;
    if( Ls->lf && Hl ){ retC Hl( kw ); setC } ls=Message();
    if( Ls->lf )
    { if( Ls->dat )ans=Tgets( strcpy( St+IO,strcpy( St+BF,St+ls ) ),Ls->lf );
      else
      { int l=Tpoint.x-2, r=l+Tv.Tw*strlen( St+ls )+2,
            t=Tpoint.y-2, b=t+Tv.Th+1;
        Tv_rect_UpXOR( l,t,r,b );
        do ans=Tv_getc(); while( ans>=_MouseMoved && ans<=_MouseReleased );
        Tv_rect_UpXOR( l,t,r,b );
      }
      if( ans==_Tab )ans=Tv_getk()&SHIFT?West:East;
      if( ans==_Enter ){;} else
      if( ans>_Enter  ){ if( (kw=ans)==_Esc )break;
                         if( kw>' ' && kw<0xF0 )kw=' '; ans=_Enter; } else
      if( ans==North_East ){ k=+1; kw=0;    } else
      if( ans==South_East ){ k=-1; kw=Nm-1; } else
      if( ans& South_East ){ k=+1; kw++;    } else
      if( ans& North_West ){ k=-1; kw--;    }
      if( Ls->dat!=NULL )
      if( strcmp( St+IO,St+BF ) )
      { if( Ls->S=='°' || Ls->S==0xF8 )AtoD( St+IO,*(double*)Ls->dat ); else
        { static char f[4]="%  "; int k=1;                   // С учетом %lx
          if( Ls->S&0x80 ){ ++k; f[1]='l'; } else f[2]=0;
              f[k]=Ls->S&~0x80;
          if( f[k]=='s' )strcpy( (char*)Ls->dat,St+IO );
                else     sscanf( St+IO,f,Ls->dat );
      } } Message();
    } else kw+=k;
  } while( ans!=_Enter ); Active=TRUE; retC return kw;
}
void Display::Back(){ if( Active )Tback(); Active=false; }
//
//      Tmenu   - Автономная процедура диалога с терминалом  /90.10.03/
//
int Tmenu( Mlist *m, int n, int x, int y, int ans )
{
  Display T( m,n,x,y ); return T.Answer( ans );
}
int Tmenu( void Hl(int), Mlist *m, int n, int x, int y, int ans )
{
  Display T( Hl,m,n,x,y ); return T.Answer( ans );
}
Display::~Display(){ Back(); }
