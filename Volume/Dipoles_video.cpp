
/**
 **     Моделирование кинематики, динамики и механики взаимодействия
 **     множества поляризованных частиц первого и второго порядка
 **     (пространственные вихреисточники и диполи)
 **                                                  (c)2021 ‏יְרוּשָׁלַיִם
 **/
#include "Dipoles.h"
#include "../Window/View.h"
#include "../Window/ConIO.h"

struct status ex;                  // блок ключей вычислительного эксперимента
static int  nX=6,nY=5,nZ=4;        // исходная размерность графической сетки
static Real wX=0.1,wY=0.1,wZ=0.0;  // пространственный шаг векторных отметок
static const char
 *Id[]={ "Dipole",
   "  Моделирование"," взаимной динамики"," множества активных диполей ",0 },
 *Cmds[]={
           "<пробел> ","   раскраска и вид диполей",
           "<ctrl>+<пробел> ","   векторы скорости",
           "<shift>+<пробел> ","    объемная сетка",
           "<pgup/pgdn>","±10% наложенной скорости",
//         "<enter> ","  наложение внешнего потока",
           "<tab> ","трансформация облика в потоке",
           "<backSpace>"," встряхнуть вразнос r±5%%",
           " (1-9)^3 ","количество частиц ^ в кубе",
           "  F4     ","настройка эксперимента",
           "  F1     ","краткая справка",0 },
 *Plus[]={ "<lMouse | rMouse>","  поворот|смещение",
           "<ctrl>+<стрелки>","       ближе\\дальше",
           "<shift>+<стрелки>","      вправо\\влево",
           "<стрелки>","    разворот сцены  OpenGL",
           "<home> "," приведение к исходному виду",
           "<Esc>/<ctrlC>","  стоп ",0 };
static
struct Video:     // Контекстная графика OpenGL строится на глубоких логических
public View       // последовательностях, отчего на визуализации затруднительно
{                 // исполнять параллельные алгоритмы, допуская статику данных
 Vector// Centre, // к попутному расчету центра масс всей динамической системы
       Cmin,Cmax; // прямоугольный параллелепипед - вместилище всех корпускул
  Video();        // конструктор автоматически отрабатывает запуск программы
 ~Video(){}       // ~~~ //
 Course Configuration(); // динамическая настройка вычислительного эксперимента
 virtual bool Draw();     // виртуальная функция рисования без допуска рекурсии
 void Draw_space();             // разметка формальной пространственной сетки
// virtual bool KeyBoard( fixed Key );// к простым и совсем независимым операциям
// virtual bool Timer();
} Win;                          // рабочее пространство, отображаемое на экране

Video::Video():                 // начальная настройка контекста среды OpenGL
  View( "Dipoles Array in Space",0,0,1200,800,    // размерения рабочего окна
  abs( (Vector){nX,nY,nZ})*5 )                   // исходная отдалённость сцены
  { Icon("Math"); AlfaVector( 15 );
  }
