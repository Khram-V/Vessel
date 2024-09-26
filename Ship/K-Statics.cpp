//
//    Программа расчета статической и ?? динамической остойчивости,
//              кривых элементов теоретического чертежа,
//              ?? масштаба Бонжана и ?? непотопляемости
//              на основе простой таблицы плазовых ординат Y[nZ][nX]
//                  1997, январь, В.Храмушин, Южно-Сахалинск
//                  2001, август - Владивосток, октябрь - ГС-210
//                  2006, июнь, Охотское море - "Профессор Богоров"
//                  2020, август, ‏יְרוּשָׁלַיִם‏‎ - КММС ПМ-ПУ СПбГУ
#include "Hull.h"

static int w1=2,wN=8;     // нижняя и верхняя ватерлинии на диаграмме Рида
static Real Amax=180,     // Максимальный угол накренения в градусах
            Zmet=0;       // Относительная аппликата центра тяжести
static byte Mode=0x03;    // 0x00 - плечи остойчивости формы при Zg=Zc
                          // 0x01 - остойчивость с метацентрической высотой
                          // 0x02 - положение ЦТ относительно ватерлинии
static Real HiText=1;
//
//    Главные, т.е. внутренние константы
//
const int nA=180,         // Количество углов накренения             ~ 36 ^ 5°
          nX=97,nZ=121,   // Количество точек на сплайновых кривых   ~ 96 x 120
          Bord=16;        // Пустые обрамления в графическом окне

struct WinStability: public Window
{ WinStability( int W,int H ): Window( " Ship Hull Hydrostatics",0,0,W,H ){}
 virtual bool Draw();     // главная процедура для прорисовки всех результатов
};
static WinStability *stWin=0;
static Plane  *wS=0,      // Окно проекции корпус
              *wC=0,      // Гидростат. кривые элементов теоретического чертежа
              *wT=0;      // Диаграмма остойчивости
static Place *MPL=0;      // информация от мышки

