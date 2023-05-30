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
//      здесь перебираются строки с исключением пропусков и чистых комментариев
//
static char *stringData( FILE *F )      // поиск значимой строки в файле данных
{ char *s; int Len=0;
  do{ s=getString(F); if( s[0]==';' || !memcmp(s,"//",2) )*s=0; Len=strcut(s);
    } while( !feof( F ) && !Len ); return s;
}
// попробуем организовать перебор шпаций и точек на шпангоутах с учетом штевней
//
static Real mzL=-1.0,mzR=-1.0; // нижний уровень точек шпангоута за фальшбортом

bool Hull::RNext( int sp,unsigned &m,Vector &R,byte *Sm ) // s!=sp[0..N]->в нос
{ static int s=-1,k=0; int i,j;          // не предполагается повторного чтения
  if( s!=sp ){ s=sp; k=0; }               // выбор нового левого шпангоута sp
  if( !s )                                // левее других шпангоутов больше нет
  { if( k<Frame[0].length )                 // на последовательное перечисление
    { R=Frame[0][m=k++]; return true; }  // пробная точка правее ахтерштевня
  } else
  if( s<Nframes )                           // иначе выбор вправо внутри шпации
  { Flex &G=Frame[s];
    for( j=-1,i=0; i<Stem.length; i++ )if( !Sm[i] ) // выбор точек на форштевне
    { _Vector S=Stem[i];
      if( mzR>0.0&&S.z>=mzR )break;  // правый шпангоут загибается за фальшборт
//    if( S==G[k] ){ k++; j=i; } else        // в приоритете точка на форштевне
      if( S==G[k] ){ Sm[i]=2; break; }       // в приоритете точка на шпангоуте
      if( Keel[s-1]<=S.x && S.x<=Keel[s] )  //!<= шпации с накрытием шпангоутов
      { if( !k ){ if( S.z<=G[0].z )j=i; } else    // точки под форштевнем и над
        if( k>=G.length ){ if( S.z>=G[k-1].z )j=i; } else    // верхней палубой
        if( S.z>=G[k-1].z && S.z<=G[k].z && !G[k-1].y && !G[k].y )j=i;// внутри
      }
      if( j>=0 ){R=S; m=StemPost|j; return Sm[j]=S.x<Keel[s]?1:2; } //= штевень
    } if( k<G.length ){ R=G[k]; m=k++; return true; }  //номер точки шпангоута
  } else                                             // =Nframes крайние справа
  { Flex &G=Frame[s-1];
    for( i=0; i<Stem.length; i++ )if( !Sm[i] )   // точки впереди форштевня
    { _Vector S=Stem[i];                        // формируют независимую шпацию
      if( Keel[s-1]<=S.x ){ R=S; m=StemPost|i; return Sm[i]=Keel[s-1]<=S.x?1:2; }
    }                                             // из свободных треугольников
    if( !Stem.length && k<G.length )                       // один раз делается
    { R=G[k=G.length-1]; m=LeftFrame|k++; return true; }   // транцевая затычка
  } return false;             // сваливается в одну путаную бесконтрольную кучу
}                                                             // от кормы в нос
bool Hull::LNext( int sp,unsigned &m,Vector &L,byte *St )     //    s!=sp[0..N]
{ static int s=-1,k=0;         // одноразовый подход целиком ко всему шпангоуту
  if( s!=sp ){ s=sp; k=0; }    // фиксируется правый шпангоут, левый - в поиске
  if( s==Nframes )             // правее шпангоутов нет, есть только форштевень
  { if( k<Frame[s-1].length )  // обычная поисковая фиксация ++ по перечислению
      { L=Frame[s-1][k]; m=LeftFrame|k++; return true; }
  } else
  if( s>0 )                          // вариант со средними шпациями-трапециями
  { Flex &G=Frame[s-1];
    for( int j=-1,i=0; i<Stern.length; i++ )if( !St[i] )
    { _Vector S=Stern[i];
      if( mzL>0.0 && S.z>=mzL )break; // левый шпангоут загибается за фальшборт
//    if( S==G[k] ){ k++; j=i; } else      // в приоритете точка на форштевне
      if( S==G[k] ){ St[i]=1; break; }     // в приоритете точка на шпангоуте
      if( Keel[s-1]<=S.x && S.x<=Keel[s] ) //!<= правому шпангоуту временная отметка
      { if( !k ){ if( S.z<=G[0].z )j=i; } else
        if( k>=G.length ){ if( S.z>=G[k-1].z )j=i; } else
        if( S.z>=G[k-1].z && S.z<=G[k].z && !G[k-1].y && !G[k].y )j=i;
      }
      if( j>=0 ){ L=S; m=SternPost|j; return St[j]=S.x<Keel[s]?1:2; }
    } if( k<G.length ){ m=LeftFrame|k; L=G[k++]; return true; }
  } else      // точки за ахтерштевнем формируют шпацию свободных треугольников
  { Flex &G=Frame[0];
    for( int i=0; i<Stern.length; i++ )if( !St[i] )// перебор прикаянных вершин
    { _Vector S=Stern[i];
      if( S.x<=Keel[0] ){ L=S; m=SternPost|i; return St[i]=S.x<=Keel[0]?1:2; }
    }
    if( !Stern.length && k<G.length ){ m=k=G.length-1; L=G[k++]; return true; }
  } return false;
}
static Vector e5( _Vector W ) // округление записи для точных сравнений 0.01 мм
{ return (Vector){ round( W.x*1e5 )/1e5,
      W.y<=eps?0.0:round( W.y*1e5 )/1e5,
                   round( W.z*1e5 )/1e5 };
}
//    Интерполяция с разрезанием шпангоутов для фиксации точек на ватерлинии
//     ... отчасти исключаются из рассмотрения вершины пустых треугольников
//
static void Control( Flex &FL, _Vector Ofs, bool Stm=false )
{ for( int k=0;; k++ )if( k>=FL.length )break; else
  { Vector &A=FL[k]; A-=Ofs;                                   //! осторожность
    if( A.z && k>0 ){ _Vector B=FL[k-1];                       // В-уже изменен
      if( A.z*B.z<0 ){ if( fabs( A.z )<eps )A.z=0.0; else
        if( Stm || A.y || B.y )FL.Insert( k++ )=e5( B-(A-B)*(B.z/(A.z-B.z)) );
  } } }
}
struct Lint{ Real *_Y,*_X; const int len;      // ~~~ длина изначально известна
  Lint( int L ); ~Lint(){ free( _X ); }        // конструкторы временной памяти
  Real operator()( _Real A );                  // ... для линейной интерполяции
  Real& operator()( int k );                   // X индексная выборка аргумента
  Real& operator[]( int k );                   // Y - и собственно функции
};
Lint::Lint( int L ):len( L ){ _Y=( _X=(Real*)malloc( L*sizeof(Real)*2 ) )+L; }
Real& Lint::operator[]( int k ){ return _Y[minmax( 0,k,len-1 )]; }
Real& Lint::operator()( int k ){ return _X[minmax( 0,k,len-1 )]; }
Real Lint::operator()( _Real A )               // одна точка выходит константой
{ if( len<2 )return len?_Y[0]:0.0; int k; Real *x=_X;      // либо рассечение в
  for( k=0; k<len-1 && A>x[1]; k++,x++ ); Real *y=_Y+k,a,h; // парном интервале
  if( !(h=x[1]-x[0]) )return (y[0]+y[1])/2;             // среднеарифметическое
  if( !(a=A-x[0]) )return y[0];                         // или точное попадание
  return y[0]+a*(y[1]-y[0])/h;                          //?? и беда при малом h
}
//     Считывание корпуса отмечается успехом, либо полным завершением программы
//
bool Hull::Read( const char FName[],   // здесь имя приходит в кодировке W-1251
                 _Real newDraught )    // переустановка конструктивной осадки
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
  { Vector V; Real x,y,z;            // здесь продолжается серия проверок
    int i,j,k,n;                     //  ... или к циклу двойной вложенности
    Ofs=Zero;                        // приведение к миделю и основной линии OЛ
    if( Str=strchr( Str,'<' ) )
    if( s=strchr( ++Str,'>' ) )      // выборка имени проекта в угловых скобках
    { *s=0; while( *Str<=' ' )++Str; // ?если нет названия проекта <- имя файла
      free( ShipName ); ShipName=strdup( strcut( Str )?Str:FileName );
    }
    sscanf( stringData( Fh ),"%d%d",&Nframes,&Mid );
    sscanf( stringData( Fh ),"%lf%lf%lf%lf",&Length,&Breadth,&Draught,&Ofs.z );
    Keel=(Real*)Allocate( (Nframes)*sizeof(Real),Keel );    // килевая разметка
    Frame=(Flex*)Allocate( (Nframes)*sizeof(Flex),Frame );  // точки шпангоутов
    for( i=0; i<Nframes; i++ )Frame[i].length=0;    // вычистка всех шпангоутов
    Stem.length=Stern.length=0;        // расчистка и обнуление адресных ссылок
    if( newDraught )Draught=newDraught;// новое пересчитывание с переустановкой
    //                                                    конструктивной осадки
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
                    else z=B(j),y=B[j],x=A(z),j++; V=e5( (Vector){ x,y,z } );
          if( fs && y>0.0 )Stern+=(Vector){ V.x,0,V.z };      // крайние точки
              fs=false;    Stern+=V;                          // занулённые
        } if( !fs && y>0.0 && Stern.length>0 )Stern+=(Vector){ V.x,0,V.z };
    } }
    //   Собственно корпус ( - в том же последовательном потоке )
    //
    for( k=0; k<Nframes; k++ )           // перебор от ахтерштевня до форштевня
    { n=strtol( stringData( Fh ),&s,0 ); // количество теоретических шпангоутов
      Keel[k]=x=strtod( s,&s );          // абсциссы шпангоутов килевой дорожки
      for( i=0; i<n; i++ )    // в начале количество точек и абсцисса шпангоута
      { z=strtod( s,&s ); y=strtod( s,&s );       // затем аппликаты и ординаты
        if( !i )if( y>0.0 )Frame[k]+=(Vector){ x,0,z };
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
                  else    z=B(j),y=B[j],x=A(z),j++; V=e5( (Vector){ x,y,z } );
          if( fs && y>0.0 )Stem+=(Vector){ V.x,0,V.z }; // только нижняя точка
              fs=false;    Stem+=V;                     // зануляется автоматом
        } if( !fs && y>0.0 && Stem.length>0 )Stem+=(Vector){ V.x,0,V.z };
      }
    } fclose( Fh );                         // работа с файлом данных завершена
    //
    //   == ещё и ещё раз новая попытка снять неоднозначности штевней ==
    //
    for( n=0; n<Nframes; n++ )
    { Flex &F=Frame[n];
      for( i=0; i<F.length; i++ )
      { for( k=Stem.length-1; k>=0; k-- )if( F[i]==Stem[k] )Stem.Delete( k );
        for( k=Stern.length-1; k>=0; k-- )if( F[i]==Stern[k] )Stern.Delete( k );
    } }
    //    Чтение последовательного потока завершено, но если смещена основная
    //    плоскость, то необходим проход по всем точкам плазовых аппликат
    //
    Ofs.z+=Draught; // все отсчёты аппликат приводятся к действующей ватерлинии
    Ofs.x=Nframes>2?minmax( Keel[1],Keel[Mid],Keel[Nframes-2] )
                         :( Keel[0]+Keel[Nframes-1] )/2;     // крайние пополам
                         Control( (Stern),Ofs,true );            // контроль
    for( n=0; n<Nframes; n++ )                                   // со сдвигом
       { Keel[n]-=Ofs.x; Control( (Frame[n]),Ofs ); }            // пересечения
                         Control( (Stem),Ofs,true );             // ватерлинии
    //
    //   предустановка точки на ватерлинии в случае отсутствия контуров штевней
    //
    if( !Stern.length ){ V=Frame[0][k=Frame[0].length-2];
      if( k>2 )if( V.z>0 && Frame[0][k-2].z<0 )Frame[0]+=(Vector){ V.x,0,0 }; }
    if( !Stem.length ){ n=Nframes-1; V=Frame[n][k=Frame[n].length-2];
      if( k>2 )if( V.z>0 && Frame[n][k-2].z<0 )Frame[n]+=(Vector){ V.x,0,0 }; }
    //
    //  Собираем треугольники в оболочку Shell "как есть" с веером у днища
    //  на основной линии для последующей оптимизации от киля по максимуму
    //  площади треугольников относительно половины квадрата минимального ребра
    //    0 - расширение ахтерштевня <<~шпации~>> на форштевне - Nframes+2
    //     .. вначале треугольники в расширении кормового транца
    //
 // Shell=(unsigned**)Allocate( Nframes+3,24*sizeof(unsigned) );  //! двумерный
    Shell=(unsigned**)Allocate( (Nframes+3)*sizeof(unsigned*) );  //!  адресный
    //
    //!  Транцевые площадки собираются по ординатам на штевнях
    //
    for( n=0; n<=Nframes+2; n+=Nframes+2 )         // асимметричная расстановка
    { Flex &S = !n ? Stern:Stem;                   // транцевых треугольников
      for( i=0; i<S.length; i++ )Ins( n )=i;
    }
    //  сначала просто шпангоуты - начинаются от киля вверх до палубы/ширстрека
    //    и плазовые ординаты разбрасываются парами точек
    //      по смежным шпангоутам внутри расчетной шпации
    //
   byte *St=(byte*)calloc( sizeof( byte ),Stern.length+2 ), // признаки учёта+2
        *Sm=(byte*)calloc( sizeof( byte ),Stem.length+2 );  // точек на штевнях
