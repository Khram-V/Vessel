
#include "Ship.h"

const char Future[]="FREE!ship"; // признак FREE!Ship цифровой модели Fbm и Ftm
const char *sVer[]={
 "1.0",  "1.1",  "1.2",  "1.3",  "1.4",  "1.5",  "1.6",  "1.65", "1.7",  "1.8",
 "1.9",  "1.91", "1.95", "1.98", "2.0",  "2.01", "2.1",  "2.2",  "2.3",  "2.4",
 "2.5",  "2.6",  "2.7+", "2.8+", "2.94+","2.95+","2.96+","2.97+","2.98+","3.0+",
 "3.02+","3.03+","3.08+","3.09+","3.12+","3.13+","3.16+","3.27+","3.3+","3.34+",
 "3.4",  "4.2",  "4.3",  "4.6.2","5.0" };
const int nVer=sizeof( sVer )/sizeof( char* );        // =>45 количество версий
static FileVersion FV=fv261;
static bool isBin=false;       // признак двоичной или текстовой записи корпуса
static FILE *F=NULL;           // локальный файл открывается временно
static string Str;             // рабочая строчка изначально имеет 2К

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
  { int l=0; fread( &l,1,4,F );                Str[0]=0;      //print(" l=%d ",l);
    for( int i=0; i<l; i++ )Str[i]=fgetc( F ); Str[l]=0;
    *src=strdup( WintU( Str ) );
  }
}
struct Image{ int W,H,Size;; int Read(); };
int Image::Read()
{ W=getInt(); H=getInt(); Size=getInt();
  if( isBin )fseek( F,Size,SEEK_CUR ); else getString( F );     print( " Pic[%d·%d]=%d байт\n",W,H,Size );
  return Size;
}
struct BackImage
{ ViewType VT;
  bool Visible,Transparent;
  int Quality, OrgX,OrgY, Blend, TransColor,Tolerance; Real Scale; Image Pic;
  void Read();
};
void BackImage::Read()
{ VT     =(ViewType)getInt();                                   print( " Type=%d",VT );
  Visible=getByte();                                            print( " V=%d",Visible );
  Quality=getInt(),OrgX=getInt(),OrgY=getInt();                 print( " Q:%d,<%d,%d>",Quality,OrgX,OrgY );
  Scale  =getFloat();                                           print( " S:%g",Scale );
  Blend  =getInt();                                             print( " B:%d",Blend );
  Transparent=getByte(); TransColor=getInt();                   print( " T=%X:%X",Transparent,TransColor );
  Tolerance=getInt();                                           print( " Tol:%d",Tolerance );
  Pic.Read();
}
//   Основная процедура считывания цифрового проекта корабля
//
bool Ship::LoadProject()
{ isBin=OpenFile( FName );
   if( !F )return YesShip=false;                                print( "Открыт %s файл: %s\n",isBin?"двоичный":"текстовый",Name); textcolor(LIGHTBLUE);
   FV=(FileVersion)getVersion();                                print( "Версия = %d = '%s'\n",FV,sVer[FV]);
   PT=(PrecisionType)getInt();                                  print( "Точность = %s[%d]\n",((const char*[]){"Low","Medium","High","VeryHigh"})[PT],PT);
   //
   //   Сначала параметры визуализации
   //
   Visio.ModelView=(BoardView)getInt();                         textcolor( LIGHTGRAY ),print( "< Visibility >:\n1^2 борта = %d\n",Visio.ModelView );
   Visio.ControlNet=getByte();                                  print( "ControlNet = %d\n",Visio.ControlNet );
   Visio.InteriorEdges=getByte();                               print( "InteriorEdges = %d\n",Visio.InteriorEdges );
   Visio.Stations=getByte();                                    print( "Stations = %d\n",Visio.Stations );
   Visio.Buttocks=getByte();                                    print( "Buttocks = %d\n",Visio.Buttocks );
   Visio.Waterlines=getByte();                                  print( "Waterlines = %d\n",Visio.Waterlines );
   Visio.Normals=getByte();                                     print( "Normals = %d\n",Visio.Normals );
   Visio.Grid=getByte();                                        print( "Grid = %d\n",Visio.Grid );
   Visio.Diagonals=getByte();                                   print( "Diagonals = %d\n",Visio.Diagonals );
   Visio.Markers=getByte();                                     print( "Markers = %d\n",Visio.Markers );
   Visio.Curvature=getByte();                                   print( "Curvature = %d\n",Visio.Curvature );
   Visio.CurvatureScale=getFloat();                             print( "CurvatureScale = %g\n",Visio.CurvatureScale );
   if( FV>=fv195 )
   { Visio.ControlCurves=getByte();                             print( "195+ ControlCurves = %d\n",Visio.ControlCurves );
     if( FV>=fv210 )
     { Visio.CursorIncrement=getFloat();                        print( "210+ CursorIncrement = %g\n",Visio.CursorIncrement );
       if( fabs( Visio.CursorIncrement )<1e-7 )Visio.CursorIncrement=0.1;
       if( FV>=fv220 )
       { Visio.HydrostaticData=getByte();                       print( "220+ HydrostaticData = %d\n",Visio.HydrostaticData );
         Visio.HydrostDisplacement=getByte();                   print( "220+ HydrostDisplacement = %d\n",Visio.HydrostDisplacement );
         Visio.HydrostLateralArea=getByte();                    print( "220+ HydrostLateralArea = %d\n",Visio.HydrostLateralArea );
         Visio.HydrostSectionalAreas=getByte();                 print( "220+ HydrostSectionalAreas = %d\n",Visio.HydrostSectionalAreas );
         Visio.HydrostMetacentricHeight=getByte();              print( "220+ HydrostMetacentricHeight = %d\n",Visio.HydrostMetacentricHeight );
         Visio.HydrostLCF=getByte();                            print( "220+ HydrostLCF = %d\n",Visio.HydrostLCF );
         if( FV>=fv250 )
         { Visio.lFlowline=getByte();                           print( "250+ logFlowlines = %d\n",Visio.lFlowline );
         } //=250
       }   //=220
     }     //=210
   }       //=195
   //
   //  Затем описание характеристик и размерений корпуса с авторскими ссылками
   //
   readText( &Set.Name );                    textcolor( WHITE ),print( "< Project >\nName=%s\n",Set.Name );
   readText( &Set.Designer );                                   print( "Name=%s\n",Set.Designer );
       Length=getFloat();                                       print( "Length=%g\n",Length );
       Beam=getFloat();                                         print( "Beam=%g\n",Beam );
       Draft=getFloat();                                        print( "Draft=%g\n",Draft );
   Set.MainparticularsHasBeenset=getByte();                     print( "MainparticularsHasBeenset=%d\n",Set.MainparticularsHasBeenset );
   Set.WaterDensity=getFloat();                                 print( "WaterDensity=%g\n",Set.WaterDensity );
   Set.AppendageCoefficient=getFloat();                         print( "AppendageCoefficient=%g\n",Set.AppendageCoefficient );
   Set.ShadeUnderwaterShip=getByte();                           print( "ShadeUnderwaterShip=%d\n",Set.ShadeUnderwaterShip );
       UnderWaterColor.C=getInt();                              print( "UnderWaterColor=%08X\n",UnderWaterColor );
       UnderWaterColorAlpha=UnderWaterColor.c[3]=255-UnderWaterColor.c[3];
   Set.Units=(UnitType)getInt();                                print( "Units = %d\n",Set.Units );
   Set.UseDefaultSplitSectionLocation=getByte();                print( "UseDefaultSplitSectionLocation=%d\n",Set.UseDefaultSplitSectionLocation );
   Set.SplitSectionLocation=getFloat();                         print( "SplitSectionLocation=%g\n",Set.SplitSectionLocation );
   if( FV>=fv165 ){ Set.DisableModelCheck=getByte();            print( "DisableModelCheck=%d\n",Set.DisableModelCheck );
                  }
   readText( &Set.Comment );                                    print( "Comment=%s\n",Set.Comment );
   readText( &Set.CreatedBy );                                  print( "CreatedBy=%s\n",Set.CreatedBy );
   Set.SavePreview=true;
   if( FV>=fv210 )
   { Set.HydrostaticCoefficients=(HydrostaticCoefficient)getInt(); print( "210+ HydrostaticCoefficients = %d\n",Set.HydrostaticCoefficients );
     Set.SavePreview=getByte();                                 textcolor( YELLOW ),print( "210+ SavePreview=%d\n",Set.SavePreview );
     if( Set.SavePreview )
     { Image Fon;                                               print( "210+ " );
       Fon.Read();
//     int W,H,Size; W=getInt(); H=getInt(); Size=getInt();
//     if( isBin )fseek( F,Size,SEEK_CUR ); else getString( F ); print( "210+ Картинка[%d·%d]=%d байт\n",W,H,Size );
     }
     if( FV>=fv230 )
     { Set.SimplifyIntersections=getByte();                     textcolor( LIGHTCYAN ),print( "230+ SimplifyIntersections=%d\n",Set.SimplifyIntersections );
       if( FV>=fv250 )
       { Set.StartDraft=getFloat();                             print( "250+ StartDraft=%g\n",Set.StartDraft );
         Set.EndDraft=getFloat();                               print( "250+ EndDraft=%g\n",Set.EndDraft );
         Set.DraftStep=getFloat();                              print( "250+ DraftStep=%g\n",Set.DraftStep );
         Set.Trim=getFloat();                                   print( "250+ Trim=%g\n",Set.Trim );

         Set.NoDisplacements=getInt();                          print( "250+ NoDisplacements=%d\n",Set.NoDisplacements );
         Set.Displacements=(Real*)Allocate( Set.NoDisplacements*sizeof(Real) );
         for( int i=0; i<Set.NoDisplacements; i++ )Set.Displacements[i]=getFloat();

         Set.MinimumDisplacement=getFloat();                    print( "250+ MinimumDisplacement=%g\n",Set.MinimumDisplacement );
         Set.MaximumDisplacement=getFloat();                    print( "250+ MaximumDisplacement=%g\n",Set.MaximumDisplacement );
         Set.DisplIncrement=getFloat();                         print( "250+ DisplIncrement=%g\n",Set.DisplIncrement );
         Set.UseDisplIncrements=getByte();                      print( "250+ UseDisplIncrements=%d\n",Set.UseDisplIncrements );

         Set.NoAngles=getInt();                                 print( "250+ NoAngles=%d\n",Set.Angles );
         Set.Angles=(Real*)Allocate( Set.NoAngles*sizeof(Real) );
         for( int i=0; i<Set.NoAngles; i++ )Set.Angles[i]=getFloat();

         Set.NoTrims=getInt();                                  print( "250+ NoTrims=%d\n",Set.Trims );
         Set.Trims=(Real*)Allocate( Set.NoTrims*sizeof(Real) );
         for( int i=0; i<Set.NoTrims; i++ )Set.Trims[i]=getFloat();

         Set.FreeTrim=getByte();                                print( "250+ FreeTrim=%d\n",Set.FreeTrim );
         Set.FVCG=getFloat();                                   print( "250+ FVCG=%g\n",Set.FVCG );
         if( FV>=fv317 )
         { Set.EnableModelAutoMove=getByte();                   print( "317+ EnableModelAutoMove=%d\n",Set.EnableModelAutoMove );
           if( FV>=fv332 )
           { Set.EnableBonjeanSAC=getByte();                    print( "317+ EnableBonjeanSAC=%d\n",Set.EnableBonjeanSAC );
           } //=332
         }   //=317
       }     //=250
     }      /* =230 */                                          textcolor( LIGHTGRAY );
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
   NoStations=getInt();                                         print( "< Stations > = %d frames\n",NoStations );
   Stations=(InterSection*)Allocate( NoStations*sizeof( InterSection ) );
   for( I=0; I<NoStations; I++ ){ Stations[I].Read(); Stations[I].ReConnect(); } //Station(); }
   NoButtocks=getInt();                                         print( "< Buttocks > = %d curves\n",NoButtocks );
   Buttocks=(InterSection*)Allocate( NoButtocks*sizeof( InterSection ) );
   for( I=0; I<NoButtocks; I++ ){ Buttocks[I].Read(); Buttocks[I].ReConnect(); }
   NoWaterlines=getInt();                                       print( "< Waterlines > = %d lines\n",NoWaterlines );
   Waterlines=(InterSection*)Allocate( NoWaterlines*sizeof( InterSection ) );
   for( I=0; I<NoWaterlines; I++ )Waterlines[I].Read();
   if( FV>=fv180 )
   { NoDiagonals=getInt();                                      print( "180+ < Diagonals > = %d fishes\n",NoDiagonals );
     Diagonals=(InterSection*)Allocate( NoDiagonals*sizeof( InterSection ) );
     for( I=0; I<NoDiagonals; I++ )Diagonals[I].Read();
     if( FV>=fv191 )                                            print( "191+ < Markers > = %d \n",NoMarkers );
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
       { for( I=0; I<17; I++ )getFloat(); getByte(),getByte();  /// выравнивание до 4 байт
          if( isBin )fseek( F,2,SEEK_CUR );                     print( "210+ DelftSeriesResistanceData = 17x4+2x1(+2) = %d\n",17*4+2+2 ); //=72
         for( I=0; I<9; I++ )getFloat(); getByte();
          if( isBin )fseek( F,3,SEEK_CUR );                     print( "210+ KAPERResistanceData = 9x4+1(+3) = %d\n",9*4+1+3 ); //=40
         if( FV>=fv250 )
         { BackImage *Pic; N=getInt();                          print( "250+ BackGroundImages = %d\n",N );
           if( N>0 )
           { Pic = (BackImage*)Allocate( N*sizeof( BackImage ) );
             for( I=0; I<N; I++ )Pic[I].Read();                 print( "250+ Фон №_%d ",I );
           }
           N=getInt();                                          print( "250+ Flow Lines = %d\n",N );
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
void Surface::Read()
{ int I,K,N,EdErr;
   isLoad=true; textbackground( BLUE );                // First load layerdata
   N=getInt();                                                  print( "< Surface.LayerData >\nNoLaeyrs = %d -> %d\n",NoLayers,N );
   if( NoLayers>0 )memset( L,0,NoLayers*sizeof( Surface::Layers ) );
   NoLayers=N;
   L=(Surface::Layers*)Allocate( N*sizeof( Surface::Layers ) );
   if( N )
   { for( I=0; I<N; I++ )
     { /* Layer=AddNewLayer(); */                               print( "AddNewLaeyr[%02d]: ",I );
       readText( &(L[I].Description) );                      // print( "'%-24s'",L[I].Description );
       L[I].ID=getInt();                                        print( " ID=%02d ",L[I].ID );
       L[I].LClr.C=getInt(); L[I].LClr.c[3]=255-L[I].LClr.c[3]; print( "%8X",L[I].LClr.C );
       L[I].Visible=getByte();                                  print( "{%d,",L[I].Visible );
       L[I].Symmetric=getByte();                                print( "%d,",L[I].Symmetric );
       L[I].Developable=getByte();                              print( "%d,",L[I].Developable );
       if( FV>=fv180 )
       { L[I].UseforIntersection=getByte();                     textcolor( LIGHTCYAN ),print("%d,",L[I].UseforIntersection );
         L[I].UswinHydrostatic=getByte();                       print( "%d}",L[I].UswinHydrostatic );
         if( FV>=fv191 )
         { L[I].MaterialDensity=getFloat();                     textcolor( LIGHTGREEN ),print(" MD=%g",L[I].MaterialDensity );
           L[I].Thickness=getFloat();                           print( " Tn=%-3g",L[I].Thickness );
           if( FV>=fv201 )
           { L[I].ShowInLineSpan=getByte();                     textcolor( LIGHTRED ),print(" Shoe=%d",L[I].ShowInLineSpan );
             if( FV>=fv261 )L[I].AlphaBlend=getInt();           textcolor( YELLOW ),print(" Alfa=%d ",L[I].AlphaBlend );
           } // =201
         }   // =191
       }     /* =180 */                                         textcolor( LIGHTGRAY ),print("'%-24s'\n",L[I].Description );
     }       // for
   }
   //  ... здесь начинается разборка со сплайнами и кривыми Безье-поверхностями
   //
   N=getInt();                                                  print( "index of active layer = %d\n",N );
   ActiveLayer=L[N];

   N=getInt();                                                  print( "< ControlPoints > %d",N );
   if( NoCoPoint>0 )memset( P,0,NoCoPoint*sizeof( Surface::CoPoint ) );
   NoCoPoint=N;
   P=(Surface::CoPoint*)Allocate( N*sizeof( Surface::CoPoint ) );
   for( I=0; I<N; I++ )
   { P[I].V=getPoint();
     P[I].T = (VertexType)getInt();         // тип точки: угловая и т.п.
     P[I].Selected=getByte();               // отмечена жёлтым выбором (чушь.)
     if( FV>=fv198 )P[I].Locked=getByte();  // и бит блокировки (зачем-то тоже)
   }
                                                                if( N>0 )print( " = {%g,%g,%g}`0`",P[0].V.x,P[0].V.y,P[0].V.z );
   N=getInt();                                                  print( "\n< ControlEdges > %d",N );
   NoEdges=N; EdErr=0;
   G=(Surface::Edges*)Allocate( N*sizeof( Surface::Edges ) );
   if( NoEdges>0 )memset( G,0,NoEdges*sizeof( Surface::Edges ) );
   for( I=0; I<N; I++ )
   { Edges &Edge=G[I];
     K=getInt(); if( K==-1 )K=0; Edge.StartIndex=K;             if( K>=NoCoPoint )EdErr++;
                                 Edge.StartPoint=P[K].V;
     K=getInt(); if( K==-1 )K=0; Edge.EndIndex=K;
                                 Edge.EndPoint=P[K].V;          if( K>=NoCoPoint )EdErr++;
     Edge.Crease = getByte();
     Edge.Selected = getByte();
     Edge.ControlEdge = true; // здесь как бы фиксируется факт считывания
   }                                                            if( N>0 )print( " = {н:%d+к:%d}`0` E(%d)",G[0].StartIndex,G[0].EndIndex,EdErr ); print( "\n" );
   if( FV>=fv195 )
   { N=getInt();                                                print( "195+ < ControlCurves > %d",N );
     if( NoCurves>0 )memset( G,0,NoCurves*sizeof( Surface::Curves ) );
     NoCurves=N; EdErr=0;
     C=(Surface::Curves*)Allocate( N*sizeof( Surface::Curves ) );
     for( I=0; I<N; I++ )
     { K=getInt();
       C[I].Capacity=K;
       C[I].P=(int*)Allocate( K*sizeof(int) );
       for( int j=0; j<K; j++ )if( (C[I].P[j]=getInt())>=NoCoPoint )EdErr++;
       C[I].Selected=getByte();                                 print( ", K=%d[%d]",K,C[I].Selected );
     }                                                          print( " E(%d)\n",EdErr );
   }
   N=getInt();                                                  print( "< ControlFaces >\n" );
   if( NoFaces>0 )memset( F,0,NoFaces*sizeof( Surface::Faces ) );
   NoFaces=N; EdErr=0;                                          print( "NoFaces=%d - количество площадок обшивки  ",NoFaces );
   F=(Surface::Faces*)Allocate( N*sizeof( Surface::Faces ) );
   for( I=0; I<N; I++ )
   { K=getInt();
     F[I].Capacity=K;
     F[I].P=(int*)Allocate( K*sizeof(int) );    // <++ Control Points
     for( int j=0; j<K; j++ ){ int m=getInt();
                               if( m>=NoCoPoint || m<0 )EdErr++;
                               if( m==-1 )m=0; F[I].P[j]=m; }
     F[I].LayerIndex=getInt();
     F[I].Selected=getByte();
   }                                                            print( " # E(%d) \n",EdErr );
                                                                textbackground( BLACK );
   Extents();               // расчёт - переопределение графических экстремумов
}
//   Выборка контуров теоретического чертежа со сплайновыми параметрами
//
void InterSection::Read()
{ int K,N;
  IT=(IntersectionType)getInt();                           //   print( "\n IT=%d",K );
  if( FV>=fv191 )ShowCurvature=getByte();
            else ShowCurvature=false;                      //   print( " ShowCurvature=%d",St[I].ShowCurvature );
  Pl=getPlane();
  Build=getByte();
  NIt=N=getInt(); NPt=0;
  T=(Items*)Allocate( N*sizeof(Items) );                   //   textcolor( YELLOW );print( "\n a=%g,b=%g,c=%g,d=%g,Build=%d,N=%d",St[I].a,St[I].b,St[I].c,St[I].d,St[I].Build,N );
  for( int n=0; n<N; n++ )
  { T[n].NoSplines=K=getInt(); NPt+=K;
    T[n].S=(Spline*)Allocate( K*sizeof( Spline ) );        //   print( "\nN=%d, K=%d  ",N,K ); getch();
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
//   Полноценный вариант в варианте с базовым форматом, без излишеств
//
bool Ship::LoadFEF()      // Ship.fef == FreeShip Exchange Format
{ int I; //char *str;
  if( !(F=_wfopen( FName,L"rb" ) ) )return false; print( "Открыт файл: %s (Free!Ship exchange format)\n",Name); textcolor(LIGHTCYAN);
  readText( &Set.Name );                          print( "< Project >\nName     =%s\n",Set.Name );
  readText( &Set.Designer );                      print(              "Designer =%s\n",Set.Designer );
  readText( &Set.Comment );                       print(              "Comment  =%s\n",Set.Comment );
  readText( &Set.CreatedBy );                     print(              "CreatedBy=%s\n",Set.CreatedBy );
  sscanf( getString( F ),"%lg%lg%lg%lg%lg%d%d%d",
   &Length,&Beam,&Draft,
   &Set.WaterDensity,
   &Set.AppendageCoefficient,
   &Set.Units,&I,&PT );
   Set.MainparticularsHasBeenset=I;               print( "L,B,T={ %g, %g, %g },\n\t WaterDensity=%g, A?C=%g, Units=%d, Been=%d, Точность=%d\n\n",Length,Beam,Draft,Set.WaterDensity,Set.AppendageCoefficient,Set.Units,Set.MainparticularsHasBeenset,PT );
//
// TFreeSubdivisionSurface.ImportFEFFile
//
   Shell.ReadFEF();
   fclose( F );
   return YesShip=true;
}
//   чтение собственно секций всех сплайновых геометрических поверхностей
//
void Surface::ReadFEF()
{ int I,K;
   isLoad=true;                                              // First load layerdata
   K=getInt();                                                  print( "< Surface.LaeyrData >\nNoLaeyrs = %d -> %d\n",NoLayers,K );
   if( !K )K=getInt();                                          print( "NoLaeyrs(повтор) = %d\n",K );
   NoLayers=K;
   L=(Surface::Layers*)Allocate( max(1,NoLayers)*sizeof( Surface::Layers ) );
   for( I=0; I<NoLayers; I++ )
   { Layers &T=L[I]; int v,d,s,u,w,p; char str[12]="";
     readText( &T.Description );                                print( "[%d]'%-12s'",I,T.Description );
     sscanf( getString( ::F ),"%d%s%i%i%i%i%i%i%lg%lg",&T.ID,
       str,&v,&d,&s,&u,&w,&p,&T.MaterialDensity,&T.Thickness );
       T.Visible=v; T.LClr.C=S2I( str ); T.LClr.c[3]=255-T.LClr.c[3];
       T.Developable=d,
       T.Symmetric=s,
       T.UseforIntersection=u,
       T.UswinHydrostatic=w,
       T.ShowInLineSpan=p;                                      print( ", ID=%d, Color=%X, Vis=%i, Symm=%i, ... Плотность=%g, Толщина=%g \n",T.ID,T.LClr.C,T.Visible,T.Symmetric,T.MaterialDensity,T.Thickness );
   }
   if( NoCoPoint>0 )memset( P,0,NoCoPoint*sizeof( Surface::CoPoint ) );   print( "< ControlPoints > %d",NoCoPoint );
   NoCoPoint=getInt();
   P=(Surface::CoPoint*)Allocate( NoCoPoint*sizeof( Surface::CoPoint ) );
   for( I=0; I<NoCoPoint; I++ )
   { CoPoint &T=P[I];
     sscanf( getString( ::F ),"%lg%lg%lg%i%i",&T.V.x,&T.V.y,&T.V.z,&T.T,&K );
                          T.Selected=K;
   }                                                            if( NoCoPoint>0 )print( " = {%g,%g,%g}`0`",P[0].V.x,P[0].V.y,P[0].V.z ); print( "\n" );
   NoEdges=getInt();                                            print( "< ControlEdges > %d",NoEdges );
   G=(Surface::Edges*)Allocate( NoEdges*sizeof( Surface::Edges ) );
   if( NoEdges>0 )memset( G,0,NoEdges*sizeof( Surface::Edges ) );
   for( I=0; I<NoEdges; I++ )
   { int K1,K2;
     sscanf( getString( ::F ),"%i%i%i%i",&K1,&K2,&G[I].Crease,&K );
       G[I].StartPoint=P[G[I].StartIndex=K1].V;   G[I].Selected=K;
         G[I].EndPoint=P[G[I].EndIndex=K2].V;
   }                                                            if( NoEdges>0 )print( " = {н:%d+к:%d}",G[0].StartIndex,G[0].EndIndex ); print( "\n" );
   NoFaces=getInt();                                            print( "< ControlFaces >\nNoFaces=%d - количество фрагментов обшивки\n",NoFaces );
   F=(Surface::Faces*)Allocate( NoFaces*sizeof( Surface::Faces ) );
   for( I=0; I<NoFaces; I++ ) // и здесь уж чтение напрямую из текстового файла
   { fscanf( ::F,"%d",&K ); F[I].Capacity=K;
     F[I].P=(int*)Allocate( K*sizeof(int) );             /// <++ Control Points
     for( int j=0; j<K; j++ )fscanf( ::F,"%d",&F[I].P[j] );
     sscanf( getString( ::F ),"%d%i",&F[I].LayerIndex,&K  );
                                      F[I].Selected=K;
   }                                                            textbackground( BLACK );
   Extents();               // расчёт - переопределение графических экстремумов
}
//#include <wctype.h>

void Ship::WriteVSL()
{ int i,j,n,M;
  if( NoStations<1 )
  { Break( "~Для числовой модели необходимы\n контуры теоретических шпангоутов" ); return;
  }
  if( (F=_wfopen( U2W( fext( Name,"vsl" )),L"wt" ))==NULL )
//  if( (F=FileOpen( Name,L"wt",L"vsl",
//         L"Ship Hull Form (*.vsl)\0*.vsl\0All Files\0*.*\0\0",
//         L"Запись числовой модели в формате Hull+Aurora.vsl" ))==NULL )
  { Break( "~Запись $s\n не получается, странно...",Name ); return;
  }
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
     M=Id-Iu; if( M<0 )M+=N; // M++;
     fprintf( F,"\n\n%3d",M );
     for( i=Id; i!=Iu; i=(i+N-1)%N )fprintf( F," %7.4f %7.4f",VB[i].z,VB[i].x );
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
  { M=Iu-Id+1; if( M<0 )M+=N; //..M++;
    fprintf( F,"\n\n 0\n%3d",M );
    for( i=Id; ; (++i)%=N )
       { fprintf( F," %7.4f %7.4f",VB[i].z,VB[i].x ); if( i==Iu )break; }
  //for( i=Id,j=0; j<M; j++,i=(i+1)%N )fprintf( F," %7.4f %7.4f",VB[i].z,VB[i].x );
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
        fclose( F );
}

