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
//          ++ При переходе к Мичеллу неправильно учитывается осадка
//
#include "Hull.h" // 132 строки
/*
Оглавление:
      K_Hull.cpp     222 Корпус + форштевень + ахтерштевень & образец МИДВ
      K_Frame.cpp    683 Численное определение одного шпангоута со сломами
      K_Math.cpp     891 Обращение к процедурам математического описания
      K_Statics.cpp 1258 Остойчивость и кривые элементов теоретического чертежа
      K_Wave.cpp    1658 из 1998 Сопротивление по Мичеллу и волнообразование
*/
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

//   "K_Hull.cpp"
//   г---------------------------------------------------------¬
//   ¦                                                         ¦
//   ¦  Программа математического определения (моделирования)  ¦
//   ¦       формы корпуса судна, с определением базовых       ¦
//   ¦             функций для его описания:                   ¦
//   ¦                                                         ¦
//   ¦   float K_Hull  ( float z, float x ); // Корпус         ¦
//   ¦   float K_Astern( float z );          // Ахтерштевень   ¦
//   ¦   float K_Stern ( float z );          // Форштевень     ¦
//   ¦                                                         ¦
//   L=========================================================-
//                       1997,январь, В.Храмушин, Южно-Сахалинск
//                  2001, август - Владивосток, октябрь - ГС-210
//
const int Lsr=16000;
static char *Str=strcpy( (char*)malloc( Lsr ),
            "\0\n<<МИДВ>>\n (c)1975-2001, В.Н.Храмушин, Южно-Сахалинск\n" );
static FILE *Fh;
//   г--------------------------------------------------------¬
//   ¦                                                        ¦
//   ¦   Считывание/запись численного описания формы корпуса  ¦
//   ¦     (при первом обращении возвращается корпус МИДВ)    ¦
//   ¦                                                        ¦
//   L========================================================-
static char* UtR( char &s, char *U );    // символ UTF-8 -> Russian-OEM(866)alt
static void FGetS()
{ int l,k,Len;
 char s[Lsr],*c,b; s[0]=0;
  do
  { fgets( s,sizeof( s ),Fh );
    if( (c=strchr( s,';'  ))!=NULL )*c=0;
    if( (c=strstr( s,"//" ))!=NULL )*c=0;
    Len=strcut( s );
  } while( !feof( Fh ) && !Len );
  c=s;
  for( l=k=0; *c && k<Len; k++ )
  { c=UtR( b,c );
     if( (Str[l]=b)!=9 )l++; else do Str[l]=' '; while( ++l%8 );
  }       Str[l]=0;
}
static Real Fort_F( char *s, int l, int d )
{ char b,sign=0; int i; double w=0.0;
  for( i=strlen( s ); i<l; i++ )s[i]=0;
  for( i=0; i<l; i++,s++ )
  if( (b=s[0])=='-' )sign=1; else
  if(  b=='.' )d=l-i-1;      else
    { if( unsigned(b-='0')>9u )b=0; w=w*10.0+Real( b );
    }
  for( i=0; i<d; i++ )w/=10.0; return sign?-w:w;
}
static void FPutS( const char _Comm[], const char _Fmt[], ... )
{ int l;
  va_list ap;
  va_start( ap,_Fmt ); l=vsprintf( Str,_Fmt, ap ); va_end (ap);
  if( _Comm )
  { while( l<79 )Str[l++]=' '; strcpy( Str+79-strlen( _Comm ),_Comm );
  } fprintf( Fh,"%s\n",W2D( Str ) );
}
//
// Польские материалы от А.Дегтярева
//
int Hull::Read_from_Polland()
{                         //
 int i,j,k,l,m,Nx,Nz,n=0; // Размерения судна и размерность таблицы ординат
 Real W;                 //
 Point P;
  sscanf( Str,"%lf%lf%lf",&Length,&Draught,&Breadth );
  fscanf( Fh,"%d%d%s",&Nz,&Nx,Str );
  fgets( Str,Lsr-1,Fh );          // сброс конца строки с заголовком таблицы
  if( Nx<3 || Nz<2 )return 1;    //
  Simple_Hull( Nx,Nz );          //
  for( i=0; i<Nx; i++ )              // Считывание таблицы с номерами
  { fscanf( Fh,"%d%d%lg",&j,&k,&W ); //  шпангоутов и их абсциссами
    if( j!=i+1 )Break( "(%d!=%d) на списке абсцисс\n",j,i+1 );
    F[i].X=W/1000.0;
  } P.y=0.0;
  for( j=0; j<=(Nz-1)/8; j++ )
  {    i=j*8;
       k=i+8;
       if( k>Nz )k=i+( Nz%8 );
    for( l=i; l<k; l++ )                      //
    { fscanf( Fh,"%lg",&(P.x) ); P.x/=1000.0; // аппликаты ватерлиний
      for( m=0; m<Nx; m++ )F[m].z[l]=P.x;     // у всех теоретических
      Stx[l]=P; Sty[l]=P;                     //  линий одинаковы
      Asx[l]=P; Asy[l]=P;                     //
    }
    for( m=0; m<Nx; m++ )
    { fscanf( Fh,"%d",&n );
      if( n!=m+1 )Break( "(%d!=%d) на таблице ординат\n",n,m+1 );
      for( l=i; l<k; l++ )
      { Real &y=F[m].y[l]; fscanf( Fh,"%lg",&y ); y/=1000.0;
  } } }
  if( Hull_Keys&0x01 )for( i=0; i<Nx; i++ )F[i].SpLine();
//
//      Предварительная расчистка штевней
//
  for( i=0; i<Nz; i++ ){ Asx[i].y=F[0].X; Stx[i].y=F[Nx-1].X; }
//
//      И попытка выборки штевней из исходного файла
//
  fscanf( Fh,"%d%d",&i,&j );
  for( i=0;   i<Nz; i++ ){ Real &y=Asx[i].y; fscanf( Fh,"%lf",&y ); y/=1000.0; } --j;
  for( i=j+1; i<Nz; i++ )Asx[i].y=Asx[j].y;
  fscanf( Fh,"%d%d",&i,&j );
  for( i=0;   i<Nz; i++ ){ Real &y=Stx[i].y; fscanf( Fh,"%lf",&y ); y/=1000.0; } --j;
  for( i=j+1; i<Nz; i++ )Stx[i].y=Stx[j].y;

  Height=F[Nx/2].z[Nz-1];
  Lwl=Stx( Draught )-Asx( Draught );
  Xm=Lwl/2;
  Do=0.0;
  Bwl=(*this)( Xm,Draught )*2;
  { Field M; Asx.Extreme( M ); Xo=M.Jy;
             Stx.Extreme( M ); Length=M.Jy+M.Ly-Xo;
  } Init();  return 0;
}                         //
int Hull::Read( const char* _Name ) // Имя файла с описанием корпуса
{                         //
 int i,j,k,Nx,Nz;
 Real w;
 Field W;
 static char MIDV[]=
"//MODEL JOB '158015ИКТК',KHRAMUSHIN,CLASS=D,PRTY=6                     850827\n"
"// ...\n"
"// Гипотетическая модель судна МИДВ \\ Ленинград, Корабелка-1985\n"
"*    (малыми моментами инерции площади действующей ватерлинии)\n"
"011\n"
" 24 18\n"
"  -950  -950 -1120 -1230 -1320 -1390 -1400 -1352 -1200\n"
"  -720   070   620   760  1220  1796  2200  2320  2600\n"
" 22000 24340 24470 24600 24520 24200 23640 22950 22360\n"
" 22000 21832 21720 21700 21732 21800 21866 21900 21900\n"
"     0   150   180   220   230   220   180   120    30\n"
"     0     0     0     0     0     0     0     0     0\n"
"     0    60   120   200   160    60    20     0     0\n"
"     0     0     0    10    30    70   100   120     0\n"
"       0.0 1.5 2.0 4.0 6.0 8.0 10. 12. 14. 16. 18. 20. 22. 24. 26. 28.28802900\n"
" -1100   0   0   0   0   0   0   0   0   0  46 180 268 312 303 250  90   0   0\n"
"     0   0   0   0   0   0   0   0   0  93 221 351 457 520 530 512 476 400 400\n"
"  1100   0   0   0   0  11  33  51 117 232 366 484 578 640 663 647 610 538 538\n"
"  2200   0   0   0  44 185 192 187 251 370 498 603 688 746 767 754 728 671 671\n"
"  3300  33  61  77 190 423 423 398 427 512 621 711 782 838 852 851 832 802 802\n"
"  4400  70 122 150 447 695 688 639 623 663 730 798 856 905 938 942 929 911 911\n"
"  5500  90 145 231 755 974 959 874 817 807 830 867 914 964 9991013100810001000\n"
"  6600 100 150 4981033119611791086 991 937 920 932 9661012106010891088 939 939\n"
"  7700 100 150 656122913591336124111361043 990 971 9851030108211171119 969 969\n"
"  8800 100 150 8611360148014561358124011221031 989 9991046110111421148 997 997\n"
"  9900 100 150 936142115471523141912831153105610111018105011161158115910101010\n"
" 11000 100 150 958146615801569146213171175106710191021106011171159116010111011\n"
" 12100 100 150 980149615991570146413121169105810121017105811161159116010111011\n"
" 13200 100 29010101490158315531447129111501042 993 9981040110111451149 996 996\n"
" 14300 100 53010261476155915231411125911121000 948 953 999106211051109 957 957\n"
" 15400 100 6751038142914971450133511911050 936 883 892 940100110481052 900 900\n"
" 16500 100 772105413411390133912231083 942 831 772 773 819 891 947 956 802 800\n"
" 17600 100 83010091213124111821070 941 803 682 621 618 658 726 793 821 665 665\n"
" 18700 100 795 89610531068 996 880 740 608 503 443 437 468 522 584 651 651 651\n"
" 19800 100 672 745 856 857 776 653 518 392 302 259 252 273 319 380 460 460 460\n"
" 20900 100 543 592 677 664 578 452 313 204 129  98  98 117 152 202 267 267 267\n"
" 22000   0 420 471 533 508 420 298 157  58   0   0   0   0   0   0   0   0   0\n"
" 23100   0 322 361 418 378 271 129   0   0   0   0   0   0   0   0   0   0   0\n"
" 24200   0 150 225 309 249   0   0   0   0   0   0   0   0   0   0   0   0   0\n"
"/*\n";
//"* Список рабочих водоизмещений (посадок) при креновании\n"
//"  2592 10042 28773 45514 52675 59363 65902 79596 94761\n";
//
//    Считывание заголовков и проверка наличия управляющих кодов
//
  strcpy( Name,_Name );
  if(  !(Fh=Topen( Name,"rt","vsl","? выбрать корпус или - Esc - для модели МИДВ" ) ) )
  if(  !(Fh=fopen( strcpy( Name,"Hull.vsl" ),"rt" ) ) )
  { if( (Fh=fopen( Name,"wt" ) )!=NULL )
    { fputs( W2D( MIDV ),Fh ); fclose( Fh );
      if( (Fh=fopen( Name,"rt" ) )!=NULL )goto Ok;
    } goto back;
  }
Ok:  strcpy( Name,fname( _Name ) );
      FGetS();
  if( Str[0]==' ' )
    { k=Read_from_Polland(); if( k )goto back; fclose( Fh ); return k;
    } else              //
  if( Str[0]=='' )     //
    { k=Read_from_Frames(); if( k )goto back; fclose( Fh ); return k;
    } FGetS();
  for( i=0; i<3; i++ )if( (unsigned char)(Str[i]-'0')>3u )goto back;
  if( (unsigned char)(Str[3]-'0')<=3u )  //
    { for( i=0; i<3; i++ )FGetS();       // Пропуск задания
      sscanf( Str,"%d",&Nx );            // по остойчивости
      for( i=0; i<Nx+2; i++ )FGetS();    //
    } FGetS();                           // Размерность и тип данных
  sscanf( Str,"%d%d%s",&Nx,&Nz,Str+40 ); //
  if( Nx<3 || Nz<2 )goto back;
  Simple_Hull( Nx,Nz );
//
//    Если задано описание штевней, то считывание (j#0 - признак)
//
  if( (j=strcmp( strupr( Str+40 ),"SHORT" ))!=0 )
  { for( i=0; i<Nz; i++ )                       //
      { if( !(k=i%9) )FGetS();                  // Абсциссы ахтерштевня
        Asx[Nz-i-1].y=Fort_F( Str+k*6,6,2 ); }  //
    for( i=0; i<Nz; i++ )                       //
      { if( !(k=i%9) )FGetS();                  // Абсциссы форштевня
        Stx[i].y=Fort_F( Str+k*6,6,2 ); }       //
    for( i=0; i<Nz; i++ )                       // Ординаты для кормового
      { if( !(k=i%9) )FGetS();                  // транца и расправленной
        Asy[Nz-i-1].y=Fort_F( Str+k*6,6,2 ); }  // раковины
    for( i=0; i<Nz; i++ )                       // Ординаты для линий
      { if( !(k=i%9) )FGetS();                  // скулы, расправленных
        Sty[i].y=Fort_F( Str+k*6,6,2 ); }       // на срезе форштевня
  }                                             //
//
//  Считывание апликат расчетных ватерлиний
//
  FGetS();
  for( i=0; i<Nz; i++ )
  { w=Fort_F( Str+i*4+6,4,2 );
    for( k=0; k<Nx; k++ )F[k].z[i]=w; Asx[i].x=w; Asy[i].x=w;
                                      Stx[i].x=w; Sty[i].x=w;
  }
//  Считывание таблицы ординат теоретичексго чертежа корпуса судна
//
  Breadth=0;
  for( k=0; k<Nx; k++ )
  { FGetS();
    F[k].X=Fort_F( Str,6,2 );
    for( i=0; i<Nz; i++ )
    { F[k].y[i]=w=Fort_F( Str+i*4+6,4,2 ); if( Breadth<w )Breadth=w;
    } if( Hull_Keys&0x01 )F[k].SpLine();
  } fclose( Fh );
//
//  Если таблица короткая (без описания штевней)
//
  if( !j )for( i=0; i<Nz; i++ )
  { Asy[i].y=Sty[i].y=0.0; Asx[i].y=F[0].X; Stx[i].y=F[Nx-1].X;
  } Asx.Extreme( W );    Xo=W.Jy;
    Stx.Extreme( W );    Length=W.Jy+W.Ly-Xo;
    Draught=Stx[Nz/2].x; Height=Stx[Nz-1].x; Do=Stx[0].x;
    Lwl=Stx( Draught )-Asx( Draught );
    Xm=Lwl/2;
    Bwl=(*this)( Xm,Draught )*2; Breadth*=2; Init(); return 0;
back: if( Fh )fclose( Fh ); Fh=0; return 1;
}
void Hull::Simple_Hull( int Nx, int Nz )
{ int i;
  allocate( Nx );
  for( i=0; i<Nx; i++ )F[i].allocate( Nz );
    Stx.allocate( Nz ); Sty.allocate( Nz );
    Asx.allocate( Nz ); Asy.allocate( Nz );
  for( i=0; i<Nz; i++ )Stx[i]=Sty[i]=Asx[i]=Asy[i]=0.0;
}                               //
void Hull::Set_SpLine()         // Установка сплайновой разметки
   { for( int i=0; i<Ns; i++ )F[i].SpLine();
   }                            //
