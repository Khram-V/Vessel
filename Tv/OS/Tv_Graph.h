///                    +Var ++2.1.12  /90.12.18-11.05.15/
///  Tv_GRAPHics.cpp
///                    -bgi = 0 -  ( Mode )   - VGAMAX
#ifndef __Tv_Graph__    //   1 -  320 x 240  - CGA
#define __Tv_Graph__    //   2 -  640 x 350  - EGA
#include <StdIO.h>      //   3 -  640 x 480  - VGA
#include "Graphics.h"   //   4 -  800 x 600  - SVGA
#include "Basis.h"      //   5 - 1024 x 768  - VESA
                        //   6 - 1280 x 1024
typedef void *HIMG;     //   7 - 1600 x 1200
                        //   8 - 1920 x 1440
int Tv_Graphics_Start( int Mode=-1,int Width=0,int Height=0 ); // Инициализация

class Bfont:Basis_Area  //
{ char *CHR;            // Адрес цифрового массива для прорисовок букв в памяти
  short H,b;            // Заданная и считанная высота и основание букв
  int __OutText(char*); // Собственно вывод строки в настройках углов Base_Area
public:                                // + регистрируемый векторный шрифт:
  short Tw,Th;                         // Ширина и высота одной растровой буквы
  Bfont( void* _f=0,const int _H=-1 ); // Шрифт и его размер, _f=0: растр
void Font( void* _f,const int _H=-1 ); // -1 - не изменять
int Height( const int _Height );       //  0 - взять равным Tv.Th
                                       // >1 - новый размер
                                       // При установке растрового шрифта
                                       // 0: 10x8 <= 640x480; 18x9 >= 800x600
                                      // >0: приводится к ближайшему из нулевых
  int StrLen( const char* );          // Длина строки в точках экрана
  int Text(        Real,Real, const char* );
  int Text( Course,Real,Real, const char* );
  int Text(        int x,int y,const char* );  // Запись горизонтального текста
  int Text( Course,int x,int y,const char* );  // с учетом смещения
  int Text(        point p,    const char* );  // относительно первой точки
  int Text( Course,point p,    const char* );  //
//int OutText( Point,                  char* ); // ?по заданному размеру шрифта
  int OutText( Point,Point,            char* ); // Запись между двумя точками
  int OutText( Point,Point,Point,      char* ); // --//-- с наклоном
  int OutText( Point,Point,Point,Point,char* ); // -- по паре точек в основании
                                                // и покрытии четырехугольника
};                      //
struct  TvMode: Bfont   // Структура базисных параметров
{ short Color,          // Текущий цвет для текстов и рисунков
        BkColor,FColor, // Цвета фона pattern и его штриховки
        Fill,           // Режим закраски фона
        mX,mY;          // Экстремумы размеров X-width и Y-height для терминала
  TvMode();             // Constructor
};                      // ~~~~~~~~//
extern TvMode Tv;                  //
extern point Tpoint;               // Текущая контрольная точка
extern viewporttype Tv_port,Tport; // Графическое и текстовое окно
extern unsigned char _Small_font[];
extern unsigned char _Simplex_font[];
//
//      Tv_place процедура инициализации графического поля
//            AT - {}>0  левый нижний угол и размер окна в процентах
//                 {}<=0 отступы от нижней(Jy) - верхней(Ly),
//                                  левой (Jx) и правой (Lx)
//                                  границ текущего окна в пикселях
//            AF - логическая область существования функции
//
   void Tv_place ( field *AT, Field *AF=0 ); // Разметка области
   void Tv_window( field *AT, Field *AF=0 ); // Выделение и
