#include <gsl_math.h>

double gsl_ldexp( const double x, const int e )
{ int ex;
  if( x==0.0 )return x;
  { double y = gsl_frexp( x,&ex );
    double e2 = e + ex;
    if (e2 >= DBL_MAX_EXP)
      {	y *= pow( 2.0,e2-DBL_MAX_EXP+1 );
	e2 = DBL_MAX_EXP-1;
      }
    else if (e2 <= DBL_MIN_EXP)
      {	y *= pow( 2.0,e2-DBL_MIN_EXP-1 );
	e2 = DBL_MIN_EXP + 1;
      }
    return y * pow( 2.0,e2 );
  }
}
double gsl_frexp( const double x, int *e )
{ if( x==0.0 ){ *e = 0;  return 0.0; } else
  if( !gsl_finite( x ) ){ *e = 0; return x; } else // Handle the common case
  if( fabs( x )>=0.5 && fabs( x )<1 ){ *e=0; return x; } else
  { double ex = ceil( log( fabs( x ) )/M_LN2 ),f;
    int ei = (int) ex;         // Prevent underflow and overflow of 2**(-ei)
    if( ei<DBL_MIN_EXP )ei=DBL_MIN_EXP;
    if( ei>-DBL_MIN_EXP )ei=-DBL_MIN_EXP;
        f = x*pow( 2.0,-ei );                     // This should not happen
    if( !gsl_finite( f ) ){ *e=0; return f; }
    while( fabs( f )>=1.0 ){ ei++; f/=2.0;  }
    while( fabs( f )>0 && fabs( f )<0.5 ){ ei--; f*=2.0; }
    *e = ei;
    return f;
  }
}
