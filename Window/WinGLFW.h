//!
//!   Обобщенный класс Window-Place для GLFW-OpenGL
//         == кросс-платформенная графическая среда
//
//                  ©2019-июнь, OpenGL++Данила Гусев, кафедра КММС ПМ-ПУ СПбГУ
//
#include "glfw3.h"

class Window: public Place            // стандартное окно Windows для OpenGL
{ const char* Caption;                // изначальный титульный заголовок окна
public:
  friend class Place;               // доступ для базовой графической площадки
  Window *Next;                    // следующее окно в простом списке от начала
  GLFWwindow *glfwWindow;  // pointer to glfw window associated with this class
  //
  //    Локальные данные для обслуживания главного/большого окна Windows
  int ScreenWidth,ScreenHeight;       // размеры графического экрана компьютера
  int WindowX,WindowY;              // выделенное местоположение активного окна
  //
  //    X,Y {>0 левый/верхний}~{0,0=>2/3 остатка,1/4 высоты}~{<0 правый/нижний}
  //        Width,Height >0 -> ширина/высота {0,0 => 800,600}
  //             исходный шрифт в конструкторе: Courier-18
 explicit Window( const char* T=NULL, int X=0,int Y=0, int W=0,int H=0 );
 virtual ~Window();   // деструктор срабатывает в эпилогах производных структур
  void Close();       // иначе: вариант завершения только для базовой структуры
  //      -- позиционирование +левого/-правого +верхнего/-нижнего угла Window и
  Window& Locate( int X,int Y, int Width,int Height ); // с изменением размеров
  Window& Refresh();         // разборка наложенных площадок c выводом на экран
  //
  //       блок сбора данных от клавиатуры, внутренний контроль мышки и таймера
  //
  void PutChar( fixed Key );                // один символ --> кольцевой буфер
  void PutMouse( UINT State, int x,int y ); // внутренняя обработка прерываний
  void PutTimer();                          //
  //
  // обращение к клавиатуре через активное и контекстно настроенное окно Window
  //        1/2  +  3 - Left/Right.Shift     64 - ScrollLock     ??  - CapsLock
  //        4/8  + 12 - Left/Right.Ctrl     128 - Insert         ??  -  NumLock
  //       16/32 + 48 - Left/Right.Alt
  //!                                    минимальный набор клавиатурных функций
 virtual bool KeyBoard( fixed key );
  Window& KeyBoard( bool( *inKey )( fixed ) ){ extKey=inKey; return *this; }
  fixed GetKey(),ScanKey(); // выборка символа или кода без остановки программы
  fixed ScanStatus();  // сопутствующий код от клавиатуры или из буфера выборки
  fixed WaitKey();     // ожидание нового символа от клавиатуры с приостановкой
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
 struct{ fixed Code,Key; } KeyBuffer[lKey+1]; // накопительный буфер клавиатуры
  int KeyPos,KeyPas;   // кольцевые счетчики для последовательности символов
  int volatile         // отметки рекурсивности сдерживают повтор изображений
      isTimer,         // по таймеру отслеживается уровень рекурсии транзакций
      isMouse,         // мышка тоже перехлестывается в обрывочной рекурсии
      onKey;           // признаки вхождения в прерывание или ожидание символа
//    KeyInterrupt;    // признак первого/неоднократного вхождения в прерывание
  Real dTime,nextTime; // шаг времени и ожидаемое прерывание от таймера   [сек]
  fixed KeyStates( fixed Code=0 ); // выбор модификации по специальным символам
  bool(*extKey)(fixed); // свободная процедура обработки прерываний клавиатуры
  bool(*extTime)();    // внешняя процедура, на выход запрос нового изображения
  int mods;            //=0;
public:
  void WaitEvents( bool Stop=false ); // ожидание следующего события falsе-мимо
  bool isCursorInside; //=true;     // Flag if cursor inside this window or not
                                   //
  void setMods(int m=0){mods=m;}; // Специальный методу устанавливает последнюю
                                // полученную информацию о состоянии клавиатуры
  Window& Above();        // вынесение Window на передний план экрана в Windows
  Window& Title( const char* A );  // заголовок дополняется к начальной записи
  Window& Icon( const char* A );  // замена иконки, если это не Icon в RC файле
};
//    Активация с последующим восстановлением исходной графической среды
//
class glContext{ GLFWwindow *was;  // временное задействование контекстного
public:                            // графического 3D-интерфейса того же OpenGL
explicit glContext(const Window*); // конструктор=пролог графического конвейера
        ~glContext();             // деструктор=эпилог-возврат былого контекста
};
#define Xpm( X ) ( GetSystemMetrics( SM_CXSCREEN )*Real( X )/100.0 )     // %%X
#define Ypm( Y ) ( GetSystemMetrics( SM_CYSCREEN )*Real( Y )/100.0 )     // %%Y

