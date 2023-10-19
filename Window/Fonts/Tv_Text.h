
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
};
                        //
struct  TvMode: Bfont   // Структура базисных параметров
{ short Color,          // Текущий цвет для текстов и рисунков
        BkColor,FColor, // Цвета фона pattern и его штриховки
        Fill,           // Режим закраски фона
        mX,mY;          // Экстремумы размеров X-width и Y-height для терминала
  TvMode();             // Constructor
};                      // ~~~~~~~~//

extern unsigned char _Small_font[];
extern unsigned char _Simplex_font[];
