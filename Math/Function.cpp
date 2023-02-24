//
//!   Function.сpp                                             /2008.11.09/
//!   Программы построения скалярных одно-двух-трехмерных контурных функций
//
#include <math.h>
#include "Function.h"
#include "../Tools/Allocate.h"
//
//!     Построение гладких числовых одномерных и параметрических функций
//      Функция может быть определена на произвольном массиве аргументов
//      Если аргумент не задан, то им считается номер индекса точки
//           (т.е. - единичный шаг от 0 до length-1)
//
Function::Function():_Y(0),_X(0),_2(0),met(easy),m(0),length(0){}
Function::Function( size_t L ):_X(0),_2(0),met(easy),length(L),m(L)// если есть
                  { _Y=(Real*)Allocate( length*sizeof( Real ) ); }
Function::Function( Real *Y,size_t L ): // без длины при использовании Allocate
                   _Y(Y),_X(0),_2(0),met(easy),length(L),m(0) // m=0 к признаку
                  { if( !L )length=isAlloc(Y)/sizeof(Real); } // внешнего ряда
Function::Function( Real *Y, Real *X, size_t L ):           // два внешних ряда
                  _Y(Y),_X(X),_2(0),met(easy),length(L),m(0)// одинаковой длины
                  { if( !L )length=isAlloc( Y )/sizeof( Real ); }
Function::~Function() // + надо быть осторожным и не удалять чужие массивы
{ if( m )             //   если есть m - ряды числе внутренние
  { if( _Y )Allocate( 0,_Y );
    if( _X )Allocate( 0,_X );
  } if( _2 )Allocate( 0,_2 ); _2=_Y=_X=NULL; length=m=0; met=easy;
}
//  In/Out  доступ к конкретной точке
//     память для вектора-аргумента выделяется при любом первом обращении
//
Function& Function::check( int &k ) // в первом обращении
{ if( k<0 )k=0;          // с нулевым индексом будет выделено 48 новых точек
  if( k>=length )        // в случае требования увеличения длины числового ряда
  { met=easy;            // сброс к простой ломаной при попытке увеличения ряда
    if( (!m && !length) || (m && length) )  // либо пусто, либо - всё синхронно
    if( k>=m )                             // реальное расширение объёма памяти
    {         _Y=(Real*)Allocate( (m=k+48)*sizeof(Real),_Y ); // запас 48 точек
      if( _X )_X=(Real*)Allocate( m*sizeof( Real ),_X ); }
    if( !m )k=length-1; else length=k+1;
  } return *this;
}
Real& Function::operator[]( int k ){ check( k ); return _Y[k]; }
Real& Function::operator()( int k ){ check( k ); // если вектора аргумента
  if( !_X )                                      // еще нет, то он выделяется,
  { if( !m ){ static Real Em; return Em=k; }     // внешним индекс назначается,
     _X=(Real*)Allocate( m*sizeof( Real ) );     // заполняется целыми числами
    for( size_t i=0; i<length; i++ )_X[i]=i;     // что замедляет работу,
  } return _X[k];                                // но при этом особо не влияет
}                                                // на результат
//
//   Подготовка коэффициентов сплайна - Numerical Recipes, Cambridge university
//
Function& Function::SpLine( _Real yp1,  // Активизация Сплайн-интерполяции
                            _Real ypn ) // Производная в начале и конце отрезка
{ if( length>=3 )                       // если нет 4 точек, то - выход
  { int  i,n=length-1;
    Real un,qn,*u,*y=_Y;
      _2=(Real*)Allocate( length*sizeof( Real ),_2 );
       u=(Real*)Allocate( n*sizeof( Real ) );
    if( _X )                         // метод прогонки для трехлинейной матрицы
    { Real p,sig,*x=_X;              // вариант с неравномерным шагом аргумента
      if( yp1>0.99e6 )_2[0]=u[0]=0.0; else
        { _2[0]=-0.5; p=x[1]-x[0]; u[0]=3*( (y[1]-y[0])/p - yp1 )/p;
        } x++,y++;
      for( i=1; i<n; i++,x++,y++ )
      { sig=( x[0]-x[-1] )/( x[1]-x[-1] ); p=sig*_2[i-1]+2; _2[i]=( sig-1 )/p;
        un = ( y[1]-y[0] )/( x[1]-x[0] ) - ( y[0]-y[-1] )/( x[0]-x[-1] );
        u[i]=(6.0*un/(x[1]-x[-1])-sig*u[i-1])/p;
      }
      if( ypn>0.99e6 )qn=un=0.0; else
        { qn=0.5; p=x[0]-x[-1]; un=3*( ypn-( y[0]-y[-1] )/p )/p;
        }                               //
    } else                              // несколько упрощенный вариант расчета
    { if( yp1>0.99e6 )_2[0]=u[0]=0.0;   //   для единичного шага аргумента
                 else _2[0]=-0.5,u[0]=3*( y[1]-y[0]-yp1 ); y++;
      for( i=1; i<n; i++,y++ )
        u[i]=(_2[i]=-1/(_2[i-1]+4) )*(-6*( y[1]-2*y[0]+y[-1] )+u[i-1] );
      if( ypn>0.99e6 )qn=un=0.0; else qn=0.5,un=3*( ypn-y[0]+y[-1] );
    }                      _2[n]=( un-qn*u[n-1] )/( qn*_2[n-1]+1.0 );
    for( i=n-1; i>=0; i-- )_2[i]=_2[i]*_2[i+1]+u[i]; Allocate( 0,u );
         met = spline;
  } else met = easy; return *this;
}
//      Расчет первых производных в узлах интерполируемой функции (a+b)/a = a/b
//                                                                 м = (1+√5)/2
#define Iph 1.6180339887498948482          // граничные веса на золотом сечении
#define Ihp 0.3819660112501051518          // диагональ к стороне пятиугольника

