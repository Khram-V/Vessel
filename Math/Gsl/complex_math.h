//
//    GSL 2.6 complex_math.h
//
#ifndef __complex_GSL__
#define __complex_GSL__
#include <Math.h>

typedef struct{ double x,y; } complex; // Complex numbers
#define real( z ) ( z.x )
#define imag( z ) ( z.y )

complex polar( const double& r,const double& theta ); // r×e^(i×theta)
#define ONE    ((complex){1.0,0.0})
#define ZERO   ((complex){0.0,0.0})
#define NEGONE ((complex){-1.0,0.0})

// Properties of complex numbers

double arg(complex z);    // return arg(z), -п<arg(z)<=+п
double abs(complex z);    // return |z|
double abs2(complex z);   // return |z|²
double logabs(complex z); // return log|z|

// Complex arithmetic operators

complex add (complex a, complex b);    // r=a+b
complex sub (complex a, complex b);    // r=a-b
complex mul (complex a, complex b);    // r=a*b
complex div (complex a, complex b);    // r=a/b

complex add_real (complex a,double x); // r=a+x
complex sub_real (complex a,double x); // r=a-x
complex mul_real (complex a,double x); // r=a*x
complex div_real (complex a,double x); // r=a/x

complex add_imag (complex a,double y); // r=a+iy
complex sub_imag (complex a,double y); // r=a-iy
complex mul_imag (complex a,double y); // r=a*iy
complex div_imag (complex a,double y); // r=a/iy

complex conjugate(complex z);          // r=conj(z)
complex inverse  (complex a);          // r=1/a
complex negative (complex a);          // r=-a

// Elementary Complex Functions

complex sqrt (complex z);              // r=sqrt(z)
complex sqrt_real (double x);          // r=sqrt(x) (x<0 ok)

complex pow (complex a, complex b);    // r=a^b
complex pow_real (complex a,double b); // r=a^b

complex exp (complex a);               // r=exp(a)
complex log (complex a);               // r=log(a) (base e)
complex log10 (complex a);             // r=log10(a) (base 10)
complex log_b (complex a, complex b);  // r=log_b(a) (base=b)

// Complex Trigonometric Functions

complex sin (complex a);               // r=sin(a)
complex cos (complex a);               // r=cos(a)
complex sec (complex a);               // r=sec(a)
complex csc (complex a);               // r=csc(a)
complex tan (complex a);               // r=tan(a)
complex cot (complex a);               // r=cot(a)

// Inverse Complex Trigonometric Functions

complex arcsin (complex a);            // r=arcsin(a)
complex arcsin_real (double a);        // r=arcsin(a)
complex arccos (complex a);            // r=arccos(a)
complex arccos_real (double a);        // r=arccos(a)
complex arcsec (complex a);            // r=arcsec(a)
complex arcsec_real (double a);        // r=arcsec(a)
complex arccsc (complex a);            // r=arccsc(a)
complex arccsc_real (double a);        // r=arccsc(a)
complex arctan (complex a);            // r=arctan(a)
complex arccot (complex a);            // r=arccot(a)

// Complex Hyperbolic Functions

complex sinh (complex a);              // r=sinh(a)
complex cosh (complex a);              // r=coshh(a)
complex sech (complex a);              // r=sech(a)
complex csch (complex a);              // r=csch(a)
complex tanh (complex a);              // r=tanh(a)
complex coth (complex a);              // r=coth(a)

// Inverse Complex Hyperbolic Functions

complex arcsinh (complex a);           // r=arcsinh(a)
complex arccosh (complex a);           // r=arccosh(a)
complex arccosh_real (double a);       // r=arccosh(a)
complex arcsech (complex a);           // r=arcsech(a)
complex arccsch (complex a);           // r=arccsch(a)
complex arctanh (complex a);           // r=arctanh(a)
complex arctanh_real (double a);       // r=arctanh(a)
complex arccoth (complex a);           // r=arccoth(a)

#endif
