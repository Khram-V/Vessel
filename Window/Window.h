//
//! «Контекстная графика» – („Window-Place”)
//!  Контекстно-зависимая среда построения трехмерной графики OpenGL
//!       с использованием виртуальных процедур C++
//!       и многооконного интерфейса Windows/Linux-GLFW
//!       со стековым наложением графических и текстовых фрагментов
//!    ++ независимая визуализация при параллельном вычислительном процессе
//!  Обобщенные структуры/классы Window::Place для OpenGL (и GLFW)
//
//                        ©2010-май, В.Н.Храмушин, СахГУ №2010615850/2010-09-08
#if !defined( _Window_h_ )
#define _Window_h_           // аппаратное окружение и стандартная среда
#include <windows.h>         // программирования MinGW_Microsoft-Windows
#include "../Type.h"         // собственно настройка локальных операций

struct Window;               // производный класс постройки нового окна Windows
enum Place_Signs{ PlaceOrtho=0x40,     // трёхмерное ортогональное пространство
                  PlaceAbove=0x80 };   // сохранение-восстановление изображения
class Place                  // структура графической площадки/текстового листа
{ Place *Up;                 // адрес в последовательном списке наложений
  Window *Site;              // ссылка на родительский контекст устройства
  byte Signs;                // признаки управления страницей: Place_Signs\+Img
  int MouseState,xo,yo,      // внутренний статус и отсчётное местоположение
      pX,pY,                 // местоположение относительно родительского окна
      chX,chY,               // позиция символа печати по экрану, как по листу
      cbX;                   // начальный отступ от границ листа +слева/-справа
  unsigned *Img;             // временное хранилище для растрового отображения
protected:                   // информация доступная из базового класса Window
  struct Font                // присоединенный шрифт, ссылка на базовый Window
  { byte *Bit;               // назначение растрового шрифта из эпохи DispCCCP
    HFONT hF;                // шрифт сохраняется для внутрисистемной метрики
    int Base,W,H;            // индекс ТrueТype-OpenGL, ширина и высота символа
  } *Fnt;                    // ссылка на новый шрифт или базовый шрифт Window
public:                      // открытая информация и прочие внешние процедуры
  int Width,Height;          // размерения фрагмента / графической площадки
  friend class Window;       // взаимный доступ к элементам связных объектов
  //
  //!    Конструктор Place изначально привязывается к производному окну Window
  //       что позволяет пользоваться его контекстом для внутренних настроек
  //
 explicit Place( Window*,byte=PlaceAbove );   // ~~ | PlaceOrtho - тип площадки
         ~Place();           // деструктор с очисткой всех переменных и шрифтов
  Window* Ready();           // контроль связности и активности окна Window
  //
  //! определения и процедуры управления наложенными фрагментами экрана
  //  реализуется создание, позиционирование, активизация и очистка фрагментов
  //  каждая площадка привязывается к окну Window, в том числе временно
  //
  Place& Activate( bool=false ); // активизация графического контекста площадки
  Place& Area( int X,int Y,int W,int H );  // определение и проявление площадки
  //   если X,Y > 0 - отсчеты от левого верхнего угла, <=0 - от правого нижнего
  //   если Width,Height > 0 - отсчеты в символах, если 0 - до границы окна,
  //                если < 0 - в пикселях и нормальных отсчетах Y - снизу вверх
 virtual bool Draw();    // подготовка среды, вызов внешней процедуры рисования
  Place& Draw( bool( *inDraw )() ){ extDraw=inDraw; return *this; }
  Place& Clear( bool=true ); // расчистка фоновым/true или текущим/false цветом
  Place& Show();    // всего лишь проявление на экране фрагмента буфера экрана
  Place& Save();    // сохранение изображения в собственной памяти площадки Img
  Place& Rest();    // прямое восстановление фрагмента Img на экране и в буфере
  Place& Refresh(); // обновление изображения сразу всех площадок в окне
  //int  Dive();       -- погружение с временным сбросом наслоений (никому не?)
  //void Rise( int );  -- возврат вверх на указанное число площадок
  //void Free() = деструктор - принудительное освобождение страницы от окна
  //
  // в случае с мышкой возможны виртуальные процедуры производного класса, если
  // ж таких нет, или в них нет переопределений, то исполняются базовые функции
  // - в случае свободной транзакции мышка сообщает о необходимости перерисовки
 virtual bool Mouse(       int x,int y );    // такие транзакции отрисовываются
 virtual bool Mouse( int s,int x,int y );    // на площадке или в исходном окне
  Place& Mouse( bool(*inPass)(     int,int ) ){ extPass=inPass; return *this; }
  Place& Mouse( bool(*inPush)( int,int,int ) ){ extPush=inPush; return *this; }
protected:
  bool( *extPass )( int X,int Y );         // Две внешние независимые процедуры
  bool( *extPush )( int State,int X,int Y );   // обработки прерываний от мышки
private:
  bool( *extDraw )();  // Рисование по стандартному полю графического контекста
  bool volatile isDraw, // новая прорисовка сбрасывается по незавершении старой
                isMouse;
//
//! шрифт привязан к конкретной графической площадке - текстовой странице Place
//
  Place& String( const char* );      // чисто неформатная строчка не изменяется
  Place& String( Course,const Real*,const char* ); //+ горизонтальные надписи
public:                                     // выбор текстового прямоугольника
  SIZE AlfaRect( const char*,bool=false ); // c кодировками true ? ANSI:UTF-8
  Place& AlfaBit( byte *CCCP=NULL );      // OEM сходу исключён _8x08_8x14_8x16
  Place& Alfabet( int=0,const char* ="Courier New",int=FW_MEDIUM,bool=false );
  Place& Print( int x,int y, const char *f, ... ); // лист y/x<=0 -снизу/справа
  Place& Print( const char *f, ... );              // контекстная печать
  Place& Text( Course Dir, _Real X,_Real Y,_Real Z, const char* _fmt, ... );
  Place& Text( Course Dir, const Real *P, const char* fmt, ... );
};
#include <GL/gl.h>   // контекстная графическая среда OpenGL
///
///   Главный объект и процедуры графического окна OpenGL в MicroSoft-Windows
//
class Window: public Place        // стандарт/ное окно Windows для OpenGL
{ friend class Place;             // доступ для базовой графической площадки
  const char* Title;              // изначальный титульный заголовок окна
  //
  //  Локальные данные для обслуживания главного/большого окна Windows
  //! ——————————————————————————— ~~ Site - признак вполне живого рабочего окна
  int WindowX,WindowY;            // выделенное местоположение активного окна
public:
  int ScreenWidth,ScreenHeight;   // размеры графического экрана компьютера
  //
  //    X,Y {>0 левый/верхний}~{0,0=>2/3 остатка,1/4 высоты}~{<0 правый/нижний}
  //        Width,Height >0 -> ширина/высота {0,0 => 800,600}
  //              исходный шрифт в конструкторе: Courier-18
  Window *Next;       // следующее окно в простом списке от начала
       HDC   hDC;     //   ++  PIXELFORMATDESCRIPTOR pfd;
       HWND  hWnd;    //   ++  WNDCLASS wc;
       HGLRC hRC;     //   ++  ATOM atom;
       DWORD mSec;    // интервал прерываний активированного таймера [мил.сек]
  bool InterruptProcedure( UINT message,WPARAM wParam,LPARAM lParam );
 explicit Window( const char* Title=NULL, int X=0,int Y=0, int W=0,int H=0 );
 virtual ~Window();   // деструктор срабатывает в эпилогах производных структур
  void Close();       // иначе: вариант завершения только для базовой структуры
  //        -- позиционирование +левого/-правого +верхнего/-нижнего угла Window
  Window& Locate( int X,int Y, int Width,int Height ); // с изменением размеров
  Window& Refresh();      // и разборка наложенных площадок c выводом на экран
  Window& Above();        // вынесение Window на передний план экрана в Windows
  //
  // обращение к клавиатуре через активное и контекстно настроенное окно Window
  //      1/2  +  3 - Left/Right.Shift   64 - ScrollLock  ?? - CapsLock
  //      4/8  + 12 - Left/Right.Ctrl   128 - Insert      ?? -  NumLock
  //     16/32 + 48 - Left/Right.Alt
  //                           минимальный набор клавиатурных запросов-операций
 virtual bool KeyBoard( byte key );
  Window& KeyBoard( bool( *inKey )( byte ) ){ extKey=inKey; return *this; }
  byte GetKey(),ScanKey();  // выборка символа или кода без остановки программы
  byte ScanStatus();   // сопутствующий код от клавиатуры или из буфера выборки
  byte WaitKey();      // ожидание нового символа от клавиатуры с приостановкой
  //
  //   процедуры таймера, работающего от достигнутого времени ?? с надбавкой
  //      выполняется привязка к основному/большому графическому экрану
  //
  Window& SetTimer( DWORD mSec, bool( *inTime )()=NULL ); // время и транзакция
  Window& KillTimer();                                // если 0 - сброс таймера
 virtual bool Timer();                               // ++ виртуальных процедур
  bool volatile isTimer;   // таймер не отслеживает уровень рекурсии транзакций
  //
  //! Динамическое окошко для кратких подсказок с управляющими кодами программы
  //   Name[0]   Title Заголовок для титульной строки графического окна
  //   Name[1-3] - строки расширенного названия, и признак движения
  //   Text      - парное описание основных кодов с их предназначением
  //   Plus      - то же блока блока дополнительных описаний
  //    ++ определение каждого блока строк заканчивается нулевым адресом
  void Help( const char *N[],const char *C[],const char *P[],int X=-1,int Y=1 );
  //
  void PutChar( byte Key );                  // один символ --> кольцевой буфер
private:      //! подборка скрытых параметров и свободных/отвязанных транзакций
  void PutTimer();      // DWORD iTime )   ==  со временем на момент прерывания
 struct{ byte Code,Key; } KeyBuffer[0x40]; // накопительный буфер от клавиатуры
  int KeyPos,KeyPas;    // кольцевые счетчики для последовательности символов
  UINT_PTR idEvent;     // идентификатор встроенного таймера прерываний =0x12++
  bool volatile         // отметки рекурсивности сдерживают повтор изображений
       isMouse,         // мышка тоже перехлестывается в обрывочной рекурсии
       onKey;           // признаки вхождения в прерывание или ожидание символа
  //
  //   блок сбора данных от клавиатуры, внутренний контроль мышки и таймера
  //
  void PutMouse( UINT State, int x,int y );  // внутренняя обработка прерываний
  bool (*extKey)(byte); //  свободная процедура обработки прерываний клавиатуры
  bool (*extTime)();   // внешняя процедура, на выход запрос нового изображения
};
#define Xpm( X ) ( GetSystemMetrics( SM_CXSCREEN )*Real( X )/100.0 )     // %%X
#define Ypm( Y ) ( GetSystemMetrics( SM_CYSCREEN )*Real( Y )/100.0 )     // %%Y
//
//!  Определения и процедуры управления наложенными фрагментами экрана
//   реализуется создание, позиционирование, активизация и очистка фрагментов
//
extern DWORD volatile
             StartTime, // время начала исполнения программы от запуска Windows
             RealTime;  // время исполнения параллельной процедуры из WaitTime
       DWORD GetTime(), // в миллисекундах = timeGetTime = GetTickCount
         ElapsedTime(); // продолжительность - опрокидывание через ~49,7 суток
