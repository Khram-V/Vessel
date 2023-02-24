CC  SWHET.FOR - Single precision Whetstone program.
CC              Measures FORTRAN and CPU performance in
CC              Whetstone-instructions per second.
CC
CC
CC              References on Whetstones:
CC
CC                        -  Computer Journal Feb 76
CC                           pg 43-49 vol 19 no 1.
CC                           Curnow and Wichmann.
CC
CC                        -  Timing Studies Using a
CC                           Synthetic Whetstone Benchmark,
CC                           S. Harbaugh & J. Forakris
CC
CC              References on FORTRAN Benchmarks:
CC
CC                        -  Computer Languages, Jan 1986
CC                        -  EDN, Oct 3, 1985, Array Processors
CC                           for PCs
CC                        -  Byte, Feb 1984.
CC

      INTEGER*4   j, k, l, i, isave
      INTEGER*4   n2, n3, n4, n6, n7, n8, n9, n11
      INTEGER*4   inner, outer, kount, npass, max_pass

      REAL*4      x, y, z, t1, t2, t3, e1(4)
      REAL*8      whet_save, dif_save, kilowhet
      REAL*8      begin_time, end_time, dif_time
      REAL*8      error, whet_err, percent_err
      REAL*8      secndwall

      COMMON      t1, t2, t3, e1, j, k, l


C
C     Initialize pass-control variables.  SWHET must make at least
C     two passes to calculate sensitivity terms.  NPASS counts passes;
C     MAX_PASS is the maximum number of passes.  Currently the program
C     is written such that MAX_PASS should have a value of 2.  For higher
C     values of MAX_PASS, modifications to the program are required.
C
      npass    =    0
      max_pass =    2

      WRITE (*,9000 )
      READ  (*,*) inner
      WRITE (*,9100 )
      READ  (*,*) outer

      DO WHILE( npass .LT. max_pass)
         WRITE (*,9200) npass + 1, outer, inner
         WRITE (*,*) ('=', j = 1, 60)
         kount      = 0
         begin_time = secndwall()
C
C        Begininning of timed interval
C
         DO WHILE( kount .LT. outer )
C
C           Whetstone code begins here.  First initialize variables
C           and loop counters based on the number of inner loops.
C
C           Loops 2 and 3 (described below) use variations of the
C           following transformation statements:
C
c                  x1 = ( x1 + x2 + x3 - x4) * 0.5
c                  x2 = ( x1 + x2 - x3 + x4) * 0.5
c                  x3 = ( x1 - x2 + x3 + x4) * 0.5
c                  x4 = (-x1 + x2 + x3 + x4) * 0.5
C
C           Theoretically this set tends to the solution
C
c                  x1 = x2 = x3 = x4 = 1.0
C
C           The variables t1, t2, and t3 are terms designed to limit
C           convergence of the set.
C
            t1 = 0.499975
            t2 = 0.50025
            t3 = 2.0
C
C           The variables n2-n11 are counters for Loops 2-11.
C           Based on earlier statistical work (Wichmann, 1970),
C           loops 1, 5, and 10 are omitted from the program.
C
            isave = inner
            n1    = 10  * inner
            n2    = 12  * inner
            n3    = 14  * inner
            n4    = 345 * inner
            n6    = 210 * inner
            n7    = 32  * inner
            n8    = 899 * inner
            n9    = 616 * inner
            n10   = 512 * inner
            n11   = 93  * inner
C
C           The values in array e1 are arbitrary.
C
            e1(1) =  1.0
            e1(2) = -1.0
            e1(3) = -1.0
            e1(4) = -1.0
C
C           Loop 1 - Convergence test using real numbers.  The
C           execution of this loop was found to be statistically
C           invalid, but is included here for completeness.
C
!$omp do
            DO i = 1, n1
               x1 = ( x1 + x2 + x3 - x4) * t1
               x2 = ( x1 + x2 - x3 + x4) * t1
               x3 = ( x1 - x2 + x3 + x4) * t1
               x4 = (-x1 + x2 + x3 + x4) * t1
            END DO
!$omp end do
C
C           Loop 2 - Convergence test using array elements.
C
            DO i = 1, n2
               e1(1) = ( e1(1) + e1(2) + e1(3) - e1(4)) * t1
               e1(2) = ( e1(1) + e1(2) - e1(3) + e1(4)) * t1
               e1(3) = ( e1(1) - e1(2) + e1(3) + e1(4)) * t1
               e1(4) = (-e1(1) + e1(2) + e1(3) + e1(4)) * t1
            END DO

C
C           Loop 3 - Convergence test using subroutine calls.
C
            DO i = 1, n3
               CALL sub1( e1 )
            END DO
C
C           Loop 4 - Conditional jumps.  Repeated iterations
C           alternate the value of j between 0 and 1.
C
            j = 1
            DO i = 1, n4
               IF( j - 1 ) 20, 10, 20
   10          j = 2
               GOTO 30
   20          j = 3
   30          IF( j - 2 ) 50, 50, 40
   40          j = 0
               GOTO 60
   50          j = 1
   60          IF( j - 1 ) 70, 80, 80
   70          j = 1
               GOTO 90
   80          j = 0
   90          CONTINUE
            END DO
C
C           Loop 6 - Integer arithmetic and array addressing.
C           The values of integers j, k, and l remain unchanged
C           through iterations of loop.
C
            j = 1
            k = 2
            l = 3
            DO i = 1, n6
               j = j * (k - j) * (l - k)
               k = l * k - (l - j) * k
               l = (l - k) * (k + j)
               e1(l - 1) = j + k + l
               e1(k - 1) = j * k * l
            END DO
