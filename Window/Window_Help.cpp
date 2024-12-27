///
///  Динамическое окошко для кратких подсказок с управляющими кодами программы
///
#include "Window.h"
  //   Name[0]   Title подзаголовок - название набора инструкций для Window
  //   Name[1-3] – три строки расширенного названия (–и признак движения)
  //   Сmds      – парное описание команд и операций с кратким предназначением
  //   Plus      – то же для блока дополнительных инструкций и подсказок
  //    ++ определение каждого блока строк заканчивается нулевым адресом
  //
void Window::Help
( const char *Name[], const char *Cmds[], const char *Plus[], int X, int Y )
{ Place P( this,PlaceAbove );
  int i,k,c,n=0,t=0,p=0,h=0,l=0;        // Количество строк в Name, Text и Plus
//while( GetKey() );                    // Отсечение всех предыдущих обращений
  P.Activate().AlfaBit( _8x16 );        // ссылка по размерам растрового шрифта
  i=P.AlfaRect( Name[0] ).cx+1;         // Здесь определяется общая ширина окна
  if( Name )while( Name[n] )n++;        //     и собственно прорисовка таблички
  if( Cmds )while( Cmds[t] )t++; if( t )h++;       // с информацией и запросами
  if( Plus )while( Plus[p] )p++; if( p )h++; h+=n+(p+1)/2+(t+1)/2+1;
  for( c=1; c<n; c++ )if( (k=i+P.AlfaRect( Name[c] ).cx+1)>l )l=k;
  { TextContext TX;  // до запроса клавиатуры восстанавливается контекст OpenGL
    P.Area( X,Y,3+(l+4)/P.AlfaWidth(),h ).Activate( true ); // текст по строкам
    glColor4f( .6,.8, 1, .6),glRecti( 0,P.Height-3,P.Width,0 );  //    и столбцам
    glColor4f(.98,.98,.9,.8),glRecti( 3,P.Height-6,P.Width-3,12 );
    glColor3f( 0,0,0.8 );    P.Print( 2,2,Name[0] );
    glColor3f( 0,0,0 );      P.Print( Name[1] );
    for( c=3,k=2; k<n; k++  )P.Print( 2+i/P.AlfaWidth(),c++,Name[k] );
    for( c++,k=0; k<t; k+=2 )glColor3f( 0,0,0 ),P.Print( 2,c++,Cmds[k] ),
                             glColor3f( 0,0,1 ),P.Print( Cmds[k+1] );
    for( c++,k=0; k<p; k+=2 )glColor3f( 0,.8,0 ),P.Print( 2,c++,Plus[k] ),
                             glColor3f( 0,.6,.6 ),P.Print( Plus[k+1] );
    P.AlfaVector( 7,0 );
    glColor4f( 1,1,0,0.8 ); P.Print( -1,0,"©75÷24 В.Храмушин" );
  } P.Show(); //Save().Refresh();            // среда восстанавливается, а
    WaitKey();                               // окно будет снято Р-деструктором
}
