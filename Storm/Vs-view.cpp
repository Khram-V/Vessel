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
//atic Real ArLen=0.1;      // относительная длина для стрелок на шпациях

Vertex::Vertex( _Vector _a_ ) // { *this=V; }  // конструктор и собственно
 { w=Storm->Value( Point::operator=(Vessel->out( Vector::operator=(_a_) )) ); }
Vertex& Vertex::operator = ( _Vector _a_ ){ return *this = _a_; }

void Hull::drawTriangle(_Vertex a,_Vertex b,_Vertex c ) // отработка трёх точек
{ if( !drawHull )Three( Level,a,b,c );    // единожды производится динамический
  else                                    //  перерасчет характеристик обводов
  { const byte Mode=DrawMode&3;           //  для полупрозрачной картинки нужна
    if( !Part && Level>0 || Part && Level<=0 || !Level && Mode>2 )return;
//  int wLine = a.z>0 && b.z>0 && c.z>0 ? 1:-1;   // двухэтапная перепрорисовка
    color( !Level?lightblue               // поверхность действующей ватерлинии
           : (wLine<0?green:dimgray),     // подводные обводы и надводный борт
         ( abs( Level )<2?0.75:1.0 )*( Level>0&&wLine<0?-0.25:     // затенение
                                       Level<0&&wLine>0?-0.15:0.0 ), //    воды
               !Level ? (Mode&&Mode<3?0.7:0.2 ) :        // прозрачность борта
                 ( Level>0 ? ( Mode>2?0.8:0.2 ) :        // настраивается по
                             ( Mode>1?0.9:0.2 ) ) );     // режиму визуализации
   const Point &A=(Point)a,&B=(Point)b,&C=(Point)c; glNormal3dv( (B-A)*(C-A) );
    glBegin( DrawMode&4?GL_LINE_LOOP:GL_TRIANGLES );    // контуры или покрытия
    glVertex3dv( (Real*)( &A ) ),                       // dir ( ?? )
    glVertex3dv( (Real*)( &B ) ),
    glVertex3dv( (Real*)( &C ) ); glEnd();
#if 0  // здесь отладочная визуализация скоростей на элементарных треугольниках
#define _(P) (*((Vector*)(&P)))                      // это стрелки ВЛ-нормалей
/*if( !Level )
{ Vector W=( _(A)+_(B)+_(C) )/3.0; arrow( W,W+dir( (A-B)*(A-C) )*5,1,red );
}*/
if( Level<0 )
{ Vector m=( a+b+c )/3.0,M=Tensor(*this)*m; // _(A)+_(B)+_(C) )/3.0,
  Vector V=(Route[-1]-Route[-2])/(Storm->Tstep/Storm->tKrat);
  Vector W=Whirl[-1],
         n=(a-b)*(a-c),N=Tensor(*this)*n;
// arrow( M,M+V,0.2,green );               // скорость хода - набегающий поток
// arrow( M,M+dir( N ),0.2,red );          // единичные нормали к треугольникам
   arrow( M,M+dir(N)*(V*dir(N)),0.2,blue ); // векторы касательных скоростей
}
#undef _
#endif
  }
}
//!  сборка сортировкой двух фрагментов ватерлинии в интервале одной шпации
//           (здесь надо найти локализованное решение по выбору ориентации)
//
void Hull::waterPoints( _Vector N,_Vector Q,_Vector P ){ wL+=N;
  if( (Tensor(*this)*(N*(P-Q))).z>=0 ){ wL+=Q; wL+=P; } else { wL+=P; wL+=Q; }
}
void Hull::divideTriangle
( _Vertex T,_Real t, _Vertex R,_Real r, _Vertex L,_Real l )
{ _Vertex rR=(Vector)T+(R-T)*(t/(t-r)),       // правая точка пересечения ребра
          lL=(Vector)T+(L-T)*(t/(t-l)); Level=t>=0?-1:1;// треугольника и левая
  if( rR!=lL )waterPoints( dir( (lL-T)*(T-rR) ),lL,rR ); // +++
          drawTriangle( T,rR,lL );
  if( !l && !r )return;                 Level=t<0?-1:1;
  if( !l )drawTriangle( L,rR,R ); else
  if( !r )drawTriangle( R,L,lL ); else
  if( fabs( r )>fabs( l ) )drawTriangle( R,L,lL ),drawTriangle( R,lL,rR );
                      else drawTriangle( L,lL,rR ),drawTriangle( L,rR,R );
}
void Hull::Triangle( Vertex a,Vertex b,Vertex c )     // обработка треугольника
{ if( a.y || b.y || c.y )            // ~~ далее точки на базисе открытого моря
  { Real aZ=a.w-a.Z,                 // (+)погружение (-)борт над водой = метка
         bZ=b.w-b.Z,                 // обшивке под/над действующей ватерлинией
         cZ=c.w-c.Z;                 // WL на подъем или спуск?
//  if( aZ==0.0 && bZ==0.0 && cZ==0.0 )return;       // значит будут повторения
//  wLine = a.z>=0.0 && b.z>=0.0 && c.z>=0.0 ? 1:-1; //  действующая ватерлиния
    wLine = a.z+b.z+c.z>=0 ? 1:-1; // здесь пересечений ватерлинии не ожидается
    Level = -2; //wLine*2; //-2;
    if( aZ==0&&bZ==0 ){ if( cZ>0 )waterPoints( dir((b-c)*(c-a)),b,a ); else Level=2; } else
    if( bZ==0&&cZ==0 ){ if( aZ>0 )waterPoints( dir((c-a)*(a-b)),c,b ); else Level=2; } else
    if( cZ==0&&aZ==0 ){ if( bZ>0 )waterPoints( dir((a-b)*(b-c)),a,c ); else Level=2; } else
    if( aZ<=0&&bZ<=0&&cZ<=0 )Level=2; else   // треугольник целиком над волной
    if( aZ>=0&&bZ>=0&&cZ>=0 )Level=-2; else  // треугольник полностью под водой
    { Real ab=aZ*bZ,bc=bZ*cZ,ca=cZ*aZ;       // иначе рассечение по ватерлинии
      if( ab<0&&ca<=0 ){ divideTriangle( a,aZ,b,bZ,c,cZ ); return; } // выбор вершины
      if( bc<0&&ab<=0 ){ divideTriangle( b,bZ,c,cZ,a,aZ ); return; } // треугольника
      if( ca<0&&bc<=0 ){ divideTriangle( c,cZ,a,aZ,b,bZ ); return; } // для деления
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
  wL.len=0;      // ватерлиния с нормалями и запутанными разделёнными отрезками
Part_of_hull:    // разделение корпуса по уровням надводной и смоченной обшивки
  //
  //  ...транцы на штевнях должны отрабатываться вогнутыми контурами... !!!
  //
  for( k=0; k<=1; k++ ){ Flex &S=k?Stem:Stern;
    for( i=0; i<S.len; i++ ){ Q=S[i];
      if( k )Q.y=-Q.y;
      if( i>0 ){ if( P.y )Triangle( P,~P,~Q );
                 if( Q.y )Triangle( Q,P,~Q ); } P=Q;
  } }
  //   собственно цикл покрытия оболочки бортовой обшивки
  //   по шпациям между теоретическими шпангоутами корпуса
  //
  for( k=0; k<=Nframes; k++ )                    // штевни и шпангоуты Nframes
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
  } } }
  //! Ватерлиния выведена из расчетного блока по форме и объему обводов корпуса
              // без ватерлинии будет теоретический центр => ноль на ватерлинии
  if( !Part ) //   теоретическая и действующая ватерлиния готовятся с нормалями
  { Vector wM={ 0,0,0 }; Real l,L=0.0; Level=0;
    for( i=0; i<wL.len; i+=3 )
      { l=abs( wL[i+2]-wL[i+1] ); L+=l; wM += 0.5*l*( wL[i+1]+wL[i+2] ); }
    if( L>eps )wM/=L;
    for( i=0; i<wL.len; i+=3 )drawTriangle( wL[i+1],wL[i+2],wM );
    if( !Storm->Kt )             // конструктивная или теоретическая ватерлиния
      for( WaterLine.len=i=0; i<wL.len; i++ )WaterLine += wL[i];
    //
    // завершение геометрической графики просто белая конструктивная ватерлиния
    //
    if( onlyDraw ){ color( !Trim?silver:cyan ); glLineWidth( 5 );
     const Real aL=Breadth/64;
      for( i=0; i<WaterLine.len; i+=3 )
         line( out( WaterLine[i+1] ),out( WaterLine[i+2] ) ); glLineWidth( 1 );
      for( i=0; i<wL.len; i+=3 ){ Vector q=(wL[i+1]+wL[i+2])/2;
         arrow( out( wL[i+1] ),out( wL[i+2] ),aL,navy );
         arrow( out( q ),out( q+wL[i]*(aL*3) ),aL,gray );
      }
    }
    // действующая ватерлиния выстраивается из фрагментов пересечения
    // треугольников в шпациях, с нормалями и стрелками вперед по курсу корабля
/*
#define Wline( L )for( i=0; i<L.len-2; i+=2 ){ q=( (L[i]+L[i+2])*0.5 );   \
 arrow(out(L[i]),out(L[i+2]),ArLen*.67),arrow(out(q),out(q+L[i+1]),ArLen*.5); }
      color( lightblue,DrawMode&3?0.0:0.3 ); Wline( wR ) Wline( wL )
*/
    //
    //! собственно блок моделирования отражения потоков/волн от корпуса корабля
/*  else
    if( Storm->Exp.wave>1 )      // отражение локальных скоростей от ватерлинии
    { for( i=0; i<wR.len-2; i+=2 )Storm->Slicks( out(wR[i]),out(wR[i+2]),x ); // dir( (wR[i+1]+wR[i+3])*0.5 ) );
      for( i=0; i<wL.len-2; i+=2 )Storm->Slicks( out(wL[i]),out(wL[i+2]),x ); // dir( (wL[i+1]+wL[i+3])*0.5 ) );
    }
*/  Part=true;                       // однократный возврат к перерисовке
    if( onlyDraw )goto Part_of_hull; // только надводного борта, здесь Course=x
  }
  //
  // выборка и расчёт обновленных параметров корпуса, увеличение счетчика цикла
  //
  if( !onlyDraw )ThreeFixed(); drawHull=false; return *this;
}
//  Здесь ведется прорисовка всего графического окружения для корпуса,
//  затем выполняется его одноразовая прорисовка без перерасчетов его текущих
//  геометрических параметров и векторов/тензоров движения
//               DrawMode: 0 - ватерлиния строится при любом раскладе
                      //   1 - только собственно штевни и шпангоуты
                      //   2 - подводная часть со шпангоутами над водой
                      //   3 - весь корпус прорисовывается целиком
                      // х04 - всё тоже, но только с триангуляционными ребрами
