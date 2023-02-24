//
//       Застарелые полиномы - POWER.c                 VER 3.2 /88.01.27/
//
#include "Complex.h"
bool Power_root( complex*,int );   // коэффициенты -> корни полинома и порядок
void Power_Vietta( complex*,int ); // восстановление комплексного полинома по его корням
void Power_Sorta( complex*,int ); // процедура сортировки корней по модулю от единицы
void Power_Binom( complex*,int ); // построение массива биномиальных коэффициентов

#define PI 3.14159265358979323846264338328
#define EPS 1.0E-12
#define EPF 1.0E-24
//
//      Вычисление значений корней полинома по схеме Горнера
static
complex HORNER( complex *PZ, int n, // массив коэффициентов и его степень
                const complex &Z )  // комплексный аргумент
{ complex WZ={0.0};                 // комплексный результат
   while( n>=0 ){ WZ=PZ[n--]+WZ*Z; } return WZ;
}
//      Расчет остаточного полинома по схеме Горнера
static
void HORNAG( complex *PZ, int n, const complex &Z )
{ complex WZ,WB;
  WZ=PZ[n]; PZ[n--]=0;
  while( n>0 ){ WB=PZ[n]; PZ[n--]=WZ; WZ=WZ*Z+WB; } PZ[0]=WZ;
}
//      Поиск корней комплексного полинома
//
#define M 3                        // степень поискового комплекса (3-симплекс)
#define KE 256                     // допустимый уровень итерации
#define LR 1.6180339887498948482   // 1.24 = коэффициент роста площади
#define KR 0.3819660112501051518   // 0.36 = коэффициент сжатия площади
bool
Power_root( complex *PZ, int N )   // коэффициенты -> корни полинома и порядок
{ complex FZ[M+1],ZF[M+1],ZR[M+1]; // минимизируемые функции и их аргументы
  double RK,WM,WR;                 // перебор корней и модули полиномов
  int i,j,k,KM,                    // отсчеты точек полинома и комплекса
         ER=0;                     // счетчик просчетов и ошибок
   ZF[0]=PZ[0];  //1.0;            //----    P O w E R    -----------------
  for( i=1; i<=M; i++ )            // форма и ориентация
   ZR[i]=polar( 1,2.0*(i-1)*PI/M );//   оптимизационного шаблона
  for( i=N; i>0; i-- )             // перебор корней по номерам
  { RK=1.0/3.0;                    // начальный радиус поиска
    for( k=1; k<=KE; k++ )         // цикл поиска корня или минимума
    { for( j=1; j<=M; j++ )ZF[j]=ZF[0]+ZR[j]*RK;       // построение
      for( j=0; j<=M; j++ )FZ[j]=HORNER( PZ,i,ZF[j] ); // оптимизацион.шаблона
      KM=0; WM=abs( FZ[0] );
      for( j=1; j<=M; j++ )        // выбор минимального значения полинома
       if( (WR=abs( FZ[j] ))<WM ){ KM=j; WM=WR; }
      if( KM==0 )RK*=KR; else{ RK*=LR; ZF[0]=ZF[KM]; }
      if( WM<EPS || RK<EPF )break;
    }
    HORNAG( PZ,i,ZF[0] );
    PZ[i]=ZF[0];                   // корень в высшую степень полинома
    ZF[0]=~ZF[0];                  // сопряженная начальная точка поиска
    if( k>KE )ER++;
  } return ER==0;
}
//    восстановление комплексного полинома по его корням
void
Power_Vietta( complex *PZ, int n )
{ complex PK;                      // умножение полинома на одночлен
  for( int ik=1; ik<=n; ik++ )
  { PK=PZ[ik]; PZ[ik]=PZ[ik-1]; PK=-PK;
    for( int ki=ik-1; ki>0; ki-- )PZ[ki]=PZ[ki]*PK+PZ[ki-1]; PZ[0]*=PK;
} }
//    процедура сортировки корней по модулю от единицы
void
Power_Sorta( complex *PZ, int n )
{ for( int i=1; i<=n; i++ ){ double WX=fabs( abs( PZ[i] )-1.0 ); int j=i;
    for( int k=i+1; k<=n; k++ ){ double WF=fabs( abs( PZ[k] )-1.0 );
      if( WX>WF ){ WX=WF; j=k; }
    } if( j!=i  ){ complex WZ=PZ[i]; PZ[i]=PZ[j]; PZ[j]=WZ; }
} }
//    построение массива биномиальных коэффициентовВ
void
Power_Binom( complex *PZ, int n )
{ int k=0;
  double v,w; PZ[0]=1;             // первый коэфф. равен единице
  for( int i=1; i<=n; i++ )        // сверху по таблице NEWTON-А
  { imag( PZ[i] )=0; v=1.0; k=i/2; // повторяем v = единице
    for( int j=1; j<=k; j++ )      // проход по половине линии
    { w=real( PZ[j] );             //  запоминаем значение сверху
        real( PZ[j] )=v+w;         //  нижний равен сумме верхних
      v=w;                         //  левый элемент запоминает правый
    } if( i&1 )real( PZ[k+1] )=real( PZ[k] );
  } for( int i=n; i>=k; i-- )real( PZ[i] )=real( PZ[n-i] );
}
