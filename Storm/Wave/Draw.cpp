//
//	Математические модели настройки вычислительных экспериментов
//	с использованием вертикальных профилей трохоидальных волн
//
//                                           ©2019 ПМ-ПУ СПбГУ, Старый Петергоф
//
#include "Wave.h"               // собственно графическая оболочка Window-Place

static Vector arrow( Vector a, Vector b,_Real l, const colors clr=empty )
{ Vector d=a-b; d*=l/abs( d );
  Vector e={ d.z/5,d.x/5,d.y/5 },f={ e.z,e.x,e.y },g=b+d/2; line( a,d+=b,clr );
   glBegin( GL_LINE_LOOP ),dot( g ),dot( d+e ),dot( b ),dot( d-e ),
                           dot( g ),dot( d+f ),dot( b ),dot( d-f ),
                           dot( g ),glEnd(); return b;
}
#define hX( H,s,y,z ) ( H[x]+vX( s,y,z ) )
#define vX( s,y,z )  ((Vector){Long/-2+(x*Long)/Nx+s,y,z })
#define curve(x,xo,xe,dx,v) \
  { glBegin(GL_LINE_STRIP); for( x=xo; x<=xe; x+=dx )glVertex3dv(v); glEnd(); }

Waves& Waves::Drawing()
{ Vector A,B,C; int x;                                //    три рабочих вектора
  Real X,Z,Depth=(Long+8*Ds)*Height/Width;            //    на случай изменения
  Activate( false );                                  //        размеров экрана
  glMatrixMode( GL_PROJECTION ),glLoadIdentity();     // перенастройка разметки
  glOrtho( X=-4*Ds+Long/-2,4*Ds+Long/2,Depth*-0.5,Depth*0.5,-1,1 ); // картинки
  glMatrixMode( GL_MODELVIEW ); glLoadIdentity();     // возврат матрицы модели
  glRotated( -90,1,0,0 );    // замена ординаты аппликатой - постановка на киль
  glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT ); // расчистка всего экрана
  //Clear();                                          // случай фрагмента Place
  glBegin( GL_QUADS );                                // подводное освещение
  glColor3d( .7,.85,1 ),dot( (Vector){X,-.1,Depth/-2} ),dot( (Vector){-X,-.1,Depth/-2} );
  glColor3d( .95,1,.97 ),dot( (Vector){-X,-.1,0} ),dot( (Vector){X,-.1,0} );
  glEnd();
  color( lightgreen );                          //  ватерлиния тихой воды
  line( A=Long/-2,B=Long/2 ); A.z=B.z+=Depth/6; line( A,B );
  //  //   шкала узлов расчетной сетки по верхней кромке графического поля
  //
  for( x=0,Z=Depth/2; x<=Nx; x++ )  { X=Long/-2+(x*Long)/Nx; glBegin( GL_LINE_STRIP );    dot( (Vector){ H[x].x+X,    0.1,     Z },lightblue );    dot( (Vector){ X,           0.0,0.92*Z },lightcyan );    dot( (Vector){ X+Ds/2,      0.0,0.88*Z },lightcyan );    dot( (Vector){ V[x].x+X+Ds/2,0.1,0.8*Z },lightgreen ); glEnd();  }
  //
  //   интенсивность закрутки опорных радиус-векторов волновой поверхности
  //
  color( green ); glBegin( GL_LINE_STRIP ); for( x=0; x<=Nx; x++ )
    dot( (Vector){Long/-2+(x*Long)/Nx,0.1,Z*0.9-G[x]*Depth/20} ); glEnd();
  color( red ); glBegin( GL_LINE_STRIP ); for( x=0; x<Nx*2+2; x++ )
    dot( (Vector){Long/-2+(x*Long)/Nx/2,0.1,Z*0.9-D[x]*Depth/20} ); glEnd();
  //
  //   Сетка смещений внутренних узлов в новообразованной герстнеровской волне
  //
  color( lightblue );                            // линии уровней волнения
  for( Z=0.0; Z>=Depth/-2; Z-=5 )
  { curve( X,Long/-2,Long/2,Long/Width,Wave(Tlaps,(Vector){X,0,Z}) )
    if( !Z )color( lightcyan );
  }
  for( X=Long/-2; X<=Long/2; X+=5.0 )                 // сетка нормалей уровней
    curve( Z,Depth/-2.0,0.0,Depth/Height,Wave(Tlaps,(Vector){X,0,Z}) )
  //
  //   Векторы скорости в смещенных теоретических узлах в толще морской волны
  //
  color( lightred );                             // распределение векторов
  for( Z=0.0; Z>=Depth/-2; Z-=5 )                     //   мгновенных скоростей
  for( X=Long/-2; X<=Long/2; X+=5.0 )
  { B=Wave( Tlaps+0.5,(Vector){X,0,Z} )-(A=Wave( Tlaps-0.5,(Vector){X,0,Z} ));
    arrow( A,A+B,abs( B )/4 );
  }
  //   Траектория частиц жидкости под гребнями трохоидального морского волнения
  //
  color( olive );                                     // кривые траектории
  for( Z=0; Z>Depth/-2; Z-=9 )                        //  частиц жидкости
  for( X=Long/1.5; X>=Long/-1.5; X-=Long/3 )
  { Real Tt=max( 0.0,Tlaps-3.75*Length/Cw-fmod( Tlaps,Length/Cw ) );
    glBegin( GL_LINE_STRIP ); C=Wave( Tt,A=(Vector){X,0,Z} );
    for( Real T=Tt; T<Tlaps; T+=dT/6 )
       { dot( C ); B=Wave(T+dT/12,A)-Wave(T-dT/12,A); C+=B; A.x+=B.x; }
    glEnd();
  }
  //   неравномерная сетка - векторы скорости
  //                   ++  опорные радиусы расчетных точек на поверхности волны
  color( lightblue );
  for( x=0; x<=Nx; x+=2 )
  { A=hX( H,0,0,Depth/6 ); B=tKrat*dH[x]/dT;
      arrow( A+B,A-B,abs( B )/4 );                   // векторы смещений частиц
      arrow( vX( 0,0.1,Depth/6 ),A,abs( B )/4 );     // опорные радиус-векторы
  }  color( lightgreen );
  for( x=0; x<Nx; x+=2 )
  { A=hX( V,Ds/2,0,Depth/6 ); B=tKrat*dV[x]*Length/Cw/_Pd/dT;
      arrow( A+B,A-B,abs( B )/4 );
  }
  //  разметка зоны отражаемой волны внутри подвижной отражающей границы
  //
  if( abs( wH )<Long/2 )
  { Real X1,X2; x=0; X1=hX( H,0,0,0 ).x;    for( x=1; x<=Nx; X1=X2,x++ )    if( X1<wH^(X2=hX( H,0,0,0 ).x)<=wH )
    { glBegin( GL_POLYGON ); dot( (Vector){ X2,0,Depth/3 },lightcyan ); //gray );                             dot( (Vector){ X1,0,Depth/3 } );                             dot( (Vector){ X1,0,Depth/5.9 } );                             dot( (Vector){ X2,0,Depth/5.9 } ); glEnd();                             spot( hX(H,0,.1,Depth/6),9.0,blue );
    }    x=0;        X1=hX( V,0,0,0 ).x+Ds/2;    for( x=1; x<=Nx; X1=X2,x++ )    if( X1<wH^(X2=hX( V,0,0,0 ).x+Ds/2)<=wH || X1>X2 )
    { glBegin( GL_POLYGON ); dot( (Vector){ X2,0,Depth/6.1 },lightgreen );                             dot( (Vector){ X1,0,Depth/6.1 } );                             dot( (Vector){ X1,0,0 } );                             dot( (Vector){ X2,0,0 } ); glEnd();                             spot( hX(V,Ds/2,.1,Depth/6.0),9.0,magenta );    }
    arrow( (Vector){wH,.1,Depth/-6},(Vector){wH,.1,Depth/2.5},Depth/24,navy );
  }
  //!  и теперь полученный результат ...
  //
  //   Теоретическая огибающая девятых валов в групповой структуре морских волн
  //
  color( white );
  curve( X,Long/-2,Long/2,Long/Width,(Squad(Tlaps,(Vector){X,0,Depth/8})+(Vector){0,0,Depth/8} ) );
  color( lightgray );          // оконтуривающая линия групповой структуры
  curve( x,0,Nx,1,hX( M,0,0.1,Depth/4 ) ); //  (Vector){Long/-2+(x*Long)/Nx,.1,Depth/4 }) );
  color( blue );              // герстнеровская волна в неискаженной форме
  curve( x,0,Nx,1,hX( H,0,.1,Depth/6 ) );
