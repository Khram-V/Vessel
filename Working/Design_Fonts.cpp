//
//   Формирование двоичного дампа векторных буквочек в формате DesignCAD
//
#include <StdIO.h>
#include <String.h>
#include "../Window/ConIO.h"
#include "../Type.h"

int main()
{
 string Design;        // здесь будет сформирован новый шрифт в стиле DesignCAD
    int L=0,           // длина нового шрифта контролируется внешним счётчиком
        Tw=1,Th=1,     // усреднённая ширина и базовая высота шрифта
        Tb=0;          // вертикальное смещение базовой линии

 const char InName[]="DesignCAD.alfa",
//         OutName[]="..\\..\\Window\\Fonts\\DesignCAD.characters";
           OutName[]="Sym_Design.c++";

 FILE *F,*T;
 char *S;
 int nS=0;                                             // номер активной строки

printf( " ‏יְרוּשָׁלַיִם \n открывается файл %s",InName );

  if( F=fopen( InName,"rt" ) )
  if( T=fopen( OutName,"wb" ) )
  { fprintf( T,"//\n//  DesignCAD complex font\n//\n"
             "static const\nsigned char _Design_font[]={ " );
    for(;;)
    if( strcut( S=getString( F ) )>0 )  // пустые строки просто пропускаются
    if( *S!=' ' )                       // пусть всё решается по первой позиции
    { if( feof( F ) )break;
      if( !nS )
      { Design[L++]=Tw=strtol( S,&S,10 ); // осреднённая ширина буквочек
        Design[L++]=Th=strtol( S,&S,10 ); // заданная фиксированная высота букв
        Design[L++]=Tb=strtol( S,&S,10 ); // вертикальный отсчет базовой линии
        Design[L++] =  strtol( S,&S,10 ); // -- пока неопределённо -- 4 байта
        fprintf( T," %d, %d, %d, 0,",Tw,Th,Tb );
        nS=1;
      } else
      { int j=0,k,l,n,nc=0,x=0,y=0;
        l=strlen(strcpy( Design+L,Uget( S )) ); // выборка в UTF-коде, как есть
        fprintf( T,"\n/* %s */ ",Design+L );    //! с графическим комментарием
        for( int i=0; i<l; i++ )j+=fprintf( T,"%d,",Design[L+i] );
        for( ; j<12; j++ )fprintf( T," " );
        k=(L+=l);                         // здесь обновлённое количество точек
        n=strtol( S+l+1,&S,10 );          // исходное количество точек
        Design[k+1]=strtol( S,&S,10 );    // ширина символа
        Design[L+=2]=0;
        for( int i=0; i<n; i++ )
        { x=strtol( S,&S,10 );
          y=strtol( S,&S,10 );            // x  всегда и только неотрицательна
/***/     if( x<0 && x!=y )printf( "\n\7 ~x<0~(%d)->%d, x=%d, y=%d",nS,i,x,y );
          if( x<0 && x==y )Design[L]=0x80; else// количество значимых пар точек
          { Design[L++]+=x; Design[L++]=y+128; Design[L]=0; nc++;
            if( nc>255 )
            { do{ L-=2; nc--; }while( (Design[L]&0x80)==0 );
/***/         printf( "\n\7 ~Ns(%d)<-%d отсечение",nS,nc ); break;
            }
        } }
        fprintf( T," %2d,%-2d,",Design[k]=nc,Design[k+1] ); k+=2;

        for( int i=0; i<nc; i++ )
          fprintf( T," %d,%d,",Design[k+i*2],int( Design[k+i*2+1] )-128 );
        ++nS;
    } }
    fclose( F ); fprintf( T,"\n        0 };\n" );    // завершение блока данных
    fclose( T );
  }
  textcolor( LIGHTGREEN ); printf("\n считано %d полезных строк",nS ); getch();
  return 0;
}