//!  main::VideoStage - процедура прерывает основной вычислительный процесс
//   на указанный интервал времени в миллисекундах, например, задаваемый
//   в долях от исполняемого такта вычислительного эксперимента.
//!  Устанавливается активное ожидание с фоновой поддержкой всех процедур
//   визуализации и внешнего управления программой по прерываниям от
//   клавиатуры или курсора.
//!  Здесь же целесообразно проводить перенастройку параметров численных
//   моделей и методов визуализации, допуская возможность полного
//   перезапуска всего вычислительного эксперимента.
//
bool VideoStage()
{ int i,k,stat;
  if( k=Win.GetKey() )                    // запрос key-символа
  {  stat=Win.ScanStatus();                 // со stat-статусом клавиатуры
    switch( k )
    { case _Tab: ex.Size^=true; break;      // динамическая внешность корпускул
//    case _Enter: ex.Flow^=true; break;    // наложение поступательного потока
      case ' ': if( stat&SHIFT )ex.Grid++;  // большая сетка абсолютного базиса
         else if( stat&CTRL )ex.Field++;    // выбор полных векторных полей
         else ex.Model--; break;            // раскраска, ребра и центр диполя
      case _Esc: Win.Close();
      case _BkSp:{ Dipoles &D=Dipoles_array[Time_count%Dipole_route];
                   const Real r=RAND_MAX/2;
                   for( i=0; i<nDip; i++ )
                   { Vector V={ rand()-r,rand()-r,rand()-r };
                     if( !(stat&SHIFT) )V/=20.0; D[i].R+=V/r;      // 0.05 = 5%
                 } } break;
      case _PgDn: ExtFlow=min( 1.0,ExtFlow+0.1 ); break;  // встречное течение
      case _PgUp: ExtFlow=max( -1.0,ExtFlow-0.1 ); break; // или задом наперёд
      case _F1: Win.Help( Id,Cmds,Plus,-2,2 );     break;
      case _F4: Win.GetKey(); Win.Configuration(); break;
     default: if( k>='0' && k<='9' )           // количество активных корпускул
      { k-='0'; ReInstall_TimeSpace( !k?10:k!=nDip ? k:k*k*k ); // один или куб
    } }
  }
  if( WinReady() )                   // Ready -> c исполнением Windows запросов
  { static unsigned T=0,T1; T1=ElapsedTime(); //GetTime();
    if( T1-T>Quantum_video && Time_count>Video_count*2 ){ Win.Draw(); T=T1; } return true;
  } else return false;
}
//! блок памяти и операций интервальной визуализации результатов эксперимента
//  пространство моделирования процессов корпускулярной механики располагается
//  во внешней статической области памяти со свойствами абсолютного глобального
//  базиса, просто использующего для визуализации однородные координаты OpenGL
//
//    Графическия процедура инициируется по таймеру с блокировкой рекурсии и
//  реентерабельности по счетчику RealTime в обще-системном таймере WaitTime
//
bool Video::Draw()
{ static bool Recurse=false;
  if( Recurse )return false;
      Recurse=true;

  wX=minmax( 0.0,fabs( wX ),0.5 ); nX=minmax( 0,nX,12 ); // лишняя проверка
  wY=minmax( 0.0,fabs( wY ),0.5 ); nY=minmax( 0,nY,10 ); // полезна при неявной
  wZ=minmax( 0.0,fabs( wZ ),0.5 ); nZ=minmax( 0,nZ,8  ); // модификации данных
  //
  //  необходимо определиться с размерами и дальностью центра графической сцены
  //
 static Model Mxl;             // модель-пустышка без излишних инициализаций
 Dipoles &Dp=Dipoles_array[Time_count%Dipole_route]; // действующий рой частиц
 _Vector Center=Dp.Mean.R;
  for( int i=0; i<nDip; i++ )
  { _Vector P=Dp[i].R;
    if( !i )Cmin=Cmax=P; else         // к самым дальним координатам размещения
    { if( Cmin.x>P.x )Cmin.x=P.x; else if( Cmax.x<P.x )Cmax.x=P.x;
      if( Cmin.y>P.y )Cmin.y=P.y; else if( Cmax.y<P.y )Cmax.y=P.y; // эXтремумs
      if( Cmin.z>P.z )Cmin.z=P.z; else if( Cmax.z<P.z )Cmax.z=P.z;
  } } Cmin+=(Vector){-nX,-nY,-nZ}-Center; //   оконтуренные размеры большого
      Cmax+=(Vector){ nX, nY, nZ}-Center; //     графического пространства
  //
  // начальная настройка чистой графической сцены(умолчанием по корабельной СК)
  //
  View::Draw();
  View::Clear(); // glTranslated( Centre.x,Centre.y,Centre.z );
  //
  // контрольные надписи о прохождении вычислительных процессов
  //
 Real rt=RealTime,dM;                               // время и размер корпускул
  color( gray ),Print( 2,-1,"Ч:%d [рис:%u/счет:%u]: T=%s = %1.2f%% <== %g ",
     nDip,++Video_count,Time_count,DtoA(rt/3600000.0),rt*100.0/ElapsedTime(),
     Dipoles_array[Time_count%Dipole_route].T );
  Title( _Format( "Ч:%d [рис:%u/счет:%u]: T=%s = %1.2f%%    ==  %ld тики <== %g",
     nDip,Video_count,Time_count,DtoA(rt/3600000),rt*100/ElapsedTime(),RealTime,
     Dipoles_array[Time_count%Dipole_route].T ) );
  //
  //  прорисовка векторного пространства скоростей вызванных, суммарных
  //                                             и суммарно-осреднённых
  if( ex.Field )
  { const Real Sc=0.12;                           // масштаб отрисовки векторов
          Real x,y,z,dx=max(.01,wX),dy=max(.01,wY),dz=max(.01,wZ);
    if( wX )if((Cmax.x-Cmin.x)/wX>320)dx=(Cmax.x-Cmin.x)/320; // ускорение
    if( wY )if((Cmax.y-Cmin.y)/wY>320)dy=(Cmax.y-Cmin.y)/320; // прорисовки
    if( wZ )if((Cmax.z-Cmin.z)/wZ>320)dz=(Cmax.z-Cmin.z)/320; // сетки векторов
    color( lightgreen );                                      // контур поля XY
    glBegin( GL_LINE_LOOP );
      dot( (Point){Cmin.x,Cmin.y} ),dot( (Point){Cmax.x,Cmin.y} );
      dot( (Point){Cmax.x,Cmax.y} ),dot( (Point){Cmin.x,Cmax.y} ); glEnd();
    if( ex.Edge )
    for( z=(wZ?Cmin.z:0); z<=(wZ?Cmax.z:0); z+=dz )   // дипольный
    for( y=(wY?Cmin.y:0); y<=(wY?Cmax.y:0); y+=dy )   // момент по
    for( x=(wX?Cmin.x:0); x<=(wX?Cmax.x:0); x+=dx )   // полуоси х
    { Vector P={x,y,z},V={0},M;                       // точка в сеточном узле
      for( int i=0; i<nDip; i++ )                     // действующий рой частиц
      { V+=dipole_v(Dp[i].M*EqSphere,P+Center-Dp[i].R); ///? вызванные скорости
      }                                     // усреднённое  поле полного потока
      if( ex.Field&1 )arrow( P-V*Sc,P+V*Sc,0.25,blue );       // фоновый
// !! if( ex.Flow )V-=(Vector){ ExtFlow*EqSphere,0,0 };       // поток
//            else V+=Dp.Mean.V*EqSphere;                     // средний
                   V+=(Vector){ ExtFlow };                    // заданный
      if( ex.Field&2 )arrow( P-V*Sc,P+V*Sc,0.25,lightgreen ); // для контроля
    }
  }
  // прорисовка всех диполей в центрированном расчётном пространстве
  //
  for( int i=0; i<nDip; i++ )
  { const Dipole &D=Dp[i]; Vector P=D.R-Center;
    if( ex.Model ){ spot( P,12,red );
      glLineWidth( 2 );
      arrow( P,P+D.M*EqSphere,0.25,navy  );     // стрелка дипольного момента и
      arrow( P,P+D.V*EqSphere,0.25,green );     // встречной локальной скорости
      arrow( P+D.W*EqSphere,P,0.25,lightred );  // встречной локальной скорости
//    arrow( P-D.M*EqSphere/2,P+D.M*EqSphere/2,0.25,navy  ); // стрелка дипольного момента
//    arrow( P-D.V*EqSphere/2,P+D.V*EqSphere/2,0.25,green ); // и встречной локальной скорости
      glLineWidth( 1 );
      if( ex.Model>1 )                  // 0-только трек 1-точка 2-ребра 3-цвет
      { (Mxl=*(Point*)&P).set( D.M );   // местоположение и вектор массы
        if( ex.Size )dM=1.0-abs(D.V)/abs(D.M); else dM=0.0;
        Mxl.dipole( dM,ex.Model==3 ); // изображение диполя в движении
      }
    }
    if( Time_count<1 )break;             // и если маршрут еще не сформировался
    glBegin( GL_LINE_STRIP ); color( lightmagenta );
    for( int k=max( 0u,Time_count-Dipole_route+1 ); k<Time_count; k++ )
    { dot( Dipoles_array[k%Dipole_route][i].R-Center );
    } glEnd();
  }
  Draw_space();                              // самоцентрированное пространство
  //
  //                   Информация на графическом поле
  //
  color( ex.Body?red:navy );
  Print( 2,-5,ex.Body?"Динамика корпускул с учётом массы и инерции\n" :
                      "Чисто кинематическое безынерционное взаимодействие\n" );
  color( blue );
  Print( "Ориентация по поляризации: %s",ex.Edge==1 ? "линейный сброс" :
                                         ex.Edge==2 ? "единичный поток" :
                                         ex.Edge==3 ? "удвоение в центре" :
                                         ex.Edge==4 ? "сглаживание к нулю" : "нет" );
  Print("\nСиловое взаимопритяжение: %s",ex.Grav==1 ? "сопряженный потенциал" :
                                         ex.Grav==2 ? "упругое отталкивание" :
                                         ex.Grav==3 ? "гладкое+упругое расталкивание":"отсутствует" );
  Print("\nВнешние поля: %s ≈ %s %5.2f ",ex.Field==1 ? "только вызванные" :
                                         ex.Field==2 ? "видимые полные" :
                                         ex.Field==3 ? "вызванные + видимые" : "нет",
             fabs(ExtFlow)<eps?"без обтекания":"набегающий поток=",-ExtFlow );
//Print( 2,-3,ex.Flow ? "Внешний поток поддерживается"
//                    : "Окружающий поток свободно изменяется" );
  Print("\nR={%.1f,%.1f,%.1f}",Center.x,Center.y,Center.z );
  if( Time_count>0 )
  { Dipoles &Dq=Dipoles_array[(Time_count-1)%Dipole_route];// прошлый рой частиц
    Vector V=(Center-Dp.Mean.R)/(Dp.T-Dq.T);
    color( green ); Print( ", V={%.1f,%.1f,%.1f} delta={%.2f,%.2f,%.2f} ",
           V.x,V.y,V.z, V.x-Dp.Mean.V.x,V.y-Dp.Mean.V.y,V.z-Dp.Mean.V.z );
  }
  Show();
  Save(); Refresh();
  Text_to_ConIO( Dp ); //   WaitKey();
  return Recurse=false;
}
void Video::Draw_space()                   // сетка Эйлерова этапа эксперимента
{ if( ex.Grid&1 )                          //      локально центрируется к нулю
    axis( *this,(Cmax.x-Cmin.x)/2+1,(Cmax.y-Cmin.y)/2+1,(Cmax.z-Cmin.z)/2+1,"X","Y","Z",cyan );
//  axis( *this,nX,nY,nZ,"X","Y","Z",red ); //green );
  //
  //     рёбра трёхмерной пространственной сетки (немного устаревшее будущее)
  //
  if( ex.Grid&2 )
  { int x,y,z;
    color( lightgray,0,0.3 );
    for( z=-nZ; z<=nZ; z++ )
    for( y=-nY; y<=nY; y++ )if(y||z)line((Vector){-nX,y,z},(Vector){nX,y,z});
    color( lightblue,0,0.3 );
    for( y=-nY; y<=nY; y++ )
    for( x=-nX; x<=nX; x++ )if(y||x)line((Vector){x,y,-nZ},(Vector){x,y,nZ});
    color( lightgreen,0,.3 );
    for( x=-nX; x<=nX; x++ )
    for( z=-nZ; z<=nZ; z++ )if(x||z)line((Vector){x,-nY,z},(Vector){x,nY,z});
  }
}
Course Video::Configuration()        // таблица запросов настройки эксперимента
{ const char *Grid[]={ "нет","оси","сетка","всё" };
  const char *GVid[]={ "только маршрут","красная точка","контуры","диполь" };
  const char *Edge[]={ "нет","линейное сопряжение","единичный поток","удвоенное ядро","с нулём в центра" };
  const char *Grav[]={ "нет","гладкий потенциал","упругое сталкивание","шарик+функция (/60)" };
  const char *Field[]={ "нет","вызван","полный","вместе" };
  const char *Body[]={ "~кинематика поляризованных частиц",
                       "~динамика независимых корпускул" };
//const char *exFlow[]={ "свободно изменяется","поддерживается извне" };
  int K=-1,N=nDip;
  do                                   // текущая позиция в меню
  { Mlist Menu_C[]=                    // собственно список запросов и настроек
    { { 0,0,"   <<<- видео-конфигурация ->>>" }
    , { 2,0,"Тип корпускулы: "},{0,15,GVid[ex.Model]},{0,3," количество %d",&N} // 1-3
    , { 1,0,"Эксперимент: "},{ 0,32,Body[ex.Body] }                             // 4-5
    , { 1,0,"Отображение потока:  "},{ 0,6,Field[ex.Field] }                     // 6-7
    , { 0,4," =  %4.2lf",&wX },{ 0,4,",%4.2lf",&wY },{ 0,4,",%4.2lf",&wZ }      // 8-10
    , { 1,0,"Рёбра и оси единичной разметки: "},{ 0,5,Grid[ex.Grid] }           // 11-12
    , { 0,2,"%2d",&nX },{ 0,2,",%2d",&nY },{ 0,2,",%2d",&nZ }                   // 13-15
    , { 1,0,"Электрополяризация диполя: " },{ 0,15,Edge[ex.Edge] }              // 16-17
    , { 1,0,"Гравитационное притяжение: " },{ 0,15,Grav[ex.Grav] } };           // 18-19
//  , { 1,0,"Внешний набегающий поток: " },{ 0,20,exFlow[ex.Flow] } };          // 20-21
    TextMenu T( Mlist( Menu_C ),&Win,-1,2 );
    switch( K=T.Answer( K ) )
    { case  2: ex.Model--;     break; // картинка для корпускулы
      case  5: ex.Body^=true;  break; // инерционная динамика/чистая кинематика
      case  7: ex.Field++;     break; // варианты отображения наведённых полей
      case 12: ex.Grid++;      break; // запросы для прорисовки осевой разметки
      case 17: (++ex.Edge)%=4; break; // ориентация поляризованных корпускул
      case 19: ex.Grav++;      break; // действие гравитационного притяжения
//    case 21: ex.Flow^=true; /*ReInstall_TimeSpace( nDip ); */ break;
      case _Esc: return _Esc;
    } Draw();
  } while( N==nDip );             // Break( "~N=%d,nDip=%d",N,nDip );
  ReInstall_TimeSpace( N );       // VideoStage();
  return _Center;
}
