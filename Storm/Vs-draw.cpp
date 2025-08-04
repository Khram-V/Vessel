//
//      Несколько независимых информационных процедур без предустановок
//      отрабатываются в текущем/произвольном графическом окне OpenGL
//                                                           2021-03-23 יְרוּשָׁלַיִם
//
#include "Aurora.h"       // объекты и производные операции с корпусом на волне

Hull& Hull::Contour_Lines()        // рисуем контуры габаритного прямоугольного
{ glAct( this );                   // параллелепипеда по заданным размерениям
  glLineWidth( 0.05 );             // сглаживание линий
  glEnable( GL_LINE_SMOOTH );
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
 const Real // &Trun=F.Trun,
      D=-hypot( Draught,Breadth/2 ),
      dP=D/9,dQ=dP/6;                   // профильное заглубление и детальность
 Real S=hypot( F.Long,F.Wide ),dS=F.dS; // дистанция и шаг разметки профиля
 Vector Ahead=x;    Ahead.z=0;    Ahead=S*dir( Ahead );
 Vector Traverse=y; Traverse.z=0; Traverse=S*dir( Traverse );
 Vector R,V,P; R.x=fabs( F.Long/Ahead.x )/2;
               R.y=fabs( F.Long/Traverse.x )/2;
  Ahead *= R.x<0.5 ? R.x : F.Wide/fabs( Ahead.y )/2;
  Traverse*=R.y<0.5 ? R.y : F.Wide/fabs( Traverse.y )/2;
//glLineWidth( 0.01 ); arrow( -Traverse,Traverse,Length/240,lightcyan );
//                     arrow( -Ahead,Ahead,Length/240,yellow );
  glLineWidth( 0.01 ); arrow( -Traverse,Traverse,0.005,lightcyan );
                       arrow( -Ahead,Ahead,0.005,yellow );
  glLineWidth( 1 );                           // ускоренная прорисовка без волн
  if( !F.Exp.wave )goto Exit; color( lightblue );
  for( Real w=0; w>=D; w+=dP )
  { S=abs( Ahead ); glBegin( GL_LINE_STRIP );
    for( Real s=0; s<S*2; s+=dS )              // профили волн по курсу корабля
    { R = -Ahead + ( s/S )*Ahead; R.z=w; dot( F.Wave( Trun,R ) ); } glEnd();
    S=abs( Traverse ); glBegin( GL_LINE_STRIP );
    for( Real s=0; s<S*2; s+=dS )                   // профили волн по траверзу
    { R = -Traverse + ( s/S )*Traverse; R.z=w; dot( F.Wave(Trun,R)); } glEnd();
  }
  // пусть будет еще лишняя сотня кривых линий вертикальных граней жидких ячеек
  //                                          0.01
  dS=( S=abs( Traverse ) )/120.0; glLineWidth( 0.1 ); // длина и шаг профиля и
  for( Real s=0; s<S*2; s+=dS )           //! зелёные стрелки скоростей течений
  { R=( s/S )*Traverse -Traverse; color( lightgreen ); glBegin( GL_LINE_STRIP );
    for( R.z=0; R.z>=D; R.z+=dQ )dot( F.Wave(Trun,R) ); glEnd(); color( green );
    for( R.z=0; R.z>=D; R.z+=dP ){ P=F.WaveV(Trun,R,V); arrow(P,P+V,0.2 ); }    //abs(V)/6); }
  }
  dS=( S=abs( Ahead ) )/120.0;
  for( Real s=0; s<S*2; s+=dS )
  { R=( s/S )*Ahead-Ahead; color( lightgreen ); glBegin( GL_LINE_STRIP );
    for( R.z=0; R.z>=D; R.z+=dQ )dot( F.Wave(Trun,R) ); glEnd(); color( green );
    for( R.z=0; R.z>=D; R.z+=dP ){ P=F.WaveV(Trun,R,V); arrow(P,P+V,0.2 ); }    //abs(V)/6); }
  }
Exit: glLineWidth( 1 ); return *this;
}
//! общая навигационная информация о состоянии корабля
//
const static byte Rumb[]=
  { _North,_North_East,_East,_South_East,_South,_South_West,_West,_North_West
  };
