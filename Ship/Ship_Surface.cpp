
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
    for( I=0; I<K; I++)V+=P[F[N].P[I]].V;              // C.c[3]=V[0].z>=Draught?0xFF:0x7F; glColor4ubv(C.c);
    glNormal3dv( W=(V[2]-V[0])*(V[1]-V[0]) ); glBegin( GL_POLYGON );
    for( I=0; I<K; I++ )
    { C.C=Layer.LClr.C;
      if( V[I].z<Draft+Min.z )
      { C.c[3]-=C.c[3]/6;  //C.c[3]=V[I].z>Draft+Min.z?0xFF:0xBF;
        C.c[0] = ( C.c[0]+UnderWaterColor.c[0] )/2;
        C.c[1] = ( C.c[0]+UnderWaterColor.c[1] )/2;
        C.c[2] = ( C.c[0]+UnderWaterColor.c[2] )/2;
      } glColor4ubv( C.c ); dot( V[I] );
    } glEnd();
    if( Sides==mvBoth && Layer.Symmetric )
    { glNormal3dv( ~W ); glBegin( GL_POLYGON );
      for( I=K-1; I>=0; I-- )
      { C.C=Layer.LClr.C;
        if( V[I].z<Draft+Min.z )
        { C.c[3]-=C.c[3]/6;  //C.c[3]=V[I].z>Draft+Min.z?0xFF:0xBF;
          C.c[0] = ( C.c[0]+UnderWaterColor.c[0] )/2;
          C.c[1] = ( C.c[0]+UnderWaterColor.c[1] )/2;
          C.c[2] = ( C.c[0]+UnderWaterColor.c[2] )/2;
        } glColor4ubv( C.c ); dot( ~V[I] );
      } glEnd();
    }
  } glLineWidth( 0.2 );
}
void Surface::Extents()
{ for( int i=0; i<NoCoPoint; i++ )
  if( !i )Min=Max=P[0].V; else { Vector &V=P[i].V;
    if( Min.x>V.x )Min.x=V.x; else if( Max.x<V.x )Max.x=V.x;
    if( Min.y>V.y )Min.y=V.y; else if( Max.y<V.y )Max.y=V.y;
    if( Min.z>V.z )Min.z=V.z; else if( Max.z<V.z )Max.z=V.z;
  }
}
