//
//   Tv - сборник примеров для визуализации и конвертации графики и геобаз
//
#include "OS/Window.h"
                             //
struct Mario{ Point G;       // географические координаты точки
             char *cp;       // текстовое название мореографного пункта
            };               //
extern Button All;           // Кнопочка для масштабируемых картинок
//
//      Блок констант и операций ввода/вывода для форматов NetCDF
//
typedef enum            //
{       NC_UNSPECIFIED, //              0 private
        NC_BYTE,        //              1
        NC_CHAR,        //              2
        NC_SHORT,       //              3
        NC_LONG,        //              4
        NC_FLOAT,       //              5
        NC_DOUBLE,      //              6
        NC_BITFIELD,    //              7 private
        NC_STRING,      //              8
        NC_IARRAY,      //              9
        NC_DIMENSION,   //              10 xA
        NC_VARIABLE,    //              11 xB
        NC_ATTRIBUTE    //              12 xC
}       nc_type;        //
extern const char *NC_Type_name[13];
float check( float f );
char  *s_read(FILE* W);  // строка по целым словам из 4 байт
short  i_read(FILE* W);
int    l_read(FILE* W);
float  f_read(FILE* W);
double d_read(FILE* W);
