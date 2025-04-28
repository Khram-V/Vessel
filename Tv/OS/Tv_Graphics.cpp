//
//      Tv_GRAPHics.cpp
//      Графическая инструментальная система         / 90.08.17 - Храмушин В.H.
//           лаб. вычислительной гидромеханики       + 97.01.08 остров Сахалин
//      << Русское географическое общество >>       (с)1986-2011 гг.
//
//nclude <Signal.h>
#include "Tv_Graph.h"
#include "SYM_CCCP.inc"

static char *str=NULL;              // =[1024] пустая статическая строчка
viewporttype Tport,                 // Текстовое окно размерами JX-Tx, JY-Ty
             Tv_port;               // Графическое окно
       point Tpoint={ 0,0 };        // Контрольная точка для текста и графики
                                    //
 static const int JX=3,JY=6;        // Отступы и длина строки для текста
 static Field Fp={ 0,0,1,1 };       // Field of Data boundaries
 static Real _cY, _cX,              // Scaling Coefficients
             _aY, _aX;              // Левый нижний угол текущего изображения
 static field Ap={ 0,0,0,0,0 },     // Graphics Device --//--
              Tt={ 0,0,0,0,0 };     // Text window dimensions
//
//  Tv_place процедура инициализации графического поля
//      AF - логическая область существования функции
//      AT - {}>0  левый нижний угол и размер окна в процентах
//           {}<=0 отступы от нижней(Jy) - верхней(Ly), левой (Jx)
//                и правой (Lx) границ текущего окна в пикселях
#define K Tpoint               //  - контрольная точка для текста и графики

TvMode Tv;                     //
TvMode::TvMode(): Bfont(),     //
        Color( WHITE ),        // Текущий цвет рисунков
        FColor( GREEN ),       // Цвет штриховки
        BkColor( BLACK ),      // Стандартный цвет фона
        Fill( SOLID_FILL ),    // Режим закраски фона
        mX( 639 ), mY( 479 )   // Экстремумы терминала
      { Tw=8; Th=10;           // Ширина и высота одной буквы 8:10, 9:16, 9:18
      }
int Tv_Graphics_Start( int Mode, int width, int height )
{
//  signal( SIGFPE,(void(*)(int))SIG_IGN );        // Блокировка прерываний FPU
    if( !str )str=(char*)calloc( 1024,2 );         // Allocate( 4096 );
    initwindow( width,height,Mode );
    getviewsettings( &Tport );
//  setviewport( Tport.left,Tport.top,Tport.right,Tport.bottom,Tport.clip=0 );
    Tv.mY=getmaxy()-2;
    Tv.mX=getmaxx();
    Tv_place( &Ap );                    //
    Tv.Font( 0,0 );                     // Стандартный растровый шрифт
    Tv.Height( 0 );                     // Подключение его к векторным функциям
    Tt.Ly=( Tv.mY-JY+1 )/Tv.Th;         //
    Tt.Lx=( Tv.mX-JX+1 )/Tv.Tw;
    Set_Palette( 0,256,(RGB_palette)Default_Palettes );
    color( Tv.Color );
    bkcolor( Tv.BkColor );
    pattern( Tv.Fill,Tv.FColor );
  return 1;
}                                       //
void Tv_resize( int width, int height ) // Изменение размеров
{   Ghelp();                            // с полной расчисткой масштабов
    sizewindow( width,height );         //
    getviewsettings( &Tport );
//  setviewport( Tport.left,Tport.top,Tport.right,Tport.bottom,Tport.clip=0 );
    Tv.mY=getmaxy();  Ap.Jx=Ap.Jy=Ap.Lx=Ap.Ly=Ap.wb=0; Tt=Ap;
    Tv.mX=getmaxx();  Fp.Jx=Fp.Jy=0.0; Fp.Lx=Fp.Ly=1.0;
    Tv_place( &Ap );
    Tv.Height( 0 );                color( Tv.Color );
    Tt.Ly=( Tv.mY-JY )/Tv.Th;    bkcolor( Tv.BkColor );
    Tt.Lx=( Tv.mX-JX )/Tv.Tw;    pattern( Tv.Fill,Tv.FColor );
}
static void BND( int& J,int& L, int j,int l,int s )
{  if( j>0 )J=( j*s )/100;   else J=-j;
   if( l>0 )L=( l*s )/100+J; else L=s+l; //-1;
}
void Tv_place( field *AT, Field *AF )
{                               //
 point J,S,L;                   // Full & Activite window dimensions
  getviewsettings( &Tv_port );  //
  if( AT ){ field W=Ap; Ap=AT[0]; AT[0]=W; }
  BND( J.y,L.y, Ap.Jy,Ap.Ly, S.y=Tv_port.bottom-Tv_port.top );
  BND( J.x,L.x, Ap.Jx,Ap.Lx, S.x=Tv_port.right-Tv_port.left );
  Tv_port.left  =J.x;
  Tv_port.top   =max( 0,S.y-L.y );
  Tv_port.bottom=min( int( Tv.mY+1 ),S.y-J.y )-1;
  Tv_port.right =min( int( Tv.mX+1 ),min( S.x,L.x ) )-1;
  if( Ap.wb )
    { Tv_port.left += Ap.wb+1; Tv_port.right -= Ap.wb+1;
      Tv_port.top  += Ap.wb+1; Tv_port.bottom-= Ap.wb+1;
    }
  L.x=Tv_port.right-Tv_port.left-2; J.x=Tv_port.left+1;
  L.y=Tv_port.bottom-Tv_port.top-2; J.y=Tv_port.bottom-1;
  if( AF )
    { Field W=Fp; Fp=*AF; *AF=W; if( AT ){ Tv_bar(); Tv_rect(); }
    } _cY=Fp.Ly/L.y; _aY=Fp.Jy+J.y*_cY;
      _cX=Fp.Lx/L.x; _aX=Fp.Jx-J.x*_cX;
}
//// ================================================================
///  Процедуры, заменяющие базовые функции и убранные из <Graphics.h>
//
HIMG Tv_GetImage( int l, int t, int r, int b )
  { HIMG M=Allocate( imagesize( l,t,r,b ) ); getimage( l,t,r,b,M ); return M; }
