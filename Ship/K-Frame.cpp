//
//     Численное определение одного шпангоута
//
//     X(0),Y(0) - точка киля
//     X(1),Y(1) - ДП или точка на сломе борта к палубе (ширстрек)
//     X(-1),Y...-//- то же - левого борта
//
//                      2001.11.02, В.Храмушин, НИС "П.Гордиенко",
//                                              Татарский пролив
#include "Hull.h"
#define eps 1.0e-12

Frame::Frame( int l )
     { Spl=false; y=z=_x=_y=0; N=0; X=min=max=0; if( l>0 )allocate( l ); }
Frame::~Frame(){ allocate( 0 ); }
void Frame::allocate( int l )
{ Easy(); y=(Real*)Allocate( l*sizeof(Real),y );
  N=l-1;  z=(Real*)Allocate( l*sizeof(Real),z );  //! N-1 количество интервалов
         _x=(Real*)Allocate( l*sizeof(Real),_x ); /// !!! N - не точек !!!
         _y=(Real*)Allocate( l*sizeof(Real)*2,_y );
  if( l>0 ){ for( l=0; l<=N; l++ )_x[l]=l; memset( _y,0,sizeof( Real )*l*2 ); }
}
void Frame::Double( int k )
{ ++N; allocate( N+1 ); for( int i=N; i>k; i-- )z[i]=z[i-1],y[i]=y[i-1];
}
static void MM( Frame &F )
{ F.min=F.max=F(0); for( int i=1; i<=F.N; i++ )
                    { if( F.min>F(i) )F.min=F(i); if( F.max<F(i) )F.max=F(i); }
  if( Do>F.min )Do=F.min; if( Depth<F.max )Depth=F.max;
}
void Hull::MinMax()
{ Bmx=F[Ms][0]; Xo=F[0].X; Lmx=F[Ns-1].X; Do=Stx(0); Depth=Stx(Stx.N); Real S;
  MM( Sty ); MM( Stx ); for( int i=0; i<=Stx.N; i++ )Lmx=max( Lmx,Stx[i] );
  MM( Asy ); MM( Asx ); for( int i=0; i<=Asx.N; i++ )Xo=min( Xo,Asx[i] );
  for( int j=0; j<Ns; j++ )        // поиск-выборка максимальной ширины корпуса
  for( int i=0; i<=F[j].N; i++ ){ MM( F[j] ); Bmx=max( Bmx,F[j][i] ); }
  Lmx-=Xo; Bmx*=2.0; Nstem=Ns-1;         // максимальные длина и ширина корпуса
  Lwl=Stx.G( Draught,true )-Asx.G( Draught,true );       // длина по ватерлинии
  if( Ns<3 )Xm=( F[0].X+F[1].X )/2; else Xm=F[Ms].X;     // абсцисса для миделя
  Bwl=Y( Xm,Draught )*2.0;                       // ширина ватерлинии на миделе
}
//!   Предварительный расчет для построения сплайн-функции
//!   с аргументом-параметром по реальному расстоянию между точками
//
void Frame::SpLine( bool old )
{ if( N<3 ){ Easy(); return; } Spl=true; if( old )return;
 Real *xi=_x,*zi=z+1,*u,               // вариант с неравномерным шагом аргумента
      *_z,   *yi=y+1,*v;
  v=( u=(Real*)Allocate( N*sizeof(Real)*2 ) )+N; xi[0]=0.0; ++xi;
  (_z=_y+(N+1))[N]=0.0; _z[0]=0.0;
             _y[N]=0.0; _y[0]=0.0;
  for( int i=1; i<=N; i++ )_x[i]=_x[i-1]+hypot( y[i]-y[i-1],z[i]-z[i-1] );
  for( int i=1; i<N; i++,xi++,yi++,zi++ )
  if( (xi[1]-xi[0])<eps || (xi[0]-xi[-1])<eps )u[i-1]=v[i-1]=u[i]=v[i]=0.0; else // _zi всё-таки аргумент
  { Real h=xi[1]-xi[-1],sig=( xi[0]-xi[-1] )/h,
    p=sig*_y[i-1]+2,
    w=(yi[1]-yi[0])/(xi[1]-xi[0])-(yi[0]-yi[-1])/(xi[0]-xi[-1]);
    v[i]=(6*w/h-sig*v[i-1])/p; _y[i]=(sig-1)/p;
    p=sig*_z[i-1]+2;
    w=(zi[1]-zi[0])/(xi[1]-xi[0])-(zi[0]-zi[-1])/(xi[0]-xi[-1]);
    u[i]=(6*w/h-sig*u[i-1])/p; _z[i]=(sig-1)/p;
  } _y[N]=_z[N]=0.0;
  for( int i=N-1; i>=0; i-- )_y[i]=_y[i]*_y[i+1]+v[i],
                             _z[i]=_z[i]*_z[i+1]+u[i]; Allocate( 0,u );
}
/*
static int find( Real *A,_Real Ar,int N )   // Двоичный поиск ближайшего левого
{ int k,i=0; bool d=A[N]>A[0];              //  индекса для заданного аргумента
  if( Ar<=A[0] )return d?0:N-1;             //   с учетом знака его прироста
  if( Ar>=A[N] )return d?N-1:0;
  while( N-i>1 )
  { k=(N+i)/2; if( d ){ if( Ar<A[k] )N=k; else i=k; }
                 else { if( Ar>A[k] )N=k; else i=k; } } return i;
}*/
void Frame::AYZ( Real A,Real &Y,Real &Z )            // Аргумент: 0.0<= A <=1.0
{ if( A<0.0 ){ Y=0.0; Z=z[0]; return; }              //  и только для рисовалки
  if( A>1.0 ){ Y=0.0; Z=z[N]; return; } A*=_x[N];    // индексно-сплайновый
 int k=0; while( k<N-1 && A>_x[k+1] )k++; A-=_x[k];  // первый из неоднозначных
 Real h=_x[k+1]-_x[k]; if( fabs( h )<eps ){ Y=(y[k+1]+y[k])/2;
                                            Z=(z[k+1]+z[k])/2; return; }
  if( Spl )
  { Real B=1.0-(A/=h),b=( B*B-1.0 )*B,           // сплайн без сломанной кривой
         *_z=_y+(N+1),a=( A*A-1.0 )*A;
    Y = B*y[k]+A*y[k+1] + ( b*_y[k]+a*_y[k+1] )*h*h/6;
    Z = B*z[k]+A*z[k+1] + ( b*_z[k]+a*_z[k+1] )*h*h/6;
  } else
  { A/=h; Y = y[k] + A*( y[k+1]-y[k] );        // простая линейная интерполяция
          Z = z[k] + A*( z[k+1]-z[k] );
  }
}
Real Frame::G( _Real az, bool bound )           // эмуляция плазовой ординаты
{ if( N<1 )return y[0];                          // одна точка на другие случаи
  if( az<=min )return bound?y[0]:0.0;            // точка лежит ниже шпангоута
  if( az>=max )return bound?y[N]:0.0;            // точка лежит выше шпангоута
#if 0
 int j=0;               // где y - неоднозначная функция, ищем первое вхождение
  for( j=0; j<N && z[j]<az; j++ ); if( j>1 )j--;
    return y[j]+(az-z[j])*(y[j+1]-y[j])/(z[j+1]-z[j]); // немного по простецки
#else
  for( int k=0; ; k++ )                          // где Y - однозначная функция
  { if( k>=N )return bound?y[N]:0.0;             // теперь точка выше шпангоута
    if( az<=z[k+1] && z[k+1]>z[k] || k==N-1)     //       линейная интерполяция
    { Real X,Y=0,Z=z[k+1]-z[k];
      X=( _x[k]+(az-z[k])*(_x[k+1]-_x[k])/Z )/_x[N]; AYZ( X,Y,Z ); return Y;
  } }
#endif
}
//         Блок операторов для корпуса в целом c простой линейной интерполяцией
//                                          ординат в шпациях между шпангоутами
Real Hull::Y( _Real x,_Real z )           // за пределами обнуление
{ Real A,S,a=Asx.G( z,true ),s=Stx.G( z,true ); int k;  // граничного шпангоута
  if( x<a || x>s )return 0.0;             // особый анализ точек в оконечностях
  if( x<F[0].X ){ A=Asy.G( z ),s=F[0].X,S=F[0].G(z); } else  // за ахтерштевнем
  if( x>F[Ns-1].X ){ S=Sty.G(z),a=F[Ns-1].X,A=F[Ns-1].G(z); }else // форштевнем
  { for( k=0; k<Ns-2 && x>F[k+1].X; k++ ); // поиск ближайшего левого индекса
    if( a>F[k].X )A=Asy.G( z ); else a=F[k].X,A=F[k].G( z );
    if( s<F[k+1].X )S=Sty.G( z ); else s=F[k+1].X,S=F[k+1].G( z );
    if( a<=F[k].X && s>=F[k+1].X )
    { Real Z=F[k].max; if( z>Z+(x-a)*(F[k+1].max-Z)/(s-a) )return 0.0;
           Z=F[k].min; if( z<Z+(x-a)*(F[k+1].min-Z)/(s-a) )return 0.0;
  } } return max( A+(x-a)*(S-A)/(s-a),0.0 );
}
//    Водоизмещение и площадь смоченной поверхности корпуса
//
void Hull::Init()
{ const int mZ=196,mX=720;     // простой расчет полного объема корпуса
 Real x,Dx,dx=Lmx/mX,          //      (по внутренним трапециям)
      z,Dz,dz=(Draught-Do)/mZ, // шаг по аппликате с учетом заглубления бульба
      y,   ds=dx*dz,           // ордината с элементарной площадкой обшивки
      Vs,Vm;                   // площадь шпангоута
      Vm=Volume=Surface=0;     // водоизмещение и смоченная поверхность корпуса
 int i,j;
//pragma omp parallel for reduction(+: Volume,Surface )
  for( j=0,z=Do;        j<=mZ; j++,z+=dz )    // Водоизмещение
  { for( i=0,x=Xo,Vs=0; i<=mX; i++,x+=dx )
    { Vs += y=Y( x,z );                       // Объем корпуса...метод трапеций
      if( !j )Surface += y*dx*2; else         // площадь плоского днища
      if( !i || i==mX )Surface += y*dz*2;     // площадь транцев в оконечностях
      Dx = Y( x+dx,z );
      Dz = Y( x,z+dz );                       // Смоченная поверхность корпуса
      if( y>0 || Dz>=0 || Dz>0 )
          Surface+=y=sqrt( 1+norm( ( y-Dx )/dx,( y-Dz )/dz ) )*ds*2;
      if( !j || j==mZ )Surface-=y/2;          // ... снова к методу трапеций
    } Volume += Vs*ds*2;
    if( !j || j==mZ )Volume -= Vs*ds;
  }
  for( i=0; i<Ns; i++ )           // ??? поиск наибольшего по площади шпангоута
  { x=F[i].X;
    if( i==Ns-1 )dx=x; else dx=F[i+1].X;    // правая отсечка для ширины шпации
    if( !i     )dx-=x; else dx-=F[i-1].X;   // левая отсечка под метод трапеций
         z=Do; ds=dx*dz;                    // ds здесь удвоено в средней части
    for( j=0,Vs=0; j<=mZ; z+=dz,j++ )       // Водоизмещение
    { Vs += y=Y( x,z )*ds;                  // Объем корпуса
      if( !j || j==mZ )Vs -= y/2;           //   ... под метод трапеций
    }
    if( Vs>Vm ){ Vm=Vs; if( Ms<0 )Ms=-1-i;} // небольшая подработка для
  }                     if( Ms<0 )Ms=-1-Ms; // нового местоположения миделя
  Nstem=Ns-1; Dx=-1;                       // номер нулевого шпангоута к началу
  for( i=Nstem; i>=Ns/2; i-- )            // поиска ставится на первый шпангоут
  for( j=0; j<=F[i].N; j++ )
  { x=norm( F[i](j)-Draught,F[i][j] );
    if( Dx<0 )Dx=x; else if( x<Dx ){ Nstem=i; Dx=x; }
  }
}
