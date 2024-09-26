//
//      Некоторый блок процедур для записи корпуса
//      в стандартных трёхмерных графических форматах
//                                                         2019-08-22 Иерусалим
//
#include "Aurora.h"       // объекты и производные операции с корпусом на волне
static int fColor=-1;
static FILE *F=NULL;
static void printF( Vector V )
{ e6( V ); if( V.y<0 )V.y=0; fprintf( F,"v %lg %lg %lg\n",V.x,V.z,V.y );
                             fprintf( F,"v %lg %lg %lg\n",V.x,V.z,-V.y );
}
static void crossPoint( _Point A,_Point B,_Point C, int a,int b,int c )
{ if( fabs( A.Y )<eps && fabs( B.Y )<eps && fabs( C.Y )<eps )return;
  int l=A.Z<=0 && B.Z<=0 && C.Z<=0; // A.Z+B.Z+C.Z<=0.0;
  if( fColor!=l )fprintf( F,"usemtl %s\n",(fColor=l)?"green":"gray" );
                 fprintf( F,"f %d %d %d\n",a,b,c );
}
static void crossDC( Vector A,Vector B,Vector C )
{ e6( A ); e6( B ); e6( C );
  if( fabs( A.y )<eps && fabs( B.y )<eps && fabs( C.y )<eps )return;
  fprintf(F,"31 4 1 0 0 %d\n%lg %lg %lg\n%lg %lg %lg\n%lg %lg %lg\n%lg %lg %lg\n",
  A.z<=0&&B.z<=0&&C.z<=0?3:9,A.x,A.z,A.y,B.x,B.z,B.y,C.x,C.z,C.y,A.x,A.z,A.y );
}
Hull& Hull::Write( int format ) // Wavefront Technologies 4 Advanced Visualizer
{ int i,j,k,n;
  if( format==1 )        // здесь тексты сохраняются в рабочем формате UTF-8
  { if( !(F=_wfopen( U2W( fext( FileName,"obj" ) ),L"wb" )) )return *this;
   Point P,Q,q; int L[Nframes+2];
    memset( L,0,sizeof(int)*(Nframes+2) ); fColor=-1;
    //
    //     последовательная запись всего массива опорных точек в текстовый файл
    //
    fprintf( F,"# < %s >\nmtllib Aurora.mtl\n# Stern[%d]\n",
                      ShipName,n=Stern.len );                   // заголовок
    for( i=0; i<n; i++ )printF( Stern[i] ); L[0]=n*2;           // ахтерштевень
    for( k=0; k<=Nframes+1; k++ )                               // и следом все
    { if( k )L[k]=L[k-1]+n*2;                                   // шпангоуты
      fprintf( F,"# Sp%d[%d]=%d\n",k,n=Frame[k].len,L[k] );
      for( i=0; i<n; i++ )printF( Frame[k][i] );
    } L[k]=L[k-1]+n*2;
    fprintf( F,"# Stem[%d]=%d \n",n=Stem.len,L[k] );
    for( i=0; i<n; i++ )printF( Stem[i] );
    fprintf( F,"# ShipHull[%d]\n#~~~~~~~~~~\n",L[k]+n*2 );
    //
    //    формирование индексов треугольников с единым правилом обхода площадок
    //
    for( k=-1; k<2; k+=2 )
    { bool b=k<0; Flex &S=b?Stern:Stem; n=b?0:L[Nframes+2];
      fprintf( F,b?"# Stern\n":"# Stem\n" );
      for( i=0; i<S.len; i++ ){ Q=*((Point*)(&(S[i]))); //if( !b )Q.Y=-Q.Y;
        if( i>0 )
        { if( P.Y )crossPoint( P,P,Q, n+(i+1)*2 -!b, n+(i+1)*2 + b, b+n+i*2 );
          if( Q.Y )crossPoint( Q,P,Q, n+i*2     -!b, n+(i+1)*2 -!b, b+n+i*2 );
        } P=Q;
    } }
    for( k=0; k<=Nframes; k++ )                         // Nframes+1 шпангоутов
    if( Shell[k] )if( Shell[k][0]>0 )                   // -- если есть корпус
    { int nP,nQ,nq,iP,iQ,iq;                            // -- то будет и шпация
      fprintf( F,"# Sp%d\n",k );
      for( int Board=-1; Board<2; Board+=2 ) // сначала левый затем правый борт
      { Q=InSpan( nQ=k,iQ=Board,false );     // это блок стандартных шпаций
        q=InSpan( nq=k,iq=Board*2,false );   //    - переход на правый борт
        for( i=3; i<=Shell[k][0]; i++ )      // всех теоретических шпангоутов
        { P=InSpan( nP=k,iP=Board*i,false ); // попутная разборка треугольников
          if( Board<0 )crossPoint( Q,q,P,L[nQ]+iQ,L[nq]+iq,L[nP]+iP );
                  else crossPoint( q,Q,P,L[nq]+iq,L[nQ]+iQ,L[nP]+iP );
          if( Shell[k][i]&LeftFrame )nQ=nP,iQ=iP,q=P;        // перед кормовым
                              else nq=nP,iq=iP,Q=P;          // перпендикуляром
    } } }
  }
  else if( format==2 )
  { //
    //!    теперь запись в формате DС2, по варианту изготовления новых чертежей
    //
    if( !(F=_wfopen( U2W( fext( FileName,"dc2" ) ),L"wb" )) )return *this;
    //
    //  во первой строке письма общие размерения графического изображения
    //
    fprintf( F,"%g %g %g %g\n",                      // здесь задаются масштабы
                Keel[0],-Draught,Keel[Nframes-1]-Keel[0],Draught*2 );
    fprintf( F,"20 0 0 0 0 0\n1.0,0.1\n1.0\n*\n; ANSI-1251 DesignCAD Russian\n"
               "23 5 0 0 0 0\n%s\nGrid\nFrames\nStern and Stem line\n"
               "Aft and Bow breadth\n21 1\n",UtA( fname( FileName) ) );
    fprintf( F,"; Baseline\n1 %d 0.5 0 3 3 0 1 1\n",Nframes+2 );  // разметка
    for( i=0; i<Nframes+2; i++ )fprintf( F,"%lg %lg 0\n",Keel[i],Draught );
    fprintf( F,"; Frames\n" );
    for( i=1; i<=Nframes; i++ )
      fprintf( F,"1 2 0.5 0 1 14 0 1 1\n%lg %lg 0\n%lg %lg 0\n",
      Keel[i],Frame[i][0].z+Draught,Keel[i],Frame[i][Frame[i].len-1].z+Draught);
    fprintf( F,"; Plane\n1 5 0.5 0 1 14 0 1 1\n"
     "%lg %lg %lg\n%lg %lg %lg\n%lg %lg %lg\n%lg %lg %lg\n%lg %lg %lg\n",
     Keel[0],Draught,Breadth/2,Keel[Nframes+1],Draught,Breadth/2,Keel[Nframes+1],
     Draught,-Breadth/2,Keel[0],Draught,-Breadth/2,Keel[0],Draught,Breadth/2 );
    fprintf( F,"; Board box\n"
     "1 4 0.5 0 1 14 0 1 1\n%lg %lg 0\n%lg 0 0\n%lg 0 0\n%lg %lg 0\n",
     Keel[0],Draught,Keel[0],Keel[Nframes+1],Keel[Nframes+1],Draught );
    fprintf( F,"21 2\n; %d Frames\n",Nframes );
    for( j=0,k=1; k<=Nframes; )
    { for( i=n=0; i<Frame[k].len; i++ )if( Frame[k][i].x==Keel[k] )n++;
      fprintf( F,"1 %d 1 0 0 %d 0 1 2\n",n,k-Mid==0?1:12 );
      for( i=n=0; i<Frame[k].len; i++ )if( Frame[k][i].x==Keel[k] )
      { Real S=Frame[k][i].y;
        if( k-Mid<=0 && j==0 )S=-S;
        fprintf( F,"%lg %lg %lg\n",S,Frame[k][i].z+Draught,Frame[k][i].x );
      } if( k-Mid==0 && j==0 )++j; else k++;
    }

    fprintf( F,"21 3\n; Stern\n1 %d 1 0 0 8 0 1 3\n",n=Stern.len );
    for( i=0; i<n; i++ )
      fprintf( F,"%g %g %g\n",Stern[i].x,Stern[i].z+Draught,-Stern[i].y );
    fprintf( F,"; Stem\n1 %d 1 0 0 8 0 1 3\n",n=Stem.len );
    for( i=0; i<n; i++ )
      fprintf( F,"%g %g %g\n",Stem[i].x,Stem[i].z+Draught,Stem[i].y );

    fprintf( F,"21 4\n; Stern wide\n1 %d 0.5 0 1 8 0 1 4\n",n=Stern.len );
    for( i=0; i<n; i++ )
      fprintf( F,"%g %g %g\n",-Stern[i].y,Stern[i].z+Draught,Stern[i].x );
    fprintf( F,"; Stem wide\n1 %d 0.5 0 1 8 0 1 4\n",n=Stem.len );
    for( i=0; i<n; i++ )
      fprintf( F,"%g %g %g\n",Stem[i].y,Stem[i].z+Draught,Stem[i].x );
  }                                  ///
  else                               /// варианты красивой трехмерной картинки
  if( format>2 && format<5 )         ///
  { if( !(F=_wfopen( U2W( fext( FileName,"dc3" ) ),L"wb" )) )return *this;
    fprintf( F,"%g %g %g %g 0 %g %g\n",
     Keel[0],-Draught,Keel[Nframes+1]-Keel[0],Draught*2,Breadth/-2,Breadth/2 );

    fprintf( F,"20 0 0 0 0 0\n1.0,0.1\n1.0\n*\n; ANSI-1251 DesignCAD Russian\n"
               "23 6 0 0 0 0\n%s\nGrid\nPortside\nStarboard\n"
               "Stern and Stem line\nHull plating\n21 1\n; Baseline\n"
               "1 %d 0.5 0 3 3 0 1 1\n",UtA( fname( FileName ) ),Nframes+2 );
    for( i=0; i<Nframes+2; i++ )fprintf( F,"%g 0 0\n",Keel[i] );
    fprintf( F,"; Frames\n" );
    for( i=1; i<=Nframes; i++ )
    fprintf( F,"1 2 0.5 0 1 14 0 1 1\n%lg %lg 0\n%lg %lg 0\n",
             Keel[i],Frame[i][0].z,Keel[i],Frame[i][Frame[i].len-1].z );
    fprintf( F,"; Plane\n1 5 0.5 0 1 14 0 1 1\n%lg 0 %lg\n%lg 0 %lg\n"
               "%lg 0 %lg\n%lg 0 %lg\n%lg 0 %lg\n",Keel[0],Breadth/2,
             Keel[Nframes+1],Breadth/2,Keel[Nframes+1],-Breadth/2,
             Keel[0],-Breadth/2,Keel[0],Breadth/2 );
    fprintf( F,"; Board box\n1 4 0.5 0 1 14 0 1 1\n%lg 0 0\n%lg %lg 0\n"
               "%lg %lg 0\n%lg 0 0\n",Keel[0],Keel[0],-Draught,Keel[Nframes+1],
             -Draught,Keel[Nframes+1] );

    fprintf( F,"21 2\n; Starboard + gap\n" ); // борт без изъятий, но с минусом
    for( k=0; k<Nframes+2; k++ )
    { fprintf( F,"; Frame(%d)\n1 %d 1 0 0 12 0 1 2\n",k,n=Frame[k].len );
      for( i=0; i<n; i++ )
        fprintf( F,"%g %g %g\n",Frame[k][i].x,Frame[k][i].z,Frame[k][i].y );
    }
    fprintf( F,"21 3\n; Portside\n" );    // правый борт с погрызенными концами
    for( k=1; k<=Nframes; k++ )
    { for( i=n=0; i<Frame[k].len; i++ )if( Frame[k][i].x==Keel[k] )n++;
      fprintf( F,"; Frame(%d)\n1 %d 1 0 0 12 0 1 3\n",k,n );
      for( i=0; i<Frame[k].len; i++ )if( Frame[k][i].x==Keel[k] )
      fprintf( F,"%g %g %g\n",Frame[k][i].x,Frame[k][i].z,-Frame[k][i].y );
    }
    fprintf( F,"21 4\n; Stern\n1 %d 1 0 0 1 0 1 4\n",n=Stern.len );
    for( i=0; i<n; i++)fprintf(F,"%g %g %g\n",Stern[i].x,Stern[i].z,Stern[i].y);
    fprintf( F,"; Stem\n1 %d 1 0 0 1 0 1 4\n",n=Stem.len );
    for( i=0; i<n; i++)fprintf(F,"%g %g %g\n",Stem[i].x,Stem[i].z,Stem[i].y );

    if( format>3 )     //! здесь добавлена обшивка корпуса в качестве украшения
    { fprintf( F,"21 5\n; Hull plating\n" ); Vector P,Q,q;      // -- то будет и шпация
      //
      //  ...транцы на штевнях должны отрабатываться вогнутыми контурами... !!!
      //
      for( k=0; k<=1; k++ ){ Flex &S=k?Stem:Stern;
        fprintf( F,"; %s[%d]\n",k?"Stem":"Stern",S.len );
        for( i=0; i<S.len; i++ ){ Q=S[i];
          if( !k )Q.y=-Q.y;
          if( i>0 ){ if( P.y )crossDC( P,~P,~Q );
                     if( Q.y )crossDC( Q,P,~Q ); } P=Q; } }
      for( k=0; k<=Nframes; k++ )                       // Nframes+1 шпангоутов
      if( Shell[k] )if( (n=Shell[k][0])>0 )             // -- если есть корпус
      { for( int Board=-1;Board<2;Board+=2 ) // сначала левый затем правый борт
        { fprintf( F,"; Shell(%d)[%d]~%s\n",k,n,Board<0?"portside":"starboard");
          Q=Select( k,Board );               // это блок стандартных шпаций
          q=Select( k,Board*2 );             //    - переход на правый борт
          for( i=3; i<=n; i++ )              // всех теоретических шпангоутов
          { P=Select( k,Board*i );           // попутная разборка треугольников
            if( Board<0 )crossDC( Q,q,P ); else crossDC( q,Q,P );
            if( Shell[k][i]&LeftFrame )Q=P; else q=P;        // перед кормовым
    } } } }                                                  // перпендикуляром
  }
  { int D,m,d,y; char c='#',*sm,*sw,*sn=sname( FileName );
    julday( D=julday(),m,d,y ); sm=(char*)_Mnt[m-1],sw=(char*)_Day[D%7];
    if( format!=1 ){ c=';'; sn=UtA( sn ); } fprintf( F,"%c\n%c %s\n",c,c,sn );
    if( format!=1 )sm=UtA( sm ); fprintf( F,"%c %04d.%s.%02d ",c,y,sm,d );
    if( format!=1 )sw=UtA( sw ); fprintf( F,"%s%s\n",sw,DtoA(onetime(),3,":"));
  } fclose( F ); return *this;
}
/*      &Entity_Type,       Собственно тип объекта
        &Number_of_Points,  Количество точек
        &Pattern_Scale,     Масштаб для закраски
        &Line_Width,        Толщина линии
        &Line_Type,         Тип линии                (0-8)
        &The_Color          Цвет объекта             (1-16)
enum{ _0,_1=51,_2=104,_3=153,_4=204,_5=255 };
static RGB_colors DC_colors_16[16] =     //
        { { _5,_5,_5,0 },{ _0,_0,_0,0 }  //  0 - WHITE,    BLACK
        , { _0,_0,_2,0 },{ _0,_2,_0,0 }  //  2 - BROWN,    GREEN
        , { _3,_0,_0,0 },{ _0,_1,_3,0 }  //  4 - BLUE,     LIGHTBROWN
        , { _2,_3,_0,0 },{ _3,_0,_3,0 }  //  6 - CYAN,     LIGHTMAGENTA
        , { 84,84,84,0 },{ _3,_3,_3,0 }  //  8 - GRAY,     LIGHTGRAY
        , { _0,_0,_5,0 },{ _1,_3,_0,0 }  // 10 - RED,      LIGHTGREEN
        , { _5,_3,_1,0 },{ _1,_5,_5,0 }  // 12 - LIGHTBLUE,YELLOW
        , { _4,_4,_0,0 },{ _5,_2,_5,0 }  // 14 - LIGHTCYAN,LIGHTMAGENTA
        };                               */
