/*﻿    Вариант раскодирования UTF-8 для построения таблиц OEM-866(alt)      ©2019
                                                и Windows-1251(rus)      Code Contination Minimum Maximum        0xxxxxxx    0       0     127        10xxxxxx  err        110xxxxx    1     128    2047        1110xxxx    2    2048   65535 без 55296-57343        11110xxx    3   65536 1114111      в отличие от VAX+RT-11 символы UTF-8 в IBM в обратном порядке*/#include <string.h>#include "../Type.h"
static int nS=0;                       // контрольное число байт в строчках UTF#define X( x )if( (U[x]&0xC0)==0x80 )  // контроль старших бит из других байтов

/*
char* UtC( unsigned &c,const char *V )// синтез Unicode-4 из полноценного UTF-8
{ unsigned u=0; char *U=(char*)V;   // со столь же излишним контролем кодировки
  if( (u=*U)&0x80 )
  { if((*U&0xE0)==0xC0){ X(1){ u=(*U++&0x1F)<<6; u|=*U&0x3F; nS=2; } } else
    if((*U&0xF0)==0xE0){ X(1)X(2){ u=(*U++&0xF)<<12; u|=(*U++&0x3F)<<6; u|=*U&0x3F; nS=3; } } else
    if((*U&0xF8)==0xF0){ X(1)X(2)X(3){ u=(*U++&7)<<18; u|=(*U++&0x3F)<<12; u|=(*U++&0x3F)<<6; u|=*U&0x3F; nS=4; } }
  } else nS=1; c=u; return U+1;
}
unsigned UtC( const char *U ){ unsigned c; UtC( c,U ); return c; }
const char* CtU( unsigned u ){ static char w[5];         // раскодирование UniCode-4 в текстовую строчку UTF  if(u<0x80){ w[0]=u; nS=1; } else  if(u<0x0800){ w[0]=(u>>6)&0x1F|0xC0; w[1]=u&0x3F|0x80; nS=2; } else  if(u<0x10000){ w[0]=(u>>12)&0xF|0xE0; w[1]=(u>>6)&0x3F|0x80; w[2]=u&0x3F|0x80; nS=3; } else  if(u<0x110000){ w[0]=(u>>18)&0x7|0xF0; w[1]=(u>>12)&0x3F|0x80; w[2]=(u>>6)&0x3F|0x80; w[3]=u&0x3F|0x80; nS=4; }  else{ w[0]=0xEF; w[1]=0xBF; w[2]=0xBD; nS=3; } w[nS]=0; return w;}
char* Uget( const char *U )               // однократная выборка UTF-8 символа{ static char S[5],*u; u=(char*)U; nS=0;  // - на пустой строке - выход с нулём  if( *U ){ if( (S[nS++]=*u++)&0x80 )     // иначе выборка хотя бы одного байта  { if((*U&0xE0)==0xC0){ X(1)S[nS++]=*u++; } else    if((*U&0xF0)==0xE0){ X(1)X(2){ S[nS++]=*u++; S[nS++]=*u++; } } else    if((*U&0xF8)==0xF0){ X(1)X(2)X(3){ S[nS++]=*u++; S[nS++]=*u++; S[nS++]=*u++; } }  } } S[nS]=0; return S;}
char* Uset( const char *UTF, int k ){ return Uset( k,UTF ); }
char* Uset( int &k, const char *UTF )          // установка в k-позицию символа{ nS=0; if( !k )return (char*)UTF; else        //  -1 в конец, последовательный
  { int i,l=strlen( UTF ); char *U=(char*)UTF; //     перебор по формату UTF-8
    for( i=0; *U && nS<l && i!=k; i++ )
     if( !(*U&0x80) ){ U++; nS++; } else
     if( (*U&0xE0)==0xC0 ){ X(1){ U+=2; nS+=2; } } else
     if( (*U&0xF0)==0xE0 ){ X(1)X(2){ U+=3; nS+=3; } } else
     if( (*U&0xF8)==0xF0 ){ X(1)X(2)X(3){ U+=4; nS+=4; } }
    k=i; return U;     // U и k приводятся к смещению от начала строки в байтах
} }
int Ulen( const char* UTF )                     // количество реальных символов
{ int k=-1; Uset( k,UTF ); return k;            // в строке UTF-8, пересчёт nS
}
char* Uset( char* U, int k, const char* Sym, bool ins ){ char *s,*b; int bl,cl=0,len=strlen( U );       // вставка символа в позицию k      s=(char*)Uset( U,k );  k=nS;               // k=-1 - в самый конец строки   if( !ins ){ Uget( s );   cl=nS; }             // если старый надо убрать             b=Uget( Sym ); bl=nS;               // новый символ для вставки   if( bl!=cl )                                  // заменяемое поле изменяется
   { if( len-k-cl>0 )memmove( s+bl,s+cl,len-k-cl+1 ); // освобождение фрагмента       else U[len+bl-cl]=0;                          // и фиксация конца строки
   } return (char*)memcpy( s,b,bl )+bl;           // на выходе следующий символ}*/
