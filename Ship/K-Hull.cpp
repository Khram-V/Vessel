//
//     Программа математического определения (моделирования)
//          формы корпуса судна, с определением базовых
//                функций для его описания:
//
//      Real K_Hull  ( Real z, Real x );  // Корпус
//      Real K_Astern( Real z );          // Ахтерштевень
//      Real K_Stern ( Real z );          // Форштевень
//
//                  1997, январь, В.Храмушин, Южно-Сахалинск
//                  2001, август - Владивосток, октябрь - ГС-210
//                  2018, июль - Санкт-Петербург, ПМ-ПУ СПбГУ
//                  2020 - ‏יְרוּשָׁלַיִם‏‎ .
#include <stdio.h>
#include "Hull.h"

static char *Str=(char*)"\0\n<<МИДВ-85>>\n©1975-2024, В.Храмушин,"
                      " Сахалин - Санкт-Петербург  - ‏יְרוּשָׁלַיִם‏‎ - \n",
            *FileName=0;
static FILE *Fh=NULL;       // файл исходных данных по таблице плазовых ординат
static void FGetS();        // чтение строки с переадресацией на внешнюю память
static void FPutS( const char _Comm[], const char fmt[], ... );
/*
static Real maxY( Frame &F )           // экстремумы функции от исходных точек
     { Real Y=F[0]; for( int i=1; i<=F.N; i++ )if( Y<F[i] )Y=F[i]; return Y; }
static Real minY( Frame &F )
     { Real Y=F[0]; for( int i=1; i<=F.N; i++ )if( Y>F[i] )Y=F[i]; return Y; }
*/
Hull::Hull(): Ns( 0 ),Ms( 1 ),Nstem( 2 ),F( 0 )
          { ( Name=(char*)malloc( MAX_PATH*4 ) )[0]=0; }
// Hull::~Hull(){ allocate( 0 ); free( Name ); }
//
//      Считывание/запись численного описания формы корпуса
//        (при первом обращении возвращается корпус МИДВ)
//
void Hull::allocate( int N )
   { F=(Frame*)Allocate( (Ns=N)*sizeof(Frame),F ); Ms=Ns/2;
   }
void Hull::Simple_Hull( int Nx, int Nz, int Nf )
{ for( int i=Nx; i<Ns; i++ )F[i].allocate( 0 ); allocate( Nx );
  for( int i=0; i<Ns; i++ )F[i].allocate( Nz ); Ms=Ns/2;
  Stx.allocate( Nf ),Sty.allocate( Nf ),
  Asx.allocate( Nf ),Asy.allocate( Nf );
}
void Hull::Aphines( _Real cX,_Real cY,_Real cZ )
{ Xo*=cX; Xm*=cX; Length*=cX;  Lwl*=cX; Lmx*=cY;
                  Breadth*=cY; Bwl*=cY; Bmx*=cY;
  Do*=cZ;         Draught*=cZ; Depth*=cZ;
  Volume *= cX*cY*cZ;
  Surface*= pow( cX*cY*cZ,2.0/3.0 ); // (cX*cX+cY*cY+cZ*cZ)/3.0;
  for( int k=0; k<Ns; k++ )
  { for( int i=0; i<=F[k].N; i++ )F[k].z[i]*=cZ,F[k].y[i]*=cY;
    F[k].X*=cX; F[k].SpLine();
  }
  for( int i=0; i<=Stx.N; i++ )Stx[i]*=cX,Stx(i)*=cZ; //Stx.Easy();
  for( int i=0; i<=Asx.N; i++ )Asx[i]*=cX,Asx(i)*=cZ; //Asx.Easy();
  for( int i=0; i<=Sty.N; i++ )Sty[i]*=cY,Sty(i)*=cZ; //Sty.Easy();
  for( int i=0; i<=Asy.N; i++ )Asy[i]*=cY,Asy(i)*=cZ; //Asy.Easy();
  MinMax();
  Init();
}
static void Free( Frame &F ){ int i,j;
  for( i=1,j=0; i<=F.N; i++ )
  { if( fabs( F(j)-F(i) )>1e-12 || fabs( F[j]-F[i] )>1e-12 )j++;
    if( i!=j ){ F(i)=F(j); F[i]=F[j]; }
  } F.N=j;
}
inline void Add( Frame &F,_Real z,_Real f,int k ){ F(k)=z; F[k]=f; }