// void Tv_back();                          //  сброс окна
   void TvClip();                          // Отсечение внешней области
   void TvUnClip();                       //  и ее востановление
   void Tv_resize( int w, int h );       //  Изменение размеров окна
   void Tv_revert(); // bool=false );   // Обращение цветовой палитры
   void Tv_rect();                     // Рамка и
   void Tv_bar();                     //  закраска области
   void Tv_bar( int l, int t, int r, int b );
   void Tv_rect_x( int l, int t, int r, int b );
   void Tv_rect_up( int l, int t, int r, int b, int w );
   void Tv_rect_UpXOR( int l, int t, int r, int b );
   HIMG Tv_store( int,int,int,int, const field *A=0 );
  field Tv_restore();      // Собственно выделение и удаление окна
                           // с запоминанием старых физических размерностей:(A)
//
//     Tv_fl: пересчет размерностей данных в координаты изображения
//
   int Tv_x ( Real X ); Real Tv_X( int x );
   int Tv_y ( Real Y ); Real Tv_Y( int y );
  bool Tv_fc( Point *P );                     // контроль координат изображения
//
//      Tv_cross: отметка точки крестиком
//      Tv_Pause: специальное обращение для отслеживания маркера
//         Size < 0 нанести отметку без сохранения
//              = 0 стереть переносимую отметку
//              > 0 перенести отметку в новое место
//         Step - шаг маркера, управляемый с клавиатуры
//
void    Tv_cross( Point P, int Size=16 );
Course  Tv_pause( Point *P, int Size=16, Real Step=0.0 );
//
//  г==========================================================¬
//  ¦  Блок процедур обычного (параллельного) доступа к мышке  ¦
//  L-----------------------------------------------------------
//
enum{ NoCheck=-20, NoMouse=-10, MouseInstalled=1 };
enum{ _MouseLeftPress   = 2, _MouseRightPress   = 4,
      _MouseLeftDblClick= 8, _MouseRightDblClick=16,
      _MouseLeftRelease =32, _MouseRightRelease =64 };
extern int Tv_MouseState;       //
 int Tv_Start_Mouse();          // Активизация параллельной мышки
 int Tv_Stop_Mouse();           //
 int Tv_MouseStatus();          //
 int Tv_MouseX();               // Текущие координаты
 int Tv_MouseY();               //  мышиного курсора
 int Tv_GetCursor( int&,int& ); // Прямое считывание
void Tv_SetCursor( int, int  ); //  и установка координат курсора
void Tv_CursorEnable();         // Визуализация
void Tv_CursorDisable();        //  и скрытие мышиного курсора
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//      Tv_getc : запрос команды от терминала
//      Tv_getw : -//- без остановки программы
//      Tv_getk : запрос дополнительных клавиш в предыдущей команде
//      Tv_draw : рекурсивная процедура для дорисовывания картинки
//        key:  @P - Печать графической копии терминала на принтере
//              @S - Выход в DOS через SYSTEM (command.com)
//              @D - Рекурсивная процедура дооформления рисунка
//              ^C - exit
//      return(Cr) - <Enter>  - обычный выход из командного режима
//            (14) - <Insert> - выход по команде включения точки
//            (15) - <Delete> - выход по команде исключения точки
//            (16) - <BackSpace>
//            (17) - <Tab>
//            (27) - <Escape>  __________
//       (0xF1..a) - <Function>         //
   void Tv_draw ();                     // Key = <alt>D
   void Tv_close();                     // Key = <ctrl>C
   void Tv_bell ();                     //
   bool Tv_isKey();                     // опрос Win32 - без последствий
 Course Tv_getc( const int _mSec=0 );   //     BLACK ... DARKGRAY
 Course Tv_getw();                      //      BLUE ..x LIGHTBLUE
 Course Tv_getw( Course Simb );         //     GREEN .x. LIGHTGREEN
 Course Tv_getk();                      //      CYAN .xx LIGHTCYAN
//                                      ~~       RED x.. LIGHTRED
//      Две процедуры для разметки осей ~~   MAGENTA x.x LIGHTMAGENTA
   void Tv_axis( bool Lab=false );       //     BROWN xx. YELLOW
   Real Tv_step( Real );                 // LIGHTGRAY xxx WHITE
