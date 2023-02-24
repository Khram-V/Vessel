//!
//!   Обобщенный класс Window-Place для GLFW-OpenGL
//         == кросс-платформенная графическая среда
//
//                  ©2019-июнь, OpenGL++Данила Гусев, кафедра КММС ПМ-ПУ СПбГУ
//
#ifndef __Window_h__  /// Контекстно-зависимая среда построения трехмерной
#define __Window_h__  /// графики OpenGL с использованием виртуальных процедур
#include <Windows.h>  /// C++ и многооконного интерфейса Windows со стековым
#include <GL\Gl.h>    ///     наложением графических и текстовых фрагментов
#include "..\..\Type.h"
                      //
struct Window;        // производный класс для построения рабочего окна Windows
                      //
enum Place_Signs{ PlaceOrtho=0x40, // трёхмерное ортогональное пространство
                  PlaceAbove=0x80 // сохранение-восстановление фоновой подложки
                };                            // совсем простенькие старенькие
extern unsigned char _8x08[],_8x14[],_8x16[]; // растровые шрифты из эпохи СССР

class Place              // базовый класс графической площадки/текстового листа
{ Window *Site;               // ссылка на родительский контекст устройства
  Place *Up;                  // адрес в последовательном списке наложений
  byte *BitFont;              // временная установка старого растра из DispCCCP
  int MouseState,             // внутренний статус мышки
      pX,pY,                  // местоположение относительно родительского окна
      FontBase,               // индекс списка ТrueТype/Windows шрифта в OpenGL
      chX,chY;                // позиция символа печати по экрану, как по листу
  HFONT hFont;                // шрифт сохраняется для внутрисистемной метрики
  unsigned *Img;              // временное хранилище для растрового отображения
protected:
  byte Signs;                 // признаки управления страницей: Place_Signs
  int FontWidth,FontHeight;   // заданные ширина и высота символа
public:
  int Width,Height;           // размерения фрагмента / графической площадки
  friend class Window;        // взаимный доступ к элементам связных объектов
  //
  //!     Конструктор должен изначально привязываться к конкретному окну Window
  //        что позволяет пользоваться его контекстом для внутренних настроек
  //
 explicit Place( Window*,byte=0 );    // PlaceOrtho | PlaceAbove - тип площадки
 virtual ~Place();           // деструктор с очисткой всех параметров и адресов
  Window* Ready();           // контроль связности и активности исходного окна
  //! _____________________  ~~ Site - признак вполне живого рабочего окна
  //
  //!  определения и процедуры управления наложенными фрагментами экрана
  //   реализуется создание, позиционирование, активизация и очистка фрагментов
  //   каждая площадка привязывается к окну Window, в том числе временно
  //
  Place& Activate( bool=false ); // активизация графического контекста площадки
  Place& Area( int X,int Y,int W,int H );  // определение и проявление площадки
  //    если X,Y > 0 - отсчеты от левого верхнего угла, иначе - правого нижнего
  //    если Width,Height > 0 - отсчеты в символах, если 0 - до границы окна,
  //                если < 0 - в пикселях и нормальных отсчетах Y - снизу вверх
 virtual bool Draw();    // подготовка среды, вызов внешней процедуры рисования
//{ if( extDraw )if( extDraw() )Save(); Refresh(); return false; }
  Place& Draw( bool( *inDraw )() ){ extDraw=inDraw; return *this; }
  Place& Clear( bool=true ); // расчистка фоновым/true или текущим/false цветом
  Place& Show();    // всего лишь проявление на экране фрагмента буфера экрана
  Place& Save();    // сохранение изображения в собственной памяти площадки Img
  Place& Rest();    // прямое восстановление фрагмента Img на экране и в буфере
  Place& Refresh(); // обновление изображения сразу всех площадок в данном окне
  //
  // в случае с мышкой возможны виртуальные процедуры производного класса, если
  // ж таких нет, или в них нет переопределений, то исполняются базовые функции
  // - в случае свободной транзакции мышка сообщает о необходимости перерисовки
  virtual bool Mouse( int x,int y );
  virtual bool Mouse( int s,int x,int y );
  Place& Mouse( bool( *Pass )(     int,int ) ){ extPass=Pass; return *this; }
  Place& Mouse( bool( *Push )( int,int,int ) ){ extPush=Push; return *this; }
private:
  bool(*extDraw)();    // Рисование по стандартному полю графического контекста
  bool(*extPass)( int X,int Y );            // Две внешние независимые процедуры
  bool(*extPush)( int State, int X,int Y ); //     обработки прерываний от мышки
//
//  шрифт привязан к конкретной графической площадке - текстовой странице Place
//
  Place& String( const char* str );                // чисто неформатная строчка
  Place& String( Course Dir, const Real *P, const char* str );
public:
  SIZE AlfaRect( const char*,bool=0 );// вычисление размеров текстовой подложки
  Place& AlfaBit( byte *CCCP=_8x16 ); // старый русский растр (сходу исключён)
  Place& Alfabet(int=18,const char* ="Courier New",int=FW_DONTCARE,bool=false);
  Place& Text( Course Dir, _Real X, _Real Y, _Real Z, const char* _fmt, ... );
  Place& Text( Course Dir, const Real *P, const char* fmt, ... );
  Place& Print( int x,int y, const char *f, ... ); // лист y/x<=0 -внизу/справа
  Place& Print( const char *f, ... );              // контекстная печать
};
#include "glfw3.h"

