/* sys/gsl_sys.h
 */
#ifndef __GSL_SYS_H__
#define __GSL_SYS_H__
#include <Math.h>

inline double gsl_log1p (const double x)
{ volatile double y=1+x, z=y-1; return log(y)-(z-x)/y; } // cancels errors with IEEE arithmetic
inline double gsl_fdiv (const double x,const double y ){ return x/y; }
double gsl_expm1 (const double x);
double gsl_hypot (const double x, const double y);
double gsl_hypot3(const double x, const double y, const double z);
double gsl_acosh (const double x);
double gsl_asinh (const double x);
double gsl_atanh (const double x);

int gsl_isnan (const double x);
int gsl_isinf (const double x);
int gsl_finite(const double x);

double gsl_nan (void);
double gsl_posinf (void);
double gsl_neginf (void);

double gsl_coerce_double (const double x);
float  gsl_coerce_float  (const float x);
long double gsl_coerce_long_double (const long double x);

double gsl_ldexp(const double x, const int e);
double gsl_frexp(const double x, int * e);

int gsl_fcmp (const double x1, const double x2, const double epsilon);

#endif /* __GSL_SYS_H__ */
