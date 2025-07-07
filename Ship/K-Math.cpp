//
//    Обращение к процедурам математического описания
//        или интерполяции реального корпуса (K-HULL)
//
#include "Hull.h"
#define __Water  glColor3f( 0,0.5,0 );
#define __Board  glColor3f( 0,0,1 );
#define __Marks  glColor3f( 0.3,0.3,0.3 );
#define __Deck   glColor3f( 0.25,0.25,0 );
#define __Grid   glColor3f( 0,0.75,0.75 );
extern
   MainDraw Win;   // прорисовка экранного изображения с дублированием в памяти
       Hull  Kh;        // Собственно корпус
static Real  Lh=2.0,    // Расчетная длина корпуса
             Bh=1.0,    //   и его опорная ширина
             Ts=0.75,   // Заглубление носового бульба
             Ns=0.6,    // Степень кривизны форштевня (1/Ns)
             Na=1.6,    // Степень крутизны кормового подзора
             Fs=0.0,    // Наклон форштевня
             Ls=0.1,    // Относительная длина бульба
             La=0.15,   // Длина кормового подзора
             Cx=0.5,    // Косинусная модуляция параболических ватерлиний
             Px=3,      // Степень полноты параболических ватерлиний
             Pz=6,      // Степень полноты шпангоутов
             Tz=0.5,    // Коэффициент заглубления бульба
             Ps=1.2,    // Параметры обратной полноты: P ў ]0Ў-[
             Pa=1.2,    //    P>10  косинусоиды
                        // 1 <P< 5  бульбовые ватерлинии
                        //    Pў 1  форма капли
                        //    P<.5  форма крыла ската
         Ks=0.8,Kp=1;   // Наклон борта: K>1 - развал; K<1 - завал
                        //
static int Hull_Type=2; // 0 - синустепенной; 1 - параболический корпус;
                        // 2 - реальный корабль по таблице ординат со штевнями
static Mlist
       Menu_a[]=
        { { 1,0,"         Описание штевней" }
        , { 2,4,"       Наклон форштевня Fs [-1~+1] = %4.2lf",&Fs }
        , { 1,4,"     Заглубление бульба Ts [0.5-1] = %4.2lf",&Ts }
        , { 1,4,"  Длина носового бульба Ls [0-0.5[ = %4.2lf",&Ls }
        , { 1,4,"       Степень кривизны Ns ]1-~2~] = %4.2lf",&Ns }
        , { 1,4,"Длина кормового подзора La [0-0.5[ = %4.2lf",&La }
        , { 1,4,"       Крутизна подзора Na ]1-~2~] = %4.2lf",&Na }
        , { 1,6,"      Длина корпуса Length       = %6.2lf", &Lwl }
        , { 1,6,"      Ширина       Breadth       = %6.2lf", &Breadth }
    //  , { 1,6,"      Высота борта   Depth       = %6.2lf", &Height  }
        , { 1,6,"      Осадка       Draught       = %6.2lf", &Draught } },
       Menu_s[]=
        { { 1,0," Синусоидально-показательные функции"  }
        , { 2,0,"Параметры обратной полноты: P є ]0-*[" }
        , { 1,0,"    P > 10   косинусоиды"              }
        , { 1,0," 1 <P < 5    бульбовые ватерлинии"     }
        , { 1,0,"    P є 1    каплеобразность"          }
        , { 1,0,"    P < 0.5  к форме крыла ската"      }
        , { 1,4,"   Ps = %3.1lf",&Ps },{ 0,0," форштевень"    }
        , { 1,4,"   Pa = %3.1lf",&Pa },{ 0,0," ахтерштевень"  }
        , { 1,4," Заглубление Ts = %4.2lf",&Tz },{ 0,0," є ]0-1]" }
        , { 1,4,"Hаклон борта Ks = %4.2lf",&Ks },{ 0,0," развал>1>завал" } },
       Menu_p[]=
        { { 1,0,"   Корпус, построенный на"  }
        , { 1,0,"     степенных функциях"    }
        , { 2,4,"Шпангоуты  Pz = %4.1lf",&Pz }
        , { 1,4,"Ватерлинии Pх = %4.1lf",&Px }
        , { 1,4,"Заострение Сх = %4.2lf",&Cx },{ 0,0," є [0-1]" }
        , { 1,4,"Борт Kp= %4.2lf",&Kp },{ 0,0," развал>1>завал" }
        };
