//
//	Complex Number Library - Include File
//	class complex:  declarations for complex numbers.
//
#ifndef _COMPLEX_H_
#define _COMPLEX_H_

#include <Math.h>

typedef double Real;
class complex{ // Implementation
   Real re,im;
public: complex(){ re=im=0; }
        complex( Real re_val, Real im_val=0 ){ re=re_val; im=im_val; }

// complex manipulations
  friend Real&   real( complex& z ){ return z.re; } // the real part
  friend Real&   imag( complex& z ){ return z.im; } // the imaginary part
  friend complex conj( const complex& ); // the complex conjugate
  friend Real    norm( const complex& ); // the square of the magnitude
  friend Real    arg ( const complex& ); // the angle in the plane
  friend complex polar( Real mag, Real angle );
	 // Create a complex object given polar coordinates

// Overloaded ANSI C math functions
  friend Real    abs  (const complex&);
  friend complex dir  (const complex&);
  friend complex inv  (const complex&);
  friend complex sqrt (const complex&);
  friend complex sqr  (const complex&);
  friend complex acos (const complex&);
  friend complex asin (const complex&);
  friend complex atan (const complex&);
  friend complex cos  (const complex&);
  friend complex cosh (const complex&);
  friend complex exp  (const complex&);
  friend complex log  (const complex&);
  friend complex log10(const complex&);
  friend complex pow  (const complex& base, Real expon);
  friend complex pow  (Real base, const complex& expon);
  friend complex pow  (const complex& base, const complex& expon);
  friend complex sin  (const complex&);
  friend complex sinh (const complex&);
  friend complex tan  (const complex&);
  friend complex tanh (const complex&);

// Binary Operator Functions
  friend complex operator+(const complex&, const complex&);
  friend complex operator-(const complex&, const complex&);
  friend complex operator*(const complex&, const complex&);
  friend complex operator/(const complex&, const complex&);
  friend complex operator+( Real&, const complex& );
  friend complex operator-( Real&, const complex& );
  friend complex operator*( Real&, const complex& );
  friend complex operator/( Real&, const complex& );
  friend complex operator+(const complex&,  Real& );
  friend complex operator-(const complex&,  Real& );
  friend complex operator*(const complex&,  Real& );
  friend complex operator/(const complex&,  Real& );
  friend int	operator==(const complex&, const complex&);
  friend int	operator!=(const complex&, const complex&);
       complex& operator+=(const complex&);
       complex& operator-=(const complex&);
       complex& operator*=(const complex&);
       complex& operator/=(const complex&);
       complex& operator+=(Real);
       complex& operator-=(Real);
       complex& operator*=(Real);
       complex& operator/=(Real);
       complex  operator+();
       complex  operator-();
};

// Inline complex functions

inline complex complex::operator+(){ return *this; }
inline complex complex::operator-(){ return complex( -re,-im ); }

// Definitions of compound-assignment operator member functions

inline complex& complex::operator+=(const complex& z)
{			       re+=z.re; im+=z.im;	return *this;
}
inline complex& complex::operator+=(Real re_val2)
{			       re+=re_val2;	      	return *this;
}
inline complex& complex::operator-=(const complex& z)
{			       re-=z.re; im-=z.im;   	return *this;
}
inline complex& complex::operator-=(Real re_val2)
{			       re-=re_val2;	      	return *this;
}
inline complex& complex::operator*=(const complex& z)
{				 *this=*this*z;		return *this;
}
inline complex& complex::operator*=(Real re_val2)
{		            re*=re_val2; im*=re_val2; 	return *this;
}
inline complex& complex::operator/=(const complex& z)
{			*this *= conj( z )/abs( z );	return *this;
}
inline complex& complex::operator/=(Real re_val2)
{			    re/=re_val2; im/=re_val2; 	return *this;
}

// Definitions of non-member complex functions

inline complex conj ( const complex& z ){ return complex( z.re,-z.im ); }
inline  Real   abs  ( const complex& z ){ return hypot( z.im,z.re );	  }
inline complex dir  ( const complex& z ){ return z/abs( z );		  }
inline  Real   arg  ( const complex& z ){ return atan2( z.im,z.re );	  }
inline  Real   norm ( const complex& z ){ return z.re*z.re+z.im*z.im;	  }
inline complex inv  ( const complex& z ){ return conj( z )/norm( z );   }
inline complex sqr  ( const complex& z )
{      return complex( z.re*z.re-z.im*z.im,2*z.re*z.im );
}
inline complex polar( Real mag, Real angle )
{      return complex( mag*cos(angle), mag*sin(angle) );
}
inline complex exp ( const complex& P ){ return polar( exp( P.re ),P.im ); }

// Definitions of non-member binary operator functions

inline complex operator+( const complex& z1,const complex& z2 )
{      return complex( z1.re+z2.re, z1.im+z2.im);
}
inline complex operator-( const complex& z1,const complex& z2 )
{      return complex( z1.re-z2.re, z1.im-z2.im);
}
inline complex operator*( const complex& z1,const complex& z2 )
{      return complex( z1.re*z2.re-z1.im*z2.im,z1.re*z2.im+z1.im*z2.re );
}
inline complex operator/( const complex& z1,const complex& z2 ){
       return z1*conj(z2)/norm( z2 );
}

inline complex operator+( const complex& z, Real& r )
{      return  complex( z.re+r, z.im );
}
inline complex operator-( const complex& z, Real& r ){ return z+ -r; }
inline complex operator*( const complex& z, Real& r )
{      return  complex( z.re*r,z.im*r );
}
inline complex operator/( const complex& z, Real& r )
{      return  complex( z.re/r,z.im/r );
}

inline complex operator+( Real& r, const complex& z ){ return  z+r;     }
inline complex operator-( Real& r, const complex& z ){ return complex( r-z.re,z.im ); }
inline complex operator*( Real& r, const complex& z ){ return  z*r;     }
inline complex operator/( Real& r, const complex& z ){ return r*inv(z); }

inline int  operator==(const complex& z1, const complex& z2)
{      return z1.re == z2.re && z1.im == z2.im;
}
inline int  operator!=(const complex& z1, const complex& z2)
{      return z1.re != z2.re || z1.im != z2.im;
}
#endif