void Tv_PutImage( int l,int t,HIMG p,int OP ){ putimage( l,t,p,OP ); }
void Tv_FreeImage( HIMG p ){ Allocate( 0,p ); }

void Set_Palette( int index, int count, const RGB_palette pal )
{ for( int k=0; k<count && index+k<256; k++ )
     setrgbpalette( index+k,pal[k].r,pal[k].g,pal[k].b );
}
void Get_Palette( int index, int count, RGB_palette pal )
{ for( int k=0; k<count && index+k<256; k++ )
     getrgbpalette( index+k,&(pal[k].r),&(pal[k].g),&(pal[k].b) );
}
void Tv_revert()           // Временное обращение цветовой палитры
{                          //  с возможностью рекурсивного
 char *s;                  //   восстановления и подсветки
 int i,j;                  //
 Real m,n,k,ch;
 static int l=0,rec,recursive=0;
 static BGIimage *Img,*Cur;
   if( recursive>1 )return; k=1.0;
   if( !recursive )
   { l=imagesize( 0,0,Tv.mX,Tv.mY );  Cur=(BGIimage*)Allocate( l ),
     getimage( 0,0,Tv.mX,Tv.mY,Cur ); Img=(BGIimage*)Allocate( l );
   } rec = ++recursive;
 Cont:
   memcpy( Img,Cur,l );
         s=Img->bits;
   for( i=0; i<int(Img->height)*(Img->width)*4; i+=4,s++ )
   { n=m=Real( unsigned( s[0] )         // b
             + unsigned( s[1] )         // g
             + unsigned( s[2] ) )/3.0;  // r
     if( k!=1.0 )m=pow( m/255.0,k )*255.0;
     if( recursive==1 )n=255.0-m;
     for( j=0; j<3; j++,s++ )
     { ch=unsigned( s[0] );
       if( n>m )ch=255-(255-ch)*(255-n)/(255-m); else
       if( n<m )ch*=n/m;
       s[0]=(char)ch;
   } }
   putimage( 0,0,Img,COPY_PUT );
   for( ;; )
   if( !(j=Tv_getw()) )
   { if( rec==recursive )Sleep( 250 );
     else { rec=recursive; goto Cont; }
   } else
   if( j==North ){ k*=1.1; goto Cont; } else
   if( j==South ){ k/=1.1; goto Cont; } else break;
   if( !--recursive )
   { Allocate( 0,Img ); putimage( 0,0,Cur,COPY_PUT );
     Allocate( 0,Cur );
   }                            //
// else                         // Смена фона и цвета первых 16 цветов
// { static bool First=true;    //
//   static int i,Inv[16]={ 15, 9,10,11,12,13,14, 8,7, 1,2,3,4,5,6, 0 };
//    for( i=0; i<16; i++ )
//    Set_Palette( i,1,(const RGB_palette)Default_Palette+(First?Inv[i]:i) );
//    First=!First;
// }
}
#define str_FMT( l ) \
  { va_list V; va_start( V,_f ); l vsprintf( str,_f,V ); va_end( V ); }
