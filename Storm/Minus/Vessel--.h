//
//      Кафедра компьютерного моделирования и многопроцессорных систем
//          факультет Прикладной математики - процессов управления
//              Санкт-Петербургский государственный университет
//      Подсекция мореходных качеств судов в штормовых условиях
//              Сахалинское отделение Российского научно-технического
//              общества судостроителей имени Алексея Николаевича Крылова
//      ‏יְרוּשָׁלַיִם
//!     проект Vessel — штормовой и экстремальный ход корабля
//             корпус —> волны -> графика
//             статика —> динамика -> реакции
//             кинематика —> излучение -> гидромеханика
//
//      <Tab>  - перестроение картинки в различные контуры\профили,
//               под полную цветовую раскраску или расчистку
//      <Enter> - раскраска поверхностей или исходная триангуляционная сетка
//
//      Контуры шпангоутов определяются параметрическими функциями, и потому
//         могут быть неоднозначными (в перспективе ката- и тримараны),
//         и отчего востребуется учет многократного пересечения поверхности
//         воды 1 - левый/кормовой и Nframes+1 - правый/носовой перпендикуляры.
//      Штевни задаются функциями абсциссы и ординаты от аппликаты,
//         и устанавливаются относительно диаметральной плоскости корабля
//         0 - ахтерштевень; Nframes+2 - форштевень.
//      Плоскости днища, палубы и транцев замыкаются на диаметральной плоскости
//         автоматически, т.е. нули в таблице плазовых ординат могут опускаться
//
#ifndef __Vessel_
#define __Vessel_
#include <StdIO.h>
#include "../Window/View.h"       // собственная графическая среда Window-Place
#include "../Window/ConIO.h"      // консольные операции — отладочные текстовки

#define StemPost  0x20000000      // индексная маска шпаций вблизи форштевня и
#define SternPost 0x40000000      // в межшпангоутных треугольниках ахтерштевня
#define LeftFrame 0x80000000      // признак смежного левого(в корму) шпангоута
#define FramePost 0xE0000000      // общая маска к особому шпангоутному индексу

extern class Hull *Vessel;        // прямой доступ к графическому контексту
extern class Field *Storm;        // для корпуса корабля и его волнового поля
extern FILE *VIL;                 // настройки с протоколами опытовых испытаний
extern const char *Model[];       // строки описаний модели гидромеханики судна
extern const char *ModelWave[];   // характеристика режима морского волнения
extern const char *Rmbs[];        /* Nord,NtO,NNO,NOtN,NO,NOtO,ONO,OtN,
                                      Ost,OtS,OSO,SOtO,SO,SOtS,SSO,StO,
                                     Su”d,StW,SSW,SWtS,SW,SWtW,WSW,WtS,
                                     West,WtN,WNW,NWtW,NW,NWtN,NNW,NtW */
const Vector Zero={ 0,0,0 },Zenit={ 0,0,1 };
const Real eps = 0.0001,          // 0.1 мм - допустимое сближение точек
            ro = 1025.0,          // плотность морской~опреснённой воды [кг/м³]
            hW = 8.0*_Pi/3.0;     // 1.134 - превышение гребня при выравнивании
                                  //         скорости частиц со скоростью волны
                                  // 0.614 - ослабление / подвсплытие в подошве
                                  //         h·k≈0.75 или λ/h≈π·8/3×2 ƒ{⅜}ћ½
