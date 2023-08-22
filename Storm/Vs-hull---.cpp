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
//    Интерполяция с разрезанием шпангоутов для фиксации точек на ватерлинии
//     ... отчасти исключаются из рассмотрения вершины пустых треугольников
//
static void Control( Flex &F, _Vector Ofs, bool Stm=false ) // def- шпангоутам
{ for( int k=0; k<F.len; k++ )                              // предосторожность
  { Vector &A=F[k]; A-=Ofs;                                 // привод по осадке
    if( fabs( A.z )<eps )A.z=0.0; else                      // к нулю аппликаты
    { if( k>0 ){ _Vector B=F[k-1];                          // В - уже изменен
        if( A.z*B.z<0 )                                     // по любой стороне
        if( Stm || A.y || B.y )F.Insert( k++ )=e5( B-(A-B)*(B.z/(A.z-B.z)) );
} } } }
static
Real Inter( _Real x,_Real x0,_Real x1,_Real y0,_Real y1 ) //~ любое направление
{ if( x==x0 )return y0;                                 // точное совпадение по
  if( x==x1 )return y1;                                 // крайним аргументам и
//if( x==x1 || x0==x1 )return y1;                       // с выборкой последней
  if( x0==x1 )return (y0+y1)/2.0;                       // среднеарифметическое
              return y0+(x-x0)*(y1-y0)/(x1-x0);         // ? и беда при малом h
}
//     Слияние точек штевней с переработкой катамаранных "замкнутых" контуров
//     -- первым ходом абсциссам последовательно даются поднимающиеся ординаты
#if 1
static void Alliance( Flex &S, Flex &Y, bool St, _Real X ) // St ? Stern : Stem
{ int i,j,k,m,n; Vector V;          // S+=S[0] точка последняя <- начало кольца
  if( !Y.len )return;
  if( !S.len ){ Y[0].x=X; S+=Y[0]; } // единственная точка ставится на шпангоут
#define inx( x,s ) ( (St && (s<=x))||(!St && (x<=s)) )  // за штевнем [s<=x<=s]
#define oux( x,s ) ( (St && (x<s) )||(!St && (s<x) ) )  // вне штевня  [x<s<x]
  //
  //  для начала расставляются абсциссы на всех транцевых расширениях
  //     по дальним точкам на контурах штевней в диаметральной плоскости
  //
  for( m=n=-1,k=0; k<Y.len; k++ )                       // одна абсцисса штевня
  if( S.len==1 ){ Y[k].x=S[0].x,m=0,n=1; } else         // для всех расширений
  if( S.len==2 ){ Y[k].x=Inter(Y[k].z,S[0].z,S[1].z,S[0].x,S[1].x),m=0,n=2; } else
  { Real x,mx; V=Y[k];               // корректируются точки транцевых контуров
    for( j=-1,i=0; i<S.len-1; i++ )  // последовательно в контрольном интервале
    if( intoi( S[i].z,V.z,S[i+1].z ) ) // смежные полосы не повторяют аппликаты
    { x = Inter( V.z,S[i].z,S[i+1].z,S[i].x,S[i+1].x ); // x - идёт вне штевней
      if( j<0 ){ j=i; mx=x; n=k; if( m<0 )m=k; } else if( inx( mx,x ) )mx=x;
    } if( j>=0 )Y[k].x=mx;
  }
  if( m>0 )for( k=0; k<m; k++ )Y[k].x=Y[m].x;  // заграничные экстраполяции при
  if( n>=0 )for( k=n+1; k<Y.len; k++ )Y[k].x=Y[n].x; // абсциссах крайних точек
  //
  //   теперь необходимо все координаты собрать в единый фрагмент для кривой
  //   штевней с оконтуриванием транцевых расширений и оконечных утолщений
  //
  if( Y.len==1 )
  { V=Y[0];         // одна точка означает одинаковое утолщение по всему штевню
    if( S.len==1 )  // если на штевне одна точка, то возможно добавление другой
    { S[0].y=V.y; if( V.z!=S[0].z ){ V.x=S[0].x; S.Insert( V.z>S[0].z )=V; }
    } else          // все утолщения одинаковы, и точка расширения не пропадает
    for( i=0; i<S.len; i++ )
    { S[i].y=V.y;
      if( !i && V.z<S[0].z )j=0; else
      if( i==S.len-1 && V.z>S[-1].z )j=S.len-2; else
      if( i!=0 && intox( S[i-1].z,V.z,S[i].z ) )j=i; else continue;
      V.x=Inter( V.z,S[j].z,S[j+1].z,S[j].x,S[j+1].x ); S.Insert( j )=V;
  } } else
  //          точка штевня обязана попадать точно в угол четырехугольника {x,z}
  //                   на кривой ординат, то есть, под его самое первое касание
  //                   или подводное утолщение штевня по нижней ординате транца
  { for( V=Y[0],i=0; i<S.len-1; i++ )// подкильный участок транцевых расширений
     if( intoi( S[i+1].z,V.z,S[i].z ) // здесь полагаются все ординаты нулевыми
      && intoi( S[i+1].x,V.x,S[i].x ) ){ for( j=i-1;j>=0;j-- )Y/=S[j]; break; }
    for( V=Y[m=Y.len-1],j=S.len-1; j>i; j-- )        // надпалубный фрагмент в
     if( intoi( S[j].z,V.z,S[j-1].z )                // диаметральной плоскости
      && intoi( S[j].x,V.x,S[j-1].x ) ){ for(++j;j<S.len;j++ )Y+=S[j]; break; }
    //
    //   поиск верхнего слома на штевне для включения старой последовательности
    //
    for( n=1; n<S.len; n++ )
     if( St && S[n].z>=0 && S[n].z<=S[n-1].z && S[n].x>S[n-1].x )break; else
     if( !St && S[n].z>=0 && S[n].z<=S[n-1].z && S[n].x<S[n-1].x )break;
    //
    //!   основной блок формирование контуров штевней с транцевыми расширениями
    //
    for( k=i; k<m; k++ )      // k: [i..m]
    { for( j=0; j<=n; j++ )   // расстановка интерполированных точек из штевней
      { V=S[j];               //  n - ограничивает выборку козырьком фальшборта
        for( i=j+1; i<=n; i++ )if( S[i].z<=V.z )j=i; V=S[j];       //??? скачок
        if( intoi( Y[k].z,V.z,Y[k+1].z ) )         // по неоднозначным функциям
        { V.y=Inter(V.z,Y[k].z,Y[k+1].z,Y[k].y,Y[k+1].y); // транцевых контуров
          if( Y[k]!=V && Y[k+1]!=V ){ Y.Insert( ++k )=V; ++m; }
    } } }
    //!   в заключение неоднозначные надпалубные обгрызки надстроек
    //

    //!   переброс векторной кривой с возвращением ординатной сборки на штевни
    //
    for( S.len=i=0; i<Y.len; i++ )S+=Y[i];
  }
  for( i=1,k=0; i<S.len; i++ ){ if( S[k]!=S[i] )k++; if( k!=i )S[k]=S[i]; }
  if( S.len ){ V=S[0];  if( V.y ){ V.y=0.0; S/=V; }    // так, на всякий случай
               V=S[-1]; if( V.y ){ V.y=0.0; S+=V; } }  //   обнуление оконцовок
}
#else
#endif
//     сопровождение векторного списка с помощью индексов для оболочки Shell
//
struct uList{ unsigned *m; int n,len;
  uList(): m( NULL ),n(0),len(0){} ~uList(){ if( m )free( m ); m=0; n=len=0; }
  unsigned& operator[]( int ); unsigned& operator+=( unsigned );     // [k%len]
};                 // *m-вектор индексных масок, n-истинная и len-текущая длина
unsigned& uList::operator[]( int k ){ return m[minmax(0,k>=0?k:k+len,len-1)]; }
unsigned& uList::operator+=( unsigned p ) // выбор [k] обраткой и += дополнение
{ if(len>=n)m=(unsigned*)realloc(m,(n+=96)*sizeof(unsigned));return m[len++]=p;
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
  { Flex L,R; Vector V,W;            // векторы шпангоутных контуров по шпациям
   Real x,y,z; int i,j,k,l,m,n;      // ... точки по циклам двойной вложенности
    Ofs=Zero;                        // приведение к миделю и основной линии OЛ
    if( Str=strchr( Str,'<' ) )      // подзаголовок проекта в угловых скобках
    if( s=strchr( ++Str,'>' ) )      // ?если нет названия проекта <- имя файла
    { *s=0; while( *Str<=' ' )++Str;
      free( ShipName ); ShipName=strdup( strcut( Str )?Str:FileName );
    }
    sscanf( stringData( Fh ),"%d%d",&Nframes,&Mid );
    sscanf( stringData( Fh ),"%lf%lf%lf%lf",&Length,&Breadth,&Draught,&Ofs.z );
    Keel=(Real*)Allocate( (Nframes+2)*sizeof(Real),Keel );  // килевая разметка
    Frame=(Flex*)Allocate( (Nframes+2)*sizeof(Flex),Frame );// точки шпангоутов
    for( i=0; i<=Nframes+1; i++ )Frame[i].len=0;    // вычистка всех шпангоутов
    Stem.len=Stern.len=L.len=R.len=0;  // расчистка и обнуление адресных ссылок
    if( newDraught )Draught=newDraught;// новое пересчитывание с переустановкой
    //                                                    конструктивной осадки
    //    Ахтерштевень (начало считывания последовательного потока данных)
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
      { z=strtod( s,&s ); y=strtod( s,&s );       // затем аппликаты и ординаты
        if( !i )if( y>0.0 )Frame[k]+=(Vector){ x,0,z };
                           Frame[k]+=(Vector){ x,y,z };
      } if( i>0 && Frame[k].len>0 )if( y>0.0 )Frame[k]+=(Vector){ x,0,z };
    }
    // Форштевень
    //  отсчет абсцисс шпангоутов и штевней ведется от кормового перпендикуляра
    //
    k=strtol( stringData( Fh ),&s,0 );
    for( V.x=0,i=0; i<k; i++ )V.z=strtod( s,&s ),V.y=strtod( s,&s ),R+=V;
    n=strtol( stringData( Fh ),&s,0 );
    for( V.y=0,i=0; i<n; i++ )V.z=strtod( s,&s ),V.x=strtod( s,&s ),Stem+=V;
                                            //
    fclose( Fh );                           // работа с файлом данных завершена
    Alliance( Stern,L,true,Keel[1] );       // первая перестройка штевней после
    Alliance( Stem,R,false,Keel[Nframes] ); // считывания всех данных о корпусе
    //
    //    Чтение последовательного потока завершено, но если смещена основная
    //    плоскость, то необходим проход по всем точкам плазовых аппликат
    //
    Ofs.z+=Draught; // все отсчёты аппликат приводятся к действующей ватерлинии
    Ofs.x=Nframes>2?minmax( Keel[1],Keel[Mid],Keel[Nframes] )
                         :( Keel[1]+Keel[Nframes] )/2;      // крайние пополам
                         Control( Stern,Ofs,true );         // контроль и сдвиг
    for( n=0; n<=Nframes+1; n++ )                           // для новой точки
       { Keel[n]-=Ofs.x; Control( Frame[n],Ofs ); }         // пересечения
                         Control( Stem,Ofs,true );          // ватерлинии
    //
    //   Экстремумы для килевой линии и новые крайние шпангоуты
    //       ~~ крайние точки на килевой или палубной аппликате
    //       == становятся крайними на шпангоутах
    //
    for( Keel[0]=Keel[1],i=0; i<Stern.len; i++ ){ const Vector &S=Stern[i];
      if( S.x<Keel[0] )Keel[0]=S.x; // нижний экстремум абсциссы по ахтерштевню
      if( S.x<Keel[1] )Frame[0]+=S; // кривой шпангоут в кормовой оконечности
    }                               // осредняем предыдущий к пустому шпангоуту
    if( !Frame[0].len ){ for( z=i=0; i<Frame[1].len; i++ )z+=Frame[1][i].z;
                          if( i )Frame[0]+=(Vector){ Keel[0],0,z/i };
                       } n=Nframes; // носовая оконечность на конечных отсчётах
    for( Keel[n+1]=Keel[n],i=0; i<Stem.len; i++ ){ const Vector &S=Stem[i];
      if( S.x>Keel[n+1] )Keel[n+1]=S.x;  // экстремум абсциссы по форштевню
      if( S.x>Keel[n] )Frame[n+1]+=S;    // с образованием свободного шпангоута
    }                                    //        который может и не сложиться
    if( !Frame[n+1].len ){ for( z=i=0; i<Frame[n].len; i++ )z+=Frame[n][i].z;
                            if( i )Frame[n+1]+=(Vector){ Keel[n+1],0,z/i };
                         }
    //    Здесь пробуем расставить сразу все точки с координатами на штевнях
    //       относительно диаметральной плоскости в продолжение и в разрывах
    //           исходных шпангоутных контуров
    //
   Real zu,zd,z0,z1;
   bool rb,li,lfr=false;    // выбор форштевеня со смежными правыми шпангоутами
    do
    for( n=1; n<=Nframes; n++ )
    { Flex *F,&S = lfr ? Stern : Stem;            // фиксация ссылки на штевень
      i=l=0;                 // изначально полагается нижняя точка на шпангоуте
    Rep: li=lfr;             // выборка цельного фрагмента штевня внутри шпации
      for( j=-1; i<S.len; i++ )                   // подборка точек на штевнях
      if( intoi( Keel[n],S[i].x,Keel[n+1] ) )     // в пределах одной шпации
      {      if( j<0 )j=l=i; else ++l;            // индекс i будет зашкаливать
      } else if( j>=0 ){ if( i<j )--i; break; }                     // получен интервал от j до l
             if( j<0 )continue;                   // фрагмент контура не найден
      //
      //     экстремумы аппликат считаются с учетом крайних точек на шпангоутах
      //
      zu=zd=S[j].z;
      for( k=j+1; k<=l; k++ ){ Real Z=S[k].z; if( zd>Z )zd=Z; if( zu<Z )zu=Z; }
      //
      //     сначала определяются подкильные направления по контурам штевней
      //
      if( zd<=(z0=Frame[n][0].z) || zd<=(z1=Frame[n+1][0].z) ) // килевые точки
      { if( z0!=z1 )li=z0>z1; F = &Frame[n+!li]; x=Keel[n+li]; //  у шпангоутов
        if( lfr^li )
          { //if( S[j].z==(*F)[0].z )(*F)[0].y=S[j++].y;
            if( S[l].x==Keel[n+li] )--l; for( m=j; m<=l; m++ )*F/=S[m];
          } else
          { //if( S[l].z==(*F)[0].z )(*F)[0].y=S[l--].y;
            if( S[j].x==Keel[n+li] )++j; for( m=l; m>=j; m-- )*F/=S[m];
          }
//        { for( m=j; m<=l; m++ )if( S[m].x!=x )*F/=S[m]; } else
//        { for( m=l; m>=j; m-- )if( S[m].x!=x )*F/=S[m]; }
        if( i<S.len )goto Rep; else { i=l; continue; }
      }
      //              и по верхним палубам всё иначе - только внутренний контур
      //
      if( zu>=(z0=Frame[n][-1].z) || zu>=(z1=Frame[n+1][-1].z) )
      { if( z0!=z1 )li=z0<z1; F = &Frame[n+!li]; x=Keel[n+li];    // привязка к
        if( lfr^li )                       // шпангоуту и исключение повторений
          { if( S[j].x==Keel[n+li] )++j; for( m=l; m>=j; m-- )*F+=S[m]; } else
          { if( S[l].x==Keel[n+li] )--l; for( m=j; m<=l; m++ )*F+=S[m]; }
//        { for( m=l; m>=j; m-- )if( S[m].x!=x )*F+=S[m]; } else
//        { for( m=j; m<=l; m++ )if( S[m].x!=x )*F+=S[m]; }
        if( i<S.len )goto Rep; else { i=l; continue; }            // к следующей шпации
      }
      //    внутри контура шпангоута (здесь требуется тщательная отработка >=<)
      //                                      отсечка по корме и в сторону носа
      if( lfr ){ if( S[j].x==Keel[n+1] )++j; if( S[l].x==Keel[n+1] )--l; }
          else { if( S[j].x==Keel[n] )++j; if( S[l].x==Keel[n] )--l; }
      for( F=&Frame[n+!lfr],k=1; k<F->len; k++ )     // сначала выбор шпангоута
       if( intor( (*F)[k-1].z,(zu+zd)/2,(*F)[k].z ) )
//     { for( m=j; m<=l; m++,k++ )                   // снизу - вверх
//        if( (*F)[k]!=S[m] )F->Insert( k )=S[m]; break;
       { for( m=l; m>=j; m-- )F->Insert( k )=S[m]; break;  // всё лепим на F[k]
       }
       if( i<S.len )goto Rep; else i=l;
    } while( lfr^=true );  // сначала форштевень=false, затем ахтерштевень=true
    //
    //    как-то надо подчистить рассогласования в полученной подкильной полосе
    //                                   крайние шпангоуты-заглушки не правятся
/*  for( n=1; n<=Nframes; n++ ){ Flex &F=Frame[n];
      for( k=0; k<F.len-2; k++ )if( F[k].x!=Keel[n] && F[k+1].x==Keel[n] )
      { if( F[k+1].y>=F[k+2].y || F[k+1].z>=F[k+2].z )F.Delete( k+1 ); break;
    } }
*/  //!  Собираем треугольники в оболочку Shell "как есть" с веером у днища на
    //  основной линии для последующей оптимизации от киля по максимуму площади
    //  треугольников относительно половины квадрата минимального ребра
    //                   ... вначале треугольники в расширении кормового транца
    //
    Shell=(unsigned**)Allocate( (Nframes+1)*sizeof(unsigned*) );     //! [0..N]
    //!                                                               индексный
    //  сначала просто шпангоуты - начинаются от киля вверх до палубы/ширстрека
    //          и плазовые ординаты разбрасываются парами точек
    //            по смежным шпангоутам внутри расчетной шпации
   uList rf,lf;                       // индексы доступа к точкам
   int sl,sr,zl,zr;                   // на шпангоутах (теперь без штевней)
    for( n=1; n<=Nframes+1; n++ )     // начальные точки совпадают, и ненулевые
    { L.len=R.len=lf.len=rf.len=0;
      //
      //   раздельная несинхронная выборка контурных точек штевней и шпангоутов
      //   с образованием простых координатных последовательностей, и ставятся
      //   естественные (неединичные) абсциссы шпангоутов для угловых измерений
      //
      //   здесь выполняется выборка точек в новые контуры числовых шпангоутов,
      //   и вышибаются рассогласования у килевой линии до первых значимых хорд
      //           ширина всех шпаций теперь не устанавливается одинаковой в 1м
      //
      for( x=Keel[n-1],k=0; k<Frame[n-1].len; k++ )  // слева без левых загибов
       if( (V=Frame[n-1][k]).x>=x ){ L+=V; lf+=unsigned(k)|LeftFrame; }
      for( x=Keel[n],k=0; k<Frame[n].len; k++ )// справа без правых искривлений
       if( (V=Frame[n][k]).x<=x ){ R+=V; rf+=unsigned( k ); }
      //
      //  исправления, как бы для согласования и выравнивания геометрических
      //  особенностей с выверкой нулевого контура в диаметральной плоскости
      //
      for( x=Keel[n-1],i=0; i<L.len; i++ )
         { if( L[i].x>x )L[i].y=0,L[i].x=x; else L[i].y/=3; }
      for( x=Keel[n],i=0; i<R.len; i++ )
         { if( R[i].x<x )R[i].y=0,R[i].x=x; else R[i].y/=3; }
      //
      //! оптимизация последовательности треугольников покрытия судовой обшивки
      //
      Ins( n )=lf[0],Ins( n )=rf[0];   // левая точка впереди - это существенно
      sl=sr=1;                 // поиски с третьих по контурам шпангоутов точек
      if( L.len>1 && R.len>1 ) // четырёхугольник отодвигается от диаметральной
      { if( norm( R[1]-L[0] )>=norm( R[0]-L[1] ) )                 // плоскости
        { Ins( n )=lf[1]; Ins( n )=rf[1];
          for( i=2; i<L.len; i++ )if( norm( R[1]-L[i] )<norm( R[1]-L[i-1] ) )
             { Ins( n )=lf[i]; sl=i; } else break;
        } else
        { Ins( n )=rf[1]; Ins( n )=lf[1];
          for( i=2; i<R.len; i++ )if( norm( R[i]-L[1] )<norm( R[i-1]-L[1] ) )
             { Ins( n )=rf[i]; sr=i; } else break;
      } } else                                         // веер из треугольников
      { for( i=1; i<L.len; i++ )Ins( n )=lf[i];        //  либо далее до другой
        for( i=1; i<R.len; i++ )Ins( n )=rf[i]; continue; // к следующей шпации
      }                            // пропуск всех подводных и прикилевых точек
      if( (zl=sl)<5 )zl=min( 5,L.len-1 ); while( zl<L.len-1 && L[zl].z<0 )++zl;
      if( (zr=sr)<5 )zr=min( 5,R.len-1 ); while( zr<R.len-1 && R[zr].z<0 )++zr;
      //
      //!  перенастройка и продолжение оптимизации линий бортовых сломов
      //                         опорные точки s# уже занесены в обшивку
      //                         контрольные z# только ожидают обработки
      //
      while( zl<L.len || zr<R.len )
      { //       предварительный просмотр поверхности с поиском бортовых сломов
        // выбирается углы хорд относительно обоих локалей шпангоутных контуров
       const Real C12=0.9781476007,C20=0.9396926208,C24=0.9135454576,
                  S08=0.139173101,S12=0.2079116908;
       int l=L.len-zl+1,r=R.len-zr+1; // вначале поиске всего от киля до палубы
        //
        //   как бы полупроход с поиском сломов и совпадающих точек
#if 0
        do
        { if( l ){ if( ++zl>=L.len-1 || L[zl]==L[zl+1]
                   || dir( L[zl+1]-L[zl] )%dir( L[zl]-L[zl-1] )<C12 )
                   { l=false; if( r )r=R.len-zr+3; } else --l; }
          if( r ){ if( ++zr>=R.len-1 || R[zr]==R[zr+1]
                   || dir( R[zr+1]-R[zr] )%dir( R[zr]-R[zr-1] )<C12 )
                   { r=false; if( l )l=L.len-zl+3; } else --r; }
          if( !l || !r ){ V=dir( R[sr]-L[sl] );
                          W=dir( R[zr]-L[zl] ); if( W.x<.88||W%V<C20 ) break; }
        } while( r || l );
#else
        if( sl==zl && sr==zr )                // обход начальных предустановок
        while( l>0 || r>0 )                   // по индексам бортовых сломов
        { if( zl>=L.len-2 || zr>=R.len-2 )    // палуба перекрывается без каких
            { zl=L.len; zr=R.len; break; }    //  - либо дополнительных условий
          W = dir( R[zr+1]-L[zl] ),           // подъём к правой стороне шпации
          V = dir( R[zr]-L[zl+1] );           // +по левому кормовому шпангоуту
          if( ( rb=(W.x>V.x) )==true )goto Rst;
        Lst:
          if( l>0 ){ ++zl; if( dir( L[zl+1]-L[zl] )%dir( L[zl]-L[zl-1] )<C12
                                || L[zl]==L[zl+1] ){ l=0; if( r>0 )r=2; } else
                        { if( V.x<C20 && r>0 )++zr,r=0; else --l; } continue; }
          if( rb )continue;
        Rst:
          if( r>0 ){ ++zr; if( dir( R[zr+1]-R[zr] )%dir( R[zr]-R[zr-1] )<C12
                                || R[zr]==R[zr+1] ){ r=0; if( l>0 )l=2; } else
                        { if( W.x<C20 && l>0 )++zl,l=0; else --r; } continue; }
          if( rb )goto Lst;
        }
#endif
        // теперь выбор допустимо малого угла отклонения ребра от горизонтали
        //
        W=dir( R[zr]-L[zl] ); if( W.x<C24 )
        { if( W.z>S08 )for( i=zr-1; i>sr; i-- )
            { V=dir( R[i]-L[zl] ); if( V.z>=S12 )zr=i; else break; } else
          if( W.z<-S08 )for( i=zl-1; i>sl; i-- )
            { V=dir( R[zr]-L[i] ); if( V.z<=-S12 )zl=i; else break; }
        }
        // zl=L.len; zr=R.len;
        //
        // с поиском новой и наиболее параллельной грани в текущих ограничениях
        //
        while( sl<zl || sr<zr )             // завершающий веер треугольников
        if( sl>=zl )Ins( n )=rf[++sr]; else // установка обоих точек для нового
        if( sr>=zr )Ins( n )=lf[++sl]; else // четырехугольника, по возможности
#if 1
        { W=dir( R[sr+1]-L[sl] );
          V=dir( R[sr]-L[sl+1] ); if( W.x>V.x )Ins( n )=rf[++sr];
                                         else  Ins( n )=lf[++sl];
#else
        { Real vr,vl;
          if( (vr=norm(R[sr+1]-L[sl]))>(vl=norm(R[sr]-L[sl+1])) )
          {                             Ins( n )=lf[++sl];
            if( norm(R[sr+1]-L[sl])<vl )Ins( n )=rf[++sr];
          } else {                      Ins( n )=rf[++sr];
            if( norm(R[sr]-L[sl+1])<vr )Ins( n )=lf[++sl];
          }
#endif
        }
        //   особая перепроверка (=пропуск) зараз всех смежных совпадающих точек
        //
        while( sl<L.len-1 )if( L[sl]==L[sl+1] )zl=++sl; else break;
        while( sr<R.len-1 )if( R[sr]==R[sr+1] )zr=++sr; else break;
    } }
    //  небольшая перенастройка визуализации графической сцены с кораблем
    //
    Distance=-2.4*sqrt(sqr(Length)+sqr(Breadth*2)+sqr(Draught*4)); // дальность
    eye=(Vector){ 45,-15 },look=(Vector){ -5 }; // векторы ориентации и взгляда
    return true;
  }
Bk: textcolor( YELLOW,RED ),
    cprint( 2,7,"  >>> отмена или ошибка чтения данных: %s",FileName ),
    textcolor( LIGHTGRAY,BLACK );         free( ShipName ); ShipName=0;
  return false;
} /*
C06=0.9945218954,C08=0.9902680687,C12=0.9781476007,C18=0.9510565163,C20=0.9396926208,
C24=0.9135454576,C30=0.8660254038,C45=0.70710067812,C48=0.6691306064,C75=0.2588190451,
C78=0.2079116908,C82=0.139173101,C86=0.06975647374 ...cos(24°), 78°=sin(12°)
S06=0.1045284633,S12=C78,S08=C82,S04=C86; */
