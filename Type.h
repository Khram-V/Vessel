#ifndef __Type_h__
#define __Type_h__     /// _ подборка констант и типичных операций ¹²³_
#include <math.h>      //  ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
#include <stdlib.h>    // …∞ⁿªºₒₐₓₑₔ‘’„“”«»‹›•∙×±≥≤÷≈≡≠∂δ∆∧∀∨⇒⇔↦←↑→↓↔↕↨∏∑∩∫ƒєæΘ
typedef double Real;   //  просто число ко всем арифметическим операциям §µ₪א®™
typedef const Real& _Real;   // числовая ссылка–константа, процедурный параметр
struct Event; typedef const Event& _Event; // Обобщение / синтез даты и времени
                                                     // ©75 ⇒ Сахалин ↔ ‏יְרוּשָׁלַיִם
//  a 6378245 [м] Сфероид Красовского морских карт России
//  b 6356863.0188 SN\φ Эллипсоид Красовского
//  c 6399698.9018 EW/λ (b+c)/2=6367554.0094 ε≈298.3
//  м 1855.35626248109543 м — сфероидальная миля
//  c ≈ 299 792 458 м·сˉ¹ ρ ≈ 1.025 кг/дм³ ≡ 25‰
const Real _Mile=1852.24637937659918,            // Морская(равнообъёмная) миля
    // радиус эквивалентной сферы по отношению к равнообъёмному единичному кубу
    EqSphere=0.62035049089940001666800681204778, //   r = ³√(3/4/π)   ┌┬─┐╔═╦╗
      // 1.24070098179880003333601362409556 — диаметр D = ³√(6/π)     ├┼░┤║▓║║
     _Pi=3.14159265358979323846264338327950288,  //   π      M_PI     ││▒│╠█╬╣
     _Pd=6.28318530717958647692528676655900576,  //   π x 2  Ø        └┴─┘╚═╩╝
     _Ph=1.57079632679489661923132169163975144,  //   π / 2  M_PI_2   ╒╤╕╓╥╖▐▌
     _iP=0.31830988618379067153776752674503,     //   1 / π  M_1_PI   ╞╪╡╟╫╢▄▀
     _Rd=57.295779513082320876798154814105,      // 180 / π  °\rad    ╘╧╛╙╨╜|¦
     _dR=0.01745329251994329576923690768489,     //   π / 180 rad\°  ¯¯¯¯¯¯¯¯
     _s2=1.41421356237309504880,                 //  M_SQRT2
     _g=9.8106; // м/с²≈9.780318·(1+.005302·sin²φ-.000006·sin²2φ)-.000003086·h
//   _e  2.71828182845904523536028747135266249   //   M_E ⁰ⁱ⁴⁵⁶⁷⁸⁹⁺⁻⁼⁽⁾ⁿ₀₁₂₃₄₅₆₇₈₉
//   φ ≈ 1.61803398874989484820458683436563811 = 1/φ+1 ⇒ (√5+1)/2 = В/Б=Б/мал
enum{ RIGHT=1,LEFT,SHIFT, LCTRL,RCTRL=8,CTRL=12, L_ALT=16,R_ALT=32,ALT=48 };
enum{ _MouseMove,_MouseLeft,_MouseRight,_MouseMiddle=4,_MouseWheel=8 };
enum Course
{ _North_West=3,_North=1,_North_East=9, _Home=3,   _Up =1,_PgUp =9,
        _West=2,_Zenith=0,     _East=8, _Left=2,_Center=0,_Right=8,
  _South_West=6,_South=4,_South_East=12,_End =6, _Down =4,_PgDn=12,
  _Enter=13,_BkSp,_F1,_F2,_F3,_F4,_F5,_F6,_F7,_F8,_F9,_F10,_F11,_F12,
  _Esc=27,_Ins,_Del,_Tab,_Blank=32                 // +5,+7,+10,+31 — в запасе
};
void julday( long day,int& m,int& d,int& y ); // Julian( day ) ⇒ дата( m,d,y )
long julday( int m, int d, int y );          // возврат: дата( m,d,y ) ⇒ Julian
long julday(); Real onetime();              // Текущие дата и время[дни и часы]
struct Event{ long D; Real T;              // Юлианская дата и часы от полуночи
  Event& Now();                           // здесь текущее компьютерное время
  Event(){ Now(); }                      //  ++ в простом/чистом конструкторе
  Event& Check();                      // согласование внешних сложений времени
  Event( _Event cur ): D( cur.D ),T( cur.T ){ Check(); } // заданное событие и
  Event( long d,_Real t=0.0 ): D( d ),T( t ){ Check(); } //   в дате и времени
  Event  operator + (_Real t ){ return Event( D,T+t ); }
  Event  operator - (_Real t ){ return Event( D,T-t ); }
  Event& operator +=(_Real t ){ T+=t; return Check(); }
  Event& operator -=(_Real t ){ T-=t; return Check(); }
  Event& UnPack( int &m,int &d,int &y ){ julday( D,m,d,y ); return *this; }
  long     Pack( int m, int d, int y  ){ return D = julday( m,d,y ); }
  friend Event operator+(_Event B,_Real t ){ Event E( B.D,B.T+t ); return E; }
  friend Real  operator-(_Event A,_Event B ){ return (A.T-B.T)+(A.D-B.D)*24; }
  friend bool operator> (_Event A,_Event B ){ return ( A-B )> 0.0; }
  friend bool operator>=(_Event A,_Event B ){ return ( A-B )>=0.0; }
  friend bool operator< (_Event A,_Event B ){ return ( A-B )< 0.0; }
  friend bool operator<=(_Event A,_Event B ){ return ( A-B )<=0.0; }
  friend bool operator==(_Event A,_Event B ){ return ( A-B )==0.0; }
  friend bool operator!=(_Event A,_Event B ){ return ( A-B )!=0.0; }
};
//    Строчки с предустановкой по 2К, и запасом по 256 байт <<в StrCpy.cpp>>
//
class string{ char *s; int len;       // текстовая строчка неограниченной длины
public: string(); ~string();          // начальная установка=2k очистка выходом
 operator char*(){ return s; }        // обычный доступ по адресу начала строки
 char& operator[]( int k );           //   и до каждого символа, но с контролем
};
//    Операции с угловыми градусами и разметкой шага шкал включены в julian.cpp
//
Real AxisStep( Real );                    // Step for Axis Estimation
char* DtoA( Real,int=0,const char* ="°"); // (±)ⁿ123°45'67"00 как круговая мера
char* AtoD( char*, Real& );               // (°)÷(:) с возможной заменой на ':'
template<class real>int find( const real *A, const real &Ar, int len );
//template<class real>inline real Angle( const real &A )          // A° = ±180°
//                                           { return remainder( A,360 ); }
//    Работа с обычными текстовыми строчками
//
int strcut( char* Str );                 // Убираются концевые пробелы, символы
void Break( const char Msg[], ... );     // с подсчетом оставшейся длины строки
char* fname( const char* FileName );     // Поиск собственно самого имени файла
char* sname( char* ShortFileName  );     // то же имя, с отсечённым расширением
char* fext ( char*, const char* Ext=0 ); // c принудительной заменой расширения
#if defined(_STDIO_H_)||defined(_INC_STDIO) // имён файлов: FileName.Extensions
FILE *FileOpen                           // ++ кодировка длинных строк FileOpen
  ( char *fn, const char *tp, const char *ex, const char *ch, const char *tl );