Function& Function::First() // производные берутся по среднему арифметическому,
{ if( length>=3 )           //    что приводит к небольшому сглаживанию в узлах
  { Real *y=_Y+1; size_t k;
    _2 = (Real*)Allocate( length*sizeof( Real ),_2 );
    if( _X )
    { Real *x=_X+1;
      for( k=1; k<length-1; k++,x++,y++ )
       _2[k] = ( (y[1]-y[0])/(x[1]-x[0]) + (y[0]-y[-1])/(x[0]-x[-1]) )/2.0;
       _2[0] = ((_Y[1]-_Y[0])/(_X[1]-_X[0])-_2[1]*Ihp )*Iph;
       _2[length-1] = ((y[0]-y[-1])/(x[0]-x[-1])-_2[length-2]*Ihp )*Iph;
    } else
    { for( k=1; k<length-1; k++,y++ )_2[k] = ( y[1]-y[-1] )/2.0;
       _2[0] = ( _Y[1]-_Y[0]-_2[1]*Ihp )*Iph;
       _2[length-1] = ( y[0]-y[-1]-_2[length-2]*Ihp )*Iph;
    }    met=first;
  } else met=easy; return *this;
}
//   оператор ориентируется на контекстные установки исходной функции
//
Real Function::operator()( _Real Ar )
{ Real *y,a,B,h=1.0; int k;
  if( length<2 )return length?_Y[0]:0.0; // на всякий случай и контроль сойдет
  if( _X )                              // задан неравномерный аргумент - поиск
  { k=find( _X,Ar,length ); Real *x=_X+k; y=_Y+k;
    if( !(h=(x[1]-x[0])) )return (y[0]+y[1])/2.0; B=(a=Ar-x[0])/h;
  } else
  { k=minmax( 0,int( Ar ),int(length-2) ); a=B=Ar-k; y=_Y+k;
  }
  if( !a )return y[0];                  // точное попадание в узел
  if( _2 )
  { Real *d=_2+k;
    if( met==spline )                   // производные для сплайн-интерполяции
    { Real A=1-B;                       //   обратный отсчет для схемы Горнера
      return A*y[0]+B*y[1] + ( ( A*A-1 )*A*d[0] + ( B*B-1 )*B*d[1] )*h*h/6.0;
    } else
    if( met==first )                    // производные в узлах функции
    { Real c=(y[1]-y[0])/h;             // текущий градиент здесь на сохранялся
      return y[0] + a*( d[0] + B*( c-d[0] + (B-1)*( d[0]+d[1]-2*c ) ) );
    }
  } return y[0] + B*( y[1]-y[0] );
}
//   функция может принудительно переключаться на полином p-степени, если p>=0
//
Real Function::value( _Real Ar, int p )      // Интерполяция по Лагранжу p>=0
{ if( p<0 )return (*this)( Ar );             // если p<0 - обычная интерполяция
 int i,kb,kf,n=length-1;                     // Поиск ближайшего левого индекса
  if( _X )i=find( _X,Ar,length ); else i=minmax( 0,int( Ar ),n );
  if( p>n )p=n;                              // степень на один узел меньше
  if( !p )return _Y[i]; kb=( kf=i+(p/2)+1 )-p;
  if( kf>n )kf=n,kb=n-p; else                // центрованные пограничные сдвиги
  if( kb<0 )kb=0,kf=p;                       // при сохранении степени полинома
 Real po,pi,yo=0; i=kb;
  do{ po=pi=1.0;  n=kb;
    do if( i!=n )
    { if( _X )pi*=_X[i]-_X[n],po*=Ar-_X[n];  // если задан аргумент, иначе
        else  pi*=i-n,po*=Ar-n;              // достаточно номеров индексов
    } while( ++n<=kf ); yo+=_Y[i]*po/pi;     // (можно if !_X вынести за цикл)
  }   while( ++i<=kf ); return yo;
}
//   в расчетах производной повторяются управляющие циклы интерполяции
//
Real Function::derivative( _Real Ar, int p ) // первая производная
{ if( p<0 )                                  // и здесь будут сплайны с Эрмитом
  { Real *y,*d, a,b,q,c,h=1.0;               //
    if( _X )                                 // если задан неравномерный
    { int k=find( _X,Ar,length );            //   аргумент
      Real *x=_X+k; y=_Y+k; d=_2+k; h=x[1]-x[0]; a=Ar-x[0]; b=a/h;
    } else                                   // параметрический/единичный шаг
    { int k=minmax( 0,int( Ar ),int(length-2) ); y=_Y+k; d=_2+k; a=b=Ar-k;
    }                                        //
    c=( y[1]-y[0] )/h; q=1-b;                // градиент и смежные интервалы
    if( _2 )                                 //
    { if( met==spline )return c - ( (3*q*q-1)*d[0] - (3*b*b-1)*d[1] )*h/6; else
      if( met==first )return d[0] + b*( 2*(c-d[0]) + (3*b-2)*(d[0]+d[1]-2*c) );
    } return c;                              //
  } else                                     // Полиномиальная производная
  { int i,l,kb,kf,n=length-1;                // Поиск ближайшего левого индекса
    if( !_X )i=minmax( 0,int( Ar ),n ); else i=find( _X,Ar,length );
    if( p>n )p=n;                            // степень на один узел меньше
    if( !p )return 0; kb=( kf=i+(p/2)+1 )-p; //
    if( kf>n )kf=n,kb=n-p;                   // центрованные пограничные сдвиги
    if( kb<0 )kb=0,kf=p;                     // при сохранении степени полинома
   Real p2,pi,y1=0,s1; i=kb;                 //
    do{ pi=1.0; s1=0;  n=kb;                 //
      do if( i!=n )                          // если задан аргумент, иначе
      { p2=1.0;                              //
        pi *= _X ? _X[i]-_X[n] : i-n;        // достаточно номеров индексов
        for( l=kb; l<=kf; l++ )if( l!=i && l!=n )p2 *= _X ? Ar-_X[l] : Ar-l;
        s1 += p2;
      } while( ++n<=kf ); y1+=_Y[i]*s1/pi;
    }   while( ++i<=kf ); return y1;
  }
}
//  Выборка экстремумов путем простого перебора узлов функции и аргумента
//
Function&
Function::extrem( Real &xMin, Real &yMin, Real &xMax, Real &yMax, bool his )
{ Real *y=_Y; size_t k=0;                     // предотвращение деления на ноль
  if( !his ){ yMax=( yMin= *y-1e-6 )+2e-6; ++k; }  // с точностью до миллионной
  while( k<length ){ if(yMin>*y)yMin=*y; else if(yMax<*y)yMax=*y; k++; y++; }
  if( _X )
  { Real *x=_X; k=0; if( !his ){ xMax=( xMin= *x-1e-6 )+2e-6; ++k; }
    while( k<length ){ if(xMin>*x)xMin=*x; else if(xMax<*x)xMax=*x; k++; x++; }
  } else
  { if( !his ){ xMin=0,xMax=length-1; } else
    { if( xMin>0 )xMin=0; if( xMax<length-1 )xMax=length-1; }
  } return *this;
}
