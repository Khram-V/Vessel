//
//      Некоторый блок процедур для записи корпуса
//      в стандартных трёхмерных графических форматах
//                                                         2019-08-22 Иерусалим
//
#include "Vessel.h"       // объекты и производные операции с корпусом на волне
static int fColor=-1;
static FILE *F=NULL;

static void printF( Vector V )
{ V=e5( V ); fprintf( F,"v %lg %lg %lg\n",V.x,V.z,V.y );
             fprintf( F,"v %lg %lg %lg\n",V.x,V.z,V.y>0?-V.y:0.0 );
}
static void crossPoint( _Point A,_Point B,_Point C, int a,int b,int c )
{ if( fabs( A.Y )<eps && fabs( B.Y )<eps && fabs( C.Y )<eps )return;
  int l=A.Z+B.Z+C.Z<0.0;
  if( fColor!=l )fprintf( F,"usemtl %s\n",(fColor=l)?"green":"gray" );
                 fprintf( F,"f %d %d %d\n",a,b,c );
}
Hull& Hull::Write() //! запись в Wavefront Technologies for Advanced Visualizer
{ if( !(F=_wfopen( U2W( fext( FileName,"obj" ) ),L"wb" )) )return *this;
  Point P,Q,q; int i,k,n,L[Nframes+2];
  memset( L,0,sizeof(int)*(Nframes+2) ); fColor=-1;
  //
  //       последовательная запись всего массива опорных точек в текстовый файл
  //
  fprintf( F,"# < %s >\nmtllib Vessel.mtl\n# Stern[%d]\n",
                      ShipName,n=Stern.len );                   // заголовок
  for( i=0; i<n; i++ )printF( Stern[i] ); L[0]=n*2;             // ахтерштевень
  for( k=0; k<=Nframes+1; k++ )                                 // и следом все
  { if( k )L[k]=L[k-1]+n*2;                                     // шпангоуты
    fprintf( F,"# Sp%d[%d]=%d\n",k,n=Frame[k].len,L[k] );
    for( i=0; i<n; i++ )printF( Frame[k][i] );
  } L[k]=L[k-1]+n*2;
  fprintf( F,"# Stem[%d]=%d \n",n=Stem.len,L[k] );
  for( i=0; i<n; i++ )printF( Stem[i] );
  fprintf( F,"# ShipHull[%d]\n#~~~~~~~~~~\n",L[k]+n*2 );
  //
  //      формирование индексов треугольников с единым правилом обхода площадок
  //
  for( k=-1; k<2; k+=2 )
  { bool b=k<0; Flex &S=b?Stern:Stem; n=b?0:L[Nframes+2];
    fprintf( F,b?"# Stern\n":"# Stem\n" );
    for( i=0; i<S.len; i++ )
    { Q=*((Point*)(&(S[i]))); //if( !b )Q.Y=-Q.Y;
      if( i>0 )
      { if( P.Y )crossPoint( P,P,Q, n+(i+1)*2 -!b, n+(i+1)*2 + b, b+n+i*2 );
        if( Q.Y )crossPoint( Q,P,Q, n+i*2     -!b, n+(i+1)*2 -!b, b+n+i*2 );
      } P=Q;
  } }
  for( k=0; k<=Nframes; k++ )                           // Nframes+1 шпангоутов
  if( Shell[k] )if( Shell[k][0]>0 )                     // -- если есть корпус
  { int nP,nQ,nq,iP,iQ,iq;                              // -- то будет и шпация
    fprintf( F,"# Sp%d\n",k );
    for( int Board=-1; Board<2; Board+=2 )   // сначала левый затем правый борт
    { Q=InSpan( nQ=k,iQ=Board,false );       // это блок стандартных шпаций
      q=InSpan( nq=k,iq=Board*2,false );     //    - переход на правый борт
      for( i=3; i<=Shell[k][0]; i++ )        // всех теоретических шпангоутов
      { P=InSpan( nP=k,iP=Board*i,false );   // попутная разборка треугольников
        if( Board<0 )crossPoint( Q,q,P,L[nQ]+iQ,L[nq]+iq,L[nP]+iP );
                else crossPoint( q,Q,P,L[nq]+iq,L[nQ]+iQ,L[nP]+iP );
        if( Shell[k][i]&LeftFrame )nQ=nP,iQ=iP,q=P;          // перед кормовым
                              else nq=nP,iq=iP,Q=P;          // перпендикуляром
    } }
  } fprintf( F, "#\n# %s\n#",sname( FileName ) ); fclose( F ); return *this;
}
