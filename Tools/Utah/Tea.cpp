/*
 * TeaSet(tm) rendering code. (c) Fri Dec 24 1999 Pawel W. Olszta
 */
#include <GL/GL.h>
#include <Math.h>
#include <String.h>

static const int Solid_N_SubDiv=9, // General defs
                 Wire_N_SubDiv=12;
static const bool Solid_Normals=true;

/* Bernstein coefficients only have to be precomputed once
 * (number of patch subdivisions is fixed)
 * Can thus define arrays for them here, they will be filled upon first use.
 * 3rd order Bezier surfaces have 4 Bernstein coeffs.
 * Have separate caches for solid and wire as they use a different number of
 * subdivisions _0 is for Bernstein polynomials, _1 for their first derivative
 * (which we need for normals)
 *
 * Bit of caching: vertex indices and normals only need to be generated once
 * for a given number of subdivisions as they don't change with scale.
 * Vertices can be cached and reused if scale didn't change.
 */
/// -- PRIVATE FUNCTIONS ----------------------------------------------------
//
static void DrawGeometrySolid11
( float *vertices, float *normals, float *textcs, const int numVertices,
  short *vertIdxs, int numParts, int numVertIdxsPerPart )
{  glEnableClientState( GL_VERTEX_ARRAY );
   glEnableClientState( GL_NORMAL_ARRAY );
   glVertexPointer( 3,GL_FLOAT,0,vertices );
   glNormalPointer( GL_FLOAT,0,normals );
   if( textcs ){ glEnableClientState( GL_TEXTURE_COORD_ARRAY );
                 glTexCoordPointer( 2,GL_FLOAT,0,textcs );
               }
   if( !vertIdxs )glDrawArrays( GL_TRIANGLES,0,numVertices ); else
   if( numParts>1 )for( int i=0; i<numParts; i++ )
     glDrawElements( GL_TRIANGLE_STRIP,numVertIdxsPerPart,GL_UNSIGNED_SHORT,vertIdxs+i*numVertIdxsPerPart );
   else
     glDrawElements( GL_TRIANGLES,numVertIdxsPerPart,GL_UNSIGNED_SHORT,vertIdxs );
   glDisableClientState( GL_NORMAL_ARRAY );
   if( textcs )glDisableClientState( GL_TEXTURE_COORD_ARRAY );
   if( Solid_Normals )               //!++ Generate + Draw Normal Visualization
   { float currentColor[4],          // Setup draw color: (1,1,1)-shape's color
           verticesForNormalVisualization[numVertices*6];
     for( int i=0,j=0; i<numVertices*3; i+=3,j+=6 )
     { verticesForNormalVisualization[j+0] = vertices[i+0];
       verticesForNormalVisualization[j+1] = vertices[i+1];
       verticesForNormalVisualization[j+2] = vertices[i+2];
       verticesForNormalVisualization[j+3] = vertices[i+0] + normals[i+0]/36;
       verticesForNormalVisualization[j+4] = vertices[i+1] + normals[i+1]/36;
       verticesForNormalVisualization[j+5] = vertices[i+2] + normals[i+2]/36;
     }
     glGetFloatv( GL_CURRENT_COLOR,currentColor );
     glColor4f( 1-currentColor[0],1-currentColor[1],1-currentColor[2],currentColor[3] );
     glVertexPointer( 3,GL_FLOAT,0,verticesForNormalVisualization );
     glDrawArrays( GL_LINES,0,numVertices*2 );
     glColor4f( currentColor[0],currentColor[1],currentColor[2],currentColor[3] );
   }
   glDisableClientState(GL_VERTEX_ARRAY);
}
/* Version for OpenGL (ES) 1.1 */
static void DrawGeometryWire11( float *vertices, float *normals,
            short *vertIdxs, int numParts, int numVertPerPart )
{  glEnableClientState( GL_VERTEX_ARRAY );
   glEnableClientState( GL_NORMAL_ARRAY );
   glVertexPointer( 3,GL_FLOAT,0,vertices ); // Draw per face (TODO: could use
   glNormalPointer( GL_FLOAT,0,normals );    // glMultiDrawArrays if available)
   if( !vertIdxs )for( int i=0; i<numParts; i++ )
     glDrawArrays( GL_LINE_STRIP,i*numVertPerPart, numVertPerPart );
   else for( int i=0; i<numParts; i++ )
     glDrawElements( GL_LINE_STRIP,numVertPerPart,GL_UNSIGNED_SHORT,vertIdxs+i*numVertPerPart);
   glDisableClientState( GL_VERTEX_ARRAY );
   glDisableClientState( GL_NORMAL_ARRAY );
}
// evaluate 3rd order Bernstein polynomial and its 1st deriv
static void bernstein3( int i,float x,float *r0,float *r1 )
{ float invx=1.0f-x;           // r0: zero order coeff, r1: first deriv coeff
  switch( i )
  { case 0: *r0 = invx*invx*invx;              // invx * invx * invx
            *r1 = -3*invx*invx;         break; //   -3 * invx * invx
    case 1: *r0 = 3*x*invx*invx;               // 3 * x * invx * invx
            *r1 = 3*invx*invx-6*x*invx; break; // 3 * invx*invx - 6*x*invx
    case 2: *r0 = 3*x*x*invx;                  // 3*x*x*invx
            *r1 = 6*x*invx - 3*x*x;     break; // 6*x*invx - 3*x*x
    case 3: *r0 = x*x*x;                       // x * x * x
            *r1 = 3*x*x;                break; // 3 * x * x
   default: *r0=*r1=0;
  }
}
static void pregenBernstein(int nSubDivs, float (*bern_0)[4], float (*bern_1)[4])
{ for( int s=0; s<nSubDivs; s++ )
  { float x=s/(nSubDivs-1.f );    // 3rd order polynomial
    for( int i=0; i<4; i++ )bernstein3( i,x,bern_0[s]+i,bern_1[s]+i );
  }
}
// based on flag either rotate patches around y axis to other 3 quadrants (flag=4) or reflect patch across x-y plane (flag=2)
static void rotOrReflect(int flag, int nVals, int nSubDivs, float *vals)
{ int u,i,o;
  if (flag==4)
  { int i1=nVals,i2=nVals*2,i3=nVals*3;
    for( o=0; o<nVals; o+=3 )
    { vals[i1+o+0] =  vals[o+2]; // 90° rotation
      vals[i1+o+1] =  vals[o+1];
      vals[i1+o+2] = -vals[o+0];
      vals[i2+o+0] = -vals[o+0]; // 180° rotation
      vals[i2+o+1] =  vals[o+1];
      vals[i2+o+2] = -vals[o+2];
      vals[i3+o+0] = -vals[o+2]; // 270° rotation
      vals[i3+o+1] =  vals[o+1];
      vals[i3+o+2] =  vals[o+0];
    }
  }
  else if (flag==2)      // copy over values, reversing row order to keep
  {                      // winding correct, and negating z to perform the flip
    for( u=0; u<nSubDivs; u++ )           // per row
    { int off=(nSubDivs-u-1)*nSubDivs*3;  // read last row first from the already existing rows
           o  = nVals + u   *nSubDivs*3;  // write last row as first row to output
      for( i=0; i<nSubDivs*3; i+=3,o+=3 ) // each row has nSubDivs points consisting of three values
      { vals[o+0] =  vals[off+i+0];
        vals[o+1] =  vals[off+i+1];
        vals[o+2] = -vals[off+i+2];
      }
    }
  }
}
// verts array should be initialized to 0!
static int evalBezierWithNorm(float cp[4][4][3], int nSubDivs, float (*bern_0)[4], float (*bern_1)[4], int flag, int normalFix, float *verts, float *norms)
{ int nVerts=nSubDivs*nSubDivs;
  int nVertVals=nVerts*3; // number of values output for one patch, flag (2 or 4) indicates how many times we will write this to output
  int u,v,i,j,o;
  for( u=0,o=0; u<nSubDivs; u++ )   // generate vertices and coordinates for the patch
  { for (v=0; v<nSubDivs; v++,o+=3) // for normals, get two tangents at the vertex using partial derivatives of 2D Bezier grid
    { float tan1[3]={0}, tan2[3]={0}, len;
      for( i=0; i<=3; i++ )
      { float vert_0[3]={0},vert_1[3]={0};
        for( j=0; j<=3; j++ )
        { vert_0[0] += bern_0[v][j] * cp[i][j][0];
          vert_0[1] += bern_0[v][j] * cp[i][j][1];
          vert_0[2] += bern_0[v][j] * cp[i][j][2];
          vert_1[0] += bern_1[v][j] * cp[i][j][0];
          vert_1[1] += bern_1[v][j] * cp[i][j][1];
          vert_1[2] += bern_1[v][j] * cp[i][j][2];
        }
        verts[o+0] += bern_0[u][i]*vert_0[0];
        verts[o+1] += bern_0[u][i]*vert_0[1];
        verts[o+2] += bern_0[u][i]*vert_0[2];

        tan1[0] += bern_0[u][i]*vert_1[0];
        tan1[1] += bern_0[u][i]*vert_1[1];
        tan1[2] += bern_0[u][i]*vert_1[2];
        tan2[0] += bern_1[u][i]*vert_0[0];
        tan2[1] += bern_1[u][i]*vert_0[1];
        tan2[2] += bern_1[u][i]*vert_0[2];
      } // get normal through cross product of the two tangents of the vertex
      norms[o+0] = tan1[1] * tan2[2] - tan1[2] * tan2[1];
      norms[o+1] = tan1[2] * tan2[0] - tan1[0] * tan2[2];
      norms[o+2] = tan1[0] * tan2[1] - tan1[1] * tan2[0];
      len=(float)sqrt( norms[o+0]*norms[o+0] + norms[o+1]*norms[o+1] + norms[o+2]*norms[o+2] );
      norms[o+0] /= len;
      norms[o+1] /= len;
      norms[o+2] /= len;
    }
  }
  if( normalFix ) // Fix normal vector if needed
  { for (o=0; o<nSubDivs*3; o+=3) // whole first row (first nSubDivs normals) is broken: replace normals for the whole row
    { norms[o+0] = 0.f;
      norms[o+1] = normalFix==1? 1.f:-1.f;
      norms[o+2] = 0.f;
  } } // now based on flag either rotate patches around y axis to other 3 quadrants (flag=4) or reflect patch across x-y plane (flag=2)
  rotOrReflect(flag, nVertVals, nSubDivs, verts);
  rotOrReflect(flag, nVertVals, nSubDivs, norms);
  return nVertVals*flag;
}                                    // verts array should be initialized to 0!
static int evalBezier(float cp[4][4][3], int nSubDivs, float (*bern_0)[4], int flag, float *verts)
{ int nVerts    = nSubDivs*nSubDivs;
  int nVertVals = nVerts*3; // number of values output for one patch, flag (2 or 4) indicates how many times we will write this to output
  int u,v,i,j,o;            // generate vertices and coordinates for the patch
  for (u=0,o=0; u<nSubDivs; u++)
  { for (v=0; v<nSubDivs; v++, o+=3)
    { for (i=0; i<=3; i++)
      { float vert_0[3]={0};
        for (j=0; j<=3; j++)
        { vert_0[0] += bern_0[v][j] * cp[i][j][0];
          vert_0[1] += bern_0[v][j] * cp[i][j][1];
          vert_0[2] += bern_0[v][j] * cp[i][j][2];
        }
        verts[o+0] += bern_0[u][i]*vert_0[0];
        verts[o+1] += bern_0[u][i]*vert_0[1];
        verts[o+2] += bern_0[u][i]*vert_0[2];
    } }
  } // now based on flag either rotate patches around y axis to other 3 quadrants (flag=4) or reflect patch across x-y plane (flag=2)
  rotOrReflect( flag,nVertVals,nSubDivs,verts );
  return nVertVals*flag;
}
static void TeaSet( float scale, bool useWireMode,
                    const float (*cpdata)[3], const short (*patchdata)[16],
                    short *vertIdxs, float *verts, float *norms, float *texcs,
                    float *lastScale, bool *inited,
                    bool needNormalFix, bool rotFlip, float zOffset,
                    int nVerts,int nInputPatches,int nPatches,int nTriangles )
{
  int nSubDivs=useWireMode?Wire_N_SubDiv:Solid_N_SubDiv;

  if( !*inited || scale!=*lastScale )     // check if need to generate vertices
  { float cp[4][4][3], // for internal use to hold pointers to static vars/arrays
          bern_0[nSubDivs][4],      // Get relevant static arrays and variables
          bern_1[nSubDivs][4];
    // set vertex array to all 0 (not necessary for normals and vertex indices)
    memset( verts,0,nVerts*3*sizeof(float) );
    // pregen Berstein polynomials and their first derivatives (for normals)
    if( !*inited )pregenBernstein( nSubDivs,bern_0,bern_1 );
    int p,o;                               // generate vertices and normals
    for( p=0,o=0; p<nInputPatches; p++ )   // set flags for evalBezier function
    { int flag = rotFlip?p<6?4:2:1; // For teapot and teacup, first six patches get 3 copies (rotations), others get 2 copies (flips). No rotating or flipping at all for teaspoon
      int normalFix = needNormalFix?p==3?1:p==5?2:0:0; // For teapot, fix normal vectors for vertices on top of lid (patch 4) and on middle of bottom (patch 6). Different flag value as different normal needed
      for( int i=0; i<16; i++ )  // collect control points
      { /* Original code draws with a 270° rot around X axis, a scaling and a translation along the Z-axis.
         * Incorporating these in the control points is much cheaper than transforming all the vertices.
         * Original:
         * glRotated( 270.0,1.0,0.0,0.0 );
         * glScaled( 0.5*scale,0.5*scale,0.5*scale );
         * glTranslated( 0.0,0.0,-zOffset ); -> was 1.5 for teapot, but should be 1.575 to center it on the Z axis. Teacup and teaspoon have different offsets
         */
        cp[i/4][i%4][0] =  cpdata[patchdata[p][i]][0]         *scale/2.f;
        cp[i/4][i%4][1] = (cpdata[patchdata[p][i]][2]-zOffset)*scale/2.f;
        cp[i/4][i%4][2] = -cpdata[patchdata[p][i]][1]         *scale/2.f;
      }
      if( !*inited )  // eval bezier patch first time, generate normals as well
        o+=evalBezierWithNorm( cp,nSubDivs,bern_0,bern_1,flag,normalFix,verts+o,norms+o );
      else            // only need to regen vertices
        o+=evalBezier( cp,nSubDivs,bern_0,flag,verts+o );
    }
    *lastScale = scale;
    if( !*inited )
    { int r,c;  // generate texture coordinates if solid teapot/teacup/teaspoon
      if( !useWireMode )                   // generate for first patch
      { for (r=0,o=0; r<nSubDivs; r++)
        { float u=r/(nSubDivs-1.f );
          for( c=0; c<nSubDivs; c++,o+=2)
          { float v=c/(nSubDivs-1.f);
            texcs[o+0] = u;
            texcs[o+1] = v;
        } }
        for( p=1; p<nPatches; p++ )   // copy it over for all the other patches
          memcpy( texcs+p*nSubDivs*nSubDivs*2,texcs,nSubDivs*nSubDivs*2*sizeof(float) );
      }
      if( useWireMode )                           // build vertex index array
      { // build vertex indices to draw teapot/teacup/teaspoon as line strips
        // first strips along increasing u, constant v
        for( p=0,o=0; p<nPatches; p++ )
        { int idx = nSubDivs*nSubDivs*p;
          for( c=0; c<nSubDivs; c++ )
          for( r=0; r<nSubDivs; r++,o++ )vertIdxs[o]=idx+r*nSubDivs+c;
        }                         // then strips along increasing v, constant u
        for( p=0; p<nPatches; p++ )    // don't reset o, we continue appending!
        { int idx = nSubDivs*nSubDivs*p;
          for (r=0; r<nSubDivs; r++)
          { int loc = r*nSubDivs;
            for( c=0; c<nSubDivs; c++,o++ )vertIdxs[o]=idx+loc+c;
        } }
      }
      else  // build vertex indices to draw teapot/teacup/teaspoon as triangles
      { for( p=0,o=0; p<nPatches; p++ )
        { int idx = nSubDivs*nSubDivs*p;
          for (r=0; r<nSubDivs-1; r++)
          { int loc = r*nSubDivs;
            for (c=0; c<nSubDivs-1; c++,o+=6)
            { int row1 = idx+loc+c; // ABC ACD, where B and C are one row lower
              int row2 = row1+nSubDivs;
               vertIdxs[o+5] = row1+0; //! изменен порядок обхода треугольников
               vertIdxs[o+4] = row2+0; //  -- на обратный
               vertIdxs[o+3] = row2+1;
               vertIdxs[o+2] = row1+0;
               vertIdxs[o+1] = row2+1;
               vertIdxs[o+0] = row1+1;
            }
          }
        }
      } *inited = true;
  } }
  if( useWireMode ) /// draw
    DrawGeometryWire11( verts,norms,vertIdxs,nPatches*nSubDivs*2,nSubDivs );
  else
    DrawGeometrySolid11( verts,norms,texcs,nVerts,vertIdxs,1,nTriangles*3 );
}
///* -- INTERFACE FUNCTIONS --------------------------------------------------

