#ifndef __Type_h__
#ifndef __cplusplus
#error C++ compiler or compartible is neccessary
#endif
#define __Type_h__
//
//      Type mathematical                                            /85.08.16/
//
#include <StdDef.h>
#include "Point.h"

struct Event; typedef const Event& _Event; //
Real onetime();                            // Текущее время в часах от полуночи
void julday( long, int& m,int& d,int& y ); // Julian -> ( m,d,y )
long julday( int m,int d,int y );          // ( m,d,y ) -> Julian
long julday();                             // Настоящая юлианская дата
struct Event                               //
{ Real T;                                  // Время от полуночи в часах
  long D;                                  // Юлианская дата
  Event& Check()                           //
  { if( T>=24 ){ D+=long( T/24 );   T =  fmod( T,24 ); } else
    if( T<0.0 ){ D-=long( T/-24)+1; T=24-fmod(-T,24 ); } return *this;
  }
  Event& Now(){ D=julday(); T=onetime(); return *this; }
  Event(){ Now(); }
  Event( _Event New ): D( New.D ),T( New.T ){ Check(); }
  Event( long d, Real t=0.0 ): D( d ),T( t ){ Check(); }
//Event& operator = ( _Event E ){ D=E.D,T=E.T; return *this; }
  Event& operator += (  long d ){ D+=d; return *this;   }
  Event& operator -= (  long d ){ D-=d; return *this;   }
  Event& operator += ( _Real t ){ T+=t; return Check(); }
  Event& operator -= ( _Real t ){ T-=t; return Check(); }
//Event& operator += (_Event e ){ D+=e.D; T+=e.T; return Check(); }
//Event& operator -= (_Event e ){ D-=e.D; T-=e.T; return Check(); }
  long   Pack( int m, int d, int y  ){ return D=julday( m,d,y ); }
  void UnPack( int &m,int &d,int &y ){          julday( D,m,d,y ); }
  friend Real operator-( _Event A,_Event B )                     //
       { return Real( A.D-B.D )*24.0 + A.T-B.T;                  // t = A-B
       }                                                         // A = B+t
  friend Event operator +( _Event B,_Real t ){ return Event( B.D,B.T+t ); }
  friend bool operator > ( _Event A,_Event B ){ return A-B > 0.0; }
  friend bool operator >=( _Event A,_Event B ){ return A-B >=0.0; }
  friend bool operator < ( _Event A,_Event B ){ return A-B < 0.0; }
  friend bool operator <=( _Event A,_Event B ){ return A-B <=0.0; }
  friend bool operator ==( _Event A,_Event B ){ return A-B ==0.0; }
  friend bool operator !=( _Event A,_Event B ){ return A-B !=0.0; }
};
typedef unsigned short fixed;
typedef unsigned char  byte;
typedef          char* string;
typedef struct{ int Jx,Jy,Lx,Ly,wb; } field;    typedef const field& _field;
typedef struct{ Real Jx,Jy,Lx,Ly;   } Field;    typedef const Field& _Field;

enum Course
{ North_West=3, North=1,North_East=9,
        West=2,Center=0,      East=8,
  South_West=6, South=4,South_East=12, // Enter,Insert,Delete,BkSp,Tab,Esc=27,
       _Home=3,_Up   = 1,_PgUp =9,
       _Left=2,_Center=0,_Right=8,
       _End =6,_Down = 4,_PgDn=12, _Enter,_Ins,_Del,_BkSp,_Tab,_Esc=27,
  _MouseMoved,
  _MousePressed,
  _MouseReleased,_Space=' '
};
enum{ RIGHT=1,LEFT,SHIFT,CTRL=4,ALT=8,ScrollLock=16,NumLock=32,CapsLock=64,
                                                          InsertSwitch=128 };
size_t Allocate( void* A );                   // На выходе длина в байтах
void  *Allocate( size_t Sz, void *A=0 );     //
void **Allocate( size_t Ny, size_t Sz, void *A=0 );
                                           //
template <class real> real Angle( real ); // D°=±180°
char* DtoH( Real,int=0 );                // Исправление градусов (°) в часы (h)
char* DtoA( Real,int=0,char=0xF8 );     // (-)1°2'3"  h=0xFC
char* AtoD( char*,float& );            // (°)?(:)
char* AtoD( char*,Real& );             //
char* _Fmt( const char* fmt, ... );    // -- формирование статической строчки
char* scpy( char*,const char*,int=0 ); // n>0 - с концевыми пробелами, без нуля
char* scat( char*,const char*,int=0 ); // n=0 - n=-32767
char* fext( char*,const char* =0 );    // FileName Extensions.EXT
char* fname( const char* );            // Find the Name in PATH
char* sname( char* );                  // NAME without PATH @ EXT
 int strcut( char*,int=0 );            // Cleaning last blank symbols
byte    W2D( byte );                   // Windows->DOS
char*   W2D( const char* );            //   --//-- с одноразовым буфером
#if defined( min ) && defined( max )
#undef min
#undef max
#endif
template<class T>inline const T &max(const T &a, const T &b){ return a>b?a:b; }
template<class T>inline const T &min(const T &a, const T &b){ return a<b?a:b; }
template<class T>inline const T &minmax( const T &a, const T &b, const T &c )
                                       { return a>=b?a:( c<=b?c:b ); }
unsigned coreleft();
void Break( const char[],... );
void Error();
#endif
