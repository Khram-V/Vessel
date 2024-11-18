/**
 *      Пакет физико-математических программ с использованием языка
 *      программирования С++ и графических пакетов Window-Place + OpenGL
 *      (c)2010-2016, В.Храмушин, Сахалинский & Санкт-Петербургский университет
 **/
#include <GL\GLu.h>
#include "..\Math\Model.h"
#include "..\Math\Volume.h"
#include "..\Window\View.h"
//                    Блок анализа и настройки управляющих параметров программы
Real Init_Flight_Points( Array& );           // на выходе размер ячейки-частицы
//              Силовые реакции и приращение скорости при взаимодействии частиц
void One_Inter_Step_of_Point( Array&,bool );
//           Блок расчета кинематики частицы под действием внешних сил и трения
void One_Flight_Step_of_Point( Array&,int );
//
//                                  Пространственные размерности видимого поля
static const Real vX=16,vY=16,vZ=16,Screen_Size=max( max( vX,vY ),vZ );   // 4;
static View Win                     // пространство, отображаемое на экране ЭВМ
 ( "Моделирование корпускулярных взаимодействий",
   0,0,1600,1000,Screen_Size*2 );
//
//    Объединение геометрических фигур для динамического выбора и перенастройки
//
static enum Model_Type{ None,Cube,Torus,Sphere,Dipole } MTyp=Dipole;
static void Model_Draw( Model_Type typ );       // Простые равнообъёмные фигуры
static Array Fly;                 // собственно свора поляризованных корпускул
static Model Mxl;                 // собственный базис геометрического объекта
//
//    Определение основных рабочих параметров
//
int  kT=-1;                      // расчётные\текущие индексы шагов во времени
Real ks=0.4,                     // коэффициент сопротивления для шара[0.2-0.4]
     wall=2; //1.32,             // стенки аквариума (пока бесконечно высокого)
bool Gravity_mode=false,         // свободное/гравитационное перемещение частиц
     linear_Model=false;         // линейно-упругая или ядерная высокой энергии
                                 // модель взаимодействия свободных частиц
const int Fly_Count=1440;        // количество шагов на видимой траектории
static bool painting=true,       // объемная или контурная прорисовка фигуры
            expMoving=false;     // вращение всего пространства сцены в OpenGL
static int  Draw_flow = 3;       // 0-нет; 1-вызванное; 2-полное; 3-вместе
static Real Dipole_offset=0.0,   // скоростная деформация диполя
            Particle_volume=1.0, // контрольный объем моделируемой частицы
            fx=0.25,fy=0.25,fz=0;// пространственный щаг векторных разметок
static Vector Mc={ 0,0,0 };     // текущий сдвиг центра масс для всех корпускул