//
//      Текстовая строчка, получаемая стандартным форматированием
//
char* _Fmt( const char* _f,... ){ str_FMT( ; ) return str; }
//
//      Tv_x,y: пересчет размерностей данных в координаты изображения
//      Tv_X,Y: пересчет координаты изображения в размерность данных
//
#define _ix_( _X ) int( (_X-_aX)/_cX )
#define _iy_( _Y ) int( (_aY-_Y)/_cY )
#define _iX_( _x )    ( _aX + _x*_cX )
#define _iY_( _y )    ( _aY - _y*_cY )

 int Tv_x( Real X ){ return _ix_( X ); }
 int Tv_y( Real Y ){ return _iy_( Y ); }
Real Tv_X( int  x ){ return _iX_( x ); }
Real Tv_Y( int  y ){ return _iY_( y ); }

point::operator Point(){ Point P; P.x=_iX_( x ),P.y=_iY_( y ); return P; }
Point::operator point(){ point p; p.x=_ix_( x ),p.y=_iy_( y ); return p; }
//
//     Отсечение внешней области изображения
//      ( ?? без разрешения повторного масштабирования )
//
void TvClip()
{ field _ap={ 0,0,0,0,0 };
  setviewport( Tv_port.left,Tv_port.top,Tv_port.right,Tv_port.bottom,true );
  Tv_place( &_ap );
  Tv_place( 0,&Fp );  Ap=_ap;     // области внутри картинки
}
void TvUnClip()                   // Восстановление полноэкранного изображения
{ setviewport( 0,0,Tv.mX,Tv.mY,0 ); Tv_place( &Ap ); Tv_place( 0,&Fp );
}
//     Store/Restore: Базисные процедуры активизации(!) малого окна
//
static struct WinList{ short Ws,y,x; HIMG Img; field Ab; }
      *Wl=0,                            //
       WB={ 0,0,0,NULL,{ 0,0,0,0,0 } }; // Окно и стековый список
static int Wn=0;                        // Количество записей в списке
                                        //
HIMG Tv_store( int left,int top,int right,int bottom, const field *At )
{ if( WB.Ws )
  { Wl=(WinList*)Allocate( (Wn+1)*sizeof(WinList),Wl ); Wl[Wn]=WB; ++Wn;
  } WB.Ab=At?*At:Ap;
    WB.Ws=1;
    WB.Img=Tv_GetImage( left,top,right,bottom );
    WB.y=top;
    WB.x=left;
//  if( At ){ bar(left,top,right,bottom); rectangle(left,top,right,bottom); }
    return WB.Img;
}
field Tv_restore()
{ field Ab=WB.Ab;
  if( WB.Ws ){ if( WB.Img ){ Tv_PutImage( WB.x,WB.y,WB.Img );
                             Tv_FreeImage( WB.Img ); } WB.Ws=0; }
  if( Wn>0 )
  { --Wn; WB=Wl[Wn]; Wl=(WinList*)Allocate( Wn*sizeof(WinList),Wl );
  } return Ab;
}
//   Tv_window - аналог Tv_place с сохранениеm изображения
//
void Tv_window( field *AT, Field *AF )
{ if( AT )Tv_place( AT ); Tv_store( Tv_port.left  - Ap.wb-2,
                                    Tv_port.top   - Ap.wb-2,
                                    Tv_port.right + Ap.wb+2,
                                    Tv_port.bottom+ Ap.wb+2 );
  if( AF ){ Tv_place( 0,AF ); if( AT )Tv_bar(),Tv_rect(); }
}
//void Tv_back(){ Tv_place( &(Tv_restore()) ); }
//
//  Tv_picture: установка текущих цветов и расцветок
//
int bkcolor( int bc )                            // Установка цвета фона
           { int k=Tv.BkColor; setbkcolor( Tv.BkColor=bc ); return k; }
