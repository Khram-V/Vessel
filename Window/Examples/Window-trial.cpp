//
//      Простенький интерфейс с экранными окнами для графики
//      Проверяется параллельная работа с графикой по таймеру
//      в двух независимых окнах, на которые может накладываться
//      текстовый фрагмент с подсказкой F1.
//
#include <Omp.h>
#include <StdIO.h>
#include "../Window.h"
#include "../ConIO.h"
#include "../../Tools/Glut/Geometry.h"

void UtahTeaPot( float size, bool Solid=true ); // Renders a wired teapot ...
void UtahTeaCup( float size, bool Solid=true ); // Renders a wired teacup ...
void UtahTeaSpoon( float size, bool=true );     // и чайная ложечка
///
//! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
static void Rotation( float theta )
{ glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); glColor3f( 0.9,1,1 );
  glRotatef( theta+= 0.16f,sin( theta/200 ),cos( theta/100 ),1.0f );
  glTranslatef( sin( theta/200 )/3,cos( theta/10 )/2,sin( theta/150 )/3 );
  glutSolidCube( 0.25 );                      glColor3f( 0.3,0.6,1 );
  glutWireDodecahedron();             //      glutSolidDodecahedron();
  glutWireOctahedron();               //      glutSolidOctahedron();
  glutWireIcosahedron();              //      glutSolidIcosahedron();
  glutWireTetrahedron();              //      glutSolidTetrahedron();
                                              glColor3f( 1,1,1 );
  glutWireSphere( 0.24,32,18 );       //      glutSolidSphere( 0.1,32,32 );
  glPushMatrix();
  glTranslatef( 0,0,-1 );
  glutSolidCylinder( 0.025,2, 32,1 );         glColor3f( 0,1,0.5 );
  glutWireCylinder( 0.05,2.05,12,1 );
  glPopMatrix();                              glColor4f( 1,1,0.2,0.5 );
  glutWireCone( 0.6,-0.8,36,6 );              glColor3f( 0.4,1,0.6 );
 float offset[3]={-1.1f + float( sin(theta/4.4)/128 ),
                          float( sin(theta/1.3)/64) ,
                          float( cos(theta/2.4)/32 ) };
  glutSolidSierpinskiSponge( 4,offset,1 );    glColor3f( 0,1,1 );
  glutSolidTorus( 0.12,0.5,36,48 );
  glutSolidCone( 0.24,0.24,4,1 );
  glRotatef    ( 180,1,0,0 );
  glutSolidCone( 0.24,0.24,4,1 );
  glPopMatrix(),glPushMatrix(); /// +++
  glRotatef( theta+=0.16f,cos( theta/36 ),cos( theta/24 ),sin( theta/12 ) );
  glTranslatef( cos( theta/200 )/3,cos( theta/50 )/3,sin( theta/10 )/2 );
  glBegin( GL_TRIANGLES );
    glNormal3f( 0.0,  0.0, 1.0 ); glColor3f( 0.5,1.0,0.5 );
    glVertex3f( 0.87,-0.5,-0.1 ); glColor3f( 1.0,0.5,0.5 );
    glVertex3f( 0.0,  1.0, 0.1 ); glColor3f( 0.5,0.5,1.0 );
    glVertex3f(-0.87,-0.5,-0.1 );
  glEnd();                                          //  glCullFace( GL_FRONT );
  glScaled( 1,(3+cos( theta/3 ))/3.5,1 );
  glTranslatef( cos( theta/100 )/2,cos( theta/30 )/1.5,sin( theta/10 )/3 );
  switch( int( fabs( theta/100 ) )%3 )
  { case 0: glColor3f( 1,1,1 );      UtahTeaPot( 0.5 );
            glColor3f( 1,0.75,0.5 ); UtahTeaPot( 0.525,false ); break;
    case 1: glColor3f( 1,1,1 );      UtahTeaCup( 0.6 );
            glColor3f( 1,0.75,0.5 ); UtahTeaCup( 0.625,false );  break;
    case 2: glColor3f( 1,1,1 );      UtahTeaSpoon( 0.7 );
            glColor3f( 1,0.75,0.5 ); UtahTeaSpoon( 0.725,false );
  } glPopMatrix();               //  glCullFace( GL_FRONT_AND_BACK );
}
static void Visual3D_Lightening()
{ glClearColor( 1.0f,1.0f,0.8f,0.5f );
//glEnable( GL_CULL_FACE );
//glFrontFace( GL_CCW );      // CCW видимы грани с обходом по часовой стрелке
//glCullFace( GL_FRONT_AND_BACK );
//glPolygonMode( GL_FRONT,GL_FILL );
//glPolygonMode( GL_BACK,GL_POINT ); //LINE );
  glShadeModel( GL_SMOOTH );       // FLAT закраска с использованием полутонов
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS );
  glLightfv( GL_LIGHT0,GL_AMBIENT, (const float[]){.1,.1,.1,.1});//light_ambient
  glLightfv( GL_LIGHT0,GL_DIFFUSE, (const float[]){.5,.5,.5,.5});// цвет света
  glLightfv( GL_LIGHT0,GL_SPECULAR,(const float[]){.8,.8,.8,.5});//light_specular
  glLightfv( GL_LIGHT0,GL_POSITION,(const float[]){1,2,18,.5} );//light_position
  glMaterialfv(GL_FRONT,GL_AMBIENT,(const float[]){.7,.7,.7,.5});// mat_ambient
  glMaterialfv(GL_FRONT,GL_DIFFUSE,(const float[]){.8,.8,.8,.5});// mat_diffuse
  glMaterialfv(GL_FRONT,GL_SPECULAR,(const float[]){.5,.5,.5,.5});// mat_specular
  glMaterialfv(GL_FRONT,GL_SHININESS,(const float[]){ 100 } );// high_shininess
  glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER,true );         // видовая СК
  glLightModeli( GL_LIGHT_MODEL_TWO_SIDE,true );            // для обеих сторон
  glEnable( GL_LIGHT0 );
  glEnable( GL_LIGHTING );
  glEnable( GL_NORMALIZE );
  glEnable( GL_COLOR_MATERIAL );
}
const char
 *Nam1[]={ "Window"," One - внешний производный",
           "класс с запросом клавиатуры","и затем с работой по таймеру",0 },
 *Nam2[]={ "Second"," работа в прерываниях таймера",
           "с клавиатурой и площадками",0 },
 *Cmds[]={ "F1 ","краткая справка / текст в окошке",
           "F2-F12"," прочие настройки / процедуры",0 },
 *Plus[]={ "<Пробел>","расчистка",
           "<altC>  ","смена палитры",
           "<altX>/<ctrlC>"," стоп ",0 };
