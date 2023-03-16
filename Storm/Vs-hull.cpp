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
  if( s!=sp ){ s=sp; k=0; }               // выбор нового левого шпангоута sp
  if( !s )                                // левее других шпангоутов больше нет
  { if( k<Frame[0].length )               // на последовательное перечисление
      { R=Frame[0][m=k++]; return true; } // пробная точка правее ахтерштевня
  } else
  if( s<Nframes )                         // иначе выбор вправо внутри шпации
  { _Real Fx=Keel[s-1]; Flex &G=Frame[s];
    for( int j=-1,i=0; i<Stem.length; i++ )
    if( !Sm[i] )                          // выбор точек форштевня
    { _Vector S=Stem[i];
      if( mzR>0.0 && S.z>=mzR )break;// правый шпангоут загибается за фальшборт
      if( S==G[k] ){ k++; j=i; } else
      if( Fx<=S.x && S.x<=Keel[s] )  //!<= внутри шпации с накрытием шпангоутов
      { if( !k ){ if( S.z<=G[0].z )j=i; } else // точки киля под днищем корпуса
        if( k>=G.length ){ if( S.z>G[k-1].z )j=i; } else // над верхней палубой
        { if( S.z>G[k-1].z && S.z<=G[k].z )j=i; }        // внутри и слева
      }
      if( j>=0 ){R=S; m=StemPost|j; return Sm[j]=S.x<Keel[s]?1:2;}// форштевень
    } if( k<G.length ){ R=G[k]; m=k++; return true; } // номер точки шпангоута
  } else                                             // =Nframes крайние справа
  { Flex &G=Frame[s-1];
    for( int i=0; i<Stem.length; i++ )if( !Sm[i] )   // точки впереди форштевня
    { _Vector S=Stem[i];                        // формируют независимую шпацию
      if( Keel[s-1]<=S.x ){ R=S; m=StemPost|i; return Sm[i]=Keel[s-1]<=S.x?1:2; }
    }                                             // из свободных треугольников
    if( !Stem.length && k<G.length )                       // один раз делается
    { R=G[k=G.length-1]; m=LeftFrame|k++; return true; }   // транцевая затычка
  } return false;             // сваливается в одну путаную бесконтрольную кучу
}
bool Hull::LNext( int sp, unsigned &m,Vector &L,byte *St ) // s!=sp[0..N] от кормы в нос
{ static int s=-1,k=0;         // одноразовый подход целиком ко всему шпангоуту
  if( s!=sp ){ s=sp; k=0; }    // фиксируется правый шпангоут, левый - в поиске
  if( s==Nframes )             // правее шпангоутов нет, есть только форштевень
  { if( k<Frame[s-1].length )  // обычная поисковая фиксация ++ по перечислению
      { L=Frame[s-1][k]; m=LeftFrame|k++; return true; }
  } else
  if( s>0 )                          // вариант со средними шпациями-трапециями
  { _Real Fx=Keel[s]; Flex &G=Frame[s-1];
    for( int j=-1,i=0; i<Stern.length; i++ )if( !St[i] )
    { _Vector S=Stern[i];
      if( mzL>0.0 && S.z>=mzL )break; // левый шпангоут загибается за фальшборт
      if( S==G[k] ){ k++; j=i; } else
      if( Keel[s-1]<=S.x && S.x<=Fx ) //!<= правому шпангоуту временная отметка
      { if( !k ){ if( S.z<=G[0].z )j=i; } else
        if( k>=G.length )if( S.z>=G[k].z )j=i; else
        { if( S.z>=G[k-1].z && S.z<=G[k].z )j=i; }
      }
      if( j>=0 ){ L=S; m=SternPost|j; return St[j]=S.x<Fx?1:2; }
    } if( k<G.length ){ m=LeftFrame|k; L=G[k++]; return true; }
  } else      // точки за ахтерштевнем формируют шпацию свободных треугольников
  { Flex &G=Frame[0];
    for( int i=0; i<Stern.length; i++ )if( !St[i] )// перебор прикаянных вершин
    { _Vector S=Stern[i];
      if( S.x<=Keel[0] ){ L=S; m=SternPost|i; return St[i]=S.x<=Keel[0]?1:2; }
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
  Lint( int L ):len( L ){ _Y=( _X=(Real*)malloc( L*sizeof(Real)*2 ) )+L; }
 ~Lint(){ free( _X ); }
  Real& operator[]( int k ){ return _Y[minmax( 0,k,len-1 )]; }   // заграничные
  Real& operator()( int k ){ return _X[minmax( 0,k,len-1 )]; }   //   равенства
  Real operator()( _Real A );
};
Real Lint::operator()( _Real A )               // простая линейная интерполяция
{ if( len<2 )return len?_Y[0]:0.0; int k; Real *x=_X;
  for( k=0; k<len-1 && A>x[1]; k++,x++ ); Real *y=_Y+k,a,h;
  if( !(h=x[1]-x[0]) )return (y[0]+y[1])/2;
  if( !(a=A-x[0]) )return y[0];
  return y[0]+a*(y[1]-y[0])/h;
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
  { Real x,y,z;                      // здесь продолжается серия проверок
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
                                       // конструктивной осадки
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
          if( fs && y>0.0   )Stern+=(Vector){ x,0,z };       // нижняя точка
              fs=false;      Stern+=e5( (Vector){ x,y,z } ); // зануляется
        } if(!fs && y>0.0 && Stern.length>0 )Stern+=(Vector){ x,0,z };
    } }
    //    Собственно корпус ( - в том же последовательном потоке )
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
                  else    z=B(j),y=B[j],x=A(z),j++;
          if( fs && y>0.0 )Stem+=(Vector){ x,0,z };     // только нижняя точка
              fs=false;  Stem+=e5( (Vector){ x,y,z } ); // зануляется автоматом
        } if(!fs && y>0.0 && Stem.length>0 )Stem+=(Vector){ x,0,z }; // верхняя
    } }
    fclose( Fh );         // работа с файлом данных завершена
    //
    //    Чтение последовательного потока завершено, но если смещена основная
    //    плоскость, то необходим проход по всем точкам плазовых аппликат
    //
    Ofs.x=Nframes>2?minmax( Keel[1],Keel[Mid],Keel[Nframes-2] )
                         :( Keel[0]+Keel[Nframes-1] )/2;     // крайние пополам
    Ofs.z+=Draught; // все отсчёты аппликат приводятся к действующей ватерлинии
    WaterLine.length=0;  Control( (Stern),WaterLine,Ofs,true ); // контроль
    for( n=0; n<Nframes; n++ )                                  // со сдвигом
       { Keel[n]-=Ofs.x; Control( (Frame[n]),WaterLine,Ofs ); } // пересечения
                         Control( (Stem),WaterLine,Ofs,true );  // ватерлинии
    //
    //   == ещё и ещё раз новая попытка снять неоднозначности штевней ==
    //
