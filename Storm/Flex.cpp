//
//    Flex — пространственная вектор-функция абсолютного скалярного аргумента
//         = параметрическое построение неразрывного трехмерного контура/кривой
//         # отслеживание векторных величин в зависимости от шага во времени
//         k>=0 є { 0÷len-1 } — положительные индексы с прямым отсчётом P[k]
//         k<0  є { len+k÷0 } — отрицательные ≡ обратный отсчёт  P[len-k]
//
#include "Aurora.h"

Vector operator * (_Vector a,_Vector b ){ return (Vector){ a.y*b.z-a.z*b.y,
                                                           a.z*b.x-a.x*b.z,
                                                           a.x*b.y-a.y*b.x }; }
Real operator %( _Vector a,_Vector b ){ return a.x*b.x+a.y*b.y+a.z*b.z; }
bool operator==( _Vector a,_Vector b ){ return a.x==b.x&&a.y==b.y&&a.z==b.z; }
bool operator!=( _Vector a,_Vector b ){ return a.x!=b.x||a.y!=b.y||a.z!=b.z; }


Vector operator - ( Vector a ){ a.x=-a.x; a.y=-a.y; a.z=-a.z; return a; }
Vector operator * ( _Real w,Vector c ){ c.x*=w; c.y*=w; c.z*=w; return c; }
Vector operator * ( Vector c,_Real w ){ c.x*=w; c.y*=w; c.z*=w; return c; }
Vector operator / ( Vector c,_Real w ){ c.x/=w; c.y/=w; c.z/=w; return c; }
Vector operator + ( Vector c,_Vector e){c.x+=e.x,c.y+=e.y,c.z+=e.z; return c; }
Vector operator - ( Vector c,_Vector e){c.x-=e.x,c.y-=e.y,c.z-=e.z; return c; }

Matrix& Matrix::axiY( _Real a )
{ y.y=1,y.x=y.z=x.y=z.y=0,z.z=x.x=cos(a),x.z=-(z.x=sin(a)); return *this; }
Matrix& Matrix::axiZ( _Real a )
{ z.z=1,z.x=z.y=x.z=y.z=0,x.x=y.y=cos(a),y.x=-(x.y=sin(a)); return *this; }
//Vector operator * ( _Matrix m,_Vector v )      // и опусканием на АСК из ЛСК
//{ return (Vector){ v%m.x,v%m.y,v%m.z }; }      // или проекции в базисные оси
Vector operator / ( _Vector v,_Matrix m )        // подъем индексов в операции
{ Real d=(m.x*m.y)%m.z;                          // чисто для тензоров инерции
  Vector xi=(m.y*m.z)/d,yi=(m.z*m.x)/d,zi=(m.x*m.y)/d;
  return (Vector){ v % xi, v % yi, v % zi };     // произведения с лок.базисом
}

Matrix& Matrix::operator *= ( _Matrix r ){           // преобразование тензоров
    x=(Vector){ x%r.x,x%r.y,x%r.z }, // !! - странно, что даже при такой записи
    y=(Vector){ y%r.x,y%r.y,y%r.z }, // матриц, все равно получается сложение
    z=(Vector){ z%r.x,z%r.y,z%r.z }; // произведений абы как - по столбцам *(-)
//  Scale*=r.Scale;                  // без влияния на действующий масштаб
  return *this;
}
static Matrix roll(_Real a,_Vector v ) // поворот относительно произвольной оси
{ const Real c=cos( a ),s=sin( a ),c1=1-c; return (Matrix){
     { v.x*v.x*c1+c,     v.x*v.y*c1-v.z*s, v.x*v.z*c1+v.y*s },
     { v.y*v.x*c1+v.z*s, v.y*v.y*c1+c,     v.y*v.z*c1-v.x*s },
     { v.z*v.x*c1-v.y*s, v.z*v.y*c1+v.x*s, v.z*v.z*c1+c },1.0 }; }
Matrix rolZ( _Real a )
     { const Real c=cos( a ),s=sin( a );
       return (Matrix){ { c,-s,0 },{ s,c,0 },{ 0,0,1 },1.0 }; }
Vector& Vector:: rotZ(_Real a )
     { Real c=cos(a),s=sin(a),w=x*c-y*s; y=y*c+x*s; x=w; return *this; }
Matrix& Matrix::Rotate( _Real a, _Vector v )     // поворот по произвольной оси
                               { return Matrix::operator *= ( roll( a,v ) ); }
