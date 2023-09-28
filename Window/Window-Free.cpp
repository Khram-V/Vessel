//
//!   Подборка независимых от среды программирования транзакций в OpenGL
//
#include "Window.h"
//
//! (пере)Установка размеров рабочей площадки/фрагмента внутри окна Window
//   если X,Y > 0 - отсчеты от левого верхнего угла, иначе - правого и нижнего
//   если Width,Height > 0 - отсчеты в символах, если 0 - до самой границы окна
//                если < 0 - в пикселях и нормальных отсчетах, Y - снизу вверх
//     отсутствие шрифта - подключение окна (не приводит к размеру точки [1x1])
//
Place& Place::Area( int X,int Y, int W,int H )   // достаточно сделать шаг
{ int fw=AF.W,fh=AF.H;                           // единичным и далее по Place,
// всё по горизонтали
   if( !W )W=Site->Width;  else if( W<0 )W=-W,fw=1; else W=W*fw; // связанной с
   if( X>0 )X=(X-1)*fw; else X = Site->Width - W + X*fw;      // базовой Window
       X=minmax( 0,X,Site->Width-AF.W );              // перерасчет с проверкой
   if( X+W>Site->Width  )                             // жертвуем шириной окна
     { if( W==Site->Width )W-=X; else X=max( 0,Site->Width-W ); }
// затем по вертикали
   if( !H )H=Site->Height; else if( H<0 )H=-H,fh=1; else H=H*fh; ++H;
   if( fh>1 ){ if( Y>0 )Y = Site->Height - H - fh*(Y-1); else Y = -Y*fh; }
       else { if( Y<=0 )Y = Site->Height - H + Y; }
   if( H>Site->Height )H=Site->Height;            // относительно главного окна
   if( Y+H>Site->Height )Y=Site->Height-H; if( Y<0 )Y=0;
// if( Signs & PlaceAbove )
   if( Img )   // если ранее сохранялась фоновая подложка, то восстанавливается
   if( X!=pX || Y!=pY || Width!=W || Height!=H )Rest();  // растянутая картинка
   pX=X,pY=Y,Width=W,Height=H;
   if( chY>Height-AF.H )chY=Height-AF.H;            // аппликата верхней строки
// if( Signs & PlaceAbove )Save();
   return *this;
}
//      Комплекс встроенных и внутренних процедур
//      Подготовка площадки для растровых манипуляций
//
RasterSector::RasterSector( int X,int Y, int W,int H ) // настройка чисто
  { PushMatrix();
    glLoadIdentity(); glMatrixMode( GL_PROJECTION );
    glLoadIdentity(); glViewport( X,Y,W,H );
    glOrtho( 0,W,0,H,-1,1 ); glMatrixMode( GL_MODELVIEW );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );
  }
TextContext::TextContext( bool b ): Base( b )
//    dp( glIsEnabled( GL_DEPTH_TEST ) ),
//    cf( glIsEnabled( GL_CULL_FACE ) ),
//    lh( glIsEnabled( GL_LIGHTING ) )
{ if( Base )PushMatrix(); else glPushAttrib( GL_ENABLE_BIT|GL_POLYGON_BIT );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_CULL_FACE );
  glDisable( GL_LIGHTING );
  glPolygonMode( GL_BACK,GL_FILL );