void Hull::Del_SpLine()         // Снятие сплайновой разметки
   { for( int i=0; i<Ns; i++ )F[i].allocate( F[i].N+1 );
   }
//
//   г----------------------------------------------------------¬
//   ¦                                                          ¦
//   ¦  Считывание корпуса  "по контурам штевней и шпангоутов"  ¦
//   ¦                                                          ¦
//   L==========================================================-
//                                                       01-12-01
int Hull::Read_from_Frames()         //
{                                    // Здесь продолжается
 char *s,*c;                         // серия проверок:
  if( (s=strchr( Str,'<' ))!=NULL )  //
  if( (c=strchr( Str,'>' ))!=NULL )  // 2. ?должно быть имя в скобках
  { *c=0;                            //
    while( *(++s)<=' ' )if( *s==0 )break;
    if( strcut( s )>0 )              // 3. ?поле имени не пустое
    {                                //
     int i,k,n;
     Real w,o;
      strcpy( Name,s );  FGetS();    //
      sscanf( Str,"%d%d",&n,&k );    //
      if( n>3 && k>0 && k<n )        // 4. количество шпангоутов >3
      { allocate( n );               //    и мидель не с краю
        Ms=k;                        //
        FGetS();                                          o=0.0;
        sscanf( Str,"%lf%lf%lf%lf",&Length,&Breadth,&Draught,&o );
        //
        // Ахтерштевень (начало прямого считывания из файла данных)
        //
        FGetS(); n=strtol( Str,&s,0 ); Asx.allocate( max(2,n) );
        for( i=0; i<n; i++ )Asx[i].x=strtod( s,&s )-o,Asx[i].y=strtod( s,&s );
        FGetS(); n=strtol( Str,&s,0 ); Asy.allocate( max(2,n) );
        for( i=0; i<n; i++ )Asy[i].x=strtod( s,&s )-o,Asy[i].y=strtod( s,&s );
        //
        // Собственно корпус  ( - в том же последовательном потоке )
        //
        for( k=0; k<Ns; k++ )
        { Frame &W=F[k];
          FGetS(); W.allocate( n=strtol( Str,&s,0 ) ); W.X=strtod( s,&s );
          for( i=0; i<n; i++ )
          { W.z[i]=w=strtod( s,&s )-o; W.y[i]=strtod( s,&s );
            if( !i && !k )Do=Height=w; else
            { if( Height<w )Height=w;
              if( Do>w )Do=w;
          } }
        } Height-=Do;
        //
        // Форштевень (табличные данные здесь заканчиваются, далее - строки)
        //
        FGetS(); n=strtol( Str,&s,0 ); Sty.allocate( max(2,n) );
        for( i=0; i<n; i++ )Sty[i].x=strtod( s,&s )-o,Sty[i].y=strtod( s,&s );
        FGetS(); n=strtol( Str,&s,0 ); Stx.allocate( max(2,n) );
        for( i=0; i<n; i++ )Stx[i].x=strtod( s,&s )-o,Stx[i].y=strtod( s,&s );
        //
        // ...по необходимости лучше дополнить описание штевней
        //    нежели решать ротозейские проблемы из за их отсутствия
        //
        if( !Asx.N )Asx[0].x=Do,Asx[0].y=F[0].X,Asx[1].x=Height,Asx[1].y=F[0].X;
        if( !Asy.N )Asy[0].x=Do,Asy[0].y=0.0,   Asy[1].x=Height,Asy[1].y=0.0;

        if( !Sty.N )Sty[0].x=Do,Sty[0].y=0.0,      Sty[1].x=Height,Sty[1].y=0.0;
        if( !Stx.N )Stx[0].x=Do,Stx[0].y=F[Ns-1].X,Stx[1].x=Height,Stx[1].y=F[Ns-1].X;

        //
        // Уточнение основных параметров нового корпуса
        //
        n=Sty.N-1; if( Height<Sty[n].x )Height=Sty[n].x;
        n=Asy.N-1; if( Height<Asy[n].x )Height=Asy[n].x;
        { Field M; Asx.Extreme( M ); Xo=M.Jy;
                   Stx.Extreme( M ); Length=M.Jy+M.Ly-Xo;
        }
        Lwl=Stx( Draught )-Asx( Draught );
        Bwl=((*this)( Xm=F[Ms].X,Draught ))*2; Init();
        return 0;
  } } } return 1;
}
//   г---------------------------------------------------------------¬
//   ¦                                                               ¦
//   ¦  Запись корпуса в формате "по контурам штевней и шпангоутов"  ¦
//   ¦                                                               ¦
//   L===============================================================-
//                                                           01-11-30
int Hull::Write()
{
 const Real g5=1.000001;
 double t;
 int i,k,p,m,d,y; julday( julday(),m,d,y );
  sprintf( Str,"%02d%02d%02d%02d",y%100,m,d,int( t=onetime() ) );
  Thelp( "? Имя файла:" ); Tgo( 14,0 ); p=Tv.BkColor;
  color( YELLOW );
  bkcolor( BLUE );
  for( ;; )
  switch( Wgets( fext( Str,"vsl" ),-64 ) )
  { case _Enter:
    if( (Fh=fopen( fext( Str,"vsl" ),"wt" ))!=NULL )
    { bkcolor( p );
      fprintf( Fh,W2D( "//\n// Модель корпуса корабля,\n"
                      "//        построенная на контурах штевней и шпангоутов\n" ) );
      FPutS( "(c)01.Khram","//" );
      FPutS( "·","//" );
      FPutS( "признак и название","<%s>",Name );
      FPutS( "количество шпангоутов и номер миделя"," %d %d",Ns,Ms );
      FPutS( "длина, ширина, осадка"," %.5lg %.5lg %.5lg",Length*g5,Breadth*g5,Draught*g5 );
      //
      // Ахтерштевень
                              fprintf( Fh,"\n%3d ",Asx.N );
      for( i=0; i<Asx.N; i++ )fprintf( Fh," %.5lg %.5lg", Asx[i].x*g5,Asx[i].y*g5 );
                              fprintf( Fh,"\n%3d ",Asy.N );
      for( i=0; i<Asy.N; i++ )fprintf( Fh," %.5lg %.5lg", Asy[i].x*g5,Asy[i].y*g5 );
                              fprintf( Fh,"\n" );
      // Шпангоуты
      //
      for( k=0; k<Ns; k++ )
      {                           fprintf( Fh,"\n%3d %.5lg",  F[k].N+1,F[k].X*g5 );
        for( i=0; i<=F[k].N; i++ )fprintf( Fh," %.5lg %-5.5lg",F[k].z[i]*g5,F[k].y[i]*g5 );
      }                           fprintf( Fh,"\n" );
      //
      // Форштевень
                              fprintf( Fh,"\n%3d ",Sty.N );
      for( i=0; i<Sty.N; i++ )fprintf( Fh," %.5lg %.5lg", Sty[i].x*g5,Sty[i].y*g5 );
                              fprintf( Fh,"\n%3d ",Stx.N );
      for( i=0; i<Stx.N; i++ )fprintf( Fh," %.5lg %.5lg", Stx[i].x*g5,Stx[i].y*g5 );
                              fprintf( Fh,"\n\n" );

        FPutS( "водоизмещение",            ";  W=%.5lg",Volume );
        FPutS( "смоченная поверхность",    ";  S=%.5lg",Surface );
        FPutS( "коэффициент общей полноты",";  d=%.5lg",Volume/Bwl/Lwl/Draught );
      sprintf( Str+80,"%04d.%s.%02d %s%s",y,Months[m-1],d,WeekDay[int(t)%7],DtoA( t ) );
        FPutS( Str+80,";\n;" );
       fclose( Fh );
               Fh=0; return 0;
    }
    case _Esc : bkcolor( p ); return 1;
    default:;
  }
}
//
//      Установка режимов обработки корпуса
//
int Set_Parameters()
{
 int i,j,n,Ans=0; Real Av,Mv,w;
  do{
    if( Hull_Keys&0x01 )
    {   Av=Mv=0.0;
      for( i=n=0; i<Kh.Ns; i++ )
      for( j=0; j<Kh.F[i].N; j++ )
      { w=hypot( Kh.F[i]._z[j],Kh.F[i]._y[j] );
        Av+=w;    ++n;
        if( Mv<w )Mv=w;
      }    sprintf( Str,"\t Кривизна( %.3lg ) <= %.3lg",Av/=n,Mv );
    } else sprintf( Str,"\t Сплайн не используется " );
    { Mlist Menu[]=
        { { 1,26,Str },{ 1,0 },{ 0,6,"\t Пороговое число: %.3lg",&Mv } };
      Ans=Tmenu( Menu,Hull_Keys&0x01?3:2,1,-1 );
    }
    switch( Ans )
    { case 0: if( !(Hull_Keys^=0x01) )Kh.Del_SpLine();
                                else  Kh.Set_SpLine();
              Building();       break;
      case 2: for( i=0; i<Kh.Ns; i++ )
              for( j=Kh.F[i].N; j>=0; j-- )
               if( hypot( Kh.F[i]._z[j],Kh.F[i]._y[j] )>Mv )
                   Kh.F[i].Double( j );
                   Kh.Del_SpLine();
                   Kh.Set_SpLine();
              Kh.Init(); Building(); break;
    }
  } while( Ans!=_Esc ); return 0;
}
//  Выборка из файла UniCode.cpp для считывания текстов в формате UTF-8 -> OEM
//
#define X( x )if( (U[x]&0xC0)==0x80 )   // контроль старших бит в других байтах
static int nS=0;                        // контрольное число байт в символе UTF

