#ifndef __Point_hxx
#define __Point_hxx
#include <Math.h>
//
//      Представление точки в двумерном пространстве (Point)
//         и в дискретном поле графического монитора (point)
//
typedef double Real;  typedef const Real&  _Real;  // повторный переход
struct Point; typedef const Point& _Point; // к 64-разрядной арифметике
struct point; typedef const point& _point; //
                                                  //
struct point                                     // Точка с дискретным отсчетом
{ int x,y; operator Point();                    //
   point  &operator += ( _point p ){ x+=p.x,y+=p.y; return *this; }
   point  &operator -= ( _point p ){ x-=p.x,y-=p.y; return *this; }
  friend bool operator ==( _point p,_point q ){ return p.x==q.x && p.y==q.y; }
  friend bool operator !=( _point p,_point q ){ return p.x!=q.x || p.y!=q.y; }
  friend point operator + ( point p,_point q ){ return p+=q; }
  friend point operator - ( point p,_point q ){ return p-=q; }
};                                          //
struct Point                                // Представление точки на плоскости
{ Real x,y;   operator point();             //
  friend bool operator==( _Point A,_Point B ){ return A.x==B.x && A.y==B.y; }
  friend bool operator!=( _Point A,_Point B ){ return A.x!=B.x || A.y!=B.y; }
  friend Real  norm( _Point P ){ return P.x*P.x + P.y*P.y; }
  friend Point conj( _Point P ){ return (Point){ P.x,-P.y }; }
   Point operator -(){ return (Point){ -x,-y }; }
   Point operator ~(){ return (Point){  y, x }; }
  Point& operator =( _Real  X ){ x=X,y=0.0;     return *this; }
  Point& operator+=( _Point P ){ x+=P.x,y+=P.y; return *this; }
  Point& operator-=( _Point P ){ x-=P.x,y-=P.y; return *this; }
  Point& operator*=( _Real  A ){ x*=A,y*=A;     return *this; }
  Point& operator/=( _Real  A ){ x/=A,y/=A;     return *this; }
  Point& operator*=( _Point B ){ return *this=(Point){ x*B.x-y*B.y,x*B.y+y*B.x }; }
  Point& operator/=( _Point B ){ return *this*=conj( B )/norm( B ); }
  friend Point operator + ( Point P,_Point Q ){ return P+=Q; }
  friend Point operator - ( Point P,_Point Q ){ return P-=Q; }
  friend Point operator * ( Point P,_Point Q ){ return P*=Q; }
  friend Point operator / ( Point P,_Point Q ){ return P/=Q; }
  friend Point operator + ( Point P, _Real D ){ P.x+=D; return P; }
  friend Point operator - ( Point P, _Real D ){ P.x-=D; return P; }
  friend Point operator * ( Point P, _Real D ){ return P*=D; }
  friend Point operator * ( _Real D, Point P ){ return P*=D; }
  friend Point operator / ( Point P, _Real D ){ return P/=D; }
};
//line Point set( _Real X,_Real Y=0.0 ){ return (Point){ X,Y }; }
inline  Real norm ( _Real X,_Real Y ){ return X*X + Y*Y; }
inline  Real sqr  ( _Real R  ){ return R*R; }
inline  Real abs  ( _Point P ){ return hypot( P.y,P.x ); }
inline  Real arg  ( _Point P ){ return atan2( P.y,P.x ); }
inline Point dir  ( _Point P ){ return P/hypot( P.y,P.x ); }
inline Point sqr  ( _Point P ){ return (Point){ P.x*P.x-P.y*P.y,2*P.x*P.y }; }
inline Point polar( _Real A  ){ return (Point){ cos( A ),sin( A ) }; }
inline Point polar( _Real M,_Real A ){ return M*polar( A ); }
inline Point log  ( _Point P ){ return (Point){ log( abs( P ) ),arg( P ) }; }
inline Point exp  ( _Point P ){ return polar( exp( P.x ),P.y ); }
inline Point pow  ( _Point P,_Real pw ){ return exp( log( P )*pw ); }
inline Point pow  ( _Point P,_Point qw ){ return exp( log( P )*qw ); }
inline Point sqrt ( _Point P ){ return pow( P,0.5 ); }
#endif
