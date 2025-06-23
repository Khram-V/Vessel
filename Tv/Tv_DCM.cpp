//
//      Считывание МореоГрафной кривой оцифрованной в Design-CAD(ascii)
//
//                                  1993 Ноябрь, В.Храмушин, Южно-Сахалинск
#include "Tv.h"
#include "OS/Curve.h"

static Point *P=NULL;    // Блок для 201 точки изображения
static point *p=NULL;    // Блок для 201 точки терминала
static Base B;           // Базис косоугольных координат
static int Active=0,     // Признаки активных данных
                         //   0x01 - Признак открытого файла
                         //   0x02 - 1-dw2; 0-dc2 - двоичный или ASCII файл
                         //   0x04 - 1-bln  - признак текстового файла .BLN
       The_Layer=1,      // Текущий уровень
       Entity_Type,      // Собственно тип объекта
       Number_of_Points, // Количество точек
       Line_Type=0,      // Тип линии            (0-8)
       The_Color=1,      // Цвет объекта         (1-16)
       Group_Number=0;   // Номер группы (0-без группы)
static Real              //
       Points_Scale,     // Толщина трех точек и терминала
       Pattern_Scale,    // Масштаб для закраски
       Line_Width,       // Толщина линии
       gScale=1.0;       // Масштаб для пересчета текста и графики
static FILE *DC=NULL,*DT=NULL; // Исходный файл и файл записи текстовых строк
static char *Msg=NULL; // [MAX_PATH*2] Текущая/рабочая строчка текстового файла
static bool ANSI=true; // если исходный файл в кодировке ANS-Windows
//
//      Распределение цветов палитры в стиле программы DesignCAD
//
enum{ _WHITE,_BLACK,_BROWN,_GREEN,_BLUE,_LIGHTRED,_CYAN,_MAGENTA,_DARKGRAY,
      _LIGHTGRAY,_RED,_LIGHTGREEN,_LIGHTBLUE,_YELLOW,_LIGHTCYAN,_LIGHTMAGENTA
    };
static struct Field_Window: Window
{ int  Type;
  char *Name;
  int  DCReadItem();
  void Draw( Field& wF );
  void Initial(){ Focus(); Draw( F ); }
} Map;

struct Set_Line_Width
{      Set_Line_Width()
         { if( Points_Scale<Line_Width )
                         setlinestyle( SOLID_LINE,0,THICK_WIDTH ); }
      ~Set_Line_Width(){ setlinestyle( SOLID_LINE,0,NORM_WIDTH ); }
};
//   Считывание одной строки по заданному формату
//
void static DCRead( const char *Fmt=0,... )
{ while( fgets( Msg,MAX_PATH*2-1,DC ) )if( Msg[0]!=';' || !Fmt )break;
  if( Msg && Fmt )
    { va_list A; va_start( A,Fmt ); vsscanf( Msg,Fmt,A ); va_end( A ); }
}
static float DCget(){ float w; fread( &w,1,4,DC ); return w; }
static void DColor(){ color( ((The_Color-1)%15)+1 ); }