static char* UtI( unsigned &u, char *U ) // сборка сложного символа из 1-4 байт
{ union{ char c[4]; unsigned d; } v; nS=1; // совмещение с выполнением проверки
  if( (v.d=*U)&0x80 )                     // случится ошибка при nS<2 и v.d>127
  { if( (*U&0xE0)==0xC0 ){ X(1){ v.c[1]=*U++; v.c[0]=*U; nS=2; } } else
    if( (*U&0xF0)==0xE0 ){ X(1)X(2){ v.c[2]=*U++; v.c[1]=*U++; v.c[0]=*U; nS=3; } } else
    if( (*U&0xF8)==0xF0 ){ X(1)X(2)X(3){ v.c[3]=*U++; v.c[2]=*U++; v.c[1]=*U++; v.c[0]=*U; nS=4; } }
  } u=v.d; return ++U;            // на выходе адрес той же строки со смещением
}
static char* UtR( char &s, char *U )     // символ UTF-8 -> Russian-OEM(866)alt
{ unsigned d=U[0];                       // здесь в UTF-8 меняется порядок байт
  if( d<128 )s=*U++,nS=1; else           // особый контроль латыни на ускорение
  {   U=UtI( d,U );                      // сборка сложного символа из 1-4 байт
    if( d>=53392 && d<53440 )s=d-53264; else   // 1040-128 = А..п 53264
    if( d>=53632 && d<53648 )s=d-53408; else   // 1040-224 = р..я 53408
    if( d>=0xE29590 && d<=0xE295AC )
    { static char k[29]= { 205,186,213,214,201,184,183,187,212,211,200,190,189,
             188,198,199,204,181,182,185,209,201,203,207,208,202,216,215,206 };
      s = k[d-0xE29590]; } else    // °±·(Ё,ё)
    { static unsigned u[] = { 0xC2B0,0xC2B1,0xC2B2,0xC2B7,0xC3B7,0xD081,0xD191,
       0xE281BF,0xE28899,0xE2889A,0xE28988,0xE289A1,0xE289A4,0xE289A5,0xE28CA0,
       0xE28CA1,0xE29480,0xE29482,0xE2948C,0xE29490,0xE29494,0xE29498,0xE2949C,
       0xE294A4,0xE294AC,0xE294B4,0xE294BC,0xE29680,0xE29684,0xE29688,0xE2968C,
       0xE29690,0xE29691,0xE29692,0xE29693,0xE296A0 }; // 0xEF8C81 = '?' ? 0xFF
      static char k[]={ 248,241,253,250,246,133,165,252,249,251,247,240,
                        243,242,244,245,196,179,218,191,192,217,195,180,
                        194,193,197,223,220,219,221,222,176,177,178,254 };
      int i; for( i=0; i<36; i++ )if( d==u[i] ){ s=k[i]; break; }
                                  if( i==36 )s=0xFF;
  } } return U;                          // окончание строки
}                                        //    и возврат её последующего адреса

//   "K_Frame.cpp"
//   г---------------------------------------------------------¬
//   ¦                                                         ¦
//   ¦       Численное определение одного шпангоута            ¦
//   ¦                                                         ¦
//   ¦  X(0),Y(0) - точка киля                                 ¦
//   ¦  X(1),Y(1) - точка на сломе борта к палубе (ширстрек)   ¦
//   ¦  X(-1),Y... -//- то же - левого борта                    ¦
//   ¦                                                         ¦
//   ¦                                                         ¦
//   L=========================================================-
//                    2001.11.02, В.Храмушин, НИС "П.Гордиенко",
//                                            Татарский пролив
#define eps 1.0e-12

Frame:: Frame( int l ){ z=y=_z=_y=0; N=0; X=0.0; if( l>0 )allocate( l ); }
Frame::~Frame()       { free(); }

Frame& Frame::allocate( int l )
{ if( l<=0 )free(); else
    { if( _z ){ Allocate( 0,_z ),Allocate( 0,_y ); _z=_y=0; }
      z=(Real*)Allocate( l*sizeof(Real),z );
      y=(Real*)Allocate( l*sizeof(Real),y ); N=l-1;
    } return *this;
}
Frame& Frame::free()
{ if( z ){ Allocate( 0,z ); Allocate( 0,y );
    if(_z )Allocate( 0,_z ),Allocate( 0,_y );
  } z=y=_z=_y=0; N=0; return *this;
}
void Frame::Double( int k )
{ int i; z=(Real*)Allocate( (N+2)*sizeof(Real),z );
         y=(Real*)Allocate( (N+2)*sizeof(Real),y ); ++N;
  for( i=N; i>k; i-- ){ z[i]=z[i-1];
                        y[i]=y[i-1];
}                     }
//
//   Предварительный расчет для построения сплайн-функции
//
Frame& Frame::SpLine()
{ if( N<3 )
  { if( _z )Allocate( 0,_z ),Allocate( 0,_y ); _z=_y=0; return *this;
  }
 Real *u,*v,*zi=z+1,
            *yi=y+1;
    u=(Real*)Allocate( N*sizeof( Real )*2 ); v=u+N;
  (_z=(Real*)Allocate( (N+1)*sizeof( Real ),_z ))[N]=0; *_z=0;
  (_y=(Real*)Allocate( (N+1)*sizeof( Real ),_y ))[N]=0; *_y=0;
  for( int i=1; i<N; i++,zi++,yi++ )
  { _z[i]=-1.0/( _z[i-1]+4.0 );
    _y[i]=-1.0/( _y[i-1]+4.0 );
    if( zi[1]==zi[0] || zi[0]==zi[-1] )u[i-1]=v[i-1]=u[i]=v[i]=0.0; // _zi всё-таки аргумент
    else u[i]=_z[i]*( u[i-1]-6.0*(zi[1]-2.0*zi[0]+zi[-1]) ),
         v[i]=_y[i]*( v[i-1]-6.0*(yi[1]-2.0*yi[0]+yi[-1]) );
  }
//  for( int i=1; i<N; i++,zi++,yi++ )
//  { _z[i]=-0.5/(_z[i-1]/2+2); u[i]=-2*_z[i]*(3*(zi[1]-2*zi[0]+zi[-1])-u[i-1]/2);
//    _y[i]=-0.5/(_y[i-1]/2+2); v[i]=-2*_y[i]*(3*(yi[1]-2*yi[0]+yi[-1])-v[i-1]/2);
//  }
  for( int i=N-1; i>=0; i-- ){ _z[i]=_z[i]*_z[i+1]+u[i];
                               _y[i]=_y[i]*_y[i+1]+v[i];
                             }  Allocate( 0,u ); return *this;
}                               //
Real Frame::Z( Real A )         // Параметрическое определение
{                               // аппликаты шпангоута по
  if( A<0 || A>1 )return 0;     // аргументу: 0.0<=A<=1.0
 int k;    A*=N;              //
 double b; k=max( min( int( A ),N-1 ),0 ); b=A-k;
  if( _z )
  { double a=1-b;
    return a*z[k]+b*z[k+1]+( (a*a-1)*a*_z[k]+(b*b-1)*b*_z[k+1] )/6;
  } return z[k]+b*( z[k+1]-z[k] );
}                             //
Real Frame::Y( Real A )       // Параметрическое определение
{                             // ординаты шпангоута по
  if( A<0 || A>1 )return 0;   // аргументу: 0.0<=A<=1.0
 int k;    A*=N;              //
 double b; k=max( min( int( A ),N-1 ),0 ); b=A-k;
  if( _y )
  { double a=1-b;
    return a*y[k]+b*y[k+1]+( (a*a-1)*a*_y[k]+(b*b-1)*b*_y[k+1] )/6;
  } return y[k] + b*( y[k+1]-y[k] );
}
void Frame::YZ( Real A, Real &Y, Real &Z )
{ if( A<0 ){ Y=0; Z=z[0]; return; } //
  if( A>1 ){ Y=0; Z=z[N]; return; } // Аргумент: 0.0<=A<=1.0
 int k;      A*=N;                  //
 double b;   k=max( min( int( A ),N-1 ),0 ); b=A-k;
  if( y[k]==y[k+1] && z[k]==z[k+1] )Y=y[k],Z=z[k]; else
  if( _y )
  { double a,a1,b1;
    a=1-b;
    a1=(a*a-1)*a/6;
    b1=(b*b-1)*b/6; Y = a*y[k] + b*y[k+1] + a1*_y[k] + b1*_y[k+1];
                    Z = a*z[k] + b*z[k+1] + a1*_z[k] + b1*_z[k+1];
  } else
  { Y = y[k] + b*( y[k+1]-y[k] );
    Z = z[k] + b*( z[k+1]-z[k] );
  }
}
Real Frame::operator()( Real _z_ ) // эмуляция плазовой ординаты
{ int k;                             //  Y - однозначная функция
  for( k=0;;k++ )                    //
  { Real d=z[k+1];                    //
    if( k>=N-1 || d>=_z_ )             // линейная интерполяция
    { d-=z[k]; if( d<eps )return y[k]; // индексного аргумента
      return Y( (k+(_z_-z[k])/d)/N );  //
} } }                           //
int Frame::iZ( Real &_y_ )      // последовательный поиск
{ static int k=0;               // корней неоднозначной функции
  Real a,b;
  int i;
// union{ Real r; point s; }a,b;  //
  a=y[k]-_y_;
  for( i=k;;i++ )
  { if( i>=N )return k=0;                   // ничего не найдено
    if( fabs( a )<eps ){ _y_=z[k]; break; } // попадание в точку
    b=y[i+1]-_y_;                   //
    if( a*b<0 )                     // пересечение по интервалу аргумента
    { _y_=Z( (i-a/(b-a))/N ); break;
    } a=b;
  } k=i+1; return k;    // возвращвется порядковый номер найденной точки
}
//   г---------------------------------------------------------¬
//   ¦                                                         ¦
//   ¦ Блок операторов для корпуса в целом                     ¦
//   ¦                                                         ¦
//   L=========================================================-
//
Real Hull::operator()( Real x, Real z )
{
 Real A,S; int k;             //
  if( x>Stx( z ) )return 0;     // Поиск ближайшего левого индекса
  if( x<Asx( z ) )return 0;     //
  for( k=0; k<Ns-2 && x>F[k+1].X; k++ ); S=F[k+1]( z );
                                         A=F[k]( z );
 return max( A+(S-A)*(x-F[k].X)/(F[k+1].X-F[k].X),Real( 0.0 ) );
}
//   г---------------------------------------------------------¬
//   ¦                                                         ¦
//   ¦ Грузовой размер и площадь смоченной поверхности корпуса ¦
//   ¦                                                         ¦
//   L=========================================================-
//
void Hull::Init()
{
 const int mZ=48;       //
   int i,j;             // простой расчет полного объема корпуса
  Real x,Dx,dx,         //      (по внутренним трапециям)
       z,Dz,dz,         //
       y,dy,Z,          //
       Vs,Vm;           // Площадь шпангоута
       Z=Draught-Do;    //
       dz=Z/mZ;
       Vm=Volume=Surface=0;
  for( i=0; i<Ns; i++ )
  { Vs=0.0;
    x=F[i].X;
    if( i==Ns-1 )dx=x;  else dx=F[i+1].X;
    if( !i      )dx-=x; else dx-=F[i-1].X;
    dy=dx*dz;                           // dy здесь удвоено
    for( j=0,z=Do; j<=mZ; z+=dz,j++ )   // Водоизмещение
    { Vs+=y=(*this)( x,z )*dy;          // Объем корпуса
      if( y>0 )
      { if( !j )Surface+=y*dx;
        Dx=( (*this)( x+dx/2,z )-(*this)( x-dx/2,z ) )/dx;
        Dz=( (*this)( x,z+dz/2 )-(*this)( x,z-dz/2 ) )/dz;
        Surface+=sqrt( 1+Dx*Dx+Dz*Dz )*dy; // Смоченная поверхность корпуса
    } } Volume+=Vs;
    if( Vs>Vm ){ Vm=Vs; if( Ms<0 )Ms=-1-i; }
  }                     if( Ms<0 )Ms=-1-Ms;

}
//
//   Несколько программ анализа "Curve-Point" векторов
//
Curve& Curve::allocate( int Sz )
{ _C=(Point*)Allocate( (N=Sz)*sizeof( Point ),_C ); return *this; }
Curve& Curve::free(){ if( _C )_C=(Point*)Allocate( N=0,_C ); return *this; }
#define  _X( k ) ( _C[(k)].x )
int Curve::find( Real Ar )       // Двоичный поиск ближайшего левого
{ int i=0,k,n=N-1,d=_X(n)>_X(0); //  индекса для заданного аргумента
  while( n-i>1 )                 //   с учетом знака его прироста
  {   k=(n+i)/2;                 //
    if( d ){ if( Ar<_X(k) )n=k; else i=k; }
      else { if( Ar>_X(k) )n=k; else i=k; }
  } return i;
}
Real Curve::operator()( Real Ar )
{  int k;
 Point *P=_C+( k=find( Ar ) );
  return P[0].y + ( Ar-P[0].x )*(P[1].y-P[0].y)/(P[1].x-P[0].x);
}
//      Подготовка экстремумов функции и аргумента
//
void Curve::Extreme( Field& Fm )
{ for( int i=0; i<N; i++ )
  { Point P=_C[i];
    if( i )
    { if( Fm.Jy>P.y )Fm.Jy=P.y; else if( Fm.Ly<P.y )Fm.Ly=P.y;
      if( Fm.Jx>P.x )Fm.Jx=P.x; else if( Fm.Lx<P.x )Fm.Lx=P.x;
    } else
    { Fm.Lx=( Fm.Jx=P.x )+1.0e-12;
      Fm.Ly=( Fm.Jy=P.y )+1.0e-12;
  } } Fm.Ly-=Fm.Jy;
      Fm.Lx-=Fm.Jx;
}

