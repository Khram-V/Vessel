
#include "Ship.h"

void Surface::R90Zright() // поворот вправо по курсу
{ for( int i=0; i<NoCoPoint; i++ )
  { Vector W=P[i].V; P[i].V.x=W.y; P[i].V.y=-W.x; }
}
void Surface::R90Xright() // поворот вправо по курсу
{ for( int i=0; i<NoCoPoint; i++ )
  { Vector W=P[i].V; P[i].V.z=W.y; P[i].V.y=-W.z; }
}
void Surface::Revolute() // обращение последовательности обхода рёбер по граням
{ int left,right,n,p,*P;
  for( int i=0; i<NoFaces; i++ )
  { P=F[i].P; n=F[i].Capacity; left=0,right=n;
    while( left<right )p=P[left],P[left++]=P[--right],P[right]=p;
  }
}
void Surface::EditMenu( Window *Win ) // сдвиг и масштаб
{ Vector Afins={1,1,1},Shift={0,0,0};
  Mlist Menu[]={ { 1,0," Смещение и масштабирование корпуса" }             // 0
  , { 2,8," Сдвиг  x:%6.6lf",&Shift.x},{0,8,", y:%6.6lf",&Shift.y},{0,8,", z:%6.6lf",&Shift.z}
  , { 1,8,"Масштаб x:%6.6lf",&Afins.x},{0,8,", y:%6.6lf",&Afins.y},{0,8,", z:%6.6lf",&Afins.z}
  };
  TextMenu T( Mlist(Menu),Win,1,1 );
  if( T.Answer()!=_Esc )
      for( int i=0; i<NoCoPoint; i++ )P[i].V+=Shift,P[i].V&=Afins;
}
//
//   прорисовка исходных многоугольников
//
#define uWater { c.c[3]-=c.c[3]/6; c.c[0]=( c.c[0]+UnderWaterColor.c[0] )/2; \
                                   c.c[1]=( c.c[1]+UnderWaterColor.c[1] )/2; \
                                   c.c[2]=( c.c[2]+UnderWaterColor.c[2] )/2; }

