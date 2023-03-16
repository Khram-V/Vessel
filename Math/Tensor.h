/**
 Пакет векторно-тензорных математических преобразований в сеточном пространстве
       с локально определёнными корпускулами среди связных живых сил
         (с)2016, В.Храмушин, Санкт-Петербургский университет <GPL>
                  Иерусалим - Санкт-Петербург.
 **/
#ifndef __Matrix_Tensor_Base_
#define __Matrix_Tensor_Base_
#include "Vector.h"

struct Matrix; typedef const Matrix& _Matrix; // чистая матрица тензора
struct Tensor; typedef const Tensor& _Tensor; // тензор собственного базиса
struct Base;   typedef const Base&   _Base;   // определённый Эвклидовый базис
//
//   Тензоры служат фиксации физических полей в локальных базисах с целью
//   — взаимного преобразования векторных координат и тензорных компонент
//   — изначально задается ортогональный ортонормированный базис, который
//     изнутри всегда видится единичной и ортогональной координатной системой
//   — три вектора образуют один столбец матрицы, их компоненты - по строкам
//
struct Matrix    // три базисных вектора с быстрым доступом к их области памяти
{ Vector x,y,z;            // тривекторы Евклида косоугольной системы координат
  Matrix& set( _Vector );              // —— самые простые производные операции
  Matrix& set( _Vector,_Vector );
  Matrix& operator=( _Real a ){ x=y.y=z.z=a; y.x=y.z=z.x=z.y=0; return *this; }
  Matrix& operator+=( _Matrix m ){ x+=m.x,y+=m.y,z+=m.z; return *this; }
  Matrix& operator-=( _Matrix m ){ x-=m.x,y-=m.y,z-=m.z; return *this; }
  Matrix& operator*=( _Real s ){ x*=s,y*=s,z*=s; return *this; }
  Matrix& operator/=( _Real r ){ x/=r,y/=r,z/=r; return *this; }
  Matrix& operator*=( _Matrix );

  friend Matrix operator*( Matrix A, _Real R  ){ return A*=R; }
  friend Matrix operator*( Matrix A,_Matrix B ){ return A*=B; }
  friend Matrix operator+( Matrix A,_Matrix B ){ return A+=B; }
  friend Matrix operator-( Matrix A,_Matrix B ){ return A-=B; }
  //                                                                                                                                      //
  //   Переход из локального базиса к абсолютной системе координат
  //                                               (проекции с базисными осями)
  friend Vector operator * ( _Matrix,_Vector );     // переход из ЛСК в АСК
  friend Vector operator * ( _Vector,_Matrix );     // проекции на базисные оси
  //          { return (Vector){ v%m.x,v%m.y,v%m.z }; }
//operator Real(){ return (x*y)%z; }          //... определитель — объем триады
};
Matrix roll( _Real,_Vector ),  // поворот относительно произвольной оси-вектора
       rolX( _Real ),rolY( _Real ),rolZ( _Real );  // матрицы внешних поворотов
//
//   Смещение базисных осей для тензора инерции (Гюйгенс-Штейнер)
//   M - тензор инерции; С - координаты центра масс; R - новая точка отсчета
//
Matrix Steiner( _Matrix M,_Vector C ),           // К центру (-)C
       Steiner( _Matrix M,_Vector C,_Vector R ); // К центру (-)C, затем к (+)R
//
 //  Пространственно-геометрические повороты в радианах и по часовой стрелке
//      x(a)- крен на левый борт вниз, правое крыло вверх;
//      y(b)- дифферент/тангаж - корма вниз, нос вверх (на взлёт);
//      z(c)- рыскание - по курсу вправо         { Roll,  Pitch,  Yaw }
//      приводится к последовательному исполнению: axiX(a).rotY(b).rotZ(c)
//
Matrix Krylov( _Real,_Real=0,_Real=0 ), // крен дифферент рыскание
       Euler( _Real,_Real=0,_Real=0 );  // прецессия, нутация, вращение
                                        // { вперёд - вправо - вверх }