//    "K_Math.cpp"
//    Обращение к процедурам математического описания
//      или интерполяции реального корпуса (K-HULL)
//
       Hull  Kh;        // Собственно корпус
static Real  Lh=2.0,    // Расчетная длина корпуса
             Bh=1.0,    //   и его опорная ширина
             Ts=0.75,   // Заглубление носового бульба
             Ns=0.6,    // Степень кривизны форштевня (1/Ns)
             Na=1.6,    // Степень крутизны кормрвого подзора
             Fs=0.0,    // Наклон форштевня
             Ls=0.1,    // Относительная длина бульба
             La=0.15,   // Длина кормового подзора
             Cx=0.5,    // Косинусная модуляция параболических ватерлиний
             Px=3,      // Степень полноты параболических ватерлиний
             Pz=6,      // Степень полноты шпангоутов
             Tz=0.5,    // Коэффициент заглубления бульба
             Ps=1.2,    // Параметры обратной полноты: P ў ]0Ў-[
             Pa=1.2,    //    P>10  косинусоиды
                        // 1 <P< 5  бульбовые ватерлинии
                        //    Pў 1  форма капли
                        //    P<.5  форма крыла ската
         Ks=0.8,Kp=1;   // Наклон борта: K>1 - развал; K<1 - завал
                        //
static int Hull_Type=2; // 0-Синустепенной; 1-параболический корпус
                        // 2-Реальный
static Mlist            //
       Menu_a[]=
        { { 1,0,"\t         Описание штевней" }
        , { 2,4,"\t       Наклон форштевня Fs [0\xB6\x30.5] = %4.2lf",&Fs }
        , { 1,4,"\t     Заглубление бульба Ts [0.5\xB6\x31] = %4.2lf",&Ts }
        , { 1,4,"\t  Длина носового бульба Ls [0\xB6\x30.5[ = %4.2lf",&Ls }
        , { 1,4,"\t       Степень кривизны Ns ]1\xB6\xBA\x32\xBA] = %4.2lf",&Ns }
        , { 1,4,"\tДлина кормового подзора La [0\xB6\x30.5[ = %4.2lf",&La }
        , { 1,4,"\t       Крутизна подзора Na ]1\xB6\xBA\x32\xBA] = %4.2lf",&Na }
        , { 1,6,"\t      Длина корпуса Length       = %6.2lf",  &Lwl }
        , { 1,6,"\t      Ширина       Breadth       = %6.2lf",  &Breadth }
//      , { 1,6,"\t      Высота борта   Depth       = %6.2lf",  &Height  }
        , { 1,6,"\t      Осадка       Draught       = %6.2lf",  &Draught } },
       Menu_s[]=
        { { 1,0,"\tПараметры обратной полноты: P\xBA]0\xB6-[" }
        , { 1,0,"\t    P>10  косинусоиды"               }
        , { 1,0,"\t 1 <P< 5  бульбовые ватерлинии"      }
        , { 1,0,"\t    P\xBA 1  форма капли"               }
        , { 1,0,"\t    P<.5  форма крыла ската"         }
        , { 1,4,"\t    P- stem  = %4.1lf",&Ps            }
        , { 1,4,"\t    P-astern = %4.1lf",&Pa            }
        , { 1,4,"\t Заглубление = %4.2lf",&Tz },{ 0,0,"\t \xBA ]0\xB6\x31]" }
        , { 1,4,"\tHаклон борта = %4.2lf",&Ks },{ 0,0,"\t развал>1>завал" } },
       Menu_p[]=
        { { 1,0,"\t   Корпус построенный на"  }
        , { 1,0,"\t     степенных функциях"   }
        , { 2,4,"\tШпангоуты  Pz = %4.1lf",&Pz }
        , { 1,4,"\tВатерлинии Pх = %4.1lf",&Px }
        , { 1,4,"\tЗаострение Сх = %4.2lf",&Cx },{ 0,0,"\t \xBA ]0\xB6\x31]" }
        , { 1,4,"\tНаклон борта  = %4.2lf",&Kp },{ 0,0,"\t развал>1>завал" }
        };                                  //
static Real m_Stern( Real _z );             // Ахтерштевень
static Real m_Stem ( Real _z );             // Форштевень
static Real m_Hull ( Real _z, Real _x );    // Корпус
                                            //
static int Set_Stem()
{ int i,j;
  Ns=1/Ns; Tmenu( Mlist( Menu_a ),1,-1 );
  Ns=1/Ns;
  scpy( Kh.Name,
   W2D( Hull_Type?"Синусно-степенной корпус":"Параболическая модель корпуса") );
  if( Breadth<0.1 )Breadth=0.1;
  if( Draught>Breadth )Draught=Breadth;
  if( Lwl<Breadth*3 )Lwl=Breadth*3;
//-------
  Lh*=Lwl/m_Stem( Draught ); //-m_Astern( Draught ) );
  Xm =Lwl/2;
  Height=Draught*2.0;
  Xo  = m_Stern( 0 );
  Length=m_Stem( 0 );
  for( i=1; i<32; i++ )
  { Do=i*Height/32;
    if(    Xo>m_Stern( Do ) )Xo=m_Stern( Do );
    if( Length<m_Stem( Do ) )Length=m_Stem( Do );
  } Length-=Xo;
    Bwl=0.0;
  for( i=0; i<32; i++ )
  { Do=i*Draught/32;
    if( Bwl<m_Hull( Do,Xm ) )Bwl=m_Hull( Do,Xm );
  } Bh *= Breadth/Bwl/2;
    Bwl=m_Hull( Draught,Xm )*2;
    Do=0.0;
 //
 // Производится перестроение аналитического корпуса к обычному виду
 //
 const int Nx=24,Nz=36;
 Real Dx,Dz,x,z;        Dx=Length/( Nx+1 );
 static Frame *Deck=NULL;
 if( !Deck ){ (Deck=(Frame*)calloc(sizeof(Frame),1))->allocate(5); } // без new
 { Real *X=Deck->z,*Z=Deck->y; Z[0]=Height*0.9,X[0]=Xo;
                               Z[1]=Height*0.9,X[1]=Xo+Dx/2;
                               Z[2]=Height*0.8,X[2]=Length/2;
                               Z[3]=Height,    X[3]=Length-Dx;
                               Z[4]=Height,    X[4]=Length; Deck->SpLine();
 } Kh.Simple_Hull( Nx,Nz );
  for( i=0; i<Nx; i++ )                         // pow( Nz-1,0.648054273 );
  { Kh.F[i].X=x=Xo+Dx*(i+1); Dz=(*Deck)( x )/pow( Nz-1,1.543080635 );
    for( j=0; j<Nz; j++ )
    { z=Dz*pow( j,1.543080635 );
                   Kh.F[i].z[j]=z;
                   Kh.F[i].y[j]=m_Hull( z,x );
      if( i==Nx-1 )Kh.Stx[j].x=Kh.Sty[j].x=z,
                   Kh.Stx[j].y=m_Stem( z );
      else if( !i )Kh.Asx[j].x=Kh.Asy[j].x=z,
                   Kh.Asx[j].y=m_Stern( z );
  } } Hull_Type=2; Kh.Init(); return 0;
}
int Set_Sinus()
{ Hull_Type=1; Tmenu( Mlist( Menu_s ),1,-1 ); return Set_Stem();
}
int Set_Power()
{ Hull_Type=0; Tmenu( Mlist( Menu_p ),1,-1 ); if( Cx>1 )Cx=1; return Set_Stem();
}
static Real m_Stem( Real z )
{ extern Real K_Stem( Real ); if( Hull_Type==2 )return Kh.Stx( z );
  { Real m = Ts*Ns/(1-Ts),
         l = pow( Ts,m )*pow( 1-Ts,Ns );
    z = 1-z/Draught;
    return Lh *
     ( 1 + ( 1-(Fs*(z+1))/2 )           // наклон форштевня
         * ( 1+Ls*( z<0?
                    z*z*(1-Ls):
                    pow( fabs(z),m )
                 *  pow( 1-z,Ns )/l) )/( 1+Ls ) )/2;
} }
static Real m_Stern( Real z )
{ extern Real K_Stern( Real ); if( Hull_Type==2 )return Kh.Asx( z );
  z = 1-z/Draught;
  return Lh*( 1 + sin( tanh( z*Na )*2.25 )*( z+2 )*La/3-1 )/2;
}
static Real m_Hull( Real z, Real x )
{ if( Hull_Type==2 )return Kh( x,z );
  Real A,S,Z;
   if( (S= m_Stem( z ))<x )return 0;
   if( (A=m_Stern( z ))>x )return 0;
   x = ( 2*x-S-A )/( S-A );
   z = 1-z/Draught;
   Z = z*Tz;
  return
  Hull_Type
  ? Bh * pow( 1-z,1/(2+8/(Pa+Ps)) )   // Мидель шпангоут
       / ( 1+1/Pa+1/Ps )/2            // Нормировка на полуосадку
       * ( 1+cos( M_PI*x ) )          // Синусная ватерлиния
       * ( 1+pow( Ps*( 1-x ),-Z-1 )   // Носовая группа линий
            +pow( Pa*( 1+x ),+Z-1 ) ) // Кормовые множители
       * ( 1+(1-Ks)*z )               // Коэффициент завала бортов
  : Bh * ( 1-pow( fabs( x ),Px ) )    // Степенная ватерлиния
       * ( cos( x*M_PI_2*Cx ) )       // и ее косинусная модуляция
       * ( 1-(z>0?pow( z,Pz ):0) )    // Степенной шпангоут
       * ( 1+(1-Kp)*z );         //====           +
}                               //        -+     -+-  o
                               //         -+-   --+/--=¬  ¦