Hull& Hull::Drawing( byte type )  // 0 - DrawMode; 1 - корпус; 2 + профили волн
{ const Real ArLen=Length/132.0;  // относительная длина для стрелок на шпациях
  //if( !Shell )return *this;   // glTranslated( -Frame[Mid][0].X,0,-Draught )
  if( type!=1 )Contour_Lines(); // габариты и вертикальные профили пакетов волн
  //
  //  исходные и действующие центры и плечи гидростатических сил и моментов
  //
 Point P=out( Buoyancy ),Q=out( Gravity ),      // начальной величины и тяжести
       C=out( vB ),F=out( vF ),M=F,I=out(Zero), // центр объёма, ВЛ и метацентр
       W=out( Floatable ),S=W; M.Z=I.Z+vM.x;    // исходные центры ВЛ и мета.
 Point K=out( vC );       S.Z=I.Z+Metacenter.x;
///ctor R = Gravity-vB;   гидростатическое плечо
 colors c = vM.z>=hX ? green : ( vM.z<0 ? red:yellow );
  //
  //  оси корабельных координат - векторы локального базиса от центра величины
  //
// Vector R=Buoyancy; R.z=vB.z;
// Point O=out( R );
  Text( _Up,arrow( K-z*Draught*2, K+z*Draught*3,ArLen,blue ),"z" );
  Text( _Up,arrow( K+Breadth*y,   K-y*Breadth,  ArLen ),     "y" );
  Text( _Up,arrow( K+x*Length*-.6,K+x*Length*.6,ArLen ),     "x" );
  //
  //  метацентрический флажок из гидростатических центров
  //
 GLboolean CF;
  glGetBooleanv( GL_CULL_FACE,&CF );
  if( CF )glDisable( GL_CULL_FACE );   // режима отбора треугольников выключен
  glBegin( GL_POLYGON );
    color(  cyan,0,0.8 ); dot( F );    // текущий центр действующий ватерлинии
    color(  blue,0,0.8 ); dot( C );    // действующий центр подводного объёма
    color( black,0,0.8 ); dot( Q );    // точка гравитационного центра тяжести
    color(     c,0,0.8 ); dot( M );    // мгновенный быстро подвижный метацентр
  glEnd();
  if( CF )glEnable( GL_CULL_FACE );    // включение режима отбора треугольников
  //
  //  треугольник исходных центров площади ватерлинии и величины,
  //                             с центром тяжести и метацентром
  glLineWidth( 5 ); glBegin( GL_LINE_LOOP );
    dot( W,cyan );                      // исходный центр площади ватерлинии
    dot( P,blue );                      // исходный центр величины тихой воды
    dot( Q,black );                     // центр тяжести после загрузке в порту
    dot( S,c );                         // начальный метацентр по тихой воде
  glEnd();
  //
  //  направление действия архимедовых сил всплытия
  //
  //  arrow( O,O + 2*Breadth*dir( vW ),1,red );
  //  arrow( O,O + 2*Breadth*(Tensor(*this)*(dir( vW ))),1,red );
  //
  //  разметка гидростатических центров
  //
  glLineWidth( 3 );
  Text( _Left,spot( Q,18,navy ),"G " );    // центр гравитационной тяжести
  Text( _Left,spot( F,18,cyan ),"F " );    // текущий центр площади ватерлинии
  if( W!=F )arrow( spot( W,12 ),F,ArLen ); // действующий центр величины
  Text( _Down,spot( C,18,blue ),"C" );     // действующий центр величины
  if( C!=P )arrow( spot( P,12 ),C,ArLen ); // исходный центр величины корпуса
  Text( _Up,spot( M,18,c ),"M " );         // действующий метацентр
  if( S!=M )arrow( spot( S,12 ),M,ArLen ); // начальный метацентр - тихой воды
  glLineWidth( 1 ); arrow( C,M,ArLen );    // метацентрический радиус вертикаль
  arrow( arrow( Q,spot( out( vD ),8,green ),ArLen ),F,ArLen );
  spot( out( vC ),24,gray );               // центр гидродинамических пар сил
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

 byte Mode=DrawMode&3;                     // собственно 4 режима прорисовки
//if( Mode>1 )glDisable( GL_LINE_SMOOTH ); // без сглаживание линий, и зачем?
  if( Mode && Mode!=3 )                    // режимы рисования только для 1 и 2
  for( j=k=0; k<=Nframes+1; k++ )
  { for( i=0; i<Frame[k].len-1; i++ )
      L2( Frame[k][i],Frame[k][i+1],Frame[k][i].z<0 ? green:dimgray )
    if( k>0 )
    { if( Stem.len==0 || Stern.len==0 )        // без форштевня или ахтерштевня
        L2( Frame[k][0],Frame[k-1][0],green ) else // рисуем раскрытие днища
      if( Frame[k][0].x<=Stem[0].x && Frame[k-1][0].x>=Stern[0].x )
        L2( Frame[k][0],Frame[k-1][0],green )    // иначе днище между штевнями
      L2( Frame[k][i],Frame[k-1][j],lightblue ) //  и раскрытие ширстрека
    } j=i;                                     // вершина предыдущего шпангоута
  }
  if( Mode ) // теперь прорисовка штевней с оконтуриванием транцевых расширений
  { for( k=0; k<Stern.len-1; k++ )
     L1( Stern[k],Stern[k+1],Stern[k].z<0 ? green:dimgray )
    for( k=0; k<Stem.len-1; k++ )
     L1( Stem[k+1],Stem[k],Stem[k].z<0 ? green:dimgray )
  }
  if( Mode>1 )glEnable( GL_LINE_SMOOTH );   // восстановление сглаживания линий
                    // включение одноразовой прорисовки корпуса вместо расчетов
  if( type )DrawMode|=3; Floating( true );  // изображение корпуса только здесь
  if( type )(DrawMode&=~3)|=Mode;           //    -- с 3D-обшивкой
  if( !type )NavigaInform( this );          // навигационная информация
  return *this;                             //  о текущем состоянии корабля
}
