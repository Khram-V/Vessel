//
//   г---------------------------------------------------------¬
//   ¦                                                         ¦
//   ¦  Программа математического определения (моделирования)  ¦
//   ¦       формы корпуса судна, с определением базовых       ¦
//   ¦             функций для его описания:                   ¦
//   ¦                                                         ¦
//   ¦   float K_Hull  ( float z, float x ); // Корпус         ¦
//   ¦   float K_Astern( float z );          // Ахтерштевень   ¦
//   ¦   float K_Stern ( float z );          // Форштевень     ¦
//   ¦                                                         ¦
//   L=========================================================-
//                       1997,январь, В.Храмушин, Южно-Сахалинск
//                  2001, август - Владивосток, октябрь - ГС-210
//
#include "Hull.h"

const int Ls=16000;
static char *Str=strcpy( (char*)malloc( Ls ),
            "\0\n<<МИДВ>>\n (c)1975-2001, В.Н.Храмушин, Южно-Сахалинск\n" );
static FILE *Fh;
//   г--------------------------------------------------------¬
//   ¦                                                        ¦
//   ¦   Считывание/запись численного описания формы корпуса  ¦
//   ¦     (при первом обращении возвращается корпус МИДВ)    ¦
//   ¦                                                        ¦
//   L========================================================-
static char* UtR( char &s, char *U );    // символ UTF-8 -> Russian-OEM(866)alt
static void FGetS()
{ int l,k,Len;
 char s[Ls],*c,b; s[0]=0;
  do
  { fgets( s,sizeof( s ),Fh );
    if( (c=strchr( s,';'  ))!=NULL )*c=0;
    if( (c=strstr( s,"//" ))!=NULL )*c=0;
    Len=strcut( s );
  } while( !feof( Fh ) && !Len );
  c=s;
  for( l=k=0; *c && k<Len; k++ )
  { c=UtR( b,c );
     if( (Str[l]=b)!=9 )l++; else do Str[l]=' '; while( ++l%8 );
  }       Str[l]=0;
}
static Real Fort_F( char *s, int l, int d )
{ char b,sign=0; int i; double w=0.0;
  for( i=strlen( s ); i<l; i++ )s[i]=0;
  for( i=0; i<l; i++,s++ )
  if( (b=s[0])=='-' )sign=1; else
  if(  b=='.' )d=l-i-1;      else
    { if( unsigned(b-='0')>9u )b=0; w=w*10.0+Real( b );
    }
  for( i=0; i<d; i++ )w/=10.0; return sign?-w:w;
}
static void FPutS( const char _Comm[], const char _Fmt[], ... )
{ int l;
  va_list ap;
  va_start( ap,_Fmt ); l=vsprintf( Str,_Fmt, ap ); va_end (ap);
  if( _Comm )
  { while( l<79 )Str[l++]=' '; strcpy( Str+79-strlen( _Comm ),_Comm );
  } fprintf( Fh,"%s\n",W2D( Str ) );
}
//
// Польские материалы от А.Дегтярева
//
int Hull::Read_from_Polland()
{                         //
 int i,j,k,l,m,Nx,Nz,n=0; // Размерения судна и размерность таблицы ординат
 Real W;                 //
 Point P;
  sscanf( Str,"%lf%lf%lf",&Length,&Draught,&Breadth );
  fscanf( Fh,"%d%d%s",&Nz,&Nx,Str );
  fgets( Str,Ls-1,Fh );          // сброс конца строки с заголовком таблицы
  if( Nx<3 || Nz<2 )return 1;    //
  Simple_Hull( Nx,Nz );          //
  for( i=0; i<Nx; i++ )              // Считывание таблицы с номерами
  { fscanf( Fh,"%d%d%lg",&j,&k,&W ); //  шпангоутов и их абсциссами
    if( j!=i+1 )Break( "(%d!=%d) на списке абсцисс\n",j,i+1 );
    F[i].X=W/1000.0;
  } P.y=0.0;
  for( j=0; j<=(Nz-1)/8; j++ )
  {    i=j*8;
       k=i+8;
       if( k>Nz )k=i+( Nz%8 );
    for( l=i; l<k; l++ )                      //
    { fscanf( Fh,"%lg",&(P.x) ); P.x/=1000.0; // аппликаты ватерлиний
      for( m=0; m<Nx; m++ )F[m].z[l]=P.x;     // у всех теоретических
      Stx[l]=P; Sty[l]=P;                     //  линий одинаковы
      Asx[l]=P; Asy[l]=P;                     //
    }
    for( m=0; m<Nx; m++ )
    { fscanf( Fh,"%d",&n );
      if( n!=m+1 )Break( "(%d!=%d) на таблице ординат\n",n,m+1 );
      for( l=i; l<k; l++ )
      { Real &y=F[m].y[l]; fscanf( Fh,"%lg",&y ); y/=1000.0;
  } } }
  if( Hull_Keys&0x01 )for( i=0; i<Nx; i++ )F[i].SpLine();
//
//      Предварительная расчистка штевней
//
  for( i=0; i<Nz; i++ ){ Asx[i].y=F[0].X; Stx[i].y=F[Nx-1].X; }
//
//      И попытка выборки штевней из исходного файла
//
  fscanf( Fh,"%d%d",&i,&j );
  for( i=0;   i<Nz; i++ ){ Real &y=Asx[i].y; fscanf( Fh,"%lf",&y ); y/=1000.0; } --j;
  for( i=j+1; i<Nz; i++ )Asx[i].y=Asx[j].y;
  fscanf( Fh,"%d%d",&i,&j );
  for( i=0;   i<Nz; i++ ){ Real &y=Stx[i].y; fscanf( Fh,"%lf",&y ); y/=1000.0; } --j;
  for( i=j+1; i<Nz; i++ )Stx[i].y=Stx[j].y;

  Height=F[Nx/2].z[Nz-1];
  Lwl=Stx( Draught )-Asx( Draught );
  Xm=Lwl/2;
  Do=0.0;
  Bwl=(*this)( Xm,Draught )*2;
  { Field M; Asx.Extreme( M ); Xo=M.Jy;
             Stx.Extreme( M ); Length=M.Jy+M.Ly-Xo;
  } Init();  return 0;
}                         //
int Hull::Read( const char* _Name ) // Имя файла с описанием корпуса
{                         //
 int i,j,k,Nx,Nz;
 Real w;
 Field W;
 static char MIDV[]=
"//MODEL JOB '158015ИКТК',KHRAMUSHIN,CLASS=D,PRTY=6                     850827\n"
"// ...\n"
"// Гипотетическая модель судна МИДВ \\ Ленинград, Корабелка-1985\n"
"*    (малыми моментами инерции площади действующей ватерлинии)\n"
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
"/*\n";
//"* Список рабочих водоизмещений (посадок) при креновании\n"
//"  2592 10042 28773 45514 52675 59363 65902 79596 94761\n";
//
//    Считывание заголовков и проверка наличия управляющих кодов
//
  strcpy( Name,_Name );
  if(  !(Fh=Topen( Name,"rt","vsl","? выбрать корпус или - Esc - для модели МИДВ" ) ) )
  if(  !(Fh=fopen( strcpy( Name,"Hull.vsl" ),"rt" ) ) )
  { if( (Fh=fopen( Name,"wt" ) )!=NULL )
    { fputs( W2D( MIDV ),Fh ); fclose( Fh );
      if( (Fh=fopen( Name,"rt" ) )!=NULL )goto Ok;
    } goto back;
  }
Ok:  strcpy( Name,fname( _Name ) );
      FGetS();
  if( Str[0]==' ' )
    { k=Read_from_Polland(); if( k )goto back; fclose( Fh ); return k;
    } else              //
  if( Str[0]=='' )     //
    { k=Read_from_Frames(); if( k )goto back; fclose( Fh ); return k;
    } FGetS();
  for( i=0; i<3; i++ )if( (unsigned char)(Str[i]-'0')>3u )goto back;
  if( (unsigned char)(Str[3]-'0')<=3u )  //
    { for( i=0; i<3; i++ )FGetS();       // Пропуск задания
      sscanf( Str,"%d",&Nx );            // по остойчивости
      for( i=0; i<Nx+2; i++ )FGetS();    //
    } FGetS();                           // Размерность и тип данных
  sscanf( Str,"%d%d%s",&Nx,&Nz,Str+40 ); //
  if( Nx<3 || Nz<2 )goto back;
  Simple_Hull( Nx,Nz );
//
//    Если задано описание штевней, то считывание (j#0 - признак)
//
  if( (j=strcmp( strupr( Str+40 ),"SHORT" ))!=0 )
  { for( i=0; i<Nz; i++ )                       //
      { if( !(k=i%9) )FGetS();                  // Абсциссы ахтерштевня
        Asx[Nz-i-1].y=Fort_F( Str+k*6,6,2 ); }  //
    for( i=0; i<Nz; i++ )                       //
      { if( !(k=i%9) )FGetS();                  // Абсциссы форштевня
        Stx[i].y=Fort_F( Str+k*6,6,2 ); }       //
    for( i=0; i<Nz; i++ )                       // Ординаты для кормового
      { if( !(k=i%9) )FGetS();                  // транца и расправленной
        Asy[Nz-i-1].y=Fort_F( Str+k*6,6,2 ); }  // раковины
    for( i=0; i<Nz; i++ )                       // Ординаты для линий
      { if( !(k=i%9) )FGetS();                  // скулы, расправленных
        Sty[i].y=Fort_F( Str+k*6,6,2 ); }       // на срезе форштевня
  }                                             //
//
//  Считывание апликат расчетных ватерлиний
//
  FGetS();
  for( i=0; i<Nz; i++ )
  { w=Fort_F( Str+i*4+6,4,2 );
    for( k=0; k<Nx; k++ )F[k].z[i]=w; Asx[i].x=w; Asy[i].x=w;
                                      Stx[i].x=w; Sty[i].x=w;
  }
//  Считывание таблицы ординат теоретичексго чертежа корпуса судна
//
  Breadth=0;
  for( k=0; k<Nx; k++ )
  { FGetS();
    F[k].X=Fort_F( Str,6,2 );
    for( i=0; i<Nz; i++ )
    { F[k].y[i]=w=Fort_F( Str+i*4+6,4,2 ); if( Breadth<w )Breadth=w;
    } if( Hull_Keys&0x01 )F[k].SpLine();
  } fclose( Fh );
//
//  Если таблица короткая (без описания штевней)
//
  if( !j )for( i=0; i<Nz; i++ )
  { Asy[i].y=Sty[i].y=0.0; Asx[i].y=F[0].X; Stx[i].y=F[Nx-1].X;
  } Asx.Extreme( W );    Xo=W.Jy;
    Stx.Extreme( W );    Length=W.Jy+W.Ly-Xo;
    Draught=Stx[Nz/2].x; Height=Stx[Nz-1].x; Do=Stx[0].x;
    Lwl=Stx( Draught )-Asx( Draught );
    Xm=Lwl/2;
    Bwl=(*this)( Xm,Draught )*2; Breadth*=2; Init(); return 0;
back: if( Fh )fclose( Fh ); Fh=0; return 1;
}
void Hull::Simple_Hull( int Nx, int Nz )
{ int i;
  allocate( Nx );
  for( i=0; i<Nx; i++ )F[i].allocate( Nz );
    Stx.allocate( Nz ); Sty.allocate( Nz );
    Asx.allocate( Nz ); Asy.allocate( Nz );
  for( i=0; i<Nz; i++ )Stx[i]=Sty[i]=Asx[i]=Asy[i]=0.0;
}                               //
void Hull::Set_SpLine()         // Установка сплайновой разметки
   { for( int i=0; i<Ns; i++ )F[i].SpLine();
   }                            //
