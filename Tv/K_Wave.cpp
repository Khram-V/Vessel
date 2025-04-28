//
//                                              /05.03/  85.04.02
//
//  Расчет волнового сопротивления тонкого судна по формулам Мичелла.
//
#include "Hull.h"
#define  _fmt "L=%.3lg, B=%.3lg, S=%lg, V=%lg, d=%.3lg, R=%.3lg, C=%.3lg\n"
static
Real WaveRes( Real **Hull,   // Корпус
              Real Froud,    // Число Фруда
              Real Length,   // Длина
              Real Depth,    // Осадка
              int Nx, int Nz // Индексы по длине и по осадке
            );
static void line_x( int x1, int y1, int x2, int y2 )
{ int c=color( LIGHTGRAY );
  setlinestyle( DOTTED_LINE,0,NORM_WIDTH );
  setwritemode( XOR_PUT );  line( x1,y1,x2,y2 );
  setwritemode( COPY_PUT );
  setlinestyle( SOLID_LINE,0,NORM_WIDTH );
  color( c );
}
void Hull_Wave( const int Type )
{
 const int   Nr=50;
 const Real  mFn=1.0,            // Предельное число Фруда
             Ro=1.025,           // кг/дм.куб
              g=9.8106;          // м/сек.кв
 int   i,x,y,c=color( CYAN );
 Real  V,Rm=0,R[Nr],Fn,w=0,
       W,Cm=0,C[Nr],   v=0;
 Window WR;                      // Окно корпуса
 Plaze  PL( 32,63 );             // Рабочий корпус для ускоренных расчетов
        PL.build( Draught );
        V=PL.V/( Lwl*Draught*Bwl ); // Коэффициент общей полноты
  Ghelp();
  setactivepage( 1 ); // Работа с графическим изображением
  setvisualpage( 1 ); // кривых элементов теоретического чертежа,
  clear();            // статической остойчивости и масштабом Бонжана
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
 int h=Tv.Th; Tv.Th=12;
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
//                                                 /05.03/  85.04.02
//  Расчет волнового сопротивления тонкого судна по формулам Мичелла
//
const double g=9.8106, // Ускорение свободного падения
             ro=1.025; //  и плотность морской воды

static int Nx,Nz;
static double Tl,Fr, Hx, R; //Xl,Xu;

static void AtaBee( double *_Y, double &Ac, double &As )
{ double dX,X,Y;                                     //
   Ac=As=0; dX=2.0/double( Nx-1 );                   // dX=(Ac+2)/(As+Nx);
   for( int j=0; j<Nx; j++ )                         // J=NA+1 TO NS-1;
   { X=R*(dX*(Nx-1-j)-1.0); Y=_Y[j]; Ac+=cos( X )*Y; // [ -1.0 -:- +1.0 ]
                                     As+=sin( X )*Y; //
   } X=(1.0-cos( R*dX ))/( R*R*dX )/(Nz-1); Ac*=X;   // = 2*FFF/Nx/2;
                                            As*=X;   //
}                                       //
static double **Y;                      // Матрица ординат корпуса
                                        //
static double AtaBlue( const double &Al )
{
 double C1,C2,Fw,Sumci=0,W=0,A,
        S1,S2,D2,Sumsi=0,P; Fw=Fr*Fr; R=Al/Fw/2; A=Al*Al; P=Tl*A/Fw/(Nz-1);
  AtaBee( Y[Nz-1],C1,S1 );      //
  for( int k=Nz-2; k>=0; k-- )  // I=KU+1 TO KD
  { W+=P;                       //
    if( W>64 )D2=0; else { D2=exp( -W ); AtaBee( Y[k],C2,S2 ); }
    W=D2*C2; Sumci += C1+W; C1=W;
    W=D2*S2; Sumsi += S1+W; S1=W;
  }
  Fw=Sumci*Sumci+Sumsi*Sumsi; return Hx<=0 ? Fw:A*Fw/sqrt( A-1 );
}
// Простое интегрирование аналитически определенной функции
//
static double QG4( const double &Xl, const double &Xu,
                   double F( const double& ) )
{ double A = 0.5*( Xu+Xl ),B=Xu-Xl,C,D;
         C = .4305681557970263 * B;
         D = .1739274225687269 * ( F( A+C )+F( A-C ) );
         C = .1699905217924281 * B;
  return B * (D+.3260725774312731 * ( F( A+C )+F( A-C )));
}                               //
static
Real WaveRes( Real **_Hull,     // Корпус
              Real Froud,       // Число Фруда
              Real Length,      // Длина
              Real Depth,       // Осадка
               int _Nx,         // Индексы по длине
               int _Nz )        //  и по осадке
{ double Wres=0,W,Xl,Xu;        //
   Tl=Depth/Length;
   Y=_Hull;  Nx=_Nx;
   Fr=Froud; Nz=_Nz;
   Hx=-1;
   Wres=AtaBlue( 1 )*0.0106 + AtaBlue( Xl=1.0001 )*0.00353;
   Hx=3.33*Fr*Fr;
   W=1.9-Fr*3;
   W=W*W+3;
   for( int i=0; i<3; i++,Hx*=3 )       // DO I=1,3;
   for( Real j=1.0; j<=W; j++  )        // DO J=1 TO 3+(1.9-FROUD*3)**2;
   {                 Xu=Xl+Hx;
     Wres += QG4( Xl,Xu,AtaBlue );
                  Xl=Xu;
   } return 2*Wres*ro*g*pow( Depth/Froud/Froud/Froud,2 )/Length/M_PI;
}
#if 0
    FROUD= V/SQRT(G*AL);                        TL=T/AL;
    WRES = WAVERES((FI),(FROUD),(TL));
    TL   = WRES*RO*G*T*T*B*B/FROUD**6/AL/PI/4;
    WRES = WRES*T*T*B*B/FROUD**8/AL**2/OMEGA/PI/2;
    PUT SKIP LIST('  FROUD='!!FROUD!!'  R/W='!!TL/DOUBLE!!
                '  R/S='!!TL/OMEGA!!' *** TIME='!!TIME!!' *** '!!L);
#endif
                        //
static double WL;       // Длина поперечной волны
                        //
double Plaze::Amplint( const double &La )
{   int i,j;                  //
 double X,Z,Wk=2.0*M_PI/La,   // Волновое число
            Mw=sqrt( WL/La ); // Число Маха
 Point A={ 0,0 },W;           // Интегральная амплитуда
  for( i=Nx-1; i>=0; i--  )   //
  { Z=0;
    W=0;
    X=Wk*dX*(Nx-1-i)/Mw;
    for( j=Nz-1; j>=0; j-- )
    { W += exp( (Point){ -Z,X } )*QV[j][i];
      Z += Wk*dZ;
    } A += W;
     Wx[i]=norm( A )*Mw/sqrt( Mw*Mw-1 )/La/La;
  } return norm( A )*Mw/sqrt( Mw*Mw-1 )/La/La;
}
double Plaze::QG4( const double &Xl, const double &Xu )
{ double A = 0.5*( Xu+Xl ),B=Xu-Xl,C,D;
         C = .4305681557970263 * B;
         D = .1739274225687269 * ( Amplint( A+C )+Amplint( A-C ) );
         C = .1699905217924281 * B;
  return B * (D+.3260725774312731 * ( Amplint( A+C )+Amplint( A-C )));
}
Real Plaze::Michell( Real Fn )
{   int i;      //
 double Rw=0.0, // Искомое волновое сопротивление
          Vo,   // Скорость в м/с
           l;   // Аргумент интегрирования
                //
  Vo = Fn*sqrt( g*Lwl );                //
  WL = 2.0*M_PI * Vo*Vo/g;              // Макс. длина волны
#if 0                                   //
  for( j=0; j<Nz;   j++ )
  for( i=0; i<Nx-1; i++ )               //
  { QV[j][i]=( Y[j][i+1]-Y[j][i] )*dZ;  // Распределеные источники
//  QV[j][i] = dX*dZ*X; //*Wide;        // с коэффициентом наклона
//  YY[j][i] =( Y[j][i]+Y[j][i+1] )/2.0;
//  if( j>Nz ) //>0 )
//  { Z = ( Y[j][i]-Y[j-1][i] )/Dz;
//    QV[j][i] *= sqrt( 1+ X*X+ Z*Z );  //= sqrt( 1+ X*X );
//  }
  }
#endif
                                        //
 const int la=24;                       // Количество дроблений по волнам
 static                                 //
 double La[la+1]={ 0.0 };               // Фазовая длина волны
       l=double( 1.0 )/la;              //
  for( i=1; i<=la; i++ )La[i]+=La[i-1]+sin( M_PI_2*l*i )*sqrt( 1.0-l*i );
  for( i=1; i<=la; i++ )La[i]*=WL/La[la];
  for( i=1; i<=la; i++ )if( La[i]>dX )
                        Rw+=QG4( La[i-1],La[i] );
 return 4*g*ro*Rw;
}
#if 0
struct Michell{ Real Vo;        // Скорость в м/с
                Real Vl;        // Водоизмещение
                Real Sf;        // Смоченная поверхность
                Real Rw;        // Сопротивление
                Real Cw;        // Коэффициент сопротивления
} Michell_Results={ 0,0,0,0 };  //
  Michell_Results.Vo=Vo;        // Скорость в м/с
  Michell_Results.Vl=Vl;        // Водоизмещение
  Michell_Results.Sf=Sf;        // Смоченная поверхность
  Michell_Results.Rw=Rw;        // Сопротивление
  Michell_Results.Cw=Cw;        // Коэффициент сопротивления

  for( j=0; j<Nz; j++ )                         //
  { Z = Dz*Wk*j;        if( Z>69 )continue;     //
    X = Dx/2;           // -0.5*Length;         // От носа к корме
    for( i=0; i<Nx-1; i++)                      //
    { A += QV[j][i] * exp( complex( -Z,Wk*X/Mw ) );
      X += Dx;
  } }

#endif
