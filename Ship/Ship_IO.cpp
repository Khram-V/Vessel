
#include "Ship.h"

const char Future[]="FREE!ship"; // признак FREE!Ship цифровой модели Fbm и Ftm
const char Future_part[]="FREE!ship partfile"; // тоже для дельных вещей
const char *sVer[]={
 "1.0",  "1.1",  "1.2",  "1.3",  "1.4",  "1.5",  "1.6",  "1.65", "1.7",  "1.8",
 "1.9",  "1.91", "1.95", "1.98", "2.0",  "2.01", "2.1",  "2.2",  "2.3",  "2.4",
 "2.5",  "2.6",  "2.7+", "2.8+", "2.94+","2.95+","2.96+","2.97+","2.98+","3.0+",
 "3.02+","3.03+","3.08+","3.09+","3.12+","3.13+","3.16+","3.27+","3.3+","3.34+",
 "3.4",  "4.2",  "4.3",  "4.6.2","5.0",  "5.1" };
const int nVer=sizeof( sVer )/sizeof( char* );        // =>46 количество версий
static FileVersion FV=fv261;
static bool isBin=false;       // признак двоичной или текстовой записи корпуса
static FILE *F=NULL;           // локальный файл открывается временно
static string Str;             // рабочая строчка изначально имеет 2К
//atic int LastLayer=-1;       // ID такой без последовательного перечисления
static Real Scale=1.0;         // масштаб на случай совмещения моделей ...part.
const Real Foot=0.3048;

