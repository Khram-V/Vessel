#include <gsl_sys.h>

int gsl_fcmp( const double x1, const double x2, const double epsilon )
{
  int exponent;
  double delta, difference; // Find exponent of largest absolute value
  { double max = (fabs (x1) > fabs (x2)) ? x1 : x2;
    frexp( max,&exponent );
  }
  delta = ldexp( epsilon,exponent ); // Form a neighborhood of size 2*delta
  difference = x1 - x2;
  if( difference>delta )return 1; else    // x1 > x2 
  if( difference<-delta  )return -1; else // x1 < x2 
  else              // -delta <= difference <= delta
        return 0;                       // x1 ~=~ x2 
}

