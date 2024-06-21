/**
      Векторные числовые объекты и арифметические операции
          (с)2016, В.Храмушин, ... Южно-Сахалинск
 **/
#ifndef __Vector_
#define __Vector_
//#include <math.h>
#include "../Type.h"
                                              //
struct Point;  typedef const Point& _Point;   // точка дальних мировых отсчетов
struct Vector; typedef const Vector& _Vector; // свободный разностный вектор
                    //
struct Vector       // Определение векторных операций в трехмерном пространстве
{ Real x,y,z;       //   ( совершенно открытый разностный и свободный объект )
  //
  //  операции с уже существующими векторами +- геометрическое сложение
  //            */ (Real) изменение длины и весьма опасное деление вектора
  //            *= площадь под нормалью двух векторов (векторное произведение)
  //
  // Vector(): x(0),y(0),z(0){}
  // Vector& operator - (){ x=-x,y=-y,z=-z; return *this; }
  Vector& operator =  ( _Real r ){ x=r; y=z=0.0; return *this; }
  Vector& operator *= ( _Real r ){ x*=r,y*=r,z*=r; return *this; }
  Vector& operator /= ( _Real r ){ x/=r,y/=r,z/=r; return *this; }
//Vector& operator =  ( _Vector d ){ x=d.x,y=d.y,z=d.z; return *this; }
  Vector& operator += ( _Vector d ){ x+=d.x,y+=d.y,z+=d.z; return *this; }
  Vector& operator -= ( _Vector d ){ x-=d.x,y-=d.y,z-=d.z; return *this; }
  // %-скалярное, &-покомпонентное и *-векторное умножение свободных векторов
  Vector& operator *= ( _Vector d ); // произведение ортогонального вектора
  Vector& operator &= ( _Vector d ); // { x*=d.x,y*=d.y,z*=d.z; return *this; }
                                     // покомпонентное произведение поправок
///??
//Vector& operator *= (_Matrix );  // выход из местного базиса в глобальную СК
//Vector& operator /= (_Tensor );  // переход в локальный ортогональный базис
//Vector& operator &= ( Vector d ){ return *this = d *= x*d.x+y*d.y+z*d.z; }
//Vector& operator ^= ( Vector n ){ return *this-=(n*=2*(x*n.x+y*n.y+z*n.z)); }
  //
  //      Три поворота вектора относительно совместной координатной системы
  //
  Vector& rotX(_Real a ); //{ Real c=cos(a),s=sin(a),w=y*c-z*s; z=z*c+y*s; y=w; return *this; }
  Vector& rotY(_Real a ); //{ Real c=cos(a),s=sin(a),w=z*c-x*s; x=x*c+z*s; z=w; return *this; }
  Vector& rotZ(_Real a ); //{ Real c=cos(a),s=sin(a),w=x*c-y*s; y=y*c+x*s; x=w; return *this; }
  //
  //      Две операции переформатирован и для открытия доступа
  //       ко всяким стандартным и различным пакетам программ
  //
//operator Real(){ return sqrt( x*x+y*y+z*z ); }  // ... пусть пока будет так
  operator Real*(){ return (Real*)this; }         // это для вектора OpenGL-3dv
//operator float*(){ static float b[3]; b[0]=x,b[1]=y,b[2]=z; return b; }
};
const Vector Zero={ 0,0,0 },Zenit={ 0,0,1 };
//
//     Арифметические операции с векторами в (не)дружественной среде
//                                                             встречный вектор
bool   operator ! ( _Vector ), operator + ( _Vector ),
       operator ==( _Vector,_Vector ), operator !=( _Vector,_Vector );
Vector operator - ( _Vector ),
       operator / ( _Vector,_Real ),
       operator * ( _Vector,_Real ),   operator * ( _Real,_Vector ),
       operator + ( _Vector,_Vector ), operator - ( _Vector,_Vector ),
       operator & ( _Vector,_Vector ), operator * ( _Vector,_Vector );
//     { return (Vector){ a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x }; }
//
//   * - нормаль - векторное произведение для ориентированной площади основания
//   & - покомпонентное перемножение, изменение масштабов или внесение поправок
//   % - проекция от скалярного произведения векторов - сброс косого скольжения
//                                    = скалярное произведение проекции вектора
Real operator % ( _Vector a,_Vector b ); // { return a.x*b.x+a.y*b.y+a.z*b.z; }
//
//     В абсолютных координатах используются упрощённые операции с точкой Point
//
struct Point     // Координатная точка в масштабах абсолютного пространства
{ Real X,Y,Z;    // обозначаются прописными буквами абсолютно зависимых величин
  Point& operator =(_Real r ){ X=r; Y=Z=0.0; return *this; }         // очистка
//Point& operator/=(_Real r ){ X/=r,Y/=r,Z/=r; return *this; }       // масштаб
  Point& operator+=(_Vector d ){ X+=d.x,Y+=d.y,Z+=d.z; return *this; } // сумма
  Point& operator-=(_Vector d ){ X-=d.x,Y-=d.y,Z-=d.z; return *this; }
  //
  //     Исходные арифметические действия с координатными точками и векторами  //
 friend Point  operator + ( Point c,_Vector e ){ return c+=e; }
 friend Point  operator - ( Point c,_Vector e ){ return c-=e; }
 friend Vector operator - (_Point c,_Point e )
             { return (Vector){ c.X-e.X,c.Y-e.Y,c.Z-e.Z }; }
//operator Vector(){ return *((Vector*)this); }
 operator Real*(){ return (Real*)this; }     // для внешних операций и графики
};
//
//     Элементарные и раскрываемые операции-функции
//
const Real  sqr(_Real a );                // { return a*a; }
const Real  abs(_Real a );                // { return fabs( a ); }
const Real norm(_Real a,_Real b );        // { return a*a + b*b; }
const Real norm(_Real a,_Real b,_Real c); // { return a*a + b*b + c*c; }
const Real norm(_Vector a );              // { return a.x*a.x+a.y*a.y+a.z*a.z;}
Real   abs(_Vector a );                  // { return sqrt( norm( a ) ); }
Vector dir(_Vector a );                  // { return a/sqrt( norm( a ) ); }
Vector In3(_Vector A,_Vector B,_Vector C,_Real x ); // Кривая в трёх точках
//{return(A*(x-1)*x+C*x*(x+1))/2-B*(x+1)*(x-1);} // с координатами A:-1,B:0,C:1
Vector In2(_Vector A,_Vector B,_Real x );        // Прямая линия по двух точкам
//{ return A*(1-x) + B*x; }                      // единичных координат A:0,B:1
#endif
