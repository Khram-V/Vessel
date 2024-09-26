/*!
       Проверяется настройка шрифтов, базовых и наложенных фрагментов/площадок.
       Также запускается таймер. Вызываются процедуры для наложенных экранных
       подсказок и меню настройки параметров объёмных геометрических фигур
*/
static const char
 *Name[]={ "Window-Place ",
  " Контекстно-зависимая","  объектная библиотека","  интерактивной графики ",0 },
 *Cmds[]={ " i I  "," Show info",
           "=+ -_ ","Increase/Decrease slices",
           ",< .> ","Decrease/Increase stacks",
           "9( 0) ","Decr./Incr. depth (Sierpinski Sponge)",
           "up/down ","Increase/Decrease \"outer radius\"",
           "left/right ","Decrease/Increase \"inner radius\"",0 },
 *Plus[]={ "F1 ","справка",
           "F2 ","меню настроек",
           "F3 ","графический шрифт",
           "PgUp/PgDn ","Next shape-drawing function",
           "Esc/<altX>/<ctrlC>"," стоп ",0
         },
 *_Mnt[]={ "январь","февраль","март","апрель","май","июнь","июль","август","сентябрь","октябрь","ноябрь","декабрь" },
 *_Day[]={ "понедельник","вторник","среда","четверг","пятница","суббота","воскресенье" };

#include "View.h"
#include "ConIO.h"
#include "Geometry.h"
#include <StdIO.h>
#define NUMBEROF( x ) ((sizeof(x))/(sizeof(x[0])))

static int   function_index, slices=16, stacks=16, depth=4;;
static float irad=0.3, orad=0.6, offset[3]={ 0,0,0 };
static bool  show_info=true;

inline void drawSolidTetrahedron()        { glutSolidTetrahedron();                    }
inline void drawWireTetrahedron()         { glutWireTetrahedron();                     }
inline void drawSolidCube()               { glutSolidCube( 1 );                        }
inline void drawWireCube()                { glutWireCube( 1 );                         }
inline void drawSolidOctahedron()         { glutSolidOctahedron();                     }
inline void drawWireOctahedron()          { glutWireOctahedron();                      }
inline void drawSolidDodecahedron()       { glutSolidDodecahedron();                   }
inline void drawWireDodecahedron()        { glutWireDodecahedron();                    }
inline void drawSolidRhombicDodecahedron(){ glutSolidRhombicDodecahedron();            }
inline void drawWireRhombicDodecahedron() { glutWireRhombicDodecahedron();             }
inline void drawSolidIcosahedron()        { glutSolidIcosahedron();                    }
inline void drawWireIcosahedron()         { glutWireIcosahedron();                     }
inline void drawSolidSierpinskiSponge()   { glutSolidSierpinskiSponge(depth,offset,1); }
inline void drawWireSierpinskiSponge()    { glutWireSierpinskiSponge( depth,offset,1); }
inline void drawSolidTeapot()             { glutSolidTeapot( 0.85 );                   }
inline void drawWireTeapot()              { glutWireTeapot( 0.85 );                    }
inline void drawSolidTorus()              { glutSolidTorus( irad,orad,slices,stacks ); }
inline void drawWireTorus()               { glutWireTorus( irad,orad,slices,stacks );  }
inline void drawSolidSphere()             { glutSolidSphere( 0.9,slices,stacks );      }
inline void drawWireSphere()              { glutWireSphere( 0.9,slices,stacks );       }
inline void drawSolidCone()               { glutSolidCone( 1,1,slices,stacks );        }
inline void drawWireCone()                { glutWireCone( 1,1,slices,stacks );         }
inline void drawSolidCylinder()           { glutSolidCylinder( 0.75,1,slices,stacks ); }
inline void drawWireCylinder()            { glutWireCylinder( 0.75,1,slices,stacks );  }
//
// This structure defines an entry in our function-table.
//
typedef struct{ const char *const name; void(*solid)(); void(*wire)(); } entry;

