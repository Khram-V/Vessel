/*﻿
                                                и Windows-1251(rus)
#define X( x )if( (U[x]&0xC0)==0x80 )  // контроль старших бит из других байтов
char* UtC( unsigned &u, char *U ) // синтез буквы Unicode из полноценного UTF-8
#endif
static char* UtI( unsigned &u, char *U ) // сборка сложного символа из 1-4 байт
{ union{ char c[4]; unsigned d; } v; nS=1; // совмещение с выполнением проверки
  if( (v.d=*U)&0x80 )                     // случится ошибка при nS<2 и v.d>127
  { if((*U&0xE0)==0xC0){ X(1){ v.c[1]=*U++; v.c[0]=*U; nS=2; } } else
    if((*U&0xF0)==0xE0){ X(1)X(2){ v.c[2]=*U++; v.c[1]=*U++; v.c[0]=*U; nS=3; } } else
    if((*U&0xF8)==0xF0){ X(1)X(2)X(3){ v.c[3]=*U++; v.c[2]=*U++; v.c[1]=*U++; v.c[0]=*U; nS=4; } }
  } u=v.d; return ++U;            // на выходе адрес той же строки со смещением
}
char* Uget( const char *U )               // однократная выборка UTF-8 символа
char* Uset( const char *UTF, int k ){ return Uset( k,UTF ); }
char* Uset( int &k, const char *UTF )          // установка в k-позицию символа
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
char* Uset( char* U, int k, const char* Sym, bool ins )
   { if( len-k-cl>0 )memmove( s+bl,s+cl,len-k-cl+1 ); // освобождение фрагмента
   } return (char*)memcpy( s,b,bl )+bl;           // на выходе следующий символ
    { if( c==130||c>131&&c<138||c==139||c>144&&c<154||c==155||c==185 )++l; } else
  static char U[2]={0,0}; U[0]=R; return U;
}
  if( W&0x80 )return UTF2Win[W&0x7f];
                                // с единым статическим адресом
char* UlA( char* U, bool oem )                     // UTF-8 -> OEM(866)alt в
  { if( oem )s=UtR( c,s ); else s=UtW( c,s ); U[i++]=c; if( (k+=nS)>=l )break;
{ char c,*s=(char*)U; int i=0,k=0,l=strlen( s );   // последовательном переборе
  while( *s )                                      //  с возможностью наложения
  { if( bit )s=UtR( c,s ); else s=UtW( c,s ); LS[i++]=c; if( (k+=nS)>=l )break;
  } LS[i]=0; nS=k; return LS;
}
char* AtU( const char *str, bool oem )               // здесь также допускается
//
//    всё будет на UTF-8, и только открытие файлов в Unicode-Windows (UTF-16le)
//
char* W2U( const wchar_t *str )
{ int iL=WideCharToMultiByte( CP_UTF8,0,str,-1,NULL,0,NULL,NULL ); LS[iL]=0;
       ::WideCharToMultiByte( CP_UTF8,0,str,-1,LS,iL,NULL,NULL ); return LS;
}
wchar_t* U2W( const char* str )
{ int uL=::MultiByteToWideChar( CP_UTF8,0,str,-1,NULL,0 ); LS[uL*2]=0;// uL*2+1
         ::MultiByteToWideChar( CP_UTF8,0,str,-1,(LPWSTR)((char*)LS),uL );
  return (wchar_t*)((char*)LS);
}
