                     //
#include "View.h"    // Очередная отработка элементарных графических примитивов
SeaColor[black+257] =    // переопределение расцветки подобно как в палитре-256
{ {255,255,255},{192,192,192},{160,160,160},{128,128,128},{96,96,96},{64,64,64},// white silver lightgray gray dimgray darkgray
  {255,255,0},{0,128,0},{0,255,0},{160,255,64},{64,255,96},                     // yellow green lime olive lightgreen
  {0,0,128},{0,0,255},{64,96,255},{0,160,160},{0,255,255},{96,212,212},         // navy blue lightblue cyan aqua lightcyan
  {128,0,0},{255,0,0},{255,96,96},{255,128,0},{255,192,204},                    // maroon red lightred orange pink
  {128,0,128},{192,0,192},{255,0,255},{255,96,255},{0,0,0}                      // purple magenta fuchsia lightmagenta black
//{ 0xFFFFFF,0xC0C0C0,0xA0A0A0,0x808080,0x606060,0x404040,
//  0x00FFFF,0x008000,0x00FF00,0x40FFA0,0x60FF40,
//  0x800000,0xFF0000,0xFF6040,0xA0A000,0xFFFF00,0xD4D460,
//  0x000080,0x0000FF,0x6060FF,0x0080FF,0xCCC0FF,
//  0x800080,0xC000C0,0xFF00FF,0xFF60FF,0x000000
};                                              // = 27\{28}
void color( colors clr ){ glColor4ubv((GLubyte*)(SeaColor[clr].c));}// alfa=255
void color( colors clr,_Real b,_Real a )  // bright: -1 на черный; +1 до белого
#define B( c )((b<0?(c*(1+b)):(c+(255-c)*b))/255) // ...с затенением\подсветкой
{ byte *C=SeaColor[clr].c; glColor4d(B(C[0]),B(C[1]),B(C[2]),a ); } // alfa:0-1
#undef B
//   тонкая линия из точки (a) в точку (b)  ... в однородных координатах OpenGL
  glPointSize( (float)Size ); glBegin( GL_POINTS ); dot( a,color ); glEnd();
  glPopAttrib(); return a;
}
const Real * line( const Real* a, const Real* b )
{ glBegin( GL_LINES ),glVertex3dv( a ),glVertex3dv( b ),glEnd(); return b; }
const Real * line( const Real* a, const Real* b, colors clr )
{ glBegin( fill?GL_POLYGON:GL_LINE_LOOP ); for( Real q=0; q<_Pd; q+=_Ph/8 )
  glVertex3d( a[0]+r*sin( q ),a[1]+r*cos( q ),a[2] ); glEnd(); return a;
}
void rectangle( const Real *LD,const Real *RU,bool fill ) // прямоугольник X-Y
{ glBegin( fill?GL_QUADS:GL_LINE_LOOP ),
  glVertex3dv( LD ),glVertex3d( RU[0],LD[1],LD[2] ),
  glVertex3dv( RU ),glVertex3d( LD[0],RU[1],RU[2] ),glEnd();
}
//    ... из точки (a) в точку (b) с объемной стрелкой в долях длины
                           line( a,d+=b,clr );
  const char *x,const char *y,const char *z,colors clr )
  P.Text( _North,0,0,Z,z )
   .Text( _North,0,Y+l,0,y )
   .Text( _North_East,L+l,0,0,x );
}
View::View( const char* Tt, int X,int Y, int W,int H, _Real Size )
    : Window( Tt,X,Y,W,H ),
    eye( (Vector){ -130,-10,0 } ),look( (Vector){ 0,0,30 } ),
    Distance( Size?-Size:-.8*Width ),   // расстояние от камеры до места съёмки
    mx( 0 ),my( 0 )                     //  указатель мышки в нулевое положение
  { View_initial();
//  glFogf( GL_FOG_START,-Distance );
//  glFogf( GL_FOG_END,Distance );
  }
