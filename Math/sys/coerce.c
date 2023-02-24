#include <gsl_sys.h>

double gsl_coerce_double (const double x){ volatile double y; y = x; return y; }
float gsl_coerce_float (const float x){ volatile float y; y = x; return y; }

/* The following function is not needed, but is included for completeness */

long double gsl_coerce_long_double (const long double x){ volatile long double y; y = x; return y; }

