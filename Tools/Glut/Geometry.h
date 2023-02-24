#ifndef __glut_Geometry_h__
#define __glut_Geometry_h__
//
//   Cube, Sphere, Cone, Cylinder, Torus
//   Dodecahedron, Octahedron, Tetrahedron, Icosahedron, RhombicDodecahedron
//   SierpinskiSponge, Teapot
//
void glutSolidCube( float size ),glutWireCube( float size );

void glutSolidSphere( float radius, int slices, int stacks ),
     glutWireSphere( float radius, int slices, int stacks );

void glutSolidCone( float base, float height, int slices, int stacks ),
     glutWireCone( float base, float height, int slices, int stacks );

void glutSolidCylinder( float radius, float height, int slices, int stacks ),
     glutWireCylinder( float radius, float height, int slices, int stacks );

void glutSolidTorus( float rInner, float rOuter, int nSides, int nRings ),
     glutWireTorus( float rInner, float rOuter, int nSides, int nRings );

void drawSolidCuboctahedron(),drawWireCuboctahedron();
void glutSolidOctahedron(),glutWireOctahedron();
void glutSolidTetrahedron(),glutWireTetrahedron();
void glutSolidIcosahedron(),glutWireIcosahedron();
void glutSolidDodecahedron(),glutWireDodecahedron();
void glutSolidRhombicDodecahedron(),glutWireRhombicDodecahedron();

void glutSolidSierpinskiSponge( int num_levels, float offset[3], float scale ),
     glutWireSierpinskiSponge( int num_levels, float offset[3], float scale );

void glutSolidTeapot( float size ),glutWireTeapot( float size );

#endif
