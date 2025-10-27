                     //
#include "View.h"    // Очередная отработка элементарных графических примитивов                     //                                  ©2018-08-22 ‏יְרוּשָׁלַיִםconst char*_Mnt[]={"январь","февраль","март","апрель","май","июнь","июль","август","сентябрь","октябрь","ноябрь","декабрь"},*_Day[]={"понедельник","вторник","среда","четверг","пятница","суббота","воскресенье"};static union{ unsigned u; byte c[4]; } // чистый цвет '33=51,'66=102,'99=153,'CC=204
SeaColor[black+257] =    /* переопределение расцветки подобно как в палитре-256
 {255,255,255},{192,192,192},{160,160,160},{128,128,128},{96,96,96},{64,64,64},
 {204,221,238},{255,255,0},{0,128,0},{0,255,0},{160,255,64},{64,255,96},
 {0,0,128},{0,0,255},{64,96,255},{0,160,160},{0,255,255},{96,212,212},
 {128,0,0},{255,0,0},{255,96,96},{255,128,0},{255,192,204},
 {128,0,128},{192,0,192},{255,0,255},{255,96,255},{0,0,0} */
{ 0xFFFFFF,0xC0C0C0,0xA0A0A0,0x808080,0x606060,0x404040,// white silver lightgray gray dimgray darkgray
  0xEEDDCC,0x00FFFF,0x008000,0x00FF00,0x40FFA0,0x60FF40,// freeboard yellow green lime olive lightgreen
  0x800000,0xFF0000,0xFF6040,0xA0A000,0xFFFF00,0xD4D460,// navy blue lightblue cyan aqua lightcyan
  0x000080,0x0000FF,0x6060FF,0x0080FF,0xCCC0FF,         // maroon red lightred orange pink
  0x800080,0xC000C0,0xFF00FF,0xFF60FF,0x000000          // purple magenta fuchsia lightmagenta black
};                                                      // = 28\{29}
void color( const colors clr ){ glColor4ubv((GLubyte*)(SeaColor[clr].c));}// alfa=255
void color( const colors clr,_Real b,_Real a ) // bright:-1 на черный; +1 до белого
#define B( c )((b<0?(c*(1+b)):(c+(255-c)*b))/255) // ...с затенением\подсветкой
{ byte *C=SeaColor[clr].c; glColor4d( B(C[0]),B(C[1]),B(C[2]),a ); }// alfa:0÷1
#undef B
//
//   тонкая линия из точки (a) в точку (b)  ... в однородных координатах OpenGL//const Real* dot( const Real* a, const colors clr ){ if( clr!=empty )color( clr ); glVertex3dv(a); return a; } // осторожная точкаconst Real* spot( const Real* a,_Real Size, const colors clr ) // и повторение{ glPushAttrib( GL_POINT_BIT ); glPointSize( (float)Size );
  glBegin( GL_POINTS ); if( clr!=empty )color( clr ); glVertex3dv(a); glEnd();
  glPopAttrib(); return a;
}
const Real* line( const Real* a, const Real* b )
{ glBegin( GL_LINES ),glVertex3dv( a ),glVertex3dv( b ),glEnd(); return b; }
const Real* line( const Real* a, const Real* b, const colors clr ){ if( clr!=empty )color( clr ); line( a,b ); return b; }void liney( const Real* a,const Real* b, const colors clr )     // две линии{ if( clr!=empty )color( clr );                                 // по ординатам  glBegin( GL_LINES ); glVertex3dv( a ); glVertex3dv( b );         glVertex3d( a[0],-a[1],a[2] ); glVertex3d( b[0],-b[1],b[2] ); glEnd();}                                                         // 32 секторных румбаconst Real * circle( const Real *a, _Real r, bool fill )  //    в плоскости X-Y
{ glBegin( fill?GL_POLYGON:GL_LINE_LOOP ); for( Real q=0; q<_Pd; q+=_Ph/8 )
  glVertex3d( a[0]+r*sin( q ),a[1]+r*cos( q ),a[2] ); glEnd(); return a;
}
void rectangle( const Real *LD,const Real *RU,bool fill )  // прямоугольник X-Y
{ glBegin( fill?GL_QUADS:GL_LINE_LOOP ),
  glVertex3dv( LD ),glVertex3d( RU[0],LD[1],LD[2] ),
  glVertex3dv( RU ),glVertex3d( LD[0],RU[1],RU[2] ),glEnd();
}
View::View( const char* Tt, int X,int Y, int W,int H, _Real Size )
    : Window( Tt,X,Y,W,H ), eyeX( -130 ),eyeY( -10 ),eyeZ( 0 ),
                           lookX( 0 ),lookY( 0 ),lookZ( 0 ),
    Distance( Size?-Size:-.8*Width ),   // расстояние от камеры до места съёмки
    mx( 0 ),my( 0 )                     //  указатель мышки в нулевое положение
  { View_initial( Size>0?Size:1 );
//  glFogf( GL_FOG_START,-Distance/3 );
//  glFogf( GL_FOG_END,Distance/3 );
  }