C
C           Loop 7 - Trigonometric functions.  The following loop
C           almost transforms x and y into themselves and produces
C           results that slowly vary.  (The value of t1 ensures
C           slow convergence, as described above.)
C
            x = 0.5
            y = 0.5
            DO i = 1, n7
               x = t1 * ATAN( t3 * SIN( x ) * COS( x ) /
     +             (COS( x + y ) + COS( x - y ) - 1.0) )
               y = t1 * ATAN( t3 * SIN( y ) * COS( y ) /
     +             (COS( x + y ) + COS( x - y ) - 1.0) )
            END DO

C
C           Loop 8 - Subroutine calls.  Values of x, y, and z
C           are arbitrary.
C
            x = 1.0
            y = 1.0
            z = 1.0
            DO i = 1, n8
               CALL sub2( x, y, z )
            END DO

C
C           Loop 9 - Array references and subroutine calls.
C
            j = 1
            k = 2
            l = 3
            e1(1) = 1.0
            e1(2) = 2.0
            e1(3) = 3.0
            DO i = 1, n9
               CALL sub3
            END DO

C
C           Loop 10 - Simple integer arithmetic.  The execution
C           of this loop was found to be statistically invalid,
C           but is included here for completeness.
C
            j = 2
            k = 3
            DO i = 1, n10
               j = j + k
               k = j + k
               j = j - k
               k = k - j - j
            END DO
C
C           Loop 11 - Standard functions SQRT, EXP, and ALOG.
C
            x = 0.75
            DO i = 1, n11
               x = SQRT( EXP( ALOG( x ) / t2 ) )
            END DO
C
C           End of Whetstone code.
C
            inner = isave
            kount = kount + 1
         END DO
C
C        End of timed interval
C
         end_time = secndwall()
         dif_time = end_time - begin_time
C
C        1000 whetstones (kilowhetstones) = 100 * loops per second
C
         kilowhet = 100.0D+00 * DBLE( outer * inner ) / dif_time

         WRITE (*,9300) dif_time, kilowhet/1000.0
C
C        Repeat with inner count doubled.
C
         npass = npass + 1
         IF( npass .LT. max_pass ) THEN
            dif_save  = dif_time
            whet_save = kilowhet
            inner     = inner * max_pass
         ENDIF
      END DO
C
C     Compute sensitivity.
C
      error       =   dif_time - (dif_save * max_pass )
      whet_err    =   whet_save - kilowhet
      percent_err =   whet_err * 100.0D+00 / kilowhet
      WRITE (*,*)
      WRITE (*,*)
      WRITE (*,*) ('=', j = 1, 60)
      WRITE (*,9400) error, whet_err / 1000.0, percent_err
      IF( dif_time .LT. 10.0D00 )
     +   WRITE (*,*) 'TIME is less than 10 seconds -- ',
     +               'suggest larger inner loop'

 9000 FORMAT( ' Number of inner loops (suggest more than 3):  ' $ )
 9100 FORMAT( ' Number of outer loops (suggest more than 1):  ' $ )
 9200 FORMAT( //' Pass #', I3.2, ': ', I10, ' outer loop(s),', I10,
     +           ' inner loop(s)' )
 9300 FORMAT( ' Elapsed time =', F12.2, ' seconds' /
     +        ' Whetstones   =', F12.2,
     +        ' single-precision megawhets/second' )
 9400 FORMAT( ' Time error   =', F12.2, ' seconds' /
     +        ' Whet error   =', F12.2, ' megawhets/sec' /
     +        ' %    error   =', F12.2, ' % whet error' )

      END
C
C     Subroutines for arithmetic, array assignments
C
      SUBROUTINE sub1( e )
      REAL*4 t1, t2, t3, e(4)
      COMMON t1, t2, t3
      DO i = 1, 6
         e(1) = (e(1)  + e(2) + e(3) - e(4)) * t1
         e(2) = (e(1)  + e(2) - e(3) + e(4)) * t1
         e(3) = (e(1)  - e(2) + e(3) + e(4)) * t1
         e(4) = (-e(1) + e(2) + e(3) + e(4)) / t3
      END DO
      RETURN
      END

      SUBROUTINE sub2( x, y, z )
      REAL*4 t1, t2, t3, x1, y1, x, y, z
      COMMON t1, t2, t3
      x1 = x
      y1 = y
      x1 = (x1 + y1) * t1
      y1 = (x1 + y1) * t1
      z  = (x1 + y1) / t3
      RETURN
      END

      SUBROUTINE sub3
      REAL*4 t1, t2, t3, e1(4)
      COMMON t1, t2, t3, e1, j, k, l
      e1(j) = e1(k)
      e1(k) = e1(l)
      e1(l) = e1(j)
      RETURN
      END

CC  SECNDS - Calls GETTIM function to find current time.
CC
CC  Return:  Number of seconds since midnight.
CC
      REAL*8 FUNCTION  secndwall()
c      INTEGER*2 hour, minute, second, hundredth
c      CALL GETTIM( hour, minute, second, hundredth )
c      secnds = ((DBLE( hour ) * 3600.0) + (DBLE( minute) * 60.0) +
c     +           DBLE( second) + (DBLE( hundredth ) / 100.0))
      REAL it(2)
        secndwall = etime( it )
      RETURN
      END