class Window: public Place            // стандартное окно Windows для OpenGL
{ const char* Title;                  // изначальный титульный заголовок окна
  const int ScreenWidth,ScreenHeight; // размеры графического экрана компьютера
public:
  friend class Place;               // доступ для базовой графической площадки
  Window *Next;                    // следующее окно в простом списке от начала
  GLFWwindow *glfwWindow;  // pointer to glfw window associated with this class
  //
  //    Локальные данные для обслуживания главного/большого окна Windows
  int WindowX,WindowY;            // выделенное местоположение активного окна
  //
  //    X,Y {>0 левый/верхний}~{0,0=>2/3 остатка,1/4 высоты}~{<0 правый/нижний}
  //        Width,Height >0 -> ширина/высота {0,0 => 800,600}
  //             исходный шрифт в конструкторе: Courier-18
  Window( const char* Title=NULL, int X=100,int Y=200, int W=800,int H=600 );
 ~Window();
//void close();
  //      -- позиционирование +левого/-правого +верхнего/-нижнего угла Window и
  Window& Locate( int X,int Y, int Width,int Height ); // с изменением размеров
  Window& Refresh();         // разборка наложенных площадок c выводом на экран
  //
  //       блок сбора данных от клавиатуры, внутренний контроль мышки и таймера
  //
  void PutChar( byte Key );                 // один символ --> кольцевой буфер
  void PutMouse( UINT State, int x,int y ); // внутренняя обработка прерываний
  void PutTimer();                          //
  //
  // обращение к клавиатуре через активное и контекстно настроенное окно Window
  //        1/2  +  3 - Left/Right.Shift     64 - ScrollLock     ??  - CapsLock
  //        4/8  + 12 - Left/Right.Ctrl     128 - Insert         ??  -  NumLock
  //       16/32 + 48 - Left/Right.Alt
  //!                                    минимальный набор клавиатурных функций
 virtual bool KeyBoard( byte key )
      { if( extKey )if( extKey( key ) ){ Draw(); return true; } return false; }
  Window& KeyBoard( bool(*inKey )( byte )=NULL ){ extKey=inKey; }
  byte GetKey(),ScanKey();            // запрос символа без остановки программы
  byte ScanStatus();                  // запрос сопутствующего кода из буфера
  byte WaitKey();                     // ожидание нового символа от клавиатуры
  //
  //   процедуры таймера, работающего от достигнутого времени с надбавкой
  //   выполняется привязка к основному/большому консольному экрану
  //
 virtual bool Timer();
  //
  //! Динамическое окошко для кратких подсказок с управляющими кодами программы
  //   Name[0]   Title Заголовок для титульной строки графического окна
  //   Name[1-3] - строки расширенного названия, и признак движения
  //   Text      - парное описание основных кодов с их предназначением
  //   Plus      - то же блока блока дополнительных описаний
  //    ++ определение каждого блока строк заканчивается нулевым адресом
  void Help( const char *N[],const char *C[],const char *P[],int X=-1,int Y=1 );

