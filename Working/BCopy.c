//
//	Процедура преобразования двоичных файлов
//      - вырезка одного сегмента
//      - перестановка 2/4/8/16 байт (в перспективе с перепаковкой float-чисел)
//
#include <StdIO.h>
#include <String.h>
#define Block 64000
int main( int Ac, char **Av )
{ if( Ac<3 )printf( "\n? BCopy <in-file> <out-file> [beg [leng]]\n" ); else
  while( Ac>1 )
  { FILE *Fi,*Fo;
    printf( "%s <> %s ",Av[0],Av[--Ac] );
//    if( (Fi=fopen( Av[Ac],"rb"  ))!=NULL
//    &&  (Fo=fopen( Av[Ac],"r+b" ))!=NULL )
//    { size_t Len;
//      char Buff[Block];
//      while( (Len=fread( Buff,1,Block,Fi ))>1 )
//      { swab( Buff,Buff,Len&0xFFFE );
//	fwrite( Buff,1,Len,Fo );
//	if( Len!=Block ) break;
//      }  printf( "OK\n" );
//    } else printf( "Shut" ); fclose( Fi ); fclose( Fo );
  }
}

char* memmove( char *A, char *B, int L )    //! странности из GCC-10.1
{ char *res=A; if( A>B )while( L-->0 )*A++=*B++; else
                 { A+=L; B+=L; while( L-->0 )*--A=*--B; } return res;
}
-fno-tree-loop-distribution 