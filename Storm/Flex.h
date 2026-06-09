//
//      Подсекция мореходных качеств судов в штормовых условиях
//              Сахалинское отделение Российского научно-технического
//              общества судостроителей имени Алексея Николаевича Крылова
//      Подборка базовый геометрических объектов и операций       ‏יְרוּשָׁלַיִם
//
#include "..\Window\View.h"

struct Vector; typedef const Vector& _Vector; // свободный разностный вектор
struct Matrix; typedef const Matrix& _Matrix; // чистая матрица для тензора
struct Vertex; typedef const Vertex& _Vertex; //  ≡ процедурный параметр

struct Vector       // Определение векторных операций в трехмерном пространстве
{ Real x,y,z;       // ( совершенно открытый разностный и свободный объект )
  Vector& operator =  ( _Real r ){ x=r; y=z=0.0; return *this; }
  Vector& operator *= ( _Real r ){ x*=r,y*=r,z*=r; return *this; }
  Vector& operator /= ( _Real r ){ x/=r,y/=r,z/=r; return *this; }
  Vector& operator += ( _Vector d ){ x+=d.x,y+=d.y,z+=d.z; return *this; }
  Vector& operator -= ( _Vector d ){ x-=d.x,y-=d.y,z-=d.z; return *this; }
  Vector& operator &= ( _Vector d ){ x*=d.x,y*=d.y,z*=d.z; return *this; }
  //
  // Три поворота вектора относительно совместной координатной системы
  //
  Vector& rotX(_Real a){ Real c=cos(a),s=sin(a),w=y*c-z*s; z=z*c+y*s; y=w; return *this; }
  Vector& rotY(_Real a){ Real c=cos(a),s=sin(a),w=z*c-x*s; x=x*c+z*s; z=w; return *this; }
  Vector& rotZ(_Real a){ Real c=cos(a),s=sin(a),w=x*c-y*s; y=y*c+x*s; x=w; return *this; }
  operator Real*(){ return (Real*)this; }     // для внешних операций и графики
};
const Vector Zero={ 0,0,0 },Zenit={ 0,0,1 };
struct Matrix               // три базисных вектора с быстрым доступом к памяти
{ Vector x,y,z;            // тривекторы Евклида косоугольной системы координат
  Matrix &Identity(){ x=y.y=z.z=1.0; y.x=y.z=z.x=z.y=0.0; return *this; }
  Matrix &operator=( _Real a ){ x=y.y=z.z=a; y.x=y.z=z.x=z.y=0; return *this; }
//Matrix &operator=( _Matrix a ){ x=a.x,y=a.y,z=a.z; return *this; }
//
//       Операции с произвольными ортонормированными матрицами в пространстве
//
  Matrix &operator += ( _Matrix m );
  Matrix &operator -= ( _Matrix m );
  friend Matrix operator - ( Matrix A,_Matrix B );
  friend Matrix operator + ( Matrix A,_Matrix B );

  Matrix &operator*=(_Matrix ); // подъем проекции к абсолютной системе отсчета
  friend Vector operator / ( _Vector v,_Matrix m );
//friend Vector operator * ( _Matrix m,_Vector v );
//
//      Ортонормированная ортогональная матрица для координат с масштабом
//
Matrix &operator*=( _Real s ){ x*=s,y*=s,z*=s; return *this; }  // или масштаб
// Matrix &operator/=( _Real s ){ Scale/=s; return *this; }     // без контроля
 Matrix &Rotate( _Real a,_Vector v ),  // поворот относительно произвольной оси
        &axiY(_Real), &axiZ(_Real), &rotZ(_Real); // поворот в главных осях XYZ
 Vector AtL( _Vector A ) const;           // из глобального к локальному базису
 Vector LtA( _Vector A ) const;          // возвращение к абсолютному счислению
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
//     Свободные функции зависимых операций
//
Vector operator * ( _Vector, _Vector ), // ортогональное произведение вектора
       operator * ( _Real,Vector ),   operator * ( Vector,_Real ),
       operator + ( Vector,_Vector ), operator - ( Vector,_Vector ),
       operator / ( Vector,_Real ),   operator - ( Vector );
Real operator % ( _Vector,_Vector );
bool operator ==( _Vector,_Vector ),  operator !=( _Vector,_Vector );

Matrix Krylov( _Real,_Real=0,_Real=0 );          // крен дифферент рыскание
Matrix Steiner( _Matrix M,_Vector C ),           // К центру (-)C
       Steiner( _Matrix M,_Vector C,_Vector R ); // К центру(-)C, затем к(+)R
Matrix rolZ( _Real );

inline const Real sqr( _Real a ){ return a*a; }
Real abs( _Vector a ),norm( _Vector a );
Vector dir( _Vector a );
Real&  angle( Real &A );                 // -180°÷180°
Real   angle( _Real A,_Real B );         // { A-B: -п÷п }
Vector angle( Vector A );
Vector operator ~( _Vector );            // для другого борта
bool intor( _Real F,_Real S,_Real G );   // с включением базовой точки G
bool intoi( _Real F,_Real S,_Real G );   // захват
bool intox( _Real F,_Real S,_Real G );   // и без
Real Inter( _Real x,_Real x0,_Real x1,_Real y0,_Real y1 ); // любое направление
Vector Inter( _Real x, _Real x0,_Real x1, _Vector v0,_Vector v1 );

