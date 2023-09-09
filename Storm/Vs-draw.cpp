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
#define V(a,b,c)glVertex3dv \
      ( out( (Vector){xm a Length/2,b Breadth/2,c Draught/2-Draught/2} ) ),
 Real xm=(Keel[0]+Keel[Nframes+1])/2;
  color( cyan ),glBegin(GL_LINE_LOOP),V(+,+,+)V(+,-,+)V(-,-,+)V(-,+,+)glEnd();
  color( gray ),glBegin(GL_LINE_LOOP),V(+,+,-)V(+,-,-)V(-,-,-)V(-,+,-)glEnd();
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
    for( R.z=0; R.z>=D; R.z+=dP ){ P=F.WaveV(Trun,R,V); arrow(P,P+V,abs(V)/6); }
  }
  dS=( S=abs( Ahead ) )/120.0;
  for( Real s=0; s<S*2; s+=dS )
  { R=Ahead*( s/S )-Ahead; color( lightgreen ); glBegin( GL_LINE_STRIP );
    for( R.z=0; R.z>=D; R.z+=dQ )dot( F.Wave(Trun,R) ); glEnd(); color( cyan );
    for( R.z=0; R.z>=D; R.z+=dP ){ P=F.WaveV(Trun,R,V); arrow(P,P+V,abs(V)/6); }
  } glLineWidth( 1 ); return *this;
}
//! общая навигационная информация о состоянии корабля
//
const static byte Rumb[]=
 { _North,_North_East,_East,_South_East,_South,_South_West,_West,_North_West };

#define P( x,y ) glVertex3dv( B*(Vector){ x,y } )
inline Real i90( _Real r ){ return r>_Ph ? _Pi-r : ( r<-_Ph ? -_Pi-r : r ); }