bool View::Draw(){ Activate();       // с исключением двойной перенастройки
  glMatrixMode( GL_PROJECTION );     // размерах фрагмента экрана сброс текущих
  glLoadIdentity();                  // матричных координат, настройка обзора и
  gluPerspective( 16.2,Real( Width )/Height,-1,1 );  // экранных пропорций
  gluLookAt( 0,0,Distance,lookX,lookY,lookZ,0,1,0 ); // местоположение сцены
  glMatrixMode( GL_MODELVIEW ); glLoadIdentity();    // в исходное положение
  glRotated( eyeY-90,1,0,0 );                        // поставить на киль
  glRotated( eyeZ,   0,1,0 );                        // дифферент
  glRotated( eyeX,   0,0,1 );                        // рыскание
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
            eyeZ+=Real( x-mx )/24,Distance-=( y-my )*Distance/Height; else
            eyeY-=Real( y-my )/24,eyeX+=Real( x-mx )/16; break;
      case _MouseRight: lookX-=( x-mx )*Distance/Width,            // смещение
                        lookY-=( y-my )*Distance/Width;
    } my=y,mx=x;
  } if( ret )return( Draw() );
  return Place::Mouse( button,x,y );
}
bool View::KeyBoard( fixed key )   // к спуску из внешних виртуальных транзакций
{ static Real Di=0; if( !Di )Di=Distance;  // запоминается из первого обращения
         Real Ds=6*Distance/Width;
  switch( key )
  { case _Left: if( ScanStatus()&SHIFT )lookX+=Ds; else
                if( ScanStatus()&CTRL  )eyeZ--;    else eyeX--; break;
    case _Right:if( ScanStatus()&SHIFT )lookX-=Ds; else
                if( ScanStatus()&CTRL  )eyeZ++;    else eyeX++; break;
    case _Up:   if( ScanStatus()&CTRL  )Distance*=1.1; else
                if( ScanStatus()&SHIFT )lookY+=Ds; else eyeY++; break;
    case _Down: if( ScanStatus()&CTRL  )Distance/=1.1; else
                if( ScanStatus()&SHIFT )lookY-=Ds; else eyeY--; break;
    case _Home: Distance=Di,             // ...от точки взгляда до места обзора
          eyeX=-130,eyeY=-10,eyeZ=0; lookX=-1,lookY=-1,lookZ=0; break;
   default: return Window::KeyBoard(key);// передача в цикл ожидания клавиатуры
  } Draw(); return true;                 // либо - запрос от клавиатуры погашен
}
// Настройка начальной раскраски и освещенности графического пространства/сцены
//              ... здесь необходима предварительная установка контекста OpenGLvoid View_initial( Real D ){  glClearColor( 0.9,0.95,0.99,1 );   // светлый оттенок экранного фона и затем   glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT ); // полная расчистка окна//        | GL_ACCUM_BUFFER_BIT|GL_STENCIL_BUFFER_BIT );
//+glFrontFace( GL_CCW );              // CW грани с обходом по часовой стрелке
//+glCullFace ( GL_BACK );             // FRONT_AND_BACK какие отбираются грани
//+glEnable   ( GL_CULL_FACE );        // включение режима отбора треугольников   glHint     ( GL_POINT_SMOOTH_HINT,GL_NICEST );   glEnable   ( GL_POINT_SMOOTH );   glPointSize( 1.0 );   glHint     ( GL_LINE_SMOOTH_HINT,GL_NICEST );   glEnable   ( GL_LINE_SMOOTH );       // сглаживание линий   glLineWidth( 1.0 );
   glPolygonMode( GL_FRONT,GL_FILL );
   glPolygonMode( GL_BACK,GL_LINE );    // POINT );
   glEnable   ( GL_POLYGON_SMOOTH);     // Really Nice Perspective Calculations   glHint     ( GL_POLYGON_SMOOTH_HINT,GL_NICEST );   glHint     ( GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST );   glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA );
   glShadeModel( GL_SMOOTH );       // FLAT закраска с использованием полутонов
   glAlphaFunc( GL_ALWAYS,0 );
   glEnable( GL_BLEND );                // Включаем смешивание
   glEnable( GL_DITHER );               // Предопределение графической среды
   glEnable( GL_ALPHA_TEST );
   glFogi( GL_FOG_MODE,GL_EXP2 );       //  GL_EXP2
   glFogf( GL_FOG_DENSITY,1.0/D );      //   0.0016
   glFogf( GL_FOG_START,-D );           // -Distance
   glFogf( GL_FOG_END,D );              // +Distance
   glHint( GL_FOG_HINT,GL_NICEST );
   glEnable( GL_FOG );                  // glEnable( GL_STENCIL_TEST );
   glLightModeli( GL_LIGHT_MODEL_TWO_SIDE,true );
// glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER,true );
// glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR );
// glLightModelfv( GL_LIGHT_MODEL_AMBIENT,(const float[]){.2,.2,.2,1} );  //!??
// glColorMaterial( GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE );
// glMateriali( GL_FRONT_AND_BACK,GL_SHININESS,255 );   // 255 степень отсветки
// glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT, (const float[]){.2,.2,.2,1}); //{1,1,1,0}
// glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE, (const float[]){.4,.4,.4,1}); //{1,1,1,0}
// glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,(const float[]){.6,.6,.6,1}); //{1,1,1,0}
// glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,(const float[]){.1,.1,.1,1}); //{0,0,0,1}
// glLightfv( GL_LIGHT0,GL_AMBIENT,           (const float[]){.3,.3,.3,1}); // окружение
// glLightfv( GL_LIGHT0,GL_DIFFUSE,           (const float[]){.4,.4,.4,1}); // рассеяние
// glLightfv( GL_LIGHT0,GL_SPECULAR,          (const float[]){.5,.5,.5,1}); // отражение
// glLightfv( GL_LIGHT0,GL_EMISSION,          (const float[]){.6,.6,.6,1}); // излучение
// glLightfv( GL_LIGHT0,GL_POSITION,          (const float[]){ D,-D,-D,1});
// glLightfv( GL_LIGHT0,GL_SPOT_DIRECTION,    (const float[]){-D, D, D,1});
// glLighti ( GL_LIGHT0,GL_SPOT_CUTOFF,90 );
// glLightfv( GL_LIGHT1,GL_AMBIENT,           (const float[]){.3,.3,.3,1}); // окружение
// glLightfv( GL_LIGHT1,GL_DIFFUSE,           (const float[]){.4,.4,.4,1}); // рассеяние
// glLightfv( GL_LIGHT1,GL_SPECULAR,          (const float[]){.5,.5,.5,1}); // отражение
// glLightfv( GL_LIGHT1,GL_EMISSION,          (const float[]){.6,.6,.6,1}); // излучение
// glLightfv( GL_LIGHT1,GL_POSITION,          (const float[]){-D,-D,-D,1});
// glLightfv( GL_LIGHT1,GL_SPOT_DIRECTION,    (const float[]){ D, D, D,1});
// glLighti ( GL_LIGHT1,GL_SPOT_CUTOFF,90 );
//! Рендеринг
#define Light( I )                                                  \
   glLightfv( GL_LIGHT##I,GL_AMBIENT, (const float[]){.0,.0,.0,1}); \
   glLightfv( GL_LIGHT##I,GL_DIFFUSE, (const float[]){.5,.5,.5,1}); \
   glLightfv( GL_LIGHT##I,GL_SPECULAR,(const float[]){.8,.8,.8,1}); \
   glLighti ( GL_LIGHT##I,GL_SPOT_CUTOFF,45 );
   Light( 0 ) glLightfv( GL_LIGHT0,GL_POSITION,      (const float[]){0,0,1,0 });
              glLightfv( GL_LIGHT0,GL_SPOT_DIRECTION,(const float[]){0,0,-1  });
   Light( 1 ) glLightfv( GL_LIGHT1,GL_POSITION,      (const float[]){0,0,-1,0});
              glLightfv( GL_LIGHT1,GL_SPOT_DIRECTION,(const float[]){0,0,1   });
   Light( 2 ) glLightfv( GL_LIGHT2,GL_POSITION,      (const float[]){1,0,0,0 });
              glLightfv( GL_LIGHT2,GL_SPOT_DIRECTION,(const float[]){-1,0,0  });
   Light( 3 ) glLightfv( GL_LIGHT3,GL_POSITION,      (const float[]){-1,0,0,0});
              glLightfv( GL_LIGHT3,GL_SPOT_DIRECTION,(const float[]){ 1,0,0  });
   glEnable( GL_COLOR_MATERIAL );
   glEnable( GL_LIGHTING );
   glEnable( GL_LIGHT0 ); glEnable( GL_LIGHT1 );
   glEnable( GL_LIGHT2 ); glEnable( GL_LIGHT3 );
   glEnable( GL_DEPTH_TEST );        // растровая разборка отсечений по глубине
   glDepthMask( GL_TRUE );   glClearDepth( 2e3 );  // 2000.0 );   // Enables Clearing Of The Depth Buffer
   glDepthRange( 1,0 );  // 0,1 - Distance           взаимное накрытие объектов
   glDepthFunc( GL_LEQUAL );//~EQUAL~GEQUAL GREATER LEQUAL NOTEQUAL LESS ALWAYS
   glEnable( GL_NORMALIZE );   glEnable( GL_AUTO_NORMAL );
   if( !SeaColor[black+1].u ){ for( int i=0; i<256; i++ )/*   SeaColor[1+black+i].c[0]=byte(   pow(Real(i)/255,4)*180), // red   красный
     SeaColor[1+black+i].c[1]=byte(48+pow(Real(i)/255,2)*162), // green зеленый
     SeaColor[1+black+i].c[2]=byte(96+pow(Real(i)/255,3)*94 ); // blue  синий
*/   SeaColor[1+black+i].c[0]=byte(    pow(Real(i)/255,4 )*210 ), //+ 210 red   красный
     SeaColor[1+black+i].c[1]=byte( 96+pow(Real(i)/255,.8)*134 ), //+ 230 green зеленый
     SeaColor[1+black+i].c[2]=byte(192+pow(Real(i)/255,3 )*63 );  //+ 255 blue  синий
     for( int i=0; i<black+256; i++ )SeaColor[i].c[3]=0xFF;      // alfa-сплошной
   }
}