//
//    Flex — пространственная вектор-функция абсолютного скалярного аргумента
//         = параметрическое построение неразрывного трехмерного контура/кривой
//         # отслеживание векторных величин в зависимости от шага во времени
//         k>=0 є { 0÷length-1 } — положительные индексы с прямым отсчётом P[k]
//         k<0  є { length+k÷0 } — отрицательные ≡ обратный отсчёт  P[length-k]
//
class Flex{ Vector *P;            // пространственный контур на корпусе корабля
public: int length;               // означенная длина последовательности точек
  Flex(); ~Flex();                // специальная пространственная интерполяция
 Vector& operator[]( int k );     // доступ к конкретной точке [k]
 Vector& Insert( int k );         // вставка новой точки по индексу [k]
 Vector  Delete( int k );         // точка [k] выталкивается из векторного ряда
 Vector& operator+=( _Vector R ){ return Insert( length )=R; }// в конце списка
 Vector& operator/=( _Vector R ){ return Insert( 0 )=R; } // замещение в начале
};
typedef const Flex& _Flex;  // числовая ссылка-константа ≡ процедурный параметр
//
//!   Каждая вершина треугольника содержит морские и корабельные координаты,
//       а также зарегистрированный на данный момент времени уровень моря
//
struct Vertex; typedef const Vertex& _Vertex;
struct Vertex: Vector,Point       // вершина исчисляемого треугольника обоих СК
{ Real w;                         //  и уровень моря над вершиной треугольника
// Vertex( _Vector V ){ Vertex::operator = ( V ); } // конструктор и собственно
 Vertex( _Vector V ){ *this=V; }     // конструктор и собственно
 Vertex& operator=( _Vector );       // операция присваивания вектора <Vs-view>
 friend bool operator != ( _Vertex a,_Vertex b )
                          { return a.x!=b.x || a.y!=b.y || a.z!=b.z; }
 friend Vector operator - ( _Vector a,_Vertex b ){ return a-(Vector)b; }
 friend Vector operator + ( _Vector a,_Vertex b ){ return a+(Vector)b; }
};
//!   Исходная таблица плазовых ординат и все параметры штормового хода корабля
//
class Hull: public Base,public View // Связный корабельный базис и окно графики
{ int Nframes,Mid;                 // общее количество и номер мидельшпангоута
  Real *Keel;                     // абсциссы шпангоутов под таблицей ординат
  Flex Stern,Stem,                // штевни оконтуривают транцевые расширения
           *Frame,                //   и собственно список шпангоутных контуров
        WaterLine;                // конструктивная ватерлиния - рабочая осадка
  unsigned **Shell;               // обшивка строится по вершинам треугольников
  unsigned& Ins( int n,int=0 );   // вставка в список, 0 в конец, -l от конца
  Vector InShell( int n, unsigned m ); // номер шпации и отсчет индексной маски
  Vector Select( int n,int k );        // оболочка Shell[0..N+1][1..length]
  Point InSpan( int&,int&,bool=true ); // индексы точки: 0 и Nframes+1 - штевни
  virtual bool KeyBoard( byte );  // краткая справка и методы работы с корпусом
//void FullFree();                // destructor: очистка данных старого корпуса
public:                           // конструктор, авторская метка и обнуление
  Hull();                         // чтение теоретического корпуса и 1-й чертеж
//~Hull(){ FullFree(); }          // здесь освобождается вся оперативная память
  Hull& Get( char* ); // изменение посадки, выбор курса и скорости хода корабля
  Hull& Original(); // предустановка гидромеханики вычислительного эксперимента
  Hull& Initial()   // возврат корпуса в исходную позицию по курсу и дифференту
      { Base::Identity(); axiY( Trim ); rotZ( -Course ); return *this; }
  //
  // геометрия корпуса, состояние движения и маршрутные записи курса и скорости
  //
  Real Length,Breadth,Draught,    // конструктивные длина, ширина и осадка  [м]
       Course,dCs,  // курс корабля и обратное время для полуциркуляции п·dT/30
        Speed,cSp;  // настоящая и устанавливаемая скорость хода корабля  [м/с]
  Vector Locate;    // местоположение корабля для отсчётов волновых полей   [м]
  Flex Route,       // маршрут - траектория контролируемого хода корабля  МБ[м]
       Rate,        // скорости смещения корпуса в корабельном базисе   КБ[м/с]
       Swing,Whirl; // и вращательные движения корпуса в углах Крылова  [р,р/с]
  virtual bool Draw();          // виртуальная процедура с настройкой сцены
  Hull& Drawing( byte type=0 );   // 0 ->DrawMode; 1<- корпус; 2<+ профили волн
                                  // прорисовка без настройки графической сцены
  Hull& Moving( _Real Ts );       // движение корпуса под действием внешних сил
  Hull& Floating( bool=false );   // кинематика корабля на морском волнении
                                  // true графика или включение перерасчетов
  Hull& Write();                  // Wavefront Technologies Advanced Visualizer
  byte Statum;  //   0 - кинематическое позиционирование со скольжением корпуса
                //       над волнением по хордам четырёх точек склона, иначе:
                //   1 - объёмное гравитационное всплытие: борт-киль-вертикаль
                //   2 - гидростатическое давление в волне на смоченной обшивке
                //   3 ÷ ≈ вихреисточники обтекания корпуса набегающим потоком
                //   4 - компоненты скорости на обшивке под волновыми гребнями
                //  ~~ ~ полные потоки в механике отражения волн(в перспективе)
                //  ++ ~ обтекание корпуса с трохоидальными волнами по глубине
                //  ~~ - динамическое изменение тензора инерции и центра масс
                //  ~~ - перепады давления по градиентам поверхности ватерлинии
  char *FileName,*ShipName; // Имя корабля или название исходного проекта UTF-8
  Vector DampF, DampM;  // м=0 без демпфирование, м>0 торможение скорости качки
      // под большими углами крена и до опрокидывания сопротивление утраивается
//private:
  byte DrawMode; // 0 прозрачный корпус с проявлением гидростатических центров
	         // 1 поверхность ватерлинии с контурами штевней и шпангоутов
                 // 2 к действующей ватерлинии добавляется подводные обводы
                 // 3 корпус изображается целиком, но без штевней и шпангоутов
                 // 0х4 те же режимы 0÷3, но только с триангуляционными ребрами
  Hull& Config();      // установка параметров корпуса и методов его прорисовки
  Hull& Contour_Lines();   // отрисовка габаритов и профилей волн на белом фоне
  //
  //  размерения и динамические параметры корпуса корабля (Volume=0-обновление)
  //   локальная корабельная система координат определяется в составе структуры
  //   Hull::Base и включает координаты пространственного местоположения
  //   Point{X,Y,Z} и тензор тривекторов поворотов Matrix::Vector{x,y,z}
  //
  Real hX,// поперечная метацентрическая высота задаёт аппликату центра тяжести
      Volume,  iV, // исходное водоизмещение, мгновенный погруженный объем V м³
      Surface, iS, // подводная и смоченная поверхность корпуса            S м²
      Floatage,iF, // исходная и действующая площадь ватерлинии            F м²
      sT,          // интервал времени для графиков с кинематикой XYZ-качки сек
      Trim;        // дифферент задается простым смещением центра величины[рад]
  Vector Ofs,      // внутренние смещения отсчетов по миделю и ватерлинии {x,z}
      Gravity,       // центр тяжести в локальных корабельных координатах     G
      Buoyancy,  vB, // координаты центра величины,центра объема корпуса     cB
      Floatable, vF, // центр площади исходной и действующей ватерлинии      cF
      //         vP, // накопительный вектор сил давления треугольников   [Н/ρ]
                 vW, // нормаль интегро-осредненной площади ватерлинии      (δ)
                 vS, // накопительный вектор сил от перепада уровня моря  [H/ρ]
                 vR, // поворотный момент от криво-наклонной ватерлинии   [Н·м]
      Metacenter,vM; // начальный + текущий поперечные метацентры    в КБ Zm(x)
            // с оценкой мгновенной величины метацентрической высоты в МБ! h(z)
  Matrix inWater,mW, // моменты инерции площади действующей ватерлинии     I м⁴
         inMass; //,mM; моменты инерции исходного и смещенного корпуса     G м⁴

