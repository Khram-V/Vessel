/*
    GSL complex arithmetic functions
*/
#include "complex_math.h"

inline double sqr( const double &a ){ return a*a; }              // a²
inline bool null( complex a ){ return real(a)==0.0 && imag(a)==0.0; }
/*
 * Properties of complex numbers
 */
double arg( complex z )
{ if( null( z ) )return 0.0; else return atan2( imag( z ),real( z ) ); }
double abs( complex z ){ return hypot( real(z),imag(z)); }       // = |z|
double abs2( complex z ){ return( sqr(real(z))+sqr(imag(z)) ); } // = |z|²
double logabs( complex z )                                    // = log|z|
{ double xabs=fabs( real( z ) ),
         yabs=fabs( imag( z ) ); // Handle underflow when u is close to 0
  if( xabs>=yabs )return log( xabs ) + 0.5*log1p( sqr( yabs/xabs ) );
           else   return log( yabs ) + 0.5*log1p( sqr( xabs/yabs ) );
}
complex polar( const double &r, const double &theta )   // r×exp(i×theta)
{ return (complex){ r*cos(theta),r*sin(theta) }; }
/*
 * Complex arithmetic operators
 */
complex add (complex a, complex b)
{ return (complex){ real(a) + real(b), imag(a) + imag(b) }; } // z=a+b
complex add_real( complex a,double x )
{ return (complex){ real(a)+x, imag(a) }; }                   // z=a+x
complex add_imag( complex a,double y )
{ return (complex){ real(a), imag(a)+y }; }                   // z=a+iy
complex sub ( complex a,complex b )
{ return (complex){ real(a)-real(b),imag(a)-imag(b) }; }      // z=a-b
complex sub_real (complex a, double x)
{ return (complex){ real(a) - x, imag(a) }; }                 // z=a-x
complex sub_imag( complex a, double y )
{ return (complex){ real(a), imag(a)-y }; }                   // z=a-iy
complex mul( complex a,complex b )                            // z=a*b
{ return
  (complex){ real(a)*real(b)-imag(a)*imag(b),real(a)*imag(b)+imag(a)*real(b) };
}
complex mul_real( complex a,double x )
{ return (complex){ x*real(a), x*imag(a) }; }                 // z=a*x
complex mul_imag (complex a, double y)
{ return (complex){ -y*imag(a), y*real(a) }; }                // z=a*iy
complex div_real( complex a,double x )
{ return (complex){ real(a)/x,imag(a)/x }; }                  // z=a/x
complex div( complex a,complex b )                            // z=a/b
{ const double &ar=real(a),&ai=imag(a),
               &br=real(b),&bi=imag(b);
  double s=1.0/abs( b ),sbr=s*br,sbi=s*bi;
  return (complex){ ( ar*sbr+ai*sbi )*s,( ai*sbr-ar*sbi )*s };
}
complex div_imag( complex a,double y )
{ return (complex){ imag(a)/y,-real(a)/y }; }                 // z=a/(iy)
complex conjugate( complex a )
{ return (complex){ real(a),-imag(a) }; }                     // z=conj(a)
complex negative( complex a )
{ return (complex){ -real(a),-imag(a) }; }                    // z=-a
complex inverse( complex a )                                  // z=1/a
{ double s=1.0/abs(a); return (complex){ (real(a)*s)*s,-(imag(a)*s)*s }; }
/*
 * Elementary complex functions
 */
complex sqrt_real( double x )                                 // z=sqrt(x)
{ if( x>=0 )return (complex){ sqrt(x),0.0 };
       else return (complex){ 0.0,sqrt(-x) };
}
complex sqrt( complex a )                                     // z=sqrt(a)
{ if( null(a) )return a; else
  { double x=fabs( real(a) ),y=fabs( imag(a) ),w;
    if( x>=y )
    { w = sqrt(0.5*x*(1.0+sqrt( 1.0+sqr( y/x ) ))); } else
    { w = sqrt(0.5*y*(x/y+sqrt( 1.0+sqr( x/y ) ))); }
    if( real(a)>=0.0 ){ return (complex){ w,imag(a)/(2.0*w) }; } else
    { double vi = (imag(a)>=0) ? w:-w;
      return (complex){ imag(a)/(2.0*vi),vi };
  } }
}
complex exp( complex a ){ return polar( exp( real(a) ),imag(a) ); } // z=exp(a)

