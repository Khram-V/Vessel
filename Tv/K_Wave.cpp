//
//                                              /05.03/  85.04.02
//
//  Расчет волнового сопротивления тонкого судна по формулам Мичелла.
//
#include "Hull.h"
#define  _fmt "L=%.3lg, B=%.3lg, S=%lg, V=%lg, d=%.3lg, R=%.3lg, C=%.3lg\n"
                             //
Real WaveRes( Real **Hull,   // Корпус
              Real Froud,    // Число Фруда
              Real Length,   // Длина
              Real Depth,    // Осадка
              int Nx, int Nz // Индексы по длине и по осадке
            );               //
inline void line_x( int x1, int y1, int x2, int y2 )
{ int c=color( LIGHTGRAY );
  setlinestyle( DOTTED_LINE,0,NORM_WIDTH );
  setwritemode( XOR_PUT );  line( x1,y1,x2,y2 );
  setwritemode( COPY_PUT );
  setlinestyle( SOLID_LINE,0,NORM_WIDTH );
  color( c );
}                                //
void Hull_Wave( const int Type ) //
{                                //
 const int   Nr=50;              //
 const Real  mFn=1.0,            // Предельное число Фруда
             Ro=1.025,           // кг/дм.куб
              g=9.8106;          // м/сек.кв
 int   i,x,y,c=color( CYAN );    //
 Real  V,Rm=0,R[Nr],Fn,w=0,      //
       W,Cm=0,C[Nr],   v=0;      //
 Window WR;                      // Окно корпуса
 Plaze  PL( 32,63 );             // Рабочий корпус для ускоренных расчетов
        PL.build( Draught );     //
        V=PL.V/( Lwl*Draught*Bwl ); // Коэффициент общей полноты
  Ghelp();              //
  setactivepage( 1 );   // Работа с графическим изображением
  setvisualpage( 1 );   // кривых элементов теоретического чертежа,
  clear();              // статической остойчивости и масштабом Бонжана
//
//      Предварительная проработка изображения
//
 field f;      f.Jy=2; f.Ly=24; f.Jx=f.Lx=f.wb=0;
  WR.Set( f ); f.Jx=i=int( (Tv_port.bottom-Tv_port.top)*Breadth/(Draught-Do)-Tv.mX );
  WR.Set( f ); PL.drawlines();    //
                                  // Подготовка рабочего окна
  WR.F.Jx=mFn/Nr;      WR.F.Jy=0; //
  WR.F.Lx=mFn-WR.F.Jx; WR.F.Ly=2; // Экстремумы кривой сопротивления
  f.Jx=0; f.Lx=-2-i-Tv.mX;        // Окно предварительного рисования
  WR.Set( f );                    //
  WR.Focus();
  Tv_rect();
  Rm=0;
  for( w=0.1; w<mFn; w+=0.1 )line( w,0.0,w,0.125 );
  color( LIGHTCYAN ); w=0.5; line( w,0.0,w,0.25 );
  PL.DrawWaves();
                              //
  for( v=w=0,i=0; i<Nr; i++ ) // Расчет кривых волнового сопротивления
  { Fn=(i+1)*mFn/Nr;          //
    if( !Type )W=PL.Michell( Fn ); //Michell( PL.Y,Fn,Lwl,Draught,PL.Nx,PL.Nz );
         else  W=WaveRes( PL.Y,Fn,Lwl,Draught,PL.Nx,PL.Nz );
    R[i]=W/PL.V;
    C[i]=W*2/Lwl /Fn/Fn /Ro/g /PL.S;
    color( LIGHTGRAY );
    line( Fn,R[i],Fn-mFn/Nr,w );       w=R[i];
    line( Fn,0.0,Fn-mFn/Nr,0.0 ); if( Rm<R[i] )Rm=R[i];
    color( GREEN );
    line( Fn,C[i]*100,Fn-mFn/Nr,v );   v=C[i]*100;
    line( Fn,0.0,Fn-mFn/Nr,0.0 ); if( Cm<C[i] )Cm=C[i];
  }
  PL.DrawWaves();
  //
  // График удельного сопротивления
  //
  color( CYAN );
  WR.F.Ly=Rm;    Tv_bar();
  WR.Focus();    Tv_rect();
  for( w=0.1; w<mFn; w+=0.1 )
  { line( w,0.0,w,Rm/6 );
    Tv.Text( North_East,w+0.006,Rm/50,_Fmt( "%0.1lf",w ) );
  }
  color( LIGHTCYAN ); w=0.5; line( w,0.0,w,Rm/3 );
  color( WHITE );                         moveto( Real( 0.0 ),0.0 );
  for( i=0; i<Nr; i++ ){ Fn=(i+1)*mFn/Nr; lineto( Fn,R[i] ); }
  Tv.Text( South_East,mFn/36,Rm*0.88,_Fmt( "R/D=%.3lg",Rm ) );
  //
  // График коэффициента волнового сопротивления
  //
  WR.F.Ly=Cm;
  WR.Focus();
  color( LIGHTGREEN );                    moveto( Real( 0.0 ),0.0 );
  for( i=0; i<Nr; i++ ){ Fn=(i+1)*mFn/Nr; lineto( Fn,C[i] ); }
  Tv.Text( South_East,mFn/36,Cm*0.80,_Fmt( "Cw=%.3lg",Cm ) );
  color( YELLOW );
  Tv.Text( South_East,mFn/36,Cm*0.96,Type?"WaveRes":"Michell" );
 Course Ans;
 int h=Tv.Th;
       Tv.Th=10;
  for(;;)
  { Tv_GetCursor( x,y );
    if( WR.Is( x,y ) )
    { w=Tv_X( x );
      i=min( max( 0,int( Nr/mFn*w-0.5 )),Nr-1 );
    //Ghelp( "Fn[%d]=%.2lg, Rw/D=%.3lg, Cw=%.3lg",i,w,R[i],C[i] );
      Tgo( 0,0 );
      Tprintf( "Fn[%d]=%.2lg, Rw/D=%.3lg, Cw=%.3lg\n",i,w,R[i],C[i] );
      line_x( x,Tv_port.bottom,x,Tv_port.top ); Ans=Tv_getc();
      line_x( x,Tv_port.bottom,x,Tv_port.top );
      if( Ans==_MousePressed )PL.DrawWaves( w );
    } else
    { Ans=Tv_getc();
      if( i )
      { i=0; //Ghelp( "L=%.3lg, B=%.3lg, S=%lg, V=%lg, d=%.3lg, R=%.3lg, C=%.3gl. F2-запись",Lwl,Bwl,PL.S,PL.V,V,Rm,Cm );
        Tgo( 0,0 );
        Tprintf( _fmt,Lwl,Bwl,PL.S,PL.V,V,Rm,Cm );
    } }              //
    if( Ans==0xF2 ) // Запись результатов расчета волнового сопротивления
    { static int n=1;
      static char Nm[13]; FILE *Om; sprintf( Nm,"~WR_%04d.txt",n++ );
      if( (Om=fopen( Nm,"wt" ))!=NULL )
      { fprintf( Om,_fmt,Lwl,Bwl,PL.S,PL.V,V,Rm,Cm );
        for( i=0; i<Nr; i++ )
        fprintf( Om,"%6.3lg %.4lg %.4lg\n",(i+1)*mFn/Nr,R[i],C[i] );
        fclose( Om );
      }
    }
    if( Ans==_Esc )break;
  }
  Tv.Th=h;
  color( c );
  setactivepage( 0 );
  setvisualpage( 0 );
}
//
//      Прорисовка профиля корабельной волны
//
void Plaze::DrawWaves( Real Fn )
{
 int j,k,c=color( BLUE );
 field f={ 0,Tv_port.top-Tv.mY-3,0,0 };
 Field F={ 0,-Breadth/2,Nx-1,Breadth };
       f.Ly=int( Tv.mX*Breadth/Length )-Tv_port.top+3;
  Tv_place( 0,&F );
  Tv_place( &f );
  if( !(Tv_getk()&(SHIFT|CTRL)) )Tv_bar( 0,0,Tv.mX,Tv_y( 0 ) );

 static Real W=1.0;                    //
  if( Fn>0 )                           // Волновая линия
  { Real V;                            //
    Michell( Fn );
    if( !(Tv_getk()&(SHIFT|CTRL)) )
    { W=0.0;
      for( k=0; k<Nx; k++ )if( W<Wx[k] )W=Wx[k];
    }    V=Tv_Y( 0 );
    for( k=0; k<Nx; k++ )Wx[k]*=V/W;
    color( LIGHTCYAN );  moveto( Real( 0 ),Wx[0] );
    for( k=0; k<Nx; k++ )lineto( Real( k ),Wx[k] );
  } else                              //
  { Tv_bar();                         // Ватерлинии
    Tv_rect();                        //
    for( k=1; k<Nx; k++ )line( Real( k ),0.0,Real( k ),-Bwl/2 );
                                      color( GREEN );
    for( j=Nz-1; j>=0; j-- )
    { if( !j )color( YELLOW );
                           moveto( Real( 0 ), -Y[j][0] );
      for( k=1; k<Nx; k++ )lineto( Real( k ), -Y[j][k] );
    }                      line( Real( 0 ),0.0,Real( Nx-1 ),0.0 );
  }
  color( LIGHTBLUE );  moveto( Real( 0 ),Y[0][0] );
  for( k=1; k<Nx; k++ )lineto( Real( k ),Y[0][k] );
  color( c );
  Tv_place( &f );
  Tv_place( 0,&F );
}
//      Прорисовка расчетного корпуса
//        основанного на таблице плазовых ординат
//
void Plaze::drawlines()
{  int i,k,c;
 Field F;               F.Jx=Breadth/2,F.Jy=0,F.Lx=-Breadth,F.Ly=Nz;
  Tv_place( 0,&F );     //
  c=color( CYAN );      // Окно корпуса
  Tv_rect();            //
  color( LIGHTGRAY ); line( (Real)0,0.0,0.0,(Real)Nz );
  for( i=0; i<Nx; i++ )                         // Построение картинки
  { color( !(i%(Nx-1)) ? LIGHTRED:LIGHTGRAY );
    for( k=0; k<Nz; k++ )
    if( !k )moveto( i<Nx/2?Y[0][i]:-Y[0][i],0.0 );
       else lineto( i<Nx/2?Y[k][i]:-Y[k][i],Real( k ) );
  } color( c );
}