//glListBase( FontBase );
}
RasterSector::~RasterSector(){ PopMatrix(); }
TextContext::~TextContext(){ if( Base )PopMatrix(); else glPopAttrib(); }

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
{ if( Site )
  { RasterSector Sv( pX,pY,Width,Height ); // фонового буфера в видимый рисунок
    glRasterPos2i( 0,0 );                        glDrawBuffer( GL_FRONT );
    glCopyPixels( pX,pY,Width,Height,GL_COLOR ); glDrawBuffer( GL_BACK );
  //if( Site->Up==this )Site->Save();      // первая площадка сохраняет фоновую
    if( Signs&PlaceAbove && Site!=this )Save();// на фоне идёт сборная картинка
    WinExecute( Site->hWnd ); glFinish();
  } return *this;
}
//!    сохранение текущего изображения/фрагмента в буфере оперативной памяти
//        Img[0] - общая, контрольная длина массива
//        Img[1] x Img[2] x Img[3] - x·y·z - трехмерные размерности изображения
Place& Place::Save()                   // фрагментация здесь вполне естественна
{ if( Width>0 && Height>0 )            // и не требует особой работы со списком
  { unsigned Size=sizeof( unsigned )*( Width*Height+4 );
  //glPushAttrib( GL_VIEWPORT_BIT );
    glViewport( 0,0,Site->Width,Site->Height );   // возврат размерений полного
    if( !Img )(Img=(unsigned*)malloc(Size))[0]=Size; else        // окна Window
    if( Size>Img[0] )(Img=(unsigned*)realloc(Img,Size))[0]=Size; // приращение
    if( Img )                                        // памяти на всякий случай
    { Img[3]=0; glReadPixels
      ( pX,pY,Img[1]=Width,Img[2]=Height,GL_RGBA,GL_UNSIGNED_BYTE,Img+4 );
    } glViewport( pX,pY,Width,Height ); // <=> glPopAttrib();
  } return *this;
}
Place& Place::Rest()     //! прямое восстановление растра из собственной памяти
{ if( Img )              //  в фоновый буфер изображения с перемасштабированием
  { RasterSector Sv( pX,pY,Width,Height ); glRasterPos2i( 0,0 );
    if( Width!=int( Img[1] ) || Height!=int( Img[2] ) )
     glPixelZoom( float( Width )/Img[1],float( Height )/Img[2] );
    glDrawPixels( Img[1],Img[2],GL_RGBA,GL_UNSIGNED_BYTE,Img+4 );
    glPixelZoom ( 1.0,1.0 );
  } glFlush(); return *this;
}
//   восстановление картинки для всех фрагментов с опцией PlaceAbove+(Img)
//
Window& Window::Refresh()    // сборка изображения с копий в оперативной памяти
{ glContext Set( this ); Clear();           // glAdjust( this ) => навсегда
  for( Place *S=(Place*)this; S; S=S->Up )S->Rest();  // если есть Show()
  Show();                                   // только обновление новой картинки
//SwapBuffers( hDC );                       // возможно так значительно быстрее
  glFlush(); WinExecute( hWnd );            // картинка исполнена и - сохранена
  return *this;                             // и больше ничего рисовать не надо
}
Place& Place::Refresh(){ Site->Refresh(); return *this; }// туда же на всё окно
Window& Window::Above(){ SetForegroundWindow( hWnd ); return Refresh(); }
//
//!   Работа с текстами в стиле Microsoft-Windows
//                                растровый шрифт DOS-OEM(866)alt из эпохи CCCP
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
   glListBase( base ); glCallLists( strlen( str ),GL_UNSIGNED_BYTE,str );
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
    { if( cbX<=0 )chX-=strlen(str)*AF.W;
      glRasterPos2i( chX,chY ); _OutBitText( str,AF.Bit,chX,chY,cbX ); } else
#if 0                         /// так должно быть, но работает только в Intel
    { if( cbX<=0 )chX-=AlfaRect( str,true ).cx;
      glRasterPos2i( chX,chY ); _OutText( str,AF.Base,chX,chY,cbX,pX );
#else                         // небольшая перепутаница в AMD-OpenGL - что жаль
    { long l=AlfaRect( str,true ).cx;  if( cbX<=0 )chX-=l; int hk=chX;
      glRasterPos2i(chX,chY); _OutText( str,AF.Base,chX,chY,cbX,pX );
      if( chX==hk )chX+=l+1; //
#endif
    }
    if( !w )break; w[0]='\n'; chY-=AF.H; str=w+1;     //  будет еще одна строка
    if( cbX>0 )chX=(cbX-1)*AF.W; else chX=Width+AF.W*cbX;
  } return *this;
}
//!  Прорисовка плоско/горизонтального текста по предварительному определению
//!       размеров букв и смещением надписи относительно точки в плоскости XY
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
         else  _OutText( str,AF.Base,dx,dy,1,pX ); return *this;
}
#include <stdio.h>   // блок для четырёх вариантов текстовых надписей в формате
#include <stdarg.h>  // с реентерабельной\стековой подстрочкой переменной длины
#define Arg va_list a; va_start( a,fmt ); int l=vsnprintf( 0,0,fmt,a )+1; \
              { char s[l]; vsnprintf( s,l,fmt,a ); va_end( a ); String
#define Str ; } return *this; // просто с "временной" строчкой на всё и про всё
Place& Place::Print( const char *fmt,... ){ Arg( s )Str }
Place& Place::Print( int x,int y, const char *fmt,... )
{ cbX=x; chY=y>0 ? Height-AF.H*y : 3-AF.H*y;  // запоминается начальная позиция
  if( x>0 )chX=(x-1)*AF.W; else chX=Width+AF.W*x; Arg( s )Str
}
Place& Place::Text( Course Dir, const Real *P, const char* fmt, ... )
     { Arg( Dir,P,s )Str }
Place& Place::Text( Course Dir, _Real X,_Real Y,_Real Z, const char* fmt, ... )
     { const Real R[]={X,Y,Z }; Arg( Dir,R,s )Str }
