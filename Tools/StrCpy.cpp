#include <string.h>
#include "../Type.h"
static void dtoa( double v, char *b, int bufsize, int p );
//
//    Минимальная подборка текстовых операций со строчками неограниченной длины
//
string::string(){ s=(char*)calloc( 8,256 ); len=7*256; } // ≈1792 по началу ≡2K
string::~string(){ free(s); } // безусловная свобода, без сохранения строки для
char& string::operator[]( int k ) // c контролем размерности и удлинением в +2k
{ if( k>=len ){ s=(char*)realloc( s,(len=( k/256+7)*256 )+256 ); } // align=256
  return s[k];   // и только для ~ разовых операций без рекурсии и параллелизма
}
string& string::operator << ( const char *str  )
{ int i=strlen( s ),l=strlen( str ); operator[]( i+l )=0; memcpy( s+i,str,l );
  return *this;
}
string& string::operator << ( const int d )
{ int i=strlen( s );
  if( i+24>len )operator[]( i+32 )=0; itoa( d,s+i,10 ); return *this;
}/*
string& string::operator << ( const Real R )
{ int i=strlen( s );
  if( i>len-256 )operator[]( i+256 )=0; dtoa( R,s+i,9,4 ); return *this;
}
*  Контроль от -180° до +180°  ||  [-360:360]  =>  A -= floor( A/360.0 )*360.0
template <class real> real Angle( const real &A ){ return remainder( A,360 ); }
   {A=fmod(A,360);return A>180?A-360:(A<-180?A+360:A);} // контроль направления
template double Angle<double>( double );                // - лишние операции
template float Angle<float>( float );
*/
// двоичный поиск ближайшего левого индекса в поступательной последовательности
//                                                  с учетом знака его прироста
template <class real> int find( const real *A, const real &Ar, int N )
{ int k,i=0; bool d=A[--N]>A[0];
   if( d ){ if( Ar<=A[0] )return 0; else if( Ar>=A[N] )return N-1; }
     else { if( Ar>=A[0] )return 0; else if( Ar<=A[N] )return N-1; }
  while( N-i>1 ){ k=(N+i)/2; if( d ){ if( Ar<A[k] )N=k; else i=k; }
                               else { if( Ar>A[k] )N=k; else i=k; } } return i;
}
template int find<double>( const double *A, const double &Ar, int N );
template int find<float>( const float *A, const float &Ar, int N );
// extern "C" {
//  Грубое отсечение всех управляющих кодов и концевых пробелов в строке
//
int strcut( char* S )
{ int len=0;
  if( S ){ len=strlen( S ); S+=len; while( len>0 && *--S<=' ' )--len,*S=0; }
  return len;
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
//  Процедуры градусно-часовых преобразований           /1991.05.11-2015.01.25/
//                                             -- оценка округленного интервала
Real AxisStep( Real D )                        // для разметки осевых линий
{ Real iPart = floor( D=log10( D ) ); D=exp( (D-iPart)*M_LN10 );
  return pow( 10.0,iPart )*( D>6 ?2 : D>3 ?1 : D>1.5 ?0.5:0.2 );
}                                              // M_LN10=2.30258509299404568402
//
//   Формирование текстовой строки F=0:{-123°46'57"89} 1-deg, 2+min, 3+sec,-hnd
//
static char W[12][16]={{0}},w=0;                   // буфер-192 на дюжину чисел
static char *put10( char *s, int d )               // - составная рекурсия -
     { if( d>=10 )s=put10( s,d/10 ); *s++ =d%10+'0'; return s;
     }
#define put2hnd *s++=d/10+'0',*s++=d%10+'0';         // -- под сотки из цифирек
char *DtoA( Real D, int F, const char *c )
{ const Real rnd[]={ 0.499999999,8.333333333e-3,1.388888888e-4 };
  char *S=W[w],*s=S; int d,f,l=0; (++w)&=0x3;      // счётчик заполняемых полей
  if( (D=remainder( D,360.0 ))<0.0 ){ D=-D; *s++='-'; } else *s++=' '; // ±180°
  if( !F )F=-3; f=F;
  if( F>0 )D+=rnd[F-1]; else{ F=-F; D+=rnd[F-1]/100.0; } d=D; D-=d;
                     if( d||(!l&&F<2) ){ s=put10( s,d ); strcpy( s,c );
                                                s+=strlen( c ); ++l; }
  if( F>1 ){d=D*=60; if( d||(!l&&F<3)||(l&&f<0)){put2hnd *s++='\'';++l;} D-=d;}
  if( F>2 ){d=D*=60; if( d || f<0 || !l ){ put2hnd *s++='"';  ++l; } D-=d; }
  if( f<0 ){d=D*100; if( d )put2hnd } *s=0; return S;
}
char *RtoA( Real D,int f,int n )      // заполняемое поле и цифры после запятой
   { char *S=W[w]; (++w)&=0x3; dtoa( D,S,min(f,16),min(n,f-3) ); return S; }
char *RtoA( _Real R ){ return RtoA( R,16,6 ); }
//
//   Разбор строчки с градусами, со свертыванием с минутами и секундами в число
//
static char* getright( char *s, Real& D )
{ Real d=1.0; while( *s>='0' && *s<='9' )D+=(*s++ -'0')/(d*=10); return s;
}
static char* getleft( char *s, Real& D )
{ while( *s>='0' && *s<='9' ){ D*=10.0; D+= *s++ -'0'; }
  if( *s=='.' || *s==',' )s=getright( ++s,D ); return s;
}
char *AtoD( char *S, Real& _D )           // на выходе следующее свободное поле
{ Real D=0.0,m;      char *s,*rs=S;           // в OEM есть градусы ° как 248
  short n=0,sign=0; while( *rs && *rs<=' ' )++rs; // остальное как в Ascii-1251
    if( !(*rs) )return rs;                        // пустая строка пропускается
    if( *rs=='-' )sign=1;                         // знак отделяется от разбора
    if( *rs=='+' || sign )++rs;    unsigned c;
  Rept:  rs=getleft( s=rs,m=0.0 ); rs=UtC( c,rs ); // учетом кодировки UTF-8
    switch( c )
    { case 176:                          // 248 : = '°' = 0xF8:
      case ':' : if( n<1 ){ n|=1,D+=m;      goto Rept; } break;
      case '\'': if( n<2 ){ n|=2,D+=m/60;   goto Rept; } break;
      case '"' : if( n<4 ){ n|=4,D+=m/3600; goto Rept; } break;
      default  : if( !n )D=m; else          // повторное считывание
      { rs=getright( s,m=0.0 );             // - ничего не попишешь
        if( n&4 )D+=m/3600; else if( n&2 )D+=m/60; else D+=m;
    } } if( sign )D=-D; _D=D; return rs;
}
// dtoa() — компактная функция преобразования double → строка
// с защитой от переполнения (999...999) и без временных буферов
// Разработано при участии AI-ассистента, 2026
// v — число, b — буфер, bufsize — размер буфера < 19, p — знаков после точки

void dtoa( double v, char *b, int space, int p )
{ if( !b )return;    --space;                 // резервируется место для '\0'
  if( v<0 ){ *b++='-'; space--; v=-v; }       // минус сбрасывается смещением
  if( space<1 ){ *b=0; return; }              // минимум: "0\0"
  if( p<0 )p=0; if( p>15 )p=15;
 char *t=b;                                   // с поправкой на сдвинутый минус
 int i,need=0;
 unsigned long long f=1,s=1,ip,fp;
  for( i=0; i<space && v>=s; i++ )s*=10ULL; if( v>=s )v=s-1.0; // только девятки
  // { if( s<ULLONG_MAX/10ULL )s*=10ULL; else break; } if( v>=s )v=s-1.0; //<19
  if( p>=space-i )p=space-i-1; for( i=0; i<p; i++ )f*=10ULL; // множитель дроби
  s=(unsigned long long)( v*f+0.5 ),ip=s/( f?f:1 ),fp=( p?s%f:0 );
  if( !ip )*t++='0',need++; else    // Разворачиваем целую часть на месте
  { char *l=t,*r,p;                // Записываем целую часть в обратном порядке
    while( ip ){ *t++ ='0'+(ip%10); ip/=10; ++need; }
    for( r=t-1; l<r; l++,r-- ){ p=*l; *l=*r; *r=p; }
  }
  if( p && need<space ){ *t++='.'; need++;   // выход без точки и дробная часть
   unsigned long long div=f/10;
    for( i=0; i<p && space>need; i++ )
     { *t++='0'+(fp/(div?div:1)); fp%=(div?div:1); if(div)div/=10; need++; }
  }
  while( b[need-1]=='0' && need>1 )--need;
  if( b[need-1]=='.' && need>1 )--need; b[need]='\0';
}
//~~extern
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
