//
//                                              /05.03/  85.04.02
//
//  Расчет волнового сопротивления тонкого судна по формулам Мичелла
//              -- алгоритм аспиранта Атанасова, Болгария
//
#include <Math.h>
#include "..\Type.h"

const Real g=9.8106,           // Ускорение свободного падения
           ro=1.025;           //     и плотность морской воды
static int  Nx,Nz;             // размерность исходной таблицы плазовых ординат
static Real Hx,R;              // утроенный Фруд в квадрате и ~~~волновое число

inline Real sqr( _Real a ){ return a*a; }
//             Простое интегрирование гладкой аналитически определенной функции
inline
Real QG4( _Real Xl,_Real Xu, Real F( _Real ) ) // Гаусов*4 интеграл для Мичелла
{ Real A=0.5*( Xu+Xl ),B=Xu-Xl,C,D;
   C=0.4305681557970263*B; D=0.1739274225687269*(F(A+C)+F(A-C));
   C=0.1699905217924281*B; D+=0.3260725774312731*(F(A+C)+F(A-C)); return B*D;
}
static void AtaBee( Real *_Y, Real &Ac,Real &As )        // по одной ватерлинии
{ Real X,dX=2.0/Real( Nx-1 ); Ac=As=0.0;                 // dX=(Ac+2)/(As+Nx);
   for( int j=0; j<Nx; j++ )                             // J=NA+1 TO NS-1;
   { X=R*(dX*(Nx-1-j)-1.0); Ac+=cos( X )*_Y[j];          // [ -1.0 -:- +1.0 ]
                            As+=sin( X )*_Y[j];
   } X=(1.0-cos(R*dX))/(R*R*dX)/(Nz-1); Ac*=X; As*=X;    // = 2*FFF/Nx/2;
}
static Real Tl,                        // отношение Lenght/Depth длины к осадке
            Fq,                        // скорость по Фруду в квадрате
           **Y;                        // ссылка на матрицу ординат корпуса
static Real AtaBlue( _Real Al )
{ Real C1,C2,A=0.0, Sumci=0.0,
       S1,S2,P,D2,W,Sumsi=0.0; R=Al/Fq/2; P=Tl*Al*Al/Fq/( Nz-1 );
  AtaBee( Y[Nz-1],C1,S1 );
  for( int k=Nz-2; k>=0; k-- )                                // I=KU+1 TO KD
  { if( (A+=P)>64 )D2=0; else { D2=exp( -A ); AtaBee( Y[k],C2,S2 ); }
    W=D2*C2; Sumci += C1+W; C1=W;
    W=D2*S2; Sumsi += S1+W; S1=W;
  } P=sqr( Sumci )+sqr( Sumsi ); A=sqr( Al ); return Hx<=0?P:A*P/sqrt( A-1 );
}
Real WaveRes( Real **_Hull, Real Froud,                // Корпус и число Фруда
              Real Length,Real Breadth,Real Depth,     // Длина, ширина, осадка
              Real Surface,                    // Смоченная поверхность корпуса
              int _Nx, int _Nz )               // Индексы по длине и по осадке
{ Real Wres=0,W,Xl,Xu; Y=_Hull; Nx=_Nx; Fq=sqr( Froud );
   Hx=-1;                       Nz=_Nz; Tl=Depth/Length;
   Wres=AtaBlue( 1.0 )*0.0106 + AtaBlue( Xl=1.0001 )*0.00353;
   Hx=3.33*Fq;
   W=sqr( 1.9-Froud*3.0 )+3.0;
   for( int i=0; i<3; i++,Hx*=3 )         //    DO I=1,3;
   for( Real f=1.0; f<=W; f+=1.0 )        //!+1 DO J=1 TO 3+(1.9-FROUD*3)**2;
   {
     Wres+=QG4( Xl,Xu=Xl+Hx,AtaBlue ); Xl=Xu;
   }
// return Wres * sqr( Tl*Breadth/pow( Froud,1.5 ) )/Surface/M_PI/2;
   return 2.0*Wres*ro*g*g*sqr( Depth/Froud/Froud/Froud )/Length/M_PI;
}
/**     OMEGA=SURFACE( T );
        DOUBLE=VOLUME( T );
        FROUD= V/SQRT(G*AL);         TL=T/AL;
        WRES = WAVERES((FI),(FROUD),(TL));
        TL   = WRES*RO*G*T*T*B*B/FROUD**6/AL/PI/4;
        WRES = WRES*T*T*B*B/FROUD**8/AL**2/OMEGA/PI/2;
        PUT SKIP LIST('  FROUD='!!FROUD!!'  R/W='!!TL/DOUBLE!!
                      '  R/S='!!TL/OMEGA!!' *** TIME='!!TIME!!' *** '!!L);
*/