complex pow( complex a,complex b )                             // z=a^b
{ if( null(a) )
  { if( null(b) )return (complex){ 1.0,0.0 };
                 else return (complex){ 0.0,0.0 };
  }
  else if( real(b)==1.0 && imag(b)==0.0 )return a;
  else if( real(b)==-1.0 && imag(b)==0.0 )return inverse( a ); // else
  { double logr=logabs(a),theta=arg(a);
    return polar( exp( logr*real(b)-imag(b)*theta ),theta*real(b)+imag(b)*logr );
  }
}
complex pow_real( complex a,double b )                          // z=a^b
{ if( null(a) ){ return (complex){ b==0.0?1.0:0.0,0.0 }; } else
  { return polar( exp(logabs(a)*b),arg(a)*b ); }
}
complex log(complex a){ return (complex){ logabs(a),arg(a) }; }  // z=log(a)
complex log10(complex a){ return mul_real( log(a),M_LOG10E ); }  // 1/log(10)
complex log_b(complex a,complex b){ return div( log(a),log(b) ); }
/*
 * Complex trigonometric functions
 */
complex sin( complex a )                                 // z = sin(a)
{ double R = real(a), I = imag(a);
  if( I==0.0 ){ return (complex){ sin(R),0.0 }; } else
  { return (complex){ sin(R)*cosh(I),cos(R)*sinh(I) }; }
}
complex cos( complex a )                                 // z = cos(a)
{ double R = real(a), I = imag(a);
  if( I==0.0 ){ return (complex){ cos( R ),0.0 }; } else
  { return (complex){ cos(R)*cosh(I),sin(R)*sinh(-I) }; }
}
complex tan( complex a )                                 // z = tan(a)
{ const double &R=real(a),&I=imag(a);
  double D=sqr( cos(R) )+sqr( sinh(I) );
  if( fabs(I)<1.0 )
  { return (complex){ 0.5*sin(2*R)/D, 0.5*sinh(2*I)/D }; } else
  { double F=1.0+sqr(cos(R)/sinh(I));
    return (complex){ 0.5*sin(2*R)/D,1.0/(tanh(I)*F) };
} }
complex sec( complex a ){ return inverse( cos( a ) ); } // z = sec(a)
complex csc( complex a ){ return inverse( sin( a ) ); } // z = csc(a)
complex cot( complex a ){ return inverse( tan( a ) ); } // z = cot(a)

/*
 * Inverse Complex Trigonometric Functions
 */
complex arcsin( complex a )                          // z = arcsin(a)
{ const double &R=real(a),&I=imag(a);
  if( I==0.0 )return arcsin_real(R); else
  { double x=fabs(R),y=fabs(I);
    double r=hypot(x+1,y),s=hypot(x-1,y),A=0.5*(r+s),B=x/A,y2=y*y;
    double re,im;
    const double A_crossover=1.5,B_crossover=0.6417;
    if( B<=B_crossover )re=asin( B ); else
    { if( x<=1 )re=atan( x/sqrt(0.5*(A+x)*(y2/(r+x+1)+(s+(1-x)))) );
           else re=atan( x/(y*sqrt(0.5*((A+x)/(r+x+1)+(A+x)/(s+x-1))) ) );
    }
    if( A<=A_crossover )
    { double Am1;
      if( x<1 )Am1=0.5*(y2/(r+(x+1)) + y2/(s+(1-x)));
          else Am1=0.5*(y2/(r+(x+1)) + (s+(x-1)));
      im = log1p( Am1+sqrt( Am1*(A+1) ) );
    } else { im = log( A+sqrt( A*A-1 ) ); }
    return (complex){ (R>=0) ? re:-re, (I>=0) ? im:-im };
} }
complex arcsin_real (double a)                          // z = arcsin(a)
{ if (fabs (a) <= 1.0){ return (complex){ asin (a), 0.0 }; } else
  { if( a<0.0 ){ return (complex){ -M_PI_2, acosh (-a) }; } else
               { return (complex){ M_PI_2, -acosh (a) }; }
} }
complex arccos (complex a)                              // z = arccos(a)
{ const double &R=real(a),&I=imag(a);
  if( I==0 ){ return arccos_real(R); } else
  { double x=fabs(R),y=fabs(I),r=hypot(x+1,y),s=hypot(x-1,y);
    double A=0.5*(r+s),B=x/A,y2=y*y,re,im;
    const double A_crossover = 1.5, B_crossover = 0.6417;
    if (B <= B_crossover){ re = acos (B); } else
    { if( x<=1 )re = atan( sqrt(0.5*(A+x)*(y2/(r+x+1)+(s+(1-x))))/x );
           else re = atan( (y*sqrt(0.5*((A+x)/(r+x+1)+(A+x)/(s+(x-1)))))/x);
    }
    if( A<=A_crossover )
    { double Am1;
      if( x<1 ){ Am1 = 0.5*(y2/(r+(x+1))+y2/(s+(1-x))); }
          else { Am1 = 0.5*(y2/(r+(x+1))+(s+(x-1))); }
        im = log1p( Am1+sqrt(Am1*(A+1)) );
    } else { im = log( A+sqrt(A*A-1) ); }
    return (complex){ (R>=0) ? re:M_PI-re, (I>=0) ? -im:im };
} }
complex arccos_real( double a )                         // z = arccos(a)
{ if( fabs(a)<=1.0 ){ return (complex){ acos(a),0 }; } else
  { if( a<0.0 ){ return (complex){ M_PI,-acosh(-a) }; } else
               { return (complex){ 0, acosh(a) }; }
} }
complex arctan (complex a)                              // z = arctan(a)
{ const double &R=real(a),&I=imag(a); complex z;
  if( I==0 ){ z=(complex){ atan(R),0 }; } else
  { double r=hypot(R,I),im;
    double u=2*I/(1+r*r);
    if( fabs(u)<0.1 )im=0.25*(log1p(u)-log1p(-u));
                else im=0.5*log(hypot(R,I+1)/hypot(R,I-1));
    if( R==0 )
    { if( I>1 ){ z=(complex){ M_PI_2,im }; } else
      if( I<-1){ z=(complex){-M_PI_2,im }; } else
               { z=(complex){ 0,im }; };
    } else { z=(complex){ 0.5*atan2(2*R,((1+r)*(1-r))),im }; }
  } return z;
}
complex arcsec(complex a){ return arccos (inverse (a)); } // z = arcsec(a)
complex arcsec_real( double a )                           // z = arcsec(a)
{ if( a<=-1.0 || a>=1.0 )return (complex){ acos(1/a),0.0 }; else
  { if( a>=0.0 ){ return (complex){ 0,acosh(1/a) }; }
           else { return (complex){ M_PI,-acosh(-1/a) }; }
} }
complex arccsc (complex a){ return arcsin(inverse(a)); } // z = arccsc(a)
complex arccsc_real( double a )                          // z = arccsc(a)
{ if( a<=-1.0 || a>=1.0){ return (complex){ asin(1/a),0.0 }; } else
  { if( a>=0.0 ){ return (complex){ M_PI_2,-acosh(1/a) }; }
           else { return (complex){-M_PI_2,acosh(-1/a) }; }
} }
complex arccot (complex a)                               // z = arccot(a)
{ if( null(a) ){ return (complex){ M_PI_2,0 }; }
          else { return arctan (inverse(a)); }
}
/*
 * Complex Hyperbolic Functions
 */
