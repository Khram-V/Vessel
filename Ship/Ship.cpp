
#include "Ship.h"

Ship::Ship(){ memset( this,0,sizeof( Ship ) ); }

int main( int argc, char **argv )
{
  Ship Hull;
  Window Graphics( "Ship",0,0,600,400 );
  texttitle( "FREE!Ship" );
  if( argc>1 )Hull.ReadFile( argv[1] );   // Ship.Alfabet( 28,"Times New Roman" ).Print( "__" );
  do
  { Graphics.Clear()
      .Print( 2,2,"%s ",DtoA( Real(ElapsedTime())/3600000.0,-3 ) )
      .Show();
    WaitTime( 500 );
  } while( WinReady() && Graphics.GetKey()!=_Esc );
  return 0;
}
