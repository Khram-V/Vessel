/**
 **     Первый пакет математических процедур
 **       с использованием языка программирования OpenGL + Window-Place + С++
 **         простейший вариант отработки консольных команд с мышиной вознёй
 **                      (c)2013, В.Храмушин, Санкт-Петербургский университет
 **/
#include <GL\GLU.h>
#include <stdio.h>
#include "..\Math\Tensor.h"
#include "..\Window\Window.h"
#include "..\Window\ConIO.h"

const GLubyte white[]={ 255,255,255 },   red[]={ 255,  0,  0 },
              green[]={   0,192,  0 },  blue[]={   0,  0,255 },
             purple[]={ 127, 63,255 },  cyan[]={   0,255,255 },
             yellow[]={ 255,255,  0 }, black[]={   0,  0,  0 };

void put( int x,int y, Real* );
void put( int x,int y, COLORS c, const char *Fmt,... );
//
//    Собственно программа, работающая с графическим пакетом OpenGL
//    Определение основных рабочих параметров
//
const Real Screen_Size=2.6;
static Window Win( "Tensor Parameters in Window-Place + ConIO + OpenGL/C++" );
static Base Tv( Screen_Size );     // пространство, отображаемое на экране ЭВМ
//
//    Определение графического окна
//      --  здесь необходимо собрать все графические операции
//
static void line( _Point a,_Point b, const GLubyte *color=NULL )
{ glBegin( GL_LINES ); if( color )glColor3ubv( color );
  glVertex3dv( Tv.in( a ) ),glVertex3dv( Tv.in( b ) ); glEnd();
}
static Vector arrow( _Point a,_Point b,_Real l=0.1, const GLubyte *color=NULL )
{ Vector d=b-a,c=Tv.in( b );
  Vector e={ d.z,d.x,d.y }; e*=l/32;         // это просто трехмерная стрелка
  Vector f={ e.z,e.x,e.y };
   line( a,a-d*(l-1),color );                   glBegin( GL_LINE_LOOP );
   glVertex3dv( Tv.in( a + d*( 1-l/2 ) ) );
   glVertex3dv( Tv.in( a + d*( 1-l ) + d*e ) ); glVertex3dv( c );
   glVertex3dv( Tv.in( a + d*( 1-l ) - d*e ) );
   glVertex3dv( Tv.in( a + d*( 1-l/2 ) ) );
   glVertex3dv( Tv.in( a + d*( 1-l ) + d*f ) ); glVertex3dv( c );
   glVertex3dv( Tv.in( a + d*( 1-l ) - d*f ) );
   glVertex3dv( Tv.in( a + d*( 1-l/2 ) ) );     glEnd(); return c;
}
static void cubic( Base &Model,bool painting_Model )      // QUADS:LINE_STRIP
{ const Real o=-0.4,i=0.6;
//{ const Real o=0,i=1;
  const Vector V[]={ {o,o,o},{o,o,i},{o,i,o},{o,i,i},
                     {i,o,o},{i,o,i},{i,i,o},{i,i,i} };
//const GLubyte *c[]={ white,yellow,purple,red,cyan,green,blue,black };
  const GLubyte *c[]={ black,blue,green,cyan,red,purple,yellow,white };
  glBegin( painting_Model?GL_QUADS:GL_LINE_STRIP );
 #define Vertex( k ) glColor3ubv(c[k]),glVertex3dv( Tv.in( Model.out(V[k]) ) )
         Vertex( 2 ),Vertex( 3 ),Vertex( 1 ),Vertex( 0 );    // левый   - x
         Vertex( 1 ),Vertex( 5 ),Vertex( 4 ),Vertex( 0 );    // нижний  - y
         Vertex( 4 ),Vertex( 6 ),Vertex( 2 ),Vertex( 0 );    // дальний - z
  if( !painting_Model )glEnd(),glBegin( GL_LINE_STRIP );     // + разрыв
         Vertex( 7 ),Vertex( 3 ),Vertex( 2 ),Vertex( 6 );    // верхний
         Vertex( 7 ),Vertex( 5 ),Vertex( 1 ),Vertex( 3 );    // ближний
         Vertex( 7 ),Vertex( 6 ),Vertex( 4 ),Vertex( 5 );    // правый
 #undef Vertex
  glEnd();
 #define Vertex( k,m ) Model.out( (V[k]-V[0])*m+V[0] )
  Win.Text( _East, arrow( Vertex(0,1.2),Vertex(4,1.5),0.2,c[4] ),"x" );
  Win.Text( _North,arrow( Vertex(0,1.2),Vertex(2,1.5),0.2,c[2] ),"y" );;
  Win.Text( _South,arrow( Vertex(0,1.2),Vertex(1,1.5),0.2,c[1] ),"z" );;
 #undef Vertex
}
static void axis( _Real L )
  { arrow((Point){-L,0,0},(Point){L,0,0},.1,red  ),Win.Text(_East, Tv.in((Point){L*1.04,0,0}),"X");
    arrow((Point){0,-L,0},(Point){0,L,0},.1,green),Win.Text(_North,Tv.in((Point){0,L*1.04,0}),"Y");
    arrow((Point){0,0,-L},(Point){0,0,L},.1,blue ),Win.Text(_South,Tv.in((Point){0,0,L*1.04}),"Z");
  }
