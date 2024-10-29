///
///   Первый пакет векторно-тензорных математических преобразований
///      (c)2013, В.Храмушин, Санкт-Петербургский университет
///
//    Традиционные векторные операции обособлены в аналитической геометрии
//
#include "Vector.h"
//
// ... или арифметико-логические операции с векторами в (не)дружественной среде
//
//                                                             встречный вектор
Vector operator - ( Vector a ){ a.x=-a.x; a.y=-a.y; a.z=-a.z; return a; }       // (Vector){ -a.x,-a.y,-a.z }; }
Vector operator * ( Vector c,_Real w ){ c.x*=w; c.y*=w; c.z*=w; return c; }     // (Vector){ c.x*w,c.y*w,c.z*w }; }
Vector operator * ( _Real w,Vector c ){ c.x*=w; c.y*=w; c.z*=w; return c; }     //  return (Vector){ w*c.x,w*c.y,w*c.z }; }
Vector operator / ( Vector c,_Real w ){ c.x/=w; c.y/=w; c.z/=w; return c; }     //  return (Vector){ c.x/w,c.y/w,c.z/w }; }
Vector operator + ( Vector c,_Vector e){ c.x+=e.x,c.y+=e.y,c.z+=e.z; return c; } // (Vector){c.x+e.x,c.y+e.y,c.z+e.z}; }
Vector operator - ( Vector c,_Vector e){ c.x-=e.x,c.y-=e.y,c.z-=e.z; return c; } // (Vector){c.x-e.x,c.y-e.y,c.z-e.z}; }

bool operator + ( _Vector a ){ return a.x!=0 || a.y!=0.0 || a.z!=0.0; }
bool operator ! ( _Vector a ){ return a.x==0 && a.y==0.0 && a.z==0.0; }
bool operator ==( _Vector a,_Vector b ){ return a.x==b.x&&a.y==b.y&&a.z==b.z; }
bool operator !=( _Vector a,_Vector b ){ return a.x!=b.x||a.y!=b.y||a.z!=b.z; }
//
//   %-скалярное, &-покомпонентное и *-векторное умножение свободных векторов
//   =
//   * - нормаль - векторное произведение для ориентированной площади основания
//   & - покомпонентное перемножение, изменение масштабов или внесение поправок
//   % - проекция от скалярного произведения векторов - сброс косого скольжения
//                                или = скалярное произведение проекции вектора
//
Real operator % ( _Vector a,_Vector b ){ return a.x*b.x+a.y*b.y+a.z*b.z; }
Vector& Vector::operator*=( _Vector d )  // произведение ортогонального вектора
  { Real a=y*d.z-z*d.y,b=z*d.x-x*d.z; z=x*d.y-y*d.x; x=a; y=b; return *this; }
Vector operator * ( Vector a,_Vector b ){ return a*=b; }
                // покомпонентное произведение/сопоставление скалярных поправок
Vector& Vector::operator&=( _Vector d ){ x*=d.x,y*=d.y,z*=d.z; return *this; }
Vector operator & ( Vector a,_Vector b ){ return a&=b; }
  //
  //      Три поворота вектора относительно совместной координатной системы
  //
Vector& Vector::rotX( _Real a )
            { Real c=cos(a),s=sin(a),w=y*c-z*s; z=z*c+y*s; y=w; return *this; }
Vector& Vector::rotY( _Real a )
            { Real c=cos(a),s=sin(a),w=z*c-x*s; x=x*c+z*s; z=w; return *this; }
Vector& Vector::rotZ( _Real a )
            { Real c=cos(a),s=sin(a),w=x*c-y*s; y=y*c+x*s; x=w; return *this; }
