//
//    Исходный вариант корпуса корабля в соответствии форматом записи в файле
//
#include "Aurora.h"
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
{ int l=isAlloc( (void*)(Shell[--n]) )/sizeof(unsigned);// длина внутри массива
   if( !l )Shell[n]=(unsigned*)Allocate( (l=120)*sizeof(unsigned) ); else
   if( Shell[n][0]>=l-2 )
       Shell[n]=(unsigned*)Allocate( (l+120)*sizeof(unsigned),Shell[n] );
       l = ++(Shell[n][0]);                 // на нулевом индексе длина вектора
       k = k<=0?l+k:minmax( 1,k,l );        // вставка точки +с начала,-с конца
   while( l>k ){ Shell[n][l]=Shell[n][l-1]; l--; } return Shell[n][k];
}
//    Быстрая выборка координат точки на обшивке по таблице плазовых ординат
//
Vector Hull::InShell( int n,unsigned m ) // номер шпации,отсчет индексной маски
{ if( m&LeftFrame )return Frame[n][~LeftFrame&m];               // левый отсчет
//if( m&SternPost )return Stern[~FramePost&m];                  // ахтерштевень
//if( m&StemPost  )return Stem[~FramePost&m];                   // форштевень
                   return Frame[n+1][m];                        // сам шпангоут
}
Vector Hull::Select( int n,int k )              // оболочка Shell[0..N][1..len]
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
    if( m&LeftFrame )P=Frame[n][k=~LeftFrame&m]; else           // левый отсчет
