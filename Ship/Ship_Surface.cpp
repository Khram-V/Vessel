
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
{ Vector norm=Zero;
  for( int i=i1+1; i<i2; i++ )norm+=(Cont[i+1]-Cont[i1])*(Cont[i]-Cont[i1]);
  glNormal3dv( norm );          // в гидромеханике этот расчёт долен быть здесь
  glBegin( GL_POLYGON );
    for( int i=i1; i<=i2; i++ ){ (norm=Cont[i]).z+=delta; dot( norm ); }
  glEnd();
  if( right )
  { glBegin( GL_POLYGON );
    for( int i=i2; i>=i1; i-- )
    { (norm=Cont[i]).z+=delta; norm.y=-norm.y; dot( norm ); }
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
{ static Flex W,wL; Real delta=Draft+Min.z; int i,I,J,K,N; Color c;
  for( N=0; N<NoFaces; N++ ) // синхронная прорисовка треугольников двух бортов
  if( (K=F[N].Capacity)>2 )             // у граней должно быть боле двух рёбер
  { Layers &Layer=L[min(NoLayers,F[N].LayerIndex)]; // извне указанные свойства
    bool right=(Sides==mvBoth && Layer.Symmetric); wL.len=W.len=J=i=0;
    glLineWidth( 1 );    /// Alice AI из Яндекса стала эдесь хорошим помощником
#if 1
   Vector v,V1,V2; J=-1,i=0;
    while( i<=K+J )
    { (V2=P[F[N].P[i%K]].V).z-=delta;
      if( i++ )                        // i - показывает следующий узел = длину
      if( inInter( V1,V2 ) ){          //     однократно, но по всем рёбрам
/*R:*/  if( J<0 || W.len==0 ){ v=V1; if( J<0 )J=i,W.len=0; } else v=V2;
        if( v.z==0.0 )W+=v; else W+=newInter( V1,V2 );
        if( W.len>1 )
        { // if( W.len>2 ) --- невидимые двойки пусть нарисуются, аль нет {wL}?
          { c.C=Layer.LClr.C;
            if( W[1].z<0 )uWater else wL+=W[0],wL+=W[-1];                       // if( W[0].x<=W[-1].x )wL+=W[0],wL+=W[-1]; else wL+=W[-1],wL+=W[0];
            glColor4ubv( c.c );                                                 // print("%i,",W.len);color(W[1].z>0?lightcyan:magenta); // необходима доработка
            Draw( W,0,W.len-1,delta,right );
          } W[0]=W[-1]; W.len=1;                                                // W.len=0; goto R; == первый вариант с лишним переходом
      } }
      if( J<0 || W.len>0 )W+=V2; V1=V2; // J<0 по началу, и от пересечения нуля
    }
    if( J<0 ) // if( W.len>0 )        // c заданной расцветкой для каждой грани
    { c.C=Layer.LClr.C; if( W[0].z<0 )uWater glColor4ubv( c.c );
      Draw( W,0,W.len-1,delta,right );
    }
#else
  static Flex V; int j; V.len=j=0;
    for( I=0; I<K; I++ )(V+=P[F[N].P[I]].V).z-=delta;
    while( i<=K+J ){ I=i%K; int I1=(i+K-1)%K; i++;
      if( inInter(V[I1],V[I]) )
      { if( W.len )j=I; else { j=I1; if( !J )J=I+1; }
        if( V[j].z==0.0 )W+=V[j]; else W+=newInter( V[I1],V[I] );
        if( j==I )
        { c.C=Layer.LClr.C; if( W[1].z<0 )uWater else wL+=W[0],wL+=W[-1];
          glColor4ubv( c.c );
          Draw( W,0,W.len-1,delta,right ); W.len=0; --i;// с одной левой точкой
      } } if( W.len )W+=V[I];
    }
    if( !W.len ){ c.C=Layer.LClr.C; if( V[0].z<0 )uWater; glColor4ubv( c.c );
                  Draw( V,0,V.len-1,delta,right );
                }
#endif
    for( i=0; i<wL.len; i++ )wL[i].z+=delta;
    color( white );                                    glDisable( GL_LIGHTING );
    glLineWidth( 2 );                               // glNormal3d( 0,0,-1 );
    for( i=0; i<wL.len; i+=2 )liney( wL[i],wL[i+1] );  glEnable( GL_LIGHTING );
  } color( lightmagenta );
  for( K=0; K<NoCurves; K++ )
  { Vector V,W;
    for( I=0; I<C[K].Capacity; I++ ){ W=P[C[K].P[I]].V;
      if( I ){ if( Sides==mvBoth )liney( V,W ) ; else line( V,W ); } V=W;
  } } glLineWidth( 0.2 );
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
