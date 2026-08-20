//
//
//    Самые разные числовые форматы кораблей и судов из CAD-систем
//     .vsl (.vil) - традиционная таблица плазовых ординат со штевнями
//     .fef,.ftm,.fbm,.part - всякие упражнения c предвычислениями во free!Ship
//     .obj - Wavefront Technologies Advanced Visualizer
//     .stl - Triangle...
//
#include "Aurora.h"
#include <Time.h>
#include <CType.h>

static Color UnderWaterColor;
static byte  UnderWaterColorAlpha,
             Units; // 0=метрик, иначе - империал
 typedef enum{ fv100,fv110,fv120,fv130,fv140, fv150,fv160,fv165,fv170,fv180,
               fv190,fv191,fv195,fv198,fv200, fv201,fv210,fv220,fv230,fv240,
               fv250,fv261,fv270,fv280,fv290, fv295,fv296,fv297,fv298,fv300,
               fv302,fv303,fv305,fv309,fv310, fv313,fv314,fv317,fv327,fv332,
               fv335,fv421,fv430,fv462,fv500, fv510 } FileVersion; // всего=46
 FileVersion FV=fv261;
//
//!                конструктор с расчисткой и считыванием новой числовой модели
//
freeShip::freeShip(): Hull(),
  ProjName( " название проекта" ),
  Designer( " автор проекта"    ),
  Comment ( " расширенное описание" ),
 CreatedBy( " изготовитель цифровой модели" ),
 NoLayers( 0 ),NoCoPoint( 0 ),NoFaces( 0 ), L( NULL ),P( NULL ),F( NULL )
{ //Vessel=this;
  ActiveLayer.Description="Чистый слой";
  ActiveLayer.ID=0;                    // изначально здесь ноль
  ActiveLayer.LClr.C=0xAAFFFFAA;       // предварительная раскладка
  ActiveLayer.AlphaBlend=0xAA;         // прозрачная проницаемость остекления
  ActiveLayer.Visible=true;            // слой видимый
  ActiveLayer.Developable=false;       // включать в раскрой обшивки
  ActiveLayer.Symmetric=false;         // пока без правого дублирования
  ActiveLayer.UseinHydrostatic=true;   // включать в гидродинамические расчёты
  ActiveLayer.UseforIntersection=true; // в построение теоретические контуров
  ActiveLayer.ShowInLineSpan=true;     // включается в теоретические чертежи
  ActiveLayer.MaterialDensity=1.0;     // плотность воды
  ActiveLayer.Thickness=1.0;           // если 1 - получится площадь
}
WCHAR *FName;                    // имя открытого файла в кодировке от Windows
FILE *FM=NULL;                   // пусть так будет единственно открытый файл
const char Future[]="FREE!ship"; // признак FREE!Ship цифровой модели Fbm и Ftm
static string Str;               // рабочая строчка изначально имеет 2К
bool isBin=false;                // признак двоичной и текстовой записи корпуса