#define ENTRY( e ) { #e, drawSolid##e, drawWire##e }
static const entry table[] =
{ ENTRY( Torus        ), ENTRY( Sphere        ), ENTRY( Cylinder ),
  ENTRY( Cone         ), ENTRY( Icosahedron   ), ENTRY( RhombicDodecahedron ),
  ENTRY( Dodecahedron ), ENTRY( Cuboctahedron ), ENTRY( Tetrahedron ),
  ENTRY( Cube         ), ENTRY( Octahedron    ), ENTRY( SierpinskiSponge ),
  ENTRY( Teapot       ) };
#undef ENTRY
//
//!     Производная структура - большое окно
//
static struct Pro:Window
{ Pro():
  Window( "Window Shapes example / типа того в UTF-8 ‏יְרוּשָׁלַיִם",0,0,1200,600 ){}
   virtual bool KeyBoard( byte );
} Tvm;
static Real Amax=123.5;
static Mlist Menu_a[]=
{ { 0,0,"   <<<-  меню конфигурации  ->>>" }
, { 2,2,"Номер графического объекта [0 < %d",&function_index },{ 0,0,"> 12" }
, { 1,4,"внутренний:%4.1f",&irad},{0,4," и внешний:%4.1f",&orad},{0,0," радиусы"}
, { 1,14,"Максимальный угол крена %-3°",&Amax }
, { 1,3,"depth (Sierpinski Sponge): %d",&depth }
, { 1,3,"     slices: %d",&slices },{ 0,3,",  stacks: %d",&stacks }
};
bool Pro::KeyBoard( byte key )
{ textcolor( LIGHTGREEN,BLACK ),
  print(2,7,"Key = %d\\x%X = '%s' + <%X>",key,key,WtU(key),ScanStatus() );
  switch( key )
  { case 'I': case 'i': show_info^=true;         break;
    case '=': case '+': slices++;                break;
    case '-': case '_': if( slices>-1 )slices--; break;
    case ',': case '<': if( stacks>-1 )stacks--; break;
    case '.': case '>': stacks++;                break;
    case '9': case '(': if( depth>-1 )depth--;   break;
    case '0': case ')': ++depth;                 break;
    case ' ': clrscr(); Clear();                 break;
    case _F1: Help( Name,Cmds,Plus,-1,2 );       break;
    case _F2: while( TextMenu( Mlist( Menu_a ),this ).Answer()!=_Esc ); break;
    case _F3: // GetKey(); -- в прерывания символ из буфера не выбира(ется)лись
    { Place R( this,PlaceAbove ); glClearColor( 1,1,0.8,1 ),glColor3f( 0,0,1 );
      const char
      *s="│≡Первая строчка ˉ○◙♪"
         "  \n"
         "│ !\"#$%&'()*+,-./0123456789:;<=≠>? \n"
         "│ ©…@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ \n"
         "│ `abcdefghijklmnopqrstuvwxyz{|}~ \n"
         "│ ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜø£Ø₧ƒ   \n"
         "│ áíóúñÑªº¿⌐¬½¼¡«»│─║═   \n"
         "│ ░▒▓┤╡╢╖╕╣║╗╝╜╛┐└┴┬├┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀ \n"
         "│ ßµδ∞∩∝≡±≥≤⌠⌡∂∫∮†‡∑§÷≈°∙·∇√ⁿ²■¦↑→←↓ \n"
         "│ АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ  \n"
         "│ абвгдеёжзийклмнопрстуфхцчшщъыьэюя  \n"
         "│ ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ \n"
         "│ αβψδεϕφγηίξκλμνοπρστϑωχυζ🌀 │ окончание блока W"
/*
         "│ !\"#$%%&'()*+,-./ 0123456789:;<=>? │.\n"
         "│ @ABCDEFGHIJKLMNO PQRSTUVWXYZ[\\]^_ │.\n"
         "│ `abcdefghijklmno pqrstuvwxyz{|}~  │.\n"
         "│ АБВГДЕЁЖЗИЙКЛМНОП РСТУФХЦЧШЩЪЫЬЭЮЯ │.\n"
         "│ абвгдеёжзийклмноп ░▒▓│┤╡╢╖╕╣║╗╝╜╛┐ │.\n"
         "│ └┴┬├─┼╞╟╚╔╩╦╠═╬╧ ╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀ │.\n"
         "│ рстуфхцчшщъыьэюя ≡±≥≤⌠⌡÷≈°∙·√ⁿ²■ │.\n"
         "│ ΐΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩΪΫάέήίΰ │.\n"
         "│ αβγδεζηκλμνξοπρςστυφχψωϊϋόύώ θι  │.\n"
         "│ ©…∞ₒₐₓₑₔ‘’„“”«»‹›•∙×±≥≤÷≈≡≠∂δ∆∇∧∀∨⇒⇔↦←↑→↓↔↕↨∏∑∩∫ƒєæΘ │ окончание блока W"
*/        ;
//          R.AlfaBit( _8x16 )
            R.AlfaVector( 0 )
             .Area( 3,2,68,15 ).Clear().Print( 1,1,s ).Show();
             glClearColor( 0.9,1,0.95,1 );     // восстановление фонового цвета
            WaitKey();
    } break;
    default:
    { switch( key )
      { case _PgUp : --function_index; break;
        case _PgDn : ++function_index; break;
        case _Up   : orad *= 2;        break;
        case _Down : orad /= 2;        break;
        case _Right: irad *= 2;        break;
        case _Left : irad /= 2;        break;
       default: key=0;
  } } }
  if( function_index<0 )function_index = NUMBEROF( table )-1;
  if( NUMBEROF( table )<=(unsigned)function_index)function_index=0;
  return key!=0;
}
/// ===========================
static void display( Place &Tv )
{ double t=ElapsedTime()/1000.0; static int cTv=0;
    Tv.Activate( true ).Clear();
  Real rat = Real( Tvm.Width )/Tvm.Height;
    if( Tv.Width!=Tvm.Width )
    { //TextContext B(false);
      color( white ),rectangle( (Point){-1,-1,-1},(Point){1,-0.6,-1} );
      for( int i=0; i<256; i++ )
      { color( (colors)(i+black+1) ); rectangle( (Point){-1+2.0*i/256.0,-0.95 },
                                        (Point){ -1+2.0*(i+1)/256.0,-0.8} ); }
      for( int i=0; i<=black; i++ )
      { color( (colors)i ); rectangle( (Point){ -1+2*i/Real(black+1),-0.8 },
                               (Point){ -1+2*(i+1)/Real(black+1),-0.7} ); }
    }
    //Tvm.WaitKey(); Tvm.Show(); return;
    Tv.Activate( false );
    gluPerspective( 32,rat, -1,1 );                    // + украшательство со
    gluLookAt( 1,2,3, 0,0,-0.5, 0,0.5,0 );             // взгляда наперекосяк
    glColor3f( 0,0,1 );
    Tv.Print( -1, 0,"Счетчик кадров: № %d = %3.2g fps, %0.3g сек ||",++cTv,cTv/t,t );
    Tv.Print( -1,-1,"~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=||" );
    glColor3f( 0,1,0 ); Tv.Print( 0,1,"^^WWWWWWWWWWW||" ); Tv.Print( 0,0,"W" );
    glTranslated( 0,0,-0.12 ); glColor4f( 0.5,0.8,1,0.2 ); glRectf( -1,-0.5,1,1 );
    glTranslated( 0,0, 0.04 ); glColor4f( 1,1,1,0.8 ); glRectf( -.96,-.48,.96,.96 );
    glTranslated( 0,0, 0.08 ); glFinish();
    rat=1.0+(sin(t/3)+1)*M_1_PI/3.0;
    glScaled( rat,rat,rat );
    glEnable( GL_LIGHTING );
    rat=fmod( cos( t/2 )*20.0,20.0 )/100.0;
    glColor4f( 0.2-rat,0.3+rat,0.4,0.5 );
    glPushMatrix(),glScalef( 0.4,0.4,0.4 ),glTranslated( 2.5,1.25,0 );
    rat*=200.0*M_PI;
    glRotated( rat,sin(t),cos(t/1.5),cos(t/2) ); table[function_index].solid();
    glPopMatrix();
    glPushMatrix(),glScalef( 0.4,0.4,0.4 ),glTranslated( 2.5,-0.75,0 );
    glRotated( rat,sin(t),cos(t/1.5),cos(t/2) ); table[function_index].wire();
    glPopMatrix();
    Tv.Activate( false );
    glFlush();
    glPushMatrix(),glTranslated( 0,1,1 ),glRotated( t*10,0,1,0 );
    glColor3f( 0,0,1 ),glLineWidth( 0 ),glDisable( GL_LIGHTING );
      Tv.Text( _Center,    -0.6,-0.4,0, "Центр" );
      Tv.Text( _East,      -0.2,-0.4,0, "East"  );
      Tv.Text( _West,      -1,  -0.4,0, "West"  );
      Tv.Text( _North,     -0.6,-0.2,0, "North" );
      Tv.Text( _South,     -0.6,-0.6,0, "South" );
      Tv.Text( _North_East,-0.2,-0.2,0, "СВ" );
      Tv.Text( _North_West,-1,  -0.2,0, "СЗ" );
      Tv.Text( _South_East,-0.2,-0.6,0, "ЮВ" );
      Tv.Text( _South_West,-1,  -0.6,0, "ЮЗ" );
    glColor3f( 0,1,0 ),glLineWidth( 1 );
    glBegin( GL_LINE_LOOP );  // GL_TRIANGLES ); //GL_QUADS );
//    glNormal3f( 0,0,-1 );
      glVertex3f( -1,-.2, 0 );
      glVertex3f(-.2,-.2, 0 ); glVertex3f(-.2,-.6, 0 );
      glVertex3f( -1,-.6, 0 ); glVertex3f(-.2,-.2, 0 );
      glVertex3f(-.2,-.59,0 ); glVertex3f( -1,-.59,0 );
      glVertex3f( -1,-.2, 0 ); glVertex3f(-.2,-.6, 0 );
    glEnd();
    glFlush();
    glPopMatrix();
    if( show_info )
    { glColor3f( 1,0,1 ); Tv.Print( 1,1,"Shape PgUp PgDn: [°·] " );
      glColor3f( 0,1,0 ); Tv.Print( "(как же то по русски?) " );
      glColor3f( 0,0,0 ); Tv.Print( "%s",table[function_index].name );
      Tv.Print( 3,3,"Slices + -: %d   Stacks < >: %d", slices, stacks );
      Tv.Print( 3,4,"nSides + -: %d   nRings < >: %d", slices, stacks );
      Tv.Print( 3,5,"Depth  ( ): %d", depth );
      Tv.Print( 3,6,"Outer radius  Up  Down : %g", orad );
      Tv.Print( 3,7,"Inner radius Left Right: %g", irad );
      glColor3f( 1,0,0 ); Tv.Print( "\n - And after\n\nthis questions? \n" );
      glColor3f( 0,1,0 ); Tv.Print( "and oiiver ЯяЯя --" );
      glColor3f( 0,0,1 ); Tv.Print( " что получится " );
    } else printf( "Shape %d, %s', slides %d stacks %d\n",
           function_index,table[function_index].name,slices,stacks );
    Tv.Save();                  // так будет немного быстрее
//  Tv.Show();                  // -- здесь получится двойная прорисовка
    Tv.Refresh();               // и собственно так, если много площадок
}
static bool MouseMove( int X,int Y )
{ textbackground( BLUE ); textcolor( YELLOW );
  print( 3,12,"Tv.Move :   { %d,%d } \n",X,Y ); return false; }