Matrix& Matrix::rotZ( _Real a ){ return Matrix::operator *= ( rolZ( a ) ); }
Matrix Krylov(_Real a,_Real b,_Real c ) // крен, дифферент, рыскание в радианах
{ return (Matrix)                       // c1*a2*b3     -- классика !!
  { { cos( c )*cos( b ),sin( c )*cos( b ),-sin( b ) },
    { sin( a )*sin( b )*cos( c )-cos( a )*sin( c ),
      sin( a )*sin( b )*sin( c )+cos( a )*cos( c ),sin( a )*cos( b ) },
    { cos( a )*sin( b )*cos( c )+sin( a )*sin( c ),
      cos( a )*sin( b )*sin( c )-sin( a )*cos( c ),cos( a )*cos( b ) },1.0 };
}
Vector Matrix::AtL( _Vector A ) const { return (Vector){ A%x,A%y,A%z }/Scale; }
Vector Matrix::LtA( _Vector A ) const { return Scale*(Vector)
                  { x.x*A.x+y.x*A.y+z.x*A.z,     // умножение вектора справа
                    x.y*A.x+y.y*A.y+z.y*A.z,     // на собственный тензор слева
                    x.z*A.x+y.z*A.y+z.z*A.z }; } // - возврат из ЛСК в АСК
//
//     Смещение базисных осей для тензора инерции (иначе по Гюйгенсу-Штейнеру)
//     M - тензор инерции; С - координаты центра масс; R - новая точка отсчета
//
static Matrix SH( _Vector A )
     { return (Matrix){ {  A.y*A.y+A.z*A.z,-A.x*A.y,-A.x*A.z },
                        { -A.y*A.x, A.x*A.x+A.z*A.z,-A.y*A.z },
                        { -A.z*A.x,-A.z*A.y, A.x*A.x+A.y*A.y },1.0 }; }
Matrix Steiner( _Matrix M,_Vector C ){ return M-SH( C ); }   // просто к центру
Matrix Steiner( _Matrix M,_Vector C,_Vector R )  // Сначала к базисному центру,
              { return M - SH( C ) + SH( R ); } // затем к новой точке вращения

Real  abs(_Vector a ){ return sqrt( norm( a ) ); }
Real norm(_Vector a ){ return a.x*a.x+a.y*a.y+a.z*a.z;}
Vector dir( _Vector a ){ Real n=norm(a); if(n>0)return a/sqrt(n); return Zero; }

void e6( Real &R ){ R = round( R*1e5 )/1e5; }
void e6( Vector &W )        // округление записи для точных сравнений 0.01 мм
   { e6( W.x );
     e6( W.y ); // W.y<=eps?0.0:e5( W.y );
     e6( W.z );
   }
Real& angle( Real &A ){ return A=remainder( A,_Pd  ); }           // -180°÷180°
//{ if( A>=0 )A=fmod( A,_Pd ); else A=_Pd-fmod(-A,_Pd ); return A; } // 0°÷360°
Real angle( _Real A,_Real B ){ return remainder( A-B,_Pd ); }     // A-B: -п÷п
Vector& angle( Vector &A){ angle( A.x ),angle( A.y ),angle( A.z ); return A; }
Vector operator ~(_Vector v){ return (Vector){v.x,-v.y,v.z}; } // другого борта
//
//    ... из точки (a) в точку (b) с объемной стрелкой в долях её длины
//
const Real* arrow( const Real *_a,const Real *_b, _Real l, const colors clr )
{ Vector &a=*(Vector*)_a,&b=*(Vector*)_b,d=a-b; d*=l ;// abs( d );
  Vector e={ d.z/5,d.x/5,d.y/5 },f={ e.z,e.x,e.y },g=b+d/2; line( a,d+=b,clr );
   glBegin( GL_LINE_LOOP ),dot( g ),dot( d+e ),dot( b ),dot( d-e ),
                           dot( g ),dot( d+f ),dot( b ),dot( d-f ),
                           dot( g ),glEnd(); return _b;
}
void axis( Place &P,_Real L,_Real Y,_Real Z,
  const char *x, const char *y, const char *z, const colors clr )
{ arrow( (Vector){ 0,0,-Z },(Vector){ 0,0,Z },0.0l,clr ),
  arrow( (Vector){ 0,-Y,0 },(Vector){ 0,Y,0 },0.01 ),
  arrow( (Vector){ -L,0,0 },(Vector){ L,0,0 },0.0l ); color( clr,-0.5 );
  P.Text( _North,0,0,Z,z )
   .Text( _North,0,Y+L/100,0,y )
   .Text( _North_East,L*1.01,0,0,x );
}
//
//! Процедуры интерполяции и быстрой работы с расширяемыми последовательностями
//
        Flex:: Flex(): len( 0 ),P( NULL ){}
        Flex::~Flex(){ if( len ){ Allocate( 0,P ); len=0; P=NULL; } }
Vector& Flex:: operator[]( int k )              // выборка внутренней точки [k]
  { return P[ minmax( 0,k>=0?k:k+len,len-1 ) ]; }
