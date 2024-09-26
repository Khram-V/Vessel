//
//! Подборка независимых от среды программирования транзакций в OpenGL
//! (пере)Установка размеров рабочей площадки/фрагмента внутри окна Window
//   если X,Y > 0 - отсчеты от левого верхнего угла, иначе - правого и нижнего
//   если Width,Height > 0 - отсчеты в символах, если 0 - до самой границы окна
//                если < 0 - в пикселях и нормальных отсчетах, Y - снизу вверх
//     отсутствие шрифта - подключение окна (не приводит к размеру точки [1x1])
//
Place& Place::Area( int X,int Y, int W,int H )   // достаточно сделать шаг
{ Real fw=Tw,fh=Th;                         // единичным и далее по Place,
// всё по горизонтали
   if( !W )W=Site->Width;  else if( W<0 )W=-W,fw=1; else W=W*fw; // связанной с
   if( X>0 )X=(X-1)*fw; else X = Site->Width - W + X*fw;      // базовой Window
       X=minmax( 0,X,Site->Width-int( Tw ) );         // перерасчет с проверкой
   if( X+W>Site->Width  )                             // жертвуем шириной окна
     { if( W==Site->Width )W-=X; else X=max( 0,Site->Width-W ); }
// затем по вертикали
   if( !H )H=Site->Height; else if( H<0 )H=-H,fh=1; else H=H*fh; ++H;
   if( fh>1 ){ if( Y>0 )Y=Site->Height-H-fh*(Y-1); else Y=-Y*fh; }
        else { if( Y<=0 )Y=Site->Height-H+Y; }
   if( H>Site->Height )H=Site->Height;            // относительно главного окна
   if( Y+H>Site->Height )Y=Site->Height-H; if( Y<0 )Y=0;
// if( Signs & PlaceAbove )
   if( Img )   // если ранее сохранялась фоновая подложка, то восстанавливается
   if( X!=pX || Y!=pY || Width!=W || Height!=H )Rest();  // растянутая картинка
   pX=X,pY=Y,Width=W,Height=H+Th/5;
/* if( chY>Height-Th ) */ chY=Height-Th; chX=1;     // аппликата верхней строки
// if( Signs & PlaceAbove )Save();
   return *this;
}
//      Комплекс встроенных и внутренних процедур
//      Подготовка площадки для растровых манипуляций
//
//#include <GL/Glu.h>
RasterSector::RasterSector
  ( int pX,int pY, int W,int H, Real Scale )  // настройка чисто
  { PushMatrix();
    glLoadIdentity(); glMatrixMode( GL_PROJECTION ); //cX=cY=0;
    glLoadIdentity(); glViewport( pX,pY,W,H );
    glOrtho( 0,W/Scale,0,H/Scale,0,1 );
//  gluOrtho2D( cX-cX/Scale,W/Scale,
//              cY-cY/Scale,H/Scale );              // gluOrtho2D( 0,W,0,H );
    glMatrixMode( GL_MODELVIEW );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );
  }
RasterSector::~RasterSector(){ PopMatrix(); }

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
    glFlush(); //glFinish();
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
  glFlush();                                // картинка исполнена и - сохранена
  return *this;                             // и больше ничего рисовать не надо
}
Place& Place::Refresh(){ Site->Refresh(); return *this; }// туда же на всё окно
Window& Window::Above(){ SetForegroundWindow( hWnd ); return Refresh(); }