int   color( int c ){ int k=Tv.Color; setcolor( Tv.Color=c ); return k; }
void pattern( int p,int c ){ setfillstyle( Tv.Fill=p,Tv.FColor=c ); }
void pattern( int c ){ pattern( Tv.Fill,c ); bkcolor( c ); }
void Tv_rect_x( int l, int t, int r, int b )            // rectangle( l,t,r,b )
{ int x,y; setwritemode( XOR_PUT );
  if(l>r)x=l,l=r,r=x; x=Tv_port.right-Tv_port.left; if( l<Tv_port.left   )l+=x;
                                                    if( r>Tv_port.right  )r-=x;
  if(t>b)y=b,b=t,t=y; y=Tv_port.bottom-Tv_port.top; if( t<Tv_port.top    )t+=y;
                                                    if( b>Tv_port.bottom )b-=y;
  if( l<=r )line( l-1,t,r+1,t ),line( l-1,b,r+1,b );
       else line( l,t,Tv_port.right,t ),line( Tv_port.left,t,r,t ),
            line( l,b,Tv_port.right,b ),line( Tv_port.left,b,r,b );
  if( t<=b )line( l,t-1,l,b+1 ),line( r,t-1,r,b+1 );
       else line( l,t,l,Tv_port.bottom ),line( l,b,l,Tv_port.top ),
            line( r,t,r,Tv_port.bottom ),line( r,b,r,Tv_port.top );
  setwritemode( COPY_PUT );
}
void Tv_rect_up( int l, int t, int r, int b, int w )
{    setcolor( BLACK );    rectangle( l,t,r+w+1,b+w+1 );
     setcolor( WHITE );    rectangle( l-w-1,t-w-1,r,b ); setcolor( LIGHTGRAY );
  for( int i=1; i<=w; i++ )rectangle( l-i,t-i,r+i,b+i ); setcolor( Tv.Color );
}
void Tv_rect_UpXOR( int l, int t, int r, int b )
{    setwritemode( XOR_PUT );
     setcolor( Tv.BkColor^WHITE ); line( l,b,l,t );   line( l+1,t,r,t );
     setcolor( Tv.BkColor^BLACK ); line( r,t+1,r,b ); line( r-1,b,l+1,b );
     setwritemode( COPY_PUT );
     setcolor( Tv.Color );
}
void Tv_rect()
{ if( !Ap.wb )rectangle( Tv_port.left,Tv_port.top,Tv_port.right,Tv_port.bottom );
  else Tv_rect_up( Tv_port.left,Tv_port.top,Tv_port.right,Tv_port.bottom,Ap.wb );
}
void Tv_bar( int left, int top, int right, int bottom )
{ int c = Tv.FColor,f=Tv.Fill;
    pattern( SOLID_FILL,Tv.BkColor ); bar( left,top,right,bottom );
    pattern( f,c );
}
void Tv_bar(){ Tv_bar( Tv_port.left,Tv_port.top,Tv_port.right,Tv_port.bottom ); }
void clear(){ setbkcolor( Tv.BkColor ); clearviewport(); }
//
//       line  : проведение линии
//       moveto: прием текущей отсчетной точки
//       lineto: соединение с текущей отсчетной точкой
//       needle: изображение стрелочки
//
void needle( int X,int Y, int x,int y )
{ int ly,lx;
  moveto( X,Y ); Real l=hypot( lx=(x-X+2)/3,ly=(y-Y+2)/3 );
  if( l>Tv.Th )lx=int( lx*(Tv.Th/l) ),
               ly=int( ly*(Tv.Th/l) );
  lineto( X=x-lx,Y=y-ly ); lx/=6; ly/=6;
  lineto( X-ly,  Y+lx   ); lineto( x,y );
  lineto( X+ly,  Y-lx   ); lineto( K.x=X,K.y=Y );
}
void moveto( Real X,Real Y ){ moveto( K.x=_ix_(X),K.y=_iy_(Y) ); }
void lineto( Real X,Real Y ){ lineto( K.x=_ix_(X),K.y=_iy_(Y) ); }
void needle( Real X,Real Y,Real x,Real y )
           { needle( _ix_(X),_iy_(Y),_ix_(x),_iy_(y) ); }
void line  ( Real X,Real Y,Real x,Real y )
           { line( _ix_(X),_iy_(Y),_ix_(x),_iy_(y) ); }
