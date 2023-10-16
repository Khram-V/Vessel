#include <string.h>
#include "../Type.h"
//
//    Минимальная подборка текстовых операций со строчками неограниченной длины
//
string::string(){ s=(char*)calloc( 8,256 ); len=7*256; } // ≈1792 по началу ≡2K
string::~string(){ free(s); } // безусловная свобода, без сохранения строки для
char& string::operator[]( int k ) // c контролем размерности и удлинением в +2k
{ if( k>=len ){ s=(char*)realloc( s,(len=( k/256+7)*256 )+256 ); } // align=256
  return s[k];   // и только для ~ разовых операций без рекурсии и параллелизма
}
//  Грубое отсечение всех управляющих кодов и концевых пробелов в строке
//
int strcut( char* S )
{ int len=strlen( S ); S+=len; while(len>0 && *--S<=' ')--len,*S=0; return len;
}
//  Адрес имени файла без полного пути по директориям и с отсечением расширения
//
static char* _back( char* S, int l )
{ while( l>0 ){ if( S[-1]=='\\' || S[-1]=='/' || S[-1]==':' )break; --l; --S; }
  return S;
}
char* fname( const char *S )  // просто поиск адреса к собственному имени файла
{ int len=strlen( S ); return _back( (char*)S+len,len );
}
char* sname( char *S )          // фактически то же, но с отсечением расширения
{ int len=strlen( S );
  for( int i=len-1; i>=0; i-- )if( S[i]=='.' ){ S[len=i]=0; break; }
  return _back( S+len,len );
}
//    Принудительная замена расширения имени файла
//
char* fext( char* Name, const char* Ext )
{ char *S=sname( Name ); if( Ext )if( *Ext )
     { if( *Ext!='.' )strcat( S,"." ); strcat( S,Ext ); } return Name;
}
#if 0
char* fext( char* Name, const char* Ext )
{ if( Ext )if( *Ext )
    { char *S=sname( Name ); if( *Ext!='.' )strcat( S,"." );
                                            strcat( S,Ext ); } return Name;
}
inline char* memmove( char *A, char *B, size_t L )   //! странности из GCC-10.1
{ char *res=A; if( A<B )while( L-- > 0 )*A++=*B++; else
                 { A+=L; B+=L; while( L-- > 0 )*--A=*--B; } return res;
}
char* scpy( char* D, const char* _S, int n ) // n>0 итоговая строка с пробелами
{ if( !n )strcpy( D,_S ); else if( n<0 )strncpy( D,_S,-n ); // без нуля в конце
   else { int l=strlen( _S ); if( l<n )memcpy( D,_S,l ),memset( D+l,' ',n-l );
                                  else memcpy( D,_S,n ); } return D;
}
int strcut( char* S, int len )                   // Отсечение концевых пробелов
{ char *s=S+len;                                 // от наперед заданной позиции
  if( !len )while( *s>0 )++len,++s; else *s=0;   // концевой байт ( S+len+1 )=0
            while( len>0 && *--s<=' ' )*s=0,--len; return len;
}
#endif
