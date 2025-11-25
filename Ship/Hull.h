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
#include <Math.h>
#include "../Window/Window.h"
#define eps 1.0e-12

extern const char *_Mnt[],*_Day[];      // названия месяцев и дней недели
extern Real              //                разметка: { Xo,Do },{ Xo+Lmx,Depth }
 Xo,Xm,Length,Lmx,Lwl,   // Длина конструктивная, максимальная и по ватерлинии
       Breadth,Bmx,Bwl,  // Ширина конструктивная, максимальная и по ватерлинии
 Do,   Draught,Depth,    // Осадка и высота борта
       Volume,           // Водоизмещение
       Surface;          // Смоченная поверхность
extern unsigned Hull_Keys; // Набор ключей - параметров
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
class Frame
{ Real *_x,      // индексный массив 0..N или длина кривой по сплайну
       *_y;      // вторая производная в узлах сплайн-функции
  bool Spl;      // признак включения сплайн-функции
public: Frame( int l=0 ); ~Frame();
 int  N;         // количество интервалов в узловых интерполяционных точках{-1}
 Real X,min,max, // абсцисса и границы аппликаты теоретического шпангоута
     *y,*z;      // аппликаты и ординаты точек сплайна шпангоутов
 Real& operator()( int k ){ return z[k]; };  // аргументу по индексному отсчету
 Real& operator[]( int k ){ return y[k]; };  // In/Out доступ к функции
 Real G( _Real Z, bool=false );     // эмуляция плазовой ординаты по аппликате
 void SpLine( bool old=false );      // включение и ...
 void Easy(){ Spl=false; }           //       ... и сброс сплайн-функции
 void allocate( int ); //void free();
 Real operator()( _Real Z ){ return G( Z ); }
 void Double( int k );                 // вставка сломанной точки на шпангоуте
 void AYZ( Real A, Real &Y,Real &Z );  // и, - не совсем последовательный поиск
 void Opt( _Real Lc, _Real Ac );       // сброс отрезков меньше Lc под углом Ac
};
///   Уточненный вариант корпуса, разделенного на шпангоуты
//
class Hull{ public:
 int  Ns,Ms,Nstem;         // Общее количество, номер миделя и нулевой шпангоут
 char *Name;                            // [MAX_PATH*4];  // Название модели
 Frame *F,                              // Ряд Ns\Ms теоретических шпангоутов
       Stx,Sty,Asx,Asy;                 // Форштевень и ахтерштевень, нос\корма
  Hull(); // ~Hull();
  void Analytics();                           // варианты аналитических обводов
  void allocate( int N );                     // вектор адресов для шпангоутов
  void Init();                                // водоизмещение, площади и др.
  void Aphines( _Real cX,_Real cY,_Real cZ,   // абсциссы, ординаты, аппликаты
                _Real dX,_Real dZ,            // смещение абсциссы и аппликаты
                 Real Lc, Real Ac );          // min.отрезок под max.углом
  void ModelEx( Real &L,Real &B,int N,int M ); // Кубик, Эллипс, Шлюпка, Корпус
  void BilgeEx( _Real Height,  // скуловые формирования обводов по осадке
                _Real Power ); // или по борту и cо степенью заострения
  Real Y( _Real X,_Real Z ); // ордината-Y корпуса по абсциссе-X и аппликате-Z
  int Write();          // запись таблицы ординат корпуса в обновлённом формате
  bool Read();          // чтение корпуса: <имя> - новый; * ЛКИ; ' ' - Польша
private:// исходный корпус Игоря Степанова кафедры конструкции судов КорФак ЛКИ
  int Read_from_Polland();// польский теоретический чертёж Александра Дегтярева
  int Read_from_Frames(); // и обновлённая таблица плазовых ординат со штевнями
  void Simple_Hull( int Nx, int Nz, int Ns );  // шпангоуты, ватерлинии, штевни
  void MinMax();
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
  bool Is( int x,int z );          // { z=H-z; return x>ax&&x<ux&&z>az&&z<uz; }
  Real X( int x );                 // { return aX+x*dx; } Нормальные координаты
  Real Z( int z );                 // { return aZ+z*dz; } внутри окна ViewPort
   int x( _Real X );               // { return int( ( X-aX )/dx ); }
   int z( _Real Z );               // { return int( ( Z-aZ )/dz ); }
//
// Координаты анти-относительно реальных отсчетов внутри активного окна Windows
//
  Real wX( int x );                // { return aX+(x-ax)*dx; }   // X( x-ax )
  Real wZ( int z );                // { return aZ+(H-z-az)*dz; } // Y( H-y-ay )
//int xw( _Real X ){ return x( X )+ax; }
//int zw( _Real Z ){ return H-z( Z )-az; } - ??
};
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
extern fixed Active;      // 0x0001 - файл считанн и подготовлен к работе
                          // 0x0002 - полностью выполнена первая прорисовка