static void DirWave( const Waves &W, colors C, Place &D )
{ if( W.Height )
  { Real H=W.Height/Vessel->Length*4,L=W.Length/Vessel->Length;
   _Tensor B=W; if( H/L<.06 )H=L*.06; color( C,0,.6 ); glBegin( GL_POLYGON );
    P(0,0),P(-H,-L+H),P(0,-L),P(H,-L+H),P(-H,L),P(0,L-H),P(H,L); glEnd();
    color( C ); if( (H=angle( H=atan2( W.x.y,-W.x.x )))<0.0 )H+=_Pd;
   char S[24],*s=S;
    snprintf( S,23,"%3s:%3.1f/%3.1f\n",
              Rmbs[int(H*32/_Pd+0.49)%32],W.Height,W.Length/W.Cw );
    if( s=strchr( S,'.' ) ){ *s=',';
      if( s=strchr( s,'.' ) )*s='\"'; } D.Print( S );
} }
Hull& Hull::Naviga_Inform( Window *Win )
{ Field &S=*Storm; Tensor B;     // единичный масштаб пространственного тензора
 Vector C=Swing[-1]; C.z+=_Ph;   // крен, дифферент и курс корабля(в геобазисе)
 int i,l=-0.18*hypot( Win->Width,Win->Height );       // размер из аксонометрии
 Place Compass( Win,PlaceOrtho );      // | PlaceAbove ~~ c единичной разметкой
 TextContext TS( true ); Compass.Area( 0,0,1.25*l,l ).Activate( true );
  //
  //! картушка морского волнения, курса, скорости корабля
  //+  отметки трёх волновых полей Wind+Swell+Surge = Length+Height+=Tensor::x
  //                                                        амплитуда=>высота/2
 Real L=min( 1.0,Length     // условная высота и длина стрелки волны в картушке
      / max( max( S.Wind.Length,S.Swell.Length ),S.Surge.Length ) );
  glTranslated( 0,-0.1,0 );
  glScaled( 0.9*L,0.9*L,1 );
  color( white,0,.33 ),circle( (Point){0},1/L );  // круг картушки 110% корпуса
 bool left=int( 2+Course/_Ph )&1; Compass.Print( left?-1:1,1,"" ); color(navy);
  if( S.Exp.wave&3 )DirWave( S.Wind,green,Compass ),        // ветровая волна
                    DirWave( S.Swell,blue,Compass ),        // свежая зыбь
                    DirWave( S.Surge,cyan,Compass );        // реликтовый накат
  //
  //  стрелка-указатель заданного курса корабля
  //
  B.axiZ( _Ph-Course )/=L;
  for( i=0; i<2; i++ )                            // стрелка выбранного курса
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
    if( fabs( angle( D-Course,_Pi ) )<_Ph/8.1 )
      color( navy ); else color( green,0,0.5 );
      arrow( spot( V*0.8,big?10:6 ),V,abs( V )*(big?0.15:0.1) );
    if( (!left && (i<11 || i>15)) || (left && (i<17 || i>21 ) ) )
    Text( (::Course)Rumb[((i+18)/4)%8],0.96*V,"%s",Rmbs[(i+16)%32] );
  }
  //  контроль положения руля весьма полезен для понимания маневров на волнении
  //
 Real U,H; B.axiZ( C.z );
  angle( H=Hull::Course+Swing[-1].z ); // текущее отклонение от заданого курса
  { const Vector R=(Vector){ -0.85 },N=(Vector){ 0,0.025 }; Vector W;
    if( fabs( H )<_Pi/32 )W=(Vector){ 0.2 }; else   // полрумба в доле градуса°
    if( dCs<_Pi/45 )W=(Vector){ 0.2,0.06 }; else W=(Vector){ 0.18,0.125 };
    if( H<0 )W.y=-W.y;     spot( B*R,10,gray );
    glBegin( GL_POLYGON ); dot( B*(R-W),blue ); dot( B*(R-N) );
                           dot( B*(R+W/1.5) );  dot( B*(R+N) ); glEnd();
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
    P(C.x,0),P(C.x-L,H),P(C.x-.9*L,.6*H),P(C.x-L/2,0),
    P(C.x-.9*L,-.6*H),P(C.x-L,-H); glEnd();
  }
  //  контрольная полупрозрачная плоскость для чисто конструктивной ватерлинии
  //
  glBegin( GL_TRIANGLES ); //GL_POLYGON ); //GL_TRIANGLE_FAN );
  for( i=0; i<WaterLine.len; i+=3 )
  { color( lightcyan,0,0.5 ); dot( (Point){0} ); color( lightgray,0,0.5 );
                              dot( B*WaterLine[i+1] );
                              dot( B*WaterLine[i+2] ); } glEnd();
  //
  //  поверхность действующей и сильно искривлённой ватерлинии
  //
  B.axiZ( -_Ph )/=Length/2; B=Tensor(*this)*B; color( gray,-0.25 );
  for( i=0; i<wL.len; i+=3 )line( B*wL[i+1],B*wL[i+2] );
  //
  //  в центре картушки подписывается текущий курс и действующая скорость судна
  //
  color( lightblue,-0.25 );
  Text( _Center,(Point){0},"%s ",DtoA( atan2(-x.y,x.x )*_Rd,-1 ) );
  Text( _Down,(Point){0},"%3.1fуз",Speed*3600/_Mile );
  //
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
  DtoA( S.Trun/3600,3 ),S.Tstep,Speed*3600/_Mile,cSp*3600/_Mile,
  Speed/sqrt(_g*Length),sqr(Speed)*_Pd/_g/Length, Volume,iV,
  Surface,iS, Floatage,iF, Metacenter.x,vM.x, hX,vM.z );
  if( Win==this )
  if( (l=Route.len-1)>=12 )
  { Real Dt=S.Tstep/S.tKrat;             // расчетный шаг в циклах эксперимента
    //                                                                z=x.y-y.x
    //!  внешний блок графиков по кинематике корпуса на штормовом волнении моря
    //    c масштабированием под размах бортовой, килевой и вертикальной качки
    //   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Vector Max;                                  // экстремумы графиков качки
   Compass.Area( 25,0,0,7 ).Activate( true );   // семь строк от нижней границы
   Real Lp=Real(Compass.Width)/Compass.Height,  // размеры информационного окна
        Lt=11.0*Real(Fnt->W)/Compass.Height;    // и удвоенный отступ справа
    //
    //   здесь надо выделить временной интервал записи графиков в экранном поле
    //
   Real r,mz; int k=max( 0,l-int( sT/Dt ) ); L=(Lp*2-Lt)/(l-k);
    for( i=k; i<=l; i++ ) // коэффициент отсчёта времени с учётом опрокидывания
    { r=fabs( i90( Swing[i].x ) );
                            if( i==k )Max.x=r+eps; else if( Max.x<r )Max.x=r;
      r=fabs( Swing[i].y ); if( i==k )Max.y=r+eps; else if( Max.y<r )Max.y=r;
    }                               // только полвертикали + центровка графиков
    for( Max.z=(mz=Route[k].z-eps)+eps*2,i=k+1; i<=l;i++ ) // относительно нуля
    { r=Route[i].z; if( Max.z<r )Max.z=r; else if( mz>r )mz=r;
    } (Max.z*=2)-=(mz*=2);
    //
    //  разметка поля графиков с выделением полос в 10° и 20° по бортовой качке
    //
    H=max( Max.x,Max.y );   // дальнейшее масштабирование по максимуму от X и Y
    U=10.0*_dR/H;
//  color( lightcyan,.8,.75 ); rectangle( (Point){-Lp,-1},(Point){Lp-Lt,1} );
    if( U<1.0 )
    { color( lightgreen,0.0,0.2 );        // <10° - комфортная радость на борту
      rectangle( (Point){-Lp,U},(Point){Lp-Lt,min(1.0,2*U)} );
      rectangle( (Point){-Lp,-U},(Point){Lp-Lt,max(-1.0,-2*U)} );
//    if( U<0.5 )
//    { color( lightblue,0.7,0.5 );       // >20° - невыносимое веселье в качке
//      rectangle( (Point){-Lp,2*U},(Point){Lp-Lt,1} );
//      rectangle( (Point){-Lp,-2*U},(Point){Lp-Lt,-1} );
//    }
    }
    //   подпись экстремальных величин на правом незатеняемом участочке
    //
    Compass.Alfabet( 20,"Times New Roman",FW_MEDIUM,true );
    //
    //   разметка шкалы времени
    //
   Real dT=(l-k)*Dt;                   // видимый интервал времени эксперимента
    U=dT<60?AxisStep(dT)/5:AxisStep(dT/60)*10;  // поминутный шаг шкалы времени
    color( green,-0.25 ); liney( (Point){ -Lp,1 },(Point){ Lp-Lt,1 } );
    for( i=0; i*U<dT; i++ )
    { Real mX = -Lp + i*U*L/Dt;
      liney( (Point){ mX,1 },(Point){ mX,i%(dT<60?5:6)?0.92:0.85 } );
      if( mX<Lp-Lt*1.2 && i && !(i%(dT<60?5:6)) )
      Compass.Text( _North,(Point){ mX,1 },DtoA( i*U/3600,3 ) );
    } Compass.Text( _North_East,(Point){ Lp-Lt,1 },DtoA( dT/3600,3 ) );
    //
    //   закрашенный профиль волны по вертикальной качке
    //
    color( lightgreen,.7,.8 ); rectangle( (Point){Lp-Lt,-.5},(Point){Lp,.46} );
    glBegin( GL_QUAD_STRIP );
    for( i=k; i<=l; i++ )
      glVertex3d( -Lp+(i-k)*L,0,0 ),
      glVertex3d( -Lp+(i-k)*L,(Route[i].z*2-mz)/Max.z-0.5,0 ); glEnd();
    color( green ); glLineWidth( 1.6 ); glBegin( GL_LINE_STRIP );
    for( i=k; i<=l; i++ )
      glVertex3d( -Lp+(i-k)*L,(Route[i].z*2-mz)/Max.z-0.5,0 ); glEnd();
    Compass.Print( -1,3,"%3.2f",(Max.z+mz)/2 )
           .Print( -1,5,"%3.2f",mz/2 )
           .Print(  0,4,"/Z " );        // вертикальная качка вдоль аппликаты Z
    //
    // поверху накладывается бортовая и килевая качка
    //
    color( blue ); glBegin( GL_LINE_STRIP );
    for( i=k; i<=l; i++)glVertex3d(-Lp+(i-k)*L,(Swing[i].y-Trim)/H,0); glEnd();
    Compass.Print( -1,Max.x>Max.y?2:1,DtoA( (Trim+Max.y)*_Rd,-1 ) )
           .Print( -3,4,"/Y" );          // дифферент вокруг ординаты  по оси Y
    color( magenta ); glBegin( GL_LINE_STRIP );
    for( i=k; i<=l; i++)glVertex3d( -Lp+(i-k)*L,i90(Swing[i].x)/H,0 ); glEnd();
    Compass.Print( -1,Max.x>Max.y?1:2,DtoA( Max.x*_Rd,-1 ) )
           .Print( -5,4,"X " );          // крен относительно абсциссы на оси X
    //
    // и три ускорения свободного падения по наблюдениям в оконечностях корпуса
    //
   Real A1,A2=0,A3,F1,F2=0,F3,maxA,minA,maxF,minF,D2=Dt*Dt*_g;
    glBegin( GL_QUAD_STRIP );
    for( i=k; i<=l; i++ )
    { U = Length*sin( Swing[i].y )/2;
      F3=Route[i].z-U;             // нос - аппликата ватерлинии на форштевне
      A3=Route[i].z+U;             // корма -- на ахтерштевне
      if( i-k>2 )                  // кривая лента носовых и кормовых ускорений
      { H=(F1-F2-F2+F3)/D2;        // ускорение на форштевне
        if( i-k==3 )maxF=(minF=H-eps)+2*eps; else if( maxF<H )maxF=H;
                                             else if( minF>H )minF=H;
        color( blue,0,0.1 ); glVertex3d( -Lp+(i-k-1)*L,H/2,0 );
        H=(A1-A2-A2+A3)/D2;        // ускорение на ахтерштевне
        if( i-k==3 )maxA=(minA=H-eps)+2*eps; else if( maxA<H )maxA=H;
                                             else if( minA>H )minA=H;
        color( red,0,0.1 ); glVertex3d( -Lp+(i-k-1)*L,H/2,0 );
      } A1=A2,A2=A3,F1=F2,F2=F3;
    } glEnd();
    glLineWidth( 3 ); color( yellow ); glBegin( GL_LINE_STRIP );
    for( i=k; i<=l; i++ )
    { A3 = Route[i].z;                                        // мидельшпангоут
      if( i-k>2 )glVertex3d( -Lp+(i-k-1)*L,(A1-A2-A2+A3)/D2/2,0 ); A1=A2,A2=A3;
    } glEnd();
    glLineWidth( 1 ); color( cyan ); glBegin( GL_LINE_STRIP );
    for( i=k; i<=l; i++ )
    { F3=Route[i].z - Length*sin( Swing[i].y )/2;             // нос
      if( i-k>2 )glVertex3d( -Lp+(i-k-1)*L,(F1-F2-F2+F3)/D2/2,0 ); F1=F2,F2=F3;
    } glEnd();
    Compass.Print( -1,-1,"%+3.1f/%+3.1f",maxF+1,minF+1 );
    color( magenta,0.5 ); glBegin( GL_LINE_STRIP );
    for( i=k; i<=l; i++ )
    { A3=Route[i].z + Length*sin( Swing[i].y )/2;             // корма
      if( i-k>2 )glVertex3d( -Lp+(i-k-1)*L,(A1-A2-A2+A3)/D2/2,0 ); A1=A2,A2=A3;
    } glEnd();
    Compass.Print( -1,0,"%+3.1f/%+3.1f",maxA+1,minA+1 );
    U=60.0*Lp/Real(Compass.Width);
    H=55.0/Real(Compass.Height);
    line( (Point){-Lp},(Point){Lp-Lt},cyan );         // уровень среднего
    glColor3f( BkColor );
    rectangle( (Point){-Lp,1-H},      (Point){-Lp+U,1-H/4} ); // сверху
    rectangle( (Point){-Lp,-H/2.2},   (Point){-Lp+U,H/3}   ); // в центре
    rectangle( (Point){-Lp,-1+H*0.75},(Point){-Lp+U,-1 }   ); // внизу
    color( yellow,-0.4 );
    line( (Point){-Lp,-0.5},(Point){Lp-Lt,-0.5} );
    line( (Point){-Lp,0.5},(Point){Lp-Lt,0.5} );
    Compass.Text( _North, Lp=U/2-Lp,-1,0," -g" );
    Compass.Text( _Center,Lp,        0,0,"  g" );
    Compass.Text( _South, Lp,        1,0," 3g" );
  } return *this;
}
//  Простая визуализация формы исходного корпуса в независимом графическом окне
//  с прорисовкой корпуса корабля после специальной настройки графической среды
//
bool Hull::Draw()                  // Виртуальная процедура простой перерисовки
{ static bool Recurse=false;
  if( Recurse || !ShipName )return false; Recurse=true; Activate();
  glEnable( GL_LIGHTING );                                // OpenGL - на корпус
  glEnable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST );
  glClearColor( BkColor,1 );                  // голубоватый цвет фона и полная
  glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT ); // расчистка всего экрана
  glMatrixMode( GL_PROJECTION ); glLoadIdentity(); // сброс матричных координат
  gluPerspective( 15,Real(Width)/Height,-1e6,1e1 ); // к перспективной проекции
  gluLookAt( 0,0,Distance,look.x,look.y,look.z,0,1,0 ); // позиция точки обзора
  glMatrixMode( GL_MODELVIEW ); glLoadIdentity();         // исходное состояние
  glRotated( eye.y-90,1,0,0 );                            //  поставить на киль
  glRotated( eye.x+atan2(x.x,x.y)*_Rd,0,0,1 );            //           рыскание
  glRotated( eye.z,0,1,0 );                               //          дифферент
//axis(*this,Length/1.8,Breadth/1.8,Draught,"X","Y","Z"); // Координатные оси и
  Drawing(); color( blue );                               //   рисунок в памяти
  Print( 2,1,"%s\n { %s }\n %s",sname( FileName ),ShipName,Model[Statum] );
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
