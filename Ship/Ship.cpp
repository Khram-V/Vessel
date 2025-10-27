
#include "Ship.h"
//
//    общие данные как бы в стиле Fortran-common блоков
//
Vector Min={0.0},Max={0.0};    // Экстремумы исходного графического изображения
byte UnderWaterColorAlpha=0xFF;// для алгоритмов с точным положением ватерлинии
Color UnderWaterColor={ 0xFFFFFF }; // ~~~ выведены в общий доступ ~~~
 Real Length=6.0,                   // длина
      Beam=2.0,                     // ширина
      Draft=1.0;                    // осадка
//
//   подготовленные в предыдущем расчёте теоретические контуры
//
void InterSection::Drawing( BoardView Sides )                  // mvPort,mvBoth
{ if( Sides==mvBoth )
  for( int j=0; j<NIt; j++ )
  if( T[j].NoSplines>0 )
  { glBegin( GL_LINE_STRIP ); //dot( ~(T[j].S[0].P) );
    for( int i=0; i<T[j].NoSplines; i++ )dot( ~(T[j].S[i].P) ); glEnd();
  }
  for( int j=0; j<NIt; j++ )
  if( T[j].NoSplines>0 )
  { glBegin( GL_LINE_STRIP ); //dot( T[j].S[0].P );
    for( int i=0; i<T[j].NoSplines; i++ )dot( T[j].S[i].P ); glEnd();
  }
}
//   Главная виртуальна процедура изображения всего корабля/проекта
//
bool FreeShip::Draw()               // виртуальная процедура с настройкой сцены
{ BoardView &B=Visio.ModelView; int i,k;  // glDepthRange( -Distance,0 ); // Distance );
                                          // glDepthFunc( GL_LESS ); // ~EQUAL~GEQUAL GREATER LEQUAL NOTEQUAL LESS ALWAYS взаимное накрытие объектов
  View::Draw();
  glEnable( GL_LIGHTING );                    // расцветка под теневые закраски
  glTranslated( (Max.x+Min.x)/-1.75,0,        // -Set.SplitSectionLocation
                (Max.z+Min.z)/-2 );           // Set.Length/-2
  Clear(); color( lightgray );                 glLineWidth( .2 );
  axis(*this,Length,Beam,Draft*2,"x","y","z"); glLineWidth( .5 );
  Shell.Drawing( B );                          glLineWidth( 1 );
  color( cyan );  for( k=0; k<NoStations;   k++ )Stations[k].Drawing( B );
  color( green ); for( k=0; k<NoWaterlines; k++ )Waterlines[k].Drawing( B );
  color( blue );  for( k=0; k<NoButtocks;   k++ )Buttocks[k].Drawing( B );
  color( olive ); for( k=0; k<NoDiagonals;  k++ )Diagonals[k].Drawing( B );
  color( lightgreen );                         glLineWidth( 2 );
  for( k=0; k<NoFlowLines; k++ ){ glBegin( GL_LINE_STRIP );
    for( i=0; i<FlowLines[k].len; i++ )dot( FlowLines[k][i] ); glEnd();
    if( B==mvBoth ){ glBegin( GL_LINE_STRIP );
      for( i=0; i<FlowLines[k].len; i++ )dot( ~FlowLines[k][i] ); glEnd();
  } }                                          glLineWidth( 1 );
  Window::Show(); return true;
}
#include <wctype.h>

