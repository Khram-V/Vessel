#ifndef __GSL_POW_INT_H__
#define __GSL_POW_INT_H__
inline double gsl_pow_2(const double &x) { return x*x;   }
inline double gsl_pow_3(const double &x) { return x*x*x; }
inline double gsl_pow_4(const double &x) { double x2=x*x;   return x2*x2;    }
inline double gsl_pow_5(const double &x) { double x2=x*x;   return x2*x2*x;  }
inline double gsl_pow_6(const double &x) { double x2=x*x;   return x2*x2*x2; }
inline double gsl_pow_7(const double &x) { double x3=x*x*x; return x3*x3*x;  }
inline double gsl_pow_8(const double &x) { double x2=x*x; x2=x2*x2; return x2*x2; }
inline double gsl_pow_9(const double &x) { double x3=x*x*x; return x3*x3*x3; }
       double gsl_pow_int( double x, int n );
       double gsl_pow_uint( double x, unsigned int n );
#endif
