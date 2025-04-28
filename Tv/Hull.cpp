//
//      Программа рисования теоретического чертежа корпуса судна
//
//      В главной процедуре сосредоточены консольные запросы
//              и информационные справки
//
//                                 (c)1975-03.08.31, В.Н.Храмушин
//                              лаб. Вычислительной гидромеханики
//                                   и морских исследований
//                      Сахалинского государственного университета
//
// Текущие недоработки:
// 02-02-09 -- К Мичеллу необходимо добавить форму волны в размерном виде
//          +- Перерисовка теоретических шпангоутов, ватерлиний и батоксов
// 03-07-25 -- Включить в выбор экстремумов штевни
//          ++ При переходе к Мичеллу неправильно учитывется осадка
//
#include "Hull.h"
#include "OS\Tf_topo.h"
//extern "C"
//{ _cdecl matherr( struct exception *a ){ a->retval=0.0; return 1; }
//  _cdecl _matherrl( struct _exceptionl *a ){ a->retval=0.0; return 1; }
//}
const char
*Months[]={ "январь","февраль","март","апрель","май","июнь",
            "июль","август","сентябрь","октябрь","ноябрь","декабрь" },
*WeekDay[]={ "понедельник","вторник","среда","четверг","пятница",
             "суббота","воскресенье"
           };
static char Msgs[]="%ld Мб, H справка "
        "F1 Метод F2 Запись F3 Чтение F4 Модель F5 Статика F8 Волна F9 Мичелл";

int  Hull_Keys=0x00;                 // Набор ключей - параметров
                                     // x01 -  задействовать слайн-интерполяцию

Real Xo=0,Xm=1,Length=2,Lwl=2,       // Длина
               Breadth=.2,Bwl=.2,    // Ширина
     Do=0,     Draught=.1,Height=.2, // Осадка
               Volume=0.4,           // Водоизмещение
               Surface=0.2;          // Площадь смоченной поверхности
                                    //
void Hull_Wave( const int Type );  // Волнообразование и волновое сопротивление
void Hull_Statics();               // Кривые элементов теоретического чертежа
//
//   Программа построения корпусов судов
//  и расчетов их волнового сопротивления
// ---------------------------------------
                                        //
DrawPlane wH( "Корпус",    "Y","Z" ),   // Окно проекции корпус,
          wM( "Бок",       "X","Z" ),   //  бок
          wW( "Полуширота","X","Y" );   //   и полуширота
                                        //
static int Console();