#include "Tea.inc"
#define MEM( M )                                        \
 const int N=M,NV=N*Solid_N_SubDiv*Solid_N_SubDiv,      \
           NT=N*(Solid_N_SubDiv-1)*(Solid_N_SubDiv-1)*2,\
           NW=N*Wire_N_SubDiv*Wire_N_SubDiv;            \
  static bool  initW=false,initS=false;                 \
  static short vIdxW[NW*2],vIdxS[NT*3];                 \
  static float normW[NW*3],normS[NV*3],                 \
               vertW[NW*3],vertS[NV*3],texcS[NV*2],     \
               lastW=0.0,  lastS=0.0;
//
// NV,NW - N_SUBDIV^2 vertices per patch
// NT if e.g.7x7 vertices for each patch, there are 6*6 squares for each patch.
//    Each square is decomposed into 2 triangles
//
void UtahTeaPot( float size, bool Solid )      // Renders a wired teapot ...
{ MEM(6*4+4*2) // 6 patches are reproduced(rotated) 4 times, 4 patches(flipped) 2 times
  if( Solid )TeaSet( size,false,cpPot,phPot, vIdxS,vertS,normS,texcS,
                     &lastS,&initS,true,true,1.575, NV,TeaPot_N,N,NT );
        else TeaSet( size,true,cpPot,phPot, vIdxW,vertW,normW,NULL,
                     &lastW,&initW,true,true,1.575, NW,TeaPot_N,N,0 );
}
void UtahTeaCup( float size, bool Solid )      // Renders a wired teacup ...
{ MEM(6*4+1*2) // 6 patches are reproduced (rotated) 4 times, 1 patch (flipped) 2 times
  if( Solid )TeaSet( size/2.5,false,cpCup,phCup, vIdxS,vertS,normS,texcS,
                   &lastS,&initS, false,true,1.5121,NV,TeaCup_N,N,NT );
        else TeaSet( size/2.5,true,cpCup,phCup, vIdxW,vertW,normW,NULL,
                   &lastW,&initW,false,true,1.5121,NW,TeaCup_N,N,0 );
}
void UtahTeaSpoon( float size, bool Solid )    // Renders a wired teaspoon ...
{ MEM( TeaSpoon_N )
  if( Solid )TeaSet( size/2.5,false,cpSpoon,phSpoon, vIdxS,vertS,normS,texcS,
                     &lastS,&initS, false,false,-0.0315,NV,N,N,NT );
        else TeaSet( size/2.5,true,cpSpoon,phSpoon, vIdxW,vertW,normW,NULL,
                     &lastW,&initW,false,false,-0.0315,NW,N,N,0 );
}
