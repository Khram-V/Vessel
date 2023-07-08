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
       l = ++(Shell[n][0]);                 // на нулевом индексе длина вектора
       k = k<=0?l+k:minmax( 1,k,l );        // вставка точки +с начала,-с конца
   while( l>k ){ Shell[n][l]=Shell[n][l-1]; l--; } return Shell[n][k];
}
//    Быстрая выборка координат точки на обшивке по таблице плазовых ординат
//
Vector Hull::InShell( int n,unsigned m ) // номер шпации,отсчет индексной маски
{ if( m&LeftFrame )return Frame[n-1][~FramePost&m];             // левый отсчет
//if( m&SternPost )return Stern[~FramePost&m];                  // ахтерштевень
//if( m&StemPost  )return Stem[~FramePost&m];                   // форштевень
                   return Frame[n][m];                          // сам шпангоут
}
Vector Hull::Select( int n,int k )            // оболочка Shell[0..N+2][1..len]
{ Vector P={ 0,0,0 }; bool Board=k>0; if( !Board )k=-k;           // k=[1..len]
  if( Shell[n] )                           // по индексной маске общего массива
  if( Shell[n][0]>=k )P=InShell( n,Shell[n][k] );   // для вершин треугольников
  return Board?P:~P;             // мировая система отсчета - правый:левый борт
}
//    Выборка координат и индексов точки в таблице плазовых ординат
//            с учетом удвоения отсчетов по левому и правому борту
//
Point Hull::InSpan( int &n,int &k, bool off )   // точки 0 и Nframes+1 - штевни
{ Vector P={0,0,0}; bool Board=k>0; if( !Board )k=-k;    // k=[1..len] правый>0
  if( Shell[n] )                  // проверка построения трёхмерной поверхности
  if( Shell[n][0]>=k )            // сеткой существующих вершин треугольников в
  { unsigned m=Shell[n][k];       // индексной маске таблицы ординат шпангоутов
    if( m&LeftFrame )P=Frame[--n][k=~FramePost&m]; else         // левый отсчет
//  if( m&SternPost )P=Stern[k=~FramePost&m],n=0; else          // ахтерштевень
//  if( m&StemPost  )P=Stem[k=~FramePost&m],n=Nframes+1; else   // и форштевень
                     P=Frame[n][k=m];                           // сам шпангоут
    k=k*2+1; if( !Board )k++;    // индексы шпангоутов от [1..len] - левый борт
  }                             // со штевнями в граничных кривых n=0 и n=len+1
  if( off )return out( Board?P:~P );                 // мировая система отсчета
      else return (Point){ P.x,Board?P.y:-P.y,P.z }; // из локальной координаты
}
//      здесь перебираются строки с исключением пропусков и чистых комментариев
//
static char *stringData( FILE *F )      // поиск значимой строки в файле данных
{ char *s; int Len=0;
  do{ s=getString(F); if( s[0]==';' || !memcmp(s,"//",2) )*s=0; Len=strcut(s);
    } while( !feof( F ) && !Len ); return s;
}
// попробуем организовать перебор шпаций и точек на шпангоутах с учетом штевней
//
static Vector e5( _Vector W ) // округление записи для точных сравнений 0.01 мм
{ return (Vector){ round( W.x*1e5 )/1e5,
      W.y<=eps?0.0:round( W.y*1e5 )/1e5,
                   round( W.z*1e5 )/1e5 };
}
//    Интерполяция с разрезанием шпангоутов для фиксации точек на ватерлинии
//     ... отчасти исключаются из рассмотрения вершины пустых треугольников
//
static void Control( Flex &F, _Vector Ofs, bool Stm=false ) // def- шпангоутам
{ for( int k=0; k<F.len; k++ ) // if( k>=F.len )break; else // предосторожность
  { Vector &A=F[k]; A-=Ofs;                                 // привод по осадке
    if( A.z && k>0 ){ _Vector B=F[k-1];                     // В - уже изменен
      if( A.z*B.z<0 ){ if( fabs( A.z )<eps )A.z=0.0; else   // к нулю аппликаты
        if( Stm || A.y || B.y )F.Insert( k++ )=e5( B-(A-B)*(B.z/(A.z-B.z)) );
  } } }
}
static Real
Inter( _Real x, _Real x0,_Real x1, _Real y0,_Real y1 )  // ± любое направление
{ if( x==x0 )return y0;                                 // точное совпадение по
//if( x==x1 || x0==x1 )return y1;                       // крайним аргументам и
  if( x==x1 )return y1;                                 // крайним аргументам и
  if( x0==x1 )return (y0+y1)/2;                         // среднеарифметическое
              return y0+(x-x0)*(y1-y0)/(x1-x0);         //?? и беда при малом h
}
/*static Vector Inter( _Real z,_Real z0,_Real z1, _Vector v0,_Vector v1 ) // ±
{ if( z==z0 )return v0;                                 // точное совпадение по
  if( z==z1 )return v1;                                 // крайним аргументам и
  if( z0==z1 )return (v0+v1)/2;                         // среднеарифметическое
              return v0+(z-z0)*(v1-v0)/(z1-z0);         //?? и беда при малом h
}*/
//     Слияние точек штевней с переработкой катамаранных "замкнутых" контуров
//     -- первым ходом абсциссам последовательно даются поднимающиеся ординаты

