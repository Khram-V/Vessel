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
       "  Моделирование"," взаимной динамики","множества активных диполей",0 },
 *Cmds[]={ "<пробел> ","           раскраска диполей",
           "<ctrl>+<пробел> ","   векторы скорости",
           "<shift>+<пробел> ","     объемная сетка",
           " (1-9)^3 ","  количество частиц ^ в кубе",
           "  F4     ","настройка эксперимента",
           "  F1     ","краткая справка",0 },
 *Plus[]={ "<lMouse•rMouse>"," поворот|смещение",
           "<ctrl>+<стрелки>","       ближе\\дальше",
           "<shift>+<стрелки>","        вправо\\влево",
           "<стрелки>","   разворот сцены  OpenGL",
           "<home> ","   восстановление исходного",
           "<Esc>/<ctrlC>","    стоп ",0 };
static
struct Video:     // Контекстная графика OpenGL строится на глубоких логических
public View       // последовательностях, отчего на визуализации затруднительно
{                 // исполнять параллельные алгоритмы, допуская статику данных
 Vector// Centre, // к попутному расчету центра масс всей динамической системы
       Cmin,Cmax; // прямоугольный параллелепипед - вместилище всех корпускул
  Video();        // конструктор автоматически отрабатывает запуск программы
 ~Video(){}       // ~~~ //
 Course Configuration(); // динамическая настройка вычислительного эксперимента
 virtual bool KeyBoard( byte Key ); // к простым и совсем независимым операциям
 virtual bool Draw();     // виртуальная функция рисования без допуска рекурсии
 virtual bool Timer();
 void Draw_space();         // разметка формальной пространственной сетки
} Win;                      // рабочее пространство, отображаемое на экране ЭВМ

Video::Video()                    // начальная настройка контекста среды OpenGL
     : View("Dipoles Array in Space",0,0,1600,1024, // размерения рабочего окна
                    abs( (Vector){nX,nY,nZ} )*5 ){} // исходная дальность сцены
