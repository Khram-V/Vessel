//
//      Простая интерполяция на месте по Эрмиту второго порядка
//
#include "Hermite.h"
#define Iph 1.6180339887498948482          // граничные веса на золотом сечении
#define Ihp 0.3819660112501051518          // диагональ к стороне пятиугольника
                                           //       (a+b)/a = a/b  м = (1+√5)/2
Vector Hermite( Vector *F, _Real Arg, int N, Vector *D )    // по целочисленным
{ if( N<2 ){ if( D )*D=0.0; return F[0]; }                  // номерам узлов
  if( N<3 ){ if( D )*D=F[1]-F[0]; return F[0]+( F[1]-F[0] )*Arg; }
 int k=Arg; --N;                              // расчет индекса ближайшей точки
//if( k<-1 || k>N )return 0;                  // и опять, может зараз 10,001км?
  if( k<0 )k=0; else if( k>=N )k=N-1; F+=k;
 Real a=Arg-k;
 Vector c=F[1]-F[0],f0,f1; f1=0.0,f0=0.0;
  if( k>0    )f0=( F[1]-F[-1] )/2.0;          // производные по хордам смежных
  if( k<N-1  )f1=( F[2]-F[0]  )/2.0;          // узлов аргумента функции или
  if( !k     )f0 = ( c-f1*Ihp )*Iph;          // экстраполяционное упреждение
  if( k==N-1 )f1 = ( c-f0*Ihp )*Iph;          // на концах интервала
  if( D )*D = f0 + a*( 2*(c-f0) + (3*a-2)*(f0+f1-2*c) );  // запрос производной
 return F[0] + a*( f0 + a*( c-f0 + (a-1)*(f0-2*c+f1) ) ); // собственно функция
}
//     Одномерная интерполяция на регулярных неравномерных узлах
//
Vector Hermite( Vector *Y, Real *X,_Real Arg, int N, Vector *D )
{ if( N<2 ){ if( D )*D=0.0; return Y[0]; } Vector Z; Real H;
  if( N<3 )
    { if( fabs( H=X[1]-X[0] )<1e-6 )return (Y[0]+Y[1])/2; else Z=(Y[1]-Y[0])/H;
      if( D )*D=Z; return Y[0]+(Arg-X[0])*Z;
    }
 int k=find( X,Arg,N-- );   // с остановкой поиска на правее или на левой точке
  Y+=k; X+=k;
  if( fabs( H=X[1]-X[0] )<1e-6 )return (Y[0]+Y[1])/2; else Z=(Y[1]-Y[0])/H;
 Vector f0,f1;    // производные на крайних сторонах интерполируемого интервала
  if( k>0   ) f0 = ( Z + (Y[0]-Y[-1])/(X[0]-X[-1]) )/2.0;
  if( k<N-1 ) f1 = ( Z + (Y[2]-Y[1])/(X[2]-X[1]) )/2.0;
  if( !k     )f0 = ( Z - f1*Ihp )*Iph;         // экстраполяционное упреждение
  if( k==N-1 )f1 = ( Z - f0*Ihp )*Iph;         // на концах интервала
 Real A=Arg-X[0];                              // запрос производной
  if( D )*D = f0 + ( 2.0*(Z-f0)+(f0-2.0*Z+f1)*(2.0*(Arg-X[1])+A)/H )*A/H;
  return Y[0] + A*(f0 + ( Z-f0 +(f0-2.0*Z+f1)*( Arg-X[1] )/H )*A/H );
}
//   Одномерная интерполяция на регулярных неравномерных узлах
//
Real Hermite( Real *Y, Real *X,_Real Arg, int N, Real *D )
{ Real Z,H;                                             // всё надо согласовать
  if( N<2 ){ if( D )*D=0.0; return Y[0]; }
  if( N<3 )
    { if( fabs( H=X[1]-X[0] )<1e-6 )return (Y[0]+Y[1])/2; else Z=(Y[1]-Y[0])/H;
      if( D )*D=Z; return Y[0]+(Arg-X[0])*Z;
    }
 int k=find( X,Arg,N-- );   // с остановкой поиска на правее или на левой точке
  Y+=k; X+=k;
  if( fabs( H=X[1]-X[0] )<1e-6 )return (Y[0]+Y[1])/2; else Z=(Y[1]-Y[0])/H;
 Real f0,f1;      // производные на крайних сторонах интерполируемого интервала
  if( k>0   ) f0 = ( Z + (Y[0]-Y[-1])/(X[0]-X[-1]) )/2.0;
  if( k<N-1 ) f1 = ( Z + (Y[2]-Y[1])/(X[2]-X[1]) )/2.0;
  if( !k     )f0 = ( Z - f1*Ihp )*Iph;         // экстраполяционное упреждение
  if( k==N-1 )f1 = ( Z - f0*Ihp )*Iph;         // на концах интервала
 Real A=Arg-X[0];                              // запрос производной
  if( D )*D = f0 + ( 2.0*(Z-f0)+(f0-2.0*Z+f1)*(2.0*(Arg-X[1])+A)/H )*A/H;
  return Y[0] + A*(f0 + ( Z-f0 +(f0-2.0*Z+f1)*( Arg-X[1] )/H )*A/H );
}
Real Hermite( Real *F,_Real Arg,int N,Real *D ) // по целым узлам по их номерам
{ if( N<2 ){ if( D )*D=0.0; return F[0]; }
  if( N<3 ){ if( D )*D=F[1]-F[0]; return F[0]+Arg*(F[1]-F[0]); }
 int k=Arg; --N;                              // расчет индекса ближайшей точки
//if( k<-1 || k>N )return 0;                  // и опять, может зараз 10,001км?
  if( k<0 )k=0; else if( k>=N )k=N-1; F+=k;
 Real a=Arg-k,c=F[1]-F[0],f1=0.0,f0=0.0;
  if( k>0    )f0=( F[1]-F[-1] )/2.0;          // производные по хордам смежных
  if( k<N-1  )f1=( F[2]-F[0]  )/2.0;          // узлов аргумента функции или
  if( !k     )f0 = ( c-f1*Ihp )*Iph;          // экстраполяционное упреждение
  if( k==N-1 )f1 = ( c-f0*Ihp )*Iph;          // на концах интервала
  if( D )*D = f0 + a*( 2*(c-f0) + (3*a-2)*(f0+f1-2*c) );  // запрос производной
 return F[0] + a*( f0 + a*( c-f0 + (a-1)*(f0-2*c+f1) ) ); // собственно функция
}
//   Случай двумерной поверхности на регулярной сетке
//
Real Hermite2D( Real **F,_Real X,_Real Y, int Nx, int Ny )
{ Real FY[4]; int y=Y-1.0;                    // Расчет индекса ближайшей точки
  if( X<-1 || Y<-1 || Y>Ny || X>Nx )return 0; // может лучше +или- 10,001 км?
  if( y<0 )y=0; else if( y>Ny-4 )y=Ny-4;      //
  for( int k=0; k<4; k++ )FY[k]=Hermite( F[y+k],X,Nx );
  return Hermite( FY,Y-y,4 );
}

