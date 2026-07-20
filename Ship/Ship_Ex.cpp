
#include "Ship.h"

bool Ship::Import( fixed Fmt )
{ isBin=false;             // чисто текстовое представление числовой информации
  if( !(FM=_wfopen( FName,L"rt" ) ) )
  { print( "?\7 не открывается инородный %s ",W2U(FName) ); getch(); exit( 2 );
  }
 time_t lt=time(0); char *S=asctime( gmtime( &lt ) ); strcut( S );
  if( !Set.Name    )Set.Name=strdup( fname( Name ) ); // ~ без вычистки
  if( !Set.Designer)Set.Designer="@2026-Ship.exe viewer for „free!Ship“";
  if( !Set.Comment )Set.Comment="Application for «Аврора» stormy seakeeping of ship";
  if( !Set.CreatedBy)Set.CreatedBy=S;
//if( !Set.CreatedBy)Set.CreatedBy=ctime( &lt );
  if( Fmt==2 )Shell.ReadStl( W2U( FName ) ); else
  if( Fmt==1 )Shell.ReadObj( W2U( FName ) );
  fclose( FM ); FM=NULL;
//Visio.BothSides=false;
//BoardView( Visio.ModelView )=mvPort;
  Visio.ModelView=mvPort;
  Shell.Extents( false );       // расчёт - переопределение графических экстремумов
  if( Min.z<0 && Max.z>0 )Draft=-Min.z;
  return true;
}
void Surface::ReadObj( char *Path )           // временный оригинал имени файла
{ char *S,*Name=strdup( Path ); Real r,g,b,a; char *s; // ссылка не текст в буфере файла
  print( "\nОткрыт WaveFront файл: %s",Name );
 int NoL=NoLayers,                      // уровни будут дополняться сверху
     NoC=NoCoPoint-1;                   // узловые точки отделяются от прошлого
  ActiveLayer.Description="WaveFront";  // Technologies Advanced Visualizer";
  ActiveLayer.ID=NoL;                   // изначально здесь ноль
  ActiveLayer.Symmetric=false;          // пока без правого дублирования
  if( !NoLayers )  // на случай отсутствия послойного описания свойств, будет 1
  { L=(Layers*)Allocate( (NoLayers+1)*sizeof( Layers ),L );
    memcpy( &L[NoLayers],&ActiveLayer,sizeof( Layers ) );
  }
  while( !feof( FM ) )
  { if( (s=strchr( S=getString( FM ),'#' ))!=NULL )*s=0;
    if( strcut( S )<3 )continue;
    S[0]=tolower( S[0] );        // сначала первый символ, а затем и вся строка
    if( !strncmp( S,"v ",2) )
    { P=(CoPoint*)Allocate( ++NoCoPoint*sizeof( CoPoint ),P );
     CoPoint &p=P[NoCoPoint-1];
//    sscanf( S+2,"%lg%lg%lg",&p.V.y,&p.V.x,&p.V.z ); p.V.x=-p.V.x; // Новик здесь
      sscanf( S+2,"%lg%lg%lg",&p.V.x,&p.V.z,&p.V.y ); // так готовится в Авроре #+# p.V.y=-p.V.y;
      p.T=svRegular; // svCrease; // svDart; // svCorner;
    } else
    if( !strncmp( S,"f ",2) )
    { char *z,*w=S+2;
      int k=0,*Rc=(int*)calloc( sizeof( int ),4 ); // Allocate не для маленьких
      do{ s=strchr( w,' ' ); if( s )*s=0;
          z=strchr( w,'/' ); if( z )*z=0;
          if( k>3 )Rc=(int*)realloc( Rc,sizeof(int)*(k+1) );
          while( *w<=' ' )w++;
          Rc[k]=atoi( w )+NoC; w=s+1; ++k;  // нормали получаются задом наперёд
      } while( s );
      if( k>2 )                             // наверняка прямые так не строятся
      { F=(Faces*)Allocate( ++NoFaces*sizeof(Faces),F );
        Faces &f=F[NoFaces-1]; f.Capacity=k;
                               f.P=Rc;
                               f.LayerIndex=ActiveLayer.ID;
        L[ActiveLayer.ID].ID++;
      }
/**  здесь не всегда только три точки
     int a,b,c;
      s=strchr(w,' '); *s=0; z=strchr(w,'/'); if(z)*z=0; a=atoi(w); w=s+1;
      s=strchr(w,' '); *s=0; z=strchr(w,'/'); if(z)*z=0; b=atoi(w); w=s+1;
                             z=strchr(w,'/'); if(z)*z=0; c=atoi(w);
      if( a!=b && b!=c && c!=a )
      { F=(Faces*)Allocate( ++NoFaces*sizeof(Faces),F );
        Faces &f=F[NoFaces-1]; f.Capacity=3;
                               f.P=(int*)Allocate( 3*sizeof(int) );
                               f.LayerIndex=ActiveLayer.ID;
        f.P[0]=a+NoC;
        f.P[1]=b+NoC;
        f.P[2]=c+NoC;
      } */
    } else
    if( !strncmp( Slower( S ),"usemtl",6 ) ) // Slower дале уже готов для всех
    { ActiveLayer.ID=-1;
      if( NoL<NoLayers )
      for( int i=NoL; i<NoLayers; i++ )
      if( !strcmp( S+7,L[i].Description ) )
        { ActiveLayer.ID=i;  // print( "\n%d %s[%d] ",i,L[i].Description,i+1 );
          break;             //  ... или первый из попавшихся
        }
      if( ActiveLayer.ID==-1 )    // если слой не найден, тогда создание нового
      { L=(Layers*)Allocate( ++NoLayers*sizeof( Layers ),L );
        L[NoLayers-1]=ActiveLayer; // memcpy( &L[NoLayers-1],&ActiveLayer,sizeof( Layers ) );
        L[NoLayers-1].Description=strdup( S+7 );         // новое имя по ссылке
        L[NoLayers-1].ID=0; ActiveLayer.ID=NoLayers-1;   // NoLayers;
      }
    } else
    if( !strncmp( S,"mtllib",6 ) )  // разборка расцветки по уровням расслоений
    { strcpy( fname( Name ),S+7 );                                              // print( "\n собран файл %s ",Name );
     FILE *W=_wfopen( U2W( Name ),L"rt" );   // файл.mtl может быть перепрочтён
      if( !W )print( "\n? %s не открывается.\n",Name ); else
      { while( !feof( W ) )
        { if( (s=strchr( S=getString( W ),'#' ))!=NULL )*s=0;
          if( strcut( S )<3 )continue;
          if( !strncmp( Slower( S ),"newmtl",6 ) )
          { L=(Layers*)Allocate( ++NoLayers*sizeof( Layers ),L );
            L[NoLayers-1]=ActiveLayer; // memcpy( &L[NoLayers-1],&ActiveLayer,sizeof( Layers ) );
            L[NoLayers-1].Description=strdup( S+7 );
            L[NoLayers-1].ID=0; /*NoLayers;*/ } else
          if( !strncmp( S,"kd ",3 ) )
          { Color &c=L[NoLayers-1].LClr;        c.c[3]=0xFF;
            sscanf( S+3,"%lg%lg%lg",&r,&g,&b ); c.c[2]=byte( b*255 );
                                                c.c[1]=byte( g*255 );
                                                c.c[0]=byte( r*255 ); } else
          if( !strncmp( S,"d ",2 ) )
          { sscanf( S+2,"%lg",&a ); L[NoLayers-1].LClr.c[3]=byte( 22+a*220 ); } //! [22-222] - пусть пока временно
        } fclose( W );
      }
    }
  }
//if( !NoLayers )  // на случай отсутствия послойного описания свойств, будет 1
//  { L=(Layers*)Allocate( (NoLayers+1)*sizeof( Layers ),L );
//    memcpy( &L[NoLayers],&ActiveLayer,sizeof( Layers ) );
//  }
       int j=0;
  for( int i=0; i<NoLayers; i++ )if( L[i].ID>0 )
  { for( int k=0; k<NoFaces; k++ )if( F[k].LayerIndex==i )F[k].LayerIndex=j;
    if( i!=j )L[j]=L[i];
    j++;
  } NoLayers=j;                                                                 for( int I=NoL; I<NoLayers; I++ )print( "\nID=%d Descr=%s Color=%X",L[I].ID,L[I].Description,L[I].LClr.C );
//  Extents( false );       // расчёт - переопределение графических экстремумов
}
void Surface::ReadStl( char *Path )           // временный оригинал имени файла
{ char *S,T[80]; Vector N;
  if( !(S=fgets( T,6,FM )) )return;
  if( memcmp( S,"solid",5 ) )goto binarySTL; //! с двоичными файлами чуть позже
  print( "\nОткрыт Triangles.ascii файл: %s",Path ); rewind( FM );
  //
  //   Standard Triangle Library - ascii
  //
  while( !feof( FM ) )                       // первая строка "solid имя" мимо
  { while( !strcut( S=getString( FM ) ) );
    if( !memcmp( S,"solid",5 ) )               // начинаем считывание франмента
    { L=(Layers*)Allocate( ++NoLayers*sizeof( Layers ),L );
      memcpy( &L[NoLayers-1],&ActiveLayer,sizeof( Layers ) );
      L[NoLayers-1].Description=strdup( S+6 );
      L[NoLayers-1].Symmetric=true;
      L[NoLayers-1].ID=NoLayers-1;
      while( !feof( FM ) )
      { while( !strcut( S=getString( FM ) ) );   // пропуск непустой строки
//      if( !(S=getString( FM ) ) )break;        //        facet normal x,y,z
        if( !memcmp( S,"endsolid",8 ) )break;     // конец данных по фрагменту
        sscanf( S,"%s %s %lg%lg%lg",T,T,&(N.x),&(N.y),&(N.z) );
        if( !(S=getString( FM ) ) )break;        //         пропуск outer loop
        P=(CoPoint*)Allocate( (NoCoPoint+=3)*sizeof( CoPoint ),P );
        F=(Faces*)Allocate( ++NoFaces*sizeof(Faces),F );
       Faces &f=F[NoFaces-1];
        f.P=(int*)Allocate( 3*sizeof( int ) );
        f.Capacity=3;
        f.LayerIndex=NoLayers-1;
        for( int i=0; i<3; i++ )
        { if( !(S=getString( FM ) ) )break;  // последовательно читаются x,y,z
         Vector &V=P[NoCoPoint+i-3].V;
          sscanf( S,"%s %lg%lg%lg",T,&(V.x),&(V.y),&(V.z) );
          f.P[i]=NoCoPoint+i-3;
        }
        if( +N )
        { Vector &W=P[NoCoPoint-1].V;        // контроль ориентации лишь путает
          if( ((P[NoCoPoint-2].V-W)
              *(P[NoCoPoint-3].V-W))%N>0 ){ int C=f.P[0]; f.P[0]=f.P[2]; f.P[2]=C; }
        }
        if( !(S=getString( FM ) ) )break;        // endloop
        if( !(S=getString( FM ) ) )break;        // endfacet
    } }      //
  } return;  // Standard Triangle Library - binary
binarySTL:   //
 int n,NoL=NoLayers; fixed c;
  fclose( FM ); if( !(FM=_wfopen( FName,L"rb" ) ) )return; // ~ переоткрытие
  print( "\nОткрыт Triangles.binary файл: %s",Path );
  fread( T,1,80,FM );                    // Читается заголовок (80 байт)
         T[79]='\0';                      // гарантируется null-termination
  print( "\n«%s»\n",T );
  isBin=true; n=getInt();                 // Читается количество треугольников
                                          // Выделяем память всем треугольникам
  P=(CoPoint*)Allocate( (NoCoPoint+n*3)*sizeof( CoPoint ),P );
  F=(Faces*)Allocate( (NoFaces+n)*sizeof(Faces),F );
  for( int i=0; i<n; i++ )          // Читаем все треугольники
  { N=getPoint();
   Faces &f=F[NoFaces+i]; int j=NoCoPoint+i*3;
    f.P=(int*)Allocate( 3*sizeof( int ) );
    f.Capacity=3;
    P[j].V = getPoint(); f.P[0]=j;
    P[j+1].V=getPoint(); f.P[1]=j+1;
    P[j+2].V=getPoint(); f.P[2]=j+2;
    if( +N )
    { Vector &W=P[j].V;                  // контроль ориентации лишь запутывает
      if( ((P[j+1].V-W)
          *(P[j+2].V-W))%N<0 ){ int C=f.P[0]; f.P[0]=f.P[2]; f.P[2]=C; }
    }
   Color C;
    fread( &c,2,1,FM );                 // расцветка разбирается в расслоениях
    if( c&0x8000 )
    { C.C=0;
      C.c[0]=(255*( c&0x1F ))/31;
      C.c[1]=(255*((c>>=5)&0x1F))/31;
      C.c[2]=(255*((c>>=5)&0x1F))/31; C.c[3]=0xFF;
    } else C.C=ActiveLayer.LClr.C;
   bool k=false; j=NoL;
    while( j<NoLayers )
    { if( L[j].LClr.C==C.C ){ f.LayerIndex=j; k=true; break; } ++j; } j=NoLayers;
    if( !k )                                              // j==NoLayers )
    { L=(Layers*)Allocate( ++NoLayers*sizeof( Layers ),L );
      memcpy( &L[NoLayers-1],&ActiveLayer,sizeof( Layers ) );
      sprintf( T,"%i Цвет: r:%i,g:%i,b:%i - %i",j,C.c[0],C.c[1],C.c[2],C.c[3] );
      L[j].Symmetric=true;
      L[j].Description=strdup( T );
      L[j].ID=NoLayers-1;                         //      C.c[3]=0xFF;
      L[j].LClr=C;
    }
  } NoCoPoint+=n*3;
    NoFaces+=n;
/*if( !NoLayers )  // случай отсутствия послойного описания свойств, будет 1
  { L=(Layers*)Allocate( (NoLayers+1)*sizeof( Layers ),L );
    memcpy( &L[NoLayers],&ActiveLayer,sizeof( Layers ) );
  } */
}
void Surface::WriteSTL( char *FileName )
{ int ans=MessageBoxW( 0,L"   «Да» -  текстовые фрагменты по расслоениям\n"
                         L"  «Нет» -  двоичный блок с внутренней расцветкой\n",
                         L"Stereolithography.stl - триангуляция одобрена !",
                         MB_YESNO );
  if( ans==IDNO )
  { char *S=strdup( FileName ); int nTr=0;
    if( strlen( S=sname( S ) )>80 )S[80]=0;
    fprintf( FM,"%-80s",S );
    fwrite( &nTr,4,1,FM );
    for( int i=0; i<NoFaces; i++ )
    { Color &C2=L[F[i].LayerIndex].LClr;
      fixed c=0x8000 | ((31*C2.c[2])/255)<<10
                     | ((31*C2.c[1])/255)<<5 | (31*C2.c[0])/255; //c=0x8888;
      for( int j=0; j<F[i].Capacity-2; j++ )
      { Vector &A=P[F[i].P[0]].V,
               &B=P[F[i].P[j+1]].V,
               &C=P[F[i].P[j+2]].V;
        float M[3]={0,0,0}; nTr++;    fwrite( M,4,3,FM );
        M[0]=A.x; M[1]=A.y; M[2]=A.z; fwrite( M,4,3,FM );
        M[0]=B.x; M[1]=B.y; M[2]=B.z; fwrite( M,4,3,FM );
        M[0]=C.x; M[1]=C.y; M[2]=C.z; fwrite( M,4,3,FM ); fwrite( &c,2,1,FM );
      }
    } free( S ); fseek( FM,80,SEEK_SET ); fwrite( &nTr,4,1,FM );
  } else
  { for( int k=0; k<NoLayers; k++ )
    { fprintf( FM,"solid %s\n",L[k].Description );
      for( int i=0; i<NoFaces; i++ )if( F[i].LayerIndex==k )
      for( int j=0; j<F[i].Capacity-2; j++ )
      { Vector &A=P[F[i].P[0]].V,
               &B=P[F[i].P[j+1]].V,
               &C=P[F[i].P[j+2]].V;
        Vector N=dir( (B-A)*(C-A) );
        fprintf( FM," facet normal %g %g %g\n"
                    "  outer loop\n"
                    "   vertex %g %g %g\n"
                    "   vertex %g %g %g\n"
                    "   vertex %g %g %g\n"
                    "  endloop\n"
                    " endfacet\n",
             N.x,N.y,N.z,A.x,A.y,A.z,B.x,B.y,B.z,C.x,C.y,C.z );
      } fprintf( FM,"endsolid\n" );
    }
  } fclose( FM ); FM=NULL;
}

