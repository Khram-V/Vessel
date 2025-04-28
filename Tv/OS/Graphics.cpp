//
//      WinBGI - Graphics
//      Цветовая схема приведена к палитре в 256 цветов с переопределением фона
//                                                        на нулевом элементе
//
#include <Math.h>
#include "Graphics.h"
#include "Win_Palette.inc"
#define   MAX_PAGES 4            //! - два или четыре ?, и пока больше не нужно
//
//   Обработка прерывания от клавиатуры и мышки переведена в Tv_getc
//
void WindowChar( TCHAR Char );
void WindowKeyDown( WPARAM wVkey, LONG dwKeyData );
void WindowMouse( UINT messg, int X, int Y );
extern "C"
{      char **_av_=NULL;                 // в Win32 параметры _argv и _argc
       int    _ac_=0;                    // стали виртуальными и неприкасаемыми
static HWND    hWnd;
static HDC     hdc[2];
static HRGN    hRgn;
static HPEN    hPen=0;
static HBRUSH  hBrush[USER_FILL+1],hBackgroundBrush;
static HBITMAP hBitmap[MAX_PAGES];
static BITMAPINFO BMI =
   { { sizeof(BITMAPINFOHEADER),1,1,1,32,BI_RGB,0, 0,0,0,0 },{ { 0,0,0,0 } }
   };              // first  parameter is bitmapinfoheader size
                   // second parameter is bitmap image width
                   // third  parameter is bitmap image height
                   // fourth is planes of image
                   // fifth  is compression (BI_RGB is no compression)
                   // sixth  is image size (number of pixels*4 bytes per pixel)
static const char* WinTitle=NULL;
static const int
  border_left=GetSystemMetrics( SM_CXFIXEDFRAME ),
  border_top=GetSystemMetrics( SM_CYCAPTION )+GetSystemMetrics( SM_CYFIXEDFRAME ),
  border_height=border_top+GetSystemMetrics( SM_CYFIXEDFRAME ),
  screen_width=GetSystemMetrics( SM_CXSCREEN )-border_left*2,
  screen_height=GetSystemMetrics( SM_CYFULLSCREEN )-4,   //-border_height,
  line_style_cnv[]={ PS_SOLID,PS_DOT,PS_DASHDOT,PS_DASH,PS_DASHDOTDOT },
  write_mode_cnv[]={ R2_COPYPEN,R2_XORPEN,R2_MERGEPEN,R2_MASKPEN,R2_NOTCOPYPEN },
  bitblt_mode_cnv[]={ SRCCOPY,SRCINVERT,SRCPAINT,SRCAND,NOTSRCCOPY };

static int
  aspect_ratio_x=10000, window_width=800,
  aspect_ratio_y=10000, window_height=600,
  write_mode=COPY_PUT,  visual_page=0,active_page=0,
  color=BLACK,          // BGI's color for drawing lines and text.
  bcolor=WHITE,         // Background color
  wincolor=BLUE;        // Color that Windows plans to use for text & filling

#define COLOR( r,g,b ) (0x04000000 | RGB(r,g,b)) // палитра Windows
static COLORREF *Rcolor=NULL;                    // исходная палитра iBGR
static PALETTEENTRY *Bcolor=NULL;                // с наложенным из ДОС образом
static viewporttype view_settings;
static fillpatterntype user_pattern={ 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static fillsettingstype fill_settings;
static linesettingstype line_settings;
static arccoordstype ac;
const unsigned short
  SolidBrushBitmap     [8]={ ~0xFF,~0xFF,~0xFF,~0xFF,~0xFF,~0xFF,~0xFF,~0xFF },
  LineBrushBitmap      [8]={ ~0x00,~0x00,~0x00,~0x00,~0x00,~0x00,~0x00,~0xFF },
  LtslashBrushBitmap   [8]={ ~0x01,~0x02,~0x04,~0x08,~0x10,~0x20,~0x40,~0x80 },
  SlashBrushBitmap     [8]={ ~0x81,~0x03,~0x06,~0x0C,~0x18,~0x30,~0x60,~0xC0 },
  BkslashBrushBitmap   [8]={ ~0xC0,~0x60,~0x30,~0x18,~0x0C,~0x06,~0x03,~0x81 },
  LtbkslashBrushBitmap [8]={ ~0x80,~0x40,~0x20,~0x10,~0x08,~0x04,~0x02,~0x01 },
  HatchBrushBitmap     [8]={ ~0x01,~0x01,~0x01,~0x01,~0x01,~0x01,~0x01,~0xFF },
  XhatchBrushBitmap    [8]={ ~0x81,~0x42,~0x24,~0x18,~0x18,~0x24,~0x42,~0x81 },
  InterleaveBrushBitmap[8]={ ~0x55,~0xAA,~0x55,~0xAA,~0x55,~0xAA,~0x55,~0xAA },
  WidedotBrushBitmap   [8]={ ~0x00,~0x00,~0x00,~0x10,~0x00,~0x00,~0x00,~0x01 },
  ClosedotBrushBitmap  [8]={ ~0x44,~0x00,~0x11,~0x00,~0x44,~0x00,~0x11,~0x00 };

static char *_Extr_Key( const char _p[] )
{ char *s=NULL;
  for( int i=1; i<_ac_; i++ )if( s )_av_[i-1]=_av_[i]; else
  {  char *a=_av_[i];
     if( *a=='-' || *a=='/' )if( strstr( strlwr( a+1 ),_p ) )s=a;
  }  if( s )_av_[--_ac_]=NULL; return s; // после выхода -> free( s );
}
#include <stdio.h>
static int getdefaultmode()
{ if( !_ac_ && !_av_ )
  { char *Key; int i,M=-1;
    _av_=(char**)malloc( __argc*sizeof( char* ) );  // Дубль входных параметров
     for( _ac_=0; _ac_<__argc; _ac_++ )_av_[_ac_]=strdup( __argv[_ac_] );
     if( (Key=getenv(  "bgi" ) )!=NULL )M=atoi( Key );
     if( (Key=_Extr_Key( "bgi=" ))!=NULL ){ M=atoi( Key+5 ); free( Key ); }
     Rcolor=(COLORREF*)(Bcolor=(PALETTEENTRY*)calloc(256,sizeof(PALETTEENTRY)));
     for( i=0; i<256; i++ )
     { Bcolor[i].peRed = Default_Palettes[i].r,
       Bcolor[i].peGreen=Default_Palettes[i].g,
       Bcolor[i].peBlue =Default_Palettes[i].b;
     } // SetPaletteEntries( hPalette,index,1,&BGIpalette[index] );
     return M>0?M:VGAMAX;
  }  return VGAMAX;
}
static int bgiemu_default_mode=getdefaultmode(); /*! конструктор режимов
                                           -bgi = 1 -  320 x 240  - CGA
                                                  2 -  640 x 350  - EGA
                                                  3 -  640 x 480  - VGA
                                                  4 -  800 x 600  - SVGA
                                                  5 - 1024 x 768  - VESA
                                                  6 - 1280 x 1024
                                                  7 - 1600 x 1200
                                                  8 - 1920 x 1440 - VGAMAX=0 */
int getmaxx(){ return window_width-1; }
int getmaxy(){ return window_height-1; }
int getx()
{ POINT pos; GetCurrentPositionEx( hdc[active_page==visual_page ? 0:1],&pos );
  return pos.x;
}
int gety()
{ POINT pos; GetCurrentPositionEx( hdc[active_page==visual_page ? 0:1],&pos );
  return pos.y;
}
static void portview( bool key=true )
{ if( hRgn )
    { if( key )
      { SelectClipRgn( hdc[1],hRgn );
        SelectClipRgn( hdc[0],hRgn );
        SetViewportOrgEx( hdc[1],view_settings.left,view_settings.top,NULL );
        SetViewportOrgEx( hdc[0],view_settings.left,view_settings.top,NULL );
      } else
      { SelectClipRgn( hdc[0],NULL ); SetViewportOrgEx( hdc[0],0,0,NULL );
        SelectClipRgn( hdc[1],NULL ); SetViewportOrgEx( hdc[1],0,0,NULL );
      }
    }
}
void getviewsettings( viewporttype *viewport ){ *viewport=view_settings; }
void setviewport( int x1, int y1, int x2, int y2, int clip )
{    view_settings.left   = x1;
     view_settings.top    = y1;
     view_settings.right  = x2;
     view_settings.bottom = y2;
     view_settings.clip = clip;
  if( hRgn ){ portview( false ); DeleteObject( hRgn ); }
  if( !clip )hRgn=NULL; else{ hRgn=CreateRectRgn( x1,y1,x2,y2 ); portview(); }
  moveto( 0,0 );
}
int getcolor(){ return color; }
int getbkcolor(){ return bcolor; }
static int convert_userbits( DWORD buf[32], unsigned pattern )
{ int i=0,j;
  pattern &= 0xFFFF; pattern = (pattern<<16)|pattern;
  while( true )
  { for( j=0; pattern&1; j++ )pattern>>=1; buf[i++]=j;
    if( !pattern ){ buf[i++]=4-j; return i; }
    for( j=0; !(pattern&1); j++ )pattern>>=1; buf[i++]=j;
} }
void setcolor( int c )
{    SetTextColor( hdc[0],Rcolor[c] );
     SetTextColor( hdc[1],Rcolor[c] ); color=wincolor=c;
     if( hPen ){ DeleteObject( hPen ); hPen=0; }
     if( line_settings.linestyle==USERBIT_LINE )
     { LOGBRUSH lb;
        lb.lbColor=Rcolor[c];
        lb.lbStyle=BS_SOLID;
       DWORD style[32];
        hPen=ExtCreatePen( PS_GEOMETRIC|PS_USERSTYLE,line_settings.thickness,
                 &lb,convert_userbits( style,line_settings.upattern ),style );
     }
     if( !hPen )hPen = CreatePen( line_style_cnv[line_settings.linestyle],
                                  line_settings.thickness,Rcolor[c] );
     SelectObject( hdc[0],hPen );
     SelectObject( hdc[1],hPen );
}
void setbkcolor( int bc )
{    SetBkColor( hdc[0],Rcolor[bcolor=(bc&=0xFF)] );
     SetBkColor( hdc[1],Rcolor[bc] );
     DeleteObject( hBackgroundBrush );
     hBackgroundBrush=CreateSolidBrush( Rcolor[bc] ); setcolor( color );
}
void setrgbpalette( int index, int red, int green, int blue )
{   color = (index&=0xFF);
    Bcolor[index].peRed = red;   // & 0xFC  Mask RGB values to use only
    Bcolor[index].peGreen=green; // & 0xFC  most significant 6 bits from
    Bcolor[index].peBlue =blue;  // & 0xFC  least significant byte:
    if( index==bcolor )setbkcolor( index ); else setcolor( color );
}
void getrgbpalette( int index, byte *red, byte *green, byte *blue )
{   *red  =Bcolor[index].peRed;
    *green=Bcolor[index].peGreen;
    *blue =Bcolor[index].peBlue;
}
void setlinestyle( int style, unsigned int pattern, int thickness )
{    line_settings.linestyle=style;
     line_settings.thickness=thickness;
     line_settings.upattern =pattern; setcolor( color );
}
void getlinesettings( linesettingstype* ls ){ *ls=line_settings; }
int  setwritemode( int mode )
{ int ret_mode=write_mode;
   if( (COPY_PUT<=mode)&&(mode<=NOT_PUT) )write_mode=mode; setcolor( color );
  return ret_mode;
}
void setfillstyle( int style, int c )
{    fill_settings.pattern = style;
     fill_settings.color = c&0xFF;
     SelectObject( hdc[0],hBrush[style] );
     SelectObject( hdc[1],hBrush[style] ); setcolor( color );
}
void getfillsettings( fillsettingstype* fs ){ *fs=fill_settings; }
void getfillpattern( fillpatterntype& fp )
   { memcpy( fp,user_pattern,sizeof user_pattern );
   }
void setfillpattern( fillpatterntype upattern, int c )
{ static HBITMAP hFillBitmap=0;
  static short bitmap_data[8];
    memcpy( user_pattern,upattern,sizeof user_pattern );
    for( int i=0; i<8; i++ )bitmap_data[i]=(byte)~upattern[i];
  HBITMAP h=CreateBitmap( 8,8,1,1,bitmap_data );
  HBRUSH hb=CreatePatternBrush( h );
                     DeleteObject( hBrush[USER_FILL] );
    if( hFillBitmap )DeleteObject( hFillBitmap );
    hFillBitmap=h;
    hBrush[USER_FILL]=hb;
    SelectObject( hdc[0],hb );
    SelectObject( hdc[1],hb );
    fill_settings.color=c&0xFF;
    fill_settings.pattern=USER_FILL; setcolor( color );
}                               //
static bool __writemode()       // для линий и всего прочего
{ static bool first=false;       // статическая постановка режима рисования
  first=!first;                   //
  if( write_mode != COPY_PUT )
  { int mode = first ? write_mode_cnv[write_mode] : R2_COPYPEN;
                                  SetROP2( hdc[1],mode );
    if( visual_page==active_page )SetROP2( hdc[0],mode );
  } return first;
}                             //
static void __fillmode()        // сильно странная конструкция с цветами текста
{ bool first=__writemode();      //
  if( wincolor != fill_settings.color )
  { int color = first ? fill_settings.color:wincolor;
                                  SetTextColor( hdc[1],Rcolor[color] );
    if( visual_page==active_page )SetTextColor( hdc[0],Rcolor[color] );
} }
void moveto( int x, int y ){    MoveToEx( hdc[1],x,y,NULL );
  if( visual_page==active_page )MoveToEx( hdc[0],x,y,NULL );
}
void moverel( int dx, int dy )
{ POINT pos; GetCurrentPositionEx( hdc[1],&pos ); moveto( pos.x+dx,pos.y+dy );
}
const double pi = 3.14159265358979323846;

static void arc_coords( double angle, double rx, double ry, int& x, int& y )
{   if( rx==0 || ry==0 ){ x=y=0; return; }
 double s = sin( angle*pi/180.0 ), c = cos( angle*pi/180.0 );
    if( fabs( s )<fabs( c ) )
    { double tg=s/c,xr=sqrt( rx*rx*ry*ry/(ry*ry+rx*rx*tg*tg) );
        x = int( (c >= 0) ? xr : -xr );
        y = int( (c >= 0) ? -xr*tg : xr*tg );
    } else
    { double ctg=c/s,yr=sqrt( rx*rx*ry*ry/(rx*rx+ry*ry*ctg*ctg) );
        x = int( (s >= 0) ? yr*ctg : -yr*ctg );
        y = int( (s >= 0) ? -yr : yr );
    }
}
void ellipse( int x,int y, int start_angle,int end_angle, int rx,int ry )
{   ac.x = x;
    ac.y = y;
    arc_coords( start_angle, rx, ry, ac.xstart, ac.ystart );
    arc_coords( end_angle,  rx, ry, ac.xend, ac.yend );
    ac.xstart += x; ac.ystart += y;
    ac.xend += x; ac.yend += y;
  __writemode();
    Arc( hdc[1],x-rx,y-ry,x+rx,y+ry, ac.xstart,ac.ystart,ac.xend,ac.yend );
    if( visual_page==active_page )
    Arc( hdc[0],x-rx,y-ry,x+rx,y+ry, ac.xstart,ac.ystart,ac.xend,ac.yend );
  __writemode();
}
void fillellipse( int x,int y, int rx,int ry )
{ __fillmode();                   Ellipse( hdc[1],x-rx,y-ry,x+rx,y+ry );
    if( visual_page==active_page )Ellipse( hdc[0],x-rx,y-ry,x+rx,y+ry );
  __fillmode();
}
static void allocate_new_graphic_page( int page )
{ RECT scr = { 0,0,window_width,window_height };
    hBitmap[page]=CreateCompatibleBitmap( hdc[0],window_width,window_height );
    SelectObject ( hdc[1],hBitmap[page] );
    FillRect( hdc[1],&scr,hBackgroundBrush ); portview();
}
void setactivepage( int page )
{ if( (page<0) || (page>=MAX_PAGES) )return;
  if( hBitmap[page]==NULL )allocate_new_graphic_page( page );
                   else SelectObject( hdc[1],hBitmap[page] ); active_page=page;
}
void setvisualpage( int page )
{ POINT pos;
   if( (page<0) || (page>=MAX_PAGES) )return;
   if( hBitmap[page]==NULL )allocate_new_graphic_page( page );
   portview( false );
   SelectObject( hdc[1], hBitmap[page] );
   BitBlt( hdc[0],0,0,window_width,window_height,hdc[1],0,0,SRCCOPY );
   portview();
   if( page!=active_page ){ SelectObject(hdc[1], hBitmap[active_page]); }
   if( active_page!=visual_page )
     { GetCurrentPositionEx( hdc[1],&pos );
       MoveToEx( hdc[0],pos.x,pos.y,NULL );
     } visual_page=page;
}
void hidevisualpage(){ visual_page=-1; }
void showactivepage()
{ if( active_page!=visual_page )
  { SelectObject( hdc[1], hBitmap[active_page] );
    portview( false );
    BitBlt( hdc[0],0,0,window_width,window_height,hdc[1],0,0,SRCCOPY );
    portview();
    visual_page=active_page;
  }
}
void setaspectratio( int ax, int ay ){ aspect_ratio_x=ax; aspect_ratio_y=ay; }
void getaspectratio( int* ax, int* ay ){ *ax=aspect_ratio_x; *ay=aspect_ratio_y; }

void circle( int x, int y, int radius )
{ int ry = (unsigned)radius*aspect_ratio_x/aspect_ratio_y;
  int rx = radius;
  __writemode(); Arc( hdc[1], x-rx, y-ry, x+rx, y+ry, x+rx, y, x+rx, y );
  if( visual_page==active_page )
                 Arc( hdc[0], x-rx, y-ry, x+rx, y+ry, x+rx, y, x+rx, y );
  __writemode();
}
void arc( int x, int y, int start_angle, int end_angle, int radius )
{   ac.x = x;
    ac.y = y;
    ac.xstart= x + int( radius*cos( start_angle*pi/180.0 ) );
    ac.ystart= y - int( radius*sin( start_angle*pi/180.0 ) );
    ac.xend  = x + int( radius*cos( end_angle*pi/180.0 ) );
    ac.yend  = y - int( radius*sin( end_angle*pi/180.0 ) ); __writemode();
    Arc( hdc[1],x-radius,y-radius,x+radius,y+radius,
                 ac.xstart,ac.ystart,ac.xend,ac.yend );
    if( visual_page==active_page )
      Arc( hdc[0],x-radius,y-radius,x+radius,y+radius,
                   ac.xstart,ac.ystart,ac.xend,ac.yend );   __writemode();
}
void getarccoords( arccoordstype *arccoords ){ *arccoords=ac; }

void pieslice( int x, int y, int start_angle, int end_angle, int radius )
{   ac.x = x;
    ac.y = y;
    ac.xstart = x + int( radius*cos(start_angle*pi/180.0) );
    ac.ystart = y - int( radius*sin(start_angle*pi/180.0) );
    ac.xend  =  x + int( radius*cos(end_angle*pi/180.0) );
    ac.yend  =  y - int( radius*sin(end_angle*pi/180.0) );
  __writemode();
    Pie( hdc[1],x-radius,y-radius,x+radius,y+radius,ac.xstart,ac.ystart,ac.xend,ac.yend );
    if( visual_page==active_page )
     Pie( hdc[0],x-radius,y-radius,x+radius,y+radius,ac.xstart,ac.ystart,ac.xend,ac.yend );
  __writemode();
}
void sector(int x, int y, int start_angle, int end_angle, int rx, int ry)
{   ac.x = x; __writemode();
    ac.y = y;
    arc_coords( start_angle, rx, ry, ac.xstart, ac.ystart );
    arc_coords( end_angle, rx, ry, ac.xend, ac.yend );
    ac.xstart += x; ac.ystart += y;
    ac.xend += x; ac.yend += y;
    Pie( hdc[1],x-rx,y-ry,x+rx,y+ry,ac.xstart,ac.ystart,ac.xend,ac.yend );
    if( visual_page==active_page)
      Pie( hdc[0],x-rx,y-ry,x+rx,y+ry,ac.xstart,ac.ystart,ac.xend,ac.yend );
    __writemode();
}
void bar( int left, int top, int right, int bottom )
{ RECT r;            /* Turbo C corrects for badly ordered corners */
    if( left>right )r.left=right,r.right=left;
              else  r.left=left,r.right=right;
    if( bottom<top )r.top=bottom,r.bottom=top;
              else  r.top=top,r.bottom=bottom;           __fillmode();
    FillRect( hdc[1],&r,hBrush[fill_settings.pattern] );
    if( visual_page==active_page )
    FillRect( hdc[0],&r,hBrush[fill_settings.pattern] ); __fillmode();

}
void bar3d(int left, int top, int right, int bottom, int depth, int topflag)
{ int temp;
    if( left>right ){ temp=left; left=right; right=temp; }
    if( bottom<top ){ temp=bottom; bottom=top; top=temp; }
    bar( left+line_settings.thickness,top+line_settings.thickness,
        right-line_settings.thickness+1,bottom-line_settings.thickness+1 );
  int dy=int( (double)depth/1.73205080756887729352 ); // 1.0 / tan( 30 )
  POINT p[11];
    p[0].x = right, p[0].y = bottom;
    p[1].x = right, p[1].y = top;
    p[2].x = left,  p[2].y = top;
    p[3].x = left,  p[3].y = bottom;
    p[4].x = right, p[4].y = bottom;
    p[5].x = right+depth, p[5].y = bottom-dy;
    p[6].x = right+depth, p[6].y = top-dy;
    p[7].x = right, p[7].y = top;
    if( topflag )
    {   p[8].x = right+depth,p[8].y = top-dy;
        p[9].x = left+depth, p[9].y = top-dy;
        p[10].x= left,       p[10].y= top;
    }
  __fillmode();                   Polyline( hdc[1],p,topflag ? 11:8 );
    if( visual_page==active_page )Polyline( hdc[0],p,topflag ? 11:8 );
  __fillmode();
}
void lineto( int x, int y )
{ __writemode();                  LineTo( hdc[1],x,y );
    if( visual_page==active_page )LineTo( hdc[0],x,y );
  __writemode();
}
void linerel( int dx, int dy )
{ POINT pos; GetCurrentPositionEx( hdc[1],&pos ); lineto( pos.x+dx,pos.y+dy );
}
void drawpoly( int n_points, int* points )
{ __writemode();                  Polyline( hdc[1],(POINT*)points,n_points );
    if( visual_page==active_page )Polyline( hdc[0],(POINT*)points,n_points );
  __writemode();
}
void line( int x0,int y0, int x1,int y1 )
     { int q[]={ x0,y0,x1,y1 }; drawpoly( 2,q );
     }
void rectangle( int l, int t, int r, int b )        // left,top,right,bottom
     { int q[]={l,t,r,t,r,b,l,b,l,t}; drawpoly( 5,q );
     }
void fillpoly( int n_points, int* points )
{ __fillmode();                   Polygon( hdc[1],(POINT*)points,n_points );
    if( visual_page==active_page )Polygon( hdc[0],(POINT*)points,n_points );
  __fillmode();
}
void floodfill( int x, int y, int border )
{ __fillmode();                   FloodFill( hdc[1],x,y,Rcolor[border] );
    if( visual_page==active_page )FloodFill( hdc[0],x,y,Rcolor[border] );
  __fillmode();
}
void cleardevice()
{ RECT scr = { 0,0,window_width,window_height };
  portview( false );            FillRect( hdc[1],&scr,hBackgroundBrush );
  if( visual_page==active_page )FillRect( hdc[0],&scr,hBackgroundBrush );
  portview();
  moveto( 0,0 );
}
void clearviewport()
{ if( visual_page==active_page )
    FillRect( hdc[0],(RECT*)(&view_settings),hBackgroundBrush );
    FillRect( hdc[1],(RECT*)(&view_settings),hBackgroundBrush ); moveto( 0,0 );
}
static void Error();
//
//!  И еще малость о внешнем представлении изображений
//
unsigned int imagesize( int x1,int y1, int x2,int y2 )
{ return 2*sizeof(short) + (7+(x2-x1+1)*(y2-y1+1)*32)/8;
  // 2 shorts(2 bytes each) + #of pixels* 32 bits per pixel / 8 bits per byte
  // this was 8 + (((x2-x1+8) & ~7) >> 1)*(y2-y1+1);
}
void getimage( int x1,int y1, int x2,int y2, void* image ) // память для image
{ BGIimage* bim = (BGIimage*)image;                         // выделена извне
  int *tempBits,localImageSize;
  HDC tempHDC;
  HBITMAP tempBMP;
    BMI.bmiHeader.biWidth=bim->width=short(x2-x1+1); // 32 bits per pixel
    BMI.bmiHeader.biHeight=bim->height=short(y2-y1+1); // 8 bits per byte
    BMI.bmiHeader.biSizeImage=localImageSize=4*int(bim->width)*(bim->height);
    tempHDC=CreateCompatibleDC( hdc[1] );
    tempBMP=CreateDIBSection( tempHDC,&BMI,DIB_RGB_COLORS,(void**)&tempBits,0,0 );
    if( !tempBMP )Error();
    if( !SelectObject( tempHDC,tempBMP ) )Error();
    if( !BitBlt( tempHDC,0,0,bim->width,bim->height,hdc[1],x1,y1,SRCCOPY ) )Error();
    memcpy( bim->bits,tempBits,localImageSize );
    DeleteObject( tempBMP );
    DeleteDC( tempHDC );
}
void putimage( int x, int y, void* image, int bitblt )
{ BGIimage* bi=(BGIimage*)image;
  HDC tempHDC;
  HBITMAP tempBMP;
  byte *bitmapBits=0;
  unsigned localImageSize;
    BMI.bmiHeader.biWidth=bi->width;           //32 bits/pixel, 8 bits/byte
    BMI.bmiHeader.biHeight=bi->height;
    BMI.bmiHeader.biSizeImage=localImageSize=4*int(bi->width)*(bi->height);
    tempHDC=CreateCompatibleDC( hdc[1] );
    if( !SetDIBitsToDevice( tempHDC,0,0,bi->width,bi->height,0,0,0,bi->height-1,&(bi->bits),&BMI,DIB_RGB_COLORS ) )Error();
    tempBMP=CreateDIBSection( tempHDC,&BMI,DIB_RGB_COLORS,(void**)(&bitmapBits),0,0 );
    if( !SelectObject( tempHDC,tempBMP ) )Error();
    memcpy( bitmapBits,bi->bits,localImageSize );  // Select the bitmaps into the compatible DC.
    if( visual_page==active_page )
    { if( SelectObject( hdc[0],tempBMP ) )Error();   // was tempBMP
      if( !BitBlt(hdc[0],x,y,bi->width,bi->height,tempHDC,0,0,bitblt_mode_cnv[bitblt]) )Error(); //bi->hdc was tempHDC
    } if( SelectObject( hdc[1],tempBMP ) )Error();   // was tempBMP
      if( !BitBlt(hdc[1],x,y,bi->width,bi->height,tempHDC,0,0,bitblt_mode_cnv[bitblt]) )Error(); //bi->hdc was tempHDC
      DeleteObject( tempBMP );
      DeleteDC( tempHDC );
}
void putbits( int x, int y, int w, int h, void* imgbit, int bitblt )
{ HBITMAP BitMap = CreateBitmap( w,h, 1,1, imgbit );
  static HDC BDc = 0;
   if( !BDc )BDc = CreateCompatibleDC( hdc[0] );
                   SelectObject( BDc,BitMap );
   BitBlt( hdc[1], x,y,w,h, BDc,0,0, bitblt_mode_cnv[bitblt] );
   if( visual_page==active_page )
       BitBlt( hdc[0], x,y,w,h, BDc,0,0, bitblt_mode_cnv[bitblt] );
   DeleteObject( BitMap );
}
unsigned int getpixel( int x,int y )
{ int c;
  COLORREF rgb=GetPixel( hdc[1],x,y );
    if( rgb==CLR_INVALID )return CLR_INVALID;
  int red = GetRValue( rgb );
  int blue= GetBValue( rgb );
  int green=GetGValue( rgb );
    for( c=0; c<256; c++ )
    { if( Bcolor[c].peRed==red && Bcolor[c].peGreen==green
       && Bcolor[c].peBlue==blue )return c;
    } return COLOR( red,green,blue );
}
void putpixel( int x,int y, unsigned c )
{ if( c<256 )c=Rcolor[c];       SetPixel( hdc[1],x,y,c );
  if( visual_page==active_page )SetPixel( hdc[0],x,y,c );
}
static void deletecontext()
{ for( int i=0; i<MAX_PAGES; i++ )if( hBitmap[i] )
     { SelectObject( hdc[1],hBitmap[i]  );
       DeleteObject( hBitmap[i] );
       hBitmap[i]=0;
     } DeleteObject( hdc[1] ); hdc[0]=0;
}
///
///              Главная процедура виндовского цикла очередей
static
LRESULT CALLBACK WndProc( HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam )
{ static PAINTSTRUCT ps;
  if( messg>=WM_MOUSEFIRST && messg<=WM_MOUSELAST )
      WindowMouse( messg,int(lParam&0xffff),int((lParam>>16)&0xffff) );
  else switch( messg )                               //
  { case WM_PAINT:                                   // Real window create here
    if( !hdc[0] )                                    //
    { hdc[1]=CreateCompatibleDC( hdc[0]=BeginPaint( hWnd,&ps ) );
      hBitmap[active_page]=CreateCompatibleBitmap( hdc[0],window_width,window_height );
      SelectObject( hdc[1],hBitmap[active_page] );
      SetTextColor( hdc[0],Rcolor[wincolor] );
      SetTextColor( hdc[1],Rcolor[wincolor] );
      SetBkColor  ( hdc[0],Rcolor[bcolor] );
      SetBkColor  ( hdc[1],Rcolor[bcolor] );
      SelectObject( hdc[0],hBrush[fill_settings.pattern] );
      SelectObject( hdc[1],hBrush[fill_settings.pattern] );
      RECT scr = { 0,0,window_width,window_height };
      FillRect( hdc[1],&scr,hBackgroundBrush );
    } portview( false );
    if( visual_page>=0 )
    { if( visual_page!=active_page )SelectObject( hdc[1],hBitmap[visual_page] );
      BitBlt( hdc[0],0,0,window_width,window_height,hdc[1],0,0,SRCCOPY );
    } portview();
    if( visual_page!=active_page )SelectObject( hdc[1],hBitmap[active_page] );
    ValidateRect( hWnd,NULL ); break; // return DefWindowProc( hWnd,messg,wParam,lParam );
//  case WM_SIZE: window_width=LOWORD(lParam);
//                window_height=HIWORD(lParam); break;
//  case WM_TIMER: KillTimer( hWnd,TIMER_ID ); timeout_expired=true; break;
    case WM_CHAR: WindowChar( (TCHAR)wParam ); break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN: WindowKeyDown( wParam,lParam ); break;
    case WM_DESTROY: EndPaint( hWnd,&ps ); deletecontext(); break;
    case WM_CLOSE:
    case WM_QUIT :  DestroyWindow( hWnd );
    default: return DefWindowProc( hWnd,messg,wParam,lParam );
  }          return 0;
}
void closegraph(){ DestroyWindow( hWnd ); PostQuitMessage( 0 ); }

static void detect( int &mode, int &width,int &height ) // Режимы /bgi=:
{ short VM[][2]={ {screen_width,screen_height},         // 1 CGA  -  320 x 240
     {320,240}, {640,350},  {640,480},  {800,600},      // 2 EGA  -  640 x 350
    {1024,768},{1280,1024},{1600,1200},{1920,1440} };   // 3 VGA  -  640 x 480
    if( (unsigned)mode>8 )mode=VGAMAX;                  // 4 SVGA -  800 x 600
    width  = VM[mode][0];                               // 5 VESA - 1024 x 768
    height = VM[mode][1];                               // 6      - 1280 x 1024
    if( width>screen_width )width=screen_width;         // 7      - 1600 x 1200
    if( height>screen_height )height=screen_height;     // 8      - 1920 x 1440
    if( width==screen_width && height==screen_height )mode=VGAMAX;
//  height=SystemParametersInfo( SPI_GETWORKAREA,?,?,? );
}
static void graphdefaults()                // с некоторым перебором определений
{   setcolor  ( WHITE );                   // белые линии
    setbkcolor( BLACK );                   // на черном фоне
    line_settings.thickness= 1;            // Set fill style
    line_settings.linestyle= SOLID_LINE;
    line_settings.upattern = ~0;
    fill_settings.pattern = SOLID_FILL;    // Set fill pattern
    fill_settings.color = WHITE;
    write_mode = COPY_PUT;
    active_page=visual_page=0;
    view_settings.left=view_settings.top=0;
    view_settings.right=window_width;      // Set viewport
    view_settings.bottom=window_height;    //  to entire screen:
    aspect_ratio_x=aspect_ratio_y=10000;
    SelectClipRgn( hdc[0],NULL ); SetViewportOrgEx( hdc[0],0,0,NULL );
    SelectClipRgn( hdc[1],NULL ); SetViewportOrgEx( hdc[1],0,0,NULL );
    SelectObject( hdc[0],hBrush[fill_settings.pattern] );
    SelectObject( hdc[1],hBrush[fill_settings.pattern] );
    moveto( 0,0 );                              // Move current position to 0,0
}
void initwindow( int width, int height, int mode )
{ static WNDCLASS wcApp;
  double rh=0.0,rw=0.0;
    if( wcApp.lpszClassName==NULL )
    {   wcApp.lpszClassName="BgiA";
        wcApp.hInstance=0;
        wcApp.lpfnWndProc=WndProc;
        wcApp.hCursor=LoadCursor( NULL,IDC_ARROW );
        wcApp.hIcon=0;
        wcApp.lpszMenuName=0;
        wcApp.hbrBackground=(HBRUSH)GetStockObject( BLACK_BRUSH );
        wcApp.style=CS_SAVEBITS | CS_DBLCLKS;
        wcApp.cbClsExtra=0;
        wcApp.cbWndExtra=0;
        if( !RegisterClass(&wcApp) )Error();
        hBackgroundBrush=CreateSolidBrush( Rcolor[BLACK] );
        hBrush[EMPTY_FILL]=(HBRUSH)GetStockObject( NULL_BRUSH );
#define _Br_( a,b ) hBrush[a]=CreatePatternBrush( CreateBitmap( 8,8,1,1,b ) )
        _Br_( SOLID_FILL,     SolidBrushBitmap      ); //
        _Br_( LINE_FILL,      LineBrushBitmap       ); // Added HBRUSH typecast.
        _Br_( LTSLASH_FILL,   LtslashBrushBitmap    ); // Michael Main,
        _Br_( SLASH_FILL,     SlashBrushBitmap      ); // Dec 21, 1999.
        _Br_( BKSLASH_FILL,   BkslashBrushBitmap    ); //
        _Br_( LTBKSLASH_FILL, LtbkslashBrushBitmap  );
        _Br_( HATCH_FILL,     HatchBrushBitmap      );
        _Br_( XHATCH_FILL,    XhatchBrushBitmap     );
        _Br_( INTERLEAVE_FILL,InterleaveBrushBitmap );
        _Br_( WIDE_DOT_FILL,  WidedotBrushBitmap    );
        _Br_( CLOSE_DOT_FILL, ClosedotBrushBitmap   );
        _Br_( USER_FILL,      SolidBrushBitmap      );
#undef _Br_
    }
    if( mode>0 )bgiemu_default_mode=mode;
    detect( bgiemu_default_mode,window_width,window_height );
    if( height<10 || width<10 )width=window_width,height=window_height;
    if( height>screen_height )rh=double( height )/screen_height;
    if( width>screen_width )rw=double( width )/screen_width;
    if( rh>rw )rw=rh;
    if( rw>1.0 ){ height=height/rw; width=width/rw; }
    if( !( hWnd=CreateWindow
         ( wcApp.lpszClassName,   // указатель на зарегистрированное имя класса
	    WinTitle?WinTitle:"Tv", // указатель на имя окна
	    WS_BORDER|WS_VISIBLE, //WS_POPUPWINDOW -  стиль окна
            (screen_width-width)/3, //CW_USEDEFAULT,        // горизонтальная позиция окна
            (screen_height-height)/5, //CW_USEDEFAULT,        // вертикальная позиция окна
            width+border_left*2,  // ширина окна
            height+border_height, // высота окна
           (HWND)NULL,           // дескриптор родительского или окна владельца
           (HMENU)NULL,          // дескриптор меню или ID дочернего окна
           (HINSTANCE)0,         // hInstance дескриптор экземпляра приложения
           NULL )                // указатель на данные создания окна
          ) )Error(); // grOK
    window_width = width;
    window_height = height;
    ShowWindow( hWnd,SW_SHOWNORMAL ); //VGAMAX ? SW_SHOWMAXIMIZED:SW_SHOWNORMAL
    UpdateWindow( hWnd );
//static RECT wRect; // Проверка на случай нарушения границ виндовскими рамками
//  GetClientRect( hWnd,&wRect ); window_height=wRect.bottom-wRect.top+1;
//                                window_width=wRect.right-wRect.left+1;
    graphdefaults();
}
//  В конце собраны всякие перенастройки с установками графических режимов
#if 0
void sizewindow( int width, int height )
{ double rh=0.0,rw=0.0;
    if( height>screen_height )rh=double( height )/screen_height;
    if( width>screen_width )rw=double( width )/screen_width;
    if( rh>rw )rw=rh;
    if( rw>1.0 ){ height=height/rw; width=width/rw; }
    view_settings.left=view_settings.top=0;
    view_settings.bottom=(window_height=height)-1;
    view_settings.right=(window_width=width)-1;
//  ReleaseDC( hWnd,hdc[0] );
    MoveWindow( hWnd,(screen_width-width)/3,
                     (screen_height-height)/3,width+border_left*2,
                                              height+border_height,true );
//  deletecontext();
//  ShowWindow( hWnd,SW_SHOWNORMAL );
//  UpdateWindow( hWnd );
//  graphdefaults();
}
#else
void sizewindow( int W,int H ){ DestroyWindow( hWnd ); initwindow( W,H,bgiemu_default_mode ); }
#endif
void setwindowtitle( const char* title ){ SetWindowText( hWnd,WinTitle=title); }
 int getactivepage (){  return active_page;   }
 int getvisualpage (){  return visual_page<0?active_page:visual_page;   }
 int getscreenwidth(){  return screen_width;  }
 int getscreenheight(){ return screen_height; }
 int getmodewidth(){ int x,y,m=bgiemu_default_mode; detect( m,x,y ); return x; }
 int getmodeheight(){ int x,y,m=bgiemu_default_mode; detect( m,x,y ); return y; }
void setwindowmouse( int X, int Y )
{ RECT WindowRect; GetWindowRect( hWnd,&WindowRect );
   SetCursorPos( X+WindowRect.left+border_left,Y+WindowRect.top+border_top );
}
static void Error()
{ int i=GetLastError(); char* lpMsgBuf;
  FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,0,i,
  MAKELANGID( LANG_NEUTRAL,SUBLANG_DEFAULT ),(LPTSTR)&lpMsgBuf,0,0 ); // Default language
  MessageBox( NULL,lpMsgBuf,"GetLastError",MB_OK|MB_ICONINFORMATION);// Display the string.
  LocalFree( lpMsgBuf ); exit( i );                                 // Free the buffer.
}
/// close the extern "C" block
}
