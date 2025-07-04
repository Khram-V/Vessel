//
//      Некоторый блок процедур для записи корпуса
//      в стандартных трёхмерных графических форматах
//                                                         2019-08-22 Иерусалим
//
#include "Aurora.h"       // объекты и производные операции с корпусом на волне
static int fColor=-1;
static FILE *F=NULL;
static Real T=0.0,St=0.0;    // действующее смещение для приведения к оригиналу
static void e5R( Real &R ){ R-=remainder( R,1e-5L ); } // if( fabs( R )<=1e-5 )R=0; else
static void e5V( Vector &V ){ e5R( V.x ); e5R( V.y ); e5R( V.z ); }
static void printF( Vector V )
{ e5V( V ); if( V.y<0 )V.y=0; fprintf( F,"v %lg %lg %lg\n",V.x,V.z,V.y );
                              fprintf( F,"v %lg %lg %lg\n",V.x,V.z,!V.y?0:-V.y );
}
static void crossPoint( _Vector A, _Vector B, _Vector C, int a, int b, int c )
{ if( abs( (A-C)*(B-C) )<eps )return;                          // малая площадь
  if( fabs( A.y )<eps && fabs( B.y )<eps && fabs( C.y )<eps )return;    // В ДП
  if( A.z*B.z<0 || B.z*C.z<0 || C.z*A.z<0 )return;                 // через КВЛ
  int l=A.z<=0 && B.z<=0 && C.z<=0; // A.Z+B.Z+C.Z<=0.0;
  if( fColor!=l )fprintf( F,"usemtl %s\n",(fColor=l)?"green":"gray" );
                 fprintf( F,"f %d %d %d\n",a,b,c );
}
static void crossDC( Vector A, Vector B, Vector C )
{ e5V( A ); e5V( B ); e5V( C );
  if( fabs( A.y )<eps && fabs( B.y )<eps && fabs( C.y )<eps )return;
  fprintf(F,"31 4 1 0 0 %d\n%lg %lg %lg\n%lg %lg %lg\n%lg %lg %lg\n%lg %lg %lg\n",
  A.z<=0&&B.z<=0&&C.z<=0?3:9,
       A.x+St,A.z+T,A.y,B.x+St,B.z+T,B.y,C.x+St,C.z+T,C.y,A.x+St,A.z+T,A.y );
}
Hull& Hull::Write( int format )
{
 int i,j,k,n,*oL=(int*)Allocate( (Nframes+3)*sizeof( int ) );
 string FName; FName<<FileName;
  T=Ofs.z; St=Ofs.x;
  if( !format )                                    // Free!Ship exchange format
  { if( !(F=_wfopen( U2W( fext( FName,"fef" ) ),L"wb" )) )return *this;
    fprintf( F,"%s\n"      // ProjectName
            "Создатель:\n" // ProjectDesigner
         "©«Аврора»: %s\n" // ProjectFileCreatedBy  ~\_
             "Описание:\n" // ProjectComment        ~/
             "%lg %lg %lg" // длина ширина осадка
              " 1.025"     // WaterDensity
              " 1.0"       // AppendageCoefficient
              " 0"         // ProjectUnits: Metric,Imperial
              " 1"         // MainparticularsHasBeenset = true
              " 0\n"       // Precision: fpLow-Medium-High-VeryHigh
              "2\n"        // NumberOfLayers количество слоёв цифровой модели
        "Обводы корпуса\n" // название слоя пусть будет на Русском
              "0"          // Layer[I].LayerID - номер слоя дополняет название
              " $8000"     // Подводный зелёный
              " 1"         // SurfaceVisible: 0 - поверхность будет прозрачной
              " 0"         // Developable - разворачиваемость для раскроя
              " 1"         // Symmetric - дополнение модели правым бортом
              " 1"         // UseForIntersections - для теоретических чертежей
              " 1"         // UseInHydrostatics - включение в гидростатику
              " 1"         // ShowInLinesplan - ну такое уж прям слишком....
              " 0.0"       // MaterialDensity плотность металла\дерева обшивки
              " 0.0\n"     // Thickness обшивки для дополнения массы
    "Борт и архитектура\n" // название слоя пусть будет на Русском = FreeBoard
              "1"          // Layer[I].LayerID - номер слоя дополняет название
              " $EEDDCC"   // 16777200 светло-шаровый цвет - 32768 - подводный
              " 1 0 1 1 1 1 0.0 0.0\n",
              ShipName, fname( FileName ), Length, Breadth, Draught );
  struct Faces{ int Len,N,*T;
    Faces(){ Len=N=0; T=NULL; } ~Faces(){ if( T )Allocate( 0,T ); }
    void Add(_Vector A,_Vector B,_Vector C,int a,int b,int c,bool Board=false )
    { if( abs( (A-C)*(B-C) )<eps )return;
      if( Len<=N+8 )T=(int*)Allocate( 4*(Len+=40)*sizeof(int),T ); // 30 в запасе
      if( Board )T[N*4]=a,T[N*4+1]=b,T[N*4+2]=c;
            else T[N*4]=a,T[N*4+1]=c,T[N*4+2]=b;
      T[N*4+3]=A.z+B.z+C.z<=0?0:1; ++N; // 0 обводы 1 общекорабельная архитектура
    } } Tri;
#define Vertex " %lg 3 0\n"
   Vector P,Q,V;
   long nt=0,kf=ftell( F );   // фиксируется метка для количества треугольников
    fprintf( F,"     \n" ); // под будущую перезапись количества точек "%3d",nt
    for( k=0; k<2; k++ )
    { Flex &S=k?Stem:Stern;
      for( i=0; i<S.len; i++ )   // VertexType={svRegular-Crease-Dart-Corner}
      { e5V( P=S[i] );
        if( !i )
        { if( P.y>0 )
          { fprintf( F,"%lg %lg"Vertex"%lg 0"Vertex,P.x,P.y,P.z,P.x,P.z );
            nt+=2;
        } } else
        if( Q.y<=0 && P.y>0 )
        { fprintf( F,"%lg 0"Vertex"%lg %lg"Vertex"%lg 0"Vertex,
                   Q.x,Q.z, P.x,P.y,P.z, P.x,P.z );   // один новый треугольник
          Tri.Add( (Vector){Q.x,0,Q.z},P,(Vector){P.x,0,P.z},
                   nt,nt+2,nt+1,!k ); nt+=3;          // три новые точки
        } else
        if( Q.y>0 && P.y<=0 )
        { fprintf( F,"%lg 0"Vertex,P.x,P.z );  // один старый треугольник
          Tri.Add( Q,P,(Vector){P.x,0,P.z},
                  nt-2,nt-1,nt,!k ); ++nt;          // одна новая точка
        } else
        if( Q.y>0 && P.y>0 )
        { fprintf( F,"%lg %lg"Vertex"%lg 0"Vertex,P.x,P.y,P.z,P.x,P.z );
          V=P;V.y=0;Tri.Add( Q,P,V,nt-2,nt-1,nt,!k ); // два косых треугольника
          V=Q;V.y=0;Tri.Add( Q,P,V,nt,nt-1,nt+1,!k ); nt+=2; // две новые точки
        } Q=P;
      }
    }                           // начинается запись узлов по корпусной обшивке
    oL[0]=nt;
    for( k=0; k<=Nframes+1; k++ )
    { for( i=0; i<Frame[k].len; i++ )
      { e5V( V=Frame[k][i] ); fprintf( F,"%g %g"Vertex,V.x,V.y,V.z ); ++nt;
      } oL[k+1]=nt;
    }
    for( k=0; k<=Nframes; k++ )                // Nframes+1 шпангоутов
    if( Shell[k] )if( Shell[k][0]>0 )          // -- если есть корпус
    { int A,B,C,K,M; // nP,nQ,nq,iP,iQ,iq;             // -- то будет и шпация
      Q=InSpan( K=k,M=1,false ); A=oL[K]+(M-1)/2; // это блок стандартных шпаций
      V=InSpan( K=k,M=2,false ); B=oL[K]+(M-1)/2; //    - переход на правый борт
      for( i=3; i<=Shell[k][0]; i++ )          // всех теоретических шпангоутов
      { P=InSpan( K=k,M=i,false ); C=oL[K]+(M-1)/2; // попутная разборка треугольников
        if( Shell[k][i]&LeftFrame ){ Tri.Add( Q,V,P,A,B,C ); A=C,Q=P; } // перед кормовым
                             else  { Tri.Add( V,P,Q,B,C,A ); B=C,V=P; } // перпендикуляром
    } }
    fprintf( F,"0\n%d\n",Tri.N );
    for( i=0; i<Tri.N*4; i+=4 )
         fprintf( F,"3 %d %d %d %d 0\n",Tri.T[i],Tri.T[i+1],Tri.T[i+2],Tri.T[i+3] );
    fseek( F,kf,SEEK_SET ); fprintf( F,"%3d",nt );
    fseek( F,0,SEEK_END );  fprintf( F,"\n" );
  }
  else            //! Wavefront Technologies 4 Advanced Visualizer
  if( format==1 ) // здесь также тексты .obj сохраняются в рабочем формате UTF8
  { if( !(F=_wfopen( U2W( fext( FName,"obj" ) ),L"wb" )) )return *this;
   Vector P,Q,q;
    fColor=-1;
    //
    //     последовательная запись всего массива опорных точек в текстовый файл
    //
    fprintf( F,"# < %s >\nmtllib Aurora.mtl\n# Stern[%d]\n",
                      ShipName,n=Stern.len );                   // заголовок
    for( i=0; i<n; i++ )printF( Stern[i] ); oL[0]=n*2;           // ахтерштевень
    for( k=0; k<=Nframes+1; k++ )                               // и следом все
    { fprintf( F,"# Sp%d[%d]=%d\n",k,n=Frame[k].len,oL[k] );     // шпангоуты
      for( i=0; i<n; i++ )printF( Frame[k][i] );
      oL[k+1]=oL[k]+n*2;
    }
    fprintf( F,"# Stem[%d]=%d \n",n=Stem.len,oL[k] );
    for( i=0; i<n; i++ )printF( Stem[i] );
    fprintf( F,"# ShipHull[%d]\n#~~~~~~~~~~\n",oL[k]+n*2 );
    //
    //    формирование индексов треугольников с единым правилом обхода площадок
    //
    for( k=0; k<2; k++ )
    { Flex &S=k?Stem:Stern; n=(k?oL[Nframes+2]:0)-1;        // -1 для учёта i<0
      fprintf( F,k?"# Stem\n":"# Stern\n" );
      for( i=0; i<S.len; i++ ){ Q=S[i];                    // if( !b )Q.y=-Q.y;
        if( i>0 )                                          // P.z|Q.z != 0 ???
        { crossPoint( P,P,Q, n+i*2+k,n+i*2+1-k,n+i*2+3 );  // P.z
          crossPoint( P,Q,Q, n+i*2,n+i*2+3-k,n+i*2+2+k );  // Q.z
        } P=Q;
    } }
    for( k=0; k<=Nframes; k++ )                         // Nframes+1 шпангоутов
    if( Shell[k] )if( Shell[k][0]>0 )                   // -- если есть корпус
    { int A,B,C,K,M;                                    // -- то будет и шпация
      fprintf( F,"# Sp%d\n",k );
      for( int Board=-1; Board<2; Board+=2 ) // сначала левый затем правый борт
      { Q=InSpan( K=k,M=Board,false );   A=oL[K]+M;  // блок стандартных шпаций
        q=InSpan( K=k,M=Board*2,false ); B=oL[K]+M; //- переход на правый борт
        for( i=3; i<=Shell[k][0]; i++ )        // всех теоретических шпангоутов
        { P=InSpan( K=k,M=Board*i,false ); C=oL[K]+M; // попутная разборка
          if( Board<0 )crossPoint( Q,q,P,A,B,C );     // треугольников
                  else crossPoint( q,Q,P,B,A,C );
          if( Shell[k][i]&LeftFrame )A=C,q=P;        // перед кормовым
                                else B=C,Q=P;        // перпендикуляром
    } } }
  }
  //!    теперь запись в формате DС2, по варианту изготовления новых чертежей
  //
  else if( format==2 )
  { if( !(F=_wfopen( U2W( fext( FName,"dc2" ) ),L"wb" )) )return *this;
    //
    //  во первой строке письма общие размерения графического изображения
    //
    fprintf( F,"%g 0 %g %g\n",                      // здесь задаются масштабы
                Keel[0],Keel[Nframes+1]-Keel[0],T*2 );
    fprintf( F,"20 0 0 0 0 0\n1.0,0.1\n1.0\n*\n; ANSI-1251 DesignCAD Russian\n"
               "23 5 0 0 0 0\n%s\nGrid\nFrames\nStern and Stem line\n"
               "Aft and Bow breadth\n21 1\n",UtWin( fname( FileName) ) );
    fprintf( F,"; Baseline\n1 %d 0.5 0 3 3 0 1 1\n",Nframes+2 );    // разметка
    for( i=0; i<Nframes+2; i++ )fprintf( F,"%lg %lg 0\n",Keel[i],T );
    fprintf( F,"; Frames\n" );
    for( i=1; i<=Nframes; i++ )
      fprintf( F,"1 2 0.5 0 1 14 0 1 1\n%lg %lg 0\n%lg %lg 0\n",
      Keel[i],Frame[i][0].z+T,Keel[i],Frame[i][Frame[i].len-1].z+T);
    fprintf( F,"; Plane\n1 5 0.5 0 1 14 0 1 1\n"
     "%lg %lg %lg\n%lg %lg %lg\n%lg %lg %lg\n%lg %lg %lg\n%lg %lg %lg\n",
     Keel[0],T,Breadth/2,Keel[Nframes+1],T,Breadth/2,Keel[Nframes+1],
     T,-Breadth/2,Keel[0],T,-Breadth/2,Keel[0],T,Breadth/2 );
    fprintf( F,"; Board box\n"
     "1 4 0.5 0 1 14 0 1 1\n%lg %lg 0\n%lg 0 0\n%lg 0 0\n%lg %lg 0\n",
     Keel[0],T,Keel[0],Keel[Nframes+1],Keel[Nframes+1],T );
    fprintf( F,"21 2\n; %d Frames\n",Nframes );
    for( j=0,k=1; k<=Nframes; )
    { for( i=n=0; i<Frame[k].len; i++ )if( Frame[k][i].x==Keel[k] )n++;
      fprintf( F,"1 %d 1 0 0 %d 0 1 2\n",n,k-Mid==0?1:12 );
      for( i=n=0; i<Frame[k].len; i++ )if( Frame[k][i].x==Keel[k] )
      { Real S=Frame[k][i].y;
        if( k-Mid<=0 && j==0 )S=-S;
        fprintf( F,"%lg %lg %lg\n",S,Frame[k][i].z+T,Frame[k][i].x );
      } if( k-Mid==0 && j==0 )++j; else k++;
    }

    fprintf( F,"21 3\n; Stern\n1 %d 1 0 0 8 0 1 3\n",n=Stern.len );
    for( i=0; i<n; i++ )
      fprintf( F,"%g %g %g\n",Stern[i].x,Stern[i].z+T,-Stern[i].y );
    fprintf( F,"; Stem\n1 %d 1 0 0 8 0 1 3\n",n=Stem.len );
    for( i=0; i<n; i++ )
      fprintf( F,"%g %g %g\n",Stem[i].x,Stem[i].z+T,Stem[i].y );

    fprintf( F,"21 4\n; Stern wide\n1 %d 0.5 0 1 8 0 1 4\n",n=Stern.len );
    for( i=0; i<n; i++ )
      fprintf( F,"%g %g %g\n",-Stern[i].y,Stern[i].z+T,Stern[i].x );
    fprintf( F,"; Stem wide\n1 %d 0.5 0 1 8 0 1 4\n",n=Stem.len );
    for( i=0; i<n; i++ )
      fprintf( F,"%g %g %g\n",Stem[i].y,Stem[i].z+T,Stem[i].x );
  }                                  ///
  else                               /// варианты красивой трехмерной картинки
  if( format>2 && format<5 )         ///
  { if( !(F=_wfopen( U2W( fext( FName,"dc3" ) ),L"wb" )) )return *this;
    fprintf( F,"%g 0 %g %g 0 %g %g\n",
             Keel[0]+St,Keel[Nframes+1]-Keel[0],T*2,Breadth/-2,Breadth );
    fprintf( F,"20 0 0 0 0 0\n1.0,0.1\n1.0\n*\n; ANSI-1251 DesignCAD Russian\n"
               "23 6 0 0 0 0\n%s\nGrid\nPortside\nStarboard\n"
               "Stern and Stem line\nHull plating\n21 1\n; Baseline\n"
               "1 %d 0.5 0 3 3 0 1 1\n",UtWin( fname( FileName ) ),Nframes+2 );
    for( i=0; i<Nframes+2; i++ )fprintf( F,"%g 0 0\n",Keel[i]+St );
    fprintf( F,"; Frames\n" );
    for( i=1; i<=Nframes; i++ )
    fprintf( F,"1 2 0.5 0 1 14 0 1 1\n%lg %lg 0\n%lg %lg 0\n",
             Keel[i]+St,Frame[i][0].z+T,Keel[i]+St,Frame[i][Frame[i].len-1].z+T );
    fprintf( F,"; Plane\n1 5 0.5 0 1 14 0 1 1\n%lg 0 %lg\n%lg 0 %lg\n"
               "%lg 0 %lg\n%lg 0 %lg\n%lg 0 %lg\n",Keel[0]+St,Breadth/2,
             Keel[Nframes+1]+St,Breadth/2,Keel[Nframes+1]+St,-Breadth/2,
             Keel[0]+St,-Breadth/2,Keel[0]+St,Breadth/2 );
    fprintf( F,"; Board box\n1 4 0.5 0 1 14 0 1 1\n%lg 0 0\n%lg 0 0\n"
               "%lg 0 0\n%lg 0 0\n",Keel[0]+St,Keel[0]+St,Keel[Nframes+1]+St,
             Keel[Nframes+1]+St );

    fprintf( F,"21 2\n; Starboard + gap\n" ); // борт без изъятий, но с минусом
    for( k=0; k<Nframes+2; k++ )
    { fprintf( F,"; Frame(%d)\n1 %d 1 0 0 12 0 1 2\n",k,n=Frame[k].len );
      for( i=0; i<n; i++ )
      fprintf( F,"%g %g %g\n",Frame[k][i].x+St,Frame[k][i].z+T,Frame[k][i].y );
    }
    fprintf( F,"21 3\n; Portside\n" );    // правый борт с погрызенными концами
    for( k=1; k<=Nframes; k++ )
    { for( i=n=0; i<Frame[k].len; i++ )if( Frame[k][i].x==Keel[k] )n++;
      fprintf( F,"; Frame(%d)\n1 %d 1 0 0 12 0 1 3\n",k,n );
      for( i=0; i<Frame[k].len; i++ )if( Frame[k][i].x==Keel[k] )
      fprintf( F,"%g %g %g\n",Frame[k][i].x+St,Frame[k][i].z+T,-Frame[k][i].y );
    }
    fprintf( F,"21 4\n; Stern\n1 %d 1 0 0 1 0 1 4\n",n=Stern.len );
    for( i=0; i<n; i++)fprintf(F,"%g %g %g\n",Stern[i].x+St,Stern[i].z+T,Stern[i].y);
    fprintf( F,"; Stem\n1 %d 1 0 0 1 0 1 4\n",n=Stem.len );
    for( i=0; i<n; i++)fprintf(F,"%g %g %g\n",Stem[i].x+St,Stem[i].z+T,Stem[i].y );

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
  } Allocate( 0,oL );
  { int D,m,d,y; char c='#',*sm,*sw,*sn=sname( FileName );  // UtWin - в локали
    julday( D=julday(),m,d,y ); sm=(char*)_Mnt[m-1],sw=(char*)_Day[D%7];
    if( format>1 ){ c=';'; sn=UtWin(sn); } fprintf( F,"%c\n%c %s\n",c,c,sn );
    if( format>1 )sm=UtWin(sm); fprintf( F,"%c %04d.%s.%02d ",c,y,sm,d );
    if( format>1 )sw=UtWin(sw); fprintf( F,"%s%s\n",sw,DtoA(onetime(),3,":"));
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
