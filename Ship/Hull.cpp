//
//    Программа рисования теоретического чертежа корпуса судна,
//      ++ расчетов гидростатических кривых и статической остойчивости
//      ++ волнового сопротивления по Шебалову-Атанасову, Готман и др.
//
//    В главной процедуре сосредоточены консольные запросы
//      и информационные материалы
//                                      ©1975-2022.01.27, В.Н.Храмушин, Сахалин
//                                      лаб. вычислительной гидромеханики СахГУ
//                                      каф. компьютерного моделирования СПбГУ
#include <fenv.h>
#include "Hull.h"

fixed Active=0;                   // 0x01 файл считанн и подготовлен к работе
                                  // 0x02 полностью выполнена первая прорисовка
unsigned Hull_Keys=0x00;          // Набор ключей - параметров
                                  // 0x01 - задействовать сплайн-интерполяцию
Real Xo=0,Xm=1,Length=2,Lmx=2,Lwl=2,   // Длина и длина по ватерлинии
          Breadth=0.2,Bmx=0.2,Bwl=0.2, // Ширина и ширина по ватерлинии
     Do=0,Draught=0.1,Depth=0.2,       // Осадка и высота борта
          Volume=0.4,                  //  Водоизмещение
          Surface=0.2;                 //   Площадь смоченной поверхности
const char
 *_Mnt[]={ "январь","февраль","март","апрель","май","июнь","июль","август","сентябрь","октябрь","ноябрь","декабрь" },
 *_Day[]={ "понедельник","вторник","среда","четверг","пятница","суббота","воскресенье" };
//
//   Программа построения корпусов судов и расчетов волнового сопротивления
//  ------------------------------------------------------------------------
MainDraw Win;
MainDraw::MainDraw():
          Window( " Теория и мореходные качества корабля",-12,12,500,160 ){}
                                               // контрольная высота буквочек
Plane wH( "Корпус",     "Y","Z",&Win ),        // Окно проекций: корпус,
      wM( "Бок",        "X","Z",&Win ),        //   бок
      wW( "Полуширота", "X","Y",&Win );        //    и полуширота
static Place MPL( &Win,PlaceAbove ),           // -- позиция и координаты мышки
             TPL( &Win,PlaceAbove );           // -- параллельное время таймера
static void Hull_Help()
{ const char *Name[]={ " Hull   ","Обводы, гидростатика",
                                  " и остойчивость корабля",
                                  " + волновое сопротивление",0 },
             *Cmds[]={ "F1    ","краткая справка",
                       "F2/F3 "," запись/считывание корпуса",
                       "F4   ","Математическая аналитика",
                       "F5   ","Гидростатика + остойчивость",
                       "Волнообразование и ходкость",":",
                       "F7   ","остаточное от Ады Ш.Готман",
                       "F8   ","по «Штормовой мореходности»",   // «»
                       "F9   ","из Корабелки + А.Н.Шебалов",0 },
             *Plus[]={ "<Enter> "," масштаб по ширине окна",
                       "<Space> "," шпангоуты сплайн\\линии",
                       "<Esc>/<ctrlC>","   завершение",0
                     }; Win.Help( Name,Cmds,Plus,0,1 );
}
void Hull_Wave( const int Type );  // волнообразование и волновое сопротивление
void Hull_Statics();  // Остойчивость и кривые элементов теоретического чертежа

Plane::Plane( const char *i, const char *x, const char *z, Window *Wm ):
       Place( Wm,0 ),iD( i ),sX( x ),sZ( z ),W( Wm->Width ),H( Wm->Height )
       { ax=az=0; uX=ux=Width;
         aX=aZ=0.0; uZ=uz=Height; dx=dz=1.0; } // Signs=0;
Plane& Plane::Focus(){ glMatrixMode( GL_PROJECTION ); glLoadIdentity();
         glOrtho( aX-ax*dx, uX+( W-ux )*dx,        // left, right
                  aZ-az*dz, uZ+( H-uz )*dz,0,1 ); // bottom, top
         glMatrixMode( GL_MODELVIEW ); glLoadIdentity(); return *this; }