void static DCReadItemHeader()
{                         //
 int N=Number_of_Points;  // Длина заголовка в байтах
 static int _EnS=2;       // Сохраняемое значение счетчика
  if( N>0 )Number_of_Points=get2( DC ); else
           Number_of_Points=_EnS;
      _EnS=Number_of_Points;
  if( N>2 ){ The_Color=get2( DC ); if( The_Color<0 )The_Color=-The_Color; }
  if( N>4 )get2( DC ); //
  if( N>6 )get2( DC ); // RGB color
  if( N>8 )get2( DC ); // Solid ID (2D=0)
  if( N>10 )Group_Number=get2( DC );
  if( N>12 )Line_Type  = get2( DC );
  if( N>14 ){ Line_Width=DCget(); N-=2; }
  if( N>18 ){ Pattern_Scale=DCget(); N-=4; }
  while( --N>=16 )getc( DC );
  DColor();
}
void static DCReadItemPoints()
{ if( Number_of_Points>201u )
      Break( "Entity_Type=%d, Number_of_Points=%u > 201\n",
      Entity_Type,Number_of_Points );
 int i;
  if( Active&2 )
  { for( i=0; i<Number_of_Points; i++ )P[i].x=DCget();
    for( i=0; i<Number_of_Points; i++ )P[i].y=DCget();
  } else
    for( i=0; i<Number_of_Points; i++ )DCRead( "%lg%lg",&(P[i].x),&(P[i].y) );
}
void static _01_Line()
{ int i;
  Set_Line_Width Auto;
  if( Active&2 )DCReadItemHeader();
                DCReadItemPoints(); moveto( P[0] );
  if( Entity_Type!=4 )
  for( i=1; i<Number_of_Points; i++ )
  { if( Line_Type==-1 && (i&1)==0)moveto( P[i] ); else lineto( P[i] ); } else
  { int j=1,N=Number_of_Points; Real l,L=0;
    for( i=1; i<N; i++ )
    { l=abs( P[i]-P[j-1] ); if( l>0.0 ){ L+=l; P[j]=P[i]; j++; } } N=j;
    { Curve Cy( N ),
            Cx( N ); L=0;
      for( i=0; i<N; i++ )
      { if( i )Cy[i].x=Cx[i].x=L+=abs( P[i]-P[i-1] );
               Cy[i].y=P[i].y;
               Cx[i].y=P[i].x;
      }        Cy.SpLine(); N =( N-1 )*6+1;
               Cx.SpLine(); L/=( N-1 );
      for( i=1; i<N; i++ )lineto( Cx( L*i ),Cy( L*i ) );
    }
  }
}
void static _03_Text()
{ int i,l;
  if( Active&2 )
  { DCReadItemHeader();
    DCReadItemPoints(); l=get2( DC ); fread( Msg,1,l,DC ); Msg[l]=0;
  } else
  { for( i=0; i<Number_of_Points; i++ )DCRead( "%lf%lf",&(P[i].x),&(P[i].y) );
    DCRead(); strcut( Msg );
    if( ANSI )CharToOemA( Msg,Msg ); //! OEM - русские шрифты сходу заменяются
  }
  if( Entity_Type!=22 )
  { if( Number_of_Points<3 )Tv.OutText( P[0],P[1],Msg ); else
    if( Number_of_Points<4 )Tv.OutText( P[0],P[1],P[2],Msg ); else
                            Tv.OutText( P[0],P[1],P[2],P[3],Msg );
  } else
  if( (l=strlen( Msg ))>0 )
  {
   Real dS,dA=abs( P[3]-P[0] );
   Point R,_p={ 0,dA },
           _q={ dA,0 };
    B.Axis( _q,_p );
    dA=arg( B.Goint( P[2]-P[0] )/B.Goint( P[1]-P[0] ) )/l;
    if( P[3]==P[2] )dA=-dA;
    R=B.Goint( P[3]-P[0] );
    dS=dA/fabs( Tv.StrLen( Msg )/Real( l ) );
    for( i=0; i<l; i++ )
    { static char s[2]={ 0x20,0 }; s[0]=Msg[i];
      Tv.OutText
      ( P[0]+B.Goext( R ),
        P[0]+B.Goext( R*polar( Tv.StrLen( s )*dS ) ),s
      ); R*=polar( dA );
  } }
}
//      Эллипсы проходят с кодом 2, эллиптические дуги с кодом 7
//      при этом оси могут хранится в первых двух точках, если их
//      общее количество меньше 7-ми, или в последних (6-7), тогда
//      в первых двух будет находится точки начала и конца эллипса
//
void static _02_Ellipse()
{
 const Real n=48.0;
 Real dA=M_PI*( 2.0/n );
 Point R={ 1.0,0.0 };
 Set_Line_Width Auto;
  if( Active&2 )DCReadItemHeader();
                DCReadItemPoints();
  if( Number_of_Points<=5 )B.Axis( P[1]-P[0],P[2]-P[0] ); else
  {                        B.Axis( P[5]-P[0],P[6]-P[0] );
                           R=dir( B.Goint( P[1]-P[0] ) );
      if( Entity_Type!=2 )dA=arg( B.Goint( P[2]-P[0] )/R )/n;
  }                       moveto( P[0]+B.Goext( R ) );
  for( int k=0; k<n; k++ )lineto( P[0]+B.Goext( R*=polar( dA ) ) );
}
void static _16_Circle()
{ if( Active&2 )DCReadItemHeader();
                DCReadItemPoints();
  for( int i=0; i<3; i++ )p[i]=P[i]; p[1]-=p[0]; p[2]-=p[0];
  if( ( p[1].x || p[1].y )
   && ( p[2].x || p[2].y ) )
  { if( Entity_Type==5 ){ p[3]=p[2]; p[2]=p[1]; p[1]=p[3]; }
    if( P[1]==P[2] )
    circle( int( p[0].x ),int( p[0].y ),int( hypot( p[1].x,p[1].y )+0.5 ) );
    else
    arc( int( p[0].x ),int( p[0].y ),
         int( atan2( p[1].x,p[1].y )*180.0/M_PI-89.5 ),
         int( atan2( p[2].x,p[2].y )*180.0/M_PI-89.5 ),
         int( hypot( p[1].x,p[1].y )+0.5 )
       );
} }
void static _17_Hatch()
{ if( Active&2 )DCReadItemHeader();
                DCReadItemPoints();
  if( Active&2 )fread( Msg,get2( DC ),1,DC ); else DCRead();
  for( int i=0; i<Number_of_Points; i++ )p[i]=P[i];
   pattern( CLOSE_DOT_FILL,The_Color ); color( 0 );
  fillpoly( Number_of_Points,(int*)p );
   pattern( EMPTY_FILL,1 ); DColor();
}
void static _21_Layer()
{ if( Active&2 )DCReadItemHeader(); The_Layer=Number_of_Points;
}
void static _24_Arrow()
{ int i;
  if( Active&2 )DCReadItemHeader();
                DCReadItemPoints();    moveto( P[0] );
  for( i=1; i<Number_of_Points-2; i++ )lineto( P[i] );
  if( Entity_Type==70 )
  { point q=P[i],r=P[i+1]; r-=q;
      circle( q.x,q.y,int( hypot( r.x,r.y )+0.5 ) );
  }
//    color( LIGHTGREEN );
//    needle( P[i-1]+P[i],P[i-1]+P[i+1] );
}
int Field_Window::DCReadItem()
{ if( Active&2 )
  { Entity_Type     =get2( DC ); // Номер объекта
    Number_of_Points=get2( DC ); // Длина заголовка в байтах
  }else
  { DCRead( "%d%d%lf%lf%d%d", //
         &Entity_Type,      // Собственно тип объекта
         &Number_of_Points, // Количество точек
         &Pattern_Scale,    // Масштаб для закраски
         &Line_Width,       // Толщина линии
         &Line_Type,        // Тип линии                (0-8)
         &The_Color         // Цвет объекта             (1-16)
    );   DColor();
  }
  if( feof( DC ) )return EOF;
  { switch( Entity_Type )
    { case 32: Entity_Type=0; break;
      case 34:
      case 33:                //
      case 31: Entity_Type=1; // Линии после конвертации файлов AutoCAD-а
      case  4: // Curve       //
      case 11: // Bezier
      case  1: /* Line */   _01_Line();    break;
      case  7: // Back Ellipse
      case  2: /* Ellipse*/ _02_Ellipse(); break;
      case 13: // Текст в DC3
      case 15: // Attribute
      case 22: // Text Arc
      case  3: /* Text   */ _03_Text();    break;
      case  5: // Circular or Elliptical Arc
      case 16: // Circle,Circular,Arc
                            _16_Circle();  break;
      case 21: /* Layer  */ _21_Layer();   break;
      case 17: /* Hatch  */ _17_Hatch();   break;
      case 70:
      case 24: /* Arrow */  _24_Arrow();   break;
      case 20: if( (Active&2)==0 )
               { DCRead(); DCRead( "%lf",&gScale ); DCRead(); //Number_of_Points=14; goto r_Empty;
               } break;
      case 40:
      case 41: if( (Active&2)==0 )
               { DCRead( "%lf",&gScale ); --Number_of_Points; Focus();
               } goto r_Empty;
      case 42: if( (Active&2)==0 )
               { DCRead( "%lf",&gScale );
                 while( --Number_of_Points>0 )DCRead(); Focus();
               } break;
      case 1309: if( Active&2 )
               { gScale=DCget(); Number_of_Points-=4; Focus();
               }
     default: r_Empty:
     if( Active&2 )
      { if( Entity_Type>=1300 )
        { for( int i=0; i<Number_of_Points; i++ )getc( DC );
                          Number_of_Points=0;
        } else
        { DCReadItemHeader(); DCReadItemPoints(); }
      } else
      for( int i=0; i<Number_of_Points; i++ )DCRead( "%lf%lf",&(P[i].x),&(P[i].y) );
      for( int i=0; i<Number_of_Points; i++ )line( P[i],P[i] );
    }
  } return feof( DC );
}

