//
//    Моделирование морского волнения на основе групповых (Кельвиновских)
//    структур трохоидальных (Герстнеровских) волн со взаимным наложением
//    крутых гребней ветровых (Wind), свежей зыби (Swell) и реликтовых волн
//      от удаленных ураганов (Surge)
//                                                   ©2018-10-04 ПМ-ПУ СПбГУ
//      Fn = V/sqrt( gL ) - скорость по Фруду
//      C = sqrt( gL/2п ) - фазовая скорость трохоидальной волны
//
#include "Vessel.h"
static Real           // на случай, пока не встроены дисперсионные эффекты волн
     GrWave=8.89*2.0; // количество гребней в структуре одного волнового пакета
void
Waves::iWave( _Real Xdis, // сдвиг-распространение в базисе групповой структуры
              Vector &P,  // локальные координаты внутри моделируемой акватории
              Vector &V ) // скорость частицы жидкости по трохоидальной орбите
{ const Real L=_Pd/Length;                // фазовая длина прогрессивной волны
  if( P.x+Xdis>=0.0 )                     // к диагонали дальнего угла бассейна
  { Real A=Height/2.0,                    // радиус трохоиды - полувысота волны
//      Cr=hW*A/Length,                   // коэффициент максимальной амплитуды
        xW=L*( P.x+Xdis ),                // гармоническое смещение через волну
        xG=L*( P.x+Xdis/2 )/GrWave,       // Lg=2×8,89 с поправкой к цугам волн
        yG=P.y*_Pi/Extend,                // протяжённость фронта девятого вала
        Rx=cos( xG+yG ),                  // ромбические фазовые отсчеты для
        Ry=sin( xG-yG ),                  // формирования волновых цугов со
        x=cos( xW ),y=sin( xW );          // + x,y направляющие ko\синусы
    if( Rx>=0.0 && Ry>=0.0 )xG=x,yG=y; else  // сдвигами фазовых отсчётов по
    if( Rx<0.0 && Ry<0.0 )xG=-x,yG=-y; else  // формулам приведения ко\синусов
    if( Rx>0.0 )xG=-y,yG=x; else xG=y,yG=-x; // и с учётом ориентации движения
   Real R = A * exp( L * ( P.z             // глубина жидкости -> радиус орбиты
          + A * ( xG-1.0 ) ) );            // протяженность групповой структуры
    if( Wind )                             // ветровое напряжение проявляется
        R *= exp( R*L*Wind*( 1-y ) );      // асимметрией подветренной крутизны
        R *= exp( -L*R*hW*A*x/Length )     // корректировка уровня во впадинах
           * fabs( Rx*Ry );                // удлинённость волновых фронтов
    if( xW<_Pd )R *= 0.5-0.5*cos( xW/2 );  // сглаживание первого вступления
        P.z -= ( V.x = -xG*R ); V.y=0.0;   // от исходной прямоугольной сетки
        P.x += ( V.z = -yG*R ); V *= Ow;   // радиус ++Z ? == смещение узлов
} }
Vector Waves::Wave( _Real T,_Vector R )    // на текущее время и местоположение
{ Vector l=Vessel->Locate,w,r; l.z=R.z;    // сводится к повороту в плоскости Z
         iWave( T*Cw-Diagonal,r=( R+l )/Tensor(*this),w );
  return Tensor(*this) * r - l;
}
void Waves::Wave( _Real T, Vector &R, Vector &W ) // и скорость потока на выход
{ Vector l=Vessel->Locate,w,r; l.z=R.z;           // аппликата как бы неизменна
         iWave( T*Cw-Diagonal,r=( R+l )/Tensor(*this),w );
  R = Tensor(*this) * r - l;
  W = Tensor(*this) * w;
}
Vector Waves::Wave( _Real T,int y,int x )       // в отсчетах сеточных индексов
{ return Wave( T,(Vector){ Ds*(x-Mx/2),Ds*(y-My/2),0.0 } );
}
Vector Field::Wave( _Real T, _Vector R )        // новое местоположение частицы
{ Vector W=Wind.Wave( T,R )+Swell.Wave( T,R )+Surge.Wave( T,R )-R*2; return W;
}
Vector Field::WaveV( _Real T,_Vector R,Vector &W ) // местоположение и скорость
{ Vector P=R;
  if( !Exp.wave )W=0; else
  { Vector p,v; Wind.Wave( T,P,W );
               Swell.Wave( T,p=R,v ); P+=p-R; W+=v;
               Surge.Wave( T,p=R,v ); P+=p-R; W+=v;
  } return P;
}
//!  Раздельная визуализация волновых процессов в независимом графическом окне
//      -=- вычисление текущего смещения точки в пространстве в новом времени
//
//#include "../Window/Sym_CCCP.c"
bool Field::Timer()                //! к несчастью все расчеты пока "есть"здесь
{ static volatile int recurse=0,   // статика для обнаружения рекурсии, и
         pause=false,stop=false;   // для задержки и приостановки эксперимента
  int M,D,Y;                       // подпись независима от состояния расчётов
  Event T; T.UnPack( M,D,Y );      // время для обработки прерываний от таймера
  { TextContext Save( true ); color( navy );
    Info.Area( 0,0,60,1 ).Clear().Print( 2,0,"%04d %s %02d, %s%s <= %d",
                                Y,_Mnt[M-1],D,_Day[int(T.D%7)],DtoA(T.T,3),Kt);
    if( pause||stop ){ color(magenta); Info.Print(stop?" <стоп>":" <pause>" ); }
    if( Tlaps*3600-Trun>1.0 ){ color( red );
        Info.Print( " {%s",DtoA( Tlaps,3 ) );    // реальное время эксперимента
        Info.Print("%s}",DtoA(Trun/3600-Tlaps,3)); }// отставание со знаком "-"
    if( recurse ){ color( green ); Info.Print( " %d=затор",recurse ); }
    Info.Save().Show();
  }                            // приостановка <Ctrl> и блокировка <Ctrl+Shift>
  if( pause=ScanStatus()&CTRL ) // вычислительного процесса - до повтора <Ctrl>
    { stop=ScanStatus()&SHIFT; Instant=T-Tlaps; } else
  if( stop )Instant=T-Trun/3600; else// если Ctrl отпущен,то требование полной
                        // остановки зафиксируется после аккорда <Ctrl+Shift>
  if( !recurse++ )     // блокировка случаев скороспешных прерываний от таймера
  { Tlaps=T-Instant;  // отсчеты времени волнового поля фиксируются за надписью
    Simulation();    // собственно вычислительный эксперимент на штормовое море
    recurse=0;      // успешное завершение разблокирует запрос отрисовки экрана
  } return false;   // восстановление картинок производится в обработке таймера
}
//!  Собственно моделирование строится по минимальным интервалам времени
//   по критериям Куранта для свободных волн в полных потоках, либо, если идёт
//   простое кинематическое построение картинок с корабликом на волне,
//   то счёт исполняется чисто в реальном времени
//   ++шаг должен подстраиваться к кинематике волн, корабля и их взаимодействия
//
Field& Field::Simulation()  // моделирование волнения в интервале Trun до Tlaps
{ Real Tnew=Tlaps*3600.0;   //~ в режиме прерываний проводить счет нежелательно
  GrWave=Exp.wave>1?10:17.6; // количество волн для одного структурного цикла
  if( Trun<Tnew )           // на свободной границе нужен точный отсчёт времени
  { Real Tr=Trun;           // начальный отсчёт времени по выходу через секунду
    if( Vessel->Statum || Exp.wave>1 )Ts=Tstep/tKrat;  // гидромеханика корабля
                    else Ts=Tstep/tKrat; //Tnew-Trun;  // или только кинематика
    //
    //   Главный цикл во времени для проведения вычислительного эксперимента
    //
    while( Trun<=Tnew       //! вычисления вдогонку из Trun через Tstep -> Tnew
        && Trun-Tr<=1.0 )   //  случай отставания от реального времени на 1 сек
    { Trun += Ts;           // запоминается точный отсчёт интервала для времени
      if( Exp.wave==1 ){    //! моделирование динамики корабля на м.модели волн
#pragma omp parallel for
        for( int y=0; y<mY; y++ )  // Построение результирующего волнового поля
        for( int x=0; x<mX; x++ )  // без разделения на независимые структуры и
        if( Exp.peak )             // со сдвигами поверхностных частиц жидкости
        { Vector W={ dS*x-Long/2,dS*(y-Real(mY-1)/2),0 };
          Ws[y][x] = Wind.Wave( Trun,W )               // здесь предвычисляются
                  + Swell.Wave( Trun,W ) - W           // координаты всех ячеек
                  + Surge.Wave( Trun,W ) - W;
        } else                        // простые волны в полных потоках и без
        { Vector &W=Ws[y][x]; W.z=0;  // дрейфовых сдвигов поверхностных частиц
          W.z = Wind.Wave( Trun,W ).z
             + Swell.Wave( Trun,W ).z // координаты хранятся в исходном массиве
             + Surge.Wave( Trun,W ).z;
        }
      } else             //! прямой вычислительный эксперимент разделением волн
      if( Exp.wave>1 )   // по всем трём независимым структурам из частиц-ячеек
      { Vector dR={0,0,0};  // продвижение по курсу корабля за один шаг времени
        if( Vessel->Route.len>1 )dR=Vessel->Route[-2]-Vessel->Route[-1];
#pragma omp parallel sections
        {
#pragma omp section             // длиннопериодный накат трансокеанских штормов
          Surge.Solving( Trun,dR );
#pragma omp section             // первая система свежей и наиболее крутой зыби
          Swell.Solving( Trun,dR );
#pragma omp section            // интенсивное волнение с обрушающимися гребнями
          Wind.Solving( Trun,dR );
        }
        if( Exp.wave ){
#pragma omp parallel for
          for( int y=0; y<mY; y++ )         // Общие результаты по уровням моря
          for( int x=0; x<mX; x++ )if( Exp.peak )
          { Vector W={ dS*x-Long/2,dS*(y-Real(mY-1)/2),0 };
              Ws[y][x] = Wind.Amd( W ) + Swell.Amd( W ) + Surge.Amd( W ) - W*2;
          } else
          { Vector &W=Ws[y][x];
                   W.z = Wind.AmH( W ) + Swell.AmH( W ) + Surge.AmH( W );
        } }
      }
      Vessel->Floating()     // наложение уровня + расчёт воздействия на корпус
             .Moving();      // моделирование гидродинамики корабля на волнении
            ++Kt;                // счётчик циклов вычислительного эксперимента
      if( !WinReady(this) )break;// переисполнение очереди посторонних запросов
  } } else WinReady(); return *this;
}
//     Эксперимент выполняется с заданным расчетным шагом до истечения
//     контрольного интервала времени, или до возникновения сигнала
//     о нарушении аппроксимационных критериев, или - приостанавливается
//     в случае возникновении внешних прерываний, с установлением реально
//     достигнутого времени моделирования
Waves&
Waves::Solving( _Real Tr, _Vector dR )    // Время и ранее сделанный шаг в пути
{ //
  //   На новый отсчет времени Trun автоматически смещаются граничные условия,
  //   соответственно желательно сместить волновые поля на продвижение корабля
  //   для разностных расчётов обновляемых волновых полей к тому же времени +Ts
  //
 Real S=Ts,Step=Ts-(dR%x)/Cw;     // с проекцией скорости волны на курс корабля
 int kS=max( 1,(int)fabs( Step/Ts+0.9 ) ); if( kS>1 )S/=kS,Step/=kS;
  for( int i=1; i<=kS; i++ )
   if( Storm->Exp.peak )ThreeBoundary( Tr+i*S,S,Step ),ThreeSurface( Step );
                  else   FreeBoundary( Tr+i*S,S,Step ),FreeSurface( Step );
  return *this;
}
//!    Моделирование поперечных волн на воде простыми волновыми уравнениями
//
Waves& Waves::FreeSurface( _Real Step )  // + поле скорости задается в реальном
{ Real Sigma=_Pd*Cw/Length,              // = волновая частота          [1/сек]       dTs=Cw*Step/Ds/Sigma;             //   масштабе подвижных частиц [м/сек]
#pragma omp parallel for
  for( int x=1; x<Mx; x++ )
  for( int y=1; y<My; y++ )
    H[y][x].z -= dTs*( (H[y+1][x].y-H[y][x].y)+(H[y][x+1].x-H[y][x].x) );
  dTs=Sigma*Cw*Step/Ds;
#pragma omp parallel for
  for( int x=0; x<=Mx; x++ )
  for( int y=0; y<=My; y++ )
  { if( y )H[y][x].y -= dTs*(H[y][x].z-H[y-1][x].z);
    if( x )H[y][x].x -= dTs*(H[y][x].z-H[y][x-1].z);
  } return *this;
}
//     Граничные условия с излучением свободных волн по Зоммерфельду
//
Waves& Waves::FreeBoundary
( _Real Tr, _Real S, _Real Step )         // Обход по периметру акватории для
{ Real Bh=0,B1=H[0][1].z,Dh,Dx,Dy;        // пере-экстраполяции граничных точек
  for( int by=0,bx=0,y=0,x=1;; )          // с привнесением внешнего возмущения
  { int Ys=y>0?-1:0,Yn=y<My?1:0,jy=Ys+Yn, // над градиентом волнового склона
        Xw=x>0?-1:0,Xe=x<Mx?1:0,ix=Xw+Xe;
    Dh=4.0*( Wave( Tr+S/8,y,x ).z-Wave( Tr-S/8,y,x ).z );              // [м]
    Dx=( H[y][Xe+x].z-H[y][x+Xw].z )/( Ds*(Xe-Xw) ) - Tensor::x.x*Dh*Step/S;
    Dy=( H[Yn+y][x].z-H[y+Ys][x].z )/( Ds*(Yn-Ys) ) - Tensor::x.y*Dh*Step/S;
    Dh += Dy*jy + Dx*ix;   // дифференциал с оценкой величины и знака градиента
    H[by][bx].z+=Bh;      // последовательный обход по периметру всей акватории
    by=y,bx=x; Bh=Dh;                                                 // x«———x
    if( !x ){ y--; if( !y ){ Dh=H[0][1].z; H[0][1].z=B1,B1=Dh; } else // ¦    ¦
                   if( y<0 ){ H[0][1].z=B1; break; } } else           // v    ¦
    if( y==My )--x; else                                              // o+——»x
    if( x==Mx )y++; else if( !y )++x;
  } H[0][0].z+=Bh; return *this;
}
//!    Трохоидальное морское волнение штормовой интенсивности
//
Waves& Waves::ThreeSurface( _Real Step ) // + поле скорости задается в реальном
{ Real Sigma=_Pd*Cw/Length,              // = волновая частота          [1/сек]
         dTs=Cw*Step/Ds/Sigma;             //   масштабе подвижных частиц [м/сек]
#pragma omp parallel for
  for( int y=1; y<My; y++ )
  for( int x=1; x<Mx; x++ )
  { H[y][x].z-=dTs*( (V[y+1][x].y-V[y][x].y)+(V[y][x+1].x-V[y][x].x) );
    H[y][x].y-=dTs*( V[y+1][x].z-V[y][x].z );
    H[y][x].x-=dTs*( V[y][x+1].z-V[y][x].z );
  } dTs=Sigma*Cw*Step/Ds;
#pragma omp parallel for
  for( int y=0; y<=My; y++ )
  for( int x=0; x<=Mx; x++ )
  { if( y>0 && x>0 )
      V[y][x].z-=dTs*( (H[y][x].y-H[y-1][x].y-Ds)+(H[y][x].x-H[y][x-1].x-Ds) );
    if( y>0 )V[y][x].y-=dTs*( H[y][x].z-H[y-1][x].z );
    if( x>0 )V[y][x].x-=dTs*( H[y][x].z-H[y][x-1].z );
  } return *this;
}
//const Real tKrat=...;     //! динамическое дробление шага времени управляется
                            //  характером отражения волны от корабля