//
//      Tv_fc: контроль координат изображения
//
bool Tv_fc( Point *P )
{ bool cy=false,cx=false;
  Real Wy=Fp.Jy+Fp.Ly,
       Wx=Fp.Jx+Fp.Lx;
  if( P->y>Wy )P->y=Wy; else if( P->y<Fp.Jy )P->y=Fp.Jy; else cy=true;
  if( P->x>Wx )P->x=Wx; else if( P->x<Fp.Jx )P->x=Fp.Jx; else cx=true;
  return cy&cx;
}
//      Tv_cross: отметка точки крестиком
//        (Size)< 0 нанести отметку без сохранения
//              = 0 стереть переносимую отметку
//              > 0 перенести отметку в новое место
//
void Tv_cross( Point P, int Size )
{   int j;                      //
 static int      oS=0;          // Максимальный размер Size=16
 static unsigned Save[66],*S;   //                     66=Sx*4+2
 static point    p={0,0};       //
  if( oS>0 )
  { S=Save; for( j=p.y-oS; j<=p.y+oS; j++ )putpixel( p.x,j,*S++ );
            for( j=p.x-oS; j<=p.x+oS; j++ )putpixel( j,p.y,*S++ );
  }   oS=Size; K=p=P;
  if( oS>0 )
  { if( oS>16 )oS=16;
    S=Save; for( j=p.y-oS; j<=p.y+oS; j++ )*S++=getpixel( p.x,j );
            for( j=p.x-oS; j<=p.x+oS; j++ )*S++=getpixel( j,p.y );
  }
  if( oS )line( p.x,p.y+oS,p.x,p.y-oS ),
          line( p.x-oS,p.y,p.x+oS,p.y );
}
static void __cross( const point &p, int s )  // c желтой подложкой
{ int mode=setwritemode( XOR_PUT );           //
  setlinestyle( SOLID_LINE,0,THICK_WIDTH+2 ),setcolor( Tv.BkColor^YELLOW );
  line( p.x,p.y+s,p.x,p.y-s ),line( p.x-s,p.y,p.x+s,p.y ),circle( p.x,p.y,s/2 );
  setlinestyle( SOLID_LINE,0,NORM_WIDTH ),setcolor( Tv.BkColor^Tv.Color );
  line( p.x,p.y+s,p.x,p.y-s ),line( p.x-s,p.y,p.x+s,p.y ),circle( p.x,p.y,s/2 );
  setwritemode( mode );
}
//      Tv_Pause: специальное обращение для отслеживания маркера
//         Size - параметр Tv_cross
//         Step - шаг движения управляемого с клавиатуры
//
Course Tv_pause( Point *P, int Size, Real Step )
{  Course A; //int c=WHITE;
 Repeat:
   if( Size>0 )
   { Tv_fc( P ); point p=*P; __cross( p,Size );
                A=Tv_getc(); __cross( p,Size );
   } else       A=Tv_getc();
   if( A==_MouseReleased )
   { point C; Tv_GetCursor( C.x,C.y ); *P=C; A=_Center; } else
   { Point St;
     if( Step!=0.0 )St=(Point){ Step,Step }; else
     { static unsigned mS=1;
       if( A>='0'&&A<='9' ){ mS=A<'2'?1:mS*((unsigned)A-'0'); goto Repeat; }
         St.x=_cX*mS;
         St.y=_cY*mS;
     }
     if( A<_Enter )
     { if( A & North )P->y+=St.y; if( A & West )P->x-=St.x;
       if( A & South )P->y-=St.y; if( A & East )P->x+=St.x;
       if( Size>0 )Tv_fc( P );
   } } if( Size>0 )setcolor( Tv.Color ); return A;
}
void Tv_close(){ closegraph(); }
////
///  <<  Второй блок заменяет операции вывода растрового текста >>
//
static int Tv_PutText( int x, int y, int _mx, const char *textstring )
{ if( y>=0 && --y+Tv.Th<=Tv.mY+1 )
  { unsigned char *_sf; int i,j,k,h;
    unsigned short bits[16];
    if( Tv.Th<14 )_sf=(unsigned char*)_8x08,h=8;  else
//  if( Tv.Th<16 )_sf=(char*)_8x14,h=14; else
                  _sf=(unsigned char*)_8x16,h=16;
    for( k=0; textstring[k]; k++,x+=Tv.Tw )
    { if( x+8>_mx )break;
      if( x>=0 )
      { for( i=0,j=h*textstring[k]+h-1; i<h; i++ )bits[i]=~(_sf[j-i]);
        putbits( x,y+1,9,h,bits,COPY_PUT );
    } } return k;
  }     return 0;
}
////
///   Ghelp - вывод подсказки с восстановлением графического поля
//
void Ghelp( const char* _f,... )
{
 static void *Hw=NULL;
 static int y,Mw=0,m=0;
 int l=0;
  if( m )putimage( 0,y,Hw,COPY_PUT ),m=0;
  if( _f )if( *_f )
  { str_FMT( l= )
    if( l>0 )
    { setcolor( Tv.BkColor==BLACK?LIGHTCYAN:CYAN ); str[l++]=' ';
                                                    str[l]=0;
      if( (l*=Tv.Tw)>Tv.mX )l=Tv.mX;  m=imagesize( 0,y=Tv.mY-Tv.Th+2,l,Tv.mY );
      if( m>Mw)Hw=Allocate( Mw=m+1024,Hw );         getimage( 0,y,l,Tv.mY,Hw );
      Tv_PutText( 0,y,l,str );                      setcolor( Tv.Color );
  } }
}
void Ghelp(){ Ghelp( 0 ); }
void Thelp( const char* _f, ... )
{ if( _f ){ str_FMT( ; ); Ghelp( W2D( str ) ); } else Ghelp();
}
//
/// Text window: Пакет для текстового диалога в графическом режиме
//
//     Twindow: выделение текстового окна на графическом поле
//     Tt.Jy,Jx > 0 - позиционирование от левого-верхнего угла
//             <= 0 - относительно правого нижнего угла изображения
//     Tt.Ly,Lx     - размер текстового окна в символах
//
static void InitWindow()
{
 int wb=Tt.wb+2,
     ly=Tt.Ly*Tv.Th+JY+2 + wb,
     lx=Tt.Lx*Tv.Tw+JX+1 + wb;
  if( Tt.Jy<=0 )
  { Tport.top=Tv.mY+Tt.Jy*Tv.Th-ly;  if( Tport.top<wb )Tport.top=wb; }else
  { Tport.top=(Tt.Jy-1)*Tv.Th+wb; if( Tport.top+ly>Tv.mY )Tport.top=Tv.mY-ly;
  } Tport.bottom=Tport.top+Tt.Ly*Tv.Th+JY+2;
  if( Tt.Jx<=0 )
  { Tport.left=Tv.mX+Tt.Jx*Tv.Tw-lx;  if( Tport.left<wb )Tport.left=wb; }else
  { Tport.left=(Tt.Jx-1)*Tv.Tw+wb; if( Tport.left+lx>Tv.mX )Tport.left=Tv.mX-lx;
  } Tport.right =Tport.left+Tt.Lx*Tv.Tw+JX+1;
}
void Twindow( int jx, int jy, int lx, int ly, int wb )
{
 int w = wb+2,
     Y = ( Tv.mY-JY-2-w*2 )/Tv.Th, // Допустимые размеры текстового поля
     X = ( Tv.mX-JX-1-w*2 )/Tv.Tw; //       с учетом рамки
  if( ly>Y )ly=Y;                  // Проверка допустимого размера окна
  if( lx>X )lx=X;                  //
 field Wt=Tt;                      // Сохранение предыдущего текстового окна
  Tt=(field){ jx,jy,lx,ly,wb };
  InitWindow();
  Tv_store( Tport.left-w,Tport.top-w,Tport.right+w,Tport.bottom+w,&Wt );
  Tv_bar( Tport.left,Tport.top,Tport.right,Tport.bottom );
  if( wb || Tv.Color!=Tv.BkColor )
  Tv_rect_up( Tport.left,Tport.top,Tport.right,Tport.bottom,Tt.wb );
  Tgo( 1,1 );
}
void Tback(){ Tt=Tv_restore(); InitWindow(); }
//
//   Tgo : Позиционирование курсора
//               относительно верхнего левого угла терминала
//       x,y>0 - Обычное позиционирование внутри текстового окна
//       x <=0 - Строка приводится к левому краю окна изображения
//       y <=0 - Ноль - нижняя строка, далее отсчет строк снизу-вверх
//
void Tgo( int x, int y )
{ if( x<=0  )K.x=JX-x*Tv.Tw;   else K.x=Tport.left+JX+Tv.Tw*(x-1);
  if( --y<0 )K.y=Tv.mY+y*Tv.Th+2; else K.y=Tport.top +JY+Tv.Th*y;
  moveto( K.x,K.y );
}
void Tis( int &x, int &y ){ x=( K.x-JX-Tport.left )/Tv.Tw+1;
                            y=( K.y-JY-Tport.top  )/Tv.Th+1; }
