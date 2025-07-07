//
//! Подборка независимых от среды программирования транзакций в OpenGL
//! (пере)Установка размеров рабочей площадки/фрагмента внутри окна Window
//   если X,Y > 0 - отсчеты от левого верхнего угла, иначе - правого и нижнего
//   если Width,Height > 0 - отсчеты в символах, если 0 - до самой границы окна
//                если < 0 - в пикселях и нормальных отсчетах, Y - снизу вверх
//     отсутствие шрифта - подключение окна (не приводит к размеру точки [1x1])
//
Place& Place::Area( int X,int Y, int W,int H )     // достаточно сделать шаг
{ Real fw=AlfaWidth(),fh=AlfaHeight(),Tw=fw,Th=fh; // единичным, далее по Place
// всё по горизонтали glAct( Site );      // среда opеnGL здесь не используется
   if( !W )W=Site->Width;  else if( W<0 )W=-W,fw=1; else W=W*fw; // связанной с
   if( X>0 )X=(X-1)*fw; else X = Site->Width - W + X*fw;      // базовой Window
       X=minmax( 0,X,Site->Width-int( Tw ) );         // перерасчет с проверкой
   if( X+W>Site->Width  )                             //  жертвуем шириной окна
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

static void PushMatrix()     //! сброс в стек координатных систем OpenGL-Window
{ glPushAttrib( GL_VIEWPORT_BIT | GL_POLYGON_BIT | GL_ENABLE_BIT ); // что надо
  glPushMatrix(),glMatrixMode( GL_PROJECTION ),glPushMatrix(),
                 glMatrixMode( GL_MODELVIEW );
}
static void PopMatrix()          //! и затем восстановление исходного состояния
{ glMatrixMode( GL_PROJECTION ),glPopMatrix(),
  glMatrixMode( GL_MODELVIEW ),glPopMatrix(),glPopAttrib(); // обратный порядок
}
RasterSector::RasterSector
  ( int pX,int pY, int W,int H, Real Scale )  // настройка чисто
  { PushMatrix(); glPushAttrib( GL_ENABLE_BIT );
    glLoadIdentity(); glMatrixMode( GL_PROJECTION ); //cX=cY=0;
    glLoadIdentity(); glViewport( pX,pY,W,H );
    glOrtho( 0,W/Scale,0,H/Scale,0,1 );   // gluOrtho2D( 0,W/Scale,0,H/Scale );
    glMatrixMode( GL_MODELVIEW );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );
  }
RasterSector::~RasterSector(){ PopMatrix(); glPopAttrib(); }

TextContext::TextContext( bool b ): Base( b )
//    dp( glIsEnabled( GL_DEPTH_TEST ) ),
//    cf( glIsEnabled( GL_CULL_FACE ) ),
//    lh( glIsEnabled( GL_LIGHTING ) )
{ if( Base )PushMatrix(); else glPushAttrib( GL_ENABLE_BIT|GL_POLYGON_BIT );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_CULL_FACE );
  glDisable( GL_LIGHTING );
  glPolygonMode( GL_BACK,GL_FILL );
}
TextContext::~TextContext(){ if( Base )PopMatrix(); else glPopAttrib(); }

Place& Place::Clear( bool back ) // очистка фоном/true или текущим/false цветом
{ //if( glAct( Site ) )              // есть окно - есть картинка
  { glScissor( pX,pY,Width,Height ); // Режим обрезки внешнего окружения и его
    glEnable ( GL_SCISSOR_TEST );   // временное включение для очистки площадки
    if( back )glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); else
    { float c[8]; glGetFloatv( GL_COLOR_CLEAR_VALUE,c );
                  glGetFloatv( GL_CURRENT_COLOR,c+4 );
                  glClearColor( c[4],c[5],c[6],c[7] );
                  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                  glClearColor( *c,c[1],c[2],c[3] );
    } //glScissor( 0,0,0,0 );         // разблокирование графических фрагментов
    glDisable( GL_SCISSOR_TEST );     //   и отключение обрезки
  } return *this;
}
Place& Place::Show()                     //! прорисовка растрового фрагмента из
{ if( glAct( Site ) )                    // фонового буфера в видимый рисунок
  { RasterSector Sv( pX,pY,Width,Height );       // glFinish();
    glRasterPos2i( 0,0 );                        glDrawBuffer( GL_FRONT );
    glCopyPixels( pX,pY,Width,Height,GL_COLOR ); glDrawBuffer( GL_BACK );
    glFlush(); // glFinish();
  //if( Site->Up==this )Site->Save();      // первая площадка сохраняет фоновую
    if( Signs&PlaceAbove && Site!=this )Save();// на фоне идёт сборная картинка
  } return *this;
}
//!    сохранение текущего изображения/фрагмента в буфере оперативной памяти
//        Img[0] - общая, контрольная длина массива
//        Img[1] x Img[2] x Img[3] - x·y·z - трехмерные размерности изображения
Place& Place::Save()                   // фрагментация здесь вполне естественна
{ if( Width>0 && Height>0 )            // и не требует особой работы со списком
  if( glAct( Site ) )
  { unsigned Size=sizeof( unsigned )*( Width*Height+4 ); // glAct( Site );
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
{ if( Img )    //  в фоновый буфер изображения с перемасштабированием
  { if( glAct( Site ) )
    { RasterSector Sv( pX,pY,Width,Height ); glRasterPos2i( 0,0 );
      if( Width!=int( Img[1] ) || Height!=int( Img[2] ) )
        glPixelZoom( float( Width )/Img[1],float( Height )/Img[2] );
      glDrawPixels( Img[1],Img[2],GL_RGBA,GL_UNSIGNED_BYTE,Img+4 );
      glPixelZoom ( 1.0,1.0 ); glFlush();
  } } return *this;
}
//glEnable( GL_SCISSOR_TEST ); glScissor( pX,pY,W,H );
//glDisable( GL_SCISSOR_TEST );
//glListBase( FontBase );
