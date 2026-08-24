#ifndef __View_      //#define __View_      // Очередная отработка элементарных графических примитивов                     //                                    ©2018-08-22  יְרוּשָׁלַיִם
#include <GL/glu.h>#include "Window.h"             // собственно графическая среда Window-Place
//#include "../Math/Vector.h"   // извне базовые структуры тензорной математики
typedef enum                    // словесные прописи цветовых названий{ white,silver,lightgray,gray,dimgray,darkgray,freeboard,yellow,green,lime,
  olive,lightgreen,navy,blue,lightblue,cyan,aqua,lightcyan,maroon,red,lightred,
  orange,pink,purple,magenta,fuchsia,lightmagenta,black,empty=-1   //!=27\{28}
} colors;                       // чистый цвета '33=51,'66=102,'99=153,'CC=204
union Color{ unsigned C; byte c[4]; };
extern const char *_Mnt[],*_Day[];                 // месяцев года, дней недели
//// Настройка начальной раскраски и освещенности графического пространства/сцены//                                   ... предустановка графической среды OpenGLvoid View_initial( Real Distance=600 );//
//  Небольшой комплекс процедур времени проведения вычислительного эксперимента
//
const Color &color( const Color& ); // непосредственный выбор цвета в [ rgb+a ]
const Color &color( const Color&,  //... с относительной подсветкой/затенением
                _Real bright,     //   -1 => от чёрного; +1 => до белого
                _Real alfa=1 );  // прозрачность\смешение 1=>0 выцветание blend
const Color &color( const colors );  // выбор цвета в палитре SeaColor
const Color &color( const colors, _Real bright,_Real alfa=1 );
const Color &seaColor( colors );                // выборка табличного цвета.
////    тонкая линия из точки (a) в точку (b) ... в однородных координатах OpenGL//
#define aR const Real*
inline aR dot( aR a ){ glVertex3dv( a ); return a; }     // контекстная точка  aR dot( aR, const colors );                            // с установкой цвета
  aR spot( aR,_Real Size, const colors=empty );          // • завершенная точка  aR line( aR,aR );                                      // завершённая прямая  aR line( aR,aR, const colors );                        // с конкретным цветом
  aR arrow( aR,aR,_Real dl=0.025, const colors=empty );  // стрелка, доли длины
  aR circle( aR center, _Real radius, bool=true );       // кружочек в {x-y}
void rectangle( aR LeftDown,aR RightUp, bool=true );     // прямоугольник {x-y}
void liney( aR,aR, const colors=empty );                 // отражение y-ординат
////   ... из точки (a) в точку (b) с объемной стрелкой в долях длины (a->b)////     aR arrow( aR a,aR b,_Real ab=0.06, const colors=empty );#undef aR                          // автоматическая разметка координатных осей
void axis( Place&,_Real,_Real,_Real,     // с чуть затемненными надписями xyz
         const char *x,const char *y,const char *z, const colors=cyan );                    //
class View:         //! блок визуализации возвращается сюда до лучших времен...
public Window{      //       собственно доступ к экрану, указателю и клавиатуре
virtual bool Mouse( int x,int y ){ return Place::Mouse( mx=x,my=y ); }  // мимо
virtual bool Mouse( int s,int x,int y );   // курсор-указатель - мышь свободная
protected:
   int mx,my;              // растровые координаты курсора мышки в окне
   Real lookX,lookY,lookZ, // координаты местоположения сцены
        eyeX,eyeY,eyeZ,    // и направление взгляда
        Distance;       // расстояние от точки установки камеры до места обзора
public: View( const char* Title,int X=0,int Y=0,int W=0,int H=0,_Real Size=0 );
virtual ~View(){ this->~Window(); } // в продолжение последовательности виртуальных операций
virtual bool KeyBoard( fixed Keyb );
virtual bool Draw();
};
/* glEnable( GL_COLOR_LOGIC_OP );
   glLogicOp( GL_XOR ); => GL_COPY | GL_SET
*/
#endif // __View_