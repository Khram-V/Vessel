//
//      Попытка восстановления студенческих идей по
//      моделированию обтекания тела произвольной формы
//
//                              Версия 2.4 от 11 апреля 1993 года.
//                              Василий Храмушин, Сахалин
#include "OS/Tv_graph.h"

const Real D2R=M_PI/180.0;
const int Blen=64000;       // Длина списков координат внутренних точек

Real  DCy=12,DCx=12,        // Размер простой фигуры
       dY=0.5,dX=0.5;       // Вертикальный и горизонтальный
Field F={ -15,-10,30,20 };  // размер и шаг прорисовки поля

static Real  dCy=0.2,       // Вертикальный и
             dCx=0.2,       //     горизонтальный шаг сканирования объекта
             dL =0.2,       // Шаг обхода вдоль контура
             FD =32.0,      // Угол набегающего потока в градусах
             Vm,Vd;         // Вычисляемые экстремумы для поля скорости
static Point P,             // Просто рабочий вектор или точка
             V,W,           // Вычисляемый вектор скорости
             D,             // Вектор направления внешнего потока
             M;             // Дипольный момент для квадратика.
static int Nb=0,            // Текущая длина для ряда внутренних точек
         Ny,y,Nx,x;         // Размерения матрицы поля вызванных скоростей
                            // Признаки готовности настроек к визуализации поля
static byte FMod=3,         // Вычислительная модель из диполей+вихреисточников
                            // 0 - вычисляется по методу панелей из Флетчера
                            // 1 - расчёт с контурным(поверхностным) интегралом
                            // 2 - расчёт с поверхностным(объемным) интегралом
            FVid=0,         // Блок управляющих признаков к выводу результатов
                            // 1 - требование расчета во внутренней области
                            // 2 - признак необходимости оценки положения точки
             **BF=NULL;     // Массив признаков для пространственных точек
static Point **EF=NULL,     // Массив для сохранения внешнего вызванного потока
           Body[Blen],      // Координаты точек образующих обтекаемое фигуру
           Flow[Blen];      // Собственные потоки в этих точках
void Curve_View();
void Curve_Show(); // Real&,Real&,Real&,Real& );
int  Curve_Access( Point );
Real Curve_Normal( Real D );            // D - Максимальная длина отрезка
                                        //   - На выходе полная длина контура
int  Curve_Normal( Point& R,Point& N ); // R - Центр контрольного отрезка
                                        // N - Нормаль к отрезку на контуре