class Tensor:      // Арифметика линейной трансформации элементарной корпускулы
public Matrix      // — три базисных вектора, всегда ненулевые и некомпланарные
{ Vector xi,yi,zi; // — обратный тензор служит быстрому исполнению произведений
  Real d;          // — определитель - смешанное произведение базисных векторов
  //
  // пост-конструктор предваряет контроль состояния сплошной среды
  //                и допустимых операций над корпускулами
 Tensor &init(),                  // Декартов ортогональный нормированный базис
        &init( _Real R ),         // Евклидов базис прямых углов без нормировки
        &det();                   // детерминант с (обратно)сопряжённым базисом
public:
 //
 // Блок конструкторов C++ включает пересыльные процедуры прямого доступа set:
 //                     — сначала ортогональный и ортонормированный на единицу
 Tensor(){ init(); }
 Tensor(_Real m ){ init( m ); }           // ортогональный базис с масштабом: m
 Tensor(_Matrix M ): Matrix( M ){ det(); }      // базис Эвклидова пространства
 Tensor(_Vector a ){ set( a ); det(); }          // конструирование из векторов
 Tensor(_Vector a,_Vector b ){ set( a,b ); det(); }   // с ортогональными осями
 Tensor(_Vector a,_Vector b,_Vector c ):Matrix( (Matrix){ a,b,c } ){ det(); }

// Tensor& operator=( _Real m ){ init( m ); return det(); }
 Tensor& operator=( _Matrix m ); // { x=m.x,y=m.y,z=m.z; return det(); }
  //
  //   Блок основных пространственно-геометрических преобразований
  //
 Tensor &operator*=( _Real s ), &operator/=( _Real s ), // — изменение масштаба
        &operator*=( _Matrix ), // подъем проекции к абсолютной системе отсчета
        &operator/=( _Tensor ); // возврат глобального отсчета в местный базис
  //
  //   Трансформация вектора для перехода в местную/дуальную систему координат
  //                                                (вектор на обратный тензор)
 friend Vector operator / ( _Vector,_Tensor );
  //
  //   Повороты вокруг базисных (локальных) координатных осей — в радианах
  //
 Tensor &Identity(){ return init(); }  // —— ортогональный нормированный тензор
 Tensor &Rotate( _Real a,_Vector v ),  // поворот относительно произвольной оси
        &axiX(_Real), &axiY(_Real), &axiZ(_Real), // выбор ориентации и угловой
        &rotX(_Real), &rotY(_Real), &rotZ(_Real); // поворот в главных осях XYZ
  //
  //   В качестве модуля/нормы  - детерминант/определитель
  //
  // operator _Real(){ return d; }
};
struct Base: Tensor,Point                     // тензор формы с местоположением
{ Base():Tensor(),Point((Point){0,0,0}){}     // без излишних преобразований
  Base(_Real S ):Tensor( S ),Point((Point){0,0,0}){}// пространственный масштаб
  Base(_Point R,_Real S ): Tensor( S ),Point( R ){} //   -- смещение и масштаб
  Base(_Point R,_Vector a,_Vector b,_Vector c ): Point( R ),Tensor( a,b,c ){}
  Base& Identity(){ X=Y=Z=0.0; Tensor::Identity(); return *this; }
  Base& operator =( _Point R  ){ Point::operator=( R );  return *this; }
  Base& operator =( _Matrix m ){ Tensor::operator=( m ); return *this; }
  Base& operator+=( _Vector v ){ Point::operator+=( v ); return *this; }
  Base& operator-=( _Vector v ){ Point::operator-=( v ); return *this; }
  //
  //   Можно задавать переходы между абсолютной и локальной СК
  //
  Point out( _Vector a ); //{ return (Point)*this + (Matrix)*this * a; }
  Vector in( _Point A ); //{ return Vector( A-(Point)*this ) / (Tensor)*this; }
};
#endif // __Matrix_Tensor_Base_
