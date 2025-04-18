//
//   Curve.cpp				Ver 3.2  /89.01.27-93.03.22/
//   Несколько программ анализа "Point" векторов
//
#include "Curve.h"

Curve:: Curve( int S ){ _C=0; _2=0; N=0; if( S>0 )allocate( S ); }
Curve::~Curve()       { free(); }

Curve& Curve::allocate( int Sz )
{ if( _2 )_2=(Real*)Allocate( 0,_2 );
	  _C=(Point*)Allocate( (N=Sz)*sizeof( Point ),_C ); return *this;
}
Curve& Curve::free()
{ if( _2 )_2=(Real*)Allocate( 0,_2 );
  if( _C )_C=(Point*)Allocate( 0,_C ); N=0; return *this;
}
//Point& Curve::operator[]( int Ind )
//{ if( fixed( Ind )>=N )Break( "?Curve.%d[%d] off range",N,Ind );
//  return _C[Ind];
//}
Curve& Curve::operator += ( Point& P )
{ Point *C=_C;
  for( int k=0; k<N; k++ )*C++ += P; return *this;
}
#define  _X( k ) ( _C[(k)].x )
				 //
int Curve::find( Real Ar )	 // Двоичный поиск ближайшего левого
{ int i=0,k,n=N-1,d=_X(n)>_X(0); //  индекса для заданного аргумента
  while( n-i>1 )		 //   с учетом знака его прироста
  {   k=(n+i)/2;		 //
    if( d ){ if( Ar<_X(k) )n=k; else i=k; }
      else { if( Ar>_X(k) )n=k; else i=k; }
  } return i;
}
Real Curve::value( Real Ar, int Pw ) // Интерполяция по Лагранжу
{			       //
 int i=find( Ar ),Nb,Nf,n=N-1; // Поиск ближайшего левого индекса
  if( !Pw  )return _C[i].y;
  if( Pw>n )Pw=n;	       //
  if( (Nf=i+(Pw/2)+1)>n )Nf=n,Nb=n-Pw; else
  if( (Nb=Nf-Pw)<0      )Nb=0,Nf=Pw;
 Real yo=0,po,pi; i=Nb;
  do
  { n=Nb; po=pi=1;
    do if(   i!=n  ){ pi *=_X(i)-_X(n); po*=Ar-_X(n); }
    while( ++n<=Nf ); yo +=_C[i].y*po/pi;
  } while( ++i<=Nf ); return yo;
}
#define X( k ) ( P[k].x )
#define Y( k ) ( P[k].y )
Real Curve::operator()( Real Ar )
{  int k;
 Point *P=_C+( k=find( Ar ) ); Real h=X(1)-X(0), b=( Ar-X(0) )/h;
  if( _2 )
  { Real a=1-b; return a*Y(0) + b*Y(1)
			+ ( (a*a-1)*a*_2[k]
			  + (b*b-1)*b*_2[k+1] )*(h*h)/6.0;
  } return Y(0) + b*( Y(1)-Y(0) );
}
void Curve::SpLine()
{ if( N<3 ){ if( _2 )Allocate( 0,_2 ); _2=0; return; }
 int i,k,n=N-1;
 Real p,qn,sig,un,*u;
 Point *P=_C+1; (_2=(Real*)Allocate( N*sizeof( Real ),_2 ))[0]=0;
                (u=(Real*)Allocate( n*sizeof( Real ) ))[0]=0;
  for( i=1; i<n; i++,P++ )
  { sig=( X(0)-X(-1) )/( X(1)-X(-1) ); p=sig*_2[i-1]+2;
   _2[i]=( sig-1 )/p;
    u[i]=( Y(1)-Y(0)  )/( X(1)-X(0) )
	-( Y(0)-Y(-1) )/( X(0)-X(-1) );
    u[i]=(6.0*u[i]/(X(1)-X(-1))-sig*u[i-1])/p;
  }
  qn=un=0.0;             _2[n]=( un-qn*u[n-1] )/( qn*_2[n-1]+1.0 );
  for( k=n-1; k>=0; k-- )_2[k]=_2[k]*_2[k+1]+u[k];
  Allocate( 0,u );
}
#if 0
void Curve::SpLine( Real yp1, Real ypn )
{ if( N<3 ){ if( _2 )Allocate( 0,_2 ); _2=0; return; }
 int i,k,n=N-1;
 Real p,qn,sig,un,*u;
 Point *P=_C;	       _2=(Real*)Allocate( N*sizeof( Real ),_2 );
			u=(Real*)Allocate( n*sizeof( Real ) );
  if( yp1>.99e30 )_2[0]=u[0]=0.0; else
    { _2[0]=-0.5;
      p=X(1)-X(0); u[0]=3*( (Y(1)-Y(0))/p - yp1 )/p;
    }      	     P++;
  for( i=1; i<n; i++,P++ )
  { sig=( X(0)-X(-1) )/( X(1)-X(-1) ); p=sig*_2[i-1]+2;
   _2[i]=( sig-1 )/p;
    u[i]=( Y(1)-Y(0)  )/( X(1)-X(0) )
	-( Y(0)-Y(-1) )/( X(0)-X(-1) );
    u[i]=(6.0*u[i]/(X(1)-X(-1))-sig*u[i-1])/p;
  }
  if( ypn>.99e30 )qn=un=0.0; else
    { qn=0.5;
      p=X(0)-X(-1); un=3*( ypn-( Y(0)-Y(-1) )/p )/p;
    }
			 _2[n]=( un-qn*u[n-1] )/( qn*_2[n-1]+1.0 );
  for( k=n-1; k>=0; k-- )_2[k]=_2[k]*_2[k+1]+u[k];
  Allocate( 0,u );
}
#endif
#undef X
#undef Y
//
//	Подготовка экстремумов функции и аргумента
/*
void Curve::Extreme( Field& Fm )
{ for( int i=0; i<N; i++ )
  { Point P=_C[i];
    if( i )
    { if( Fm.Jy>P.y )Fm.Jy=P.y; else if( Fm.Ly<P.y )Fm.Ly=P.y;
      if( Fm.Jx>P.x )Fm.Jx=P.x; else if( Fm.Lx<P.x )Fm.Lx=P.x;
    } else
    { Fm.Lx=( Fm.Jx=P.x )+1.0e-12;
      Fm.Ly=( Fm.Jy=P.y )+1.0e-12;
  } } Fm.Ly-=Fm.Jy;
      Fm.Lx-=Fm.Jx;
}
*/