static void Alliance( Flex &S,Flex &Y,bool As ) // в приоритете абсциссы точек штевней
{ int i,k; Vector V; // S /= S[-1];      // точка последняя -> начальная кольца
  //
  //  первым проходом все ординаты дополняются сопутствующими абсциссами
  //  здесь абсцисса штевня представляется однозначной функцией от аппликаты
  //
  if( !Y.len )return;                       // бестранцевые штевни - как есть
  if( S.len>1 )                             // для интерполяции надо 2 точки
  for( k=0; k<Y.len; k++ )                  // последовательный поиск ординат
  { if( Y[k].z<S[i=S.len-1].z )             //~(не)исключаются подкильные точки
    while( !intor( S[i-1].z,Y[k].z,S[i].z ) )if( i<=1 )break; else i--;
    Real R = Inter( Y[k].z,S[i-1].z,S[i].z,S[i-1].x,S[i].x );
    if( As ){ if( Y[k].x<=R )Y[k].x=R; }  // минимальные отсчеты по ахтерштевню
       else { if( Y[k].x>=R )Y[k].x=R; }  //                      и форштевню
  }
  //   вто рым шагом интерполируются ординаты транцев
  //       {с кольцевым замыканием} и добавлением оконцовок на штевнях
  //
  for( k=0,i=S.len-1; i>=0; i-- )
   if( S[i].z<Y[0].z ){ Y/=S[i]; if( !k )k=i+1; }// ниже указанных точек транца
  //
  //   расстановка абсциссы штевня по всем неоднозначным транцевым расширениям
  //
  for( i=k; i<S.len; i++ )
  for( k=0; k<Y.len-1; k++ )if( Y[k]!=S[i] && Y[k+1]!=S[i] )
   if( intox( Y[k].z,S[i].z,Y[k+1].z ) )       // или неравенство само из себя
   { V=S[i]; V.y=Inter( V.z,Y[k].z,Y[k+1].z,Y[k].y,Y[k+1].y ); Y.Insert(++k)=V;
   }

                                k=Y.len-1;             // и на текущий момент
  for( i=0; i<S.len; i++ )if( Y[k].z<S[i].z )Y+=S[i];  // в верхнее дополнение
  //
  //   может не очень эффективно, но всё ж все повторенья должны исключаться
  //
  for( S.len=k=0; k<Y.len; k++ ){ if( k )if( Y[k-1]==Y[k] )continue; S+=Y[k]; }
  //
  //  временный контроль концевых треугольников по старому образцу
  //
  if( S.len ){ V=S[0];  if( V.y ){ V.y=0.0; S/=V; }
               V=S[-1]; if( V.y ){ V.y=0.0; S+=V; } }
}
//   Считывание корпуса отмечается успехом, либо полным завершением программы
//
bool Hull::Read( const char FName[],   // здесь имя приходит в кодировке W-1251
                  _Real newDraught )   //   переустановка конструктивной осадки
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
    } goto Bk;                             // здесь продолжается серия проверок
  } //FileTitle[0]='?';
