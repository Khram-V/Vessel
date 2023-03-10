//
//    Блок формирования/моделирования кинематики
//       корпускулярных вычислительных объектов
//      -- пусть на кинематический этап остается как есть
//         - частицы собственно движутся свободно по инерции
//           и под действием внешних сил
//      -- второй этап влияет на переориентацию направления движения
//           в строгом соответствии с суммарным потоком эфира,
//         - с частичной перенастройкой инерционного приращения скорости
//
#include "..\Math\Volume.h"

extern                // признаки режимов движения
bool Gravity_mode,    // =false свободное или гравитационное перемещение частиц
     linear_Model;    // =false линейно-упругая или ядерная модель корпускул
extern                //
Real ks,              // 0.4  коэффициент сопротивления для шара [0.2-0.4]
     wall,            // 1.32 стенки аквариума (высокого, пока)
     init_Scale;      // 1.0  начальный масштаб для каждой частицы
                      //
Real Init_Flight_Points( Array& );          // на выходе размер ячейки-частицы
void One_Inter_Step_of_Point( Array&,bool );
void One_Flight_Step_of_Point( Array&,int );

const double           //
      g=9.8106,        // ускорение свободного падения [м/с2];
      R_Earth=6367554, // радиус Земли, сфероид Красовского [м]
      ro=1.29,         // плотность воздуха 1.29 [кг/м3];
      Rm=0.2,          // радиус шарообразного тела [м] - 0.1
      M=0.5;           // масса тела
//
//      Блок анализа и настройки управляющих параметров программы
//
#define Vo( p ) Fly.Dip[p].V
#define Ro( p ) Fly.Dip[p].R

