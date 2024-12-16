//
//    Независимо существующая прорисовка трех волновых структур
//    удерживается на экране и активизируется мышкой и таймером
//
//    C = sqrt( gL/2п ) - фазовая скорость трохоидальной волны
//    Fn = V/sqrt( gL ) - скорость по Фруду
//                                                        ©2018-08-22 Иерусалим
//
#include "Aurora.h"                 // графическая среда OpenGL - Window-Place
                                    // производные операции с корпусом на волне
//
//!   Блок графической прорисовки волнения в контурах и раскрасках
//
const colors Surf=lightcyan,   // светлая надводная раскраска волнового гребня
             Subw=lightgreen;  // и зеленоватый окрас в подошве штормовой волны

int Waves::Color( _Real mH )   // 256 бело-синих оттенков высоты волн Герстнера
 { return minmax( 0,int( (mH/(Length*(1.134/_Pd/2.0))+1.0)*127.5 ),255 );
 }
//  Прорисовка волновых полей посредством внутренних операций числового объекта
//                          -- рисунок с внешними настройками графической среды
Waves& Waves::Ghost( _Real Tr ){
#pragma omp parallel for
  for( int j=0; j<=My; j++ )                           //!?+ под поле скоростей
  for( int i=0; i<=Mx; i++ )
  { Vector W=Wave(Tr,j,i); if( Storm->Exp.peak )H[j][i]=W; else H[j][i].z=W.z;
  } return *this;
}
#define colcel( z ) shadow? colors( black+Color( z ) ): z>0?Surf:Subw

Waves& Waves::View( _Real z, int k, bool shadow )
{ if( Storm->Exp.peak )    // .peak=1 просмотр в регулярной неравномерной сетке
  { const Vector oZ={ 0,0,z };    // с искусственным смещением z по вертикали
    for( int j=0; j<=My-k; j+=k ) // работа в векторном поле верна на удивленье
    { Vector *_p=H[j],*_q=H[j+k]; glBegin( GL_QUAD_STRIP );
      for( int i=0; i<=Mx; i+=k )
      { _Vector p=*_p,q=*_q; _p+=k,_q+=k;            // нормаль косым крестиком
        dot( q+oZ,colcel( q.z ) );
        dot( p+oZ,colcel( p.z ) );
        if( !Storm->Exp.draw )if( i<=Mx-k )glNormal3dv( (*_p-q)*(*_q-p) );
      } glEnd();
  } } else                // .peak=0 - иначе на регулярной сетке с равным шагом
  { Real y=-0.5*Ds*My,d=Ds*k;     // с перерасчётом координат на этом же месте,
    Vector *_p,p,pi, *_q,q,qi;    // с небольшой оптимизацией узловых переборов
    for( int j=0; j<=My-k; j+=k ) // упрощение математики=усложнение алгоритмов
    { pi.y=p.y=y;   p.z=(_p=H[j])->z;   p.x=q.x=-0.5*Ds*Mx;
      qi.y=q.y=y+d; q.z=(_q=H[j+k])->z; glBegin( GL_QUAD_STRIP );
      for( int i=0; i<=Mx; i+=k )
      { if( i<=Mx-k ){ pi.z=(_p+=k)->z; pi.x=qi.x=p.x+d;
                       qi.z=(_q+=k)->z;
                       if( !Storm->Exp.draw )glNormal3dv( (pi-q)*(qi-p) ); }
        color( colcel( q.z ) ); q.z+=z; dot( q ); q=qi;
        color( colcel( p.z ) ); p.z+=z; dot( p ); p=pi;
      } glEnd(); y+=d;
  } } const Real x=Ds*Mx/2,y=Ds*My/2;
  liney( (Vector){-x,0,z},(Vector){-x,y,z},lightgray ), // граничная рамка для
  liney( (Vector){-x,y,z},(Vector){ x,y,z} ),         // контроля контура волны
  liney( (Vector){ x,y,z},(Vector){ x,0,z} ); return *this;
}
//     Пусть прорисовка идет вместе с настройкой графической площадки\сцены
//
inline int ColorSelect( int a, int b )
{ if( a*b<0 )return copysign( sqrt( fabs( a*b ) ),a+b );
  if( a>=0 )return max( a,b ); return min( a,b );
}
//        Exp.draw: 0-раскраска 1-контуры 2+разрядкой 3-только профили волн
//           +view: 0-в свете 1-с контуром 2+разделение 3-разделение в контуре
//
bool Field::Draw()
{ static bool Recurse=false;
  if( !IsWindowVisible( hWnd ) || IsIconic( hWnd ) )return false;
  if( Recurse || !Kt )return false; Recurse=true; Activate();
  glEnable( GL_LIGHTING );
//  glEnable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST );
  glMatrixMode( GL_PROJECTION );     // размерах фрагмента экрана сброс текущих
  glLoadIdentity();                  // матричных координат, настройка обзора и
  gluPerspective( 16.2,Real( Width )/Height,-63,63 );     // экранных пропорций
  gluLookAt( 0,0,Distance,lookX,lookY,lookZ,0,1,0 );     // точка обзора
  glMatrixMode( GL_MODELVIEW ); glLoadIdentity();       // в исходное положение
  glClearColor( BkColor,1 );                      // цвет фона окна и
  glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );   // полная расчистка
  glRotated( eyeY-90,1,0,0 );                          // поставить на киль
  glRotated( eyeZ,   0,1,0 );                          // дифферент
  glRotated( eyeX,   0,0,1 );                          // рыскание
  AlfaVector(Height/WinLines);
  //
  //   рисуем оси координат и габаритный прямоугольный параллелограмм
  //
  axis( *this,Long*0.52,Wide*0.53,30,"N","W","n" );