struct Hydrostatic        // подборка гидростатических кривых
{ Real  Lmin,Lmax,        // отрицательное и максимальное плечо
             EpsV,        // минимальный расчетный объем
            dZ,dX,        // шаг сетки по таблице плазовых ординат
           Y[nZ][nX],     //  Y    таблица плазовых ординат
   aX[nZ],aY[nZ],sX[nZ],sY[nZ], // ахтерштевень и форштевень
         Vol[nZ],         //  V    Грузовой размер
         zCV[nZ],         // zC    Аппликата центра величины
         Swl[nZ],         // Swl   Площадь ватерлинии
         Srf[nZ],         //  S    Смоченная поверхность корпуса
         xCV[nZ],         // xC    Абсцисса центра величины
         xCW[nZ],         // xSwl  Абсцисса центра площади ватерлинии
          Jx[nZ],         //  Jx   Поперечный момент инерции площади ватерлинии
          Jy[nZ],         //  Jy   Продольный момент инерции площади ватерлинии
          rx[nZ],         //  r    Поперечный метацентрический радиус
          Ry[nZ],         //  R    Продольный метацентрический радиус
          zM[nZ],         // zM    Аппликата метацентра
          aV[nA][nZ];     // -+-   массив водоизмещений с разными углами крена
  complex aC[nA][nZ];     // -+-   аппликат и ординат новых центров величины
  Hydrostatic();          // конструктор с заполненим таблицы плазовых ординат
  void Initial();         // перерасчёт основных гидростатических характеристик
  void Graphics();
  void Stability();
  void Stability_Lines();
  void Axis_Statics( _Real A, bool clear=true );
  int  Stability_Menu();
  Real In( Real z, Real *F );
};
Real Hydrostatic::In( Real z, Real *F ) // nZ>2 всегда
{ int k=minmax(0,int(z/=dZ),nZ-2); Real *f=F+k; return f[0]+(f[1]-f[0])*(z-k);
}
Hydrostatic::Hydrostatic(): Lmax( Depth ),Lmin( Do )
{ int i,k; Real x,z; EpsV=Volume/1000;
  dX=Lmx/nX; //(nX-1);
  dZ=(Depth-Do)/nZ; //(nZ-1);            // Do - предустанавливается при вызове
  for( z=Do,k=0; k<nZ; k++,z+=dZ )       //  Y - таблица плазовых ординат
  { for( x=Xo,i=0; i<nX; i++,x+=dX )Y[k][i]=Kh.Y( x,z );
        aX[k]=Kh.Asx.G( z ); sX[k]=Kh.Stx.G( z ); // абсциссы и
        aY[k]=Kh.Asy.G( z ); sY[k]=Kh.Sty.G( z ); // ординаты транцевых штевней
  }
  for( k=0; k<nZ; k++ )
  { Vol[k]=zCV[k]=Swl[k]=Srf[k]=xCW[k]=xCV[k]=Jx[k]=Jy[k]=rx[k]=Ry[k]=zM[k]=0;
    for( int i=0; i<nA; i++ )aV[i][k]=0.0,       // грузовой размер с креном
                             aC[i][k]=0.0;       // центр величины от ОЛ
} }
//   Основные кривые элементов теоретического чертежа формы
//   корпуса судна, с определением его базовых характеристик
//
void Hydrostatic::Initial()      // Процедура предварительного перерасчета всех
{ int i,k; // bool b;            //  массив и характеристик, необходимых для
  Real R,r,cX,S, x,y,z=Do+dZ/2;  //  построения кривых элементов теоретического
  for( k=0; k<nZ; k++,z+=dZ )    //  чертежа судна по массивам плазовых ординат
  { x=Xo+dX/2;   //-dX/2;        //  для вычисления площадей, объемов
//  z=Do+dZ*k; //(Real( k )+0.5) //  их моментов инерции (z+1/2 - центр объема)
    R=r=cX=S=0.0;               // Первый прогон интегрирования, нулевая ширина
    for( i=0; i<nX; i++,x+=dX ) //   и метацентрических радиусов
    if( (y=Y[k][i])>0.0 )
    { S += y;
      cX+= y*x;
      r += y*y*y;                     // стр.61 у Владимира Вениаминовича С-Т-Ш
      R += y*x*x;
#if 0
      if( i>0 )b=Y[k][i-1]<=0.0; else b=false;
      if( !i || b )
      { Real ix=In( z,aX ),iy=max( 0.0,In( z,aY ) ),
        sy = (ix-x)*(y+iy) /dX/4,
        sx = x - (x-ix)*( y+2*iy )/(y+iy)/3;          // центр площади трапеции
//        S += sy;
sy=(y+iy)/dX/2;
//        r += sy*sy*sy;
//        R += sy*sx*sx;
//        cX+= sy*sx;
      }
      if( i<nX-1 )b=Y[k][i+1]<=0.0; else b=false;
      if( i==nX-1 || b )
      { Real ix=In( z,sX ),iy=max( 0.0,In( z,sY ) ),
        sy = (ix-x)*(y+iy) /dX/4,
        sx = x +  (ix-x)*( y+2*iy )/(y+iy)/3;
//        S += sy;
sy=(y+iy)/dX/2;
//        r += sy*sy*sy;
//        R += sy*sx*sx;
//        cX+= sy*sx;
      }
#endif
    }
    Swl[k]=( S*=2*dX );              // Площадь ватерлинии
            cX*=2*dX;                // Статический момент площади ватерлинии
    if( !k ){ Vol[0]=0; zCV[0]=Do; xCV[0]=Xm; if( S>EpsV/dZ )xCV[0]=cX/S; }else
    if( !S )Vol[k]=Vol[k-1],zCV[k]=zCV[k-1],xCV[k]=xCV[k-1]; else
    { Real v=dZ*(S+Swl[k-1])/2;      // Чистый метод трапеций
      Vol[k]=Vol[k-1] + v;           // Грузовой размер
      zCV[k]=zCV[k-1] + v*(z-dZ/2);  // Момент для аппликаты центра величины zC
      xCV[k]=xCV[k-1] + cX*dZ;       // Момент для абсциссы центра величины xC
    }
    if( !k && !S )xCW[k]=Xm,Jy[k]=Jx[k]=0; else
    if( k && !S )xCW[k]=xCW[k-1],Jy[k]=Jy[k-1],Jx[k]=Jx[k-1]; else
    { xCW[k]=cX/S; //+Xo;        // Центр площади ватерлинии
      Jy[k]=(R*2*dX-cX*cX/S);    // /Vol[k]->Продольный метацентрический радиус
      Jx[k]= r*2*dX/3;           // /Vol[k]->Поперечный --//--
    }
  }                              //
  Srf[0]=S=Swl[0];               // Площадь смоченной поверхности
  for( k=1; k<nZ; k++ )          //
  { S+=( Y[k][0]+Y[k-1][0]+Y[k][nX-1]+Y[k-1][nX-1] )*dZ;// без двойки два борта
    if( k==1 )
    for( i=1; i<nX; i++ )
    { S += (Y[0][i-1]+Y[0][i])*dX; //if( i && i<nX-1 )S+=y; // второй борт под трапециями
    }
    for( i=1; i<nX; i++ )
    { y=Y[k][i]; Real yx=Y[k][i-1],yz=Y[k-1][i];
      if( y>0||yx>0||yz>0 )
      { S += y = sqrt( 1.0+norm( (yx-y)/dX,(yz-y)/dZ ) )*dX*dZ*2;
//      if( i==1 || i==nX-1 )S -= y/2;       // трапеции строго по оконечностям
      }
    } Srf[k]=S;
  }
  zCV[0]=Do;
  xCV[0]=xCW[0];
  for( k=0; k<nZ; k++ )
  { if( !k )Ry[0]=rx[0]=0; else
    { xCV[k]/=Vol[k]+Swl[k]*dZ;               // половинка счетной ватерлинии
      zCV[k]/=Vol[k]; // (Vol[k]+Vol[k-1])/2;
      rx[k]=Jx[k]/Vol[k];
      Ry[k]=Jy[k]/Vol[k];
    }
//  zCV[k]-=dZ/2;   //  Do;
    zM[k]=rx[k]+zCV[k];

  }
}
static void MinMax( Real *F, int N, Real &Min, Real &Max, const int mx=0 )
{                         if( !mx )Min=F[0]-0.1,Max=F[0]+0.1;
  for( int i=0; i<N; i++ )if( Min>F[i] )Min=F[i]; else
                          if( Max<F[i] )Max=F[i];
}
static void Graphic_for_Element
( Real *C,            // собственно прорисовываемая кривая (NULL - если ее нет)
  int   y,            // смешение оси по вертикали
  Real  I,            // начальный отсчет шкалы
  Real  L,            // длина шкалы оси
  const int up,       // длина маркеров отметки шкалы (-вверх, +вниз, 0-нет)
  const int sp,       // местоположение метки
  const char Label[], // подпись на шкале и на графике
  const int Scale=1   // признак перемасштабирования (округления) шкалы
){
 Real dL,Y,uY;
  if( Scale )
  { if( L==0.0 )MinMax( C,nZ,I,L ); dL=AxisStep( L-I );
    I=floor( I/dL )*dL;
    L=ceil( (L-I)/dL )*dL;
  } else dL=AxisStep( L-=I );
     wC->Set( I,0, I+L,nZ );
   Y=wC->Z( y );
  uY=wC->Z( y-up*2 )-Y;
  if( up )
  { Real x=int( I/dL )*dL; stWin->AlfaVector( HiText*0.8,1 );
    line( I,Y,I+L,Y );
    for( int i=0; x<I+L+dL/5; i++,x+=dL/5 )    // Разметка шкалы
    { line( x,Y,x,i%5?Y+uY/3:Y+uY );
      if( i>0 && i%5==0 )
      stWin->Text(up>0?_South_West:_North_West,x,Y+uY/5,0,"%.9g",e5(x));
    } stWin->AlfaVector( HiText,2 )
            .Text(up>0?_South_East:_North_East,I,Y+uY/5,0,Label )
            .AlfaVector( HiText,1 );

  }
  if( C )
  { glBegin( GL_LINE_STRIP );
    for( int i=1; i<nZ-1; i++ )glVertex2d( C[i],i );      // !!! -2 или
    glEnd(); stWin->Text( _South_East,C[sp],sp,0,Label ); // без днища и палубы
  }
}
//  Блок расчета грузового размера и смоченной поверхности корпуса
//
void Hydrostatic::Graphics()
{ int i,j,k=nZ/4;
  Real mn=0,mx=0,dW=Draught/10;
  stWin->Activate(); wC->Focus().Clear();
  gl_BLUE;
    Graphic_for_Element( Vol,-2.5*HiText,0,Vol[nZ-1],-4,k++,"V" );
    Graphic_for_Element( Srf,-2.5*HiText,0,Srf[nZ-1], 4,k++,"S" );
  gl_LIGHTRED;
    stWin->Text( _South_West,0,0,0,"КЭТЧ " );
  gl_GREEN;
    MinMax( xCW+1,nZ-2,mn,mx );
    MinMax( xCV+1,nZ-2,mn,mx,1 ); if( mn<Xo )mn=Xo;
                                  if( mx>Xo+Lwl )mx=Xo+Lwl;
    Graphic_for_Element(   0,-5*HiText,mn,mx,4,k,"xS,xC" ); // Абсциссы
    Graphic_for_Element( xCV,-5*HiText,mn,mx,0,k++, "xC" ); // величины
    Graphic_for_Element( xCW,-5*HiText,mn,mx,0,k+=2,"xS" ); // и площади
    MinMax( Swl+1,nZ-2,mn=0,mx=0 );
    Graphic_for_Element( Swl,-5*HiText,mn-0.1,mx+0.1,-4,k+=2,"Swl" );
  gl_RED;
    MinMax( rx+1,nZ-2,mn=0,mx=0 );
    Graphic_for_Element( rx,-7.5*HiText,0,min(mx,Breadth*2 ),-4,k++,"r" );
    MinMax( Ry+1,nZ-2,mn=0,mx=0 );
    Graphic_for_Element( Ry,-7.5*HiText,0,min(mx,Length*2),+4,k++,"R" );
  gl_MAGENTA;
    Graphic_for_Element( Jx,-10*HiText,0,0,-4,k++,"Jx" );   // Инерция
    Graphic_for_Element( Jy,-10*HiText,0,0,+4,k++,"Jy" );   // ватерлинии
  gl_BLACK;
    Graphic_for_Element(   0,0,0,Depth,4,k++,"Z,zC,zM",0 );
    Graphic_for_Element( zCV,0,0,Depth,0,k++,"zC"     ,0 );
    Graphic_for_Element( zM, 0,0,Depth,0,k++,"zM"     ,0 );
  gl_LIGHTGRAY;                            // контрольная диагональ
    wC->Set( 0,Do,Depth,Depth );           glEnable( GL_LINE_STIPPLE );
    line( Do,Do,  Depth,Depth );           glDisable( GL_LINE_STIPPLE );
  for( k=w1*2; k<=wN*2; k++ )  // контрольные посадки по диаграмме остойчивости
  { if( k==10 )gl_LIGHTMAGENTA; else if( k<10 )gl_LIGHTGREEN;else gl_LIGHTCYAN;
    if( k&1 )glEnable( GL_LINE_STIPPLE ); line( 0,k*dW,Depth,k*dW );
    if( k&1 )glDisable( GL_LINE_STIPPLE );
  }
  gl_BLUE;
  stWin->AlfaVector( HiText,2 )
   .Print(-3,-5," %s \n Длина / ширина / осадка:   %1.0f / %0.1f / %0.1f \n"
                " Водоизмещение / смоченная обшивка:  %0.1f / %0.1f",
                Kh.Name,Length,Breadth,Draught,Volume,Surface );
  stWin->AlfaVector( HiText,1 );
  //
  //  отметка аппликат метацентров и центров величины для расчетных ватерлиний
  //
  wS->Set( Breadth/-2,Do,Breadth/2,Depth ).Clear(); gl_LIGHTCYAN;
  for( i=0; i<nX; i++ )
  { glBegin( GL_LINE_STRIP );
    for( k=0; k<nZ; k++)glVertex2d((i<nX/2)?-Y[k][i]:Y[k][i],k*dZ+Do); glEnd();
  }
  Draw_Hull( 2,*wS );     // переход к двойному изображению в проекции "Корпус"
  glEnable( GL_POINT_SMOOTH );
  for( k=w1*2; k<=wN*2; k++ )
  { j=(k*dW-Do)/dZ; Real zm=zM[j],zc=zCV[j],a=0.66;        // для всех посадок
    glPointSize( i=(k==10?16:(k%2?8:12)) ); i-=3;          // отмечаются центры
    if( k==10 )glColor4f(0,.6,0,a); else                   // величины
    if( k<10 )glColor4f(0,.9,.6,a); else glColor4f(.6,.9,0,a);
    glBegin( GL_POINTS ); glVertex2d( 0,zc ); glEnd();
    glPointSize( i );                                      // и действующие
    if( k==10 )glColor4f(.9,.1,.1,a); else                 //   метацентры
    if( k<10 )glColor4f(.9,0,.6,a); else glColor4f(.9,.6,0,a);
    glBegin( GL_POINTS ); glVertex2d( 0,zm ); glEnd();
} }
//   Простая прорисовка осей для диаграммы Рида
//
void Hydrostatic::Axis_Statics( _Real A,bool clear )
{ int i,k;
 Real dA=5*M_PI/180.0,z,dz=AxisStep( 0.75*( Lmax-Lmin ) ); wT->Focus();
  if( clear )                   // подчистка для перерисовки диаграммы Рида
      glColor4f( 1,0.98,0.96,1 ),
      glRectd( wT->X(wT->ax-20),wT->Z(-20),wT->X(wT->ux),wT->Z(wT->uz-13) );
  glBegin( GL_LINE_STRIP ); gl_LIGHTGRAY; glVertex2i( 0,0 ),glVertex2d( A,0 ),
      glVertex2d( A,Lmin ),glVertex2d( 0,Lmin ),glVertex2d( 0,Lmax ); glEnd();
  glColor3f( 0.75,0.75,0.25 ),
  glBegin( GL_LINES ); glVertex2i( 1,0 ),glVertex2d( 1,Lmax );
  gl_GREEN;
  for( k=0; k<=36 && k*dA<A; k++ )
     glVertex2d( k*dA,Lmin ),
     glVertex2d( k*dA,wT->Z( wT->z( Lmin )+(k&1?4:8) ) );
  gl_LIGHTGRAY;
  for( k=1,z=dz/2; z<=Lmax; k++,z+=dz/2 )
     glVertex2d( wT->X( (k%2)?6:12 ),z ),glVertex2d( 0,z );
  for( k=1,z=-dz/2; z>=Lmin; k++,z-=dz/2 )
     glVertex2d( wT->X( (k%2)?6:12 ),z ),glVertex2d( 0,z );
  glEnd(); gl_CYAN;
  stWin->AlfaVector( HiText,1 ).Text( _East,wT->X( 64 ),Lmax,0,
                 !Mode ? "Остойчивость формы: Zg = Zc + %4.2f м" :
               Mode==1 ? "Метацентрическая высота: h = %4.2f м"  :
               Mode==2 ? "Центр тяжести над ватерлинией:  Zg-T = %4.2f м" :
                         "Аппликата фиксирована: Zg = %4.2f м", Zmet );
  gl_BLACK;
  stWin->AlfaVector( HiText*0.9,1 ); i = A*180.0/M_PI>90?6: A*180.0/M_PI>45?3:2;
  for( k=0; k<=36 && k*dA<A; k++ )
   if( !(k%i) )stWin->Text( _South,k*dA,wT->Z( wT->z( Lmin )-2 ),0,"%i",k*5 );
  for( z=dz; z<=Lmax; z+=dz )stWin->Text( _North_East,wT->X( 6 ),z,0,"%.3g",z );
  for( z=-dz; z>Lmin+dz; z-=dz )stWin->Text(_South_East,wT->X(3),z,0,"%.3g",z );
}
//   Расчёт плеч статической остойчивости формы
//
void Hydrostatic::Stability()
{ int  j,                // Индекс угла накренения
       k;                // Индекс контрольной осадки (*cos)
 Real  Z,A,dA,           // Ведущие аргументы
       V,Mx,My;          // Объемные сумматоры моментов элементарной площадки
//const Real DM=Depth-Do;
  stWin->Activate(); wT->Set( 0.0,Lmin=-Depth,A=M_PI*Amax/180.0,Lmax=Depth );
  Axis_Statics( A ); dA=A/=nA;          // Шаг по углу накренения
  for( j=0; j<nA; j++,A+=dA )
  for( k=0; k<nZ; k++ )
  { V=Mx=My=0.0;                        // Подбор наклоненного прямоугольника
    if( !k ){ Z=Breadth*sin( -A )/2; if( A>M_PI_2 )Z+=Depth*cos( A ); }
       else { Z+=( fabs( Depth*cos( A ) )+Breadth*sin( A ) )/(nZ-1); }
#pragma omp parallel for reduction(+: V,Mx,My )
    for( int i=0; i<nX; i++ )  // Перебор индексов шпангоутов
    { int l,                   // Индекс номера точки на контуре шпангоута
          m,                   // Признак обратного прохода по шпангоуту (+-1)
          n,                   // Признак определения первой точки на шпангоуте
          o;                   // Признак обратного входа в контур шпангоута
     complex f0={0,Y[0][i]},f1,f2,a,b,c;  // Три путевые, три контрольные точки
      f1=f0*=polar( A );                  // Абсцисса первой точки всегда = 0
      if( Z-f1.x>=0 ){ a=c=f1; n=1; } else n=0; o=0;
      for( l=m=1; l>=-1; )
      { if( l<0 )f2=f0; else              // Контур одного шпангоута
        { f2.x=l*dZ;
          f2.y=m*Y[l][i];                 // Строится обход против
          f2*=polar( A );                 //       часовой стрелки
        }
        if( l<0 || Z-f1.x>=0 || Z-f2.x>=0 )
        do
        { if( l<0 && Z-f1.x<0 && Z-f2.x<0 )b=c; else
          { b=f2;
            if( Z-f2.x<0 ){ b.x=Z; b.y=f1.y+(Z-f1.x)*(f2.y-f1.y)/(f2.x-f1.x); }
            if( Z-f1.x<0 )
            { if( !o ){ o=1; b.x=Z; b.y=f2.y+(Z-f2.x)*(f2.y-f1.y)/(f2.x-f1.x); }
                 else   o=0;
          } }
          if( !n ){ c=b; n=1; } else
          { Real dV=( a.y*b.x-b.y*a.x )/2; // Элементарная площадка
            Mx += ( a.x+b.x )*dV/3.0;      // Центр элементарной площади
            My += ( a.y+b.y )*dV/3.0;      // ОМР почему-то не любит структуры
            V += dV;
          } a=b;
        } while( o );
        f1=f2;
        if( m>0 && l==nZ-1 )m=-1; else l+=m; // Закрутка на другой борт
      }
    }
    if( V>0 )aC[j][k]=(complex){Mx/V,My/V};// Координаты новых центров величины
             aV[j][k]=V*dX;               // Массив водоизмещений с накренением
    //
    // Предварительная прорисовка результатов в левом нижнем поле экрана
    //
    if( !stWin->Ready() )return; else
    if( j>0 )
    { glBegin( GL_LINES );
      gl_CYAN; glVertex2d( A-dA,aC[j-1][k].x ),glVertex2d( A,aC[j][k].x );
      gl_BLUE; glVertex2d( A-dA,aC[j-1][k].y ),glVertex2d( A,aC[j][k].y );
      glEnd();
    }
    if( !k && j%(nA/16)==1 )stWin->Show();
  }
}
void Hydrostatic::Stability_Lines()           // Быстрая отрисовка плеч
{ int i,j,k;                                  // статической остойчивости формы
 Real dA=M_PI*Amax/( nA*180.0 ),A=dA,         // Шаг по углу накренения
      dW=Draught/10,Sw[nZ]={0.0};
 complex dC; Lmin=Lmax=0.0;
  if( w1*dW*2 < dZ )w1 = dZ/dW/2+1;                   // проверка доступности
  if( wN*dW*2 > Depth-dZ )wN = (Depth-dZ)/dW/2;       // интервала ватерлиний
 Real aP[nA][(wN-w1)*2+1];                            //  Res[a,d]
  //
  //  Процедура пересчета всех массивов к равнообъёмным накренениям
  //
  for( j=0; j<nA; j++,A+=dA )
  { for( k=1; k<nZ; k++ )
    { Real V=Vol[k];                                // Поиск аргумента-осадки
      for( i=1; aV[j][i]<V && i<nZ-1; )i++;         //   по грузовому размеру
      Sw[k]=i-(aV[j][i]-V)/(aV[j][i]-aV[j][i-1]);   // - индексная метка осадки
    }
    for( k=w1*2; k<=wN*2; k++ )                 // расчетные плечи остойчивости
    { Real dV,z=k*dW-Do+dZ,&Sm=aP[j][k-w1*2];   // формы/статики ...
      int m=minmax( 0,int( z/dZ ),nZ-1 );       // интерполяция к теоретическим
      int i=minmax( 0,int( Sw[m] ),nZ-2 );      // ватерлиниям -- только --
                                                // -- с одной промежуточной
      dV = Sw[m]-i;                             //  0 - от центра величины
      dC = aC[j][i+1]*dV + aC[j][i]*( 1-dV );   //  1 - Метацентрическая высота
      switch( Mode&0x3 )                        //  2 - относительно ватерлинии
      { case 0: Sm=dC.y-( In( z,zCV )+Zmet-Do )*sin( A ); break;
        case 1: Sm=dC.y-( In( z,zM )-Zmet-Do )*sin( A ); break;
        case 2: Sm=dC.y-( z+Zmet  )*sin( A ); break;
        case 3: Sm=dC.y-( Zmet-Do )*sin( A );                   // 3 ЦТ над ОЛ
      }
      if( Sm>Lmax )Lmax=Sm; else if( Sm<Lmin )Lmin=Sm;
  } }
  stWin->Activate();
  Lmax=max( 0.01,Lmax*1.12 );
  Lmin=min(-0.01,Lmin*1.12 );
  wT->Set( 0,Lmin,A,min( Lmax,Breadth ) );
  Axis_Statics( A );                     // в разметке срезан верхний экстремум
  for( k=w1*2; k<=wN*2; k++ )
  { if( !(k&1) )
    { if( k==10 )gl_RED; else if( k<10 )gl_GREEN; else gl_BLUE; } else
    { glEnable( GL_LINE_STIPPLE ); if( k<10 )gl_LIGHTGREEN; else gl_LIGHTCYAN; }
    glBegin( GL_LINE_STRIP );       glVertex2d( 0,0 );
    for( j=0,A=dA; j<nA; j++,A+=dA )glVertex2d( A,aP[j][k-w1*2] ); glEnd();
    glDisable( GL_LINE_STIPPLE );
  } Axis_Statics( M_PI*Amax/180.0,false );
}
///   Информационные процедуры для активного окна
///
static bool Mouse_in_Window( int x, int y )
{ if( (Active&3)!=3 )return false;
  Plane *M; stWin->Activate(); MPL->Clear();
  if( wS->Is( x,y ) )M=wS; else
  if( wC->Is( x,y ) )M=wC; else
  if( wT->Is( x,y ) )M=wT; else M=NULL;
/*
glEnable( GL_COLOR_LOGIC_OP );
glLogicOp( GL_XOR ); => GL_COPY | GL_SET
*/
  if( M ){ gl_LIGHTRED; MPL->Print( 1,1,"%s \n%s=%.2f, %s=%.2f ",
                        M->iD,M->sX,M->wX(x),M->sZ,M->wZ(y) ); } MPL->Show();
  return false;
}
int Hydrostatic::Stability_Menu()
{ const Real z=Draught-Do;
  Mlist Menu_S[]={ { 1,0,"   Диаграмма Рида" }
                 , { 2,1 },{ 0,5,"%5.2lf",&Zmet }       // выбор типа диаграммы
                 , { 1,5,"Максимальный угол крена %1°",&Amax }
                 , { 1,2,"Выбор ватерлиний  с %2d",&w1},{0,2,"  по %2d",&wN} };
  TextMenu T( Mlist(Menu_S),stWin,(wC->ax)/9-2,0 );
  static int ans=-1;
  switch( Mode&3 )
    { case 0: Menu_S[1].Msg="ЦТ над центром величины "; break;
      case 1: Menu_S[1].Msg="Метацентрическая высота "; break;
      case 2: Menu_S[1].Msg="Относительно ватерлинии "; break;
      case 3: Menu_S[1].Msg="ЦТ над основной линией  "; break;
    }
  if( (ans=T.Answer( ans ))==1 )
  switch( ++Mode&=3 )
  { case 0: Zmet -= In( z,zCV );             break;
    case 1: Zmet  = In( z,rx )-Zmet;         break;
    case 2: Zmet  = In( z,zM )-Zmet-Draught; break;
    case 3: Zmet += Draught;
  } else
  if( ans )
  { if( fabs( Amax )>180 )Amax=180; // и контроль
    w1=minmax( 1,w1,9 );            // на всякий случай
    wN=minmax( 1,wN,9 ); if( w1>wN )w1=wN=(w1+wN)/2;
  } return ans;
}
//
//!    параллельная работа внешних и графических устройств
//
static Hydrostatic *stLD=NULL;
static bool First=false;                // Блокировка пролога и повторов вызова