bool freeShip::Read( const char *Name,_Real newDraught )  // смену осадки особо
{ strcpy( FileName,Name );
  FName=wcsdup( U2W( Name ) );
  if( !LoadExtFile(true) )
  if( !LoadProject() )Break( "? неопознанная числовая модель: %s ",Name );
  ShipName=fname( FileName );                // "Вариант иной числовой модели";
    //
    //     небольшая перенастройка визуализации сцены с кораблем
    //        обновлённая дальность, векторы ориентации и обзора
    //
 Real xi;
  View_initial( xi = 2.4*sqrt( sqr(Length)+sqr(Breadth*2)+sqr(Draught*4) ) );
  Distance=-xi;
  eyeX=45,eyeY=-15,eyeZ=0; lookX=-1,lookY=-1,lookZ=0;
  return true;
}
static bool OpenFile()           // открытие цифровой модели корпуса
{ char FTyp[14];
  FM=_wfopen( FName,L"rb" ); fread( FTyp,1,13,FM );
  if( !strncmp( Future,FTyp,9 ) )
    { fclose(FM); FM=_wfopen(FName,L"rt"); fgets(FTyp,13,FM); return false; }
  else if( ((int*)FTyp)[0]==9 && !strncmp( Future,FTyp+4,9 ) )return true;
  fclose( FM ); FM=NULL; return false;
}
static fixed getVersion()
{ const char *sVer[]={
 "1.0","1.1","1.2","1.3","1.4","1.5","1.6","1.65","1.7","1.8","1.9","1.91",
 "1.95","1.98","2.0","2.01","2.1","2.2","2.3","2.4","2.5","2.6","2.7+","2.8+",
 "2.94+","2.95+","2.96+","2.97+","2.98+","3.0+","3.02+","3.03+","3.08+","3.09+",
 "3.12+","3.13+","3.16+","3.27+","3.3+","3.34+","3.4","4.2","4.3","4.6.2","5.0","5.1" };
 const fixed nVer=sizeof( sVer )/sizeof( char* );     // =>46 количество версий
  if( isBin )return fgetc( FM ); else
  { char *str=getString( FM );                        // изначально в строке 2К
    fixed i; for( i=0; i<nVer && strcmp( str,sVer[i] ); i++ ); return i;
} }
static Real getFloat()
{ if( isBin ){ float R; fread( &R,1,4,FM ); return R; } else return atof( getString( FM ) );
}
Vector getPoint()
{ Vector P; if( isBin ){ P.x=getFloat(); P.y=getFloat(); P.z=getFloat(); }
  else sscanf( getString( FM ),"%lg%lg%lg",&P.x,&P.y,&P.z ); return P;
}
static void readText( char **src )
{ if( *src )free( *src ); // на входе-выходе просто адрес = строчка - недотрога
  if( !isBin )*src=strdup( getString( FM ) ); else
  { int l=0; fread( &l,1,4,FM );                Str[0]=0;
    for( int i=0; i<l; i++ )Str[i]=fgetc( FM ); Str[l]=0;
    *src=strdup( WintU( Str ) );
} }
static int S2I( char *s )
{ int I; for( I=0; I<strlen( s ); I++ )if( s[I]>' ' )break; s+=I; I=0;
  if( s[0]!='$' )sscanf( s,"%i",&I ); else sscanf( s+1,"%X",&I ); return I;
}
int getInt()
{ if( isBin ){ int I=0; fread( &I,1,4,FM ); return I; } return S2I( getString( FM ) );
}
static byte getByte()
{ if( isBin )return fgetc( FM ); else return atoi( getString( FM ) );
}
//   Полноценная числовая модель в варианте с базовым форматом, без излишеств
//
bool freeShip::LoadFEF()      // Ship.fef == FreeShip Exchange Format
{ int I; isBin=false; char *str=NULL;
  if( !(FM=_wfopen( FName,L"rt" ) ) )Break( "?не открывается free!Ship Exchange Format %s ",FileName );
  readText( &str );
  for( I=0; I<6 && str[I]; I++ )               // в первой строке текстовое имя
    if( !isdigit( str[I] ) ){ I=-1; break; }   // или сразу количество узлов
  if( I<=0 ){ ProjName=str;
    readText( &Designer );
    readText( &Comment );
    readText( &CreatedBy );
   Real WaterDensity=1.025,AppendageCoefficient=1;     // или по отсутствию
    Units=0;
    sscanf( getString( FM ),"%lg%lg%lg%lg%lg%d",       // "%d%d" --
     &Length,&Breadth,&Draught,
     &WaterDensity,
     &AppendageCoefficient,
     &Units ); //,&I,&PT );
  } else
  { I=atoi( str ); free( str );
  }
// TFreeSubdivisionSurface.ImportFEFFile
//
   ReadFEF( I );
   fclose( FM ); FM=NULL;
   return true;
}
//   чтение собственно секций всех сплайновых геометрических поверхностей
//
void freeShip::ReadFEF( int K )    // количество узлов или их <= одного
{ int I,NoC,NoL,NoI; isBin=false; // isLoad=true; First load layerdata
   NoL=NoLayers;
   if( K<=1 )
   { K=getInt();
     if( !K )K=getInt();
     NoLayers+=K;                            // счетчик слоёв здесь идет в начало
     L=(Layers*)Allocate( max(1,NoLayers+1)*sizeof( Layers ),L );
     for( I=NoL; I<NoLayers; I++ )
     { Layers &T=L[I]; int v,d,s,u,w,p; char str[12]="";
       readText( &T.Description );
       sscanf( getString( FM ),"%d%s%i%i%i%i%i%i%lg%lg",&T.ID,
         str,&v,&d,&s,&u,&w,&p,&T.MaterialDensity,&T.Thickness );
         T.Visible=v; T.LClr.C=S2I( str ); T.LClr.c[3]=255-T.LClr.c[3];
         T.Developable=d,
         T.Symmetric=s,
         T.UseforIntersection=u,
         T.UseinHydrostatic=w,
         T.ShowInLineSpan=p;
     } K=getInt();
   } else L=(Layers*)Allocate( (NoLayers+1)*sizeof( Layers ),L ); // сверх-слой
   L[NoLayers].LClr.C=0xAAAAAA88;                  // серенькая сверх-подсветка
   L[NoLayers].Symmetric=false; //true;
   NoC=NoCoPoint;
   NoCoPoint+=K;
   P=(CoPoint*)Allocate( NoCoPoint*sizeof( CoPoint ),P );
   for( I=NoC; I<NoCoPoint; I++ )
   { CoPoint &T=P[I]; int J=0;/*T.T=svRegular;*/ K=0; // последних может не быть
     sscanf( getString( FM ),"%lg%lg%lg%i%i",&T.V.x,&T.V.y,&T.V.z,&J,&K );
   }
   K=getInt();
   for( I=0; I<K; I++ )
   { int K1,K2,Ck;                               Ck=0;
     sscanf( getString( FM ),"%i%i%i%i",&K1,&K2,&Ck,&NoI );
   }
   NoI=NoFaces;
   NoFaces+=getInt();
   F=(Faces*)Allocate( NoFaces*sizeof( Faces ),F );
   for( I=NoI; I<NoFaces; I++ ) // здесь уж чтение напрямую из текстового файла
   { char *S=strtok( getString( FM )," " );
     K=0; sscanf( S,"%i",&K ); F[I].Capacity=K;
     F[I].P=(int*)Allocate( K*sizeof(int) );             /// <++ Control Points
     for( int j=0; j<K; j++ )
        { int &M=F[I].P[j]; S=strtok( 0," " ); sscanf( S,"%i",&M ); M+=NoC; }
     K=0; S=strtok( 0," " ); if( S )sscanf( S,"%i",&K ); F[I].LayerIndex=K+NoL; // № слоя по площадке
   //K=0; S=strtok( 0," " ); if( S )sscanf( S,"%i",&K ); F[I].Selected=K!=0;    // метка выбора
   }
   //
   //  теперь выборка загибулин - мимо
   //
   Extents();               // расчёт - переопределение графических экстремумов
}
//   Основная процедура считывания цифрового проекта корабля
//
bool freeShip::LoadProject()
{ int N=0; isBin=OpenFile();
   if( !FM )return false; //Break( "LoadProject: %s не открыт",FileName );
   FV=FileVersion( getVersion() );
   /*PT=(PrecisionType)    */ getInt();
   /* ModelView=(BoardView)*/ getInt(); // Сначала параметры визуализации Visio
   /* ControlNet=          */ getByte();
   /* InteriorEdges=       */ getByte();
   /* Stations=            */ getByte();
   /* Buttocks=            */ getByte();
   /* Waterlines=          */ getByte();
   /* Normals=             */ getByte();
   /* Grid=                */ getByte();
   /* Diagonals=           */ getByte();
   /* Markers=             */ getByte();
   /* Curvature=           */ getByte();
   /* CurvatureScale=      */ getFloat();
   if( FV>=fv195 )
   { /* ControlCurves=     */ getByte();
     if( FV>=fv210 )
     { /* CursorIncrement= */ getFloat();
       if( FV>=fv220 )
       { /* HydrostaticData=          */ getByte();
         /* HydrostDisplacement=      */ getByte();
         /* HydrostLateralArea=       */ getByte();
         /* HydrostSectionalAreas=    */ getByte();
         /* HydrostMetacentricHeight= */ getByte();
         /* HydrostLCF=               */ getByte();
         if( FV>=fv250 )
         { /* lFlowline=              */ getByte();
         } //=250
       }   //=220
     }     //=210
   }       //=195
   //
   //  Затем описание характеристик и размерений корпуса с авторскими ссылками
   //
   readText( &ProjName );
   readText( &Designer );
   Length=getFloat();
   Breadth=getFloat();
   Draught=getFloat();
   /* Set.MainparticularsHasBeenset= */ getByte();
   /* Set.WaterDensity=              */ getFloat();
   /* Set.AppendageCoefficient=      */ getFloat();
   /* Set.ShadeUnderwaterShip=       */ getByte();
      UnderWaterColor.C=getInt();
      UnderWaterColorAlpha=UnderWaterColor.c[3]=255-UnderWaterColor.c[3];
      Units=getInt(); // 0 - метрик, иначе - империал
   /* Set.UseDefaultSplitSectionLocation= */ getByte();
   /* Set.SplitSectionLocation=           */ getFloat();
   /* if(FV>=fv165)Set.DisableModelCheck= */ getByte();
   readText( &Comment );
   readText( &CreatedBy );
   if( FV>=fv210 )
   {  getInt();        // Set.HydrostaticCoefficients=(HydrostaticCoefficient)
     if( getByte() )   // Set.SavePreview )
     { int Size; getInt(); getInt(); Size=getInt();
       if( isBin )fseek( FM,Size,SEEK_CUR ); else getString( FM );
     }
     if( FV>=fv230 )
     { /* Set.SimplifyIntersections= */ getByte();
       if( FV>=fv250 )
       { /* Set.StartDraft= */ getFloat();
         /* Set.EndDraft=   */ getFloat();
         /* Set.DraftStep=  */ getFloat();
         /* Set.Trim=       */ getFloat();
         /* Set.NoDisplacements= */ N=getInt();
         for( int i=0; i<N; i++ )/* Set.Displacements[i]= */getFloat();
         /* Set.MinimumDisplacement= */ getFloat();
         /* Set.MaximumDisplacement= */ getFloat();
         /* Set.DisplIncrement=      */ getFloat();
         /* Set.UseDisplIncrements=  */ getByte();
         /* Set.NoAngles= */ N=getInt();
         for( int i=0; i<N; i++ )/* Set.Angles[i]=*/ getFloat();
         /* Set.NoTrims= */ N=getInt();
         for( int i=0; i<N; i++ )/* Set.Trims[i]=*/ getFloat();
         /* Set.FreeTrim=*/ getByte();
         /* Set.FVCG=*/     getFloat();
         if( FV>=fv317 )
         { /* Set.EnableModelAutoMove= */ getByte();
           if( FV>=fv332 )
           { /* Set.EnableBonjeanSAC= */ getByte();
           } //=332
         }   //=317
       }     //=250
     }      /* =230 */
     if( FV>=fv500 )UnderWaterColorAlpha=getInt();
   }     ///* =210
   //
   //!   чтение основных данным по обводам корабля - форме корпусной обшивки
   //
   freeRead();
   //
   //!   ... и всякое сбоку-припёку - мимо
   //
   fclose( FM ); FM=NULL; return true;
}
//!  считывание собственно секций всех сплайновых геометрических поверхностей
//
void freeShip::freeRead( bool Part )
{ int I,J,K,N,NoI,NoL,NoC,EdErr;                     // isLoad=true;
   NoL=NoLayers;                                     // все новые слои вдогонку
   N=getInt();
   NoLayers+=N;
   L=(Layers*)Allocate( NoLayers*sizeof( Layers ),L );
   if( N )
   { for( I=NoL; I<NoLayers; I++ )
     { readText( &(L[I].Description) );
       L[I].ID=getInt();
       L[I].LClr.C=getInt(); L[I].LClr.c[3]=255-L[I].LClr.c[3];
       L[I].Visible=getByte();
       L[I].Symmetric=getByte();
       L[I].Developable=getByte();
       if( FV>=fv180 )
       { L[I].UseforIntersection=getByte();
         L[I].UseinHydrostatic=getByte();
         if( FV>=fv191 )
         { L[I].MaterialDensity=getFloat();
           L[I].Thickness=getFloat();
           if( FV>=fv201 )
           { L[I].ShowInLineSpan=getByte();
             if( FV>=fv261 )L[I].AlphaBlend=getInt();
           } // =201
         }   // =191
       }     /* =180 */
     }       // for
   }
   //  ... здесь начинается разборка со сплайнами и кривыми Безье-поверхностями
   //
   if( !Part )I=getInt();
   NoC=NoCoPoint;
   NoCoPoint+=getInt();
   P=(CoPoint*)Allocate( NoCoPoint*sizeof( CoPoint ),P );
   for( I=NoC; I<NoCoPoint; I++ )
   { P[I].V=getPoint();
     /* P[I].T = (VertexType)  */ getInt();  // тип точки: угловая и т.п.
     /* P[I].Selected=         */ getByte(); // отмечена жёлтым выбором (чушь.)
     if( FV>=fv198 )/* P[I].Locked=*/ getByte(); // и бит блокировки (зачем-то тоже)
   }
   K=getInt();
   for( I=0; I<K; I++ )
   { getInt();
     getInt();  /* Edge.Crease = */ getByte();
     if( !Part )/* Edge.Selected = */ getByte();
   }
   if( !Part )
   if( FV>=fv195 )
   {
nPart: J=getInt();
     for( I=0; I<J; I++ ){ getInt();
       for( int j=0; j<K; j++ )getInt();
       if( !Part )getByte();
     } if( Part )goto Ret;
   }
   NoI=NoFaces;
   NoFaces+=getInt(); EdErr=0;
   F=(Faces*)Allocate( NoFaces*sizeof( Faces ),F );
   for( I=NoI; I<NoFaces; I++ )
   { K=getInt();
     F[I].Capacity=K;
     F[I].P=(int*)Allocate( K*sizeof(int) );    // <++ Control Points
     for( int j=0; j<K; j++ )
     { int m=getInt();
       if( m+NoC>=NoCoPoint || m<0 )EdErr++;
       if( m==-1 )m=0; F[I].P[j]=m+NoC;
     } F[I].LayerIndex=getInt()+NoL;
     if( !Part )/*F[I].Selected=*/getByte();
   }
   if( Part )goto nPart;
Ret: Extents();             // расчёт - переопределение графических экстремумов
}
void freeShip::Extents( bool Sizes )                // Экстремумы по всем контрольным точкам
{ for( int i=0; i<NoCoPoint; i++ )
  if( !i )Min=Max=P[0].V; else MinMax( Min,Max,P[i].V );
  if( !Sizes || !NoLayers ){ Length=Max.x-Min.x,Breadth=Max.y-Min.y,Draught=-Min.z; }
  for( int i=0; i<NoCoPoint; i++ )P[i].V.z-=Draught+Min.z,
                                  P[i].V.x-=(Max.x+Min.x)/2;
}
//
//   левые приблуды к открытому frreShip формату корабельной поверхности
//
bool freeShip::LoadExtFile( bool New )
{ int L;
  if( !New )
  { FM=FileOpen( FileName, L"rb", L"part",      // простая выборка нового имени
    L"Ship [*.fef *.part *.obj *.stl]\1*.fef;*.part;*.obj;*.stl\1"
     "[ free!Ship Exchange Format ].fef\1*.fef\1"      // с заменой заголовков
     "[ Дельная вещь или фрагмент ].part\1*.part\1"    // записи без излишеств
     "[ WaveFront Technologies ].obj\1*.obj\1"         // ~ Advanced Visualizer
     "[ Stereolithography ].stl\1*.stl\1"              // ~ Standard Triangle/+
     "Все файлы (*.*)\1*.*\1\1",                       // Tessellation Language
    L"? Считывание бортовых конструкций и специальных корабельных секций" );
    if( FM!=NULL ){ fclose( FM ); FM=NULL; }
  }
  L=strlen( FileName ); //wcscpy( FName,U2W( FileName ) );// если New остановка
  FName=wcsdup( U2W( FileName ) );
//if( L>5 && strcmp( strlwr( FileName+L-5 ),".part" )==0 )LoadPart( New ); else
  if( L>4 && strcmp( strlwr( FileName+L-4 ),".fef" )==0 )LoadFEF(); else // с заменой заголовков
  if( L>4 && strcmp( strlwr( FileName+L-4 ),".obj" )==0 )Import(1); else // в дополнение
  if( L>4 && strcmp( strlwr( FileName+L-4 ),".stl" )==0 )Import(2); else return false;
  return true;
}
inline WCHAR* Slower( WCHAR *str )
{ int l=wcslen( str ); while( --l>=0 )str[l]=towlower( str[l] ); return str;
}
inline char* Slower( char *str ) // пригодно лишь к латинским буквочкам
{ int l=strlen( str ); while( --l>=0 )str[l]=tolower( str[l] ); return str;
}
bool freeShip::Import( fixed Fmt )
{ isBin=false;             // чисто текстовое представление числовой информации
  if( !(FM=_wfopen( FName,L"rt" ) ) )
  { print( "?\7 не открывается инородный %s ",W2U(FName) ); getch(); exit( 2 );
  }
 time_t lt=time(0); char *S=asctime( gmtime( &lt ) ); strcut( S );
  if( !ProjName )ProjName=strdup( ShipName ); // ~ без вычистки
  if( !Designer )Designer="@2026-Ship.exe viewer for „free!Ship“";
  if( !Comment  )Comment="Application for «Аврора» stormy seakeeping of ship";
  if( !CreatedBy)CreatedBy=S;
//if( !CreatedBy)Set.CreatedBy=ctime( &lt );
  if( Fmt==2 )ReadStl(); else
  if( Fmt==1 )ReadObj();
  fclose( FM ); FM=NULL;
//Visio.BothSides=false;
//BoardView( Visio.ModelView )=mvPort;
//Visio.ModelView=mvPort;
  Extents( false );   // расчёт - переопределение графических экстремумов
  if( Min.z<0 && Max.z>0 )Draught=-Min.z;
  return true;
}
void freeShip::ReadObj()                      // временный оригинал имени файла
{ char *S,*MtName=strdup( FileName ); Real r,g,b,a; char *s; // ссылка не текст
  print( "\nОткрыт WaveFront файл: %s",FileName );       // в буфере файла
 int NoL=NoLayers,                      // уровни будут дополняться сверху
     NoC=NoCoPoint-1;                   // узловые точки отделяются от прошлого
  ActiveLayer.Description="WaveFront";  // Technologies Advanced Visualizer";
  ActiveLayer.ID=NoL;                   // изначально здесь ноль
  ActiveLayer.Symmetric=false;          // пока без правого дублирования
  if( !NoLayers )  // на случай отсутствия послойного описания свойств, будет 1
  { L=(Layers*)Allocate( (NoLayers+1)*sizeof( Layers ),L );
    memcpy( &L[NoLayers],&ActiveLayer,sizeof( Layers ) );
  }
  while( !feof( FM ) )
  { if( (s=strchr( S=getString( FM ),'#' ))!=NULL )*s=0;
    if( strcut( S )<3 )continue;
    S[0]=tolower( S[0] );        // сначала первый символ, а затем и вся строка
    if( !strncmp( S,"v ",2) )
    { P=(CoPoint*)Allocate( ++NoCoPoint*sizeof( CoPoint ),P );
     CoPoint &p=P[NoCoPoint-1];
//    sscanf( S+2,"%lg%lg%lg",&p.V.y,&p.V.x,&p.V.z ); p.V.x=-p.V.x; // Новик здесь
      sscanf( S+2,"%lg%lg%lg",&p.V.x,&p.V.z,&p.V.y ); // так готовится в Авроре #+# p.V.y=-p.V.y;
//    p.T=svRegular; // svCrease; // svDart; // svCorner;
    } else
    if( !strncmp( S,"f ",2) )
    { char *v,*w=S+2;
      int k=0,*Rc=(int*)calloc( sizeof( int ),4 ); // Allocate не для маленьких
      do{ s=strchr( w,' ' ); if( s )*s=0;
          v=strchr( w,'/' ); if( v )*v=0;
          if( k>3 )Rc=(int*)realloc( Rc,sizeof(int)*(k+1) );
          while( *w<=' ' )w++;
          Rc[k]=atoi( w )+NoC; w=s+1; ++k;  // нормали получаются задом наперёд
      } while( s );
      if( k>2 )                             // наверняка прямые так не строятся
      { F=(Faces*)Allocate( ++NoFaces*sizeof(Faces),F );
        Faces &f=F[NoFaces-1]; f.Capacity=k;
                               f.P=Rc;
                               f.LayerIndex=ActiveLayer.ID;
        L[ActiveLayer.ID].ID++;
      }
    } else
    if( !strncmp( Slower( S ),"usemtl",6 ) ) // Slower дале уже готов для всех
    { ActiveLayer.ID=-1;
      if( NoL<NoLayers )
      for( int i=NoL; i<NoLayers; i++ )
      if( !strcmp( S+7,L[i].Description ) )
        { ActiveLayer.ID=i; break;             //  ... или первый из попавшихся
        }
      if( ActiveLayer.ID==-1 )    // если слой не найден, тогда создание нового
      { L=(Layers*)Allocate( ++NoLayers*sizeof( Layers ),L );
        L[NoLayers-1]=ActiveLayer; // memcpy( &L[NoLayers-1],&ActiveLayer,sizeof( Layers ) );
        L[NoLayers-1].Description=strdup( S+7 );         // новое имя по ссылке
        L[NoLayers-1].ID=0; ActiveLayer.ID=NoLayers-1;   // NoLayers;
      }
    } else
    if( !strncmp( S,"mtllib",6 ) )  // разборка расцветки по уровням расслоений
    { strcpy( fname( MtName ),S+7 );
     FILE *W=_wfopen( U2W( MtName ),L"rt" );   // файл.mtl может быть перепрочтён
      if( !W )print( "\n? %s не открывается.\n",MtName ); else
      { while( !feof( W ) )
        { if( (s=strchr( S=getString( W ),'#' ))!=NULL )*s=0;
          if( strcut( S )<3 )continue;
          if( !strncmp( Slower( S ),"newmtl",6 ) )
          { L=(Layers*)Allocate( ++NoLayers*sizeof( Layers ),L );
            L[NoLayers-1]=ActiveLayer; // memcpy( &L[NoLayers-1],&ActiveLayer,sizeof( Layers ) );
            L[NoLayers-1].Description=strdup( S+7 );
            L[NoLayers-1].ID=0; /*NoLayers;*/ } else
          if( !strncmp( S,"kd ",3 ) )
          { Color &c=L[NoLayers-1].LClr;        c.c[3]=0xFF;
            sscanf( S+3,"%lg%lg%lg",&r,&g,&b ); c.c[2]=byte( b*255 );
                                                c.c[1]=byte( g*255 );
                                                c.c[0]=byte( r*255 ); } else
          if( !strncmp( S,"d ",2 ) )
          { sscanf( S+2,"%lg",&a ); L[NoLayers-1].LClr.c[3]=byte( 22+a*220 ); } //! [22-222] - пусть пока временно
        } fclose( W );
      }
    }
  }    int j=0;
  for( int i=0; i<NoLayers; i++ )if( L[i].ID>0 )
  { for( int k=0; k<NoFaces; k++ )if( F[k].LayerIndex==i )F[k].LayerIndex=j;
    if( i!=j )L[j]=L[i];
    j++;
  } NoLayers=j;
//  Extents( false );       // расчёт - переопределение графических экстремумов
}
void freeShip::ReadStl()                      // временный оригинал имени файла
{ char *S,T[80]; Vector N;
  if( !(S=fgets( T,6,FM )) )return;
  if( memcmp( S,"solid",5 ) )goto binarySTL; //! с двоичными файлами чуть позже
  print( "\nОткрыт Triangles.ascii файл: %s",FileName ); rewind( FM );
  //
  //   Standard Triangle Library - ascii
  //
  while( !feof( FM ) )                       // первая строка "solid имя" мимо
  { while( !strcut( S=getString( FM ) ) );
    if( !memcmp( S,"solid",5 ) )               // начинаем считывание франмента
    { L=(Layers*)Allocate( ++NoLayers*sizeof( Layers ),L );
      memcpy( &L[NoLayers-1],&ActiveLayer,sizeof( Layers ) );
      L[NoLayers-1].Description=strdup( S+6 );
      L[NoLayers-1].Symmetric=true;
      L[NoLayers-1].ID=NoLayers-1;
      while( !feof( FM ) )
      { while( !strcut( S=getString( FM ) ) );   // пропуск непустой строки
//      if( !(S=getString( FM ) ) )break;        //        facet normal x,y,z
        if( !memcmp( S,"endsolid",8 ) )break;     // конец данных по фрагменту
        sscanf( S,"%s %s %lg%lg%lg",T,T,&(N.x),&(N.y),&(N.z) );
        if( !(S=getString( FM ) ) )break;        //         пропуск outer loop
        P=(CoPoint*)Allocate( (NoCoPoint+=3)*sizeof( CoPoint ),P );
        F=(Faces*)Allocate( ++NoFaces*sizeof(Faces),F );
       Faces &f=F[NoFaces-1];
        f.P=(int*)Allocate( 3*sizeof( int ) );
        f.Capacity=3;
        f.LayerIndex=NoLayers-1;
        for( int i=0; i<3; i++ )
        { if( !(S=getString( FM ) ) )break;  // последовательно читаются x,y,z
         Vector &V=P[NoCoPoint+i-3].V;
          sscanf( S,"%s %lg%lg%lg",T,&(V.x),&(V.y),&(V.z) );
          f.P[i]=NoCoPoint+i-3;
        }
        if( +N )
        { Vector &W=P[NoCoPoint-1].V;        // контроль ориентации лишь путает
          if( ((P[NoCoPoint-2].V-W)
              *(P[NoCoPoint-3].V-W))%N>0 ){ int C=f.P[0]; f.P[0]=f.P[2]; f.P[2]=C; }
        }
        if( !(S=getString( FM ) ) )break;        // endloop
        if( !(S=getString( FM ) ) )break;        // endfacet
    } }      //
  } return;  // Standard Triangle Library - binary
binarySTL:   //
 int n,NoL=NoLayers; fixed c;
  fclose( FM ); if( !(FM=_wfopen( FName,L"rb" ) ) )return; // ~ переоткрытие
  print( "\nОткрыт Triangles.binary файл: %s",FileName );
  fread( T,1,80,FM );                     // Читается заголовок (80 байт)
         T[79]='\0';                      // гарантируется null-termination
  isBin=true; n=getInt();                 // Читается количество треугольников
                                          // Выделяем память всем треугольникам
  P=(CoPoint*)Allocate( (NoCoPoint+n*3)*sizeof( CoPoint ),P );
  F=(Faces*)Allocate( (NoFaces+n)*sizeof(Faces),F );
  for( int i=0; i<n; i++ )          // Читаем все треугольники
  { N=getPoint();
   Faces &f=F[NoFaces+i]; int j=NoCoPoint+i*3;
    f.P=(int*)Allocate( 3*sizeof( int ) );
    f.Capacity=3;
    P[j].V = getPoint(); f.P[0]=j;
    P[j+1].V=getPoint(); f.P[1]=j+1;
    P[j+2].V=getPoint(); f.P[2]=j+2;
    if( +N )
    { Vector &W=P[j].V;                  // контроль ориентации лишь запутывает
      if( ((P[j+1].V-W)
          *(P[j+2].V-W))%N<0 ){ int C=f.P[0]; f.P[0]=f.P[2]; f.P[2]=C; }
    }
   Color C;
    fread( &c,2,1,FM );                 // расцветка разбирается в расслоениях
    if( c&0x8000 )
    { C.C=0;
      C.c[0]=(255*( c&0x1F ))/31;
      C.c[1]=(255*((c>>=5)&0x1F))/31;
      C.c[2]=(255*((c>>=5)&0x1F))/31; C.c[3]=0xFF;
    } else C.C=ActiveLayer.LClr.C;
   bool k=false; j=NoL;
    while( j<NoLayers )
    { if( L[j].LClr.C==C.C ){ f.LayerIndex=j; k=true; break; } ++j; } j=NoLayers;
    if( !k )                                              // j==NoLayers )
    { L=(Layers*)Allocate( ++NoLayers*sizeof( Layers ),L );
      memcpy( &L[NoLayers-1],&ActiveLayer,sizeof( Layers ) );
      sprintf( T,"%i Цвет: r:%i,g:%i,b:%i - %i",j,C.c[0],C.c[1],C.c[2],C.c[3] );
      L[j].Symmetric=true;
      L[j].Description=strdup( T );
      L[j].ID=NoLayers-1;                         //      C.c[3]=0xFF;
      L[j].LClr=C;
    }
  } NoCoPoint+=n*3;
    NoFaces+=n;
/*if( !NoLayers )  // случай отсутствия послойного описания свойств, будет 1
  { L=(Layers*)Allocate( (NoLayers+1)*sizeof( Layers ),L );
    memcpy( &L[NoLayers],&ActiveLayer,sizeof( Layers ) );
  } */
}
//
//  изображение на волне
//  прорисовка исходных многоугольников
//
#define uWater { c.c[3]-=c.c[3]/6; c.c[0]=( c.c[0]+UnderWaterColor.c[0] )/2; \
                                   c.c[1]=( c.c[1]+UnderWaterColor.c[1] )/2; \
                                   c.c[2]=( c.c[2]+UnderWaterColor.c[2] )/2; }