Plane& Plane::Set(_Real X,_Real Z,_Real _X,_Real _Z )// отступ от правой-нижней
       { aX=X,uX=_X; dx=( uX-aX )/( ux-ax );       // границ и размерения новой
         aZ=Z,uZ=_Z; dz=( uZ-aZ )/( uz-az );       // площадки - отрицательны
         Area( ux-W,uz-H,ax-ux,az-uz );            // правый\нижний и размер
         Focus(); return *this;
       }
//     Экранные и физические координаты отмеряются от заданного прямоугольника
//
bool Plane::Is( int x,int z ){ z=H-z; return x>ax&&x<ux&&z>az&&z<uz; }
Real Plane::X( int x ){ return aX + x*dx; }
Real Plane::Z( int z ){ return aZ + z*dz; }// Нормальные координаты во ViewPort
 int Plane::x( _Real X ){ return int( ( X-aX )/dx ); }
 int Plane::z( _Real Z ){ return int( ( Z-aZ )/dz ); }
//
// Координаты анти-относительно реальных отсчетов внутри активного окна Windows
//
Real Plane::wX( int x ){ return aX + (x-ax)*dx; }   // X( x-ax )
Real Plane::wZ( int z ){ return aZ + (H-z-az)*dz; } // Y( H-y-ay )
//int Plane::xw( _Real X ){ return x( X )+ax; }
//int Plane::zw( _Real Z ){ return H-z( Z )-az; } - ??
//
//static void Loft_Place( bool rShape=false )
void MainDraw::Loft( bool rShape )          // общая разметка графического поля
{ const int Up=12,Left=32,Down=120,Band=24,Bord=6;   // под текущую ширину окна
 int &TW=Width, mTW=ScreenWidth; if( TW!=500 && TW<=mTW )mTW=TW;
 int &TH=Height,mTH=ScreenHeight;// размеры графического экрана в целом
  if( rShape )TW=mTW,TH=mTH;
 Real X = TW-Up-Left-Bord*2.0,
      Y = TH-Up-Band-Down-Bord*2.0;
  if( rShape )
  { Real YX=(Depth-Do+Breadth/2)/Lmx;
     if( YX>Y/X ){ X=Y/YX; TW=int(X+Up+Left+Bord*2); } // Перенос и перерисовка
            else { Y=X*YX; TH=int(Y+Up+Band+Down+Bord*2); }// графического поля
//   AlfaVector( hText=TH/40 ).Activate().Clear().Show();
     AlfaVector( Up );
     Locate( mTW-TW,(mTH-TH)/3,TW,TH );
  }   wW.ax=wM.ax = Left+Bord;  wW.az = wW.uz = Down+Bord;
      wW.ux=wM.ux = TW-Up-Bord; wW.uz+= int( Y/(1.0+2.0*(Depth-Do)/Breadth) );
      wM.az=wH.az = wW.uz+Band; wW.Set( Xo,Breadth/2.0,Xo+Lmx,0.0 );
      wM.uz=wH.uz = TH-Up-Bord; wM.Set( Xo,Do,Xo+Lmx,Depth );
      wH.ax=wM.x( Kh.Ns>2?Kh.F[Kh.Ms].X:(Kh.F[0].X+Kh.F[1].X)/2 )+wM.ax;
      wH.ux=wW.uz-wW.az;
//if( wH.ux*3>TW )wH.ux=TW/3;   -- шар и куб теперь занимают всю проекцию бок
      wH.ax-=wH.ux;             // Здесь "корпус" вписывается на мидель в "бок"
      wH.ux=wH.ux*2+wH.ax;
      wH.Set( Breadth/-2,Do,Breadth/2,Depth );
      MPL.AlfaVector( 14,0 ).Area( 0,18,20,-28 );                  // это мышка
      TPL.AlfaVector( 14,0 ).Area( 0,1,50,-15 );                   //  и таймер
}
static bool Wid=false,              // перерисовка на полный или заданный экран
          First=false;              // блокировка при неготовности конструктора
