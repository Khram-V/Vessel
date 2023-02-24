/**
 **     Моделирование кинематики, динамики и механики взаимодействия
 **     множества поляризованных частиц первого и второго порядка
 **     (пространственные вихреисточники и диполи)
 **                                                  (c)2021 ‏יְרוּשָׁלַיִם
 **/
#include "Dipoles.h"
#include "../Window/View.h"
//
//! прорисовка диполя в радужно-доплеровских расцветках (синий нос,красный зад)
//
Model& Model::dipole( _Real s, bool paint ){  // s: внутри {-1 <s< +∞ } снаружи
 const colors C[]={purple,blue,lightblue,cyan,lightcyan,lightgreen, // радужная
                   olive,yellow,orange,lightred,red,silver };       // покраска
 const int ns=33,rs=24,cp=sizeof( C )/sizeof( colors ); // числа многоугольника
 const Real R=exp(s)*EqSphere,H=R*sin(_Pi*(1-1/Real(ns-1)))/2;// радиус и хорда
 const Vector M={ EqSphere,0.0,0.0 };                  // дипольный момент: 1.0
 Vector Meridian[ns]={ { 0.0,R,0.0 } },                // диаметральная точка
        W=dir( dipole_v( M,(Vector){ H/-2,R } )-M )*H; // экваториальный отсчет
  for( int i=1; i<ns; i++ )                            // четвертинка меридиана
  { Vector V=dir( dipole_v( M,Meridian[i-1]+W )-M )*H;    // разметка одного
    Meridian[i]=Meridian[i-1]+(V*1.0737+W)/2.0737; W=V; } // меридиана
  if( !paint )glPushAttrib( GL_POLYGON_BIT ),             // спасение состояния
              glPolygonMode( GL_FRONT_AND_BACK,GL_LINE ); // и контурный случай
  for( int i=1-ns; i<ns-1; i++ ){ glBegin( GL_QUAD_STRIP );        // параллели
    for( int j=0;  j<=rs;  j++ ){ Real t=(_Pd*(j%rs))/rs;          // меридианы
      for( int k=0; k<=1;  k++ ){ W=Meridian[abs(i+k)];            // закраска
        if( i+k<0 )W.x=-W.x;                       // дополнение абсциссы
        if( i==1-ns && !k || i==ns-2 && k )W.y/=6; // замыкание оконцовок/торца
        color( C[(i+k+ns)*cp/ns/2],0.125,0.75 );   // осветление и прозрачность
        dot( out( (Vector){ W.x,W.y*cos(t),W.y*sin(t) } ) ); // = glVertex3dv
    } } glEnd();
  } if( !paint )glPopAttrib(); return *this;
}
//!  промежуточная печать на текстовой консоли
//
#include <time.h>
#include "../Window/ConIO.h"
void Text_to_ConIO( Dipoles &D )
{ static int ct=0; time_t lt=time(0); // к текстовому окну с подключением мышки
  if( !ct )textsize( 80,25,25 );      //   при неявной внешней инициализации
  textcolor( LIGHTGREEN );
  cprint( 2,2,"<%d/%d>=<%d/%d> %s ",
        ++ct,Time_count,nDip,RealTime,asctime( (tm*)localtime( &lt ) ) );
  for( int i=0; i<nDip && i<32; i++ )
  { Dipole &B=D[i]; cprint( 2,i+3,"R={%5.2f,%5.2f,%5.2f}=%-5.2f V=%+.2f ",
                                   B.R.x,B.R.y,B.R.z,abs(B.R),abs(B.V)-1 );
  }
}
