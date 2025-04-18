//
//   Window - минимальна€ подборка дл€ мышки в окне с кнопочками
//
#include "Tv_Graph.h"

struct Window                        //
{ Field F; field f;                  // экранные границы действующего фрагмента
  viewporttype b;                    // Tv_port - разметка окна Window
  int _x,_y,x,y;                     // =( l,t,r,b ) рамка дл€ мышкина окошка
  Window();                          // конструктор с приустановкой
  Field Focus(); bool Is( int X,int Y );
  Course Console( int Fl( int,int,bool ) );     // -1 -команда на перерисовку ?
};
//
//     ”прощенные процедуры реагировани€ на кнопочки
//
class Button: public Window                             // Window здесь излишне
{ char *_sign;                                          // ..кнопочка с текстом
public:Button(): Window(),_sign( NULL ){}               // пустой конструктор
      ~Button(){ if( _sign )free( _sign ); }            // + деструктор
  void Initial( const char* sign, int xt,int yt );      // установка надписи
  void Place( int xt,int yt );                          // с местоположением
  void Draw();
};