static void Draw( Flex &Cont, int i1,int i2, _Real delta, bool right ) // i1-i2 включительно
{ Vector V,W=Zero;
  for( int i=i1+1; i<i2; i++ )W+=(Cont[i+1]-Cont[i1])*(Cont[i]-Cont[i1]);
  glNormal3dv( W );          // в гидромеханике этот расчёт долен быть здесь
  glBegin( GL_POLYGON );
    for( int i=i1; i<=i2; i++ ){ (V=Cont[i]).z+=delta; dot( V ); }
  glEnd();
  if( right )
  { W.y=-W.y; glNormal3dv( W );
    glBegin( GL_POLYGON );
    for( int i=i2; i>=i1; i-- ){ (V=Cont[i]).z+=delta; V.y=-V.y; dot( V ); }
    glEnd();
  }
}
inline bool inInter( _Vector V1, _Vector V2 )
{    return (V1.z>=0 && V2.z<0) || (V2.z>=0 && V1.z<0);
}
inline Vector newInter( _Vector V1, _Vector V2 )
{    return V1 - V1.z*( V2-V1 )/( V2.z-V1.z );
}
void Surface::Drawing( BoardView Sides )
{ static Flex W,wL; const Real delta=Draft+Min.z; int K; Color c; wL.len=0;
//#pragma omp parallel for shared( wL ) //private( K ) // reduction(+: wL.len )
  for( int N=0; N<NoFaces; N++ ) // синхронная прорисовка треугольников двух бортов
  if( (K=F[N].Capacity)>2 )             // у граней должно быть боле двух рёбер
  { const Layers &Layer=L[min(NoLayers,F[N].LayerIndex)]; // указанные свойства
    const bool right=(Sides==mvBoth && Layer.Symmetric);
    glLineWidth( 1 );    /// Alice AI из Яндекса стала эдесь хорошим помощником
#if 1
   Vector v,V1,V2; int J=-1,i=0; W.len=0;
    while( i<=K+J )
    { (V2=P[F[N].P[i%K]].V).z-=delta;
      if( i++ )                        // i - показывает следующий узел = длину
      if( inInter( V1,V2 ) )           //     однократно, но по всем рёбрам
      { if( J<0 || W.len==0 ){ v=V1; if( J<0 )J=i,W.len=0; } else v=V2;
        if( v.z==0.0 )W+=v; else W+=newInter( V1,V2 );
        if( W.len>1 )
        { if( W.len>2 )  //--- невидимые двойки пусть нарисуются, аль нет {wL}?
          { c.C=Layer.LClr.C;
            if( W[1].z<0 )uWater else wL+=W[0],wL+=W[-1]; glColor4ubv( c.c );
            Draw( W,0,W.len-1,delta,right );
          } W[0]=W[-1]; W.len=1;
      } }
      if( J<0 || W.len>0 )W+=V2; V1=V2; // J<0 по началу, и от пересечения нуля
    }
    if( J<0 ) // if( W.len>0 )        // c заданной расцветкой для каждой грани
    { c.C=Layer.LClr.C;
      if( W[0].z<0 )uWater glColor4ubv( c.c ); Draw( W,0,W.len-1,delta,right );
    }
#else
   Flex V; int i=0,j=0,J=0; W.len=0;
    for( int I=0; I<K; I++ )(V+=P[F[N].P[I]].V).z-=delta;
    while( i<=K+J ){ int I=i%K,I1=(i+K-1)%K; i++;
      if( inInter(V[I1],V[I]) )
      { if( W.len )j=I; else { j=I1; if( !J )J=I+1; }
        if( V[j].z==0.0 )W+=V[j]; else W+=newInter( V[I1],V[I] );
        if( j==I )
        { c.C=Layer.LClr.C;
          if( W[1].z<0 )uWater else wL+=W[0],wL+=W[-1]; glColor4ubv( c.c );
          Draw( W,0,W.len-1,delta,right ); W.len=0; --i; // одной левой точкой
      } } if( W.len )W+=V[I];
    }
    if( !W.len ){ c.C=Layer.LClr.C; if( V[0].z<0 )uWater; glColor4ubv( c.c );
                  Draw( V,0,V.len-1,delta,right );
                }
#endif
  }
  for( int i=0; i<wL.len; i++ )wL[i].z+=delta; color( white ); glLineWidth(2);
  glDisable( GL_LIGHTING );                       // glNormal3d( 0,0,-1 );
  for( int i=0; i<wL.len; i+=2 )                  // белая ватерлиния 1|2 борта
// if( Sides!=mvBoth )line( wL[i],wL[i+1] ); else
                     liney( wL[i],wL[i+1] );
  color( lightmagenta );                          // кривые контрольные контуры
  for( int K=0; K<NoCurves; K++ )
  { Vector V,W;
    for( int I=0; I<C[K].Capacity; I++ ){ W=P[C[K].P[I]].V;
      if( I ){ if( Sides==mvBoth )liney( V,W ) ; else line( V,W ); } V=W;
    }
  } glEnable( GL_LIGHTING ); glLineWidth( 0.2 );
}
void Surface::Extents( bool Sizes )                // Экстремумы по всем контрольным точкам
{ for( int i=0; i<NoCoPoint; i++ )
  if( !i )Min=Max=P[0].V; else { Vector &V=P[i].V;
    if( Min.x>V.x )Min.x=V.x; else if( Max.x<V.x )Max.x=V.x;
    if( Min.y>V.y )Min.y=V.y; else if( Max.y<V.y )Max.y=V.y;
    if( Min.z>V.z )Min.z=V.z; else if( Max.z<V.z )Max.z=V.z;
  }
  if( !Sizes || !NoLayers ){ Length=Max.x-Min.x,
                             Beam=Max.y-Min.y,
                             Draft=-Min.z; }
}