//  Комплекс процедур исполняется для двух простых форматов байтовых символов://   R)true  DOS-OEM(866)alt с минимумом мат.знаков и палочками для обрамлений//   W)false Windows-1251 - и этот как есть, с кучей всяких "промиллей"//int Usize( const char* str, bool oem )   // длина строки после перевода в UTF-8{ int k,l,m=strlen( str ); unsigned char c;  for( l=k=0; k<m; k++,l++ )if( (c=str[k])>127 ){ ++l;   if( !oem )
   { if( c==130||c>131&&c<138||c==139||c>144&&c<154||c==155||c==185 )++l; } else   if( c>175&&c<224||c==240||c>241&&c<246||c==247||c==249||c>250&&c<253||c>253 )++l;  } return l;}static const char* RtU( char R )    // перекодирование 866(alt) в Unicode-UTF-8{ static const char UTF2OEM[128][4]  = { "А","Б","В","Г","Д","Е","Ж","З", "И","Й","К","Л","М","Н","О","П",  // 128      "Р","С","Т","У","Ф","Х","Ц","Ч", "Ш","Щ","Ъ","Ы","Ь","Э","Ю","Я",  // 144      "а","б","в","г","д","е","ж","з", "и","й","к","л","м","н","о","п",  // 160      "░","▒","▓","│","┤","╡","╢","╖", "╕","╣","║","╗","╝","╜","╛","┐",  // 176      "└","┴","┬","├","─","┼","╞","╟", "╚","╔","╩","╦","╠","═","╬","╧",  // 192      "╨","╤","╥","╙","╘","╒","╓","╫", "╪","┘","┌","█","▄","▌","▐","▀",  // 208      "р","с","т","у","ф","х","ц","ч", "ш","щ","ъ","ы","ь","э","ю","я",  // 224      "≡","±","≥","≤","⌠","⌡","÷","≈", "°","∙","·","√","ⁿ","²","■",""   // 240   // "Ё","ё","Є","є","Ї","ї","Ў","ў",                 "№","¤"    };  if( R&0x80 )return UTF2OEM[R&0x7f];
  static char U[2]={0,0}; U[0]=R; return U;
}
static const char* WtU( char W )    // перекодирование Win-1251 в Unicode-UTF-8{ static const char UTF2Win[128][4]  = { "Ђ","Ѓ","‚","ѓ","„","…","†","‡", "€","‰","Љ","‹","Њ","Ќ","Ћ","Џ",  // 128      "ђ","‘","’","“","”","•","–","—", "₪","™","љ","›","њ","ќ","ћ","џ",  // 144      " ","Ў","ў","Ј","¤","Ґ","¦","§", "Ё","©","Є","«","¬","­","®","Ї",  // 160      "°","±","І","і","ґ","µ","¶","·", "ё","№","є","»","ј","Ѕ","ѕ","ї",  // 176      "А","Б","В","Г","Д","Е","Ж","З", "И","Й","К","Л","М","Н","О","П",  // 192      "Р","С","Т","У","Ф","Х","Ц","Ч", "Ш","Щ","Ъ","Ы","Ь","Э","Ю","Я",  // 208      "а","б","в","г","д","е","ж","з", "и","й","к","л","м","н","о","п",  // 224      "р","с","т","у","ф","х","ц","ч", "ш","щ","ъ","ы","ь","э","ю","я"   // 240    };
  if( W&0x80 )return UTF2Win[W&0x7f];  static char U[2]={0,0}; U[0]=W; return U;}