//
//     Элементарные и раскрываемые операции-функции
//
const Real  sqr( _Real a ){ return a*a; }
const Real  abs( _Real a ){ return fabs( a ); }
const Real norm( _Real a,_Real b ){ return a*a + b*b; }
const Real norm( _Real a,_Real b,_Real c ){ return a*a + b*b + c*c; }
const Real norm(_Vector a ){ return a.x*a.x + a.y*a.y + a.z*a.z; }
const Real  abs(_Vector a ){ Real n=norm(a); if(n>0)return sqrt(n); return 0; }
Vector dir( _Vector a ){ Real n=norm(a); if(n>0)return a/sqrt(n); return Zero; }
Vector In3( _Vector A, _Vector B, _Vector C, _Real x ) // Кривая в трёх точках
 { return (A*(x-1)*x + C*x*(x+1))/2 - B*(x+1)*(x-1); } // в точках A:-1,B:0,C:1
Vector In2( _Vector A, _Vector B, _Real x )      // Прямая линия по двух точкам
 { return A*(1-x) + B*x; }                       // единичных координат A:0,B:1
//
//    В геометрических построениях и физических операциях тензоры
//        служат для фиксации физических полей в локальных базисах,
//    и - для взаимного преобразования координат векторных отсчетов
//                           и пространственных физических объектов
#include "Tensor.h"
//
// const Real r=0.70710678118654752440084436210485; // sqrt(1/2)
// const Real q=0.57735026918962576450914878050196; // sqrt(1/3)
//
//    Опубличенные set.конструкторы для заполнения матриц существующих тензоров
//
Matrix& Matrix::set( _Vector a )       // задается ось Х и другие Y,Z - абы как
 { z = ( x=a )*dir( y=abs( a )*dir( (Vector){-a.z,a.y,a.x}*a ) ); return *this;
 }
Matrix& Matrix::set( _Vector a,_Vector b )         // по двум векторам строится
 { z=(x=a)*(y=b); z/=sqrt( abs(z) ); return *this; // нормаль к параллелограмму
 }                                                 // по корню опорной площади
Tensor& Tensor::init(){ x.x=y.y=z.z=xi.x=yi.y=zi.z=d=1.0;   // Декартовый базис
   x.y=x.z=y.x=y.z=z.x=z.y=xi.y=xi.z=yi.x=yi.z=zi.x=zi.y=0.0; return *this; }
Tensor& Tensor:: init(_Real R){ x.x=y.y=z.z=R; xi.x=yi.y=zi.z=1/R; d=R*R*R;
   x.y=x.z=y.x=y.z=z.x=z.y=xi.y=xi.z=yi.x=yi.z=zi.x=zi.y=0.0; return *this; }
Tensor& Tensor::det()                                   // Эвклидовы координаты
 { d=(x*y)%z,xi=(y*z)/d,yi=(z*x)/d,zi=(x*y)/d; return *this;     // ++ обратный
 }
Tensor& Tensor::operator=( _Matrix m ){ x=m.x,y=m.y,z=m.z; return det(); }
//
//  Умножение\произведение тензорных объектов:
//   ортогональную матрицу поворота можно приставлять справа (домножать на нее)
//    !! надо бы ... продумать соответствующие позиции векторов строка/столбец,
//      чтобы получающиеся произведения проходили простыми векторными свертками
//
Matrix& Matrix::operator *= ( _Matrix r ){           // преобразование тензоров
  #if 0                                              //! ?как бы надо домножать
    x=(Vector){ x.x*r.x.x + x.y*r.y.x + x.z*r.z.x,   //! ?? только на обратный!
                x.x*r.x.y + x.y*r.y.y + x.z*r.z.y,   // использовано тензорное
                x.x*r.x.z + x.y*r.y.z + x.z*r.z.z }, // правило суммирования
    y=(Vector){ y.x*r.x.x + y.y*r.y.x + y.z*r.z.x,   // немых индексов
                y.x*r.x.y + y.y*r.y.y + y.z*r.z.y,   // : абсолютный базис с
                y.x*r.x.z + y.y*r.y.z + y.z*r.z.z }, //   нижними индексами
    z=(Vector){ z.x*r.x.x + z.y*r.y.x + z.z*r.z.x,   //   расположен слева,
                z.x*r.x.y + z.y*r.y.y + z.z*r.z.y,   // * локальный - справа
                z.x*r.x.z + z.y*r.y.z + z.z*r.z.z };
  #else
    x=(Vector){ x%r.x,x%r.y,x%r.z }, // !! - странно, что даже при такой записи
    y=(Vector){ y%r.x,y%r.y,y%r.z }, // матриц, все равно получается сложение
    z=(Vector){ z%r.x,z%r.y,z%r.z }; // произведений абы как - по столбцам *(-)
  #endif
  return *this;
}
//   Блок основных пространственно-геометрических преобразований
//
Tensor& Tensor::operator*=( _Real s )           // - простое изменение масштаба
      { x*=s; y*=s; z*=s; d*=(s*s*s); xi/=s,yi/=s,zi/=s; return *this; }
