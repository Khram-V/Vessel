//
//                                              /05.03/  85.04.02
//
//  Расчет волнового сопротивления тонкого судна по формулам Мичелла
//
#include <Math.h>

const double g=9.8106, // Ускорение свободного падения
             ro=1.025; //  и плотность морской воды

static int Nx,Nz;
static double Tl,Fr, Hx, R; //Xl,Xu;

static void AtaBee( double *_Y, double &Ac, double &As )
{ double dX,X,Y;                                     //
   Ac=As=0; dX=2.0/double( Nx-1 );                   // dX=(Ac+2)/(As+Nx);
   for( int j=0; j<Nx; j++ )                         // J=NA+1 TO NS-1;
   { X=R*(dX*(Nx-1-j)-1.0); Y=_Y[j]; Ac+=cos( X )*Y; // [ -1.0 -:- +1.0 ]
                                     As+=sin( X )*Y; //
   } X=(1.0-cos( R*dX ))/( R*R*dX )/(Nz-1); Ac*=X;   // = 2*FFF/Nx/2;
                                            As*=X;   //
}                                       //
static double **Y;                      // Матрица ординат корпуса
                                        //
static double AtaBlue( const double &Al )
{
 double C1,C2,Fw,Sumci=0,W=0,A,
        S1,S2,D2,Sumsi=0,P; Fw=Fr*Fr; R=Al/Fw/2; A=Al*Al; P=Tl*A/Fw/(Nz-1);
  AtaBee( Y[Nz-1],C1,S1 );      //
  for( int k=Nz-2; k>=0; k-- )  // I=KU+1 TO KD
  { W+=P;                       //
    if( W>64 )D2=0; else { D2=exp( -W ); AtaBee( Y[k],C2,S2 ); }
    W=D2*C2; Sumci += C1+W; C1=W;
    W=D2*S2; Sumsi += S1+W; S1=W;
  }
  Fw=Sumci*Sumci+Sumsi*Sumsi; return Hx<=0 ? Fw:A*Fw/sqrt( A-1 );
}
//
// Простое интегрирование аналитически определенной функции
//
static double QG4( const double &Xl, const double &Xu,
                   double F( const double& ) )
{ double A = 0.5*( Xu+Xl ),B=Xu-Xl,C,D;
         C = .4305681557970263 * B;
         D = .1739274225687269 * ( F( A+C )+F( A-C ) );
         C = .1699905217924281 * B;
  return B * (D+.3260725774312731 * ( F( A+C )+F( A-C )));
}                               //
#include "..\Ship\Hull.h"

Real WaveRes( Real **_Hull,     // Корпус
              Real Froud,       // Число Фруда
              Real Length,      // Длина
              Real Depth,       // Осадка
               int _Nx,         // Индексы по длине
               int _Nz )        //  и по осадке
{ double Wres=0,W,Xl,Xu;        //
   Tl=Depth/Length;
   Y=_Hull;  Nx=_Nx;
   Fr=Froud; Nz=_Nz;
   Hx=-1;
   Wres=AtaBlue( 1 )*0.0106 + AtaBlue( Xl=1.0001 )*0.00353;
   Hx=3.33*Fr*Fr;
   W=1.9-Fr*3;
   W=W*W+3;
   for( int i=0; i<3; i++,Hx*=3 )       // DO I=1,3;
   for( Real j=1.0; j<=W; j++  )        // DO J=1 TO 3+(1.9-FROUD*3)**2;
   {                 Xu=Xl+Hx;
     Wres += QG4( Xl,Xu,AtaBlue );
                  Xl=Xu;
   } return 2*Wres*ro*g*pow( Depth/Froud/Froud/Froud,2 )/Length/M_PI;
}
#if 0
    FROUD= V/SQRT(G*AL);                        TL=T/AL;
    WRES = WAVERES((FI),(FROUD),(TL));
    TL   = WRES*RO*G*T*T*B*B/FROUD**6/AL/PI/4;
    WRES = WRES*T*T*B*B/FROUD**8/AL**2/OMEGA/PI/2;
    PUT SKIP LIST('  FROUD='!!FROUD!!'  R/W='!!TL/DOUBLE!!
                '  R/S='!!TL/OMEGA!!' *** TIME='!!TIME!!' *** '!!L);
#endif

