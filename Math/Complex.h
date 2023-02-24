//
//!                     Elementary Functions of a Complex Variable ©∙2020-09-25
#ifndef __complex_h__
#define __complex_h__
#include <Math.h>
struct complex
{ double x,y;                                         // real & imaginary part
  friend double& real( complex &c ){ return c.x; } double& real(){ return x; }
  friend double& imag( complex &c ){ return c.y; } double& imag(){ return y; }
  complex& operator=( const double& r ){ x=r,y=0.0; return *this; }
  complex& operator=( const complex& c ){ x=c.x,y=c.y; return *this; }
     bool  operator!()const{ return x==0.0 && y==0.0; }    // если чистый ноль?
  complex  operator-()const{ return (complex){ -x,-y }; }  // оригинал сохранен
  complex  operator~()const{ return (complex){ x,-y }; }   // сопряженная копия
  complex  operator++(int)const{ return (complex){ -y,x }; } //! ×i  сохранён++
  complex  operator--(int)const{ return (complex){ y,-x }; } //! ×-i  направо--
  complex& operator++(){ double w=-y; y=x,x=w; return *this; } //! ×i ++изменён
  complex& operator--(){ double w=y; y=-x,x=w; return *this; } //! ×-i --вправо
  complex& operator+=( const complex& c ){ x+=c.x; y+=c.y; return *this; }
  complex& operator-=( const complex& c ){ x-=c.x; y-=c.y; return *this; }
  complex& operator*=( const double& r ){ x*=r; y*=r; return *this; }
  complex& operator/=( const double& r ){ x/=r; y/=r; return *this; }
  complex& operator*=( const complex& c ){ double w=(x*c.x)-(y*c.y); y=(x*c.y)+(y*c.x); x=w; return *this; }
  complex& operator/=( const complex& c ){ return (*this*=~c)/=(c.x*c.x+c.y*c.y); }
};
inline double sqr( const double& d ){ return d*d; }
inline double abs( const complex& c ){ return hypot( c.y,c.x ); }
inline double arg( const complex& c ){ return atan2( c.y,c.x ); }
inline double norm( const complex& c ){ return c.x*c.x+c.y*c.y; }
inline double norm( const double& x, const double& y ){ return x*x+y*y; }
inline bool operator==( const complex& c,const complex& e ){ return c.x==e.x && c.y==e.y; }
inline bool operator!=( const complex& c,const complex& e ){ return c.x!=e.x || c.y!=e.y; }
inline bool operator< ( const complex& c,const complex& e ){ return norm( c )< norm( e ); }
inline bool operator<=( const complex& c,const complex& e ){ return norm( c )<=norm( e ); }
inline bool operator> ( const complex& c,const complex& e ){ return norm( c )> norm( e ); }
inline bool operator>=( const complex& c,const complex& e ){ return norm( c )>=norm( e ); }
inline double  operator % ( const complex& c,const complex& e ){ return c.x*e.x +c.y*e.y; }
inline complex operator + ( complex c,const double& r ){ c.x+=r; return c; }
inline complex operator - ( complex c,const double& r ){ c.x-=r; return c; }
inline complex operator * ( complex c,const double& r ){ return c*=r; }
inline complex operator / ( complex c,const double& r ){ return c/=r; }
inline complex operator + ( const double& r,complex c ){ c.x+=r; return c; }
inline complex operator - ( const double& r,complex c ){ c.x=r-c.x,c.y=-c.y; return c; }
inline complex operator * ( const double& r,complex c ){ return c*=r; }
inline complex operator / ( const double& r,const complex &c ){ return ~c*=r/norm(c); }
inline complex operator + ( complex c,const complex& e ){ return c+=e; }
inline complex operator - ( complex c,const complex& e ){ return c-=e; }
inline complex operator * ( complex c,const complex& e ){ return c*=e; }
inline complex operator / ( const complex& c,const complex& e ){ return ~e*c/norm( e ); }
inline complex polar( const double& a ){ return (complex){ cos( a ),sin( a ) }; }
inline complex polar( const double& r,const double& a ){ return polar( a )*r; }
inline complex dir( const complex& c ){ return c/abs( c ); }
inline complex inv( const complex& c ){ return ~c/norm( c ); }
inline complex sqr( const complex& c ){ return (complex){ (c.x-c.y)*(c.x+c.y),c.x*c.y*2.0 }; }
inline complex exp( const complex& c ){ return polar( exp( c.x ),c.y ); }
inline complex log( const complex& c ){ return (complex){ log( abs( c ) ),arg( c ) }; }
inline complex log10( const complex& c ){ return log( c )*M_LOG10E; } // log10=0.43429448190325182765
inline complex log_b( const complex& c,const complex& b ){ return log( c )/log( b ); }
inline complex pow( const complex& c,const double& p ){ return exp( log( c )*p ); }
inline complex pow( const complex& c,const complex& z ){ return exp( log( c )*z ); }
inline complex sqrt( const complex& z ){ return pow( z,0.5 ); }
inline complex csqrt( const double& r )
  { if( r>=0 )return (complex){sqrt(r),0}; else return (complex){0,sqrt(-r)}; }
inline complex sin ( const complex& c ){ return (complex){ sin(c.x)*cosh(c.y),cos(c.x)*sinh(c.y) }; }
inline complex cos ( const complex& c ){ return (complex){ cos(c.x)*cosh(c.y),sin(c.x)*sinh(-c.y) }; }
inline complex sinh( const complex& c ){ return (complex){ cos(c.y)*sinh(c.x),sin(c.y)*cosh(c.x) }; }
inline complex cosh( const complex& c ){ return (complex){ cos(c.y)*cosh(c.x),sin(c.y)*sinh(c.x) }; }
inline complex tan ( const complex& c ){ return sin( c )/cos( c ); }
inline complex cot ( const complex& c ){ return cos( c )/sin( c ); }
inline complex tanh( const complex& c ){ return sinh( c )/cosh( c ); }
inline complex coth( const complex& c ){ return cosh( c )/sinh( c ); }
inline complex sec ( const complex& c ){ return inv( cos( c ) ); }
inline complex csc ( const complex& c ){ return inv( sin( c ) ); }
inline complex sech( const complex& c ){ return inv( cosh( c ) ); }
inline complex csch( const complex& c ){ return inv( sinh( c ) ); }
inline complex arcsinh( const complex& z ){ return log( z+sqrt( sqr( z )+1.0 ) ); }
inline complex arccosh( const complex& z ){ return log( z+sqrt( sqr( z )-1.0 ) ); }
inline complex arctanh( const complex& z ){ return log( (1.0+z)/(1.0-z) )*0.5; }
inline complex arccoth( const complex& z ){ return log( (z+1.0)/(z-1.0) )*0.5; }
inline complex arcsin ( const complex& z ){ return --arcsinh( z++ ); }
inline complex arccos ( const complex& z ){ return --arccosh( z++ ); } //=M_PI_2-arcsin( z );
inline complex arctan ( const complex& z ){ return --arccoth( z++ ); }
inline complex arcctg ( const complex& z ){ return --arctanh( z++ ); }
inline complex arcsec ( const complex& z ){ return arccos( inv( z ) ); }
inline complex arccsc ( const complex& z ){ return arcsin( inv( z ) ); }
inline complex arcsech( const complex& z ){ return arccosh( inv( z ) ); }
inline complex arccsch( const complex& z ){ return arcsinh( inv( z ) ); }
#endif