Tensor& Tensor::operator/=( _Real s )           // - или то же, но наоборот
      { x/=s; y/=s; z/=s; d/=(s*s*s); xi*=s,yi*=s,zi*=s; return *this; }
Tensor& Tensor::operator*=( _Matrix m )  // подъем к абсолютной системе отсчета
      { *this = Matrix::operator*=( m ); return det(); }
Tensor& Tensor::operator /= ( _Tensor r ){           // преобразование тензоров
#if 1                                                //! ?как бы надо домножать
   x=(Vector){ x.x*r.xi.x + x.y*r.yi.x + x.z*r.zi.x, //!  - только на обратный!
               x.x*r.xi.y + x.y*r.yi.y + x.z*r.zi.y, // использовано тензорное
               x.x*r.xi.z + x.y*r.yi.z + x.z*r.zi.z },  // правило суммирования
   y=(Vector){ y.x*r.xi.x + y.y*r.yi.x + y.z*r.zi.x,    // немых индексов
               y.x*r.xi.y + y.y*r.yi.y + y.z*r.zi.y,    // : абсолютный базис с
               y.x*r.xi.z + y.y*r.yi.z + y.z*r.zi.z },  //   нижними индексами
   z=(Vector){ z.x*r.xi.x + z.y*r.yi.x + z.z*r.zi.x,    //   расположен слева,
               z.x*r.xi.y + z.y*r.yi.y + z.z*r.zi.y,    // * локальный - справа
               z.x*r.xi.z + z.y*r.yi.z + z.z*r.zi.z };
#else
    x=(Vector){x%r.xi,x%r.yi,x%r.zi},
    y=(Vector){y%r.xi,y%r.yi,y%r.zi},
    z=(Vector){z%r.xi,z%r.yi,z%r.zi};
#endif
  return det();
}
//   Переход из локального базиса к абсолютной системе координат
//
Vector operator * ( _Matrix m, Vector v ){ return v=(Vector)
#if 1
       { m.x.x*v.x + m.y.x*v.y + m.z.x*v.z,      // умножение вектора справа
         m.x.y*v.x + m.y.y*v.y + m.z.y*v.z,      // на собственный тензор слева
         m.x.z*v.x + m.y.z*v.y + m.z.z*v.z };    // - возврат из ЛСК в АСК
#else
       { m.x % v, m.y % v, m.z % v };            // вектор локальный справа с
#endif
     }                                           // и опусканием на АСК из ЛСК
Vector operator * ( Vector v, _Matrix m )       // или проекции в базисные оси
     { return v=(Vector){ v%m.x, v%m.y, v%m.z }; }
//
//   Трансформация вектора для перехода в локальный ортогональный базис
//               ( дуальную систему координат )
//
Vector operator / ( Vector v, _Tensor m )       // подъем индексов в операции
     { return v=(Vector)                           // произведения с лок.базисом
  #if 0
       { v.x*m.xi.x + v.y*m.yi.x + v.z*m.zi.x,   // умножение левого вектора на
         v.x*m.xi.y + v.y*m.yi.y + v.z*m.zi.y,   // обратный тензор справа
         v.x*m.xi.z + v.y*m.yi.z + v.z*m.zi.z }; // - переход из АСК в ЛСК
  #else
       { v % m.xi, v % m.yi, v % m.zi };         // и снова - есть вопросы ???
  #endif
     }