static bool MousePress( int State, int X,int Y )
{ textbackground( LIGHTBLUE ); textcolor( WHITE );
  print( 3,13,"Tv.Press: %X { %d,%d } \n",State,X,Y ); return false; }
static bool OneMove( int X,int Y )
{ textbackground( BLUE ); textcolor( COLORS( YELLOW|32 ) );
  print( 3,15,"One.Move : { %d,%d } \n",X,Y ); return false; }
static bool OnePress( int State, int X,int Y )
{ textbackground( LIGHTBLUE ); textcolor( WHITE );
  print( 3,16,"One.Press: %X { %d,%d } \n",State,X,Y ); return false;
}
static bool Init=false;
Place *aOne=NULL;
static bool Clock()    // - регулярное обновление картинки делается в процедуре
{ Event T; int M,D,Y; T.UnPack( M,D,Y );     // обработки прерываний от таймера
  glColor3f( 0,0.5,1 );             // наложение маленькой площадки со временем
 static Place *W=NULL; // порождается окно в пиксельных размерах и без подложки
  if( !W ){ W=new Place( &Tvm,PlaceAbove );
//  W->AlfaBit( _8x16 );            //- это поправит размер по-умолчанию [9:18]
    W->AlfaVector( 16 );
  } W->Area( 0,1,44,1 ).Activate().Clear().Print( 0,0," %04d %s %02d, %s%s ",
                       Y,_Mnt[M-1],D,_Day[int(T.D%7)],DtoA( T.T,0 ) ).Show();
  if( Init )display( *aOne );
  return false;                             // все картинки уже выведены в окно
}
#if 0
static void Lighting()      // простая настройка OpenGL - может быть вовне
{ glClearColor( 0.9,1,0.95,1 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//glCullFace( GL_FRONT_AND_BACK );
  glCullFace( GL_BACK );
  glEnable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST ); //glDepthFunc( GL_LESS ); //GREATER ); //ALWAYS );
  glShadeModel( GL_SMOOTH ); // GL_FLAT );
  glEnable( GL_LINE_SMOOTH );  glEnable( GL_DITHER );
  glEnable( GL_POINT_SMOOTH ); glEnable( GL_BLEND );
  glEnable( GL_LIGHT0 );       glEnable( GL_LIGHTING );
  glEnable( GL_NORMALIZE );    glEnable( GL_COLOR_MATERIAL );
  glHint( GL_LINE_SMOOTH_HINT,GL_NICEST );
  glHint( GL_POINT_SMOOTH_HINT,GL_NICEST );
  glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA );
  glLightfv(GL_LIGHT0,GL_AMBIENT, (const float[]){.6,.7,.8,.5});//light_ambient
  glLightfv(GL_LIGHT0,GL_DIFFUSE, (const float[]){.8,.7,.6,.5});//light_diffuse
  glLightfv(GL_LIGHT0,GL_SPECULAR,(const float[]){.9,.9,.9,.5});//light_specular
  glLightfv(GL_LIGHT0,GL_POSITION,(const float[]){-1,2,3,.9} ); //light_position
  glMaterialfv(GL_FRONT,GL_AMBIENT,(const float[]){.6,.6,.6,.5});//mat_ambient
  glMaterialfv(GL_FRONT,GL_DIFFUSE,(const float[]){.8,.8,.8,.5});//mat_diffuse
  glMaterialfv(GL_FRONT,GL_SPECULAR,(const float[]){1,1,1,.5});// mat_specular
  glMaterialfv(GL_FRONT,GL_SHININESS,(const float[]){ 64 } ); // high_shininess
  glLineWidth( 1 );
}
#endif
int main( int argc,char *argv[] )
{   texttitle( "Internal ConIO procedures: образец текста по русски שָׁלוֹם" );
    textsize( 80,25 );                              // 32 - минимум - типа нуля
    printf( "|\n|\nFirst программа - кодировка UTF-8 (с OEM-866/alt) שָׁלוֹם\n" );
    //
    //  Этот текст должен появляться на экране без дополнительных Show-указаний
    //
//  Tvm.AlfaBit( _8x16 )                          // в GLFW пока происходит
//  Tvm.Alfabet( 24,"Times New Roman" )           // подмена на растр DispCCCP
    Tvm.AlfaVector( 0 ) //.Area( 1,1,200,100 ).Activate()
    .Print
       ( 6,1,"Текст в окошке со сдвигом\n▓ без доп.настроек в исходном цвете\n"
             "█ - третья строка ЩЩ");
    glColor3f(1,1,0); Tvm.Print("ЩЩ,\n запятая и продолжение\n с новой записи ЩЩ");
    glColor3f(0,1,1); Tvm.Print("ЩЩ, \n запятая, и снова продолжение");
    glColor3f(1,1,0); Tvm.Print(-6,7,"Строки справа-налево\nс общим отступом = -6");
    glColor3f(1,1,1); Tvm.Print
//       ( -12,-8,"Это -3 строки снизу и справа\n"
//                "с отличным отступом = -12 и\n"
//                "!важно! построчно сверху-вниз\n = ++ " );
       ( -12,-8,"Это -3 строки в отсчётах снизу и справа-налево\n"
                "с отличным отступом справа = -12 и\n"
                "!важно! печать восстановлена построчно сверху-вниз\n = ++ " );
    glColor3f(1,0,0); Tvm.Print( "...и продолжение после перевода строки " );
    glColor3f(0,1,0);
    for( int i=0; i<=black; i++ )
    { color( (colors)i );
      if( !i )rectangle( (Point){-1,-1},(Point){1,-0.7} );
      rectangle( (Point){ -1+2.0*i/Real(black+1),-0.9 },
                 (Point){ -1+2.0*(i+1)/Real(black+1),-0.8 } );
    }
    Tvm.Save();   //Refresh(); //Show();
    Tvm.Above();
    //
    //  Теперь устраивается перехват мышки и таймера независимыми транзакциями
    //
    Tvm.Mouse( MouseMove );
    Tvm.Mouse( MousePress );
    Tvm.SetTimer( 500,Clock );                    // часы в правом верхнем углу
    Tvm.WaitKey();      //! простая приостановка обязательно должна срабатывать
    { Place Zero( &Tvm,PlaceAbove ); glClearColor( 1,1,1,0.5 );
      glColor3f(0,0,1); Zero.Area(12,-14,40,1).Clear().Print(1,1,"Строчка в окошке");
      glColor3f(0,.5,0); Zero.AlfaVector( 12 ).Print( " со сменой шрифта" );
      Zero.Show(); Tvm.WaitKey();
    }
    View_initial(); glDepthRange( -1,1 ); Init=true;
    display( *(aOne=(Place*)&Tvm) );  //  картинка по основному фоновому окну
    Tvm.WaitKey();                    //  приостановка с работающим таймером
    Tvm.Locate( 100,20,1280,980 );    //! Locate(1,1,0,0) - во весь экран
    Tvm.Save();
   Place One( &Tvm,PlaceAbove|PlaceOrtho ); // вторая - наложенная страница
   aOne=&One;
    One.Mouse( OneMove )
       .Mouse( OnePress )
//     .AlfaBit( _8x08 )               //  маленькие буквочки на большом окне
//     .Alfabet( 12,"Arial" )
       .AlfaVector( 16 )
       .Area( 20,2,112,36 )
//     .Area( 0,0,0,0 )
       .Activate( true )
       ;
    Tvm.AlfaVector( 12 );
    while( Tvm.Ready() )
    { byte ans=Tvm.GetKey();
      if( ans==_Esc )break;
      if( ans ){ textcolor( YELLOW );
                 print( 2,8,"Key = %d\\x%X = '%s' + <%X> ",
                         ans,ans,WtU(ans),Tvm.ScanStatus() ); }
      display( Tvm ); WaitTime( 1000 );
    }
  //Tvm.KillTimer();
  Break( "~Как бы всё благополучно" );
  return EXIT_SUCCESS;
}