//
//     Первое окно ориентировано на виртуальные функции
//
struct _One: public Window
{ DWORD cw,dw,cnt;
  _One():Window( "One::First OpenGL window",100,100,800,600 ),dw(0),cnt(0)
        { Icon( "Flag" ); };
  virtual ~_One(){ print( 1,15,"One::Destructor \n" ); gotoxy( 1,21 ); }
  virtual bool KeyBoard( fixed key )
  { if( key==_F1 ){ Help( Nam1,Cmds,Plus ); return true; } return false;
  }                      // остальное в очередь ожиданий для get- или waitKey()
  virtual bool Timer()
  { Activate( true ); Rotation( Real(cw=ElapsedTime())/250.0 );  // WaitTime( 500 );
    glColor3f( 0,0,1 ); Print( 1,0,"One::счётчик кадров: № %d + %d ms",cnt,cw-dw );
    textcolor( LIGHTCYAN ); print( 1,12,"One::Timer[%d] = %d = %d \n",cnt,cw,cw-dw );
    dw=cw; ++cnt; Save(); return true; //->Save().Refresh();
  }
  virtual bool Draw(){ return Timer(); }
  virtual bool Mouse( int x,int y ){ print( 1,13,"One::Move( %d, %d ) \n",x,y ); return false; }
  virtual bool Mouse( int s,int x,int y ){ print( 1,14,"One::Press( %04X, %d, %d ) \n",s,x,y ); return false; }
} One;
//
//     Второй блок с освобожденные от графической среды функциями
//
static Window *Win2=NULL;    // случай свободной процедуры со ссылкой на Window
static bool FreeHelp( fixed Keyb )
{ if( Win2 && Keyb==_F1 ){ Win2->Help( Nam2,Cmds,Plus ); return true; }
                                                         return false; }