static void DrawL( Flex &Cont, int i1,int i2, _Real delta, bool right ) // i1-i2 включительно
{ Vector V,W=Zero;
  for( int i=i1+1; i<i2; i++ )W+=(Cont[i+1]-Cont[i1])*(Cont[i]-Cont[i1]);
  glNormal3dv( W );          // в гидромеханике этот расчёт должен быть здесь
  glBegin( GL_POLYGON );
    for( int i=i1; i<=i2; i++ ){ (V=Cont[i]).z+=delta; dot( V ); }
  glEnd();
  if( right )
  { W.y=-W.y; glNormal3dv( W );
    glBegin( GL_POLYGON );
    for( int i=i2; i>=i1; i-- ){ (V=Cont[i]).z+=delta; V.y=-V.y; dot( V ); }
    glEnd();
  }
}
inline bool inInter( _Vector V1, _Vector V2 )
{    return (V1.z>=0 && V2.z<0) || (V2.z>=0 && V1.z<0);
}
inline Vector newInter( _Vector V1, _Vector V2 )
{    return V1 - V1.z*( V2-V1 )/( V2.z-V1.z );
}
void freeShip::freeDraw( BoardView Sides )
{ const Real delta=Draught+Min.z;
//    Flex &W=WaterLine;
 static Flex W,wL;
 int K; Color c; wL.len=0;
  for( int N=0; N<NoFaces; N++ ) // синхронная прорисовка треугольников двух бортов
  if( (K=F[N].Capacity)>2 )             // у граней должно быть боле двух рёбер
  { const Layers &Layer=L[min(NoLayers,F[N].LayerIndex)]; // указанные свойства
    const bool right=(Sides==mvBoth && Layer.Symmetric);
    glLineWidth( 1 );    /// Alice AI из Яндекса стала эдесь хорошим помощником
 #if 1
   Vector v,V1,V2; int J=-1,i=0; W.len=0;
    while( i<=K+J )
    { (V2=P[F[N].P[i%K]].V).z-=delta;
      if( i++ )                        // i - показывает следующий узел = длину
      if( inInter( V1,V2 ) )           //     однократно, но по всем рёбрам
      { if( J<0 || W.len==0 ){ v=V1; if( J<0 )J=i,W.len=0; } else v=V2;
        if( v.z==0.0 )W+=v; else W+=newInter( V1,V2 );
        if( W.len>1 )
        { if( W.len>2 )  //--- невидимые двойки пусть нарисуются, аль нет {wL}?
          { c.C=Layer.LClr.C;
            if( W[1].z<0 )uWater else wL+=W[0],wL+=W[-1]; glColor4ubv( c.c );
            DrawL( W,0,W.len-1,delta,right );
          } W[0]=W[-1]; W.len=1;
      } }
      if( J<0 || W.len>0 )W+=V2; V1=V2; // J<0 по началу, и от пересечения нуля
    }
    if( J<0 ) // if( W.len>0 )        // c заданной расцветкой для каждой грани
    { c.C=Layer.LClr.C;
      if( W[0].z<0 )uWater glColor4ubv( c.c ); DrawL( W,0,W.len-1,delta,right );
    }
#else
   Flex V; int i=0,j=0,J=0; W.len=0;
    for( int I=0; I<K; I++ )(V+=P[F[N].P[I]].V).z-=delta;
    while( i<=K+J ){ int I=i%K,I1=(i+K-1)%K; i++;
      if( inInter(V[I1],V[I]) )
      { if( W.len )j=I; else { j=I1; if( !J )J=I+1; }
        if( V[j].z==0.0 )W+=V[j]; else W+=newInter( V[I1],V[I] );
        if( j==I )
        { c.C=Layer.LClr.C;
          if( W[1].z<0 )uWater else wL+=W[0],wL+=W[-1]; glColor4ubv( c.c );
          DrawL( W,0,W.len-1,delta,right ); W.len=0; --i; // одной левой точкой
      } } if( W.len )W+=V[I];
    }
    if( !W.len ){ c.C=Layer.LClr.C; if( V[0].z<0 )uWater; glColor4ubv( c.c );
                  DrawL( V,0,V.len-1,delta,right );
                }
#endif
  }
  for( int i=0; i<wL.len; i++ )wL[i].z+=delta; color( white ); glLineWidth(2);
  glDisable( GL_LIGHTING );                       // glNormal3d( 0,0,-1 );
  for( int i=0; i<wL.len; i+=2 )                  // белая ватерлиния 1|2 борта
// if( Sides!=mvBoth )line( wL[i],wL[i+1] ); else
                     liney( wL[i],wL[i+1] );
/*color( lightmagenta );                          // кривые контрольные контуры
  for( int K=0; K<NoCurves; K++ )
  { Vector V,W;
    for( int I=0; I<C[K].Capacity; I++ ){ W=P[C[K].P[I]].V;
      if( I ){ if( Sides==mvBoth )liney( V,W ) ; else line( V,W ); } V=W;
    }
  }*/
  glEnable( GL_LIGHTING ); glLineWidth( 0.2 );
}