  Window& SetTimer( DWORD mSec,bool(*in)()=NULL );  // время и адрес исполнения
  Window& KillTimer(){ SetTimer( 0 ); }             // сброс таймера - если 0
  //
  //   подборка скрытых параметров и свободных/отвязных транзакций
private:
 struct{ byte Code,Key; } KeyBuffer[0x40]; // накопительный буфер от клавиатуры
  int KeyPos,KeyPas;   // кольцевые счетчики для последовательности символов
  int volatile         // отметки рекурсивности сдерживают повтор изображений
      isTimer,         // по таймеру отслеживается уровень рекурсии транзакций
      isMouse,         // мышка тоже перехлестывается в обрывочной рекурсии
      KeyInterrupt;    // признак первого/неоднократного вхождения в прерывание
  Real dTime,nextTime; // шаг времени и ожидаемое прерывание от таймера   [сек]
  void WaitEvents( bool Stop=false );       // для ожидания следующего события
  byte ModifyKeyWithState( byte Code=0 );   // модификация специальных символов
  bool(*extKey)(byte); // свободная процедура обработки прерываний клавиатуры
  bool(*extTime)();    // внешняя процедура, на выход запрос нового изображения
  int mods;            //=0;
public:
  bool isCursorInside; //=true;     // Flag if cursor inside this window or not
                                   //
  void setMods(int m=0){mods=m;}; // Специальный методу устанавливает последнюю
                                // полученную информацию о состоянии клавиатуры
};
bool WinReady( Window *W=NULL );   // без указания адреса опрашиваются все окна

#define Xpm( X ) ( GetSystemMetrics( SM_CXSCREEN )*Real( X )/100.0 )     // %%X
#define Ypm( Y ) ( GetSystemMetrics( SM_CYSCREEN )*Real( Y )/100.0 )     // %%Y
//
//!  Определения и процедуры управления наложенными фрагментами экрана
//   реализуется создание, позиционирование, активизация и очистка фрагментов
//
extern DWORD volatile
             StartTime; // время начала исполнения программы от запуска Windows
       DWORD GetTime(), // в миллисекундах = timeGetTime = GetTickCount
         ElapsedTime(); // опрокидывание через ~49,7 суток

//
//!  Внешние запросы в виде наложенных на графическое поле текстовых страничек
//   и несколько дополнительных процедур, строго привязанных к конкретному окну
//
struct TextContext{ bool Base,dp,cf,lh; GLenum pm[2]; TextContext(bool=false); ~TextContext(); };
struct Rastr_Sector{ bool dp;  // Подготовка площадки для растровых манипуляций
       Rastr_Sector( int X,int Y, int W,int H ); ~Rastr_Sector(); };
//
//   Процедура регулярно вызывает inStay с задержкой на интервал mSec,
//   во время которой проводится активная обработка прерываний Windows
//   Последовательность вызовов прерывается, если inStay возвращает false
//
void WaitTime( DWORD mSec,bool(*inStay)()=NULL,DWORD=0 ); // пуск интервального таймера
//
struct Mlist{ short skip,lf; const char *Msg; void *dat; };
#define Mlist( L ) L,( sizeof( L )/sizeof( Mlist ) )
//
//      Mlist - Список параметров для одного запроса на терминал
//       skip : пропуск строк              --> номер сроки
//        lf  : 0 - запрос не производится --> длина входного сообщения
//        Msg : NULL - чистое входное поле --> выходной формат
//        dat : NULL & lf<>0 - меню-запрос --> адрес изменяемого объекта
//
class TextMenu: Place //! запрос текстового меню с отсрочкой полного завершения
{ const Mlist *M;   // собственно список меню Mlist/mlist
  const int Num,   // количество строк меню
            Y,X;  // местоположение на экране (++/слева-сверху,--/снизу-справа)
  int Lx,Ly,K;    // размеры и номер редактируемого поля / последнего обращения
//void(*)(int);   // прерывание/подсказка при переходе на новый запрос из меню
  bool  Up;       // признак установки меню на экране
public: TextMenu( const Mlist*,const int, Window*, int=1,int=1 ); //~TextMenu();
  int  Answer( int=-1 );       // немного отсроченный вызов для пущей настройки
};                                             // совсем простенькие старенькие
#endif //__Window_h__