static Matrix B={{0},1};         // единичный масштаб пространственного тензора
static void P( _Real x,_Real y ){ glVertex3dv( B.LtA( (Vector){ x,y } ) ); }
static Real i90( _Real r ){ return r>_Ph ? _Pi-r : ( r<-_Ph ? -_Pi-r : r ); }

static void DirWave( const Waves &W, colors C, Place &D )
{ if( W.Height )
  { Real H=W.Height/Vessel->Length*4,L=W.Length/Vessel->Length; B=W;
    if( H/L<0.06 )H=L*0.06; color( C,0,0.6 ); glBegin( GL_POLYGON );
    P(0,0),P(-H,-L+H),P(0,-L),P(H,-L+H),P(-H,L),P(0,L-H),P(H,L); glEnd();
    color( C ); if( (H=angle( H=atan2( W.x.y,-W.x.x )))<0.0 )H+=_Pd;
   char S[24],*s=S;
    snprintf( S,23,"\n%3s:%3.1f/%3.1f",
              Rmbs[int(H*32/_Pd+0.49)%32],W.Height,W.Length/W.Cw );
    if( s=strchr( S,'.' ) ){ *s=',';
      if( s=strchr( s,'.' ) )*s='\"'; } D.Print( S );
  }
}
Hull& Hull::NavigaInform( Window *Win )
{ Field &S=*Storm; glAct( Win );
 Vector C=Head[-1]; C.z+=_Ph;   // крен, дифферент и курс корабля (в геобазисе)
 int i,l=-0.18*hypot( Win->Width,Win->Height );       // размер из аксонометрии
 bool GMod = Pic.kart==0;      // режим с разделением графиков качки и ходкости
 Place Compass( Win,PlaceOrtho );  // | PlaceAbove ~~ случай единичной разметки
 TextContext TS( true );
  Compass.AlfaVector( Win==this?AlfaHeight():S.AlfaHeight() );
  if( Win!=this || !GMod )Compass.Area( 0,-Compass.AlfaHeight()/2,1.25*l,l );
                    else  Compass.Area( 2,-6,28,20 );
                          Compass.Activate( true );
  //
  //! картушка морского волнения, курса, скорости корабля
  //+   отметки трёх волновых полей Wind+Swell+Surge = Length+Height+=Tensor::x
  //                                                        амплитуда=>высота/2
 Real L=min( 1.0,Length     // условная высота и длина стрелки волны в картушке
      / max( max( S.Wind.Length,S.Swell.Length ),S.Surge.Length ) );
  glTranslated( 0,-0.1,0 ); color( cyan,0,.1 ); // круг картушки 110% корпуса
  glScaled( 0.9*L,0.9*L,1 ); circle( (Vector){0},1.0/L );
 bool left=int( 2+Course/_Ph )&1;
 int right=(S.Wind.Height!=0)+(S.Swell.Height!=0)+(S.Surge.Height!=0);
  color(navy); Compass.Print( left?-1:1,(GMod&&Win==this)+(right?3.1-right:2),right?"":" Штиль" );
  if( S.Exp.wave&3 )DirWave( S.Wind,green,Compass ),        // ветровая волна
                    DirWave( S.Swell,blue,Compass ),        // свежая зыбь
                    DirWave( S.Surge,cyan,Compass ); else   // реликтовый накат
  if( right )Compass.Print( 1,2+2*(GMod&&Win==this),"Заштилело" );
  //
  //                                  стрелка-указатель заданного курса корабля
  B.axiZ( _Ph-Course ).Scale=1.0/L;
  for( i=0; i<2; i++ )                              // стрелка выбранного курса
  { if( !i ){ glLineWidth( 2 ); color( gray,0,.75 ); glBegin( GL_LINE_LOOP ); }
       else { glLineWidth( 1 ); color( yellow,0,.3 ); glBegin( GL_POLYGON ); }
    P(.75,0),P(.4,-.05),P(.5,-.01),P(-.75,-.05),
    P(-.6,0),P(-.75,.05),P(.5,.01),P(.4,.05); glEnd();
  }
  //                       подпись - текущий курс в градусах и скорость в узлах
  for( i=0; i<32; i++ )
  { Real D=Real( i )*_Ph/8.0;                              //! картушка компаса
    Vector V=(Vector){ sin( D ),cos( D ),0 }/-L; bool big=(i%4)==0;
    if( fabs( angle( D-Course,_Pi ) )<_Ph/8.1 )color( navy );
                                          else color( green,.25,0.5 );
    arrow( spot( 0.8*V,big?10:6 ),V,big?0.15:0.1 );
    if( Win!=this || !GMod )
    if( !(i&1) )
    if( (!left && (i<11 || i>15)) || (left && (i<17 || i>21 ) ) )
    { color( blue );
      Compass.Text( (::Course)Rumb[((i+18)/4)%8],0.96*V,"%s",Rmbs[(i+16)%32] );
  } }
  //  контроль положения руля весьма полезен для понимания маневров на волнении
  //
 Real U,H; B.axiZ( C.z );               // в том же масштабе
  angle( H=Hull::Course+Head[-1].z );   // текущее отклонение от заданого курса
  { const Vector R=(Vector){ -0.85 },N=(Vector){ 0,0.025 }; Vector W;
    line(B.LtA((Vector){0,-.8}),B.LtA((Vector){0,.8}),lightblue); glLineWidth(2);
    spot( arrow( B.LtA((Vector){-1}),B.LtA((Vector){0.975}),0.12 ),5,blue );
    if( fabs( H )<_Pi/32 )W=(Vector){ 0.2 }; else   // полрумба в доле градуса°
    if( dCs<_Pi/45 )W=(Vector){ 0.2,0.06 }; else W=(Vector){ 0.18,0.125 };
    if( H<0 )W.y=-W.y; glBegin( GL_POLYGON );
    dot( B.LtA( (R-W) ) ),dot( B.LtA( (R-N) ) );
    dot( B.LtA( (R+W/1.5) ) ),dot( B.LtA( (R+N) ) ); glEnd();
    spot(B.LtA(R),6,lightmagenta );
  }
  //          кренометр по границе круга картушки полупрозрачным треугольничком
  //
  B.axiZ( C.x-_Ph );
  for( i=0; i<2; i++ )
  { if( !i ){ glLineWidth(3); color( yellow,0,0.3 ); glBegin( GL_LINE_LOOP ); }
       else { glLineWidth(1); color( red,0,0.2 ); glBegin( GL_POLYGON ); }
        P(1,0),P(.92,.12),P(.91,.05),P(.75,0),P(.91,-.05),P(.92,-.12); glEnd();
  }
  //   треугольник расходящейся волны Кельвина отмечает скорость хода корабля
  //
  B.axiZ( C.z )*=L/Length/2;
  L=copysign( Speed*Speed,Speed )*_Pd/_g; // длина поперечной корабельной волны
  H=L*sqrt( 0.125 );          // ширина в основании расходящихся волн Кельвина
  C=WaterLine[Speed>0?-1:0]; // точка входа форштевня на действующей ватерлинии
  for( i=0; i<2; i++ )
  { if( !i ){ glLineWidth(8); color( silver,0,0.3 ); glBegin( GL_LINE_LOOP ); }
       else { glLineWidth(1); color( olive, 0,0.3 ); glBegin( GL_POLYGON ); }
    P(C.x,0),P(C.x-L,H),P(C.x-.9*L,.6*H),P(C.x-L/2,0),
    P(C.x-.9*L,-.6*H),P(C.x-L,-H); glEnd();
  }
  //   контрольная полупрозрачная плоскость для чисто конструктивной ватерлинии
  //
  B.axiZ(-_Ph ).Scale=Length/2; // фиксируется масштаб в половину длины корпуса
  B*=Matrix(*this);
  glBegin( GL_TRIANGLES ); //GL_POLYGON ); //GL_TRIANGLE_FAN );
  for( i=0; i<WaterLine.len; i+=3 )
  { color( lightcyan,0,0.5 ); dot( (Vector){0} ); color( lightblue,0,0.5 );
                              dot( B.AtL( WaterLine[i+1] ) );
                              dot( B.AtL( WaterLine[i+2] ) ); } glEnd();
  //
  //  поверхность действующей и сильно искривлённой ватерлинии
  //
  color( magenta,-0.25 );
  for( i=0; i<wL.len; i+=3 )line( B.AtL( wL[i+1] ),B.AtL( wL[i+2] ) );
  //
  //  в центре картушки подписывается текущий курс и действующая скорость судна
  //
  color( lightblue,-0.25 ),Text( _Down,(Vector){0},
        "%s %3.1fуз",DtoA( atan2(-x.y,x.x )*_Rd,-1 ),Speed*3600/_Mile );
  //   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //!  графики качки в окне аксонометрической проекции корпуса корабля !!
  //   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
  color( Win==Vessel?navy:gray );
  Win->Print( 1,0," Time%s +%.2g\"/%.2g\n"
                  " Speed %.1f от %.1f узл\n"
                  "    Fr %.2f ~ %.1f длин\n"
                  " Volume %.0f << %.0f\n"
                  " Surface %.0f << %.0f\n"
                  " Floatable %.0f << %.0f\n"
                  " Metacenter %.1f << %.1f\n"
                  "            h %.1f << %.1f",
           DtoA( Trun/3600,Trun>3600?2:(Trun>60?3:-3) ),TimeStep,
           tKrat,Speed*3600/_Mile,cSp*3600/_Mile,Speed/sqrt(_g*Length),
           sqr(Speed)*_Pd/_g/Length,Volume,iV,Surface,iS,Floatage,iF,
           Metacenter.x,vM.x,hX,vM.z );
  if( Win==this )
  if( (l=Route.len-1)>=12 )
  {                                                               //~ z=x.y-y.x
//!      внешний блок графиков по кинематике корпуса на штормовом волнении моря
//         c масштабированием под размах бортовой, килевой и вертикальной качки
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Vector Max,Min;                              // экстремумы графиков качки
   Compass.Area( 29,-1,0,7 ).Activate( true );  // семь строк от нижней границы
   Real Lt=12*Compass.AlfaWidth()/Compass.Height, // и удвоенный отступ справа
        Lp=Lt+Compass.Width/Compass.Height;     // размеры информационного окна
    //
    //   здесь надо выделить временной интервал записи графиков в экранном поле
    //
   Real r; int k=max( 0,l-int( sT/Ts ) ); L=(Lp*2-Lt)/(l-k);
    for( i=k; i<=l; i++ ) // коэффициент отсчёта времени с учётом опрокидывания
    { r=i90( Head[i].x );
      if( i==k )Max.x=r+eps,Min.x=r-eps;
       else if( Max.x<r )Max.x=r; else if( Min.x>r )Min.x=r;
      r=Head[i].y;
      if( i==k )Max.y=r+eps,Min.y=r-eps;
       else if( Max.y<r )Max.y=r; else if( Min.y>r )Min.y=r;
    }                               // только полвертикали + центровка графиков
    for( i=k; i<=l; i++ )           // относительно нуля
    { r=max(Route[i].z,Mix[i].z); if(i==k)Max.z=r+eps; else if(Max.z<r)Max.z=r;
      r=min(Route[i].z,Mix[i].z); if(i==k)Min.z=r-eps; else if(Min.z>r)Min.z=r;
    } (Max.z*=2)-=(Min.z*=2);
    //
    //  разметка поля графиков с выделением полос в 10° и 20° по бортовой качке
    //
   Real Hx=max( Max.x,-Min.x ),
        Hy=max( Max.y,-Min.y );
    H=max( Hx,Hy );  // общее дальнейшее масштабирование х-крена и у-дифферента
    U=10.0*_dR/H;    //        ↓↓↓ - надо поправить, чтоб без тени повторений
//  color( lightcyan,.8,.75 ); rectangle( (Vector){-Lp,-1},(Vector){Lp-Lt,1} );
    if( U<1.0 )
    { color( lightgreen,0.0,0.2 );        // <10° - комфортная радость на борту
      rectangle( (Vector){-Lp,U},(Vector){Lp-Lt,min(1.0,2*U)} );
      rectangle( (Vector){-Lp,-U},(Vector){Lp-Lt,max(-1.0,-2*U)} );
      if( U<0.5 ){ color( lightblue,0.7,0.5 );// >20° невыносимые условия качки
        rectangle( (Vector){-Lp,2*U},(Vector){Lp-Lt,1} );
        rectangle( (Vector){-Lp,-2*U},(Vector){Lp-Lt,-1} ); }
    }
    //   разметка шкалы времени
    //
   const Real dT=(l-k)*Ts; int j=dT<60?5:6;    // интервал времени эксперимента
    U=dT<60?AxisStep(dT)/5:AxisStep(dT/60)*10; // поминутный шаг шкалы времени
    color( cyan,0.25 );   line( (Vector){ -Lp,0 }, (Vector){ Lp-Lt,0  } );
    color( green,0.25 ); liney( (Vector){ -Lp,.5 },(Vector){ Lp-Lt,.5 } );
    color( green,-0.25 ); line( (Vector){ -Lp,-1 },(Vector){ Lp-Lt,-1 } );
                          line( (Vector){ -Lp,1  },(Vector){ Lp,1 } );
    Compass.AlfaVector( Compass.AlfaHeight()*0.85,1 );
    for( i=0; i*U<dT+0.1; i++ )
    { Real mX = -Lp + i*U*L/Ts;
      liney( (Vector){ mX,1 },(Vector){ mX,i%j?0.92:0.85 } );
      //if( mX<Lp-Lt*1.2 )
      if( i && !( i%j ) )
        Compass.Text( _South,(Vector){ mX,-1 },DtoA( i*U/3600,3 ) );    //!-0.8
    } if( --i%j>1 )
        Compass.Text( _South_West,(Vector){ Lp,-1 },DtoA( dT/3600,3 ) );
    //
    //      закрашенный профиль волны под корпусом судна при вертикальной качке
    //
    color( lightgreen,.7,.8 );
    rectangle( (Vector){Lp-Lt,-.5},(Vector){Lp,.46} );
    glLineWidth( 2 );
    glBegin( GL_QUAD_STRIP );
    for( i=k; i<=l; i++ )
    { color( aqua,.2 ); glVertex3d( -Lp+(i-k)*L,(Mix[i].z*2-Min.z)/Max.z-.5,0);
      color( silver ); glVertex3d(-Lp+(i-k)*L,(Route[i].z*2-Min.z)/Max.z-.5,0);
    } glEnd();
    color( aqua,-0.2 ); glBegin( GL_LINE_STRIP );
    for( i=k; i<=l; i++ )
         glVertex3d( -Lp+(i-k)*L,(Mix[i].z*2-Min.z)/Max.z-0.5,0 ); glEnd();
    color( silver,-0.2 ); glBegin( GL_LINE_STRIP );
    for( i=k; i<=l; i++ )
         glVertex3d( -Lp+(i-k)*L,(Route[i].z*2-Min.z)/Max.z-0.5,0 ); glEnd();
    //
    //   подпись экстремальных величин на правом незатеняемом участочке
    //
    color( dimgray );
    Compass.Print( -1,3.6,"%3.2f",(Max.z+Min.z)/2 )
           .Print(  0,4.7,"·ζ]" )       // вертикальная качка вдоль аппликаты Z
           .Print( -1,5.8,"%3.2f",Min.z/2 );
    //
    // поверху накладываются графики с отсчётами углов бортовой и килевой качки
    //
    color( blue ); glBegin( GL_LINE_STRIP );
    for( i=k; i<=l; i++)glVertex3d(-Lp+(i-k)*L,(Head[i].y-Trim)/H,0); glEnd();
    Compass.Print( -1,Hx>Hy?2.2:1.1,DtoA( (Trim+Max.y)*_Rd,-1 ) );
    Compass.Print( -1,Hx<Hy?8.3:7.2,DtoA( (Trim+Min.y)*_Rd,-1 ) )
           .Print( -2.9,4.7,"·φ" );    //=ψ дифферент вокруг ординаты  по оси Y
    color( magenta ); glBegin( GL_LINE_STRIP );
    for( i=k; i<=l; i++)glVertex3d( -Lp+(i-k)*L,i90(Head[i].x)/H,0 ); glEnd();
    Compass.Print( -1,Hx>Hy?1.1:2.2,DtoA( Max.x*_Rd,-1 ) );
    Compass.Print( -1,Hx<Hy?7.2:8.3,DtoA( Min.x*_Rd,-1 ) )
           .Print( -5.2,4.7,"[ϑ" );      // крен относительно абсциссы на оси X
///
///     площадка графиков скорости хода, рыскания и трёх вертикальных ускорений
///     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Compass.AlfaVector( Compass.AlfaHeight()/0.85,1 );
    if( GMod )
    { Compass.Area( 29,-8,0,7 ).Activate( true );  // и ещё +7 строчек вверх
      color( green,0.25 ); liney( (Vector){ -Lp,1 },(Vector){ Lp-Lt,1 } );
      for( i=0; i*U<dT; i++ )
      { const Real mX = -Lp + i*U*L/Ts;         // новая шкала разметки времени
        liney( (Vector){ mX,1 },(Vector){ mX,i%(dT<60?5:6)?0.95:0.9 } );
    } }
    // и три ускорения свободного падения по наблюдениям в оконечностях корпуса
    //
   const Real D2=Ts*Ts*_g;
   Real A1=0,F1=0,A2=0,F2=0,A3,F3,A,F,
        maxA=eps,minA=-eps,maxM=eps,minM=-eps,maxF=eps,minF=-eps;
    glBegin( GL_QUAD_STRIP );
    for( i=k; i<=l; i++ )
    { U=Length*sin( Head[i].y )/2; // вертикальное смещение относительно миделя
      F3=Route[i].z-U;             // нос  -- аппликата ватерлинии на форштевне
      A3=Route[i].z+U;             // корма -- на ахтерштевне
      if( i-k>2 )                  // кривая лента носовых и кормовых ускорений
      { color(blue,0,.3); glVertex3d( -Lp+(i-k-1)*L,(F=(F1-F2-F2+F3)/D2)/2,0 );
        color(green,0,.3); glVertex3d(-Lp+(i-k-1)*L,(A=(A1-A2-A2+A3)/D2)/2,0 );
        if( maxA<A )maxA=A; else if( minA>A )minA=A;
        if( maxF<F )maxF=F; else if( minF>F )minF=F;
      } A1=A2,A2=A3,F1=F2,F2=F3;
    } glEnd();
    glLineWidth( 3 ); color( yellow ); glBegin( GL_LINE_STRIP ); A1=A2=0;
    for( i=k; i<=l; i++ )
    { A3=Route[i].z; H=(A1-A2-A2+A3)/D2;            // в районе мидельшпангоута
      if( i-k>2 ){ glVertex3d( -Lp+(i-k-1)*L,H/2,0 );
                   if( maxM<H )maxM=H; else if( minM>H )minM=H; } A1=A2,A2=A3;
    } glEnd();
    glLineWidth( 1 );
    Compass.AlfaVector( Compass.AlfaHeight()*0.85,1 );
    if( GMod )     // центральная подпись для ускорений в средней части корпуса
    { color( lightcyan,.75,.3 ); rectangle( (Vector){Lp-Lt,-1},(Vector){Lp,1} );
      color( blue );       Compass.Print( 0.0,4.5,"St)" );
      color( green );      Compass.Print(-3.8,4.5,"(As·" );
      color( yellow,-.6 ); Compass.Print(-1,3.4,"%+3.1f",maxM )
                                  .Print(-1,5.6,"%+3.1f",minM );
      color( yellow,-.4 );
      liney( (Vector){-Lp,0.5},(Vector){Lp-Lt,0.5} );
      line( (Vector){-Lp},(Vector){Lp-Lt},cyan );           // уровень среднего
    //
    //              график скорости хода и поступательной качки по ходу корабля
    //
      for( i=k; i<=l; i++ )                             // разбег скорости хода
      { r=Rate[i].x;
        if( i==k )Max.x=r+eps,Min.x=r-eps; else
        if( Max.x<r )Max.x=r; else if( Min.x>r )Min.x=r;
      } color( green );
      Compass.Print( -1,0.9,"%+3.1f",Max.x*3600/_Mile )
             .Print( -1,8,"v:%+3.1f",Min.x*3600/_Mile ); Max.x-=Min.x;
      glBegin( GL_LINE_STRIP );
      for( i=k;i<=l;i++ )glVertex3d(-Lp+(i-k)*L,2*(Rate[i].x-Min.x)/Max.x-1,0);
      glEnd();
    //
    //  рыскание на курсе по отсчетам углового положения направления курса по Z
    //
      for( i=k; i<=l; i++ )
      { r=angle( r=-Mix[i].y-Head[i].z );
        if( i==k )Max.y=r+eps,Min.y=r-eps; else
        if( Max.y<r )Max.y=r; else if( Min.y>r )Min.y=r;
      } color( blue );
      Compass.Print( -1,2,DtoA( Max.y*_Rd,-1 ) );
      Compass.Print( -1,7,"χ%s",DtoA( Min.y*_Rd,-1 ) );
      if( Max.y<-Min.y )Max.y=-Min.y; else if( Min.y>-Max.y )Min.y=-Max.y;
      glBegin( GL_LINE_STRIP ); Max.y-=Min.y;
      for( i=k; i<=l; i++ )
      glVertex3d(-Lp+(i-k)*L,2*(angle(r=-Mix[i].y-Head[i].z)-Min.y)/Max.y-1,0 );
      glEnd();
    }
    //                дополнительная разметка поля графиков с тремя ускорениями
    //
    color( gray );      Compass.Print( 1,1.2,"3g·{ " );            // максимумы
    color( blue );      Compass.Print( "%+0.1f,", maxA );          // по корме
    color( pink,-0.5 ); Compass.Print( "%+0.1f,", maxM );          // на миделе
    color( green );     Compass.Print( "%+0.1f }",maxF );          // по носу
    color( gray );      Compass.Text( _Center,-Lp,0,0,"+g" )
                               .Print( 1,-.25,"-g·{ " );
    color( blue );      Compass.Print( "%+0.1f,", minA );
    color( pink,-0.5 ); Compass.Print( "%+0.1f,", minM );
    color( green );     Compass.Print( "%+0.1f }",minF );
  } return *this;
}
//  Простая визуализация формы исходного корпуса в независимом графическом окне
//  с прорисовкой корпуса корабля после специальной настройки графической среды
//
bool Hull::Draw()                  // Виртуальная процедура простой перерисовки
{ static bool Recurse=false;
//if( !IsWindowVisible( hWnd ) || IsIconic( hWnd ) )return false;
  if( Recurse || !ShipName )return false; Recurse=true; Activate();
  glEnable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST );
  glClearColor( BkColor,1 );                  // голубоватый цвет фона и полная
  glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT ); // расчистка всего экрана
  glMatrixMode( GL_PROJECTION ); glLoadIdentity(); // сброс матричных координат
  gluPerspective( 15,Real(Width)/Height,-1e6,1e1 ); // к перспективной проекции
  gluLookAt( 0,0,Distance,lookX,lookY,lookZ,0,1,0 );    // позиция точки обзора
  glMatrixMode( GL_MODELVIEW ); glLoadIdentity();        //  исходное состояние
  glRotated( eyeY-90,1,0,0 );                            //   поставить на киль
  glRotated( eyeX+atan2(x.x,x.y)*_Rd,0,0,1 );            //            рыскание
  glRotated( eyeZ,0,1,0 );                               //           дифферент
//axis(*this,Length/1.8,Breadth/1.8,Draught,"X","Y","Z"); // Координатные оси и
  AlfaVector( Height/WinLines ); Drawing();               //   рисунок в памяти
  color( blue );
  Print( 2,1,"%s\n { %s }\n %s",sname( FileName ),ShipName,Model[Statum] );
  if( Statum>3 && Storm->Exp.wave )           // подводные волновые воздействия
    { color( green ); Print( lFlow?", увлечение волной":", над волной" ); }
  color( gray );  Print( ", сток/исток(%g) ",Kv );
  if( Pic.flow )
  { color( green ); Print(2,4,VView[Pic.flow] );
    color( cyan );  Print( " = набегающий поток + " );
    if( Pic.flow<3 ){ color( lightblue ); Print( "скользящий поток" );
      if( Pic.flow>1 )color( lightred ),Print( " + исток/сток непротекания " );
    } else color( lightmagenta ),Print( "вектор отражения (вихреисточник)" );
  }
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