Vector& Flex::Insert( int k )                   // вставка точки по индексу [k]
  { int l=isAlloc( (void*)P )/sizeof( Vector ); //= длина внутри памяти массива
      k=minmax( 0,k,len ); ++len;               //  от нуля до следующей ячейки
    if( l<=len )P=(Vector*)Allocate( (len+120)*sizeof(Vector),P );        //+48
    for( l=len-1; l>k; l-- )P[l]=P[l-1]; return P[k];
  }                              // одна точка вытаскивается из векторного ряда
void Flex::Delete( int k )       //   без укорочения числового массива в памяти
  { if( k>=0 && k<len )for( --len; k<len; k++ )P[k]=P[k+1];
  }
Vector& Flex::operator+=(_Vector R){ return Insert(len)=R;} // оконечный вектор
Vector& Flex::operator/=(_Vector R){ return Insert(0)=R;} // замещение в начале

bool intor( _Real F,_Real S,_Real G )          // с включением базовой точки G
          { return G>F ? F<S^S>G :             //       S>F && S<=G == ]F<S<=G]
                   G<F ? F>S^S<G : S==G; }     //       S<F && S>=G == [G<=S<F[
bool intoi( _Real F,_Real S,_Real G ){ return (S-F)*(S-G)<=0.0; } // с захватом
bool intox( _Real F,_Real S,_Real G ){ return (S-F)*(S-G)<0.0; }  //   и без
//    Интерполяция с разрезанием шпангоутов для фиксации точек на ватерлинии
//     ... отчасти исключаются из рассмотрения вершины пустых треугольников
Real Inter( _Real x,_Real x0,_Real x1,_Real y0,_Real y1 ) //~ любое направление
{ if( x==x0 )return y0;                                 // точное совпадение по
  if( x==x1 )return y1;                                 // крайним аргументам и
//if( x==x1 || x0==x1 )return y1;                       // повторное совпадение
  if( x0==x1 )return (y0+y1)/2;                         // среднеарифметическое
              return y0+(x-x0)*(y1-y0)/(x1-x0);         // ? и беда при малом h
}
Vector Inter( _Real x, _Real x0,_Real x1, _Vector v0,_Vector v1 )
{ if( x==x0 )return v0; if( x==x1 )return v1; if( x0==x1 )return 0.5*(v0+v1);                       // среднеарифметическое
  return v0 + ( (x-x0)/(x1-x0) )*( v1-v0 );             // беда при малом x1-x0
}
#if 0                                 /// -- временно  исключено
operator Flex::Vector*(){ return P; } // адресный доступ ко всему вектору точек
Vector Flex::Get( int k )         // выборка с изъятием из существующего списка
{ Vector Q; if( len<=0 )Q=0.0; else   // в пустой последовательности нет данных
  { Q=P[k=minmax(0,k,--len)]; while(++k<=len)P[k-1]=P[k]; } return Q;
}
Vector Flex::operator()( Real arg )           +++ необходима проверка на len==0
  { if( arg<0.0 )return *P; else                 // аргумент: 0.0<=A<=1.0
    if( arg>1.0 )return *(P+(len-1)); else       // в обход индексного контроля
    { int k=minmax( 0,int(arg*=len-1),len-2 );   // * интервальный отсчёт
      Vector *R=P+k; return R[0]+(R[1]-R[0])*(arg-k);  // линейная интерполяция
    }                           // последовательность в трехмерном пространстве
  }
//!    Параболическая интерполяция (второго порядка) по трём точкам (варианты)
//                             аргумент внутри интервала аргумента для [A-B-C]
inline Vector x_Value( _Vector A, _Vector B, _Vector C, _Real x )
{ Real a=x-A.x,b=x-B.x,c=x-C.x,ab=A.x-B.x,bc=B.x-C.x,ac=A.x-C.x;
       return A*(b*c/ab/ac) - B*(a*c/ab/bc) + C*(a*b/ac/bc);
}
inline Vector y_Value( _Vector A, _Vector B, _Vector C, _Real y )
{ Real a=y-A.y,b=y-B.y,c=y-C.y,ab=A.y-B.y,bc=B.y-C.y,ac=A.y-C.y;
       return A*(b*c/ab/ac) - B*(a*c/ab/bc) + C*(a*b/ac/bc);
} //! #else
inline Vector x_Value( _Vector a, _Vector b, _Vector c, _Real x )
{ Vector ba=(b-a)/(b.x-a.x),cb=(c-b)/(c.x-b.x);
       return a + ba*(x-a.x) + (cb-ba)*((x-a.x)*(x-b.x)/(c.x-a.x)); }
inline Vector y_Value( _Vector a, _Vector b, _Vector c, _Real y )
{ Vector ba=(b-a)/(b.y-a.y),cb=(c-b)/(c.y-b.y);
       return a + ba*(y-a.y) + (cb-ba)*((y-a.y)*(y-b.y)/(c.y-a.y));
}
#endif