#if !__x86_64__ //! __i386__
   const Real C24=0.9135454576;                  // cos( 24° ) и 78°=sin( 12° )
   struct uList // *m-вектор индексных масок, n-истинная и length-текущая длина
   { unsigned *m; int n,length;
     uList(): m( NULL ),n( 0 ),length( 0 ){} ~uList(){ if( m )free( m ); }
     unsigned& operator[]( int k ){ return m[minmax(0,k,length-1)]; } // доступ
     unsigned& operator+=( unsigned p )                   // простое дополнение
     { if( length>=n )m=(unsigned*)realloc( m,(n+=128)*sizeof( unsigned ) );
       return m[length++]=p;
   } }          lf,rf;     // метки доступа к точкам на шпангоутах и на штевнях
   static Flex  Lf,Rf;     // векторы точек для шпангоутных контуров по шпациям
   Vector RS,rs,LS,ls; Real VR,VL; unsigned u; int sl,sr,zl,zr;

    for( n=0; n<=(k=Nframes); n++ ) // начальные точки совпадают, они ненулевые
    {
      Lf.length=Rf.length=lf.length=rf.length=0; sl=sr=2; zl=zr=2; u=0;
      //
      //   несинхронная выборка контурных точек на штевнях и шпангоутах
      //   с образованием простых координатных последовательностей, и ставятся
      //   естественные (неединичные) абсциссы шпангоутов для угловых измерений
      //
      if( n )  // оставляем первый бит для точек без повторений на пересечениях
      { for( i=0; i<Stem.length; i++ )Sm[i]&=1;  // снятие временных блокировок
        for( i=0; i<Stern.length; i++ )St[i]&=1; // при выборе точек на штевнях
      }
      //   здесь выполняется выборка точек в новые контуры числовых шпангоутов,
      //   и вышибаются рассогласования у килевой линии до первых значимых хорд
      //           ширина всех шпаций теперь не устанавливается одинаковой в 1м
      //
      for( x=n?Keel[n-1]:Keel[0]*2-Keel[1]; LNext( n,u,V,St ); )
      { V.x=x; V.y/=3; if( lf.length==1 )if( V==Lf[0] )continue; Lf+=V,lf+=u; }
      for( x=n<Nframes?Keel[n]:Keel[n-1]*2-Keel[n-2]; RNext( n,u,V,Sm ); )
      { V.x=x; V.y/=3; if( rf.length==1 )if( V==Rf[0] )continue; Rf+=V,rf+=u; }
      //
      //! оптимизация последовательности треугольников покрытия судовой обшивки
      //                               должен присутствовать начальный интервал
      Ins( n+1 )=lf[0];             // опорные точки s# уже занесены в обшивку,
      Ins( n+1 )=rf[0];             // контрольные z# только ожидают обработки
      if( lf.length>1 && rf.length>1 )
      { if( norm( Rf[1]-Lf[0] )>norm( Rf[0]-Lf[1] ) ) // первый четырёхугольник
        { Ins( n+1 )=lf[1]; Ins( n+1 )=rf[1]; } else
        { Ins( n+1 )=rf[1]; Ins( n+1 )=lf[1]; } } else // веер из треугольников
      { for( i=1; i<rf.length; i++ )Ins( n+1 )=rf[i]; // дальше к другой шпации
        for( i=1; i<lf.length; i++ )Ins( n+1 )=lf[i]; continue;
      }
    NewFind:  //! перенастройка и продолжение оптимизации линий бортовых сломов
      //
      //  выбирается угол хорды относительно обоих локалей шпангоутных контуров
      //
     int l=-1,r=-1,al=-1,ar=-1;
      while( zl<lf.length || zr<rf.length )
      { /*
        if( Rf[sr].z<0 && Lf[sl].z<0 ) // подводные обводы никак не разбираются
        { for( r=rf.length-1; r>=0; r-- )if( Rf[r].z<0 )break;
          for( l=lf.length-1; l>=0; l-- )if( Lf[l].z<0 )break;
        } else */
        //       предварительный просмотр поверхности с поиском бортовых сломов
        //
        if( zl==lf.length )++zr; else                 // внутреннее ограничение
        if( zr==rf.length )++zl; else                // с единичным опережением
        if( (VR=norm( Rf[zr]-Lf[zl-1] ))>(VL=norm( Rf[zr-1]-Lf[zl] )) )
               { ++zl; if( norm( Rf[zr]-Lf[zl-1] )<=VL )++zr; }  // с подборкой
          else { ++zr; if( norm( Rf[zr-1]-Lf[zl] )<=VR )++zl; } // до параллели
        //
        //   проверяются впереди идущие совпадающие точки и сломы на шпангоутах
        //
      InnFind:
        if( r<0 )                     // градиентные ограничения точек\отрезков
        { if( zr==rf.length )r=zr; else
          if( !(RS=Rf[zr]-Rf[zr-1]) )r=zr; else if( zr>1 )  // верхняя точка
          { if( +(rs=Rf[zr-1]-Rf[zr-2]) )if( dir(RS)%dir(rs)<C24 )r=zr; }
        }
        if( l<0 )                     // пропуск если повтор и не было смещения
        { if( zl==lf.length )l=zl; else
          if( !(LS=Lf[zl]-Lf[zl-1]) )l=zl; else if( zl>1)
          { if( +(ls=Lf[zl-1]-Lf[zl-2]) )if( dir(LS)%dir(ls)<C24 )l=zl; }
        }
        // фрагменты с бортовыми сломами нуждаются в контроле углов расхождения
        //                                           повторение если меньше 15°
        if( l>=0 && r>=0 ){ zr=r; zl=l; break; } //! это единая удачная находка
                                                //       иначе
        if( l>=0 ){ zl=l; if( ar<0 )ar=zr;     // фиксируются две базовые точки
          if( +(RS=Rf[zr]-Rf[zr-1]) )if( dir( RS )%dir( Rf[zr]-Lf[l] )<.23 )
          { if( norm( Lf[l]-Rf[ar-1] )>=norm( Rf[ar]-Lf[l] ) )++ar;
            ++zr; goto InnFind;  // повтор поиска внутри треугольного фрагмента
          } zr=ar; break;
        }
        if( r>=0 ){ zr=r; if( al<0 )al=zl;    // правая отсечка без левой точки
          if( +(LS=Lf[zl]-Lf[zl-1]) )if( dir( LS )%dir( Lf[zl]-Rf[r] )<.23 )
          { if( norm( Rf[r]-Lf[al-1] )>=norm( Rf[r]-Lf[al] ) )++al;
            ++zl; goto InnFind;
          } zl=al; break;
      } }                               // завершение поиска оптимального слома
      if( zr<=sr && sr<rf.length )zr =sr+1;  // весьма странно, четырёхугольник
      if( zl<=sl && sl<lf.length )zl =sl+1;  // должен обрабатываться корректно

/* if( zl>lf.length || zr>rf.length )
   { cprint( 2,26,"~!!! n=%d, zl=%d(%d<%d), zr=%d(%d<%d)    ",
               n,sl,zl,lf.length,sr,zr,rf.length ); getch(); }
*/
      //!  с поиском новой и наиболее параллельной грани в текущих ограничениях
      //
      while( sl<zl || sr<zr )               // завершающий веер треугольников
      if( sl==zl )Ins( n+1 )=rf[sr++]; else // установка обоих точек для нового
      if( sr==zr )Ins( n+1 )=lf[sl++]; else // четырехугольника, по возможности
      { if( (VR=norm(Rf[sr]-Lf[sl-1]))>(VL=norm(Rf[sr-1]-Lf[sl])) )
        {                                Ins( n+1 )=lf[sl++];
          if( norm(Rf[sr]-Lf[sl-1])<=VL )Ins( n+1 )=rf[sr++];
        } else {                         Ins( n+1 )=rf[sr++];
          if( norm(Rf[sr-1]-Lf[sl])<=VR )Ins( n+1 )=lf[sl++];
      } }
/* if( sl>zl || sr>zr )
   { cprint( 2,26,"~$$$ n=%d, sl=%d(%d<%d), sr=%d(%d<%d)    ",
          n,sl,zl,lf.length,sr,zr,rf.length ); getch(); }
*/
      //   особая перепроверка (=пропуск) зараз всех смежных совпадающих точек
      //
      while( sl<lf.length )if( Lf[sl]==Lf[sl-1] )zl= ++sl; else break;
      while( sr<rf.length )if( Rf[sr]==Rf[sr-1] )zr= ++sr; else break;
//      if( sl>1 && sl<lf.length && Lf[sl-1]==Lf[sl] )zl= ++sl;
//      if( sr>1 && sr<rf.length && Rf[sr-1]==Rf[sr] )zr= ++sr;

      if( sl<lf.length || sr<rf.length )goto NewFind; // к возобновлению поиска
    }