void Tln()
{    K.x=Tport.left+JX;                 // Левая точка в начало строки
     K.y+=Tv.Th;                        // Перевести одну строку вниз
 if( K.y+Tv.Th>=Tport.bottom )K.y=Tport.top+JY;
     moveto( K.x,K.y );
}
//
//  Tv_print - печатание текста около логической ячейки
//
static void Text_Print(){ K.x += Tv_PutText( K.x,K.y,Tport.right,str )*Tv.Tw; }
//
//  Tprint: печатание текста в графическом режиме
//
int Tprintf( const char* _f, ... )
{ int cnt; str_FMT( cnt= )
  if( cnt>0 )
  { bool crl = str[cnt-1]=='\n';
    if( crl )str[--cnt]=0; Text_Print();
    if( crl )
    { if( K.x<Tport.right-1 )Tv_bar( K.x,K.y,Tport.right-1,K.y+Tv.Th-2 ); Tln();
  } } return cnt;               //
}                               // Вывод символа с сохранением рабочей строки
void Tputc( char ch )           //
{ int s=*(int*)str; *(int*)str=ch; Text_Print(); *(int*)str=s;
}
//      Tgetc,s	: запрос команды от терминала
//        S     - Изменяемая строка
//        len   - Длина окна запроса (len<0 - сохранение поля)
//        pos   - Начальная позиция курсора
//      return( cr ) - <return> - обычный выход из командного режима
//            ( 14 ) - <insert> - выход по команде включения точки
//            ( 15 ) - <delete> - выход по команде исключения точки
//            ( 27 ) - <escape> - выход с отменой команды
//       ( 0xF1..a ) - <function>
//
static void _i()
{ Tv.BkColor^=WHITE; Tv_rect_UpXOR( K.x-1,K.y,K.x+Tv.Tw-1,K.y+Tv.Th-2 );
  Tv.BkColor^=WHITE;
}
Course Tgetc(){ Course ans;
  _i(); do ans=Tv_getc(); while( ans>=_MouseMoved && ans<=_MouseReleased );
  _i(); return ans;
}
static void lins( int VWin, int cnt, byte First=FALSE )
{ int l=K.x-2,t=K.y-1,r=l+cnt+2,b=t+Tv.Th;
  if( VWin )
    { if( !First )Tv_restore();
            else{ Tv_store( l,t,r,b,&Ap ); Tv_bar( l,t,r,b ); }
    } else        Tv_rect_UpXOR( l,t,r,b );
}
Course Tgets( char* S, int L, int X )
{
 Course ans=_Esc;
 static int OldP=0;
 int C=true,x=K.x,wp=L<0,cnt; L=wp?-L:L; lins( wp,cnt=L*Tv.Tw,true );
  if( X<0  )X=OldP;
  if( X>=L )X=L-1;
  do
  { switch( ans )
    { case _Esc:
      if( !C )goto Leave; else
      { strncpy ( str,S,L );
        C=strlen( str ); while( C<L )str[C++]=' '; str[L]=0; K.x=x;
        Text_Print();
      } C=false; break;
      case East : if( X>=L-1 )goto Leave; ++X; break;
      case West : if( X<=0   )goto Leave; --X; break;
      case _BkSp: if( X>0    )--X;
      case _Del : K.x=x; scpy(str+X,str+X+1,L-X); Text_Print(); C=true; break;
      case _Ins :
      if( X<L-1 )
      { K.x=x;
        for( C=L-1;C>X;C-- )str[C]=str[C-1]; str[C]=' '; Text_Print(); C=true;
      } break;
      default:
      if( ans>=' ' && ans<0xF1 )
      { Tputc( str[X]=char( ans ) ); if( ans>=' ' )
                                     if( ++X>=L )X=L-1; C=true;
      } else goto Leave;
    } K.x=x+X*Tv.Tw; ans=Tgetc();
  } while( ans!=_Enter );
Leave:     K.x = x;
  if( C && ans!=_Esc )
  { while( --L>=0 && str[L]<=' ' )str[L]=0; strcpy( S,str );
  }
  lins( wp,cnt ); OldP=X; return ans;
}                                     //
Course Wgets( char* S, int L, int X ) // С перкодировкой из ДОС
{ Course Ans;                         // в Виндовс и обратно
  CharToOemBuff( S,S,strcut( S )+1 ); Ans=Tgets( S,L,X );
  OemToCharBuff( S,S,strcut( S )+1 ); return Ans;
}                                     //
//#include "Tv_GraphTxt.cpp"          // снятие ограничений на ст.графику
//
//!    Управление представлением и размером текстовых строк
//
struct HDR                     // Начальные 128 байт префикса удалены
{ unsigned short nchrs;        // number of characters in file
  unsigned char  mystery;      // Currently Undefined
  unsigned char  first;        // first character in file
    signed short cdefs;        // offset to char definitions
  unsigned char  scan_flag;    // True if set is scanable
    signed char  org_to_cap;   // Height from origin to top of capitol
    signed char  org_to_base;  // Height from origin to baseline
    signed char  org_to_dec;   // Height from origin to bot of decender
  unsigned char  fntname[5];   // Four character name of font
  unsigned char  sig;          // SIGNATURE byte
};                             //  =16
#define Header     (*( (HDR*)( CHR ) ) )
#define Offset     ( (short*)( CHR+sizeof(HDR) ) )
#define Char_Width (  (char*)( Offset+Header.nchrs ) )