void Fletcher                // Клайв Флетчер - C.A.J.Fletcher
( int   _N,                  // Количество точек контура (без соединения)
  Point *P,                  // Координаты узловых точек окаймляющих тело
  Point _V,                  // Компоненты скорости набегающего потока
  Point *V=0,                // Требование вычислить скорости в абсолютной
  Point *Q=0                 //    и локальной системах координат
);
Point Fletcher( Point );     // Выборка вызванной скорости в произвольной точке
//
//      Прорисовка расчётных полей и объектов
//      SM - 0x0001 - отметка внутренних точек тела
//           0x0002 - контур исходного тела
//           0x0004 - внутреннее поле потоков
//           0x0010 - векторные поля изображаются в изолиниях
//           0x0020 - ортогональная сеть изолиний
//           0x0100 - Y: внешнее поле потоков
//                    N: поле вызванных скоростей
//
static void Draw( Real Scale, short SM=0x8002 )
{ { Field _f=F; Tv_place( 0,&_f ); }
  Ghelp(); clear(); Tv.Height( 8 );                         color( LIGHTBLUE );
  Tgo( 98,-3 ); Tprintf( W2D( "(c)1993-99, В.Храмушин" ) ); color( CYAN );
  Tgo( 99,-2 ); Tprintf( W2D( "Лаб.вычислительной гидромеханики" ) );
  Tgo( 99,-1 ); Tprintf( W2D( "Сахалинский государственный университет" ) );
  color( LIGHTGRAY ); Tv_axis( true ); Tv.Height( 0 );
  color( LIGHTCYAN ); Tv_rect();              // разметка прямоугольной сетки
  if( FMod&1 || SM&2 || !FMod )               // изображение замкнутого контура
  { Curve_Normal( dL );  Curve_View();        // с очень маленькими нормальками
  }
  if( FMod&2 && SM>0 )                     // подготовка поверхностной картинки
  { color( BLUE );                         // квадратные ячейки плоской фигуры
    for( int k=0; k<Nb; k++ )
    { Point &Q=Body[k]; rectangle( Tv_x( Q.x-dCx/2 ),Tv_y( Q.y-dCy/2 ),
                                   Tv_x( Q.x+dCx/2 ),Tv_y( Q.y+dCy/2 ) );
    }
    color( LIGHTGRAY );                 // разметка наличия внешних контрольных
    if( BF )                            // крестиков внутри двумерного контура
     for( P.y=F.Jy+dY,y=0; y<Ny; P.y+=dY,y++ )
     for( P.x=F.Jx+dX,x=0; x<Nx; P.x+=dX,x++ )if( BF[y][x] )Tv_cross( P,-2 );
  }
  color( SM&0x100?LIGHTGREEN:LIGHTCYAN );
  if( SM>=0 )                                 // это стрелки по всему
  for( P.y=F.Jy+dY,y=0; y<Ny; P.y+=dY,y++ )   // графическому полю
  for( P.x=F.Jx+dX,x=0; x<Nx; P.x+=dX,x++ )   // в узлах EF[Ny][Nx]
  if( SM&4 || BF[y][x]==0 )
  { W=( SM&0x100 ? EF[y][x]/Vd-D:EF[y][x]/2 )*Scale; needle( P-W,P+W );
  } color( YELLOW );
  Scale=hypot( F.Ly,F.Lx )/12; needle( -D*Scale,D*Scale );
}
//      Опрос клавиатуры на терпение гидромеханика
//             <Escape> - пауза в расчете
/*
static int Interrupt()
{ if( Tv_getw()==_Esc  ){ Thelp("?B начало(Y/N)"); int ans=Tv_getc()|0x20;
      Ghelp(); if( ans=='y' || ans==_Enter  )return 1; } return 0;
}*/
//    --------------------------------------------------¬
//    ¦ Расчет экстремумов и нормировка по контуру тела ¦
//    L--------------------------------------------------
static void Contour()
{ int j,k,l=1;
 Point P,Q,R,N,V;
 Real  L=Curve_Normal( dL );
  color( YELLOW ); Tgo( 1,1 ); Tprintf( "Lenght=%.1lf\n",L ); Vm=Vd=0;
  color( LIGHTRED );           Tprintf( " Wait Please\n" );
  color( CYAN );
  if( FMod==1 )                // работа чисто по контуру
  { color( LIGHTMAGENTA );
    for( j=0; j<l; j++ )
    { for( V=0,k=0; Curve_Normal( R,N ); k++ )
      { if( k!=j )
        { if( k==j+1 )Q=R; R-=P; V-=M*N*R/norm( R )/2;
        } else if( !j )P=R;
      } if( !j )l=k; needle( P-V,P+V ); P=Q;
        if( Vd<(V*D).x )Vd=(V*D).x;
        if( Vm<abs( V ) )Vm=abs( V );
    } Tgo( 1,2 ); Tprintf( W2D( "Экстремум:  %4lg\n" ),Vm );
                  Tprintf( W2D( "Нормировка: %4lg\n" ),Vd );
  }
  if( !FMod )           // предварительный перерасчет для интегрального решения
  { Nb=0; while( Curve_Normal( R,N ) )Body[Nb++]=R;
    Tgo( 1,1 ); Tprintf( "Lenght[%d]=%.1lf\n",Nb,L );
    Fletcher( Nb,Body,-D,Body+Nb );
    for( k=0; k<Nb; k++ ){ Real M=abs( Body[Nb+k] ); if( Vm<M )Vm=M; } Vd=1;
  }
  if( FMod )Vm=Vd=0;                  // зачем-то здесь сбрасываются экстремумы
}
//      ----------------------------------------------------¬
//      ¦ Расчет вызванной скорости интегрированием контура ¦
//      L----------------------------------------------------
static Point Contour( Point& P )
{ Real A,B;
  Point V={ 0,0 },R,N;
         Curve_Normal( dL );
  while( Curve_Normal( R,N ) )
  { if( (A=norm( R-=P ))<(B=norm( N )/2) )A=B; V-=M*N*R/A/2;
  } return V;
}
//      -------------------------------------------------------¬
//      ¦ Расчет вызванной скорости интегрированием по площади ¦
//      L-------------------------------------------------------
static Point Surface( Point& P )
{ Real Ds=dCy*dCx;
  Point V={ 0,0 },W,E; dCy/=2; dCx/=2;
  for( int k=0; k<Nb; k++ )
  { W=P-Body[k];
    E=FMod&1 ? Flow[k] : M;
    if( abs( W.y )>=dCy || abs( W.x )>=dCx )
//      V+=E*sqr( W )*Ds/pow( norm( W ),1.5 );
        V+=E*sqr( W )*Ds/sqr( norm( W ) );
    else { if( FVid&1 )BF[y][x]=1; V+=E*sqr( W )*16/Ds; }
  } dCx*=2; dCy*=2;
  return V;
}
//      -------------------------------------¬
//      ¦  Расчет и подготовка поля потоков  ¦
//      ¦ в соответствии с требованиями METH ¦
//      L====================================-
static void Flow_Field()
{
//      Расчет вызванных скоростей и их экстремумов
//
 Point P,Q;
  BF =(byte** )Allocate( Ny=int( F.Ly/dY-0.5 ),Nx=int( F.Lx/dX-0.5 ),BF );
  EF =(Point**)Allocate( Ny,Nx*sizeof(Point),EF );
 int nY=0,mY=0;
 Real diagonal=hypot( dY,dX );
  if( FMod&2 )FVid|=1; Contour(); color( CYAN );
  if( FMod==3 )                                  // просчёт по "тензорам"
  { for( int k=0; k<Nb; k++ )
    { Flow[k]=conj( D-(Q=Contour( P=Body[k] )) )/M_PI; line( P,P+Q ); }
  }
  for( P=0;; )
  { if( fixed( y=Ny/2+mY )>=Ny ){ if( nY>=Ny )break; }else
    { ++nY;
           P.y=F.Jy+dY*(y+1);
      for( P.x=F.Jx+dX,x=0; x<Nx; P.x+=dX,x++ )
      {
        BF[y][x]=FVid&1?0:Curve_Access( P ); // 0 - везде, либо вне фигуры

        if( !FMod )V=Fletcher( P ); else
        { if( FMod&1 )V=Contour( P ); else V=0.0;
          if( FMod&2 )V+=Surface( P );
          if( !BF[y][x] ){ if( Vm<abs( V ))Vm=abs( V );
                           if( Vd<(V*D).x )Vd=( V*D ).x; }
        }
        EF[y][x]=V;     // if( Interrupt() )return;
        needle( P-dir( V )*diagonal/2,P+dir( V )*diagonal/2 );
      }
    } if( mY>0 )mY=-mY; else mY=1-mY;
  } FVid&=~1;
  color( YELLOW ); Tgo( 1,2 ); Tprintf( W2D( "Экстремум:  %4lg\n" ),Vm );
  color( WHITE  );             Tprintf( W2D( "Нормировка: %4lg\n" ),Vd );
}
//      +------------------------------------------+
//      ¦ Главная процедура для алгоритма плоского ¦
//      ¦    обтекания тела произвольной формы     ¦
//      -==========================================-
int main()
{ Mlist Menu[] =
  { { 1,0,"\tРазмеры расчетной области" }
  , { 1,4," Y: %4.2lf",&F.Jy},{0,4,"\t \xB6 %4.2lf",&F.Ly},{0,4,", dY %4.2lf",&dY }
  , { 1,4," X: %4.2lf",&F.Jx},{0,4,"\t \xB6 %4.2lf",&F.Lx},{0,4,", dX %4.2lf",&dX }
  , { 2,4,"\tНаправление потока %4.2lf",&FD  },{ 0,0,"\t°" }
  , { 2,4,"\tШаг вдоль контура: %4.2lf",&dL  }
  , { 1,4,"\tДробление тела dY: %4.2lf",&dCy }
  , { 1,4,  "               dX: %4.2lf",&dCx }
  , { 2,0,"\t  Интеграл по" },{ 0,10 }
  };
 static field Tr={ -32,-60,0,0,0 };
 int ans=0;
  Tv_Graphics_Start();                    // Установка драйвера на 256 цветов
  setwindowtitle( "Flow - fluidmechanics" );
  Tv_place( &Tr );
  M=conj( (D=(Point){ cos( FD*D2R ),sin( FD*D2R ) })/M_PI );
  void Curve_Analytic( int );
       Curve_Analytic( 2 );
//
//      Инициализация параметров расчетной области
//
Start: // pattern( BLUE,LIGHTCYAN );
  {
//   static Real ymn=-6,ymx=6, // Вертикальный и
//               xmn=-6,xmx=6; //  горизонтальный размер построенной фигуры
   Display T( Mlist( Menu ),1,-1 ); Draw( 1.0 );
    do
    { Thelp( "F4 Figure  F5 Start  %sF10 Exit",EF?"F6 Show  ":"" );
      color( CYAN );
      Menu[13].Msg=(char*)( FMod==2?"\t площади"
                          : FMod==1?"\t контуру"
                          : !FMod?"\t панелям":"\t тензорам" );
      ans=T.Answer();
      switch( ans )
      { case 0xF4: T.Back(); Draw( 1.0 ); Curve_Show(); // ymn,ymx,xmn,xmx );
        case 0xF5: break;
        case 0xF6: if( !EF )continue; goto Request;
        case 0xFa: return 0;
        case   13: (++FMod)&=3;
       default: if( ans>11 )continue;
      }
      M=conj( (D=(Point){ cos( FD*D2R ),sin( FD*D2R ) })/M_PI );
      Nb=0;
      T.Back();
      Draw( 1.0 );
      //
      // Построение тела сканированием попаданий внутрь замкнутой кривой
      //
      color( BLUE );
      if( FMod&2 )
      for( P.y=int( F.Jy/dCy-1 )*dCy; P.y<F.Jy+F.Ly; P.y+=dCy )
      for( P.x=int( F.Jx/dCx-1 )*dCx; P.x<F.Jx+F.Lx; P.x+=dCx )
       if( Curve_Access( P ) )
       { rectangle( Tv_x( P.x-dCx/2 ),Tv_y( P.y-dCy/2 ),
                    Tv_x( P.x+dCx/2 ),Tv_y( P.y+dCy/2 ) );
         Body[Nb++]=P;
         if( Nb>=Blen )Break( "Нет памяти для списка внутренних точек" );
       }
    } while( ans!=0xF5 ); // выход к исполнению основных расчетов
  }
//      Построение маленького информационного окошка
//
  { color( LIGHTCYAN );
    Twindow( 5,-1,20,3 );
    if( FMod&2 )
    { color( LIGHTGRAY ); Tprintf( W2D( "Объем : %3.1lf (%d)" ),dCy*dCx*Nb,Nb );
    }
  }
  Flow_Field();
//
//!     здесь устраиваются бегающие стрелки
Request:
 static Point VS,Vs,VV,Vv,VQ,VP={ 0.0,0.0 };
 static fixed VMode=1;               // Режим изображения результирующих полей
 static Real VScale=1;               // Дополнительный масштабный множитель
        Real VSt=Tv_X(1)-Tv_X(0);
  for( ;; )
  { Thelp
    ( "F1 Вызванные F2 Результат  F3 Настройка  F4 К нормировке  <Enter>В начало  <Esc>Выход" );
    do
    { setwritemode( XOR_PUT ); VQ=VP;
      if( FMod&2 )
      { Vv=Surface( VP );     color( LIGHTGRAY );
        VV=dir(Vv/Vd-D)*VSt*64; line( VQ,VQ+VV );
        Vv=dir(Vv)*VSt*32;      line( VQ,VQ+Vv );
      }
      if( FMod&1 || !FMod )
      { if( FMod&1 )
        { Vs= Contour( VP ); color( LIGHTMAGENTA ); }else
        { Vs=Fletcher( VP ); color( WHITE );    }
        VS=dir( Vs/Vd-D)*VSt*64; line( VQ,VQ+VS );
        Vs=dir( Vs )*VSt*32;     line( VQ,VQ+Vs );
      }
      ans=Tv_pause( &VP,0,VSt*(Tv_getk()&SHIFT?12:1) );
      if( FMod&1 || !FMod )
      { color( FMod&1?LIGHTMAGENTA:WHITE); line( VQ,VQ+VS ),line( VQ,VQ+Vs ); }
      if( FMod&2 ){ color(LIGHTGRAY);      line( VQ,VQ+VV ),line( VQ,VQ+Vv ); }
      setwritemode( COPY_PUT );
    } while( ans<_Enter );
//
//  Настройка картинки с обтекаемым телом
//
  Rd: switch( ans )
    { case _Esc: Ghelp( "?Are you sure" );
        if( (Tv_getc()|0x20)!='y' )break; Tv_close(); return 0;
      case _Enter:Tback(); Ghelp(); goto Start;
      case 0xF1:  VMode &=~0x100;  goto Dr;
      case 0xF2:  VMode |= 0x100;  goto Dr;
      case 0xF4:
      { char Msg[20];
        Tgo( 12,3 ); sprintf( Msg," %-8lg",Vd ); color( WHITE );
        if( (ans=Tgets( Msg,-9,1 ))!=_Esc )
        { sscanf( Msg,"%lf",&Vd );
          Tgo( 12,3 ); Tprintf( " %-8lg",Vd ); ans=0xF2; goto Rd;
        }
      } break;
      case 0xF3: do
      { Mlist Menu[]=
        { { 1,0,"\t  Параметры изображения" }
        , { 2,26,(char*)((VMode&1)==0?"\tБез отметки активных узлов":"\tС отметкой диполей") }
        , { 1,26,(char*)((VMode&2)==0?"\tБез изображения контура":"\tС исходных контуром тела") }
        , { 1,26,(char*)((VMode&4)==0?"\tБез внутренних потоков":"\tС внутренними потоками") }
        , { 2,5,"\t  Множитель: %5.3lf",&VScale }
        };
        Display T( Mlist( Menu ),1,-1 ); Ghelp(); color( CYAN );
        switch( ans=T.Answer( ans ) )
        { case 1: VMode^=1; break;
          case 2: VMode^=2; break;
          case 3: VMode^=4; break;
          case _Esc: goto Dr;
        }
      } while( ans<_Esc ); goto Rd;
    } continue;
Dr: color( BLACK ); Twindow( 5,-1,20,3 ); Draw( VScale,VMode ); Tback();
  } // for(;;)
}
