
#include "Ship.h"

const char Future[]="FREE!ship"; // признак FREE!Ship цифровой модели Fbm и Ftm
const char *sVer[]={
 "1.0",  "1.1",  "1.2",  "1.3",  "1.4",  "1.5",  "1.6",  "1.65", "1.7",  "1.8",
 "1.9",  "1.91", "1.95", "1.98", "2.0",  "2.01", "2.1",  "2.2",  "2.3",  "2.4",
 "2.5",  "2.6",  "2.7+", "2.8+", "2.94+","2.95+","2.96+","2.97+","2.98+","3.0+",
 "3.02+","3.03+","3.08+","3.09+","3.12+","3.13+","3.16+","3.27+","3.3+","3.34+",
 "3.4",  "4.2",  "4.3",  "4.6.2","5.0" };
static FileVersion FV=fv261;
static bool isBin=false;       // признак двоичной или текстовой записи корпуса
static FILE *F=NULL;           // локальный файл открывается временно
static string Str;             // рабочая строчка изначально имеет 2К

static bool OpenFile( char *FileName )// открытие файла цифровой модели корпуса
{ char FTyp[14];
  F=fopen( FileName,"rb" ); fread( FTyp,1,13,F );
  if( !strncmp( Future,FTyp,9 ) )
  { fclose( F ); F=fopen( FileName,"rt" ); fgets( FTyp,13,F ); return false; }
  else if( ((int*)FTyp)[0]==9 && !strncmp( Future,FTyp+4,9 ) )return true;
  fclose( F ); F=NULL;
  return false;
}
static FileVersion getVersion()
{ if( isBin )return FileVersion( fgetc( F ) ); else
  { int i; char *str=getString( F );                  // изначально в строке 2К
    for( i=0; i<44 && strcmp( str,sVer[i] ); i++ ); return FileVersion( i );
} }
static byte getByte()
{ if( isBin )return fgetc( F ); else return atoi( getString( F ) );
}
static int getInt()
{ if( isBin ){ int I; fread( &I,1,4,F ); return I; }
                 else return atoi( getString( F ) );
}
static Real getFloat()
{ if( isBin ){ float R; fread( &R,1,4,F ); return R; }
                 else return atof( getString( F ) );
}
static Point getPoint()
{ Point P;
  if( isBin ){ P.x=getFloat(); P.y=getFloat(); P.z=getFloat(); }
  else sscanf( getString( F ),"%g%g%g",&P.x,&P.y,&P.z );
 return P;
}
static Plane getPlane()
{ Plane P;
  if( isBin )P.a=getFloat(),P.b=getFloat(),P.c=getFloat(),P.d=getFloat();
  else sscanf( getString( F ),"%g%g%g%g",&P.a,&P.b,&P.c,&P.d );
 return P;
}
static void readText( char **src )
{ if( *src )free( *src ); // на входе-выходе просто адрес = строчка - недотрога
  if( !isBin )*src=strdup( getString( F ) ); else
  { int l=0; fread( &l,1,4,F );                Str[0]=0; //print(" l=%d ",l);
    for( int i=0; i<l; i++ )Str[i]=fgetc( F ); Str[l]=0; *src=strdup( AtU( Str ) );
  }
}

struct Image
{ int W,H,Size;;
  int Read();
};
struct BackImage
{ ViewType VT;
  bool Visible;
  int  Quality, OrgX,OrgY;
  Real Scale;
  int  Blend;
  bool Transparent;
  int  TransColor;
  int  Tolerance;
  Image Pic;
  void Read();
};

void BackImage::Read()
{ VT     =(ViewType)getInt();                    print( " Type=%d",VT );
  Visible=getByte();                             print( " V=%d",Visible );
  Quality=getInt(),OrgX=getInt(),OrgY=getInt();  print( " Q:%d,<%d,%d>",Quality,OrgX,OrgY );
  Scale  =getFloat();                            print( " S:%g",Scale );
  Blend  =getInt();                              print( " B:%d",Blend );
  Transparent=getByte(); TransColor=getInt();    print( " T=%X:%X",Transparent,TransColor );
  Tolerance=getInt();                            print( " Tol:%d",Tolerance );
  Pic.Read();
}
int Image::Read()
{ W=getInt(); H=getInt(); Size=getInt();
  if( isBin )fseek( F,Size,SEEK_CUR ); else getString( F ); print( " Pic[%d·%d]=%d байт\n",W,H,Size );
  return Size;
}