static bool OpenFile(WCHAR *FileName) // открытие файла цифровой модели корпуса
{ char FTyp[14];
  F=_wfopen( FileName,L"rb" ); fread( FTyp,1,13,F );
  if( !strncmp( Future,FTyp,9 ) )
    { fclose(F); F=_wfopen(FileName,L"rt"); fgets(FTyp,13,F); return false; }
  else if( ((int*)FTyp)[0]==9 && !strncmp( Future,FTyp+4,9 ) )return true;
  fclose( F ); F=NULL; return false;
}
static FileVersion getVersion()
{ if( isBin )return FileVersion( fgetc( F ) ); else
  { int i; char *str=getString( F );                  // изначально в строке 2К
    for( i=0; i<nVer && strcmp( str,sVer[i] ); i++ ); return FileVersion( i );
} }
static byte getByte()
{ if( isBin )return fgetc( F ); else return atoi( getString( F ) );
}
static int S2I( char *s )
{ int I;
  for( I=0; I<strlen( s ); I++ )if( s[I]>' ' )break; s+=I;
  if( s[0]!='$' )return atoi( s );         //  return strtol( S+1,&S,16 );
  I=0; sscanf( s+1,"%X",&I ); return I;    //  return atoi( getString( F ) );
}
static int getInt()
{ int I; if( isBin ){ fread( &I,1,4,F ); return I; }
  return S2I( getString( F ) );
/*
  char *S=getString( F );
  for( I=0; I<strlen( S ); I++ )if( S[I]>' ' )break; S+=I;
  if( S[0]!='$' )return atoi( S );         //  return strtol( S+1,&S,16 );
  sscanf( S+1,"%X",&I ); return I;         //  return atoi( getString( F ) );
*/
}
static Real getFloat()
{ if( isBin ){ float R; fread( &R,1,4,F ); return R; } else return atof( getString( F ) );
}
static Vector getPoint()
{ Vector P;
  if( isBin ){ P.x=getFloat(); P.y=getFloat(); P.z=getFloat(); }
  else sscanf( getString( F ),"%lg%lg%lg",&P.x,&P.y,&P.z );
 return P;
}
static Plane getPlane()
{ Plane P;
  if( isBin )P.a=getFloat(),P.b=getFloat(),P.c=getFloat(),P.d=getFloat();
  else sscanf( getString( F ),"%lg%lg%lg%lg",&P.a,&P.b,&P.c,&P.d );
 return P;
}
static void readText( char **src )
{ if( *src )free( *src ); // на входе-выходе просто адрес = строчка - недотрога
  if( !isBin )*src=strdup( getString( F ) ); else
  { int l=0; fread( &l,1,4,F );                Str[0]=0;                     // print(" l=%d ",l);
    for( int i=0; i<l; i++ )Str[i]=fgetc( F ); Str[l]=0;
    *src=strdup( WintU( Str ) );
  }
}
struct Image{ int W,H,Size;; int Read(); };
int Image::Read()
{ W=getInt(); H=getInt(); Size=getInt();
  if( isBin )fseek( F,Size,SEEK_CUR ); else getString( F );                     print( " Pic[%d·%d]=%d байт\n",W,H,Size );
  return Size;
}
struct BackImage
{ ViewType VT;
  bool Visible,Transparent;
  int Quality, OrgX,OrgY, Blend, TransColor,Tolerance; Real Scale; Image Pic;
  void Read();
};
void BackImage::Read()
{ VT     =(ViewType)getInt();                                                   print( " Type=%d",VT );
  Visible=getByte();                                                            print( " V=%d",Visible );
  Quality=getInt(),OrgX=getInt(),OrgY=getInt();                                 print( " Q:%d,<%d,%d>",Quality,OrgX,OrgY );
  Scale  =getFloat();                                                           print( " S:%g",Scale );
  Blend  =getInt();                                                             print( " B:%d",Blend );
  Transparent=getByte(); TransColor=getInt();                                   print( " T=%X:%X",Transparent,TransColor );
  Tolerance=getInt();                                                           print( " Tol:%d",Tolerance );
  Pic.Read();
}
//   Основная процедура считывания цифрового проекта корабля
//
bool Ship::LoadProject()
{ isBin=OpenFile( FName );
   if( !F )return YesShip=false;                                                print( "Открыт %s файл: %s\n",isBin?"двоичный":"текстовый",Name); textcolor(LIGHTBLUE);
   FV=(FileVersion)getVersion();                                                print( "Версия = %d = '%s'\n",FV,sVer[FV]);
   PT=(PrecisionType)getInt();                                                  print( "Точность = %s[%d]\n",((const char*[]){"Low","Medium","High","VeryHigh"})[PT],PT);
   //
   //   Сначала параметры визуализации
   //
   Visio.ModelView=(BoardView)getInt();                                         textcolor( LIGHTGRAY ),print( "< Visibility >:\n1^2 борта = %d\n",Visio.ModelView );
   Visio.ControlNet=getByte();                                                  print( "ControlNet = %d\n",Visio.ControlNet );
   Visio.InteriorEdges=getByte();                                               print( "InteriorEdges = %d\n",Visio.InteriorEdges );
   Visio.Stations=getByte();                                                    print( "Stations = %d\n",Visio.Stations );
   Visio.Buttocks=getByte();                                                    print( "Buttocks = %d\n",Visio.Buttocks );
   Visio.Waterlines=getByte();                                                  print( "Waterlines = %d\n",Visio.Waterlines );
   Visio.Normals=getByte();                                                     print( "Normals = %d\n",Visio.Normals );
   Visio.Grid=getByte();                                                        print( "Grid = %d\n",Visio.Grid );
   Visio.Diagonals=getByte();                                                   print( "Diagonals = %d\n",Visio.Diagonals );
   Visio.Markers=getByte();                                                     print( "Markers = %d\n",Visio.Markers );
   Visio.Curvature=getByte();                                                   print( "Curvature = %d\n",Visio.Curvature );
   Visio.CurvatureScale=getFloat();                                             print( "CurvatureScale = %g\n",Visio.CurvatureScale );
   if( FV>=fv195 )
   { Visio.ControlCurves=getByte();                                             print( "195+ ControlCurves = %d\n",Visio.ControlCurves );
     if( FV>=fv210 )
     { Visio.CursorIncrement=getFloat();                                        print( "210+ CursorIncrement = %g\n",Visio.CursorIncrement );
       if( fabs( Visio.CursorIncrement )<1e-7 )Visio.CursorIncrement=0.1;
       if( FV>=fv220 )
       { Visio.HydrostaticData=getByte();                                       print( "220+ HydrostaticData = %d\n",Visio.HydrostaticData );
         Visio.HydrostDisplacement=getByte();                                   print( "220+ HydrostDisplacement = %d\n",Visio.HydrostDisplacement );
         Visio.HydrostLateralArea=getByte();                                    print( "220+ HydrostLateralArea = %d\n",Visio.HydrostLateralArea );
         Visio.HydrostSectionalAreas=getByte();                                 print( "220+ HydrostSectionalAreas = %d\n",Visio.HydrostSectionalAreas );
         Visio.HydrostMetacentricHeight=getByte();                              print( "220+ HydrostMetacentricHeight = %d\n",Visio.HydrostMetacentricHeight );
         Visio.HydrostLCF=getByte();                                            print( "220+ HydrostLCF = %d\n",Visio.HydrostLCF );
         if( FV>=fv250 )
         { Visio.lFlowline=getByte();                                           print( "250+ logFlowlines = %d\n",Visio.lFlowline );
         } //=250
       }   //=220
     }     //=210
   }       //=195
   //
   //  Затем описание характеристик и размерений корпуса с авторскими ссылками
   //
   readText( &Set.Name );                                                       textcolor( WHITE ),print( "< Project >\nName=%s\n",Set.Name );
   readText( &Set.Designer );                                                   print( "Name=%s\n",Set.Designer );
       Length=getFloat();                                                       print( "Length=%g\n",Length );
       Beam=getFloat();                                                         print( "Beam=%g\n",Beam );
       Draft=getFloat();                                                        print( "Draft=%g\n",Draft );
   Set.MainparticularsHasBeenset=getByte();                                     print( "MainparticularsHasBeenset=%d\n",Set.MainparticularsHasBeenset );
   Set.WaterDensity=getFloat();                                                 print( "WaterDensity=%g\n",Set.WaterDensity );
   Set.AppendageCoefficient=getFloat();                                         print( "AppendageCoefficient=%g\n",Set.AppendageCoefficient );
   Set.ShadeUnderwaterShip=getByte();                                           print( "ShadeUnderwaterShip=%d\n",Set.ShadeUnderwaterShip );
       UnderWaterColor.C=getInt();                                              print( "UnderWaterColor=%08X\n",UnderWaterColor );
       UnderWaterColorAlpha=UnderWaterColor.c[3]=255-UnderWaterColor.c[3];
   Set.Units=(UnitType)getInt();                                                print( "Units = %d\n",Set.Units );
   Set.UseDefaultSplitSectionLocation=getByte();                                print( "UseDefaultSplitSectionLocation=%d\n",Set.UseDefaultSplitSectionLocation );
   Set.SplitSectionLocation=getFloat();                                         print( "SplitSectionLocation=%g\n",Set.SplitSectionLocation );
 /*if( FV>=fv165 )*/{ Set.DisableModelCheck=getByte();                          print( "DisableModelCheck=%d\n",Set.DisableModelCheck );
                    }
   readText( &Set.Comment );                                                    print( "Comment=%s\n",Set.Comment );
   readText( &Set.CreatedBy );                                                  print( "CreatedBy=%s\n",Set.CreatedBy );
   Set.SavePreview=true;
   if( FV>=fv210 )
   { Set.HydrostaticCoefficients=(HydrostaticCoefficient)getInt();              print( "210+ HydrostaticCoefficients = %d\n",Set.HydrostaticCoefficients );
     Set.SavePreview=getByte();                                                 textcolor( YELLOW ),print( "210+ SavePreview=%d\n",Set.SavePreview );
     if( Set.SavePreview )
     { Image Fon;                                                               print( "210+ " );
       Fon.Read();
//     int W,H,Size; W=getInt(); H=getInt(); Size=getInt();
//     if( isBin )fseek( F,Size,SEEK_CUR ); else getString( F ); print( "210+ Картинка[%d·%d]=%d байт\n",W,H,Size );
     }
     if( FV>=fv230 )
     { Set.SimplifyIntersections=getByte();                                     textcolor( LIGHTCYAN ),print( "230+ SimplifyIntersections=%d\n",Set.SimplifyIntersections );
       if( FV>=fv250 )
       { Set.StartDraft=getFloat();                                             print( "250+ StartDraft=%g\n",Set.StartDraft );
         Set.EndDraft=getFloat();                                               print( "250+ EndDraft=%g\n",Set.EndDraft );
         Set.DraftStep=getFloat();                                              print( "250+ DraftStep=%g\n",Set.DraftStep );
         Set.Trim=getFloat();                                                   print( "250+ Trim=%g\n",Set.Trim );

         Set.NoDisplacements=getInt();                                          print( "250+ NoDisplacements=%d\n",Set.NoDisplacements );
         Set.Displacements=(Real*)Allocate( Set.NoDisplacements*sizeof(Real) );
         for( int i=0; i<Set.NoDisplacements; i++ )Set.Displacements[i]=getFloat();

         Set.MinimumDisplacement=getFloat();                                    print( "250+ MinimumDisplacement=%g\n",Set.MinimumDisplacement );
         Set.MaximumDisplacement=getFloat();                                    print( "250+ MaximumDisplacement=%g\n",Set.MaximumDisplacement );
         Set.DisplIncrement=getFloat();                                         print( "250+ DisplIncrement=%g\n",Set.DisplIncrement );
         Set.UseDisplIncrements=getByte();                                      print( "250+ UseDisplIncrements=%d\n",Set.UseDisplIncrements );

         Set.NoAngles=getInt();                                                 print( "250+ NoAngles=%d\n",Set.Angles );
         Set.Angles=(Real*)Allocate( Set.NoAngles*sizeof(Real) );
         for( int i=0; i<Set.NoAngles; i++ )Set.Angles[i]=getFloat();

         Set.NoTrims=getInt();                                                  print( "250+ NoTrims=%d\n",Set.Trims );
         Set.Trims=(Real*)Allocate( Set.NoTrims*sizeof(Real) );
         for( int i=0; i<Set.NoTrims; i++ )Set.Trims[i]=getFloat();

         Set.FreeTrim=getByte();                                                print( "250+ FreeTrim=%d\n",Set.FreeTrim );
         Set.FVCG=getFloat();                                                   print( "250+ FVCG=%g\n",Set.FVCG );
         if( FV>=fv317 )
         { Set.EnableModelAutoMove=getByte();                                   print( "317+ EnableModelAutoMove=%d\n",Set.EnableModelAutoMove );
           if( FV>=fv332 )
           { Set.EnableBonjeanSAC=getByte();                                    print( "317+ EnableBonjeanSAC=%d\n",Set.EnableBonjeanSAC );
           } //=332
         }   //=317
       }     //=250
     }      /* =230 */                                                          textcolor( LIGHTGRAY );
     // UnderWaterColorAlpha = 0xFF; // Hull<FREE!ship>.UnderWaterColorAlpha;
     if( FV>=fv500 )UnderWaterColorAlpha=getInt(),print( "500+ UnderWaterColorAlpha=%d\n",UnderWaterColorAlpha );
   }      ///* =210
   //
   //!   чтение основных данным по обводам корабля - форме корпусной обшивки
   //
   Shell.Read();
   //
   //!   ... и всякое сбоку-припёку
   //
  int I,K,N;
   NoStations=getInt();                                                         print( "< Stations > = %d frames\n",NoStations );
   Stations=(InterSection*)Allocate( NoStations*sizeof( InterSection ) );
   for( I=0; I<NoStations; I++ ){ Stations[I].Read(); Stations[I].ReConnect(); } //Station(); }
   NoButtocks=getInt();                                                         print( "< Buttocks > = %d curves\n",NoButtocks );
   Buttocks=(InterSection*)Allocate( NoButtocks*sizeof( InterSection ) );
   for( I=0; I<NoButtocks; I++ ){ Buttocks[I].Read(); Buttocks[I].ReConnect(); }
   NoWaterlines=getInt();                                                       print( "< Waterlines > = %d lines\n",NoWaterlines );
   Waterlines=(InterSection*)Allocate( NoWaterlines*sizeof( InterSection ) );
   for( I=0; I<NoWaterlines; I++ )Waterlines[I].Read();
   if( FV>=fv180 )
   { NoDiagonals=getInt();                                                      print( "180+ < Diagonals > = %d fishes\n",NoDiagonals );
     Diagonals=(InterSection*)Allocate( NoDiagonals*sizeof( InterSection ) );
     for( I=0; I<NoDiagonals; I++ )Diagonals[I].Read();
     if( FV>=fv191 )                                                            print( "191+ < Markers > = %d \n",NoMarkers );
     { NoMarkers=getInt();
       Marks=(Marker*)Allocate( NoMarkers*sizeof(Marker) );
       for( I=0; I<NoMarkers; I++ )
       { Marks[I].Visible=getByte();
         if( FV>=fv261 )Marks[I].Selected=getByte();
         Marks[I].ShowCurvature=getByte();
         Marks[I].CurvatureScale=getFloat();
         Marks[I].NoSplines=getInt();
         Marks[I].S=(Spline*)Allocate( Marks[I].NoSplines*sizeof( Spline ) );
//       Marks[I].S=(Marker::Spline*)Allocate( Marks[I].NoSplines*sizeof( Marker::Spline ) );
         for( K=0; K<Marks[I].NoSplines; K++ )
         { Marks[I].S[K].P=getPoint();
           Marks[I].S[K].Knuckle=getByte();
         }
       }
       if( FV>=fv210 )
       { for( I=0; I<17; I++ )getFloat(); getByte(),getByte(); /// выравнивание до 4 байт
          if( isBin )fseek( F,2,SEEK_CUR );                                     print( "210+ DelftSeriesResistanceData = 17x4+2x1(+2) = %d\n",17*4+2+2 ); //=72
         for( I=0; I<9; I++ )getFloat(); getByte();
          if( isBin )fseek( F,3,SEEK_CUR );                                     print( "210+ KAPERResistanceData = 9x4+1(+3) = %d\n",9*4+1+3 ); //=40
         if( FV>=fv250 )
         { BackImage *Pic; N=getInt();                                          print( "250+ BackGroundImages = %d\n",N );
           if( N>0 )
           { Pic = (BackImage*)Allocate( N*sizeof( BackImage ) );
             for( I=0; I<N; I++ )Pic[I].Read();                                 print( "250+ Фон №_%d ",I );
           }
           N=getInt();                                                          print( "250+ Flow Lines = %d\n",N );
           if( N>0 )
           { NoFlowLines=N;
             FlowLines=(Flex*)Allocate( N*sizeof( Flex ) );
             for( I=0; I<NoFlowLines; I++ )
             { getFloat(), // FProjectionPoint.X
               getFloat(); // FProjectionPoint.Y
               getInt();   // fvBodyplan,fvProfile,fvPlan,fvPerspective: ViewType
               getByte(),getByte(); // FBuild,AddSorted
               N=getInt();
               for( K=0; K<N; K++ ){ FlowLines[I]+=getPoint(); getByte(); }
             }
           }
         } // =250
       }   // =210
     }     // =191
   }       // =180
   fclose( F );
   return YesShip=true;
}
//!  считывание собственно секций всех сплайновых геометрических поверхностей
//
void Surface::Read( bool Part )
{ int I,K,N,NoI,NoL,NoC,EdErr;                       // isLoad=true;
   textbackground( BLUE );                           // First load layerdata
   NoL=NoLayers;                                     // все новые слои вдогонку
   N=getInt();
   NoLayers+=N;                                                                 print( "< Surface.LayerData >\nNoLaeyrs = %d => %d\n",NoL,NoLayers );
   L=(Surface::Layers*)Allocate( NoLayers*sizeof( Surface::Layers ),L );
   if( N )
   { for( I=NoL; I<NoLayers; I++ )
     { /* Layer=AddNewLayer(); */                                               print( "Layer[%02d]: ",I );
       readText( &(L[I].Description) );                                      // print( "'%-24s'",L[I].Description );
//     if( (N=getInt()+NoL)>LastLayer )LastLayer=N;;
       L[I].ID=getInt();                                                        print( " ID{%d} <= %d ",I,L[I].ID );
       L[I].LClr.C=getInt(); L[I].LClr.c[3]=255-L[I].LClr.c[3];                 print( ":%8X",L[I].LClr.C );
       L[I].Visible=getByte();                                                  print( "{%d,",L[I].Visible );
       L[I].Symmetric=getByte();                                                print( "%d,",L[I].Symmetric );
       L[I].Developable=getByte();                                              print( "%d,",L[I].Developable );
       if( FV>=fv180 )
       { L[I].UseforIntersection=getByte();                                     textcolor( LIGHTCYAN ),print("%d,",L[I].UseforIntersection );
         L[I].UswinHydrostatic=getByte();                                       print( "%d}",L[I].UswinHydrostatic );
         if( FV>=fv191 )
         { L[I].MaterialDensity=getFloat();                                     textcolor( LIGHTGREEN ),print(" MD=%g",L[I].MaterialDensity );
           L[I].Thickness=getFloat();                                           print( " Tn=%-3g",L[I].Thickness );
           if( FV>=fv201 )
           { L[I].ShowInLineSpan=getByte();                                     textcolor( LIGHTRED ),print(" Shoe=%d",L[I].ShowInLineSpan );
             if( FV>=fv261 )L[I].AlphaBlend=getInt();                           textcolor( YELLOW ),print(" Alfa=%d ",L[I].AlphaBlend );
           } // =201
         }   // =191
       }     /* =180 */                                                         textcolor( LIGHTGRAY ),print("'%-24s'\n",L[I].Description );
     }       // for
   }
   //  ... здесь начинается разборка со сплайнами и кривыми Безье-поверхностями
   //
   if( !Part ){ ActiveLayer=L[getInt()];                                        print( "index of active layer = %d\n",ActiveLayer ); }
   NoC=NoCoPoint;
   NoCoPoint+=getInt();                                                         print( "< ControlPoints > %d => %d",NoC,NoCoPoint );
   P=(Surface::CoPoint*)Allocate( NoCoPoint*sizeof( Surface::CoPoint ),P );
   for( I=NoC; I<NoCoPoint; I++ )
   { P[I].V=Scale*getPoint();
     P[I].T = (VertexType)getInt();         // тип точки: угловая и т.п.
     P[I].Selected=getByte();               // отмечена жёлтым выбором (чушь.)
     if( FV>=fv198 )P[I].Locked=getByte();  // и бит блокировки (зачем-то тоже)
   }
   NoI=NoEdges;                                                                 if( N>0 )print( " = {%g,%g,%g}`0`",P[0].V.x,P[0].V.y,P[0].V.z );
   NoEdges+=getInt(); EdErr=0;                                                  print( "\n< ControlEdges  > %d => %d",NoI,NoEdges );
   G=(Surface::Edges*)Allocate( NoEdges*sizeof( Surface::Edges ),G );
// if( NoEdges>0 )memset( G,0,NoEdges*sizeof( Surface::Edges ) );
   for( I=NoI; I<NoEdges; I++ )
   { Edges &Edge=G[I];
     K=getInt(); if( K==-1 )K=0; Edge.StartIndex=K+NoC;                         if( K+NoC>=NoCoPoint )EdErr++;
                                 Edge.StartPoint=P[K+NoC].V;
     K=getInt(); if( K==-1 )K=0; Edge.EndIndex=K+NoC;
                                 Edge.EndPoint=P[K+NoC].V;                      if( K+NoC>=NoCoPoint )EdErr++;
     Edge.Crease = getByte();
     if( !Part )
     { Edge.Selected = getByte();
       Edge.ControlEdge = true; // здесь как бы фиксируется факт считывания
   } }                                                                          if( N>0 )print( " = {н:%d+к:%d}`0` #(%d)",G[0].StartIndex,G[0].EndIndex,EdErr ); print( "\n" );
   if( !Part )
   if( FV>=fv195 )
   {                                                                            print( "195+" );
nPart:
     NoI=NoCurves;
     NoCurves+=getInt(); EdErr=0;                                               print( "< ControlCurves > %d => %d",NoI,NoCurves );
     C=(Surface::Curves*)Allocate( NoCurves*sizeof( Surface::Curves ),C );
     for( I=NoI; I<NoCurves; I++ )    //! создание рёбер здесь временно опущено
     { K=getInt();
       C[I].Capacity=K;
       C[I].P=(int*)Allocate( K*sizeof(int) );
       for( int j=0; j<K; j++ )if( (C[I].P[j]=getInt()+NoC)>=NoCoPoint )EdErr++;
       if( !Part )C[I].Selected=getByte();                                      print( ", K=%d[%d]",K,C[I].Selected );
     }                                                                          print( " #(%d)\n",EdErr );
     if( Part )goto Ret;
   }
   NoI=NoFaces;
   NoFaces+=getInt(); EdErr=0;                                                  print( "< ControlFaces  > %d => %d ",NoI,NoFaces );
   F=(Surface::Faces*)Allocate( NoFaces*sizeof( Surface::Faces ),F );
   for( I=NoI; I<NoFaces; I++ )
   { K=getInt();
     F[I].Capacity=K;
     F[I].P=(int*)Allocate( K*sizeof(int) );    // <++ Control Points
     for( int j=0; j<K; j++ )
        { int m=getInt();
          if( m+NoC>=NoCoPoint || m<0 )EdErr++;
          if( m==-1 )m=0; F[I].P[j]=m+NoC;
        }
//   N=getInt(); // сборка индексов в простой последовательности числового ряда
//   for( K=NoL; K<NoLayers; K++ )if( N==L[K].ID ){ N=K; break; }
//   if( K==NoLayers ){ print( "%d#%d,",N,L[K-1].ID ); ++EdErr; }
//     if( K==NoLayers ){ print( "%d,",N ); ++EdErr; }   ~~~ что-то здесь не то
                F[I].LayerIndex=getInt()+NoL;
     if( !Part )F[I].Selected=getByte();
   }                                                                            print( " #(%d) \n",EdErr );
   if( Part )
   { //for( I=0; I<NoEdges; I++ ) // согласование на контуре сшивки...
     //{ Edges &Edge=G[I];
     //  if( Edge.NoFaces!=2 && !Edge.Crease )Edge.Crease=true;
     //}
     goto nPart;
   }
Ret:                                                                            textbackground( BLACK );
   Extents();               // расчёт - переопределение графических экстремумов
}
//   Выборка контуров теоретического чертежа со сплайновыми параметрами
//
void InterSection::Read()
{ int K,N;
  IT=(IntersectionType)getInt();                                           //   print( "\n IT=%d",K );
  if( FV>=fv191 )ShowCurvature=getByte();
            else ShowCurvature=false;                                      //   print( " ShowCurvature=%d",St[I].ShowCurvature );
  Pl=getPlane();
  Build=getByte();
  NIt=N=getInt(); NPt=0;
  T=(Items*)Allocate( N*sizeof(Items) );                                   //   textcolor( YELLOW );print( "\n a=%g,b=%g,c=%g,d=%g,Build=%d,N=%d",St[I].a,St[I].b,St[I].c,St[I].d,St[I].Build,N );
  for( int n=0; n<N; n++ )
  { T[n].NoSplines=K=getInt(); NPt+=K;
    T[n].S=(Spline*)Allocate( K*sizeof( Spline ) );                        //   print( "\nN=%d, K=%d  ",N,K ); getch();
    for( int k=0; k<K; k++ )
    { Vector &P=T[n].S[k].P;
      if( FV >=fv160 )
      { if( IT==fiStation ){ P.x=-Pl.d; P.y=getFloat(); P.z=getFloat(); } else
        if( IT==fiButtock ){ P.x=getFloat(); P.y=-Pl.d; P.z=getFloat(); } else
        if( IT==fiWaterline){P.x=getFloat(); P.y=getFloat(); P.z=-Pl.d; }
        else P = getPoint();
      } else P = getPoint();
      T[n].S[k].Knuckle=getByte();
    }
  }
}
bool Ship::LoadExtFile( bool New )
{ char FileName[MAX_PATH]; int L; strcpy( FileName,W2U( FName ) );
  if( !New )
  { F=FileOpen( FileName, L"rb", L"part",      // простая выборка нового имени
    L"Ship [*.fef *.part *.obj]\1*.fef;*.part;*.obj\1"
     "[ free!Ship Exchange Format ].fef\1*.fef\1"      // с заменой заголовков
     "[ Дельная вещь или фрагмент ].part\1*.part\1"    // записи без излишеств
     "[ WaveFront Technologies ].obj\1*.obj\1"         // ~ Advanced Visualizer
     "Все файлы (*.*)\1*.*\1\1",
    L"? Считывание бортовых конструкций и специальных корабельных секций" );
    if( F!=NULL ){ fclose( F ); F=NULL; }
  }
  L=strlen( FileName ); //wcscpy( FName,U2W( FileName ) );// если New остановка
  FName=wcsdup( U2W( FileName ) );
  if( L>5 && strcmp( FileName+L-5,".part" )==0 )LoadPart( New ); else
  if( L>4 && strcmp( FileName+L-4,".fef" )==0 )LoadFEF(); else // с заменой заголовков
  if( L>4 && strcmp( FileName+L-4,".obj" )==0 )LoadObj(); else
  return false;
  return true;
}
bool Ship::LoadObj()
{ isBin=false;             // чисто текстовое представление числовой информации
  if( !(F=_wfopen( FName,L"rt" ) ) )
  { print( "?не открывается WaveFront Visualiser %s ",W2U( FName ) ); getch(); exit( 2 );
  }
 time_t lt=time(0); char *S=asctime( gmtime( &lt ) ); strcut( S );
  if( !Set.Name    )Set.Name=strdup( fname( Name ) ); // ~ без вычистки
  if( !Set.Designer)Set.Designer="@2026-Ship.exe viewer for free!Ship";
  if( !Set.Comment )Set.Comment="Ship for Aurora stormy seakeeping of ship";
  if( !Set.CreatedBy)Set.CreatedBy=S;
//if( !Set.CreatedBy)Set.CreatedBy=ctime( &lt );
  Shell.ReadObj( W2U( FName ) );
  fclose( F ); F=NULL;
  return true;
}
void Surface::ReadObj( char *Path )           // временный оригинал имени файла
{ char *S,*Name=strdup( Path ); Real r,g,b,a; char *s; // ссылка не текст в буфере файла
  print( "\nОткрыт WaveFront файл: %s",Name );         textcolor( LIGHTCYAN );
 int NoL=NoLayers,                            // уровни будут дополняться
     NoC=NoCoPoint-1;
  memset( &ActiveLayer,0,sizeof(Layers) );
  ActiveLayer.Description="WaveFront"; // Technologies Advanced Visualizer";
  ActiveLayer.Symmetric=false;
  ActiveLayer.ID=NoL;                  // изначально здесь ноль
  ActiveLayer.LClr.C=0xFFAAAAAA;       // предварительная раскладка
  ActiveLayer.Visible=true;
  ActiveLayer.ShowInLineSpan=true;
  while( !feof( ::F ) )
  { if( (s=strchr( S=getString( ::F ),'#' ))!=NULL )*s=0;
    if( strcut( S )<3 )continue;
    S[0]=tolower( S[0] );        // сначала первый символ, а затем и вся строка
    if( !strncmp( S,"v ",2) )
    { P=(CoPoint*)Allocate( ++NoCoPoint*sizeof( CoPoint ),P );
     CoPoint &p=P[NoCoPoint-1];
//    sscanf( S+2,"%lg%lg%lg",&p.V.y,&p.V.x,&p.V.z ); p.V.x=-p.V.x; // Новик здесь
      sscanf( S+2,"%lg%lg%lg",&p.V.x,&p.V.z,&p.V.y ); // так готовится в Авроре
///   sscanf( S+2,"%lg%lg%lg",&p.V.y,&p.V.z,&p.V.x ); p.V.x=-p.V.x;
    } else
    if( !strncmp( S,"f ",2) )
    { char *z,*w=S+2;
      int k=0,*Rc=(int*)calloc( sizeof( int ),4 ); // Allocate не для маленьких
      do{ s=strchr( w,' ' ); if( s )*s=0;
          z=strchr( w,'/' ); if( z )*z=0;
          if( k>3 )Rc=(int*)realloc( Rc,sizeof(int)*(k+1) );
          Rc[k]=atoi( w )+NoC; w=s+1; ++k;  // нормали получаются задом наперёд
      } while( s );
      if( k>2 )                          // наверное прямые так не строятся ...
      { F=(Faces*)Allocate( ++NoFaces*sizeof(Faces),F );
        Faces &f=F[NoFaces-1]; f.Capacity=k;
                               f.P=Rc;
                               f.LayerIndex=ActiveLayer.ID;
      }
/**  здесь не всегда только три точки
     int a,b,c;
      s=strchr(w,' '); *s=0; z=strchr(w,'/'); if(z)*z=0; a=atoi(w); w=s+1;
      s=strchr(w,' '); *s=0; z=strchr(w,'/'); if(z)*z=0; b=atoi(w); w=s+1;
                             z=strchr(w,'/'); if(z)*z=0; c=atoi(w);
      if( a!=b && b!=c && c!=a )
      { F=(Faces*)Allocate(++NoFaces*sizeof(Faces),F);
        Faces &f=F[NoFaces-1]; f.Capacity=3;
                               f.P=(int*)Allocate( 3*sizeof(int) );
                               f.LayerIndex=ActiveLayer.ID;
        f.P[0]=a+NoC;
        f.P[1]=b+NoC;
        f.P[2]=c+NoC;
      }
*/
    } else
    if( !strncmp( Slower( S ),"usemtl",6 ) ) // Slower дале уже готов для всех
    {  if( NoL<NoLayers )
       for( int i=NoL; i<NoLayers; i++ )
       if( !strcmp( S+7,L[i].Description ) )
         { ActiveLayer.ID=i;    // print( "\n%d %s[%d] ",i,L[i].Description,i+1 );
           break;
         }
    } else
    if( !strncmp( S,"mtllib",6 ) ) // разборка расцветки по уровням расслоений
    { strcpy( fname( Name ),S+7 );                                                      // print( "\n собран файл %s ",Name );
     FILE *W=_wfopen( U2W( Name ),L"rt" );   // файл.mtl может быть перепрочтён
      while( !feof( W ) )
      { if( (s=strchr( S=getString( W ),'#' ))!=NULL )*s=0;
        if( strcut( S )<3 )continue;
        if( !strncmp( Slower( S ),"newmtl",6 ) )
        { NoLayers++; L=(Layers*)Allocate( NoLayers*sizeof( Layers ),L );
          memcpy( &L[NoLayers-1],&ActiveLayer,sizeof( Layers ) );
          L[NoLayers-1].Description=strdup( S+7 );
          L[NoLayers-1].ID=NoLayers; } else
        if( !strncmp( S,"kd ",3 ) )
        { Color &c=L[NoLayers-1].LClr;
          sscanf( S+3,"%lg%lg%lg",&r,&g,&b ); c.c[0]=byte( r*255 );
                                              c.c[1]=byte( g*255 );
                                              c.c[2]=byte( b*255 ); } else
        if( !strncmp( S,"d ",2 ) )
        { sscanf( S+2,"%lg",&a ); L[NoLayers-1].LClr.c[3]=byte( 22+a*200 ); //! [22-222] - пусть пока временно
        }
      } fclose( W );                                                            for( int I=NoL; I<NoLayers; I++ )print( "\nID=%d Descr=%s Color=%X",L[I].ID,L[I].Description,L[I].LClr.C );
    }
  }
  if( !NoLayers ) // на случай отсутствия послойного описания свойств, будет 1
  { L=(Layers*)Allocate( sizeof( Layers ) );
                memcpy( &L[0],&ActiveLayer,sizeof( Layers ) ); ++NoLayers;
  }
  Extents();              // расчёт - переопределение графических экстремумов
}
//
//   free!Ship.part = дельная вещь или фрагмент числовой модели корпуса
//
bool Ship::LoadPart( bool New )  // [Ship].part == дельная вещь
{ isBin=true;
  if( (F=_wfopen( FName,L"rb" ))!=NULL )
  { char FTyp[20];
    if( getInt()==18  )
    { fread( FTyp,1,18,F ); if( strncmp( Future_part,FTyp,18 )==0 )goto Cont;
    } fclose( F ); F=NULL;
  }
  if( !New )return false; else
  { print( "\n?не открываются дельные вещи: %s ",W2U( FName ) );
    getch(); exit( 18 );
  }
Cont:
 UnitType Units;
   print( "\nОткрыт файл: %s (дельные вещи и фрагменты free!Ship)\n",W2U( FName ) ); textcolor(LIGHTCYAN);
   FV=(FileVersion)getInt(); /*getVersion*/                                     print( "Версия фрагмента = %d = '%s'\n",FV,sVer[FV]);
   Units=(UnitType)getInt();                                                    print( "Units = %s\n",Set.Units?"Имперский":"Метрический" );
   if( Units==Set.Units )Scale=1.0; else
   { if( Units==fuMetric )Scale=1.0/Foot;
                     else Scale=Foot;
   }
   //
   //   чтение дополнительных данных для ранее открытой модели
   //
   Shell.Read( true );
   fclose( F );
   F=NULL;
   return true;
}
//   Полноценная числовая модель в варианте с базовым форматом, без излишеств
//
bool Ship::LoadFEF()      // Ship.fef == FreeShip Exchange Format
{ int I; isBin=false;     // char *str=NULL;
  if( !(F=_wfopen( FName,L"rt" ) ) )
  { print( "?не открывается free!Ship Exchange Format %s ",W2U( FName ) ); getch(); exit( 2 );
  } print( "\nОткрыт файл: %s (free!Ship exchange format)\n",W2U( FName ) );    textcolor( LIGHTCYAN );
  readText( &Set.Name );                                                        print( "< Project >\nName     =%s\n",Set.Name );
  readText( &Set.Designer );                                                    print(              "Designer =%s\n",Set.Designer );
  readText( &Set.Comment );                                                     print(              "Comment  =%s\n",Set.Comment );
  readText( &Set.CreatedBy );                                                   print(              "CreatedBy=%s\n",Set.CreatedBy );
  sscanf( getString( F ),"%lg%lg%lg%lg%lg%d%d%d",
   &Length,&Beam,&Draft,
   &Set.WaterDensity,
   &Set.AppendageCoefficient,
   &Set.Units,&I,&PT );
   Set.MainparticularsHasBeenset=I;                                             print( "L,B,T={ %g, %g, %g },\n\t WaterDensity=%g, A?C=%g, Units=%d, Been=%d, Точность=%d\n\n",Length,Beam,Draft,Set.WaterDensity,Set.AppendageCoefficient,Set.Units,Set.MainparticularsHasBeenset,PT );
//
// TFreeSubdivisionSurface.ImportFEFFile
//
   Shell.ReadFEF();
   fclose( F ); F=NULL;
   return YesShip=true;
}
//   чтение собственно секций всех сплайновых геометрических поверхностей
//
void Surface::ReadFEF()
{ int I,K,NoL,NoC,NoI; isBin=false; // isLoad=true; First load layerdata
   K=getInt();                                                                  print( "< Surface.LaeyrData >\nNoLaeyrs = %d -> %d\n",NoLayers,K );
   if( !K )K=getInt();                                                          print( "NoLaeyrs(повтор) = %d\n",K );
   NoL=NoLayers;
   NoLayers+=K;                            // счетчик слоёв здесь идет в начало
   L=(Surface::Layers*)Allocate( max(1,NoLayers)*sizeof( Surface::Layers ),L );
   for( I=NoL; I<NoLayers; I++ )
   { Layers &T=L[I]; int v,d,s,u,w,p; char str[12]="";
     readText( &T.Description );                                                print( "[%d]'%-12s'",I,T.Description );
     sscanf( getString( ::F ),"%d%s%i%i%i%i%i%i%lg%lg",&T.ID,
       str,&v,&d,&s,&u,&w,&p,&T.MaterialDensity,&T.Thickness );
       T.Visible=v; T.LClr.C=S2I( str ); T.LClr.c[3]=255-T.LClr.c[3];
       T.Developable=d,
       T.Symmetric=s,
       T.UseforIntersection=u,
       T.UswinHydrostatic=w,
       T.ShowInLineSpan=p;                                                      print( ", ID=%d, Color=%X, Vis=%i, Symm=%i, ... Плотность=%g, Толщина=%g \n",T.ID,T.LClr.C,T.Visible,T.Symmetric,T.MaterialDensity,T.Thickness );
   }
   NoC=NoCoPoint;                                                               print( "< ControlPoints > %d",NoCoPoint );
   NoCoPoint+=getInt();
   P=(Surface::CoPoint*)Allocate( NoCoPoint*sizeof( Surface::CoPoint ),P );
   for( I=NoC; I<NoCoPoint; I++ )
   { CoPoint &T=P[I]; T.T=svRegular; K=0;        // и последних может не быть
     sscanf( getString( ::F ),"%lg%lg%lg%i%i",&T.V.x,&T.V.y,&T.V.z,&T.T,&K );
                          T.Selected=K;
   }                                                                            if( NoCoPoint>0 )print( " = {%g,%g,%g}`0`",P[0].V.x,P[0].V.y,P[0].V.z ); print( "\n" );
   NoI=NoEdges;
   NoEdges+=getInt();                                                           print( "< ControlEdges > %d",NoEdges );
   G=(Surface::Edges*)Allocate( NoEdges*sizeof( Surface::Edges ),G );
   for( I=NoI; I<NoEdges; I++ )
   { int K1,K2,Ck;                                Ck=K=0;
     sscanf( getString( ::F ),"%i%i%i%i",&K1,&K2,&Ck,&K );
       G[I].StartPoint=P[G[I].StartIndex=K1+NoC].V; G[I].Selected=K!=0;
         G[I].EndPoint=P[G[I].EndIndex=K2+NoC].V;   G[I].Crease=Ck!=0;
   }                                                                            if( NoEdges>0 )print( " = {н:%d+к:%d}",G[NoI].StartIndex,G[NoI].EndIndex ); print( "\n" );
   NoI=NoFaces;
   NoFaces+=getInt();                                                           print( "< ControlFaces > %d - количество фрагментов обшивки\n",NoFaces );
   F=(Surface::Faces*)Allocate( NoFaces*sizeof( Surface::Faces ),F );
   for( I=NoI; I<NoFaces; I++ ) // здесь уж чтение напрямую из текстового файла
   { fscanf( ::F,"%d",&K ); F[I].Capacity=K;
     F[I].P=(int*)Allocate( K*sizeof(int) );             /// <++ Control Points
     for( int j=0; j<K; j++ ){ int &M=F[I].P[j]; fscanf( ::F,"%d",&M ); M+=NoC; }
     fscanf( ::F,"%d",&F[I].LayerIndex ); F[I].LayerIndex+=NoL; K=0;
     sscanf( getString( ::F ),"%i",&K ); F[I].Selected=K!=0;
   }                                                                            textbackground( BLACK );
   Extents();               // расчёт - переопределение графических экстремумов
}
void Surface::WriteFEF()
{ fprintf( ::F,"%d\n",NoLayers );
  for( int i=0; i<NoLayers; i++ )
  { Color C=L[i].LClr; C.c[3]=255-C.c[3];
    fprintf( ::F,"%s\n%d $%X %i %i %i %i %i %i %g %g\n",
           L[i].Description,L[i].ID,C,
           L[i].Visible,
           L[i].Developable,
           L[i].Symmetric,
           L[i].UseforIntersection,
           L[i].UswinHydrostatic,
           L[i].ShowInLineSpan,
           L[i].MaterialDensity,
           L[i].Thickness
         );
  }
  fprintf( ::F,"%i\n",NoCoPoint );
  for( int i=0; i<NoCoPoint; i++ )
  { fprintf( ::F,"%g %g %g",P[i].V.x,P[i].V.y,P[i].V.z );
    if( P[i].Selected )fprintf( ::F," %i 1",P[i].T ); else
    if( P[i].T!=svRegular )fprintf( ::F," %i",P[i].T ); fprintf( ::F,"\n" );
  }
  fprintf( ::F,"%i\n",NoEdges );
  for( int i=0; i<NoEdges; i++ )
  { fprintf( ::F,"%i %i %i",G[i].StartIndex,G[i].EndIndex,G[i].Crease );
    if( G[i].Selected )fprintf( ::F," 1" ); fprintf( ::F,"\n" );
  }
  fprintf( ::F,"%i\n",NoFaces );
  for( int i=0; i<NoFaces; i++ )
  { fprintf( ::F,"%i",F[i].Capacity );
    for( int j=0; j<F[i].Capacity; j++ )fprintf( ::F," %d",F[i].P[j] );
    fprintf( ::F," %i",F[i].LayerIndex );
    if( F[i].Selected )fprintf( ::F," 1" ); fprintf( ::F,"\n" );
  }
}
void Ship::WriteVSL()
{ int i,j,n,M; bool vsl=NoStations>0;
  char FileName[MAX_PATH]; strcpy( FileName,sname( W2U( FName ) ) ); fext( FileName,"" );
  if( ( F=FileOpen( FileName,L"wt",vsl?L"vsl":L"fef", // простая выборка нового имени
                     vsl? //L"Aurora-Ship [*.vsl *.fef]\1*.vsl;*.fef\1"
                          L"[ Вычислительный эксперимент ].vsl\1*.vsl\1"
                           "[ free!Ship Exchange Format ].fef\1*.fef\1"
                           "Все файлы (*.*)\1*.*\1\1"
                        : //L"free!Ship [*.fef]\1*.fef\1"
                          L"[ free!Ship Exchange Format ].fef\1*.fef\1"
                           "Все файлы (*.*)\1*.*\1\1",
                          L"? Запись для вычислительного эксперимента Aurora.vsl"
                           ", или сохранение в обменном  формате free!Ship.fef"
                    ) )==NULL )
    { print( "\n~Запись: %s\n не получается, странно...",FileName ); return; }
  M=strlen( FileName );
  if( M>4 && strcmp( FileName+M-4,".fef" )==0 )
  { fprintf( F,"%s\n%s\n%s\n%s\n%g %g %g %g %g %d 1 %d\n",
           Set.Name,Set.Designer,Set.Comment,Set.CreatedBy, Length,Beam,Draft,
           Set.WaterDensity,
           Set.AppendageCoefficient,
           Set.Units,PT );               // Set.MainparticularsHasBeenset=true
    Shell.WriteFEF();
    fclose( F ); F=NULL;
    return;
  } else
  if( M>4 && strcmp( FileName+M-4,".vsl" )!=0 )
    { fclose( F ); _wremove( U2W(FileName) ); return; }

  fprintf( F,";\n; %s\n; %s\n; %s\n; %s\n;\n\x1E < %s >\n %d %d\n %g %g %g %g\n", // \x1E=
           Set.Name,Set.Designer,Set.Comment,Set.CreatedBy,
           fext( fname( W2U( FName ) ),"" ),NoStations,NoStations/2,
           Length,Beam,Draft,Min.z ); // Set.StartDraft  );
  //
  //  Ахтерштевень
  //
 Vector *VB; int N,Id,Iu;        //                NoButtocks=0;
  if( !NoButtocks )fprintf( F,"\n\n 0\n 0\n\n" ); else
  { Real D=Buttocks[0].T[0].S[0].P.y; int I=0;
     for( i=1; i<NoButtocks; i++ )                  // поиск ближайшего к ДП
      if( Buttocks[i].T[0].S[0].P.y<D ){ D=Buttocks[i].T[0].S[0].P.y; I=i; }
     VB=Buttocks[I].ReButtocks( Id,Iu );
     N=Buttocks[I].NPt;
     M=Id-Iu; if( M<0 )M+=N; else M++;
     fprintf( F,"\n\n%3d",M );
     for( i=0; i<M; i++ )fprintf( F," %7.4f %7.4f",VB[(Id-i+N)%N].z,VB[(Id-i+N)%N].x );
//   for( i=Id; i!=Iu; i=(i+N-1)%N )fprintf( F," %7.4f %7.4f",VB[i].z,VB[i].x );
     fprintf( F,"\n 0\n" );
  }
  //
  //   Теоретические шпангоуты - таблица плазовых ординат
  //          ~!~ и где-то они уже предварительно отсортированы от кормы к носу
  //
  for( i=0; i<NoStations; i++ )
  { InterSection &St=Stations[i];                            // St.ReStation();
    Vector &First=St.T[0].S[0].P,                            // St.ReConnect();
           &Last=St.T[St.NIt-1].S[St.T[St.NIt-1].NoSplines-1].P;
    fprintf( F,"\n%3d %7.4f ",St.NPt,First.x );
    if( First.z<Last.z || (First.z==Last.z && Last.y>=First.y) )
    for( j=0; j<St.NIt; j++ )
    { for( n=0; n<St.T[j].NoSplines; n++ ){ Vector &V=St.T[j].S[n].P;
        fprintf( F,"  %7.4f %7.4f",V.z,V.y );
      } fprintf( F,"  " );
    } else
    for( j=St.NIt-1; j>=0; j-- )
      { for( n=St.T[j].NoSplines-1; n>=0; n-- ){ Vector &V=St.T[j].S[n].P;
        fprintf( F,"  %7.4f %7.4f",V.z,V.y );
      } fprintf( F,"  " );
    }   fprintf( F," <%d>",St.NIt );
  }
  //
  //  Форштевень
  //
  if( !NoButtocks )fprintf( F,"\n\n 0\n 0\n" ); else
  { M=Iu-Id; if( M<0 )M+=N; else M++;
    fprintf( F,"\n\n 0\n%3d",M );

//  for( i=Id; ; (++i)%=N )
//     { fprintf( F," %7.4f %7.4f",VB[i].z,VB[i].x ); if( i==Iu )break; }
    for( i=0; i<M; i++ )fprintf( F," %7.4f %7.4f",VB[(i+Id)%N].z,VB[(i+Id)%N].x );
    fprintf( F,"\n\n" );
//} //
    // Это чисто для сверки батоксовых связок при прояснении сбоев в алгоритмах
    //
    fprintf( F,"\n\n%3d < %g:[%d-%d] >",N,VB[0].y,Id,Iu );
    for( i=0; i<N; i++ )                  // контрольный батокс - весь как есть
    { if( i==Id )fprintf( F,"  Id:< %d > ",Id );
                 fprintf( F,"  %3.2f %3.2f",VB[i].z,VB[i].x );
      if( i==Iu )fprintf( F,"  Iu:< %d > ",Iu );
  } }
  //
  //!  ...и вся пропущенная информация в заключение
  //
  fprintf( F,"\n\n%s\n",fname( W2U( FName ) ) );
  fprintf( F,"\n Длина:  [ %6.2f - %-6.2f ] = %g,  мидель : %g  ",Min.x,Max.x,Max.x-Min.x,Set.SplitSectionLocation );
  fprintf( F,"\n Ширина: [ %6.2f - %-6.2f ] = %g ",Min.y,Max.y,(Max.y>-Min.y?Max.y:-Min.y)*2 );
  fprintf( F,"\n Высота: [ %6.2f - %-6.2f ] = %g,  осадка : %g ",Min.z,Max.z,Max.z-Min.z,Draft );

  for( i=0; i<NoButtocks; i++ ){ InterSection &C=Buttocks[i];
    if( !i )fprintf( F,"\n\nБатоксы");
    fprintf( F,"\n%3d %6.4f",C.NPt,C.T[0].S[0].P.y );
    for( j=0; j<C.NIt; j++ )
    { for( n=0; n<C.T[j].NoSplines; n++ ){ Vector &V=C.T[j].S[n].P;
        fprintf( F,"  %6.4f %6.4f",V.z,V.x );
      } fprintf( F," | ",n );
    }   fprintf( F," <%d>",j );
  }
  for( i=0; i<NoWaterlines; i++ ){ InterSection &C=Waterlines[i];
    if( !i )fprintf( F,"\n\nВатерлинии" );
    fprintf( F,"\n%3d %6.4f",C.NPt,C.T[0].S[0].P.z );
    for( j=0; j<C.NIt; j++ )
    { for( n=0; n<C.T[j].NoSplines; n++ ){ Vector &V=C.T[j].S[n].P;
        fprintf( F,"  %6.4f %6.4f",V.x,V.y );
      } fprintf( F," | " );
    }   fprintf( F," <%d>",j );
  }
  for( i=0; i<NoDiagonals; i++ ){ InterSection &C=Diagonals[i];
    if( !i )fprintf( F,"\n\nРыбины" );
    fprintf( F,"\n%3d",C.NPt );
    for( j=0; j<C.NIt; j++ )
    { for( n=0; n<C.T[j].NoSplines; n++ ){ Vector &V=C.T[j].S[n].P;
        fprintf( F,"  %6.4f %6.4f %6.4f",V.x,V.y,V.z );
      } fprintf( F," | " );
    }   fprintf( F," <%d>",j );
  }     fprintf( F,"\n\n" );
        fclose( F ); F=NULL;
}