char *getString( FILE *F );              // Чтение строки на статическом адресе
char *getString( FILE *F, int t );       // неограниченной длины +(-)табуляторы
#endif                                   // результат - в подстрочках getString
int  Usize( const char *A, bool=false ); // длина на выходе из DOS/Win в UTF-8
int  Ulen( const char* UTF );            // количество позиций в строке UTF-8
char* Uget( const char *U );             // однократная выборка UTF-8 символа
char* Uset( const char *UTF, int k );    // установка на индекс -1 конец строки
char* Uset( int &k, const char *U );     //  ++ с подтверждением местоположения
char* Uset( char* U,int k,const char* S,bool ins=false ); // вставка S на U[k]
char* UtR( char &s, char *U );           // символ UTF-8 -> Russian-OEM(866)alt
char* UtW( char &s, char *U );           // символ UTF-8 -> Russian-Win-1251
const char* RtU( const char R );         // получение UTF-8 кода из символа OEM
const char* WtU( const char W );         // получение UTF-8 кода из Win-1251
char* UlA( char* U, bool oem=false );    // Конвертация из Unicode на месте
char* UtA( const char* U, bool=false );  // Преобразование строк в буфере ввода
char* AtU( const char* A, bool=false );  // для OEM-866(true) и Win-1251(false)
char* W2U( const wchar_t* WU );          // чисто Windows прилады перекодировок
wchar_t* U2W( const char* U8 );          // UTF-8 -> UTF-16(LE)=Unicode-Windows
//char* UtC( unsigned &u, char *U );     // UTF-8 -> UniCode, на выходе адрес
//const char* CtU( unsigned u );         // UniCode -> UTF-8 (int->string)

//template<class T>inline const T abs(const T &A){ return A<0?-A:A; }
template<class T>inline const T &minmax( const T &a, const T &b, const T &c )
                                  { return a<b^b>c ? b : ( a<c^b>c ? a:c ); }
//                                { return c>=a ? ( a>=b?a:( c<=b?c:b ) )
//                                              : ( c>=b?c:( a<=b?a:b ) ); }
#if !defined( min ) && !defined( max )
template<class T>inline const T &max(const T &a, const T &b){ return a>b?a:b; }
template<class T>inline const T &min(const T &a, const T &b){ return a<b?a:b; }
#endif
#ifndef __Allocate__
void  *Allocate( size_t Sz, void *A=NULL );         // Линейный массив в байтах
void **Allocate( size_t Ny,size_t Sz,void* =NULL ); // Количество и длина строк
size_t isAlloc( void* A ); // ?оперативный объём или количество строк в матрице
#endif
#endif
