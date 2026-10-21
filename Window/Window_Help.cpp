//
//!  Динамическое окошко для кратких подсказок с управляющими кодами программы
//
#include "Window.h"
//
//    Name[0]   Title подзаголовок - название набора инструкций для Window
//    Name[1-3] – три строки расширенного названия (–и признак движения)
//    Сmds      – парное описание команд и операций с кратким предназначением
//    Plus      – то же для блока дополнительных инструкций и подсказок
//             ++ определение каждого блока строк заканчивается нулевым адресом
//
Course Help
( const char *Name[],const char *Cmds[],const char *Plus[],Window *W,int X,int Y )
{ int i,k,c,n=0,t=0,p=0,h=0,l=0,tw;     // Количество строк в Name, Text и Plus
  if( Name ){ Place P( W,PlaceAbove );
//  while( GetKey() );                  // Отсечение всех предыдущих обращений
    P.AlfaBit(_8x16); tw=P.AlfaWidth(); // ссылка по размерам растрового шрифта
    i=P.AlfaRect( Name[0] ).cx+1;       // Здесь определяется общая ширина окна
    while( Name[n] )                    // и собственно прорисовка таблички
    if( (k=i+P.AlfaRect( Name[n++] ).cx+1)>l )l=k; // с информацией и запросами
    if( Cmds )while( Cmds[t] )t++; if( t )h++;
    if( Plus )while( Plus[p] )p++; if( p )h++; h+=n+(p+1)/2+(t+1)/2+1;
    { TextFrame TX( &P,X,Y,l=3+(l+4)/tw,h,true );      // до запроса клавиатуры
      glColor3f( 0,0,0.8 );    P.Print( 2,2,Name[0] ); // восстанавливается
      glColor3f( 0,0,0 );      P.Print( Name[1] );     // контекст OpenGL
      for( c=3,k=2; k<n; k++  )P.Print( 2+i/tw,c++,Name[k] );
      for( c++,k=0; k<t; k+=2 )glColor3f( 0,0,0 ),P.Print( 2,c++,Cmds[k] ),
                               glColor3f( 0,0,1 ),P.Print( Cmds[k+1] );
      for( c++,k=0; k<p; k+=2 )glColor3f( 0,.8,0 ),P.Print( 2,c++,Plus[k] ),
                               glColor3f( 0,.6,.6 ),P.Print( Plus[k+1] );
      glColor4f( 1,.9,.8,.5 );
      P.AlfaVector( 7,0 ).Print( -1,0,"©75÷26 В.Храмушин" );
    } P.Show();
#pragma omp barrier
#pragma omp single
    W->WaitKey();                        // среда восстанавливается,
  } W->Refresh(); return _Center;        // а окно будет снято Р-деструктором
}
//#pragma omp master // single
//#pragma omp taskwait
//#pragma omp task
// while( WinReady( this ) && !GetKey() )WaitTime( 50 ); //Sleep( 500 );

