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
