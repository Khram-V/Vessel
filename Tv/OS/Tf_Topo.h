#ifndef __TF_Topo_Hxx__
#define __TF_Topo_Hxx__
//
//   Простая прорисовка цветной карты с теневой разметкой градиентов
//   Amax(-) - белые вершины; Amin(+) - черные глубины
//
void Color_Paint( float **_D,int y,int x,int _y,int _x );
//
//      Tv_topo - основная программа рисования линий уровня          /91.03.01/
//        Cmd = true  - по текущему списку
//              false - с автоматическим построением списка
//
template <class real>
    void Tf_Topo( real **_D,int y,int x,int _y,int _x,bool Cmd=false );
//
//       Добавления линии уровня в список Topo
//          return - количество линий в списке
//
     int Tf_List( Real fl, const int cl ); // cl - цвет
     int Tf_List( int NL=0 );              // Установка количества линий
//
//      Изображение одной изолинии без включения в список
//
template <typename real>
    void Tf_Line( real **_D,int y,int x,int _y,int _x,Real fl, int clr );
//
//      Программа управления выводом изолиний
//
//id Tf_Show( float **_D,int y,int x,int _y,int _x,void(*axis)(bool)=Tv_axis );
void Tf_Color
 ( const int Cplus=LIGHTBLUE,const int Cnull=YELLOW,const int Cminus=GREEN );
#endif
