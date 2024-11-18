/**
 *      Вторая попытка формирования нового математического пакета трехмерных
 *  векторно-тензорных преобразований с использованием контекстно-зависимого
 *  языка графических построений  Window-Place + OpenGL + С++
 *                                                         ©2010-15, В.Храмушин
 */
#include "..\..\Math\Function.h"
#include "..\..\Math\Hermite.h"
#include "..\View.h"
//
//      Собственно программа, работающая с графическим пакетом OpenGL
//
const int Npt=12;                         // =10 длина интерполируемого массива
static int index=Npt/2;                            // номер подвижного узла
static Real X[]={ 0, 1,2,3,  4,5,6,7,8,9,10,11 }, // аргумент и
            Y[]={ -0.2,1.45,2,1.9,3,4,3,1.45,1.5,1.35,-1,.5 }; // значения в узлах
static Function F1( Npt ),F2( Npt ),            // функция без Х и c аргументом
                FX( Npt ),FY( Npt );           // и их раздельное представление
static Real xMin,xMax,yMin,yMax;
//
//  подготовка и инициализация графического экрана
//
Window Win( "Interpolation & Extrapolation in Window-Place + OpenGL/C++",0,0,1600,768 );

void grid()
{ Win.Activate();
  glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
  glLineWidth( 1 );
    line( (Point){-1, 3,-0.1},(Point){Npt, 3,-0.1},lightgray );
    line( (Point){-1, 0,-0.1},(Point){Npt, 0,-0.1} );
    line( (Point){-1,-3,-0.1},(Point){Npt,-3,-0.1} );
  for( int k=0; k<Npt; k++ )
  { Point Down={ k,-5,-0.1 }; line( Down,(Point){ k,5,-0.1 } );
    Win.Text( _South, Down.X,Down.Y,Down.Z, "%d",k+1 );
  }
  color( magenta ); Win.Print( 2,-5," Sleek - второй степени +1 \n" );
  color( green   ); Win.Print( " Чисто линейная интерполяция \n" );
  color( yellow  ); Win.Print( " Полный полином степени %d \n",Npt-1 );
  color( blue    ); Win.Print( " Стандартный сплайн \n" );
  color( red     ); Win.Print( " Полином Эрмита по узловым градиентам" );
}
static bool drawing()
{ const Real st=0.01; Real w,d0,d1,f0,f1; grid(); // glPointSize( 16 );
    spot( (Point){ F2(index),F2[index],0 },16,white );
    spot( (Point){ index,F1[index],0 },    16,white );
    spot( (Point){ FX[index],FY[index],0 },16,white );
    //
    //! вариант непосредственной интерполяции по Эрмиту второй степени
    //
    glLineWidth( 4 );
    for( w=-1; w<Npt; w+=st )
      line( (Point){w,Hermite(Y,X,w,Npt,&d0),0},
            (Point){w+st,Hermite(Y,X,w+st,Npt,&d1),0},magenta ),
      line( (Point){w,d0,0},(Point){w+st,d1,0},lightmagenta );
    //
    //!  и сама функция без чрезмерных приведений и пустых приукрашений
    //
    f0=Hermite( Y,w=-1,Npt,&d0 ); glLineWidth( 0.05 );
    for( w=st-1; w<Npt; w+=st )
    { f1=Hermite( Y,w,Npt,&d1 );
      line( (Point){ w,f0},(Point){ w+st,f1},magenta ); f0=f1;
      line( (Point){ w,d0},(Point){ w+st,d1},lightmagenta ); d0=d1;
    }
    //
    //! сначала раскладываются по полкам все три желтых полинома с производной
    //
    color( yellow ); glLineWidth( 2.0 );
    for( w=-1; w<Npt; w+=st )line( (Point){ w,F2.value(w,Npt)},(Point){ w+st,F2.value(w+st,Npt)} );
    for( w=-1; w<Npt; w+=st )line( (Point){ w,F1.value(w,Npt)},(Point){ w+st,F1.value(w+st,Npt)} );
    for( w=-1; w<Npt; w+=st )line( (Point){ FX.value(w,Npt),FY.value(w,Npt)},(Point){ FX.value(w+st,Npt),FY.value(w+st,Npt)} );
    glLineWidth( 0.5 );
    for( w=-1; w<Npt; w+=st )line( (Point){ w,F1.derivative( w,Npt )},(Point){w+st,F1.derivative( w+st,Npt )} );
    //
    //! функции по аргументу с переменным шагом
    //
    glLineWidth( 2.5 );
    F2.Easy();   for( w=-1; w<Npt; w+=st )line( (Point){ w,F2(w)},(Point){ w+st,F2(w+st)},green );
    F2.SpLine(); for( w=-1; w<Npt; w+=st )line( (Point){ w,F2(w)},(Point){ w+st,F2(w+st)},blue );
    F2.First();  for( w=-1; w<Npt; w+=st )line( (Point){ w,F2(w)},(Point){ w+st,F2(w+st)},red );
    //
    //! функции без определенного аргумента
    //
    F1.Easy();   for( w=-1; w<Npt; w+=st )line( (Point){ w,F1(w)},(Point){ w+st,F1(w+st)},green );
    F1.SpLine(); for( w=-1; w<Npt; w+=st )line( (Point){ w,F1(w)},(Point){ w+st,F1(w+st)},blue );
    F1.First();  for( w=-1; w<Npt; w+=st )line( (Point){ w,F1(w)},(Point){ w+st,F1(w+st)},red );
    //
    //! то же самое для производных
    //
#define s -1
    for( int i=4; i>=0; i-=4 )
    { float f=i+1; glLineWidth( f ); f=-1/f;
      F1.Easy();   for( w=-1; w<Npt; w+=st )line((Point){w,F1.derivative(w,s),f},(Point){w+st,F1.derivative(w+st,s),f},i?lightgray:green );
      F1.SpLine(); for( w=-1; w<Npt; w+=st )line((Point){w,F1.derivative(w,s),f},(Point){w+st,F1.derivative(w+st,s),f},i?lightgray:blue );
      F1.First();  for( w=-1; w<Npt; w+=st )line((Point){w,F1.derivative(w,s),f},(Point){w+st,F1.derivative(w+st,s),f},i?lightgray:red );
    }
#undef s
    //! раздельное представление тех же функций
    //
    glLineWidth( 2.5 );
    FX.Easy(); FY.Easy();
    for( w=-1; w<Npt; w+=st )line( (Point){ FX(w),FY(w)},(Point){ FX(w+st),FY(w+st)},green );
    FX.SpLine(); FY.SpLine();
    for( w=-1; w<Npt; w+=st )line( (Point){ FX(w),FY(w)},(Point){ FX(w+st),FY(w+st)},blue );
    FX.First(); FY.First();
    for( w=-1; w<Npt; w+=st )line( (Point){ FX(w),FY(w)},(Point){ FX(w+st),FY(w+st)},red );
    color( white );
    F2.extrem( xMin,yMin,xMax,yMax,true );
    Win.Print( 0,0,"xMin=%g, yMin=%g,  xMax=%g, yMax=%g ",xMin,yMin,xMax,yMax )
       .Save()
       .Show(); return true;
}
//!  Главная программа
//!
int main( int argc,char** argv )
{   //
    //  подготовка функции (разнесение графиков на экране по вертикали)
    //
    for( int k=0; k<Npt; k++ )F1[k]=(Y[k]/=3),
                              F2[k]=Y[k]+3,  F2(k)=X[k],
                              FY[k]=Y[k]-3,  FX[k]=X[k], Y[k]+=1.0;
    F1.extrem( xMin,yMin,xMax,yMax );
    //
    //   Утилиты Window обеспечивают взаимодействие с операционной системой
    //
    View_initial();
    glDisable( GL_BLEND );
    Win.Icon( "Flag" ).AlfaVector( 22,1 );
    Win.Draw( drawing );
    glClearColor( 0.5,0.8,1,1 );  // glMatrixMode(GL_PROJECTION);
    glOrtho(-2,Npt+1,-7,6,-1,1 ); // glMatrixMode(GL_MODELVIEW),glLoadIdentity()
  int key=0;
    do
    { Real com=0;
      switch( key )
      { case _North:      com=+0.1; break; // вверх
        case _South:      com=-0.1; break; // вниз
        case _East:       com=+0.1; break; // вправо
        case _West:       com=-0.1; break; // влево
//      case _North_West: com=+0.1; break; // home
//      case _South_West: com=-0.1; break; // end
//      case _North_East: com=1.1;  break; // PgUp
//      case _South_East: com=0.9;  break; // PgDn
        case _Esc: case 'q': exit( 2 ); break;    // жесткий выход из программы
       default: if( key=='0' )index=9; else
                if( key=='-' || key=='_' )index=10; else
                if( key=='=' || key=='+' )index=11; else // выбор номера точки
                if( key<='9' && key>='1' )index=key-'1'; // на графиках функции
                 // index=minmax(1,key=='0'?10:key-'0',Npt)-1;
      }
      switch( key )
      { case _North:case _South: F1[index]+=com; Y[index]+=com; // вверх-вниз
                                 F2[index]+=com; FY[index]+=com; break;
        case _West: case _East:  F2(index)+=com; FX[index]+=com;// вправо-влево
                                                 X[index]+=com; break;
      } drawing();
    } while( Win.Ready() && (key=Win.WaitKey())!=_Esc ); return 0;
}
