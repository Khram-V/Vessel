#ifndef __View_      //#define __View_      // Очередная отработка элементарных графических примитивов                     //                                    ©2018-08-22  יְרוּשָׁלַיִם
#include <GL/glu.h>#include "Window.h"              // собственно графическая среда Window-Place
//#include "../Math/Vector.h"   // извне базовые структуры тензорной математики
typedef enum{ white,silver,lightgray,gray,dimgray,darkgray,yellow,green,lime,olive,
  lightgreen,navy,blue,lightblue,cyan,aqua,lightcyan,maroon,red,lightred,
  orange,pink,purple,magenta,fuchsia,lightmagenta,black,empty=-1 //!=27\{28}
} colors;                                           // словесные прописи имён
extern const char *_Mnt[],*_Day[];                 // месяцев года, дней недели
//// Настройка начальной раскраски и освещенности графического пространства/сцены//void View_initial();              // ... предустановка графической среды OpenGL//
//  Небольшой комплекс процедур времени проведения вычислительного эксперимента
//
void color( const colors );      // выбор цвета в палитре SeaColor
void color( const colors clr,    //  ... с относительной подсветкой/затенением
             _Real bright,       //   -1 => от чёрного; +1 => до белого
             _Real alfa=1.0 );   // прозрачность\смешение 1=>0 выцветание blend
////    тонкая линия из точки (a) в точку (b) ... в однородных координатах OpenGL//
#define aR const Real*
inline aR dot( aR a ){ glVertex3dv( a ); return a; }     // контекстная точка  aR dot( aR, const colors );                            // с установкой цвета
  aR spot( aR,_Real Size, const colors=empty );          // • завершенная точка  aR line( aR,aR );                                      // завершённая прямая  aR line( aR,aR, const colors );                        // с конкретным цветом
  aR circle( aR center, _Real radius, bool=true );       // кружочек в {x-y}
void rectangle( aR LeftDown,aR RightUp, bool=true );     // прямоугольник {x-y}
void liney( aR,aR, const colors=empty );                 // отражение y-ординат
////   ... из точки (a) в точку (b) с объемной стрелкой в долях длины (a->b)//  aR arrow( aR a,aR b,_Real ab=0.06, const colors=empty );#undef aR                          // автоматическая разметка координатных осей
void axis( Place&,_Real,_Real,_Real,       // с чуть затемненными надписями xyz
           const char *x,const char *y,const char *z, const colors=cyan );                    //
class View:         //! блок визуализации возвращается сюда до лучших времен...
public Window{      //       собственно доступ к экрану, указателю и клавиатуре
virtual bool Mouse( int x,int y ){ return Place::Mouse( mx=x,my=y ); }// мимо
virtual bool Mouse( int s,int x,int y ); // курсор-указатель - мышь свободная
protected:
   Vector eye,look;  // направление взгляда и координаты местоположения сцены
   Real Distance;    // расстояние от точки установки камеры до места обзора
   int mx,my;        // растровые координаты курсора мышки в окне
public: View( const char* Title,int X=0,int Y=0,int W=0,int H=0,_Real Size=1 );
//     ~View(){}     // - в продолжение последовательности виртуальных операций
virtual bool KeyBoard( fixed Keyb );
virtual bool Draw();
};
/*
  glEnable( GL_COLOR_LOGIC_OP );
  glLogicOp( GL_XOR ); => GL_COPY | GL_SET
*/
#endif // __View_