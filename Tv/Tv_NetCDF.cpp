//
// <Home>       Установка первого месяца в обобщеном блоке
// <End>        Установка последнего месяца и года в исходных данных
// <Left/Right> - последовательное (помесячное) листание
// <Up/Down>    - смена года для того же месяца
// <Shift+Up/Left/Down/Right> - быстрый перебор изображений
// <Space>      Перерисовка и сброс линии профиля
// <Delete>     Удаление профильной линии (если она есть), затем
//                 восстановление исходного масштаба изображения
// <F4>         Табличка для ручного заполнения координат сегмента
// <F4+Shift>   - раскрытие на весь земной шар с подвижкой в 60°
// <F4+Ctrl>    - охотоморский регион
//
// <LMouse>     Разметка нового масштабного прямоугольника (15х11)
// <RMouse>     Выбор линии для построения профильного разреза
//
// <Esc>        Безусловное завершение программы
//
#include "Tv.h"
const char
 *NC_Type_name[13]={ "UnSpecified",
                     "Byte","Char","Short","Long","Float","Double",
                     "BitField","String","Array","Dimension",
                     "Variable","Attribute"
                   },
 *_Months[12]={ "January","February","March","April","May","June","Jule",
                "August","September","October","November","December"
              };         //
const int sbuf=2048;     // смещение по второй половине буфера строки
static FILE *F=NULL;     // 359*179= 64261 : +1275
static char buf[4096],   // 360*180= 64800 : +736
          *title =NULL,  //   2^16 = 65536
          *units =NULL,  //
          *source=NULL;
static struct Dimension{ long N; char *S; } *dim=NULL;
                         //
static int  Nd=0,        // Количество размерностей
            Nl=0;        // Длина линии профильного разреза
static long Ny=0,        // число в начале файла - количество лет
            Np=0,        // Количество точек в списке данных
            Mons,        // Местоположение списка месяцев
            Lats,        // Начало списка широт,
            Lons,        //                  долгот
            cLms,        //   и первого блока обобщенных,
            yLms,        //  - и собственно исходных данных
            cMin,yMin,   // Считываемые экстремумы, используемые
            cMax,yMax;   //  для масштабирования цветов на рисунке
static Real cF=1,yF=1,   // Scale Factor - масштабный коэффициент
            cO=0,yO=0,   //   Add Offset - смещение нулевой точки
            Max,         // = cO + cMax*cF + yO + yMax*yF
            Min,         // = cO + cMin*cF + yO + yMin*yF
            Msc,         // = Max-Min;
            Ldst,        // Длина линии профиля в км.
            *Line=NULL;  // Значения функции
static char *Lmsk=NULL;  // Маска заполняемости точки на линии
static Point *Lp=NULL;   //  и координат на линии разреза
static field Tvm={ 0,8, 0,0, 2 };
static Field Map={ 0,0,360,180 };