//#include "simplex_font.c++"

void Field_Window::Draw( Field& wF ) // Typ = 5,6 типа входного файла
{ //
  // Открытие или установка файла в начало считывания
  //
  Focus();        pattern( SOLID_FILL,255 );
  Tv_bar();       Active=0;             //
  if( Type==5 )                         // ASCII.DC2
  { if( (DC=fopen( Name,"rt" ))!=0 )    //
    { Active=1; DCRead( "%lf%lf%lf%lf",&(F.Jx),&(F.Jy),&(F.Lx),&(F.Ly) );
      Focus();
  } } else                              //
  if( Type==6 )                         // BINARY.DW2
  { if( (DC=fopen( Name,"rb" ))!=0 )    //
    { if( get2( DC )==1000 )            // ? 2-D
      {   get2( DC );                   // Version
          get2( DC );                   // 4-single, 8-double
        if( get2( DC )==1200 )          // Drawing size entity
        if( get2( DC )==16   )          //
        { F.Jx=DCget(); F.Jy=DCget();
          F.Lx=DCget(); F.Ly=DCget(); Active=3; Focus(); // 0x01 | 0x02
    } } }
  } else                        // SURFER - Golden SoftWare .bln
  if( Type==7 )                 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  { int j,k,l;                  // Текстовый формат контурных карт
    Real Y,X;
    if( (DC=fopen( Name,"rt" ))!=0 )
    { for( k=0; !feof( DC ) && !ferror( DC ); k++ )
      { l=0; DCRead( "%d",&l );      // количество точек в контуре
        for( j=0; j<l; j++ )    // выборка чисел начинется с новой строки
        { Y=X=0.0; DCRead( "%lg%lg",&Y,&X );
          if( !k && !j )F.Jx=F.Lx=X,      // Выбор экстремумов для файла .BLN
                        F.Jy=F.Ly=Y; else
          { if( X<F.Jx )F.Jx=X; else if( X>F.Lx )F.Lx=X;
            if( Y<F.Jy )F.Jy=Y; else if( Y>F.Ly )F.Ly=Y;
          }
        } Ghelp( "%5d / %d",k,l );        //
      } Ghelp();    F.Lx-=F.Jx; Active=5; // 0x01 | 0x04
      rewind( DC ); F.Ly-=F.Jy;           //
  } }
  if( !Active ){ if( DC )fclose( DC ); Break( "? %s не открыт",Name ); } else
  { Real Ly,Lx,dF,dL;
    Ly=Tv_port.bottom-Tv_port.top;       // Выборка текущих размерностей и
    Lx=Tv_port.right-Tv_port.left;       // пересчет их в меркаторскую проекцию
    if( !wF.Ly || !wF.Lx )wF=F;
    { int ax,ay; getaspectratio( &ax,&ay );
      if( wF.Lx<0 )wF.Jx+=wF.Lx,wF.Lx=-wF.Lx;     // на случай опрокидывания
      if( wF.Ly<0 )wF.Jy+=wF.Ly,wF.Ly=-wF.Ly;     // размерности прямоугольника
          dF = ax*wF.Ly/Ly/ay;
          dL = ay*wF.Lx/Lx/ax;
      if( dF>dL ){ dL=ax*dF*Lx/ay; wF.Jx-=dL/2-wF.Lx/2; wF.Lx=dL; }
            else { dF=ay*dL*Ly/ax; wF.Jy-=dF/2-wF.Ly/2; wF.Ly=dF; }
      F=wF;
      Focus();                             // Установка пересчитанного масштаба
    }
    if( !(Active&0x04) )
    { Points_Scale=hypot( Tv_Y( 0 )-Tv_Y( 1 ),
                          Tv_X( 0 )-Tv_X( 1 ) )*2;
      P=(Point*)Allocate( 201*sizeof( Point ) );
      p=(point*)Allocate( 201*sizeof( point ) );
//    Tv.Font( _Small_font );
      Tv.Font( _Simplex_font );
      while( !DCReadItem() )                       // Прорисовка Design-CAD 5/6
      if( DT )if( Entity_Type==1 || Entity_Type==4 )
      { fprintf( DT,"%3d %2u",Number_of_Points,Entity_Type );
        for( int i=0; i<Number_of_Points; i++ )
        fprintf( DT," %7.4lf %6.4lf",P[i].y/gScale,P[i].x/gScale );
        fprintf( DT,"\n" );
      } Allocate( 0,P );
        Allocate( 0,p );
    } else
    { int j,l;
     Real Y,X,sY=0,sX=0;
     char *Comment=Msg+(MAX_PATH*10); // [=MAX_PATH*2]; //136];
      color( GREEN );
      while( !feof( DC ) && !ferror( DC ) )
      { DCRead( "%d",&l );           // количество точек в контуре
       char* Ct;                     //
        Ct=strchr( Msg,'"' );
        if( !Ct )Ct=strchr( Msg,'\'' ); else
          { char* ct=strrchr( Ct+1,'"' ); if( ct )ct[0]=0;
          }
        if( Ct )
          { strcpy( Comment,Ct+1 );
            if( Ct[1]==0 )Ct=NULL; else{ sY=sX=0; strcut( Comment ); }
          }
        for( j=0; j<l; j++ )        //
        { DCRead( "%lg%lg",&Y,&X ); // выборка чисел с новой строки
          if( Ct ){ sY+=Y; sX+=X; } //
          if( !j )moveto( X,Y ); else lineto( X,Y );
        }
        if( Ct )
        { int c=color( CYAN ); Tv.Text( East,sX/l,sY/l,Comment ); color( c );
      } }
    } fclose( DC );                            // Восстановление исходных
      Tv.Font( 0,0 );                          // графических параметров
      pattern( Active&4?(int)WHITE:(int)_WHITE );
  }
}
// enum{ _0,_1=51<<2,_2=104<<2,_3=153<<2,_4=204<<2,_5=255<<2 }
enum{ _0,_1=51,$1=84,_2=104,_3=153,$3=192,_4=204,_5=255 };
                                        //
