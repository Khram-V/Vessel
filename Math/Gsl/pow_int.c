#include <gsl_pow_int.h>

double gsl_pow_uint( double x,unsigned int n )
{ double value=1.0; // returns 0.0^0 = 1.0, so continuous in x
  do{ if( n&1 )value*=x; // repeated squaring method for n odd
          n >>= 1;
          x *= x;
    } while( n ); return value;
}
double gsl_pow_int( double x,int n )
{ if( n<0 )x=1.0/x,n=-n; return gsl_pow_uint( x,(unsigned)n );
}
