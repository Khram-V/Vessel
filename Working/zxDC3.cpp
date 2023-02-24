/**
 *  переинтерполяция ватерлиний к шпангоутам
 */
#include <StdIO.h>
#include "..\Type.h"
#include "..\Math\Vector.h"
#include "..\Window\ConIO.h"
#include "Contour.h"

const Real LW=1.0; // пусть это будет размер шпации
#define N(k) ( isAlloc( WL[k] )/sizeof( Point ) )
static int QS( const void *A1, const void *A2 )
{ Vector &P=*(Vector*)A1,
         &Q=*(Vector*)A2;
  if( P.y<Q.y )return -1;                          // в КАДах Y идет аппликатой
  if( P.y>Q.y)return 1;  if( P.z<Q.z )return -1;   // -- и ось Z как ордината
                         if( P.z>Q.z )return 1; return 0;
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
     { printf( "%d+%d,",iD,n );
       WL=(Point**)Allocate( nW+1,0,WL );
       WL[nW]=(Point*)Allocate( n*sizeof(Point) );
       for( int i=0; i<n; i++ )
       { St=getString( F );
         sscanf( St,"%lg%lg%lg",&(WL[nW][i].X),&(WL[nW][i].Y),&(WL[nW][i].Z) );
         WL[nW][i].Z=-WL[nW][i].Z;
         WL[nW][i]/=gScale;
//!      if( WL[nW][i].Y<0.0 )WL[nW][i].Y=0.0;
//!      if( WL[nW][i].Y>0.1 )WL[nW][i].Y+=0.01;
         if( !nW && !i )miX=maX=WL[0][0].X; else // установление максимал.длины
         { if( miX>WL[nW][i].X )miX=WL[nW][i].X;
           if( maX<WL[nW][i].X )maX=WL[nW][i].X;
         }
       } ++nW; n=0;
     }
     for( int i=0; i<n; i++ )getString( F ); // пропуск неиспользованных строк
   } while( !feof( F ) );
   fclose( F );
   //
   //   контрольный вывод числового массива в текстовый файл
   //
   printf( "\n\nВыполняется запись в текстовый файл «%s» %d ватерлиний\n"
           "  длина парохода: %0.2f < %0.2f > %0.2f \n",
           fext( argv[1],"txt" ),nW,miX,maX-miX,maX );
   if( (F=fopen( argv[1],"wt" ))==NULL )return 2;
   for( int k=0; k<nW; k++ )
   { fprintf( F,"%4d ",n=N(k) );
     for( int i=0; i<n; i++ )
     fprintf( F," %7.3f %5.2f %6.3f",WL[k][i].X,WL[k][i].Y,WL[k][i].Z );
     fprintf( F,"\n" );
   } fclose( F );
   //
   //  формирование шпангоутных ребер
   //
  Contour *W=(Contour*)Allocate( nW*sizeof(Contour) );
   for( int k=0; k<nW; k++ )                            // ~бесконструкторская~
      (W[k]=Contour( WL[k] )); ///.Mode( easy,true );  // сборка левых векторов
   SP=(Point**)Allocate( nF=1+(floor(maX)-ceil(miX))/LW,0,SP );
   nS=(int*)Allocate( nF*sizeof(int) );
   printf( "\nСформирован массив шпангоутных контуров, количество: %d \n",nF );
   for( int i=0; i<nF; i++ )
   { Real X=(i+ceil( miX/LW ))*LW;
     int m=0,j=0;
     for( int k=0; k<nW; k++ )if( (X-W[k][0].X)*(X-W[k][N(k)-1].X)<=0.0 )++m; // printf( "%g=%d ",X,m );
     if( m )                          // и после подсчета повтор с действием
     { SP[i]=(Point*)Allocate( (nS[i]=m)*sizeof(Point) );
       for( int k=0; k<nW; k++ )
       if( (X-W[k][0].X)*(X-W[k][N(k)-1].X)<=0.0 )SP[i][j++]=W[k].forX( X );
       if( X-SP[i][j-1].X!=0.0 )printf( "%g[%d]=%g ",X,i,X-SP[i][j-1].X );
     }
     qsort( SP[i],m,sizeof(Point),QS );
   }
   //
   //  после сортировки необходима проверка впадающих внутрь точек
   //
#if 0
   bool L=false;
   int i,j,k;
   for( i=0; i<nF; i++ )
   { int n=isAlloc( SP[i] )/sizeof(Point);
     for( j=0,k=1; k<n; k++ )
     { //if( SP[i][k].Z>16.099 )SP[i][k].Z=16.1;
       if( abs( SP[i][k]-SP[i][j] )<1e-6 ){ SP[i][j=k]=10000; --nS[i]; } else
       if( abs( SP[i][k]-SP[i][j] )<0.001 && k<n-1 )
       { if( !L )L=true;
         else{ SP[i][j]=10000; --nS[i]; }
       } else j=k,L=false;       // это сброс числа
   } }
#endif
   printf("\nВыполняется запись таблицы ординат корпуса «%s»\n",fext(argv[1],"vsl"));
   if( (F=fopen( argv[1],"wt" ))==NULL )return 3;
   for( int i=0; i<nF; i++ )
   { int x=0;
     for( x=0; x<nS[i]; x++ )if( SP[i][x].Z>0 )break;
     fprintf( F,"%4d %3.3g ",nS[i]-(x>0?x-1:0),(i+ceil( miX/LW ))*LW );
     n=isAlloc( SP[i] )/sizeof(Point);
     for( int k=(x>0?x-1:0); k<n; k++ ) //!!! здесь ручная настройки оптимизация форматов
     if( SP[i][k].X<9999 )
     fprintf( F,!k || k==n-1?" %6.3f %6.3f":" %6.3f %6.3f",SP[i][k].Y,SP[i][k].Z );
     fprintf( F,"\n" );
   } fprintf( F,"\n ==== \n\n" );
   printf( "   получаются те же %d шпангоутов\n"
           "   и запись всех линий с изменчивой аппликатой \n\n",nF );
   for( int k=0; k<nW; k++ )
   if( fabs( W[k][(n=N(k))-1].Z)<1e4 && fabs(W[k][0].Z)<1e4 )
   { //qsort( WL[k],n,sizeof(Point),QS ); // та же сортировка по аппликате
    bool a=WL[k][0].Y>WL[k][n-1].Y;  //WL[k][0].X>(maX+miX)/2;
     fprintf( F,"%4d ",n );
     for( int i=0; i<n; i++ )fprintf( F," %6.3f %7.3f",WL[k][a?n-i-1:i].Y,WL[k][a?n-i-1:i].X );
     fprintf( F,"\n%4d ",n );
     for( int i=0; i<n; i++ )fprintf( F," %6.3f %7.3f",WL[k][a?n-i-1:i].Y,WL[k][a?n-i-1:i].Z );
     fprintf( F,"\n\n" );
   } fclose( F );
}