Ok: Str=stringData( Fh );            // Длинная строка в буфере входного файла
  if( *Str++==30 )                   //  '\30'=''
  { Flex L,R;  Vector V;             // векторы шпангоутных контуров по шпациям
   Real x,y,z; int i,k,n;            // ... точки по циклам двойной вложенности
    Ofs=Zero;                        // приведение к миделю и основной линии OЛ
    if( Str=strchr( Str,'<' ) )      // подзаголовок проекта в угловых скобках
    if( s=strchr( ++Str,'>' ) )      // ?если нет названия проекта <- имя файла
    { *s=0; while( *Str<=' ' )++Str;
      free( ShipName ); ShipName=strdup( strcut( Str )?Str:FileName );
    }
    sscanf( stringData( Fh ),"%d%d",&Nframes,&Mid );
    sscanf( stringData( Fh ),"%lf%lf%lf%lf",&Length,&Breadth,&Draught,&Ofs.z );
    Keel=(Real*)Allocate( (Nframes+2)*sizeof(Real),Keel );    // килевая разметка
    Frame=(Flex*)Allocate( (Nframes+2)*sizeof(Flex),Frame );  // точки шпангоутов
    for( i=0; i<=Nframes+1; i++ )Frame[i].len=0;    // вычистка всех шпангоутов
    Stem.len=Stern.len=0;        // расчистка и обнуление адресных ссылок
    if( newDraught )Draught=newDraught;// новое пересчитывание с переустановкой
    //                                                    конструктивной осадки
    //    Ахтерштевень (начало считывания последовательного потока данных)
    //
    Stern.len=L.len=0; n=strtol( stringData( Fh ),&s,0 );
    for( V.y=0,i=0; i<n; i++ )V.z=strtod( s,&s ),V.x=strtod( s,&s ),Stern+=V;
                      k=strtol( stringData( Fh ),&s,0 );
    for( V.x=0,i=0; i<k; i++ )V.z=strtod( s,&s ),V.y=strtod( s,&s ),L+=V;
    //
    //   Собственно корпус ( - в том же последовательном потоке )
    //
    for( k=1; k<=Nframes; k++ )           // перебор от ахтерштевня до форштевня
    { n=strtol( stringData( Fh ),&s,0 ); // количество теоретических шпангоутов
      Keel[k]=x=strtod( s,&s );          // абсциссы шпангоутов килевой дорожки
      for( i=0; i<n; i++ )    // в начале количество точек и абсцисса шпангоута
      { z=strtod( s,&s ); y=strtod( s,&s );       // затем аппликаты и ординаты
        if( !i )if( y>0.0 )Frame[k]+=(Vector){ x,0,z };
                           Frame[k]+=(Vector){ x,y,z };
      } if( i>0 && Frame[k].len>0 )
        if( y>0.0 )Frame[k]+=(Vector){ x,0,z };
    }
    // Форштевень
    //  отсчет абсцисс шпангоутов и штевней ведется от кормового перпендикуляра
    //
    Stem.len=R.len=0; k=strtol( stringData( Fh ),&s,0 );
    for( V.x=0,i=0; i<k; i++ )V.z=strtod( s,&s ),V.y=strtod( s,&s ),R+=V;
                      n=strtol( stringData( Fh ),&s,0 );
    for( V.y=0,i=0; i<n; i++ )V.z=strtod( s,&s ),V.x=strtod( s,&s ),Stem+=V;
    fclose( Fh );                          /// работа с файлом данных завершена

    for( i=0; i<L.len; i++ )L[i].x=Keel[Nframes];      // подготовка к поиску и
    for( i=0; i<R.len; i++ )R[i].x=Keel[1];            // выборке крайних точек
    Alliance( Stern,L,false );           // первая перенастройка штевней после
    Alliance( Stem,R,true );             // полного считывания данных о корпусе
    //
    //    Экстремумы для килевой линии и новые крайние шпангоуты
    //      ~~ крайние точки на килевой или палубной аппликате
    //      == крайние на шпангоутах
    //
    for( Keel[0]=Keel[1],i=0; i<Stern.len; i++ ){ Vector &S=Stern[i];
      if( S.x<Keel[0] )Keel[0]=S.x; // нижний экстремум абсциссы по ахтерштевню
      if( S.x<=Keel[1] )Frame[0]+=S; // кривой шпангоут в кормовой оконечности
    } n=Nframes;
    for( Keel[n+1]=Keel[n],i=0; i<Stem.len; i++ ){ Vector &S=Stem[i];
      if( S.x>Keel[n+1] )Keel[n+1]=S.x;    // экстремум на форштевне
      if( S.x>=Keel[n] )Frame[n+1]+=S;     //  образование шпангоута
    }
    //
    //  попробуем зараз расставить все точки вблизи диаметральной плоскости
    //
    for( n=0; n<=Nframes+1; n++ )                     // (מ)-левый (n+1)-правый
    { if( n<=Nframes )for( i=0; i<Stem.len; i++)      // все точки на форштевне
      if( Keel[n]<Stem[i].x && Stem[i].x<=Keel[n+1] ) // и без правых наложений
      { Vector &V=Stem[i]; Flex &F=Frame[n+1];        // точка внутри шпации
        if( V.z<F[0].z )F/=V; else                    // и её правый шпангоут
        if( V.z>=F[-1].z )F+=V; else
        for( k=1; k<F.len; k++ )
         if( intor( F[k].z,V.z,F[k-1].z ) )
//       if( intox( F[k].x,V.x,F[k-1].x ) )
           { F.Insert( k )=V; break; }
      }
      if( n>0 )for( i=0; i<Stern.len; i++)             // все точки ахтерштевня
      if( Keel[n-1]<=Stern[i].x && Stern[i].x<Keel[n] )// левые кривули штевней
      { Vector &V=Stern[i]; Flex &F=Frame[n-1];        // точка внутри шпации
        if( V.z<F[0].z )F/=V; else                     // и её левый шпангоут
        if( V.z>=F[-1].z )F+=V; else
        for( k=1; k<F.len; k++ )
         if( intor( F[k].z,V.z,F[k-1].z ) )
//       if( F[k].x<V.x && V.x>F[k-1].x )
           { F.Insert( k )=V; break; }
    } }
    //
    //    Чтение последовательного потока завершено, но если смещена основная
    //    плоскость, то необходим проход по всем точкам плазовых аппликат
    //
    Ofs.z+=Draught; // все отсчёты аппликат приводятся к действующей ватерлинии
    Ofs.x=Nframes>2?minmax( Keel[1],Keel[Mid],Keel[Nframes] )
                         :( Keel[1]+Keel[Nframes] )/2;      // крайние пополам
                         Control( (Stern),Ofs,true );       // контроль и сдвиг
    for( n=0; n<=Nframes+1; n++ )                           // для новой точки
       { Keel[n]-=Ofs.x; Control( (Frame[n]),Ofs ); }       // пересечения
                         Control( (Stem),Ofs,true );        // ватерлинии
    //
    //   предустановка точки на ватерлинии в случае отсутствия контуров штевней
    //
    if( !Stern.len ){ V=Frame[1][k=Frame[1].len-2];
      if( k>2 )if( V.z>0 && Frame[1][1].z<0 )Frame[0]+=(Vector){ V.x,0,0 }; }

    if( !Stem.len ){ n=Nframes; V=Frame[n][k=Frame[n].len-2];
      if( k>2 )if( V.z>0 && Frame[n][1].z<0 )Frame[n+1]+=(Vector){ V.x,0,0 }; }
    //
    //  Собираем треугольники в оболочку Shell "как есть" с веером у днища
    //  на основной линии для последующей оптимизации от киля по максимуму
    //  площади треугольников относительно половины квадрата минимального ребра
    //    0 - расширение ахтерштевня <<~шпации~>> на форштевне - Nframes+2
    //     .. вначале треугольники в расширении кормового транца
    //
 // Shell=(unsigned**)Allocate( Nframes+3,24*sizeof(unsigned) );  //! двумерный
    Shell=(unsigned**)Allocate( (Nframes+2)*sizeof(unsigned*) );  //!  адресный
    //
    //!  Транцевые площадки собираются по ординатам на штевнях
    //
/*  for( n=0; n<=Nframes+2; n+=Nframes+2 )         // асимметричная расстановка
    { Flex &S = !n ? Stern:Stem;                   // транцевых треугольников
      for( i=0; i<S.len; i++ )Ins( n )=unsigned( i )|( !n?SternPost:StemPost );
    }
*/  //  сначала просто шпангоуты - начинаются от киля вверх до палубы/ширстрека
    //    и плазовые ординаты разбрасываются парами точек
    //      по смежным шпангоутам внутри расчетной шпации
    //
   struct uList // *m-вектор индексных масок, n-истинная и len-текущая длина
   { unsigned *m; int n,len;
     uList(): m( NULL ),n( 0 ),len( 0 ){} ~uList(){ if( m )free( m ); }
     unsigned& operator[]( int k ){ return m[minmax(0,k,len-1)]; } // доступ
     unsigned& operator+=( unsigned p )                   // простое дополнение
     { if( len>=n )m=(unsigned*)realloc( m,(n+=128)*sizeof( unsigned ) );
       return m[len++]=p;
   } }    rf,lf;           // метки доступа к точкам на шпангоутах и на штевнях
   Vector RS,LS,rs,ls; Real VR,VL; int sl,sr,zl,zr,l,r,al,ar;

    for( n=1; n<=Nframes+1; n++ )     // начальные точки совпадают, и ненулевые
    {
      L.len=R.len=lf.len=rf.len=0; sl=sr=zl=zr=2;
      //
      //   несинхронная выборка контурных точек на штевнях и шпангоутах
      //   с образованием простых координатных последовательностей, и ставятся
      //   естественные (неединичные) абсциссы шпангоутов для угловых измерений
      //
      //   здесь выполняется выборка точек в новые контуры числовых шпангоутов,
      //   и вышибаются рассогласования у килевой линии до первых значимых хорд
      //           ширина всех шпаций теперь не устанавливается одинаковой в 1м
      //
      for( x=Keel[n-1],k=0; k<Frame[n-1].len; k++ )    // левый без точек левее
      { V=Frame[n-1][k];
        if( n==1 || V.x>=x ){ V.x=x,V.y/=3; L+=V; lf+=unsigned(k)|LeftFrame; }
      }
      for( x=Keel[n],k=0; k<Frame[n].len; k++ ) // и правый без тех, что правее
      { V=Frame[n][k];
        if( n==Nframes+1 || V.x<=x ){ V.x=x,V.y/=3; R+=V; rf+=unsigned(k); }
      }
      //
      //! оптимизация последовательности треугольников покрытия судовой обшивки
      //                               должен присутствовать начальный интервал
      Ins( n )=lf[0];                // опорные точки s# уже занесены в обшивку
      Ins( n )=rf[0];                // контрольные z# только ожидают обработки
      if( lf.len>1 && rf.len>1 )
      { if( norm( R[1]-L[0] )>norm( R[0]-L[1] ) )     // первый четырёхугольник
        { Ins( n )=lf[1]; Ins( n )=rf[1]; } else
        { Ins( n )=rf[1]; Ins( n )=lf[1]; } } else    // веер из треугольников
      { for( i=1; i<rf.len; i++ )Ins( n )=rf[i];      // дальше к другой шпации
        for( i=1; i<lf.len; i++ )Ins( n )=lf[i]; continue;
      }
    NewFind:  //! перенастройка и продолжение оптимизации линий бортовых сломов
      l=r=al=ar=-1;
      while( zl<lf.len || zr<rf.len )
      { //       предварительный просмотр поверхности с поиском бортовых сломов
        // выбирается углы хорд относительно обоих локалей шпангоутных контуров
        //
        if( zl==lf.len )++zr; else                    // внутреннее ограничение
        if( zr==rf.len )++zl; else                   // с единичным опережением
        if( (VR=norm( R[zr]-L[zl-1] ))>(VL=norm( R[zr-1]-L[zl] )) )
          { ++zl; if( norm( R[zr]-L[zl-1] )<=VL )++zr; } else  // с подборкой
          { ++zr; if( norm( R[zr-1]-L[zl] )<=VR )++zl; }      // до параллели
        //
        //   проверяются впереди идущие совпадающие точки и сломы на шпангоутах
        //
      InnFind:
#define fine( a ){ a=z##a+1; goto InnFind; }
/*
 if( zl>lf.len || zr>rf.len )
   { cprint( 2,26,"~!!! n=%d, zl=%d(%d<%d), zr=%d(%d<%d)    ",
               n,sl,zl,lf.len,sr,zr,rf.len ); getch(); }
*/
        if( zr>=rf.len )r=zr;         // по одному контрольному фрагменту для
        if( zl>=lf.len )l=zl;         // двух смежных шпангоутов рабочей шпации
        if( l<0 )                     // пропуск если повтор и не было смещения
        { if( !(LS=L[zl]-L[zl-1]) ){ ++sl,++zl; goto InnFind; }
          if( zl<lf.len-1 ){ if( !L[zl+1].z )fine( l ) else
                             if( !( ls=L[zl+1]-L[zl] ) )fine( l ) else
                             if( dir( LS )%dir( ls )<0.96 )fine( l ) }
          if( ( VL=dir( LS )%dir( L[zl]-R[zr] ) )<-0.1 )
            if( VL>dir( LS )%dir( L[zl+1]-R[zr] ) )fine( l )
        }
        if( r<0 )                     // градиентные ограничения точек\отрезков
        { if( !(RS=R[zr]-R[zr-1]) ){ ++zr,++sr; goto InnFind; }
          if( zr<rf.len-1 ){ if( !R[zr+1].z )fine( r ) else
                             if( !( rs=R[zr+1]-R[zr] ) )fine( r ) else
                             if( dir( RS )%dir( rs )<0.96 )fine( r ) }
          if( ( VR=dir( RS )%dir( R[zr]-L[zl] ) )<-0.1 )
            if( VR>dir( RS )%dir( R[zr+1]-L[zl] ) )fine( r )
        }
        // фрагменты с бортовыми сломами нуждаются в контроле углов расхождения
        //
        if( l>=0 && r>=0 ){ zr=r; zl=l; break; } // это двойная удачная находка
        //
        // иначе фиксируется пара контрольных точек для углублённого поиска
        //
        if( l>=0 && zr<rf.len ){ zl=l; if( ar<0 )ar=zr; //
          if( +(rs=R[zr]-R[zr-1]) )if( dir( rs )%dir( R[zr]-L[l] )<0.12 )
          { if( norm( R[ar-1]-L[l] )>norm( R[ar]-L[l] ) )++ar;
            ++zr; goto InnFind;  // повтор поиска внутри треугольного фрагмента
          } zr=ar; break;
        }
        if( r>=0 ){ zr=r; if( al<0 )al=zl;    // правая отсечка без левой точки
          if( +(ls=L[zl]-L[zl-1]) )if( dir( ls )%dir( L[zl]-R[r] )<0.12 )
          { if( norm( R[r]-L[al-1] )>norm( R[r]-L[al] ) )++al;
            ++zl; goto InnFind;
          } zl=al; break;
        }
      }                          //! завершение цикла поиска оптимального слома
//    if( zr==sr&&zl>sl&&zr<rf.len )++zr; // весьма странно, четырёхугольник
//    if( zl==sl&&zr>sr&&zl<lf.len )++zl; // должен обрабатываться корректно
      //
      //!  с поиском новой и наиболее параллельной грани в текущих ограничениях
      //
      while( sl<zl || sr<zr )               // завершающий веер треугольников
      if( sl==zl )Ins( n )=rf[sr++]; else // установка обоих точек для нового
      if( sr==zr )Ins( n )=lf[sl++]; else // четырехугольника, по возможности
      if( (VR=norm(R[sr]-L[sl-1]))>(VL=norm(R[sr-1]-L[sl])) )
        {                             Ins( n )=lf[sl++];
          if( norm(R[sr]-L[sl-1])<VL )Ins( n )=rf[sr++];
        } else {                      Ins( n )=rf[sr++];
          if( norm(R[sr-1]-L[sl])<VR )Ins( n )=lf[sl++];
        }
/*
 if( sl>zl || sr>zr )
   { cprint( 2,26,"~$$$ n=%d, sl=%d(%d<%d), sr=%d(%d<%d)    ",
          n,sl,zl,lf.len,sr,zr,rf.len ); getch(); }
*/
      //   особая перепроверка (=пропуск) зараз всех смежных совпадающих точек
      //
      while( sl<lf.len )if( L[sl]==L[sl-1] )zl=++sl; else break;
      while( sr<rf.len )if( R[sr]==R[sr-1] )zr=++sr; else break;

      if( sl<lf.len || sr<rf.len )goto NewFind; // к возобновлению поиска
    }
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
// C06=0.9945218954,C12=0.9781476007,C24=0.9135454576,C30=0.8660254038,
// C45=0.70710067812,C48=0.6691306064,C75=0.2588190451,C78=0.2079116908,
// C82=0.139173101,C86=0.06975647374; ... cos( 24° ) и 78° = sin( 12° )