#else
   unsigned ml=0,mr=0;         // индексы с маской для выборки точек шпангоутов
   bool Left,sl,Right,sr;   // точки на левом и правом шпангоуте/стороне шпации
   Real LSP=Length/Nframes;// условное усреднённое пространство (=длина) шпации
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
      //
      //  основной алгоритм выборки вершин и оптимизации бортовых треугольников
      //!  более-менее приемлемый и простой алгоритм функциональной оптимизации
      //
#define L_pass {L=Ll;do{sl=LNext(n,ml,Ll,St);Ll.x=0.0;}while(sl&& !(LS=Ll-L));}
#define R_pass {R=Rr;do{sr=RNext(n,mr,Rr,Sm);Rr.x=LSP;}while(sr&& !(RS=Rr-R));}
#define L_plus {Ins(n+1)=ml;L=Ll;sl=LNext(n,ml,Ll,St);Ll.x=0.0;if(!(LS=Ll-L))NY=NR;else NY=NL;}
#define R_plus {Ins(n+1)=mr;R=Rr;sr=RNext(n,mr,Rr,Sm);Rr.x=LSP;if(!(RS=Rr-R))NY=NL;else NY=NR;}
     int fb=0;
     Real BL,BR,UX;
     Vector L,Ll,LS=Zero,VL,NL,NR,RL,          // первая нормаль указывает вниз
            R,Rr,RS=Zero,VR,NY={0,0,-1}; NL=NR=NY;   // Shell номера n+1
      sl=LNext( n,ml,Ll,St ); Ll.x=0.0; L=Ll;        // предустановка новой
      sr=RNext( n,mr,Rr,Sm ); Rr.x=LSP; R=Rr;        // вершины треугольника
      if( sl ){ Ins( n+1 )=ml; L_pass } // первые две точки и сразу проверяется
      if( sr ){ Ins( n+1 )=mr; R_pass }// наличие второй сопряженной пары точек
