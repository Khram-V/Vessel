//
//      Блок графических процедур обеспечивающих весь комплекс геометрических
//      построений для моделирования механики движения корабля,
//      -- независимо существующая прорисовка корпуса корабля
//      -- удерживается на экране и активизируется мышкой и таймером
//
//                                                        ©2018-08-22 Иерусалим
//
#include "Aurora.h"       // объекты и производные операции с корпусом на волне
                          // + дополнения графической среды OpenGL-Window:Place
static int //Board=0,       // 'о' штевни; '-' левый и '+' правый борт
           Level=-2,        // -2-днище -1-вода 0-ватерлиния 1-смочен 2-сухой
           wLine=1;         // -1-ниже; +1-выше цвета конструктивной ватерлинии
//atic Flex wL;             // изменчивые отрезки фрагментов ватерлинии
                            // по уровню воды в шпациях левого/правого борта
static bool drawHull=false, // прорисовка корпуса | гидродинамический процесс
            Part=false;     // false= днище и ватерлиния; true= надводный борт
//static Real ArLen=0.1;    // относительная длина для стрелок на шпациях
//Vertex::Vertex( _Vector a )  // конструктор и собственно
//{ w=Storm->Value( Point::operator=(Vessel->out( Vector::operator=( a ) )) ); }