static RGB_colors DC_colors_16[16] =    //
        { { _5,_5,_5,0 },{ _0,_0,_0,0 } //  0 - WHITE,    BLACK
        , { _0,_0,_2,0 },{ _0,_2,_0,0 } //  2 - BROWN,    GREEN
        , { _3,_0,_0,0 },{ _0,_1,_3,0 } //  4 - BLUE,     LIGHTBROWN
        , { _2,_3,_0,0 },{ _3,_0,_3,0 } //  6 - CYAN,     LIGHTMAGENTA
        , { $1,$1,$1,0 },{ _3,_3,_3,0 } //  8 - GRAY,     LIGHTGRAY
        , { _0,_0,_5,0 },{ _1,_3,_0,0 } // 10 - RED,      LIGHTGREEN
        , { _5,_3,_1,0 },{ _1,$3,_4,0 } // 12 - LIGHTBLUE,YELLOW
        , { _4,_4,_0,0 },{ _5,_2,_5,0 } // 14 - LIGHTCYAN,LIGHTMAGENTA
        };                              //
static Course Console();                // Процедура интерактивного управления
                                        // Инициализация графического
void DCM_View( char *Name, const int Type )
{ Tv_Graphics_Start(); setwindowtitle( "Tv - Design-CAD ver.5,6" );
  if( !Msg )Msg=(char*)malloc( MAX_PATH*12 );
  pattern( SOLID_FILL,_WHITE );
  bkcolor( _WHITE );
  if( Type!=7 )Set_Palette( 0,16,DC_colors_16 );
//  { Set_Palette(  0,8,DC_colors_16   );
//    Set_Palette( 56,8,DC_colors_16+8 );
//    Set_Palette( 20,1,DC_colors_16+6 );
//  }
  clear();
  Tv_Start_Mouse();
  Map.Name=Name;
  Map.Type=Type;
  Map.Initial();
  All.Initial( " 1:1 ",0,0 ); while( Console() ) ;
}
static int Info( int x,int y, bool st )
{ static int ActIn=0;           // 1 надпись установлена
  if( Map.Is( x,y ) )           //
  { if( !ActIn ){ Map.Focus(); ActIn=true; }
         Ghelp( "%s,  <Space>OEM\\ANSI  <W>rite  = { %.5lg,%-.4lg } ",
           W2D( Map.Name ),Tv_X(x)/gScale,Tv_Y(y)/gScale );
  } else
  if( ActIn ){ Ghelp(); ActIn=0; }
  color( (Active&4?GREEN^WHITE:_MAGENTA^_WHITE) );
  return ActIn;
}
static Course Console()         //
{                               // Главная процедура организации
 int x,y,Ans;                   //  диалога с консолью ЭВМ
 Field F={ 0,0,0,0 };           //
  for( ;; )
  { if( (Ans=Map.Console( Info ))==_Esc )return Center;
        Tv_GetCursor( x,y );
    if( Ans==_MousePressed )
    { if( All.Is( x,y ) ){ F.Jx=F.Lx=F.Jy=F.Ly=0; Map.Draw( F ); All.Draw(); }
    } else
    if( Ans==-1 )
    { F.Jy=Tv_Y( Map._y ); F.Ly=Tv_Y( Map.y )-F.Jy;
      F.Jx=Tv_X( Map._x ); F.Lx=Tv_X( Map.x )-F.Jx;
      Map.Draw( F );
      All.Draw();
    } else
    if( (Ans|=0x20)=='w' )
    { if( (DT=fopen( fext( strcpy( Msg,Map.Name ),"txt" ),"wt" ))!=NULL )
      { Map.Draw( F );
        All.Draw();
        fclose( DT ); DT=NULL;
      }
    } else
    if( Ans==_Space )
    { ANSI^=true;
      Map.Draw( F );
      All.Draw();
    }
  }
}