/*  for( k=Stern.length-1; k<=0; k-- )
    for( n=0; n<Nframes; n++ ){ if( Stern[k] }
*/
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
   unsigned ml=0,mr=0;         // индексы с маской для выборки точек шпангоутов
   byte *St=(byte*)calloc( sizeof( byte ),Stern.length ),   // признаки учёта
        *Sm=(byte*)calloc( sizeof( byte ),Stem.length );    // точек на штевнях
#if !__x86_64__              // __i386__
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
#define OPT ( VL.x>VR.x )
#define L_pass {ls=LS;L=Ll;do{sl=LNext(n,ml,Ll,St);Ll.x=0.0;}while(sl&&!(LS=Ll-L));}
#define R_pass {rs=RS;R=Rr;do{sr=RNext(n,mr,Rr,Sm);Rr.x=LSP;}while(sr&&!(RS=Rr-R));}
#define L_plus {Ins(n+1)=ml;ls=LS,L=Ll;sl=LNext(n,ml,Ll,St);Ll.x=0.0;if(!(LS=Ll-L))NY=NR;else NY=NL;}
#define R_plus {Ins(n+1)=mr;rs=RS,R=Rr;sr=RNext(n,mr,Rr,Sm);Rr.x=LSP;if(!(RS=Rr-R))NY=NL;else NY=NR;}
#define NEW { if( UP ){ if( OPT ){ L_plus R_plus } else { R_plus L_plus } }  \
                 else { if( OPT ){ L_plus if( BR<-.2 )R_plus }               \
                            else { R_plus if( BL>0.2 )L_plus } } continue; }
