//
//       омплекс графических программ класса Depth                   /90.08.20/
//
#include <Math.h>
#include "Tv_Graph.h"
#include "Tf_Topo.h"
#define Nlist 16                          //
static  Real     Lf[Nlist];               // ћассив линий уровн€
static  unsigned Lc[Nlist];               //   и их цветов
static  int Nl=0,                         // “екущее количество изобат дл€ TOPO
        Cp=LIGHTBLUE,C0=YELLOW,Cm=GREEN;  //   и их расцветка
                                          //
static int Tf_Color( Real fl ){ return fl>0.0 ? Cp : fl<0.0 ? Cm:C0; }

void Tf_Color( const int Cplus, const int Cnull, const int Cminus )
{    Cp=Cplus; C0=Cnull; Cm=Cminus;
}
//      ƒобавлени€ линии уровн€ в список Topo                        /91.01.08/
//
int Tf_List( Real fl, const int c )
{ if( Nl<Nlist ){ unsigned *C=Lc,cl=c; Real *F=Lf;
    if( cl<0 )cl=Tf_Color( fl );
    if( Nl>0 )
    { Real W=F[0];
      for( int k=0;; ){ if(fl==W ){ C[0]=cl; return Nl; } F++,C++;
                        if( ++k==Nl )break; W+=F[0];
                      } fl-=W;
    } F[0]=fl,C[0]=cl; ++Nl;
  } return Nl;
}
int Tf_List( int NL ){ if( NL<Nl )Nl=NL; return Nl; } //уменьшение длины списка

#define LIST( Y,X ) P.x=x X, P.y=y Y, Lst[k*4+Nb[k]++]=P;
//
//    онтурные карты высот и глубин, с возможностью
//   автоматической расстановки кратного количества изолиний (Cmd=0)
//
template <class real>
void Tf_Topo( real **_D, int Jy,int Jx, int Ly,int Lx, bool Cmd )
{ if( !_D )return;                         //  онтурна€ карта по массиву данных
  if( !Cmd )                               // јвтоматическое построение списка
  { Real fl; real *_1,D,Max=0,Min=0; int x,y; Nl=0;
    for( y=0; y<Ly; y++ )
    for( _1=_D[y+Jy]+Jx,x=0; x<Lx; x++ )
       { D=*_1++; if( Max<D )Max=D; else if( Min>D )Min=D; }
    if( (fl=Max-Min)>0.001 )
    { y=int( Max/( D=Tv_step( fl ) ) );
      for( fl=y*D; fl>=Min; fl-=D )Tf_List( fl );
  } }
  if( Nl>0 )
  { int lx=Tv_port.right-Tv_port.left,
        ly=Tv_port.bottom-Tv_port.top,cl=getcolor(),c; c=cl;
    if( lx<=Lx && ly<=Ly )
    { int Y1 = Jy+Ly-1;     // ≈сли размерность матрицы больше размерности окна
      for( int y=0; y<ly; y++ )
      { int X1=Jx,Y2=Jy+Ly-minmax( 0,(y*Ly)/ly,Ly-1 )-1;
        for( int x=0; x<lx; x++ )
        { int X2=Jx+minmax( 0,(x*Lx)/lx,Lx-1 );
          real F[4] = { _D[Y1][X1],_D[Y1][X2],_D[Y2][X1],_D[Y2][X2] };
          for( int k=0; k<Nl; k++ )
          { for( int b=0; b<4; b++ )F[b]-=Lf[k];
            if( F[0]*F[3]<=0.0 || F[1]*F[2]<=0.0 )
              { putpixel( x+Tv_port.left,y+Tv_port.top,Lc[k] ); break; }
          } X1=X2;
        }   Y1=Y2;
      }                                 //
    } else                              // иначе настраиваетс€ прорисовка линий
    { point Lst[Nlist*4];               // внутри контрольного пр€моугольника
      Field Tf={Jx,Jy,Lx-1,Ly-1}; Tv_place( 0,&Tf );
      for( int y=Jy; y<Jy+Ly-1; y++ )
      { real *_1=_D[y]+Jx;   real fr=*_1++;
        real *_2=_D[y+1]+Jx; real FR=*_2++;
        for( int x=Jx; x<Jx+Lx-1; x++ )
        { int b,k,Nb[Nlist]={ 0 };
          for( b=0; b<4;  b++ )
          { Real f,F,Fr;
            if(  b<1  ){ y++;       f=fr; Fr=F=FR; } else{ f=Fr;
              if( b<2 ){ x++;        Fr=F = *_2++; } else
              if( b<3 ){ --y; FR=Fr, Fr=F = *_1++; } else
                       { --x; F=fr,fr=Fr; }
            }
            for( k=0; k<Nl; k++ ){ Point P; Real Dr=Lf[k]; F-=Dr; f-=Dr;
              if( F==0 )LIST( , ) else
              if( F*f<0 )
              { Dr=F/( b>1 ? f-F:F-f ); if( b&1 )LIST( ,-Dr ) else LIST( -Dr, )
            } }
          }
          for( k=0; k<Nl; k++ )if( (b=Nb[k])>1 )
          { point *L=Lst+k*4;
            if( c!=Lc[k] )setcolor( c=Lc[k] );
            if( b<4  )L[b]=L[3-b];
                      if( L[0]!=L[2] )line( L[0],L[2] );
            if( b==3 )if( L[2]!=L[1] )line( L[2],L[1] );
            if( b>2  )if( L[1]!=L[3] )line( L[1],L[3] );
        } }
        if( !(y%(Ly/12)) )Tv_isKey();   // 12 раз за врем€ полной прорисовки
      } Tv_place( 0,&Tf );              // Windows исполн€ет внутренние запросы
    }   color( cl );
} }
template void Tf_Topo( float**,int,int,int,int,bool );   // Instantiates
template void Tf_Topo( double**,int,int,int,int,bool );
//
//   Tf_line    »зображение одной изолинии без включени€ в список
//
template <class real>
void Tf_Line( real **_D,int Jy,int Jx,int Ly,int Lx,Real fl,int clr )
{ int NL=Nl,C=Lc[0];
  Real F=Lf[0]; Nl=1;  Lf[0]=fl; Lc[0]=clr; Tf_Topo( _D,Jy,Jx,Ly,Lx,true );
                Nl=NL; Lf[0]=F;  Lc[0]=C;
}
template void Tf_Line( float**,int,int,int,int,Real,int );   // Instantiates
template void Tf_Line( double**,int,int,int,int,Real,int );