static bool SecondTimer()
{ static DWORD cw,dw=0,cnt=0;
  Win2->Activate( true ); Rotation( Real(cw=ElapsedTime())/-333.0 );
  glColor3f( 0,0.5,0 ); Win2->Print(1,0,"Ext::Счетчик кадров: № %d ",cnt );
  textcolor( LIGHTGREEN ); print(1,16,"Ext::SecondTimer[%d] = %d = %d \n",cnt,cw,cw-dw );
  dw=cw; ++cnt; //
  return true; // == Win2->Save().Refresh();
}
static bool MouseMove( int x,int y )           // мышки пишут только на консоли
{ print( 1,17,"Ext::Move: x=%d, y=%d  \n",x,y ); return false; }
static bool MousePress( int State, int x,int y )
{ print(1,18,"Ext::Press: State: %04X, x=%d, y=%d \n",State,x,y);return false;
}
//  сначала строится кинематика в цикле затем включаются таймеры для двух окон
//
int main( int argc, char **argv )
{ Real theta=0.0f; fixed Key=0; int oTv=0;
#pragma omp parallel
  { static int np=0;
    printf("Hello world: %d => Thread_num %d, num_threads %d, max_threads %d\n",
      np++,omp_get_thread_num(),omp_get_num_threads(),omp_get_max_threads() );
  }
  DISPLAY_DEVICE display_device;
  display_device.cb = sizeof(display_device);
  printf( "\nsizeof(DISPLAY_DEVICE) = %d\n",display_device.cb );
  for( int i=0; EnumDisplayDevices( NULL,i,&display_device,0 ); ++i )
  if( display_device.StateFlags )
  {                   // дальше выводите информацию из структуры display_device
    printf( "Display(%d), ",i+1 );   display_device.DeviceKey[69]=0;
    printf( "DeviceName=%s  ",display_device.DeviceName );
    printf( "StateFlags=%u\n",display_device.StateFlags );
    printf( "DeviceID  =%s\n",display_device.DeviceID );
    printf( "DeviceKey =%s\n",display_device.DeviceKey );
/*DWORD cb;
  CHAR DeviceString[128];
  DWORD StateFlags;
  CHAR DeviceID[128];
  CHAR DeviceKey[128];
*/
  }
  Visual3D_Lightening();            // здесь пока существует только первое окно
  One.Above();
  One.Window::Mouse( ::MouseMove );
  One.Window::Mouse( ::MousePress );
  while( (Key=One.GetKey())!=_Esc )    // ... или на предельно большой скорости
  {  if( !One.Ready() )return 10; WaitTime( 50 );
     One.Activate( true );
     Rotation( theta+=0.32 );
     glColor3f( 0.5,0,0 );
     One.Print( 1,0,"Счетчик циклов: № %d ",++oTv ).Show();
  }
  One.SetTimer( 250 );
  Window Second; Win2=&Second;
         Second.Mouse( ::MouseMove ).Mouse( ::MousePress );
         Second.SetTimer( 150,SecondTimer ).KeyBoard( FreeHelp );
  Visual3D_Lightening(); // временно полагается активным второе внутреннее окно
  do
  { switch( Key=One.GetKey() )
    { case ' ': One.Clear(),clrscr(); break;
      case _Esc: One.Close(); //~_One();
    }
    switch( Key=Second.GetKey() )
    { case ' ': Second.Clear(),clrscr(); break;
//    case _F1: TextHelp( &Second,Nam2,Cmds,Plus ); break;
      case _Esc: print( 1,19,"Second::Destructor \n" ); gotoxy(1,21);
                 Second.Close(); //~Window();
    }
//} while( One.Ready() && Second.Ready() );
  } while( WinReady() );
  Break( "~ Окончание программы:\n\t { %X,%X } ",One.Ready(),Second.Ready());
}