void Building()               //       ___\¦ г--+-+--°°L¬\+/
{                            //        ----+-¦---=======¦=¦===¦
 field f; Real x,y,z;       //         --__________________v_/ ___
          int  k,n,m;      //   ~~~~~~ЁЁ¦8                  /~~--
//                      __//~~~ ~~   ~~ L-------------------
//   г---------------------------------------------------------¬
//   ¦                                                         ¦
//   ¦   Проекция полуширота Ёё                                ¦
//   ¦                                                         ¦
//   L=========================================================-
  clear();
  f.Jx=9;  f.Lx=90; f.wb=0;
  f.Jy=30; f.Ly=33;
  wW.Set( f );
  wW.F.Jx=Xo;     wW.F.Jy= Breadth/2.0;
  wW.F.Lx=Length; wW.F.Ly=-Breadth/2.0;
  wW.Focus();              //
  for( k=0; k<Kh.Ns; k++ ) // Первыми прорисовываются ватерлинии
  { x=Kh.F[k].X;           //
    color( CYAN );
    setlinestyle( k==Kh.Ms?SOLID_LINE:DOTTED_LINE,0,NORM_WIDTH );
    line( x,0.0,x,Breadth/2 );
    color( LIGHTGRAY ); setlinestyle( SOLID_LINE,0,NORM_WIDTH );
    Tv.Text( South,x,Breadth*.502,_Fmt( "%d",k ) );
  }
  color( CYAN );
  x=Kh.Asx( Draught );
  for( k=0,z=0; k<=5; z+=Breadth/10,k++ )
  { setlinestyle( k%5?DOTTED_LINE:SOLID_LINE,0,NORM_WIDTH );
    line( x,z,x+Lwl,z );
  }
  for( z=0.0; z<=Height; z+=Draught/5 )
  { color( z>Draught?LIGHTCYAN:LIGHTGREEN );
    for( x=Xo,moveto( x,Kh( x,z ) ); x<Length; x+=Length*0.001 )
              lineto( x,Kh( x,z ) );
  }
  color( YELLOW );                      //
  moveto( x=Kh.Asx[Kh.Asx.N-1].y,0.0 ); // Прорисовка линии палубы
  lineto( x,Kh.Asy[Kh.Asy.N-1].y );     //
          y=Kh.Stx[Kh.Stx.N-1].y;
  for( k=0; k<Kh.Ns; k++ )
  { z=Kh.F[k].X;
    if( z>x && z<y )lineto( z,Kh.F[k].y[Kh.F[k].N] );
  } lineto( y,Kh.Sty[Kh.Sty.N-1].y );
    lineto( y,0.0 );
     color( WHITE );
  Tv.Text( North_West,Tv_port.right-4,Tv_port.bottom-2,Kh.Name );
//
//   г---------------------------------------------------------¬
//   ¦                                                         ¦
//   ¦   Проекция бок                                          ¦
//   ¦                                                         ¦
//   L=========================================================-
//
  f.Jy=64; f.Ly=36;
  wM.Set( f );
  wM.F.Ly=Height; wM.F.Jy=Do;
  wM.F.Lx=Length; wM.F.Jx=Xo;
  wM.Focus();
  x=Kh.Asx( Draught );
  for( k=0,z=0.0; z<Height; z+=Draught/5,k++ )
  { color( CYAN ); setlinestyle( k%5?DOTTED_LINE:SOLID_LINE,0,NORM_WIDTH );
    line( x,z,x+Lwl,z );
    color( LIGHTGRAY );  setlinestyle( SOLID_LINE,0,NORM_WIDTH );
    Tv.Text( West,Xo-Lwl/30,z,_Fmt( k!=5?"%d":"WL",k ) );
  }
  //
  // Разметка шпангоутов
  //
  color( CYAN );
  for( k=0; k<Kh.Ns; k++ )
  { setlinestyle( k==Kh.Ms?SOLID_LINE:DOTTED_LINE,0,NORM_WIDTH );
    x=Kh.F[k].X;  line( x,0.0,x,Height );
  } setlinestyle( SOLID_LINE,0,NORM_WIDTH );
  //
  // Выделение и расчистка окна для проекции корпус
  //
  n=Tv_x( Kh.F[Kh.Ms].X );
  k=( int( Breadth*( Tv_y( Do )-Tv_y( Do+Height ) )/Height )+1 )/2;
  if( k*3>Tv.mX )k=Tv.mX/3;
  f.Jx=k-n;
  f.Lx=n+k-Tv.mX;
  wH.Set( f );
  wH.F.Lx=Breadth;    wH.F.Ly=Height;
  wH.F.Jx=-Breadth/2; wH.F.Jy=Do;     Tv_bar();
  //
  // Батоксы
  //
  wM.Focus();
  {                     //
   Real **A,Z;         // - рисуются как изолинии
   int   z,x;           //
    A=(Real**)Allocate( 81,101*sizeof(Real) ); Tf_List( 0 );
    for( z=0; z<=80;  z++ )
    for( x=0; x<=100; x++ )A[z][x]=Kh( Xo+x*Length/100,Do+z*Height/80 );
    for( Z=Breadth/10; Z<Breadth*0.502; Z+=Breadth/10 )Tf_List( Z,LIGHTGRAY );
                                                 Tf_Topo( A,0,0,81,101,TRUE );
    Allocate( 0,0,A );
  }
  //
  // Разметка контура: палуба - штевни - киль
  //
  color( YELLOW );
  m=Kh.Stx.N-1;
  n=Kh.Asx.N-1;           moveto( ~Kh.Asx[0] ); //
  for( k=1; k<=n;    k++ )lineto( ~Kh.Asx[k] ); // Ахтерштевень
  for( k=0; k<Kh.Ns; k++ )
  { x=Kh.F[k].X;
    if( x>Kh.Asx[n].y && x<Kh.Stx[m].y )lineto( x,Kh.F[k].Z( 1.0 ) );
  }
  for( k=m; k>=0; k-- )lineto( ~Kh.Stx[k] );    // Форштевень
  for( k=Kh.Ns-1; k>=0; k-- )
  { x=Kh.F[k].X;
    if( x>Kh.Asx[0].y && x<Kh.Stx[0].y )lineto( x,Kh.F[k].z[0] );
  }                                     lineto( ~Kh.Asx[0] );
  //
  // Вызов рисовалки для проекции корпус
  //
  wH.Focus();
  Draw_Hull();
}
//   г----------------------------------------------------------------¬
//   ¦  Прорисовка проекции корпус (основного теоретического чертежа) ¦
//   L================================================================-
//
void Draw_Hull( int id ) // Проекция корпус
{                        //
 int i,k; Real x,y,z;    // Сетка
  color( CYAN );         //
  for( k=0,z=0.0; z<=Height; k++,z+=Draught/5 )
  { setlinestyle( k%5?DOTTED_LINE:SOLID_LINE,0,NORM_WIDTH );
    line( -Breadth/2,z,Breadth/2,z );
  }
  for( k=0,y=-0.4*Breadth; k<=8; k++,y+=Breadth/10 )
  { setlinestyle( k==4?SOLID_LINE:USERBIT_LINE,0x5555,NORM_WIDTH );
    line( y,Do,y,Height );
  } setlinestyle( SOLID_LINE,0,NORM_WIDTH );
  if( id )
  { color( WHITE ); Tv.Text( East,Breadth/2,Draught,W2D( " ВЛ" ) );
                    Tv.Text( South,(Real)0.0,0.0,   W2D( "ДП" ) );
  }                     //
       i=0;             // Прорисовка исходных теоретических шпангоутов
  for( k=0; k<Kh.Ns; k++ )
  { if( !i && k==Kh.Ms+1 ){ i=1; k=Kh.Ms; }
    for( x=0.0; x<=1.0; x+=0.001 )
    { Kh.F[k].YZ( x,y,z );
      color( z>Draught?LIGHTCYAN:LIGHTGREEN );
      if( !i )y=-y;
      if( !x )moveto( y,z ); else lineto( y,z );
  } }                        moveto( ~conj( Kh.Asy[0] ) ); color( WHITE );
  for( k=1; k<Kh.Asy.N; k++ )lineto( ~conj( Kh.Asy[k] ) ); color( LIGHTCYAN );
            i=Kh.Ns/2+1;        //
  for( k=0; k<Kh.Ns; k++ )      // Палуба (и транцы)
  {                             //
    if( i && k==i ){ k=i-1; i=0; lineto( y,z ); moveto( -y,z ); }
    y=Kh.F[k].y[Kh.F[k].N];
    z=Kh.F[k].z[Kh.F[k].N];
    if( !y && k==Kh.Ns-1 )lineto( ~Kh.Sty[Kh.Sty.N-1] );
    if( i )y=-y; lineto( y,z );
    if( !y && !k )lineto( ~conj( Kh.Asy[Kh.Asy.N-1] ) );
  }      k=Kh.Sty.N-1;
         i=0;
  for( ; k>=0; k-- ){ lineto(~Kh.Sty[k]); if(!i)color(i=WHITE); }
}
//   г---------------------------------------------------------¬
//   ¦ Подготовка упрощенного варианта теоретического корпуса  ¦
//   L=========================================================-
//
void Plaze::allocate( int z, int x )
{  Y=(Real**)Allocate( z,x*sizeof(Real),Y ); Nx=x; Nz=z;
  QV=(Real**)Allocate( z,x*sizeof(Real),QV); // Источники со смещенными узлами
  Xa=(Real*)Allocate( z*sizeof(Real),Xa );   // Абсциссы ахтерштевня
  Xs=(Real*)Allocate( z*sizeof(Real),Xs );   //        и форштевня
  Ya=(Real*)Allocate( z*sizeof(Real),Ya );   // Аппликаты на форштевне
  Ys=(Real*)Allocate( z*sizeof(Real),Ys );   //         и ахтерштевне (транце)
  Wx=(Real*)Allocate( x*sizeof(Real),Wx );   // Вектор волнообразования
}
void Plaze::build( Real _z )
{ int  i,j;
  Real x,y,z,sx,sz;
  if( Nx<3 || Nz<2 )allocate( 12,48 ); // если не выделена память
  dX=Length/(Nx-1);
  dZ=(Depth=_z-Do)/(Nz-1);
  V=S=0.0;
  for(   z=Do,j=0; j<Nz; j++,z+=dZ )
  { for( x=Xo,i=0; i<Nx; i++,x+=dX )V+=Y[j][i]=Kh( x,z );
    Xa[j]=m_Stern( z );         // ахтерштевень
    Xs[j]=m_Stem( z );          // форштевень
  }
  V*=dX*dZ*2.0;                 // водоизмещение
  for( i=1; i<Nx; i++ )
  for( j=0; j<Nz; j++ )
  { y=Y[j][i];
    if( y>0.0 )
    { if( j<Nz-1 )
      if( (sz=Y[j+1][i])>0.0 )  // Смоченная поверхность корпуса
      { sx=( Y[j][i-1]-y )/dX;
        sz=( sz-y )/dZ;
        S+=sqrt( 1+sx*sx+sz*sz )*dX*dZ; goto Lx;
      } S+=y*dX;                             Lx:;
  } }   S*=2.0;
  for( j=0; j<Nz; j++ )         // Распределенные источники
  {
    for( i=1; i<Nx-1; i++ )QV[j][i]=(Y[j][i+1]-Y[j][i-1] )*dZ/-2.0;
    QV[j][Nx-1]=QV[j][Nx-2]/2.0;
    QV[j][0]=QV[j][1]/2.0;
  }
}

//    "K_Statics.cpp"
//    +----------------------------------------------------------------+
//    ¦ Расчёт диаграмм Рида и кривых элементов теоретического чертежа ¦
//    *================================================================*
//                     1997, январь, В.Храмушин, Южно-Сахалинск
//                     2001, август - Владивосток, октябрь - ГС-210
//
const int _nZ=36,_nX=72, // Количество точек на сплайновых кривых
          _nA=36;        // Количество углов накренения
const Real mA=180;       // Максимальный угол накренения в градусах
static Real mX=0;        // аппликата метацентра
static Real Amin=-0.1,Amax=0.1;

