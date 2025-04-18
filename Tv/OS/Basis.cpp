//
//      BASIS.cpp                               Ver 1.0  90.11.20
//
//      Процедуры построения альтернативного геометрического Базиса
//             и базиса по четырехугольной деформированной площадке
//
//          ¦ xX  yX ¦         ¦ Xx  Yx ¦       Y   y         Def
//      M = ¦        ¦     m = ¦        ¦       A   A         --¬
//          ¦ xY  yY ¦         ¦ Xy  Yy ¦       ¦   ¦  A ¬a    -¦
//                                              ¦   ¦   /       ї
//           -1   conjg( M )   ¦ yY -yX ¦       ¦   ¦  /
//      m = M   = ---------- = ¦        ¦ / ¦M¦ ¦   ¦ /
//                 det( M )    ¦-xY  xX ¦       ¦   ¦ r
//                                              ¦   +------------>x
//      A = Re + a • M                          ¦  Re
//      a = ( A-Re ) / M                        ¦
//                                              L==================>X
#include "Tv_Graph.h"
//
//   Здесь добавляется возможность деформации базиса,    ( 93.11.23 )
//         что нежелательно нагружать на Basis.
//    Area                                                    Lu  Ru
//    ( Aim     - Результирующая прямоугольная площадка      3-----¬4
//      Corners - четыре угловых точки просматриваемых как:   ¦ Cs ¦
//    )         - на выходе площадь под угловыми точками     1L-----2
//                                                            Ld  Rd
//                                                          Aim
Basis_Area::Basis_Area():Basis(),B( (Field){ 0,0,1,1 } ),ky( 0.0 ),kx( 0.0 )
{   S.Axis( (Point){ 1.0,1.0 },-y,-x );
}
Real Basis_Area::Area( Field Aim, Point Ld, Point Rd, Point Lu, Point Ru )
{  Real A=Basis::Axis( Ld,Lu-Ld,Rd-Ld ); B=Aim;
   Point D=Basis::Goint( Ru ); kx=D.x-1;
                               ky=D.y-1;
  return ( A+S.Axis( Ru,Rd-Ru,Lu-Ru ) )/2.0;
}
//    Area
//    ( Aim     - Результирующая прямоугольная площадка     LuЇ
//      Corners - Тройка угловых точек просматриваемых как:   ¦Crs[3]
//    )         - на выходе площадь под угловыми точками    Ld+---¬Rd
//                                                            Aim
Real Basis_Area::Area( Field Aim, Point Ld, Point Rd, Point Lu )
{ return Area( Aim,Ld,Rd,Lu,Rd+Lu-Ld );
}
Real Basis_Area::Area( Field Aim, Point Ld, Point Rd ) // Масштаб с пересчетом
{ Real s=Aim.Ly/Aim.Lx;                                 // на пикселы экрана !!
  point ld=(Point)Ld,
        rd=(Point)Rd,lu={ int(ld.x+s*(rd.y-ld.y)),int(ld.y-s*(rd.x-ld.x)) };
  return Area( Aim,Ld,Rd,lu );
}                                              //
Point Basis_Area::Goint( Point P )             // Переход в локальный базис Aim
{ Point A=S.Goint( P ),                        //
        a=Basis::Goint( P ); A.x=B.Jx+B.Lx*( a.x/( A.x+a.x ) );
                             A.y=B.Jy+B.Ly*( a.y/( A.y+a.y ) ); return A;
}                                              //
Point Basis_Area::Goext( Point p )             // Возврат в центральный базис
{ Point A,a; A.x=( p.x-B.Jx )/B.Lx;            //  It's Right !!!
             A.y=( p.y-B.Jy )/B.Ly; a.x=A.x*( 1+kx*A.y );
                                    a.y=A.y*( 1+ky*A.x );
  return Basis::Goext( a );
}
//   Процедура построения альтернативного или географического базиса
//   ( Эйлеровы координаты )                                  Ver.1.0 90.11.20
//   E U L E R - Определение локального базиса внутри глобального
//           - Углы отсчитываются от меридиана (вертикальной оси Y) в градусах
//     Re    - Местоположение источника
//     angle - ориентация меридиана в градусах (от Y вправо по часовой стрелке)
//     size  - размеры полуосей определяющего эллипса
//
Real Basis::Euler( _Point Re, Real angle, _Point size )
{ Real x=sin( angle*=M_PI/180.0 ),y=cos( angle );
  return Axis( Re,size.y*(Point){ x,y },size.x*(Point){ y,-x } );
}
//
//  Поиск точки пересечения двух прямых линий, даже если таковые не сходятся
//
int Cross( Point L11, Point L12, Point L21, Point L22, Point& Cx )
{ Base Det;
   if( fabs(Det.Axis( L22-L21,L12-L11 ))<1.e-12 )return -2; // -- линии
   L21=L22=Det.Goint( L22-L11 );                           // почти параллельны
   L21.y=0; Cx=Det.Goext( L21 )+L11;
   if( L22.x<0.0 || L22.y<0.0 )return -1;
   if( L22.x>1.0 || L22.y>1.0 )return 1; return 0;
}
//
//  Билинейная интерполяция по узлам сетки с равным шагом
//
#if 0
Real Value( float **F, Real X, Real Y, int Nx, int Ny )
{                                              // Расчет индекса
 int y=Y,x=X;                                  // ближайшей точки
 double dy,dx,yd,xd;                           //
  if( y<-1 || x<-1 || y>Ny || x>Nx )return 0;  // может лучше +или- 10,001 км?
  if( y<0 )y=0; else if( y>=Ny-1 )y=Ny-2;      //
  if( x<0 )x=0; else if( x>=Nx-1 )x=Nx-2;      //
  dy=Y-y,yd=1.0-dy,                            // Расчет разностей аргумента и
  dx=X-x,xd=1.0-dx;  F+=y;                     //  смещение по строкам массива
  return ( xd*( F[0][x] ) + dx*( F[0][x+1] ) )*yd
       + ( xd*( F[1][x] ) + dx*( F[1][x+1] ) )*dy;
}
#endif
//
//   Прямая интерполяция по исходному ряду с помощью Эрмита второй степени
//
Real Sleek( float *Y, Real X, int Nx )        // по целым узлам - по их номерам
{ int x=X;                              Nx--; // расчет индекса ближайшей точки
  if( x<0 )x=0; else if( x>=Nx )x=Nx-1; Y+=x; // с любым удалением от границ
 Real a=X-x,c=Y[1]-Y[0],f1,f0;
  if( x>0     )f0=( Y[1]-Y[-1] )/2;
  if( x<Nx-1  )f1=( Y[2]-Y[0] )/2;
  if( !x      )f0=( c-f1*0.3819660113 )*1.6180339887;     // концы в пропорциях
  if( x==Nx-1 )f1=( c-f0*0.3819660113 )*1.6180339887;     // "золотого сечения"
//!       D = f0 + a*( 2*(c-f0) + (3*a-2)*(f0+f1-2*c) );  // это - производная
  return Y[0] + a*( f0 + a*( c-f0 + (a-1)*(f0-2*c+f1) ) );
}
Real Value( float **F, Real X, Real Y, int Nx, int Ny )
{ int y=Y-1.0; float FY[4];                   // Расчет индекса ближайшей точки
//if( y<-1 || x<-1 || y>Ny || x>Nx )return 0; // может лучше +или- 10,001 км?
  if( y<0 )y=0; else if( y>Ny-4 )y=Ny-4;      //
  for( int k=0; k<4; k++ )FY[k]=Sleek( F[y+k],X,Nx );
  return Sleek( FY,Y-y,4 );
}
//
//      Процедуры стандартной разметки осей                     /91.01.14/
//
#if 0
float Tv_step( float Di )        // Вычисление шага
{ float iPart=floor( Di=log10( Di ) );
        Di = exp( (Di-iPart)*M_LN10 );
 return pow( 10.0,iPart )*( Di>6.0 ?2.0 : Di>3.0 ?1.0 : Di>1.5 ?0.5:0.2 );
}
#endif
//      Разметка графического поля в соответствии с физическими экстремумами
//
void Tv_axis( bool lab )
{
 Field F,f={ 0,0,1,1 };
 Point P;
 point p;
 Bfont S( _Small_font,2*Tv.Th/3 );
  Tv_place( 0,&f ); F=f; Tv_place( 0,&f ); P.x=F.Jx; P.y=F.Jy; p=P;
  Tv_rect();
  p.y+=2; f.Jy=Tv_step( F.Ly ); F.Ly+=F.Jy;
  p.x-=2; f.Jx=Tv_step( F.Lx ); F.Lx+=F.Jx;
//axis-X
       P.y=Tv_Y( p.y );
  for( P.x=f.Jx*floor( F.Jx/f.Jx+1 ); P.x<F.Lx; P.x+=f.Jx )
  { line( P.x,F.Jy,P.x,F.Ly );
    if( lab )S.Text( South,point( P ),_Fmt( "%g",P.x ) );
  }
//axis-Y
       P.x=Tv_X( p.x+3 );
  for( P.y=f.Jy*floor( F.Jy/f.Jy+1 ); P.y<F.Ly; P.y+=f.Jy )
  { line( F.Jx,P.y,F.Lx,P.y );
    if( lab )S.Text( West,point( P ),_Fmt( "%g-",P.y ) );
  }
}
//
//      B_geo.c                                                   /91.05.11/
//      Процедуры градусно-часовых преобразований
//
template <class real> real Angle( real A )
       { A=fmod( A,360.0 ); return A>180.0?A-360.0:( A<-180.0?A+360.0:A );
       }                                   //
