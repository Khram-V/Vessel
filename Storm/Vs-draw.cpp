//
//      Несколько независимых информационных процедур без предустановок
//      отрабатываются в текущем/произвольном графическом окне OpenGL
//
//                                                           2021-03-23 יְרוּשָׁלַיִם
//
#include "Vessel.h"       // объекты и производные операции с корпусом на волне

Hull& Hull::Contour_Lines()      // рисуем контуры габаритного прямоугольного
{ glLineWidth( 0.05 );           // параллелепипеда по заданным размерениям
  glEnable( GL_LINE_SMOOTH );    // сглаживание линий
#define V(a,b,c)glVertex3dv(out((Vector){a Length/2,b Breadth/2,c Draught/2-Draught/2})),
  color(cyan),glBegin(GL_LINE_LOOP),V(+,+,+)V(+,-,+)V(-,-,+)V(-,+,+)glEnd();
  color(gray),glBegin(GL_LINE_LOOP),V(+,+,-)V(+,-,-)V(-,-,-)V(-,+,-)glEnd();
#undef V
///
//!   прорисовка линий тока и профилей волн по курсу и траверзу корабля
//
 Field &F=*Storm;
 const Real &Trun=F.Trun,
      D=-1.5*Draught,dP=D/9,dQ=dP/6;    // профильное заглубление и детальность
 Real S=hypot( F.Long,F.Wide ),dS=F.dS; // дистанция и шаг разметки профиля
 Vector Ahead=x;    Ahead.z=0;    Ahead=dir( Ahead )*S;
 Vector Traverse=y; Traverse.z=0; Traverse=dir( Traverse )*S;
 Vector R,V,P; R.x=fabs( F.Long/Ahead.x )/2;
               R.y=fabs( F.Long/Traverse.x )/2;
  Ahead *= R.x<0.5 ? R.x : F.Wide/fabs( Ahead.y )/2;
  Traverse*=R.y<0.5 ? R.y : F.Wide/fabs( Traverse.y )/2;
  glLineWidth( 0.01 ); arrow( -Traverse,Traverse,Length/240,lightcyan );
                       arrow( -Ahead,Ahead,Length/240,yellow );
  glLineWidth( 1 );                           // ускоренная прорисовка без волн
  if( !F.Exp.wave )return *this; color( lightblue );
  for( Real w=0; w>=D; w+=dP )
  { S=abs( Ahead ); glBegin( GL_LINE_STRIP );
    for( Real s=0; s<S*2; s+=dS )              // профили волн по курсу корабля
    { R = -Ahead + Ahead*( s/S ); R.z=w; dot( F.Wave( Trun,R ) ); } glEnd();
    S=abs( Traverse ); glBegin( GL_LINE_STRIP );
    for( Real s=0; s<S*2; s+=dS )                   // профили волн по траверзу
    { R = -Traverse + Traverse*( s/S ); R.z=w; dot( F.Wave(Trun,R)); } glEnd();
  }
  // пусть будет еще лишняя сотня кривых линий вертикальных граней жидких ячеек
  //
  dS=( S=abs( Traverse ) )/120.0; glLineWidth( 0.01 );   // длина и шаг профиля
  for( Real s=0; s<S*2; s+=dS )
  { R=Traverse*( s/S )-Traverse; color( lightgreen ); glBegin( GL_LINE_STRIP );
    for( R.z=0; R.z>=D; R.z+=dQ )dot( F.Wave(Trun,R) ); glEnd(); color( cyan );
    for( R.z=0; R.z>=D; R.z+=dP ){ F.Wave(Trun,P=R,V); arrow(P,P+V,abs(V)/6); }
  }
  dS=( S=abs( Ahead ) )/120.0;
  for( Real s=0; s<S*2; s+=dS )
  { R=Ahead*( s/S )-Ahead; color( lightgreen ); glBegin( GL_LINE_STRIP );
    for( R.z=0; R.z>=D; R.z+=dQ )dot( F.Wave(Trun,R) ); glEnd(); color( cyan );
    for( R.z=0; R.z>=D; R.z+=dP ){ F.Wave(Trun,P=R,V); arrow(P,P+V,abs(V)/6); }
  } glLineWidth( 1 ); return *this;
}
//! общая навигационная информация о состоянии корабля
//
const static byte Rumb[]
 ={_North,_North_East,_East,_South_East,_South,_South_West,_West,_North_West };

#define P( x,y ) glVertex3dv( B*(Vector){ x,y } )

