/**                                                                        ¹²³ⁿ
 *      Подборка геометрических фигур с собственным пространственным базисом
 *      (c)2020-10-08 ‏יְרוּשָׁלַיִם
 */
#include "Model.h"
#include "Volume.h"

Model& Model::cubic( bool painting_Model )
{ const Real o=-0.5,i=0.5;
  const Vector V[]={ {o,o,o},{o,o,i},{o,i,o},{o,i,i},
                     {i,o,o},{i,o,i},{i,i,o},{i,i,i} };
  const colors c[]={ black,blue,green,cyan,red,magenta,yellow,white };
  glBegin( painting_Model?GL_QUADS:GL_LINE_STRIP );
 #define Vertex( k ) dot( out( V[k] ),c[k] )
         Vertex( 0 ),Vertex( 1 ),Vertex( 3 ),Vertex( 2 );     // левый   - x
         Vertex( 0 ),Vertex( 4 ),Vertex( 5 ),Vertex( 1 );     // нижний  - y
         Vertex( 0 ),Vertex( 2 ),Vertex( 6 ),Vertex( 4 );     // дальний - z
  if( !painting_Model )glEnd(),glBegin( GL_LINE_STRIP );      //- один разрыв
         Vertex( 6 ),Vertex( 2 ),Vertex( 3 ),Vertex( 7 );     // верхний
         Vertex( 3 ),Vertex( 1 ),Vertex( 5 ),Vertex( 7 );     // ближний
         Vertex( 5 ),Vertex( 4 ),Vertex( 6 ),Vertex( 7 );     // правый
 #undef Vertex
  glEnd(); return *this;
}
//     локальный класс для временного сброса закраски в кэш-буфер
//
struct painting_Model{ bool paint;
       painting_Model( bool p ): paint( p )
                     { if( !p )glPushAttrib( GL_POLYGON_BIT ),
                               glPolygonMode( GL_FRONT_AND_BACK,GL_LINE ); }
      ~painting_Model(){ if( !paint )glPopAttrib(); }
};
//!    Тор и сфера
static const colors C[]={ white,lightred,orange,yellow,olive,green,lightgreen,
         cyan,lightcyan,lightblue,lightmagenta,purple,darkgray };
static const int cp=sizeof( C )/sizeof( colors ); // =15