static Course Configuration()
{ const char *styp[]={ "None","Cube","Torus","Sphere","Dipole" },
             *flow[]={ " нет ","вызван","полная","вместе" };
  static int K=1;                       // текущая позиция в меню
         int Typ=MTyp;                  // тип изображаемого объекта
  Mlist Menu_C[]=                       // собственно меню запросов и настроек
   { { 0,0,"   <<<- видео-конфигурация ->>>" }
   , { 2,0,"Рисуемый тип корпускулы: " },{ 0,6,styp[Typ] }
   , { 1,0,"Скорость: "},{ 0,4,flow[Draw_flow] }
   , { 0,3," = %3.1lf",&fx },{ 0,3,",%3.1lf",&fy },{ 0,3,",%3.1lf",&fz }
   };
  TextMenu T( Mlist( Menu_C ),&Win,-1,1 );
  switch( K=T.Answer( K ) )
  { case 2: if( ++Typ>Dipole )Typ=None; MTyp=Model_Type( Typ ); break;
    case 4: --Draw_flow&=3; break;
    case _Esc: return _Esc;
  }
  fx=min( 1.0,fabs( fx ) );
  fy=min( 1.0,fabs( fy ) );
  fz=min( 1.0,fabs( fz ) ); return _F2;
}
bool display()
{ //Win.Draw();                                  // установка стандартной сцены
  glClearColor( expMoving?0.6:0.8,0.9,0.9,0.9 );
  glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
//
//  Подпись к желаемому действу и разметка полных и вызванных скоростей течений
//
  if( !Fly.nc )
  color( green ),Win.Print( 1,-1,"Simple OpenGL demonstration" ); else
  color( black ),Win.Print( 1,-1,"%+d = ",kT ),
  color( blue ),Win.Print( "%s-%s(%0.3g){ %d } by %s - vsc( %3.1f ) ... ",
      Gravity_mode?"Gravity":"Flying",
      (const char*[]){ "*","Куб","Тор","Шар","Диполь"}[MTyp],
      Particle_volume,Fly.nc,linear_Model?"solid":"edges",ks );
  if( Draw_flow && Fly.nc>0 )
  { Real Sc=hypot( fx,fy );                   // масштаб отрисовки векторов
    Vector V;                      // накапливается усреднённое поле скорости
  //if( MTyp==Sphere || MTyp==Dipole )Mxl*=EqSphere; else (Tensor)Mxl/=2.0;
    if( Draw_flow&3 )
    for( Real z=fz?-vZ:0; z<=(fz?vZ:0); z+=max(0.1,fz) )  // дипольный момент
    for( Real y=fy?-vY:0; y<=(fy?vY:0); y+=max(0.1,fy) )  //  по полуоси от х
    for( Real x=fx?-vX:0; x<=(fx?vX:0); x+=max(0.1,fx) )
    { Point P={ x,y,z }; int k; P+=Mc;                 // точка в сеточном узле
      if( Draw_flow&1 )
      { for( k=0,V=(Vector){0,0,0}; k<Fly.nc; k++ )
        { Ball &B=Fly.Dip[k];
          V+=dipole_v( B.M*EqSphere,P-B.R );              // вызванные скорости
        } V/=Fly.nc; arrow( P-V*Sc,P+V*Sc,0.05,purple );
      }
      if( Draw_flow&2 )
      { for( k=0,V=(Vector){0,0,0}; k<Fly.nc; k++ )
        { Ball &B=Fly.Dip[k];
          V+=dipole_flow( B.M*EqSphere,P-B.R );       // суммарный полный поток
        } V/=Fly.nc; arrow( P-V*Sc,P+V*Sc,0.1,green );
    } }
  } //Win.Show();
//
//  прорисовка корпускул со следами траекторий, и стенок ящика [XY] ограничений
//
  if( !Fly.nc )
  { if( expMoving )
    { Real angle = (ElapsedTime()/120)%360;
      glRotated( angle,angle/360,angle/180,angle/90 );  // проворот всей сцены
      angle *= M_PI/180.0;                              // градусы -> в радианы
      Mxl += (Vector){ sin(angle),cos(angle),0 }/60;    // плоское перемещение
    } Model_Draw( MTyp );
  } else                       // динамическое уточнение общего центра масс для
  { Mc=0.0;                    // всего конгломерата поляризованных коропускул
//#pragma omp parallel for
    for( int j=0; j<Fly.nc; j++ )
    { Ball &B=Fly.Dip[j]; Mxl.set( B.M );
      Point R=B.R; Mxl=R; Model_Draw( MTyp ); Mc+=(Vector){ R.X,R.Y,R.Z};
      if( Fly.lt>0 )
      { glBegin( GL_LINE_STRIP );    dot( R,purple );
        for( int k=0; k<Fly.lt; k++ )dot( R+=B.dR[(k+Fly.kt)%Fly.lt] );
        glEnd();
    } } Mc/=Fly.nc;                              //  стенки XZ-ящика ограничений на случай
    if( Gravity_mode )              // запуска сил гравитационного притяжения
    { color( lightcyan,0.75,0.5 );
      glBegin( GL_QUAD_STRIP );
      glVertex3d( -wall,-wall,0 ),glVertex3d( -wall,-wall,wall/2 );
      glVertex3d( -wall, wall,0 ),glVertex3d( -wall, wall,wall/2 );
      glVertex3d(  wall, wall,0 ),glVertex3d(  wall, wall,wall/2 );
      glVertex3d(  wall,-wall,0 ),glVertex3d(  wall,-wall,wall/2 );
      glVertex3d( -wall,-wall,0 ),glVertex3d( -wall,-wall,wall/2 ); glEnd();
    }
  } axis( Win,Screen_Size/2,Screen_Size/2,Screen_Size/2,"X","Y","Z" );
  Win.Save().Refresh(); return false;
}
//     Процедура управления отображением на объекте Screen::Tv (экране ЭВМ)
//
static bool timer_drawing()                       // моделирование в
{ if( Fly.nc )                                    // реальном времени
  { One_Inter_Step_of_Point( Fly,!linear_Model ); // взаимное притяжение
    if( Fly.lt>0 )                                // индекс разностей координат
    if( --Fly.kt<0 )Fly.kt=Fly.lt-1;              // для прорисовки траектории
//#pragma omp parallel for
    for( int iPart=0; iPart<Fly.nc; iPart++ )     // и свободное движение
        One_Flight_Step_of_Point( Fly,iPart );    // в поле сил притяжения
  }                                               // и вязкого сопротивления
  WinReady( &Win );                               // запрос завершения операций
  if( !(++kT%12) )return true;
//        Win.Draw();
           return false;
}
static bool keyboard( fixed key )
{ switch( key )
  { case _Esc: exit( 0 );                         // выход из программы
    case _F1:
    { static const char
      *Name[]={ "Fluid  ","Аналитическая механика  ",
                       "поляризованных корпускул    .",0 },
      *Cmds[]={ "F1    ","краткая справка",
                "F2    ","настройка визуализации",
                "Tab   ","частицы или корпускулы",
                "ksdtn ","куб/сфера/диполь/тор/ничего",
                "1-9   ","количество частиц/корпускул",
                "0/-/= ","27(кобальт) / 125 и 729",
                "V/v   ","трение воздуха 0,2-0,4",
                "R/r   ","вращение сцены в OpenGL",
                "<BkSp+Shift>","Исходная сцена + зенит",
                "<Insert>","    вброс новой частицы",
                "<Enter> ","    свобода или гравитация",
                "Up/Down Right/Left Home/End","  X,Y,Z",0 },
      *Plus[]={ "<Пробел>"," каркас / раскраска",
                "<Escape>/<ctrlC>"," стоп ",0 };
      Win.Help( Name,Cmds,Plus ); }           break;
    case _F2: while( Configuration()!=_Esc ); break;
    case _Ins:
    { Point R={5,4,3}; Vector V={-5,-4,-3};
      if( Win.ScanStatus()&SHIFT )R.X=-R.X,R.Y=-R.Y,R.Z=-R.Z,V=-V;
      Fly.Add( R,(Vector){Particle_volume,0,0},V );} break;
    case 'V': ks+=0.2; break; case 'v':ks-=0.2; break; // трение о воздух
    case _Enter: if( !Fly.nc )expMoving^=true;         // вольное движение
                     else  Gravity_mode^=true;  break;
    case _Tab: linear_Model^=true; break;         // переключение типа частиц
//  case _Del:                        // Win.eye=(Vector){ -130,-10,0.0 },
//        STime=ElapsedTime();        // Win.look=(Vector){ 0.0,0.0,30.0 },
//        Particle_volume=1.0;        // Win.Distance=-2*Screen_Size,
//  case _BkSp:(Mxl=(Point){0,0,0}).Identity(); break; //cx=cy=cz=0; break;
   default :
    { fixed Typ=MTyp,Sts=Win.ScanStatus();
      switch( key )
      { case ' ':
          if( Sts&CTRL )++Typ%=(Dipole+1); else   // закраска граней модели
          if( Sts&SHIFT )Typ=!Typ?Dipole:Typ-1; else painting^=true; break;
        case 'N': case 'n': Typ=None;              break;
        case 'K': case 'k': Typ=Cube;              break;
        case 'T': case 't': Typ=Torus;             break;
        case 'S': case 's': Typ=Sphere;            break;
        case 'D': case 'd': Typ=Dipole;            break;
        case _PgUp: Particle_volume*=1.1; Typ=255; break;
        case _PgDn: Particle_volume/=1.1; Typ=255; break;
       default:
        if( key>='0' && key<='9' || key=='-' || key=='=' )
        { if( Fly.nc )Fly.Clear(),(Point)Mxl=(Point){0,0,0}; else
          { Fly.Create( key=='='?729                        // остановка кубика
                    : ( key=='-'?125                        // признак запуска
                    : ( key=='0'?27:key-'0')),Fly_Count );  // на выходе размер
            Mxl=Particle_volume=Init_Flight_Points(Fly);   // на старте частицы
            for( int i=0; i<Fly.nc; i++ )Fly.Dip[i].M=Particle_volume; // сжаты
          } Typ=255; kT=-1;                // linear_Model=Win.ScanStatus()&ALT
        } else return true;                // не обработанное сбрасывается
      }
      if( Typ!=MTyp )
      { if( Typ!=255 )MTyp=Model_Type( Typ );
        for( int i=0; i<Fly.nc; i++ )Fly.Dip[i].M=Particle_volume;
      }
    }
  } Win.Draw(); return true;
}
//      Главная программа
//
int main( int argc, char *argv[], char *envp[] )
{ int key=0;
    View_initial();
    glPolygonMode( GL_FRONT_AND_BACK,GL_FILL );
    Win.Window::Icon( "Math" ).Draw( display );
    Win.Window::KeyBoard( keyboard );
    Win.SetTimer( 100,timer_drawing );          // 50 без перегрузки процессора
//  Win.Alfabet( 22,"Courier" );
    Fly.Create( 0,Fly_Count );       // на старте одинокая свободная корпускула
    do{ //if( keyboard( key ) )
//      display();
        WaitTime( 50 );
      }
    while( Win.Ready()
//         && (key=Win.WaitKey())!=_Esc
         );
    return 0;
}
static void Model_Draw( Model_Type typ ) // Простые равнообъёмные фигуры
{
//   Первые пробные функции для построения объемных (1м³)
//      Сфера: Ώ = п·D³/6    = 4·п·R³/3     S = 4·π·R²   = 3·Ώ/R
//      Тор:   Ώ = п²·d²·D/4 = 2·п²·r²·R    S = 4·π²·R·r = 2·Ώ/r
//             R — радиус орбиты вращения малой окружности тора
//             r — радиус малой образующей окружности тора
  switch( typ )
  { case Cube  : Mxl*=pow( Particle_volume/(Real)Mxl,0.333 );
                 Mxl.cubic( painting ); break;
    case Sphere: Mxl*=pow( Particle_volume*0.75/_Pi/(Real)Mxl,0.333);
                 Mxl.sphere( 15,24,painting ); break;
    case Dipole: Mxl*=pow( Particle_volume*0.75/_Pi/(Real)Mxl,0.333);
                 Mxl.dipole( Dipole_offset,painting ); break;
    case Torus:{ const Real K=0.01,r=1.0;       // коэффициент к диаметру дырки
                 Mxl*=pow( Particle_volume*_Pi/_Pi/2/(1+K)/(Real)Mxl,0.333 )/2;
                 Mxl.torus( r,r*( 1+K ),15,24,painting ); break;
               }         // painting - признак закраски граней подвижной модели
  }
}
