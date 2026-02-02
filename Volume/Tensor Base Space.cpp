/**
 *      Второй пакет поверочных математических алгоритмов с
 *      использованием языка программирования C++ OpenGL + Window-Place
 *      (c)2015, В.Храмушин, Санкт-Петербургский государственный университет
 *      (c)2016, + Южно-Сахалинск
 **/
#include "..\Math\Model.h"
#include "..\Math\Volume.h"
#include "..\Window\ConIO.h"
const Real* arrow( const Real *_a,const Real *_b,_Real l, const colors clr=empty )
{ Vector &a=*(Vector*)_a,&b=*(Vector*)_b,d=a-b; d*=l/abs( d );
  Vector e={ d.z/5,d.x/5,d.y/5 },f={ e.z,e.x,e.y },g=b+d/2; line( a,d+=b,clr );
   glBegin( GL_LINE_LOOP ),dot( g ),dot( d+e ),dot( b ),dot( d-e ),
                           dot( g ),dot( d+f ),dot( b ),dot( d-f ),
                           dot( g ),glEnd(); return b;
}
static void axis( Place &P,_Real L,_Real Y,_Real Z,
  const char *x,const char *y,const char *z, const colors clr=empty )
{ const Real l=L/100;
   arrow( (Vector){ 0,0,-Z },(Vector){ 0,0,Z },l,clr ),
   arrow( (Vector){ 0,-Y,0 },(Vector){ 0,Y,0 },l ),
   arrow( (Vector){ -L,0,0 },(Vector){ L,0,0 },l ); color( clr,-0.5 );
  P.Text( _North,0,0,Z,z )
   .Text( _North,0,Y+l,0,y )
   .Text( _North_East,L+l,0,0,x );
}
void put( int x,int y, Real* ); // вывод справочных данных на текстовую консоль

//!   сеточное пространство Space [по умолчанию] = ( 24,16,20,{0,0,0},1 )
static Space Sp( 12,8,10,          // 6(вправо),4(вверх),5(ближе)
        (Point){-0.5,-0.5,-0.5},   // относительное смещение до центра сетки
                 1.0 );            // масштаб - шаг сетки
//
//    Поверочная программа, работающая с графическим пакетом OpenGL.
//    Определение основных рабочих параметров.
//
static Real Screen_Size=max( max( Sp.nX,Sp.nY ),Sp.nZ );
static long STime=ElapsedTime();
static View                        // пространство, отображаемое на экране ЭВМ
 Win( "Tensor-Part, Surface-Side and Volume-Space",0,0,1600,768,Screen_Size*2 );

static enum Model_Type{ None,Cube,Torus,Sphere,Dipole } MTyp=Dipole;
static void Model_Draw( Model_Type typ );       // Простые равнообъёмные фигуры
static Model Mxl;                  // собственный базис геометрического объекта
static bool painting=true,         // объемная или контурная прорисовка фигуры
            rotate_OpenGL=false,   // вращение всего пространства OpenGL
            moving_Model=false,    // собственное перемещение объекта
            Draw_grid = false;     // 0-чисто; 1-всё пространство
static int  Draw_flow = 2;         // 0-нет; 1-вызванное; 2-полное; 3-вместе
static Real Dipole_offset=0.0,     // скоростная деформация диполя
            Particle_volume=1.0,   // контрольный объем моделируемой частицы
            fx=.125,fy=.125,fz=0;  // шаг с исключением разметки по вертикали
                                   //
static Course Configuration()      // меню настройки эксперимента
{ const char *styp[]={ "None","Cube","Torus","Sphere","Dipole" };
  const char *flow[]={ " нет ","вызван","полная","вместе" };
  static int K=-1;                      // текущая позиция в меню
         int Typ=MTyp;                  // тип изображаемого объекта
  Mlist Menu_C[]=                       // собственно меню запросов и настроек
   { { 0,0,"   <<<- видео-конфигурация ->>>" }
   , { 2,0,"Рисуемый тип корпускулы: " },{ 0,6,styp[Typ] }
   , { 1,0,"Сеть числовых узлов/ячеек: "},{ 0,4,Draw_grid?"есть":"нет " }
   , { 1,0,"Скорость: "},{ 0,4,flow[Draw_flow] }
   , { 0,3," = %3.1lf",&fx },{ 0,3,",%3.1lf",&fy },{ 0,3,",%3.1lf",&fz }
   };
  TextMenu T( Mlist( Menu_C ),&Win,-1,1 );
  switch( K=T.Answer( K ) )
    { case 2: if( ++Typ>Dipole )Typ=None; MTyp=Model_Type( Typ ); break;
      case 4: Draw_grid^=1;   break;
      case 6: --Draw_flow&=3; break;
      case _Esc: return _Esc;
    }
    fx=min( 10.0,fabs( fx ) );
    fy=min( 10.0,fabs( fy ) );
    fz=min( 10.0,fabs( fz ) );
  return _F4;
}
#define a( t ) ( R*M_PI*sin( fmod( angle/60/t,M_PI*2 ) ) )
static bool drawing()
{ Real angle=(ElapsedTime()-STime)/60; // время запуска программы в милиминутах
  Real R=1.0;
    Win.Activate();                   // привязка к основному графическому окну
    glClearColor( rotate_OpenGL?0.75:0.9,0.95,0.99,1 );      // цвет фона окна
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );   // полная расчистка
//    glTranslatef( 0.0,0.0,-0.5 );                          // отдаление на 1.0
    if( rotate_OpenGL )glRotated(a(7)*120,a(.5),a(.4),1); // поворот всей сцены
    if( moving_Model )                                   // модель выставляется
    { (Tensor)Mxl*=Krylov( a(1),a(1.3),a(1.6) );
//    (Tensor)Mxl=Euler( Mxl,1,1.3,1.6 );
//    Mxl.Identity();//.Euler( a(1),a(1.3),a(1.6) );    // в собственном базисе
      R=pow( Real( Mxl ),0.33 );
      Mxl=(Point){ a(1),a(1.2)/2,a(1.6)/3 }; //*pow( Real( Mxl ),0.33 );
//    Mxl=(Tensor){ dir( (Vector){ sin(angle),cos(angle),sin(angle/3) } ) };
      glTranslated( Mxl.X/-30,Mxl.Y/-20,-Mxl.Z/60 );
    }
    axis( Win,Screen_Size/8,Screen_Size/8,Screen_Size/8,"X","Y","Z" );
    Model_Draw( MTyp );
    put( 2,4,(Real*)(&Mxl) );