Bfont::Bfont( void *_font, const int __H ) // Регистрируемый шрифт
     : Basis_Area()                        //  и его размер
{   H=Th=Tv.Th;
      Tw=Tv.Tw;         // При установке растрового шрифта
      Font( _font );    // 0: 10x8 <= 640x480; 18x9 >= 800x600
      Height( __H );    // >0: приводится к ближайшему из нулевых
}
void Bfont::Font( void *_f, const int _h )
{ if( (CHR=(char*)_f)!=NULL )Th=Header.org_to_cap-Header.org_to_dec+2,
                              b=Header.org_to_base;
                       else  Th=Tv.Th,b=0;
  Height( _h );
}
int Bfont::Height( const int _H ) // __H: -1 - не изменять размер
{ if( _H>=0 )                    //        0 - взять h или Tv.Th
  { if( !CHR )                  //        >1 - новый размер
    { if( _H>0 )H=_H; else
      if( Tv.mX<=640 )H=10; else H=18;
      if( H<16 ){ Th=10; Tw=8; } else { Th=18; Tw=9; } H=Th;
    } else if( _H>0 )H=_H; else H=Tv.Th;
  } return H;
}
int Bfont::StrLen( const char *_s )
{ if( !CHR )return strlen( _s )*Tw;
  int l,i; for( l=i=0; _s[i]; i++ )l+=( H*Char_Width[_s[i]-Header.first] )/Th; return l;
}
//
//!  <<  Процедуры для прямого доступа к векторным шрифтам  >>
//
struct xy{ char x,y; };

