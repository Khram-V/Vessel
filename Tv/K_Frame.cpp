//
//   г---------------------------------------------------------¬
//   ¦                                                         ¦
//   ¦       Численное определение одного шпангоута            ¦
//   ¦                                                         ¦
//   ¦  X(0),Y(0) - точка киля                                 ¦
//   ¦  X(1),Y(1) - точка на сломе борта к палубе (ширстрек)   ¦
//   ¦  X(-1),Y... -//- то же - левого борта                    ¦
//   ¦                                                         ¦
//   ¦                                                         ¦
//   L=========================================================-
//                    2001.11.02, В.Храмушин, НИС "П.Гордиенко",
//                                            Татарский пролив
#include "Hull.h"
#define eps 1.0e-12

Frame:: Frame( int l ){ z=y=_z=_y=0; N=0; X=0.0; if( l>0 )allocate( l ); }
Frame::~Frame()       { free(); }

Frame& Frame::allocate( int l )
{ if( l<=0 )free(); else
    { if( _z ){ Allocate( 0,_z ),Allocate( 0,_y ); _z=_y=0; }
      z=(Real*)Allocate( l*sizeof(Real),z );
      y=(Real*)Allocate( l*sizeof(Real),y ); N=l-1;
    } return *this;
}
Frame& Frame::free()
{ if( z ){ Allocate( 0,z ); Allocate( 0,y );
    if(_z )Allocate( 0,_z ),Allocate( 0,_y );
  } z=y=_z=_y=0; N=0; return *this;
}
void Frame::Double( int k )
{ int i; z=(Real*)Allocate( (N+2)*sizeof(Real),z );
         y=(Real*)Allocate( (N+2)*sizeof(Real),y ); ++N;
  for( i=N; i>k; i-- ){ z[i]=z[i-1];
                        y[i]=y[i-1];
}                     }
//
//   Предварительный расчет для построения сплайн-функции
//
Frame& Frame::SpLine()
{ if( N<3 )
  { if( _z )Allocate( 0,_z ),Allocate( 0,_y ); _z=_y=0; return *this;
  }
 Real *u,*v,*zi=z+1,
            *yi=y+1;
     u=(Real*)Allocate( N*sizeof( Real )*2 ); v=u+N;
   (_z=(Real*)Allocate( (N+1)*sizeof( Real ),_z ))[N]; *_z=0;
   (_y=(Real*)Allocate( (N+1)*sizeof( Real ),_y ))[N]; *_y=0;
  for( int i=1; i<N; i++,zi++,yi++ )
  { _z[i]=-1.0/( _z[i-1]+4.0 );
    _y[i]=-1.0/( _y[i-1]+4.0 );
    if( zi[1]==zi[0] || zi[0]==zi[-1] )u[i-1]=v[i-1]=u[i]=v[i]=0.0; // _zi всё-таки аргумент
    else u[i]=_z[i]*( u[i-1]-6.0*(zi[1]-2.0*zi[0]+zi[-1]) ),
         v[i]=_y[i]*( v[i-1]-6.0*(yi[1]-2.0*yi[0]+yi[-1]) );
  }
//  for( int i=1; i<N; i++,zi++,yi++ )
//  { _z[i]=-0.5/(_z[i-1]/2+2); u[i]=-2*_z[i]*(3*(zi[1]-2*zi[0]+zi[-1])-u[i-1]/2);
//    _y[i]=-0.5/(_y[i-1]/2+2); v[i]=-2*_y[i]*(3*(yi[1]-2*yi[0]+yi[-1])-v[i-1]/2);
//  }
  for( int i=N-1; i>=0; i-- ){ _z[i]=_z[i]*_z[i+1]+u[i];
                               _y[i]=_y[i]*_y[i+1]+v[i];
                             }  Allocate( 0,u ); return *this;
}                               //
Real Frame::Z( Real A )         // Параметрическое определение
{                               // аппликаты шпангоута по
  if( A<0 || A>1 )return 0;     // аргументу: 0.0<=A<=1.0
 int k;    A*=N;              //
 double b; k=max( min( int( A ),N-1 ),0 ); b=A-k;
  if( _z )
  { double a=1-b;
    return a*z[k]+b*z[k+1]+( (a*a-1)*a*_z[k]+(b*b-1)*b*_z[k+1] )/6;
  } return z[k]+b*( z[k+1]-z[k] );
}                             //
Real Frame::Y( Real A )       // Параметрическое определение
{                             // ординаты шпангоута по
  if( A<0 || A>1 )return 0;   // аргументу: 0.0<=A<=1.0
 int k;    A*=N;              //
 double b; k=max( min( int( A ),N-1 ),0 ); b=A-k;
  if( _y )
  { double a=1-b;
    return a*y[k]+b*y[k+1]+( (a*a-1)*a*_y[k]+(b*b-1)*b*_y[k+1] )/6;
  } return y[k] + b*( y[k+1]-y[k] );
}
void Frame::YZ( Real A, Real &Y, Real &Z )
{ if( A<0 ){ Y=0; Z=z[0]; return; } //
  if( A>1 ){ Y=0; Z=z[N]; return; } // Аргумент: 0.0<=A<=1.0
 int k;      A*=N;                  //
 double b;   k=max( min( int( A ),N-1 ),0 ); b=A-k;
  if( y[k]==y[k+1] && z[k]==z[k+1] )Y=y[k],Z=z[k]; else
  if( _y )
  { double a,a1,b1;
    a=1-b;
    a1=(a*a-1)*a/6;
    b1=(b*b-1)*b/6; Y = a*y[k] + b*y[k+1] + a1*_y[k] + b1*_y[k+1];
                    Z = a*z[k] + b*z[k+1] + a1*_z[k] + b1*_z[k+1];
  } else
  { Y = y[k] + b*( y[k+1]-y[k] );
    Z = z[k] + b*( z[k+1]-z[k] );
  }
}
Real Frame::operator()( Real _z_ ) // эмуляция плазовой ординаты
{ int k;                             //  Y - однозначная функция
  for( k=0;;k++ )                    //
  { Real d=z[k+1];                    //
    if( k>=N-1 || d>=_z_ )             // линейная интерполяция
    { d-=z[k]; if( d<eps )return y[k]; // индексного аргумента
      return Y( (k+(_z_-z[k])/d)/N );  //
} } }                           //
int Frame::iZ( Real &_y_ )      // последовательный поиск
{ static int k=0;               // корней неоднозначной функции
  Real a,b;
  int i;
// union{ Real r; point s; }a,b;  //
  a=y[k]-_y_;
  for( i=k;;i++ )
  { if( i>=N )return k=0;                   // ничего не найдено
    if( fabs( a )<eps ){ _y_=z[k]; break; } // попадание в точку
    b=y[i+1]-_y_;                   //
    if( a*b<0 )                     // пересечение по интервалу аргумента
    { _y_=Z( (i-a/(b-a))/N ); break;
    } a=b;
  } k=i+1; return k;    // возвращвется порядковый номер найденной точки
}
//   г---------------------------------------------------------¬
//   ¦                                                         ¦
//   ¦ Блок операторов для корпуса в целом                     ¦
//   ¦                                                         ¦
//   L=========================================================-
//
Real Hull::operator()( Real x, Real z )
{
 Real A,S; int k;             //
  if( x>Stx( z ) )return 0;     // Поиск ближайшего левого индекса
  if( x<Asx( z ) )return 0;     //
  for( k=0; k<Ns-2 && x>F[k+1].X; k++ ); S=F[k+1]( z );
                                         A=F[k]( z );
 return max( A+(S-A)*(x-F[k].X)/(F[k+1].X-F[k].X),Real( 0.0 ) );
}
//   г---------------------------------------------------------¬
//   ¦                                                         ¦
//   ¦ Грузовой размер и площадь смоченной поверхности корпуса ¦
//   ¦                                                         ¦
//   L=========================================================-
//
void Hull::Init()
{
 const int mZ=48;       //
   int i,j;             // простой расчет полного объема корпуса
  Real x,Dx,dx,         //      (по внутренним трапециям)
       z,Dz,dz,         //
       y,dy,Z,          //
       Vs,Vm;           // Площадь шпангоута
       Z=Draught-Do;    //
       dz=Z/mZ;
       Vm=Volume=Surface=0;
  for( i=0; i<Ns; i++ )
  { Vs=0.0;
    x=F[i].X;
    if( i==Ns-1 )dx=x;  else dx=F[i+1].X;
    if( !i      )dx-=x; else dx-=F[i-1].X;
    dy=dx*dz;                           // dy здесь удвоено
    for( j=0,z=Do; j<=mZ; z+=dz,j++ )   // Водоизмещение
    { Vs+=y=(*this)( x,z )*dy;          // Объем корпуса
      if( y>0 )
      { if( !j )Surface+=y*dx;
        Dx=( (*this)( x+dx/2,z )-(*this)( x-dx/2,z ) )/dx;
        Dz=( (*this)( x,z+dz/2 )-(*this)( x,z-dz/2 ) )/dz;
        Surface+=sqrt( 1+Dx*Dx+Dz*Dz )*dy; // Смоченная поверхность корпуса
    } } Volume+=Vs;
    if( Vs>Vm ){ Vm=Vs; if( Ms<0 )Ms=-1-i; }
  }                     if( Ms<0 )Ms=-1-Ms;

}
//
//   Несколько программ анализа "Curve-Point" векторов
//
Curve& Curve::allocate( int Sz )
{ _C=(Point*)Allocate( (N=Sz)*sizeof( Point ),_C ); return *this; }
Curve& Curve::free(){ if( _C )_C=(Point*)Allocate( N=0,_C ); return *this; }
#define  _X( k ) ( _C[(k)].x )
int Curve::find( Real Ar )       // Двоичный поиск ближайшего левого
{ int i=0,k,n=N-1,d=_X(n)>_X(0); //  индекса для заданного аргумента
  while( n-i>1 )                 //   с учетом знака его прироста
  {   k=(n+i)/2;                 //
    if( d ){ if( Ar<_X(k) )n=k; else i=k; }
      else { if( Ar>_X(k) )n=k; else i=k; }
  } return i;
}
Real Curve::operator()( Real Ar )
{  int k;
 Point *P=_C+( k=find( Ar ) );
  return P[0].y + ( Ar-P[0].x )*(P[1].y-P[0].y)/(P[1].x-P[0].x);
}
//      Подготовка экстремумов функции и аргумента
//
void Curve::Extreme( Field& Fm )
{ for( int i=0; i<N; i++ )
  { Point P=_C[i];
    if( i )
    { if( Fm.Jy>P.y )Fm.Jy=P.y; else if( Fm.Ly<P.y )Fm.Ly=P.y;
      if( Fm.Jx>P.x )Fm.Jx=P.x; else if( Fm.Lx<P.x )Fm.Lx=P.x;
    } else
    { Fm.Lx=( Fm.Jx=P.x )+1.0e-12;
      Fm.Ly=( Fm.Jy=P.y )+1.0e-12;
  } } Fm.Ly-=Fm.Jy;
      Fm.Lx-=Fm.Jx;
}
