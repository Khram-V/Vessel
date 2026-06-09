                                            //
#include "View.h"                           // стрелки и оси координат отдельно#include "..\Math\Vector.h"                 //   ©2026-06-07 ‏Lausanne [loˈzan],
//
//    ... из точки (a) в точку (b) с объемной стрелкой в долях её длины
//const Real* arrow( const Real *_a, const Real *_b, _Real l, const colors clr )
{ Vector &a=*(Vector*)_a,
         &b=*(Vector*)_b,d=l*(b-a),e={d.z/8,d.x/8,d.y/8},f={e.z,e.x,e.y};
  line( a,d=b-d,clr );
  glBegin( GL_LINE_STRIP ),dot( d+e ),dot( b ),dot( d-e ),glEnd();
  glBegin( GL_LINE_STRIP ),dot( d+f ),dot( b ),dot( d-f ),glEnd();
//glBegin( GL_TRIANGLE_FAN ),dot( b ),dot( d+e ),dot( d+f ),
//                                    dot( d-e ),dot( d-f ),dot( d+e ),glEnd();
  return b;
}
//  автоматическая разметка координатных осей с чуть затемненными надписями xyz
//
void axis( Place &P, _Real L, _Real Y, _Real Z,
  const char *x, const char *y, const char *z, const colors clr )
{  glPushAttrib( GL_LINE_BIT ); glLineWidth( 0.1 );
   arrow( (Vector){ 0,0,-Z },(Vector){ 0,0,Z },0.025,clr ),
   arrow( (Vector){ 0,-Y,0 },(Vector){ 0,Y,0 },0.025 ),
   arrow( (Vector){ -L,0,0 },(Vector){ L,0,0 },0.025 );
   color( clr,-0.5 ); glLineWidth( 0.2 );
  P.Text( _North,0,0,Z,z )
   .Text( _North,0,Y*1.01,0,y )
   .Text( _North_East,L*1.01,0,0,x ); glPopAttrib();
}