  void ThreeInitial();// начальная очистка всех величин динамических параметров
  void ThreeFixed();   // обработка всех треугольников, фиксация первой статики
  void Three           //      все элементы одного треугольника обшивки корпуса
  ( int Level,_Vertex,_Vertex,_Vertex );      // в локальном корабельном базисе
  //
  //  локальные процедуры разделения геометрических операций с обшивкой корпуса
  //
  void Triangle( _Vertex,_Vertex,_Vertex c );// обработка одного треугольника
  void divideTriangle( _Vertex,_Real,_Vertex,_Real,_Vertex,_Real );
  void drawTriangle( _Vertex,_Vertex,_Vertex );    //! элементарный треугольник
  void waterPoints( _Vector N,_Vector P,_Vector Q ); // разметка ватерлинии
  bool Read( const char[],          // чтение исходной таблицы плазовых ординат
            _Real newDraught=0.0 ); //        изменение осадки на перечитывании
  bool LNext( int,unsigned &m,Vector &L,byte*); // левый/кормовой - шпангоут
  bool RNext( int,unsigned &m,Vector &R,byte*); // правый/носовой контур шпации
 struct InList                   // индексные списки кривых последовательностей
  { unsigned *m; int n,length;   // собственно вектор, истинная и текущая длина
    InList(): m(NULL),n( 0 ),length( 0 ){} ~InList(){ if( m )Allocate( 0,m ); }
    //        operator unsigned(){ if( !length )return 0; return m[length-1]; }
    unsigned& operator[]( int k ){ return m[minmax(0,k,length-1)]; }  // доступ
    unsigned& operator+=( unsigned p )                    // простое дополнение
    { if( length>=n )m=( unsigned* )Allocate( (n+=124)*sizeof(unsigned),m );
      return m[length++]=p;
  } }  wl,wr;            // индексы отрезков ватерлинии внутри шпаций
  Flex wL,wR;            // изменчивые отрезки фрагментов ватерлинии по уровню
public:                  // воды в шпациях левого/правого борта
  Hull& Naviga_Inform    // общая навигационная информация о волнах и движении
           ( Window* );  // картушка морского волнения, курса, скорости корабля
  Hull& wPrint( bool=false ); // информация по кораблю на консоли и в протоколе
};
//
//!  § варианты построения групповых структур трохоидального морского волнения
//
struct Waves:    // пакет групповых структур заданного периода и направления
       Tensor    // с тензорным опорным базисом направления без местоположения
{ const          // где у каждой волны собственные привязки расчетного времени
  char *Title;   // заголовок к идентификации данной волны на графическом поле
  int  Mx,My;    // текущие размерности массивов двумерных волновых полей
  Real // Tw,    // Время относительно волновых фронтов по курсу корабля  [сек]
       Cw,Ow,    // Фазовая скорость и круговая частота бегущей волны [м/с,сˉ¹]
       Ds,       // шаг сетки подбирается под размер волнового бассейна  [метр]
       Wind,     // коэффициент ветрового напряжения на волновых склонах
       Diagonal, // Смещение во времени для запуска волны через границу   [сек]
       Length,   // длина собственно трохоидальной волны внутри пакета   [метр]
       Height,   // высота по двум радиусам циклоиды у вершины волны [коэф.0:1]
       Extend;   // протяженность фронта волны меньше для крутого гребня [метр]
protected:       // вычислительный эксперимент задействует интерполяцию по h,v
  Vector **H,    // аппликата и смещение крупных частиц на поверхности жидкости
         **V;    // трехмерное представление векторов поверхностных скоростей
                 //! блок параметров, доступных в производных числовых объектах
public: Waves( const char Name[] )   // конструктор с возможностью перестроения
      : Title(Name),Mx(0),My(0),Wind(0),H(0),V(0){}  // внешнего ×3× управления
//     ~Waves(){}
  Waves& Get( char*,Real&,Real&,Real& ); // уточнение характеристик пакета волн
  Waves& Initial(_Real L,_Real H,_Real Dir); // характер и направление волн [м]
  Waves& Clear(); // расчистка всех полей с установкой чистых начальных условий
  Vector Amd( _Vector R );     // интерполяция с учётом дрейфовых сдвигов узлов
  Real   AmH( _Vector A );     // со смежными разностями к ближайшей точке
  Waves& Solving(_Real,_Real,_Vector); // ход до Tr по Ts [сек] и пробег dR [м]
  Waves& View(_Real,int,bool); // изображение групповой структуры трохоидальных
  Waves& Ghost( _Real Tr );   // волн со сдвигом картинки вниз на величину ofsZ
  int Color( _Real mH );     // 256 белосиних оттенков по высоте волн Герстнера
//int Slick( _Point A,_Point B,_Vector N );    // ячейка на воде вблизи корпуса
  Vector Wave( _Real T,_Vector R );// время и местоположение частицы в трохоиде
  void Wave( _Real T, Vector &R, Vector &V );   // место и скорость на трохоиде
//Vector dWave( int y,int x );  // вертикальная скорость частицы на поверхности
private:
  void iWave( _Real X,    // сдвиг-распространение в базисе групповой структуры
            Vector &P,    // локальные координаты внутри моделируемой акватории
            Vector &V );  // скорость частицы в трохоидальной траектории-орбите
  Vector Wave( _Real T,int y,int x );     // +++  в отсчетах сеточных индексов
  Waves& ThreeSurface( _Real Ts );        // явная схема для трохоидальных волн
  Waves& ThreeBoundary                    // экстраполяция и внешнее возмущение
         ( _Real Tr, _Real Ts, _Real Step );
  Waves& FreeSurface( _Real Ts );         // ++простое решение в полных потоках
  Waves& FreeBoundary                     // граничные условиями на едином поле
         ( _Real Tr, _Real Ts, _Real Step );
};
//!  Возмущенная поверхность трохоидальных групповых структур морского волнения
//
class Field:       //! Штормовая акватория из трохоидальных волновых структур
public View        //   - средства визуализации OpenGL и интерфейс управления
{                  // §1: исходные данные и размерения для морского бассейна
  Event Instant;   //   Начальный отсчет времени при запуске вычислений в целом
  Vector **Ws;     //   Общее волновое поле уровней моря и скоростей течения
  Place Info;      //   Строчка с информацией от таймера справа-внизу окна
public: const      // §2: основные числовые объекты с процессами моделирования
  Real Tstep,      //   Шаг управляем и одинаков у всех волновых структур [сек]
       tKrat;      //   Дробление шага во времени в отношении исходного Куранта
  Real Trun,       //   Отсчет времени завершённого этапа моделирования   [сек]
       Tlaps,      //   Указание отсчета времени продолжения эксперимента [час]
       Long,Wide,  //   Протяженность и ширина модельного бассейна       [метр]
       dS;         //   Собственно шаг сетки по обобщенной поверхности   [метр]
  int mX,mY;       //   Дробление и размерность матрицы получающегося поля волн
  unsigned Kt;     //   Счётчик исполненных шагов вычислительного эксперимента
                   // §3: предопределение расчетных полей и волновых структур
  Waves Wind,      //   Свежая крутая ветровая волна с обрушающимися гребнями
        Swell,     //   Наиболее интенсивное и ранее сформировавшееся волнение
        Surge;     //   Реликтовое пологое волнение от совсем дальних ураганов
  struct status    //!  Маска настройки режимов вычислительного эксперимента
  { byte wave: 2;  //   0 чистое штилевое море без волнения и ветра
                   //   1 моделирование гидродинамики на теоретическом волнении
                   //   2 простое волновое уравнение с отражением без дисперсии
                   //   3 вычислительная модель трохоидальных волн Герстнера...
                   //   ++ континуально-корпускулярный, полный тензорный подход
    byte peak: 1;  //   0-простое решение в потоках 1-трохоидальные волны {s,z}
    byte draw: 2;  //   0-раскраска 1-контуры 2-с разрядкой 3-волновые профили
    byte view: 2;  //   Тёмная вода в световых оттенках \ светлая с разделением
                   //   + прозрачная вода / разделение слоев волновых структур
  } Exp;
  //
  //  Раздельная визуализация волновых процессов в независимом графическом окне
  //      расчеты выполняются без обращений к функциям графической визуализации
 virtual bool Draw();    // виртуальная подмена графической сцены + эксперимент
private:
 virtual bool Timer();   // регулярное обновление картинки делается в процедуре
 virtual bool KeyBoard(byte); // справка, управление визуализацией эксперимента
  Field& Simulation();  // моделирование волнения в интервале: от Trun до Tlaps
                        //  ... в режиме прерываний проводить счет нежелательно
  Field& Config();      // интерактивная настройка вычислительного эксперимента
  Field& Get( char* );  // настройка вычислительной акватории и штормовых волн
public:
  Field(_Real,_Real,_Real,   // длина и протяженность гребня, шаг времени [м,°]
         Real, Real, Real,   // ветровые волны с обрушающимися гребнями [м,%,°]
         Real, Real, Real,   // свежая морская зыбь недалеко отшумевших ветров
         Real, Real, Real ); // пологие реликтовые волны от удаленных ураганов
// ~Field(){}               // безусловное(бесконтрольное) завершение программы
  Field& Original( bool ); // начальная (true) и промежуточная=0 предустановка
                          // всего вычислительного эксперимента
   Real Value( Point ); // простая билинейная интерполяция всего волнового поля
 Vector Locas( Point );// дополнение морских координат уровнем поверхности воды
 Vector Wave( _Real T,_Vector R );           // суммарное положение
 Vector WaveV(_Real T,_Vector R,Vector &V ); // точки + скорости
//void Slicks( _Point A,_Point B,_Vector N ) // отражение нормальной компоненты
//{ Wind.Slick( A,B,N ),Swell.Slick( A,B,N ),Surge.Slick( A,B,N ); }// скорости
};
void Model_Config( Window* Win );            // запрос по моделям гидромеханики
void wavePrint();// активная информация по волновым полям для текстовой консоли
bool logTime();   // запрос необходимости протокола с протоколируемым временем
void logWave();   // протоколирование состояния групповых структур морских волн
void logMeta();   // протоколирование начальных гидростатических характеристик
void logHydro();  // модель гидромеханики качки и ходкости корабля
void logMdemp();  // коэффициенты демпфирования поступательных смещений корпуса
void logAdemp();  // коэффициенты демпфирования угловой качки
//
//  простые логико-арифметические операции в тригонометрических особенностях
//
inline Real& angle( Real &A ){ return A=remainder( A,_Pd  ); }    // -180°÷180°
//{ if( A>=0 )A=fmod( A,_Pd ); else A=_Pd-fmod(-A,_Pd ); return A; } // 0°÷360°
inline Real angle(_Real A,_Real B ){ return remainder( A-B,_Pd ); } // { -п÷п }
inline Vector& angle( Vector &A){ angle(A.x),angle(A.y),angle(A.z); return A; }
inline Vector operator ~( Vector v ){ v.y=-v.y; return v; }// для другого борта
/*
inline bool intor(_Real F,_Real S,_Real G )        // включение базовой точки G
                 { return G>F ? F<S^S>G :          //  S>F && S<=G == ]F<S<=G]
                          G<F ? F>S^S<G : S==F; }  //  S<F && S>=G == [G<=S<F[
inline bool intob(_Real F,_Real S,_Real G ){ return (S-F)*(S-G)<=0.0; } // обе
*/
#endif