//    while( sl && L.y>Ll.y )L_pass    // вблизи килевых точек может возникать
//    while( sr && R.y>Rr.y )R_pass    // путаница с разрывами аппликат
      //
      //! рабочий цикл внутри шпаций корпуса
      //
      for( fb=0; fb<=1; fb++ )      // FreeBoard, но сначала под\надводный борт
      for( int np=0;; np++ )           // поиск адекватной вершины треугольника
      {  Left=sl && ( fb || L.z<0.0 ); // учёт предустанавливаемого ограничения
        Right=sr && ( fb || R.z<0.0 ); // по осадке конструктивной ватерлинии
//
  if( !fb && np<=3  )
  { if( Ll.y<L.y ){ Ll.y=L.y; LS=Ll-L; }  //! есть проблема сложения штевней со
    if( Rr.y<R.y ){ Rr.y=R.y; RS=Rr-R; }  //  шпангоутами => временная заплатка
  } //
        if( Left && Right )               //!  подход к оптимизационному поиску
        {        // диагонали, основание трапеции и сброс ординатных градиентов
         const Real C12=0.9781476007;
          RL=R-L,VL=R-Ll,VR=Rr-L; if( fb )VL.y/=3,VR.y/=3,RL.y/=3; /// ENV
          VL=dir( VL );
          VR=dir( VR );
          RL=dir( RL );
          BL=dir( LS )%VL;
          BR=dir( RS )%VR; UX=dir( Rr-Ll ).x;
          //
          //  первыми обрабатываются предварительно установленные
          //          пары совпадающих точек на сломах шпангоутных контуров
         bool AL=!LS,
              AR=!RS;
          if( AL && AR ){ L_pass R_pass continue; }      // пропуск пары сломов
          else if( AL ){ if( BR<0.2 )R_plus else L_pass continue; }
          else if( AR ){ if( BL>-.2 )L_plus else R_pass continue; }

          if( np && fb )
          { NL=dir( RL*LS ); AL=NY%NL<C12 || fabs( BL )>=0.22;  // 0.9739 =
            NR=dir( RL*RS ); AR=NY%NR<C12 || fabs( BR )>=0.22;  //  = 13°7'9"08
            if( AL && !AR ){ R_plus continue; }                 // 0.9890158634
            if( AR && !AL ){ L_plus continue; }                 //  = 8°30'
          }
          if( UX>=VL.x && UX>=VR.x )                            ///   NEW
          { if( VL.x>VR.x ){ L_plus R_plus } else { R_plus L_plus } } else
          { if( VL.x>VR.x ){ L_plus if( BR<-.2 )R_plus }
                      else { R_plus if( BL>0.2 )L_plus } }
        }
        else if( Left  )L_plus
        else if( Right )R_plus else break;
    } }
#endif
    free( Sm ),free( St );                   // освобождение временных ресурсов
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
// C06=0.9945218954,C12=0.9781476007,C24=0.9135454576,C30=0.8660254038,C45=0.70710067812,
// C48=0.6691306064,C75=0.2588190451,C78=0.2079116908,C82=0.139173101,C86=0.06975647374;
//