char* UtOEM( char&,char*,unsigned& );
/*
char* UtOEM( char &s,char *U,unsigned &d ) // символ UTF-8->Russian-OEM(866)alt
{           // АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмноп░▒▓│┤╡╢╖╕╣║╗╝╜╛┐
            // └┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀рстуфхцчшщъыьэюя≡±≥≤⌠⌡÷≈°∙·√ⁿ²■
 const fixed OEM[64]={
 9617,9618,9619,9474,9508,9569,9570,9558, 9557,9571,9553,9559,9565,9564,9563,9488,
 9492,9524,9516,9500,9472,9532,9566,9567, 9562,9556,9577,9574,9568,9552,9580,9575,
 9576,9572,9573,9561,9560,9554,9555,9579, 9578,9496,9484,9608,9604,9612,9616,9600,
 8801, 177,8805,8804,8992,8993, 247,8776,  176,8729, 183,8730,8319, 178,9632,160};
  if( *U<=127 ){ s=*U++,nS=1; return U; } else
  { char c; U=UtC( d,U );                // сборка сложного символа из 1-4 байт
    if( d>=1040 && d<=1103 )
    { if( d<=1087 )s=d-912; else s=d-864; } else  // 1040-128 и 1104-240
    { for( c=0; c<63 && d!=OEM[c]; c++ );
      if( c<48 )c+=176; else c+=192; s=c;
  } } return U;
}
static char* UtWin( char &s, char *U )         // символ UTF-8 -> Windows-1251
{         // ЂЃ‚ѓ„…†‡€‰Љ‹ЊЌЋЏђ‘’“”•–—™љ›њќћџ ЎўЈ¤Ґ¦§Ё©Є«¬­®Ї°±Ііґµ¶·ё№є»јЅѕї
 const fixed Win1251[64]={
 1026,1027,8218,1107,8222,8230,8224,8225, 8364,8240,1033,8249,1034,1036,1035,1039,
 1106,8216,8217,8220,8221,8226,8211,8212, 152 ,8482,1113,8250,1114,1116,1115,1119,
  160,1038,1118,1032, 164,1168, 166, 167, 1025, 169,1028, 171, 172, 173, 174,1031,
  176, 177,1030,1110,1169, 181, 182, 183, 1105,8470,1108,187,1112,1029,1109,1111};
  if( *U<128 ){ s=*U++,nS=1; return U; } else
  { unsigned d; char c; U=UtC( d,U );    // сборка сложного символа из 1-4 байт
    if( d>=1040 && d<=1103 )s=d-848; else   // 1040-192
    { for( c=0; c<64 && d!=Win1251[c]; c++ );
      if( c<64 )c+=128; else c=160; s=c;
  } } return U;
}
*/
static string LS;               //! рабочая строчка текста неограниченной длины
                                // со всеобщим статическим адресом
/*
char* OEMtU( const char *OEM )
{ char *S,*V=(char*)OEM; int i=0;
  while( *V ){ S=(char*)RtU( *V++ ); while( *S )LS[i++]=*S++; } LS[i]=0;
  return LS;
}
char* WintU( const char *Win )
{ char *S,*V=(char*)Win; int i=0;
  while( *V ){ S=(char*)WtU( *V++ ); while( *S )LS[i++]=*S++; } LS[i]=0;
  return LS;
}
char* UtOEM( const char *U )    // строка в буфере UTF-8 -> Russian-OEM(866)alt
{ int k,l=Ulen( U ); char c,*V=(char*)U; unsigned u;
  for( k=0; k<l; k++ ){ V=UtOEM( c,V,u ); LS[k]=c; } LS[k]=0; return LS;
}
char* UtWin( const char *U )    // строка в буфере UTF-8 -> Russian-Win-1251
{ int k,l=Ulen( U ); char c,*V=(char*)U;
  for( k=0; k<l; k++ ){ V=UtWin( c,V ); LS[k]=c; } LS[k]=0; return LS;
}
*/
#if 1
#include <WChar.h>
/*
char* W2U( const wchar_t *str ) /// входная str не может быть унаследованной LS
{ int i,l,iL=wcslen( str ); LS[0]=0;   // в пределах инициированной длины на 2К
   for( i=l=0; i<iL; i++ ){ strcpy( &LS[l],CtU( str[i] ) ); l+=nS; } return LS;
}
wchar_t* U2W( const char* str )   // и здесь только в пределах имеющейся LS.len
{ if( str )if( *str ){ LS[0]=0;   //LS[Usize( str )*2]=0;
    char *s=(char*)str; unsigned u=0; wchar_t *w=(wchar_t*)( (char*)LS );
    while( *s ){ s=UtC( u,s ); *w++=wchar_t( u ); } *w=0;
  } return (wchar_t*)( (char*)LS );
}
*/
#else
#include <windows.h>
//
//    всё будет на UTF-8, и только открытие файлов в Unicode-Windows (UTF-16le)
//
char* W2U( const wchar_t *str )
{ int iL=::WideCharToMultiByte( CP_UTF8,0,str,-1,NULL,0,NULL,NULL ); LS[iL]=0;
         ::WideCharToMultiByte( CP_UTF8,0,str,-1,LS,iL,NULL,NULL ); return LS;
}
wchar_t* U2W( const char* str )
{ int uL=::MultiByteToWideChar( CP_UTF8,0,str,-1,NULL,0 ); LS[uL*2]=0;
         ::MultiByteToWideChar( CP_UTF8,0,str,-1,(LPWSTR)((char*)LS),uL );
  return (wchar_t*)((char*)LS);
}
#endif

