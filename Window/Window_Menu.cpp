/*!
        Display/Tmenu                               1991.06.18-2010.01.07.

        Пакет диалога с терминалом с помощью таблиц запросов
          Mlist - список параметров для запросов на терминал
          Num   - количество запросов в списке Mlist
          Y,X   - координаты левого верхнего угла для окна запросов
        return  - номер последнего активного запроса
*/
#include <stdio.h>
#include "window.h"
/* ────────────────────────────────────────────────────────────────────┐
       Mlist - Список параметров для одного запроса на терминал        │
   ≈≈≈ INPUT ≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈ ACTIVE ≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈╡
       Y   : skip string                 : номер сроки                 │
       X   : !0 - готовность             : номер позиции для сообщения │
       lf  :  0 - запрос не производится : длина входного сообщения    │
       S   : ~~~~ строящийся формат чтения данных                      │
       Msg : NULL - чистое входное поле  : выходной формат             │
       dat : NULL и lf<>0 - меню-запрос  : адрес изменяемого объекта   │
   ────────────────────────────────────────────────────────────────────┘
define Mlist( L ) L,( sizeof( L )/sizeof( Mlist ) )                   /
struct Mlist{ short skip,lf; const char *Msg; void *dat; };         */

struct mlist{ char Y,X,lf,S; char *Msg; void *dat; };

static bool ins=false;           // InsertKey - признак 1-вставки / 0-замены
static char *St,*StK,*Str=NULL;  // рабочая строчка и временно сохраняемое поле
                                 //== статика, т.к. на выходе не сохраняются