static void Init()
{ int k; color( LIGHTGRAY );
  k=Tv.Th; Tv.Th=16; Tpoint.y=Tv.mY/2; Tpoint.x=1; // i=Tv.mY/32+3;
  Tprintf( " L=%.3lg",Length );
  if( fabs( Length-Lwl )>Length/500 )
  Tprintf(  "/%.3lg",Lwl );                        Tln();
  Tprintf( " B=%.3lg",Breadth );
  if( fabs( Breadth-Bwl )>Breadth/500 )
  Tprintf(  "/%.3lg",Bwl );                        Tln();
  Tprintf( " T=%.3lg",Draught );                   Tln(); //Kh.Init();
  Tprintf( " V=%.6lg",Volume  );                   Tln();
  Tprintf( " S=%.5lg",Surface );                   Tln();
  Tprintf( " \x7F=%.3lf",Volume/Bwl/Lwl/Draught ); Tln();
  Tprintf( " \x0F=%d",Kh.Ms ); Tv.Th=k;
}
int main() // int ac, char** av )   // Предопределение графической среды
{
 Tfile_Filter=(char*)"Ship Hull Lines Drawing (*.vsl)\0*.vsl\0"
                     "All Files (*.*)\0*.*\0";
  Tv_Graphics_Start();          // Установка драйвера на 256 цветов
  setwindowtitle( "Ship - computational fluid mechanics" );
  Tv_Start_Mouse();             // Запуск указателя "мышка"
  Tv_SetCursor( 0,0 );          // Приведение мышки к верхнему левому углу
  Tv.Font( _Small_font,17 );    // Шрифт графический тонкий на 12 пикселей
  if( _ac_==1 )Kh.Read();       // При вызове без параметров -> Hull.vsl
      else Kh.Read( _av_[1] );  // Степановские, польские или новые корпуса
  Building();                   // Первая прорисовка на экране
  Init();                       // Вывод справки по корпусу
  while( Console() );           //
}
static Course Console_Wait()
{
 int M,D,Y,k,c=color( WHITE );
 Course Ans;
  do
  { Event Record_Time; //.Now();
    Record_Time.UnPack( M,D,Y ); Tpoint.y=Tv.mY-Tv.Th-10; Tpoint.x=1;
    k=Tv.Th; Tv.Th=10;
    Tprintf( "%04d.%s.%02d ",Y,W2D( Months[M-1] ),D );
    Tprintf( "%s%-10s",W2D(WeekDay[int(Record_Time.D%7)]),DtoA(Record_Time.T) );
    Tv.Th=k;
    Ans=Tv_getc( 250 );
  } while( !Ans );
  color( c );
  return Ans;           //
}                       // Признак информации в нижней строке
static fixed Is_Info=0; // 0x01 - Признак информации об окне проекции
static int Console()    // 0x02 - Общесистемная подсказка
{                       //
 const char *Name[]={ "Hull","  Обводы и штормовая",
                             "мореходность корабля",0 },
            *Text[]={ "F1    ","методы",
                      "F2/F3 ","запись, считывание",
                      "F4    ","математическая модель",
                      "F5    ","форма и статика",
                      "F8    ","волнообразование",
                      "F9    ","Мичелл - Шебалов",0 },
            *Plus[]={ "<Пробел> ","снятие справки",
                      " <altP>  ","запись графики",
                      " <altC>  ","смена палитры",
                      " <altD>  ","рисовалка",
                      " <altX>/<ctrlC> ","стоп",0, };
 int x,y,ans=1; color( CYAN );
  do
  { Tv_GetCursor( x,y );
    if( wH.Info( x,y ) )
    { int i; Real Z,Y; Z=Y=Tv_X( x );
      color( YELLOW ); Tgo( 0,-3 ); Tprintf( "Y=%5.3lf, ",Y );
      color( WHITE );
      while( (i=Kh.F[10].iZ( Z ))>0 )Tprintf( "{%d} Z=%5.3lf,  ",i,Z );
      Tprintf( "\n" );
    } else
    if( wW.Info( x,y ) ){} else
    if( wM.Info( x,y ) ){} else
    { if( Is_Info&0x01 ){ Tgo( 0,0 ); Tprintf( "\n" ); Is_Info&=~0x01; }
      if( !(Is_Info&0x02) )
      { Is_Info|=0x02; Thelp( ans==' '?"":Msgs,coreleft()>>20 );
    } }
    ans=Console_Wait();
    switch( ans )
    { case 0xF1: ans=Set_Parameters();  break;
      case 0xF2: ans=Kh.Write();        break;
      case 0xF3: ans=Kh.Read( "*.vsl"); break;
      case 0xF4:
      { int k;
        const char p[]={ 2,11 },*n[]={ "Степени","Синусы" },
                                *m[]={ "СТЕПЕНИ","СИНУСЫ" };
        for(;;)           //
        { Tgo( 1,0 );     // Количество моделей может быть увеличено
          color( GREEN ); //
          for( k=0; k<sizeof p; k++ )Tprintf( " %s ",W2D( n[k] ) );
                                     Tprintf( " \x1b\x09\x1a\n" );
          if( ans<0 )ans=sizeof( p )-1; else
          if( ans>=sizeof p )ans=0;
          color( LIGHTCYAN ); Tgo( p[ans],0 ); Tprintf( W2D( m[ans]) );
              k=Tv_getc();
          if( k==East  )ans++; else
          if( k==West  )ans--; else
          if( k==_Esc  ){ ans=1; break; } else
          if( k==_Enter){ if( ans )ans=Set_Sinus();
                              else ans=Set_Power(); break;
                        }
        } Tgo( 1,0 ); Tprintf( "\n" );
      }               Is_Info=0; break;
      case 0xF5: Hull_Statics(); break;
      case 0xF8: Hull_Wave( 0 ); break;
      case 0xF9: Hull_Wave( 1 ); break;
      case ' ' : Ghelp();        break;
      case _Esc: Thelp( "? Выход из программы [y/n]" );
           ans=Tv_getc(); if( ans==_Enter || (ans|0x20)=='y' )return 0;
      default: if( ans=='h' )ans=Help( Name,Text,Plus ); break;
    }
    if( ans!=_MouseMoved )Is_Info&=~0x02;
  } while( ans>_Enter ); if( !ans || ans==_Enter ){ Building(); Init(); }
  return 1;
}
// остатки комплекса процедур представления и управления н=-графическими окнами
//
Window::Window(){ f=Tv_port; F.Jx=F.Jy=0; F.Lx=F.Ly=1; }
void Window::Set( field a ){ Tv_place( &a ); f=Tv_port; }
bool Window::Is( int x,int y )
          { return x>=f.left && x<=f.right && y<=f.bottom && y>=f.top; }
void Window::Focus()  // Установка графического окна
  { field t;    //
    Field T=F; t.Jx=-f.left;        t.Lx=f.right-Tv.mX; t.wb=0;
               t.Jy=f.bottom-Tv.mY; t.Ly=-f.top; Tv_place( &t );
                                                 Tv_place( 0,&T );
}/*
  bool Is(){ int x,y; Tv_GetCursor( x,y ); return Is( x,y ); }
                //
  void Check( int &x, int &y )
  { int Ans=0;                                  //
    if( x<f.left   ){ Ans=1; x=f.left;   } else // Удержание
    if( x>f.right  ){ Ans=1; x=f.right;  }      // мышки в
    if( y<f.top    ){ Ans=1; y=f.top;    } else // границах
    if( y>f.bottom ){ Ans=1; y=f.bottom; }     //  текущей
    if( Ans )Tv_SetCursor( x,y );             //   карты
  }                                          //
*/
//      Информационные окошки и текстовые справки
//
bool DrawPlane::Info( int x, int y )
{
 static DrawPlane *_Prev=NULL; static int k=1;
  if( Is( x,y ) )
  { int c=color( LIGHTGRAY ); Tpoint.y=Tv.mY-Tv.Th+2; Tpoint.x=1;
    if( _Prev!=this ){ _Prev=this; Focus(); Is_Info&=~0x01; }
    if( Is_Info&0x02 ){ Ghelp(); Is_Info&=~0x02; }
    if( Is_Info&0x01 )Tpoint.x=k; else
      { Tprintf( "%s ",W2D( iD ) ); k=Tpoint.x; Is_Info|=0x01;
      }
    color( CYAN   ); Tprintf( aX );
    color( YELLOW ); Tprintf( " %.2lf ",Tv_X( x ) );
    color( CYAN   ); Tprintf( aY );
    color( YELLOW ); Tprintf( " %-12.2lf",Tv_Y( y ) );
    color( c );
    return true;
  } return false;
}