void Hull::ModelEx( Real &cL, Real &cB, int cN, int cM )
{ int i,j;
  cN = 2*( abs( cN )/2 )+1; // будет нечетное число для миделя в центре корпуса
  if( cN<3 )cN=3; Simple_Hull(cN,cN,cN); // всем всё поровну со старыми числами
  if( cL<1 )cL=1;
  if( cB<1 )cB=1;
  Xm=0; Xo=cL/-2; Length=Lmx=Lwl=cL; Ms=cN/2; Nstem=cN-1;
                 Breadth=Bmx=Bwl=cB;
  Do=0; Draught=cB/2; Depth=cB; Volume=cL*cB*cB; Surface=cB*(2*cL+cB);
 const Real dx=cL/Real(cN-1),dz=cB/Real(cN-1),da=_Pi/Real(cN-1);
  if( !cM )                                   // Кубик
  { for( i=0; i<cN; i++ )
    { F[i].X=Xo+i*dx;
      for( j=0; j<cN; j++ )Add( F[i],j*dz,Draught,j ); F[i].Easy();
      Add( Stx,i*dz,-Xo,i ); Add( Sty,i*dz,Draught,i );
      Add( Asx,i*dz, Xo,i ); Add( Asy,i*dz,Draught,i );
    } strcpy( Name,"Кирпич - прямоугольный параллелепипед" );
  } else
  { for( i=0; i<cN; i++ )
    { Real si=sin( i*da ),ci=cos( i*da ),R=Draught*si;
      F[i].X=ci*Xo;
      for( j=0; j<cN; j++ )Add( F[i],Draught-cos( j*da )*R,sin( j*da )*R,j );
      Add( Stx,Draught*( 1-ci ),-si*Xo,i); Add( Sty,Draught*( 1-ci ),0,i );
      Add( Asx,Draught*( 1-ci ),si*Xo,i ); Add( Asy,Draught*( 1-ci ),0,i );
      F[i].Easy();
    }
    if( cM==1 )strcpy( Name,"Эллипс - удлинённый шарик" );
    else
    { for( i=0; i<cN; i++ )
      { for( j=0; j<cN/2; j++ )F[i](j)=F[cN/2](j);
        if( i<cN/2 ){ Stx[i]=-Xo; Stx(i)=i*dz;
                      Asx[i]=Xo;  Asx(i)=i*dz; }
      }
      if( cM==2 )
        strcpy( Name,"Шлюпка - эллипсоид с равноудалёнными ватерлиниями" );
      else
      { strcpy( Name,"Корпус - эллипсоид с обводами кубической полноты" );
        for( i=0; i<cN; i++ )
        for( j=0; j<cN/2; j++ )
          F[i][j]=F[i][cN/2]*( 1-pow( Real( cN/2-j )*2.0/cN,3 ) );
  } } }
  Free( F[0] );
  Free( F[cN-1] );
  Stx.Easy(); Sty.Easy(); Asx.Easy(); Asy.Easy(); MinMax(); Init(); //BilgeEx()
}
//!  Это будет скуловой обвод - отдельной повторяемой командой
//                               чтоб без алымов-щины "струйных" систем обводов
static Real Bilge( _Real _x,_Real p=(5.0/3.0) )
{ return 0.5*( 1 + sin( ( pow( fabs( _x-0.5 )*2,p )/2+0.5 )*_Pd*1.5 ) );
}
void Hull::BilgeEx()
{ Real cz,cx=F[0].X,cl=F[Nstem].X-cx;  // разметка длины между перпендикулярами
  for( int i=0; i<Ns; i++ )
  { cz=Bilge( (F[i].X-cx)/cl );                           // аргумент от 0 до 1
    for( int j=0; j<F[i].N && F[i](j)<Draught; j++ )
      F[i][j] *= 1-cz*pow( ( Draught-F[i](j) )/Draught,3 ); }
  for( int i=0; i<=Asy.N; i++ )               // As-ахтерштевень, St-форштевень
   if( ( cz=Asy( i ) )<Draught )
    Asy[i] *= 1-Bilge( (Asx(cz)-cx)/cl )*pow( (Draught-cz)/Draught,3 );
  for( int i=0; i<=Sty.N; i++ )
   if( ( cz=Sty( i ) )<Draught )
    Sty[i] *= 1-Bilge( (Stx(cz)-cx)/cl )*pow((Draught-cz)/Draught,3);
  MinMax();
  Init();
}
///      Считывание корпуса "по контурам штевней и шпангоутов"
//                                                    01-12-01
Real MZ( _Real Z ){ if( Do>Z )Do=Z; if( Depth<Z )Depth=Z; return Z; }