//  View_color( lightred );                           /// !!! #########
//  curve( x,1,Nx,1,((Vector){Long/-2+(x*Long)/Nx,.1,V[x].z+Depth/6 }) );
//  View_color( red );                               /// !!! #########
//  curve( x,0,Nx,1,((Vector){Long/-2+(x*Long)/Nx,.1,H[x].z+Depth/6 }) );

//color( magenta ); curve( x,0,Nx,1,((H[x]*M[x].z*1.2/Rw)+(Vector){Long/-2+(x*Long)/Nx,0,Depth/4} ) ); //Rw+M[x].z/Rw+20}) );
  color( lightmagenta );  // собственно результат мультипликации двух волн
  curve( x,0,Nx,1,(H[x]*abs(M[x].z)/4 + vX( 0,0.1,Depth/4 ) ) ); // (Vector){Long/-2+(x*Long)/Nx,.1,Depth/4}) ); //Rw+M[x].z/Rw+20}) );
//curve( x,0,Nx,1,((H[x]+M[x])/2+(Vector){Long/-2+(x*Long)/Nx,.1,Depth/4}) ); //Rw+M[x].z/Rw+20}) );
  color( green );         // отметка величины и смещения вектора скорости
  curve( x,0,Nx-1,1,hX(V,Ds/2,.1,Depth/6 ) );  //  //    отметка текущего времени вычислительного эксперимента  //  { Event T( Instant+Tcalc/3600.0 ); char S[24];     // реальный отсчет времени
    int M,D,Y; T.UnPack( M,D,Y ); strcpy( S,DtoA( Tlaps/3600.0,-3 ) );    color( navy ); Print( -2,-1,"%04d %s %02d, %s%-12s <= %d =>%s",                          Y,_Mnt[M-1],D,_Day[int(T.D%7)],DtoA( T.T,-3 ),Kd,S );  }
  //   контрольные отсечки фазовой скорости
  //
