//
//  Tv_case                                                          92.10.16
//
//  Процедура формирования списка выбора и поиска на графическом мониторе
//
//    Ki - Номер элемента в списке с которого осуществлен выход из программы;
//  Возвращается команда выхода из поиска
//   <ret> команда с клавиатуры приведшая к выходу из программы:
//         если с клавиатуры посылается видимый символ,
//              то начинается поиск по всему списку,
//         если происходит сбой в сортировке,
//              то возвращается первый символ последнего из выбранных слов.
//
#include "..\Graphics\Tv_graph.h"
//
//     Отметка текущей записи
//
static void Mark( int ls, int y, int x, int da, int ic )
{ Tgo( x*ls+1,y );  color( ic?WHITE:Tv.BkColor );
  x=Tv.Tw*ls-4;     Tpoint.x++;
  y=Tv.Th+(da>0);   Tpoint.y+=y-2;
  moveto( Tpoint ); Tpoint.y-=y;
  lineto( Tpoint ); Tpoint.x+=x;
  lineto( Tpoint ); color( ic?(Tv.BkColor==BLACK?LIGHTGRAY:BLACK):Tv.BkColor );
                    Tpoint.y+=y;
  lineto( Tpoint ); Tpoint.x-=x;
  lineto( Tpoint );
}
Course Tcase(           //
 field& M,              // Размерения матрицы на экране монитора
   int ls,              // Максимальная длина информационной строки
 char*  Fl(int),        // Построение строчки по номеру в списке
 Course Hl(int),        // Прерывание для внешнего управления списком
                        // Если на выходе 0: перерисовка текущей таблицы
                        //               -1: перерисовка и к новой строке
   int  N,              // Полная длина списка
   int& Ki,             // Номер текущего элемента (-1 из прошлого)
   int  ret             //       1 выход с сохранением
){                      //       0 обычный вход и выход
 char * S;              //      -1 вход без инициализации
static int _K=-11;      // Запоминаемый индекс положения в списке
 int  C,L,Lp,i,k,l,y,x, // Полная длина видимого списка
      Ls,a=0,K=0,       // Длина одной записи и позиция поиска символа
      Nb,Ne,            // Номера первой и последней из видимых записей
      Ly,Lx;            // Количество строк и столбцов
 char s[12];            // Буферная строчка для организации поиска
 int  ans;              //
   C=Tv.Color;          //
  Ls=ls+2;   L=(Ly=M.Ly)*(Lx=M.Lx);
  l=L/2;     if( Ki>=0 )K=Ki; else K=_K; if( K>=N )K=N-1; if( K<0 )K=0;
  Nb=K-l;    if( Nb<0  )Nb=0;
  Ne=Nb+L-1; if( Ne>=N )
             { Ne=N-1; Nb=N-L; if( Nb<0 )Nb=0,L=N,Ly=(L+Lx-1)/Lx; }
 if( ret>=0 ){ color( LIGHTGRAY ); Twindow( M.Jx,M.Jy,M.Lx*Ls,Ly,M.wb );
             }
ReView: color( C );  k=Nb;
  for( x=0; x<Lx; x++ )
  for( y=0; y<Ly && k<=Ne; y++,k++ )
  { S=Fl( k ); Tgo( x*Ls+2,y+1 );
    for( ans=1,i=0; i<Ls-2; i++ )
    { if( ans )ans=S[i]; Tputc( ans?ans:' ' ); } color( C );
  }
  Lp=Lx>1?Ly*((Lx-1)/3+1):Ly-1;
  for( ;; )
  { Mark( Ls,y=(K-Nb)%Ly+1,x=(K-Nb)/Ly,a,1 ); Tgo( x+a+1,y );
    if(  a )ans=Tgetc(); else
    if( Hl )ans=Hl( K ); else
    do ans=Tv_getc(); while( ans>=_MouseMoved && ans<=_MouseReleased );
    Mark( Ls,y,x,a,0 ); l=0;       // Признак движения окна
                        k=K;       // Признак движения курсора
    if( ans==0xFF )ans=South,k=-1; // Признак отметки элемента списка
    if( !ans )goto ReView;         // - то же с немедленной перерисовкой
    if( ans<0xF1 && ans>=' ' )     //
    { int dir=0;                   // Запуск процесса поиска
//    ans=ucod( ans );             //
      scpy( s,S=Fl( K ),12 );      // <<ucpy>>
      if( ans>s[a] )dir=1;  else
      if( ans<s[a] )dir=-1; else ++a;
      if( dir && a<12 )
      while( (dir>0 && K<N-1) || (dir<0 && K>0) )
      { S=Fl( K+dir );
        for( i=0; i<a; i++ )if( S[i]!=s[i] )goto Leave; // <<ucod>>
        if( (i=(ans-S[a]))*dir<0 )break; K+=dir;        // <<ucod>>
        if( !i ){ ++a; break; }
      }
    Leave: if( k!=K )
      {    if( K<=Nb ){ l=-1; if( Nb>0   )Nb=K+Ly/2; }
           if( K>=Ne ){ l=1;  if( Ne<N-1 )Ne=K-Ly/2; }
    } } else
    { switch( ans )
      { case North     :        if( K>0  )--K;              break;
        case North_West: K=0;   if( Nb>0 )Nb=1;     else    break;
        case North_East: --l;
        case       West: K-=K-Ly<Nb?Lp:Ly; if( K<0 )K=0;    break;
        case South     :        if( K<N-1 )++K;             break;
        case South_West: K=N-1; if( Ne<N-1 )Ne=N-2; else    break;
        case South_East: ++l;
        case       East: K+=K+Ly>Ne?Lp:Ly; if( K>=N )K=N-1; break;
       default:
        if( !ret )Tback(); Ki=K; color( C ); return Course( ans );
      } a=0;
    }
    if( k!=K )
    { if( l<0 )
      { if( Nb>0 ){ Nb-=Lp; if( Nb<0 )Nb=0; Ne=Nb+L-1; goto ReView; } }
      else
      if( l>0 )
      { if( Ne<N-1 ){ Ne+=Lp; if(Ne>=N)Ne=N-1; Nb=Ne-L+1; goto ReView; } }
      else
      if( K<Nb || K>Ne )
      { if( K<Nb ){ Nb-=Lp; if( Nb<0  )Nb=0;   Ne=Nb+L-1; k=-1; }else
        if( K>Ne ){ Ne+=Lp; if( Ne>=N )Ne=N-1; Nb=Ne-L+1; k=-1; }
      } if( k<0  )goto ReView;
  } }
}
Course Tcase(           //
 field& M,              // Размерения матрицы на экране монитора
   int  ls,             // Максимальная длина информационной строки
 char*  Fl( int ),      // Построение строчки по номеру в списке
   int  N,              // Полная длина списка
   int& Ki,             // Номер текущего элемента (-1 из прошлого)
   int  ret             // Режим сохранения изображения
){                      //
  return Tcase( M,ls,Fl,0,N,Ki,ret );
}
