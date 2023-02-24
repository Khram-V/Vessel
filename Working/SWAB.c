//
//      Простая программа копирования с перестановкой байт
//      - (в перспективе 4/8/16 байт и с перепаковкой float-чисел)
//
#include <StdIO.h>
#include <String.h>
#include <Windows.h>
#define Block 0xFFFE

int main( int Ac, char **Av )
{ size_t L; char B[Block];
  if( Ac<2 )printf( "\n? SWAB <FileName[s]>\n" ); else
  while( --Ac>0 )
  { FILE *Fi=fopen( Av[Ac],"rb" ),
         *Fo=fopen( Av[Ac],"r+b" );
         AnsiToOem( Av[Ac],Av[Ac] );
                    printf( "SWAB <> %s",Av[Ac] );
    if( !Fi || !Fo )printf( " -- off\n" ); else
    while( (L=fread( B,1,Block,Fi ))>1 )
    { swab( B,B,L&0xFFFE ); fwrite( B,1,L,Fo ); if( L!=Block )break;
    } printf( "\n" ); fclose( Fi ); fclose( Fo );
  }
}
#if 0
#include <Dir.h>
#include <Dos.h>           //! старенький ДОС, или третий сорт уже не берут
#include <StdLib.h>
void main( int Ac, char **Av )
{ int k,done; struct ffblk F;
  for( k=1; k<Ac; k++ ){
   char Path[MAXPATH],Drive[MAXDRIVE],Dir[MAXDIR],File[MAXFILE],Ext[MAXEXT];
    fnsplit( Av[k],Drive,Dir,File,Ext );
    printf( ">%d: %s\n",k,Av[k] );
    done=findfirst( Av[k],&F,0 );
    while( !done ){
      done=(F.ff_attrib&FA_RDONLY)==0;
      printf( "%12s %8ld Read %s ",F.ff_name,F.ff_fsize,
                                        done?"Write":"Only" );
      if( done ){
        FILE *Fi,*Fo;
        fnmerge( Path,Drive,Dir,"","" ); strcat( Path,F.ff_name );
        if( (Fi=fopen( Path,"rb"  ))!=NULL
        &&  (Fo=fopen( Path,"r+b" ))!=NULL ){
         int  Len;
         char Buff[Block];
          fseek( Fi,4,SEEK_SET );
          fseek( Fo,4,SEEK_SET );
          while( (Len=fread( Buff,1,Block,Fi ))>1 ){
            swab( Buff,Buff,Len&0xFFFE );
            fwrite( Buff,1,Len,Fo );
            if( Len!=Block ) break;
          }   printf( "OK" );
        }else printf( "Shut" ); fclose( Fi ); fclose( Fo );
      }       printf( "\n" );   done=findnext( &F );
  } }
}
#endif
