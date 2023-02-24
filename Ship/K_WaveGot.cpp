//
//                                                                   2020.08.12
//  Расчет волнового сопротивления тонкого судна по формулам Мичелла
//  Поверенные математические модели и алгоритмы Ады Шоломовны Готман
//
#include <Math.h>
#include "..\Type.h"

const Real g=9.80665,  // g - ускорение свободного падения на Земле     [m/c²]
          ro=102.0,    // ρ - массовая плотность = ρ/g ??           [кг∙с²/м⁴⁺]
          nv=1.57e-6,  // ν - кинематическая вязкость жидкости          [м²/с]
          nu=1.35;     // коэффициент турбулентной вязкости
const int kw=157;      // число делений при интегрировании по пи (до π=3,14/2)

inline Real sqr( _Real a ){ return a*a; }
static Real Integral( int n, Real *f )
{ Real R=(f[0]+f[--n])/2; for(int i=1;i<n;i++)R+=f[i]; return R; } // трапеции
//int i; Real P=0.0,Q=0.0;           // оригинальное интегрирование А.Ш.Готман
//for(i=0;i<n;i++)if(i&1)Q+=f[i]; else P+=f[i]; return (P*2-f[0]-f[n-1]+Q*4)/3;}

void Ada_Sholomovna_Gotman
(     Real **Hull,   // Корпус
     _Real Froud,    // Число Фруда
     _Real Length,   // Длина по ватерлинии \ между перпендикулярами      [м]
     _Real Breadth,  // Ширина (на всякий случай)                         [м]
     _Real Depth,    // Осадка (под дробление по ватерлиниям)             [м]
     _Real Volume,   // Водоизмещение                                    [м³]
     _Real Surface,  // Смоченная поверхность корпуса                    [м²]
 const int Nx,       // Индексы размерности корпуса Hull по длине
 const int Nz,       //  и по осадке
     Real &RBasic,   // коэффициенты: главной части волнового сопротивления
     Real &RMichell, // ++ к собственно расчету на основе интеграла Мичелла
     Real &Residual  // ++ остаточное сопротивление с турбулентной вязкостью
)
{ const Real
      dX=Length/(Nx-1),           // реальная величина шпации
      dZ=Depth/(Nz-1),            // и промежуток между ватерлиниями
      Vm=Froud*sqrt( g*Length );  // скорость относительно тихой воды [м/с]
   // Rn=Vm*Length/nv;         - число Рейнольдса (скорость и длина к вязкости)
 Real R[kw],RT[kw],GINT[Nz],GTR[Nz],Swl[Nz],AB=1.0,EPK=1.0; //! без поправок
 bool ReStage=false;
  for( int z=0; z<Nz; z++ )Swl[z]=dX*Integral( Nx,Hull[z] ); // площади всех WL
DoublEQ:                          // однократный повтор для учёта реологии воды
  if( ReStage )                   // где для остаточного сопротивления вводятся
    AB=1.7-4.5*(Froud-0.14),      // поправки сдвига гребней носовых волн [???]
    EPK=exp(-nu/sqrt(g)/pow(Length,1.5)/pow(Froud,5));  // --- и турбулентность
  for( int k=0; k<kw; k++ )     // цикл строится по четверти круга фазовых волн
  { Real C0=cos( _Ph*Real( k )/kw ),
         C1=1.0/Froud/Froud/C0,DC=cos( 2.0*C1 ),FF=1.0/C1/C1,
                               DS=sin( 2.0*C1 ),FFF=FF/C1;   C1/=Length*C0;
    for( int z=0; z<Nz; z++ )
    { Real *y=Hull[z],H1=0,H2=0,K1=0,K2=0,F1,F2,
            V2=sqr( Swl[z]*exp( (z*dZ-Depth)*C1 )/C1 );
      for( int x=0; x<Nx-3; x++ )if( y[x]>0.0 ) // Поиск/выбор двух производных
        { K2=((y[x+2]-y[x+1])/dX-(K1=(y[x+1]-y[x])/dX))/dX; break; }
      for( int x=Nx-1; x>2; x-- )if( y[x]>0.0 ) // сначала путь от ахтерштевня
        { H2=((H1=(y[x]-y[x-1])/dX)-(y[x-1]-y[x-2])/dX)/dX; H1*=AB; break; }
      F1=( H1*K1 + H2*K2*FF )*FF;               // и затем возврат к форштевню
      F2=( H2*K1 - H1*K2 )*FFF;                 // ! заменен знак... ∫?≠f(x)
      GINT[z]=V2*(sqr(H1)+sqr(K1)+(sqr(H2)+sqr(K2))*FF)*FF;    // главная часть
      GTR[z]=GINT[z]-2.0*V2*( F1*DS+F2*DC )*EPK;               // и весь Мичелл
    } R[k]=Integral( Nz,GINT )*dZ*C0*C0*C0;
      RT[k]=Integral( Nz,GTR )*dZ*C0*C0*C0;                       // ?/cos³(θ)
  }
 Real Am=pow( 20.0/Length,3 ),     //! 20м - контрольная длина расчётной модели
      PRP=Am*(4.0*g*g*ro)/Vm/Vm/_Pi,                              // [кг/с²/м³]
   // PRP=Am*(4.0*g*ro)/Froud/Froud/_Pi/Length,                   // [кг/с²/м³]
      RTV=PRP*Integral( kw,RT )*_Ph/kw,  //100.0,
      RV=PRP*Integral( kw,R )*_Ph/kw,    //100.0,
      DZZ=2.0/Vm/Vm/Surface/ro;  // если Vm = Fn*sqrt( g*Length ) ∙∙∙ [с²/кг/м]
   // DZZ=2.0/Froud/Froud/Surface/ro/g/Length,  // здесь тоже самое ∙ [с²/кг/м]
  if( ReStage )           // коэффициенты
      Residual=RTV*DZZ;   // полного-остаточного сопротивления + вязкости и др.
  else{ RBasic=RV*DZZ,    // или главная часть и собственно интеграл по Мичеллу
        RMichell=RTV*DZZ; ReStage=true; goto DoublEQ;
      }
}
