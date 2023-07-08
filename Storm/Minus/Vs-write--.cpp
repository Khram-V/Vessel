//
//      Некоторый блок процедур для записи корпуса
//      в стандартных трёхмерных графических форматах
//                                                         2019-08-22 Иерусалим
//
#include "Vessel.h"       // объекты и производные операции с корпусом на волне

#define printV( x,y,z )fprintf( F,"v %lg %lg %lg\n",x,z,y ), \
                       fprintf( F,"v %lg %lg %lg\n",x,z,y>0?-y:0.0 );
static FILE *F=NULL;
static int fColor=-1;
static void crossPoint( _Point A,_Point B,_Point C, int a,int b,int c )
{ if( fabs( A.Y )<eps && fabs( B.Y )<eps && fabs( C.Y )<eps )return;
  int l=A.Z+B.Z+C.Z<0.0;
  if( fColor!=l )fprintf( F,"usemtl %s\n",(fColor=l)?"green":"gray" );
                 fprintf( F,"f %d %d %d\n",a,b,c );
}
Hull& Hull::Write()  // запись в Wavefront Technologies for Advanced Visualizer
{ F=_wfopen( U2W( fext( FileName,"obj" ) ),L"wb" ); fColor=-1;
  int i,k,n,L[Nframes+2]; memset( L,0,sizeof(int)*(Nframes+2) );
  //
  //       последовательная запись всего массива опорных точек в текстовый файл
  //
  fprintf( F,"# < %s >\nmtllib Vessel.mtl\n# Stern[%d]\n",
                      ShipName,n=Stern.length );                // заголовок
  for( i=0; i<n; i++ )printV(Stern[i].x,Stern[i].y,Stern[i].z); // ахтерштевень
  for( k=1; k<=Nframes; k++ )                                   // и следом все
  { L[k]=L[k-1]+n*2;                                            // шпангоуты
    fprintf( F,"# Sp%d[%d]=%d\n",k,n=Frame[k].length,L[k] );
    for( i=0; i<n; i++ )printV( Frame[k][i].x,Frame[k][i].y,Frame[k][i].z );
  }
  L[k]=L[k-1]+n*2; fprintf( F,"# Stem[%d]=%d \n",n=Stem.length,L[k] );
  for( i=0; i<n; i++ )printV( Stem[i].x,Stem[i].y,Stem[i].z );
  fprintf( F,"# ShipHull[%d]\n\n",L[k]+n*2 );
  //
  //   формирование индексов треугольников с единым правилом обхода площадок
  //
  for( k=0; k<=Nframes+2; k++ )                        // всего Nframes + 3
  if( Shell[k] )if( Shell[k][0]>0 )                    // -- если есть корпус
  { int nP,nQ,np,nq,iP,iQ,ip,iq;                       // -- то будет и шпация
    fprintf( F,!k?"# Stern\n":(k<Nframes+2?"# Sp%d\n":"# Stem\n"),k );
    if( !k || k==Nframes+2 )                           // -- но сначала штевни
    for( i=1; i<=Shell[k][0]; i++ )                    // транцевые расширения
    { Point P=InSpan( nP=k,iP=i+!k,false ),Q=InSpan( nQ=k,iQ=i+1-!k,false ),
            p=InSpan( np=k,ip=-(i+!k),false ),q=InSpan( nq=k,iq=-(i+1-!k),false );
      if( abs( P-p )>=eps )crossPoint( p,P,Q,L[np]+ip,L[nP]+iP,L[nQ]+iQ );
      if( abs( Q-q )>=eps )crossPoint( Q,q,p,L[nQ]+iQ,L[nq]+iq,L[np]+ip );
    } else
    for( int Board=-1; Board<2; Board+=2 )   // сначала левый затем правый борт
    { Point Q=InSpan( nQ=k,iQ=Board,false ),       // это блок стандартных шпаций
            q=InSpan( nq=k,iq=Board*2,false );     // - переход на правый борт
      for( i=3; i<=Shell[k][0]; i++ )        // всех теоретических шпангоутов
      { Point P=InSpan( nP=k,iP=Board*i,false );   // попутная разборка треугольников
        if( Board<0 )crossPoint( Q,q,P,L[nQ]+iQ,L[nq]+iq,L[nP]+iP );
                else crossPoint( q,Q,P,L[nq]+iq,L[nQ]+iQ,L[nP]+iP );
        if( Shell[k][i]&(LeftFrame|SternPost) )nQ=nP,iQ=iP,q=P; // перед кормовым
                                          else nq=nP,iq=iP,Q=P; // перпендикуляром
    } }
  } fprintf( F, "#\n# %s\n#",sname( FileName ) ); fclose( F );
    return *this;
}
