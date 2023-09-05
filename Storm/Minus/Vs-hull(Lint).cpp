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
{ if(             m&LeftFrame )return Frame[n-1][~FramePost&m]; // левый отсчет
  if( !n ||       m&SternPost )return Stern[~FramePost&m];      // ахтерштевень
  if( n>Nframes+1 || m&StemPost )return Stem[~FramePost&m];       // форштевень
                               return Frame[n][m];              // сам шпангоут
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
    if( m&LeftFrame )P=Frame[n-1][k=~FramePost&m],n--; else     // левый отсчет
    if( !n || m&SternPost )P=Stern[k=~FramePost&m],n=0; else    // ахтерштевень
    if( n>Nframes+1 || m&StemPost )P=Stem[k=~FramePost&m],n=Nframes+1; // крайний
                          else   P=Frame[n][k=m];               // сам шпангоут
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
//static Real mzL=-1.0,mzR=-1.0; // нижний уровень точек шпангоута за фальшбортом
/*
bool Hull::RNext( int sp,unsigned &m,Vector &R,byte *Sm ) // s!=sp[0..N]->в нос
{ static int s=-1,k=0;                   // не предполагается повторного чтения
  if( s!=sp ){ s=sp; k=0; }              // выбор нового левого шпангоута sp
//  if( !s )                               // левее других шпангоутов больше нет
//  { if( k<Frame[0].len )                 // на последовательное перечисление
//    { R=Frame[0][m=k++]; return true; }  // пробная точка правее ахтерштевня
//  } else
//  if( s<Nframes )                           // иначе выбор вправо внутри шпации
  { Flex &G=Frame[s];
//if( 0 )
//    for( int j=-1,i=0; i<Stem.len; i++ )if( !Sm[i] ) // выбор точек на форштевне
//    { _Vector S=Stem[i];
////    if( mzR>0.0&&S.z>=mzR )break;  // правый шпангоут загибается за фальшборт
//      if( S==G[k] ){ k++; j=i; } else        // в приоритете точка на форштевне
////    if( S==G[k] ){ Sm[i]=2; break; }       // в приоритете точка на шпангоуте
//      if( Keel[s-1]<=S.x && S.x<=Keel[s] )   //!= шпации с накрытием шпангоутов
//      { if( !k ){ if( S.z<=G[0].z )j=i; } else    // точки под форштевнем и над
//        if( k>=G.len ){ if( S.z>=G[k-1].z )j=i; } else       // верхней палубой
//        { if( S.z>G[k-1].z && S.z<=G[k].z )j=i; }        // внутри и слева
////      if( intoi( G[k-1].z,S.z,G[k].z ) && !G[k-1].y && !G[k].y )j=i;// внутри
//      }
//      if( j>=0 ){R=S; m=StemPost|j; return Sm[j]=S.x<Keel[s]?1:2; } //= штевень
//    }
    if( k<G.len ){ R=G[k]; m=k++; return true; }    // номер точки шпангоута
//  }
//  else                                             // =Nframes крайние справа
//  { Flex &G=Frame[s-1];
//    for( i=0; i<Stem.len; i++ )if( !Sm[i] )     // точки впереди форштевня
//    { _Vector S=Stem[i];                        // формируют независимую шпацию
//      if( Keel[s-1]<=S.x ){ R=S; m=StemPost|i; return Sm[i]=Keel[s-1]<=S.x?1:2; }
//    }                                             // из свободных треугольников
//    if( !Stem.len && k<G.len )                          // один раз делается
//    { R=G[k=G.len-1]; m=LeftFrame|k++; return true; }   // транцевая затычка
  } return false;             // сваливается в одну путаную бесконтрольную кучу
}                                                             // от кормы в нос
bool Hull::LNext( int sp,unsigned &m,Vector &L,byte *St )     //    s!=sp[0..N]
{ static int s=-1,k=0;         // одноразовый подход целиком ко всему шпангоуту
  if( s!=sp ){ s=sp; k=0; }    // фиксируется правый шпангоут, левый - в поиске
//  if( s==Nframes )             // правее шпангоутов нет, есть только форштевень
//  { if( k<Frame[s-1].len )  // обычная поисковая фиксация ++ по перечислению
//      { L=Frame[s-1][k]; m=LeftFrame|k++; return true; }
//  } else
//  if( s>0 )                          // вариант со средними шпациями-трапециями
  { Flex &G=Frame[s-1];
//    for( int j=-1,i=0; i<Stern.len; i++ )if( !St[i] )
//    { _Vector S=Stern[i];
////    if( mzL>0.0 && S.z>=mzL )break; // левый шпангоут загибается за фальшборт
//      if( S==G[k] ){ k++; j=i; } else      // в приоритете точка на форштевне
////    if( S==G[k] ){ St[i]=1; break; }     // в приоритете точка на шпангоуте
//      if( Keel[s-1]<=S.x && S.x<=Keel[s] ) // правому шпангоуту временная отметка
//      { if( !k ){ if( S.z<=G[0].z )j=i; } else
//        if( k>=G.len ){ if( S.z>=G[k-1].z )j=i; } else
//        { if( S.z>=G[k-1].z && S.z<=G[k].z )j=i; }
////      if( intoi( G[k-1].z,S.z,G[k].z ) && !G[k-1].y && !G[k].y )j=i;
//      }
//      if( j>=0 ){ L=S; m=SternPost|j; return St[j]=S.x<Keel[s]?1:2; }
//    }
    if( k<G.len ){ m=LeftFrame|k; L=G[k++]; return true; }
//  } else      // точки за ахтерштевнем формируют шпацию свободных треугольников
//  { Flex &G=Frame[0];
//    for( int i=0; i<Stern.len; i++ )if( !St[i] )// перебор прикаянных вершин
//    { _Vector S=Stern[i];
//      if( S.x<=Keel[0] ){ L=S; m=SternPost|i; return St[i]=S.x<Keel[0]?1:2; }
//    }
//    if( !Stern.len && k<G.len ){ m=k=G.len-1; L=G[k++]; return true; }
  } return false;
}
*/
static Vector e5( _Vector W ) // округление записи для точных сравнений 0.01 мм
{ return (Vector){ round( W.x*1e5 )/1e5,
      W.y<=eps?0.0:round( W.y*1e5 )/1e5,
                   round( W.z*1e5 )/1e5 };
}
//    Интерполяция с разрезанием шпангоутов для фиксации точек на ватерлинии
//     ... отчасти исключаются из рассмотрения вершины пустых треугольников
//
static void Control( Flex &F, _Vector Ofs, bool Stm=false )  // def- шпангоутам
{ Stm=false;
  for( int k=0; k<F.len; k++ ) // if( k>=F.len )break; else // предосторожность
  { Vector &A=F[k]; A-=Ofs; if( A.z&&k>0 ){ _Vector B=F[k-1];// В - уже изменен
      if( A.z*B.z<0 ){ if( fabs( A.z )<eps )A.z=0.0; else   // к нулю аппликаты
        if( Stm || A.y || B.y )F.Insert( k++ )=e5( B-(A-B)*(B.z/(A.z-B.z)) );
  } } }
}
static Real
Inter( _Real x, _Real x0,_Real x1, _Real y0,_Real y1 )  // ± любое направление
{ if( x==x0 )return y0;                                 // точное совпадение по
  if( x==x1 || x0==x1 )return y1;                       // крайним аргументам и
//if( x0==x1 )return (y0+y1)/2;                         // среднеарифметическое
              return y0+(x-x0)*(y1-y0)/(x1-x0);         //?? и беда при малом h
}
/*static Vector
Inter( _Real z,_Real z0,_Real z1, _Vector v0,_Vector v1 )// ± любое направление
{ if( z==z0 )return v0;                                 // точное совпадение по
  if( z==z1 )return v1;                                 // крайним аргументам и
  if( z0==z1 )return (v0+v1)/2;                         // среднеарифметическое
              return v0+(z-z0)*(v1-v0)/(z1-z0);         //?? и беда при малом h
}*/
//     Слияние точек штевней с переработкой катамаранных "замкнутых" контуров
//     -- первым ходом абсциссам последовательно даются поднимающиеся ординаты

static void Alliance( Flex &S, Flex &Y ) // в приоритете абсциссы точек штевней
{ int i,k; //Vector V; // S /= S[-1];      // точка последняя -> начальная кольца
  //
  //  первым проходом все ординаты дополняются сопутствующими абсциссами
  //  здесь абсцисса штевня представляется однозначной функцией от аппликаты
  //
  if( !Y.len )return;                         // бестранцевые штевни - как есть
  if( S.len>1 )                               // для интерполяции надо 2 точки
  for( k=0; k<Y.len; k++ )                    // последовательный поиск ординат
  { if( Y[k].z>S[i=0].z )                   //~(не)исключаются подкильные точки
    while( !intoi( S[i+1].z,Y[k].z,S[i].z ) )if( i>=S.len-2 )break; else ++i;
    Y[k].x=Inter( Y[k].z,S[i].z,S[i+1].z,S[i].x,S[i+1].x );
  }
  // вторым интерполируются ординаты транцев на штевнях с добавлением оконцовок
  //
  for( k=0,i=S.len-1; i>=0; i-- )
   if( S[i].z<=Y[0].z ){ Y/=S[i]; if( !k )k=i; } // ниже указанных точек транца

//  for( i=k; i<S.len; i++ )
//  { for( k=0; k<Y.len-1; k++ )if( Y[k]!=S[i] && Y[k+1]!=S[i] )
//    if( intox( Y[k+1].z,S[i].z,Y[k].z ) )       // или неравенство само из себя
//    { V=S[i]; V.y=Inter( V.z,Y[k].z,Y[k+1].z,Y[k].y,Y[k+1].y );
//      Y.Insert( ++k )=V;
//  } }
                                k=Y.len-1;              // и на текущий момент
  for( i=0; i<S.len; i++ )if( Y[k].z<=S[i].z )Y+=S[i];  // в верхнее дополнение
  //
  //  может не очень эффективно, но всё ж повторенья все должны исключаться
  //
  for( S.len=k=0; k<Y.len; k++ ){ if( k )if( Y[k-1]==Y[k] )continue; S+=Y[k]; }
}
/*
//      if( !j ){ if( !Y.len || Y[k].z<=S[i].z )Y+=S[i];
//                     else if( Y[k].z>S[i].z )Y/=S[i]; }
  for( i=0; i<S.len; i++ )S[i].y=-1.0; // отметки для исключения повторений
  //
  //  теперь все эти точки переносятся на штевни с множественной интерполяцией
  //
  for( k=0; k<Y.len; k++ )
  { if( Y[k].z<S[0].z )S/=Y[k]; else    // как бы получается полный охват
    if( Y[k].z>S[-1].z )S+=Y[k]; else   // по аппликатам штевней
    for( i=0; i<S.len-1; i++ )          // с повторами на совпадающих точках
     if( intoi( S[i+1].z,Y[k].z,S[i].z ) )S.Insert( ++i )=Y[k];
  }
  //  свободные точки на штевнях расталкиваются по контурам транцев
  //
  for( i=0; i<S.len; i++ )if( S[i].y<0.0 )
  for( k=0; k<S.len; k++ )if( i!=k )
  { if( S[i].z==S[k].z )S[i].y=S[k].y; else
    if( k<S.len-1 )
    if( intox( S[k].z,S[i].z,S[k+1].z ) )
    { S.Insert( k+1 )=Inter( S[i].z,S[k].z,S[k+1].z,S[k],S[k+1] ); k++;
    } else S[i].y=0;
  }


/=*=/
    if( intox( Y[k].z,S[i].z,Y[k+1].z )
    {

      S.Insert[i+1]=Inter( S[i].z,S[i].z,S[i+1].z,S[i].x,S[i+1].x ); i++;
//    S.Insert[i+1]=Inter( S[i].z,Y[k].z,Y[k+1].z,Y[k],Y[k+1] ); i++;

    } else


#else
  //
  //  пробег до минимальной аппликаты k для последующего двухстороннего подъёма
  //
//for( k=1; k<Y.len && Y[k-1].z>=Y[k].z; k++ );    // пока без внутрикатамарана
  //
  // исходные точки штевней дополняются ординатами с загибом внутрь фальшбортов
  //
  for( i=0,k=1; i<S.len && k<Y.len-1; i++ )  // последовательный поиск ординат по
  { if( S[i].z>Y[k].z )                  // возрастающему сегменту на форштевне
    while( k<Y.len-1 && !intoi( Y[k-1].z,S[i].z,Y[k].z ) )++k;
    S[i].y=Inter( S[i].z, Y[k-1].z,Y[k].z,Y[k-1].y,Y[k].y );  // = интерполяция
  }
  // вторым шагом транцевым контурам с ординатами придаются абсциссы от штевней
  //
  if( S.len>1 )
  for( k=0; k<Y.len; k++  )          // последовательный поиск ординат
  { if( Y[k].z>S[i=0].z )            // исключаются подкильные точки
    while( !intoi( S[i].z,Y[k].z,S[i+1].z ) )if( i>=S.len-2 )break; else ++i;
    if( Y[k].z==S[i].z )Y[k].x=S[i].x; else
        Y[k].x=Inter( Y[k].z,S[i].z,S[i+1].z,S[i].x,S[i+1].x );
  }
  //    теперь все ординатные отсчеты совмещаются с заданными контурами штевней
  k=0;
  if( Y.len && S.len )if( Y[0].z<S[0].z ){ S/=Y[0]; k++; }  // концевые точки
                  //  if( Y[-1].z>S[-1].z )S+=Y[-1]; }      // на штевнях
  for( ; k<Y.len; k++ )               // теперь все ординаты просто совмещаются
  for( i=0; i<S.len-1; i++ )          // со всеми  исходными абсциссами штевней
   if( S[i]!=Y[k] && S[i+1]!=Y[k] )
   if( intox( S[i+1].z,Y[k].z,S[i].z ) )
   if( intox( S[i+1].y,Y[k].y,S[i].y ) )
     S.Insert( ++i )=Y[k];
#endif //
///===
#if 0
  //
  // всегда будем полагать наличие нижней контрольной точки штевней, где она?
  //
 int l=0,r=0;
  for( i=1; i<S.len; i++ )if( S[i].z<S[l].z )l=r=i;   // правые точки исключены
  //
  // теперь пробуем строго симметризовать тупизну штевней относительно среднего
  //
  for( i=l+1; i<S.len; i++ )if( S[i].x==S[r].x && S[i].z==S[r].z )++r;
//if( r<S.len )
  while( r-l>1 ){ S.Delete( l+1 ); --r; }   // вычистка нижних траверзных точек
  //
  // перебор и расстановка симметричных точек по возрастанию аппликаты
  //
#define ir(r) (r%(S.len+1))
#define il(l) ((l+S.len)%(S.len+1))
  do                                  // полный оборот кольца как бы невозможен
  { Vector V={0.0};
    if( S[l].z>S[r].z ){ V=Inter( S[l].z,S[r].z,S[ir(r+1)].z,S[r],S[ir(r+1)] ); S.Insert( r=ir(r+1))=V; } else
    if( S[l].z<S[r].z ){ V=Inter( S[r].z,S[l].z,S[il(l-1)].z,S[l],S[il(l-1)] ); S.Insert( l )=V; l=il(l-1); }
                            else if( l==r )break;
                              else r=ir(r+1),l=il(l-1);
  } while( S[l].y<=S[r].y );

#endif
  //
  //  временный контроль концевых треугольников по старому образцу
  //
//  if( S.len ){ V=S[0];  if( V.y ){ V.y=0.0; S/=V; }
//               V=S[-1]; if( V.y ){ V.y=0.0; S+=V; } }
} */
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
static Real norm( _Vector A,_Vector B ){ Vector D=A-B; /*D.x=1;*/ return norm(D); }

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
   Real x,y,z; int i,j,k,n;            // ... точки по циклам двойной вложенности
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
#if 0
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
        } if( !fs && y>0.0 && Stern.len>0 )Stern+=(Vector){ V.x,0,V.z };
    } }
