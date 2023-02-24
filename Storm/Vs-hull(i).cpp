//
//    Исходный вариант корпуса корабля в соответствии форматом записи в файле
//
#include "Vessel.h"
#include "Aurora.inc"
//
//    Contour (centerline+wide) - контур диаметральной плоскости { x,y,z }
//     ордината { y } определяет ширину транцев, плоского днища или палубы:
//                y=0    - заостренный штевень или слом на днище;
//                y<0.001 - закруглённое окончание на штевнях и днищевом киле
//                y>=0.001 - слом на транце, ширстреке или плоском киле
//    Frame[N\M] - шпангоуты, их общее количество и номер мидель-шпангоута
//       ++ верхние точки шпангоутов означают ширстрек к палубе с погибью %3
//       ++ нижние нулевые точки на шпангоутах переносятся в килевую линию
//    Stern и Stem - штевни в традиционной/старой постановке добротного корабля
//       ++ линия по транцевым расширениям относительно диаметральной плоскости
//       ++ для катамарана это межкорпусный пролёт моста и штевни по шпангоутах
//    Shell - триангуляция поверхности корабельной обшивки выполняется строго
//       по шпациям теоретических шпангоутов, плюс оконечности с инородными
//       транцевыми плоскостями. Отсчет треугольников начинается с двух точек
//       на основной линии слева-направо (от кормы к носу), повторение точек
//       одного шпангоута - веерное покрытие, смена "флага" - перемет вперед...
//
//    Независимое изображение корпуса корабля или множественная прорисовка
//       в несвязанных окнах под управлением собственных мышек и таймеров
//
unsigned& Hull::Ins( int n, int k )    // здесь добавляется одна точка в список
{ int l=isAlloc( (void*)(Shell[n]) )/sizeof(unsigned);  // длина внутри массива
   if( !l )Shell[n]=(unsigned*)Allocate( (l=48)*sizeof(unsigned) ); else
   if( Shell[n][0]>=l-2 )
       Shell[n]=(unsigned*)Allocate( (l+48)*sizeof(unsigned),Shell[n] );
       l = ++Shell[n][0];                   // на нулевом индексе длина вектора
       k = k<=0?l+k:minmax( 1,k,l );        // вставка точки +с начала,-с конца
   while( l>k ){ Shell[n][l]=Shell[n][l-1]; l--; } return Shell[n][k];
}
//    Быстрая выборка координат точки на обшивке по таблице плазовых ординат
//
Vector Hull::InShell( int n,unsigned m ) // номер шпации,отсчет индексной маски
{ if(             m&LeftFrame )return Frame[n-2][~FramePost&m]; // левый отсчет
  if( !n ||       m&SternPost )return Stern[~FramePost&m];      // ахтерштевень
  if( n>Nframes || m&StemPost )return Stem[~FramePost&m];       // форштевень
                               return Frame[n-1][m];            // сам шпангоут
}
Vector Hull::Select( int n,int k )         // оболочка Shell[0..N+2][1..length]
{ Vector P={ 0,0,0 }; bool Board=k>0; if( !Board )k=-k;        // k=[1..length]
  if( Shell[n] )                           // по индексной маске общего массива
  if( Shell[n][0]>=k )P=InShell( n,Shell[n][k] );   // для вершин треугольников
  return Board?P:~P;             // мировая система отсчета - правый:левый борт
}
//    Выборка координат и индексов точки в таблице плазовых ординат
//            с учетом удвоения отсчетов по левому и правому борту
//
Point Hull::InSpan( int &n,int &k, bool off )   // точки 0 и Nframes+1 - штевни
{ Vector P={0,0,0}; bool Board=k>0; if( !Board )k=-k; // k=[1..length] правый>0
  if( Shell[n] )                  // проверка построения трёхмерной поверхности
  if( Shell[n][0]>=k )            // сеткой существующих вершин треугольников в
  { unsigned m=Shell[n][k];       // индексной маске таблицы ординат шпангоутов
    if( m&LeftFrame )P=Frame[n-2][k=~FramePost&m],n--; else     // левый отсчет
    if( !n || m&SternPost )P=Stern[k=~FramePost&m],n=0; else    // ахтерштевень
    if( n>Nframes || m&StemPost )P=Stem[k=~FramePost&m],n=Nframes+1; // крайний
                          else   P=Frame[n-1][k=m];             // сам шпангоут
    k=k*2+1; if( !Board )k++; // индексы шпангоутов от [1..length] - левый борт
  }                          // со штевнями в граничных кривых n=0 и n=length+1
  if( off )return out( Board?P:~P );                 // мировая система отсчета
      else return (Point){ P.x,Board?P.y:-P.y,P.z }; // из локальной координаты
}
//    здесь перебираются строки с исключением пропусков и чистых комментариев
//
static char *stringData( FILE *F )     // поиск значимой строки в файле данных
{ char *s; int Len=0;
  do{ s=getString(F); if( s[0]==';' || !memcmp(s,"//",2) )*s=0; Len=strcut(s);
    } while( !feof( F ) && !Len ); return s;
}
// попробуем организовать перебор шпаций и точек на шпангоутах с учетом штевней
//
static Real mzL=-1.0,mzR=-1.0; // нижний уровень точек шпангоута за фальшбортом

