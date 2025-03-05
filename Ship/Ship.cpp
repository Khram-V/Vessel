
#include "Ship.h"
//
//    общие данные как бы в стиле fortran-common блоков
//
Vector Min={0.0},Max={0.0};    // Экстремумы исходного графического изображения
byte  UnderWaterColorAlpha=0xFF;            // для алгоритмов
Color UnderWaterColor={ 0xFFFFFF };         //   с точным положением ватерлинии
 Real Beam=1.0,                             // ширина
      Draft=1.0,                            // осадка
      Length=1.0;                           //  длина
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
{ BoardView &B=Visio.ModelView;
  View::Draw();
  glTranslated( (Max.x+Min.x)/-1.75,0,      // -Set.SplitSectionLocation
                (Max.z+Min.z)/-2 );         // Set.Length/-2
  Clear(); color( lightcyan );                 glLineWidth( .2 );
  axis(*this,Length,Beam,Draft*2,"x","y","z"); glLineWidth( .5 );
  Shell.Drawing( B );
  color( lightblue );  for( int k=0; k<NoStations; k++ )Stations[k].Drawing( B );
  color( green ); for( int k=0; k<NoWaterlines; k++ )Waterlines[k].Drawing( B );
  color( cyan );  for( int k=0; k<NoButtocks; k++ )Buttocks[k].Drawing( B );
  color( olive ); for( int k=0; k<NoDiagonals; k++ )Diagonals[k].Drawing( B );
  color( lightgreen );                                      glLineWidth( 2 );
  for( int k=0; k<NoFlowLines; k++ ){ glBegin( GL_LINE_STRIP );
    for( int i=0; i<FlowLines[k].len; i++ )dot( FlowLines[k][i] ); glEnd();
    if( B==mvBoth ){ glBegin( GL_LINE_STRIP );
      for( int i=0; i<FlowLines[k].len; i++ )dot( ~FlowLines[k][i] ); glEnd();
  } }
  Window::Show(); return true;
}
Ship::Ship()   //! конструктор с расчисткой и считыванием новой числовой модели
{ memset( this,0,sizeof( Ship ) );
  textcolor( LIGHTCYAN );                 print( "Free!Ship [*.fbm,*.ftm]\n" );
  textcolor( LIGHTRED );
  int argc; WCHAR **argv=CommandLineToArgvW( GetCommandLineW(),&argc );
  if( argc>1 )
  if( !LoadProject( argv[1] ) )_exit( 1 );
}
//     Конструктор не содержит Matrix для движений корпуса в корабельном базисе
//
FreeShip::FreeShip():Ship(),View("Free!ship in C++ ",-12,12,640,480) //Matrix()
{ if( !Ship::Ready ){ print( "?неверный файл/incorrect datafile?" ); exit(1); }
  Icon( "Ship" ).AlfaVector( 16 );
  Locate( Xpm( 4 ),Ypm( 4 ),min( 1280,Xpm( 64 ) ),
                            min( 1024,Ypm( 72 ) ) );
  glCullFace( GL_FRONT_AND_BACK );                    // какие отбираются грани
  Distance=-1.75*( Max.x+Max.y-Min.x-Min.y + Width*(Max.z-Min.z)*0.9/Height );
//  Distance=-2*abs( Max-Min ); //  ( Set.Length+Set.Beam+Set.Draft );
//  if(Max.z-Min.z>Max.x-Min.x)Distance*=(Max.z-Min.z)*1280/(Max.x-Min.x)/1024;
  eyeX=135;  // lookX=-60;
  glDisable( GL_FOG );
  glDisable( GL_LIGHT1 );
  glLightfv( GL_LIGHT0,GL_AMBIENT,(const float[]){.01,.03,.02,1}); // окружение
  glLighti ( GL_LIGHT0,GL_SPOT_CUTOFF,60 );
  glLightfv( GL_LIGHT0,GL_POSITION,(const float[]){0,.1*Distance,-Distance,.5});
  glLightfv( GL_LIGHT0,GL_SPOT_DIRECTION,(const float[]){0,0,-1,0 });
//glLightModelfv( GL_LIGHT_MODEL_AMBIENT,(const float[]){ .1,.1,.1,0 } ); //!??
  glMateriali( GL_FRONT_AND_BACK,GL_SHININESS,255 );    // степень отсветки
  Draw();                                               // начальная прорисовка
}
//   Интерактивная настройка/управление графическим отображением проекта
//
bool FreeShip::KeyBoard( fixed Keyb )
{ BoardView &B=Visio.ModelView;
  if( Keyb==' ' ){ if( B==mvPort )B=mvBoth; else B=mvPort; return Draw(); }
  else return View::KeyBoard( Keyb );
}
int main() // int argc, char **argv )
{ texttitle( "FREE!Ship view in C++ " ); FreeShip Hull;            // заголовок
  do{ Hull.Title( DtoA( Real(ElapsedTime())/3600000.0,-3 ) ); WaitTime( 500 );
    } while( WinReady() && Hull.GetKey()!=_Esc ); return 0;
}
