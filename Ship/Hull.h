//
//      Судоводительское отделение Калининградского мореходного училища
//              Министерства Рыбного хозяйства СССР
//      Лаборатория вычислительной гидромеханики и морских исследований
//              Сахалинский государственный университет
//      Подсекция мореходных качеств судов в штормовых условиях
//              Сахалинское отделение Российского научно-технического
//              общества судостроителей имени Алексея Николаевича Крылова
//      Кафедра компьютерного моделирования и многопроцессорных систем
//              факультет Прикладной математики - процессов управления
//              Санкт-Петербургский государственный университет
//                                        ©75 Калининград-Сахалин-‏יְרוּשָׁלַיִם
#include "../Math/Complex.h"
#include "../Window/Window.h"

extern const char *_Mnt[],*_Day[];       // названия месяцев и дней недели
extern Real              //              разметка: { Xo,Do },{ Xo+Lmx,Depth }
 Xo,Xm,Length,Lmx,Lwl,   // Длина конструктивная, максимальная и по ватерлинии
       Breadth,Bmx,Bwl,  // Ширина конструктивная, максимальная и по ватерлинии
 Do,   Draught,Depth,   // Осадка и высота борта
       Volume,           // Водоизмещение
       Surface;          // Смоченная поверхность
extern int Hull_Keys;    // Набор ключей - параметров
                         // 0x01 -  задействовать сплайн-интерполяцию
struct MainDraw: public Window
{ MainDraw();        // просто конструктор
 virtual bool Draw();// главная процедура для прорисовки теоретических чертежей
 void Loft( bool=false ); // общая разметка графического поля
};
void glInitial();
//
///    Три процедуры обращения к старым (оригинальным) чертежам
//
void Building();         // Изображение проекций корпуса на графическом экране
int  Set_Sinus();        // Корпус из косинусно-экспоненциальных функций
int  Set_Power();        // Корпус из степенных (параболических) функций
//t  Set_Stem();         // Аналитическое переопределение штевней
//t  Set_Parameters();
//
///  Численное определение одного шпангоута
//               X(0), Y(0) - точка киля
//               X(1), Y(1) - точка на сломе борта к палубе (ширстрек)
//               X(-1),Y... -//- то же - левого борта
struct Frame
{ int  N;        // количество интервалов в узловых интерполяционных точках{-1}
  Real X,min,max,// абсцисса и границы аппликаты теоретического шпангоута
   *y,*z,        // аппликаты и ординаты точек сплайна шпангоутов
   *_x,          // индексный массив 0..N или длина кривой по сплайну
   *_y,*_z;      // вторая производная в узлах сплайн-функции
  Frame( int l=0 ){ y=z=_x=_z=_y=0; N=0; X=min=max=0; if( l>0 )allocate( l ); }
  void allocate( int ); void free();
 ~Frame(){ free(); }
  Real G( _Real Z, bool=false );     // эмуляция плазовой ординаты по аппликате
  Real operator()(_Real A ){ return G( A ); }
  Real& operator[]( int k ){ return y[k]; }; // In/Out доступ к функции
  Real& operator()( int k ){ return z[k]; }; // аргументу по индексному отсчету
  void SpLine(); void Easy();     // подготовка к работе и сброс сплайн-функции
  void YiZ( Real A, Real &Y,Real &Z ); // и, - не совсем последовательный поиск
  void Double( int k );                // вставка сломанной точки на шпангоуте
};
///   Уточненный вариант корпуса, разделенного на шпангоуты
//
class Hull{ public:
 int  Ns,Ms,Nstem;         // Общее количество, номер миделя и нулевой шпангоут
 char *Name;                         // Название модели
 Frame *F,                           // Ряд Ns\Ms теоретических шпангоутов
       Stx,Sty,Asx,Asy;              // Форштевень и ахтерштевень, нос\корма
  Hull(): Ns(0),Ms(1),Nstem(2),F(0){ (Name=(char*)malloc( MAX_PATH*4 ))[0]=0; }
 ~Hull(){ allocate( 0 ); free( Name ); }
  void allocate( int N ){ F=(Frame*)Allocate((Ns=N)*sizeof(Frame),F); Ms=-1; }
  void Analytics();                           // варианты аналитических обводов
  void Init();                                // водоизмещение, площади и др.
  void Aphines( _Real cX,_Real cY,_Real cZ ); // абсциссы, ординаты, аппликаты
  void Simple_Hull( int Nx, int Nz, int Ns ); // шпангоуты, ватерлинии, штевни
  Real Y( _Real X,_Real Z ); // ордината-Y корпуса по абсциссе-X и аппликате-Z
  int Write();          // запись таблицы ординат корпуса в обновлённом формате
  bool Read();          // чтение корпуса: <имя> - новый; * ЛКИ; ' ' - Польша
  void MinMax();
//Real operator()( Real x,Real z );
private:// исходный корпус Игоря Степанова кафедры конструкции судов КорФак ЛКИ
  int Read_from_Polland();// польский теоретический чертёж Александра Дегтярева
  int Read_from_Frames(); // и обновлённая талица плазовых ординат со штевнями
};
//!   Рабочее описание корпуса в виде простого массива плазовых координат
//
struct Plaze              //! приватные массивы только для программы Michell
{ Real *Xa,*Xs,*Ya,*Ys,   //  Отсчеты шпангоутов, абсциссы и ординаты штевней
      **QV,               //  Массив "источников", замещающих корпус в движении
       *Wx;               //  Рабочий массив для визуализации волнообразования
  Real xA,xS;             //  Абсциссы крайних точек штевней ниже ватерлинии
protected:
 Real **Y;                //  Таблица ординат теоретических шпангоутов
 Real dX,dZ,              //  Шаг сетки по таблице плазовых ординат
      V,S;                //  Водоизмещение и площадь смоченной поверхности
public:                   //
 const int Nx,Nz;         //  Размерности основной матрицы
  Plaze( int z,int x, _Real Z=Draught );   // Построение таблицы ординат
 ~Plaze();                                 //+ распределение массивов и буферов
  void Drawing();          // Прорисовка результатов численного моделирования
  Real Michell( _Real Fn,bool=true );    // Волновое сопротивление по Мичеллу
private:
  Real Amplint( _Real La );              // Приращение амплитуды по длине волны
//void DrawWaves( _Real Fn );            // форма корабельного волнообразования
};
void Hull_Statics();                     // Гидростатика и остойчивость
//
//!   Совсем старенький вариант управления плоской картинкой с 2D-графикой
//!       (казалось бы, что проще не бывает, но и в развитии тоже - тупик)
//!   Блок определений для визуализации формы корпуса
//
class Plane: public Place // Размеры активного окна отслеживаются Windows
{ Real aX,aZ,uX,uZ,dx,dz; //   границы графического поля и шаг перерасчетов
  int &W,&H;              //   захват ссылок на действующие размерности окна
public: int ax,az,ux,uz;  // Угловые точки листа растровой картинки внутри окна
 const char *iD,*sX,*sZ;  //   заголовки и отметки размерностей в рабочем окне
  Plane( const char *i, const char *x, const char *z, Window *Win );
  Plane& Set( _Real X,_Real Z,_Real _X,_Real _Z );
  Plane& Focus();
//
//     Экранные и физические координаты отмеряются от заданного прямоугольника
//
  bool Is( int x,int z ){ z=H-z; return x>ax&&x<ux&&z>az&&z<uz; }
  Real X( int x ){ return aX + x*dx; }
  Real Z( int z ){ return aZ + z*dz; }   // Нормальные координаты окна ViewPort
   int x( _Real X ){ return int( ( X-aX )/dx ); }
   int z( _Real Z ){ return int( ( Z-aZ )/dz ); }
//
// Координаты анти-относительно реальных отсчетов внутри активного окна Windows
//
  Real wX( int x ){ return aX + (x-ax)*dx; }   // X( x-ax )
  Real wZ( int z ){ return aZ + (H-z-az)*dz; } // Y( H-y-ay )
//int xw( _Real X ){ return x( X )+ax; }
//int zw( _Real Z ){ return H-z( Z )-az; } - ??
};
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
extern short Active;      // 0x0001 - файл считанн и подготовлен к работе
                          // 0x0002 - полностью выполнена первая прорисовка
