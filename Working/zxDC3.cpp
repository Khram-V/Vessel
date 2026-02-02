/**
 *  переинтерполяция ватерлиний к шпангоутам
 */
#include "..\Type.h"
#include "..\Math\Vector.h"
#include "..\Math\Contour.h"
#include "..\Window\ConIO.h"
#include <StdIO.h>

char *getString( FILE *F );              // Чтение строки на статическом адресе
const Real LW=1.0; // пусть это будет размер шпации
inline bool operator != ( _Point A,_Point B ){ return abs( A-B )>1e-12; }
Real e6( _Real R ){ return round( R*1e6 )/1e6; }

static int QS( const void *A1, const void *A2 )
{ Vector P=*(Vector*)A1,
         Q=*(Vector*)A2;
#if 1                                   /// это настройки на "ONR5613(vsl).dc3"
  Vector C={ 0,6.5,3.098 };
  if( P.x<-66 && P.x>-75 ){ C.y=5.5; C.z=3.23; } else
  if( P.x>60.5)C.y=0.5,C.z=0; else
  if( P.x>-26 )
  { C.y = 6.24 + 0.85*(C.x+26)/52;   /// 6.236 + 0.589
  } else C.z=-0.1;
  P.x=Q.x=0; P-=C; Q-=C;
  if( atan2( -P.y,P.z )<atan2( -Q.y,Q.z ) )return 1;
  if( atan2( -P.y,P.z )>atan2( -Q.y,Q.z ) )return -1;
#else                                    /// здесь "5415-hull.dc3"
  Vector C={ 0,16,0 };           // исходная настройка на бортовой киль
  if( P.x<18 && P.x>-70.65 )
  { if( P.x<-65 ){ C.y=6.153; C.z=3.085; }   // это будет руль
    P.x=Q.x=0; P-=C; Q-=C;
    if( atan2( -P.y,P.z )<atan2( -Q.y,Q.z ) )return 1;
    if( atan2( -P.y,P.z )>atan2( -Q.y,Q.z ) )return -1;
  } else                                         // иначе работаем с аппликатой
  { if( P.y<Q.y )return -1;                        // в КАДах Y идет аппликатой
    if( P.y>Q.y )return 1;
    if( P.z<Q.z )return -1;                          // -- и ось Z как ордината
    if( P.z>Q.z )return 1;
  }
#endif
  return 0;
}
//!
//!  Главная программа
//!
int main( int argc,char** argv )
{ Point **WL=NULL, // ватерлинии
        **SP=NULL; // шпангоуты
  Real gScale=1.0, // внутренний масштаб данных
       miX,maX;    // экстремумы всех абсцисс
  int nW=0,nF=100, // количество ватерлиний и шпангоутов
     *nS=0,        // количество точек на шпангоутах
      iD,n;        // тип цифрового объекта и его длина
  char *St;        // входная строчка
  FILE *F;         // входной текстовый файл
   SetConsoleCP( CP_UTF8 );
   SetConsoleOutputCP( CP_UTF8 );
   printf( "%s %s\n ",argv[0],argc<2?"яШар.vsl":argv[1] );
   if( argc<2 )    // без параметров делаем шар
   { const Real R=5;
     const Real EL=6.0;
     const Real N=32,dN=180.0/(2*N+1);
     F=fopen( "яШар.vsl","wt" );
     for( Real A=-90.0+dN/2; A<90.01; A+=dN )
     { Real a=R*cos( A*M_PI/180.0 ),b=R*sin( A*M_PI/180.0 );
       fprintf( F,"\n %g %7.4f ",N*2+1,b*EL );
       for( Real B=0; B<180.01; B+=180/N/2 )
       { Real c=a*cos( B*M_PI/180.0 ),d=a*sin( B*M_PI/180.0 );
         fprintf( F," %6.4f %6.4f ",c<0?-c : -R*cos( B*M_PI/180.0 ),d );   //(B-90)/90*5,d );
     } }
   }
   if( (F=fopen( argv[1],"rt" ))==NULL )return 1;
   St=getString( F  ); // какие-то экстремумы пропускаются
   do
   { St=getString( F ); if( feof( F ) )break; sscanf( St,"%d%d",&iD,&n );
                // это просто отметка номера текущего слоя
     if( iD==21 ){ printf( "\nLevel(%d) ",n ); continue; }
     else       // здесь масштаб и прочие параметры
     if( iD==42 ){ sscanf( getString( F ),"%lg",&gScale ); --n;
                   printf( "\ngScale=%g\n",gScale ); }
     else
     if( iD==1 || iD==4 )                  // остаются только сплайны и ломаные
     { int i,k;
       printf( "%d+%d,",iD,n );
       WL=(Point**)Allocate( nW+1,0,WL );
       WL[nW]=(Point*)Allocate( n*sizeof(Point) );
       for( i=0; i<n; i++ )
       { St=getString( F );
         sscanf( St,"%lg%lg%lg",&(WL[nW][i].X),&(WL[nW][i].Y),&(WL[nW][i].Z) );
         WL[nW][i].Z=-WL[nW][i].Z;
         WL[nW][i]=WL[nW][i]/gScale;
//!      if( WL[nW][i].Y<0.0 )WL[nW][i].Y=0.0;
//!      if( WL[nW][i].Y>0.1 )WL[nW][i].Y+=0.01;
         if( !nW && !i )miX=maX=WL[0][0].X; else // установление максимал.длины
         { if( miX>WL[nW][i].X )miX=WL[nW][i].X;
           if( maX<WL[nW][i].X )maX=WL[nW][i].X;
       } } ++nW; n=0;
     }
     for( int i=0; i<n; i++ )getString( F ); // пропуск неиспользованных строк
   } while( !feof( F ) );
   fclose( F );
   //
   //   контрольный вывод числового массива в текстовый файл
   //
/* printf( "\n\nВыполняется запись в текстовый файл «%s» %d ватерлиний\n"
           "  длина парохода: %0.2f < %0.2f > %0.2f \n",
           fext( argv[1],"lst" ),nW,miX,maX-miX,maX );
   if( (F=fopen( argv[1],"wt" ))==NULL )return 2;
   for( int k=0; k<nW; k++ )
   { fprintf( F,"%4d ",n=N(k) );
     for( int i=0; i<n; i++ )
     fprintf( F," %7.3f %5.2f %6.3f",WL[k][i].X,WL[k][i].Y,WL[k][i].Z );
     fprintf( F,"\n" );
   } fclose( F );
*/ //
   //  формирование шпангоутных ребер
   //
  int i,j,k,l,m;
  Contour *W=(Contour*)Allocate( nW*sizeof(Contour) );
   for( k=0; k<nW; k++ )                            // ~бесконструкторская~
     ( W[k]=Contour( WL[k] ) ); ///.Mode( easy,true ); // сборка левых векторов
   SP=(Point**)Allocate( nF=1+(floor(maX)-ceil(miX))/LW,0,SP );
   nS=(int*)Allocate( nF*sizeof(int) );
   printf( "\nСформирован массив шпангоутных контуров, количество: %d \n",nF );
   for( i=0; i<nF; i++ )
   { const Real X=(i+ceil( miX/LW ))*LW;
     for( m=k=0; k<nW; k++ )
     for( j=0; j<W[k].len-1; j++ )
      if( (X-W[k][j].X)*(X-W[k][j+1].X)<=0.0 )++m;
                                                     // printf( "%g=%d ",X,m );
     if( m )                             // и после подсчета повтор с действием
     { SP[i]=(Point*)Allocate( (nS[i]=m)*sizeof(Point) ); m=0;
       for( k=0; k<nW; k++ )
       for( j=0; j<W[k].len-1; j++ )
       if( (X-W[k][j].X)*(X-W[k][j+1].X)<=0.0 )
       { Point V = W[k][j] + ( W[k][j+1]-W[k][j] )
                           * ( (X-W[k][j].X)/(W[k][j+1].X-W[k][j].X) );
         SP[i][m++]=V;
       }
       for( j=1,k=0; j<m; j++ ) // совпадающие точки в подводной части снимаются
       { if( SP[i][k]!=SP[i][j] )k++; if( k!=j )SP[i][k]=SP[i][j]; }
       nS[i]=m=k+1;                       // реальная длина контура шпангоута
     }
     qsort( SP[i],m,sizeof(Point),QS );   // это для совсем безыскусных обводов
   }
     //
     //  для начала ищем нижнюю и верхнюю точки на диаметральной плоскости
     //      такие точки должны быть обязательно, или то - ошибка в данных
     //
   printf("\nВыполняется запись таблицы ординат корпуса «%s»\n",fext(argv[1],"txt"));
   if( (F=fopen( argv[1],"wt" ))==NULL )return 3;
   for( i=0; i<nF; i++ )
   { n=nS[i];          fprintf( F,"%4d %3.5g ",n,e6((i+ceil( miX/LW ))*LW) );
     for( k=0;k<n;k++ )fprintf(F," %6.5g %6.5g",e6(SP[i][k].Y),e6(SP[i][k].Z));
     fprintf( F,"\n" );
   } fprintf( F,"\n ==== \n\n" );
   printf( "   получаются те же %d шпангоутов\n"
           "   и запись всех линий с изменчивой аппликатой \n\n",nF );
   for( int k=0; k<nW; k++ )
   if( fabs( W[k][(n=W[k].len)-1].Z)<1e4 && fabs(W[k][0].Z)<1e4 )
   { bool a=WL[k][0].Y>WL[k][n-1].Y;  //WL[k][0].X>(maX+miX)/2;
     fprintf( F,"%4d ",n );
     for( int i=0; i<n; i++ )fprintf( F," %6.5g %7.5g",WL[k][a?n-i-1:i].Y,WL[k][a?n-i-1:i].X );
     fprintf( F,"\n%4d ",n );
     for( int i=0; i<n; i++ )fprintf( F," %6.5g %7.5g",WL[k][a?n-i-1:i].Y,WL[k][a?n-i-1:i].Z );
     fprintf( F,"\n\n" );
   } fclose( F );
}