static Base Model;                 // собственный базис геометрического объекта
static bool rotate_OpenGL=false,   // вращение всего пространства OpenGL
             moving_Model=false,   // собственное перемещение объекта
           painting_Model=false;   // признак закраски граней подвижной модели
static char *Move_mode="";         // режим поворота и деформации видео-куба
//
//   и т.д. всякие прочие прямые и обратные - графические и текстовые операции
//
static bool drawing()
{ Real angle = ElapsedTime()/60.0;  // время от запуска программы в милиминутах
    Win.Activate( true );             // привязка к основному графическому окну
    glClearColor( rotate_OpenGL?0.6:0.9,0.95,0.99,1 );      // цвет фона окна
    glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );    // полная расчистка
    gluPerspective( 12.0,1,-1,1 );                        // украшательство со
    gluLookAt( 1,4,12, 0,0,0, 0,1,0 );                   // взгляда наперекосяк
    glTranslatef( 0.0,0.0,1.0 );                         // отдаление на 1.0
    if( rotate_OpenGL )glRotated( angle,0.5,0.5,0.2 );   // поворот всей сцены
    if( moving_Model )                                   // модель выставляется
    { Real a = M_PI*angle/180.0;                         // в собственном базисе
//    Model.Euler( a/240,a/360,a/480 );
      Model=(Point){ sin( a ),sin( a/1.6 ),sin( a/2 )/2 }; //*0.66;
      Model=(Tensor){ dir( (Vector){ sin(a),cos(a),sin(a/3) } ) };
      glTranslatef( Model.X/-3,Model.Y/-2,-Model.Z );
    }
    put( 2,4,(Real*)(&Model) );
    cubic( Model,painting_Model );
    axis( 7*Screen_Size/8 );
    glColor3ubv( blue ),
    Win.Print( -1,1," %s %s ",Move_mode,DtoA( angle/1000.0/60.0 ) )
          .Save().Refresh();  //Show();
    return false;             // все уже нарисовано
}
//  Перемещение моделей с помощью мышки
//
static int  mx=0,my=0;            // местоположение мышки в локальных смещениях
static bool Motion( int x,int y ){ mx=x,my=y; }    // Мышиная возня по картинке
static bool Mouse( int button, int x, int y )      // Обработка щелчков мыши
{ if( button==_MouseLeft )
  { if( x-mx )Tv.rotY( (mx-x)*0.003 ); mx=x;          // и прямое использование
    if( y-my )Tv.rotX( (my-y)*0.003 ); my=y; drawing();
  }
  if( button==_MouseRight )     // все примочки в статике на единственный вызов
  { static bool tt=false;       // перемещение по оси Z (приближение/удаление)
    if( x==mx && y==my )tt^=true; else
    { Tv-=(Tv.out((Vector){x-mx,tt?0:my-y,!tt?0:y-my})-(Point){0,0,0})*0.008;
      mx=x,my=y; drawing();
} } }
//  Главная программа
//
int main( int argc, char** argv )  // начальная настройка контекста OpenGL
{  textsize( 80,25,25 );           // с текстовым окном и подключением мышки
/* glShadeModel( GL_SMOOTH );      // закраска с использованием полутонов
   glCullFace ( GL_FRONT );        // какие грани будут отбираться
   glFrontFace( GL_CCW );          // видимы грани с обходом по часовой стрелке
   glEnable( GL_CULL_FACE );       // включение режима отбора треугольников
   glEnable( GL_LINE_SMOOTH );     // сглаживание линий
   glHint  ( GL_LINE_SMOOTH_HINT,GL_NICEST );
   glDepthFunc( GL_LESS );
   glDepthMask( GL_TRUE );
   glEnable( GL_DITHER );          // Предопределение графической среды
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glHint( GL_FOG_HINT, GL_NICEST );
*/ glEnable( GL_DEPTH_TEST );      // разборка перекрытий объектов по глубине
   Win.Mouse( Mouse );             // возможно, если особо на скору руку.
   Win.Mouse( Motion );            // но в ограниченном случае
 int key=0,stat=0; Real com=0.0,cx=0,cy=0,cz=0;
  do
  { stat=Win.ScanStatus();
    switch( key )
    { case _Up   : com=+0.1;  break; // вверх
      case _Down : com=-0.1;  break; // вниз
      case _Left : com=+0.1;  break; // влево
      case _Right: com=-0.1;  break; // вправо
      case _Home : com=+0.1;  break; // home
      case _End  : com=-0.1;  break; // end
      case _PgDn : com=+0.9;  break; // PgDn
      case _PgUp : com=1.1111;break; // PgUp
      case _Tab  : moving_Model ^=true; break; // вольное движение
      case _Enter: rotate_OpenGL^=true; break; // для всей сцены в OpenL
      case _Del  : Tv.Identity()*=Screen_Size;
      case _BkSp : Model.Identity(); cx=cy=cz=0; break;
      case ' '   : painting_Model^=true;       // закраска граней куба
    }
    if( stat&SHIFT && stat&CTRL )
    { print( 2,3,Move_mode="Euler <ctrl+shift>" );
      switch( key )
      { case _Up   :case _Down: Model*=Euler( com,0,0 ); break;
        case _Right:case _Left: Model*=Euler( 0,com,0 ); break;
        case _Home :case _End : Model*=Euler( 0,0,com ); break;
    } } else
    if( stat&CTRL )
    { print( 2,3,Move_mode="Krylov <ctrl>     " );
      switch( key )
      { case _Up   :case _Down: Model*=Krylov( com,0,0 ); break;
        case _Right:case _Left: Model*=Krylov( 0,com,0 ); break;
        case _Home :case _End : Model*=Krylov( 0,0,com ); break;
    } } else
    if( stat&SHIFT )
    { print( 2,3,Move_mode="axisXYZ <shift>   " );
      switch( key )
      { case _Up   :case _Down: Model.axiX( cx+=com ); break; // вверх - вниз
        case _Right:case _Left: Model.axiY( cy+=com ); break; // вправо - влево
        case _Home :case _End : Model.axiZ( cz+=com ); break; // home - end
    } } else
    if( stat&ALT )
    { static Vector V={1,0,0};
      static char Msg[120]="";
      switch( key )
      { case _Up   :case _Down: V.y+=com; Model.set(V); break; // вверх - вниз
        case _Right:case _Left: V.x+=com; Model.set(V); break; // вправо -влево
        case _Home :case _End : V.z+=com; Model.set(V); break; // home - end
        case _Del  : V=(Vector){1,0,0}; break;
      }
      sprintf( Msg,"Vector{%.3g,%.3g,%.3g} <alt>",V.x,V.y,V.z );
      print( 2,3,Move_mode=Msg );

    } else
    { print( 2,3,Move_mode=(char*)(key>=_Enter?" <~~~> <~~~>      ":"rotXYZ <~~~>      " ));
      switch( key )
      { case _Up   :case _Down: cx+=com; Model.rotX( com ); break;
        case _Right:case _Left: cy+=com; Model.rotY( com ); break;
        case _Home :case _End : cz+=com; Model.rotZ( com ); break;
        case _PgUp: case _PgDn: Model*=com; break;            // PgDn - PgUp
        case _Tab: case _Enter:
        if( rotate_OpenGL || moving_Model )Win.SetTimer( 100,drawing );
                                     else  Win.KillTimer(); break;
    } }
    if( key>=' ')put(2,2,WHITE,"key=<%X>+%d='%c' ",stat,key,key<' '?' ':key);
        else     put(2,2,YELLOW,"cmd=<%X>+%d='%c' ",stat,key,key<' '?' ':key);
    drawing();
    if( key==_F1 )
    { static const char
      *Id[]={ "Vector&Tensor","  Пространственные","  повороты",0 },
      *Cmds[]={ "<Таб> ","   вольное движение модели",
                "<Ввод> "," —//—всей сцены в OpenGL",
                "<Пробел>","  закраска или рёбра",
                "<BkSP>","исходная геометрия модели",
                "<Del>","    ++ восстановление сцены",
                "  F1           ","краткая справка",0 },
      *Plus[]={ "<Ctrl+Shift> "," углы Эйлера",
                "<Ctrl>       ","    углы Крылова",
                "<Shift> ","     повороты axisXYZ",
                "<стрелки>","        —//— rotXYZ",
                "<вверх•вниз>","   по оси Х",
                "<влево•право>","    —//— Y",
                "<home•end>  ","      —//— Z",
                "<PgUp•PgDn>  ","  размер модели",
                "lMouse•rMouse>"," поворот|смещение",
                "<Esc>/<ctrlC>","    стоп ",0 };
      Win.Help( Id,Cmds,Plus,-2,2 );                    // ...справка, что выше
    }
  } while( Win.Ready() && (key=Win.WaitKey())!=_Esc ); return 0;
}
