//
//      Подсекция мореходных качеств судов в штормовых условиях
//              Сахалинское отделение Российского научно-технического
//              общества судостроителей имени Алексея Николаевича Крылова
//      Подборка базовый геометрических объектов и операций        ‏יְרוּשָׁלַיִם
//
#ifndef __Flex_
#define __Flex_
#include <StdIO.h>
#include "../Window/ConIO.h"         // консольные операции — отладочные тексты
#include "../Type.h"                 // базисный наборт типов данных и процедур

struct Vector; typedef const Vector& _Vector; // свободный разностный вектор
struct Point;  typedef const Point& _Point;   // точка дальних мировых отсчетов
struct Matrix; typedef const Matrix& _Matrix; // чистая матрица для тензора
struct Vertex; typedef const Vertex& _Vertex; //  ≡ процедурный параметр

struct Vector       // Определение векторных операций в трехмерном пространстве
{ Real x,y,z;       //   ( совершенно открытый разностный и свободный объект )
  Vector& operator =  ( _Real r ){ x=r; y=z=0.0; return *this; }
  Vector& operator *= ( _Real r ){ x*=r,y*=r,z*=r; return *this; }
  Vector& operator += ( _Vector d ){ x+=d.x,y+=d.y,z+=d.z; return *this; }
  Vector& operator -= ( _Vector d ){ x-=d.x,y-=d.y,z-=d.z; return *this; }
  Vector& operator &= ( _Vector d ){ x*=d.x,y*=d.y,z*=d.z; return *this; }
  Vector& operator /= ( _Real r ){ x/=r,y/=r,z/=r; return *this; }
  Vector &rotZ( _Real a );
  operator Real*(){ return (Real*)this; }     // для внешних операций и графики
};
struct Point     // Координатная точка в масштабах абсолютного пространства
{ Real X,Y,Z;    // обозначаются прописными буквами абсолютно зависимых величин
  Point& operator=( _Real r ){ X=r; Y=Z=0.0; return *this; }         // очистка
  friend Point operator+( Point c,_Vector e ){ c.X+=e.x,c.Y+=e.y,c.Z+=e.z; return c; }
  friend Point operator-( Point c,_Vector e ){ c.X-=e.x,c.Y-=e.y,c.Z-=e.z; return c; }
  friend Vector operator-(_Point c,_Point e ){ return (Vector){ c.X-e.X,c.Y-e.Y,c.Z-e.Z }; }
  operator Real*(){ return (Real*)this; }     // для внешних операций и графики
};
struct Matrix    // три базисных вектора с быстрым доступом к их области памяти
{ Vector x,y,z;           // тривекторы Евклида косоугольной системы координат
  Real Scale;             // если Scale>0 - матрица ортонормирована с масштабом
  Matrix &Identity(){ x=y.y=z.z=Scale=1.0; y.x=y.z=z.x=z.y=0.0; return *this; }
  Matrix &operator=(_Matrix a ){ x=a.x,y=a.y,z=a.z,Scale=a.Scale; return *this; }
  Matrix &operator=(_Real a ){ x=y.y=z.z=a; y.x=y.z=z.x=z.y=0; Scale=1; return *this; }
//
//      Операции с произвольными матрицами для случаев Scale==0.0
//
  Matrix &operator+=(_Matrix m ){ x+=m.x,y+=m.y,z+=m.z; Scale=1; return *this; }
  Matrix &operator-=(_Matrix m ){ x-=m.x,y-=m.y,z-=m.z; Scale=1; return *this; }
  friend Matrix operator-( Matrix A,_Matrix B ){ return A-=B; }
  friend Matrix operator+( Matrix A,_Matrix B ){ return A+=B; }

  Matrix &operator*=(_Matrix );  // подъем проекции к абсолютной системе отсчета
  friend Vector operator / ( _Vector v,_Matrix m );
//friend Vector operator * ( _Matrix m,_Vector v );
//
//      Ортонормированная ортогональная матрица для координат с масштабом
//
 Matrix &operator*=(_Real s ){ Scale*=s; return *this; } //— изменение масштаба
 Matrix &operator/=(_Real s ){ Scale/=s; return *this; } // без контроля
 Matrix &Rotate(_Real a,_Vector v ),   // поворот относительно произвольной оси
        &axiY(_Real), &axiZ(_Real), &rotZ(_Real); // поворот в главных осях XYZ
 Vector AtL( _Vector A ) const;           // из глобального к локальному базису
 Vector LtA( _Vector A ) const;          // возвращение к абсолютному счислению
};
struct Base: Matrix,Point                     // тензор формы с местоположением
{ Base(); // { X=Y=Z=0.0; Identity(); }       // без излишних преобразований;
  Point out( _Vector a );
  Vector in( _Point A );
};
/*      Flex — пространственная вектор-функция абсолютного скалярного аргумента
           = параметрическое построение неразрывного трехмерного контура/кривой
           # отслеживание векторных величин в зависимости от шага во времени
           k>=0 є { 0÷len-1 } — положительные индексы с прямым отсчётом P[k]
           k<0  є { len+k÷0 } — отрицательные ≡ обратный отсчёт  P[len-k]
*/
class Flex{ Vector *P;            // пространственный контур на корпусе корабля
public: int len;                  // означенная длина последовательности точек
  Flex(); ~Flex();                // специальная пространственная интерполяция
  Vector& operator[]( int k );    // доступ к конкретной точке [k]
  Vector& Insert( int k );        // вставка новой точки по индексу [k]
  void    Delete( int k );        // точка [k] выталкивается из векторного ряда
  Vector& operator += (_Vector ); // добавление новой точки р в конец списка
  Vector& operator /= (_Vector ); // замещение р в начале существующей выборки
};
//!   Каждая вершина треугольника содержит морские и корабельные координаты,
//         а также зарегистрированный на данный момент времени уровень моря
//
struct Vertex: Vector,Point       // вершина исчисляемого треугольника обоих СК
{ Real w;                         // и уровень моря над вершиной треугольника
  Vertex( _Vector V );            // конструктор и собственно    <Vs-view>
  friend bool operator != ( _Vertex a,_Vertex b );
  friend Vector operator - ( _Vector a,_Vertex b );
  friend Vector operator + ( _Vector a,_Vertex b );
};
//     Свободные функции зависимых операций
//
Vector operator * (_Vector,_Vector ), // произведение ортогонального вектора
       operator * ( _Real,Vector ),   operator * ( Vector,_Real ),
       operator + ( Vector,_Vector ), operator - ( Vector,_Vector ),
       operator / ( Vector,_Real ),   operator - ( Vector );
Real operator % ( _Vector,_Vector );
bool operator ==( _Vector,_Vector ),  operator !=( _Vector,_Vector );

Matrix Krylov( _Real,_Real=0,_Real=0 );          // крен дифферент рыскание
Matrix Steiner( _Matrix M,_Vector C ),           // К центру (-)C
       Steiner( _Matrix M,_Vector C,_Vector R ); // К центру(-)C, затем к(+)R
Matrix rolZ( _Real );

const Vector Zero={ 0,0,0 },Zenit={ 0,0,1 };
inline const Real sqr( _Real a ){ return a*a; }
Real abs( _Vector a ),norm( _Vector a );
Vector dir( _Vector a );
#endif
