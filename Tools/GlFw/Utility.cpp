///
///  Динамическое окошко для кратких подсказок с управляющими кодами программы
///
#include "Window.h"
//
//      Name[0] Title заголовок для титульной строки графического окна
//      Name[1-3] - строки расширенного названия, и признак движения
//      Text    Парное описание основных кодов с их предназначением
//      Plus  - то же для абзаца дополнительных описаний
//              Определение каждого блока строк заканчивается нулевым адресом
void Window::
Help( const char *Name[], const char *Cmds[],const char *Plus[], int X,int Y )
{ Place P( this,PlaceAbove );
  int i,k,c,n=0,t=0,p=0,h=0,l=0;        // Количество строк в Name, Text и Plus
  while( GetKey() );                     // Отсечение всех предыдущих обращений
  P.Activate().Alfabet( 0,"Century",FW_SEMIBOLD );// Размер, тип и шрифт текста
  i=P.AlfaRect( Name[0] ).cx+1;         // Здесь определяется общая ширина окна
  if( Name )while( Name[n] )n++;            // и собственно прорисовка таблички
  if( Cmds )while( Cmds[t] )t++; if( t )h++;       // с информацией и запросами
  if( Plus )while( Plus[p] )p++; if( p )h++; h+=n+(p+1)/2+(t+1)/2+1;
  for( c=1; c<n; c++ )if( (k=i+P.AlfaRect( Name[c] ).cx+1)>l )l=k;
  { TextContext TX( true );
    P.Area( X,Y,3+(l+4)/P.FontWidth,h ).Activate( true );      // текст по строкам
    glColor4f( .6,.8, 1, .6),glRecti( 0,P.Height,P.Width,0 );  //    и столбцам
    glColor4f(.98,.98,.9,.8),glRecti( 3,P.Height-3,P.Width-3,12 );
    glColor3f( 0,0,0.8 );    P.Print( 2,2,Name[0] );
    glColor3f( 0,0,0 );      P.Print( Name[1] );
    for( c=3,k=2; k<n; k++  )P.Print( 2+i/P.FontWidth,c++,Name[k] );
    for( c++,k=0; k<t; k+=2 )glColor3f( 0,0,0 ),P.Print( 2,c++,Cmds[k] ),
                             glColor3f( 0,0,1 ),P.Print( Cmds[k+1] );
    for( c++,k=0; k<p; k+=2 )glColor3f( 0,.8,0 ),P.Print( 2,c++,Plus[k] ),
                             glColor3f( 0,.6,.6 ),P.Print( Plus[k+1] );
    glColor4f(1,1,0,.8);
    P.Alfabet(9,"Times",1,1).Print(-1,0,"©75…22 В.Храмушин");
  } P.Show(); //Save().Refresh();           // среда восстанавливается,
    WaitKey();                              //  а окно будет снято деструктором
}
/*! Display/Tmenu                               1991.06.18-2010.01.07.
     Пакет диалога с терминалом с помощью таблиц запросов
      Y,X   - координаты левого верхнего угла для окна запросов
      Mlist - список параметров для запросов на терминал
      Num   - количество запросов в списке Mlist
    return  - номер последнего активного запроса
*/
#include <StdIO.h>
/* ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
       Mlist - Список параметров для одного запроса на терминал         |
   ~~~ INPUT ~~~~~~~~~~~~~~~~~~~~~~~~~ ACTIVE ~~~~~~~~~~~~~~~~~~~~~~~~~~|
        Y   : skip string                 : номер сроки                 |
        X   : !0 - готовность             : номер позиции для сообщения |
        lf  :  0 - запрос не производится : длина входного сообщения    |
        S   : ~~~~ строящийся формат чтения данных                      |
        Msg : NULL - чистое входное поле  : выходной формат             |
        dat : NULL и lf<>0 - меню-запрос  : адрес изменяемого объекта   |
   —————————————————————————————————————————————————————————————————————'
define Mlist( L ) L,( sizeof( L )/sizeof( Mlist ) )                    /
struct Mlist{ short skip,lf; const char *Msg; void *dat; };          */

struct mlist{ char Y,X,lf,S; char *Msg; void *dat; };

