//
//  Gregorian calendar was aborted on Oct.15.1582
//	Numerical Recipes in C. Cambridge University PRESS 1988.
//
#include <Math.h>
#include <Time.h>
#include "Type.h"

void julday( long julian, int& mm, int& id, int& iyyy )
{
 long ja,jalpha,jb,jc,jd,je;
  if( julian>=2299161L )
  {     jalpha=long( ( double( julian-1867216L )-0.25 )/36524.25 );
        ja=julian+1+jalpha-long( 0.25*jalpha );
  }else ja=julian;
        jb=ja+1524;
        jc=long( 6680.0+(double( jb-2439870L )-122.1)/365.25 );
        jd=long( 365*jc+double(0.25*jc) );
        je=long( double(jb-jd)/30.6001 );
  id  = int( jb-jd-int( 30.6001*je ) );
  mm  = int( je-1 );    if( mm>12  )mm-=12;
  iyyy= int( jc-4715 ); if( mm> 2  )--iyyy;  if( iyyy<=0 )--iyyy;
}
#define IGREG (15+31L*(10+12L*1582))

long julday( int mm, int id, int iyyy)
{ long jul;
  int ja,jy,jm;
   if( iyyy<0 ) ++iyyy;
   if( mm>2 ){ jy=iyyy;   jm=mm+1;  }
       else  { jy=iyyy-1; jm=mm+13; }
   jul=long( floor(365.25*jy)+floor(30.6001*jm)+id+1720995 );
   if( id+31L*(mm+12L*iyyy)>=IGREG ){ ja=int( 0.01*jy ); jul+=int( 2-ja+int(0.25*ja) ); }
  return jul;
}
#include <Windows.h>
///////////////////////////////////////////////////
static time_t iTime=time( NULL );                //
static struct tm *tBlock=localtime( &iTime );    // День и час текущего времени
static double StartTime = double( tBlock->tm_hour )
                      + ( double( tBlock->tm_min )
                      + ( double( tBlock->tm_sec )
                       -  double( GetTickCount() )/1000.0 )/60.0 )/60.0;
//!
//!  Прямое, не параллельное обращение к таймеру ДОС и Windows
//!  (расчеты в миллисекундах, опрокидывание через 49,7 суток)
//!
double onetime(){ return StartTime + double( GetTickCount() )/3600000.0; }
long julday()                                 // Функция возвращает сегодняшнее
{ tBlock=localtime( &(iTime=time( NULL )) );  // число по Юлианскому исчислению                 //
  return julday( int(tBlock->tm_mon+1),(int)tBlock->tm_mday,tBlock->tm_year+1900 );
}

#if 0
double onetime()
{ tBlock=localtime( &(iTime=time( NULL )) );    // Текущее время в часах
  return double( tBlock->tm_hour )
     + ( double( tBlock->tm_min ) + double( tBlock->tm_sec )/60.0 )/60.0;
}
Event& Event::Check()
{ if( T>=24 ){ D+=long( T/24 );   T =  fmod( T,24 ); } else
  if( T<0.0 ){ D-=long( T/-24)+1; T=24-fmod(-T,24 ); } return *this;
}
Event& Event::Now()
{ iTime=time( NULL );
  tBlock=localtime( &iTime );
    D = julday( tBlock->tm_mon+1,tBlock->tm_mday,tBlock->tm_year+1900 );
    T = double( tBlock->tm_hour )+( tBlock->tm_min + double( tBlock->tm_sec )/60.0 )/60.0;
  return *this;
}
#endif
