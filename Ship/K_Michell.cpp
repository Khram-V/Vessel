#include "Hull.h"
                       //
const  Real g=9.8106,  // Ускорение свободного падения
            ro=1.025;  //  и плотность морской воды
static Real WL=0.0;    // Длина поперечной волны
                       //
Real Plaze::Amplint( _Real La )
{ int i,j;                  //
 Real X,Z,Wk=2.0*M_PI/La,   // Волновое число
          Mw=sqrt( WL/La ); // Число Маха
 complex A={ 0,0 },W;       // Интегральная амплитуда
  for( i=Nx-1; i>=0; i-- )  //
  { Z=0;
    W=0;
    X=Wk*dX*(Nx-1-i)/Mw;
    for( j=Nz-1; j>=0; j-- )
    { W += exp( (complex){ -Z,X } )*QV[j][i];
      Z += Wk*dZ;
    } A += W;
    Wx[i]=norm( A )*Mw/sqrt( Mw*Mw-1 )/La/La;
  } return norm( A )*Mw/sqrt( Mw*Mw-1 )/La/La;
}
Real Plaze::Michell( _Real Fn, bool full )
{ int i;         //
 Real Rw=0.0,    // Искомое волновое сопротивление
      Vo,l;      // Скорость в м/с
                 //
  Vo = Fn*sqrt( Lwl*g );                //
  WL = 2.0*M_PI * Vo*Vo/g;              // Макс. длина волны
#if 0                                   //
  for( int j=0; j<Nz;   j++ )
  for(     i=0; i<Nx-1; i++ )           //
  { QV[j][i]=( Y[j][i+1]-Y[j][i] )*dZ;  // Распределённые источники
//  QV[j][i] = dX*dZ*X; //*Wide;        // с коэффициентом наклона
//  YY[j][i] =( Y[j][i]+Y[j][i+1] )/2.0;
//  if( j>Nz ) //>0 )
//  { Z = ( Y[j][i]-Y[j-1][i] )/Dz;
//    QV[j][i] *= sqrt( 1+ X*X+ Z*Z );  //= sqrt( 1+ X*X );
//  }
  }
#endif
 const int la=127;                      // Количество дроблений по волнам
 static Real La[la+1]; La[0]=La[1]=0.0; // Фазовая длина волны
      l=Real( 1.0 )/la;                 //   memset( La,0,sizeof( La ) );
  for( i=1; i<=la; i++ )La[i]+=La[i-1]+sin( M_PI_2*l*i )*sqrt( 1.0-l*i );
  for( i=1; i<=la; i++ )La[i]*=WL/La[la];
  if( full )                            // здесь расчёт волнового сопротивления
  {
#pragma omp parallel for private( i ) reduction (+: Rw)
    for( i=1; i<=la; i++ )if( La[i]>dX/6 )          // возбуждение - алиасинг
    { Real A=0.5*( La[i]+La[i-1] ),B=La[i]-La[i-1], // Аргументы интегрирования
      C=0.4305681557970263*B,D=0.1739274225687269*(Amplint(A+C)+Amplint(A-C));
      C=0.1699905217924281*B,D+=0.3260725774312731*(Amplint(A+C)+Amplint(A-C));
      Rw += B*D;
    }
  } else Amplint( La[la]-0.01 ); return 4*g*g*ro*Rw; // La[la] запредельное inf
}
#if 0
struct Michell{ float Vo;       // Скорость в м/с
                float Vl;       // Водоизмещение
                float Sf;       // Смоченная поверхность
                float Rw;       // Сопротивление
                float Cw;       // Коэффициент сопротивления
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