bool WinStability::Draw()
{ if( !First || !Ready() )return false; First=false;
  int H=Height,W=Width;
  AlfaVector( HiText=Real( H )/48, 1 ).Activate();
  glClearColor( 1,0.96,0.92,1 ); Clear();
  wT->az= min( H/12,32 );                // основание диаграммы остойчивости
  wT->uz=(wS->az=wC->az=4*H/9)-36;       // разделение с корпусом и КЭТЧ сверху
  wT->ax= wS->ax = Bord;                 // левая сторона совмещена с корпусом
  wC->uz= wS->uz = H-Bord;               // подволок корпуса и кривых элементов
  wC->ux= W-Bord;                        // правая граница кривых элементов ТЧ
  wC->ax=(wT->ux=wS->ux=2*W/5+Bord)+Bord*2; // разделение условной
  W=(wS->ux-wS->ax)/2;                      // середины экрана
 Real P=Breadth*(wS->uz-wS->az)/(Depth-Do)/2;
  if( P<W )wS->ux = wS->ax+W+P,
           wS->ax = wS->ax+W-P;
  MPL->Area( 0,2,28,-38 );           // восстановление маленького окошка мышки
  if( !stLD )                        // изображение чертежа в проекции "Корпус"
  { wS->Set( Breadth/-2,Do, Breadth/2,Depth ); Draw_Hull( 2,*wS ); } else
  { stLD->Stability_Lines();// Быстрая прорисовка плеч статической остойчивости
    stLD->Graphics();     // Прорисовка кривых элементов теоретического чертежа
  }
  Save().Refresh(); First=true; return false;
}
                                 // Работа с графическим изображением
