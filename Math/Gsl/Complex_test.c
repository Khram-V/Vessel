#include <Math.h>
#include <StdIO.h>
#include <String.h>
#include <Stdarg.h>
#include "complex_math.h"

#define GSL_DBL_EPSILON 2.2204460492503131e-16
#define GSL_FLT_EPSILON 1.1920928955078125e-07
#define TEST_FACTOR 100.0

static unsigned int tests = 0,
                  verbose = 0;
static
void gsl_test_rel( double result, double expected, double relative_error,
                   const char *test_description,... )
{ int status; //???
  va_list ap; va_start( ap,test_description );
              vprintf( test_description,ap ); va_end( ap );
    if( status==0 )
      { if( strlen( test_description )<45 )
             printf( " (%g observed vs %g expected)",result,expected  );
        else printf( " (%g obs vs %g exp)",result,expected );
      }
    else printf( " (%.18g observed vs %.18g expected)",result,expected );
    if( status==-1 )printf(" [test uses subnormal value]");
    if( status && !verbose )printf( " [%u]",tests );
    printf( "\n" );
    fflush( stdout );
}
int main(void)
{ const double tol = TEST_FACTOR * 10 * GSL_DBL_EPSILON;
  const double tolf= TEST_FACTOR * 10 * GSL_FLT_EPSILON;
  for( size_t i=0; i<10; i++ )
    { double r=(i-5.0)*0.3 ;
      double t=2.0*M_PI*i/5 ;
      double x=r*cos( t ),y=r*sin( t ) ;
      complex z=polar( r,t );
      gsl_test_rel( real(z),x,tol,"_polar real at (r=%g,t=%g)",r,t );
      gsl_test_rel( imag(z),y,tolf,"_polar imag at (r=%g,t=%g)",r,t );
      z=arctan( z );
    }
}