//
//      Twindow: выделение текстового окна на графическом поле
//         Jx,Jy > 0 - позиционирование от левого-верхнего угла
//              <= 0 - относительно правого нижнего угла изображения
//         Lx,Ly     - размер текстового окна в символах
//         wb        - толшина рамки (0 - линия цветом Color)
                                           //
   void Twindow( int,int,int,int, int=0 ); // Раскрытие текстового окна
   void Tback();                           //   восстановление подложки
//
//      Процедура формирования списка выбора и поиска на графическом мониторе
//        Возвращается команда выхода из поиска, где:
//        Ki - Номер элемента в списке с на выходе из программы;
//      <ret> команда с клавиатуры приведшая к выходу из программы:
//           если с клавиатуры посылается видимый символ, то начинается поиск
//           по всему списку, если происходит сбой в сортировке, то курсор
//           устанавливается на первом символе последнего из выбранных слов.
//
Course Tcase            //
( field& M,             // Размерения матрицы на экране монитора
    int  ls,            // Максимальная длина информационной строки
  char*  Fl(int),       // Построение строчки по номеру в списке
  Course Hl(int),       // Прерывание для запроса с комментарием
    int  N,             // Полная длина списка
    int& Ki,            // Номер текущего элемента (-1 из прошлого)
    int  ret=0          // 1 первый; 0 разовый; -1 повторный вход
);                      //
Course Tcase( field&,int,char* Fl(int),int,int&,int=0 );
//
//  Mlist - Список параметров одного для запроса на терминал
//      INPUT                           ACTIVE
//  skip:skip string                    :номер сроки
//  lf  : 0 - запрос не производится    :длина входного сообщения
//  Msg :NULL - чистое входное поле     :выходной формат
//  dat :NULL & lf<>0 - меню-запрос     :адрес изменяемого объекта
//
#define Mlist( L ) L,(sizeof( L )/sizeof( Mlist ))
 struct Mlist{ short skip,lf; const char *Msg; void *dat; };
 struct mlist{ byte Y,X,lf,S; const char *Msg; void *dat; };
//
//      Tmenu   - Автономная процедура диалога с терминалом  /90.10.03/
//        Y,X   - координаты левого верхнего угла для окна запросов
//        Mlist - список параметров для запросов на терминал
//        Nm    - количество запросов в списке Mlist
//       <Home> - выход без сохранения
//       <End > - выход со считыванием изменений
//       <PgUp> - переход в начало меню
//       <PgDn> - переход в конец меню
//       return - номер активного запроса или -1 при сбросе
//
   int Tmenu(             Mlist*, int,int x=1,int y=0,int ans=-1 );
   int Tmenu( void H(int),Mlist*, int,int x=1,int y=0,int ans=-1 );
struct Display
{
 int Active,Nm,Y,X,K; void( *Hl )(int); mlist *M;
  Display(              Mlist*,int, int=1,int=0 );
  Display( void H(int), Mlist*,int, int=1,int=0 );
 ~Display();
  int Answer( int=-1 ); void Back();
};
//
//   Процедуры обработки файлов и путей к ним
//
extern char* Tfile_Filter;          // Предопределение списка выбираемых файлов
FILE *Topen( char* Path=(char*)"*.*", // Собственное имя или маска поиска файла
       const char* Mode="r",          // Режим использования для чтения/записи
       const char* Extension=0,       // Расширение начальной выборки имени
       const char* Commentary=0 );    // Комментарий в заголовке меню выбора
long long Tfile( FILE* File );        // Вычисление длины открытого файла
long long Tfile( char* PathName );
long long Tfile( char* PathName, const char* Com );
//
//      Tgo : Позиционирование курсора
//            относительно верхнего левого угла терминала
//       x,y>0 - Обычное позиционирование внутри текстового окна
//       x <=0 - Строка приводится к абсолютному левому краю терминала
//       y <=0 - Ноль - абсолютная нижняя строка, далее отсчет снизу-вверх
//
   void Tgo( int x, int y );            // Установка позиции для буквы
   void Tis( int &x,int &y );           // Считывание текущей позиции
   void Tln();                          // Переход на новую строку
   void Tputc( char ch );               // Один символ в текущую позицию
   int  Tprintf( const char*, ... );    // текст в графическом режиме