//
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
{ //     WaitTime( 50 );                // приостановка для прохода графики
         Win.SetTimer( Quantum_video ); // интервал перерисовки 5 раз в секунду
  return Win.Ready()!=NULL;
}
//
//! блок памяти и операций интервальной визуализации результатов эксперимента
//  пространство моделирования процессов корпускулярной механики располагается
//  во внешней статической области памяти со свойствами абсолютного глобального
//  базиса, просто использующего для визуализации однородные координаты OpenGL
//
bool Video::KeyBoard( byte key )
{ switch( key )
  { case _Esc: exit( 16 );
    case _F1: Win.Help( Id,Cmds,Plus,-2,2 ); break;
    case _F4: GetKey(); Win.Configuration(); break;
    default : return View::KeyBoard( key );
  } return true;
}
//      Графическия процедура инициируется по таймеру с блокировкой
//      рекурсии и реентерабельности по счетчику RealTime
//      в обще-системном таймере WaitTime
//
bool Video::Timer()
{
//  if( !isTimer )
//  { isTimer=true; bool Res=Draw(); isTimer=false; return Res;
//  } return false;

  return Draw();
}
bool Video::Draw()
{ int i,k,key=GetKey(),                        // запрос key-символа
     stat=ScanStatus();                        // со stat-статусом клавиатуры
  static unsigned vk=0;                        // счетчик изображений на экране
  if( key )switch( key )
  { case ' ': if( stat&SHIFT )ex.Grid++;       // сетка абсолютного базиса
         else if( stat&CTRL )ex.Field++;       // выбор векторных полей
         else ex.Model--; break;               // раскраска или ребра диполя
    default: if( key>='0' && key<='9' )        // количество активных корпускул
       { vk=0; k=key-'0';
         ReInstall_TimeSpace( !k ? 10 : k!=nDip ? k:k*k*k );    // один или куб
         return false;
       }
  }
 static bool Recurse=false;                    // предотвращение рекурсии
// static DWORD Rt=0;             // картинка способна задавить другие вычисления
  if( Recurse )return false;
// || RealTime-Rt<Quantum_video )return false; // --- перестраховка
      Recurse=true;
//    Rt=RealTime;
  wX=minmax( 0.0,fabs( wX ),0.5 ); nX=minmax( 0,nX,12 ); // лишняя проверка
  wY=minmax( 0.0,fabs( wY ),0.5 ); nY=minmax( 0,nY,10 ); // полезна при неявной
  wZ=minmax( 0.0,fabs( wZ ),0.5 ); nZ=minmax( 0,nZ,8  ); // модификации данных
  //
  //  необходимо определиться с размерами и дальностью центра графической сцены
  //
 static Model Mxl;             // модель-пустышка без излишних инициализаций
//int t=Time_count;            // счетчик может нежданно поменяться - не должен
 Real rt=RealTime;
 Dipoles &Dp=Dipoles_array[Time_count%Dipole_route]; // действующий рой частиц
 _Vector Center=Dp.Mean.R;
  for( i=0; i<nDip; i++ )
  { _Vector P=Dp[i].R;
    if( !i )Cmin=Cmax=P; else         // к самым дальним координатам размещения
    { if( Cmin.x>P.x )Cmin.x=P.x; else if( Cmax.x<P.x )Cmax.x=P.x;
      if( Cmin.y>P.y )Cmin.y=P.y; else if( Cmax.y<P.y )Cmax.y=P.y; // эXтремумs
      if( Cmin.z>P.z )Cmin.z=P.z; else if( Cmax.z<P.z )Cmax.z=P.z;
  } } Cmin+=(Vector){-nX,-nY,-nZ}-Center; //   оконтуренные размеры большого
      Cmax+=(Vector){nX,nY,nZ}-Center;    //     графического пространства
  //
  // начальная настройка чистой графической сцены(умолчанием по корабельной СК)
  //
  View::Draw();
  View::Clear(); // glTranslated( Centre.x,Centre.y,Centre.z );
  //
  // контрольные надписи о прохождении вычислительных процессов
  //
  color( navy ),Print( 1,-1," [рис:%u/счет:%u]: T=%s = %1.2f%%",
           ++vk,Time_count,DtoA(rt/3600000.0),rt*100.0/ElapsedTime() );
  //
  // прорисовка всех диполей в центрированном расчётном пространстве
  //
  for( i=0; i<nDip; i++ )
  { Dipole &D=Dp[i];
    Vector P=D.R-Center;
    if( ex.Model )
    { spot( P,12,red );
      if( ex.Model>1 )                  // 0-только трек 1-точка 2-ребра 3-цвет
      { (Mxl=*(Point*)&P).set( D.M );   // местоположение и вектор массы
        Mxl.dipole( 0.0,ex.Model==3 );  // изображение диполя в движении
      }
      arrow( P,P+D.V*EqSphere,0.1,green ); //  и встречной локальной скорости
      arrow( P,P+D.M*EqSphere,0.1,navy );  // стрелка дипольного момента
    }
    if( Time_count<1 )break;             // и если маршрут еще не сформировался
    glBegin( GL_LINE_STRIP ); color( magenta );
    for( k=max( 0,Time_count-Dipole_route+1 ); k<Time_count; k++ )
    { dot( Dipoles_array[k%Dipole_route][i].R-Center );
    } glEnd();
  }
  //  прорисовка векторного пространства скоростей вызванных, суммарных
  //                                             и суммарно-осреднённых
  if( ex.Field )
  { const Real Sc=0.1;                            // масштаб отрисовки векторов
          Real x,y,z,dx=max(.01,wX),dy=max(.01,wY),dz=max(.01,wZ);
    if( wX )if((Cmax.x-Cmin.x)/wX>320)dx=(Cmax.x-Cmin.x)/320; // ускорение
    if( wY )if((Cmax.y-Cmin.y)/wY>320)dy=(Cmax.y-Cmin.y)/320; // прорисовки
    if( wZ )if((Cmax.z-Cmin.z)/wZ>320)dz=(Cmax.z-Cmin.z)/320; // сетки векторов
    color( lightgreen );                                      // контур поля XY
    glBegin( GL_LINE_LOOP );
      dot( (Point){Cmin.x,Cmin.y} ),dot( (Point){Cmax.x,Cmin.y} );
      dot( (Point){Cmax.x,Cmax.y} ),dot( (Point){Cmin.x,Cmax.y} ); glEnd();
    for( z=(wZ?Cmin.z:0); z<=(wZ?Cmax.z:0); z+=dz )  // дипольный
    for( y=(wY?Cmin.y:0); y<=(wY?Cmax.y:0); y+=dy )  // момент по
    for( x=(wX?Cmin.x:0); x<=(wX?Cmax.x:0); x+=dx )  // полуоси х
    { Vector P={x,y,z},V={0},M;                      // точка в сеточном узле
      for( i=0; i<nDip; i++ )                        // действующий рой частиц
      { V+=dipole_v(Dp[i].M*EqSphere,P+Center-Dp[i].R); ///? вызванные скорости
      }                                     // усреднённое  поле полного потока
      if( ex.Field&1 )arrow( P-V*Sc,P+V*Sc,abs(V)*Sc/2,lightblue ); // фоновый
      if( ex.Flow )V-=(Vector){ EqSphere,0,0 }; else                // поток
                   V+=Dp.Mean.V*EqSphere;                           // средний
      if( ex.Field&2 )arrow( P-V*Sc,P+V*Sc,abs(V)*Sc/2,green );     // контроль
  } }
  Draw_space();                               // самоцентрированное пространство
  //
  //                   Информация на графическом поле
  color( ex.Body?red:navy );
  Print( 2,-6,ex.Body?"Динамика корпускул с учётом массы и инерции" :
                      "Чисто кинематическое безынерционное взаимодействие" );
  color( blue );
  Print( 2,-5,"Силовое притяжения: %s ",ex.Edge==1 ? "гладкое" :
                                 ex.Edge==2 ? "упругое" :
                                 ex.Edge==3 ? "гладко-упругое":"отсутствует" );
  Print( 2,-4,"Векторы течений: %s ",ex.Field==1 ? "только вызванные" :
                                 ex.Field==2 ? "видимые полные" :
                                 ex.Field==3 ? "вызванные + видимые" : "нет" );
  Print( 2,-3,ex.Flow ? "Внешний поток поддерживается"
                      : "Окружающий поток свободно изменяется" );
  Print( 2,-2,"R[%d]={%.1f,%.1f,%.1f}",nDip,Center.x,Center.y,Center.z );
  if( Time_count>0 )
  { Dipoles &Dq=Dipoles_array[(Time_count-1)%Dipole_route];// прошлый рой частиц
    Vector V=(Center-Dq.Mean.R)/(Dp.T-Dq.T);
    color( green ); Print( ", V={%.1f,%.1f,%.1f} delta={%.2f,%.2f,%.2f} ",
           V.x,V.y,V.z, V.x-Dp.Mean.V.x,V.y-Dp.Mean.V.y,V.z-Dp.Mean.V.z );
  }
  Save();          //  Show();
  Refresh();
  Text_to_ConIO( Dp ); return Recurse=false;
}
void Video::Draw_space()                   // сетка Эйлерова этапа эксперимента
{ if( ex.Grid&1 )                          //      локально центрируется к нулю
      axis( *this,(Cmax.x-Cmin.x)/2+1,(Cmax.y-Cmin.y)/2+1,(Cmax.z-Cmin.z)/2+1,
             "X","Y","Z",green );
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
  const char *Flow[]={ "нет","вызван","полный","вместе" };
  const char *Edge[]={ "нет","гладкая связь","упругий шарик","шарик+функция" };
  const char *Body[]={ "кинематика поляризованных частиц",
                       "динамика независимых корпускул" };
  const char *exFlow[]={ "свободно изменяется","поддерживается" };
  for( int K=-1; K; )                  // текущая позиция в меню
  { Mlist Menu_C[]=                    // собственно список запросов и настроек
    { { 0,0,"   <<<- видео-конфигурация ->>>" }
    , { 2,0,"Способ изображения корпускулы: " },{ 0,14,GVid[ex.Model] }
    , { 1,0,"Эксперимент: "},{ 0,32,Body[ex.Body] }
    , { 1,0,"Направление потока:  "},{ 0,6,Flow[ex.Field] }
    , { 0,4," =  %4.2lf",&wX },{ 0,4,",%4.2lf",&wY },{ 0,4,",%4.2lf",&wZ }
    , { 1,0,"Рёбра и оси единичной разметки: "},{ 0,5,Grid[ex.Grid] }
    , { 0,2,"%2d",&nX },{ 0,2,",%2d",&nY },{ 0,2,",%2d",&nZ }
    , { 1,0,"Притяжение/отталкивание частиц: " },{ 0,13,Edge[ex.Edge] }
    , { 1,0,"Внешний набегающий поток: " },{ 0,20,exFlow[ex.Flow] } };
    TextMenu T( Mlist( Menu_C ),&Win,-1,2 );
    switch( K=T.Answer( K ) )
    { case  2: ex.Model--; break;
      case  4: ex.Body^=true; break;
      case  6: ex.Field++;    break;
      case 11: ex.Grid++;     break;
      case 16: ex.Edge++;     break;
      case 18: ex.Flow^=true; ReInstall_TimeSpace( nDip ); break;
      case _Esc: return _Esc;
  } } return _F4;
}