struct Hydrostatic:Plaze
{ Real  *Vol, //[_nZ],      // Грузовой размер
        *zCV, //[_nZ],      // Аппликата центра величины
        *Swl, //[_nZ],      // Площадь ватерлинии
        *Srf, //[_nZ],      // Смоченная поверхность корпуса
        *xCV, //[_nZ],      // Абсцисса центра величины
        *xCW, //[_nZ],      // Абсцисса центра площади ватерлинии
        *rfX, //[_nZ],      // Поперечный метацентрический радиус
        *Rfy, //[_nZ],      // Продольный метацентрический радиус
        **aV; //[_nA][_nZ]; // Массив накренютых водоизмещений
  Point **aC; //[_nA][_nZ]; // -- аппликат и ординат новых центров величины
  Hydrostatic(): Plaze( _nZ,_nX )
    { aV=(Real**)Allocate( _nA,_nZ*sizeof( Real ) );
      aC=(Point**)Allocate( _nA,_nZ*sizeof( Point ) );
      Vol=(Real*)calloc( _nZ,sizeof( Real ) );
      zCV=(Real*)calloc( _nZ,sizeof( Real ) );
      Swl=(Real*)calloc( _nZ,sizeof( Real ) );
      Srf=(Real*)calloc( _nZ,sizeof( Real ) );
      xCV=(Real*)calloc( _nZ,sizeof( Real ) );
      xCW=(Real*)calloc( _nZ,sizeof( Real ) );
      rfX=(Real*)calloc( _nZ,sizeof( Real ) );
      Rfy=(Real*)calloc( _nZ,sizeof( Real ) );
    }
 ~Hydrostatic(){ Allocate( 0,0,aV ); Allocate( 0,0,aC );
                 free( Vol ); free( zCV ); free( Swl ); free( Srf );
                 free( xCV ); free( xCW ); free( rfX ); free( Rfy ); }
  void Initial();
  void Graphics();
  void Stability();
};
//   г---------------------------------------------------------¬
//   ¦                                                         ¦
//   ¦  Основные кривые элементов теоретического чертежа       ¦
//   ¦       формы корпуса судна, с определением базовых       ¦
//   ¦                                                         ¦
//   ¦   Draw_Hull( k ) - прорисовка проекции корпус           ¦
//   ¦   Volume_and_Surface( Z ) - грузовой размер             ¦
//   ¦                             и площадь обшивки           ¦
//   ¦                                                         ¦
//   L=========================================================-
//
void Hydrostatic::Initial()
{   int i,k,l;
 double r,R,S,sX,v,
        x,y,z;
  build( Height);
//for( k=0; k<_nZ; k++ )xCW[k]=Xm;
  for( k=0; k<_nZ; k++ )
  { R=r=sX=S=0.0;               // Первый прогон интегрирования
    x=dX/2;                     // для вычисления площадей, объемов,
    z=dZ*( double( k )+0.5 );   // их моментов инерции - посредине
    for( i=0; i<_nX; i++,x+=dX )// и метацентрических радиусов
    { S+=( y=Y[k][i] );
      r+=y*y*y;                 // стр.61 у Владимира Вениаминовича С-Т-Ш
      R+=y*x*x;
      sX+=y*x;
    }
    Swl[k]=(S*=2*dX);           // Площадь ватерлинии
           sX*=2*dX;            // Момент площади ватерлинии
    if( k>0 )
    { v=dZ*(S+Swl[k-1])/2;
      Vol[k]=Vol[k-1] + v;      // Водоизмещение
      zCV[k]=zCV[k-1] + v*z;    // Момент для аппликаты центра величины zC
      xCV[k]=xCV[k-1] + sX*dZ;  // Момент для абсциссы центра величины xC
    }
    if( S<=0 ) //|| k==0 )
      xCW[k]=Xm,Rfy[k]=rfX[k]=0; else
    { xCW[k]=sX/S+Xo;           // Центр площади ватерлинии
      Rfy[k]=(R*2*dX-sX*sX/S);  // /Vol[k]->Продольный метацентрический радиус
      rfX[k]= r*2*dX/3;         // /Vol[k]->Поперечный --//--
    }
//  if( !S )continue;
//  xCW[k]=sX/S+Xo;             // Центр площади ватерлинии
//  Rfy[k]=(R*2*dX-sX*sX/S);    // /Vol[k]->Продольный метацентрический радиус
//  rfX[k]= r*2*dX/3;           // /Vol[k]->Поперечный --//--
  }
  zCV[0]=Do; //xCW[0];
  Srf[0]=S=Swl[0];              // Площадь смоченной поверхности
  for( l=k=1; k<_nZ; k++ )
  { if( l<0 || Swl[k] )
    { for( i=1; i<_nX; i++ )
      { y=Y[k][i];
        if( y>0 )S+=sqrt( 1+norm( Y[k-1][i]-y,Y[k][i-1]-y ) )*dX*dZ;
      } Srf[k]=S*2;
      zCV[k]=zCV[k]/Vol[k]+Do;
      xCV[k]=xCV[k]/Vol[k]+Xo;
      if( !Swl[k-1] && l>0 )
      for( l=k-1; l>=0; l-- )xCV[l]=xCV[k],xCW[l]=xCW[k],zCV[l]=Do; l=-1;
      if( !Swl[k] )xCW[k]=xCW[k-1];       ///???
    }
  }
}
static void MinMax( const Real *F, int N, Real &Min, Real &Max, const int mx=0 )
{ int i; if( !mx )Min=Max=F[0];
  for( i=0; i<N; i++ )if( Min>F[i] )Min=F[i]; else
                      if( Max<F[i] )Max=F[i];
}
static void Graphic_for_Element
( const Real *C,      // собственно прорисовываемая кривая (NULL - если ее нет)
   int y,             // смешение оси по вертикали
  Real I,             // начальный отсчет шкалы
  Real L,             // длина шкалы оси
  const int up,       // длина маркеров отметки шкалы (-вверх, +вниз, 0-нет)
  const int sp,       // местоположение метки
  const char Label[], // подпись на шкале и на графике
  const int Scale=1   // признак перемасштабирования (округления) шкалы
)
{ Field F; Real x,dL; int i; F.Jx=F.Jy=0.0;
  if( Scale )
  { if( L==0.0 )MinMax( C,_nZ,I,L );
    dL=Tv_step( L-I );
    I=floor( I/dL )*dL;
    L=ceil( (L-I)/dL )*dL;
  } else dL=Tv_step( L ); F.Jx=I;
                          F.Lx=L;
                          F.Ly=_nZ-1; Tv_place( 0,&F );
  if( up )
  { line( Tv.mX/2,y,Tv.mX,y );
    for( x=int(I/dL+1)*dL; x<I+L+dL/2; x+=dL )
    { i=Tv_x( x );                      //
      line( i,y,i,y+up );               // Разметка шкалы
      if( x>0 )                         //
      Tv.Text( up>0?South_West:North_West,i-1,y+up/abs(up),_Fmt( "%.9lg",x ) );
    } Tv.Text( up>0?South_East:North_East,Tv.mX/2,y+up/abs(up),Label );
  }
  if( C )
  { for( i=0; i<_nZ; i++ )if( i )lineto( C[i],(Real)i );
                            else moveto( C[i],(Real)i );
    Tv.Text( West,Tv_x( C[sp] )-3,Tv_y( sp ),Label );
  }
}
//
//  Блок расчета грузового размера и смоченной поверхности корпуса
//
void Hydrostatic::Graphics()
{ int i,k=_nZ/3;
  Real mn,mx,mz=Height-Do;

  color( YELLOW );
  Graphic_for_Element( Vol,Tv.mY/2+30,0,Vol[_nZ-1],-4,k++,"V" );
  Graphic_for_Element( Srf,Tv.mY/2+30,0,Srf[_nZ-1], 4,k++,"S" );

  color( LIGHTGREEN );
  MinMax( xCW,_nZ,mn,mx );
  MinMax( xCV,_nZ,mn,mx,1 );                               //
  Graphic_for_Element(   0,Tv.mY/2+60,mn,mx,4,k,"xS,xC" ); // Абсциссы
  Graphic_for_Element( xCW,Tv.mY/2+60,mn,mx,0,k++, "xC" ); // величины
  Graphic_for_Element( xCV,Tv.mY/2+60,mn,mx,0,k+=2,"xS" ); // и площади
  Graphic_for_Element( Swl,Tv.mY/2+60,0,0, -4,k++,"Swl" ); //

  color( LIGHTMAGENTA );                                   //
  Graphic_for_Element( rfX,Tv.mY/2+90,0,0,-4,k++,"Jx" );   // Инерция
  Graphic_for_Element( Rfy,Tv.mY/2+90,0,0,+4,k++,"Jy" );   // ватерлинии
                                                           //
  for( i=1; i<_nZ; i++ )if( Vol[i] )rfX[i]/=Vol[i],
                                    Rfy[i]/=Vol[i]; Rfy[0]=Rfy[1];
  color( LIGHTRED );                                rfX[0]=rfX[1];
  MinMax( rfX,_nZ,mn,mx );
  Graphic_for_Element( rfX,Tv.mY/2+120,0,min(mx,Breadth ),-4,k++,"r" );
  MinMax( Rfy,_nZ,mn,mx );
  Graphic_for_Element( Rfy,Tv.mY/2+120,0,min(mx,Length*2),+4,k++,"R" );
  mX=0;
  color( WHITE );
  for( i=0; i<_nZ; i++ )
  { rfX[i]+=zCV[i]; if( fabs( i*dZ-Draught+Do )<mz ){ mX=rfX[i]; mz=fabs( i*dZ-Draught+Do ); };
  }
  Graphic_for_Element(   0,Tv.mY/2,Do,Height,4,k++,"Z,zC,zM",0 );
  Graphic_for_Element( zCV,Tv.mY/2,Do,Height,0,k++,"zC"     ,0 );
  Graphic_for_Element( rfX,Tv.mY/2,Do,Height,0,k++,"zM"     ,0 );
  color( DARKGRAY ); line( Do,0.0,Height,Real( _nZ-1 ) );
}
//   Расчет плеч статической остойчивости формы
//
void Hydrostatic::Stability()
{ int i,               // Индекс номера шпангоута
      j,               // Индекс угла накренения
      k,               // Индекс контрольной осадки (*cos)
      l,               // Индекс номера точки на контуре шпангоута
      m,               // Признак обратного прохода по шпангоуту (+-1)
      n,               // Признак определения первой точки на шпангоуте
      o;               // Признак обратного входа в контур шпангоута
 Real Z,A,dA,          // Ведущие аргументы
      V,dV;            // Объемные сумматоры
 Point f0,f1,f2,a,b,c, // Три путевые и три контрольные точки
       M,dC;           // Сумматор моментов и центр элементарной площадки
  A=dA=M_PI*mA/( _nA*180.0 ); // Шаг по углу накренения
  TvClip();
  for( j=0; j<_nA; j++,A+=dA )
  for( k=0; k<_nZ; k++ )        //
  { V=0;                        // Подбор наклоненного прямоугольника
    M=0;                        //
    if( !k ){ Z=Breadth*sin( -A )/2; if( A>M_PI_2 )Z+=Height*cos( A ); }
       else { Z+=( fabs( Height*cos( A ) )+Breadth*sin( A ) )/(_nZ-1); }
                                //
    for( i=0; i<_nX; i++ )      // Перебор шпангоутов
    { f0.x=0.0;                 // Первая точка всегда = 0
      f0.y=Y[0][i];             //
      f1=f0*=polar( A );
      if( Z-f1.x>=0 ){ a=c=f1; n=1; } else n=0;
           o=0;                 //
      for( l=m=1; l>=-1; )      // Контур одного шпангоута
      { if( l<0 )f2=f0; else    //
        { f2.x=l*dZ;            //
          f2.y=m*Y[l][i];       // Строится обход против
          f2*=polar( A );       //        часовой стрелки
        }                       //
        if( l<0 || Z-f1.x>=0 || Z-f2.x>=0 )
        do
        { if( l<0 && Z-f1.x<0 && Z-f2.x<0 )b=c; else
          { b=f2;
            if( Z-f2.x<0 ){ b.x=Z; b.y=f1.y+(Z-f1.x)*(f2.y-f1.y)/(f2.x-f1.x); }
            if( Z-f1.x<0 )
            { if( !o ){ o=1; b.x=Z; b.y=f2.y+(Z-f2.x)*(f2.y-f1.y)/(f2.x-f1.x); }
                 else   o=0;
          } }
          if( !n ){ c=b; n=1; } else    //
          { dV=( a.y*b.x-b.y*a.x )/2;   // Элементарная площадь
            dC=( a+b )/3;               // Центр элементарной площади
            M+=dC*dV;                   //
            V+=dV;
          } a=b;
        } while( o );
        f1=f2;
        if( m>0 && l==_nZ-1 )m=-1; else l+=m; // Закрутка на другой борт
    } }                 //
    if( V>0 )           // Аппликаты и ординаты
      aC[j][k]=M/V;     //  новых центров величины
      aV[j][k]=V*dX;    // Массив накренютых водоизмещений
    //
    // Предварительная прорисовка результатов в левом нижнем поле экрана
    //
    if( j>0 )
    { color( DARKGRAY ); line( A-dA,aC[j-1][k].x,A,aC[j][k].x );
      color( BLUE     ); line( A-dA,aC[j-1][k].y,A,aC[j][k].y );
  } } TvUnClip();
//
//      Процедура пересчета всех массивов к равнообъёмным накренениям
//
 Real H;
  Swl[0]=0;            A=dA; Amin=-0.1; Amax=0.1; // 10 см
  for( j=0; j<_nA; j++,A+=dA )
  { for( k=1; k<_nZ; k++ )                      //
    { V=Vol[k];                                 // Поиск аргумента-осадки
      for( i=1; aV[j][i]<V && i<_nZ-1; )i++;    // по грузовому размеру
      Swl[k]=i-(aV[j][i]-V)/(aV[j][i]-aV[j][i-1]); // -индексная метка осадки
    }
    for( k=0; k<_nZ; k++ )                      //
    if( k*dZ<Draught*0.25 || k*dZ>Height*0.875 )aV[j][k]=0; else
    { i=int( Swl[k] );
      if( i<0 )i=0; else
      if( i>=_nZ-1 )i=_nZ-2;
      dV = Swl[k]-i;
      dC = dV*aC[j][i+1]+(1-dV)*aC[j][i];

//    dC.x=dV*aC[j][i+1].x+(1-dV)*aC[j][i].x;
//    dC.y=dV*aC[j][i+1].y+(1-dV)*aC[j][i].y;

      H=aV[j][k]=dC.y-(mX-Do-0.5)*sin( A );     // От центра тяжести - полметра
//    H=aV[j][k]=dC.y-(rfX[k]-Do-0.5)*sin( A ); // От метацентра - полметра
//    H=aV[j][k]=dC.y-(zCV[k]-Do)*sin( A );     // От центра величины
//    H=aV[j][k]=dC.y-k*dZ*sin( A );            // От ватерлинии
      if( Amin>H )Amin=H;
      if( Amax<H )Amax=H;
    }
  }
}
//
//      Простая прорисовка осей для диаграммы Рида
//
static void Axis_Statics( _Real A, _Real Mn, _Real Mx, bool Vert=true )
{ int k;
 Real dA,z,dZ;
  if( Mx-Mn<0.01 )return;                               // < 1.0 см
  if( Vert )
  { color( BLUE      ); line( Real( 1 ),Mn,Real( 1 ),Mx );
    color( LIGHTGRAY ); line( Real( 0 ),Mn,Real( 0 ),Mx );
  } color( LIGHTGRAY ); line( Real( 0 ),Real( 0 ),A,Real( 0 ) );
  dZ=Tv_step( (Mx-Mn)/2 );
  dA=A/36;
  for( k=0; k<=36; k++ )
  { color( LIGHTGRAY ); line( Tv_x( k*dA ),Tv_y(0),Tv_x(k*dA),Tv_y(0)+(k&1?4:8) );
    color( WHITE );
    if( !(k%6) )Tv.Text( South,Tv_x(k*dA),Tv_y(0)+8,_Fmt( "%i",k*5 ) );
  }
  if( !Vert )return;
  for( k=Mn/dZ,z=k*dZ; z<=Mx && k<24; k++,z+=dZ )
  { color( LIGHTGRAY ); line( !(k%5)?3:6,Tv_y( z ),12,Tv_y( z ) );
    color( WHITE );
    if( k && !(k&1) )Tv.Text( North_East,0,Tv_y(z),_Fmt( "%.3lg",z ) );
  }
  Tv.Text( South_East,20,Tv.mY/2+12,W2D( "Остойчивость: Lф [м] " ) );
  Tv.Text( South_East,32,Tv.mY/2+24,_Fmt("Zg = Zm(kwl)-0.5 = %4.2lf",mX-0.5)); //_Fmt( "Zg = Zc [i]   %lg < %lg ",Amin,Amax ) );
}
void Hull_Statics()
{ Ghelp();
  setactivepage( 1 );   // Работа с графическим изображением
  setvisualpage( 1 );   // кривых элементов теоретического чертежа,
 int j,k;               // статической остойчивости и масштабом Бонжана
 Real A,dA,z,dz;
 Field _F;
 field f,_f;
  Hydrostatic LD;
  LD.Initial();
  clear();
 _F.Lx=Tv.mX*Height/Tv.mY;  _F.Ly=Height;
 _F.Jx=-_F.Lx/2;            _F.Jy=Do;
  f.Jx=0;  f.Lx=50;  f.wb=0;
  f.Jy=50; f.Ly=50; _f=f; Tv_place( &_f ); Tv_place( 0,&_F ); TvClip();
                                           Draw_Hull( 0 );  TvUnClip();
  f.Lx=0;  f.Jx=50; _f=f; Tv_place( &_f ); color( YELLOW );
  Tv.Text( North_West,Tv.mX/2-3,Tv.mY/2-1,W2D( "КЭТЧ" ) );
//
//      Разметка осей
//
 _F.Jy=_F.Jx=Do;
 _F.Lx=_F.Ly=Height; Tv_place( 0,&_F );
  color( WHITE );    Tv.Text( North_West,Tv.mX-3,Tv.mY/2-1,W2D( "[м]" ) );
                     Tv.Text( North,Tv_x( Draught ),Tv.mY/2-1,W2D( "ВЛ" ) );
  k=Tv_x( 0 );       line( k,Tv.mY/2,k,0 );
//
//      Прорисовка кривых элементов теоретического чертежа
//
  LD.Graphics();
  A=M_PI*mA/180.0;
  _F.Jx=0.0;     _F.Lx=A;
  _F.Jy=-Height; _F.Ly=Height*2; f.Jx=-12,f.Jy=-16; f.wb=0; // =-40
                                 f.Lx=-Tv.mX/2-12;
                                 f.Ly=-Tv.mY/2-20;  Tv_place( 0,&_F );
                                                    Tv_place( &f );
  Axis_Statics( A,-Height,Height*2,false );
//
//      Расчет плеч статической остойчивости формы
//
  LD.Stability();
  pattern( Tv.BkColor );
  _F.Jx=0.0; _F.Lx=M_PI*mA/180.0; bar( 0,Tv.mY/2+2,Tv.mX/2-2,Tv.mY );
#if 0
  _F.Jy=Amin; _F.Ly=Amax-Amin; Tv_place( 0,&_F ); dA=A/_nA; color( WHITE );
  for( k=1; k<_nZ; k++ )
  { moveto( Real(0),Real(0) );
    for( j=0,A=dA; j<_nA; j++,A+=dA )
    { lineto( A,LD.aV[j][k] );
    }
  }
#else
  _F.Jy=Amin; _F.Ly=Amax-Amin; Tv_place( 0,&_F );
  Axis_Statics( A,Amin,Amax );
  dA=A/_nA;
  dz=LD.dZ;
  z=Draught/5-Do;
  for( k=int( z/dz ); k<int(5*Height/Draught)*(Draught/5)/dz; k++ )
  { if( fabs( k*dz-z )<dz )
    {               color( fabs( k*dz-Draught+Do )<dz?YELLOW:
                     k*dz>Draught-Do?LIGHTCYAN:LIGHTGREEN );
      z+=Draught/5; setlinestyle( SOLID_LINE,0,NORM_WIDTH );
    } else          setlinestyle( DOTTED_LINE,0,NORM_WIDTH ),
                    color( k*LD.dZ-Do>Draught?CYAN:GREEN );
                         A=dA;  moveto( Real(0),Real(0) );
    for( j=0; j<_nA; j++,A+=dA )lineto( A,LD.aV[j][k] );
    if( k*dz>=Height-Draught/5 )break;
  }
#endif
  setlinestyle( SOLID_LINE,0,NORM_WIDTH );

  Axis_Statics( M_PI*mA/180.0,Amin,Amax );

  Thelp( "%ld Мб ",coreleft()>>20 );
  while( Tv_getc()==_MouseMoved );

  Tv_place( 0,&_F );
  setactivepage( 0 );
  setvisualpage( 0 );
}
#if 0
if( k==Nz/2 )
//if( j==6 ) //&& j&1 )
{ color( Z-f2.x<0?LIGHTMAGENTA: (Z-f1.x<0?LIGHTBLUE:LIGHTRED ));
  line( (f1+Do)*polar( M_PI_2 ),(f2+Do)*polar( M_PI_2 ) ); //Tv_getc();
  color( j&1?LIGHTCYAN:YELLOW );
  line( (a+Do)*polar( M_PI_2 ),(b+Do)*polar( M_PI_2 ) ); //Tv_getc();
}
if( k==_nZ/2 )
{ Gheld( "Z[%i]=%lg, A[%i]=%lg, V=%lg, M={%lg,%lg}",k,Z,j,A,V*dX,aC[j][k].x,aC[j][k].y );
//  Tv_getc();
}
#endif