bool Ship::ReadFile( char* FileName )
{ if( !strcut( FileName ) )return false;
  isBin=OpenFile( FileName );
  if( !F )return false;                      print( "Открыт %s файл: %s \n",isBin?"двоичный":"текстовый",FileName );
   FV=(FileVersion)getVersion();             print( "Версия = %d = '%s'\n",FV,sVer[FV] );
   PT=(PrecisionType)getInt();               print( "Точность = %d\n",PT );
   //
   //   Сначала параметры визуализации
   //
   Show.ModelView=(BoardView)getInt();       print( "< Visibility >:\n1^2 борта = %d\n",Show.ModelView );
   Show.ControlNet=getByte();                print( "ControlNet = %d\n",Show.ControlNet );
   Show.InteriorEdges=getByte();             print( "InteriorEdges = %d\n",Show.InteriorEdges );
   Show.Stations=getByte();                  print( "Stations = %d\n",Show.Stations );
   Show.Buttocks=getByte();                  print( "Buttocks = %d\n",Show.Buttocks );
   Show.Waterlines=getByte();                print( "Waterlines = %d\n",Show.Waterlines );
   Show.Normals=getByte();                   print( "Normals = %d\n",Show.Normals );
   Show.Grid=getByte();                      print( "Grid = %d\n",Show.Grid );
   Show.Diagonals=getByte();                 print( "Diagonals = %d\n",Show.Diagonals );
   Show.Markers=getByte();                   print( "Markers = %d\n",Show.Markers );
   Show.Curvature=getByte();                 print( "Curvature = %d\n",Show.Curvature );
   Show.CurvatureScale=getFloat();           print( "CurvatureScale = %g\n",Show.CurvatureScale );
   if( FV>=fv195 )
   { Show.ControlCurves=getByte();           print( "195+ ControlCurves = %d\n",Show.ControlCurves );
     if( FV>=fv210 )
     { Show.CursorIncrement=getFloat();      print( "210+ CursorIncrement = %g\n",Show.CursorIncrement );
       if( fabs( Show.CursorIncrement )<1e-7 )Show.CursorIncrement=0.1; //+++
       if( FV>=fv220 )
       { Show.HydrostaticData=getByte();     print( "220+ HydrostaticData = %d\n",Show.HydrostaticData );
         Show.HydrostDisplacement=getByte();   print( "220+ HydrostDisplacement = %d\n",Show.HydrostDisplacement );
         Show.HydrostLateralArea=getByte();      print( "220+ HydrostLateralArea = %d\n",Show.HydrostLateralArea );
         Show.HydrostSectionalAreas=getByte();    print( "220+ HydrostSectionalAreas = %d\n",Show.HydrostSectionalAreas );
         Show.HydrostMetacentricHeight=getByte(); print( "220+ HydrostMetacentricHeight = %d\n",Show.HydrostMetacentricHeight );
         Show.HydrostLCF=getByte();               print( "220+ HydrostLCF = %d\n",Show.HydrostLCF );
         if( FV>=fv250 )Show.Flowlines=getByte(), print( "250+ Flowlines = %d\n",Show.Flowlines );
       } //=220
     }   //=210
   }     //=195
   //
   //  Затем описание характеристик и размерений корпуса с авторскими ссылками
   //
   readText( &Set.Name );                         print( "< Project >\nName=%s\n",Set.Name );
   readText( &Set.Designer );                     print( "Name=%s\n",Set.Designer );
   Set.Length=getFloat();                         print( "Length=%g\n",Set.Length );
   Set.Beam=getFloat();                           print( "Beam=%g\n",Set.Beam );
   Set.Draft=getFloat();                          print( "Draft=%g\n",Set.Draft );
   Set.MainparticularsHasBeenset=getByte();       print( "MainparticularsHasBeenset=%d\n",Set.MainparticularsHasBeenset );
   Set.WaterDensity=getFloat();                   print( "WaterDensity=%g\n",Set.WaterDensity );
   Set.AppendageCoefficient=getFloat();           print( "AppendageCoefficient=%g\n",Set.AppendageCoefficient );
   Set.ShadeUnderwaterShip=getByte();             print( "ShadeUnderwaterShip=%d\n",Set.ShadeUnderwaterShip );
   Set.UnderWaterColor=getInt();                  print( "UnderWaterColor=%08X\n",Set.UnderWaterColor );
   Set.Units=(UnitType)getInt();                  print( "Units = %d\n",Set.Units );
   Set.UseDefaultSplitSectionLocation=getByte();  print( "UseDefaultSplitSectionLocation=%d\n",Set.UseDefaultSplitSectionLocation );
   Set.SplitSectionLocation=getFloat();           print( "SplitSectionLocation=%g\n",Set.SplitSectionLocation );
   Set.DisableModelCheck=getByte();               print( "DisableModelCheck=%d\n",Set.DisableModelCheck );
   readText( &Set.Comment );                      print( "Comment=%s\n",Set.Comment );
   readText( &Set.CreatedBy );                    print( "CreatedBy=%s\n",Set.CreatedBy );
   Set.SavePreview=true;
   if( FV>=fv210 )
   { Set.HydrostaticCoefficients=(HydrostaticCoefficient)getInt(); print( "210+ HydrostaticCoefficients = %d\n",Set.HydrostaticCoefficients );
     Set.SavePreview=getByte();                   textcolor( YELLOW ),print( "210+ SavePreview=%d\n",Set.SavePreview );
     if( Set.SavePreview )
     { Image Fon;                                 print( "210+ " );
       Fon.Read();
//       int W,H,Size; W=getInt(); H=getInt(); Size=getInt();
//       if( isBin )fseek( F,Size,SEEK_CUR ); else getString( F ); print( "210+ Картинка[%d·%d]=%d байт\n",W,H,Size );
     }
     if( FV>=fv230 )
     { Set.SimplifyIntersections=getByte();       textcolor( LIGHTCYAN ),print( "230+ SimplifyIntersections=%d\n",Set.SimplifyIntersections );
       if( FV>=fv250 )
       { Set.StartDraft=getFloat();               print( "250+ StartDraft=%g\n",Set.StartDraft );
         Set.EndDraft=getFloat();                 print( "250+ EndDraft=%g\n",Set.EndDraft );
         Set.DraftStep=getFloat();                print( "250+ DraftStep=%g\n",Set.DraftStep );
         Set.Trim=getFloat();                     print( "250+ Trim=%g\n",Set.Trim );

         Set.NoDisplacements=getInt();            print( "250+ NoDisplacements=%d\n",Set.NoDisplacements );
         Set.Displacements=(Real*)Allocate( Set.NoDisplacements*sizeof(Real),Set.Displacements );
         for( int i=0; i<Set.NoDisplacements; i++ )Set.Displacements[i]=getFloat();

         Set.MinimumDisplacement=getFloat();      print( "250+ MinimumDisplacement=%g\n",Set.MinimumDisplacement );
         Set.MaximumDisplacement=getFloat();      print( "250+ MaximumDisplacement=%g\n",Set.MaximumDisplacement );
         Set.DisplIncrement=getFloat();           print( "250+ DisplIncrement=%g\n",Set.DisplIncrement );
         Set.UseDisplIncrements=getByte();        print( "250+ UseDisplIncrements=%d\n",Set.UseDisplIncrements );

         Set.NoAngles=getInt();                   print( "250+ NoAngles=%d\n",Set.Angles );
         Set.Angles=(Real*)Allocate( Set.NoAngles*sizeof(Real),Set.Angles );
         for( int i=0; i<Set.NoAngles; i++ )Set.Angles[i]=getFloat();

         Set.NoTrims=getInt();                    print( "250+ NoTrims=%d\n",Set.Trims );
         Set.Trims=(Real*)Allocate( Set.NoTrims*sizeof(Real),Set.Trims );
         for( int i=0; i<Set.NoTrims; i++ )Set.Trims[i]=getFloat();

         Set.FreeTrim=getByte();                  print( "250+ FreeTrim=%d\n",Set.FreeTrim );
         Set.FVCG=getFloat();                     print( "250+ FVCG=%g\n",Set.FVCG );
         if( FV>=fv317 )
         { Set.EnableModelAutoMove=getByte();     print( "317+ EnableModelAutoMove=%d\n",Set.EnableModelAutoMove );
           if( FV>=fv332 )
           { Set.EnableBonjeanSAC=getByte();      print( "317+ EnableBonjeanSAC=%d\n",Set.EnableBonjeanSAC );
           } //=332
         }   //=317
       }     //=250
     }      /* =230 */                            textcolor( LIGHTGRAY );
     Set.UnderWaterColorAlpha =  0xFF; //Hull<FREE!ship>.UnderWaterColorAlpha;
     if( FV>=fv500 )Set.UnderWaterColorAlpha=getInt(),print( "500+ UnderWaterColorAlpha=%d\n",Set.UnderWaterColorAlpha );
   }      ///* =210
   //
   //   чтение собственно секций всех сплайновых геометрических поверхностей
   //
  int I,K,N;
   Shl.isLoad=true;                             // First load layerdata
   N=getInt();                                  print( "< Surface.LaeyrData >\nNoLaeyrs = %d -> %d\n",Shl.NoLayers,N );
   if( Shl.NoLayers>0 )memset( Shl.L,0,Shl.NoLayers*sizeof( Surface::Laeyrs ) );
   Shl.NoLayers=N;
   Shl.L=(Surface::Laeyrs*)Allocate( N*sizeof(Surface::Laeyrs),Shl.L );
// Shl.L=(Surface::Laeyrs*)Allocate( N*sizeof(*Shl.L),Shl.L );
   if( N )
   { for( I=0; I<N; I++ )
     { /* Layer=AddNewLayer(); */               print( "AddNewLaeyr[%d]: ",I );
       readText( &(Shl.L[I].Description) );     print( "'%s'",Shl.L[I].Description );
       Shl.L[I].ID=getInt();                    print( " ID=%d,",Shl.L[I].ID );
       Shl.L[I].Color=getInt();                 print( "%X",Shl.L[I].Color );
       Shl.L[I].Visible=getByte();              print("{%d,",Shl.L[I].Visible );
       Shl.L[I].Symmetric=getByte();            print("%d,",Shl.L[I].Symmetric );
       Shl.L[I].Developable=getByte();          print("%d,",Shl.L[I].Developable );
       if( FV>=fv180 )
       { Shl.L[I].UseforIntersection=getByte(); textcolor( LIGHTCYAN ),print("%d,",Shl.L[I].UseforIntersection );
         Shl.L[I].UswinHydrostatic=getByte();   print("%d}",Shl.L[I].UswinHydrostatic );
         if( FV>=fv191 )
         { Shl.L[I].MaterialDensity=getFloat(); textcolor( LIGHTGREEN ),print(" MD=%g",Shl.L[I].MaterialDensity );
           Shl.L[I].Thickness=getFloat();       print(" Tn=%g",Shl.L[I].Thickness );
           if( FV>=fv201 )
           { Shl.L[I].ShoeinLineSpan=getByte(); textcolor( LIGHTRED ),print(" Shoe=%d",Shl.L[I].ShoeinLineSpan );
             if( FV>=fv261 )
               Shl.L[I].AlphaBlend=getInt();    textcolor( YELLOW ),print(" Alfa=%d",Shl.L[I].AlphaBlend );
           } //=201
         }   //=191
       }     /*=180 */                          textcolor( LIGHTGRAY ),print( "\n" );
     }       // for
   }
   N=getInt();                                  print( "index of active layer = %d\n",N );
   Shl.ActiveLayer=Shl.L[N];

   N=getInt();                                  print( "< ControlPoints > %d",N );
   if( Shl.NoCoPoint>0 )memset( Shl.P,0,Shl.NoCoPoint*sizeof( Surface::CoPoint ) );
   Shl.NoCoPoint=N;
   Shl.P=(Surface::CoPoint*)Allocate( N*sizeof( Surface::CoPoint ) );
   for( I=0; I<N; I++ )
   { Point( Shl.P[I] )=getPoint();
     Shl.P[I].T = (VertexType)getInt();
     Shl.P[I].Selected=getByte();
     if( FV>=fv198 )Shl.P[I].Locked=getByte();
   }                                             if( --I>=0 )print( " = 0 ... [%d]={%g,%g,%g}",I,Shl.P[I].x,Shl.P[I].y,Shl.P[I].z ); print( "\n" );
   N=getInt();                                   print( "< ControlEdges > %d",N );
   if( Shl.NoEdges>0 )memset( Shl.G,0,Shl.NoEdges*sizeof( Surface::Edges ) );
   Shl.NoEdges=N;
   Shl.G=(Surface::Edges*)Allocate( N*sizeof( Surface::Edges ) );
   for( I=0; I<N; I++ )
   { K=getInt(); if( K==-1 )K=0; Shl.G[I].StartIndex=K;
//                               Shl.G[I].StartPoint=Shl.P[K];
     K=getInt(); if( K==-1 )K=0; Shl.G[I].EndIndex=K;
//                               Shl.G[I].EndPoint=Shl.P[K];
     Shl.G[I].Crease = getByte();
     Shl.G[I].Selected = getByte();
   }                         if( --I>=0 )print( " = 0 ... [%d]={%d + %d}",I,Shl.G[I].StartIndex,Shl.G[I].EndIndex ); print( "\n" );

   if( FV>=fv195 )
   { N=getInt();                                           print( "195+ < ControlCurves > %d",N );
     if( Shl.NoCurves>0 )memset( Shl.G,0,Shl.NoCurves*sizeof( Surface::Curves ) );
     Shl.NoCurves=N;
     Shl.C=(Surface::Curves*)Allocate( N*sizeof( Surface::Curves ),Shl.C );
     for( I=0; I<N; I++ )
     { K=getInt();
       Shl.C[I].Capacity=K;
       Shl.C[I].P=(int*)Allocate( K*sizeof(int) );
       for( int j=0; j<K; j++ )Shl.C[I].P[j]=getInt();
       Shl.C[I].Selected=getByte();                        print( ", K=%d[%d]",K,Shl.C[I].Selected );
     }                                                     print( "\n" );
   }
   N=getInt();                                             print( "< ControlFaces > %d\n",N );
   if( Shl.NoFaces>0 )memset( Shl.G,0,Shl.NoFaces*sizeof( Surface::Faces ) );
   Shl.NoFaces=N;
   Shl.F=(Surface::Faces*)Allocate( N*sizeof( Surface::Faces ),Shl.G );
   for( I=0; I<N; I++ )
   { K=getInt();
     Shl.F[I].Capacity=K;
     Shl.F[I].P=(int*)Allocate( K*sizeof(int) );       /// <++ Control Points
     for( int j=0; j<K; j++ ){ int m=getInt(); if( m==-1 )m=0; Shl.F[I].P[j]=m; }
     Shl.F[I].LayerIndex=getInt();
     Shl.F[I].Selected=getByte();
   }

   NoStations=getInt();         print( "< Stations > = %d frames\n",NoStations );
   Stations=(InterSection*)Allocate( NoStations*sizeof( InterSection ) );
   for( I=0; I<NoStations; I++ )Stations[I].Read();

   NoButtocks=getInt();         print( "< Buttocks > = %d curves\n",NoButtocks );
   Buttocks=(InterSection*)Allocate( NoButtocks*sizeof( InterSection ) );
   for( I=0; I<NoButtocks; I++ )Buttocks[I].Read();

   NoWaterlines=getInt();       print( "< Waterlines > = %d lines\n",NoWaterlines );
   Waterlines=(InterSection*)Allocate( NoWaterlines*sizeof( InterSection ) );
   for( I=0; I<NoWaterlines; I++ )Waterlines[I].Read();

   if( FV>=fv180 )
   { NoDiagonals=getInt();      print( "180+ < Diagonals > = %d fishes\n",NoDiagonals );
     Diagonals=(InterSection*)Allocate( NoDiagonals*sizeof( InterSection ) );
     for( I=0; I<NoDiagonals; I++ )Diagonals[I].Read();

     if( FV>=fv191 )            print( "191+ < Markers > = %d \n",NoMarkers );
     { NoMarkers=getInt();
       Marks=(Marker*)Allocate( NoMarkers*sizeof(Marker) );
       for( I=0; I<NoMarkers; I++ )
       { Marks[I].Visible=getByte();
         if( FV>=fv261 )Marks[I].Selected=getByte();
         Marks[I].ShowCurvature=getByte();
         Marks[I].CurvatureScale=getFloat();
         Marks[I].NoSplines=getInt();
         Marks[I].S=(Marker::Spline*)Allocate( Marks[I].NoSplines*sizeof( Marker::Spline ) );
         for( K=0; K<Marks[I].NoSplines; K++ )
         { Marks[I].S[K].P=getPoint();
           Marks[I].S[K].Knuckle=getByte();
         }
       }
       if( FV>=fv210 )
       { for( I=0; I<17; I++ )getFloat(); getByte(),getByte();  if( isBin )fseek( F,2,SEEK_CUR );      /// выравнивание до 4 байт
                                                                print( "210+ DelftSeriesResistanceData = 17x4+2x1(+2) = %d\n",17*4+2+2 );
         for( I=0; I<9; I++ )getFloat(); getByte();             if( isBin )fseek( F,3,SEEK_CUR );
                                                                print( "210+ KAPERResistanceData = 9x4+1(+3) = %d\n",9*4+1+3 );
         if( FV>=fv250 )
         { BackImage *Pic;
           N=getInt();                                          print( "250+ BackGroundImages = %d\n",N );
           if( N>0 )
           { Pic = (BackImage*)Allocate( N*sizeof( BackImage ) );
             for( I=0; I<N; I++ ){                              print( "250+ Фон №_%d ",I );
               Pic[I].Read();
             }
           }
           N=getInt();                                          print( "250+ Flow Lines = %d\n",N );

         } // =250
       }   // =210
     }     // =191
   }       // =180
   return false;
}

