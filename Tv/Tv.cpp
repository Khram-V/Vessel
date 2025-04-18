//
//      Tv.cpp                                                   /91.03.25/
//       - сборник примеров для визуализации и конвертации графики и геобаз
//
#include "Tv.h"
#include <CType.h>
                              //
Button All;                   // Кнопочка для масштабируемых картинок

void Net_CDF ( char *Name );
void Inf_View( char *Name );
void GRD_View( char *Name );
void DCM_View( char *Name, const int Type );
void FNT_View( char *Name, const int Type );
void PCX_View( char *Name );

int main( int Ac, char **Av )
{ static char Name[]="\n>Tv \xABName\xBB"
     ".<inf|int>\t- Bathymetry Seafloor charts"
     "\n\t *.<grd>\t\t& GoldenSoft Topo fields"
     "\n\t *.<nc>\t\t- Atlas of World Surface Marine data"
     "\n\t *.<dc2|dw2|bln>\t- Design Cad & Simple line field"
     "\n\t *.<chr|vct|vfn>\t- Borland & Small American fonts"
     "\n\t *.<pcx|tv>\t- PaintBrush & Epson picture"
     "\n       \xAB-bgi=2\x97""8\xBB\t\t\t   \xA9""1991-2016 V.Khramushin";
  if( Ac<2 )Break( Name );              //
  for( int i=1; i<Ac; i++ )             // Выборка последнего имени
  if( Av[i][0]!='-' && Av[i][0]!='/' )  // в списке параметров
  { char *S;                            //
    if( (S=strrchr( strcpy( Name,Av[i] ),'.' ))!=0 )
    { S[4]=0; strlwr( S+1 );
      if( !strcmp( S+1,"inf" )
       || !strcmp( S+1,"int" ) ){ Inf_View( Name );   return 0; }
      if( !strcmp( S+1,"grd" ) ){ GRD_View( Name );   return 0; }
      if( !strcmp( S+1,"pcx" ) ){ PCX_View( Name );   return 0; }
      if( !strcmp( S+1,"chr" ) ){ FNT_View( Name,2 ); return 0; }
      if( !strcmp( S+1,"vct" ) ){ FNT_View( Name,3 ); return 0; }
      if( !strcmp( S+1,"vfn" ) ){ FNT_View( Name,4 ); return 0; }
      if( !strcmp( S+1,"dc2" )
       || !strcmp( S+1,"dc3" ) ){ DCM_View( Name,5 ); return 0; }
      if( !strcmp( S+1,"dw2" ) ){ DCM_View( Name,6 ); return 0; }
      if( !strcmp( S+1,"bln" ) ){ DCM_View( Name,7 ); return 0; }
      if( !strcmp( S+1,"nc"  ) ){ Net_CDF( Name );    return 0; }
    } break;
  }
}