bool Hull::RNext( int sp,unsigned &m,Vector &R,byte *Sm ) // s!=sp[0..N]->в нос
{ static int s=-1,k=0;                   // не предполагается повторного чтения
  if( s!=sp ){ s=sp; k=0; }              // выбор нового левого шпангоута sp
  if( !s )                                // левее других шпангоутов больше нет
  { if( k<Frame[0].length )                 // на последовательное перечисление
    { R=Frame[0][m=k++]; return true; }     // пробная точка правее ахтерштевня
  } else
  if( s<Nframes )                           // иначе выбор вправо внутри шпации
  { _Vector F=Frame[s-1][k]; Flex &G=Frame[s];
    for( int j=-1,i=0; i<Stem.length; i++ )
    if( !Sm[i] )// выбор точек форштевня
    { _Vector S=Stem[i];
      if( mzR>0.0 && S.z>=mzR )break;// правый шпангоут загибается за фальшборт
      if( F.x<=S.x && S.x<=G[0].x )   //!<= внутри шпации с накрытием шпангоутов
      { if( !k ){ if( S.z<=G[0].z )j=i; } else // точки киля под днищем корпуса
        if( k>=G.length ){ if( S.z>G[k-1].z )j=i; } else // над верхней палубой
        { if( S.z>G[k-1].z && S.z<=G[k].z )j=i;          // внутри и слева
      } }
      if( j>=0 ){R=S; m=StemPost|j; return Sm[j]=S.x<G[0].x?1:2;} // форштевень
//    if( j>=0 ){R=S; m=StemPost|j; return Sm[j]=S.x<=F.x?1:2;} // форштевень
    } if( k<G.length ){ R=G[k]; m=k++; return true; } // номер точки шпангоута
  } else                                             // =Nframes крайние справа
  { Flex &G=Frame[s-1];
    for( int i=0; i<Stem.length; i++ )if( !Sm[i] )   // точки впереди форштевня
    { _Vector S=Stem[i];                        // формируют независимую шпацию
      if( S.x>=G[0].x ){ R=S; m=StemPost|i; return Sm[i]=1; }//S.x<=G[0].x?1:2; }
    }                                             // из свободных треугольников
    if( !Stem.length && k<G.length )                       // один раз делается
    { R=G[k=G.length-1]; m=LeftFrame|k++; return true; }   // транцевая затычка
  } return false;             // сваливается в одну путаную бесконтрольную кучу
}
bool Hull::LNext( int sp, unsigned &m,Vector &L,byte *St ) // s!=sp[0..N] от кормы в нос
{ static int s=-1,k=0;         // одноразовый подход целиком к всему шпангоуту
  if( s!=sp ){ s=sp; k=0; }    // фиксируется правый шпангоут, левый - в поиске
  if( s==Nframes )             // правее шпангоутов нет, есть только форштевень
  { if( k<Frame[s-1].length )  // обычная поисковая фиксация ++ по перечислению
      { L=Frame[s-1][k]; m=LeftFrame|k++; return true; }
  } else
  if( s>0 )                          // вариант со средними шпациями-трапециями
  { _Real F=Frame[s][0].x; Flex &G=Frame[s-1];
    for( int j=-1,i=0; i<Stern.length; i++ )if( !St[i] )
    { _Vector S=Stern[i];
      if( mzL>0.0 && S.z>=mzL )break; // левый шпангоут загибается за фальшборт
      if( G[0].x<=S.x && S.x<=F )     //!<= правому шпангоуту временная отметка
      { if( !k ){ if( S.z<=G[0].z )j=i; } else
        if( k<G.length ){ if( S.z>=G[k-1].z && S.z<=G[k].z )j=i; } else
        if( S.z>=G[k].z )j=i;
      } if( j>=0 ){ L=S; m=SternPost|j; return St[j]=S.x<F?1:2; }
    }   if( k<G.length ){ m=LeftFrame|k; L=G[k++]; return true; }
  } else      // точки за ахтерштевнем формируют шпацию свободных треугольников
  { Flex &G=Frame[0];
    for( int i=0; i<Stern.length; i++ )if( !St[i] )// перебор прикаянных вершин
    { _Vector S=Stern[i];
      if( S.x<=G[0].x ){ L=S; m=SternPost|i; return St[i]=S.x<G[0].x?1:2; }
    }
    if( !Stern.length && k<G.length ){ L=G[m=(k=G.length-1)++]; return true; }
  } return false;
}
inline Vector e5( _Vector W ) // округление записи для точных сравнений 0.01 мм
{ return (Vector){ round( W.x*1e5 )/1e5,
      W.y<=eps?0.0:round( W.y*1e5 )/1e5,
                   round( W.z*1e5 )/1e5 };
}
//    Интерполяция с разрезанием шпангоутов для фиксации точек на ватерлинии
//     ... отчасти исключаются из рассмотрения вершины пустых треугольников
//
static void Control( Flex &FL, Flex &WaterLine, _Vector Ofs, bool Stem=false )
{ for( int k=0; k<FL.length; k++ ){ Vector &A=FL[k]; A-=Ofs;   //! осторожность
    if( !A.z )WaterLine+=A; else if( k>0 ){ _Vector B=FL[k-1]; // В-уже изменен
      if( A.z*B.z<0.0 )if( Stem || A.y || B.y )
                    WaterLine+=FL.Insert( k++ )=e5( B-(A-B)*(B.z/(A.z-B.z)) );
} } }
class                                          // формировании контуров штевней
Lint{ Real *_Y,*_X; const int len; public:     //     длина изначально известна
  Lint( int L ):len(L){ _Y=(_X=(Real*)malloc(L*sizeof(Real)*2))+L; }
 ~Lint(){ free( _X ); }
  Real& operator[]( int k ){ return _Y[minmax( 0,k,len-1 )]; }   // заграничные
  Real& operator()( int k ){ return _X[minmax( 0,k,len-1 )]; }   //   равенства
  Real operator()( _Real A )                   // простая линейная интерполяция
  { if( len<2 )return len?_Y[0]:0.0; int k; Real *x=_X,*y,a,h;
    for( k=0; k<len-1 && A>x[1]; k++,x++ ); y=_Y+k,h=x[1]-x[0];
    if( !(a=A-x[0]) )return y[0];
    if( !h )return (y[0]+y[1])/2; return y[0]+a*(y[1]-y[0])/h;
} };
//     Считывание корпуса отмечается успехом, либо полным завершением программы
//
bool Hull::Read( const char FName[],   // здесь имя приходит в кодировке W-1251
                 _Real newDraught )
{
//     Считывание заголовков и проверка наличия управляющих кодов
//
 static char FileTitle[]="  Выбрать *.vsl или <Esc> к образцовой «Авроре»";
 FILE *Fh; char *s,*Str;                       // первая попытка открыть корпус
  if( !FileName )FileName=(char*)calloc( MAX_PATH,4 ); // новая строчка к имени
  fext( strcpy( FileName,FName ),"vsl" );
  if( !(Fh=FileOpen( FileName,"rt","vsl","Корпус корабля (<плаз>.vsl)\1*.vsl\1"
                              "Все файлы (<плаз>.*)\1*.*\1\1",FileTitle ) ) )
  if( !(Fh=fopen( strcpy( FileName,"Aurora.vsl" ),"rt" ) ) )
  { if( (Fh=fopen( FileName,"wb" ) )!=NULL )                      // ,ccs=UTF-8
    { fputs( Aurora,Fh ); fclose( Fh );           // предусматривается «Аврора»
      if( (Fh=fopen( FileName,"rt" ) )!=NULL )goto Ok;     // новый файл открыт
    } goto Bk;
  } //FileTitle[0]='?';
Ok: Str=stringData( Fh );            // Длинная строка в буфере входного файла
  if( *Str++==30 )                   //  '\30'=''
  { Real x,y,z;                      // здесь продолжается серия проверок
    Vector Ofs={0,0,0};              // приведение к миделю и основной линии OЛ
    int i,j,k,n;                     //  ... или к циклу двойной вложенности
    if( Str=strchr( Str,'<' ) )
    if( s=strchr( ++Str,'>' ) )      // выборка имени проекта в угловых скобках
    { *s=0; while( *Str<=' ' )++Str; // ?если нет названия проекта <- имя файла
      free( ShipName ); ShipName=strdup( strcut( Str )?Str:FileName );
    }
    sscanf( stringData( Fh ),"%d%d",&Nframes,&Mid );
    sscanf( stringData( Fh ),"%lf%lf%lf%lf",&Length,&Breadth,&Draught,&(Ofs.z));
    if( newDraught )Draught=newDraught;// расчистка и обнуление адресных ссылок
       Stem.length=Stern.length=0;                     // повторное считывание
//     Allocate( 0,Frame ); Frame=NULL;                // с переустановкой
//     Allocate( 0,Shell ); Shell=NULL;                // конструктивной осадки
    //
    //    Ахтерштевень (начало считывания последовательного потока данных)
    //
    { Lint A( n=strtol( stringData( Fh ),&s,0 ) );
      for( i=0; i<n; i++ )A(i)=strtod( s,&s ),A[i]=strtod( s,&s ); // аппликата
      { Lint B( k=strtol( stringData( Fh ),&s,0 ) ); bool fs=true; y=0;
        for( j=0; j<k; j++ )B(j)=strtod( s,&s ),B[j]=strtod( s,&s );// ордината
        for( i=j=0; i<n || j<k; )
        { if( i==n )z=B(j),y=B[j],x=A[n-1],j++; else      // завершение выборки
          if( j==k )z=A(i),y=B[k-1],x=A[i],i++; else      // запредельных точек
          if( A(i)==B(j) )z=( A(i)+B(j) )/2,y=B[j++],x=A[i++]; else
          if( B(j)>A(i) )z=A(i),y=B(z),x=A[i],i++;        // пусть это верно
                    else z=B(j),y=B[j],x=A(z),j++;
          if( fs && y>0.0   )Stern+=(Vector){ x,0,z };
              fs=false;      Stern+=e5( (Vector){ x,y,z } );
        } if(!fs && y>0.0 && Stern.length>0 )
                             Stern+=(Vector){ x,0,z };
    } }
    //    Собственно корпус ( - в том же последовательном потоке )
    //
    Frame=(Flex*)Allocate( Nframes*sizeof(Flex) );// прямой доступ к шпангоутам
    for( k=0; k<Nframes; k++ )                   // от ахтерштевня до форштевня
    { n=strtol( stringData( Fh ),&s,0 ); x=strtod( s,&s );
      for( i=0; i<n; i++ )    // в начале количество точек и абсцисса шпангоута
      { z=strtod( s,&s ); y=strtod( s,&s );       // затем аппликаты и ординаты
        if( !i  )if( y>0.0 )Frame[k]+=(Vector){ x,0,z };
                            Frame[k]+=(Vector){ x,y,z };
      } if( i>0 && Frame[k].length>0 )
        if( y>0.0 )Frame[k]+=(Vector){ x,0,z };
    }
    // Форштевень
    //  отсчет абсцисс шпангоутов и штевней ведется от кормового перпендикуляра
    //
    { Lint B( k=strtol( stringData( Fh ),&s,0 ) );
     for( j=0; j<k; j++ )B(j)=strtod( s,&s ),B[j]=strtod( s,&s );
      { Lint A( n=strtol( stringData( Fh ),&s,0 ) ); bool fs=true; y=0;
        for( i=0; i<n; i++ )A(i)=strtod( s,&s ),A[i]=strtod( s,&s );
        for( i=j=0; i<n || j<k; )
        { if( i==n )z=B(j),y=B[j],x=A[n-1],j++; else
          if( j==k )z=A(i),y=B[k-1],x=A[i],i++; else
          if( A(i)==B(j) )z=(A(i)+B(j))/2,y=B[j],x=A[i],i++,j++; else
          if( A(i)<=B(j) )z=A(i),y=B(z),x=A[i],i++;
                  else    z=B(j),y=B[j],x=A(z),j++;
          if( fs && y>0.0   )Stem+=(Vector){ x,0,z };
              fs=false;      Stem+=e5( (Vector){ x,y,z } );
        } if(!fs && y>0.0 && Stem.length>0 )
                             Stem+=(Vector){ x,0,z };
    } }
    fclose( Fh );            // работа с файлом данных завершена
    //
    //    Чтение последовательного потока завершено, но если смещена основная
    //    плоскость, то необходим проход по всем точкам плазовых аппликат
    //
    Ofs.x=Nframes>2?minmax(Frame[1][0].x,Frame[Mid][0].x,Frame[Nframes-2][0].x)
                   :(Frame[0][0].x+Frame[Nframes-1][0].x)/2; // крайние пополам
    Ofs.z+=Draught; // все отсчёты аппликат приводятся к действующей ватерлинии
    WaterLine.length=0;     Control( (Stern),WaterLine,Ofs,true ); //  контроль
    for( n=0;n<Nframes;n++ )Control( (Frame[n]),WaterLine,Ofs ); // пересечения
                            Control( (Stem),WaterLine,Ofs,true ); // ватерлинии
#if 0
    //  Перенос горизонтальных килевых точек на ординатно-транцевые поверхности
    //
    if( Stem[0].z>Stern[0].z )Ofs.x=Stern[0].x; else // на нос или на корму?
    if( Stem[0].z<Stern[0].z )Ofs.x=Stem[0].x; else  // строительный дифферент
    if( Stern[0].x>Ofs.x )Ofs.x=Stern[0].x; else     // иначе установка внутрь
    if( Stem[0].x<Ofs.x )Ofs.x=Stem[0].x;            // начала контуров штевней
    //
    //  превращение днищевых плоскостей в асимметричные транцевые расширения
    //
    for( k=0; k<Nframes; k++ )                       // перебор всех шпангоутов
    if( Frame[k].length>1 )
    if( Frame[k][0].z==Frame[k][1].z )if( Frame[k][0].y==0.0 )
    { Vector A=Frame[k][1];
      Frame[k].Delete( i=0 );                        // сброс килевой точки
      if( A.x>Stern[0].x && A.x<Stem[0].x )          // поиск между штевнями
      { if( A.x<=Ofs.x )Stern/=A,i=1;
        if( A.x>=Ofs.x )Stem/=A,i=1;
      }
      if( !i )                                       // за засечками штевней
      { Flex &S = A.x>Ofs.x ? Stem : Stern;          // поиск внутри шпации
        for( i=0; i<S.length-1; i++ )                // перебор всех точек
        if( intob( S[i].x,A.x,S[i+1].x ) )           // на штевнях до первого
        if( intob( S[i].z,A.z,S[i+1].z ) )           // вхождения
        { if( A.x==S[i+1].x )S[i+1]=A; else
          if( A.x==S[i].x )S[i]=A; else S.Insert( i+1 )=A; break;
        } //!! и !! не хватает опорной точки для концевого треугольника
    } }
#endif
    //  Собираем треугольники в оболочку Shell "как есть" с веером у днища
    //  на основной линии для последующей оптимизации от киля по максимуму
    //  площади треугольников относительно половины квадрата минимального ребра
    //    0 - расширение ахтерштевня <<~шпации~>> на форштевне - Nframes+2
    //     .. вначале треугольники в расширении кормового транца
    //
 // Shell=(unsigned**)Allocate( Nframes+3,24*sizeof(unsigned) );  //! двумерный
    Shell=(unsigned**)Allocate( (Nframes+3)*sizeof(unsigned*) );  //!  адресный
    //
    //  Транцевые площадки собираются по ординатам на штевнях
    //                                          // и просто считаются без связи
    for( n=0; n<=Nframes+2; n+=Nframes+2 )      // со всеми шпациями
    { Flex &S = !n?Stern:Stem; int Previous=-1; // указываются транцевые
      for( i=0; i<S.length; i++ )               // расширения и смежные точки
      if( S[i].y>0.0 )                          // c нулевыми отсчетами ширины
      { if( i-Previous>1 )Ins( n )=i-1; Ins( n )=Previous=i;
      } else if( i>0 && i-Previous==1 )Ins( n )=Previous=i;
    }
    //  сначала просто шпангоуты - начинаются от киля вверх до палубы/ширстрека
    //    и плазовые ординаты разбрасываются парами точек
    //      по смежным шпангоутам внутри расчетной шпации
    //
   Vector L,R,   //WS,         // базовые точки, верхняя грань четырехугольника
          Ll,Rr; //,VL,VR;     // вершины и диагонали оптимизации треугольников
   unsigned ml=0,mr=0;         // индексы с маской для выборки точек шпангоутов
   byte *St=(byte*)calloc( sizeof( byte ),Stern.length ),   // признаки учёта
        *Sm=(byte*)calloc( sizeof( byte ),Stem.length );    // точек на штевнях
#if !__x86_64__             // __i386__
   bool sl,sr;              // точки на левом и правом шпангоуте/стороне шпации
    for( n=0,mzR=-1; n<=Nframes; n++ )
    { mzL=mzR; k=0;         // фальшборт поднимается выше низкой главной палубы
      if( n<Nframes )       //    правый шпангоут предустанавливается очищенным
       for( i=1,mzR=Frame[n][0].z; i<Frame[n].length; i++ )
        if( !k ){ if( mzR>Frame[n][i].z )k=1; mzR=Frame[n][i].z; }
           else   if( mzR>Frame[n][i].z )mzR=Frame[n][i].z; if( !k )mzR=-1.0;
      if( n )  // оставляем первый бит для точек без повторений на пересечениях
      { for( i=0; i<Stem.length; i++ )Sm[i]&=1;  // снятие временных блокировок
        for( i=0; i<Stern.length; i++ )St[i]&=1; // при выборе точек на штевнях
      }
     Vector NL,NR,NY={ 0,0,-1 },  // самая первая нормаль всегда указывает вниз
            VL,VR,RL;
      //  основной алгоритм выборки вершин и оптимизации бортовых треугольников
      //!    более-менее приемлемый старый вариант функциональной оптимизации
      if( sl=LNext( n,ml,L,St ) )               // в Shell номера идут как n+1
        { Ins(n+1)=ml; sl=LNext( n,ml,Ll,St ); }// назначаются первые две точки
      if( sr=RNext( n,mr,R,Sm ) )               // и сразу проверяется наличие
        { Ins(n+1)=mr; sr=RNext( n,mr,Rr,Sm ); // второй сопряженной пары точек
        } NY=dir( (R-L)*(Rr-L) );              // нормаль второго треугольника
      for( int fb=0; fb<2; fb++ )   // FreeBoard: но сначала под\надводный борт
      for( ;; )     // здесь идёт поиск будущей адекватной вершины треугольника
      { bool Left=sl && ( fb || L.z<0 ),  // c учётом верхнего ограничения по
            Right=sr && ( fb || R.z<0 );  // осадке в конструктивной ватерлинии
        if( Left && Right )               // подход к оптимизационному поиску
        { VL=R-Ll,VR=Rr-L,RL=R-L;         // две диагонали и основание трапеции
          NL=VL*RL,NR=RL*VR;              //  нормали в основании треугольников
          if( Ll==L ){ sl=LNext(n,ml,Ll,St); continue; } // пропуски по верхней
          if( Rr==R ){ sr=RNext(n,mr,Rr,Sm); continue; }    // границе трапеции
//        if(!NL){ L=Ll; sl=LNext( n,ml,Ll,St ); continue; } // нулевая площадь
//        if(!NR){ R=Rr; sr=RNext( n,mr,Rr,Sm ); continue; } // треугольников
          //
          // предполагается возможность совпадения только в плоскости шпангоута
          if( Rr==Ll )            // Rr.x-Ll.x<=0.0 по верхней границе трапеции
          { if(VL.x<=0){L=Ll;sl=LNext(n,ml,Ll,St);continue;}//левый через правый
            if(VR.x<=0){R=Rr;sr=RNext(n,mr,Rr,Sm);continue;}//правый сечёт левый
          } else if( VL.x<=0.0 )       // пропуск по правому\носовому шпангоуту
          { if( norm( VL )>norm( Rr-R ) ){ Ins( n+1 )=mr; Ins( n+1 )=ml; }
                                    else { Ins( n+1 )=ml; Ins( n+1 )=mr; }
            L=Ll; R=Rr; sl=LNext( n,ml,Ll,St ); sr=RNext( n,mr,Rr,Sm ); continue;
          } else if( VR.x<=0.0 )       // пропуск по левому\кормовому шпангоуту
          { if( norm( VR )>norm( Ll-L ) ){ Ins( n+1 )=ml; Ins( n+1 )=mr; }
                                    else { Ins( n+1 )=mr; Ins( n+1 )=ml; }
            R=Rr; L=Ll; sr=RNext( n,mr,Rr,Sm ); sl=LNext( n,ml,Ll,St ); continue;
          }
          //  оптимизационный функционал раздельно для надводного борта и
          //  смоченной поверхности судовой обшивки до конструктивной осадки
          if( !fb )VL.x=VR.x=1.0; else               // - абсцисса исключается
              VL.x=3.2-fabs( (NL=dir( NL ))%NY ),    // - ордината ослабляется
              VR.x=3.2-fabs( (NR=dir( NR ))%NY );    // - аппликата усиливается
          if( hypot(Ll.z/Draught+2,Ll.y/Breadth)*hypot(VL.y,VL.z*1.6)*VL.x
            < hypot(Rr.z/Draught+2,Rr.y/Breadth)*hypot(VR.y,VR.z*1.6)*VR.x )
                   { L=Ll; Ins( n+1 )=ml; sl=LNext( n,ml,Ll,St ); NY=NL; } else
                   { R=Rr; Ins( n+1 )=mr; sr=RNext( n,mr,Rr,Sm ); NY=NR; } } else
        if( Left  ){ L=Ll; Ins( n+1 )=ml; sl=LNext( n,ml,Ll,St ); NY=NL; } else
        if( Right ){ R=Rr; Ins( n+1 )=mr; sr=RNext( n,mr,Rr,Sm ); NY=NR; } else break;
    } }       // и ещё иначе: 4.53~0.62=0.13; 4.4~0.6; 4.34~0.59; 4.16~.53=0.12
#else         //      отклонение от слома борта вверх на 11°48(0.98)-25°84(0.9)
    //
    //        обновляемые алгоритмы с использованием предвычисления в контурах
    //           шпангоутов, то есть в качестве повышения порядка интерполяции
    for( n=0,mzR=-1; n<=Nframes; n++ )      // шпангоуты в поисковых алгоритмах
    {
    // сначала поиск связного фрагмента борта и снижения контура за фальшбортом
    //
      mzL=mzR; k=0;         // фальшборт поднимается выше низкой главной палубы
      if( n<Nframes )       //  правый шпангоут предустанавливается очищенным
       for( i=1,mzR=Frame[n][0].z; i<Frame[n].length; i++ )
        if( !k ){ if( mzR>Frame[n][i].z )k=1; mzR=Frame[n][i].z; }
           else   if( mzR>Frame[n][i].z )mzR=Frame[n][i].z; if( !k )mzR=-1;
      if( n ) // оставляем только первый бит, то без повторений на пересечениях
      { for( i=0; i<Stem.length; i++ )Sm[i]&=1; // гашение повторных блокировок
        for( i=0; i<Stern.length; i++ )St[i]&=1; // при выборе точек на штевнях
      }
      //  начало движения вверх от килевой линии, с ограничениями на ватерлинии
      //      и сломах борта, если фиксируются отсчеты в любой из сторон шпации
     int kl=0,kr=0,  // индексные отсчеты текущих выборок для точек шпангоутов
         nl=0,nr=0;  // с крайними индексами до сломов по обоим сторонам шпации
      //
      //!  начальный цикл поиска и систематизации вершин бортовых треугольников
      //            накопление индексов для прямого доступа к ординатам корпуса
      for( wl.length=0; LNext( n,ml,L,St ); )       // кормовая\левая переборка
         { wl+=ml; if( L.z<=0.0 )nl=wl.length; } if( nl && nl<wl.length )++nl;
      for( wr.length=0; RNext( n,mr,R,Sm ); )       // носовая\правая переборка
         { wr+=mr; if( R.z<=0.0 )nr=wr.length; } if( nr && nr<wr.length )++nr;

#define Lskip(W){ W=Ll; while(kl<wl.length)if( W!=(Ll=InShell(n+1,ml=wl[kl++])) )break; }
#define Rskip(W){ W=Rr; while(kr<wr.length)if( W!=(Rr=InShell(n+1,mr=wr[kr++])) )break; }
#define Lstep(W){ if(kl<=wl.length){ Ins(n+1)=ml; if(kl<wl.length)Lskip(W) } }
#define Rstep(W){ if(kr<=wr.length){ Ins(n+1)=mr; if(kr<wr.length)Rskip(W) } }
      //
      //  первые одна/две точки в основание шпации устанавливаются обязательно
      //     ... шпангоут не может состоять из одной точки
      //
      if( wl.length>0 ){ Ll=InShell( n+1,ml=wl[kl++] ); Lstep( L ) }  // левой
      if( wr.length>0 ){ Rr=InShell( n+1,mr=wr[kr++] ); Rstep( R ) }  // правой

      while( kl<wl.length || kr<wr.length ) // проход всей шпации с наложениями
      if( kl<nl && kr<nr )           // двухсторонняя оптимизация треугольников
      { Vector VL=R-Ll,VR=Rr-L,WS=Rr-Ll;  // диагонали и верхнее ребро трапеции
        //
        //   предполагается возможность совпадения только в плоскости шпангоута
        //
//      if( Rr==Ll )                    // пропуски по верхней границе трапеции
//      { if( VL.x<=0.0 ){ Lskip( L ) continue; } else    // левый через правый
//        if( VR.x<=0.0 ){ Rskip( R ) continue; } else    // правый сечёт левый
//                       { Lskip( L ) Rskip( R ) continue; }
//      } else
        if( VL.x<=0.0 )                // пропуск по правому\носовому шпангоуту
        { if( norm( VL )>norm( Rr-R ) ){ Rstep( R )Lstep( L ) }
                                  else { Lstep( L )Rstep( R ) } continue; }else
        if( VR.x<=0.0 )                // пропуск по левому\кормовому шпангоуту
        { if( norm( VR )>norm( Ll-L ) ){ Lstep( L )Rstep( R ) }
                                  else { Rstep( R )Lstep( L ) } continue;
        }
        //   полагается вариант горизонтальных линий в зависимости от аппликаты
        //    - частично в поисках с угловыми или диагональными сопоставлениями
#if 0
        VR=dir( VR ),VL=dir( VL ),WS=dir( WS ); VR.z=VL.z=WS.z=0.0;
//                                              VR.y*=.6,VL.y*=.6,WS.y*=.6;
        if( norm( VR )>=norm( VL ) )
          { Rstep( R ) if( norm( WS )>norm( VR ) )Lstep( L ) } else
          { Lstep( L ) if( norm( WS )>norm( VL ) )Rstep( R ) }
#else
        VR.x=VL.x=WS.x=Length/Nframes; //VR.y*=.6,VL.y*=.6,WS.y*=.6;
        if( norm( VR )<=norm( VL ) )
          { Rstep( R ) if( norm( WS )<norm( VR ) )Lstep( L ) } else
          { Lstep( L ) if( norm( WS )<norm( VL ) )Rstep( R ) }
#endif
      } else
      //!     после завершения выборки одного шпангоута, другой строится веером
      //!       c последующим переходом к поиску остановок на повторах и сломах
      { Vector A,B,C; //i=kl<nl;
        while( kr<nr )Rstep( R )        // построение веера-остатка сопряженных
        while( kl<nl )Lstep( L )        // треугольников рядом в углу шпангоута
//      if( nl && nr )
//      { if( !i && kl<wl.length )Lstep( L )           // ранее выбранные точки
//        if(       kr<wr.length )Rstep( R )           //     ставятся в начало
//        if(  i && kl<wl.length )Lstep( L )           // продолжающейся шпации
//      }
        //   анализ кривизны шпангоутов, поиск и выбор точек на бортовых сломах
        //   бортовой слом 0.994522=6° 0.985=10° 0.9781476=12° 0.9659258263=15°
        //        или угол 0.93969262=20° 0.8660254=30° 0.70710678=45°
        //!  контроль\предустановка\синхронизация пропущенных сломов шпангоутов
        //
        if( kl>=wl.length )kl=nl=wl.length; else
        for( nl=kl,A=L,B=Ll; nl<wl.length && A!=B; A=B,B=C )
        { if( B==(C=InShell( n+1,wl[nl++] )) )           // поиск сломов борта
          { if( nl==kl+1 )kl++;                          // и совпадающих точек
            while( nl<wl.length )
            if( B!=(C=InShell(n+1,wl[nl++])) )break; else if( nl==kl+1 )kl++; break;
          } else if( kl>1 && A.y>0.0 && dir( A-B )%dir( B-C )<=.8660254 )break; // слом 30°
        }
        if( kr>=wr.length )kr=nr=wr.length; else
        for( nr=kr,A=R,B=Rr; nr<wr.length && A!=B; A=B,B=C )
        { if( B==(C=InShell( n+1,wr[nr++] )) )
          { if( nr==kr+1 )kr++;
            while( nr<wr.length )
            if( B!=(C=InShell(n+1,wr[nr++])) )break; else if( nr==kr+1 )kr++; break;
          } else if( kr>1 && A.y>0.0 && dir( A-B )%dir( B-C )<=.8660254 )break;
        }
        //! собственно алгоритм оптимизации в построении судовой обшивки
        //
#define mnx( w,n ) w[ minmax( 0,n-1,w.length-1 ) ]           /// (n-1) ??? => n

        ( A=dir((B=InShell(n+1,mnx(wr,nr)))-InShell(n+1,mnx(wl,nl))) ).x=1; //A.z*=0.86;
        if( fabs( A.z )<0.036 )continue;  /// ??~~ слабый алгоритм - переделать
        for( i=(k=kl)+1; i<nl; i++ )
        { ( C=dir( B-InShell( n+1,wl[i] ) ) ).x=1;                          //C.z*=0.86;
          if( norm( C )<=norm( A ) ){ A=C; k=i; }
        } if( k>kl )nl=k; if( nl==kl && nl<wl.length )nl++;

        ( A=dir(InShell(n+1,mnx(wr,nr))-(B=InShell(n+1,mnx(wl,nl)))) ).x=1; //A.z*=0.86;
        for( i=(k=kr)+1; i<nr; i++ )
        { ( C=dir( InShell( n+1,wr[i] )-B ) ).x=1;                          //C.z*=0.86;
          if( norm( C )<=norm( A ) ){ A=C; k=i; }
        } if( k>kr )nr=k; if( nr==kr && nr<wr.length )nr++;
      }
      if( kr<=wr.length )Ins( n+1 )=mr;   //!?? выход из цикла не очень чист...
      if( kl<=wl.length )Ins( n+1 )=ml;
    }
#endif
                                             // освобождение временных ресурсов
    free( Sm ),free( St ); textcolor( WHITE );
    cprint( 1,7,"  >>> %s \n  >>>  { L=%g, B=%g, T=%g }^%g "
                " Stern[%d] + Hull[%d] + Stem[%d]  ",
     ShipName,Length,Breadth,Draught,Ofs.z,Stern.length,Nframes,Stem.length );
    //
    // небольшая перенастройка визуализации сцены с кораблем
    //
    Distance=-2.4*sqrt(sqr(Length)+sqr(Breadth*2)+sqr(Draught*4)); // дальность
    eye=(Vector){ 45,-15 },look=(Vector){ -5 }; // векторы ориентации и взгляда
    return true;
  }
Bk: textcolor( YELLOW,RED ),
    cprint( 2,7,"  >>> отмена или ошибка чтения данных: %s",FileName ),
    textcolor( LIGHTGRAY,BLACK );         free( ShipName ); ShipName=0;
  return false;
}