complex sinh (complex a)                                // z = sinh(a)
{ const double &R=real(a),&I=imag(a);
  return (complex){ sinh(R)*cos(I),cosh(R)*sin(I) };
}
complex cosh (complex a)                                // z = cosh(a)
{ const double &R=real(a),&I=imag(a);
  return (complex){ cosh(R)*cos(I),sinh(R)*sin(I) };
}
complex tanh (complex a)                                // z = tanh(a)
{ const double &R=real(a),&I=imag(a);
  double D=sqr(cos(I))+sqr(sinh(R));
  if( fabs(R)<1.0 )return (complex){ sinh(R)*cosh(R)/D,0.5*sin(2*I)/D }; else
  return (complex){ 1.0/(tanh(R)*(1.0+sqr(cos(I)/sinh(R)))),0.5*sin(2*I)/D };
}
complex sech( complex a ){ return inverse(cosh(a)); }   // z = sech(a)
complex csch( complex a ){ return inverse(sinh(a)); }   // z = csch(a)
complex coth( complex a ){ return inverse(tanh(a)); }   // z = coth(a)
/*
 * Inverse Complex Hyperbolic Functions
 */
complex arcsinh (complex a)                                 // z = arcsinh(a)
{ return mul_imag(arcsin(mul_imag(a,1.0)),-1.0);
}
complex arccosh (complex a)                                 // z = arccosh(a)
{ complex z = arccos(a); return mul_imag( z,imag(z)>0 ? -1.0:1.0 );
}
complex arccosh_real (double a)                             // z = arccosh(a)
{ if( a>=1 ){ return (complex){ acosh (a), 0 }; } else
  { if( a>=-1.0){ return (complex){ 0, acos (a) }; }
           else { return (complex){ acosh (-a), M_PI }; }
} }
complex arctanh (complex a)                                 // z = arctanh(a)
{ if( imag(a)==0.0 ){ return arctanh_real( real(a) ); }
  else { return mul_imag( arctan( mul_imag(a,1.0)),-1.0 ); }
}
complex arctanh_real (double a)                             // z = arctanh(a)
{ if (a > -1.0 && a < 1.0){ return (complex){ atanh(a),0 }; }
  else { return (complex){ atanh(1.0/a),(a<0) ? M_PI_2:-M_PI_2 }; }
}
complex arcsech( complex a ){ return arccosh(inverse(a)); } // z = arcsech(a)
complex arccsch( complex a ){ return arcsinh(inverse(a)); } // z = arccsch(a)
complex arccoth( complex a ){ return arctanh(inverse(a)); } // z = arccoth(a)
