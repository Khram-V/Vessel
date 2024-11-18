/*     Unicode-UTF-8 - Russian Windows-1251 + OEM-866
//     Программа формальной обработки/преобразования текстовых строк
//       -- убираются/ставятся табуляторы
//       -- сокращается количество пустых строк
//       -- удаление лишних пробелов и нулей в числах
//       -- отсекаются концевые пробелы (всегда)
//       -- устанавливается <CR><LF> или только <LF>
*/
#include <StdIO.h>
#include <Windows.h>
#define Blength 65528u            // 2^16-8 Минимальная длина буферной добавки

static char *LS=NULL;
char* W2U( const wchar_t *str )
{ int iL=WideCharToMultiByte( CP_UTF8,0,str,-1,NULL,0,NULL,NULL ); LS[iL]=0;
         WideCharToMultiByte( CP_UTF8,0,str,-1,LS,iL,NULL,NULL ); return LS;
}
//
//  Грубое отсечение всех управляющих кодов и концевых пробелов в строке
//
static int strcut( char* S )
{ int len=strlen( S ); S+=len; while(len>0 && *--S<=' ')--len,*S=0; return len;
}
#if 0                             //
static char *getString( FILE *F ) // вариант без разбора <CR> возврата каретки
{ static char *S; static int L=0; int c,k=0;       // Чтение текстовой строки
  if( !L )S=(char*)malloc( L=Blength ); S[0]=0;    //      произвольной длины
  if( feof( F ) )return NULL;
  do if( (c=fgetc( F ))!='\r' )                    // конец строки может быть
     if(  c=='\n' || c==EOF )break; else           // до и после выборки строки
       { if( k>=L )S=(char*)realloc( S,L+=Blength ); S[k++]=c; }
  while( !feof( F ) );
  S[k]=0;
  return S;
}
#else
static int ic=0;
static char *getString( FILE *F )
{ static char *S; static int L=0; int k=0;           // Чтение текстовой строки
  if( !L )S=(char*)malloc( L=Blength ); S[0]=0;      //      произвольной длины
  if( feof( F ) || ic==EOF )return NULL;
  for(;;)
  { if( !ic )ic=fgetc( F );                 // с наследием от предыдущей строки
    if( ic==EOF ){ ic=0; break; }           // конец файла без переноса строчки
    if( ic=='\r' ){ ic=fgetc(F); if( ic=='\n' )ic=0; break; } // определённости
    if( ic=='\n' ){ ic=fgetc(F); if( ic=='\r' )ic=0; break; } //  явно маловато
    if( k>=L )S=(char*)realloc( S,L+=Blength+4 ); S[k++]=ic; ic=0; // или буква
  } S[k]=0; strcut( S ); return S;
}
#endif
//      Процедура буферизации вывода в файл
//
static char *OutPut( int l )            // !! необходим предварительный вызов
{ static char *OutBuf=NULL;             //    для доступа к буферному массиву
  static long OutLen=0,OutCur=0;        // собственно выходной буфер
  if( !l )OutCur=0; else OutCur+=l;     // полная неснижаемая длина буфера
  if( OutLen-OutCur<=Blength )          // текущее положение маркера заполнения
      OutBuf=(char*)realloc( OutBuf,OutLen=(OutLen/Blength+2)*Blength );
  return OutBuf+OutCur;
}
//                                               Краткая подсказка по программе
static void Help( void )
{ printf( "\n?Use: Ftext name(s) [ -S0 ][ -T-8 ][ -R ][ -B ]\n" // [ -L72 ][ -C ]
          "     /S±n    empty strings [+max-sub]\n"
          "     /T±n    tabulations   [+set-del]\n"
          "     /B±n    comma digit and blank\n"
          "     /R      exclude <CR> symbol\n"
/*	  "	/C[s|l]	Cyrillic words [small|large]\n"
	  "	/Ln	maxima length of strings\n" */
          "\0     (c)92-24 V.Khramushin\n"
        ); exit( 2 );
}
int main()                   // int Ac, char **Av
{
 BOOL R=TRUE;                // если FALSE, то -  переносы строк без <CR>
//    B=TRUE;                //
 int  T=0,                   // Установка шага табулятора
      E=0,                   // Допустимое количество пустых строк
      B=-1,                  // максимальное количество цифр после точки
                             // =0 то лишь расчистка лишних пробелов и нулей
//    C=0,c=0,U=0,           // Перекодировка русских слов: L=Large; S=Small
      Kn,i,k,l; //,NF,nF=1;  // Количество файлов
 FILE *Fi;                  // файл только один на чтение и затем - под запись
 WIN32_FIND_DATAW FileData;  // Блок-заголовок конкретного файла
 HANDLE hSearch;             // Ссылка на файловый список по директории
 DWORD  dwAttrs=0,           // Признак разрешения записи
        FileLength=0;        // Длина файла
 char *inS,*S; //,*s;        // -- и всего не понемножку
 int Ac; WCHAR **Av=CommandLineToArgvW( GetCommandLineW(),&Ac );
     LS=(char*)calloc( 1,4096 );
//
//      Контроль количества параметров
//
  SetConsoleCP( CP_UTF8 );
  SetConsoleOutputCP( CP_UTF8 );
//SetConsoleCP( ANSI_CHARSET );                         // OEM_CHARSET=866
//SetConsoleOutputCP( OEM_CHARSET ); if( Ac<2 )Help(); // ANSI_CHARSET=1251
  if( Ac<2 )Help();
//
//     Выбор ключей из списка параметров
//
  for( l=k=1; k<Ac; )
  { if( Av[k][0]==L'-' || Av[k][0]==L'/' )
    { switch( Av[k][1]|0x20 )
      { case L'r': R=FALSE; break;
        case L't': T=8; swscanf( Av[k]+2,L"%d",&T ); if( T==0 )T=-1; break;
        case L'b': B=0; swscanf( Av[k]+2,L"%d",&B );                 break;
        case L's': swscanf( Av[k]+2,L"%d",&E );                      break;
       default  : printf( "\n\7?Incorrect switch: /%c\n",W2U( Av[k] )[1] );
       Help();
      }
    } else l++; k++; if( l<k )Av[l]=Av[k];
  } Ac=l;
//
//      Печать информации о режимах работы
//
   printf( "%s[%d] /S%+d /T%+d /B<%+d> /R<%s>\n",
            W2U( Av[Kn] ),Ac,E,T,B,!R?"cr":"-cr+lf" );
/* if( E  )printf( "/S%+d ",E );
   if( T  )printf( "/T%+d ",T );
   if( !B )printf( "/B%+d ",B );
   if( !R )printf( "/R " );
           printf( "\n" );
*/
//      Начало цикла по списку имен в параметрах
//
  for( Kn=1; Kn<Ac; Kn++ )                   // по списку из разбора параметров
  { BOOL done,fFinished=FALSE;
    printf( ">%3d: %-24s\n",Kn,W2U( Av[Kn] ) );      // печатается маска поиска
//
//                  Считывание списка имен по маске файлов в оперативную память
//
    hSearch=FindFirstFileW( Av[Kn],&FileData );
    if( hSearch==INVALID_HANDLE_VALUE )continue;
//
//               Начало цикла по обработке списка файлов из разделов директории
//
    while( !fFinished )                     // Подготовка основного имени файла
    { dwAttrs = GetFileAttributesW( FileData.cFileName );
      done = !( dwAttrs&FILE_ATTRIBUTE_READONLY );          // доступ по записи
      printf( "%8ld Read%s ",FileLength=( FileData.nFileSizeHigh*MAXDWORD )
                           + FileData.nFileSizeLow,done?"/Write":"(only)" );
      if( done && FileLength )
      { if( (Fi=_wfopen( FileData.cFileName,L"rb" ))!=NULL )
        { int Len,m,e=0,t=abs( T );
          long ni=0,no=0; S=OutPut( 0 ); ic=0;      //! установка нового буфера
          while( (inS=getString( Fi ))!=NULL )
          { //Len=0;
            //if( ni ){ if( R )S[Len++]=13; S[Len++]=10; }  // не первая строка
            Len=strlen( inS );
            if( !t ){ strcpy( S,inS ); l=Len; } else // и для начала
            for( l=k=0; k<Len; k++ )                // снимаются все табуляторы
               { if( (S[l]=inS[k])!=9 )l++; else do S[l]=' '; while( ++l%t ); }
            S[l]=0;                              // Отсечение концевых пробелов
            Len=strcut( S );
//            s=S+Len-1;                    //
//            while( Len>0 && *s<=' ' && *s!=12 ){ *s=0; --s; --Len; }
//            if( Len>1 && *s==12 )
//            { while( Len>0 && *s<=' ' ){ *s=0; --s; --Len; } S[Len++]=12;
//            }
            if( Len )e=0; else ++e;          // Запись с проверкой пустых строк
            if( !e || !E || ( E>0 && e<=E )     // + количество пустых строк
                         || ( E<0 && e>-E ) )   // - сброс по несколько строк
            { if( B>=0 )                          // снимаются все лишние пробелы
              { for( i=k=l=0;; )            // i-начало слова; k-за этим словом
                if( S[l]>' ' || S[l]==30 && l<Len )S[k++]=S[l++]; // есть буква
                else                                 // иначе новый разделитель
                { if( k==i+2 && S[i+1]=='0'          // +0 и -0 заменяются на 0
                    && (S[i]=='-' || S[i]=='+' ) )S[i++]='0',S[i++]=' '; // +=2
                  else
                  if( k>0 )                          // первый пробел не в счёт
                  { int j,p,D=0;                     // поиск чисел с точками
                    for( j=i; j<k && D<2; j++ )      // должна быть только
                      if( S[j]=='.' ){ p=j; ++D; }   //              одна точка
                    if( D==1 && k>i+1 )     // хотя бы одна цифра или значок
                    { for( j=i; j<k; j++ )  // число с точкой и знаком в начале
                      { if( j==i && (S[i]=='+' || S[i]=='-') )continue;
                        if( (S[j]<'0' || S[j]>'9') && S[j]!='.' ){ D=0; break; }
                      }
                      if( D )            // найдено число только с одной точкой
                      { int n,np=0;
                        if( B>0 && k-p>B+1 )     // обратная последовательность
                        { if( S[p+=B+1]<='4' )S[k=p]=0; else
                          while( p-->=i )
                          if( S[p]=='.' )S[k=(np=p)+1]=0; else // точка в числе
                          { if( S[p]>='0' && S[p]<='9' )       // или всё число
                            { if( ++S[p]<='9' )
                              { if( !np )S[k=p+1]=0; else S[--k]=0; break;
                              } else
                              { if( np )S[p]='0'; else S[k=p]=0; }
                            } else                         // за границей числа
                            { for( n=k-1; n>p; n-- )S[n+1]=S[n];
                              S[p+1]='1'; S[k]=0; break;
                            }
                          }
                        }
                        if( !np )                 // если точка ещё не вычищена
                        while( S[k-1]=='0' )k--; // все концевые нули убираются
                        if( S[k-1]=='.' ) // точка снимается и становится нулём
                        { if( k-1==i )S[i]='0'; else k--;
                        }
                      }
                    } S[k++]=' '; i=k; // тот самый единственный пробел
                  }
                  if( l>=Len )break;
                  while( S[l]<=' ' && l<Len )l++;         // к следующему слову
                }
                S[k++]=0; Len=strcut( S );
              } else                       // после сжатия не до табуляторов
              if( T>1 )                    // Повторная расстановка табуляторов
              { for( m=l=k=0; l<Len; k++,l++ )
                { if( l%t==0 ){ if( m>1 ){ S[k-=m]=9; k++; } m=0; }
                  if( k<l )S[k]=S[l];
                  if( S[l]==' ' )m++; else m=0;
                } Len=k;
              }
              if( !feof( Fi ) || ic==EOF ){ if( R )S[Len++]=13; S[Len++]=10; }
              S=OutPut( Len ); ni++; no+=Len;
            }
          } fclose( Fi );
            S=OutPut( 0 );              // установка буфера в начало под запись
          if( (Fi=_wfopen( FileData.cFileName,L"wb" ))!=NULL )fwrite( S,1,no,Fi );
               fclose( Fi );
               printf( " %6lu/%-4lu ⇐ %s  ",no,ni,W2U( FileData.cFileName ) );
        } else printf( "Shut" );
      }        printf( "\n" );

      if( !FindNextFileW( hSearch,&FileData ) )
      { if( GetLastError() == ERROR_NO_MORE_FILES )
        { fFinished = TRUE;
        //printf( "No more %s files. Search completed.",W2U( Av[Kn] ) );
        } else{ printf("\7 Couldn't find next file."); return 7; }
      }
    } FindClose( hSearch );                       // Закрывается цепочка поиска
  } return 0;
}
