/*﻿
    Вариант раскодирования UTF-8
    для построения таблицы OEM-866(alt)                                  ©2019

         Code Contination Minimum Maximum
         0xxxxxxx       0       0     127
         10xxxxxx   error
         110xxxxx       1     128    2047
         1110xxxx       2    2048   65535 без 55296-57343
         11110xxx       3   65536 1114111
         11111xxx   error
*/
//#include <WChar.h>
#include <StdIO.h>
//#include <StdLib.h>
//#include <String.h>
//#include <WinNLS.h>
#include <Windows.h>
#include "../../Type.h"
//char* UtR( char* UTF );                  // UTF-8 -> Russian-OEM(DOS-866)alt
//char* RtU( char *OEM );                  // DOS-866(alt) -> строка UTF-8
//const char* CtU( unsigned int u );       // UniCode -> UTF-8    (int->string)
//      char* UtC( unsigned &u, char *s ); // UTF-8 -> UniCode, на выходе адрес
                                           //   со смещением до следующей буквы
/*
#define X( x )if( (U[x]&0xC0)==0x80 )  // контроль старших бит из других байтов
static int nS=0;                       // контрольное число байт в символах UTF
char* UtC( unsigned &u, char *U ) // синтез буквы Unicode из полноценного UTF-8
{ if( (u=*U)&0x80 )               //  со столь же излишним контролем кодировки
  { if((*U&0xE0)==0xC0){ X(1){ u=(*U++&0x1F)<<6; u|=*U&0x3F; nS=2; } } else
    if((*U&0xF0)==0xE0){ X(1)X(2){ u=(*U++&0xF)<<12; u|=(*U++&0x3F)<<6; u|=*U&0x3F; nS=3; } } else
    if((*U&0xF8)==0xF0){ X(1)X(2)X(3){ u=(*U++&7)<<18; u|=(*U++&0x3F)<<12; u|=(*U++&0x3F)<<6; u|=*U&0x3F; nS=4; } }
  } else nS=1; return ++U;
}
const char* CtU( unsigned u ) // раскодирование UniCode в текстовую строчку UTF
{ static char w[5];
  if(u<0x80){ w[0]=u; nS=1; } else
  if(u<0x0800){ w[0]=(u>>6)&0x1F|0xC0; w[1]=u&0x3F|0x80; nS=2; } else
  if(u<0x10000){ w[0]=(u>>12)&0xF|0xE0; w[1]=(u>>6)&0x3F|0x80; w[2]=u&0x3F|0x80; nS=3; } else
  if(u<0x110000){ w[0]=(u>>18)&0x7|0xF0; w[1]=(u>>12)&0x3F|0x80; w[2]=(u>>6)&0x3F|0x80; w[3]=u&0x3F|0x80; nS=4; }
  else{ w[0]=0xEF; w[1]=0xBF; w[2]=0xBD; nS=3; } w[nS]=0; return w;
}
*/
typedef union{ unsigned char c[5]; unsigned int d; } charint;
int main( int l, char **Av )
{ FILE *Fi,*Fo;
   SetConsoleCP( CP_UTF8 );
   SetConsoleOutputCP( CP_UTF8 );
  if( l<2 ){ printf( "\n? UtC <FileName>\n" ); return 1; }
  if( (Fi=fopen( Av[1],"rb" ))!=NULL )
  { printf( "UtC %s",Av[1] );
    if( (Fo=fopen( strcat( sname( Av[1] ),".utf" ),"wb" ))!=NULL )
    { printf( " --> %s",fname( Av[1] ) );           // невидимый признак
      fprintf( Fo,"%c%c%c...    UTF-8       │               ↔   UniCode "
                           " │                          ↔    Windows-16 ",
                   0xEF,0xBB,0xBF );                //! UTF-8 = Unicode
      for( l=0; !feof( Fi ); l++ )
      { unsigned char i,j=0,k=0,m=0; unsigned u; charint s; s.d=0; //s.c[4]=0;
        if( (i=getc( Fi ))&0x80 )
        {    j=getc( Fi );
          if( i&0x20 )
          {  k=getc( Fi );
            if( i&0x10 )
            {   m=getc( Fi );
                   fprintf( Fo,"\n\t0x%02X%02X%02X%02X │ \"%c%c%c%c\", → %02X%02X%02X%02X",i,j,k,m,i,j,k,m,s.c[3]=i,s.c[2]=j,s.c[1]=k,s.c[0]=m ); // s=<<24|j<<16|k<<8|m; //,v.d=((i&0x7)<<18)|(j&0x3F)<<12|(k&0x3F)<<6|m&0x3F;
            } else fprintf( Fo,  "\n\t0x%02X%02X%02X   │ \"%c%c%c\", → %02X%02X%02X",i,j,k,i,j,k,s.c[2]=i,s.c[1]=j,s.c[0]=k );                      // s=i<16|j<<8 |k;      //,v.d= (i&0xF)<<12 |(j&0x3F)<<6 | k&0x3F;
          }  else  fprintf( Fo,    "\n\t0x%02X%02X     │ \"%c%c\", →   %02X%02X",i,j,i,j,s.c[1]=i,s.c[0]=j );                                         // s=i<<8|j;            //,v.d= (i&0x1F)<<6 | j&0x3F;
        }   else { fprintf( Fo,      "\n\t0x%02X       │ \"",s.c[0]=i );
        if( i!=10 )fprintf( Fo,"%c",i ); else fwrite( &(u=0x9997E2),3,1,Fo );
                   fprintf( Fo,"\", →     %02X",i ); } //  'E29799'
       charint v={ i,j,k,m,0 };
       char cv[]={ i,j,k,m };
        UtC( u,cv );
      //UtC( u,(char[]){i,j,k,m} );   // выборка десятичного UniCode
        fprintf( Fo," ↔ %5u ← ",u );
        fwrite ( &(v.d),1,v.d<0x80?1 : v.d<0x10000?2 : v.d<0x1000000?3:4,Fo );
        fprintf( Fo," ↓ %s  + %8u = %s",CtU( u ),s.d,v.c );
        //
        // теперь двухбайтовые символы Unicode Windows/
        //
       wchar_t UniCode[4]={0,0,0,0};
       size_t n=MultiByteToWideChar( CP_UTF8,0,(char*)(v.c),-1,UniCode,1 );           fprintf( Fo," %5d ",UniCode[0] );
              n=WideCharToMultiByte( CP_UTF8,0,UniCode,-1,(char*)(v.c),1,NULL,NULL ); fprintf( Fo," = %s ",v.c );
        fprintf( Fo," Win => %5d",U2W( (char*)(v.c) )[0] );
//         mbtowc( UniCode,(char*)(s.c),4 );
      }
        fprintf( Fo,"\n\n lCount=%d\n",l ); fclose( Fo );
    }    printf( " = %d\n",l );
    ///
    //! UtR -> RtU - UTF-8 <=> OEM(866)alt без промежуточной кодировки в Unicode
    ///
    rewind( Fi ); //Av[1][strlen( Av[1] )-4]=0;
    printf( "UtR %s",Av[1] );
    if( (Fo=fopen( strcat( sname( Av[1] ),".uem" ),"wb" ))!=NULL )
    { printf( " --> %s",fname( Av[1] ) );                  // невидимый признак
      fprintf( Fo,"%c%c%c...\n",0xEF,0xBB,0xBF );          //! UTF-8 = Unicode
      for( l=0; !feof( Fi ); l++ )
      { char *s=getString( Fi );                  // строчка из буфера FileOpen
        fprintf( Fo,"src: '%s' \n",s );
        fprintf( Fo,"oem: '%s' \n",s=UtA( s,true ) );
        fprintf( Fo,"bak: '%s' \n\n",s=AtU( s,true ) );
      } fprintf( Fo,"\n\n lCount=%d\n",l ); fclose( Fo );
    }    printf( " = %d\n",l );                 return 0;
  }      printf( "\7\n?«%s» not found",Av[1] ); return -1;
}
/// יִﬞײַﬠﬡﬢﬣﬤﬥﬦﬧﬨ﬩שׁשׂשּׁשּׂאַאָאּבּגּדּהּוּזּטּיּךּכּלּמּנּסּףּפּצּקּרּשּתּוֹבֿכֿפֿﭏְֱֲֳִֵֶַָֹֺֻּֽ֑֖֛֢֣֤֥֤֦֧֪֚֭֮֒֓֔֕֗֘֙֜֝֞֟֠֡֨֩֫֬֯־ֿ׀ׁׂ׃ׅׄ׆ׇאבגדהוזחטיךכלםמןנסעףפץצקרשתװױײ׳״
/// ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩⅪⅫⅬⅭⅮⅯⅰⅱⅲⅳⅴⅵⅶⅷⅸⅹⅺⅻⅼⅽⅾⅿ∞∂∆∏∑¼½¾⅐⅑⅒⅓⅔⅕⅖⅗⅘⅙⅚⅛⅜⅝⅞⅟
/// ‰®§£¥»«÷³²°⁰ⁱ⁴⁵⁶⁷⁸⁹⁺⁻⁼⁽⁾ⁿ₀₁₂₃₄₅₆₇₈₉₊₋₌₍₎ₐₑₒₓₔₕₖₗₘₙₚₛₜ‘’‚‛“”„‟•‣․‥…‧—–‒‑‐
///
///    Объект string полностью заменяет неупорядоченность байт в char и char*
///     представляя тексты в строках string и символы целыми числами unsigned
//        index(&k) - поиск с остановкой по номеру символа, -1 - длина строки
//
#if 0
class string
{ char *S; int L;                 // адрес и реальная длина строки в байтах
  char* index( int k=-1 )       // расчет количества символов в формате UTF-8
  { char *s=S; int i=0,l=strlen( s );  // с возможностью выхода по счетчику k
    while( l>0 )
    { if( i==k )return s;
      if( (*s&0xE0)==0xC0 ){ s+=2; l-=2; } else
      if( (*s&0xF0)==0xE0 ){ s+=3; l-=3; } else
      if( (*s&0xF8)==0xF0 ){ s+=4; l-=4; } else { s++; l--; } i++;
    } len=i; return s;
  }
public: int len;
  string( int l=0 ){ L=max( l,16 ); (S=(char*)malloc( L ))[0]=0; len=0; }
 ~string(){ free( S ); }
  string& operator += ( const char* str )      // сложение строк с учетом UTF-8
  { if( (len=strlen( S )+strlen( str ))<=L )S=(char*)realloc( S,L=len+1 );
    strcat( S,str ); index(); return *this;
  }
  unsigned operator[]( int k )    // выборка конкретного упорядоченного символа
  { union{ char c[4]; int d; } v; char *s=index( k ); v.d=0;
    if(  *s<128         )v.c[0]=*s; else
    if( (*s&0xE0)==0xC0 )v.c[1]=*s,v.c[0]=s[1]; else
    if( (*s&0xF0)==0xE0 )v.c[2]=*s,v.c[1]=s[1],v.c[0]=s[2]; else
    if( (*s&0xF8)==0xF0 )v.c[3]=*s,v.c[2]=s[1],v.c[1]=s[2],v.c[0]=s[3];
    else v.d=-1; return v.d;
  }
  friend bool operator==( string &s,string &str ){ return strcmp( s.S,str.S )==0; }
              // strcmp(S,s) - сортирующие различие строк  S<s:<0 S==s:0 S>s:>0
};
#endif
