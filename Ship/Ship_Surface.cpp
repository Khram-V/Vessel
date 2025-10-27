
#include "Ship.h"
//
//   прорисовка исходных многоугольников
//
void Surface::Drawing( BoardView Sides )
{ static Flex V; Vector W; int I,K,N; Color C; glLineWidth( 1 );
  for( N=0; N<NoFaces; N++ )   // синхронная по бортам прорисовка треугольников
  if( (K=F[N].Capacity)>2 )
  { Layers &Layer=L[F[N].LayerIndex];
//  V.len=0; C.C=L[F[N].LayerIndex].Color;
    V.len=0; C.C=Layer.LClr.C;
    for( I=0; I<K; I++)V+=P[F[N].P[I]].V;  // C.c[3]=V[0].z>=Draught?0xFF:0x7F; glColor4ubv(C.c);
    glNormal3dv( W=(V[2]-V[0])*(V[1]-V[0]) ); glBegin( GL_POLYGON );
    for( I=0; I<K; I++ ){ C.C=Layer.LClr.C;
      if( V[I].z<Draft+Min.z )
      { C.c[3]-=C.c[3]/6; //C.c[3]=V[I].z>Draft+Min.z?0xFF:0xBF;
        C.c[0] = ( C.c[0]+UnderWaterColor.c[0] )/2;
        C.c[1] = ( C.c[0]+UnderWaterColor.c[1] )/2;
        C.c[2] = ( C.c[0]+UnderWaterColor.c[2] )/2;
      } glColor4ubv( C.c ); dot( V[I] );
    } glEnd();
    if( Sides==mvBoth && Layer.Symmetric )
    { glNormal3dv( ~W ); glBegin( GL_POLYGON );
      for( I=K-1; I>=0; I-- ){ C.C=Layer.LClr.C;
        if( V[I].z<Draft+Min.z )
        { C.c[3]-=C.c[3]/6; //C.c[3]=V[I].z>Draft+Min.z?0xFF:0xBF;
          C.c[0] = ( C.c[0]+UnderWaterColor.c[0] )/2;
          C.c[1] = ( C.c[0]+UnderWaterColor.c[1] )/2;
          C.c[2] = ( C.c[0]+UnderWaterColor.c[2] )/2;
        } glColor4ubv( C.c ); dot( ~V[I] );
      } glEnd();
    }
  } glLineWidth( 0.2 );
}
void Surface::Extents()                // Экстремумы по всем контрольным точкам
{ for( int i=0; i<NoCoPoint; i++ )
  if( !i )Min=Max=P[0].V; else { Vector &V=P[i].V;
    if( Min.x>V.x )Min.x=V.x; else if( Max.x<V.x )Max.x=V.x;
    if( Min.y>V.y )Min.y=V.y; else if( Max.y<V.y )Max.y=V.y;
    if( Min.z>V.z )Min.z=V.z; else if( Max.z<V.z )Max.z=V.z;
} }
static void Swab( Vector &A, Vector &B ){ Vector C=A; A=B; B=C; } // пересборка
static void Swab( Items &A, Items &B ){ Items C=A; A=B; B=C; } // не по адресам
static void Revert( Items &T )                        // с полной перестановкой
{ for( int i=0; i<T.NoSplines/2; i++ )Swab( T.S[i].P,T.S[T.NoSplines-i-1].P );
}
void InterSection::ReConnect() // перенастройка по первому фрагменту образцу
{ if( NIt<2 )return;
  int I,J,Id=-1;                               // индексы с точкой присоединения
  Vector First=T[0].S[0].P,                    // начальная точка и конец
         Last=T[0].S[T[0].NoSplines-1].P;      // фрагмента в поисковых запросах
   for( int k=1; k<NIt; k++ )
   { Real D=abs( Last-T[I=k].S[J=0].P ); Id=0; // последний к первому - пропуск
     for( int i=k; i<NIt; i++ )
     for( int j=0; j<T[i].NoSplines; j+=T[i].NoSplines-1 )   // бьём по хвостам
     { Vector &V=T[i].S[j].P;
       if( i!=k || j>0 )
       if( abs( Last-V )<D ){ D=abs( Last-V ); I=i; J=j; Id=+1; }
       if( abs( First-V )<D ){ D=abs( First-V ); I=i; J=j; Id=-1; }
     }                                            // зацепка за начальную точку
     if( Id<0 ){ if( !J )Revert( T[I] ); Swab( T[0],T[I] ); First=T[0].S[0].P; } else
     if( Id>0 ){ if( J )Revert( T[I] );
                 if( I>k )Swab( T[I],T[k] ); Last=T[k].S[T[k].NoSplines-1].P; }
   }
}
void InterSection::ReStation()
{  if( NIt<1 )return;
  int I=0,J=0;         // индексы новой искомой точки
  Vector B=Max;
   B.x=T[0].S[0].P.x;  // исходная контрольная точка в шпангоуте на максималках
   //
   //       поиск нижней точки в диаметральной плоскости как условие ориентации
   //
   for( int i=0; i<NIt; i++ )
   for( int j=0; j<T[i].NoSplines; j+=T[i].NoSplines-1 )     // бьём по хвостам
   { Vector V=T[i].S[j].P;
     if( V.y<0 )V.y=0;
     if( V.y<B.y || ( V.y<1e-3 && B.y<1e-3 && V.z<B.z ) ){ B=V; I=i; J=j; }//!?
   }
   if( J )Revert( T[I] );            // пересортировка для кривой задом наперёд
   if( I )Swab( T[0],T[I] );         //{ Items W=T[0]; T[0]=T[I]; T[I]=W; }
   //
   // пробуем переприсоединять перепутанные концы теоретических контуров
   //
   ReConnect();
/* for( int k=1; k<NIt; k++ )             // с поиском по направлению вверх
   { B=T[k-1].S[T[k-1].NoSplines-1].P;    // последняя точка прошлого фрагмента
     D=abs( B-T[I=k].S[J=0].P );
     for( int i=k; i<NIt; i++ )
     for( int j=0; j<T[i].NoSplines; j+=T[i].NoSplines-1 )
       if( i!=k || j>0 )
       if( abs( B-T[i].S[j].P )<D ){ D=abs( B-T[i].S[j].P ); I=i; J=j; }
     if( J )Revert( T[I] );
     if( I!=k )Swab( T[I],T[k] );
   }
*/
}
Vector *InterSection::ReButtocks( int &Iu )
{ Real Md,Mu; int Dir=0,I=0,Id=0,i,j;  Iu=0;
       Md=Mu=T[0].S[0].P.z;
  static Vector *VB=NULL; VB=(Vector*)Allocate( NPt*sizeof( Vector ),VB );
   for( int i=0; i<NIt; i++ )     // распутывание фрагментов к простому вектору
   for( int j=0; j<T[i].NoSplines; j++ )VB[I++]=T[i].S[j].P;

   for( I=1; I<NPt; I++ ){ Vector &V=VB[I];
     if( Mu<V.z ){ Mu=V.z; Iu=I; } // ищем минимальную и максимальную аппликату
     if( Md>V.z ){ Md=V.z; Id=I;
       if( V.x>VB[I-1].x           // и направление по ходу батокса на днище
        || ( I<NPt-1 && V.x<VB[I+1].x ) )Dir=1; else Dir=-1;
   } }
   if( Dir<0 )                     // задом на перёд - нехорошо, исправляется
   { for( i=0; i<NPt/2; i++ )Swab( VB[i],VB[NPt-i-1] ); Id=NPt-Id-1; Iu=NPt-Iu-1; }
                                   // нос идет в начало, и корма обратным ходом
   if( !Id )I=Iu; else
   for( i=Id,j=0,I=0; j<NPt; j++,i=(i+1)%NPt )
      { Swab( VB[j],VB[i] ); if( I<=0 )I--; if( i==Iu )I=-I; } // подсчёт длины
   Iu=I;
   return VB;
}