//
//  Процедура регулярно вызывает inStay() с контролем времени исполнения>=mWork
//  с задержками на интервалы mWait, во время которых производится активная
//  обработка прерываний Windows для визуализации и/или внешнего управления
//  Последовательность может прерываться, если inStay() возвращает false
//  На выходе RealTime - время исполнения inStay() в миллисекундах
//
DWORD WaitTime( DWORD mWait,       // активная задержка для внешнего управления
                bool(*inFree)()=0, // собственно сам вычислительный эксперимент
                DWORD mWork=0 );   // время на исполнение рабочего цикла [мСек]
bool WinReady( Window *W=0 );      // исполнение запросов и проверка активности
//
//   Предустановки оконного Window и графического интерфейсов OpenGL
//
bool glAct( const Window* );         // простое подключение Window к OpenGL
class glContext{ HDC DC; HGLRC RC;  // временное задействование контекстного
public:                            // графического 3D-интерфейса того же OpenGL
explicit glContext(const Window*); // конструктор=пролог графического конвейера
        ~glContext();             // деструктор=эпилог-возврат былого контекста
};
class TextContext{ const bool Base; // Polygon, Depth_Test, Cool_Face, Lighting
public: TextContext( bool Space=false );  // ++ Space -> матрицы сцены и модели
       ~TextContext();
};
class RasterSector{ // Подготовка площадки растровых манипуляций под glViewPort
public: RasterSector( int X,int Y, int W,int H ); ~RasterSector();
};
//     Внешние запросы для наложенных на графическое поле текстовых страничек
//     и несколько дополнительных процедур не привязанных к конкретному окну
//
struct Mlist{ short skip,lf; const char *Msg; void *dat; };
#define Mlist( L ) L,( sizeof( L )/sizeof( Mlist ) )
//
//      Mlist - Список параметров для одного запроса на терминал
//       skip : пропуск строк              --> номер строки
//        lf  : 0 - запрос не производится --> длина входного сообщения
//        Msg : NULL - чистое входное поле --> выходной формат
//        dat : NULL & lf<>0 - меню-запрос --> адрес изменяемого объекта
//
class TextMenu:Place //! запрос текстового меню с отсрочкой полного завершения
{ const Mlist *M;   // собственно список меню Mlist/mlist
  const int Num,   // общее количество строк меню
            Y,X;  // местоположение на экране (++/слева-сверху,--/снизу-справа)
  int Lx,Ly,K,    // размеры и номер редактируемого поля / последнего обращения
      ked,        // номер реально редактируемого поля (-1 без редактирования)
      kurs;       // - положение курсора внутри поля запроса
  bool Up;        // признак установки меню на экране
//void(*)(int);   // прерывание/подсказка при переходе на новый запрос из меню
 virtual bool Draw(); // разделённая прорисовка для таймера, клавиатуры и мышки
 virtual bool Mouse( int x,int y );        // непрерывное мышиное сопровождение
 virtual bool Mouse( int b,int x,int y );  // и реакция на правую\левую клавишу
public: TextMenu( const Mlist*,const int, Window*,int=1,int=1 ); //~TextMenu();
  int Answer( int=-1 );        // немного отсроченный вызов для пущей настройки
};
extern unsigned char _8x08[],_8x14[],_8x16[]; // растровые шрифты из эпохи СССР
#endif