#else
    Stern.len=L.len=0; n=strtol( stringData( Fh ),&s,0 );
    for( V.y=0,i=0; i<n; i++ )V.z=strtod( s,&s ),V.x=strtod( s,&s ),Stern+=V;
                      k=strtol( stringData( Fh ),&s,0 );
    for( V.x=0,i=0; i<k; i++ )V.z=strtod( s,&s ),V.y=strtod( s,&s ),L+=V;
    Alliance( Stern,L );
#endif
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
#if 0
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
        } if( !fs && y>0.0 && Stem.len>0 )Stem+=(Vector){ V.x,0,V.z };
      }
    }
#else
    Stem.len=R.len=0; k=strtol( stringData( Fh ),&s,0 );
    for( V.x=0,i=0; i<k; i++ )V.z=strtod( s,&s ),V.y=strtod( s,&s ),R+=V;
                      n=strtol( stringData( Fh ),&s,0 );
    for( V.y=0,i=0; i<n; i++ )V.z=strtod( s,&s ),V.x=strtod( s,&s ),Stem+=V;
    Alliance( Stem,R );
#endif
    fclose( Fh );                         // работа с файлом данных завершена
    //
    //    Экстремумы для килевой линии и новые крайние шпангоуты
    //      ~~ крайние точки на килевой или палубной аппликате
    //      == крайние на шпангоутах
    //
    Keel[0]=Keel[1];
    for( i=0; i<Stern.len; i++ ){ Vector &S=Stern[i];
      if( S.x<Keel[0] )Keel[0]=S.x; // нижний экстремум абсциссы по ахтерштевню
      if( S.x<Keel[1] )Frame[0]+=S; // кривой шпангоут в кормовой оконечности
    }
    Keel[Nframes+1]=Keel[Nframes];
    for( i=0; i<Stem.len; i++ ){ Vector &S=Stem[i];
      if( S.x>Keel[Nframes+1] )Keel[Nframes+1]=S.x;   // экстремум на форштевне
      if( S.x>Keel[Nframes] )Frame[Nframes+1]+=S;     //  образование шпангоута
    }
    //
    //  попробуем зараз расставить все точки вблизи диаметральной плоскости
    //
   for( n=1; n<Nframes; n++ )                         // (מ)-левый (n+1)-правый
   { for( i=0; i<Stem.len; i++)                       // все точки на форштевне
     if( Keel[n]<Stem[i].x && Stem[i].x<=Keel[n+1] )  // и без правых наложений
     { Vector &V=Stem[i]; Flex &F=Frame[n+1];         // точка внутри шпации
       if( V.z<F[0].z )F/=V; else                     // и её правый шпангоут
       if( V.z>=F[-1].z )F+=V; else for( k=1; k<F.len; k++ )
       if( intor( F[k].z,V.z,F[k-1].z ) ){ F.Insert( k )=V; break; }
     }
     for( i=0; i<Stern.len; i++)                      // все точки на форштевне
     if( Keel[n-1]<=Stern[i].x && Stern[i].x<Keel[n] )// и без правых наложений
     { Vector &V=Stern[i];                            // точка внутри шпации
       Flex &F=Frame[n-1];                            // и её правый шпангоут
       if( V.z<F[0].z )F/=V; else
       if( V.z>=F[-1].z )F+=V; else for( k=1; k<F.len; k++ )
       if( intor( F[k].z,V.z,F[k-1].z ) ){ F.Insert( k )=V; break; }
     }
   }
    //    Чтение последовательного потока завершено, но если смещена основная
    //    плоскость, то необходим проход по всем точкам плазовых аппликат
    //
    Ofs.z+=Draught; // все отсчёты аппликат приводятся к действующей ватерлинии
    Ofs.x=Nframes>2?minmax( Keel[1],Keel[Mid],Keel[Nframes] )
                         :( Keel[1]+Keel[Nframes] )/2;       // крайние пополам
                         Control( (Stern),Ofs,true );            // контроль
    for( n=0; n<=Nframes+1; n++ )                                // со сдвигом
       { Keel[n]-=Ofs.x; Control( (Frame[n]),Ofs ); }            // пересечения
                         Control( (Stem),Ofs,true );             // ватерлинии
    //
    //   предустановка точки на ватерлинии в случае отсутствия контуров штевней
    //
    if( !Stern.len ){ V=Frame[0][k=Frame[0].len-2];
      if( k>2 )if( V.z>0 && Frame[0][1].z<0 )Frame[0]+=(Vector){ V.x,0,0 }; }

    if( !Stem.len ){ n=Nframes+1; V=Frame[n][k=Frame[n].len-2];
      if( k>2 )if( V.z>0 && Frame[n][1].z<0 )Frame[n]+=(Vector){ V.x,0,0 }; }
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
#if 1
    for( n=0; n<=Nframes+2; n+=Nframes+2 )         // асимметричная расстановка
    { Flex &S = !n ? Stern:Stem;                   // транцевых треугольников
      for( i=0; i<S.len; i++ )Ins( n )=i;
    }
