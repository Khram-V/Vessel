/**
 *   Третья программа для иллюстрации построений с использованием
 *   аналитических функций комплексного аргумента
 *   (Window-Place + OpenGL/С++)
 *   (c)2008, В.Храмушин, Сахалинский государственный университет
 **/
#include "..\View.h"
#include "..\..\Math\Vector.h"
static Vector arrow( Vector a, Vector b,_Real l, const colors clr=empty )
{ Vector d=a-b; d*=l/abs( d );
  Vector e={ d.z/5,d.x/5,d.y/5 },f={ e.z,e.x,e.y },g=b+d/2; line( a,d+=b,clr );
   glBegin( GL_LINE_LOOP ),dot( g ),dot( d+e ),dot( b ),dot( d-e ),
                           dot( g ),dot( d+f ),dot( b ),dot( d-f ),
                           dot( g ),glEnd(); return b;
}
//      Собственно программа, работающая с графическим пакетом OpenGL
//
static fixed KeyFunction='0';
const Real Sz=3.0;               // полуразмер сеточного квадрата, вмещающегося
                                 // в 80% стандартного графического поля [-1:1]
Window Win( "Аналитические функции: Vector - Window-Place - OpenGL/C++",0,0,1600,1000 );
//
//    Стандартные 2D функций исчисления комплексных чисел
//
inline Vector Mult( Vector R,Vector Q ){ return (Vector){ R.x*Q.x-R.y*Q.y, R.x*Q.y+R.y*Q.x,0}; }
inline Vector Polar( Vector R ){ return (Vector){ R.x*cos( R.y ),R.x*sin( R.y ),0}; }
inline Vector Conject( Vector R ){ R.y=-R.y; return R; }
inline Vector Invert  ( Vector R ){ Real L=R.x*R.x+R.y*R.y; return Conject( R/L ); }
inline Vector Exponenta( Vector R ){ R.x=exp( R.x ); return Polar( R ); }
inline Vector Logarithm( Vector R ){ return (Vector){ log( abs( R ) ),atan2( R.y,R.x ),0}; }
inline Vector Power( Vector R, Vector power )
     { return abs( R )<1.0e-12 ? (Vector){ 1.0,0,0} : Exponenta( Mult( Logarithm( R ),power ) ); }
inline Vector Power( Vector R, Real power )
     { return abs( R )<1.0e-12 ? (Vector){ 1.0,0,0} : Exponenta( Logarithm( R )*power ); }
//
//atic Vector Gradient( Vector R );
//
static void Grid_Image( Vector A( Vector ) )
{ Real x,y,d=0.25,s=1;
  const Real st=0.02;
    for( x=-Sz/s+d/2; x<Sz/s; x+=d )
    for( y=-Sz/s; y<Sz/s; y+=st )
     if( hypot( x,y )>1.0 )line( A( (Vector){ x,y,0} ),A( (Vector){ x,y+st,0} ),gray );
    glLineWidth( 2 );
    for( y=-Sz/s+d/2; y<Sz/s; y+=d )
    for( x=-Sz/s; x<Sz/s; x+=st )
     if( hypot( x,y )>1.0 )line( A( (Vector){ x,y,0} ),A( (Vector){ x+st,y,0} ),purple );
    glLineWidth( 1 );
}
static bool ViewVektorSwitch=false;

static void Vector_Image( Vector A( Vector ), Real mul=1.0  )
{ Real h,d=0.25,s=1;
  Vector w={0,0,0},W2,W1,Vr,Vi;
  if( !ViewVektorSwitch)
  { glLineWidth( 2 );
    for( w.x=-Sz/s; w.x<=Sz/s; w.x+=d )         // Рисование функции тока
    for( w.y=-Sz/s; w.y<=Sz/s; w.y+=d )         // комплексного потенциала
    { W2 = A( w - (Vector){ d/-12,0,0} );
      W1 = A( w - (Vector){ d/12,0,0} );  Vr.x=W2.x-W1.x; Vi.y=-W2.y+W1.y;
      W2 = A( w - (Vector){ 0,d/-12,0} );
      W1 = A( w - (Vector){ 0,d/12,0} );  Vr.y=W2.x-W1.x; Vi.x=W2.y-W1.y;
      h = abs( Vi )*2;
      line( w+Mult( (Vector){ 0,d/h,0},Vi ),
            w-Mult( (Vector){ 0,d/h,0},Vi ),white );
    }
    glLineWidth( 1 );
    for( w.x=-Sz/s; w.x<=Sz/s; w.x+=d )         // Рисование стрелок
    for( w.y=-Sz/s; w.y<=Sz/s; w.y+=d )         // комплексного градиента
    if( abs( w )>1.0 )
    { W2 = A( w - (Vector){ d/12,0,0} );
      W1 = A( w - (Vector){ d/-12,0,0} ); Vr.x=W2.x-W1.x; Vi.y=-W2.y+W1.y;
      W2 = A( w - (Vector){ 0,d/12,0} );
      W1 = A( w - (Vector){ 0,d/-12,0} ); Vr.y=W2.x-W1.x; Vi.x=W2.y-W1.y; Vr*=6.0,Vi*=6.0;
      if( abs( Vr )>=M_PI )Vr=(Vector){0,0,0}; // Vr.x-=M_PI*6,Vr.y-=M_PI*6;
      arrow( w - Vr*mul, w + Vr*mul, 0.1, blue );
    }
  } else
  { for( w.x=-Sz/s; w.x<=Sz/s; w.x+=d ) // расчет для смещения относительно
    for( w.y=-Sz/s; w.y<=Sz/s; w.y+=d ) // исходного поля комплексной функции
    { Real h=abs( A( (Vector){ w.x,w.y,w.z } ) )*2;
      line( w+Mult( (Vector){ 0,d/h,0}, A( (Vector){ w.x,w.y,w.z } ) ),
            w+ -Mult( (Vector){ 0,d/h,0},A( (Vector){ w.x,w.y,w.z } ) ),white );
    }
    for( w.x=-Sz/s; w.x<=Sz/s; w.x+=d )         // Рисование стрелок
    for( w.y=-Sz/s; w.y<=Sz/s; w.y+=d )         // комплексного градиента
      arrow( w-A( (Vector){ w.x,w.y,w.z } )*d*mul,
             w+A( (Vector){ w.x,w.y,w.z } )*d*mul,0.1,blue );
  }
}
//void Polar_Image(){}
//
//     Моделирующие функции
//
Vector DiPole  ( Vector R ){ return Invert( R )+R; }
Vector Linear  ( Vector R ){ return (Vector){ -0.5,0.1,0 } + Mult( R,(Vector){ 0.8,0.3,0} ); }
Vector Cilinder( Vector R ){ Vector A={ 1,1,0}; return Mult( Conject( A ),R ) + Mult( A,Invert( R ) ); }
Vector Angles  ( Vector R ){ //R=Conject( R );
                             return Power( R,(Vector){1.5,0,0} )
//                         + Power( R+(Vector){0,1.0,0},(Vector){0,1,0} )
//                         + Power( R+(Vector){0,-1.0,0},(Vector){-0.5,0,0} )
//                         + Power( R+(Vector){-1,-1.0,0},(Vector){-0.5,0,0} )
//                         + Power( R+(Vector){-1,1.0,0},(Vector){-0.5,0,0} )
                         ; }