static Real m_Stern( Real z );        // Ахтерштевень
static Real m_Stem ( Real z );        // Форштевень
static Real m_Hull ( Real z,Real x ); // Корпус
static int Set_Stem()                 // Построение корпуса по заданным штевням
{ TextMenu T( Mlist( Menu_a ),&Win,32 ); int ans;
      Ns=1/Ns; ans=T.Answer(); Ns=1/Ns;
           if( ans!=_Esc )Kh.Analytics(); else return _Esc; return 1; //ans;
}
int Set_Sinus()
{ TextMenu T( Mlist( Menu_s ),&Win ); Hull_Type=1; int ans;
Rep: if( (ans=T.Answer())!=_Esc )
       { if( (ans=Set_Stem())==_Esc )goto Rep; } return ans;
}
int Set_Power()
{ TextMenu T( Mlist( Menu_p ),&Win ); Hull_Type=0; int ans;
Rep: if( (ans=T.Answer())!=_Esc )
       { if( Cx>1 )Cx=1;
         if( (ans=Set_Stem())==_Esc )goto Rep; } return ans;
}
void Hull::Analytics()
{ strcpy( Name,Hull_Type ? "Синусно-показательный корпус"
                         : "Корпуса на степенных функциях" );
  if( Breadth<0.1 )Breadth=0.1;           Bh=Breadth/2;
  if( Length<Breadth*3 )Length=Breadth*3; Lh=Lmx=Length;
  if( Draught>Breadth )Draught=Breadth;   Depth=Draught*2.0; Do=Xo=0;
 int Jx=0,Nx=25,Nz=36,b;
 Real Y,Z,W;
  for( int i=1; i<Nz; i++ )
  { if( Lmx<(W=m_Stem( Z=i*Depth/(Nz-1) )) )Lmx=W;  // так, на всякий случай
    if( Xo>(W=m_Stern( Z )) )Xo=W;
  }
  //  Производится перестроение аналитического корпуса к обычному виду
  //
 Real Dx=(m_Stem(Draught)-m_Stern(Draught))/(Nx+1),Dz,x,z;
      Jx=0.04-Xo/Dx;
      Nx=Jx + int( 0.96+Lmx/Dx );
      Simple_Hull( Nx,Nz,Nz*2 );  Ms=12+Jx; W=Lmx-Xo;     // W=m_Stem( Depth );
 static Frame Deck( 6 ); Deck[0]=Depth*0.9,Deck(0)=Deck.min=Xo;
                         Deck[1]=Depth*0.9,Deck(1)=Xo+Dx/2;
                         Deck[2]=Depth*0.8,Deck(2)=Xo+W*0.3;          // W/2;
                         Deck[3]=Depth,    Deck(3)=Xo+W*0.75;         // W-Dx;
                         Deck[4]=Depth*1.1,Deck(4)=Lmx-Dx;            // W;
                         Deck[5]=Depth*1.1,Deck(5)=Deck.max=Lmx; Deck.SpLine();
  for( int i=0; i<Nx; i++ )
  { F[i].X=x=Dx*(i-Jx)+Dx/4;
    Dz=Deck.G( x,true )/pow( Nz-1,1.543080635 ); Y=1.0; b=1; // x ставится YG
    for( int j=0; j<Nz; j++ ){ z=Dz*pow( j,1.543080635 );
     int &N=F[i].N;                   // изменяемая длина контура шпангоута
     int k=j+N+1-Nz;                  // отсчёт с учётом сброса лишних ординат
     Real y=m_Hull( z,x );            // ордината корпуса в выбранной позиции
      if( j>0 )                       // киль остается как есть
      { if( Y && !y )b=0;             // для отскока на один ноль вверх
        if( !y && !Y )                // два нуля к пропуску ординат шпангоута
          { if( !b )b=1; else --N,--k; } }
      F[i][k]=Y=y;                    // под запись всё подряд и без разбора
      F[i](k)=z;                      // гашение двойных нулей сверху-вниз
//    if( j==Nz-1 )while( N>0 && !(F[i][N-1]) && !(F[i][N]) )N--;
    }
  }
  Nz*=2;
  for( int i=0; i<2; i++ )            // штевни с удвоенным количеством точек
  for( int j=0; j<Nz; j++ )
   if( i )Stx[j]=m_Stem(  Stx(j)=Sty(j)=j*Deck[5]/(Nz-1) ),Sty[j]=0;
     else Asx[j]=m_Stern( Asx(j)=Asy(j)=j*Deck[0]/(Nz-1) ),Asy[j]=0;

  for( int i=0; i<Nx; i ++ )
  { x=F[i].X;                         // смещение аппликаты с нулевой ординатой
    for( int j=0; j<F[i].N; j++ )     // в точку пересечения шпангоута и штевня
    { Real &Z=F[i](j),  aX=m_Stern( Z ), sX=m_Stem( Z ),
           &z=F[i](j+1),ax=m_Stern( z ), sx=m_Stem( z );
      if( F[i][j]==0 ){ if( x>sX )Z=z+(x-sx)*(Z-z)/(sX-sx);
                        if( x<aX )Z=z+(x-ax)*(Z-z)/(aX-ax); }
      if( F[i][j+1]==0 ){ if( x>sx )z=Z+(x-sX)*(Z-z)/(sX-sx);
                          if( x<ax )z=Z+(x-aX)*(Z-z)/(aX-ax); }
  } } Hull_Type=2; Lmx-=Xo; MinMax(); Init();
}
static Real m_Stem( Real z )
{ if( Hull_Type==2 )return Kh.Stx.G( z );
  { Real  m = Ts*Ns/(1-Ts),
          l = pow( Ts,m )*pow( 1-Ts,Ns );
          z = 1-z/Draught;
    return Lh *
     ( 1 + ( 1-(Fs*(z+1))/2 )           // наклон форштевня
         * ( 1+Ls*( z<0?
                    z*z*(1-Ls):
                    pow( fabs(z),m )
                 *  pow( 1-z,Ns )/l) )/( 1+Ls ) )/2;
} }
static Real m_Stern( Real z )
{ if( Hull_Type==2 )return Kh.Asx.G( z ); z=1-z/Draught;
//return Lh*( 1 + sin( tanh( z*Na )*2.25 )*( z+2 )*La/3-1 )/2;
  return Lh*sin( tanh( z*Na )*2.25 )*( z+2 )*La/6;
}
static Real m_Hull( Real z, Real x )
{ if( Hull_Type==2 )return Kh.Y( x,z );
  Real A,S,Z;
   if( z<1e-12 )return 0;
   if( (S=m_Stem( z ))<x )return 0;
   if( (A=m_Stern( z ))>x )return 0;
   x = ( 2*x-S-A )/( S-A );
   z = 1-z/Draught;
   Z = z*Tz;
  return Hull_Type
  ? Bh * pow( 1-z,1/(2+8/(Pa+Ps)) )       // Мидельшпангоут
       / ( 1+1/Pa+1/Ps )/2                // Нормировка на полуосадку
       * ( 1+cos( M_PI*x ) )              // Синусная ватерлиния
       * ( 1+pow( Ps*( 1-x ),-Z-1 )       // Носовая группа линий
            +pow( Pa*( 1+x ),+Z-1 ) )     // Кормовые множители
       * ( 1+(1-Ks)*z )                   // Коэффициент завала бортов
  : Bh * ( 1-pow( fabs( x ),Px ) )        // Степенная ватерлиния
       * ( cos( x*M_PI_2*Cx ) )           // и ее косинусная модуляция
       * ( 1-(z>0?pow( z,Pz ):0) )        // Степенной шпангоут
       * ( 1+(1-Kp)*z );                 //               +
}                                       //        -+     -┼-  o
                                       //         -┼-   --┼/--=¬  ¦
