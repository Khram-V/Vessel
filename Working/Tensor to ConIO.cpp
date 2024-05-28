//
// Вспомогательные утилиты, не имеющие прямого отношения к сути решаемой задачи
//
#include <StdIO.h>
#include "..\Type.h"
#include "..\Math\Complex.h"
#include "..\Window\ConIO.h"

bool Power_root( complex*,int );  // Коэффициенты->корни полинома и его порядок
void Power_Vietta(complex*,int);  // Восстановление комплексного полинома
                                  //                              по его корням
void put( int x,int y, COLORS c, const char *Fmt, ... )
{ char str[MAX_PATH];
   va_list V; va_start( V,Fmt ); vsprintf( str,Fmt,V ); va_end( V );
   textcolor( c ),print( x,y,str ),clreol();
}
void put( int x,int y, Real *A ) // A=Tensor& T
{ textcolor( YELLOW );    print( x,y, "Tensor = %0.3lf",A[18] ); clreol();
  textcolor( LIGHTGRAY ); print( x,y+1," { %6.2lf %6.2lf %6.2lf } ",A[0],A[1],A[2] ); textcolor( CYAN ); printf( " { %6.2lf %6.2lf %6.2lf } ",A[9], A[12],A[15] ); clreol();
  textcolor( LIGHTGRAY ); print( x,y+2," { %6.2lf %6.2lf %6.2lf } ",A[3],A[4],A[5] ); textcolor( CYAN ); printf( " { %6.2lf %6.2lf %6.2lf } ",A[10],A[13],A[16] ); clreol();
  textcolor( LIGHTGRAY ); print( x,y+3," { %6.2lf %6.2lf %6.2lf } ",A[6],A[7],A[8] ); textcolor( CYAN ); printf( " { %6.2lf %6.2lf %6.2lf } ",A[11],A[14],A[17] ); clreol();
  Real B[10]={ 0,0, 0,0, 0,0, 0,0, 0,0 },C[10]={ 0,0, 0,0, 0,0, 0,0, 0,0 };
  for( int k=0; k<10; k++ )B[k]=C[k]=0;
   B[6]=-1;                                                          C[6]=-1.0;
   B[4]=A[0]+A[4]+A[8];                                              C[4]=A[9]+A[13]+A[17];
   B[2]=A[1]*A[3]-A[0]*A[4]+A[5]*A[7]-A[4]*A[8]+A[6]*A[2]-A[0]*A[8]; C[2]=A[12]*A[10]-A[9]*A[13]+A[15]*A[11]-A[9]*A[17]+A[16]*A[14]-A[13]*A[17];
   B[0]=A[18];                                                       C[0]=1.0/A[18];
  textcolor( WHITE );     print( x,y+5,"Inv(1) + I(%0.2lf) + II(%0.2lf) + III(%0.2lf)",B[4],B[2],B[0] ); clreol();
  textcolor( LIGHTCYAN ); print( x,y+6,"Inv(1) + I(%0.2lf) + II(%0.2lf) + III(%0.2lf)",C[4],C[2],C[0] ); clreol();
  Power_root  ( (complex*)B,3 ); textcolor( WHITE );     print( x,y+8,"Rot = {%0.2lf+%0.2lfi},{%0.2lf+%0.2lfi},{%0.2lf+%0.2lfi},{%0.2lf+%0.2lfi} + {%0.2lf+%0.2lfi}",B[0],B[1],B[2],B[3],B[4],B[5],B[6],B[7],B[8],B[9] ); clreol();
  Power_root  ( (complex*)C,3 ); textcolor( LIGHTCYAN ); print( x,y+9,"Rot = {%0.2lf+%0.2lfi},{%0.2lf+%0.2lfi},{%0.2lf+%0.2lfi},{%0.2lf+%0.2lfi} + {%0.2lf+%0.2lfi}",C[0],C[1],C[2],C[3],C[4],C[5],C[6],C[7],C[8],C[9] ); clreol();
  Power_Vietta( (complex*)B,3 ); textcolor( LIGHTGRAY ); print( x,y+11,"Inv = {%0.2lf+%0.2lfi},{%0.2lf+%0.2lfi},{%0.2lf+%0.2lfi},{%0.2lf+%0.2lfi} + {%0.2lf+%0.2lfi}",B[0],B[1],B[2],B[3],B[4],B[5],B[6],B[7],B[8],B[9] ); clreol();
  Power_Vietta( (complex*)C,3 ); textcolor( CYAN );      print( x,y+12,"Inv = {%0.2lf+%0.2lfi},{%0.2lf+%0.2lfi},{%0.2lf+%0.2lfi},{%0.2lf+%0.2lfi} + {%0.2lf+%0.2lfi}",C[0],C[1],C[2],C[3],C[4],C[5],C[6],C[7],C[8],C[9] ); clreol();
  textcolor( LIGHTGREEN ); print( x,y+14,"Windows-"
#ifdef _WIN64
  "64 \n" );
#else
  "32 \n" );
#endif
  textcolor( GREEN );
  printf(" short= %d\n int  = %d\n Real = %d\n long = %d <> size_t = %d\n long long = %d\n long Real = %d\n pointer   = %d",
    sizeof(short),sizeof(int),sizeof(Real),sizeof(long),sizeof(size_t),sizeof(long long),sizeof(long double),sizeof(int*) );
}
