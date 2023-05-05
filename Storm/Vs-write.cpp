//
//      Некоторый блок процедур для записи корпуса
//      в стандартных трёхмерных графических форматах
//                                                         2019-08-22 Иерусалим
//
#include "Vessel.h"       // объекты и производные операции с корпусом на волне
static FILE *F=NULL;
static int fColor=-1;
static void printF( _Vector V )
{          fprintf( F,"v %lg %lg %lg\n",V.x,V.z,V.y );
           fprintf( F,"v %lg %lg %lg\n",V.x,V.z,V.y>0?-V.y:0.0 );
}
static void crossPoint( _Point A,_Point B,_Point C, int a,int b,int c )
{ if( fabs( A.Y )<eps && fabs( B.Y )<eps && fabs( C.Y )<eps )return;
  int l=A.Z+B.Z+C.Z<0.0;
  if( fColor!=l )fprintf( F,"usemtl %s\n",(fColor=l)?"green":"gray" );
                 fprintf( F,"f %d %d %d\n",a,b,c );
}
Hull& Hull::Write()  // запись в Wavefront Technologies for Advanced Visualizer
{ if( !(F=_wfopen( U2W( fext( FileName,"obj" ) ),L"wb" )) )return *this;
  int i,k,n,L[Nframes+3]; memset( L,0,sizeof(int)*(Nframes+3) ); fColor=-1;
  //
  //       последовательная запись всего массива опорных точек в текстовый файл
  //
  fprintf( F,"# < %s >\nmtllib Vessel.mtl\n# Stern[%d]\n",
                      ShipName,(n=Stern.length)*2 );            // заголовок
  for( i=0; i<n; i++ )printF( Stern[i] );                       // ахтерштевень
  for( k=0; k<Nframes; k++ )                                    // и следом все
  { L[k+1]=L[k]+n*2;                                            // шпангоуты
    fprintf( F,"# Sp%d[%d]=%d\n",k+1,(n=Frame[k].length)*2,L[k+1] );
    for( i=0; i<n; i++ )printF( Frame[k][i] );
  }
  L[k+1]=L[k]+n*2; fprintf( F,"# Stem[%d]=%d \n",(n=Stem.length)*2,L[k+1] );
  for( i=0; i<n; i++ )printF( Stem[i] );
  fprintf( F,"# ShipHull[%d]\n",L[k+1]+n*2 );
  //
  //   формирование индексов треугольников с единым правилом обхода площадок
  //
  for( k=0; k<=Nframes+2; k++ )                        // всего Nframes + 3
  if( Shell[k] )if( Shell[k][0]>0 )                    // -- если есть корпус
  { int nP,nQ,np,nq,iP,iQ,ip,iq; Point P,Q,p,q;        // -- то будет и шпация
    fprintf( F,!k?"# Stern\n":(k<Nframes+2?"# Sp%d\n":"# Stem\n"),k );
    if( !k || k==Nframes+2 )                           // -- но сначала штевни
    for( i=2; i<Shell[k][0]-1; i++ )                    // транцевые расширения
    { P=InSpan( nP=k,iP=i+!k,false );   p=InSpan( np=k,ip=-(i+!k),false );
      Q=InSpan( nQ=k,iQ=i+1-!k,false ); q=InSpan( nq=k,iq=-(i+1-!k),false );
      if( abs( P-p )>=eps )crossPoint( p,P,Q,L[np]+ip,L[nP]+iP,L[nQ]+iQ );
      if( abs( Q-q )>=eps )crossPoint( Q,q,p,L[nQ]+iQ,L[nq]+iq,L[np]+ip );
    } else
    for( int Board=-1; Board<2; Board+=2 )   // сначала левый затем правый борт
    { Q=InSpan( nQ=k,iQ=Board,false ),       // это блок стандартных шпаций
      q=InSpan( nq=k,iq=Board*2,false );     // - переход на правый борт
      for( i=3; i<=Shell[k][0]; i++ )        // всех теоретических шпангоутов
      { P=InSpan( nP=k,iP=Board*i,false );   // попутная разборка треугольников
        if( Board<0 )crossPoint( Q,q,P,L[nQ]+iQ,L[nq]+iq,L[nP]+iP );
                else crossPoint( q,Q,P,L[nq]+iq,L[nQ]+iQ,L[nP]+iP );
        if( Shell[k][i]&(LeftFrame|SternPost) )nQ=nP,iQ=iP,q=P; // перед кормовым
                                          else nq=nP,iq=iP,Q=P; // перпендикуляром
    } }
  } fprintf( F, "#\n# %s\n#",sname( FileName ) ); fclose( F ); return *this;
}