void Hull::Del_SpLine()         // Снятие сплайновой разметки
   { for( int i=0; i<Ns; i++ )F[i].allocate( F[i].N+1 );
   }
//
//   г----------------------------------------------------------¬
//   ¦                                                          ¦
//   ¦  Считывание корпуса  "по контурам штевней и шпангоутов"  ¦
//   ¦                                                          ¦
//   L==========================================================-
//                                                       01-12-01
int Hull::Read_from_Frames()         //
{                                    // Здесь продолжается
 char *s,*c;                         // серия проверок:
  if( (s=strchr( Str,'<' ))!=NULL )  //
  if( (c=strchr( Str,'>' ))!=NULL )  // 2. ?должно быть имя в скобках
  { *c=0;                            //
    while( *(++s)<=' ' )if( *s==0 )break;
    if( strcut( s )>0 )              // 3. ?поле имени не пустое
    {                                //
     int i,k,n;
     Real w,o;
      strcpy( Name,s );  FGetS();    //
      sscanf( Str,"%d%d",&n,&k );    //
      if( n>3 && k>0 && k<n )        // 4. количество шпангоутов >3
      { allocate( n );               //    и мидель не с краю
        Ms=k;                        //
        FGetS();                                          o=0.0;
        sscanf( Str,"%lf%lf%lf%lf",&Length,&Breadth,&Draught,&o );
        //
        // Ахтерштевень (начало прямого считывания из файла данных)
        //
        FGetS(); n=strtol( Str,&s,0 ); Asx.allocate( max(2,n) );
        for( i=0; i<n; i++ )Asx[i].x=strtod( s,&s )-o,Asx[i].y=strtod( s,&s );
        FGetS(); n=strtol( Str,&s,0 ); Asy.allocate( max(2,n) );
        for( i=0; i<n; i++ )Asy[i].x=strtod( s,&s )-o,Asy[i].y=strtod( s,&s );
        //
        // Собственно корпус  ( - в том же последовательном потоке )
        //
        for( k=0; k<Ns; k++ )
        { Frame &W=F[k];
          FGetS(); W.allocate( n=strtol( Str,&s,0 ) ); W.X=strtod( s,&s );
          for( i=0; i<n; i++ )
          { W.z[i]=w=strtod( s,&s )-o; W.y[i]=strtod( s,&s );
            if( !i && !k )Do=Height=w; else
            { if( Height<w )Height=w;
              if( Do>w )Do=w;
          } }
        } Height-=Do;
        //
        // Форштевень (табличные данные здесь заканчиваются, далее - строки)
        //
        FGetS(); n=strtol( Str,&s,0 ); Sty.allocate( max(2,n) );
        for( i=0; i<n; i++ )Sty[i].x=strtod( s,&s )-o,Sty[i].y=strtod( s,&s );
        FGetS(); n=strtol( Str,&s,0 ); Stx.allocate( max(2,n) );
        for( i=0; i<n; i++ )Stx[i].x=strtod( s,&s )-o,Stx[i].y=strtod( s,&s );
        //
        // ...по необходимости лучше дополнить описание штевней
        //    нежели решать ротозейские проблемы из за их отсутствия
        //
        if( !Asx.N )Asx[0].x=Do,Asx[0].y=F[0].X,Asx[1].x=Height,Asx[1].y=F[0].X;
        if( !Asy.N )Asy[0].x=Do,Asy[0].y=0.0,   Asy[1].x=Height,Asy[1].y=0.0;

        if( !Sty.N )Sty[0].x=Do,Sty[0].y=0.0,      Sty[1].x=Height,Sty[1].y=0.0;
        if( !Stx.N )Stx[0].x=Do,Stx[0].y=F[Ns-1].X,Stx[1].x=Height,Stx[1].y=F[Ns-1].X;

        //
        // Уточнение основных параметров нового корпуса
        //
        n=Sty.N-1; if( Height<Sty[n].x )Height=Sty[n].x;
        n=Asy.N-1; if( Height<Asy[n].x )Height=Asy[n].x;
        { Field M; Asx.Extreme( M ); Xo=M.Jy;
                   Stx.Extreme( M ); Length=M.Jy+M.Ly-Xo;
        }
        Lwl=Stx( Draught )-Asx( Draught );
        Bwl=((*this)( Xm=F[Ms].X,Draught ))*2; Init();
        return 0;
  } } } return 1;
}
//   г---------------------------------------------------------------¬
//   ¦                                                               ¦
//   ¦  Запись корпуса в формате "по контурам штевней и шпангоутов"  ¦
//   ¦                                                               ¦
//   L===============================================================-
//                                                           01-11-30
int Hull::Write()
{
 const Real g5=1.000001;
 double t;
 int i,k,p,m,d,y; julday( julday(),m,d,y );
  sprintf( Str,"%02d%02d%02d%02d",y%100,m,d,int( t=onetime() ) );
  Thelp( "? Имя файла:" ); Tgo( 14,0 ); p=Tv.BkColor;
  color( YELLOW );
  bkcolor( BLUE );
  for( ;; )
  switch( Wgets( fext( Str,"vsl" ),-64 ) )
  { case _Enter:
    if( (Fh=fopen( fext( Str,"vsl" ),"wt" ))!=NULL )
    { bkcolor( p );
      fprintf( Fh,W2D( "//\n// Модель корпуса корабля,\n"
                      "//        построенная на контурах штевней и шпангоутов\n" ) );
      FPutS( "(c)01.Khram","//" );
      FPutS( "·","//" );
      FPutS( "признак и название","<%s>",Name );
      FPutS( "количество шпангоутов и номер миделя"," %d %d",Ns,Ms );
      FPutS( "длина, ширина, осадка"," %.5lg %.5lg %.5lg",Length*g5,Breadth*g5,Draught*g5 );
      //
      // Ахтерштевень
                              fprintf( Fh,"\n%3d ",Asx.N );
      for( i=0; i<Asx.N; i++ )fprintf( Fh," %.5lg %.5lg", Asx[i].x*g5,Asx[i].y*g5 );
                              fprintf( Fh,"\n%3d ",Asy.N );
      for( i=0; i<Asy.N; i++ )fprintf( Fh," %.5lg %.5lg", Asy[i].x*g5,Asy[i].y*g5 );
                              fprintf( Fh,"\n" );
      // Шпангоуты
      //
      for( k=0; k<Ns; k++ )
      {                           fprintf( Fh,"\n%3d %.5lg",  F[k].N+1,F[k].X*g5 );
        for( i=0; i<=F[k].N; i++ )fprintf( Fh," %.5lg %-5.5lg",F[k].z[i]*g5,F[k].y[i]*g5 );
      }                           fprintf( Fh,"\n" );
      //
      // Форштевень
                              fprintf( Fh,"\n%3d ",Sty.N );
      for( i=0; i<Sty.N; i++ )fprintf( Fh," %.5lg %.5lg", Sty[i].x*g5,Sty[i].y*g5 );
                              fprintf( Fh,"\n%3d ",Stx.N );
      for( i=0; i<Stx.N; i++ )fprintf( Fh," %.5lg %.5lg", Stx[i].x*g5,Stx[i].y*g5 );
                              fprintf( Fh,"\n\n" );

        FPutS( "водоизмещение",            ";  W=%.5lg",Volume );
        FPutS( "смоченная поверхность",    ";  S=%.5lg",Surface );
        FPutS( "коэффициент общей полноты",";  d=%.5lg",Volume/Bwl/Lwl/Draught );
      sprintf( Str+80,"%04d.%s.%02d %s%s",y,Months[m-1],d,WeekDay[int(t)%7],DtoA( t ) );
        FPutS( Str+80,";\n;" );
       fclose( Fh );
               Fh=0; return 0;
    }
    case _Esc : bkcolor( p ); return 1;
    default:;
  }
}
//
//      Установка режимов обработки корпуса
//
int Set_Parameters()
{
 int i,j,n,Ans=0; Real Av,Mv,w;
  do{
    if( Hull_Keys&0x01 )
    {   Av=Mv=0.0;
      for( i=n=0; i<Kh.Ns; i++ )
      for( j=0; j<Kh.F[i].N; j++ )
      { w=hypot( Kh.F[i]._z[j],Kh.F[i]._y[j] );
        Av+=w;    ++n;
        if( Mv<w )Mv=w;
      }    sprintf( Str,"\t Кривизна( %.3lg ) <= %.3lg",Av/=n,Mv );
    } else sprintf( Str,"\t Сплайн не используется " );
    { Mlist Menu[]=
        { { 1,26,Str },{ 1,0 },{ 0,6,"\t Пороговое число: %.3lg",&Mv } };
      Ans=Tmenu( Menu,Hull_Keys&0x01?3:2,1,-1 );
    }
    switch( Ans )
    { case 0: if( !(Hull_Keys^=0x01) )Kh.Del_SpLine();
                                else  Kh.Set_SpLine();
              Building();       break;
      case 2: for( i=0; i<Kh.Ns; i++ )
              for( j=Kh.F[i].N; j>=0; j-- )
               if( hypot( Kh.F[i]._z[j],Kh.F[i]._y[j] )>Mv )
                   Kh.F[i].Double( j );
                   Kh.Del_SpLine();
                   Kh.Set_SpLine();
              Kh.Init(); Building(); break;
    }
  } while( Ans!=_Esc ); return 0;
}
//  Выборка из файла UniCode.cpp для считывания текстов в формате UTF-8 -> OEM
//
#define X( x )if( (U[x]&0xC0)==0x80 )   // контроль старших бит в других байтах
static int nS=0;                        // контрольное число байт в символе UTF