void Building()                       //       ___\│ г--+-┼--°°L¬\+/
{                                    //        ____+_¦___=======¦=¦===|
  Real x,y,z,d;                     //        --___________________v_/ ___
  int  k,n,m;                      //  ≈ ≈≈≈≈≈°|                    / ≈≈--
//                                __ ~~~ ~~ ~~ L8__________________|  ~ ~~
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//!   Проекция полуширота - собственно блок рисования теоретических чертежей
//
  Win.Activate().Clear(); wW.Focus();
  __Grid glLineStipple( 1,0x0FFF );    // Первыми прорисовываются ватерлинии
  x=Kh.Asx.G( Draught );               //
  for( k=0,z=0; k<=5; z+=Breadth/10,k++ )                       // -- батоксы
  { if( k%5 )glEnable( GL_LINE_STIPPLE ); else glDisable( GL_LINE_STIPPLE );
    line( x,z,x+Lwl,z );
  }
  m=Kh.Ns<51 ? 1 : Kh.Ns<101 ? 5:10;
  for( k=0; k<Kh.Ns; k++ )
  { x=Kh.F[k].X; __Grid                                        // -- шпангоуты
    if( k%21 && k!=Kh.Ms )glEnable( GL_LINE_STIPPLE ); line( x,0,x,Breadth/2 );
    glDisable( GL_LINE_STIPPLE ); __Marks
//  if( !((Kh.Nstem-k)%m) )
//  Win.Text( k==Kh.Ms?_North_East:_North,x,wW.Z(wW.z(0)+8),0,"%d",Kh.Nstem-k);
//  Win.Text( k==Kh.Ms?_South_East:_South,x,wM.Z(wM.z(0)),0,"%d",Kh.Nstem-k);
  }                                       // здесь прорисовываются контуры
  glEnable( GL_LINE_SMOOTH );             // пяти ватерлиний по масштабу осадки
  for( k=0,z=0.0; k<=10 && z<=Depth; k++,z+=Draught/5 )
  { d=Lmx*0.0002; x=Kh.Asx.G( z,true )-d; y=Kh.Stx.G( z,true )+2*d;
    if( k>5 )__Board else
    if( k<5 )__Water else __Deck
    glBegin( GL_LINE_STRIP );
    for( ; x<y; x+=d )glVertex2d( x,Kh.Y( x,z ) ); glEnd();
  } __Deck
  glBegin( GL_LINE_STRIP );
  glVertex2d( x=Kh.Asx[Kh.Asx.N],0.0 );         // Прорисовка линии палубы
  glVertex2d( x,Kh.Asy[Kh.Asy.N] );             // ! x - здесь встает на палубе
              y=Kh.Stx[Kh.Stx.N];
  for( k=0; k<Kh.Ns; k++ )
  { z=Kh.F[k].X; if( z>x && z<y )glVertex2d( z,Kh.F[k].y[Kh.F[k].N] );
  } glVertex2d( y,Kh.Sty[Kh.Sty.N] );
    glVertex2d( y,0.0 );
    glEnd(); __Marks
  Win.Text( _South_West,Xo+Lmx,wW.Z(-3),0,Kh.Name );
  //
  //!  Проекция бок
  //
  wM.Focus(); z=min( Depth,Draught*2 );
  for( k=0; k<Kh.Ns; k++ )                          // Разметка шпангоутов
  { if( k%21 && k!=Kh.Ms )glEnable( GL_LINE_STIPPLE );
//                  else glDisable( GL_LINE_STIPPLE );
    __Grid x=Kh.F[k].X; line( x,0,x,z );
    glDisable( GL_LINE_STIPPLE );
    __Marks
    if( !((Kh.Nstem-k)%m) )
    Win.Text( k==Kh.Ms?_South_East:_South,x,wM.Z(-2),0,"%d",Kh.Nstem-k);
  }
  // Выделение и расчистка окна для проекции корпус
  //
  glColor3f( 0.91,0.96,1 );
  wH.Focus().Clear( false );
  wM.Focus();
  //
  //  Батоксы
  {
#define kx( x ) ( Xo+x*LX )
#define kz( z ) ( Do+z*HY )
#define list( Z,X ){ Lst[k*4+Nb[k]++]=(Pts){ (float(x) X),(float(z) Z) }; }
#define lines( a,b ) \
 { if(a.x!=b.x||a.z!=b.z)glVertex2fv((float*)(&a)),glVertex2fv((float*)(&b)); }
   const int nx=720,nz=640,nl=5;
   static float **A=NULL;              // небольшая экономия оперативной памяти
   struct Pts{ float x,z; }Lst[(nl-1)*4]; // внутри контрольного прямоугольника
   float  LX=Lmx/nx,HY=(Depth-Do)/nz;
    if( !A )A=(float**)Allocate( nz+1,(nx+1)*sizeof(float) );
#pragma omp parallel for                  // { { Xo,Do },{ Xo+Length,Height } }
    for( int j=0; j<=nz; j++ )
    for( int i=0; i<=nx; i++ )A[j][i]=Kh.Y( kx( i ),kz( j ) );
    glBegin( GL_LINES ); __Grid;
    for( int z=0; z<nz; z++ ){ float *_1=A[z],*_2=A[z+1],fr=*_1++,FR=*_2++;
      for( int x=0; x<nx; x++ ){ int Nb[nl-1]={ 0 };
        for( int b=0;b<4; b++ ){ float f,F,Fr;
          if(  b<1  ){ z++;      f=fr; Fr=F=FR; } else{ f=Fr;
            if( b<2 ){ x++;       Fr=F = *_2++; } else
            if( b<3 ){ --z; FR=Fr,Fr=F = *_1++; } else
                     { --x; F=fr,fr=Fr; } }
          for( int k=0;k<nl-1;k++ ){ float Y=(k+1)*Breadth/(-2*nl),y=Y+f; Y+=F;
            if( Y==0 )list( , ) else
            if( Y*y<0){ Y=Y/(b>1?f-F:F-f); if(b&1)list( ,-Y) else list(-Y, ) }
        } }
        for( int k=0; k<nl-1; k++ )if( (m=Nb[k])>1 ){ Pts *L=Lst+k*4;
          for( int i=0;i<4;i++ ){ Pts &P=L[i]; P.x=kx( P.x ),P.z=kz( P.z ); }
          if( m<4  )L[m]=L[3-m];
                    lines( L[0],L[2] );
          if( m==3 )lines( L[2],L[1] );
          if( m>2  )lines( L[1],L[3] );
    } } } glEnd();
  }
  // Разметка сетки ватерлиний
  //
  x=Kh.Asx.G( Draught );
  for( k=0,z=0.0; k<=10 && z<Depth; z+=Draught/5,k++ )
  { if( k%5 )glEnable( GL_LINE_STIPPLE ); else glDisable( GL_LINE_STIPPLE );
    if( k==5 )__Deck       //__Water
         else gl_LIGHTGREEN; line( x,z,x+Lwl,z ); glDisable( GL_LINE_STIPPLE );
    if( k>0 && z<Draught*2 )
    { if( z>Draught )__Marks else __Water
      Win.Text( _West,Xo,z,0,k!=5?"%d ":"Квл",k );
  } }
  // Разметка контура: палуба - штевни - киль
  //
  __Deck
  glEnable( GL_LINE_SMOOTH );
  glBegin( GL_LINE_LOOP );
  m=Kh.Stx.N;
  n=Kh.Asx.N;          glVertex2d( Kh.Asx[0],Kh.Asx(0) );
  for( k=1; k<=n; k++ )glVertex2d( Kh.Asx[k],Kh.Asx(k) ); // Ахтерштевень
  for( k=0; k<Kh.Ns; k++ )
  { Frame &W=Kh.F[k]; x=W.X;
    if( x>Kh.Asx[n] && x<Kh.Stx[m] )glVertex2d( x,W.z[W.N] );
  }
  for( k=m; k>=0; k-- )glVertex2d( Kh.Stx[k],Kh.Stx(k) ); // Форштевень
  for( k=Kh.Ns-1; k>=0; k-- )
  { x=Kh.F[k].X;
    if( x>Kh.Asx[0] && x<Kh.Stx[0] )glVertex2d( x,Kh.F[k].z[0] );
  } glEnd();          //            lineto( ~Kh.Asx[0] );
  wH.Focus();         // и в завершение
//glColor3f( 0.91,0.96,1 );
  Draw_Hull( 1,Win ); //   переход к изображению проекции "Корпус"
  Win.Show();
  Active|=2;
}
//
//   Прорисовка проекции корпус (основного теоретического чертежа)
//
void Draw_Hull( int ids, Place &_W )                         // Проекция корпус
{ int i,k,m,n; Real x,y,z;
//glColor3f( 0.91,0.96,1 ); wH.Clear( false );
  if( ids )
  { __Grid z=min( Depth,Draught*2 );                              // Сетка
    for( k=0,y=-0.4*Breadth; k<=8; k++,y+=Breadth/10 )            // -- батоксы
    { if( k!=4 )glEnable( GL_LINE_STIPPLE ); else glDisable( GL_LINE_STIPPLE );
      line( y,0,y,z );
    } glDisable( GL_LINE_STIPPLE );
    __Water _W.Text( _North_East,Breadth/2,Draught,0,"ВЛ" );
    __Deck  _W.Text( _South_West,0,0,0,"ДП" );
    if( ids>1 )                                          // Разметка ватерлиний
    { x=Breadth/2;
      for( k=0,z=0.0; k<=10 && z<Depth; z+=Draught/5,k++ )
      { if( k%5 )glEnable( GL_LINE_STIPPLE ); else glDisable( GL_LINE_STIPPLE );
        if( k==5 )__Deck else if( k>5 )gl_LIGHTBLUE; else gl_LIGHTGREEN; //__Grid
        line( -x,z,x,z );
        glDisable( GL_LINE_STIPPLE );
        if( k>0 && z<Draught*2 )
        { if( z>Draught )__Marks else __Water
          _W.Text( _East,x,z,0,k!=5?" %d":"",k );
      } } glEnd();
  } }
  glEnable( GL_LINE_SMOOTH );
  for( k=m=0; k<Kh.Ns; k++ )             // Прорисовка теоретических шпангоутов
  { if( !m && k==Kh.Ms+1 ){ m=1; k=Kh.Ms; } // если нет сплайнового сглаживания
    if( (Hull_Keys&1) )                    //  то отмечаются шпангоутные точки
    { Frame &W=Kh.F[k];             // if( W.z[i]>Draught )__Board else __Water
      glPointSize( 3 ); gl_CYAN; glBegin( GL_POINTS );
      for( i=0; i<=W.N; i++ )glVertex2d( m?W.y[i]:-W.y[i],W.z[i] );
      glEnd(); glPointSize( 1 );
    }
    glBegin( GL_LINE_STRIP );
    if( Hull_Keys&1 )
    for( Real a=0; a<=1.0005; a+=0.001 )    //! 1000 точек по контуру шпангоута
    { Kh.F[k].AYZ( a,y,z );                 // контуры шпангоутов
      if( z>Draught )__Board else __Water   // синие сверху и зеленые под водой
      if( !m )y=-y; glVertex2d( y,z );
    } else                                 // или изображение исходных контуров
    { Frame &W=Kh.F[k]; Real wz,wy; __Water
      for( i=0; i<=W.N; i++ )
      { z=W.z[i],y=W.y[i];
        if( !m )y=-y;                    // пересчёт смоченной точки на контуре
        if( i>0 && wz<Draught && z>Draught )
          { y=wy+(Draught-wz)*(y-wy)/(z-wz); z=Draught; --i; }    // ватерлиния
        if( i>0 && i==W.N && y==0 && wz==z )gl_LIGHTGRAY; // без плоской палубы
        else if( z>Draught )__Board
        glVertex2d( wy=y,wz=z );
      }
    } glEnd();
  } __Deck
  glBegin( GL_LINE_STRIP );  glVertex2d( -Kh.Asy[0],Kh.Asy(0) );
  for( k=1; k<=Kh.Asy.N; k++)glVertex2d( -Kh.Asy[k],Kh.Asy(k) );
  //≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈//                         ~conj( Kh.Asy[k]
  glColor3f( 0,0,0.75 ); m=Kh.Asy.N;
            i=Kh.Ns/2+1; n=Kh.Sty.N;
  for( k=0; k<Kh.Ns; k++ )                                 // Палуба (и транцы)
  { if( i && k==i )
    { k=i-1; i=0; glVertex2d( y,z ); glEnd(); glBegin( GL_LINE_STRIP );
                  glVertex2d( -y,z );  }
    y=Kh.F[k].y[Kh.F[k].N];
    z=Kh.F[k].z[Kh.F[k].N];
    if( !y && k==Kh.Ns-1 )glVertex2d( Kh.Sty[n],Kh.Sty(n) );
    if( i )y=-y;          glVertex2d( y,z );
    if( !y && !k         )glVertex2d(-Kh.Asy[m],Kh.Asy(m) );
  }
  for( i=0; n>=0; n-- )
     { glVertex2d( Kh.Sty[n],Kh.Sty(n) ); if( !i ){ __Deck; i=1; } }
  glEnd();
}
//   Простенькие графические операции
//
void line( _Real x1,_Real z1,_Real x2,_Real z2 )
{ if( x1!=x2 || z1!=z2 )
  { glBegin( GL_LINES ); glVertex2d( x1,z1 ); glVertex2d( x2,z2 ); glEnd(); }
}
//   Подготовка упрощенного варианта теоретического корпуса с особенностями
//
Plaze::Plaze( int nz, int nx, _Real Board ): Nz( nz ),Nx( nx )
{ int  i,j;
 Real x,y,z,w;
   Y=(Real**)Allocate( Nz,Nx*sizeof(Real) );  // Таблица ординат - действующая
  QV=(Real**)Allocate( Nz,Nx*sizeof(Real) );  // Источники со смещенными узлами
  Xa=(Real*) Allocate( Nz*sizeof(Real)    );  // абсциссы ахтерштевня
  Xs=(Real*) Allocate( Nz*sizeof(Real)    );  //   и форштевня
  Ya=(Real*) Allocate( Nz*sizeof(Real)    );  // аппликаты на ахтерштевне
  Ys=(Real*) Allocate( Nz*sizeof(Real)    );  //   и форштевне (транцы)
  Wx=(Real*) Allocate( Nx*sizeof(Real)    );  // вектор волнообразования
  dZ=(Board-Do)/(Nz-1);                  // Do - предустанавливается при вызове
  for( z=Do,j=0; j<Nz; j++,z+=dZ )
  { w=Xs[j]=m_Stem( z );  if( !j )xS=w; else if( xS<w )xS=w; // форштевень
    w=Xa[j]=m_Stern( z ); if( !j )xA=w; else if( xA>w )xA=w; // ахтерштевень
  }
  V=S=0.0;
  dX=(xS-xA)/(Nx-1);
  for( z=Do,j=0; j<Nz; j++,z+=dZ )                           //++ водоизмещение
  for( x=xA,i=0; i<Nx; i++,x+=dX )
  { Y[j][i]=y=Kh.Y( x,z ); if( !i || !j || i==Nx-1 || j==Nz-1 )y/=2; V+=y;
  } V*=dX*dZ*2.0;
  for( i=1; i<Nx-1; i++ )S+=Y[1][i]*2; S+=Y[1][0]+Y[1][Nx-1]; S*=dX;   // днище
  for( j=0; j<Nz-1; j++ )                      // смоченная поверхность корпуса
  { S+=(Y[j][0]+Y[j+1][0]+Y[j][Nx-1]+Y[j+1][Nx-1])*dZ;  // без двойки два борта
    for( i=1; i<Nx; i++ )
    if( Y[j+1][i]>0.0 )
    { y=Y[j][i]; S+=sqrt( 1+norm((Y[j][i-1]-y)/dX,(Y[j+1][i]-y)/dZ) )*dX*dZ*2;
  } }
  for( j=0; j<Nz; j++ )                             // распределённые источники
  { for( i=1; i<Nx-1; i++ )QV[j][i]=(Y[j][i+1]-Y[j][i-1] )*dZ/-2.0;
    QV[j][Nx-1]=QV[j][Nx-2]/2.0;
    QV[j][0]=QV[j][1]/2.0;
} }
Plaze::~Plaze()
{ Allocate( 0,0,Y  );
  Allocate( 0,0,QV ); // Источники со смещенными узлами
  Allocate( 0,Xa );   // Абсциссы ахтерштевня
  Allocate( 0,Xs );   //    и форштевня
  Allocate( 0,Ya );   // Аппликаты на ахтерштевне
  Allocate( 0,Ys );   //    и форштевне (транцы)
  Allocate( 0,Wx );   // Вектор волнообразования
}