static void  Rainbow();
static void  Frame_of_Map();
static void  Frame_of_Map( field,Field );
static Course Tv_Answer();
static void _Break( const char msg[],... );
static void Break_Unknown_type( const char p[], unsigned type );
float check( float f )
{ unsigned &w=*((unsigned*)&f); if( (w&0x7f800000)==0x7f800000 )w=0; return f;
}                                             // &= 0xff800000
short  i_read(FILE* W){ char s[2]; s[1]=getc( W ); s[0]=getc( W ); return *(short*)s; }
int    l_read(FILE* W){ char s[4],i=4; while( i>0 )s[--i]=getc(W); return *(int*)s; }
float  f_read(FILE* W){ char s[4],i=4; while( i>0 )s[--i]=getc(W); return check( *(float*)s ); }
double d_read(FILE* W){ char s[8],i=8; while( i>0 )s[--i]=getc(W); return *(double*)s; }
char  *s_read(FILE* W)                     // строка по целым словам по 4 байта
{ int l=int( l_read(W) );                  //
   if( l>2048 )Break_Unknown_type( "Lstr>2048",l );
    for( int i=0; i<(l+3)/4; i++ )fread( buf+i*4,4,1,W ); buf[l]=0; return buf;
}
static void i_read_16( long _f, int _l, short *_d )     // считывание блока 64к
{      fseek( F,_f,SEEK_SET ); fread( _d,2,_l,F );
       swab( (char*)_d,(char*)_d,unsigned(_l)<<1 );
}
//static void Dimensions_read()
//{ int i;
//  color( WHITE );
//  Tprintf( "%s[%d]",NC_Type_name[NC_DIMENSION],Nd=int( l_read(F) ) );
//  dim=(Dimension*)Allocate( Nd*sizeof( Dimension ),dim );
//  for( i=0; i<Nd; i++ )
//  { dim[i].S=strdup( s_read(F) );
//    dim[i].N=l_read(F);
//    Tln();                                         //
//    Tprintf( "%10s = %ld",dim[i].S,dim[i].N );     // количество точек в
//    if( !strcmp( dim[i].S,"npoint" ) )Np=dim[i].N; // координатном списке
//} }                                                //
static void Attribute_read( const char *_blk="" )
{
 long i,j,k,iD,N,im,iB=0;
 Real w;
  if( !strcmp( _blk,"clm"  ) )iB=1; else
  if( !strcmp( _blk,"anom" ) )iB=2; N=l_read(F);
  for( i=0; i<N; i++ )
  { Tln(); color( LIGHTGRAY );
    Tprintf( "%12s = ",s_read(F) );    im=0;
    if( !strcmp( buf,"source"       ) )im=-2; //
    if( !strcmp( buf,"title"        ) )im=-1; // Поиск записи с
    if( !strcmp( buf,"valid_range"  ) )im=1;  // экстремумами
    if( !strcmp( buf,"scale_factor" ) )im=2;  // scale factor
    if( !strcmp( buf,"units"        ) )im=3;  // единицы измерения
    if( !strcmp( buf,"add_offset"   ) )im=4;  // сдвиг нулевого отсчета
    color( YELLOW );                          //
    switch( iD=l_read(F) )
    { case NC_CHAR: Tprintf( s_read(F) );
           if( im==-2 )source=strdup( buf );
           if( im==-1 )title =strdup( buf );
           if( im==3  )units =strdup( buf );
           break;
      case NC_SHORT: k=l_read(F);
           for( j=0; j<k; j++ )
           { int l; Tprintf( "%d, ",l=i_read(F) );
             if(    im==1 )
             { if(  iB==1 )        //
               { if( j==0 )cMin=l; // Выборка экстремумов
                 if( j==1 )cMax=l; // для блока "clm"
               } else              //
               if(  iB==2 )        //
               { if( j==0 )yMin=l; // Выборка экстремумов
                 if( j==1 )yMax=l; // для блока "anom"
           } } } if( k&1 )i_read(F); break;            //
      case NC_FLOAT: k=l_read(F);
           for( j=0; j<k; j++ )
           { Tprintf( "%lg, ",w=f_read(F) );
             if( !j )
             { if(   iB==1 )      // "clm"
               { if( im==2 )cF=w; // Масштабный коэффициент
                 if( im==4 )cO=w; // Смешение нулевого отсчета
               } else             //
               if(   iB==2 )      // "anom"
               { if( im==2 )yF=w; // Масштабный коэффициент
                 if( im==4 )yO=w; // Смешение нулевого отсчета
           } } } break;           //
      case NC_DOUBLE: k=l_read(F);
           for( j=0; j<k; j++ ){ Tprintf( "%lg, ",d_read(F) ); } break;
      case NC_LONG: k=l_read(F);
           for( j=0; j<k; j++ ){ Tprintf( "%ld, ",l_read(F) ); } break;
      default: Break_Unknown_type( "Attribute_read",int( iD ) );
} } }
static void Variable_read()
{ long i,j,k,N;
  color( WHITE );
  Tln(); Tprintf( "%s[ %ld ]\n",NC_Type_name[NC_VARIABLE],N=l_read(F) );
  for( i=0; i<N; i++ )          // Имя Variable
  { color( CYAN );              //
    Tprintf( "%4s -> ",s_read(F) ); strcpy( buf+sbuf,buf );
    color( WHITE );  k=l_read(F);
    Tprintf( "%s[ ",k==1?"Vector": k==2?"Field":"Space" );
    for( j=0; j<k; j++ )
    { if( j )Tprintf( W2D( "•" ) );
             Tprintf( "%s",dim[int(l_read(F))].S );
    }        Tprintf( " ]" );
 // if( l_read(F)==NC_ATTRIBUTE )
        l_read(F); Attribute_read( buf+sbuf );
    Tpoint.x=Tv.mX/2; color( CYAN );
    Tprintf( "%s",NC_Type_name[int( l_read(F) )] );
    Tprintf( " / %7ld /",l_read(F) );
    Tprintf( " %ld\n", k=l_read(F) );
    if( !strcmp( buf+sbuf,"mon" ) )Mons=k; //
    if( !strcmp( buf+sbuf,"lat" ) )Lats=k; // Начало для списка широт
    if( !strcmp( buf+sbuf,"lon" ) )Lons=k; //   и долгот
    if( !strcmp( buf+sbuf,"clm" ) )cLms=k; //  первого блока обобщенных и
    if( !strcmp( buf+sbuf,"anom") )yLms=k; //  собственно исходных данных
} }                                        //
static void dim_free()
{ if( dim ){ for( int i=0; i<Nd; i++ )free(dim[i].S); Allocate(0,dim); dim=0; }
}                                 //
void GRD_View( char *Name );      // Surface
void Net_CDF( char *Name )        // World Ocean Surface Atlas
{                                 //
 short *_F=0,*_L=0,*_D=0,*_A=0;   // целочисленные массивы данных
 int   m=0,Ans=Center;            // Индекс текущего месяца
 long  i,y=0;           dim=NULL; // и года - для учета аномалий
 Bfont B( _Small_font,12 ); Np=0; // Вход в графический режим
  if( (F=fopen( Name,"rb" ))==0 )_Break( "? %s not open",Name );
         fread( buf,4,1,F ); buf[4]=0;
  if( memcmp(buf,"CDF\x01",4) )_Break( "? Mark: \"%s\", (not \"CDF\1\")",buf );
  Tv.Height( 0 );
  //
  // Последовательная проверка наличия данных
  //
  Ny=l_read(F);
  if( (i=l_read(F))==NC_DIMENSION )      // Dimensions_read();
  { Nd=l_read(F);                        // пространственные размерности
    dim=(Dimension*)Allocate( Nd*sizeof( Dimension ),dim );
    for( i=0; i<Nd; i++ )
    { dim[i].S=strdup( s_read(F) );
      dim[i].N=l_read(F);                               // количество точек в
      if( !strcmp( dim[i].S,"npoint" ) )Np=dim[i].N;    // координатном списке
    }
    if( !Np ){ dim_free(); GRD_View( Name ); exit( 0 ); }
    Tv_Graphics_Start(); setwindowtitle( "Tv - Atlas of Surface Marine Data" );
    color( LIGHTCYAN ); Tv.Height( 16 ); Tprintf( "%s %s = %ld years\n",__argv[0],Name,Ny );
    color( WHITE );     Tprintf( "%s[%d]",NC_Type_name[NC_DIMENSION],Nd );
    for( i=0; i<Nd; i++ )Tln(),Tprintf( "%10s = %ld",dim[i].S,dim[i].N );
    if(   ( i=l_read(F))==NC_ATTRIBUTE ){ Attribute_read();
      if( ( i=l_read(F))==NC_VARIABLE  ){ Variable_read(); goto Graphics;
  } } } Break_Unknown_type( "main",int( i ) );
                            //
#if 0                       // Так должно бы быть по стандарту NetCDF
  for(;;)switch( l_read() ) //
          case NC_DIMENSION: Dimensions_read(); break;
          case NC_ATTRIBUTE: Attribute_read();  break;
          case NC_VARIABLE:  Variable_read();   break;
         default: goto Error; // Данные другого типа в Атласе отсутствуют
#endif
  //    Переход к процедуре прорисовки карт (пока в градусах)
  //
Graphics: i=ftell( F );
//if( Np<=0      )_Break( "? список данных: npoint=%ld",   Np );
  if( cMax<=cMin )_Break( "? экстремумы: min..max=%ld,%ld",cMin,cMax );
  if( Mons!=i    )_Break( "? смещение месяцев: %ld->%ld",  Mons,i );
  for( i=0; i<11; i++ )Tprintf( "%d,", i_read(F) );
                       Tprintf( "%d\n",i_read(F) ); Tln(); color( LIGHTCYAN );
  Tv.Height( 16 );     Tprintf( W2D( "Выводить графику [n/y]" ) );
  Tv.Height( 0  );     pattern( SOLID_FILL,WHITE );
  Ans=Tv_getc();
  if( (Ans|0x20)!='y' && Ans!=_Enter )goto Exit_net_CDF;;
  setactivepage( 1 );
  Tv_Start_Mouse();             // Активизация параллельной мышки
  Frame_of_Map( Tvm,Map );      //
  //
  //    Собственно считывание исходных данных, начало
  //
            _F=(short*)Allocate( 64800u ); // Единичные (ледяные) поля
            _L=(short*)Allocate( 64800u ); //
            _D=(short*)Allocate( 64800u ); // Распределение памяти
  if( Ny>0 )_A=(short*)Allocate( 64800u ); //  для буферных массивов
                                           //  данных в оперативной памяти
  ///
  ///  Начало главного цикла интерактивных запросов
  ///
  for( m=0; Ans!=_Esc; )
  { int la=int( Map.Jx+1.5 ),
        fi=int( Map.Jy+1.5 ),
        lm=int( Map.Jx+Map.Lx+0.5 ), iN,   //
        fm=int( Map.Jy+Map.Ly+0.5 );       // Распределение размеров
   Point P,Q;                              // Граничные точки профиля
   Real D,dy,dx; Max = cO + cMax*cF;       // и экстремумов
                 Min = cO + cMin*cF; iN=Min==0;
 // if( y>0 )  Max+= yO + yMax*yF,
 //            Min+= yO + yMin*yF;
    if( !iN )                                  //
    { Msc=Max-cO; if( ( D=cO-Min )>Msc )Msc=D; // Случай знакопеременных
      Msc+=(yMax-yMin)*yF/30;                  //  данных с симметризацией
      Max=cO+Msc;                              //  цветовой палитры по
      Min=cO-Msc;                              //  заданному смещениею
      Msc*=2;                                  //
    } else Msc=Max-Min;                        // - или только масштаб
    //
    //  Подготовка граничных данных
    //   для интерполяции значений на линию профиля
    //
    if( Nl>0 )
    { memset( Line,0,Nl*sizeof(Real) );   P=Lp[0];
      memset( Lmsk,0,Nl*sizeof(char) );   Q=Lp[Nl-1];
      if( P.y>Q.y )dy=P.y,P.y=Q.y,Q.y=dy; P.y--; Q.y++;
      if( P.x>Q.x )dx=P.x,P.x=Q.x,Q.x=dx; P.x--; Q.x++;
      Ldst=hypot( P.y-Q.y,(P.x-Q.x)*cos( M_PI*(P.y+Q.y-180)/360 ) )*1.8526*60.0;
    }
    //  Считывание и прорисовка данных
    //  с выборкой профильной линии (при необходимости)
    //
    hidevisualpage();
    for( Ans=i=0; i<Np; i+=32400L )
    { int _l=min( 32400u,unsigned( Np-i ) );
               i_read_16( Lats+i*2,                          _l,_F );
               i_read_16( Lons+i*2,                          _l,_L );
               i_read_16( cLms+2L *(i+Np*m),                 _l,_D );
      if( y>0 )i_read_16( yLms+2L*((i+Np*m)+(y-1)*(Np*12+2)),_l,_A );
      for( int k=0; k<_l; k++ )
      { int Fi=_F[k],c,
            La=_L[k];
        if( La<la )La+=360; else
        if( La>lm )La-=360;
        if( Fi>=fi && Fi<=fm && La>=la && La<=lm )
        {                           D =cO+_D[k]*cF; //
          if( y>0 ){ if( cO==32767 )D =yO+_A[k]*yF; // случай со счетчиками
                              else  D+=yO+_A[k]*yF; //
                   } c=int( 240*( D-Min )/Msc );
          if( c>240 )c=240; else if( c<1 )c=(iN && !D) ?0:1;
//        pattern( SOLID_FILL,15+c );
          pattern( 15+c );
          bar( Tv_x( La-1 ),Tv_y( Fi-1 ),Tv_x( La ),Tv_y( Fi ) );
          if( Nl )                      //
          if( Fi>P.y )if( Fi<Q.y )      //
          if( La>P.x )if( La<Q.x )      // Проверка попадания точки
          for( c=0; c<Nl; c++ )         //  в прямоугольник профиля
          { Point &R=Lp[c];             //
            if( (dy=fabs( R.y-Fi ))<1.0 )
            if( (dx=fabs( R.x-La ))<1.0 )
            { if( !Lmsk[c] )Line[c]=D;
                      else  Line[c]+=(D-Line[c])*(1.0-dy)*(1.0-dx);
                          ++Lmsk[c];
        } } }
        Ans=Tv_getw();
        if( Ans==_Up||Ans==_Down||Ans==_Left||Ans==_Right ){ i=Np; break; }
    } }
    //
    //  Прорисовка профильной линии, если это возможно и необходимо
    //
    if( !Ans && Nl>0 )
    { int k,s,f[8];
     Real Mn=0,Mx=0;
      for( k=s=0; k<Nl; k++ )if( Lmsk[k] )
      { if( !s ){ Mx=Mn=Line[k]; ++s; } else
        { if( Mn>Line[k] )Mn=Line[k]; else
          if( Mx<Line[k] )Mx=Line[k];
      } }
      if( Mx-Mn>1e-6 )
      { static field Lvm={ 0,8,45,11,1 };
               Field Lmp={ 0,Mn,Nl-1.0,Mx-Mn };
        color( BLACK ); pattern( WHITE ); Tv_place( &Lvm,&Lmp );
        for( k=0; k<Nl; k++ )if( !Lmsk[k] )Line[k]=Mn;
        for( k=Tv_port.top; k<Tv_port.bottom; k++ )
        { s=int( 240*( Tv_Y( k )-Min )/Msc );
          if( s>240 )s=240; else if( s<1 )s=1;
          color( 15+s ); line( Tv_port.left,k,Tv_port.right,k );
        } color( WHITE );      f[3]=f[5]=Tv_port.top;
        for( k=1; k<Nl; k++ ){ f[0]=f[2]=Tv_x( k-1 );
                               f[4]=f[6]=Tv_x( k );
          f[1]=max((int)min( Tv_y(Line[k-1]),Tv_port.bottom),Tv_port.top );
          f[7]=max((int)min( Tv_y(Line[k] ), Tv_port.bottom),Tv_port.top );
                             fillpoly( 4,f );
        }                    moveto( Real( 0 ),Line[0] ); color( LIGHTGRAY );
        for( k=1; k<Nl; k++ )lineto( Real( k ),Line[k] ); color( BLACK );
        for( k=3; k>0; k-=2 )             //
        { setlinestyle( SOLID_LINE,0,k ); // THICK & NORM Width
          color( k==3?WHITE:BLACK );      //
          B.Text( South_West,Tv_port.right-3,Tv_port.top+3,   _Fmt( "%.4lg",Mx ) );
          B.Text( North_West,Tv_port.right-3,Tv_port.bottom-3,_Fmt( "%.4lg",Mn ) );
          B.Text( North_East,Tv_port.left+6, Tv_port.bottom-3,_Fmt( "%.5lg km",Ldst ) );
        } Tv_rect();
          Tv_place( &Lvm );
          Tv_place( 0,&Lmp );
    } }
    //  Подписи под рисунком выполняются до считывания данных,
    //  что необходимо при быстром (Shift+) листании картинок (заблок.)
    //
    Frame_of_Map(); // Подготовка рамок и условных обозначений
    if( y>0 )
    { fseek( F,yLms-4+(y-1)*(Np*12+2)*2,SEEK_SET );        // !!!
      sprintf( buf+sbuf/2,"%d ",i_read(F) );                   // ___ year
    } sprintf( buf," %9s %4s",_Months[m],y>0?buf+sbuf/2:"" ); //
    Tv.Text( South_West,Tv.mX,Tv_port.bottom+5,buf );
  showactivepage();
  ReAns: if( !Ans )Ans=Tv_Answer();
    switch( Ans )
    { case _Esc  : goto Exit_net_CDF;;
      case _Home : m=0;  y=0;                            continue;
      case _End  : m=11; y=Ny;                           continue;
      case _Left : if( m>0 )m--; else if( y>0 )y--,m=11; continue;
      case _Right: if( m<11)m++; else if( y<Ny)y++,m=0;  continue;
      case _Up   : if( y<Ny)y++;                         continue;
      case _Down : if( y>0 )y--;                         continue;
      case _Tab  : setvisualpage( 0 ); Tv_getc(); setvisualpage( 1 ); break;
      case _Del  :                                 //
      case   0xF4:                                 // ручное редактирование
       if( Ans==_Del )                             //
       { if( Nl ){ Allocate( 0,Line ); Line=0;     // Ans=Center;
                   Allocate( 0,Lmsk ); Lmsk=0;     //
                   Allocate( 0,Lp   ); Lp=0; Nl=0;
                 } else Map.Jx=Map.Jy=0,Map.Lx=360,Map.Ly=180;
       } else
       if( Tv_getk()&SHIFT )
       { static Real L=0; Map.Jx=(L=Angle(L+30)),Map.Jy=0,Map.Lx=360,Map.Ly=180;
       } else
       if( Tv_getk()&CTRL )Map.Jx=127,Map.Lx=40,Map.Jy=122,Map.Ly=30; else
       { static Mlist Coordinates[]=
         { { 0,0,"\t Координаты: °" }
         , { 1,4,"\t ф: %2.0lf",&Map.Jy },{ 0,4," + %.0lf",&Map.Ly }
         , { 1,4,"\t л: %2.0lf",&Map.Jx },{ 0,4," + %.0lf",&Map.Lx }
         };  Map.Jy-=90; Tmenu( Mlist( Coordinates ),-15,-3 );
             Map.Jy+=90;
       } //continue;   // восстановление исходного поля
      case _Space:
       if( Map.Ly>180 )Map.Ly=180;
       if( Map.Jy+Map.Ly>180 )Map.Ly=180-Map.Jy;
       Frame_of_Map( Tvm,Map ); continue;
    }  Ans=0; goto ReAns;
  }
Exit_net_CDF:;  // delete buf;       //
                // Allocate( 0,_F ), // Вектор широт
                // Allocate( 0,_L ), //   и долгот для дискретных данных
                // Allocate( 0,_D ), // осредненные за весь срок
                // Allocate( 0,_A ); //   и исходные океанографические данные
//_Break( "\n%s = %ld years\n%s [ %s ]\n%s\n",__argv[1],Ny,title,units,source );
}
//     Блок "независимой" обработки запросов
//              к клавиатуре и мышке
//
static Course Tv_Answer()
{
 Course Ans=Center;           //
 static int lB=0,x,lx,ly,k,  // Нажата ли левая
            rB=0,y,rx,ry,l;  //  или правая клавиша мышки
  if( Nl )                   //
  { color( WHITE );      moveto( Lp[0] ); setlinestyle( SOLID_LINE,0,THICK_WIDTH );
    for( k=1; k<Nl; k++ )lineto( Lp[k] ); setlinestyle( SOLID_LINE,0,NORM_WIDTH  );
    color( BLACK );      moveto( Lp[0] );
    for( k=1; k<Nl; k++ )lineto( Lp[k] );
  }
  for(;;)
  { Tv_GetCursor( x,y ); color( LIGHTBLUE ); //°
    l=sprintf( buf,"  (%6.1lf\xF8%-5.1lf)             ",Angle( Tv_X( x ) ),Tv_Y( y )-90 );
    if( (k=getpixel( x,y ))>=15 )sprintf( buf+l-12,"= %-10.4lg",(k-15)*Msc/240.0+Min );
    Tv.Text( South_West,Tv.mX-Tv.Tw*16,Tv_port.bottom+5,buf );
    setwritemode( XOR_PUT );
    if( lB ){ color( 127 ); rectangle( lx,ly,x,y ); }
    if( rB ){ color( 164 );      line( rx,ry,x,y ); } Ans=Tv_getc();
    if( lB ){ color( 127 ); rectangle( lx,ly,x,y ); }
    if( rB ){ color( 164 );      line( rx,ry,x,y ); } setwritemode( COPY_PUT );
    switch( Ans )
    { case _MousePressed:
        if( Tv_MouseState==_MouseLeftPress  )lB=1,lx=x,ly=y;
        if( Tv_MouseState==_MouseRightPress )rB=1,rx=x,ry=y; break;
      case _MouseReleased:                   //
      if( Tv_MouseState==_MouseLeftRelease ) // Новый прямоугольник
      { lB=0;                                //
        if( lx>x )k=x,x=lx,lx=k; Map.Jx=(int)Tv_X(lx),Map.Lx=int(Tv_X(x)-Map.Jx+.5);
        if( ly<y )k=y,y=ly,ly=k; Map.Jy=(int)Tv_Y(ly),Map.Ly=int(Tv_Y(y)-Map.Jy+.5);
        if( Map.Lx<15 ){ Map.Jx=(int)Map.Jx-7; Map.Lx=15; }
        if( Map.Ly<11 ){ Map.Jy=(int)Map.Jy-5; Map.Ly=11; } Ans=_Space;
      }
      if( Tv_MouseState==_MouseRightRelease )
      { Point P={ Tv_X( rx ),Tv_Y( ry ) },Q={ Tv_X( x ),Tv_Y( y ) };
        Real L=abs( Q-P ); //
        if( L>3.0 )         // Подготовка профильной линии (2p/диагональ)
        { Nl=int( 1+L/0.707106781 );
          Line=(Real*)Allocate( Nl*sizeof( Real ),Line );
          Lmsk=(char*)Allocate( Nl*sizeof( char ),Lmsk );
          Lp =(Point*)Allocate( Nl*sizeof( Point ),Lp  );
          for( k=0; k<Nl; k++  )Lp[k]=P+k*( Q-P )/( Nl-1 ); Ans=_Space;
        } rB=0;
      } default:;
    } if( Ans<=_Esc || Ans>=_Space )return Ans;
} }
//
// Блок процедур нижнего уровня для доступа к данным
//
static void Frame_of_Map()
{
 int x=Tv_port.left,
     y=Tv_port.bottom;
  pattern( WHITE ); color( BLUE ); bar( 0,Tv_port.bottom+4,Tv.mX,Tv.mY );
  Rainbow();
  if( title )Tv.Text( South_East,x,y+=5,title ); y+=Tv.Th-2;
  Tv.Height( Tv.Th/2 ); Tgo( 0,0 );              color( GREEN );
  Tprintf( "%ldм \x1B\x12\x1A F4,Sp,Del,Tab",coreleft()>>20 );
                                                 color( DARKGRAY );
  Tv.Text( South_East,x,y+1,source?source:"." ); color( BLUE );
  Tv.Text( North, Tv.mX-480,Tv.mY-8,_Fmt( "%.4lg ",fabs( Min )>1.4?Min:0 ) );
  Tv.Text( North_West,Tv.mX,Tv.mY-8,_Fmt( " %.4lg",fabs( Max )>1.4?Max:0 ) );
  Tv.Text( North, Tv.mX-240,Tv.mY-8,_Fmt( units?" %s ":"-",units ) );
  Tv.Height( 0 );
}
static int Scale( const int x ){ return x>10?20: x>5?10: x>2?5:1; }