static char* UtI( unsigned &u, char *U ) // сборка сложного символа из 1-4 байт
{ union{ char c[4]; unsigned d; } v; nS=1; // совмещение с выполнением проверки
  if( (v.d=*U)&0x80 )                     // случится ошибка при nS<2 и v.d>127
  { if( (*U&0xE0)==0xC0 ){ X(1){ v.c[1]=*U++; v.c[0]=*U; nS=2; } } else
    if( (*U&0xF0)==0xE0 ){ X(1)X(2){ v.c[2]=*U++; v.c[1]=*U++; v.c[0]=*U; nS=3; } } else
    if( (*U&0xF8)==0xF0 ){ X(1)X(2)X(3){ v.c[3]=*U++; v.c[2]=*U++; v.c[1]=*U++; v.c[0]=*U; nS=4; } }
  } u=v.d; return ++U;            // на выходе адрес той же строки со смещением
}
static char* UtR( char &s, char *U )     // символ UTF-8 -> Russian-OEM(866)alt
{ unsigned d=U[0];                       // здесь в UTF-8 меняется порядок байт
  if( d<128 )s=*U++,nS=1; else           // особый контроль латыни на ускорение
  {   U=UtI( d,U );                      // сборка сложного символа из 1-4 байт
    if( d>=53392 && d<53440 )s=d-53264; else   // 1040-128 = А..п 53264
    if( d>=53632 && d<53648 )s=d-53408; else   // 1040-224 = р..я 53408
    if( d>=0xE29590 && d<=0xE295AC )
    { static char k[29]= { 205,186,213,214,201,184,183,187,212,211,200,190,189,
             188,198,199,204,181,182,185,209,201,203,207,208,202,216,215,206 };
      s = k[d-0xE29590]; } else    // °±·(Ё,ё)
    { static unsigned u[] = { 0xC2B0,0xC2B1,0xC2B2,0xC2B7,0xC3B7,0xD081,0xD191,
       0xE281BF,0xE28899,0xE2889A,0xE28988,0xE289A1,0xE289A4,0xE289A5,0xE28CA0,
       0xE28CA1,0xE29480,0xE29482,0xE2948C,0xE29490,0xE29494,0xE29498,0xE2949C,
       0xE294A4,0xE294AC,0xE294B4,0xE294BC,0xE29680,0xE29684,0xE29688,0xE2968C,
       0xE29690,0xE29691,0xE29692,0xE29693,0xE296A0 }; // 0xEF8C81 = '?' ? 0xFF
      static char k[]={ 248,241,253,250,246,133,165,252,249,251,247,240,
                        243,242,244,245,196,179,218,191,192,217,195,180,
                        194,193,197,223,220,219,221,222,176,177,178,254 };
      int i; for( i=0; i<36; i++ )if( d==u[i] ){ s=k[i]; break; }
                                  if( i==36 )s=0xFF;
  } } return U;                          // окончание строки
}                                        //    и возврат её последующего адреса