int Hull::Read_from_Frames()          // Здесь продолжается
{ char *s,*c;                         // серия проверок:
  if( (s=strchr( Str+1,'<' ))!=NULL )
  if( (c=strchr( Str+1,'>' ))!=NULL ) // 2. ?должно быть имя в скобках
  { *c=0;
    while( *(++s)<=' ' )if( *s==0 )break;
    if( strcut( s )>0 )               // 3. ?поле имени не пустое
    { int i,k=0,n=0; Real o;
      strcpy( Name,s );  FGetS();
      sscanf( Str,"%d%d",&n,&k );     //
      if( n>1 && k>0 && k<=n )        // 4. количество шпангоутов >1
      { allocate( n );                //    и мидель не с краю
        Ms=k;                         // для повторного чтения
        FGetS(); o=0.0;               // сдвиг ОЛ-основной линии
        sscanf( Str,"%lf%lf%lf%lf",&Length,&Breadth,&Draught,&o ); Depth=Do=0;
        //
        // Ахтерштевень (начало прямого считывания из файла данных)
        //
        FGetS(); n=strtol( Str,&s,0 ); Asx.allocate( n );
        for( i=0; i<n; i++ )Asx(i)=MZ( strtod( s,&s )-o ),Asx[i]=strtod( s,&s );
        FGetS(); n=strtol( Str,&s,0 ); Asy.allocate( n );
        for( i=0; i<n; i++ )Asy(i)=MZ( strtod( s,&s )-o ),Asy[i]=strtod( s,&s );
        //
        // Собственно корпус  ( - в том же последовательном потоке )
        //
        for( k=0; k<Ns; k++ )
        { Frame &W=F[k];
          FGetS(); W.allocate( (n=strtol( Str,&s,0 ))+2 ); W.X=strtod( s,&s );
          for( i=0;; )  // { W.z[i]=MZ(strtod(s,&s)-o); W.y[i]=strtod(s,&s); }
          { W.z[i]=MZ( strtod( s,&s )-o ); W.y[i]=strtod( s,&s );
            if( i==0 )
            { if( W.y[0]>0 ){ W.z[1]=W.z[0],W.y[1]=W.y[0],W.y[0]=0; i=1; n++; }
            }
            if( ++i==n )
            { if( W.y[i-1]>0 ){ W.z[i]=W.z[i-1]; W.y[i]=0; n++; } break; }
          } W.N=n-1;
        }
        // Форштевень (табличные данные здесь заканчиваются, далее - строки)
        //
        FGetS(); n=strtol( Str,&s,0 ); Sty.allocate( n );
        for( i=0; i<n; i++ )Sty(i)=MZ( strtod( s,&s )-o ),Sty[i]=strtod( s,&s );
        FGetS(); n=strtol( Str,&s,0 ); Stx.allocate( n );
        for( i=0; i<n; i++ )Stx(i)=MZ( strtod( s,&s )-o ),Stx[i]=strtod( s,&s );
        //
        //  ... по необходимости лучше дополнить описание штевней
        //      нежели решать ротозейские проблемы из за их отсутствия
        n=Ns-1;
#if 0
        if( !Asx.N ){ Asx.allocate(2); if( !Asx(0) )Asx(0)=F[0](0); Asx[0]=Asx[1]=Asx[0]?Asx[0]:F[0].X; Asx(1)=F[0](F[0].N); }
        if( !Stx.N ){ Stx.allocate(2); if( !Stx(0) )Stx(0)=F[n](0); Stx[0]=Stx[1]=Stx[0]?Stx[0]:F[n].X; Stx(1)=F[n](F[n].N); }
        if( !Asy.N ){ Asy.allocate(2); if( !Asy(0) )Asy(0)=Asx(0); Asy[1]=Asy[0]; Asy(1)=Asx(1); }
        if( !Sty.N ){ Sty.allocate(2); if( !Sty(0) )Sty(0)=Stx(0); Sty[1]=Sty[0]; Sty(1)=Stx(1); }
#else
        if( Asx.N<0 ){ Asx.allocate( 1 ); Asx( 0 )=F[0]( 0 ); Asx[ 0 ]=F[0].X; }
        if( Stx.N<0 ){ Stx.allocate( 1 ); Stx( 0 )=F[n]( 0 ); Stx[ 0 ]=F[n].X; }
        if( Asy.N<0 ){ Asy.allocate( 1 ); Asy( 0 )=Asx( 0 ); }
        if( Sty.N<0 ){ Sty.allocate( 1 ); Sty( 0 )=Stx( 0 ); }
#endif
        //
        //  Уточнение основных параметров нового корпуса
        //
        MinMax();
        //
        // включение сплайн-функций для шпангоутов...
        //
//#pragma omp parallel for //private( i,k )
        for( int k=0; k<Ns; k++ )      // шпангоуты
        { Frame &W=F[k];               // под сплайн со сломами
          for( int i=1; i<W.N; i++ )   // только внутренние точки на шпангоутах
          { complex A={ W.z[i]-W.z[i-1],W.y[i]-W.y[i-1] },
                    B={ W.z[i+1]-W.z[i],W.y[i+1]-W.y[i] };
            if( norm( A )>1e-6 && norm( B )>1e-6 )// существует слом больше 45°
            if( ( A.x*B.x+A.y*B.y )/hypot( A.x,A.y )/hypot( B.x,B.y )<0.71 )
               W.Double( i++ );
          } // if( Hull_Keys&1 )
            W.SpLine(); W.Easy();                      // поднастроить и забыть
        } Init();
        return 0;
    } }
  } return 1;
}
//        Запись корпуса в формате "по контурам штевней и шпангоутов"
//                                                        2001-11-30
//static Real e6( _Real R ){ return round( R*1e6 )/1e6; }