template double Angle <double> ( double ); // Контроль направления
template float Angle <float> ( float );    //
//
//    Формирование текстовой строки F=0:{-123°46'57"89} 1-deg,2+min,3+sec,-hnd
//
#define put2hnd *s++=d/10+'0',*s++=d%10+'0';      // -- под сотки из цифирек --
static char *put10( char *s, int d )              // -- составная рекурсия   --
       { int t=d%10; if( d>=10 )s=put10( s,d/10 ); *s++ =t+'0'; return s;
       }
char *DtoA( double D, int F, char c )                                  // с='°'
{ const double rnd[]={ 0.4999999,8.3333333e-3,1.3888888e-4 };
  static char S[16]; char *s=S;
  int d,f,l=0;                                  // -- счетчик заполненных полей
    if( (D=Angle( D ))<0.0 ){ D=-D; *s++='-'; } else *s++=' ';
    if( !F )F=-3; f=F;
    if( F>0 )D+=rnd[F-1]; else{ F=-F; D+=rnd[F-1]/100.0; } d=D; D-=d;
                        if( d||(!l&&F<2 ) ){ s=put10( s,d ); *s++=c; ++l;}
    if( F>1 ){ d=D*=60; if( d||(!l&&F<3 ) ){ put2hnd *s++='\''; ++l; } D-=d; }
    if( F>2 ){ d=D*=60; if( d|| !l        ){ put2hnd *s++='"';  ++l; } D-=d; }
    if( f<0 ){ d=D*100; if( d )put2hnd } *s=0; return S;
}
//
//  Разбор текстовой строки с градусами со свертыванием нового числа
//                                       (и теперь в реентерабельном контексте)
static char* getright( char *s, double& D )
{ double d=1.0; while( *s>='0' && *s<='9' )D+=(*s++ -'0')/(d*=10); return s;
}
static char* getleft( char *s, double& D )
{ while( *s>='0' && *s<='9' ){ D*=10.0; D+= *s++ -'0'; }
  if( *s=='.' || *s==',' )s=getright( ++s,D ); return s;
}
char *AtoD( char *S, double& _D )           // на выходе следующее свободное
{ double D=0.0,m;    char *s,*rs=S;         //  - (никому не нужное) поле
  short n=0,sign=0;  while( *rs<=' ' )++rs; // простая выборка пустых символов
    if( *rs=='-' )sign=1;                   // на случай установки знака числа
    if( *rs=='+' || sign )++rs;             // .. считывание и подвижка символа
  Rept: rs=getleft( s=rs,m=0.0 );           // считываются цифры до разделителя
    switch( *rs++ )                         // с перескоком по градусным меткам
    { case 0xF8: case '°':
      case ':' : if( n<1 ){ n|=1,D+=m;      goto Rept; } break;
      case '\'': if( n<2 ){ n|=2,D+=m/60;   goto Rept; } break;
      case '"' : if( n<4 ){ n|=4,D+=m/3600; goto Rept; } break; // повторное
      default  : if( !n )D+=m; else{ rs=getright( s,m=0 );     // считывание
                                     if( n&4 )D+=m/3600; else // - и ничего не
                                     if( n&2 )D+=m/60; else D+=m; } // попишешь
    } if( sign )D=-D; _D=D; return rs;
}
char *AtoD( char *s,float &f ){ double d; char *w=AtoD( s,d ); f=d; return w; }
//
//      Исправление градусов (°) в часы (h)
//
char* DtoH( double H, int f ){ return DtoA( H,f,0xFC ); }