// "K_Wave.cpp"
//                                              /05.03/  85.04.02
//  Расчет волнового сопротивления тонкого судна по формулам Мичелла.
//
#define _fmt "L=%.3lg, B=%.3lg, S=%lg, V=%lg, d=%.3lg, R=%.3lg, C=%.3lg\n"
static
Real WaveRes( Real **Hull,   // Корпус
              Real Froud,    // Число Фруда
              Real Length,   // Длина
              Real Depth,    // Осадка
              int Nx, int Nz // Индексы по длине и по осадке
            );
static void line_x( int x1, int y1, int x2, int y2 )
{ int c=color( LIGHTGRAY );
  setlinestyle( DOTTED_LINE,0,NORM_WIDTH );
  setwritemode( XOR_PUT );  line( x1,y1,x2,y2 );
  setwritemode( COPY_PUT );
  setlinestyle( SOLID_LINE,0,NORM_WIDTH );
  color( c );
}
void Hull_Wave( const int Type )
{
 const int   Nr=50;
 const Real  mFn=1.0,            // Предельное число Фруда
             Ro=1.025,           // кг/дм.куб
              g=9.8106;          // м/сек.кв
 int   i,x,y,c=color( CYAN );
 Real  V,Rm=0,R[Nr],Fn,w=0,
       W,Cm=0,C[Nr],   v=0;
 Window WR;                      // Окно корпуса
 Plaze  PL( 32,63 );             // Рабочий корпус для ускоренных расчетов
        PL.build( Draught );
        V=PL.V/( Lwl*Draught*Bwl ); // Коэффициент общей полноты
  Ghelp();
  setactivepage( 1 ); // Работа с графическим изображением
  setvisualpage( 1 ); // кривых элементов теоретического чертежа,
  clear();            // статической остойчивости и масштабом Бонжана
//
//      Предварительная проработка изображения
//
 field f;      f.Jy=2; f.Ly=24; f.Jx=f.Lx=f.wb=0;
  WR.Set( f ); f.Jx=i=int( (Tv_port.bottom-Tv_port.top)*Breadth/(Draught-Do)-Tv.mX );
  WR.Set( f ); PL.drawlines();    //
                                  // Подготовка рабочего окна
  WR.F.Jx=mFn/Nr;      WR.F.Jy=0; //
  WR.F.Lx=mFn-WR.F.Jx; WR.F.Ly=2; // Экстремумы кривой сопротивления
  f.Jx=0; f.Lx=-2-i-Tv.mX;        // Окно предварительного рисования
  WR.Set( f );                    //
  WR.Focus();
  Tv_rect();
  Rm=0;
  for( w=0.1; w<mFn; w+=0.1 )line( w,0.0,w,0.125 );
  color( LIGHTCYAN ); w=0.5; line( w,0.0,w,0.25 );
  PL.DrawWaves();
                              //
  for( v=w=0,i=0; i<Nr; i++ ) // Расчет кривых волнового сопротивления
  { Fn=(i+1)*mFn/Nr;          //
    if( !Type )W=PL.Michell( Fn ); //Michell( PL.Y,Fn,Lwl,Draught,PL.Nx,PL.Nz );
         else  W=WaveRes( PL.Y,Fn,Lwl,Draught,PL.Nx,PL.Nz );
    R[i]=W/PL.V;
    C[i]=W*2/Lwl /Fn/Fn /Ro/g /PL.S;
    color( LIGHTGRAY );
    line( Fn,R[i],Fn-mFn/Nr,w );       w=R[i];
    line( Fn,0.0,Fn-mFn/Nr,0.0 ); if( Rm<R[i] )Rm=R[i];
    color( GREEN );
    line( Fn,C[i]*100,Fn-mFn/Nr,v );   v=C[i]*100;
    line( Fn,0.0,Fn-mFn/Nr,0.0 ); if( Cm<C[i] )Cm=C[i];
  }
  PL.DrawWaves();
  //
  // График удельного сопротивления
  //
  color( CYAN );
  WR.F.Ly=Rm;    Tv_bar();
  WR.Focus();    Tv_rect();
  for( w=0.1; w<mFn; w+=0.1 )
  { line( w,0.0,w,Rm/6 );
    Tv.Text( North_East,w+0.006,Rm/50,_Fmt( "%0.1lf",w ) );
  }
  color( LIGHTCYAN ); w=0.5; line( w,0.0,w,Rm/3 );
  color( WHITE );                         moveto( Real( 0.0 ),0.0 );
  for( i=0; i<Nr; i++ ){ Fn=(i+1)*mFn/Nr; lineto( Fn,R[i] ); }
  Tv.Text( South_East,mFn/36,Rm*0.88,_Fmt( "R/D=%.3lg",Rm ) );
  //
  // График коэффициента волнового сопротивления
  //
  WR.F.Ly=Cm;
  WR.Focus();
  color( LIGHTGREEN );                    moveto( Real( 0.0 ),0.0 );
  for( i=0; i<Nr; i++ ){ Fn=(i+1)*mFn/Nr; lineto( Fn,C[i] ); }
  Tv.Text( South_East,mFn/36,Cm*0.80,_Fmt( "Cw=%.3lg",Cm ) );
  color( YELLOW );
  Tv.Text( South_East,mFn/36,Cm*0.96,Type?"WaveRes":"Michell" );
 Course Ans;
 int h=Tv.Th; Tv.Th=12;
  for(;;)
  { Tv_GetCursor( x,y );
    if( WR.Is( x,y ) )
    { w=Tv_X( x );
      i=min( max( 0,int( Nr/mFn*w-0.5 )),Nr-1 );
    //Ghelp( "Fn[%d]=%.2lg, Rw/D=%.3lg, Cw=%.3lg",i,w,R[i],C[i] );
      Tgo( 0,0 );
      Tprintf( "Fn[%d]=%.2lg, Rw/D=%.3lg, Cw=%.3lg\n",i,w,R[i],C[i] );
      line_x( x,Tv_port.bottom,x,Tv_port.top ); Ans=Tv_getc();
      line_x( x,Tv_port.bottom,x,Tv_port.top );
      if( Ans==_MousePressed )PL.DrawWaves( w );
    } else
    { Ans=Tv_getc();
      if( i )
      { i=0; //Ghelp( "L=%.3lg, B=%.3lg, S=%lg, V=%lg, d=%.3lg, R=%.3lg, C=%.3gl. F2-запись",Lwl,Bwl,PL.S,PL.V,V,Rm,Cm );
        Tgo( 0,0 );
        Tprintf( _fmt,Lwl,Bwl,PL.S,PL.V,V,Rm,Cm );
    } }              //
    if( Ans==0xF2 ) // Запись результатов расчета волнового сопротивления
    { static int n=1;
      static char Nm[13]; FILE *Om; sprintf( Nm,"~WR_%04d.txt",n++ );
      if( (Om=fopen( Nm,"wt" ))!=NULL )
      { fprintf( Om,_fmt,Lwl,Bwl,PL.S,PL.V,V,Rm,Cm );
        for( i=0; i<Nr; i++ )
        fprintf( Om,"%6.3lg %.4lg %.4lg\n",(i+1)*mFn/Nr,R[i],C[i] );
        fclose( Om );
      }
    }
    if( Ans==_Esc )break;
  }
  Tv.Th=h;
  color( c );
  setactivepage( 0 );
  setvisualpage( 0 );
}
//      Прорисовка профиля корабельной волны
//
void Plaze::DrawWaves( Real Fn )
{
 int j,k,c=color( BLUE );
 field f={ 0,Tv_port.top-Tv.mY-3,0,0 };
 Field F={ 0,-Breadth/2,Nx-1,Breadth };
       f.Ly=int( Tv.mX*Breadth/Length )-Tv_port.top+3;
  Tv_place( 0,&F );
  Tv_place( &f );
  if( !(Tv_getk()&(SHIFT|CTRL)) )Tv_bar( 0,0,Tv.mX,Tv_y( 0 ) );

 static Real W=1.0;                    //
  if( Fn>0 )                           // Волновая линия
  { Real V;                            //
    Michell( Fn );
    if( !(Tv_getk()&(SHIFT|CTRL)) )
    { W=0.0;
      for( k=0; k<Nx; k++ )if( W<Wx[k] )W=Wx[k];
    }    V=Tv_Y( 0 );
    for( k=0; k<Nx; k++ )Wx[k]*=V/W;
    color( LIGHTCYAN );  moveto( Real( 0 ),Wx[0] );
    for( k=0; k<Nx; k++ )lineto( Real( k ),Wx[k] );
  } else                              //
  { Tv_bar();                         // Ватерлинии
    Tv_rect();                        //
    for( k=1; k<Nx; k++ )line( Real( k ),0.0,Real( k ),-Bwl/2 );
                                      color( GREEN );
    for( j=Nz-1; j>=0; j-- )
    { if( !j )color( YELLOW );
                           moveto( Real( 0 ), -Y[j][0] );
      for( k=1; k<Nx; k++ )lineto( Real( k ), -Y[j][k] );
    }                      line( Real( 0 ),0.0,Real( Nx-1 ),0.0 );
  }
  color( LIGHTBLUE );  moveto( Real( 0 ),Y[0][0] );
  for( k=1; k<Nx; k++ )lineto( Real( k ),Y[0][k] );
  color( c );
  Tv_place( &f );
  Tv_place( 0,&F );
}
//      Прорисовка расчетного корпуса
//        основанного на таблице плазовых ординат
//
void Plaze::drawlines()
{  int i,k,c;
 Field F;               F.Jx=Breadth/2,F.Jy=0,F.Lx=-Breadth,F.Ly=Nz;
  Tv_place( 0,&F );     //
  c=color( CYAN );      // Окно корпуса
  Tv_rect();            //
  color( LIGHTGRAY ); line( (Real)0,0.0,0.0,(Real)Nz );
  for( i=0; i<Nx; i++ )                         // Построение картинки
  { color( !(i%(Nx-1)) ? LIGHTRED:LIGHTGRAY );
    for( k=0; k<Nz; k++ )
    if( !k )moveto( i<Nx/2?Y[0][i]:-Y[0][i],0.0 );
       else lineto( i<Nx/2?Y[k][i]:-Y[k][i],Real( k ) );
  } color( c );
}
//                                                 /05.03/  85.04.02
//  Расчет волнового сопротивления тонкого судна по формулам Мичелла
//
const double g=9.8106, // Ускорение свободного падения
             ro=1.025; //  и плотность морской воды

