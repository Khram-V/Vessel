#include <Math.h>
#include <StdLib.h>
#include "Hull.h"
                        //
const  double g=9.8106, // Ускорение свободного падения
              ro=1.025; //  и плотность морской воды
static double WL;       // Длина поперечной волны
                        //
double Plaze::Amplint( const double &La )
{   int i,j;                  //
 double X,Z,Wk=2.0*M_PI/La,   // Волновое число
            Mw=sqrt( WL/La ); // Число Маха
 Point A={ 0,0 },W;           // Интегральная амплитуда
  for( i=Nx-1; i>=0; i--  )   //
  { Z=0;
    W=0;
    X=Wk*dX*(Nx-1-i)/Mw;
    for( j=Nz-1; j>=0; j-- )
    { W += exp( (Point){ -Z,X } )*QV[j][i];
      Z += Wk*dZ;
    } A += W;
     Wx[i]=norm( A )*Mw/sqrt( Mw*Mw-1 )/La/La;
  } return norm( A )*Mw/sqrt( Mw*Mw-1 )/La/La;
}
double Plaze::QG4( const double &Xl, const double &Xu )
{ double A = 0.5*( Xu+Xl ),B=Xu-Xl,C,D;
         C = .4305681557970263 * B;
         D = .1739274225687269 * ( Amplint( A+C )+Amplint( A-C ) );
         C = .1699905217924281 * B;
  return B * (D+.3260725774312731 * ( Amplint( A+C )+Amplint( A-C )));
}
Real Plaze::Michell( Real Fn )
{   int i;      //
 double Rw=0.0, // Искомое волновое сопротивление
          Vo,   // Скорость в м/с
           l;   // Аргумент интегрирования
                //
  Vo = Fn*sqrt( g*Lwl );                //
  WL = 2.0*M_PI * Vo*Vo/g;              // Макс. длина волны
#if 0                                   //
  for( j=0; j<Nz;   j++ )
  for( i=0; i<Nx-1; i++ )               //
  { QV[j][i]=( Y[j][i+1]-Y[j][i] )*dZ;  // Распределеные источники
//  QV[j][i] = dX*dZ*X; //*Wide;        // с коэффициентом наклона
//  YY[j][i] =( Y[j][i]+Y[j][i+1] )/2.0;
//  if( j>Nz ) //>0 )
//  { Z = ( Y[j][i]-Y[j-1][i] )/Dz;
//    QV[j][i] *= sqrt( 1+ X*X+ Z*Z );  //= sqrt( 1+ X*X );
//  }
  }
#endif
                                        //
 const int la=24;                       // Количество дроблений по волнам
 static                                 //
 double La[la+1]={ 0.0 };               // Фазовая длина волны
       l=double( 1.0 )/la;              //
  for( i=1; i<=la; i++ )La[i]+=La[i-1]+sin( M_PI_2*l*i )*sqrt( 1.0-l*i );
  for( i=1; i<=la; i++ )La[i]*=WL/La[la];
  for( i=1; i<=la; i++ )if( La[i]>dX )
                        Rw+=QG4( La[i-1],La[i] );
 return 4*g*ro*Rw;
}
#if 0
struct Michell{ Real Vo;        // Скорость в м/с
                Real Vl;        // Водоизмещение
                Real Sf;        // Смоченная поверхность
                Real Rw;        // Сопротивление
                Real Cw;        // Коэффициент сопротивления
} Michell_Results={ 0,0,0,0 };  //
  Michell_Results.Vo=Vo;        // Скорость в м/с
  Michell_Results.Vl=Vl;        // Водоизмещение
  Michell_Results.Sf=Sf;        // Смоченная поверхность
  Michell_Results.Rw=Rw;        // Сопротивление
  Michell_Results.Cw=Cw;        // Коэффициент сопротивления

  for( j=0; j<Nz; j++ )                         //
  { Z = Dz*Wk*j;        if( Z>69 )continue;     //
    X = Dx/2;           // -0.5*Length;         // От носа к корме
    for( i=0; i<Nx-1; i++)                      //
    { A += QV[j][i] * exp( complex( -Z,Wk*X/Mw ) );
      X += Dx;
  } }

#endif
