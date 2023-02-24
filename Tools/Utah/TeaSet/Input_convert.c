/*
 *  Juhana Kouhia, Oct. 25, 1991
 *
 *  Load_patch(filename, patches, verticies);
 *    char *filename; int *patches, *verticies;
 *    A sample program to read Bezier patches in.
 *    Returns count of patches and verticies.
 *  User defined subroutines:
 *    B_patch(ii, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p);
 *      int ii, a, b, ..., p;
 *      Defines one Bezier patch with index number ii,
 *      indexes to points are in a, b, c, ..., p.
 *    B_point(ii, x, y, z);
 *      int ii; double x, y, z;
 *      Defines one point with index number ii.
 */

#include <StdIO.h>
#include <StdLib.h>
#include <String.h>

int main( int ac, char **av )
{ int i,j,patches,vertices,*p,*patch;
  double *v,*vertice;
  FILE *fp;
  if( ac<=1 )
  { fprintf( stderr,"\7\nUse %s <FileName=%d>\n",av[0],ac ); return ac; }
  if( !(fp=fopen( av[1],"r")) )
  { fprintf(stderr,"Load_patch: Can't open %s\n",av[1]); return 1;
  }
  fscanf( fp,"%i\n",&patches );
  p=patch=(int*)calloc( patches,16*sizeof(int) );
  for( i=0; i<patches; i++ )
  for( j=0; j<16; j++ )fscanf( fp,j==15?"%i\n":"%i,",p++ );

  fscanf( fp,"%i\n",&vertices );
  v=vertice=(double*)calloc( vertices,3*sizeof(double) );
  for( i=0; i<vertices; i++ )
  for( j=0; j<3; j++ )fscanf( fp,j==2?"%lf\n":"%lf,",v++ );

  fclose( fp );

  fp=fopen( strcat( av[1],".inc" ),"w" );
  fprintf( fp,"\n  const static short patches=%d;"
              "\n  const static short patch[patches][16] = {\n",patches ); p=patch;
  for( i=0; i<patches; i++ )
  { fprintf( fp,"  {" );
    for( j=0; j<16; j++ )fprintf( fp,j<15?"%3d,":i<patches-1?"%3d },\n":"%3d } };\n",*p++ );
  }
  fprintf( fp,"\n  const static short vertices=%d;"
              "\n  const static float cp[vertices][3] = {\n",vertices ); v=vertice;
  for( i=0; i<vertices; i++ )
  { fprintf( fp,"  {" );
    for( j=0; j<3; j++ )fprintf( fp,j<2?"%g,":i<vertices-1?"%g },\n":"%g } };\n",*v++ );
  }

  fclose( fp );
}