//   Расчистка повторяющихся узлов по граням (пока без перестроения рёбер)
//
void Surface::ReOrder()                                                         // omp_get_thread_num omp_get_num_procs()
{ int i,I,J,M,K,*L=(int*)Allocate( NoCoPoint*sizeof( int ) );                   print( "\nСчёт<~%d>: %d",omp_get_max_threads(),NoCoPoint );
#if 1
   for( I=0; I<NoEdges; I++ )L[G[I].EndIndex]++,
                             L[G[I].StartIndex]++;                              //? в файлах *.fef пока нет контуров!
   for( I=0; I<NoFaces; I++ )
   for( K=0; K<F[I].Capacity; K++ )L[F[I].P[K]]++;
   for( I=M=0; I<NoCoPoint; I++ )if( L[I]>0 )M++; else L[I]=-1;                 if( M!=NoCoPoint )print( "<~%d>",NoCoPoint-M );
   if( L[0]>0 )L[0]=0;
#endif
                                                                                print( " узлов" );
#if 1
#pragma omp parallel for shared( L ) // private( i,k ) reduction(+: J )
   for( int i=0; i<NoCoPoint; i++ )
   if( L[i]>=0 )
   { Vector &V=P[i].V;  // так должно быть
     for( int k=0; k<i; k++ )
     if( L[k]>=0 )
     { if( abs( P[k].V-V )>Eps ){ L[i]=i; WinReady(); } else // P[k].T=svCrease; ???
       { P[k].T=(P[k].T==svCorner || V.y==0.0?svCorner:svRegular); L[i]=k; break;
   } } }
   for( I=J=0; I<NoCoPoint; I++ )
   { if( L[I]>=0 )                      // пропуск по отсутствию обращений -???
     { if( L[I]==I  ){ if( I>J )P[L[I]=J]=P[I]; J++; } else L[I]=L[L[I]]; }
   }                                                                            if( NoCoPoint!=J )print( " %d сброс",J-NoCoPoint );
   NoCoPoint=J;
#else
   for( I=J=K=0; I<NoCoPoint; I++ ){ CoPoint &V=P[I]; // контроль одним потоком
     for( i=0; i<J; i++ )
      if( abs(P[i].V-V.V)<=Eps ){ L[I]=i; K++; P[i].T=svRegular; break; }
     if( i==J ){ P[i]=V; L[I]=i; J++; }
   } NoCoPoint=J;                                                               print( " %d повторов",K );
#endif
/* for( I=J=K=0; I<NoCoPoint; I++ )
   { if( L[I]==I ){ P[K]=P[I]; L[I]=K; K++; } else { J++; L[I]-=J; }
   } NoCoPoint=K;                                                               print( " c перерасчетом до %d без %d ",NoCoPoint,J );
*/                                                                              print( "; %d рёбер",NoEdges );
   for( I=0; I<NoEdges; I++ )G[I].StartIndex=L[G[I].StartIndex],
                             G[I].EndIndex=L[G[I].EndIndex];
#if 1
   for( I=J=0; I<NoEdges; I++ ){ Edges &E=G[I];  // здесь пусть пока без openMP
     if( E.StartIndex==E.EndIndex
      || abs( P[E.StartIndex].V-P[E.EndIndex].V )<Eps )i=-1; else
     for( i=0; i<J; i++ )
      if( (E.StartIndex==G[i].StartIndex && E.EndIndex==G[i].EndIndex)
       || (E.EndIndex==G[i].StartIndex && E.StartIndex==G[i].EndIndex) // { G[i].Crease=false; K++; break; }
       ){ G[i].Crease&=E.Crease || P[E.StartIndex].V.y==0              // сброс, если не ДП
                                && P[E.EndIndex].V.y==0; break; }      // по выходу из цикла поиска повторений
     if( i==J )G[J++]=E;
   }                                                                            if( J!=NoEdges )print( " %d повтор",J-NoEdges );
   NoEdges=J;
#endif
                                                                                print( "; %d граней",NoFaces );
   for( I=0; I<NoFaces; I++ )
   for( K=0; K<F[I].Capacity; K++ )F[I].P[K]=L[F[I].P[K]];
#if 1
#pragma omp parallel for private( i,I,J,K )
   for( I=0; I<NoFaces; I++ ){ Faces &G=F[I];        // это не очень, исправить
     for( K=J=0; K<G.Capacity; K++ )
     { for( i=0; i<J; i++ )if( G.P[i]==G.P[K] )break;
       if( i==J )G.P[J++]=G.P[K];
     }                                                                          //if( J!=G.Capacity )print( "%d",G.Capacity-J );
     G.Capacity=J;
   }
   for( I=M=0; I<NoFaces; I++ )
    if( F[I].Capacity<3 )Allocate( 0,F[I].P ); else F[M++]=F[I];                if( M!=NoFaces )print( " %d утеря",M-NoFaces );
   NoFaces=M;
#endif
   for( I=0; I<NoCurves; I++ )
   for( K=0; K<C[I].Capacity; K++ )C[I].P[K]=L[C[I].P[K]];                      if( NoCurves>0 )print( "; %d контуров",NoCurves ); print( ".┐" );
   Allocate( 0,L );
}
