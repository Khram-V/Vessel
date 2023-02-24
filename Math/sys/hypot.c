#include <gsl_math.h>
#define GSL_MAX( a,b ) ( (a)>(b)?(a):(b) )
#define GSL_MIN( a,b ) ( (a)<(b)?(a):(b) )

double gsl_hypot (const double x, const double y)
{
  double xabs = fabs(x) ;
  double yabs = fabs(y) ;
  double min, max;

  /* Follow the optional behavior of the ISO C standard and return
     +Inf when any of the argument is +-Inf, even if the other is NaN.
     http://pubs.opengroup.org/onlinepubs/009695399/functions/hypot.html */

  if( gsl_isinf(x) || gsl_isinf(y) ){ return GSL_POSINF; }
  if( xabs<yabs ){ min=xabs; max=yabs; } 
	    else { min=yabs; max=xabs  }
  if( min==0.0 ){ return max; }
    { double u=min/max ; return max*sqrt( 1.0+u*u );
    }
}
double gsl_hypot3( const double x,const double y,const double z )
{ double xabs = fabs(x);
  double yabs = fabs(y);
  double zabs = fabs(z);
  double w = GSL_MAX( xabs,GSL_MAX( yabs,zabs ) );
  if( w==0.0 ){ return( 0.0 ); } else
    { double r=w*sqrt( (xabs/w)*(xabs/w) +
                       (yabs/w)*(yabs/w) +
                       (zabs/w)*(zabs/w) ); return r;
    }
}