Waves& Waves::ThreeBoundary
( _Real Tr, _Real S, _Real Step )        // Проход по периметру всей акватории
{ Vector Bh={0,0,0},B1=H[0][1],Dh,Dx,Dy;  // пере-экстраполяция граничных точек
  const Real &tK=Storm->tKrat;     // к поправке дробления шага от Зоммерфельда
  Real kW=Cw*S*tK/Ds/(tK+1);   // скорость волны от скорости пересечения сетки
  for( int bx=0,by=0,j=0,i=1;; )            // по нелинейным эффектам отражения
  { int Ys=j>0?-1:0, Yn=j<My?1:0, jy=Ys+Yn,              // нормаль на выход
        Xw=i>0?-1:0, Xe=i<Mx?1:0, ix=Xw+Xe;             // из свободной границы
    ( Dx=(H[j][Xe+i]-H[j][Xw+i])/(Xe-Xw) ).x-=Ds;      // граничные градиенты
    ( Dy=(H[j+Yn][i]-H[j+Ys][i])/(Yn-Ys) ).y-=Ds;     // по уровням и скоростям
      Dx*=kW;                           // градиент исходящего волнового склона
      Dy*=kW;                           // с учетом поправки граничному Куранту
//  if( abs( jy )!=1 || abs( ix )!=1 )                   // сброс угловых точек
//  if( (i==0 && x.x<=0.0) || (i==Mx && x.x>=0.0)
//   || (j==0 && x.y<=0.0) || (j==My && x.y>=0.0) )
    { Dh = ( Wave(Tr+S/6,j,i)-Wave(Tr-S/6,j,i) )*3.0; // входящая волна [м]
      Dx -= Dh*(x.x*Step/S);                           // прямая корректировка
      Dy -= Dh*(x.y*Step/S);                           // по скорости хода
      Dh.y = x.y*Dh.x;                                  // и направление волны
      Dh.x *= x.x;                                      // без переориентации
    } // else Dh=0.0;
    Dh += Dx*ix+Dy*jy;                                                   // [м]
    H[by][bx] += Bh;           // последовательный обход по периметру акватории
    by=j,bx=i; Bh=Dh;          //    -- уточнение [0,0] по выходу --     x«———x
    if( !i ){ j--; if( !j ){ Dh=H[0][1]; H[0][1]=B1,B1=Dh; } else //     ¦    ¦
                   if( j<0 ){ H[0][1]=B1; break; } } else         //     v    ¦
    if( j==My )--i; else                                          //     o+——»x
    if( i==Mx )j++; else if( !j )++i;
  } H[0][0]+=Bh; return *this;
}
//! Внутренний поисковый алгоритм для уроня моря на неравномерных сетках
//
static void APoint( Point &A, int &y,int &x, Vector **H, int My,int Mx )
{ Real Rx,Ry,Sx,Sy;
  int i,j,ix=0,jy=0,k;
  for( k=0; k<24; k++ )                         // ведется поиск до возврата
  { Rx=A.X-H[y][x].x,Ry=A.Y-H[y][x].y;          // к любому предыдущему узлу
    Sx=A.X-H[y][x+1].x,Sy=A.Y-H[y+1][x].y;      //
    if( Rx<0 && x>0 )i=-1; else if( Sx>0 && x<Mx-1 )i=1; else i=0;
    if( Ry<0 && y>0 )j=-1; else if( Sy>0 && y<My-1 )j=1; else j=0;
    if( i && j ){ if( (i<0?-Rx:Sx)>=(j<0?-Ry:Sy) )j=0; else i=0; }
    if( i*ix<0 || j*jy<0 )break;
    if( i )ix=i,x+=i; else if( j )jy=j,y+=j; else break;
  } A.Y=Ry/( H[y+1][x].y-H[y][x].y );
    A.X=Rx/( H[y][x+1].x-H[y][x].x );
}
//!  Простая билинейная интерполяция волнового поля для одной конкретной точки
//                                 \!-и должно оптимизироваться для всего поля
Real Field::Value( Point A )
{ Real Rx=(A.X+Long/2)/dS,
       Ry=A.Y/dS+(mY-1)/2;
  int x=minmax( 0,int( floor( Rx ) ),mX-2 ),
      y=minmax( 0,int( floor( Ry ) ),mY-2 );
  if( Exp.peak )APoint( A,y,x,Ws,mY,mX ); else A.X=Rx-x,A.Y=Ry-y;
  return (1-A.X)*(Ws[y][x].z*(1-A.Y)+Ws[y+1][x].z*A.Y) // простые разности
      + A.X*(Ws[y][x+1].z*(1-A.Y)+Ws[y+1][x+1].z*A.Y); // билинейных пересчётов
}
//! дополнение морских координат уровнем поверхности воды
//
#if 1
Vector Field::Locas( Point P ){ P.Z = Value( P ); return *(Vector*)(&P); }
#else
Vector Field::Locas( Point P )
{ Vector &W=*(Vector*)(&P); W.z=0;
  W.z = Wind.Wave( Tlaps*3600,W ).z
     + Swell.Wave( Tlaps*3600,W ).z   // координаты хранятся в исходном массиве
     + Surge.Wave( Tlaps*3600,W ).z;
  return W;                           //
}
#endif
//
//! Билинейная интерполяция по неравномерной сетке к новой волновой поверхности
//
Vector Waves::Amd( _Vector A )          // интерполяция на волновой поверхности
{ Real Rx=Mx/2+A.x/Ds,                  // выборка индексов и отступов от них
       Ry=My/2+A.y/Ds;                  // с учетом дрейфовых смещений ячеек
  int x=minmax( 0,(int)floor(Rx),Mx-1 ); Rx-=x; // поиск индексов ближней точки
  int y=minmax( 0,(int)floor(Ry),My-1 ); Ry-=y; // внутри оконтуривающей ячейки
  return ( H[y][x]*(1-Ry)+H[y+1][x]*Ry )*(1.0-Rx)      // простые разности
       + ( H[y][x+1]*(1-Ry)+H[y+1][x+1]*Ry )*Rx;       // билинейных пересчётов
}
Real Waves::AmH( _Vector A )            // интерполяция на волновой поверхности
{ Real Rx=Mx/2+A.x/Ds,                  // выборка индексов и отступов от них
       Ry=My/2+A.y/Ds;                  // с учетом дрейфовых смещений ячеек
  int x=minmax( 0,(int)floor(Rx),Mx-1 ); Rx-=x; // поиск индексов ближней точки
  int y=minmax( 0,(int)floor(Ry),My-1 ); Ry-=y; // внутри оконтуривающей ячейки
  return ( H[y][x].z*(1-Ry)+H[y+1][x].z*Ry )*(1.0-Rx)  // простые разности
       + ( H[y][x+1].z*(1-Ry)+H[y+1][x+1].z*Ry )*Rx;   // билинейных пересчётов
}
/*
#if 1
static void Square( Vector** V, Vector N, int y,int x )
{ N.z=0.0;
  // if( abs( N )<eps )N=1; N=1;
  // N /= abs( N );
  // Vector& operator &= ( Vector d ){ return *this = d *= x*d.x+y*d.y+z*d.z; }
  // V[y][x]&=N,V[y][x+1]&=N,V[y+1][x]&=N,V[y+1][x+1]&=N;
  V[y][x]    = N * ( N%V[y][x]     );
  V[y][x+1]  = N * ( N%V[y][x+1]   );
  V[y+1][x]  = N * ( N%V[y+1][x]   );
  V[y+1][x+1]= N * ( N%V[y+1][x+1] );
}
int Waves::Slick( _Point A, _Point B, _Vector N )
{ Vector w; //D=dir( B-A );
  int y,x,dy,dx,ly,lx,k;                        // выбор маски местоположения
  Real Rx=Mx/2+A.X/Ds,Ry=My/2+A.Y/Ds;           //    и отражающей способности
   x=minmax( 0,(int)floor( Rx ),Mx );           // поиск индексов ближней точки
   y=minmax( 0,(int)floor( Ry ),My );           // внутри оконтуривающей ячейки
   APoint( w=*((Vector*)(&A)),y,x,H,My,My );

   Rx=Mx/2+B.X/Ds,Ry=My/2+B.Y/Ds;
   dx=minmax( 0,(int)floor( Rx ),Mx );          // поиск индексов ближней точки
   dy=minmax( 0,(int)floor( Ry ),My );          // внутри оконтуривающей ячейки
   APoint( w=*((Vector*)(&B)),dy,dx,H,My,My );
                                            // AVid( w=*((Vector*)(&A)),y,x );
                                           // AVid( w=*((Vector*)(&B)),dy,dx );
   ly=( dy-=y )<0 ? -1:1;
   lx=( dx-=x )<0 ? -1:1;
   if( dx*lx>dy*ly )for( k=0; k<dx*lx; k++ )Square( V,N,y+(lx*k*dy)/dx,x+lx*k );
              else  for( k=0; k<dy*ly; k++ )Square( V,N,y+ly*k,x+(ly*k*dx)/dy );
   return k;
}
#else
int Waves::Slick( _Vector D, int y,int x,int dy,int dx )
{ if( dy>y )++dy; if( y>dy )++y; int ly=( dy-=y )<0 ? -1:1;
  if( dx>x )++dx; if( x>dx )++x; int lx=( dx-=x )<0 ? -1:1,k; //D.z=0.0;
  if( dx*lx>dy*ly )for( k=0; k<dx*lx; k++ )V[y+(lx*k*dy)/dx][x+lx*k]%=D;
             else  for( k=0; k<dy*ly; k++ )V[y+ly*k][x+(ly*k*dx)/dy]%=D;
  return k;
}
int Waves::Slick( _Vector D, int y,int x,int dy,int dx )
{  int ly=( dy-=y )<0 ? -1:1,
       lx=( dx-=x )<0 ? -1:1,k;
   if( dx*lx>dy*ly )for( k=0; k<dx*lx; k++ )Square( V,D,y+(lx*k*dy)/dx,x+lx*k );
              else  for( k=0; k<dy*ly; k++ )Square( V,D,y+ly*k,x+(ly*k*dx)/dy );
   return k;
}
void Field::Slick( _Vector A, _Vector B, _Vector D )
{ int iy,ix,jy,jx; Vector V=dir( B-A ),a,b; V.z=0.0;
       Wind.AVid( a=A,iy,ix ).AVid( b=B,jy,jx ), Wind.Slick( V,iy,ix,jy,jx );
      Swell.AVid( a=A,iy,ix ).AVid( b=B,jy,jx ),Swell.Slick( V,iy,ix,jy,jx );
      Surge.AVid( a=A,iy,ix ).AVid( b=B,jy,jx ),Surge.Slick( V,iy,ix,jy,jx );
}
#endif
*/
