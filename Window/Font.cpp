//
//     три вариант шрифтов, включая проприетарные стандарты MicroSoft:Windows
//
#include "Font.h"  // обновлённый вариант (независимых) шрифтов

class inFont       // присоединенный шрифт со ссылкой на TrueType из ex-Windows
{ byte *Bit;       // назначение растрового или векторного шрифта из эпохи CCCP
  int TB;          // +++ из некой особенностей разметки векторных шрифтов
  HFONT hF;        // --- шрифт сохраняется для внутрисистемной метрики
friend class Place;
};
struct WinFont{ HDC oD; HFONT oF;                      // ... по микрософтовски
       WinFont( HDC D, HFONT F ): oD( D ){ oF=(HFONT)SelectObject( D,F ); }
      ~WinFont(){ if( oF )SelectObject( oD,oF ); }
};
const GLsizei ListsGroup=256; // вектор группы TrueType шрифтов
//
//!    Начальная установка базового шрифта, связанного с текущим окном OpenGL
//     ... или destructor с нулевым аргументом
//
Place& Place::AlfaBit( unsigned char *DispCCCP )     /// старый советский растр
{ if( !DispCCCP )              // return Alfabet() -- сходу не линкуется => TTF
  { if( iFn )                  //  ... пусть будет destructor для Windows - TTF
    { inFont &Fnt=*(inFont*)iFn;
      if( Fnt.TB )glDeleteLists( Fnt.TB,ListsGroup ),Fnt.TB=0;
      if( Fnt.hF )DeleteObject( Fnt.hF ),Fnt.hF=0;
      Fnt.Bit=NULL; free( iFn ); iFn=0;  // с возвратом к шрифтам базового окна
    } Tw=Th=1;
  } else
  if( !iFn )if( (iFn=calloc( sizeof(inFont),1 ))!=0 ) // и свой локальный адрес
  { inFont &Fnt=*(inFont*)iFn;
    Fnt.Bit=DispCCCP; Tw=(Fnt.Bit)[0]+1,Th=(Fnt.Bit)[1]+2;
  } return *this;
}
//!  Windows TrueType шрифт Courier New-18 устанавливается по умолчанию
//
Place& Place::Alfabet( int h, const char *fn, int weight, bool italic )
{ AlfaBit();                         //? wglMakeCurrent( Site->hDC,Site->hRC );
  if( !iFn )
  if( (iFn=calloc( sizeof(inFont),1 ) )!=0 )  // снова ставится локальный адрес
  { LOGFONT logfont={};                       // Setup Font characteristics ≡ 0
   inFont &Fnt=*(inFont*)iFn;
    logfont.lfHeight       = h<6?Site->ScreenHeight/48:h; // высота шрифта = 1?
//  logfont.lfWidth        = 0;
    logfont.lfEscapement   = GM_COMPATIBLE;               //   GM_ADVANCED - 0
//  logfont.lfOrientation  = 0;
    logfont.lfWeight       = weight;// FW_DONTCARE+THIN,ULTRALIGHT,LIGHT,NORMAL
    logfont.lfItalic       = italic; //    MEDIUM,SEMIBOLD,BOLD,EXTRABOLD,HEAVY
//  logfont.lfUnderline    = false;
//  logfont.lfStrikeOut    = false;
    logfont.lfCharSet      = RUSSIAN_CHARSET;   // DEFAULT_CHARSET ANSI_CHARSET
    logfont.lfOutPrecision = OUT_STRING_PRECIS;        // OUT_TT_PRECIS
    logfont.lfClipPrecision= CLIP_DEFAULT_PRECIS;      // OUT_STROKE_PRECIS
    logfont.lfQuality      = DEFAULT_QUALITY;          // PROOF_QUALITY
    logfont.lfPitchAndFamily=FF_MODERN|FIXED_PITCH;    // FF_ROMAN DEFAULT_PITCH
    strcpy( logfont.lfFaceName,fn?fn:"Courier New");   // имя шрифта 32 байта
    { WinFont O( Site->hDC,Fnt.hF=CreateFontIndirect( &logfont ) );
      Fnt.TB=glGenLists( ListsGroup );
      if( !wglUseFontBitmaps( Site->hDC,0,ListsGroup,Fnt.TB ) ) //! странная ??
           wglUseFontBitmaps( Site->hDC,0,ListsGroup,Fnt.TB );  //! проблема ??
//    { glDeleteLists( Fnt->TB,ListsGroup==ListsGroup );
//      Fnt->TB=0; FontWidth=FontHeight=1 } else
      { TEXTMETRIC T; GetTextMetrics( Site->hDC,&T );
        Tw=T.tmAveCharWidth;    // (AlfaRect( "_Ё°y" ).cx+3)/4;
        Th=logfont.lfHeight;    // T.tmAscent; // tmHeight+tmExternalLeading;
      }
    }
  } return *this;
}
//
//   ...далее существенен выбор главной или наложенной графической площадки
//
#define AF (*(inFont*)(iFn?iFn:Site->iFn))
//
//   Определение размеров изображения текст !!! необходим многострочный вариант
//
SIZE Place::AlfaRect( const char *s, bool ANSI )  // предварительно формируется
{ SIZE F={ ANSI?(long)strlen( s ):Ulen( s ),0 };  // в буфере или ANSI-строчка
  if( AF.Bit )F.cx*=Tw,F.cy=Th; else
  { glListBase( AF.TB ); char *str=(char*)s;
    WinFont O( Site->hDC,AF.hF );
    if( !ANSI )str=UlA( strdup( s ),false );     // сброс UTF-8 в ANSI на месте
    GetTextExtentPoint32( Site->hDC,str,F.cx,&F );
    if( !ANSI )free( str );
  } return F;
}
//
//!   Работа с текстами в стиле Microsoft-Windows
//                              растровый шрифт DOS-OEM(866)alt из эпохи CCCP
static
void _OutBitText( const char *str, byte *bit, int &X,int &Y,int bX )
{ int x=X,w=bit[0]+1,h=bit[1]+2; char c;     // в разборе текста контролируется
   glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT ); // переход на новую строку
   glPixelStorei( GL_UNPACK_SWAP_BYTES,false );     // Step through the string,
   glPixelStorei( GL_UNPACK_LSB_FIRST,false );      //  drawing each character.
   glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );        // A newline will simply
   glPixelStorei( GL_UNPACK_SKIP_ROWS,  0 );        // translate the next
   glPixelStorei( GL_UNPACK_SKIP_PIXELS,0 );        // character's insertion
   glPixelStorei( GL_UNPACK_ALIGNMENT,  1 );      // point back to the start of
   while( ( c = *str++ )!=0 )                     // the line and down one line
    if( c=='\n' ){ glBitmap( 0,0,0,0,-X,-h,NULL ); X=0; Y-=h; } else
                 { glBitmap( w-1,h-2,         // Bitmap's width and height
                              -1,1,           // The origin in the font glyph
                               w,0,           // смещение до новой позиции
                               bit+2+c*(h-2)  //  + 2 байта и номер Х на высоту
                           ); X+=w; }         // сдвиг позиции и растра символа
   if( bX<=0 )X=x; glPopClientAttrib();       // подготовка к следующей строчке
   glFlush();
}
// и стандартный TrueType шрифт Microsoft Windows-Ansi-1251 (1 байт 256 знаков)
//
static
void _OutText( const char *str, int base, int &X,int &Y, int bX,int pX )
{  glPushAttrib( GL_LIST_BIT );               // собственно печать текста
   glListBase( base );
   glCallLists( strlen( str ),GL_UNSIGNED_BYTE,str );
   glPopAttrib();
// glFlush();
  int P[4]={ 0,0,0,0 };                // для выборки новой позиции chX
   if( bX>0 ){ glGetIntegerv( GL_CURRENT_RASTER_POSITION,P ); X=P[0]-pX; }
}
Place& Place::String( const char *T )  // исходная строка реально не изменяется
{ TextContext St;
  RasterSector Sv( pX,pY,Width,Height );        // перекодировка с выбором
  char *str=UtA( T,AF.Bit!=NULL );              // DOS-866 или Windows-1251
  while( str )if( !str[0] )break; else
  { char *w=strchr( str,'\n' ); if( w )w[0]=0;  // намечается следующая строка?
    if( AF.Bit )
    { if( cbX<=0 )chX-=strlen(str)*Tw;
      glRasterPos2i( chX,chY ); _OutBitText( str,AF.Bit,chX,chY,cbX ); } else
    { WinFont O( Site->hDC,AF.hF );;
#if 0                         /// так должно быть, но работает только в Intel
      if( cbX<=0 )chX-=AlfaRect( str,true ).cx;
      glRasterPos2i( chX,chY ); _OutText( str,AF.TB,chX,chY,cbX,pX );
#else                         // небольшая перепутаница в AMD-OpenGL - что жаль
      int hk; long l=AlfaRect( str,true ).cx;  if( cbX<=0 )chX-=l; hk=chX;
      glRasterPos2i( chX,chY ); _OutText( str,AF.TB,chX,chY,cbX,pX );
      if( chX==hk )chX+=l+1;
#endif
    }
    if( !w )break; w[0]='\n'; chY-=Th; str=w+1;     //  будет еще одна строка
    if( cbX>0 )chX=(cbX-1)*Tw; else chX=Width+Tw*cbX;
  } return *this;
}
//!    Прорисовка плоско/горизонтального текста по предварительному определению
//!         размеров букв и смещением надписи относительно точки в плоскости XY
//
Place& Place::String( Course Dir, const Real *P, const char* T )
{ TextContext St;
  int dx,dy; char *str=UtA( T,AF.Bit!=NULL ); // выбор DOS-866 или Windows-1251
   glRasterPos3d( P[0],P[1],P[2] );           // плоская надпись со смещением
  SIZE sz=AlfaRect( str,true ); dx= -sz.cx/2; // _Center
   if( Dir&_East  )dx = 4;      dy= -sz.cy/6; // сдвиги и переносы строк
   if( Dir&_West  )dx =-sz.cx - 4;            // здесь отключены
   if( Dir&_North )dy = sz.cy/5 + 3;
   if( Dir&_South )dy =-sz.cy + 3; glBitmap( 0,0,0,0,dx,dy,NULL );
   if( AF.Bit )_OutBitText( str,AF.Bit,dx,dy,1 );
         else  _OutText( str,AF.TB,dx,dy,1,pX ); return *this;
}
#include <stdio.h>   // блок для четырёх вариантов текстовых надписей в формате
#include <stdarg.h>  // с реентерабельной\стековой подстрочкой переменной длины
#define Arg va_list a; va_start( a,fmt ); int l=vsnprintf( 0,0,fmt,a )+1; \
              { char s[l]; vsnprintf( s,l,fmt,a ); va_end( a ); String
#define Str ; } return *this; // просто с "временной" строчкой на всё и про всё
Place& Place::Print( const char *fmt,... ){ Arg( s )Str }
Place& Place::Print( int x,_Real y, const char *fmt,... )
{ cbX=x; chY=y>0 ? Height-Th*y:3-Th*y;  // запоминается начальная позиция
  if( x>0 )chX=(x-1)*Tw; else chX=Width+Tw*x; Arg( s )Str
}
Place& Place::Text( Course Dir, const Real *P, const char* fmt, ... )
     { Arg( Dir,P,s )Str }
Place& Place::Text( Course Dir, _Real X,_Real Y,_Real Z, const char* fmt, ... )
     { const Real R[]={X,Y,Z }; Arg( Dir,R,s )Str }

#if 0
//
//!      Управление представлением и размером текстовых строк
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
#endif