TextMenu::TextMenu( const Mlist *m,const int n, Window* Win, int x,int y ):
  Place( Win,PlaceAbove ),
  Up( false ),M( m ),Num( n ),Y( y ),X( x ),Lx( 1 ),Ly( 1 ),K( 0 ) //,Hl( NULL )
{ Activate()//AlfaVector( 20  );    // с подстройкой на текущий шрифт
            .AlfaBit( _8x16 );      // Простой растровый шрифт русского языка
  if( !Str )StK=( Str=(char*)malloc( 512 ) )+256;  // заведомо избыточная длина
}
static int Mestr( mlist *Ls )
{ char *S=Ls->Msg; void *dat=Ls->dat; int lc=0,lh=0,k=0,l=Ls->lf;
  Ls->S='s';
  if( !S )*St=0; else                            // без форматной строки St
  { lc=Ulen( strcpy( St,S ) );                   // либо делаем копию в St
    while( k<lc && *Uset( St,k )!='%' )k++;      // и ставим k на % формата
  }
  if( dat!=NULL )
  { if( k==lc )strcat( St,(char*)dat ); else     // если знак формата не найден
    { char ch,*sh=Uset( S,lc-1 );                // это последний символ строки
      if( !strcmp( sh,"°" ) )ch=':'; else ch=sh[0]|0x20;   // здесь тип формата
          lh=Uset( S,lc-2 )[0];                  //  точность числовой величины
      if( ch==':' )                              //  и блок с градусными мерами
      { lh-='0';
        Ls->S=':';
        if( unsigned( lh )>3 )lh=0; else if( *Uset( S,k+1 )=='-' )lh=-lh;
        strcpy( Uset( St,k ),DtoA( *(Real*)dat,lh,"°" ) );
      } else                                     // and another formats
      { lh=lh=='l';
        Ls->S=lh ? ch|0x80:ch;                   // (long)|(double)
        if( ch>'d' && ch<'h' )
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
    { static char f[4]="%  "; int k=1;                          // С учетом %lx
      if( L.S&0x80 ){ ++k; f[1]='l'; }else f[2]=0;
          f[k]=L.S&~0x80;
      if( f[k]=='s' )strcpy( (char*)L.dat,StK );
              else   sscanf( StK,f,L.dat );
  } } ked=-1;
}
//    Собственно представление меню и организация редактуры исходных данных
//
int TextMenu::Answer( int ans )
{ int i,k,l;
  mlist *Ls=(mlist*)M;            // собственно список запросов / форматов
  Window *A=Ready();              // ссылка для обращений к активному окну
     ked=-1;                      // номер редактируемого поля(-1 без редакции)
     kurs=0;                      // kurs - положение курсора внутри строки
   if( A )while( A->GetKey() );   // очистка буфера клавиатуры к вызову WaitKey
   else{ ans=0; goto Fin; }
   //
   //  все расчеты графических позиций и поддержка текстовых запросов
   //
   if( !Up )                         // это дополнение к конструктору
   { bool key=( Ls->X==0 ); St=Str;  // Переразметка исходного
     for( k=l=0; k<Num; Ls++,k++ )   //        списка запросов
     { if( key )Ls->Y+=Ly;           // новый номер строки
       if( Ls->Y!=Ly )l=0,Ly=Ls->Y;  // поиск новой строки
       if( key )Ls->X=l+1;           // позиция начала сообщения
           l += Mestr( Ls );         // новая длина строки
           l += Ls->lf;              // + длина входного поля +1
       if( Lx<l )Lx=l;               // выбор ширины окна     -1
     } Up=true;                      // отметка готовности разметки меню
   }                                 //
   if( ans>=0 && ans<Num )K=ans;     // надо чтобы M[K].lf >0
   Ls=(mlist*)M;                     // это уже рабочая установка для Ls
   for( i=0; i<Num; i++ )            // поиск запроса по списку вниз с повтором
    if( Ls[( K+i )%Num].lf!=0 ){ K=( K+i )%Num; break; }
   if( i==Num )return -1;            // то уж явная ошибка - меню без запросов
   //
   //   основной цикл организации запросов к конкретным строкам меню
   //
   for( ans=0;; )
   { if( Ls[K].dat )l=Ls[K].lf-1; else l=1;
     kurs=max( 0,min( l,kurs ) );             // контроль позиции внутри строки
Rep: ins=GetKeyState( VK_INSERT );            // признак 1-вставки / 0-замены
     switch( ans )
     {//case _Ins : ins^=true; break;
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
             { Uset( StK,kurs,CtU( ans ),ins ); ked=K; if( kurs<l )++kurs; }
     }
     if( ked!=-1 && ked!=K ){ K=ked; Get( Ls,ked ); }     // будет новое число?
     Draw();
     if( !(ans=A->WaitKey() ) )break;                  // случай закрытия окна
     if( ans>=_F1 && ans<=_F12 )return ans;
   }                                   // по выходу из процедуры ???
Fin: if( ans )Draw(); return ans?K:0; // площадка с текстом остается на экране
}
bool TextMenu::Draw()            // картинка со списком строк запросов/форматов
{ Activate();                    // и запросы с представлением исходных величин
  { TextContext Ts;              // здесь предусматривается изменение масштабов
    const Real Tw=AlfaWidth(),Th=AlfaHeight();
    Area( X,Y,Lx+2,Ly+2 ).Activate( true );               // активация разметки
    glScalef( Tw,-Th,1.0 ),glTranslatef(0,-Ly-2,0);       // листа для текста
    glColor4f( .6,.8,1,.6 ); glRectf( 0,0,Lx+2,Ly+2 );
    glColor4f( 1,1,.8,.94 ); glRectf( 0.3,0.3,(Lx+2)-0.3,(Ly+2)-0.5 );
    for( int k=0; k<Num; k++ )
    { mlist &L=((mlist*)M)[k]; int i=Mestr( &L ),l=i+L.X;
      glColor3f( 0,0.25,0.5 ); Print( L.X+1,L.Y+1,St );
      if( L.lf )
      { glColor3f( k==ked?0.7:1,1,k==K?0:1 ); glRectf( l,L.Y-.1,l+L.lf,L.Y+1 );
        glColor3f( 0,0,0.75 ); Print( l+1,L.Y+1,k==ked?StK:Uset( St,i ) );
        if( k==K )if( L.dat )
        { glColor3f( 1,0,0 ); kurs=minmax( 0,kurs,L.lf-1 );
          glBegin( GL_LINE_STRIP );
                   glVertex2f( l+kurs-0.1,L.Y+0.8 );      // текстовый курсор
          if( ins )glVertex2f( l+kurs-0.1,L.Y );         // изменяемого символа
              else glVertex2f( l+kurs+1,L.Y+0.8 ); glEnd();
          if( ked==-1 )strcpy( StK,Uset( St,i ) );     // изменяемая подстрочка
  } } } } Show(); return false;
}
static int State=0; // связка для двух Mouse - повторение нажатой клавиши мышки

bool TextMenu::Mouse( int x, int y )
{ if( !Up )return Place::Mouse( x,y ); x/=AlfaWidth(),y=(y-8)/AlfaHeight()+1;
  for( int k=0; k<Num; k++ )
  { mlist &L=((mlist*)M)[k]; int l=Mestr( &L )+L.X;         // это длина строки
    if( x<l^x<=l+L.lf && y<L.Y+0.2^y<=L.Y+1 )
      { K=k; kurs=minmax( 0,x-l,L.lf-1 ); break; }
    if( ked!=K )Get( &L,ked );             // и по выходу считывание результата
  }
  Draw(); State=0; return true; //Place::Mouse( x,y );
}
bool TextMenu::Mouse( int b, int x, int y )
{ if( !Up )return Place::Mouse( b,x,y );
  if( State!=b ){ K=-2; Mouse( x,y );
    if( K>=0 )    // сброс и контроль переустановки выбора редактируемой записи
    { Window *W=Ready();
      if( W ){ if( b&_MouseLeft )W->PutChar( _Enter );
               if( b&_MouseRight )W->PutChar( _Esc ); } State=b;
  } } return true;
}
