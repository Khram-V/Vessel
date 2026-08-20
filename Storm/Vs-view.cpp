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
       int //Board=0,     // 'о' штевни; '-' левый и '+' правый борт
       Level=-2,          // -2-днище -1-вода 0-ватерлиния 1-смочен 2-сухой
       wLine=1;           // -1-ниже; +1-выше цвета конструктивной ватерлинии
       bool Part=false,   // false= днище и ватерлиния; true= надводный борт
        drawHull=false;   // прорисовка корпуса | гидродинамический процесс
       Color Cx={0x00000000};

//static Real ArLen=0.1;    // относительная длина для стрелок на шпациях
//Vertex::Vertex( _Vector a )  // конструктор и собственно
//{ w=Storm->Value( Point::operator=(Vessel->out( Vector::operator=( a ) )) ); }
//static Flex wL;         // изменчивые отрезки фрагментов ватерлинии
                          // по уровню воды в шпациях левого/правого борта

void Hull::drawTriangle(_Vector a,_Vector b,_Vector c ) // отработка трёх точек
{ if( !drawHull )Three( Level,a,b,c );    // единожды производится динамический
  else                                    //  перерасчет характеристик обводов
  { const byte Mode=Pic.hull;             //  для полупрозрачной картинки нужна
    if( !Part && Level>0 || Part && Level<=0 || !Level && Mode>2 )return;
//  int wLine = a.z>0 && b.z>0 && c.z>0 ? 1:-1;   // двухэтапная перепрорисовка
    if( Cx.C )glColor3ubv( Cx.c ); else
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
    if( Level<0 )if( Pic.flow )
    if( fabs( N.y )>eps || fabs( N.x )<eps ) // штевни мимо, а днище в картинку
    { Vector V,                         // скорости отмеряются только под водой
        M=( A+B+C )/3.0,m=in( M ),// (a+b+c)/3 центр элементарного треугольника
        n=dir( N ),Vn,Vs;
      if( Statum>3 && Storm->Exp.wave )
        { Storm->WaveV( Trun,M,V ); } else V=0;       // скорость в потоке волны
      V += (Route[-2]-Route[-1])/Ts + LtA( m*Whirl[-1] ); // полный ход с вращением
        //=+= LtA( -Rate[-1] ) == случай расчётного ходового набегающего потока
      Vs=n*( V*n );              // вектор вихревого слоя от обтекающего потока
      Vn=-n*( V%n );             // вектор отраженного (-) по нормали импульса
      glLineWidth(0); arrow(M,M+V,0.1,lightcyan); // суммарный набегающий поток
      glLineWidth(1);                    // вихреисточники -- векторы скоростей
      if( Pic.flow<3 ){ arrow( M,M+Vs,0.1,lightblue );      // касательных и
        if( Pic.flow>1 )arrow( M,M+Vn,0.1,lightred );       // нормальных - или
      } else            arrow( M,M+Vn+Vs,0.1,lightmagenta );// вектор отражения
  } }
}
//!  сборка сортировкой двух фрагментов ватерлинии в интервале одной шпации
//           (здесь надо найти локализованное решение по выбору ориентации)
//
void Hull::waterPoints( _Vector N,_Vector Q,_Vector P )
{ //if( Q==P )return;
  wL+=N; /// dir( N )??
  if( LtA( N*(P-Q) ).z>=0 ){ wL+=Q; wL+=P; } else { wL+=P; wL+=Q; }
//if( out( N*(P-Q) ).z>=0 ){ wL+=Q; wL+=P; } else { wL+=P; wL+=Q; }
}
void Hull::divideTriangle
( _Vector T,_Real t, _Vector R,_Real r, _Vector L,_Real l )
{ _Vector rR=(Vector)T+(t/(t-r))*(R-T),       // правая точка пересечения ребра
          lL=(Vector)T+(t/(t-l))*(L-T); Level=t>=0?-1:1;// треугольника и левая
  if( rR!=lL )waterPoints( (lL-T)*(T-rR),lL,rR ),       //   +++
              drawTriangle( T,rR,lL );
  if( !l && !r )return;             Level=t<0?-1:1;
  if( !l )drawTriangle( L,rR,R ); else
  if( !r )drawTriangle( R,L,lL ); else
  if( fabs( r )>fabs( l ) )drawTriangle( R,L,lL ),drawTriangle( R,lL,rR );
                      else drawTriangle( L,lL,rR ),drawTriangle( L,rR,R );
}
void Hull::Triangle( Vector a, Vector b, Vector c )   // обработка треугольника
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
   wLine=a.z+b.z+c.z>=0 ? 1:-1;    // здесь пересечений ватерлинии не ожидается
   Level = -2; //wLine*2;          // установка уровня треугольника стоит здесь
    if( !aZ && !bZ ){ if(cZ>0)waterPoints((b-c)*(c-a),b,a); else Level=2; }else
    if( !bZ && !cZ ){ if(aZ>0)waterPoints((c-a)*(a-b),c,b); else Level=2; }else
    if( !cZ && !aZ ){ if(bZ>0)waterPoints((a-b)*(b-c),a,c); else Level=2; }else
    if( aZ<=0&&bZ<=0&&cZ<=0 )Level=2; else   // треугольник целиком над волной
    if( aZ>=0&&bZ>=0&&cZ>=0 )Level=-2; else  // треугольник полностью под водой
    { Real ab=aZ*bZ,bc=bZ*cZ,ca=cZ*aZ;       // иначе рассечение по ватерлинии
      if( ab<0&&ca<=0){ divideTriangle(a,aZ,b,bZ,c,cZ); return; } // выбор вершины
      if( bc<0&&ab<=0){ divideTriangle(b,bZ,c,cZ,a,aZ); return; } // треугольника
      if( ca<0&&bc<=0){ divideTriangle(c,cZ,a,aZ,b,bZ); return; } // для деления
    } drawTriangle( a,b,c );
} }
//    Кинематическая постановка корпуса корабля на объединенное волновое поле
//
HullVsl& HullVsl::Floating( bool onlyDraw )
{ // работа с треугольниками обшивки корпуса и фрагментами ватерлинии в шпациях
  // ~~   троекратная дорисовка корпуса по уровням относительно ватерлинии
  // ~~       обусловливается последовательностью наложения прозрачности
 Vector P,Q,R;
 int i; Part=false; Cx.C=0;       // разделение корпуса на прозрачные подуровни
        drawHull=onlyDraw;        // копия режима расчетов(-) или прорисовки(+)
  if( !onlyDraw )ThreeInitial();  // начальная чистка для интегрируемых величин
//else if( !Ready() )return *this;
//else if( !IsWindowVisible( hWnd ) || IsIconic( hWnd ) )return *this;
  wL.len=0;      // ватерлиния с нормалями и запутанными разделёнными отрезками
Part_of_hull:    // разделение корпуса по уровням надводной и смоченной обшивки
  //                    с разрешением проблем полупрозрачности бортовой обшивки
  //  ...транцы на штевнях должны отрабатываться вогнутыми контурами... !!!
  //
  if( onlyDraw )glEnable( GL_LIGHTING );
//#pragma omp parallel for
  for( int k=0; k<=1; k++ ){ Flex &S=k?Stem:Stern;
    for( int i=0; i<S.len; i++ ){ Q=S[i];
      if( k )Q.y=-Q.y;
      if( i>0 ){ if( P.y )Triangle( P,~P,~Q );
                 if( Q.y )Triangle( Q,P,~Q ); } P=Q;
  } }
  //   собственно цикл покрытия оболочки бортовой обшивки
  //   по шпациям между теоретическими шпангоутами корпуса
  //
//#pragma omp parallel for private( P,Q,R ) ~~
  for( int k=0; k<=Nframes; k++ ) // штевни и шпангоуты Nframes
  if( Shell[k] )              // -- есть ли сам корпус, не пропущена ли шпация?
  if( Shell[k][0]>=3 )       // в шпации присутствует хотя бы один треугольник?
  { for( int Board=-1; Board<2; Board+=2 )
    { P=Select( k,Board>0?-1:1 );            // это блок стандартных шпаций
      Q=Select( k,Board>0?-2:2 );            // сначала левый-> правый шпангоут
      for( int i=3; i<=Shell[k][0]; i++ )    // всех теоретических шпангоутов
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
    if( !KtE )                   // конструктивная или теоретическая ватерлиния
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
      L=0.0; wM=0.0; fM=0.0;     // c разделением разрывных контуров ватерлинии
      for( i=0; i<W.len-1; i++ )
        { L+=(l=abs( W[i]-W[i+1] )); wM+=l*( W[i]+W[i+1] ); }  // длина контура
      if( L>eps )wM/=2*L;                                   // центр ватерлинии
        for( i=0; i<W.len-1; i++ )fM+=(W[i]-wM)*(W[i+1]-wM); // площадь к знаку
      if( Zenit%( LtA( fM ) )<0.0 )                  // ориентация по вертикали
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

// static Real Vm=1,Vi=0; // Масштаб скорости и отсчет среднеквадратичной суммы
