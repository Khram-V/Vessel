//
#include <StdLib.h>
#define N( a,b,c ) glNormal3f( a,b,c )
#define W( v ) glVertex3fv( v )
#define M( v ) glNormal3fv( v )
#define R 0.70710678118654752440 // M_SQRT1_2
        normal[3*nSides*nRings];
  dpsi= 2.0*M_PI/(float)nRings;
}
        normal[3*nSides*nRings];
  dpsi= 2.0*M_PI/float(nRings-1);
      -(r[v[i][1]][2]-r[v[i][0]][2])*(r[v[i][2]][1]-r[v[i][0]][1]),
      -(r[v[i][1]][0]-r[v[i][0]][0])*(r[v[i][2]][2]-r[v[i][0]][2]),
      -(r[v[i][1]][1]-r[v[i][0]][1])*(r[v[i][2]][0]-r[v[i][0]][0]) );
  const float _n[12][3] =
  { { F,F,0.5},{-F,F,0.5},{-F,-F,0.5},{F,-F,0.5},{ 0,1,0},{-1,0,0},
    { 0,-1,0},{1,0,0},{F,F,-0.5},{-F,F,-0.5},{-F,-F,-0.5},{F,-F,-0.5} };
  const float _r[14][3] =
    {-R,-R,0},{R,-R,0},{R,0,-0.5},{0,R,-0.5},{-R,0,-0.5},{0,-R,-0.5},{0,0,-1}};
/* Compute lookup table of cos and sin values forming around the cirle
    for( i=1; i<size; i++ )(*sint)[i]=sin( angle*i ),
                           (*cost)[i]=cos( angle*i );    (*cost)[size]=1.0;
        V( x*radius,y*radius,z*radius );
        r0=r1; r1-=rStep; glEnd();