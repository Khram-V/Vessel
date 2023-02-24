//
//      Простая интерполяция "на месте" по Эрмиту второго порядка
//
#ifndef __Hermite__
#define __Hermite__
#include "Vector.h"
//
//   Интерполяция внутри индексов массива (-1+) с помощью Эрмита второй степени
//
Real   Hermite( Real *Y, Real *X,_Real Arg, int N, Real *D=NULL );
Vector Hermite( Vector *Y, Real *X,_Real Arg, int N, Vector *D=NULL );
//
//   Интерполяция по Эрмиту 2-го порядка на регулярных неравномерных узлах
//                                                  с осредненными производными
Vector Hermite( Vector *F, _Real Arg, int N, Vector *D=NULL );  // по целым узлам
Real   Hermite( Real *F,_Real Arg,int N,Real *D=NULL );        // - по их номерам
//
//   Случай двумерной поверхности на регулярной сетке Z{ X[Nx],Y[Ny] }
//
Real Hermite2D( Real **F,_Real X,_Real Y, int Nx, int Ny );
#endif
