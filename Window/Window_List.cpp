#include <stdio.h>
#include "..\Window\Window.h"
/*
class TextList: Place // работа с группами списка текстовых последовательностей
  int X=0,Y=0,x=32,y=24, // размерения окна в позициях символов для Place::Area
      N,L,K,k;           // общая длина списка, позиции на строке и и по строке
  char*  (*Fl)( int );   // динамическое представление последовательности строк
  Course (*Hl)( int );   // информация по строке с внешней обработкой списка
                         //   0х01 отметка под действия
                         //   0x02 ~ строка изменена ...
*/
TextList::TextList                     // Построение строчки по номеру в списке
( int _n, Window *Win,                 // Прерывание для запроса с комментарием
  int _X, int _Y,                      // Местоположение последовательности, её
  int _x, int _y                       // размерения в правилах Window-площадки
) : Place( Win,PlaceAbove ),Lines( (List*)calloc( N,sizeof( List ) ) ),
  N(_n),X(_X),Y(_Y),x(_x),y(_y),L(0),K(0),k(-1)
{ AlfaBit(_8x16 ); w=AlfaWidth(); h=AlfaHeight();     // строго в размер шрифта
  L=( Win->Height/h )-2;
  K=( Win->Width/w )/3;
  k=-1234;
}
void TextList::Initial() // первая инициализация после заполнения массива строк
{ int hMax=L,wMax=K,i,j;
  if( x<=0 || x>wMax )          // выбор ширины окна для списка по длинам строк
  for( x=12,i=0; i<N; i++ )
  { j=Ulen( Lines[i].lst )+1; if( x<j )x=j;
  } if( x>wMax )x=wMax; //else if( x<12 )x=12;
  if( y<=0 || y>=hMax || y>N )y=min( hMax-1,N );   // и теперь то же для высоты
  if( Y>0 ){ if( Y+y>hMax )Y=max( 1,hMax-y+1 ); }
      else { if( -Y+y>hMax-1 )Y=min( 0,-hMax+y ); } L=K=k=0; // разметка списка
}

TextList::~TextList(){ free( Lines ); } // Вопросы по незавершёнке должны быть?

fixed TextList::Answer()
{ if( k==-1234 )Initial();        // индекс позиции не может быть отрицательным
  Window *Win; fixed ans=_Center; // массив - может, немного неуклюже, но пусть
  while( Win=Ready() )
  { switch( ans )
    { case _East: if( k<x-1 )++k; else k=0; break;
      case _West: if( k>1 )--k; else k=x-1; break;
      case _North:Win->ScanStatus()&SHIFT ? --L:K--; break;
      case _South:Win->ScanStatus()&SHIFT ? ++L:K++; break;
      case _PgUp: Win->ScanStatus()&SHIFT ? L-=12:K-=12; break;
      case _PgDn: Win->ScanStatus()&SHIFT ? L+=12:K+=12; break;
      case _Tab:  Lines[K+L].cmd^=1; break;
      case ' ':   Lines[K+L].cmd^=2;     // пробные варианты по первой отладке
      case _Center: break;               // команды собираются в коротком слове
     default: goto Fin;                  // нет команды внутри - будет снаружи
    }
    Drawing();                           //! здесь проверяются параметры списка
    ans=Win->WaitKey();
  } Fin: return ans;
}
bool TextList::Drawing()
{ TextContext Ts;
  Frame( X,Y,x+5.5,y,true ); int i;               // и окно запросов клавиатуры
  if( K<0 ){ L+=K,K=0; } else                     // здесь простенький контроль
  if( K>y-1 ){ L+=K-y+1,K=y-1; }                          // с трансграничными
  if( L<0 )L=0; else if( L>N-y )L=N-y;                    //   перестроениями
  glColor4f( .8,.9,1,.7); glRectf( 5.2,K+1,x+6,K+2 );     // двойная прозрачная
  glLineWidth( 1 ),glColor3b( 64,64,64 ),glBegin( GL_LINES );
  for( i=2; i<=y+1; i++ )                                // построчная разметка
   glVertex2i( 5,i-1 ),glVertex2i( 5,i ),glVertex2i( 5,i ),glVertex2i( 6+x,i );
  glColor3b( 100,0,0 );                           // уголок курсора (на замену)
  glVertex2f( 5.5+k,K+1 ),glVertex2f( 5.5+k,K+1.9 );
  glVertex2f( 5.5+k,K+1.9 ),glVertex2f( 6.5+k,K+1.9 ); glEnd();
  for( i=0; i<y; i++ )
  { glColor3f(0,0,1),Print( 1.8,i+1.1,Lines[i+L].cmd&1 ? "√":"·" ); // » \x10
    glColor3f(0,.5,0),Print( 2.6,i+1.1,Lines[i+L].cmd&2 ? "≥":" " ); // \x09
    glColor3f(0,.25,.25),Print( 3.4,i+1.1,Lines[i+L].cmd&4 ? "≈":" " );
    glColor3f(0,0,0),Print( 5.5,i+1.1,Lines[i+L].lst );
  }
 char S[x]; S[i=0]=0; glColor3f( .5,.6,.7 ); AlfaBit( _8x08 );
  if( L>0 ){ Print( 4.5,1,".\x18\x18\x18" ); i=sprintf( S," %d",L ); }
                                            i+=sprintf( S+i," < %d > ",N );
  if( L<N-y ){ Print( 5.5,-1,"\x19\x19\x19" ); sprintf( S+i,"%d ",N-L-y ); }
  Print( 0,-1,S ); Show(); AlfaBit( _8x16 );             // отметка заграничная
  return false;
}
bool TextList::Mouse( int _x, int _y )
{ const Real X=minmax(  5.5,Real(_x)/w,x+5.5 ),
             Y=Real( _y )/h-0.3;        // minmax( -0.5,Real(_y)/h,y+2.0 )-0.3;
  if( X<0 ^ _x<=Width && Y<0 ^ _y<=Height )
  { if( Y<1 )--L,K=0; else
    if( Y>y+1 )++L,K=y-1; else K=Y-1;
    k=X-5.5;   Drawing();               // printf( "\nK= %d, %g, %g  ",K,X,Y );
  } return false;
}
bool TextList::Mouse( int b, int x, int y )
{ if( b&_MouseWheel )                            // аналогично Draw
    { y>=0?--L:L++; if( L<0 )L=0; else if( L>N-y )L=N-y; } else
  if( b&_MouseRight )Lines[K+L].cmd&=~1; else
  if( b&_MouseLeft )Lines[K+L].cmd=1;
  Drawing(); return false;
}
