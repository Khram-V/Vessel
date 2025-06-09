
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
  for( int j=0; j<NoItems; j++ )
  if( T[j].NoSplines>0 )
  { glBegin( GL_LINE_STRIP ); //dot( ~(T[j].S[0].P) );
    for( int i=0; i<T[j].NoSplines; i++ )dot( ~(T[j].S[i].P) ); glEnd();
  }
  for( int j=0; j<NoItems; j++ )
  if( T[j].NoSplines>0 )
  { glBegin( GL_LINE_STRIP ); //dot( T[j].S[0].P );
    for( int i=0; i<T[j].NoSplines; i++ )dot( T[j].S[i].P ); glEnd();
  }
}
//   Главная виртуальна процедура изображения всего корабля/проекта
//
bool FreeShip::Draw()               // виртуальная процедура с настройкой сцены
{ BoardView &B=Visio.ModelView; int i,k;
  View::Draw();
  glTranslated( (Max.x+Min.x)/-1.75,0,      // -Set.SplitSectionLocation
                (Max.z+Min.z)/-2 );         // Set.Length/-2
  Clear(); color( lightcyan );                 glLineWidth( .2 );
  axis(*this,Length,Beam,Draft*2,"x","y","z"); glLineWidth( .5 );
  Shell.Drawing( B );
  color( lightblue );  for( k=0; k<NoStations; k++ )Stations[k].Drawing( B );
  color( green ); for( k=0; k<NoWaterlines; k++ )Waterlines[k].Drawing( B );
  color( cyan );  for( k=0; k<NoButtocks; k++ )Buttocks[k].Drawing( B );
  color( olive ); for( k=0; k<NoDiagonals; k++ )Diagonals[k].Drawing( B );
  color( lightgreen );                                      glLineWidth( 2 );
  for( k=0; k<NoFlowLines; k++ ){ glBegin( GL_LINE_STRIP );
    for( i=0; i<FlowLines[k].len; i++ )dot( FlowLines[k][i] ); glEnd();
    if( B==mvBoth ){ glBegin( GL_LINE_STRIP );
      for( i=0; i<FlowLines[k].len; i++ )dot( ~FlowLines[k][i] ); glEnd();
  } }
  Window::Show(); return true;
}
#include <wctype.h>
inline WCHAR* Slower( WCHAR *str )
{ int l=wcslen( str ); while( --l>=0 )str[l]=towlower( str[l] ); return str;
}
Ship::Ship()   //! конструктор с расчисткой и считыванием новой числовой модели
{ memset( this,0,sizeof( Ship ) );
  textcolor( YELLOW );              print( "Free!Ship [*.fbm,*.ftm]\n" );
  int argc; WCHAR **argv=CommandLineToArgvW( GetCommandLineW(),&argc );
  if( argc<2 ){ printf( "Start Ship <имя файла модели корпуса.[ftm|fbm]>" ); getch(); exit( 1 ); }
  argc=wcslen( argv[1] );
  Visio.ModelView=mvBoth;     // на оба борта, всё другое может быть вычищенным
  if( argc>4 && wcscmp( Slower( argv[1]+argc-4 ),L".fef" )==0 )
  { if( !LoadFEF( argv[1]) )
    { textcolor( LIGHTMAGENTA );
      printf( "?File Exchange Format %s не срабатывает",W2U( argv[1] ) ); getch(); exit( argc );
    }
  } else
  if( !LoadProject( argv[1] ) )
  { textcolor( LIGHTMAGENTA );
    printf( "?Free!Ship неверный файл/incorrect datafile: %s ",W2U( argv[1] ) ); getch(); exit( argc );
  }
}
//     Конструктор не содержит Matrix для движений корпуса в корабельном базисе
//
FreeShip::FreeShip():Ship(),View("Free!ship in C++ ",-12,12,640,480) //Matrix()
{ Icon( "Ship" ).AlfaVector( 16 );
  Locate( Xpm( 4 ),Ypm( 4 ),min( 1280,Xpm( 64 ) ),
                            min( 1024,Ypm( 72 ) ) );
  glCullFace( GL_FRONT_AND_BACK );                    // какие отбираются грани
  Distance=-1.75*( Max.x+Max.y-Min.x-Min.y + Width*(Max.z-Min.z)*0.9/Height );
  eyeX=135;  // lookX=-60;
  glDisable( GL_FOG );
  glDisable( GL_LIGHT1 );
  glLightfv( GL_LIGHT0,GL_AMBIENT,(const float[]){.01,.03,.02,1}); // окружение
  glLighti ( GL_LIGHT0,GL_SPOT_CUTOFF,60 );
  glLightfv( GL_LIGHT0,GL_POSITION,(const float[]){0,.1*Distance,-Distance,.5});
  glLightfv( GL_LIGHT0,GL_SPOT_DIRECTION,(const float[]){0,0,-1,0 });
  glMateriali( GL_FRONT_AND_BACK,GL_SHININESS,255 );     // степень отсветки
  Draw();                                               // начальная прорисовка
}
//   Интерактивная настройка/управление графическим отображением проекта
//
bool FreeShip::KeyBoard( fixed Keyb ){               // С краткой подсказкой
const static char                                    // по настройкам и методам
     *Id[]={"Ship"," Корабельные форматы ",        // визуализации корпуса
                   " Free!Ship: *.ftm,*fbm.*fef..",0 },
     *Cmds[]={ " F1 "," - cправка",0 },
     *Plus[]={ " <Space>            ","борт\\полборта",
               " <Ctrl+Space>       "," грани\\рёбра",
               " <стрелки·leftMouse>"," ориентация",
               " <+Shift·rightMouse>"," смещение",
               " <+Ctrl·(roll)>"," дальность, наклон",
               " <Home>  "," вместить корпус в экран",0 };
  BoardView &B=Visio.ModelView;
  if( Keyb==_F1 )Window::Help( Id,Cmds,Plus,1,1 ),Draw(); else       // справка
  if( Keyb==_Blank )
  { if( ScanStatus()&CTRL )
    { static bool xd=false;
      if( xd^=true )glPolygonMode( GL_FRONT_AND_BACK,GL_FILL );
              else  glPolygonMode( GL_FRONT,GL_FILL ),
                    glPolygonMode( GL_BACK,GL_LINE );
    } else
    if( B==mvPort )B=mvBoth; else B=mvPort;
    return Draw();
  }
  else return View::KeyBoard( Keyb );
}
int main() // int argc, char **argv )
{ texttitle( "FREE!Ship view in C++ " ); FreeShip Hull;            // заголовок
  do{ Hull.Title( DtoA( Real(ElapsedTime())/3600000.0,-3 ) ); WaitTime( 500 );
    } while( WinReady() && Hull.GetKey()!=_Esc ); return 0;
}