bool View::Draw(){ Activate();       // с исключением двойной перенастройки
  glMatrixMode( GL_PROJECTION );     // размерах фрагмента экрана сброс текущих
  glLoadIdentity();                  // матричных координат, настройка обзора и
  gluPerspective( 16.2,Real( Width )/Height,-1,1 );     // экранных пропорций
  gluLookAt( 0,0,Distance,look.x,look.y,look.z,0,1,0 ); // местоположение сцены
  glMatrixMode( GL_MODELVIEW ); glLoadIdentity();       // в исходное положение
  glRotated( eye.y-90,1,0,0 );                          // поставить на киль
  glRotated( eye.z,   0,1,0 );                          // дифферент
  glRotated( eye.x,   0,0,1 );                          // рыскание
  return Window::Draw();
}
//   Небольшой блок реагирования на мышиную возню в текущем окне
//
bool View::Mouse( int button, int x,int y )    // и её же указания к исполнению
{ bool ret=false;                              // Activate();
  if( button==_MouseWheel )                    // на вращение колёсико/сенсора
  { if( ret=(y!=0) )Distance+=y*Distance/Height/3; } else
  if( ret=(x!=mx || y!=my) )    // местоположение курсора в локальных смещениях
  { switch( button )            //  -- и не пустой вызов без свободной реакции
    { case _MouseLeft: if( ScanStatus()&CTRL )
            eye.z+=Real( x-mx )/24,Distance-=( y-my )*Distance/Height; else
            eye.y-=Real( y-my )/24,eye.x+=Real( x-mx )/16; break;
      case _MouseRight: look.x-=( x-mx )*Distance/Width,            // смещение
                        look.y-=( y-my )*Distance/Width;
    } my=y,mx=x;
  } if( ret )return( Draw() );
  return Place::Mouse( button,x,y );
}
bool View::KeyBoard( byte key )   // к спуску из внешних виртуальных транзакций
{ static Real Di=0; if( !Di )Di=Distance;  // запоминается из первого обращения
         Real Ds=6*Distance/Width;
  switch( key )
  { case _Left: if( ScanStatus()&SHIFT )look.x+=Ds; else
                if( ScanStatus()&CTRL  )eye.z--;    else eye.x--; break;
    case _Right:if( ScanStatus()&SHIFT )look.x-=Ds; else
                if( ScanStatus()&CTRL  )eye.z++;    else eye.x++; break;
    case _Up:   if( ScanStatus()&CTRL  )Distance*=1.1; else
                if( ScanStatus()&SHIFT )look.y+=Ds; else eye.y++; break;
    case _Down: if( ScanStatus()&CTRL  )Distance/=1.1; else
                if( ScanStatus()&SHIFT )look.y-=Ds; else eye.y--; break;
    case _Home: Distance=Di,             // ...от точки взгляда до места обзора
            eye=(Vector){ -130,-10,0 },look=(Vector){ 0,0,30 }; break;
   default: return Window::KeyBoard(key);// передача в цикл ожидания клавиатуры
  } Draw(); return true;                 // либо - запрос от клавиатуры погашен
}
// Настройка начальной раскраски и освещенности графического пространства/сцены
//              ... здесь необходима предварительная установка контекста OpenGL
//+glCullFace ( GL_BACK );             // FRONT_AND_BACK какие отбираются грани
//+glEnable   ( GL_CULL_FACE );        // включение режима отбора треугольников
   glHint     ( GL_LINE_SMOOTH_HINT,GL_NICEST );
   glPolygonMode( GL_BACK,GL_LINE ); //POINT );
   glShadeModel( GL_SMOOTH );       // FLAT закраска с использованием полутонов
   glHint     ( GL_POLYGON_SMOOTH_HINT,GL_NICEST );
   glAlphaFunc( GL_ALWAYS,0 );

   glEnable( GL_DITHER );               // Предопределение графической среды
   glEnable( GL_ALPHA_TEST );
   glEnable( GL_BLEND );

   glFogi( GL_FOG_MODE,GL_EXP2 );
   glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER,true );
   glLightModeli( GL_LIGHT_MODEL_TWO_SIDE,true );

   glMaterialfv( GL_FRONT_AND_BACK,GL_AMBIENT, (const float[]){.2,.2,.2,1.} );
   glMaterialfv( GL_FRONT_AND_BACK,GL_DIFFUSE, (const float[]){.8,.8,.8,1.} );
   glMaterialfv( GL_FRONT_AND_BACK,GL_SPECULAR,(const float[]){.5,.5,.5,.5} );
   glMaterialfv( GL_FRONT_AND_BACK,GL_EMISSION,(const float[]){.0,.0,.0,1.} );
   glMateriali( GL_FRONT_AND_BACK,GL_SHININESS,127 ); // степень отсветки

   glLightfv( GL_LIGHT0,GL_AMBIENT, (const float[]){.1,.3,.2,.6}); // окружение
   glLightfv( GL_LIGHT0,GL_DIFFUSE, (const float[]){.6,.8, 1, 1}); // рассеяние
   glLightfv( GL_LIGHT0,GL_SPECULAR,(const float[]){.8,.9, 1, 1}); // отражение
   glLightfv( GL_LIGHT0,GL_EMISSION,(const float[]){.6,.6,.6, 1}); // излучение
   glLightfv( GL_LIGHT0,GL_POSITION,(const float[]){-2000,20,-100,1});

   glLightfv( GL_LIGHT1,GL_AMBIENT, (const float[]){.1,.2,.3,.8}); // окружение
   glLightfv( GL_LIGHT1,GL_DIFFUSE, (const float[]){.8, 1, 1, 1}); // рассеяние
   glLightfv( GL_LIGHT1,GL_SPECULAR,(const float[]){.6,.8,.9, 1}); // отражение
   glLightfv( GL_LIGHT1,GL_EMISSION,(const float[]){.6,.6,.6, 1}); // излучение
   glLightfv( GL_LIGHT1,GL_POSITION,(const float[]){2000,100,10,1});
#endif
   glEnable( GL_COLOR_MATERIAL );
   glEnable( GL_LIGHT0 );
   glEnable( GL_LIGHT1 );
   glEnable( GL_LIGHTING );
   glDepthRange( 1,-1 );             // ( 0,1 )
   glDepthMask( GL_TRUE );
   glDepthFunc( GL_LEQUAL );         // LESS ALWAYS взаимное накрытие объектов
   glEnable( GL_DEPTH_TEST );        // растровая разборка отсечений по глубине
   glEnable( GL_AUTO_NORMAL );
} }