//
//!   Function.сpp                                             /2008.11.09/
//!   Программы построения скалярных одно-двух-трехмерных контурных функций
//
#include <Math.h>
#include "Contour.h"
#include "..\Tools\Allocate.h"

static int find( const Real *X,_Real A,int N );// Двоичный поиск левого индекса
static int find( const Point *P,_Real X,int N );  // то же по абсциссе Point::Х

inline Point operator+(_Point c,_Point e  ){ return (Point){ c.X+e.X,c.Y+e.Y,c.Z+e.Z }; }
//line Point operator-(_Point c,_Point e  ){ return (Point){ c.X-e.X,c.Y-e.Y,c.Z-e.Z }; }
inline Point operator*(_Point c,_Point e  ){ return (Point){ c.X*e.X,c.Y*e.Y,c.Z*e.Z }; }
inline Point operator/(_Point c,_Point e  ){ return (Point){ c.X/e.X,c.Y/e.Y,c.Z/e.Z }; }
inline Point W( _Vector V ){ return (Point){V.x,V.y,V.z}; }
inline Point V( _Real R ){ return (Point){R,R,R}; }

Contour::Contour( size_t L ):
         P(0),C(0),A(0),met(easy),ext(false),length(L)
         { if( L )P=(Point*)Allocate( L*sizeof( Point ) ); }
Contour::Contour( Point *p, size_t L ):
         P(p),C(0),A(0),met(easy),ext(true),length(L)
         { if( !L )length=isAlloc( p )/sizeof( Point ); }      // длина изнутри
Contour::~Contour()        // + надо быть осторожным и не удалять чужие массивы
{ if( !ext )if( P )Allocate( 0,P ); if( C )Allocate( 0,C );
                                    if( A )Allocate( 0,A );
}
Point& Contour::operator[]( int k ){ return P[minmax( 0,k,(int)length-1 )]; }
//Real Contour::operator()( int k ){ k=minmax( 0,k,(int)length-1 );
//                                   if( !A )return k; else return A[k];
//                                 }
Point Contour::operator()( _Real R )
{ Real a,h=1.0; int k;
   if( A ){ k=find( A,R,length ); a=R-A[k]; h=A[k+1]-A[k]; }
      else{ k=minmax( 0,int( R ),int(length-2) ); a=R-k; }
   if( !a || !h )return P[k]; else
   if( C )
   { if( met==spline )
     { Real b=1-a;
       return P[k]*b + P[k+1]*a + (C[k]*(b*b+1)*b + C[k+1]*(a*a+1)*a)*h*h/6.0;
     }
   } return P[k]+ (P[k+1]-P[k])*a/h;
}
//    Активизация различных режимов интерполяции
//
Contour& Contour::Mode              // переопределение всех характеристик линии
         ( metaLine Mr,             // тип кривой: ломаная, эрмитова или сплайн
           bool arg,                // аргумент по индексам к единице или длина
          _Real a1,_Real an         // концевые сопряжения для Эрмита и сплайна
         )                          // наклон или 1е6-точка перегиба
{ met=Mr; //! - заглушка ...
  if( arg )
  { (A=(Real*)Allocate( sizeof(Point)*length,A ))[0]=0.0;
    for( int i=1; i<length; i++ )A[i]=A[i-1]+abs( P[i]-P[i-1] );
  }
//if( met==first  )return First(); else
  if( met==spline )return SpLine( a1,an ); else return *this;
}
Point Contour::forX( _Real X ) // выбор первого вхождения точки с координатой Х
{ int k=find( P,X,length );
  Real a=X-P[k].X,h=P[k+1].X-P[k].X; if( !a || !h )return P[k]; else
  if( met==easy )return P[k]+ (P[k+1]-P[k])*a/h;
           else  return operator()( A[k]+(A[k+1]-A[k])*a/h );
}
//   Подготовка коэффициентов сплайна - Numerical Recipes, Cambridge university
//
Contour& Contour::SpLine( _Real p1,  // Активизация Сплайн-интерполяции
                          _Real pn ) // Производная в начале и конце отрезка
{ if( length>=3 )                    // если нет 4 точек, то - выход
  { int  i,n=length-1; Real qn;
        Point b,un,*U=(Point*)Allocate( n*sizeof( Point ) );
             C=(Point*)Allocate( length*sizeof( Point ),C );
    if( A )                          // метод прогонки для трехлинейной матрицы
    { Real a,sig;                    // вариант с неравномерным шагом аргумента
      if( p1>0.99e6 )C[0]=0.0,U[0]=0.0; else
      { C[0]=V(-0.5); a=A[1]-A[0]; U[0]=( V(-p1)+(P[1]-P[0])/a )*(3/a);
      }
      for( i=1; i<n; i++ )
//    if( (A[i]-A[i-1])<1e-6 || (A[i+1]-A[i])<1e-6 )U[i]=0.0,C[i]=C[i-1]; else
      { a=( A[i+1]-A[i-1] ); sig=( A[i]-A[i-1] )/a;   // 0.5 или искажение шага
        b=C[i-1]*sig+V(2.0); C[i]=V(sig-1)/b;
        U[i]=( U[i-1]*-sig
               + ((P[i+1]-P[i])/(A[i+1]-A[i])-(P[i]-P[i-1])/(A[i]-A[i-1]))
                 * (6.0/a) )/b;
      }
      if( pn>0.99e6 )qn=0.0,un=0.0; else
      { qn=0.5; a=A[n]-A[n-1]; un=( V(pn)+(P[n-1]-P[n])/a )*3/a;
      }
    } else                              // несколько упрощенный вариант расчета
    { if( p1>0.99e6 )C[0]=0.0,U[0]=0.0; //   для единичного шага аргумента
                else C[0]=V(-0.5),U[0]=(V(-p1)+(P[1]-P[0]))*3.0;
      for( i=1; i<n; i++ )
      { U[i]=(C[i]=V(-1)/(C[i-1]+V(4)))
            *(U[i-1]+( (P[i+1]-P[i])-(P[i]-P[i-1]) )*-6.0);
      }
      if( pn>0.99e6 )qn=0.0,un=0.0;
      else qn=0.5,un=(V(pn)-(P[i]-P[i-1]))*3.0;
    }
                           C[n]=W(un-U[n-1]*qn)/(V(1)+C[n-1]*qn);
    for( i=n-1; i>=0; i-- )C[i]=C[i]*C[i+1]+U[i]; Allocate( 0,U );
         met = spline;
  } else met = easy; return *this;
}
// двоичный поиск ближайшего левого индекса в поступательной последовательности
//
static int find( const Real *R,_Real A,int N )  // Двоичный поиск ближайшего
{ bool d=R[--N]>R[0];                           // левого индекса для заданного
  int i=0,k;                           // аргумента с учетом знака его прироста
   while( N-i>1 ){ k=(N+i)/2; if( d ){ if( A<R[k] )N=k; else i=k; }
                                else { if( A>R[k] )N=k; else i=k; } } return i;
}
static int find( const Point *P,_Real A,int N )   // то же по абсциссе Point::Х
{ bool d=P[--N].X>P[0].X; int i=0,k;
  while( N-i>1 ){ k=(N+i)/2; if( d ){ if( A<P[k].X )N=k; else i=k; }
                               else { if( A>P[k].X )N=k; else i=k; } } return i;
}