bool MainDraw::Draw()               // Здесь также будет приветствие с повтором
{ if( !First )return false; First=false;   // блокировка рекурсии в прерываниях
  if( Active&true )Loft( Wid ); else       // Активный корпус просто размечается
  { while( Kh.Read() );             // При вызове без параметров будет Hull.vsl
      Loft( Active=true );          // Степановские, польские или новые корпуса
  }   //AlfaVector( hText=Height/36 );
      Building(); gl_BLUE;
      //AlfaVector( hText*1.2,1.2 );
      Print( 5,-5,"L = %.3g",Length );
  if( fabs( Length-Lmx )>Length/300 )Print( " \\ %.3g",Lmx );
  if( fabs( Length-Lwl )>Length/300 )Print( " / %.3g",Lwl );
      Print( 5,-4,"B = %.3g",Breadth );
  if( fabs(Breadth-Bmx)>Breadth/300 )Print( " \\ %.3g",Bmx );
  if( fabs(Breadth-Bwl)>Breadth/300 )Print( " / %.3g",Bwl );
      Print(      " { %d }",Kh.Ms );
      Print( 5,-3,"T = %.3g",Draught );
  if( fabs( Do )>Draught/300 )Print( " / %.3g",Do );
      Print( 5,-2,"V = %.5g  δ = %.3g",Volume,Volume/Bwl/Lwl/Draught );    // δ
      Print( 5,-1,"S = %.5g, %s",Surface,Hull_Keys&1?"spline":"vector" );
      Save().Show(); First=true; return false;
}
static bool Win_Timer() // Регулярное обновление картинки делается в вызывающей
{ if( Active&2 )        // процедуре обработки прерываний от таймера
  { Event T; int M,D,Y; T.UnPack( M,D,Y ); TPL.Activate().Clear(); gl_GREEN;
    TPL.Print( 0,0," %04d %s %02d, %s%-10s ",
                   Y,_Mnt[M-1],D,_Day[int( T.D%7 )],DtoA( T.T,3 ) ).Show();
  } return false;       // все уже на экране
}
//     Информационные процедуры для активного окна
//
//bool MainDraw::Mouse( int x,int y )//{ return false; }

static bool Mouse_in_Window( int x,int y )
{ if( Active&3==3 )                        // привязка к текущему окну
  { Plane *M; Win.Activate(); MPL.Clear(); // расчистка - плохо, а xor - сложно
    if( wH.Is( x,y ) )M=&wH; else          // если то верно,
    if( wW.Is( x,y ) )M=&wW; else          // то здесь активность
    if( wM.Is( x,y ) )M=&wM; else M=NULL;  // уходит на главное поле
    if( M ){ gl_LIGHTRED; MPL.Print( 1,1,"%s\n%s=%.2f, %s=%.2f ",
             M->iD,M->sX,M->wX(x),M->sZ,M->wZ(y) ); } MPL.Show();
  } return false;
}
void glInitial()
{ glShadeModel( GL_SMOOTH );
  glHint( GL_LINE_SMOOTH_HINT,GL_NICEST );  glEnable( GL_LINE_SMOOTH );
  glHint( GL_POINT_SMOOTH_HINT,GL_NICEST ); glEnable( GL_POINT_SMOOTH );
//glEnable( GL_DITHER );                  // Предопределение графической среды
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA );

//glDisable( GL_SCISSOR_TEST );
//glDisable( GL_SCISSOR_BOX );
//glDisable( GL_CLIP_PLANE1 );

}
int main() // int ans, char *argv[], char *envp[] )
{ feclearexcept( FE_ALL_EXCEPT );
  glClearColor( 0.85,0.95,0.97,1 );
  glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT ); // Уединенное приветствие
  glInitial();
  gl_BLUE; Win.AlfaVector( 24,2 ).Print( 2,1.25,"Корабль - " );
           Win.AlfaVector( 20,1 ).Print("морская гидромеханика\n" );
  gl_CYAN; Win.AlfaVector( 16,2 )
              .Print( "\n  Лаборатория вычислительной гидромеханики"
                      "\n                и морских исследований" );
  gl_LIGHTBLUE; Win.AlfaVector( 16,0 )
              .Print( "\n  РОССИЯ, Южно-Сахалинск - Санкт-Петербург\n\n" );
  gl_LIGHTRED; Win.AlfaVector( 13,1 ).Print( 2,-1.25,
                  "Вычислительные эксперименты по штормовой мореходности\n"
                  "Гидростатика, остойчивость, волновое сопротивление," );
  gl_YELLOW; Win.AlfaVector( 9,1 )
                .Print( -3,0,"©1975-24, Василий Храмушин" ).Show();
  WaitTime( 250 );
