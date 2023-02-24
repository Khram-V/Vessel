/*
    WHET.c - Обычной точности Whetstone проверка быстродействия.
             Measures CPU performance in Whetstone-instructions per second.
    References on Whetstones:
           - Computer Journal Feb 76 pg 43-49 vol 19 no 1. Curnow and Wichmann.
           - Timing Studies Using a Synthetic Whetstone Benchmark,
             S. Harbaugh & J. Forakris
    References on FORTRAN Benchmarks:
           - EDN, Oct 3, 1985, Array Processors for PCs
           - Computer Languages, Jan 1986
           - Byte, Feb 1984.
*/
#include <Math.h>
#include <StdIO.h>

typedef double real;
unsigned int j,k,l;
 real  x1,x2,x3,x4,
       t1,t2,t3,e1[4];
 real secnds();
 void sub1( real* );
 void sub2( real*,real*,real* );
 void sub3();

int main()
{
 unsigned i,isave,inner,outer,kount,npass,max_pass,n2,n3,n4,n6,n7,n8,n9,n10,n11;
 real x,y,z;
 real whet_save,dif_save,kilowhet,
      begin_time,end_time,dif_time=0.0,
      error,whet_err,percent_err;
//
//      Initialize pass-control variables.  SWHET must make at least
//      two passes to calculate sensitivity terms. NPASS counts passes;
//      MAX_PASS is the maximum number of passes. Currently the program
//      written such that MAX_PASS should have a value of 2. For higher
//      values of MAX_PASS, modifications to the program are required.
//
     npass = 0;
  max_pass = 2;

//! SetConsoleOutputCP( CP_UTF8 ); //=65001
   printf("Number of inner loops (suggest more than 3): "); scanf("%d",&inner);
   printf("Number of outer loops (suggest more than 1): "); scanf("%d",&outer);

      while( npass<max_pass )
      {
        printf("\n\n Pass #%3d: %5d outer loop(s),"
               "%5d inner loop(s)",npass+1,outer,inner );
//      printf("\nННННННННН << и это UTF-8 запись по русски >> ННННННННННННН");
//!     Beginning of timed interval

         kount      = 0;
         begin_time = secnds();
         while( kount<outer )
         {
/*          Whetstone code begins here.  First initialize variables
            and loop counters based on the number of inner loops.

            Loops 2 and 3 (described below) use variations of the
            following transformation statements:
              x1 = ( x1 + x2 + x3 - x4) * 0.5
              x2 = ( x1 + x2 - x3 + x4) * 0.5
              x3 = ( x1 - x2 + x3 + x4) * 0.5
              x4 = (-x1 + x2 + x3 + x4) * 0.5

            Theoretically this set tends to the solution
            x1 = x2 = x3 = x4 = 1.0
*/
//          The variables t1, t2, and t3 are terms designed to limit
//          convergence of the set.

            t1 = 0.499975;
            t2 = 0.50025;
            t3 = 2.0;

//          The variables n2-n11 are counters for Loops 2-11.
//          Based on earlier statistical work (Wichmann, 1970),
//          loops 1, 5, and 10 are omitted from the program.

            isave = inner;
            n2    = 12  * inner;
            n3    = 14  * inner;
            n4    = 345 * inner;
            n6    = 210 * inner;
            n7    = 32  * inner;
            n8    = 899 * inner;
            n9    = 616 * inner;
            n10   = 512 * inner;
            n11   = 93  * inner;

//          The values in array e1 are arbitrary.

            e1[0] =  1.0;
            e1[1] = -1.0;
            e1[2] = -1.0;
            e1[3] = -1.0;

//     Loop 1 - Convergence test using real numbers.  The
//            execution of this loop was found to be statistically
//            invalid, but is included here for completeness.

            for( i = 1; i<inner; i++ )
            {  x1 = ( x1 + x2 + x3 - x4 ) * t1;
               x2 = ( x1 + x2 - x3 + x4 ) * t1;
               x3 = ( x1 - x2 + x3 + x4 ) * t1;
               x4 = (-x1 + x2 + x3 + x4 ) * t1;
            }

//       Loop 2 - Convergence test using array elements.

            for( i=0; i<n2; i++ )
            {  e1[0] = ( e1[0] + e1[1] + e1[2] - e1[3]) * t1;
               e1[1] = ( e1[0] + e1[1] - e1[2] + e1[3]) * t1;
               e1[2] = ( e1[0] - e1[1] + e1[2] + e1[3]) * t1;
               e1[3] = (-e1[0] + e1[1] + e1[2] + e1[3]) * t1;
            }

//       Loop 3 - Convergence test using subroutine calls.

            for( i = 0; i<n3; i++ )sub1( e1 );

//     Loop 4 - Conditional jumps.  Repeated iterations
//          alternate the value of j between 0 and 1.

            j = 1;
            for( i = 0; i<n4; i++ )
            {  if( j!=1 )goto l20;
/*l10:*/       j = 2;
               goto l30;
  l20:         j = 3;
  l30:         if( j<=2 )goto l50;
/*l40:*/       j = 0;
               goto l60;
  l50:         j = 1;
  l60:         if( j>=1 )goto l80;
/*l70:*/       j = 1;
               continue;
  l80:         j = 0;
            }

//     Loop 6 - Integer arithmetic and array addressing.
//          The values of integers j, k, and l remain unchanged
//          through iterations of loop.

            j = 1;
            k = 2;
            l = 3;
            for( i = 0; i<n6; i++ )
            {  j = j * (k - j) * (l - k);
               k = l * k - (l - j) * k;
               l = (l - k) * (k + j);
               e1[l-2] = j + k + l;
               e1[k-2] = j * k * l;
            }
//
//     Loop 7 - Trigonometric functions.  The following loop
//          almost transforms x and y into themselves and produces
//          results that slowly vary.  (The value of t1 ensures
//          slow convergence, as described above.)
//
            x = 0.5;
            y = 0.5;
            for( i = 0; i<n7; i++ )
            {  x = t1 * atan( t3 * sin( x ) * cos( x ) /
                   (cos( x + y ) + cos( x - y ) - 1.0) );
               y = t1 * atan( t3 * sin( y ) * cos( y ) /
                   (cos( x + y ) + cos( x - y ) - 1.0) );
            }
//
//     Loop 8 - Subroutine calls. Values of x, y, and z are arbitrary.
//
            x = 1.0;
            y = 1.0;
            z = 1.0;
            for( i = 0; i<n8; i++ )sub2( &x,&y,&z );
//
//     Loop 9 - Array references and subroutine calls.
//
            j = 0;
            k = 1;
            l = 2;
            e1[0] = 1.0;
            e1[1] = 2.0;
            e1[2] = 3.0;
            for( i=0; i<n9; i++ )sub3();
//
//     Loop 10 - Simple integer arithmetic.  The execution of
//          this loop was found to be statistically invalid,
//          but is included here for completeness.
//
            j = 2;
            k = 34;
            for( i=1; i<=n10; i++ )
               j = j + k,
               k = j + k,
               j = j - k,
               k = k - j - j;
//
//     Loop 11 - Standard functions SQRT, EXP, and ALOG.
//
            x = 0.75;
            for( i = 0; i<n11; i++ )x = sqrt( exp( log( x ) / t2 ) );
//
//          End of Whetstone code.
//
            inner = isave;
            kount = kount + 1;
         }
//
//       End of timed interval
//
         end_time = secnds();
         dif_time = end_time-begin_time;
//
//       1000 whetstones (kilowhetstones) = 100 * loops per second
//
         kilowhet = 100.0 * (real)( outer*inner )/ dif_time;
         printf( "\nElapsed time =%8.2lf seconds"
                 "\nWhetstones   =%8.2lf double-precision"
                 " megaWhets/second",dif_time,kilowhet/1024.0 );
//
//       Repeat with inner count doubled.
//
         npass = npass+1;
         if( npass<max_pass )
         {  dif_save  = dif_time;
            whet_save = kilowhet;
            inner     = inner * max_pass;
         }
      }
//
//    Compute sensitivity.
//
      error       = dif_time - ( dif_save*max_pass );
      whet_err    = whet_save - kilowhet;
      percent_err = whet_err  / kilowhet;

  printf("\n\n Time error  =%8.2lf seconds"
           "\n Whet error  =%8.2lf megawhets/sec"
           "\n %%    error  =%8.2lf %% whet error\n\n\7",
                     error, whet_err/1024.0, percent_err*100 );
  if( dif_time+dif_save<10.0 )
  printf( "\nTIME(%1.0lf) is less than 10 seconds -- suggest larger inner loop",
       dif_time+dif_save );
  return 0;
}
//
//    Subroutines for arithmetic, array assignments
//
void sub1( real *e )
{ int i;
      for( i = 0; i<6; i++ )
      { e[1] = ( e[0]  + e[1] + e[2] - e[3] ) * t1;
        e[2] = ( e[0]  + e[1] - e[2] + e[3] ) * t1;
        e[3] = ( e[0]  - e[1] + e[2] + e[3] ) * t1;
        e[4] = ( -e[0] + e[1] + e[2] + e[3] ) / t3;
      }
}
void sub2( real *x, real *y, real *z )
{ real  x1, y1;
        x1 = *x;
        y1 = *y;
        x1 = (x1 + y1) * t1;
        y1 = (x1 + y1) * t1;
        *z = (x1 + y1) / t3;
}
void sub3()
{       e1[j] = e1[k];
        e1[k] = e1[l];
        e1[l] = e1[j];
}

#include <sys\timeb.h>

real secnds()
{ struct timeb t; ftime( &t ); return (real)t.time+(real)t.millitm/1024.0;
}
