//
// Два варианта шрифтов без включения проприетарные стандарты MicroSoft:Windows
//  или обновлённый вариант (независимых) начертаний векторных и растровых букв
#include "Window.h"                 // аппаратное окружение и стандартная среда
                                    // программирования MinGW_Microsoft-Windows
typedef unsigned short fixed;

#define _Design_len 9633           // допустимое количество букв в _Design_font
#include "Sym_Design.c"            //  Borland.chr + DesignCAD.vct Complex font

static struct Design_Letters
{ int W,H,B;             // ширина высота и вертикальное смещение базы шрифта
  fixed *Af;             // массив индексов прямого доступа к контурам буквочек
  Design_Letters()       // конструктор срабатывает здесь один единственный раз
  { char *v,*V=(char*)_Design_font; unsigned i,u; //Break( "~Constructor" );
    Af=(fixed*)calloc( sizeof( fixed ),_Design_len );
    W=*V++; H=*V++; B=*V++; V++;
    for( i=0;; )
    { v=UtC( u,V );                    // c пропуском ширины и высоты буквы
      if( !u )break; i+=v-V;           // c нулевым символом на выход
      if( u<_Design_len )Af[u]=i;      // смещение от начала массива векторов
          u=(v[0]+1)*2; V=v+u; i+=u;   // собственно буквочка пропускается и
  } }
  int Rect( SIZE &F, const char *S )
  { int i,w,l=Ulen( S ); char *V=(char*)S; unsigned u; F.cy=H; F.cx=w=0;
    for( i=0; i<l; i++ )
    { V=UtC( u,V );
      if( u<_Design_len )
      { if( u==' ' || u==160 )w+=W; else
        if( u=='\n' ){ F.cx=max( w,(int)F.cx ); w=0; F.cy+=H; } else
        if( Af[u] )w+=( _Design_font+(Af[u])+4 )[1];
    } } F.cx=max( w,(int)F.cx ); return l;
  }
  void Write( char *S,Real &X,Real &Y,Real bX,Real Thin ) // здесь только UTF-8
  { unsigned u,x; int lw,y; signed char *Bit=(signed char*)_Design_font+4;
    glGetIntegerv( GL_LINE_WIDTH,&lw ); glLineWidth( Thin ); // толщина букв
    do
    { S=UtC( u,S );
      if( !u )break;
      if( u<_Design_len )
      { if( u==' ' || u==160 )X+=W; else
        if( u=='\n' ){ Y-=H,X=bX; } else if( Af[u]>0 )
        { byte *T=(byte*)Bit+Af[u]; int n=*T++,w=*T++;
          glBegin( GL_LINE_STRIP );
          for( int i=0; i<n; i++ )
          { x=2+*(byte*)T++; y=B+*(signed char*)T++;     //! под размер 8+21=29
            if( x&0x80 ){ glEnd(); glBegin( GL_LINE_STRIP ); }     //== H-B=21
            glVertex2d( (x&0x7F)+X,y+Y );
          } glEnd(); X+=w;
    } } } while( *S ); glLineWidth( lw );
  }
} Design;
//
//!    Начальная установка базового шрифта, связанного с текущим окном OpenGL
//     ... или destructor с нулевым аргументом
//
Place& Place::AlfaBit( unsigned char *DispCCCP )      // старый советский растр
{ if( (Bit=DispCCCP )!=NULL ){ Tw=Bit[0]+1;
                               Th=Bit[1]+2; } else Tw=Th=Thin=1; return *this;
}
//!  Векторный шрифт из подборки Borland.chr + DesignCad.vfn
//
Place& Place::AlfaVector( _Real H,_Real T )
{ Real Scale=1.0;
  if( Bit )Bit=NULL; else
  { Scale=Th/Design.H; chX*=Scale; chY=(chY+Design.B)*Scale;
  }
  if( H<=0 || H==Design.H )Th=Design.H,Tw=Design.W;
                else Th=H,Tw=(H*Design.W)/Design.H;
  Thin=T; Scale=Th/Design.H; chX/=Scale; chY=chY/Scale-Design.B;
  return *this;
}
//   Определение размеров изображения текст !!! необходим многострочный вариант
//
SIZE Place::AlfaRect( const char *S )         // на выход подаются строки UTF-8
{ SIZE F;
  if( !Bit )Design.Rect( F,S ); else
  { int w,l; char* V=(char*)S; unsigned u; F.cy=Th,F.cx=w=0;
    for( l=0;; l++ )
    { V=UtC( u,V ); if( !u )break;
      if( u=='\n' ){ F.cx=max( w,(int)F.cx ); w=0; F.cy+=Th; } else w+=Tw;
    } F.cx=max( w,(int)F.cx );
  } return F;
}
//!   Работа с текстами в стиле Microsoft-Windows
//                              растровый шрифт DOS-OEM(866)alt из эпохи CCCP
static
void _OutBitText( const char *str, byte *bit, Real &X,Real &Y,Real bX )
{ int w=bit[0]+1,h=bit[1]+2; char c; // в разборе текста контролируется
   glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT ); // переход на новую строку
   glPixelStorei( GL_UNPACK_SWAP_BYTES,false );     // Step through the string,
   glPixelStorei( GL_UNPACK_LSB_FIRST,false );      //  drawing each character.
   glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );        // A newline will simply
   glPixelStorei( GL_UNPACK_SKIP_ROWS,  0 );        // translate the next
   glPixelStorei( GL_UNPACK_SKIP_PIXELS,0 );        // character's insertion
   glPixelStorei( GL_UNPACK_ALIGNMENT,  1 );      // point back to the start of
   while( ( c = *str++ )!=0 )                     // the line and down one line
    if( c=='\n' ){ glBitmap( 0,0,0,0,-X+bX,-h,NULL ); X=bX; Y-=h; } else
                 { glBitmap( w-1,h-2,         // Bitmap's width and height
                              -1,1,           // The origin in the font glyph
                               w,0,           // смещение до новой позиции
                               bit+2+c*(h-2)  //  + 2 байта и номер Х на высоту
                           ); X+=w; }         // сдвиг позиции и растра символа
   glPopClientAttrib();                       // подготовка к следующей строчке
}
Place& Place::String( const char *T )
{ TextContext St; Real X;                  // исходная строка не изменяется
  if( !Bit  )                              //! Вектор DesingCAD.vct-Borland.chr
  { Real Scale=Real( Th )/Design.H,X=chX;
    RasterSector Sv( pX,pY,Width,Height,Scale );
    if( bX<0 ){ SIZE l; Design.Rect( l,T ); X=-bX-l.cx; } else X=bX;
    Design.Write( (char*)T,chX,chY,X,Thin );
  } else
  { char *str=UtA( T,true );                //! три варианта растра DOS-866-ОЕМ
    RasterSector Sv( pX,pY,Width,Height );  //    раньше был ещё и Windows-1251
    if( bX>=0 )X=bX; else { X=-bX-AlfaRect( T ).cx; }
    glRasterPos2i( chX,chY-Th );
    _OutBitText( str,Bit,chX,chY,X );
  } return *this;
}
Place& Place::String( const char *T, Real x,Real y )
{ if( y>0 )chY=Height-Th*y; else chY=3-Th*(y+1);
  if( x>0 )chX=(x-1)*Tw; else chX=Width+x*Tw;
  if( Bit ){ if( y>0 )chY+=Th; }
      else { Real Scale=Th/Design.H; chX/=Scale,chY=(chY+Th/8)/Scale-Design.B; }
  bX=chX;
  if( x<=0 || y<=0 ){ SIZE l=AlfaRect( T ); // во внутренних размерениях шрифта
    if( x<=0 )chX-=l.cx,bX=-bX;
    if( y<=0 )chY+=l.cy;
  } String( T );
  return *this;
}
//!    Прорисовка плоско/горизонтального текста по предварительному определению
//!         размеров букв и смещением надписи относительно точки в плоскости XY
//
Place& Place::String( Course Dir, const Real *P, const char* T )
{ TextContext St; SIZE sz;
  Real dx,dy; char *str=(char*)T;
   if( Bit )str=UtA( T,true );                // выбор DOS-866 или Windows-1251
   glRasterPos3d( P[0],P[1],P[2] );           // плоская надпись со смещением
   sz=AlfaRect( T );      dx=-sz.cx/2;        // разбор типа записи внутри...
   if( Dir&_East  )dx= 4; dy=-sz.cy/6;        // сдвиги и переносы строк
   if( Dir&_West  )dx=-sz.cx - 4;             // здесь отключены
   if( Dir&_North )dy= sz.cy/5+3;
   if( Dir&_South )dy=-sz.cy + 3;
   if( Bit )
   { glBitmap( 0,0,0,0,dx,dy,NULL ); _OutBitText( str,Bit,dx,dy,1 ); } else
   { static int P[4]={ 0,0,0,0 };              // для выборки новой позиции chX
     Real Scale=(Real)Th/Design.H;
     glGetIntegerv( GL_CURRENT_RASTER_POSITION,P );     //dy-=Design.H;
                      P[0]+=dx*Scale; P[1]+=(dy-Design.B)*Scale;
//   RasterSector Sv( pX,pY,Width,Height,Scale );
//   RasterSector Sv( P[0],P[1],P[0]+1,P[1]+1,Real( Th )/Design.H );
     RasterSector Sv( P[0],P[1],P[0]+Width,P[1]+Height,Scale );
     Design.Write( str,dx=0,dy=0,1,Thin );
   } return *this;
}
#include <stdio.h>   // блок для четырёх вариантов текстовых надписей в формате
#include <stdarg.h>  // с реентерабельной\стековой подстрочкой переменной длины
#define Arg va_list a; va_start( a,fmt ); int l=vsnprintf( 0,0,fmt,a )+2; \
              { char s[l]; vsnprintf( s,l,fmt,a ); va_end( a ); String
#define Str ; } return *this; // просто с "временной" строчкой на всё и про всё

Place& Place::Print( const char *fmt,... ){ Arg( s )Str }
Place& Place::Print(_Real x,_Real y, const char *fmt,... ){ Arg( s,x,y )Str }
Place& Place::Text( Course Dir, const Real *P, const char* fmt, ... )
     { Arg( Dir,P,s )Str }
Place& Place::Text( Course Dir,_Real X,_Real Y,_Real Z, const char* fmt, ... )
     { Arg( Dir,(const Real[]){X,Y,Z},s )Str }

/*  == не п(р)о(д)ходит
    Real _x=X,_y=Y;
    for( int xr=1; xr>=0; xr--  )
    {  if( xr>0 )glEnable( GL_COLOR_LOGIC_OP ),glLogicOp( GL_XOR );
       else     glDisable( GL_COLOR_LOGIC_OP ),glLogicOp( GL_COPY );
       char *S=(char*)Str; X=_x; Y=_y; glLineWidth( Thin + xr*10 ); ... glFlush();
    }
*/
