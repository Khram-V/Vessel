//
//      Простая программа переименования дат с точками
//
#include <StdIO.h>
#include <Windows.h>

int main( int Ac, char **Av )
{
 const
 char *Sa=Ac<2?"*.*":Av[1]; // первый параметр на входе в программу
 char  St[MAX_PATH],        // строка к разбору параметров
       Sb[MAX_PATH];        // буфер для сборки нового имени файла
 WIN32_FIND_DATA FileData;  // Блок-заголовок конкретного файла
 HANDLE hSearch;            // Ссылка на файловый список по директории
 DWORD  dwAttrs=0,          // Признак разрешения записи
        FileLength=0;       // Длина файла
//
//      Теперь это универсальный шрифт аж до 32 бит по каждой букве
//
  SetConsoleCP( 1251 );
  SetConsoleOutputCP( 1251 );
  printf( "\nKaReName %s\n{ Name DD.MM.YY+ => Name 19YY.MM.DD+ }\n",Ac<2?"":Sa );
//
//      Считывание списка имен по маске файлов в оперативную память
//
  if( (hSearch=FindFirstFile( Sa, // на случай указания маски выборки файлов
              &FileData ))!=INVALID_HANDLE_VALUE )
  { BOOL done,fFinished=FALSE; // состояние файлового блока
    int count=0,               // счетчик переименованных файлов
        len=0,ke;              // длина имени и обратное смещение к расширению
    char *Sy=0,*Sm=0,*Sd=0;    // отметки для года.месяца и дня.
//
//      Начало цикла по обработке списка файлов из разделов директории
//
    while( !fFinished )             // Подготовка основного имени файла
    { dwAttrs = GetFileAttributes( FileData.cFileName );
      FileLength=FileData.nFileSizeHigh*MAXDWORD+FileData.nFileSizeLow;
      done =! ( dwAttrs&FILE_ATTRIBUTE_READONLY );  // доступ по записи
      strcpy( Sb,strcpy( St,FileData.cFileName ) );
      printf( "\n%10ld R%c %-30s ",FileLength,done?'W':'o',Sb );
      if( done && FileLength && (len=strlen( St ))>5 )
      { ++count;
        if( St[len-4]=='.' )
        { ke=len-( isdigit( St[len-5] )?4:5 ); Sb[ke]=0;
          if( (Sy=strrchr( Sb,'.' ))!=NULL )
          { *Sy++=0;
            if( (Sm=strrchr( Sb,'.' ))!=NULL )
            { *Sm++=0;
              if( (Sd=strrchr( Sb,' ' ))!=NULL )
              { St[++Sd-Sb]=0;
                if( atoi( Sd )<=31 )
                { if( Sb+ke-Sy<4 )strcat( St,"19" ); strcat( St,Sy );
                                  strcat( St,"." );  strcat( St,Sm );
                                  strcat( St,"." );  strcat( St,Sd );
                                  strcat( St,FileData.cFileName+ke );
                  printf( " ==> %s",St+(Sd-Sb) );
                  rename( FileData.cFileName,St );
                }
              }
            }
          } // printf( " %X %X %X",Sy,Sm,Sd );
        } //if( (Fi=fopen( FileData.cFileName,"rt" ))!=NULL ){}
      }
      if( !FindNextFile( hSearch,&FileData ) )
      { if( GetLastError()==ERROR_NO_MORE_FILES )
        { //printf( "\nNo more %s[%d] files. Search completed.",Sa,count );
          fFinished = TRUE;
        } else
        { printf("\nCouldn't find next file."); return 7;
        }
      }
    }
    FindClose( hSearch );                   // Закрываем хэндл поиска
    printf( "\n\n Renamed %d files\n",count );
         exit( 0 );
  } else exit( 1 );
}
