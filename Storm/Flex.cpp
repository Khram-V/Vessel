//
//    Flex — пространственная вектор-функция абсолютного скалярного аргумента
//         = параметрическое построение неразрывного трехмерного контура/кривой
//         # отслеживание векторных величин в зависимости от шага во времени
//         k>=0 є { 0÷length-1 } — положительные индексы с прямым отсчётом P[k]
//         k<0  є { length+k÷0 } — отрицательные ≡ обратный отсчёт  P[length-k]
//
#include "Vessel.h"

        Flex:: Flex(): length( 0 ),P( NULL ){}
        Flex::~Flex(){ if( length ){ Allocate( 0,P ); length=0; P=NULL; } }
Vector& Flex:: operator[]( int k )
{ return P[minmax(0,k>=0?k:k+length,length-1)]; // выборка внутренней точки [k]
}
Vector& Flex::Insert( int k )                   // вставка точки по индексу [k]
{ int l=isAlloc( (void*)P )/sizeof( Vector );   //= длина внутри памяти массива
      k=minmax( 0,k,length ); ++length;         //  от нуля до следующей ячейки
  if( l<=length )P=(Vector*)Allocate( (length+1020)*sizeof(Vector),P );   //+48
  for( l=length-1; l>k; l-- )P[l]=P[l-1]; return P[k];
}
Vector Flex::Delete( int k )      // одна точка вытаскивается и векторного ряда
{ Vector V={ 0,0,0 };             //  без укорочения числового массива в памяти
   if( length>0 )if( (k=minmax(0,k,length))<length )
     for( V=P[k]; k<length-1; k++ )P[k]=P[k+1]; return V;
}
#if 0
Vector& Flex::operator+=(_Vector R){ return Insert(length)=R; } // конец списка
Vector& Flex::operator/=(_Vector R){ return Insert(0)=R; }// замещение в начале

Vector Flex::Get( int k )     // выборка с изъятием из уже существующего списка
{ Vector Q; if( length<=0 )Q=0.0; else // в пустой последовательности нет точек
  { Q=P[k=minmax(0,k,--length)]; while(++k<=length)P[k-1]=P[k]; } return Q;
}
Vector Flex::operator()( Real arg )        +++ необходима проверка на length==0
  { if( arg<0.0 )return *P; else                 // аргумент: 0.0<=A<=1.0
    if( arg>1.0 )return *(P+(length-1)); else    // в обход индексного контроля
    { int k=minmax( 0,int(arg*=length-1),length-2 );   // * интервальный отсчёт
      Vector *R=P+k; return R[0]+(R[1]-R[0])*(arg-k);  // линейная интерполяция
  } }                           // последовательность в трехмерном пространстве
operator Flex::Vector*(){ return P; } // адресный доступ ко всему вектору точек
#endif
//!    Параболическая интерполяция (второго порядка) по трём точкам (варианты)
#if 0
inline Vector x_Value( _Vector A, _Vector B, _Vector C, _Real x )
{ Real a=x-A.x,b=x-B.x,c=x-C.x,ab=A.x-B.x,bc=B.x-C.x,ac=A.x-C.x;
       return A*(b*c/ab/ac) - B*(a*c/ab/bc) + C*(a*b/ac/bc);
}
inline Vector y_Value( _Vector A, _Vector B, _Vector C, _Real y )
{ Real a=y-A.y,b=y-B.y,c=y-C.y,ab=A.y-B.y,bc=B.y-C.y,ac=A.y-C.y;
       return A*(b*c/ab/ac) - B*(a*c/ab/bc) + C*(a*b/ac/bc);
} //#else
inline Vector x_Value( _Vector a, _Vector b, _Vector c, _Real x )
{ Vector ba=(b-a)/(b.x-a.x),cb=(c-b)/(c.x-b.x);
       return a + ba*(x-a.x) + (cb-ba)*((x-a.x)*(x-b.x)/(c.x-a.x)); }
inline Vector y_Value( _Vector a, _Vector b, _Vector c, _Real y )
{ Vector ba=(b-a)/(b.y-a.y),cb=(c-b)/(c.y-b.y);
       return a + ba*(y-a.y) + (cb-ba)*((y-a.y)*(y-b.y)/(c.y-a.y));
}
#endif