//Vector& Vector::operator /= ( _Tensor m ){ return *this=Vector(*this)/m; }
//   { const Vector v=*this; x=v%m.xi; y=v%m.yi; z=v%m.zi; return *this; }

Matrix roll( _Real a,_Vector v )       // поворот относительно произвольной оси
   { const Real c=cos( a ),s=sin( a ),c1=1-c; return (Matrix){
     { v.x*v.x*c1+c,     v.x*v.y*c1-v.z*s, v.x*v.z*c1+v.y*s },
     { v.y*v.x*c1+v.z*s, v.y*v.y*c1+c,     v.y*v.z*c1-v.x*s },
     { v.z*v.x*c1-v.y*s, v.z*v.y*c1+v.x*s, v.z*v.z*c1+c   } }; }
Matrix rolX( _Real a )
   { const Real c=cos(a),s=sin(a); return (Matrix){{1,0,0},{0,c,-s},{0,s,c}}; }
Matrix rolY( _Real a )
   { const Real c=cos(a),s=sin(a); return (Matrix){{c,0,s},{0,1,0},{-s,0,c}}; }
Matrix rolZ( _Real a )
   { const Real c=cos(a),s=sin(a); return (Matrix){{c,-s,0},{s,c,0},{0,0,1}}; }

Tensor& Tensor::axiX( _Real a )
    { x.x=1,x.y=x.z=y.x=z.x=0,y.y=z.z=cos(a),z.y=-(y.z=sin(a)); return det(); }
Tensor& Tensor::axiY( _Real a )
    { y.y=1,y.x=y.z=x.y=z.y=0,z.z=x.x=cos(a),x.z=-(z.x=sin(a)); return det(); }
Tensor& Tensor::axiZ( _Real a )
    { z.z=1,z.x=z.y=x.z=y.z=0,x.x=y.y=cos(a),y.x=-(x.y=sin(a)); return det(); }

//Tensor& Tensor::rotX( _Real a ){ return *this *= rolX( a ); }
//Tensor& Tensor::rotY( _Real a ){ return *this *= rolY( a ); }
//Tensor& Tensor::rotZ( _Real a ){ return *this *= rolZ( a ); }
Tensor& Tensor::Rotate( _Real a, _Vector v )     // поворот по произвольной оси
                               { return Tensor::operator *= ( roll( a,v ) ); }