//
//      Порисуем в/из Space
//
  if( Draw_grid )
  { int x,y,z;
    for( z=0; z<=Sp.nZ; z++ )
    for( y=0; y<=Sp.nY; y++ )
     if( y+z==0 )arrow( Sp.out((Vector){0,y,z}),Sp.out((Vector){Sp.nX,y,z}),0.1,lightred );
          else    line( Sp.out((Vector){0,y,z}),Sp.out((Vector){Sp.nX,y,z}),lightmagenta );
    for( y=0; y<=Sp.nY; y++ )
    for( x=0; x<=Sp.nX; x++ )
     if( y+x==0 )arrow( Sp.out((Vector){x,y,0}),Sp.out((Vector){x,y,Sp.nZ}),0.1,blue );
          else    line( Sp.out((Vector){x,y,0}),Sp.out((Vector){x,y,Sp.nZ}),lightblue );
    for( x=0; x<=Sp.nX; x++ )
    for( z=0; z<=Sp.nZ; z++ )
     if( x+z==0 )arrow( Sp.out((Vector){x,0,z}),Sp.out((Vector){x,Sp.nY,z}),0.1,green );
          else    line( Sp.out((Vector){x,0,z}),Sp.out((Vector){x,Sp.nY,z}),lightgreen );
  }
  if( Draw_flow )
  { const Real Sc=0.1;                            // масштаб отрисовки векторов
          Real x,y,z;
        Vector M=Mxl.x;
    if( MTyp==Sphere || MTyp==Dipole )M*=EqSphere; else M/=2.0;
    for( z=fz?0:Sp.nZ/2; z<=(fz?Sp.nZ:Sp.nZ/2); z+=max(0.1,fz) ) // дипольный
    for( y=fy?0:Sp.nY/2; y<=(fy?Sp.nY:Sp.nY/2); y+=max(0.1,fy) ) // момент по
    for( x=fx?0:Sp.nX/2; x<=(fx?Sp.nX:Sp.nX/2); x+=max(0.1,fx) ) // полуоси от х
    { Point P=Sp.out( (Vector){x,y,z} );       // точка в сеточном узле
      Vector V=dipole_v( M,P-Mxl ),            // вызванные скорости
             W=dipole_flow( M,P-Mxl );         // суммарное поле полного потока
       if( Draw_flow&1 )arrow( P-V*Sc,P+V*Sc,abs(V)*Sc/2,purple );
       if( Draw_flow&2 )arrow( P-W*Sc,P+W*Sc,abs(W)*Sc/2,green );   // контроль
       if( Draw_flow==3 )arrow( P-M*Sc,P+M*Sc,abs(M)*Sc/3,blue );   // фоновый
    }
  } color( navy );
    Win.Print( 2,-2,"Dipole_offset=%.3g \n"
                    "Particle_volume=%.3g << %.3g ",
                 Dipole_offset,(Real)Mxl,Particle_volume ); //.Show().Refresh()
    Win.Save(); //Save().Refresh();
    return true; //false;                          // ничего ещё не прорисовано
}
//!  Главная программа
//
int main( int argc, char** argv )
{   textsize( 80,25 );
    Sp=(Point){ Sp.nX/-2.0,Sp.nY/-2.0,Sp.nZ/-2.0 };
    View_initial();
    Win.Icon( "Math" ).Place::Draw( drawing );
    Win.Draw();
  //glFrontFace( GL_CW );      // CCW видимы грани с обходом по часовой стрелке
    glDisable( GL_LIGHTING );
  int key=0;
    do
    if( key==_F1 )
    { static const char
      *Name[]={ "Fluid  ","Представление о геометрии",
                "поляризованных корпускул    .",0 },
      *Cmds[]={ "ksdtn ","куб/сфера/диполь/тор/ничего",
                "F1    ","краткая справка",
                "F4    ","настройка эксперимента",
                "Tab  ","свободное движение корпускулы",
                "Enter ","перемещение сцены OpenGL",
                "G/g  ","большая пространственная сетка",
                "Del/BkSp ","  возврат и восстановление",
                "Up/Down Right/Left Home/End  "," X,Y,Z",0 },
      *Plus[]={ "Space "," каркас/раскраска поверхности",
                "Space+Shift или Ctrl"," выбор модели",
                "Escape/ctrlC"," стоп ",0 };
      Win.Help( Name,Cmds,Plus );
    } else
    { Real com=0.0;
      switch( key )
      { case _Up  : case _Left : case _Home: com=+0.1; break; // вращение
        case _Down: case _Right: case _End : com=-0.1; break; // по трём осям
        case _PgUp : Particle_volume*=1.1;  break; // PgUp - увеличение
        case _PgDn : Particle_volume/=1.1;  break; // PgDn - уменьшение размера
        case _Tab  : moving_Model ^= true;  break; // вольное движение объекта
        case _Enter: rotate_OpenGL^= true;  break; // и перемещение сцены
        case '+': case '=': Dipole_offset+=0.01; break; // напряженность
        case '-': case '_': Dipole_offset-=0.01; break; // дипольной оболочки
        case 'G': case 'g': Draw_grid^=true; break;      // прорисовка 3D сетки
        case 'N': case 'n': MTyp=None;       break;
        case 'K': case 'k': MTyp=Cube;       break;
        case 'T': case 't': MTyp=Torus;      break;
        case 'S': case 's': MTyp=Sphere;     break;
        case 'D': case 'd': MTyp=Dipole;     break;
        case _Del:                        // Win.eye=(Vector){ -130,-10,0.0 },
              STime=ElapsedTime();        // Win.look=(Vector){ 0.0,0.0,30.0 },
              Particle_volume=1.0;        // Win.Distance=-2*Screen_Size,
        case _BkSp:(Mxl=(Point){0,0,0}).Identity(); break; //cx=cy=cz=0; break;
        case ' ':
        { int Typ=MTyp,Sts=Win.ScanStatus();
          if( Sts&CTRL || Sts&SHIFT ){ if(++Typ>Dipole)Typ=None; MTyp=Model_Type(Typ); }
          else painting ^= true;                      // закраска граней модели
      } }
      switch( key )
      { case _Up:    case _Down: Mxl.rotX( com ); break; // вверх - вниз
        case _Right: case _Left: Mxl.rotY( com ); break; // вправо - влево
        case _Home:  case _End : Mxl.rotZ( com ); break; // home - end
      }
      if( key==_Tab || key==_Enter )
      { if( rotate_OpenGL || moving_Model )
          { Win.SetTimer( 100,drawing ); STime=ElapsedTime(); } else Win.KillTimer();
      }
    drw: Win.Draw(); // drawing();
      if( key==_F4 ){ if( Configuration()!=_Esc )goto drw; }
      //Win.Refresh();   // двойная длинная перерисовка
      if( key>=' ' )textcolor( LIGHTGRAY ),print( 2,2," key=%d='%c' \n",key,key<' '?' ':key );
          else      textcolor( YELLOW    ),print( 2,2," cmd=%d='%c' \n",key,key<' '?' ':key );
    } while( Win.Ready() && (key=Win.WaitKey())!=_Esc ); return 0;
}
static void Model_Draw( Model_Type typ ) // Простые равнообъёмные фигуры
{
//   Первые пробные функции для построения объемных (1м³)
//      Сфера: Ώ = п·D³/6    = 4·п·R³/3    S = 4·π·R²   = 3·Ώ/R
//        Тор: Ώ = п²·d²·D/4 = 2·п²·r²·R   S = 4·π²·R·r = 2·Ώ/r
//             r — радиус малой образующей окружности тора
//             R = r*(1+K) — радиус орбиты вращения малой окружности тора
  switch( typ )
  { case Cube  : Mxl*=pow( Particle_volume/(Real)Mxl,0.333 );
                 Mxl.cubic( painting ); break;
    case Sphere: Mxl*=pow( Particle_volume*0.75/_Pi/(Real)Mxl,0.333 );
                 Mxl.sphere( 15,24,painting ); break;
    case Dipole: Mxl*=pow( Particle_volume*0.75/_Pi/(Real)Mxl,0.333 );
                 Mxl.dipole( Dipole_offset,painting ); break;
    case Torus:{ const Real K=0.01,r=1.0;       // коэффициент к диаметру дырки
                 Mxl*=pow( Particle_volume*_Pi/_Pi/2/(1+K)/(Real)Mxl,0.333 )/2;
                 Mxl.torus( r,r*( 1+K ),15,24,painting ); } break;
    case None:;          // painting - признак закраски граней подвижной модели
  }
}