static void DirWave( const Waves &W, colors C, Place &D )
{ Real H=W.Height/Vessel->Length*4,L=W.Length/Vessel->Length;
 _Tensor B=W; color( C,0,0.6 ); glBegin( GL_POLYGON );
  P(0,0),P(-H,-L+H),P(0,-L),P(H,-L+H),P(-H,L),P(0,L-H),P(H,L); glEnd();
  color( C ); if( (H=angle( H=atan2( W.x.y,-W.x.x )))<0.0 )H+=_Pd;
 char S[24],*s=S;
  snprintf( S,23,"%3s:%3.1f/%3.1f\n",
            Rmbs[int(H*32/_Pd+0.49)%32],W.Height,W.Length/W.Cw );
  if( s=strchr( S,'.' ) ){ *s=',';
    if( s=strchr( s,'.' ) )*s='\"'; } D.Print( S );
}
Hull& Hull::Naviga_Inform( Window *Win )
{
 Tensor B;                       // единичный масштаб пространственного тензора
 Vector C=Swing[-1]; C.z+=_Ph;  // крен, дифферент и курс корабля (в геобазисе)
 TextContext TS( true );
 int i,l=-0.2*hypot( Width,Height );                  // размер из аксонометрии
 Place Compass( Win,PlaceOrtho );                     // | PlaceAbove
  Compass.Area( 0,0,1.25*l,l ).Activate( true );      // c единичной разметкой
  //
  //! картушка морского волнения, курса, скорости корабля
  //+ отметки трёх волновых полей Wind+Swell+Surge = Length+Height+=Tensor::x
  //                                                        амплитуда=>высота/2
 Real L=min( 1.0,Length // условная высота и длина стрелки волны в картушке
  / max( max( Storm->Wind.Length,Storm->Swell.Length ),Storm->Surge.Length ) );
  glTranslated( 0,-0.1,0 );
  glScaled( .9*L,.9*L,1 );
  color( white,0,.33 ),circle( (Point){0},1/L );  // круг картушки 110% корпуса
 bool left=int( 2+Course/_Ph )&1; Compass.Print( left?-1:1,1,"" ); color(navy);
  DirWave( Storm->Wind,green,Compass );           // ветровая волна
  DirWave( Storm->Swell,blue,Compass );           // свежая зыбь
  DirWave( Storm->Surge,cyan,Compass );           // реликтовый накат
  //
  //  стрелка-указатель заданного курса корабля
  //
  B.axiZ( _Ph-Course );
  for( i=0; i<2; i++ )      // стрелка выбранного курса
  { if( !i ){ glLineWidth( 2 ); color( blue,0,.75 ); glBegin( GL_LINE_LOOP ); }
       else { glLineWidth( 1 ); color( yellow,0,.3 ); glBegin( GL_POLYGON ); }
    P(.75,0),P(.4,-.05),P(.5,-.01),P(-.75,-.05),
    P(-.6,0),P(-.75,.05),P(.5,.01),P(.4,.05); glEnd();
  }
  //  подпись - текущий курс в градусах и скорость в узлах
  //
  for( i=0; i<32; i++ )
  { Real D=Real( i )*_Ph/8.0;                    //! картушка компаса
    Vector V=(Vector){ sin( D ),cos( D ),0 }/-L; bool big=(i%4)==0;
    if( fabs( angle( D-Course,_Pi ) )<_Ph/8.1 )color( navy ); else
                                               color( lightgreen,0,0.5 );
    arrow( spot( V*0.8,big?8:4 ),V,(big?0.2:0.1) );
    if( (!left && (i<11 || i>15)) || (left && (i<17 || i>21 ) ) )
    Text( (::Course)Rumb[((i+18)/4)%8],0.96*V,"%s",Rmbs[(i+16)%32] );
  }
  //  поверхность действующей и сильно искривлённой ватерлинии
  //
  B.axiZ( _Ph )/=Length/2; color( navy ); glBegin( GL_LINE_LOOP );
  for( i=0;i<wL.length;i+=2 )dot( B*(Tensor(*this)*wL[i]) );
  for( i=0;i<wR.length;i+=2 )dot( B*(Tensor(*this)*wR[i]) ); glEnd();
  //
  //  контроль положения руля весьма полезен для понимания маневров на волнении
  //
 Real H; B.axiZ( C.z );
  angle( H=Hull::Course+Swing[-1].z ); // текущее отклонение от заданого курса
  { const Vector P=(Vector){ -0.85 },N=(Vector){ 0,0.025 }; Vector W;
    if( fabs( H )<_Pi/32 )W=(Vector){ 0.2 }; else   // полрумба в доле градуса°
    if( dCs<_Pi/45 )W=(Vector){ 0.2,0.06 }; else W=(Vector){ 0.18,0.125 };
    if( H<0 )W.y=-W.y;     spot( B*P,10,gray );
    glBegin( GL_POLYGON ); dot( B*(P-W),blue ); dot( B*(P-N) );
                           dot( B*(P+W/1.5) );  dot( B*(P+N) ); glEnd();
  }
  //  треугольник расходящейся волны Кельвина отмечает скорость хода корабля
  //
  B/=Length/2;
  L=copysign( Speed*Speed,Speed )*_Pd/_g; // длина поперечной корабельной волны
  H=L*sqrt( 0.125 );          // ширина в основании расходящихся волн Кельвина
  C=WaterLine[Speed>0?-1:0]; // точка входа форштевня на действующей ватерлинии
  for( i=0; i<2; i++ )
  { if( !i ){ glLineWidth(8); color( silver,0,0.3 ); glBegin( GL_LINE_LOOP ); }
       else { glLineWidth(1); color( olive, 0,0.3 ); glBegin( GL_POLYGON ); }
    P(C.x,0),P(C.x-L,H),P(C.x-.9*L,.6*H),P(C.x-L/2,0),P(C.x-.9*L,-.6*H),P(C.x-L,-H);
    glEnd();
  }
  //  контрольная полупрозрачная плоскость для чисто конструктивной ватерлинии
  //
  color( lightgray,0.0,0.5 ); glBegin( GL_POLYGON );
  for( i=0; i<WaterLine.length; i++ )dot( B*WaterLine[i] );
  for( i=WaterLine.length-1; i>=0; i-- )dot( B*~WaterLine[i] ); glEnd();
  //
  //  в центре картушки подписывается текущий курс и действующая скорость судна
  //
  color( lightblue,-0.25 );
  Text( _Center,(Point){0},"%s ",DtoA( atan2(-x.y,x.x )*_Rd,-1 ) );
  Text( _Down,(Point){0},"%3.1fуз",Speed*3600/_Mile );
  ///
  //!  графики качки в окне аксонометрической проекции корпуса корабля
  //
  color( navy ); Win->Print( 1,-7," Time %s + %.2f\"\n"
                                  " Speed %.1f от %.1f узл\n"
                                  "    Fr %.2f ~ %.1f длин\n"
                                  " Volume %.0f << %.0f\n"
                                  " Surface %.0f << %.0f\n"
                                  " Floatable %.0f << %.0f\n"
                                  " Metacenter %.1f << %.1f\n"
                                  "          h %.1f << %.1f",
  DtoA( Storm->Trun/3600,3 ),Storm->Tstep,Speed*3600/_Mile,cSp*3600/_Mile,
  Speed/sqrt(_g*Length),sqr(Speed)*_Pd/_g/Length, Volume,iV,
  Surface,iS, Floatage,iF, Metacenter.x,vM.x, hX,vM.z );

  if( Win==this )
  if( (l=Route.length-1)>=12 )
  { Real Dt=Storm->Tstep/Storm->tKrat;   // расчетный шаг в циклах эксперимента
    //                                                                z=x.y-y.x
    //!  внешний блок графиков по кинематике корпуса на штормовом волнении моря
    //   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Vector Max,Min;
   Compass.Area( 25,0,0,7 ).Activate( true );   // семь строк от нижней границы
   Real Lp=Real(Compass.Width)/Compass.Height,  // размеры информационного окна
        Lt=11.0*Real(Fnt->W)/Compass.Height;    // и удвоенный отступ справа
    color( lightgreen,.7 ),rectangle( (Point){Lp-Lt,-0.5},(Point){Lp,0.46} );
    color( red );  Print( -4,-3,"X" );   // крен относительно абсциссы на оси X
    color( blue ); Print( -2,-3,"/Y" );  // дифферент вокруг ординаты  по оси Y
    color( green ),Print( -0,-3,"/Z" ); // вертикальная качка вдоль аппликаты Z
    //
    //   здесь надо выделить временной интервал записи графиков в экранном поле
    //
   int k=max( 0,l-int( sT/Dt ) );
   Real q,p; L=(Lp*2-Lt)/(l-k);                  // коэффициент отсчёта времени
    for( Max.x=(Min.x=(p=Swing[k].x)-eps)+eps*2,i=k; i<=l; i++ ) // с учётом
       { if(i>k){ if(q-p>_Pi)p+=_Pd; else if(p-q>_Pi)p-=_Pd; } // опрокидывания
         if( Max.x<p )Max.x=p; else if( Min.x>p )Min.x=p; q=p; p=Swing[i].x; }
    for( Max.y=(Min.y=-Swing[k].y-eps)+eps*2,i=k+1; i<=l; i++ )
       { Real r=-Swing[i].y; if( Max.y<r )Max.y=r; else if( Min.y>r )Min.y=r; }
    for( Max.z=(Min.z=Route[k].z-eps)+eps*2,i=k+1; i<=l; i++ )
       { Real r=Route[i].z; if( Max.z<r )Max.z=r; else if( Min.z>r )Min.z=r; }
    //
    // сохраняя асимметрию масштабов - центровка графиков относительно нуля
    //
/*  Max.x=min(_Ph,max(Max.x,-Min.x)); Min.x=max(-_Ph,min(-Max.x,Min.x));
    if( Max.x<-Min.x )Max.x=-Min.x;   if( Min.x>-Max.x )Min.x=-Max.x;
    if( Max.y<-Min.y )Max.y=-Min.y;   if( Min.y>-Max.y )Min.y=-Max.y;
    if( Max.z<-Min.z )Max.z=-Min.z;   if( Min.z>-Max.z )Min.z=-Max.z;
*/  //
    // подпись экстремальных величин на правом незатеняемом участочке
    //
    Compass.Alfabet( 20,"Times New Roman",FW_MEDIUM,true );
    q=Max.x,p=Min.x; if( q>_Pd )q-=_Pd,p-=_Pd;
                     if( p<-_Pd )q+=_Pd,p+=_Pd;
    color( red );  Compass.Print( -1,1, DtoA( q*_Rd,-1 ) );
                   Compass.Print( -1,0, DtoA( p*_Rd,-1 ) );
    color( blue ); Compass.Print( -1,2, DtoA( Max.y*_Rd,-1 ) );
                   Compass.Print( -1,-1,DtoA( Min.y*_Rd,-1 ) );
    color( green ),Compass.Print( -1,3, "%3.2f",Max.z )
                          .Print( -1,-2,"%3.2f",Min.z ); Max.z*=2,Min.z*=2;
    //
    //                       дальнейшее масштабирование выполняется только по Y
    //
    Max-=Min;                  // размах бортовой, килевой и вертикальной качки
    //
    //  разметка поля графиков с выделением полос в 10° и 20° по бортовой качке
    //
   Real U20=( 20.0*_dR-Min.x)*2/Max.x-1,U10=( 10.0*_dR-Min.x)*2/Max.x-1,
        D10=(-10.0*_dR-Min.x)*2/Max.x-1,D20=(-20.0*_dR-Min.x)*2/Max.x-1;
    if( U20<1.0 )                          // 20° - невыносимое веселье в качке
    { color(lightblue,.7,.5),rectangle((Point){-Lp,U20},(Point){Lp-Lt,1} ); }
    if( U10<1.0 )                          // 10° - комфортная радость на борту
    { color(lightgreen,0,.2),rectangle((Point){-Lp,U10},(Point){Lp-Lt,min(1.0,U20)} ); }
      color(cyan,.7,.2),rectangle((Point){-Lp,max(-1.0,D10)},(Point){Lp-Lt,min(1.0,U10)} );
    if( D10>-1.0 )
    { color(lightgreen,0,.2),rectangle((Point){-Lp,max(-1.0,D20)},(Point){Lp-Lt,D10} ); }
    if( D20>-1.0 )
    { color(lightblue,.7,.5),rectangle((Point){-Lp,-1},(Point){Lp-Lt,D20} ); }
    //
    // разметка шкалы времени
    //
   Real dT=(l-k)*Dt;                   // видимый интервал времени эксперимента
    H=dT<60?AxisStep(dT)/5:AxisStep(dT/60)*10;  // поминутный шаг шкалы времени
    color( green,-0.25 ); liney( (Point){ -Lp,1 },(Point){ Lp-Lt,1 } );
    for( i=0; i*H<dT; i++ )
    { Real mX = -Lp + i*H*L/Dt;
      liney( (Point){ mX,1 },(Point){ mX,i%(dT<60?5:6)?0.92:0.85 } );
      if( mX<Lp-Lt*1.2 && i && !(i%(dT<60?5:6)) )
      Compass.Text( _North,(Point){ mX,1 },DtoA( i*H/3600,3 ) );
    } Compass.Text( _North_East,(Point){ Lp-Lt,1 },DtoA( dT/3600,3 ) );
    //
    // закрашенный профиль волны по вертикальной качке
    //
    color( lightgreen,0,0.5 ); glBegin( GL_QUAD_STRIP );
    for( i=k; i<=l; i++ )
      glVertex3d( -Lp+(i-k)*L,0,0 ),
      glVertex3d( -Lp+(i-k)*L,(Route[i].z*2-Min.z)/Max.z-0.5,0 ); glEnd();
    color( green ); glBegin( GL_LINE_STRIP );
    for( i=k; i<=l; i++ )
      glVertex3d( -Lp+(i-k)*L,(Route[i].z*2-Min.z)/Max.z-0.5,0 ); glEnd();
    //
    // теперь поверху бортовая и килевая качка
    //
    line( (Point){-Lp,0,0},(Point){Lp-Lt,0,0},cyan );   // уровень среднего (0)
    glBegin( GL_LINE_STRIP ); color( magenta ); H=max( Max.x,Max.y );
    for( p=Swing[k].x,i=k; i<=l; i++ )
    { if( i>k ){ if( q-p>_Pi )p+=_Pd; else if( p-q>_Pi )p-=_Pd; }
      glVertex3d( -Lp+(i-k)*L,((p-Min.x)*2-Max.x)/H,0 ); q=p; p=Swing[i].x;
    } glEnd();
    glBegin( GL_LINE_STRIP ); color( blue );
    for( i=k; i<=l; i++ )
      glVertex3d( -Lp+(i-k)*L,((-Swing[i].y-Min.y)*2-Max.y)/H,0 ); glEnd();
  } return *this; // Compass.Save();
}
//  Простая визуализация формы исходного корпуса в независимом графическом окне
//  с прорисовкой корпуса корабля после специальной настройки графической среды
//
bool Hull::Draw()                // Виртуальная процедура простой перерисовки
{ static bool Recurse=false;
  if( Recurse || !ShipName )return false; Recurse=true; Activate();
  glEnable( GL_LIGHTING );                                // OpenGL - на корпус
  glEnable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST );
  glClearColor( 0.9,0.95,0.99,1 );            // голубоватый цвет фона и полная
  glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT ); // расчистка всего экрана
  glMatrixMode( GL_PROJECTION ); glLoadIdentity(); // сброс матричных координат
  gluPerspective( 15,Real(Width)/Height,-1e6,1e1 ); // к перспективной проекции
  gluLookAt( 0,0,Distance,look.x,look.y,look.z,0,1,0 ); // позиция точки обзора
  glMatrixMode( GL_MODELVIEW ); glLoadIdentity();         // исходное состояние
  glRotated( eye.y-90,1,0,0 );                            // поставить на киль
  glRotated( eye.x+atan2(x.x,x.y)*_Rd,0,0,1 );            // рыскание
  glRotated( eye.z,0,1,0 );                               // дифферент
