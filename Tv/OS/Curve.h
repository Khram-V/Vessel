//
//      Curve.h                                              /91.04.12/
//      Несколько программ анализа "Point" векторов
//
#ifndef __CURVE_TYPE
#define __CURVE_TYPE
#include "Type.h"
class Curve                //
{     Point *_C;           // Адрес массива
      Real *_2;            // Вторая производная в узлах сплайна
public: int N;             // Индексная длина линии
  Curve( int=0 );          // Constructor
 ~Curve();                 // Destructor
 Curve& allocate( int=0 ); // Распределение памяти
 Curve& free();            //
        operator Point* () { return _C;    } // Прямой доступ к вектору
 Point& operator[]( int k ){ return _C[k]; } // Выбор точки по индексу
 int    find( Real );            // Двоичный поиск левого индекса
 Real   operator()( Real );      // Линейная или Сплайн интерполяция
 Curve& operator+=( Point& );    // Параллельное смещение ряда
 void Linear(){ allocate( N ); } // Возврат к линейной интерполяции
 void Extreme( Field &_Fm );     // Выбор экстремумов по исходным точкам
 Real value( Real,int=1 );       // Степенная интерполяция по Лагранжу
 void SpLine();                  // Активизация Сплайн-интерполяции
//oid SpLine( Real Y1,Real Yn ); // C производной в начале и в конце отрезка
};                               //
#endif