TextMenu::TextMenu( const Mlist *m, const int n, Window* Win, int x,int y ):
  Place( Win,PlaceAbove ),                    // с подстройкой на текущий шрифт
  Up( false ),M( m ),Num( n ),Y( y ),X( x ),Lx( 1 ),Ly( 1 ),K( 0 ) //,Hl( NULL )
{ //Alfabet( FontHeight<2?18:FontHeight+2,"Courier New" );        // Courier-18
  //AlfaBit( _8x16 );                 // Простой растровый шрифт русского языка
}
static char *St,Str[256]="", // Просто рабочая строчка и
                StK[256]=""; //  временно сохраняемое поле
                             //  -- статика, т.к. на выходе не сохраняются
static int Mestr( mlist *Ls )
{ char *S=Ls->Msg; void *dat=Ls->dat; int lc=0,lh=0,k=0,l=Ls->lf;
  Ls->S='s';
  if( !S )*St=0; else                           // без форматной строки St
  { lc=Ulen( strcpy( St,S ) );                  // либо делаем копию в St
    while( k<lc && *Uset( St,k )!='%' )k++;     // и ставим k на % формата
  }
  if( dat!=NULL )                               //
  { if( k==lc )strcat( St,(char*)dat ); else    // если знак формата не найден
    { char ch,*sh=Uset( S,lc-1 );               // это последний символ строки
      if( !strcmp( sh,"°" ) )ch=':'; else ch=sh[0]|0x20;  // здесь тип формата
          lh=Uset( S,lc-2 )[0];                 //  точность числовой величины
      if( ch==':' )                             //  и блок с градусными мерами
      { lh-='0';                                //
        Ls->S=':';
        if( unsigned( lh )>3 )lh=0; else if( *Uset( S,k+1 )=='-' )lh=-lh;
        strcpy( Uset( St,k ),DtoA( *(Real*)dat,lh,"°" ) );
      } else                                    // and another formats
      { lh=lh=='l';                             //
        Ls->S=lh ? ch|0x80:ch;                  // (long)|(double)
        if( ch>'d' && ch<'h' )                  //
        { if( lh )          sprintf( St,S,*(double*)dat );
           else             sprintf( St,S,*(float*)dat );
        } else if( ch!='s' )sprintf( St,S,*(long*)dat );
         else               sprintf( St,S,dat );
    } }
  } else if( l ){ if( l<k )l=k; k=0; }
    Ls->lf=l;
    Uset( St,l+=k )[0]=0;
  for( lc=Ulen( St ); lc<l; lc++ )Uset( St,lc," " ); return k;
}
//   Информационные процедуры для активного окна
//
static void Get( mlist *M, int& ked )      // -- команда на считывание
{ if( ked==-1 )return; mlist &L=M[ked];    //    малого текстового поля
  if( L.dat )                              // == считывание измененных буквочек
  { if( L.S==':' )AtoD( StK,*(double*)L.dat ); else
    { static char f[4]="%  "; int k=1;                        // С учетом %lx
      if( L.S&0x80 ){ ++k; f[1]='l'; }else f[2]=0;
          f[k]=L.S&~0x80;
      if( f[k]=='s' )strcpy( (char*)L.dat,StK );
            else     sscanf( StK,f,L.dat );
  } } ked=-1;
}
//    Собственно представление меню и организация редактуры исходных данных
//
int TextMenu::Answer( int ans )
{ int ked=-1,                    // номер редактируемого поля (-1 без редакции)
      kurs=0, i,k,l;             // kurs - положение курсора внутри строки
  mlist *Ls=(mlist*)M;           // собственно список запросов / форматов
  bool ins=false;                // признак 1-вставки / 0-замены буквочек
  Window *A=Ready();             // ссылка для обращений к активному окну
   if( A )while( A->GetKey() );  // очистка буфера клавиатуры к вызову WaitKey
   else goto Fin;
   //
   //  все расчеты графических позиций и поддержка текстовых запросов
   //
   if( !Up )                        // это дополнение к конструктору
   { bool key=( Ls->X==0 );
     St=Str;                        // Переразметка исходного
     for( k=l=0; k<Num; Ls++,k++ )  //        списка запросов
     { if( key )Ls->Y+=Ly;          // новый номер строки
       if( Ls->Y!=Ly )l=0,Ly=Ls->Y; // поиск новой строки
       if( key )Ls->X=l+1;          // позиция начала сообщения
           l += Mestr( Ls );        // новая длина строки
           l += Ls->lf;             // + длина входного поля +1
       if( Lx<l )Lx=l;              // выбор ширины окна     -1
     } Up=true;                     // отметка готовности разметки меню
   }                                //
   if( ans>=0 && ans<Num )K=ans;    // надо чтобы M[K].lf >0
   Ls=(mlist*)M;                    // это уже рабочая установка для Ls
   for( i=0; i<Num; i++ )           // поиск запроса по списку вниз с повтором
    if( Ls[( K+i )%Num].lf!=0 ){ K=( K+i )%Num; break; }
   if( i==Num )return -1;           // и то уж явная ошибка - меню без запросов
   //
   //   основной цикл организации запросов к конкретным строкам меню
   //
   for( ans=0;; )
   { if( Ls[K].dat )l=Ls[K].lf-1; else l=1;   //
     kurs=minmax( 0,kurs,l );                 // контроль позиции внутри строки
Rep: switch( ans )                            //
     { case _Ins  : ins^=true; break;
       case _West : if( kurs>0 ){ --kurs; break; } if( ked==K )break;
       case _North: for( i=Num-1; i>=1; i-- )
                    if( Ls[(K+i)%Num].lf!=0 ){ K=(K+i)%Num; break; } break;
       case _East : if( kurs<l ){ ++kurs; break; } if( ked==K )break;
       case _Tab  : if( A->ScanStatus()&SHIFT ){ ans=_North; goto Rep; }
       case _South: for( i=1; i<=Num; i++ )
                    if( Ls[(K+i)%Num].lf!=0 ){ K=(K+i)%Num; break; } break;
       case _Enter: if( ked==K )Get( Ls,ked );  // принудительное считывание и
                    goto Fin;                   // выход из затянутой процедуры
       case _Esc  : if( ked!=-1 ){ ked=-1; break; }  // и сброс редактирования
                    K=_Esc; goto Fin;                // разрушение меню и выход
       case _BkSp : if( kurs>0 )--kurs;
       case _Del  : Uset( StK,kurs,"",false );
                    Uset( StK,-1," ",true ); ked=K; break;
       case _Blank: if( !Ls[K].dat ){ ked=-1; goto Fin; }
      default: if( Ls[K].dat && ans>=' ' )
             { Uset( StK,kurs,WtU( ans ),ins ); ked=K; if( kurs<l )++kurs;
             }
     }                                                    // пусть новое число?
     if( ked!=-1 && ked!=K ){ K=ked; Get( Ls,ked ); }     // goto Fin; } выхода
     //                                                                  нет...
     //      Все расчеты и представление результатов
     //       -- картинка со списком строк запросов/форматов
     //
     Area( X,Y,Lx+2,Ly+2 ).Activate(true); // активация разметки листа к тексту
     glScalef( FontWidth,-FontHeight,1.0 );
     glTranslatef( 0,-Ly-2,0 );
     { TextContext Tx;
       glColor4f( 0.8,0.9,1,1 );  glRectf( 0,0,Lx+2,Ly+2 );
       glColor4f( 1,1,0.8,0.98 ); glRectf( 0.3,0.3,(Lx+2)-0.3,(Ly+2)-0.5 );
       for( int k=0; k<Num; k++ )
       { mlist &L=Ls[k];
         int i=Mestr( &L ); glColor3f( 0,0.25,0.5 ); Print( L.X+1,L.Y+1,St );
         if( L.lf )
         { glColor3f( k==ked?0.7:1,1,k==K?0:1 );
           glRectf( L.X+i,L.Y+0.2,L.X+i+L.lf,L.Y+1 ); glColor3f( 0,0,0.75 );
           Print( L.X+i+1,L.Y+1,k==ked?StK:Uset( St,i ) );
           if( k==K )if( L.dat )
           { glBegin( GL_LINE_STRIP ); kurs=max( 0,min( L.lf-1,kurs ) );
             glColor3f( 1,0,0 );
                      glVertex2f( i+L.X+kurs-0.1,L.Y+1.1); // текстовый курсор
             if( ins )glVertex2f( i+L.X+kurs-0.1,L.Y );  // изменяемого символа
                 else glVertex2f( i+L.X+kurs+1,L.Y+1.1 ); glEnd();
             if( ked==-1 )strcpy( StK,Uset( St,i ) );  // изменяемая подстрочка
         } }
       } Show();
     } ans=A->WaitKey();
   }                                   // по выходу из процедуры ???
Fin: return K;                         // площадка с текстом остается на экране
}