static void Frame_of_Map( field t, Field m )
{
 int i,X=int( m.Jx ),x=int( m.Lx ),dx=int( Scale( x/16 ) ),
       Y=int( m.Jy ),y=int( m.Ly ),dy=int( Scale( y/12 ) );
   pattern( LIGHTGRAY );
   Tv_place( &t,&m );
   color( WHITE ); setlinestyle( SOLID_LINE,0,THICK_WIDTH );
   for( i=-720; i<=720; i+=180 )
   { if( i>=X && i<=X+x   )line( Real( i ),Real( Y ),Real( i ),Real( Y+y ) );
   } if( Y<=90 && Y+y>=90 )line( Real( X ),Real( 90),Real( X+x ),Real( 90) );
   color( DARKGRAY ); setlinestyle( SOLID_LINE,0,NORM_WIDTH );
   for( i=(X/dx)*dx; i<X+x; i+=dx )line( Real( i ),Real( Y ),Real( i ),Real( Y+y ) );
   for( i=((Y-90)/dy)*dy+90; i<Y+y; i+=dy )line( Real( X ),Real( i ),Real( X+x ),Real( i ) );
   Frame_of_Map();
}                     //
static void Rainbow() // Здесь готовится палитра радуги на 240 оттенков
{                     //
 static RGB_colors Pal[240]={ { 0,0,0 } };
 int i,k=0;
  for( k=0; k<240; k++ )
  { i=239-k;
    if( k<80  ){ Pal[i].r = 255; Pal[i].g =(255*k)/79;       } else
    if( k<120 ){ Pal[i].g = 255; Pal[i].r =(255*(120-k))/40; } else
    if( k<160 ){ Pal[i].g = 255; Pal[i].b =(255*(k-120))/39; } else
    if( k<200 ){ Pal[i].b = 255; Pal[i].g =(255*(200-k))/40; } else
               { Pal[i].b =(255*(280-k))/80;
                 Pal[i].r =(255*(k-200))/79;
  }            } Set_Palette( 16,240,Pal ); color( BLACK );
  for( k=0; k<=240; k++ )
  { pattern( k+15 );  bar( Tv.mX-(240-k)*2,Tv.mY-10,Tv.mX-(241-k)*2,Tv.mY );
  } pattern( WHITE ); line( Tv.mX-484,Tv.mY-10,Tv.mX-484,Tv.mY );
}
//
//      Варианты аварийного завершения с графической задержкой
//
static void _Break( const char msg[],... )
{ char str[132];
  va_list V; va_start( V,msg ); vsprintf( str,msg,V ); va_end( V );
  // color( LIGHTRED ); Tln(); Tprintf( str ); Tv_getc();
  Break( str );
}
static void Break_Unknown_type( const char p[], unsigned t )
{ _Break( "\n%s: Type Error \"%s\" = %u",p,NC_Type_name[t<13?t:0],t );
}
#if 0
///
///     особенности обработки чисел с плавающей запятой
///     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define VAX_SNG_BIAS	0x81
#define IEEE_SNG_BIAS	0x7f
//!
//!     на случай проблем с кодировкой чисел с плавающей запятой !!!
//!
float  _f_read(FILE* W)
{
#if 1                // WORDS_BIGENDIAN
  struct ieee_single // What IEEE single precision floating point looks like on a Vax
  { unsigned int sign    : 1;
    unsigned int exp     : 8;
    unsigned int mantissa: 23;;
  } is;
  struct vax_single // Vax single precision floating point
  { unsigned int mantissa2 : 16;
    unsigned int sign      : 1;
    unsigned int exp       : 8;
    unsigned int mantissa1 : 7;
  } vs;
  static struct sgl_limits
  {      struct vax_single s;
	 struct ieee_single ieee;
  } max={ { 0xffff, 0x0, 0xff, 0x7f },	/* Max Vax */
	  { 0x0, 0xff, 0x0 } },		/* Max IEEE */
    min={ { 0x0, 0x0, 0x0, 0x0 },	/* Min Vax */
	  { 0x0, 0x0, 0x0 } };		/* Min IEEE */
#else
  struct ieee_single // What IEEE single precision floating point looks like on a Vax
  { unsigned int mantissa: 23;;
    unsigned int exp     : 8;
    unsigned int sign    : 1;
  } is;
  struct vax_single // Vax single precision floating point
  { unsigned int mantissa1 : 7;
    unsigned int exp       : 8;
    unsigned int sign      : 1;
    unsigned int mantissa2 : 16;
  } vs;
  static struct sgl_limits
  {      struct vax_single s;
	 struct ieee_single ieee;
  } max={ { 0x7f, 0xff, 0x0, 0xffff },	/* Max Vax */
	  { 0x0, 0xff, 0x0 } },		/* Max IEEE */
    min={ { 0x0, 0x0, 0x0, 0x0 },	/* Min Vax */
	  { 0x0, 0x0, 0x0 } };		/* Min IEEE */
#endif
  //int i=0; while( i<4 )((char*)&vs)[i++]=getc( W );
  int i=4; while( i>0 )((char*)&vs)[--i]=getc( W ); Tprintf( " <<< %X >>> ",*((long*)&vs) );
  //int i=2; while( i>0 )((short*)&vs)[--i]=i_read( W );
  //fread( &vs,4,1,W );

  switch( vs.exp )
  { case 0: is=min.ieee; break; // all vax float with zero exponent map to zero
    case 1:
    case 2:  /* These will map to subnormals */
      is.exp = 0 ;
      is.mantissa = (vs.mantissa1 << 16) | vs.mantissa2; // lose some precision
      is.mantissa >>= 3 - vs.exp ;
      is.mantissa += (1 << (20 + vs.exp)) ; break ;
    case 0xff : /* max.s.exp */
      if( vs.mantissa2 == max.s.mantissa2
       && vs.mantissa1 == max.s.mantissa1 ) // map largest vax float to ieee infinity
	{ is = max.ieee ; break ; 	  } // else, fall thru
    default : is.exp = vs.exp - VAX_SNG_BIAS + IEEE_SNG_BIAS;
	      is.mantissa = (vs.mantissa1 << 16) | vs.mantissa2;
   }          is.sign = vs.sign;
   Tprintf( " <<< %X >>> ",*((long*)&is) );
  return *((float*)&is);
// return getf( W );
// char s[4],i=4; while( i>0 )s[--i]=getc(W); return *(float*)s;
}
#endif