//
//      Tgetc,s : запрос команды от терминала
//        S     - Изменяемая строка
//        len   - Длина окна запроса        (len<0 - сохранение поля)
//        pos   - Начальная позиция курсора (pos<0 - старая позиция)
//      return  (cr) - <return> - обычный выход из командного режима
//              (14) - <insert>
//              (15) - <delete>
//              (27) - <escape> - выход с отменой команды
//         (0xf1..a) - <function>
//
Course Tgetc( void );
Course Tgets( char* S,int len,int pos=0 );
Course Wgets( char* S,int len,int pos=0 ); // c перекодировкой
  void needle( int,int,int,int );
  void needle( Real,Real,Real,Real ); // изображение стрелочки
  void line  ( Real,Real,Real,Real ); // проведение линии
  void moveto( Real,Real );           // прием текущей отсчётной точки
  void lineto( Real,Real );           // соединение с текущей отсчётной точкой
  int  color  ( int );     // Установка только цвета
  int  bkcolor( int );     // Установка цвета фона
  void pattern( int,int ); // Установка типа и цвета раскраски
  void pattern( int );     // Установка только цвета фона
                           //
inline void moveto( _point p ){ moveto( p.x,p.y ); }
inline void lineto( _point q ){ lineto( q.x,q.y ); }
inline void line  ( _point p,_point q ){ line  ( p.x,p.y,q.x,q.y ); }
inline void needle( _point p,_point q ){ needle( p.x,p.y,q.x,q.y ); }
//
//     Простая и зеленая подсказка
//
  void Ghelp(); inline void Thelp(){ Ghelp(); }
  void Ghelp( const char*,... );  // Временное сообщение
  void Thelp( const char*,... );  // -- в русском
Course Help( const char *Name[], const char** Text, const char** Plus=NULL );
              //
void clear(); // { clearviewport(); }
//
//  256-colors Palette
//
void Get_Palette( int Start, int Count, RGB_palette );
void Set_Palette( int Start, int Count, const RGB_palette );
//
//  Процедуры заменяющие базовые функции и убранные из <Graphics.h>
//
HIMG Tv_GetImage( int l,int t,int r,int b );
void Tv_PutImage( int l,int t,HIMG p,int OP=COPY_PUT );
void Tv_FreeImage( HIMG p );
//
//  Сложные прикладные процедуры и графические комплексы
//
void Map_View( Field wF ); // Сегмент карты в градусах (..\Chart\WorldMap.cpp)
                  //
struct IsoLine    // Список отрезков изолиний для одного заданного уровня Z
{                 //
 int N;           // Количество отрезков
 int *L;          // Длины отрезков
 byte *T;         // Типы отрезков: 0-замкнутый контур; 2-граничная линия
 float Z,**Y,**X; // Уровень и координаты узловых точек
};                //
IsoLine *Tf_curve // Программа подготавливает полный список изолиний с
( Real _Lvl,      // уровнем _Lvl, по полю данных в открытом файле _Fs
  void *_Fs,      // -- прямоугольный массив числовых значений двумерного поля
   int _T,        // Тип данных
   int _Nx,       // Длина исходной строки в файле или в памяти
   int _Jy,int _Jx, int _Ly,int _Lx // Сектор для выборки
);
inline short get2( FILE *F ){ short d=0; fread( &d,2,1,F ); return d; }
inline float getf( FILE *F ){ float d=0; fread( &d,4,1,F ); return d; }
inline void  put2( short d, FILE *F ){ fwrite( &d,2,1,F ); }
inline void  putf( float d, FILE *F ){ fwrite( &d,4,1,F ); }
#endif