extern Hull  Kh;          // Собственно корпус, заданный отдельными шпангоутами
extern Plane wH,wM,wW;    // Окно проекции корпус, бок и полуширота
void Draw_Hull( int id, Plane &_W=wH );    // Проекция корпус в заданном окошке
void line( _Real x1,_Real z1,_Real x2,_Real z2 );
//
//  ... и все-таки нужна подборка стандартных цветов (без перевыбора)...
//
#define gl_BLACK        glColor3f( 0,0,0 )              // черный
//efine gl_WHITE        glColor3f( 1,1,1 )              // белый
#define gl_YELLOW       glColor3f( 1,1,0 )              // желтый
#define gl_LIGHTRED     glColor3f( 1,0,0 )              // красный
//efine gl_LIGHTBLUE    glColor3f( 0,0,1 )              // синий
#define gl_LIGHTCYAN    glColor3f( 0,1,1 )              // голубой
#define gl_LIGHTGREEN   glColor3f( 0,1,0 )              // зеленый
#define gl_LIGHTMAGENTA glColor3f( 1,0,1 )              // фиолетовый
//efine gl_BACKGROUND   glColor3f( 0.8,1,1 )
#define gl_LIGHTGRAY    glColor3f( .75,.75,.75 )
//efine gl_DARKGRAY     glColor3f( 0.3,0.3,0.3 )
//efine gl_GRAY         glColor3f( .6,.6,.6 )
#define gl_MAGENTA      glColor3f( .6,0,.6 )
//efine gl_BROWN        glColor3f( .4,.2,0 )
#define gl_CYAN         glColor3f( 0,.4,.4 )
#define gl_GREEN        glColor3f( 0,.4,0 )
#define gl_BLUE         glColor3f( 0,0,.5 )
#define gl_RED          glColor3f( .6,0,0 )