Real Init_Flight_Points( Array& Fly )
//{ for( int k=0; k<Fly.nc; k++ )Vo(k)=(Vector){ 0,0,0 },Ro(k)=(Point){ 0,0,0 };
{ for( int k=0; k<Fly.nc; k++ )Vo(k)=0.0,Ro(k)=0.0;
  switch( Fly.nc )
  { case 1: Vo(0)=(Vector){ -3,1,3 }, Ro(0)=(Point){ 2,0,2 };          break;   // Водород
    case 2: Vo(0).x=-(Vo(1).x=0.36);  Ro(0).X=-0.7071;                          // Гелий \~дейтерий
            Vo(0).y=-(Vo(1).y=0.24);  Ro(0).Z=0.7071;                  break;
    case 3:/*Vo(0).y=-(Vo[1].y=.24);*/Ro(2)=(Point){ 0.1,0.3,1 };               // Литий \~тритий
           /*Vo(1).x=-(Vo[2].x=.32);*/Ro(0).Z=0.7071;
                                      Ro(0).X=-0.7071;                 break;
    case 4:                           Ro(1).Z=Ro(2).Z=0.86;                     // Берилий \гелий
         /*Vo(0).y=-(Vo(2).y=0.24);*/ Ro(1).X=-(Ro(2).X=0.5);
         /*Vo(1).x=-(Vo(3).x=0.32);*/ Ro(3)=(Point){ 0,0.71,0.5 };     break;
    case 5:                           Ro(1).Z=Ro(2).Z=0.4;                      // Бор \нет
                                      Ro(1).X=-(Ro(2).X=0.6);
                                      Ro(3)=(Point){ 0,0.6,0.6 };
                                      Ro(4)=(Point){ 0,-0.6,0.6 };     break;
    case 6:                           Ro(0).X=-(Ro(1).X=0.6667);                // Азот \нет
         /* Vo(0).x=0.1;   */         Ro(2).Y=-(Ro(3).Y=0.6667);
         /* Vo(6).z=0.01;  */         Ro(4).Z=-.6667; Ro(5).Z=0.6667;  break;
    case 7:                           Ro(0).X=-(Ro(1).X=0.8);                   // Азот \ литий
         /* Vo(6).x=0.01;  */         Ro(2).Y=-(Ro(3).Y=0.8);
         /* Vo(6).z=0.01;  */         Ro(4).Z=-.8; Ro(5).Z=0.8;
                                      Ro(6)=(Point){ 0.01,0.01,-.01 }; return 0.93/2;
    case 9:                           Ro(8)=(Point){ 0.43,0.43,0.43 };          // Фтор \ берилий
    case 8:                           Ro(0).Z=Ro(1).Z=Ro(2).Z=Ro(3).Z=0.86;     // Кислород \нет
                                      Ro(2).X=Ro(3).X=Ro(6).X=Ro(7).X=0.86;
                                      Ro(1).Y=Ro(2).Y=Ro(5).Y=Ro(6).Y=0.86; return 0.86/2;
    case 27:                                                                    // Кобальт \ алюминий
#if 0
      Ro(0).Z=Ro(1).Z=Ro(2).Z=Ro( 9).Z=Ro(10).Z=Ro(11).Z=Ro(18).Z=Ro(19).Z=Ro(20).Z=-.71;
//    Ro(3).Z=Ro(4).Z=Ro(5).Z=Ro(12).Z=Ro(13).Z=Ro(14).Z=Ro(21).Z=Ro(22).Z=Ro(23).Z=0.0;
      Ro(6).Z=Ro(7).Z=Ro(8).Z=Ro(15).Z=Ro(16).Z=Ro(17).Z=Ro(24).Z=Ro(25).Z=Ro(26).Z=0.71;

//    Ro( 0).Y=Ro( 1).Y=Ro( 2).Y=Ro( 3).Y=Ro( 4).Y=Ro( 5).Y=Ro( 6).Y=Ro( 7).Y=Ro( 8).Y=0.0;
      Ro( 9).Y=Ro(10).Y=Ro(11).Y=Ro(12).Y=Ro(13).Y=Ro(14).Y=Ro(15).Y=Ro(16).Y=Ro(17).Y=0.71; //0.62;
      Ro(18).Y=Ro(19).Y=Ro(20).Y=Ro(21).Y=Ro(22).Y=Ro(23).Y=Ro(24).Y=Ro(25).Y=Ro(26).Y=-0.71;

//    Ro(0).X=Ro(3).X=Ro(6).X=Ro( 9).X=Ro(12).X=Ro(15).X=Ro(18).X=Ro(21).X=Ro(24).X=0.0;
      Ro(1).X=Ro(4).X=Ro(7).X=Ro(10).X=Ro(13).X=Ro(16).X=Ro(19).X=Ro(22).X=Ro(25).X=0.71;
      Ro(2).X=Ro(5).X=Ro(8).X=Ro(11).X=Ro(14).X=Ro(17).X=Ro(20).X=Ro(23).X=Ro(26).X=-0.71;
      /* Ro(27).X=Ro(27).Y=Ro(27).Z=12.0; */ return 0.71/2;
#else
      for( int i=0; i<27; i++ )Ro(i)=(Point){ (i%3)-1.0,i/9-1.0,(i%9)/3-1.0 }*0.71; return 0.71/2;
#endif
    case 125:
      for( int i=0; i<125; i++ )Ro(i)=(Point){ (i%5)-2.0,i/25-2.0,(i%25)/5-2.0 }*0.528; return 0.528/2;
    case 729:                                                                   // 1.32
      for( int i=0; i<729; i++ )Ro(i)=(Point){ (i%9)-4.0,i/81-4.0,(i%81)/9-4.0 }*0.35; return 0.35/2;
  } return 1.0/2;                                                               // 1.925
}
//
//      Силовые реакции и приращение скорости при взаимодействии частиц
//
void One_Inter_Step_of_Point( Array& Fly, bool MFiz )
{    //
     //  Учет силового взаимодействия свободных частиц между собой
     //
//#pragma omp parallel reduction( +: Vo(j) )
#pragma omp parallel for
  for(   int j=0; j<Fly.nc; j++ )
  {
    for( int k=0; k<Fly.nc; k++ )if( k!=j )
    { Vector r = Ro(k)-Ro(j); Real ss=norm( r ),s=sqrt( ss ); r*=Fly.dT/M_PI/s;
        if( MFiz )Vo(j) -= r*( 1.0/ss - 1.0 )/ss; else
        if( s>1.4655712319 )Vo(j) += r/ss;
                      else  Vo(j) -= r*( 1.0-s );
    }
  }
}
//      Блок расчета кинематики частицы под действием внешних сил и трения
//
void One_Flight_Step_of_Point( Array& Fly, int iPart ) // 1 шаг для iPart
{ Real  dT=Fly.dT;
  Point  R=Ro(iPart);
  Vector V=Vo(iPart);
    //
    // Если высота полета вне атмосферы, то всего лишь учитывается радиус Земли
    //
    if( R.Z>R_Earth/300.0 )                     // 22 км выше поверхности Земли
        V.z-=( dT*g )/( 1+sqr( R.Y/R_Earth ) ); //   -- для космического полета
    else
    { if( Gravity_mode )           // nPoints<=3 )//|| nPoints>=27 )
      { //
        //! Прямое действие массовых гравитационных сил
        //
        if( R.Z<0.0 ) V.z += g*dT; // на выход над поверхностью
              else    V.z -= g*dT; // и расчет новой скорости движения тела
        //
        // Расчет гидростатических сил для плотности воздуха (сила всплытия)
        //
        V.z += g*dT * ro*( (4.0/3.0) * M_PI*Rm*Rm*Rm )/M;
        //
        // Проверка на касание поверхности Земли, с отскоком
        //
        if( R.Z<=0 && V.z<=0 )V.z=-0.92*V.z; // R.y=0; с небольшим
                                             // подтормаживанием-проникновением
        if( R.X>wall && V.x>0 || R.X<-wall && V.x<0 )V.x=-0.92*V.x;
        if( R.Y>wall && V.y>0 || R.Y<-wall && V.y<0 )V.y=-0.92*V.y;
      }
      //
      //  Для учета сопротивления движению со стороны воздуха потребуется
      //  определить все инерционные и геометрические параметры,
      //  т.к. внешние силы будут приложены к внешней поверхности тела
      //
     Real v=abs( V );     // величина скорости для учета сопротивления воздуха
      if( v>0.0 )V -= V/v        // величина скорости здесь только уменьшается
                   * ks                         // коэффициент сопротивления
                   * ro                         // плотность воздуха
                   * ( v*v / 2.0 )              // и квадрат скорости
                   * M_PI * Rm*Rm               // площадь поперечного сечения
                   * dT / M;                    // инерционное приращение
    }
    //  Для расчетов новых координат тела уже не требуется
    //    учета действия внешних сил.
    //
    R += V*dT; // приращение пространственных координат
    //
    //      Проведено построение траектории движения тела
    //              и величин мгновенных значений скорости.
    //
    //      На заключительном этапе можно провести анализ и визуализацию
    //      маршрута и параметров движения тела
    //
//  if( !iPart )
//  printf( "R[%d]{ %5.2lf,%5.2lf,%5.2lf }, V{ %5.2lf,%5.2lf,%5.2lf }\n",
//          iPart, R.x,R.y,R.z, V.x,V.y,V.z );

    if( Fly.lt>0 )                               // Сохранение разностей
        Fly.Dip[iPart].dR[Fly.kt]=Ro(iPart)-R;   // траектории движения частиц                                       // координат для визуализации
    Ro(iPart)=R;                                 // сохранение нового положения
    Vo(iPart)=V;
}