extern Hull  Kh;          // Собственно корпус, заданный отдельными шпангоутами
extern Plane wH,wM,wW;    // Окно проекции корпус, бок и полуширота
void Draw_Hull( int id, Place &_W );       // Проекция корпус в заданном окошке
void line( _Real x1,_Real z1,_Real x2,_Real z2 );

struct complex
{ Real x,y;                                         // real & imaginary part
  complex& operator=( const Real& r ){ x=r,y=0.0; return *this; }
  complex& operator+=( const complex& c ){ x+=c.x; y+=c.y; return *this; }
  complex& operator*=( const complex& c ){ Real w=(x*c.x)-(y*c.y); y=(x*c.y)+(y*c.x); x=w; return *this; }
};
inline Real const abs( const complex& c ){ return hypot( c.x,c.y ); }
inline Real const norm( const Real& x, const Real& y ){ return x*x+y*y; }
inline Real const norm( const complex& c ){ return c.x*c.x+c.y*c.y; }
inline Real operator % ( const complex& a,const complex& b ){ return a.x*b.x+a.y*b.y; }
inline complex operator * ( complex c,const Real& r ){ c.x*=r; c.y*=r; return c; }
inline complex operator + ( complex c,const complex& e ){ c.x+=e.x; c.y+=e.y; return c; }
inline complex polar( const Real& a ){ return (complex){ cos( a ),sin( a ) }; }
inline complex exp( const complex& c ){ return polar( c.y )*exp( c.x ); }
inline Real e5( _Real R ){ return round( R*1e5 )/1e5; }
//
//  ... и все-таки нужна подборка стандартных цветов (без перевыбора)...
//
#define gl_BLACK        glColor3ubv((const GLubyte[]){0,0,0})       // черный
//efine gl_WHITE        glColor3ubv((const GLubyte[]){255,255,255}) // белый
#define gl_YELLOW       glColor3ubv((const GLubyte[]){255,255,0  }) // желтый
#define gl_LIGHTRED     glColor3ubv((const GLubyte[]){255,0,  0  }) // красный
#define gl_LIGHTBLUE    glColor3ubv((const GLubyte[]){128,128,255}) // синий
#define gl_LIGHTCYAN    glColor3ubv((const GLubyte[]){  0,255,255}) // голубой
#define gl_LIGHTGREEN   glColor3ubv((const GLubyte[]){  0,255,0  }) // зеленый
#define gl_LIGHTMAGENTA glColor3ubv((const GLubyte[]){255,0,  255}) // фиолетовый
//efine gl_BACKGROUND   glColor3ubv((const GLubyte[]){204,255,255})
#define gl_LIGHTGRAY    glColor3ubv((const GLubyte[]){191,191,191})
//efine gl_DARKGRAY     glColor3ubv((const GLubyte[]){ 76, 76, 76})
#define gl_GRAY         glColor3ubv((const GLubyte[]){153,153,153})
#define gl_MAGENTA      glColor3ubv((const GLubyte[]){153,0,  153})
//efine gl_BROWN        glColor3ubv((const GLubyte[]){102, 51,0  })
#define gl_CYAN         glColor3ubv((const GLubyte[]){  0,102,102})
#define gl_GREEN        glColor3ubv((const GLubyte[]){  0,102,0  })
#define gl_BLUE         glColor3ubv((const GLubyte[]){  0,0,  128})
#define gl_RED          glColor3ubv((const GLubyte[]){153,0,0})     // 1..18