Model& Model::dipole( _Real s, bool paint )   // s: внутри {-1 <s< +∞ } снаружи
{ const int ns=33,rs=24;                      // ns=33 сопрягается с весовым
  painting_Model _p( paint );                 // коэффициентом 1.253
//Vector W,Meridian[ns]={ { 0,EqSphere*max( 0.1,s+(s<0?1:exp(-s*s)) ),0 } };
#if 0
  const Vector M={ EqSphere,0,0 };
  Vector W,Meridian[ns]={ { 0,EqSphere*exp( s ),0 } };
         W=dipole_flow( M,Meridian[0] );               // экваториальный отсчет
  for( int i=1; i<ns; i++ )                                // четвертинка меридиана
  { Vector V=dipole_flow( M,Meridian[i-1]+W*M_PI/Real( ns ) );
    Meridian[i]=Meridian[i-1]+(( 1.253*V+W )/2.253)*M_PI/Real( ns ); W=V;
  }
#else
  const Real R=exp(s)*EqSphere,A=_Pd/(ns*2-2),  // радиус и угол многоугольника
             H=R*sin( _Pi*( 1-1/Real(ns-1) ) )/2; // хорда
  const Vector M={ EqSphere };
  Vector Meridian[ns]={ { 0,R } },                      // диаметральная точка
         W=dir( dipole_flow( M,(Vector){ H/2,R } ) )*H; // экваториальный отсчет
  for( int i=1; i<ns; i++ )                            // четвертинка меридиана
  { Vector V=dir( dipole_flow( M,Meridian[i-1]+W ) )*H;
    Meridian[i]=Meridian[i-1]+(V*1.0737+W)/2.0737; W=V; //( ( 1.253*V+W )/2.253 ); W=V;
  }
#endif
  for( int i=1-ns; i<ns-1; i++ ){ glBegin( GL_QUAD_STRIP );        // параллели
    for( int j=0;  j<=rs;  j++ ){ Real t=(j%rs)*(-2.0*M_PI)/rs;    // меридианы
      for( int k=0; k<=1;  k++ ){ W=Meridian[abs(i+k)];            // закраска
        if( i+k<0 )W.x=-W.x;                             // дополнение абсциссы
        if( i==1-ns && !k || i==ns-2 && k )W.y/=12;      // замыкание оконцовок
        //glNormal3dv( out( (Vector){W.x,W.y*cos( t ),W.y*sin( t )} ) );
        dot( out( (Vector){W.x,W.y*cos(t),W.y*sin(t)} ),C[(i+k+ns)*cp/ns/2] );
//             C[ (ns-i-k)*(cp-1)/ns/2] );
      }
    } glEnd();
  }
  return *this;
}
#if 1
Model& Model::sphere( int ns,int rs, bool paint )
{ painting_Model _p( paint );                              // ns>=2 - параллели
  for( int i=0; i<ns; i++ ){ glBegin( GL_QUAD_STRIP );     // rs>=3 - меридианы
    for( int j=0; j<=rs; j++ ){ Real t=(j%rs)*(2.0*M_PI)/rs;
      for( int k=0; k<=1; k++ ){ Real s=M_PI*( 0.5-Real( i+k )/ns );
        dot( out( (Vector){ sin(s),cos(s)*cos(t),cos(s)*sin(t) } ), // *R ));
                 C[(ns-i-k)*(cp-1)/ns] );                 // интервалы цветов+1
    } } glEnd();
  } return *this;
}
Model& Model::torus( _Real r,_Real R, int ns,int rs, bool paint )
{ painting_Model _p( paint );
  for( int i=0; i<ns; i++ ){ glBegin( GL_QUAD_STRIP ); // glColor3ubv(C[ns-i-1]);
    for( int j=0; j<=rs; j++ ){ Real t=M_PI*Real(j)*2/rs;
      for( int k=0; k<=1; k++ ){ Real s=M_PI*(Real((i+k)*2+1)/ns+1),a=R+r*cos(s);
      //glNormal3dv( Model.out((Vector){sin(s),cos(s)*cos(t),cos(s)*sin(t)}) );
        dot( out( (Vector){ r*sin(-s),a*cos(t),a*sin(t) } ),
               C[(ns-i-k)*(cp-1)/ns] );                     // интервалы цветов
    } } glEnd();
  } return *this;
}
#else
//     Немного отличные варианты построения тора и сферы
//
Model& Model::sphere( int ns,int rs, bool paint )
{ Real sTa=0,cTa=1,trn=(-2.0*M_PI)/rs,dt=trn; painting_Model _p( paint );
//  for( int i=rs-1; i>=0; i--,trn-=(2.0*M_PI)/rs )
  for( int i=0; i<rs; i++,trn+=dt ) //-=(2.0*M_PI)/rs )
  { Real cTe=cos( trn ),sTe=sin( trn ),t=-M_PI_2; glBegin( GL_QUAD_STRIP );
    for( int j=0; j<=ns; j++,t+=M_PI/ns )
    { Real cPh=cos( t ),sPh=sin( t );
      glNormal3dv( out( (Vector){ sPh,cTe*cPh,sTe*cPh } ) );
            point( out( (Vector){ sPh,cTe*cPh,sTe*cPh } ),C[(j*cp/ns)%cp] );//*R
      glNormal3dv( out( (Vector){ sPh,cTa*cPh,sTa*cPh } ) );
            point( out( (Vector){ sPh,cTa*cPh,sTa*cPh } ) );               //*R
    } glEnd(); cTa=cTe,sTa=sTe;
  } return *this;
}
Model& Model::torus( _Real r,_Real R, int ns,int rs, bool paint )
{ Real sTa=0.0,cTa=1.0,trn=(-2.0*M_PI)/rs;      painting_Model _p( paint );
  for( int i=rs-1; i>=0; i--,trn-=(2.0*M_PI)/rs )
  { Real cTe=cos( trn ),sTe=sin( trn ),t=-M_PI; glBegin( GL_QUAD_STRIP );
    for( int j=0; j<=ns; j++,t+=(2.0*M_PI)/ns )
    { Real cPh=cos( t ),sPh=sin( t ),dst=R+r*cPh;
      glNormal3dv( out( (Vector){   sPh, cTe*cPh, sTe*cPh } ) );
            point( out( (Vector){ r*sPh, cTe*dst, sTe*dst } ),C[(j*cp/ns)%cp] );
      glNormal3dv( out( (Vector){   sPh, cTa*cPh, sTa*cPh } ) );
            point( out( (Vector){ r*sPh, cTa*dst, sTa*dst } ) );
    } glEnd(); cTa=cTe,sTa=sTe;
  } return *this;
}
#endif
