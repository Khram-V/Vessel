/**
 **     Моделирование кинематики, динамики и механики взаимодействия
 **     множества поляризованных частиц первого и второго порядка
 **     (пространственные вихреисточники и диполи)
 **
 **     1 - все частицы имеют изначальный единичный момент (массу)
 **     2 - скорость движения и масса исходно ортонормированны на единицу
 **     3 - инерционное ускорение приводит к дефектам массы и формы частицы
 **
 **                                              (c)2021 ‏יְרוּשָׁלַיִם
 **/
#include "Dipoles.h"                              // Win64: -Wno-narrowing
#include "../Window/ConIO.h"                      //        -Wno-literal-suffix
#include "../Window/Window.h"

Dipoles *Dipoles_array=0;  // множество или рой из групп диполей в пространстве
unsigned nDip=0,           // количество корпускул в активном эксперименте
         Time_count=0,     // отсчёты временных шагов от запуска эксперимента
         Video_count=0;    // счётчик кадров видео прорисовок

void ReInstall_TimeSpace( int N )  // продолжительность во времени Dipole_route
{ clrscr();                        // в переустановке используется исходный рой
  glDisable( GL_LIGHTING );
  glPolygonMode( GL_FRONT_AND_BACK,GL_FILL );
  //
  //  начальная установка с обнулением счётчика исполненных тактов эксперимента
  //
  Time_count=Video_count=0;
  for( int i=0; i<Dipole_route; i++ )Dipoles_array[i].Install( N ); nDip=N;
                                     Dipoles_array[0].Initial();
}
// Предустановка и начальная инициализация вычислительного эксперимента в целом
//
Dipoles& Dipoles::Install( int N )     // количество условных нуклонов в группе
{ T=0.0;                                       // динамическое добавление точек
  if( nDip<N )D=(Dipole*)Allocate( N*sizeof( Dipole ),D );  // по необходимости
  for( int k=0; k<N; k++ )      // без повторений и предварительной расчисткой
  { D[k].M=(Vector){ 1,0,0 },   // дипольные моменты
    D[k].V=(Vector){ 0,0,0 },   // изначальная скорость набегающего потока
    D[k].W=(Vector){ 0,0,0 },   // наведённый диполями поток в локальном базисе
    D[k].R=(Vector){ 0,0,0 };   // координаты корпускулы в абсолютных отсчётах
    ExtFlow=0.0;                // внешний набегающий поток
  } return *this;
}
Dipoles& Dipoles::Initial()
{ int k=nDip;
  switch( nDip )              // в предустановке только начальный рой корпускул
  { case 1: D[0].M=1.0; break;                                   //  H водород
    case 2: D[1].R.x=-(D[0].R.x=.5); break;                      // ²H дейтерий
    case 3: D[0].R=(Vector){0,0.5,.28866},                       // ³H тритий
            D[1].R=(Vector){0,-.5,.28866},D[2].R=(Vector){0,0,-0.57733}; break;
    case 4: D[0].R=(Vector){0.5,0,.35355},D[2].R=(Vector){0,0.5,-.35355}, // Не гелий 2+2 ~~ Be бериллий 4+5
            D[1].R=(Vector){-.5,0,.35355},D[3].R=(Vector){0,-.5,-.35355}; break;
    case 5: D[0].R=(Vector){0,0.5,.28866},D[2].R=(Vector){0,0,-.57733}, // B бор 5+6
            D[1].R=(Vector){0,-.5,.28866},D[4].R.x=-( D[3].R.x=.7071 ); break;
    case 6: D[0].R.x=-(D[1].R.x=.7071),D[2].R.y=-(D[3].R.y=.7071), // C углерод 6+6
            D[4].R.z=-(D[5].R.z=.7071); break;    // √2 по касательной
    case 7: D[6].R.x=-(D[0].R.x=.55)-.12; k--;    // Li Литий 3+4 ~~ N Азот 7+7
          while(--k>0)D[k].R=(Vector){0,sin(k*_Pd/5),cos(k*_Pd/5)}*.759; break;
    case 8: while(k--)D[k].R=(Vector){(k%2)-.5,k/4-.5,(k%4)/2-.5}*0.866; break; // ~~ О кислород 8+8
    case 9: k--; while(k--)D[k].R=(Vector){(k%2)-.5,k/4-.5,(k%4)/2-.5};  break; // Ве Берилий 4+5 ~~ F фтор 9+10
    case 10: D[8].R.x=-(D[9].R.x=1.32); k-=2;               // ~~ Ne неон 10+10
             while(k--)D[k].R=(Vector){(k%2)-.5,k/4-.5,(k%4)/2-.5}*1.24; break;
    case 27: while(k--)D[k].R=(Vector){(k%3)-1,k/9-1,(k%9)/3-1}*0.7071;  break; // Al алюминий 13+14 ~~ Co кобальт 27+32
    case 64: while(k--)D[k].R=(Vector){(k%4)-1.5,k/16-1.5,(k%16)/4-1.5}; break; //*0.71
    case 125:while(k--)D[k].R=(Vector){(k%5)-2,  k/25-2,  (k%25)/5-2  }; break; //*0.528;
    case 216:while(k--)D[k].R=(Vector){(k%6)-2.5,k/36-2.5,(k%36)/6-2.5}; break;
    case 343:while(k--)D[k].R=(Vector){(k%7)-3,  k/49-3,  (k%49)/7-3  }; break;
    case 512:while(k--)D[k].R=(Vector){(k%8)-3.5,k/64-3.5,(k%64)/8-3.5}; break;
    case 729:while(k--)D[k].R=(Vector){(k%9)-4,  k/81-4,  (k%81)/9-4  }; break; //*0.35;
   default:
    { int x,_x=pow( nDip,1.0/3.0 )+1,y,_y=sqrt(Real(nDip)/_x)+1,z,_z=nDip/(Real(_x)*_y)+1;
      Break( "~nDip=%d => x:%d, y:%d, z:%d ",nDip,_x,_y,_z ); //exit(11);
      for( k=0,z=0; z<_z; z++ )
      for( y=0; y<_y; y++ )
      for( x=0; x<_x && k<nDip; x++ )
        D[k++].R=(Vector){ x-_x/2.0,y-_y/2.0,z-_z/2.0 };
    }   // Break( "Неверное количество[%d]"
  }     //        " ≠ 1,2,3,4,5,6,7,8,9,10,27,64,125,216,343,512,729",nDip );
  if( nDip>1 )
  for( k=0; k<nDip; k++ )
    D[k].M=norm( D[k].R )<=eps ? (Vector){-1}:dir( -D[k].R ); return Average();
}
//  Главная программа
//
int main( int argc, char** argv )   // однократное распределение всех маршрутов
{ Dipoles_array =                  // для хранения динамики движения во времени
    (Dipoles*)Allocate( Dipole_route*sizeof( Dipoles ) );
  ReInstall_TimeSpace( 6 );            // поначалу будет только одна корпускула
  texttitle( "Пространственные частицы и поляризованные корпускулы" );                                     //  что не сильно перегрузит вычислители
  glDepthFunc( GL_LEQUAL ); //NEVER~EQUAL~GEQUAL~GREATER~LEQUAL~NOTEQUAL~LESS~ALWAYS
  while( VideoStage() )
  { unsigned T=GetTime(); //WaitTime(60);// отсчет начала приоритетных расчётов
    One_Time_Step();                   // проверка, жива ли еще сама программа
    RealTime+=GetTime()-T;             // использованный интервал времени #0
  }                                    // запускается интервальный таймер и
/*{ WaitTime( Quantum_wait,            // время задержки для внешних операций
              One_Time_Step,           // собственно процедура расчётного цикла
              nDip<2?0:Quantum_exp );  // счёт по exp и приостановка на wait мс
  }*/                 // одна частичка не сильно грузит вычислительные процессы
  Break( " на выход " ); return 0;
}