#else
    for( n=0; n<=Nframes+2; n+=Nframes+2 )         // асимметричная расстановка
    { Flex &S = !n ? Stern:Stem;                   // транцевых треугольников
     int i,k=0;
      for( i=1; i<S.len; i++ )if( S[i].z<S[k].z )k=i;   // правые точки исключены
      for( i=0; i<S.len; i+=2 )Ins( n )=ir(k+i),
                               Ins( n )=il(k-i);
    }
#endif
    //  сначала просто шпангоуты - начинаются от киля вверх до палубы/ширстрека
    //    и плазовые ординаты разбрасываются парами точек
    //      по смежным шпангоутам внутри расчетной шпации
    //
   byte *St=(byte*)calloc( sizeof( byte ),Stern.len+2 ), // признаки учёта+2
        *Sm=(byte*)calloc( sizeof( byte ),Stem.len+2 );  // точек на штевнях
#if !__x86_64__ //! __i386__
   struct uList // *m-вектор индексных масок, n-истинная и len-текущая длина
   { unsigned *m; int n,len;
     uList(): m( NULL ),n( 0 ),len( 0 ){} ~uList(){ if( m )free( m ); }
     unsigned& operator[]( int k ){ return m[minmax(0,k,len-1)]; } // доступ
     unsigned& operator+=( unsigned p )                   // простое дополнение
     { if( len>=n )m=(unsigned*)realloc( m,(n+=128)*sizeof( unsigned ) );
       return m[len++]=p;
   } }          lf,rf;     // метки доступа к точкам на шпангоутах и на штевнях
   Vector RS,rs,LS,ls; Real VR,VL; unsigned u; int sl,sr,zl,zr,l,r,al,ar;

    for( n=1; n<=(k=Nframes+1); n++ ) // начальные точки совпадают, и ненулевые
    {
      L.len=R.len=lf.len=rf.len=0; sl=sr=zl=zr=2; u=0;
      //
      //   несинхронная выборка контурных точек на штевнях и шпангоутах
      //   с образованием простых координатных последовательностей, и ставятся
      //   естественные (неединичные) абсциссы шпангоутов для угловых измерений
      //
      if( n )  // оставляем первый бит для точек без повторений на пересечениях
      { for( i=0; i<Stem.len; i++ )Sm[i]&=1;  // снятие временных блокировок
        for( i=0; i<Stern.len; i++ )St[i]&=1; // при выборе точек на штевнях
      }
      //   здесь выполняется выборка точек в новые контуры числовых шпангоутов,
      //   и вышибаются рассогласования у килевой линии до первых значимых хорд
      //           ширина всех шпаций теперь не устанавливается одинаковой в 1м
      //
      for( x=Keel[n-1],k=0; k<Frame[n-1].len; k++ )    // левый без точек левее
         { V=Frame[n-1][k];
           if( V.x>=x ){ V.x=x,V.y/=2; L+=V; lf+=unsigned( LeftFrame|k ); }
         }
      for( x=Keel[n],k=0; k<Frame[n].len; k++ ) // и правый без тех, что правее
         { V=Frame[n][k];
           if( V.x<=x ){ V.x=x; V.y/=2; R+=V; rf+=unsigned( k ); }
         }
      //! оптимизация последовательности треугольников покрытия судовой обшивки
      //                               должен присутствовать начальный интервал
      Ins( n )=lf[0];             // опорные точки s# уже занесены в обшивку,
      Ins( n )=rf[0];             // контрольные z# только ожидают обработки
      if( lf.len>1 && rf.len>1 )
      { if( norm( R[1],L[0] )>norm( R[0],L[1] ) ) // первый четырёхугольник
        { Ins( n )=lf[1]; Ins( n )=rf[1]; } else
        { Ins( n )=rf[1]; Ins( n )=lf[1]; } } else // веер из треугольников
      { for( i=1; i<rf.len; i++ )Ins( n )=rf[i]; // дальше к другой шпации
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
        if( (VR=norm( R[zr],L[zl-1] ))>(VL=norm( R[zr-1],L[zl] )) )
          { ++zl; if( norm( R[zr],L[zl-1] )<=VL )++zr; } else  // с подборкой
          { ++zr; if( norm( R[zr-1],L[zl] )<=VR )++zl; }      // до параллели
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
        if( zr>=rf.len )r=zr;      // по одному контрольному фрагменту для
        if( zl>=lf.len )l=zl;      // двух смежных шпангоутов рабочей шпации
        if( l<0 )                     // пропуск если повтор и не было смещения
        { if( !(LS=L[zl]-L[zl-1]) ){ ++sl,++zl; goto InnFind; }
          if( zl<lf.len-1 ){ //if( !L[zl+1].z )fine( l ) else
                             if( !( ls=L[zl+1]-L[zl] ) )fine( l ) else
                             if( dir( LS )%dir( ls )<0.96 )fine( l ) }
          if( ( VL=dir( LS )%dir( L[zl]-R[zr] ) )<-0.1 )
            if( VL>dir( LS )%dir( L[zl+1]-R[zr] ) )fine( l )
        }
        if( r<0 )                     // градиентные ограничения точек\отрезков
        { if( !(RS=R[zr]-R[zr-1]) ){ ++zr,++sr; goto InnFind; }
          if( zr<rf.len-1 ){ //if( !R[zr+1].z )fine( r ) else
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
          { if( norm( R[ar-1],L[l] )>norm( R[ar],L[l] ) )++ar;
            ++zr; goto InnFind;  // повтор поиска внутри треугольного фрагмента
          } zr=ar+1; break;
        }
        if( r>=0 ){ zr=r; if( al<0 )al=zl;    // правая отсечка без левой точки
          if( +(ls=L[zl]-L[zl-1]) )if( dir( ls )%dir( L[zl]-R[r] )<0.12 )
          { if( norm( R[r],L[al-1] )>norm( R[r],L[al] ) )++al;
            ++zl; goto InnFind;
          } zl=al+1; break;
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
      if( (VR=norm(R[sr],L[sl-1]))>(VL=norm(R[sr-1],L[sl])) )
        {                             Ins( n )=lf[sl++];
          if( norm(R[sr],L[sl-1])<VL )Ins( n )=rf[sr++];
        } else {                      Ins( n )=rf[sr++];
          if( norm(R[sr-1],L[sl])<VR )Ins( n )=lf[sl++];
        }
/*
 if( sl>zl || sr>zr )
   { cprint( 2,26,"~$$$ n=%d, sl=%d(%d<%d), sr=%d(%d<%d)    ",
          n,sl,zl,lf.len,sr,zr,rf.len ); getch(); }
*/
      //   особая перепроверка (=пропуск) зараз всех смежных совпадающих точек
      //
//    while( sl<lf.len )if( L[sl]==L[sl-1] )zl=++sl; else break;
//    while( sr<rf.len )if( R[sr]==R[sr-1] )zr=++sr; else break;

      if( sl<lf.len || sr<rf.len )goto NewFind; // к возобновлению поиска
    }
#else
   unsigned ml=0,mr=0;         // индексы с маской для выборки точек шпангоутов
   bool Left,sl,Right,sr;   // точки на левом и правом шпангоуте/стороне шпации
   Real LSP=Length/Nframes;// условное усреднённое пространство (=длина) шпации
    for( n=0,mzR=-1; n<=Nframes; n++ )
    { mzL=mzR; k=0;         // фальшборт поднимается выше низкой главной палубы
      if( n<Nframes )       //    правый шпангоут предустанавливается очищенным
       for( i=1,mzR=Frame[n][0].z; i<Frame[n].len; i++ )
        if( !k ){ if( mzR>Frame[n][i].z )k=1; mzR=Frame[n][i].z; }
           else   if( mzR>Frame[n][i].z )mzR=Frame[n][i].z; if( !k )mzR=-1.0;
      if( n )  // оставляем первый бит для точек без повторений на пересечениях
      { for( i=0; i<Stem.len; i++ )Sm[i]&=1;  // снятие временных блокировок
        for( i=0; i<Stern.len; i++ )St[i]&=1; // при выборе точек на штевнях
      }
      //
      //  основной алгоритм выборки вершин и оптимизации бортовых треугольников
      //!  более-менее приемлемый и простой алгоритм функциональной оптимизации
      //
#define L_pass {L=Ll;do{sl=LNext(n,ml,Ll,St);Ll.x=0.0;}while(sl&& !(LS=Ll-L));}
#define R_pass {R=Rr;do{sr=RNext(n,mr,Rr,Sm);Rr.x=LSP;}while(sr&& !(RS=Rr-R));}
#define L_plus {Ins(n)=ml;L=Ll;sl=LNext(n,ml,Ll,St);Ll.x=0.0;if(!(LS=Ll-L))NY=NR;else NY=NL;}
#define R_plus {Ins(n)=mr;R=Rr;sr=RNext(n,mr,Rr,Sm);Rr.x=LSP;if(!(RS=Rr-R))NY=NL;else NY=NR;}
     int fb=0;
     Real BL,BR,UX;
     Vector L,Ll,LS=Zero,VL,NL,NR,RL,          // первая нормаль указывает вниз
            R,Rr,RS=Zero,VR,NY={0,0,-1}; NL=NR=NY;   // Shell номера n+1
      sl=LNext( n,ml,Ll,St ); Ll.x=0.0; L=Ll;        // предустановка новой
      sr=RNext( n,mr,Rr,Sm ); Rr.x=LSP; R=Rr;        // вершины треугольника
      if( sl ){ Ins( n )=ml; L_pass } // первые две точки и сразу проверяется
      if( sr ){ Ins( n )=mr; R_pass }// наличие второй сопряженной пары точек
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
// C06=0.9945218954,C12=0.9781476007,C24=0.9135454576,C30=0.8660254038,
// C45=0.70710067812,C48=0.6691306064,C75=0.2588190451,C78=0.2079116908,
// C82=0.139173101,C86=0.06975647374; ... cos( 24° ) и 78° = sin( 12° )
