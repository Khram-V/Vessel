/**
 **     openFoam - абы что + OpenGL + Window-Place + С++
 **                         (c)2026, В.Храмушин, Могилёв
 **/
#include <StdIO.h>
#include "..\Window\View.h"
#include "..\Window\ConIO.h"
#include "..\Math\Tensor.h"

static Real Screen_Size=12;
static View                        // пространство, отображаемое на экране ЭВМ
       Win( "openFoam, ночная штурмовщина, сильно дурацкая",0,0,1600,768,Screen_Size );

//static Base Tv( Screen_Size );     // пространство, отображаемое на экране ЭВМ

//!  Главная программа
//



int main( int argc, char** argv )
{   textsize( 80,25 );
    for( int i=0; i<argc; i++ )print( argv[i] ),print( i==argc-1?"\n":" " );

    print( "\nздесь будет протокол без премудростей" );
    View_initial();
    Win.Icon( "Math" ); //.Place::Draw( drawing );
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
                "Del/BkSp ","  возврат и восстановление",
                "Up/Down Right/Left Home/End  "," X,Y,Z",0 },
      *Plus[]={ "Space "," каркас/раскраска поверхности",
                "Space+Shift или Ctrl"," выбор модели",
                "Escape/ctrlC"," стоп ",0 };
      Win.Help( Name,Cmds,Plus ).Clear().Refresh(); //
    } else
    if( key==_F4 )
    { Vector Afins={1,1,1},Shift={0,0,0};
      Mlist Menu[]={ { 1,0," Смещение и масштабирование корпуса" }             // 0
      , { 2,8," Сдвиг  x:%6.6lf",&Shift.x},{0,8,", y:%6.6lf",&Shift.y},{0,8,", z:%6.6lf",&Shift.z}
      , { 1,8,"Масштаб x:%6.6lf",&Afins.x},{0,8,", y:%6.6lf",&Afins.y},{0,8,", z:%6.6lf",&Afins.z}
      };
      TextMenu T( Mlist(Menu),&Win,1,1 );
      if( T.Answer()==_Esc && 0 )exit( 11 ); Win.Clear().Show();
//         for( int i=0; i<NoCoPoint; i++ )P[i].V+=Shift,P[i].V&=Afins;
    } else
    { Win.Refresh(); //Win.Draw(); sound( 500 );
    }
    while( Win.Ready() && (key=Win.WaitKey())!=_Esc );

    return 123;
}