void Hull::drawTriangle(_Vector a,_Vector b,_Vector c ) // отработка трёх точек
{ if( !drawHull )Three( Level,a,b,c );    // единожды производится динамический
  else                                    //  перерасчет характеристик обводов
  { const byte Mode=Pic.hull;             //  для полупрозрачной картинки нужна
    if( !Part && Level>0 || Part && Level<=0 || !Level && Mode>2 )return;
//  int wLine = a.z>0 && b.z>0 && c.z>0 ? 1:-1;   // двухэтапная перепрорисовка
    color( !Level?lightblue               // поверхность действующей ватерлинии
         : (wLine<0?green:freeboard),     // подводные обводы и надводный борт
         ( abs( Level )<2?0.75:1.0 )*( Level>0&&wLine<0?0.25:      // затенение
                                       Level<0&&wLine>0?-0.15:0.0 ), //    воды
               !Level ? (Mode&&Mode<3?0.7:0.2 ) :        // прозрачность борта
                 ( Level>0 ? ( Mode>2?0.8:0.2 ) :        // настраивается по
                             ( Mode>1?0.9:0.2 ) ) );     // режиму визуализации
   Vector A=out( a ),B=out( b ),C=out( c ),N=(C-A)*(B-A); glNormal3dv( N );
    glBegin( Pic.grid?GL_LINE_LOOP:GL_TRIANGLES );      // контуры или покрытия
      glVertex3dv( A ),glVertex3dv( B ),glVertex3dv( C );
    glEnd();
/*
#define _(P) (*((Vector*)(&P)))                    // это стрелки ВЛ-нормалей
  if( !Level ){ Vector W=(A+B+C)/3.0; arrow( W,W+dir((A-B)*(A-C))*5,1,red ); }
*/
    // здесь отладочная визуализация скоростей на элементарных треугольниках
    //  ~ 1 - касательные; 2 + нормальные -> вихреисточники
    //    3 - отражённые                  => излучение волн
    //
    if( Level<0 && Pic.flow ){ Vector V,// скорости отмеряются только под водой
        M=( A+B+C )/3.0,m=in( M ),// (a+b+c)/3 центр элементарного треугольника
        n=dir( N ),Vn,Vs;
      if( Statum>3 && Storm->Exp.wave )
      { Storm->WaveV( Storm->Trun,M,V ); } else V=0; // скорость в потоке волны
      V += (Route[-2]-Route[-1])/Ts+LtA(m*Whirl[-1]); // полный ход с вращением
         //=+= LtA( -Rate[-1] ) == случай расчётного ходовый набегающего потока
      Vs=n*( V*n );              // вектор вихревого слоя от обтекающего потока
      Vn=-n*( V%n );             // вектор отраженного (-) по нормали импульса
      glLineWidth(0); arrow(M,M+V,0.1,lightcyan); // суммарный набегающий поток
      glLineWidth(1);                    // вихреисточники -- векторы скоростей
      if( Pic.flow<3 ){ arrow( M,M+Vs,0.1,blue );          // касательных
        if( Pic.flow>1 )arrow( M,M+Vn,0.1,red );          //  и нормальных
      } else            arrow( M,M+Vn+Vs,0.1,magenta );  // + вектор отражения
  } }
}
//!  сборка сортировкой двух фрагментов ватерлинии в интервале одной шпации
//           (здесь надо найти локализованное решение по выбору ориентации)
//
void Hull::waterPoints( _Vector N,_Vector Q,_Vector P )
{ //if( Q==P )return;
  wL+=N; /// dir( N )??
  if( LtA( N*(P-Q) ).z>=0 ){ wL+=Q; wL+=P; } else { wL+=P; wL+=Q; }
}
void Hull::divideTriangle
( _Vector T,_Real t, _Vector R,_Real r, _Vector L,_Real l )
{ _Vector rR=(Vector)T+(t/(t-r))*(R-T),       // правая точка пересечения ребра
          lL=(Vector)T+(t/(t-l))*(L-T); Level=t>=0?-1:1;// треугольника и левая
  if( rR!=lL )waterPoints( (lL-T)*(T-rR),lL,rR ),       //   +++
              drawTriangle( T,rR,lL );
  if( !l && !r )return;                 Level=t<0?-1:1;
  if( !l )drawTriangle( L,rR,R ); else
  if( !r )drawTriangle( R,L,lL ); else
  if( fabs( r )>fabs( l ) )drawTriangle( R,L,lL ),drawTriangle( R,lL,rR );
                      else drawTriangle( L,lL,rR ),drawTriangle( L,rR,R );
}
void Hull::Triangle( Vector a,Vector b,Vector c )     // обработка треугольника
{ if( a.y || b.y || c.y )            // ~~ далее точки на базисе открытого моря
//if( a!=b && a!=c && b!=c  )
  { Vector A=out( a ),B=out( b ),C=out( c );
    Real aZ=Storm->Value( A )-A.z,   // (+)погружение (-)борт над водой = метка
         bZ=Storm->Value( B )-B.z,   // обшивке под/над действующей ватерлинией
         cZ=Storm->Value( C )-C.z;   // WL на подъем или спуск?
//  Real aZ=a.w-a.Z,                 // (+)погружение (-)борт над водой = метка
//       bZ=b.w-b.Z,                 // обшивке под/над действующей ватерлинией
//       cZ=c.w-c.Z;                 // WL на подъем или спуск?
//  if( aZ==0.0 && bZ==0.0 && cZ==0.0 )return;       // значит будут повторения
//  wLine = a.z>=0.0 && b.z>=0.0 && c.z>=0.0 ? 1:-1; //  действующая ватерлиния
    wLine = a.z+b.z+c.z>=0 ? 1:-1; // здесь пересечений ватерлинии не ожидается
    Level = -2; //wLine*2; //-2;
    if( !aZ && !bZ ){ if(cZ>0)waterPoints((b-c)*(c-a),b,a); else Level=2; }else
    if( !bZ && !cZ ){ if(aZ>0)waterPoints((c-a)*(a-b),c,b); else Level=2; }else
    if( !cZ && !aZ ){ if(bZ>0)waterPoints((a-b)*(b-c),a,c); else Level=2; }else
    if( aZ<=0&&bZ<=0&&cZ<=0 )Level=2; else   // треугольник целиком над волной
    if( aZ>=0&&bZ>=0&&cZ>=0 )Level=-2; else  // треугольник полностью под водой
    { Real ab=aZ*bZ,bc=bZ*cZ,ca=cZ*aZ;       // иначе рассечение по ватерлинии
      if( ab<0&&ca<=0){divideTriangle(a,aZ,b,bZ,c,cZ); return;}// выбор вершины
      if( bc<0&&ab<=0){divideTriangle(b,bZ,c,cZ,a,aZ); return;}// треугольника
      if( ca<0&&bc<=0){divideTriangle(c,cZ,a,aZ,b,bZ); return;}// для деления
    } drawTriangle( a,b,c );
} }
//    Кинематическая постановка корпуса корабля на объединенное волновое поле
//
Hull& Hull::Floating( bool onlyDraw )
{ // работа с треугольниками обшивки корпуса и фрагментами ватерлинии в шпациях
  // ~~   троекратная дорисовка корпуса по уровням относительно ватерлинии
  // ~~       обусловливается последовательностью наложения прозрачности
 Vector P,Q,R;
 int i,k; Part=false;             // разделение корпуса на прозрачные подуровни
        drawHull=onlyDraw;        // копия режима расчетов(-) или прорисовки(+)
  if( !onlyDraw )ThreeInitial();  // начальная чистка для интегрируемых величин
//else if( !IsWindowVisible( hWnd ) || IsIconic( hWnd ) )return *this;
  wL.len=0;      // ватерлиния с нормалями и запутанными разделёнными отрезками
Part_of_hull:    // разделение корпуса по уровням надводной и смоченной обшивки
  //                    с разрешением проблем полупрозрачности бортовой обшивки
  //  ...транцы на штевнях должны отрабатываться вогнутыми контурами... !!!
  //
  if( onlyDraw )glEnable( GL_LIGHTING );
  for( k=0; k<=1; k++ ){ Flex &S=k?Stem:Stern;
    for( i=0; i<S.len; i++ ){ Q=S[i];
      if( k )Q.y=-Q.y;
      if( i>0 ){ if( P.y )Triangle( P,~P,~Q );
                 if( Q.y )Triangle( Q,P,~Q ); } P=Q;
  } }
  //   собственно цикл покрытия оболочки бортовой обшивки
  //   по шпациям между теоретическими шпангоутами корпуса
  //
  for( k=0; k<=Nframes; k++ )  // штевни и шпангоуты Nframes
  if( Shell[k] )              // -- есть ли сам корпус, не пропущена ли шпация?
  if( Shell[k][0]>=3 )       // в шпации присутствует хотя бы один треугольник?
  { for( int Board=-1; Board<2; Board+=2 )
    { P=Select( k,Board>0?-1:1 );            // это блок стандартных шпаций
      Q=Select( k,Board>0?-2:2 );            // сначала левый-> правый шпангоут
      for( i=3; i<=Shell[k][0]; i++ )        // всех теоретических шпангоутов
      { R=Select( k,Board>0?-i:i );          // попутная разборка треугольников
        if( Board>0 )Triangle( P,Q,R );      // правый борт и левый к кормовому
                else Triangle( Q,P,R );      //                  перпендикуляру
        if( Shell[k][i]&LeftFrame )P=R; else Q=R;
    } }
  }
  if( onlyDraw )glDisable( GL_LIGHTING );
  //! Ватерлиния выведена из расчетного блока по форме и объему обводов корпуса
              // без ватерлинии будет теоретический центр => ноль на ватерлинии
  if( !Part ) //   теоретическая и действующая ватерлиния готовятся с нормалями
  { static Flex W; Vector wM,fM; Real l,L; bool C; const Real dw=1e-4; // 0.1мм
    if( !Storm->Kt )             // конструктивная или теоретическая ватерлиния
      for( WaterLine.len=i=0; i<wL.len; i++ )WaterLine+=wL[i]; Level=0;
    while( wL.len )
    { W.len=0; C=false;
      W+=wL[-2],W+=wL[-1]; i=(wL.len-=3)-3;
      while( !C && i>=0 )
      { if( abs( W[0]-wL[i+1] )<dw )W/=wL[i+2]; else
        if( abs( W[0]-wL[i+2] )<dw )W/=wL[i+1]; else
        if( abs( W[-1]-wL[i+1] )<dw )W+=wL[i+2]; else
        if( abs( W[-1]-wL[i+2] )<dw )W+=wL[i+1]; else { i-=3; continue; }
        if( onlyDraw )                  // стрелочки вдоль и поперёк ватерлинии
        { const Real aL=Draught/12;
          Vector q=(wL[i+1]+wL[i+2])/2;
          arrow( out( wL[i+1] ),out( wL[i+2] ),0.1,navy );
          arrow( out( q ),out( q+aL*dir( wL[i] ) ),0.2,gray );
        }
        wL.Delete( i ),wL.Delete( i ),wL.Delete( i );
        C = W.len>2 && abs( W[0]-W[-1] )<dw; if( C )break;  i=wL.len-3;
      }

//Break( "~выборка wL=%d(%d) W=%d C=%d i=%d(%d)",wL.len,wL.len/3,W.len,C,i,i/3 );

      if( !C )W+=W[0];;
      L=0.0; wM=0.0; fM=0.0;    // co разделением разрывных контуров ватерлинии
      for( i=0; i<W.len-1; i++ )
        { L+=(l=abs( W[i]-W[i+1] )); wM+=l*( W[i]+W[i+1] ); } // длина контура
      if( L>eps )wM/=2*L;                                   // центр ватерлинии
      for( i=0; i<W.len-1; i++ )fM+=(W[i]-wM)*(W[i+1]-wM); // площадь для знака
      if( Zenit%( LtA( fM ) )<0.0 )           // ориентация по вертикали
          for( i=0; i<W.len-1; i++ )drawTriangle( W[i+1],W[i],wM ); else
          for( i=0; i<W.len-1; i++ )drawTriangle( W[i],W[i+1],wM );
    }
//    else
//    { wM=0.0; L=0.0;          // без слияния контуров ватерлинии - простенько
//      for( i=0; i<wL.len; i+=3 )
//      { l=abs( wL[i+2]-wL[i+1] ); L+=l; wM += 0.5*l*( wL[i+1]+wL[i+2] ); }
//      if( L>eps )wM/=L;
//      for( i=0; i<wL.len; i+=3 )drawTriangle( wL[i+1],wL[i+2],wM );
//    }
    // завершение геометрической графики просто белая конструктивная ватерлиния
    //
    if( onlyDraw )
    { color( !Trim?white:cyan ); glLineWidth( 5 ); // silver
      for( i=0; i<WaterLine.len; i+=3 )
         line( out( WaterLine[i+1] ),out( WaterLine[i+2] ) ); glLineWidth( 1 );
    }
    // действующая ватерлиния выстраивается из фрагментов пересечения
    // треугольников в шпациях, с нормалями и стрелками вперед по курсу корабля
/*
#define Wline( L )for( i=0; i<L.len-2; i+=2 ){ q=( (L[i]+L[i+2])*0.5 );   \
 arrow(out(L[i]),out(L[i+2]),ArLen*.67),arrow(out(q),out(q+L[i+1]),ArLen*.5); }
      color( lightblue,DrawMode&3?0.0:0.3 ); Wline( wR ) Wline( wL )
*/  //
    //! собственно блок моделирования отражения потоков/волн от корпуса корабля
/** else
    if( Storm->Exp.wave>1 )      // отражение локальных скоростей от ватерлинии
    { //for( i=0; i<wR.len-2; i+=2 )Storm->Slicks( out(wR[i]),out(wR[i+2]),x ); // dir( (wR[i+1]+wR[i+3])*0.5 ) );
      //for( i=0; i<wL.len-2; i+=2 )Storm->Slicks( out(wL[i]),out(wL[i+2]),x ); // dir( (wL[i+1]+wL[i+3])*0.5 ) );
      for( i=0; i<WaterLine.len; i+=3 )
         Storm->Slicks( out(WaterLine[i+1]),out(WaterLine[i+2]),x );

    }
*/
    Part=true;                       // однократный возврат к перерисовке
    if( onlyDraw )goto Part_of_hull; // только надводного борта, здесь Course=x
  }
  //
  // выборка и расчёт обновленных параметров корпуса, увеличение счетчика цикла
  //
  if( !onlyDraw )ThreeFixed(); drawHull=false; return *this;
}
/*    Здесь ведется прорисовка всего графического окружения для корпуса,
      затем выполняется его одноразовая прорисовка без перерасчетов его текущих
      геометрических параметров и векторов/тензоров движения
           DrawMode: 0 - ватерлиния строится при любом графическом раскладе
                     1 - прорисовываются только собственно штевни и шпангоуты
                     2 - оставляется подводная часть со шпангоутами над водой
                     3 - весь корпус прорисовывается целиком отчасти прозрачным
                   х04 - тоже, без обшивки и только с триангуляционными ребрами
                   х08 - исходный вариант: одно поле графиков и картушка справа
*/
Hull& Hull::Drawing( byte type )  // 0 - DrawMode; 1 - корпус; 2 + профили волн
{ //const Real ArLen=Length/132;  // относительная длина для стрелок на шпациях
  //if( !Shell )return *this;   // glTranslated( -Frame[Mid][0].X,0,-Draught )
  if( type!=1 )Contour_Lines(); // габариты и вертикальные профили пакетов волн
  //
  //  исходные и действующие центры и плечи гидростатических сил и моментов
  //
 Vector P,Q,S,W,C=out( vB ),F=out( vF ),M=C,K=F; M.z=vM.x;  // метацентр
 colors c = vM.z>=hX ? green : ( vM.z<0 ? red:yellow );     // K=out( vC ),
  //
  //  оси корабельных координат - векторы локального базиса от центра величины
  //
// Vector R=Buoyancy; R.z=vB.z;
// Point O=out( R );
  if( iV<Volume/36 )K=out( Gravity ); else    // на вылете - центр тяжести
  if( iF<Floatage/36  )K=out( Buoyancy );     // на погружение - центр величины
                                              // либо центр площади ватерлинии
  Text( _Up,arrow( K-2*Draught*z,K+3*Draught*z,0.01,blue ),"z" );
  Text( _Up,arrow( K+Breadth*y,  K-Breadth*y,  0.01 ),     "y" );
  Text( _Up,arrow( K-0.6*Length*x,K+0.6*Length*x,0.01 ),   "x" );
                  // белый центр гидродинамических пар сил и реакций - моментов
//arrow( spot( K,12,blue ),spot( out( vD ),36,maroon ),ArLen ); /// 12,white
//arrow( spot( K,24,white ),spot( out( vD ),24,lightmagenta ),ArLen/3 );
//line( line( C,spot( out( dV ),24,yellow ),green ),F ); // срединная точка
  //
  //  четырёхугольник исходных центров площади ватерлинии и величины,
  //                                 с центром тяжести и метацентром
 GLboolean CF;
  glGetBooleanv( GL_CULL_FACE,&CF );
  if( CF )glDisable( GL_CULL_FACE );   // режима отбора треугольников выключен
  glBegin( GL_POLYGON );
    dot( P=out( Buoyancy),lightblue ); // исходный центр величины тихой воды
    dot( Q=out( Gravity ),gray );      // центр тяжести после загрузки в порту
    S=Q; S.z+=hX; dot( S,c );          // начальный метацентр по тихой воде
    dot( W=out( Floatable ),cyan );    // исходный центр площади ватерлинии
//  dot( K,white );                    // центр динамической реакции корабля
  glEnd();
  if( CF )glEnable( GL_CULL_FACE );    // включение режима отбора треугольников
  //
  //  разметка подвижных гидростатических центров
  //
  glLineWidth( 2 );
  Text( _Right,spot( Q,18,gray ),"G " );   // центр гравитационной тяжести
  Text( _Left, spot( F,12,cyan ),"F " );   // текущий центр площади ватерлинии
  if( W!=F )arrow( spot( W,18 ),F,0.1 );   //   динамика ватерлинии

  Text( _Down,spot( C,12,blue ),"C" );     // действующий центр величины
  if( C!=P )arrow( spot( P,18 ),C,0.1 );   //   динамика центра величины
  Text( _Up,spot( out( vC ),18,black ),"R" );          // динамический центр
  Text( _Down,spot( out( vP ),36,lightmagenta ),"P" ); // центр давлений (ЦБС)
//            spot( out( vP ),24,lightmagenta );

  Text( _Up,spot( M,12,c ),"m " );         // действующий метацентр
  if( S!=M )arrow( spot( S,18 ),M,0.1 );   //  кинематика метацентра тихой воды
  glLineWidth( 1 ); line( C,M );           // метацентрический радиус-вертикаль
  //
  //  маршрут корабля по поверхности взволнованного моря
  //
 int i,j,k; color( blue,0,0.5 );
  for( i=Route.len-1; i>0; i-- )line( Route[i]-Locate,Route[i-1]-Locate ),
                                spot( Route[i]-Locate,5 );
  //
  //   рисуем и подписываем шпангоуты, как есть ...
  //                         -.1
#define L1( A,B,C ){ color(C,-.25),line(out(A),out(B)),line(out(~A),out(~B)); }
#define L2(_A,_B,C){ Vector &A=_A,&B=_B; if( A.z!=B.z||A.y&&B.y )L1( A,B,C ) }

 byte Mode=Pic.hull;                       // собственно 4 режима прорисовки
//if( Mode>1 )glDisable( GL_LINE_SMOOTH ); // без сглаживание линий, и зачем?
  if( Mode && Mode!=3 )                    // режимы рисования только для 1 и 2
  for( j=k=0; k<=Nframes+1; k++ )
  { for( i=0; i<Frame[k].len-1; i++ )
      L2( Frame[k][i],Frame[k][i+1],Frame[k][i].z<0 ? green:freeboard )
    if( k>0 )
    { if( Stem.len==0 || Stern.len==0 )        // без форштевня или ахтерштевня
        L2( Frame[k][0],Frame[k-1][0],green ) else // рисуем раскрытие днища
      if( Frame[k][0].x<=Stem[0].x && Frame[k-1][0].x>=Stern[0].x )
        L2( Frame[k][0],Frame[k-1][0],green )    // иначе днище между штевнями
      L2( Frame[k][i],Frame[k-1][j],lightblue ) //  и раскрытие ширстрека
    } j=i;                                     // вершина предыдущего шпангоута
  }
  if( Mode ) // теперь прорисовка штевней с оконтуриванием транцевых расширений
  { glLineWidth( 2 );
    for( k=0; k<Stern.len-1; k++ )
     L1( Stern[k],Stern[k+1],Stern[k].z<0 ? green:freeboard )
    for( k=0; k<Stem.len-1; k++ )
     L1( Stem[k+1],Stem[k],Stem[k].z<0 ? green:freeboard )
    glLineWidth( 1 );
  }
//if( Mode>1 )glEnable( GL_LINE_SMOOTH );   // восстановление сглаживания линий
                    // включение одноразовой прорисовки корпуса вместо расчетов
  if( type )Pic.hull|=3; Floating( true );  // изображение корпуса только здесь
  if( type )Pic.hull=Mode;                  //    -- с 3D-обшивкой
  if( !type )NavigaInform( this );          // навигационная информация
  return *this;                             //     о текущем состоянии корабля
}

// static Real Vm=1,Vi=0; // Масштаб скорости и отсчет среднеквадратичной суммы