inline WCHAR* Slower( WCHAR *str )
{ int l=wcslen( str ); while( --l>=0 )str[l]=towlower( str[l] ); return str;
}
//!                конструктор с расчисткой и считыванием новой числовой модели
Ship::Ship()
{ memset( this,0,sizeof( Ship ) );
  textcolor( YELLOW );              print( "Free!Ship [*.fbm,*.ftm,*.fef]\n" );
  int argc; WCHAR **argv=CommandLineToArgvW( GetCommandLineW(),&argc );
  if( argc>1 )FName=argv[1]; else
  { printf( "Start Ship <имя файла модели корпуса.[fbm|ftm|fef]>" );
                getch(); exit( 1 );
  }
  Name=strdup( W2U( FName ) ); // копия имени входного файла
  argc=wcslen( FName );
  Visio.ModelView=mvBoth;     // на оба борта, всё другое может быть вычищенным

//  print( "Открыт %s файл: %s\n",isBin?"двоичный":"текстовый",W2U(FileName)); textcolor(LIGHTBLUE);

  if( argc>4 && wcscmp( Slower( argv[1]+argc-4 ),L".fef" )==0 )
  { if( !LoadFEF() )
    { textcolor( LIGHTMAGENTA );
      printf( "?File Exchange Format %s не срабатывает",Name );
      getch(); exit( argc );
    }
  } else
  if( !LoadProject() )
  { textcolor( LIGHTMAGENTA );
    printf( "?Free!Ship неверный файл/incorrect datafile: %s ",Name );
    getch(); exit( argc );
  }
  textcolor( YELLOW );
  print( "\n Длина : [ %6.2f - %-6.2f ] = %g ",Min.x,Max.x,Max.x-Min.x );
  print( "\n Ширина: [ %6.2f - %-6.2f ] = %g ",Min.y,Max.y,(Max.y>-Min.y?Max.y:-Min.y)*2 );
  print( "\n Высота: [ %6.2f - %-6.2f ] = %g ",Min.z,Max.z,Max.z-Min.z );
}
//     Конструктор не содержит Matrix для движений корпуса в корабельном базисе
//
FreeShip::FreeShip():Ship(),View("Free!ship in C++ ",-12,12,640,480) //Matrix()
{ Icon( "Ship" ).AlfaVector( 16 );
  Locate( Xpm( 4 ),Ypm( 4 ),min( 1280L,Xpm( 64 ) ),
                            min( 1024L,Ypm( 72 ) ) );
//glCullFace( GL_FRONT_AND_BACK );                    // какие отбираются грани
  Distance=-1.75*( Max.x+Max.y-Min.x-Min.y + Width*(Max.z-Min.z)*0.9/Height );
  eyeX=135; // lookX=-60;
  glDisable( GL_FOG );
  Draw(); // начальная прорисовка
}
//   Интерактивная настройка/управление графическим отображением проекта
//
bool FreeShip::KeyBoard( fixed Keyb ){               // С краткой подсказкой
 const static char                                   // по настройкам и методам
     *Id[]={"Ship"," Корабельные форматы ",          // визуализации корпуса
                   " Free!Ship: *.ftm,*fbm.*fef..",0 },
     *Cmds[]={ " F1 "," - справка",
               " F2 "," формат «Aurora»: <имя>.vsl",0 },
     *Plus[]={ " <Space>            ","борт\\полборта",
               " <Ctrl+Space>       "," грани\\рёбра",
               " <стрелки·leftMouse>"," ориентация",
               " <+Shift·rightMouse>"," смещение",
               " <+Ctrl·(roll)>"," дальность, наклон",
               " <Home>  "," вместить корпус в экран",0 };
 BoardView &B=Visio.ModelView;
  switch( Keyb )
  { case _F1: Window::Help( Id,Cmds,Plus,1,1 ); break;
    case _F2: WriteVSL(); break;
    case _Blank:
      if( ScanStatus()&CTRL )
        { static bool xd=false;
          if( xd^=true )glPolygonMode( GL_FRONT_AND_BACK,GL_FILL ); //,glEnable( GL_FRONT_FACE ); //,glEnable( GL_CULL_FACE )
                  else  glPolygonMode( GL_FRONT,GL_FILL ),
                        glPolygonMode( GL_BACK,GL_LINE ); //,glDisable( GL_FRONT_FACE ); //,glDisable( GL_CULL_FACE )
        } else
      if( B==mvPort )B=mvBoth; else B=mvPort; break;
    default: return View::KeyBoard( Keyb );
  } return Draw();
}
int main() // int argc, char **argv )
{ texttitle( "FREE!Ship view in C++ " ); FreeShip Hull;            // заголовок
  do{ Hull.Title( DtoA( Real(ElapsedTime())/3600000.0,-3 ) ); WaitTime( 500 );
    } while( WinReady() && Hull.GetKey()!=_Esc ); _exit( 0 ); //return 0;
}