//
//      подборка управляющих параметров и структур с информацией
//
 const char *cU[4]={ " Кирпич:"," Эллипс:"," Шлюпка:"," Корпус:" };
 static Real cX=1.0,cY=1.0,cZ=1.0,cL=20.0,cB=10.0; // aФ, длина и ширина модели
 static int cM=1,cN=65;                            // индекс и количество точек
 static Mlist Menu_p[]={ { 1,0,"Аналитические обводы корпуса" } //  0
                       , { 2,28," 1 - на степенных функциях"  } //! 1
                       , { 1,28," 2 - синусно-показательных"  } //! 2
                    // , {  1,4," исправление модели штевней" }
                       , { 2,28," <<аффинная трансформация>>" } //! 3
                       , { 1,6, " X:%6.3lf",&cX }               //  4
                       , { 0,6, " Y:%6.3lf",&cY }               //  5
                       , { 0,6, " Z:%6.3lf",&cZ }               //  6
                       , { 2,28," << выбор простой модели >>" } //! 7
                       , { 1,9, cU[cM] }                        //  8
                       , { 0,3, " L=%-3.0lf",&cL }              //  9
                       , { 0,3, " B=%-3.0lf",&cB }              // 10
                       , { 0,3, " N=%-3d",   &cN }              // 11
                       , { 2,28," << обводы скуловых волн >>" } // 12
                       , { 1 } };                               // 13
//!   Собственно начало работы
//
  int ans; WCHAR **argv=CommandLineToArgvW( GetCommandLineW(),&ans );
  if( ans>1 )strcpy( Kh.Name,W2U( argv[1] ) ),
             fext( strcpy( Kh.Name,W2U( argv[1] ) ),"vsl" );
             else  strcpy( Kh.Name,"Hull.vsl" );
          Win.SetTimer( 500,Win_Timer );         // запуск таймера миллисекунды
          Win.Mouse( Mouse_in_Window );          // запуск указателя "мышка"
          First=true;
MainLoop: if( ans )Win.Draw(); Wid=false;        //  и - главный цикл запросов
  switch( ans=Win.WaitKey() )
  { case _F1: Hull_Help(); ans=0; break;
    case _F2: Kh.Write();  ans=0; break;
    case _F3: strcpy( Kh.Name,"*.vsl" ); while( Kh.Read() );
              Win.Loft( Active=true ); break;
    case _F4: // if( Win.ScanStatus()&ALT )exit( 0 );     // поочередный запрос
          { TextMenu T( Mlist( Menu_p ),&Win,2 ); cX=cY=cZ=1.0;
            while( (ans=T.Answer())>=3 )            // геометрического описания
            { if( ans==3  ){ Kh.Aphines( cX,cY,cZ ); break; }
              if( ans==7  ){ Kh.ModelEx( cL,cB,cN,cM ); break; }
              if( ans==12 ){ Kh.BilgeEx(); break; }   // скуловые обводы
              if( ans==8 )Menu_p[8].Msg=cU[++cM%=4];  // ключи выбора модели
              if( ans==_Esc )break;                   // аналитического корпуса
          } } if( ans==1 )ans=Set_Power(); else
              if( ans==2 )ans=Set_Sinus();               // ==> ans=Set_Stem();
              if( ans )Win.Loft( Active=true ); break;
    case _F5: Hull_Statics(); ans=0; break; // Гидростатика и остойчивость
    case _F7: Hull_Wave( 1 ); ans=0; break; // адаптированный к корпусу Мичелл
    case _F8: Hull_Wave( 2 ); ans=0; break; // контроль по Шебалову-Атанасову
    case _F9: Hull_Wave( 3 ); ans=0; break; // модель Ады Шоломовны Готман
    case _Blank:                            // 1 - сплайны 0 - прямые линии
     if( Hull_Keys^=1 )for( int i=0; i<Kh.Ns; i++ )Kh.F[i].SpLine();
                 else  for( int i=0; i<Kh.Ns; i++ )Kh.F[i].Easy(); break;
    case _Enter: Wid=true; break;           // безусловная перерисовка в окне
    case _Esc: Win.Close();
  }
  if( Win.Ready() ){ while( Win.GetKey() ); goto MainLoop; }
  return EXIT_SUCCESS;
}