void InterSection::Read()
{ int K,N;
  IT=(IntersectionType)getInt();                                       // print( "\n IT=%d",K );
  if( FV>=fv191 )ShowCurvature=getByte();
            else ShowCurvature=false;                                  // print( " ShowCurvature=%d",St[I].ShowCurvature );
  Plane(*this)=getPlane();
  Build=getByte();
  NoItems=N=getInt();
  T=(Items*)Allocate( N*sizeof(Items) );                               // textcolor( YELLOW );print( "\n a=%g,b=%g,c=%g,d=%g,Build=%d,N=%d",St[I].a,St[I].b,St[I].c,St[I].d,St[I].Build,N );
  for( int n=0; n<N; n++ )
  { T[n].NoSplines=K=getInt();
    T[n].S=(InterSection::Items::Spline*)Allocate( K*sizeof(InterSection::Items::Spline) );   // print( "\nN=%d, K=%d  ",N,K  ); getch();
    for( int k=0; k<K; k++ )
    { Point &P=T[n].S[k].P;
      if( FV >=fv160 )
      { if( IT==fiStation ){ P.x=-d; P.y=getFloat(); P.z=getFloat(); } else
        if( IT==fiButtock ){ P.x=getFloat(); P.y=-d; P.z=getFloat(); } else
        if( IT==fiWaterline){P.x=getFloat(); P.y=getFloat(); P.z=-d; }
        else P = getPoint();
      } else P = getPoint();
      T[n].S[k].Knuckle=getByte();
    }
  }
}