int Hull::Write()
{ Real T; long D;
 int i,k,m,d,y; julday( D=julday(),m,d,y ); T=onetime();
 char Str[MAX_PATH*2];
  sprintf( Str,"%02d%02d%02d-%02d%02d",y%100,m,d,int( T ),int( T*60 )%60 );
  if( ( Fh=FileOpen( Str, L"wb", L"vsl",
               L"Копия корпуса (*.vsl)\1*.vsl\1Все файлы (*.*)\1*.*\1\1",
               L"? Запись таблицы ординат во временный файл" ) )!=NULL )
  { fprintf( Fh,";\n; Цифровой теоретический чертёж корпуса корабля,\n"
                   ";   построенный на контурах штевней и шпангоутов\n" );
    FPutS( "©~75\\24.Khram.Калининград",";" );
    FPutS( "·",";" );
//  fprintf( Fh,"\x1E  < %s >\n",Name );
//#define G " %.5lg %.5lg"
#define G " %7.5lf %7.5lf"
    FPutS( " признак и название","\x1E < %s >",Name );
    FPutS( " количество шпангоутов и номер миделя"," %d %d",Ns,Ms );
    FPutS( " длина ширина осадка [заглубление]"," %.5lg %.5lg %.5lg",
                                  Length,Breadth,Draught );
    //! Ахтерштевень
                             fprintf( Fh,"\n%3d",Asx.N+1 );
    for( i=0; i<=Asx.N; i++ )fprintf( Fh,G,Asx(i),Asx[i] );
                             fprintf( Fh,"\n%3d",Asy.N+1 );
    for( i=0; i<=Asy.N; i++ )fprintf( Fh,G,Asy(i),Asy[i] );
                             fprintf( Fh,"\n" );
    //! Шпангоуты
    //
    for( k=0; k<Ns; k++ )
    { int n=F[k].N; i=0;
      if( n>0 )
      { if( F[k].y[n]<1e-6 && F[k].z[n]==F[k].z[n-1] )--n;
        if( F[k].y[0]<1e-6 && F[k].z[0]==F[k].z[1] )i=1;
      }                fprintf( Fh,"\n%3d %-6.5lg",n-i,F[k].X );
      for( ; i<n; i++ )fprintf( Fh," " G,F[k].z[i],F[k].y[i] );
    }                  fprintf( Fh,"\n" );
    //! Форштевень
                             fprintf( Fh,"\n%3d",Sty.N+1 );
    for( i=0; i<=Sty.N; i++ )fprintf( Fh,G,Sty(i),Sty[i] );
                             fprintf( Fh,"\n%3d",Stx.N+1 );
    for( i=0; i<=Stx.N; i++ )fprintf( Fh,G,Stx(i),Stx[i] );
                             fprintf( Fh,"\n\n" );
    FPutS( "водоизмещение",            "; W=%.5lg м³",Volume );
    FPutS( "смоченная поверхность",    "; S=%.5lg м²",Surface );
    FPutS( "коэффициент общей полноты","; δ=%5.3lg",Volume/Bwl/Lwl/Draught );
    fprintf( Fh,";\n; %s\n; © %04d.%s.%02d %s%s — ‏יְרוּשָׁלַיִם‏‎\n",
         FileName,y,_Mnt[m-1],d,_Day[D%7],DtoA( T,3 ) ); fclose( Fh ); Fh=0;
    return 0;
  } return 1;
}
static void FGetS() // чтение строки из файла с пропуском комментариев
{ int Len;          // здесь память для строки сразу не менее двух килобайт
  do{ Str=getString( Fh );
    if( Str[0]==';' || !memcmp(Str,"//",2) )Str[Len=0]=0; else Len=strcut(Str);
  } while( !feof( Fh ) && !Len );
}
static Real Fort_F( char *s, int l, int d )
{ char b,sign=0; int i; Real w=0.0;
  for( i=strlen( s ); i<l; i++ )s[i]=0;
  for( i=0; i<l; i++,s++ )
  if( (b=s[0])=='-' )sign=1; else
  if(  b=='.' )d=l-i-1;      else
    { if( unsigned(b-='0')>9u )b=0; w=w*10.0+Real( b );
    }
  for( i=0; i<d; i++ )w/=10.0; return sign?-w:w;
}
static void FPutS( const char _Comm[], const char fmt[], ... )
{ int l,k,m=Ulen( _Comm );
  va_list aV;
  va_start( aV,fmt ); va_end( aV ); l=vsprintf( Str,fmt,aV ); k=Ulen( Str );
  if( _Comm )
    { while( k+m<80 ){ Str[l++]=' '; k++; } strcpy( Str+l,_Comm );
    } fprintf( Fh,"%s\n",Str );
}
///   Польские материалы от Александра Борисовича Дегтярёва
//
int Hull::Read_from_Polland()
{ int i,j,k,l,m,Nx,Nz,n=0;    // Размерения судна и размерность таблицы ординат
 Real W,z; sscanf( Str,"%lg%lg%lg",&Length,&Draught,&Breadth );
  FGetS(); sscanf( Str,"%d%d",&Nz,&Nx );
  FGetS();                           // сброс конца строки с заголовком таблицы
  if( Nx<3 || Nz<2 )return 1;
  Simple_Hull( Nx,Nz,Nz );
  for( i=0; i<Nx; i++ )              // Считывание таблицы с номерами
  { fscanf( Fh,"%d%d%lg",&j,&k,&W ); //  шпангоутов и их абсциссами
    if( j!=i+1 )Break( "(%d!=%d) на списке абсцисс\n",j,i+1 );
    F[i].X=W/1000.0;
  }
  for( j=0; j<=(Nz-1)/8; j++ )
  {    i=j*8;
       k=i+8;
    if( k>Nz )k=i+( Nz%8 );
    for( l=i; l<k; l++ )
    { fscanf( Fh,"%lg",&z ); z/=1000.0;       // Аппликаты ватерлиний
      for( m=0; m<Nx; m++ )F[m].z[l]=z;       // у всех теоретических
      Stx(l)=Sty(l)=Asx(l)=Asy(l)=z;          //   линий одинаковы
      Stx[l]=Sty[l]=Asx[l]=Asy[l]=0;
    }
    for( m=0; m<Nx; m++ )
    { fscanf( Fh,"%d",&n );
      if( n!=m+1 )Break( "(%d!=%d) на таблице ординат\n",n,m+1 );
      for( l=i; l<k; l++ )
      { Real &y=F[m].y[l]; fscanf( Fh,"%lg",&y ); y/=1000.0;
  } } }
//      Предварительная расчистка штевней
//
  for( i=0; i<Nz; i++ ){ Asx[i]=F[0].X; Stx[i]=F[Nx-1].X; }
//
//      И попытка выборки штевней из исходного файла
//
  fscanf( Fh,"%d%d",&i,&j );
  for( i=0;   i<Nz; i++ ){ Real &y=Asx[i]; fscanf( Fh,"%lf",&y ); y/=1000; } --j;
  for( i=j+1; i<Nz; i++ )Asx[i]=Asx[j];
  fscanf( Fh,"%d%d",&i,&j );
  for( i=0;   i<Nz; i++ ){ Real &y=Stx[i]; fscanf( Fh,"%lf",&y ); y/=1000; } --j;
  for( i=j+1; i<Nz; i++ )Stx[i]=Stx[j];
  Ms=Nx/2;
  MinMax();
  Length=Lmx;
  Init(); return 0;
}
bool Hull::Read()              // Нормальный выход с новым корпусом с кодом '0'
{ const char MIDV[]=
"//MODEL JOB '158015ИКТК',KHRAMUSHIN,CLASS=D,PRTY=6                 ЛКИ  850827\n"
"// ...\n"
"// Гипотетическая модель судна МИДВ \\ Ленинград, Корабелка-1985\n"
"*   (с малыми моментами инерции площади действующей ватерлинии)\n"
"011\n"
" 24 18\n"
"  -950  -950 -1120 -1230 -1320 -1390 -1400 -1352 -1200\n"
"  -720   070   620   760  1220  1796  2200  2320  2600\n"
" 22000 24340 24470 24600 24520 24200 23640 22950 22360\n"
" 22000 21832 21720 21700 21732 21800 21866 21900 21900\n"
"     0   150   180   220   230   220   180   120    30\n"
"     0     0     0     0     0     0     0     0     0\n"
"     0    60   120   200   160    60    20     0     0\n"
"     0     0     0    10    30    70   100   120     0\n"
"       0.0 1.5 2.0 4.0 6.0 8.0 10. 12. 14. 16. 18. 20. 22. 24. 26. 28.28802900\n"
" -1100   0   0   0   0   0   0   0   0   0  46 180 268 312 303 250  90   0   0\n"
"     0   0   0   0   0   0   0   0   0  93 221 351 457 520 530 512 476 400 400\n"
"  1100   0   0   0   0  11  33  51 117 232 366 484 578 640 663 647 610 538 538\n"
"  2200   0   0   0  44 185 192 187 251 370 498 603 688 746 767 754 728 671 671\n"
"  3300  33  61  77 190 423 423 398 427 512 621 711 782 838 852 851 832 802 802\n"
"  4400  70 122 150 447 695 688 639 623 663 730 798 856 905 938 942 929 911 911\n"
"  5500  90 145 231 755 974 959 874 817 807 830 867 914 964 9991013100810001000\n"
"  6600 100 150 4981033119611791086 991 937 920 932 9661012106010891088 939 939\n"
"  7700 100 150 656122913591336124111361043 990 971 9851030108211171119 969 969\n"
"  8800 100 150 8611360148014561358124011221031 989 9991046110111421148 997 997\n"
"  9900 100 150 936142115471523141912831153105610111018105011161158115910101010\n"
" 11000 100 150 958146615801569146213171175106710191021106011171159116010111011\n"
" 12100 100 150 980149615991570146413121169105810121017105811161159116010111011\n"
" 13200 100 29010101490158315531447129111501042 993 9981040110111451149 996 996\n"
" 14300 100 53010261476155915231411125911121000 948 953 999106211051109 957 957\n"
" 15400 100 6751038142914971450133511911050 936 883 892 940100110481052 900 900\n"
" 16500 100 772105413411390133912231083 942 831 772 773 819 891 947 956 802 800\n"
" 17600 100 83010091213124111821070 941 803 682 621 618 658 726 793 821 665 665\n"
" 18700 100 795 89610531068 996 880 740 608 503 443 437 468 522 584 651 651 651\n"
" 19800 100 672 745 856 857 776 653 518 392 302 259 252 273 319 380 460 460 460\n"
" 20900 100 543 592 677 664 578 452 313 204 129  98  98 117 152 202 267 267 267\n"
" 22000   0 420 471 533 508 420 298 157  58   0   0   0   0   0   0   0   0   0\n"
" 23100   0 322 361 418 378 271 129   0   0   0   0   0   0   0   0   0   0   0\n"
" 24200   0 150 225 309 249   0   0   0   0   0   0   0   0   0   0   0   0   0\n"
"/*\n"
"* Список рабочих водоизмещений (посадок) при креновании\n"
"  2592 10042 28773 45514 52675 59363 65902 79596 94761\n"
"\n"
"     ©1985, Игорь Владимирович Степанов, инженер кафедры конструкции судов ЛКИ\n";
//
//    Считывание заголовков и проверка наличия управляющих кодов
//
 int i,j,k,Nx,Nz; Real w; // bool hew=true;
 static WCHAR FileTitle[]=L"  Выбрать *.vsl или <Esc> к образцу МИДВ корпуса";
  if( !(Fh=FileOpen( Name, L"rt", L"vsl",
                           L"Корпус корабля (<плаз>.vsl)\1*.vsl\1"
                           L"Все файлы (<плаз>.*)\1*.*\1\1",FileTitle ) ) )
  if( !(Fh=fopen( strcpy( Name,"Hull.vsl" ),"rt" ) ) )
  { if( (Fh=fopen( Name,"wb" ) )!=NULL )
    { fputs( MIDV,Fh ); fclose( Fh ); // hew=false; // случай корпуса из памяти
      if( (Fh=fopen( Name,"rt" ) )!=NULL )goto Ok;  // теперь новый файл открыт
    } goto back;
  } FileTitle[0]='?';
Ok: FileName=strdup( strcpy( Name,fname( Name ) ) );
      FGetS();        // чтение первой строки с переадресацией на Str=>Ls[2048]
  if( Str[0]==32 )    // '\32' - ' ' - пробел == польский формат от А.Дегтярёва
    { k=Read_from_Polland(); if( k )goto back; fclose( Fh ); return k;
    } else            //
  if( Str[0]==30 )    //! '\30' = ''== основной формат цифрового корпуса
    { k=Read_from_Frames(); if( k )goto back; fclose( Fh ); return k;
    } FGetS();        // иначе формат кафедры конструкции судов Игоря Степанова
  for( i=0; i<3; i++ )if( (unsigned char)(Str[i]-'0')>3u )goto back;
  if( (unsigned char)(Str[3]-'0')<=3u )  //
    { for( i=0; i<3; i++ )FGetS();       // Пропуск задания
      sscanf( Str,"%d",&Nx );            // по остойчивости
      for( i=0; i<Nx+2; i++ )FGetS();    //
    } FGetS();                           // Размерность и тип данных
  sscanf( Str,"%d%d%s",&Nx,&Nz,Str+40 ); //
  if( Nx<2 || Nz<2 )goto back;
  Simple_Hull( Nx,Nz,Nz );
  if( !strcmp( Name,"Hull.vsl" ) )strncpy( Name,MIDV+96,107 ),
                                  strncpy( Name+107,MIDV+207,111 ),Name[218]=0;
//
//    Если задано описание штевней, то считывание (j#0 - признак)
//
  if( (j=strcmp( strupr( Str+40 ),"SHORT" ))!=0 )
  { for( i=0; i<Nz; i++ )                               // Абсциссы ахтерштевня
      { if( !(k=i%9) )FGetS(); Asx[Nz-i-1]=Fort_F( Str+k*6,6,2 ); }
    for( i=0; i<Nz; i++ )                               // Абсциссы форштевня
      { if( !(k=i%9) )FGetS(); Stx[i]=Fort_F( Str+k*6,6,2 ); }
    for( i=0; i<Nz; i++ ) // Ординаты кормового транца и расправленной раковины
      { if( !(k=i%9) )FGetS(); Asy[Nz-i-1]=Fort_F( Str+k*6,6,2 ); }
    for( i=0; i<Nz; i++ ) // Ординаты линий скулы, расправленных на срезе форштевня
      { if( !(k=i%9) )FGetS(); Sty[i]=Fort_F( Str+k*6,6,2 ); }
  }
//  Считывание аппликат расчетных ватерлиний
//
  FGetS();
  for( i=0; i<Nz; i++ )
  { w=Fort_F( Str+i*4+6,4,2 );
    for( k=0; k<Nx; k++ )F[k].z[i]=w; Asx(i)=w; Asy(i)=w;
                                      Stx(i)=w; Sty(i)=w;
  }
//  Считывание таблицы ординат теоретического чертежа корпуса судна
//
  Breadth=0;
  for( k=0; k<Nx; k++ )
  { FGetS(); F[k].X=Fort_F( Str,6,2 );
    for( i=0; i<Nz; i++ )
    { F[k].y[i]=w=Fort_F( Str+i*4+6,4,2 ); if( Breadth<w )Breadth=w;
    } //if( Hull_Keys&0x01 )F[k].SpLine();
  }   //if( !hew )Hull_Keys &= ~1u;
  fclose( Fh ); Fh=NULL;
//
//  Если таблица короткая (без описания штевней)
//
  if( !j )for( ; j<Nz; j++ )Asy[j]=Sty[j]=0,Asx[j]=F[0].X,Stx[j]=F[Nx-1].X;
  Ms=Ns/2;
  Breadth*=2.0; Draught=Stx(Nz/2); MinMax(); Length=Lmx; Init(); return 0;
back: if( Fh ){ fclose( Fh ); Fh=NULL; } strcpy( Name,"*.vsl" ); return 1;
}