extern int //Board=0,       // 'о' штевни; '-' левый и '+' правый борт
       Level, //=-2,        // -2-днище -1-вода 0-ватерлиния 1-смочен 2-сухой
       wLine; //=1;         // -1-ниже; +1-выше цвета конструктивной ватерлинии
extern bool Part, //=false, // false= днище и ватерлиния; true= надводный борт
        drawHull; //=false; // прорисовка корпуса | гидродинамический процесс
extern Color Cx;

freeShip& freeShip::Floating( bool onlyDraw ) // кинематика на морском волнении
{
  if( onlyDraw )
  {
    glEnable( GL_LIGHTING );
//    freeDraw();
//    return *this;
  }



  int i; Part=false;              // разделение корпуса на прозрачные подуровни
       drawHull=onlyDraw;       // копия режима расчетов(-) или прорисовки(+)
  if( !onlyDraw )ThreeInitial();  // начальная чистка для интегрируемых величин
  wL.len=0;      // ватерлиния с нормалями и запутанными разделёнными отрезками
Part_of_hull:    // разделение корпуса по уровням надводной и смоченной обшивки

  if( onlyDraw )glEnable( GL_LIGHTING );

  for( int i=0; i<NoFaces; i++ )
  { int *k;
    const Layers &Layer=L[min(NoLayers,F[i].LayerIndex)]; // указанные свойства
    const bool right=Layer.Symmetric && Min.y>=-eps;      // && Sides==mvBoth
    Vector &A=P[*(k=F[i].P)].V;
    Cx=Layer.LClr;
    for( int j=1; j<F[i].Capacity-1; j++,k++ )
    { Vector &B=P[k[1]].V,
             &C=P[k[2]].V; Triangle( A,B,C );
      if( right )Triangle( ~A,~C,~B );
    }
  }

  if( !Part ) //   теоретическая и действующая ватерлиния готовятся с нормалями
  { static Flex W; Vector wM,fM; Real l,L; bool C; const Real dw=1e-4; // 0.1мм
    if( !KtE )                   // конструктивная или теоретическая ватерлиния
      for( WaterLine.len=i=0; i<wL.len; i++ )WaterLine+=wL[i]; Level=0;
    while( wL.len )
    { W.len=0; C=false;
      W+=wL[-2],W+=wL[-1]; i=(wL.len-=3)-3;
      while( !C && i>=0 )
      { if( abs( W[0]-wL[i+1] )<dw )W/=wL[i+2]; else
        if( abs( W[0]-wL[i+2] )<dw )W/=wL[i+1]; else
        if( abs( W[-1]-wL[i+1] )<dw )W+=wL[i+2]; else
        if( abs( W[-1]-wL[i+2] )<dw )W+=wL[i+1]; else { i-=3; continue; }
        if( onlyDraw )                  // стрелочки вдоль и поперёк ватерлинии
        { const Real aL=Draught/12;
          Vector q=(wL[i+1]+wL[i+2])/2;
          arrow( out( wL[i+1] ),out( wL[i+2] ),0.1,navy );
          arrow( out( q ),out( q+aL*dir( wL[i] ) ),0.2,gray );
        }
        wL.Delete( i ),wL.Delete( i ),wL.Delete( i );
        C = W.len>2 && abs( W[0]-W[-1] )<dw; if( C )break;  i=wL.len-3;
      }
      if( !C )W+=W[0];;
      L=0.0; wM=0.0; fM=0.0;     // c разделением разрывных контуров ватерлинии
      for( i=0; i<W.len-1; i++ )
        { L+=(l=abs( W[i]-W[i+1] )); wM+=l*( W[i]+W[i+1] ); }  // длина контура
      if( L>eps )wM/=2*L;                                   // центр ватерлинии
        for( i=0; i<W.len-1; i++ )fM+=(W[i]-wM)*(W[i+1]-wM); // площадь к знаку
      if( Zenit%( LtA( fM ) )<0.0 )                  // ориентация по вертикали
        for( i=0; i<W.len-1; i++ )drawTriangle( W[i+1],W[i],wM ); else
        for( i=0; i<W.len-1; i++ )drawTriangle( W[i],W[i+1],wM );
    }
//    else
//    { wM=0.0; L=0.0;          // без слияния контуров ватерлинии - простенько
//      for( i=0; i<wL.len; i+=3 )
//      { l=abs( wL[i+2]-wL[i+1] ); L+=l; wM += 0.5*l*( wL[i+1]+wL[i+2] ); }
//      if( L>eps )wM/=L;
//      for( i=0; i<wL.len; i+=3 )drawTriangle( wL[i+1],wL[i+2],wM );
//    }
    // завершение геометрической графики просто белая конструктивная ватерлиния
    //
    if( onlyDraw )
    { color( !Trim?white:cyan ); glLineWidth( 5 ); // silver
      for( i=0; i<WaterLine.len; i+=3 )
         line( out( WaterLine[i+1] ),out( WaterLine[i+2] ) ); glLineWidth( 1 );
    }
    // действующая ватерлиния выстраивается из фрагментов пересечения
    // треугольников в шпациях, с нормалями и стрелками вперед по курсу корабля
/*
#define Wline( L )for( i=0; i<L.len-2; i+=2 ){ q=( (L[i]+L[i+2])*0.5 );   \
 arrow(out(L[i]),out(L[i+2]),ArLen*.67),arrow(out(q),out(q+L[i+1]),ArLen*.5); }
      color( lightblue,DrawMode&3?0.0:0.3 ); Wline( wR ) Wline( wL )
*/
    Part=true;                       // однократный возврат к перерисовке
    if( onlyDraw )goto Part_of_hull; // только надводного борта, здесь Course=x
  }
  //
  // выборка и расчёт обновленных параметров корпуса, увеличение счетчика цикла
  //
  if( !onlyDraw )ThreeFixed(); drawHull=false; return *this;
}