Tensor& Tensor::rotX( _Real a ){ return Tensor::operator *= ( rolX( a ) ); }
Tensor& Tensor::rotY( _Real a ){ return Tensor::operator *= ( rolY( a ) ); }
Tensor& Tensor::rotZ( _Real a ){ return Tensor::operator *= ( rolZ( a ) ); }
//
//    Переходы в локальной Vector-in и глобальной Point-out системах координат
//
Point Base::out( _Vector a ){ return (Point)*this + (Matrix)*this * a; }
Vector Base::in( _Point A ){ return Vector( A-(Point)*this ) / (Tensor)*this; }
//
//    Полные повороты движущегося объекта в пространстве относительно осей:
//      x(a)- крен на левый борт вниз, правое крыло вверх;
//      y(b)- дифферент/тангаж - корма вниз, нос вверх (на взлёт);
//      z(c)- рыскание - по курсу вправо         { Roll,  Pitch,  Yaw }
//      приводится к последовательному исполнению: axiX(a).rotY(b).rotZ(c)
//
Matrix Krylov(_Real a,_Real b,_Real c ) // крен, дифферент, рыскание в радианах
{ return (Matrix)                       // c1*a2*b3
  { (Vector){ cos( c )*cos( b ),sin( c )*cos( b ),-sin( b ) },
    (Vector){ sin( a )*sin( b )*cos( c )-cos( a )*sin( c ),
              sin( a )*sin( b )*sin( c )+cos( a )*cos( c ),sin( a )*cos( b ) },
    (Vector){ cos( a )*sin( b )*cos( c )+sin( a )*sin( c ),
              cos( a )*sin( b )*sin( c )-sin( a )*cos( c ),cos( a )*cos( b ) }
  };
// (Vector){ cos( b )*cos( c ),sin( a )*sin( b )*cos( c )-cos( a )*sin( c ),
//                             cos( a )*sin( b )*cos( c )+sin( a )*sin( c ) },
// (Vector){ cos( b )*sin( c ),sin( a )*sin( b )*sin( c )+cos( a )*cos( c ),
//                             cos( a )*sin( b )*sin( c )-sin( a )*cos( c ) },
// (Vector){-sin( b ),sin( a )*cos( b ),cos( a )*cos( b ) } };
}
//    "Эйлеровы" углы из математической энциклопедии
//     ( последовательность в иных осях: а-прецессия, b-нутация и с-вращение )
//
Matrix Euler( _Real a,_Real b,_Real c )   // а-вперед; b-вправо; с-вверх
{ return (Matrix){                        // a-прецессия, b-нутация, с-вращение
   (Vector){ cos( a )*cos( c )-sin( a )*cos( b )*sin( c ),
            -cos( a )*sin( c )-sin( a )*cos( b )*cos( c ), sin( a )*sin( b ) },
   (Vector){ sin( a )*cos( c )+cos( a )*cos( b )*sin( c ),
            -sin( a )*sin( c )+cos( a )*cos( b )*cos( c ),-cos( a )*sin( b ) },
   (Vector){ sin( b )*sin( c ),sin( b )*cos( c ),cos( b ) } };
}
//    Смещение базисных осей для тензора инерции (иначе по Гюйгенсу-Штейнеру)
//    M - тензор инерции; С - координаты центра масс; R - новая точка отсчета
//
static Matrix SH( _Vector A )
     { return (Matrix){ (Vector){  A.y*A.y+A.z*A.z,-A.x*A.y,-A.x*A.z },
                        (Vector){ -A.y*A.x, A.x*A.x+A.z*A.z,-A.y*A.z },
                        (Vector){ -A.z*A.x,-A.z*A.y, A.x*A.x+A.y*A.y } };
     }
Matrix Steiner( _Matrix M,_Vector C ){ return M-SH( C ); }   // просто к центру
Matrix Steiner( _Matrix M,_Vector C,_Vector R )  // Сначала к базисному центру,
              { return M - SH( C ) + SH( R ); } // затем к новой точке вращения
/*
Matrix& Matrix::Krylov(_Real a,_Real b,_Real c ) // крен - дифферент - рыскание
{ x.x=cos( b )*cos( c ),x.y=sin( a )*sin( b )*cos( c ) - cos( a )*sin( c ),
                        x.z=cos( a )*sin( b )*cos( c ) + sin( a )*sin( c );
  y.x=cos( b )*sin( c ),y.y=sin( a )*sin( b )*sin( c ) + cos( a )*cos( c ),
                        y.z=cos( a )*sin( b )*sin( c ) - sin( a )*cos( c );
  z.x=-sin( b ),z.y=sin( a )*cos( b ),z.z=cos( a )*cos( b );  return *this;
}
Matrix& Matrix::Euler(_Real a,_Real b,_Real c )  // а-вперед; b-вправо; с-вверх
{ x.x= cos( a )*cos( c )-sin( a )*cos( b )*sin( c ), // a-прецессия, b-нутация
  x.y=-cos( a )*sin( c )-sin( a )*cos( b )*cos( c ), // с-вращение
  x.z= sin( a )*sin( b ),y.x= sin( a )*cos( c )+cos( a )*cos( b )*sin( c ),
  y.y=-sin( a )*sin( c )+cos( a )*cos( b )*cos( c ),y.z=-cos( a )*sin( b ),
  z.z= sin( b )*sin( c ),z.y= sin( b )*cos( c ),z.z= cos( b ); return *this;
}*/