Vector LinDivide( Vector Z ){ Real A=1,B=2,C=3,D=4; return Mult( Z*A+(Vector){ B,0,0},Invert( Z*C+(Vector){ D,0,0} ) ); }
Vector Zhukovsky( Vector Z ){ Vector A={ 1,0.5,0}; return Mult( A,Conject( Z ) ) + Power( Mult( Z,Z ) - Mult( A,A ),-0.5 ); }
//Vector Zhukovsky( Vector W ){ Vector A={1,0.5}; return ( W+Mult( Mult( A,A ),Invert( W ) ) )/2.0; }
//Vector Zhukovsky( Vector Z ){ Vector A( 1 ); Real Alf=30,V=1.0; return  V*( Z*cos( Alf ) - (Vector){ 0,sin( Alf ) )*Power( Mult( Z,Z )-Mult( A,A ),0.5 ) ); }
//Vector Zhukovsky( Vector Z ){ Vector V( 1,1 ),A( 1 ); return  Conject( Z )*V.x - Mult( (Vector){ 0,V.y,0},Power( Mult( Z,Z )+Mult( A,A ),(Vector){ 0.5,0,0} ) ); }
//
//     Самая главная процедура рисования
//
static void grid()
{ Real B=Sz*4; int k; glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
  glLineWidth( 0.5 );
  for( k=-B; k<B+0.5; k++ )line( (Vector){ k*Sz/B,-Sz,0},(Vector){ k*Sz/B,Sz,0},gray );
  for( k=-B; k<B+0.5; k++ )line( (Vector){ -Sz,k*Sz/B,0},(Vector){ Sz,k*Sz/B,0},gray );
  glLineWidth( 3 );
  color( yellow ),rectangle( (Vector){-1,-1},(Vector){1,1},false ),
  color( lightgreen );
  glBegin( GL_POLYGON );
    for( B=M_PI*2; B>0.0; B-=0.02 )dot( (Vector){ cos( B ),sin( B ),0.1 } );
  glEnd();
  glLineWidth( 1 );
}
static bool drawing()
{ char *S; Vector(*F)(Vector)=NULL; grid();
  switch( KeyFunction )
  { case '0': F=DiPole;    S="Диполь Z + 1/Z";               break;
    case '1': F=Angles;    S="Угловой слом Z^1.5 ";          break;
    case '2': F=Exponenta; S="Экспонента exp(Z)";            break;
    case '3': F=Polar;     S="Полярная x·(cos(y)+i·sin(y))"; break;
    case '4': F=Invert;    S="Обратная 1/Z";                 break;
    case '5': F=Logarithm; S="Логарифм log( Z )";            break;
    case '6': F=Linear;    S="Линейная (-0.5+i)+Z·(0.8+i/3)"; break;
    case '7': F=Cilinder;  S="Цилиндр(a=1+i); conj(a)·Z+conj(a)/Z "; break;
    case '8': F=LinDivide; S="Частное(a=1,b=2,c=3,d=4); (Z·a+b)/(Z·c+d)";break;
    case '9': F=Zhukovsky; S="Жуковский(a=1+i/2); a·conj(Z)+1/sqrt(Z·Z-a·a)";
  }
  if( F )Vector_Image( F ),Grid_Image( F ),
         color( gray ),Win.Print( 2,1,"%c: %s",KeyFunction,S );
  Win.Save().Show(); return true;
}
//!  Главная программа
//!
int main() // int argc, char** argv )
{ //
  //  подготовка и инициализация графического экрана
  //
 fixed key=0;
  //
  //   Утилиты Window обеспечивают взаимодействие с операционной системой
  //
  View_initial();
  Win.Icon( "Flag" ).Draw( drawing ).AlfaVector( 48 ).Activate( true );
  glClearColor( 1,0.98,0.95,0.8 );
  glScaled( .8/Sz,.8/Sz,1 );
  do
  { switch( key )
    { case ' ': ViewVektorSwitch^=true; break;
      default : if( key>='0' && key<='9' )KeyFunction=key;
    } drawing();
  } while( Win.Ready() && (key=Win.WaitKey())!=_Esc ); return 0;
}