void Hull_Statics()             // кривых элементов теоретического чертежа,
{                              // статической остойчивости и ? масштаба Бонжана
 static int W=1280,H=800;     // размерности графического окна в текущем сеансе
 WinStability StabWin( W,H ); stWin=&StabWin;
  StabWin.AlfaVector( HiText=Real( H )/48.0,1 ).Activate();
 Plane Hull("Корпус",      "Y","Z",stWin); wS=&Hull; // Окно проекции корпус
 Plane Stat("КЭТЧ",        "F","Z",stWin); wC=&Stat; // Гидростатические кривые
 Plane Stab("Остойчивость","a","h",stWin); wT=&Stab; // Диаграмма остойчивости
 Place Mous( stWin,PlaceAbove ); MPL=&Mous;         // информация от мышки с
  glInitial();
  glLineStipple( 1,0x1F1F );
 const char
 *Name[]={ "Stability","    Гидростатика и остойчивость",
                       "классика Семёнова Тянь-Шанского",0 },
 *Cmds[]={ "F1 ", "эта краткая справка",
           "F4 ", "параметры диаграммы остойчивости",0 },
 *Plus[]={ "<Enter> ","обновление изображения",
           "<Esc>   ","выход",
           "~~~ ","—————————————————————",
           " V  ","  грузовой размер, водоизмещение",
           " S  ","  площадь смоченной обшивки",
           " Swl", " площадь действующей ватерлинии",
           " xS ","  абсцисса площади ватерлинии",
           " xC ","  абсцисса центра величины",
           " zC ","  аппликата центра величины",
           " zM ","  аппликата метацентра",
           " r,R","  поперечный (x) и продольный (y)",
           "","        метацентрические радиусы",
           " Jx,Jy"," поперечный и продольный моменты",
           "","          инерции площади ватерлинии",0 };
  StabWin.Activate().Mouse( Mouse_in_Window );    // откат обработки прерываний
  First=true;
  StabWin.Draw();             // разметка экрана и прорисовкой проекции корпус
 int ans=_Enter;
 Hydrostatic LD; stLD=&LD;    // распределение и очистка памяти всех массивов
    LD.Initial();             // проведение расчетов гидростатических кривых
    LD.Graphics();            // расчет кривых элементов теоретического чертежа
    LD.Stability();           // процедура сделает видимыми плечи остойчивости
    Zmet=LD.In( Draught-Do,LD.zM ); // h=0 нулевая поперечная остойчивость
MainLoop:
  switch( ans )
  { case _Esc:StabWin.Close(); break;
    case _F1: StabWin.Help( Name,Cmds,Plus,0,1 ); break;
    case _F4: do{ Real A=Amax;
                  if( !(ans=LD.Stability_Menu()) )goto Ret;
                  if( A!=Amax )LD.Stability(); StabWin.Draw();
                } while( ans!=_Esc && stWin->Ready() ); break;
    case _Enter: StabWin.Draw();                  // принудительная перерисовка
  } ans=StabWin.WaitKey();
  if( StabWin.Ready() )goto MainLoop;

Ret:  W=StabWin.Width;                            // сохраняются размеры экрана
      H=StabWin.Height;                           // для последующих обращений
        First=false;
        stWin=NULL; //while( StabWin.GetKey() );
        stLD=NULL;
}