//liney( (Vector){Long/2, Wide/2,10},(Vector){Long/-2,Wide/2,10},lightgray );
//liney( (Vector){Long/-2,Wide/2,10},(Vector){Long/-2,0,10} );
//liney( (Vector){Long/2, Wide/2,10},(Vector){Long/2, 0,10} );
//liney( (Vector){Long/2,0,10},      (Vector){Long/-2,0,10} );
  liney( (Vector){Long/2, Wide/2,0}, (Vector){Long/-2,Wide/2,0},lightblue );
  liney( (Vector){Long/-2,Wide/2,0}, (Vector){Long/-2,0,0} );
  liney( (Vector){Long/2, Wide/2,0}, (Vector){Long/2,0,0} );
  //
  // сначала корпус корабля, тогда будет позволительно делать прозрачность воды
  //
  glPolygonMode( GL_FRONT_AND_BACK,GL_FILL );
  Vessel->Drawing( Exp.draw==3?2:1 );   // доступ к рисунку корпуса корабля
  Activate();                           // без исключения двойной перенастройки
  //
  //   собственно рисование для начала ведется в самом волновом объекте
  //
  glPolygonMode( GL_FRONT,Exp.draw?GL_LINE:GL_FILL );
  glPolygonMode( GL_BACK,GL_LINE ); // POINT );

  if( ( Exp.draw )!=3 )
  { int k=max( Exp.draw&2?mY/100:1,1 ); // поля волн надо для палитры раскраски
    if( Exp.wave==1)Wind.Ghost(Trun),Swell.Ghost(Trun),Surge.Ghost(Trun);
    glLineWidth( 0.01 ); // тончайшие линии на случай прорисовки контуров ячеек
    if( Exp.view&2 )
    { liney((Vector){Long/2,Wide/2,0},(Vector){Long/2,Wide/2,Long/-4},lightblue);
      liney((Vector){Long/-2,Wide/2,0},(Vector){Long/-2,Wide/2,Long/-4},lightblue);
      Surge.View( Long/-4,    k,!(Exp.view&1) );   // -200 {от 800·600}
      Swell.View( Long/-5.333,k,!(Exp.view&1) );   // -150 три разделённые
       Wind.View( Long/-8,    k,!(Exp.view&1) );   // -100 структуры волн
    }
    if( Exp.view&1 )                         // светлая вода с ватерлинией
    for( int y=0; y<mY-k; y+=k )             // монотонные раскраски с цветовым
    { glBegin( GL_QUAD_STRIP );              // разделением по нулевому контуру
      for( int x=0; x<mX; x+=k )
      { Vector &p=Ws[y][x],&q=Ws[y+k][x];
        if( x<mX-k )glNormal3dv( (q-p)*(Ws[y][x+k]-p) );
        color( q.z>0?Surf:Subw,-0.1,Exp.view&2?1:0.75 ),dot( q );
        color( p.z>0?Surf:Subw,-0.1,Exp.view&2?1:0.75 ),dot( p );
      } glEnd();
    } else
    { static int nx=0,ny=0,**Clr=NULL;       //  сборная цветовая матрица
      if( mY>ny || mX>nx )Clr=(int**)Allocate( ny=mY,(nx=mX)*sizeof(int),Clr );
#pragma omp parallel for
      for(int y=0;y<mY;y++) // выборка расцветки волновых склонов выполняется с
      for(int x=0;x<mX;x++) //  раздельным использованием исходных пакетов волн
      { Vector &W=Ws[y][x];
        int C =   ColorSelect( C=0,Wind.Color( Wind.Amd( W ).z ) );
            C =   ColorSelect( C,Swell.Color( Swell.Amd( W ).z ) );
        Clr[y][x]=ColorSelect( C,Surge.Color( Surge.Amd( W ).z ) )+black+1;
      }
     bool f2=Exp.view&2 && !Exp.draw;          // Тёмная вода на четыре уровня
      while( true )
      { for( int y=0; y<mY-k; y+=k )           // тёмная вода в световых оттенках
        { glBegin( GL_QUAD_STRIP );            // раскраска общего волнового поля
          for( int x=0; x<mX; x+=k )           // в оттенках сине-зеленого цветов
          { Vector &p=Ws[y][x],&q=Ws[y+k][x];
            if( x<mX-k )glNormal3dv( (q-p)*(Ws[y][x+k]-p) );
            if( Exp.view&2 )color( colors(Clr[y+k][x]),0.1,1 ),dot( q ),
                            color( colors(Clr[y][x]),  0.1,1 ),dot( p );
                      else  color( colors(Clr[y+k][x]),0,0.9 ),dot( q ),
                            color( colors(Clr[y][x]),  0,0.9 ),dot( p );
          } glEnd();
        }
        if( f2 ){ glPolygonMode( GL_FRONT,GL_LINE ); f2=0; continue; } else break;
      } // восстановление закраски полигонов толщины обычных контурных линий
    } glPolygonMode( GL_FRONT_AND_BACK,GL_FILL ); glLineWidth( 1 );
  }
//! Информационная табличка c режимами проведения вычислительного эксперимента
//
 Hull &V=*Vessel; _Real S=V.Speed,L=V.Length;
  V.NavigaInform( this );         // картушка морского волнения, курса+скорости
  color( navy );Print(2,1,Exp.peak?"Трохоидальные волны:":"В полных потоках:");
  color( blue );Print( ModelWave[Exp.wave&3] );
//color( blue ); Print( 2,1,"%s: %s",
//   Exp.peak?"Трохоидальные волны":"В полных потоках",ModelWave[Exp.wave&3] );
  color( black ); Print( 2,2,Model[V.Statum] ); Print( 2,3,
    "Курс %0.1f°, дрейф %0.1f°, скорость %0.1f узлов (Fr=%4.2f, Lw/L=%4.2f) ",
     V.Course*360/_Pd,angle(V.Course,-V.Head[-1].z)*360/_Pd,S*3600/_Mile,
     S/sqrt(_g*L),sqr(S)*_Pd/_g/L );
  Save().Refresh();                    // перерисовка с наложенными площадками
  return Recurse=false;
}