//axis(*this,Length/1.8,Breadth/1.8,Draught,"X","Y","Z"); // Координатные оси
  Drawing(); color( blue );                               // и рисунок в памяти
  Print( 2,1,"%s\n { %s }\n %s",sname( FileName ),ShipName,Model[Statum] );
// hiFlow&1?"Кинематика по хордам 4-х точек скольжения над волновыми склонами":
//(!(hiFlow&2)?"Вертикальная гидростатика: бортовая-килевая-вертикальная качка":
//             "Архимедовы градиенты по склонам волн: все шесть видов качки" );
  Save().Refresh(); Recurse=false; return false;
}
/*
⁰ⁱ⁴⁵⁶⁷⁸⁹⁺⁻⁼⁽⁾ⁿ₀₁₂₃₄₅₆₇₈₉
static void star( _Real r, int n=32 );
  star( 1.0,32 );
static void peak(_Real r,_Real b,_Real a,colors c )      // длина, ширина, угол
{ glBegin( GL_POLYGON ); color( c,0.5 ); glVertex3dv( (Vector){ 0,0,1 } ); color( c );
  glVertex3dv( (Vector){ b*sin( a-_Ph/2 ),b*cos( a-_Ph/2 ),1 } ); color( c,0.5 );
  glVertex3dv( (Vector){ r*sin( a ),      r*cos( a ),      1 } ); color( c,0.75 );
  glVertex3dv( (Vector){ b*sin( a+_Ph/2 ),b*cos( a+_Ph/2 ),1 } ); glEnd();
}
static void star( _Real r, int n )
{ for( int i=0; i<16; i++ )peak( r,r/40,i*_Ph/4.0+_Ph/8.0,gray );
  for( int i=0; i<8;  i++ )peak( r,r/20,i*_Ph/2.0+_Ph/4.0,cyan );
  for( int i=0; i<4;  i++ )peak( r,r/15,i*_Ph+_Ph/2.0,green );
  for( int i=0; i<4;  i++ )peak( r,r/10,i*_Ph,blue ); color( white,0.5,0.5 );
}*/