//  if( m&SternPost )P=Stern[k=~FramePost&m],n=0; else          // ахтерштевень
//  if( m&StemPost  )P=Stem[k=~FramePost&m],n=Nframes+1; else   // и форштевень
                     P=Frame[++n][k=m];                         // сам шпангоут
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
static void Control( Flex &F,_Vector Ofs, bool Stm=false )  // def - шпангоутам
{ for( int k=F.len-1; k>=0; k-- )                           // предосторожность
  { Vector &A=F[k]; A-=Ofs;                                 // привод по осадке
    if( fabs( A.z )<eps )A.z=0.0; else                      // к нулю аппликаты
    if( k<F.len-1 ){ Vector B=F[k+1];                       // В - уже изменен
//    if( A==B && A.z<0.0 )F.Delete( k ); else              // без совпадений и
      if( A.z*B.z<0.0 )                                     // по любой стороне
      { if( Stm )e6( B=Inter( 0.0,A.z,B.z,A,B ) ); else
        if( A.y || B.y ){ e6( B.y=Inter( 0.0,A.z,B.z,A.y,B.y ) ); B.z=0.0; }
        else continue;
        F.Insert( k+1 )=B;
} } } }
//
//!      Слияние точек штевней с переработкой катамаранных "замкнутых" контуров
//
static void Alliance( Flex &S, Flex &Y, _Real X )          // St ? Stern : Stem
{ if( !Y.len )return;               // штевни остаются как есть, без дополнений
  if( !S.len ){ Y[0].x=X; S+=Y[0]; } // одна новая точка сдвигается к шпангоуту
 int i,j; Vector V; Flex F;
  for( i=j=0; i<S.len || j<Y.len; F+=V )
  { if( !i && !j  )
    { V.y=Y[0].y;                   // начальная точка по минимальной аппликате
      V.x=S[0].x;
      if( fabs( S[0].z-Y[0].z )<eps ){ V.z=S[0].z; i=j=1; }   // здесь 1-я мера
      else if( S[0].z<Y[0].z )V.z=S[i++].z; else V.z=Y[j++].z;  //   дистанции
    } else
    if( i>=S.len ){ V.y=Y[j].y; V.z=Y[j].z; j++; } else       // ?? прямой угол
    if( j>=Y.len ){ V.x=S[i].x; V.z=S[i].z; i++; } else       // ? интерполяция
#if 1
    { S[i].y=Y[j].y;
      Y[j].x=S[i].x;
      if( abs( S[i]-Y[j] )<eps ){ V=S[i++]; j++; } else
      if( abs( S[i]-V )<abs( Y[j]-V ) )
      { V=S[i]; V.y=Inter( V.z,Y[j-1].z,Y[j].z,Y[j-1].y,Y[j].y ); i++; } else
      { V=Y[j]; V.x=Inter( V.z,S[i-1].z,S[i].z,S[i-1].x,S[i].x ); j++; }
    }
#else
    if( fabs( S[i].z-Y[j].z )<eps ){ V=S[i++]; V.y=Y[j++].y; } else
    if( fabs( V.z-S[i].z ) < fabs( V.z-Y[j].z ) ) // интерполяция на интервалах
      { V=S[i]; V.y=Inter( V.z,Y[j-1].z,Y[j].z,Y[j-1].y,Y[j].y ); ++i; } else
      { V=Y[j]; V.x=Inter( V.z,S[i-1].z,S[i].z,S[i-1].x,S[i].x ); ++j; }
#endif
  }
  if( F.len ){ //V=F[0];  if( V.y ){ V.y=0.0; F/=V; }  // так, на всякий случай
               //V=F[-1]; if( V.y ){ V.y=0.0; F+=V; }  //   обнуление оконцовок
    S.len=0; S+=F[0];                         // сброс исходных абсцисс  штевня
    for( i=1; i<F.len; i++ )                  // с переброской иного результата
    { if( F[i].z>0 || S[-1]!=F[i] )S+=F[i]; } // без повторов в подводной части
} }
#if 0
#define inx( x,s )( (St && (s<=x)) || (!St && (x<=s)) ) // за штевнем [s<=x<=s]
#define oux( x,s )( (St && (x<s) ) || (!St && (s<x) ) ) // вне штевня  [x<s<x]
#endif
//     сопровождение векторного списка с помощью индексов для оболочки Shell
//
static struct uList // *m-вектор индексных масок, n-истинная, len-текущая длина
{ unsigned *m; int n,len;
//uList(): m( NULL ),n(0),len(0){} ~uList(){ if( m )free( m ); m=0; n=len=0; }
  unsigned& operator[]( int ); unsigned& operator+=( unsigned );     // [k%len]
} rf={0,0,0},lf={0,0,0}; //  адреса списков к индексам доступа к точкам корпуса
unsigned& uList::operator[]( int k ){ return m[minmax(0,k>=0?k:k+len,len-1)]; }
unsigned& uList::operator+=( unsigned p ) // выбор [k] обраткой и += дополнение
{ if(len>=n)m=(unsigned*)realloc(m,(n+=96)*sizeof(unsigned));return m[len++]=p;
}
static Flex L,R;               // левый и правый шпангоуты для поисковой шпации
static bool Span( int l, int r )
#if 1
{ //return fabs( dir(R[r+1]-R[r])%dir(R[r+1]-L[l]) )
  //     < fabs( dir(L[l+1]-L[l])%dir(L[l+1]-R[r]) );
  //return dir( R[r+1]-L[l] ).x > dir( R[r]-L[l+1] ).x;
    return norm( R[r+1]-L[l] ) < norm( R[r]-L[l+1] );
#else
{ static Vector blr={0,0,-1};
 Vector Base =R[r]-L[l], Right=dir( Base*(R[r+1]-R[r]) ),
                         Left =dir( Base*(L[l+1]-L[l]) );
  if( Right%blr>Left%blr ){ if( Right%blr>0.999 ){ blr=Right; return true; } }
                     else { if( Left%blr>0.999 ){ blr=Left; return false; } }
  if( dir( R[r+1]-L[l] ).x >= dir( R[r]-L[l+1] ).x ){ blr=Right; return true; }
                                               else { blr=Left; return false; }
#endif
}
//   Считывание корпуса отмечается успехом, либо полным завершением программы
//
bool Hull::Read( const char* FName,    // здесь имя приходит в кодировке W-1251
                  _Real newDraught )   //   переустановка конструктивной осадки
{
//     Считывание заголовков и проверка наличия управляющих кодов
//
 static WCHAR FileTitle[]=L"  Выбрать *.vsl или <Esc> к образцовой «Авроре»";
 FILE *Fh; char *s,*Str;                       // первая попытка открыть корпус
//if( !FileName )FileName=(char*)calloc( MAX_PATH,4 ); // новая строчка к имени
  fext( strcpy( FileName,FName ),"vsl" );              // имя файла сохраняется
  if( !(Fh=FileOpen( FileName, L"rt", L"vsl",          // здесь имя переменно
                     L"Корпус корабля (<плаз>.vsl)\1*.vsl\1"
                     L"Все файлы (<плаз>.*)\1*.*\1\1",FileTitle ) ) )
  if( !(Fh=fopen( strcpy( FileName,"Aurora.vsl" ),"rt" ) ) )
  { if( (Fh=fopen( FileName,"wb" ) )!=NULL )                     // , ccs=UTF-8
    { fputs( Aurora,Fh ); fclose( Fh );           // предусматривается «Аврора»
      if( (Fh=fopen( FileName,"rt" ) )!=NULL )goto Ok; // новый файл открыт
    } goto Bk;                       // здесь продолжается серия проверок
  }                                  //  FileTitle[0]='?'
Ok:
  Str=stringData( Fh );              // Длинная строка в буфере входного файла
  if( *Str++==30 ){                  //  '\30=▲\x1E'
   Vector V;                         // векторы шпангоутных контуров по шпациям
   Real x,y,z; int i,j,k,l,m,n;      // ... точки по циклам двойной вложенности
    Ofs=Zero;                        // приведение к миделю и основной линии OЛ
    if( Str=strchr( Str,'<' ) )      // подзаголовок проекта в угловых скобках
    if( s=strchr( ++Str,'>' ) )      // ?если нет названия проекта <- имя файла
    { *s=0; while( *Str<=' ' )++Str;
      if( ShipName )free( ShipName );
      ShipName=strdup( strcut( Str )?Str:FileName );
    }
    sscanf( stringData( Fh ),"%d%d",&Nframes,&Mid ); ++Mid;
    sscanf( stringData( Fh ),"%lf%lf%lf%lf",&Length,&Breadth,&Draught,&Ofs.z );
    Keel=(Real*)Allocate( (Nframes+2)*sizeof(Real),Keel );  // килевая разметка
    Frame=(Flex*)Allocate( (Nframes+2)*sizeof(Flex),Frame );// точки шпангоутов
    for( i=0; i<=Nframes+1; i++ )Frame[i].len=0;    // вычистка всех шпангоутов
    Stem.len=Stern.len=L.len=R.len=0;  // расчистка и обнуление адресных ссылок
    if( newDraught )Draught=newDraught;// новое пересчитывание с переустановкой
    //                                                    конструктивной осадки
    //  Ахтерштевень (начало считывания последовательного потока данных)
    //
    n=strtol( stringData( Fh ),&s,0 );          // штевни выбираются как есть
    for( V.y=0,i=0; i<n; i++ )V.z=strtod( s,&s ),V.x=strtod( s,&s ),Stern+=V;
    k=strtol( stringData( Fh ),&s,0 );          // если концевые точки обнулены
    for( V.x=0,i=0; i<k; i++ )V.z=strtod( s,&s ),V.y=strtod( s,&s ),L+=V;
    //
    //   Собственно корпус ( - в том же последовательном потоке )
    //
    for( k=1; k<=Nframes; k++ )          // перебор от ахтерштевня до форштевня
    { n=strtol( stringData( Fh ),&s,0 ); // количество теоретических шпангоутов
      Keel[k]=x=strtod( s,&s );          // абсциссы шпангоутов килевой дорожки
      for( i=0; i<n; i++ )    // в начале количество точек и абсцисса шпангоута
      { z=strtod( s,&s ); y=strtod( s,&s ); // аппликаты и ординаты на шпангоут
        //if( !i )if( y>0.0 )Frame[k]+=(Vector){ x,0,z };
        if( Frame[k].len==1 )               // снимается нулевая точка на днище
        { Vector &v=Frame[k][0]; if( !v.y && v.z==z ){ v.y=y; continue; }
        } Frame[k]+=(Vector){ x,y,z };
      } //if( i>0 && Frame[k].len>0 )if( y>0 )Frame[k]+=(Vector){ x,0,z };
    }
    // Форштевень
    //  отсчет абсцисс шпангоутов и штевней ведется от кормового перпендикуляра
    //
    k=strtol( stringData( Fh ),&s,0 );
    for( V.x=0,i=0; i<k; i++ )V.z=strtod( s,&s ),V.y=strtod( s,&s ),R+=V;
    n=strtol( stringData( Fh ),&s,0 );
    for( V.y=0,i=0; i<n; i++ )V.z=strtod( s,&s ),V.x=strtod( s,&s ),Stem+=V;
                                         //
    fclose( Fh );                        // работа с файлом данных завершена
    Alliance( Stern,L,Keel[1] );         // первая перестройка штевней после
    Alliance( Stem,R,Keel[Nframes] );    // считывания всех данных о корпусе
    //
    //   Экстремумы по килевой линии и дополнение крайних шпангоутов
    //       ~~ крайние точки на килевой или палубной аппликате
    //       == становятся крайними на шпангоутах
    //
    for( Keel[0]=Keel[1],i=0; i<Stern.len; i++ )
    { const Vector &S=Stern[i];
      if( S.x<Keel[0] )Keel[0]=S.x;  // левый экстремум абсциссы по ахтерштевню
      if( S.x<=Keel[1] )Frame[0]+=S;  // кривой шпангоут в кормовой оконечности
    }                               // осредняем предыдущий к пустому шпангоуту

    if( !Frame[0].len ){ for( z=i=0; i<Frame[1].len; i++ )z+=Frame[1][i].z;
                          if( i )Frame[0]+=(Vector){ Keel[0],0,z/i };
                       } n=Nframes; // носовая оконечность на конечных отсчётах

    for( Keel[n+1]=Keel[n],i=0; i<Stem.len; i++ )
    { const Vector &S=Stem[i];
      if( S.x>Keel[n+1] )Keel[n+1]=S.x;  // экстремум абсциссы по форштевню
      if( S.x>=Keel[n] )Frame[n+1]+=S;   // с образованием свободного шпангоута
    }                                    //        который может и не сложиться

    if( !Frame[n+1].len ){ for( z=i=0; i<Frame[n].len; i++ )z+=Frame[n][i].z;
                            if( i )Frame[n+1]+=(Vector){ Keel[n+1],0,z/i };
                         }
    //
    //    Чтение последовательного потока завершено, но если смещена основная
    //    плоскость, то необходим проход по всем точкам плазовых аппликат
    //
    Ofs.z+=Draught; // все отсчёты аппликат приводятся к действующей ватерлинии
    Ofs.x=Nframes>2?minmax( Keel[0],Keel[Mid],Keel[n+1] )
                         :( Keel[0]+Keel[n+1] )/2;          // крайние пополам
                         Control( Stern,Ofs,true );         // контроль и сдвиг
    for( n=0; n<=Nframes+1; n++ )                           // для новой точки
       { Keel[n]-=Ofs.x; Control( Frame[n],Ofs ); }         // пересечения
                         Control( Stem,Ofs,true );          // ватерлинии
    //
    //!   Здесь пробуем расставить сразу все точки с координатами на штевнях
    //       относительно диаметральной плоскости в продолжение
    //           и в разрывах исходных шпангоутных контуров
    //
   bool lfr=false; // выбор левой стороны от штевня со смежными правыми шпангоутами
    do
    for( n=1; n<Nframes; n++ )  // концевые шпации считаются предустановленными
    {    i=j=l=0;            // будет интервал [j..l], i возможно с превышением
     Flex &S=lfr?Stern:Stem; // изначально полагается нижняя точка на шпангоуте
    Rep:                     // выборка цельного фрагмента штевня внутри шпации
      for( j=-1; i<S.len; i++ )                   // подборка точек на штевнях
      if( Keel[n]<=S[i].x && S[i].x<=Keel[n+1] )  // в пределах одной шпации
      { l=i; if( j<0 )j=l;                        // индекс i будет зашкаливать
      } else if( j>=0 )break;                     // получен интервал от j до l
             if( j<0 )continue;                   // фрагмент контура не найден
/*    if( j==l )goto Rep; else                    // ? j==l интервал на 1 точку
*/    { //
        // самая простая привязка контуров штевней к наклонам ветвей шпангоутов
        //
       Flex &F=lfr?Frame[n]:Frame[n+1];          // фиксация ссылки на шпангоут
        if( S[l].z>=F[-1].z ){ for( k=j; k<=l; k++ )if( F[-1]!=S[k] )F+=S[k]; } else
        if( S[j].z<F[0].z ){ for( k=l; k>=j; k-- )if( F[0]!=S[k] )F/=S[k]; } else
        for( m=1; m<F.len; m++ )
         if( F[m-1].y==0.0 && F[m].y==0.0 )
         if( intoi( F[m-1].z,0.5*(S[j].z+S[l].z),F[m].z ) )
//         { for( k=l; k>=j; k-- ){ V=S[k]; V.y=0; F.Insert( m )=V; } break; }
           { for( k=l; k>=j; k-- )F.Insert( m )=S[k]; break; }
        if( i<S.len )goto Rep;
      }
    }     //  уточнение крайних контуров проекции бок в диаметральной плоскости
    while( lfr^=true );                        // сначала форштевень lfr=false,
                                               // затем ахтерштевень lfr=true
    for( n=1; n<=Nframes; n++ )
    { V=Frame[n][0];  if( V.y ){ V.y=0; Frame[n]/=V; }
      V=Frame[n][-1]; if( V.y ){ V.y=0; Frame[n]+=V; }
    }
    //! Собираем треугольники в оболочку Shell - "как есть" с веером у днища на
    //  основной линии для последующей оптимизации от киля по максимуму площади
    //  треугольников относительно половины квадрата минимального ребра
    //                   ... вначале треугольники в расширении кормового транца
    //
    Shell=(unsigned**)Allocate( (Nframes+1)*sizeof(unsigned*) );     //! [0..N]
    //!                                                               индексный
    //  сначала просто шпангоуты - начинаются от киля вверх до палубы/ширстрека
    //                          и плазовые ординаты разбрасываются парами точек
    //                            по смежным шпангоутам внутри расчетной шпации
// uList rf,lf;                       // индексы доступа к точкам
    for( n=1; n<=Nframes+1; n++ )     // на шпангоутах (теперь без штевней)
    { L.len=R.len=lf.len=rf.len=0;    // начальные точки совпадают, и ненулевые
      //
      //   раздельная несинхронная выборка контурных точек штевней и шпангоутов
      //   с образованием простых координатных последовательностей, и ставятся
      //   естественные (неединичные) абсциссы шпангоутов для угловых измерений
      //
      //   здесь выполняется выборка точек в новые контуры числовых шпангоутов,
      //   и вышибаются рассогласования у килевой линии до первых значимых хорд
      //      ширина всех шпаций теперь (не)предустанавливается одинаковой в 1м
      //
      for( x=Keel[n-1],k=0; k<Frame[n-1].len; k++ )  // слева без левых загибов
       if( ( V=Frame[n-1][k]).x>=x){ L+=V; lf+=unsigned(k)|LeftFrame; }
      for( x=Keel[n],k=0; k<Frame[n].len; k++ )// справа без правых искривлений
       if( ( V=Frame[n][k] ).x<=x ){ R+=V; rf+=unsigned( k ); }
      //
      //     исправления, как бы для согласования и выравнивания геометрических
      //     особенностей со сверкой нулевого контура в диаметральной плоскости
      //
      for( x=Keel[n-1],i=0; i<L.len; i++ )L[i].x=x; //!=0.0
//     if( L[i].x>x )L[i].y=0.0,L[i].x=x; else if( L[i].z>0.0 )L[i].y/=2.0;
      for( x=Keel[n],i=0; i<R.len; i++ )R[i].x=x;   //!=1.0 ∀ Keel[n]-Keel[n-1]
//     if( R[i].x<x )R[i].y=0.0,R[i].x=x; else if( R[i].z>0.0 )R[i].y/=2.0;
      //
      //   сброс повторяющихся точек в подводной части корпуса,
      /*                                             однако они и так идут мимо
      for( i=L.len-1; i>0; i-- )if( L[i].z<0 && L[i]==L[i-1] )
        { L.Delete( i ); for( j=i; j<L.len; j++ )lf[j]=lf[j+1]; } lf.len=L.len;
      for( i=R.len-1; i>0; i-- )if( R[i].z<0 && R[i]==R[i-1] )
        { R.Delete( i ); for( j=i; j<R.len; j++ )rf[j]=rf[j+1]; } rf.len=R.len;
      */
      //!            перестройка и оптимизация последовательности треугольников
      //                                     по всему покрытию бортовой обшивки
      //
     int sl=1,sr=1;   // индексы поиска по контурам шпангоутов от третьих точек
      Ins( n )=lf[0],Ins( n )=rf[0];   // левая точка впереди - это существенно
//    Ins( n )=lf[1],Ins( n )=rf[1]; sl=sr=2;
//if(0)
      if( L.len>1 && R.len>1 ) // четырёхугольник отодвигается от диаметральной
      { if( norm( L[1]-L[0] )>=norm( R[1]-R[0] ) )       // плоскости (временно)
        { Ins( n )=lf[1]; Ins( n )=rf[1];                 // обновление выборки
           for( i=2; i<L.len; i++ )
            if( norm( R[1]-L[i] )>=norm( R[1]-L[i-1] ) )break;
             else Ins( n )=lf[sl=i];
        } else
        { Ins( n )=rf[1]; Ins( n )=lf[1];
           for( i=2; i<R.len; i++ )
            if( norm( L[1]-R[i] )>=norm( L[1]-R[i-1] ) )break;
            else Ins( n )=rf[sr=i];
        }
      } else                                           // веер из треугольников
      { for( i=1; i<L.len; i++ )Ins( n )=lf[i];         // либо далее до другой
        for( i=1; i<R.len; i++ )Ins( n )=rf[i]; continue; // к следующей шпации
      }
      //  предустановка верхней границы по палубе или конструктивной ватерлинии
      //
     int wl=sl,wr=sr;        // обратные индексные отсчеты по уровню ватерлинии
      for( wl=L.len-1; wl>sl; wl-- )if( L[wl].z<=0.0 )break;
      for( wr=R.len-1; wr>sr; wr-- )if( R[wr].z<=0.0 )break;
      //
      //! метка простого последовательного построения сначала подводных обводов
      //            и затем надводного борта с множеством сломов на надстройках
      while( sl<wl || sr<wr )               // завершающий веер треугольников
      if( sl>=wl )Ins( n )=rf[++sr]; else   // установка обоих точек для нового
      if( sr>=wr )Ins( n )=lf[++sl]; else   // четырехугольника, по возможности
      { if( Span( sl,sr ) )Ins( n )=rf[++sr]; // на выходе строго sl==wl и sr==wr
                   else    Ins( n )=lf[++sl]; //! смачиваемая поверхность корпуса
        if( sl>2 && (dir( L[sl]-L[sl-1] )%dir( L[sl+1]-L[sl] ))<=0 //>2 от киля
         || sr>2 && (dir( R[sr]-R[sr-1] )%dir( R[sr+1]-R[sr] ))<=0 )break;
      }
      for( int l=wl,r=wr,s=0; l>sl || r>sr; )
      if( l<=sl )Ins( n,s-- )=rf[r--]; else   // установка обоих точек для нового
      if( r<=sr )Ins( n,s-- )=lf[l--]; else   // четырехугольника, по возможности
      if( !Span( l-1,r-1 ) )Ins( n,s-- )=rf[r--];
                      else  Ins( n,s-- )=lf[l--];
      sl=wl; sr=wr;
      //
      ///          !.перенастройка и продолжение оптимизации по бортовым сломам
     int zl=sl,zr=sr;                // опорные точки s# уже занесены в обшивку
      wl=L.len;                      // контрольные z# только ожидают обработки
      wr=R.len;                      // ограничивающие w# к ватерлинии и палубе
      do
      { //       предварительный просмотр поверхности с поиском бортовых сломов
        // выбирается углы хорд относительно обоих локалей шпангоутных контуров
        //
       const Real C24=0.8; //0.9135454576;
        if( sl==zl && sr==zr )                // обход начальных предустановок
        { int l=wl-zl,r=wr-zr;                // поиске всего от киля до палубы
          while( l>0 || r>0 )                 //    по индексам бортовых сломов
          { if( zl>=wl-2 || zr>=wr-2 )        // палуба перекрывается без каких
              { zl=wl; zr=wr; break; }        //  - либо дополнительных условий
            lfr=Span( zl,zr );
            if( lfr )goto Rst;                // ..подтягивается к правой точке
       Lst: if( l>0 )
            { if( ++zl>=wl-1 )zl=wl,l=0; else
              if( L[zl-1]==L[zl] || dir(L[zl+1]-L[zl])%dir(L[zl]-L[zl-1])<C24 )
                { l=0; if( r>0 )r=2; }
              else if( l>=0 )--l;
              continue;
            }
            if( lfr )continue;
       Rst: if( r>0 )
            { if( ++zr>=wr-1 )zr=wr,r=0; else // на точку ниже последнего слома
              if( R[zr-1]==R[zr] || dir(R[zr+1]-R[zr])%dir(R[zr]-R[zr-1])<C24 )
                { r=0; if( l>0 )l=2; }
              else if( r>=0 )--r;
              continue;
            }   // точка слома, три попытки слева, левый отстаёт ниже горизонта
            if( lfr )goto Lst;
          }
#if 0     ///      это надо встроить как-то внутрь, в рекурсию поиска и анализа
          // теперь выбор допустимо малого угла отклонения ребра от горизонтали
          //
     const Real S20=0.3420201433;
     Vector W=R[zr]-L[zl];         // исправление индекса с нарушением отсчётчика
          if( !l && r<0 )for( i=zr-1; i>sr; i-- )
            { if( norm( V=R[i]-L[zl] )<norm( W ) )W=V,zr=i; else break; } else
          if( !r && l<0 )for( i=zl-1; i>sl; i-- )
            { if( norm( V=R[zr]-L[i] )<norm( W ) )W=V,zl=i; else break; } else
            { W=dir( W );
              if( W.z>S20 ^ R[zr+1].z<R[zr].z )for( i=zr-1; i>sr; i-- )
                { if( R[i].z-L[zl].z>=0 )zr=i; else break; }
              else
              if( W.z<-S20 ^ L[zl+1].z>L[zl].z  )for( i=zl-1; i>sl; i-- )
                { if( R[zr].z-L[i].z<=0)zl=i; else break; }
            }
#endif
        }
        // с поиском новой и наиболее параллельной грани в текущих ограничениях
        //

//zl=wl,zr=wr;

        while( sl<zl || sr<zr )             // завершающий веер треугольников
        if( sl>=zl )Ins( n )=rf[++sr]; else // установка обоих точек для нового
        if( sr>=zr )Ins( n )=lf[++sl]; else // четырехугольника, по возможности
        if( Span( sl,sr ) )Ins( n )=rf[++sr];    // по абсциссе или её проекции
                      else Ins( n )=lf[++sl];    // то проще не бывает
          // if( dir( R[sr]-L[sl] ).x<0.9396926208 ){ zl=sl; zr=sr; continue; }
        //
        //  особая перепроверка (=пропуск) зараз всех смежных совпадающих точек
        //
        while( sl<wl-1 )if( L[sl]==L[sl+1] )zl=++sl; else break;
        while( sr<wr-1 )if( R[sr]==R[sr+1] )zr=++sr; else break;
      } while( zl<wl || zr<wr );
    }
    //  небольшая перенастройка визуализации сцены с кораблем
    //
    Distance=-2.4*sqrt(sqr(Length)+sqr(Breadth*2)+sqr(Draught*4)); // дальность
    eye=(Vector){ 45,-15 },look=(Vector){ -5 };  // векторы ориентации и обзора
    return true;
  }
Bk: textcolor( YELLOW,RED ),
    print( 2,7,"  >>> отмена или ошибка в цифровой модели: %s",FileName ),
    textcolor( LIGHTGRAY,BLACK ); free( ShipName ); ShipName=0;
  return false;
}
/* C06=0.9945218954,C08=0.9902680687,C12=0.9781476007,C18=0.9510565163,
   C20=0.9396926208,C24=0.9135454576,C30=0.8660254038,C45=0.70710067812,
   C48=0.6691306064,C66=0.40673664307,C72=0.30901699437,C75=0.2588190451,
   C78=0.2079116908,C82=0.139173101,C84=0.1045284633,C86=0.06975647374 ...
   cos(78°)=sin(12°) ==> S04=C86,S06=C84,S08=C82,S12=C78,S18=C72,S24=C66 ...
   {
     if( (y=norm(R[sr+1]-L[sl]))>(x=norm(R[sr]-L[sl+1])) ) /// без фокусов
     {                            Ins( n )=lf[++sl];       // с ускорением
       if( norm(R[sr+1]-L[sl])<x )Ins( n )=rf[++sr];
     } else {                     Ins( n )=rf[++sr];
       if( norm(R[sr]-L[sl+1])<y )Ins( n )=lf[++sl];
   } }
*/
