#ifndef __Graphic_h__
#define __Graphic_h__
#include <windows.h>
#if defined(__cplusplus)
extern "C" {
#endif
enum color{ BLACK,BLUE,GREEN,CYAN,RED,MAGENTA,BROWN,LIGHTGRAY,DARKGRAY,
            LIGHTBLUE,LIGHTGREEN,LIGHTCYAN,LIGHTRED,LIGHTMAGENTA,YELLOW,WHITE };
enum write_modes{ COPY_PUT, XOR_PUT, OR_PUT, AND_PUT, NOT_PUT };
enum line_styles{ SOLID_LINE,DOTTED_LINE,CENTER_LINE,DASHED_LINE,USERBIT_LINE };
enum fill_styles{ EMPTY_FILL,SOLID_FILL,LINE_FILL,LTSLASH_FILL,SLASH_FILL,
                  BKSLASH_FILL,LTBKSLASH_FILL,HATCH_FILL,XHATCH_FILL,
                  INTERLEAVE_FILL,WIDE_DOT_FILL,CLOSE_DOT_FILL,USER_FILL };
#define VGAMAX               0
#define NORM_WIDTH           1
#define THICK_WIDTH          3
#define USER_CHAR_SIZE       0
typedef byte fillpatterntype[8];
typedef struct{ int left,top, right,bottom, clip; }viewporttype;
typedef struct{ int x,y, xstart,ystart, xend,yend; }arccoordstype;
typedef struct{ int pattern, color; }fillsettingstype;
typedef struct{ int linestyle; unsigned upattern; int thickness; }linesettingstype;
typedef struct          // This struct was moved here to allow access to struct
{  short width,height;  // 2x2 bytes (Mark Richardson 11/29/98)
     char bits[1];      // This means bits is also aligned to 32bit boundary
}    BGIimage;          //
void arc                ( int,int, int,int, int );
void bar                ( int,int, int,int );
void bar3d              ( int,int, int,int, int, int );
void circle             ( int,int, int );
void cleardevice        ();
void clearviewport      ();
void drawpoly           ( int, int*);
void ellipse            ( int, int, int, int, int, int );
void fillellipse        ( int, int, int, int );
void fillpoly           ( int, int* );
void floodfill          ( int, int, int );
void getarccoords       ( arccoordstype* );
void getaspectratio     ( int*, int* );
 int getbkcolor         ();
 int getcolor           ();
void getfillpattern     ( fillpatterntype& );
void getfillsettings    ( fillsettingstype* );
void getlinesettings    ( linesettingstype* );
 int getmaxx            ();
 int getmaxy            ();
void getviewsettings    ( viewporttype* );
 int getscreenwidth     ();
 int getscreenheight    ();
 int getmodewidth       ();
 int getmodeheight      ();
 int getx               ();
 int gety               ();
void line               ( int, int, int, int );
void linerel            ( int, int );
void lineto             ( int, int );
void moverel            ( int, int );
void moveto             ( int, int );
void pieslice           ( int, int, int, int, int );
void rectangle          ( int, int, int, int );
void sector             ( int, int, int, int, int, int );
void setaspectratio     ( int, int );
void setbkcolor         ( int );
void setcolor           ( int );
unsigned setgraphbufsize( unsigned int );
void setfillpattern     ( fillpatterntype,int );
void setfillstyle       ( int, int );
void setlinestyle       ( int, unsigned int, int );
void setviewport        ( int, int, int, int, int );
 int setwritemode       ( int );           // на выходе текущее значение режима

unsigned imagesize      ( int x1,int y1,int x2,int y2 );
void getimage           ( int x1,int y1,int x2,int y2, void *img );
void putimage           ( int x, int y, void *img, int=SRCCOPY );
void putbits            ( int x, int y, int w, int h, void *img, int=SRCCOPY );
unsigned getpixel       ( int x, int y );
void putpixel           ( int x, int y, unsigned c );

void setactivepage      ( int );  // страница воспринимает графические команды
void setvisualpage      ( int ); // эта страница становится видимой
void hidevisualpage     ();     // всегда активен только буфер в памяти
void showactivepage     ();    // возврат невидимой картинки из буфера на экран
 int getactivepage      ();
 int getvisualpage      ();

void setwindowtitle     ( const char* title );
void initwindow         ( int w,int h,int m=VGAMAX ); // height,wigth,bgimode
void sizewindow         ( int w,int h );              // height,wigth
void setwindowmouse     ( int x,int y );
void closegraph         ();

void setrgbpalette      ( int i, int r, int g, int b );
void getrgbpalette      ( int i, byte *r, byte *g, byte* b );

union RGB_colors{ struct{ byte b,g,r,a; }; unsigned int i; };  // палитра
typedef RGB_colors* RGB_palette;                    // точки и векторы палитры
extern const RGB_colors Default_Palettes[256];      // в начале и в работе
extern char **_av_;                        // в Win32 параметры _argv и _argc
extern int    _ac_;                        // стали виртуальными неприкасаемыми
#if defined(__cplusplus)
}
#endif
#endif