static int Nx,Nz;
static double Tl,Fr, Hx, R; //Xl,Xu;

static void AtaBee( double *_Y, double &Ac, double &As )
{ double dX,X,Y;                                     //
   Ac=As=0; dX=2.0/double( Nx-1 );                   // dX=(Ac+2)/(As+Nx);
   for( int j=0; j<Nx; j++ )                         // J=NA+1 TO NS-1;
   { X=R*(dX*(Nx-1-j)-1.0); Y=_Y[j]; Ac+=cos( X )*Y; // [ -1.0 -:- +1.0 ]
                                     As+=sin( X )*Y; //
   } X=(1.0-cos( R*dX ))/( R*R*dX )/(Nz-1); Ac*=X;   // = 2*FFF/Nx/2;
                                            As*=X;   //
}                                       //
static double **Y;                      // Матрица ординат корпуса
                                        //
static double AtaBlue( const double &Al )
{
 double C1,C2,Fw,Sumci=0,W=0,A,
        S1,S2,D2,Sumsi=0,P; Fw=Fr*Fr; R=Al/Fw/2; A=Al*Al; P=Tl*A/Fw/(Nz-1);
  AtaBee( Y[Nz-1],C1,S1 );      //
  for( int k=Nz-2; k>=0; k-- )  // I=KU+1 TO KD
  { W+=P;                       //
    if( W>64 )D2=0; else { D2=exp( -W ); AtaBee( Y[k],C2,S2 ); }
    W=D2*C2; Sumci += C1+W; C1=W;
    W=D2*S2; Sumsi += S1+W; S1=W;
  }
  Fw=Sumci*Sumci+Sumsi*Sumsi; return Hx<=0 ? Fw:A*Fw/sqrt( A-1 );
}
// Простое интегрирование аналитически определенной функции
//
static double QG4( const double &Xl, const double &Xu,
                   double F( const double& ) )
{ double A = 0.5*( Xu+Xl ),B=Xu-Xl,C,D;
         C = .4305681557970263 * B;
         D = .1739274225687269 * ( F( A+C )+F( A-C ) );
         C = .1699905217924281 * B;
  return B * (D+.3260725774312731 * ( F( A+C )+F( A-C )));
}                               //
static
Real WaveRes( Real **_Hull,     // Корпус
              Real Froud,       // Число Фруда
              Real Length,      // Длина
              Real Depth,       // Осадка
               int _Nx,         // Индексы по длине
               int _Nz )        //  и по осадке
{ double Wres=0,W,Xl,Xu;        //
   Tl=Depth/Length;
   Y=_Hull;  Nx=_Nx;
   Fr=Froud; Nz=_Nz;
   Hx=-1;
   Wres=AtaBlue( 1 )*0.0106 + AtaBlue( Xl=1.0001 )*0.00353;
   Hx=3.33*Fr*Fr;
   W=1.9-Fr*3;
   W=W*W+3;
   for( int i=0; i<3; i++,Hx*=3 )       // DO I=1,3;
   for( Real j=1.0; j<=W; j++  )        // DO J=1 TO 3+(1.9-FROUD*3)**2;
   {                 Xu=Xl+Hx;
     Wres += QG4( Xl,Xu,AtaBlue );
                  Xl=Xu;
   } return 2*Wres*ro*g*pow( Depth/Froud/Froud/Froud,2 )/Length/M_PI;
}
#if 0
    FROUD= V/SQRT(G*AL);                        TL=T/AL;
    WRES = WAVERES((FI),(FROUD),(TL));
    TL   = WRES*RO*G*T*T*B*B/FROUD**6/AL/PI/4;
    WRES = WRES*T*T*B*B/FROUD**8/AL**2/OMEGA/PI/2;
    PUT SKIP LIST('  FROUD='!!FROUD!!'  R/W='!!TL/DOUBLE!!
                '  R/S='!!TL/OMEGA!!' *** TIME='!!TIME!!' *** '!!L);
#endif
                        //
static double WL;       // Длина поперечной волны
                        //
double Plaze::Amplint( const double &La )
{   int i,j;                  //
 double X,Z,Wk=2.0*M_PI/La,   // Волновое число
            Mw=sqrt( WL/La ); // Число Маха
 Point A={ 0,0 },W;           // Интегральная амплитуда
  for( i=Nx-1; i>=0; i--  )   //
  { Z=0;
    W=0;
    X=Wk*dX*(Nx-1-i)/Mw;
    for( j=Nz-1; j>=0; j-- )
    { W += exp( (Point){ -Z,X } )*QV[j][i];
      Z += Wk*dZ;
    } A += W;
     Wx[i]=norm( A )*Mw/sqrt( Mw*Mw-1 )/La/La;
  } return norm( A )*Mw/sqrt( Mw*Mw-1 )/La/La;
}
double Plaze::QG4( const double &Xl, const double &Xu )
{ double A = 0.5*( Xu+Xl ),B=Xu-Xl,C,D;
         C = .4305681557970263 * B;
         D = .1739274225687269 * ( Amplint( A+C )+Amplint( A-C ) );
         C = .1699905217924281 * B;
  return B * (D+.3260725774312731 * ( Amplint( A+C )+Amplint( A-C )));
}
Real Plaze::Michell( Real Fn )
{   int i;      //
 double Rw=0.0, // Искомое волновое сопротивление
          Vo,   // Скорость в м/с
           l;   // Аргумент интегрирования
                //
  Vo = Fn*sqrt( g*Lwl );                //
  WL = 2.0*M_PI * Vo*Vo/g;              // Макс. длина волны
#if 0                                   //
  for( j=0; j<Nz;   j++ )
  for( i=0; i<Nx-1; i++ )               //
  { QV[j][i]=( Y[j][i+1]-Y[j][i] )*dZ;  // Распределеные источники
//  QV[j][i] = dX*dZ*X; //*Wide;        // с коэффициентом наклона
//  YY[j][i] =( Y[j][i]+Y[j][i+1] )/2.0;
//  if( j>Nz ) //>0 )
//  { Z = ( Y[j][i]-Y[j-1][i] )/Dz;
//    QV[j][i] *= sqrt( 1+ X*X+ Z*Z );  //= sqrt( 1+ X*X );
//  }
  }
#endif
                                        //
 const int la=24;                       // Количество дроблений по волнам
 static                                 //
 double La[la+1]={ 0.0 };               // Фазовая длина волны
       l=double( 1.0 )/la;              //
  for( i=1; i<=la; i++ )La[i]+=La[i-1]+sin( M_PI_2*l*i )*sqrt( 1.0-l*i );
  for( i=1; i<=la; i++ )La[i]*=WL/La[la];
  for( i=1; i<=la; i++ )if( La[i]>dX )
                        Rw+=QG4( La[i-1],La[i] );
 return 4*g*ro*Rw;
}
#if 0
struct Michell{ Real Vo;        // Скорость в м/с
                Real Vl;        // Водоизмещение
                Real Sf;        // Смоченная поверхность
                Real Rw;        // Сопротивление
                Real Cw;        // Коэффициент сопротивления
} Michell_Results={ 0,0,0,0 };  //
  Michell_Results.Vo=Vo;        // Скорость в м/с
  Michell_Results.Vl=Vl;        // Водоизмещение
  Michell_Results.Sf=Sf;        // Смоченная поверхность
  Michell_Results.Rw=Rw;        // Сопротивление
  Michell_Results.Cw=Cw;        // Коэффициент сопротивления

  for( j=0; j<Nz; j++ )                         //
  { Z = Dz*Wk*j;        if( Z>69 )continue;     //
    X = Dx/2;           // -0.5*Length;         // От носа к корме
    for( i=0; i<Nx-1; i++)                      //
    { A += QV[j][i] * exp( complex( -Z,Wk*X/Mw ) );
      X += Dx;
  } }

#endif
