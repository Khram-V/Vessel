
#include "Ship.h"
//
//   прорисовка исходных многоугольников
//
void Surface::Drawing( BoardView Sides, _Real Draught )
{ static Flex V; Vector W; int I,K,N; Color C; glLineWidth( 1 );
  for( N=0; N<NoFaces; N++ )   // синхронная по бортам прорисовка треугольников
  if( (K=F[N].Capacity)>2 )
  { V.len=0; C.C=L[F[N].LayerIndex].Color;
    for( I=0; I<K; I++)V+=P[F[N].P[I]].V; // C.c[3]=V[0].z>=Draught?0xFF:0x7F; glColor4ubv(C.c);
    glNormal3dv( W=(V[2]-V[0])*(V[1]-V[0]) ); glBegin( GL_POLYGON );
    for( I=0; I<K; I++ )
      { C.c[3]=V[I].z>Draught?0xFF:0xBF; glColor4ubv( C.c ); dot( V[I] ); }
    glEnd();
    if( Sides==mvBoth )
    { glNormal3dv(~W ); glBegin( GL_POLYGON );
      for( I=K-1; I>=0; I-- )
      { C.c[3]=V[I].z>Draught?0xFF:0xBF; glColor4ubv( C.c ); dot(~V[I] ); }
      glEnd();
  } } glLineWidth( .2 );
}
//    подготовленные в предыдущем расчёте теоретические контуры
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
{ BoardView &B=Shw.ModelView;
  View::Draw();
  glTranslated( -Set.SplitSectionLocation,0,0 ); // Set.Length/-2
  Clear(); color( lightcyan );                              glLineWidth( .2 );
  axis(*this,Set.Length,Set.Beam,Set.Draft*2,"x","y","z" ); glLineWidth( .5 );
  Shl.Drawing( B,Set.Draft );
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
  textcolor( LIGHTCYAN ); print( "Free!Ship [*.fbm,*.ftm]\n" );
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
  Distance=-2*( Set.Length+Set.Beam+Set.Draft );
  eyeX=135;  // lookX=-60;
  glDisable( GL_FOG );
  glDisable( GL_LIGHT1 );
  glLightfv( GL_LIGHT0,GL_AMBIENT,(const float[]){.01,.03,.02,1}); // окружение
  glLighti ( GL_LIGHT0,GL_SPOT_CUTOFF,60 );
  glLightfv( GL_LIGHT0,GL_POSITION,(const float[]){0,.1*Distance,-Distance,.5});
  glLightfv( GL_LIGHT0,GL_SPOT_DIRECTION,(const float[]){0,0,-1,0 });
//glLightModelfv( GL_LIGHT_MODEL_AMBIENT,(const float[]){ .1,.1,.1,0 } ); //!??
  glMateriali ( GL_FRONT_AND_BACK,GL_SHININESS,255 );      // степень отсветки
  Draw();                                               // начальная прорисовка
}
//   Интерактивная настройка/управление графическим отображением проекта
//
bool FreeShip::KeyBoard( fixed Keyb )
{ BoardView &B=Shw.ModelView;
  if( Keyb==' ' ){ if( B==mvPort )B=mvBoth; else B=mvPort; return Draw(); }
  else return View::KeyBoard( Keyb );
}
int main() // int argc, char **argv )
{ texttitle( "FREE!Ship view in C++ " );
  FreeShip Hull;
  do{ Hull.Title( DtoA( Real(ElapsedTime())/3600000.0,-3 ) ); // заголовок
      WaitTime( 500 );
    } while( WinReady() && Hull.GetKey()!=_Esc ); return 0;
}