#if 0
  { View_color( red );
    //glBegin(GL_LINE_STRIP);                             C.y=0.0;
    for( x=0; x<=Nx; x++ )
    { //A=dV[x];                                        //
      if( x<Nx )A=H[x+1]-H[x]; A.x+=Ds;
      B=H[x]+dH[x]*tKrat/dT;
      if( Cross( C.x,C.z=B.z, H[x].x,H[x].z,H[x].x-A.x,H[x].z-A.z ) )
      { C+=(Vector){Long/-2+(x*Long)/Nx,0,Depth/6 };
        B+=(Vector){Long/-2+(x*Long)/Nx,0,Depth/6 };
        arrow( B,C,abs(C-B)/3 );
//    glVertex3dv( C+(Vector){Long/-2+(x*Long)/Nx,0.2,Depth/6 } ); + (C.x-B.x)*tKrat/dT
      }
    } //glEnd();
  }
#endif//Tlast=Tlaps;
  TextInfo();
  return *this;
}
//
// Справка о ходе вычислительного эксперимента на текстовой консоли
//
#define Amax( n,V,S )for( W=K=0,i=0; i<n; i++ ){                  \                      if( abs( V[i] )>abs( W ) )W=V[i];           \
                      if( abs( V[i].x )>abs( K.x ) )K.x=V[i].x;   \
                      if( abs( V[i].y )>abs( K.y ) )K.y=V[i].y;   \
                      if( abs( V[i].z )>abs( K.z ) )K.z=V[i].z; } \
 printf( "\n%6.1f{ %6.1f /%g/ %-6.1f } <= [ %6.1f /%g/ %-6.1f ] -- экстремумы %s  ",abs( W ),W.x,W.y,W.z,K.x,K.y,K.z,S );

void Waves::TextInfo()
{ static bool First=true;
  if( First ){ First=false;
//  texttitle( "Вычислительный эксперимент с волнами на воде" );
    printf( "Моделирование плоской прогрессивной трохоидальной волны\n\n" );
    textcolor( YELLOW );    printf( "%6.0f \\ %.1f(%g)          -- длина[м] \\ амплитуда[м](%)\n",Length,Rw,Rw*_Pd*2.0/Length/1.134 );
    textcolor( LIGHTGREEN); printf( "%6.2f \\ %-6.4g             -- период[с] \\ круговая частота[1/с] \n",Length/Cw,_Pd*Cw/Length );
    textcolor( LIGHTCYAN ); printf( "%6.1f \\ %.4g/%.4g \\ %.3g -- скорость[м/с] \\ шаг времени[с]/крат \\ и сетки[м]\n",Cw,dT,tKrat,Ds );
  } else
  { Vector W,K; int i;
    textcolor( WHITE ); print( 1,7,"%12s \\ %d/%-6d -- время[сек] \\ шаг  \n",DtoA( Tlaps/3600 ),Kt,Kd ),clreol();
    if( abs( wH )<Long/2-2 )
    textcolor( YELLOW,BLUE ),print( 8,8,"wV=%.2f \\ wH=%-.1f -- скорость[м/с] \\ граница[м] ",wV,wH );
    textcolor( LIGHTCYAN,BLACK ); clreol(); gotoxy( 1,9 );Amax( Nx+1,H,"уровня" )
    textcolor( LIGHTGREEN ); Amax( Nx,V,"скорости" ) printf( "\n" ),clreol();
  }
}
