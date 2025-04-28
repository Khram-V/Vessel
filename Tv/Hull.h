//
//  Лаборатория вычислительной гидромеханики и морских исследований
//              Сахалинского государственного университета
//
#include "OS\Tv_Graph.h"
extern Real               //
     Xo,Xm,Length, Lwl,    // Длина
           Breadth,Bwl,    // Ширина
     Do,   Draught,Height, // Осадка и высота борта
           Volume,         // Водоизмещение
           Surface;        // Смоченная поверхность
extern int Hull_Keys;      // Набор ключей - параметров
//              0x01 -  задействовать сплайн-интерполяцию
//
// Три процедуры обращения к старым (оригинальным) чертежам
//
//   Set_Stem();  // Аналитическое определение штевней
 int Set_Sinus(); // Корпус из косинусно-экспонециальных функций
 int Set_Power(); // Корпус из степенных (параболических) функций
 int Set_Parameters();
void Building();  // Изображение проекций корпуса на графическом экране
                  //
void Draw_Hull( int Axis=0 ); // Проекция корпус в заданном окошке
//
//     +-------------------------------------------------------+
//     ¦  Численное определение одного шпангоута               ¦
//     ¦  X(0),Y(0) - точка киля                               ¦
//     ¦  X(1),Y(1) - точка на сломе борта к палубе (ширстрек) ¦
//     ¦  X(-1),Y...-//- то же - левого борта                  ¦
//     ~=======================================================~
struct Frame
{ int  N;          // Количество узловых интерполяционных точек
  Real X,          // Абсцисса теоретического шпангоута
      *z,*y,       // Аппликаты и ординаты точек сплайна шпангоутов
     *_z,*_y;      // Вторая производная в узлах сплайн-функции
  Frame( int=0 );  Frame& allocate( int );
 ~Frame();         Frame& free();
  Frame& SpLine();                   // подготовка сплайн-функции
  Real operator()( Real Z );         // эмуляция плазовой ординаты
  Real Z( Real );                    // параметрическая выборка аппликаты
  Real Y( Real );                    // и ординаты шпангоута
  void YZ( Real A,Real &Y,Real &Z ); // и, - последовательный поиск
  int  iZ( Real &_y_ );              // корней неоднозначной функции
  void Double( int k );              // дублирование одной точки на шпангоуте
};
class Curve
{     Point *_C;          // Адрес массива
 Curve& free();
 int    find( Real );     // Двоичный поиск левого индекса
public: int N;            // Индексная длина линии
  Curve( int S=0 ){ _C=0; N=0; if( S>0 )allocate(S); } // Constructor
 ~Curve(){ free(); }                                   // Destructor
 Curve& allocate( int=0 );                             // Распределение памяти
        operator Point* () { return _C;    } // Прямой доступ к вектору
 Real  operator()( Real );      // Линейная или Сплайн интерполяция
 void Extreme( Field &_Fm );     // Выбор экстремумов по исходным точкам
};                               //
//    +---------------------------------------------------------+
//    ¦  Уточненный вариант корпуса, разделенного на шпангоуты  ¦
//    +=========================================================+
class Hull
{
public:
 int  Ns,Ms;            // Количество шпангоутов и номер миделя
 char Name[256];        // Название модели
 Frame *F;              // Массив теоретических шпангоутов
 Curve Stx,Sty,         // Форштевень
       Asx,Asy;         // Ахтерштевень (корма)
  void allocate( int N )
     { F=(Frame*)Allocate( (Ns=N)*sizeof(Frame),F ); Ms=-1; }
  void Init();          //
  void Simple_Hull( int Nx, int Nz );
  void Set_SpLine();    // Установка
  void Del_SpLine();    //  и снятие сплайновой разметки
  Hull():Ns(0),F(0){ Name[0]=0; };
 ~Hull(){ allocate( 0 ); }
  Real operator()( Real x, Real z );
  int Read( const char* name="HULL.vsl" );
  int Write();
private:
  int Read_from_Polland();
  int Read_from_Frames();
};              //
extern Hull Kh; // Собственно корпус, заданный отдельными шпангоутами
// ~~~~~~~~~~~~ //
//      Блок определений для визуализации формы корпуса
//
struct Window
{ viewporttype f; Field F; Window(); void Set( field a );
  bool Is( int x,int y ); // Проверка попадания мышки
  void Focus();  // предустановка и активация графического окна
//void Check( int &x, int &y ); bool Is();
};
struct DrawPlane: public Window
{ const char *iD,*aX,*aY;
  DrawPlane( const char i[], const char x[], const char y[] ):
             Window(),iD(i),aX(x),aY(y){}
  bool Info( int x, int y );
};
// Рабочее описание корпуса в виде простого массива плазовых координат
//
struct Plaze          //
{ Real **Y,           // Таблица ординат теоретических шпангоутов
      dX,dZ,  Depth,  // Шаг сетки по таблице плазовых ординат и осадка
     *Xa,*Xs,*Ya,*Ys, // Отсчеты шпангоутов, абсциссы и ординаты штевней
      V,S,            // Водоизмещение и площадь смоченной поверхности
     **QV,            // Массив "источников", замещающих корпус в движении
     *Wx;             // Рабочий массив для визуализации волнообразования
  int Nx,Nz;          // Размерности основной матрицы
  Plaze( int z, int x )
  { Depth=V=S=0; dX=dZ=1; QV=Y=NULL; Wx=Xa=Xs=Ya=Ys=NULL; allocate( z,x ); }
 ~Plaze(){ allocate( 0,0 ); }
  void allocate( int z, int x );      // Распределение массивов и буферов
  void build( Real Z=Draught );       // Построение таблицы ординат
  void drawlines();                   // Прорисовка рисунков корпуса
  Real  Michell( Real Fn );           // Сопротивление по Мичеллу
  double Amplint( const double &La ); //
  double QG4( const double &Xl, const double &Xu );
  void DrawWaves( Real Fn=0.0 );
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
extern DrawPlane wH,    // Окно проекции корпус,
                 wM,    //  бок
                 wW;    //   и полуширота
extern const char *Months[],*WeekDay[];