int Bfont::__OutText( char *s ) // Q  R
{ Point P; point p;             // |
  if( CHR )                     // p--q
  { short ch,i,l;
    for( i=l=0; (ch=s[i])!=0; l+=Char_Width[s[i++]-Header.first] )
    { if( ch<Header.first || ch>Header.first+Header.nchrs )ch=' ';
      for( short CurCom=0;;CurCom++ )
      { xy d=((xy*)( Char_Width+Header.nchrs
                   + Offset[ch-Header.first] ))[CurCom];
        if( d.x&0x80 )
        { P.x = ((signed char)(d.x<<1))/2 + l;
          P.y = ((signed char)(d.y<<1))/2;
          p=Goext( P );
          if( d.y&0x80 )lineto( p.x,p.y ),putpixel( p.x,p.y,Tv.Color );
                  else  moveto( p.x,p.y );
        } else if( !(d.y&0x80) )break;
      }
    } return i;                 // !! Здесь необходимо сделать центровку
  } else
  { point p; P.x=0,P.y=Th; p=Goext( P ); return Text( p.x,p.y,s ); }
}
//
// Расчет размера стандартного текстового прямоугольника
//
static Field __F( const char *_s, const char *CHR )
{ Field _F={ 0,0,0,Header.org_to_cap-Header.org_to_dec }; // { 0,b,0,h-b }
  for( int i=0; _s[i]; i++ )_F.Lx += Char_Width[_s[i]-Header.first];
  return _F;
}
//
//      Печать текста внутри произвольного четырехугольника          3 - 4
//                                                                   1 - 2
int Bfont::OutText( Point p1,Point p2, char* _s )
         { Area( __F( _s,CHR ),p1,p2 ); return __OutText( _s ); }
int Bfont::OutText( Point p1,Point p2,Point p3, char* _s )
         { Area( __F( _s,CHR ),p1,p2,p3 ); return __OutText( _s ); }
int Bfont::OutText( Point p1,Point p2,Point p3,Point p4, char* _s )
         { Area( __F( _s,CHR ),p1,p2,p3,p4 ); return __OutText( _s ); }
//
//     Обычный горизонтальный текст
//
int Bfont::Text( int _x, int _y, const char *s )
{ int i;
  if( CHR )
  { int x,y; //,cnt;
    unsigned char ch;       _y+=( H*Header.org_to_cap )/Th;
    for( i=0; (ch=s[i])!=0; _x+=( H*Char_Width[s[i++]-Header.first]+1 )/Th )
    { if( ch<Header.first || ch>Header.first+Header.nchrs )ch=' ';
      xy *d=(xy*)( Char_Width+Header.nchrs+Offset[ch-Header.first] );
      for( ;; d++ )
      { x = _x+( H*(signed char)((d->x)<<1) )/( Th*2 );
        y = _y-( H*(signed char)((d->y)<<1) )/( Th*2 );
        if( d->x&0x80 )
        { if( d->y&0x80 )lineto( x,y ),putpixel( x,y,Tv.Color );
                    else moveto( x,y ); //,putpixel( x,y,Tv.Color );
        } else
        if( !(d->y&0x80) )break;
    } }
  } else
  { short _h=Tv.Th; Tv.Th=Th; i=Tv_PutText( _x,_y,Tv.mX,s ); Tv.Th=_h;
  } return i;
}
//      Прорисовка горизонтального текста в соответствии с
//       определением размеров букв и ориентацией строки
//
int Bfont::Text( Course _Dir, int x,int y, const char* _s )
{ int L;   y+=2;
  if( CHR )L=StrLen( _s );
      else L=strlen( _s )*Tw;
  if( _Dir&North )y-=H; else if( !( _Dir&South ) )y-=H/2;
  if( _Dir&West  )x-=L; else if( !( _Dir&East )  )x-=L/2;
  return Text( x,y,_s );
}
int Bfont::Text( Course d,
                 Real X,Real Y,const char* _s ){ return Text(d,_ix_(X),_iy_(Y),_s ); }
int Bfont::Text( Real X,Real Y,const char* _s ){ return Text(  _ix_(X),_iy_(Y),_s ); }

int Bfont::Text( Course d,point p,const char* _s){ return Text(d,p.x,p.y,_s ); }
int Bfont::Text(          point p,const char* _s){ return Text(  p.x,p.y,_s ); }