#define ENV { VL=R-Ll; VR=Rr-L; RL=R-L; if( fb ){ VL.y/=3,VR.y/=3,RL.y/=3; } \
           VL=dir(VL); VR=dir(VR); RL=dir(RL); BL=dir(LS)%VL; BR=dir(RS)%VR; }

     int fb=0;
     Real BL,BR;
     Vector L,Ll,LS=Zero,ls=Zero,VL,NL,NR,RL,  // первая нормаль указывает вниз
            R,Rr,RS=Zero,rs=Zero,VR,NY={0,0,-1}; NL=NR=NY; // Shell номера n+1
      sl=LNext( n,ml,Ll,St ); Ll.x=0.0; L=Ll;            // предустановка новой
      sr=RNext( n,mr,Rr,Sm ); Rr.x=LSP; R=Rr;           // вершины треугольника
      if( sl ){ Ins( n+1 )=ml; L_pass } // первые две точки и сразу проверяется
      if( sr ){ Ins( n+1 )=mr; R_pass }// наличие второй сопряженной пары точек

      for( fb=0; fb<=1; fb++ )  // FreeBoard: но сначала под\надводный борт
      for( int np=0;; np++ )   // поиск будущей адекватной вершины треугольника
      {  Left=sl && ( fb || L.z<0.0 ); // учёт предустанавливаемого ограничения
        Right=sr && ( fb || R.z<0.0 ); // по осадке конструктивной ватерлинии

if( !fb && np<=3  )
{ if( Ll.y<L.y ){ Ll.y=L.y; LS=Ll-L; }    //! есть проблема сложения штевней со
  if( Rr.y<R.y ){ Rr.y=R.y; RS=Rr-R; }    //  шпангоутами => временная заплатка
}
        if( Left && Right )               //!  подход к оптимизационному поиску
        { ENV    // диагонали, основание трапеции и сброс ординатных градиентов
         const Real C12=0.9781476007; //C03=0.9986295348,C06=0.9945218954,
                 // C24=0.9135454576,C30=0.8660254038,C45=0.7071067812;
         Real UX = dir( Rr-Ll ).x;
         bool UP = UX>=VL.x && UX>=VR.x;

          if( !ls && !rs )NEW   // ++ пара точек без дополнительных ограничений
          //
          //  1) первыми обрабатываются  предварительно установленные
          //              пары совпадающих точек на сломах шпангоутных контуров
         bool AL=!LS,AR=!RS;
          if( AL && AR ){ L_pass R_pass continue; }      // пропуск пары сломов
          else if( AL ){ if( BR<0.2 )R_plus else L_pass continue; }
          else if( AR ){ if( BL>-.2 )L_plus else R_pass continue; }

          if( np && fb )
          {
            //
            //  2) затем правка для диагоналей круче 30° относительно шпангоута
            //
            AL=fabs( BL )>=0.22;    // 0,34=70°7'23"25         cos(60°)=0,5 или
            AR=fabs( BR )>=0.22;   // 0,475=61°38'26"34         30° горизонтали
            if( AL && AR )NEW                                  // в перекрестье
            else if( AL ){ if( VR.x>=VL.x ){ R_plus continue; } }
            else if( AR ){ if( VL.x>=VR.x ){ L_plus continue; } }
            //
            //  3) обнаружение и постобработка искусственных и естественных
            //                                       сломов контуров шпангоутов
//          AL = !(!ls) && dir( ls )%dir( LS )<C12;   // при угле слома контура
//          AR = !(!rs) && dir( rs )%dir( RS )<C12;   // шпангоута больше 8°30'
//          if( AL && AR )NEW                                         // 0.9739
//          else if( AL ){ if( BR<0.01 ){ R_plus continue; } }        // <0.01
//          else if( AR ){ if( BL>-.01 ){ L_plus continue; } }        // >-0.01
            //
            // 4) нормали отличаются 30° к последней предустановке треугольника
            //
            NL=dir( RL*LS ); AL=NY%NL<C12;   // 0.9739;     // ? 24
            NR=dir( RL*RS ); AR=NY%NR<C12;   // 0.9739;     // 0.9739=13°7'9"08
            if( AL && AR )NEW
            else if( AL ){ R_plus continue; }
            else if( AR ){ L_plus continue; } //=8°30'
//          else if( AL ){ if( NY%NR>C03 ){ R_plus continue; } }
//          else if( AR ){ if( NY%NL>C03 ){ L_plus continue; } } //=8°30'
          }
#if 1
          NEW
//        if( OPT )L_plus else R_plus
#else
          VL=R-Ll,VR=Rr-L,RL=R-L;         // две диагонали и основание трапеции
          if( !fb )VL.x=VR.x=1.0; else               // - абсцисса исключается
              VL.x=3.2-fabs( (NL=dir(VL*RL))%NY ),   // - ордината ослабляется
              VR.x=3.2-fabs( (NR=dir(RL*VR))%NY );   // - аппликата усиливается
          if( hypot(Ll.z/Draught+2,Ll.y/Breadth)*hypot(VL.y,VL.z*1.6)*VL.x
            < hypot(Rr.z/Draught+2,Rr.y/Breadth)*hypot(VR.y,VR.z*1.6)*VR.x )
          L_plus else R_plus
#endif
        }
        else if( Left  )L_plus
        else if( Right )R_plus else break;
    } }
#else
   Vector L,R,   //WS,         // базовые точки, верхняя грань четырехугольника
          Ll,Rr; //,VL,VR;     // вершины и диагонали оптимизации треугольников
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