/// немного устаревшие тексты
/*
const char* RtU( const char R );         // получение UTF-8 кода из символа OEM
const char* WtU( const char W );         // получение UTF-8 кода из Win-1251
char* UtR( char &s, char *U );           // символ UTF-8 -> Russian-OEM(866)alt
char* UtW( char &s, char *U );           // символ UTF-8 -> Russian-Win-1251
char* UtOEM( char &s, char *U );         // символ UTF-8 -> Russian-OEM(866)alt
char* UlA( char* U, bool oem=false );    // Конвертация из Unicode на месте
char* UtA( const char* U,bool R=false ); // Преобразование строк в буфере ввода
char* AtU( const char* A,bool R=false ); // для OEM-866(true) и Win-1251(false)
*/                                         //
static char* UtI( unsigned &c, const char *u ) // сборка символа из 1-4 байт
{ union{ char c[4]; unsigned d; } v;     // совмещение с выполнением проверки
  char *U=(char*)u; nS=1;                // случится ошибка при nS<2 и v.d>127
  if( (v.d=*U)&0x80 )
  { if((*U&0xE0)==0xC0){ X(1){ v.c[1]=*U++; v.c[0]=*U; nS=2; } } else
    if((*U&0xF0)==0xE0){ X(1)X(2){ v.c[2]=*U++; v.c[1]=*U++; v.c[0]=*U; nS=3; } } else
    if((*U&0xF8)==0xF0){ X(1)X(2)X(3){ v.c[3]=*U++; v.c[2]=*U++; v.c[1]=*U++; v.c[0]=*U; nS=4; } }
  } c=v.d; return ++U;            // на выходе адрес той же строки со смещением
}
static char* UtR( char &s, char *U )     // символ UTF-8 -> Russian-OEM(866)alt
{ unsigned d=U[0];                       // здесь в UTF-8 меняется порядок байт
  if( d<128 )s=*U++,nS=1; else           // особый контроль латыни на ускорение
  {   U=UtI( d,U );                      // сборка сложного символа из 1-4 байт
    if( d>=53392 && d<53440 )s=d-53264; else   // 1040-128 = А..п 53264
    if( d>=53632 && d<53648 )s=d-53408; else   // 1040-224 = р..я 53408
    if( d>=0xE29590 && d<=0xE295AC )           // ═║╒╓╔╕╖╗╘╙╚╛╜╝╞╟╠╡╢╣╤╥╦╧╨╩╪╫╬
    { static char k[29]= { 205,186,213,214,201,184,183,187,212,211,200,190,189,
             188,198,199,204,181,182,185,209,201,203,207,208,202,216,215,206 };
      s = k[d-0xE29590];
    } else                         // °±²·÷(Ё,ё)ⁿ∙√≈≡≤≥⌠⌡─│┌┐└┘├┤┬┴┼ ▀▄█▌▐░▒▓■
    { static unsigned u[36]={ 0xC2B0,0xC2B1,0xC2B2,0xC2B7,0xC3B7,0xD081,0xD191,
       0xE281BF,0xE28899,0xE2889A,0xE28988,0xE289A1,0xE289A4,0xE289A5,0xE28CA0,
       0xE28CA1,0xE29480,0xE29482,0xE2948C,0xE29490,0xE29494,0xE29498,0xE2949C,
       0xE294A4,0xE294AC,0xE294B4,0xE294BC,0xE29680,0xE29684,0xE29688,0xE2968C,
       0xE29690,0xE29691,0xE29692,0xE29693,0xE296A0 }; // 0xEF8C81 = '' → 0xFF
      static char k[36]={ 248,241,253,250,246,133,165,252,249,251,247,240,
                          243,242,244,245,196,179,218,191,192,217,195,180,
                          194,193,197,223,220,219,221,222,176,177,178,254 };
      int i; for( i=0; i<36; i++ )if( d==u[i] ){ s=k[i]; break; }
                                  if( i==36 )s=0xFF;
  } } return U;                          // окончание строки
}                                        //    и возврат её последующего адреса
static char* UtW( char &s, char *U )     // символ UTF-8 -> Russian-OEM(866)alt
{ unsigned d=U[0];                       // здесь в UTF-8 меняется порядок байт
  if( d<128 )s=*U++,nS=1; else           // особый контроль латыни на ускорение
  {   U=UtI( d,U );                      // сборка сложного символа из 1…4 байт
    if( d>=53392 && d<53440 )s=d-53200; else // 1040-192 = А..п
    if( d>=53632 && d<53648 )s=d-53392; else // 1040-240 = р..я
    { // ¤¦§©«¬­®°±µ¶·»ЁЂЃЄЅІЇЈЉЊЋЌЍЎЏёђѓєѕіїјљњћќѝўџҐґ–—―‖‗‘’‚‛“”„‟†‡•…‰‹›₪€№™
      static unsigned u[]={ 0xC2A0,0xC2A4,0xC2A6,0xC2A7,0xC2A9,0xC2AB,0xC2AC,
       0xC2AD,0xC2AE,0xC2B0,0xC2B1,0xC2B5,0xC2B6,0xC2B7,0xC2BB,0xD081,0xD082,
       0xD083,0xD084,0xD085,0xD086,0xD087,0xD088,0xD089,0xD08A,0xD08B,0xD08C,
       0xD08E,0xD08F,0xD191,0xD192,0xD193,0xD194,0xD195,0xD196,0xD197,0xD198,
       0xD199,0xD19A,0xD19B,0xD19C,0xD19E,0xD19F,0xD290,0xD291,
       0xE28093,0xE28094,0xE28098,0xE28099,0xE2809A,0xE2809C,0xE2809D,0xE2809E,
       0xE280A0,0xE280A1,0xE280A2,0xE280A6,0xE280B0,0xE280B9,0xE280BA,0xE282AA,
       0xE282AC,0xE28496,0xE284A2 };
      static char k[]={160,164,166,167,169,171,172,173,174,176,177,181,182,183,
       187,168,128,129,170,189,178,175,163,138,140,142,141,161,143,184,144,131,
       186,190,179,191,188,154,156,158,157,162,159,165,180,150,151,145,146,130,
       147,148,132,134,135,149,133,137,139,155,152,136,185,153 };
      int i; for( i=0; i<64; i++ )if( d==u[i] ){ s=k[i]; break; }
                                  if( i==64 )s=160; // = это неразрывный пробел
  } } return U;               // окончание строки и возврат последующего адреса
}
//#endif
//    Преобразование простых ANSI строк в UTF-8 непосредственно в буфере ввода
//         из исходных однобайтовых OEM-866 и Win-1251 кодировок
//
char* UlA( char* U, bool oem )                  //  UTF-8 -> OEM(866)alt в
{ char *s=U,c; int i=0,k=0,l=strlen( s );       //  последовательном переборе
  while( *s )                                   /// выполняется на том же месте
  { if( oem )s=UtR( c,s ); else s=UtW( c,s ); U[i++]=c; if( (k+=nS)>=l )break;
  } U[i]=0; nS=k; return U;
}
char* UtA( const char* U, bool bit=true )               // UTF-8 -> OEM(866)alt в
{ char c,*s=(char*)U; int i=0,k=0,l=strlen( s );   // последовательном переборе
  while( *s )                                      //  с возможностью наложения
  { if( bit )s=UtR( c,s ); else s=UtW( c,s ); LS[i++]=c; if( (k+=nS)>=l )break;
  } LS[i]=0; nS=k; return LS;
}
char* AtU( const char *str, bool oem )                 // здесь вполне возможно
{ int m=strlen( str ),l=Usize( str,oem ); char *z; LS[l]=0;  // конвертирование
  while( --m>=0 && l>0 )                                // в исходном фрагменте
  { if( oem )z=(char*)RtU( str[m] ); else z=(char*)WtU( str[m] );     // памяти
    for( int j=3; j>=0; j-- )if( z[j] )LS[--l]=z[j];
  } nS=strlen( LS ); return LS;                       // nS=l - как напоминание
}
//
