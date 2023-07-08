//!
//! проект Vessel - штормовой и экстремальный ход корабля
//   ┌───────────────────────────────────────────────╖
//   │  ↓  корпус ─> волны ─> графика                ║
//   │  ↓  статика ─> динамика ─> реакции            ║
//   │  ↨  кинематика ─> излучение ─> гидромеханика  ║
//   ╞═══════════════════════════════════════════════╝
//   └── ©2018-22 Санкт-Петербург — Южно-Сахалинск — Могилев — Друть — ‏יְרוּשָׁלַיִם
//
static const char
Title[]="Штормовая мореходность корабля (вычислительный эксперимент)",
SubTitle[]="\n    Ленинград, Кораблестроительный институт"
        " \\ Санкт-Петербург, Государственный университет\n"
        "\t\t  Научно-инженерное общество судостроителей им.А.Н.Крылова,\n"
        "\t\t\t\t подсекция мореходных качеств корабля в штормовых условиях",
CopyRight[]="\n «МИДВ» ©1975-2023 Калининград–Севастополь–Ленинград–Сахалин"
            " – יְרוּשָׁלַיִם – В.Храмушин";
const char
 *Rmbs[]= { "Nord","NtO","NNO","NOtN","NO","NOtO","ONO","OtN", // Стороны света
             "Ost","OtS","OSO","SOtO","SO","SOtS","SSO","StO", // + роза ветров
           "Su\"d","StW","SSW","SWtS","SW","SWtW","WSW","WtS", // ῥόμβος — юла
            "West","WtN","WNW","NWtW","NW","NWtN","NNW","NtW" },
 *Model[]={ "Кинематика качки на 4-х точках по склону волн",  // ⌠ Hull::Statum
            "Объёмная плавучесть переменного водоизмещения",  // │
            "Давление смоченных элементов бортовой обшивки",  // │
            "Давление на встречном потоке без морских волн",  // │
            "Давление с волновыми и ходовыми потоками воды" },// ⌡
 *ModelWave[]={ " чистое штилевое море без волнения и ветра",// Field::Exp.wave
                " моделирование гидродинамики на теоретическом волнении",
                " вычислительный эксперимент с отражением волн от корпуса",
                " эксперимент с учётом скоростей течений и дисперсией волн" };
#include <unistd.h>
#include "Vessel.h"
Field *Storm=0; /// адресная ссылка для доступа к акватории с морским волнением
Hull *Vessel=0; /// теоретический корпус корабля в собственном тензорном базисе
FILE *VIL=0;    // исходный файл с настройками и протоколами опытовых испытаний
static char *Lst=(char*)calloc( MAX_PATH,4 ); // временная строчка 1k вне стэка

const byte                //! константы исходных управляющих кодов и параметров
    Mekhanik_Status=1,   //  режим гидро\механики хода и маневрирования корабля
                        // 0-склоны 1-водоизмещение 2-давление 3+ход 4+волнение
    Drawing_Hull=0x3;   /* изображение корпуса закраской, гранями треугольников
                     0 - ватерлиния строится при любом графическом раскладе
                     1 - прорисовываются только собственно штевни и шпангоуты
                     2 - оставляется подводная часть со шпангоутами над водой
                     3 - весь корпус прорисовывается целиком отчасти прозрачным
                   х04 - тоже, без обшивки и только с триангуляционными ребрами
                   х10 - разблокировка графики с включением надводного борта */

Hull::Hull():View      // в прицепе View окошко графической визуализации OpenGL
( " Аксонометрический вид корпуса корабля и профилей морских волн ",
    -12,12,450,150               //  Xpm( 4 ),Ypm( 4 ), Xpm( 64 ),Ypm( 72 )
),FileName( 0 ),ShipName( 0 ),   // Исходный Файл и название исходного проекта
  Keel( 0 ),Frame( 0 ),Shell( 0 ), // шпангоуты + штевни и обечайка по шпациям
  Statum( Mekhanik_Status ),     // индекс режимов вычислительного эксперимента
  DrawMode( Drawing_Hull ),      // изображение корпуса закраской или контурами
  hX( 1.0 ),                     // поперечная метацентрическая высота      [м]
  sT( 30.0 ),                    // интервал кинематической визуализации  [сек]
  Trim( 0.0 ),                   // дифферент по смещению центра величины [рад]
  Course( 0.0 ),dCs( _Ph/60.0 ), // курс, руль на борт(1мин), полборта(2) [рад]
  Speed( 0.0 ),cSp( 0.0 ),       // скорость погашена, машины остановлены [м/с]
  Locate( Zero ),                // прямая ссылка на текущее местоположение [м]
  Route(),Rate(),Swing(),Whirl() // конструкторы кинематики во времени
{ Vessel=this;                  // ссылка на корпус, пусть будет внешний доступ
  Nframes=Mid=0; Length=Breadth=Draught=1.0;
  DampF=(Vector){ 0.04,0.8,0.4 }; /// демпфер по абсциссе, ординате и аппликате
  DampM=(Vector){ 0.2,0.5,0.6 };  /// ... по бортовой, килевой качке и рысканию
  Ofs=Zero; mx=my=0;              // и мышку тоже в исходное
  Activate();                     // привязка к Window::Place, и первый рисунок
  glPolygonMode( GL_FRONT_AND_BACK,GL_FILL );
  color( navy ); Alfabet(22,"Arial",800).Print( 2,1," Vessel  -  " );
  color( blue ); Alfabet(20,"Courier",800).Print("штормовая гидромеханика\n");
  color( cyan ); Alfabet( 19,"Lucida",600 ).Print
                 ( "   Штормовая мореходность корабля\n"
                   "       вычислительный эксперимент\n" );
  color(green ); Alfabet( 18,"Times",1,1 ).Print
                 ( "©1975-2022 Калининград - Сахалин - יְרוּשָׁלַיִם\n\n" );
  color(black ); Alfabet( 20,"Times",1,1 ).Print( -2,-2,
                   "Оптимальное проектирование корабля\n"
                   "эффективность океанского мореплавания" );
  color(yellow); Alfabet( 12,"Times",1,1 )
                .Print( -3,0,"©Василий Храмушин" ).Save().Show();
  // WaitTime( 500 );
 int Ac; WCHAR **Av=CommandLineToArgvW( GetCommandLineW(),&Ac );
  if( !Read(  Ac>1 ? W2U( Av[1] ) : "*.vsl" ) )
       Break( "Ошибка чтения корпуса %s -> %s",W2U( Av[1] ),FileName );
  Window::Locate( Xpm( 4 ),Ypm( 4 ),Xpm( 64 ),Ypm( 72 ) ).Clear();
}
//                      Shell  n = 0 - ахтерштевень;      n = 1 - корма;
//void Hull::FullFree()    //  n = Frames+2 - форштевень; n = Frames+1 - нос;
//{ if( Nframes>0 )        //  n = [2..Frames] - корпус,  всего Frames-1 шпаций
//  { for( int i=0; i<Nframes+3; i++ )
//       { if( i<Nframes )(Frame+i)->Flex::~Flex(); Allocate( 0,Shell[i] ); }
//    Allocate( 0,Frame ),Allocate( 0,Shell ); free( FileName ),free(ShipName);
//    FileName=ShipName=NULL; Shell=NULL,Frame=NULL,Nframes=0,Keel=0;
//  }
//}
///  Начальные процедуры и повторная конфигурация параметров волновых полей
//
Field::                            // объединенная акватория морского волнения
Field(_Real L,_Real W,_Real T,     // длина, ширина бассейна, шаг времени [м,с]
       Real Lw,Real Hw,Real Dw,    // волны с обрушающимися гребнями    [м,%,°]
       Real Ls,Real Hs,Real Ds,    // свежая морская зыбь прошумевших штормов
       Real Lr,Real Hr,Real Dr )   // реликтовые волны от удаленных ураганов
: View( " Корабль и трохоидальные штормовые структуры морских волн ",
  Xpm(4),Ypm(1),Xpm(95),Ypm(91) ),  // размерения графического окна в процентах
  Tstep( T ),tKrat( M_SQRT2*3.0 ),  // интервал и кратность шага во времени [с]
  Kt( 0 ),    // нулевой счетчик инициирует начало вычислительного эксперимента
  Long( L ),Wide( W ),Ws( 0 ),  // длина, ширина и адрес опытового бассейна [м]
  Wind( "Волна" ),Swell( "Зыбь" ),Surge( "Вал" ), // 3 групповые структуры волн
  Info( this,PlaceAbove )         // текущая информация исполнительного таймера
{ const
 int Split=2; // Дробление общей акватории относительно поля ветрового волнения
 Hull &V=*Vessel; // копия ссылки доступа к определению цифровой модели корабля
  Storm=this; // Ссылка разрешения внешнего доступа к общей штормовой акватории
  Exp.wave=1; //! маска выбора варианта волнового поля и режима моделирования
              //  0 чистое штилевое, застилевшее море без волнения и ветра
              //  1 моделирование гидродинамики на чисто теоретическом волнении
              //  2 вычислительный эксперимент с трохоидальными волнами на море
              // ++ континуально-корпускулярный, полный тензорный подход
  Exp.peak=1; //  0 - простое решение в потоках 1 - трохоидальные волны {s,z}
  Exp.draw=0; //  0 - 3 метода закраски и сеточной прорисовки морского волнения
  Exp.view=0; //  0 - световая раскраска или 1 - фон с разделением границ волн
  Wind.Wind=M_SQRT1_2;  // ветровая асимметрия гребней волн под действием ветра
/*Exp.ship=1; // 1 - гидростатика  +  2 - учет скорости и давления по Бернулли
  Trun=(Tlaps=Instant.T)*3600; // отсчет реального времени для моделирования[s]
  Instant.T=0.0; //- отсчет времени от начала суток - или всегда условно разный
*/Trun=Tlaps=0.0;            // отсчет реального времени моделирования[сек,час]
  //
  //!  все начальные установки считаны, и теперь можно их заново корректировать
  //                            из файлов начальной инициализации эксперимента
       int l=1;
  for( int i=0; i<2; i++ )   // повторение выборки для Vessel.vil и <Model>.vil
  { strcpy( fname( strcpy( Lst,V.FileName ) ),"Vessel.vil" );
    if( !i )VIL=_wfopen( U2W( Lst ),L"rt" ); else    // настройка по директории
    { if( VIL ){ fclose( VIL ); ++i; }               //++ с отметкой её наличия
      VIL=_wfopen( U2W( fext( V.FileName,"vil" ) ),L"rb+" ); // файл для модели
      if( VIL && i<2 )       // без общей настройки волны в долях длины корабля
        { Lw*=V.Length/100; Ls*=V.Length/100; Lr*=V.Length/100; } // не мудрёно
    }
    if( VIL )while( !feof( VIL ) )                        // к настройке модели
    { char *s=getString( VIL );
      if( !(l=strcut( s )) )break;
      if( s[0]==';' || !memcmp( s,"//",2 ) )continue;
      while( *s && *s<=' ' )s++;
      //  подстройка трёх групповых структур штормового морского волнения
      if( !memcmp(s,Wind.Title,l=strlen(Wind.Title) ))Wind.Get( s+l,Lw,Hw,Dw );
      if( !memcmp(s,Swell.Title,l=strlen(Swell.Title)))Swell.Get(s+l,Ls,Hs,Ds);
      if( !memcmp(s,Surge.Title,l=strlen(Surge.Title)))Surge.Get(s+l,Lr,Hr,Dr);
      if( !memcmp( s,"Wave:",5 )  )Wind.Get(  s+5,Lw,Hw,Dw );
      if( !memcmp( s,"Swell:",6 ) )Swell.Get( s+6,Ls,Hs,Ds );
      if( !memcmp( s,"Surge:",6 ) )Surge.Get( s+6,Lr,Hr,Dr );
      // изменение посадки с назначением курса и заданной скорости хода корабля
      if( !memcmp( s,"Корабль:",l=strlen( "Корабль:" ) ) )V.Get( s+l );
      if( !memcmp( s,"Ship:",5 ) )V.Get( s+5 );
      //  настройка вычислительной акватории и условий генерации штормовых волн
      if( !memcmp( s,"Море:",l=strlen( "Море:" ) ) )Get( s+l );
      if( !memcmp( s,"Sea:",4 ) )Get( s+4 );
  } }
  Wind.Initial( Lw,Hw,Dw );  // ветровые волны с обрушающимися гребнями [м,%,°]
  Swell.Initial( Ls,Hs,Ds ); // свежая морская зыбь недавно прошумевших штормов
  Surge.Initial( Lr,Hr,Dr ); // реликтовые пологие волны от удаленных ураганов
  //
  //    по самой мелкой сетке настраивается общая акватория со Split дроблением
  //
  mX=max( max( Wind.Mx,Swell.Mx ),Surge.Mx )*Split; // сетка по максимуму ячеек
  dS=Long/( mX-1 );                 // создание результирующей матрицы аппликат
  mY=Wide/dS+1;                     // уровней моря (и поверхностных скоростей)
  Ws=(Vector**)Allocate( mY,mX*sizeof(Vector) );  // на общей равномерной сетке
  //
  //!  переключение входного файла инициализации в режим протокола эксперимента
  //
  if( VIL )
  { int M,D,Y; Instant.UnPack( M,D,Y );
    fseek( VIL,ftell( VIL ),SEEK_SET );
    fprintf( VIL,"\n\n %s\n %s\n"+(l?0:2),::Title,
             "≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈" );
    fprintf( VIL," %s %04d %s %02d,%s\n"
    " акватория: { %.0f×%.0f м }[%d·%d]~δS=%.2f м, { δt=%.1g\"/%.1f }[%+d]\n",
            _Day[int(Instant.D%7)],Y,_Mnt[M-1],D,DtoA( Instant.T,3,"ª" ),
                                     Long,Wide,mX,mY,dS,Tstep,tKrat,Kt );
  }
  eye=(Vector){ 150.0,-16.4,0.0 },look=(Vector){ 0.0,-10.0,0.0 };  // под обзор
  Distance=-0.8*Long;   // расстояние от точки взгляда до места в центре обзора
  mx=my=0;              // указатель мыши ставится в исходное/нулевое положение
}
//     Поле скорости появляется только в случае трохоидальных волн peak=true
//      ? в случае проведения реального вычислительного эксперимента wave>1
Waves&                                       //! Cw,Ow строго связаны Length=Lw
Waves::Initial(_Real Lw,_Real Hw,_Real Dir ) // характер и направление волнения
{ Cw = sqrt( _g*Lw/_Pd );         // фазовая скорость трохоидальной волны [м/с]
  Ow = _Pd*Cw/Lw,                 // круговая частота чисто прогрессивной волны
  Ds = Storm->Tstep*Cw;           //    контролируемый шаг квадратной сетки [м]
  Mx=int(Storm->Long/Ds/2+1)*2+1, //  волновой бассейн строится только в чётной
  My=int(Storm->Wide/Ds/2+1)*2+1; // размерности с малым перекрытием результата
  H =(Vector**)Allocate( My,Mx*sizeof(Vector),H ); //+ осреднённый поток [м²/с]
  --Mx,--My;                   // чётная размерность индексов в матрицах [0..N]
  Length = Lw,                 // заданная длина свободной для пакета волны [м]
  Extend = Lw*_Pd/Hw,          // протяженность центрального девятого вала  [м]
  Height = Lw*Hw/hW;           //! высота, радиус×2·трохоиды на поверхности [м]
  axiZ( _Pi-_dR*Dir );         // тензор направления движения волны из картушки
  return *this;
}
Waves& Waves::Clear()  // очистка для перезапусков вычислительных экспериментов
{ Diagonal=hypot(Mx,My)*Ds/2+Storm->Trun*Cw; // дистанция свободной границы [м]
  if( Storm->Exp.wave<2 )Diagonal=-Diagonal; // мат.модель действует всегда
  if( Storm->Exp.wave>1 && Storm->Exp.peak )
  { V=(Vector**)Allocate( My+1,(Mx+1)*sizeof( Vector ),V );   // скорость [м/с]
    for( int j=0; j<=My; j++ )memset( V[j],0,(Mx+1)*sizeof( Vector ) );
  } else// иначе остаётся только простой массив уровней моря с полными потоками
  { if( V )Allocate( 0,0,V ); V=NULL;
  } //Tw=0.0;             // фазовый отсчет для первого вступления фронта волны
  for( int j=0; j<=My; j++ )
   if( Storm->Exp.peak )    // штилевая координатная поверхность в пространстве
    for( int i=0; i<=Mx; i++ ) // координаты точек в массиве если там не потоки
      H[j][i]=(Vector){ Ds*(i-Mx/2),Ds*(j-My/2),0 };
   else memset( H[j],0,(Mx+1)*sizeof( Vector ) );                   // [м]
 /* H[22][20].z=22; H[22][21].z=23; H[22][22].z=23; H[22][23].z=22; // и всякий
    H[23][20].z=20; H[23][21].z=22; H[23][22].z=22; H[23][23].z=20; // пожарный
 */ return *this;
}
///    Динамическая или интерактивная перестройка всех волновых полей разом
//                  или исходная предустановка вычислительного эксперимента
//
Field& Field::Original( bool init ) // очистка или переустановка волнового поля
{ for( int y=0; y<mY; y++ )      // предварительная разметка сеточных координат
  for( int x=0; x<mX; x++ )      // визуализации для обобщенного волнового поля
     { Vector &W=Ws[y][x]; W.x=x*dS-Long/2; W.y=dS*(y-Real(mY-1)/2); W.z=0.0; }
  if( init || Exp.wave>=2 )
    { if( logTime() )fprintf( VIL," ⇒ возобновление ⇒ 0\"[0]\n" );
      Trun=Tlaps=0.0; Kt=0;      // реальное время для моделирования [сек, час]
      Instant.Now();             // текущее время к фазе волн на данный момент
      Vessel->Original();
    } Wind.Clear();                             // ветер
      Swell.Clear();                            // зыбь
      Surge.Clear(); wavePrint(); return *this; // вал
}
Hull& Hull::Original()
{ if( !Storm )return *this;      // здесь пролог может быть ещё не завершён
  Volume=Surface=Floatage=0.0;   // согласовать с Vs-math
  inWater=0.0; inMass=0.0; vR=0.0; // mM=0.0; - на Three-Initial
  Gravity=Buoyancy=Floatable=Metacenter=Zero; Locate=Zero;
  //! контролируемый начальный отсчет для запуска всего процесса моделирования
  Route.length=Rate.length=Swing.length=Whirl.length=0;
  Route += Zero;             // первый координатный отсчет на маршруте парохода
  Rate  += Zero;             // начальная скорость хода == 0              [м/с]
  Swing += Zero;             // ориентация в пространстве в трёх углах Крылова
  Whirl += Zero;             // скорости изменения крена, дифферента и курса
 return Initial().Floating();// установка главных осей и геометрические расчеты
//               .wPrint( true );
}
//!    Интерактивное управление выполняющимся вычислительным экспериментом
///
static bool AllKeyb( byte Keyb )
{ Hull &Ship=*Vessel;                    // Field &Sea=*Storm;
  switch( Keyb )
  { case _BkSp:                          // временное прекращение\возобновление
       { static byte wave=0;             //   морского волнения (если оно было)
         Field &S=*Storm; wave|=S.Exp.wave<<2; S.Exp.wave=wave&3; wave>>=2;
         if( logTime() )
         { if( wave )fprintf(VIL,"  ⇒ исключение морского волнения [%d]",wave);
           else if( S.Exp.wave )
                fprintf( VIL,"  ⇒ возобновление: «%s»",ModelWave[S.Exp.wave] );
         } Storm->Original( false );   // возврат вычислений в начальный момент
       } break;
    case '1': case '!': { Vector W={ _Ph/3.0 }; W=Ship*W; // вызов Крылова в МБ
                          Ship *= Krylov( W.x,W.y,W.z ); } break;   // задаётся
    case '3': case '#': { Vector W={-_Ph/3.0 }; W=Ship*W; // импульс крена ±30°
                          Ship *= Krylov( W.x,W.y,W.z ); } break;
    case '8': case '*': Ship.cSp += _Mile/1800.0; break;      // +2 узла->м/сек
    case '2': case '@': Ship.cSp -= _Mile/1800.0; break;      // реверс -2 узла
    case '0': case ')': Ship.cSp = 0.0;           break;      // стоп машина
    case '7': case '&':           Ship.dCs=_Pi/60.0; Keyb=0;
    case '4': case '$': if( Keyb )Ship.dCs=_Pd/60.0;
                        angle(Ship.Course-=_Ph*0.125); break; // курс ±1 румб
    case '9': case '(':           Ship.dCs=_Pi/60.0; Keyb=0;
    case '6': case '^': if( Keyb )Ship.dCs=_Pd/60.0;
                        angle(Ship.Course+=_Ph*0.125); break; // курс ±1 румб
    case '5': case '%': angle(Ship.Course=-Ship.Swing[-1].z); // так держать
//  case _Esc: return false;
  } return true;                  // и все иные запросы клавиатуры сбрасываются
}
//     ... и далее раздельно для каждого окна
//
static const char
 *Plus[]={ "§ ",  "« Управление ходом корабля »",
           "<8/2>"," ± два узла вперёд/обратно",
           "<7/9>"," ± румб влево/право полборта",
           "<4/6>"," ± румб влево/вправо на борт",
           "<1/3>"," ± крен 30° на левый/правый",
           "<5/Alt>"," руль прямо/так держать",
           "<0>    ","   стоп машина",
           "§ ",  "« Настройки моделирования »",
           "<Tab/Space>", "   закраски/контуры",
           "<Ctrl/+Shift> ","задержка или стоп",
           "<стрелки•leftMouse>",  "ориентация",
           "<+Shift•rightMouse>",  "  смещение",
           "<+Ctrl•(roll)>"," дальность, наклон",
           "<Del>+<Shift> "," исходный обзор",
           "<BkSp> "," остановка волнения",0
         };
bool Hull::KeyBoard( byte Keyb )                     // С краткой подсказкой по
{ const static char                                  // настройкам и методам
     *Id[]={ "Ship","  Целевое проектирование,  ",   // визуализации корпуса и
                   "   теория и штормовая",          // всего вычислительного
                   "    мореходность корабля",0 },   // эксперимента
     *Cmds[]={ "F1 ","- краткая справка",
               "F2 ","запись в формате «Vessel».obj",
               "F3 ","выбор и чтение иного корпуса",
               "F4 ","параметры корпуса/картинки",
               "F8 ","гидромеханика штормования",0 };     // Activate();
  switch( Keyb )
  { case _F1: Help( Id,Cmds,Plus,1,1 ); break;            // справка с корпусом
    case _F2: Write(); break;                 // запись 3D-модели в формате.obj
    case _F3: if( !Read( "*.vsl" ) )
              Break( "Ошибка повторного чтения корпуса %s",FileName );
          Storm->Original( false ); break;    // очистка волнового поля
    case _F4: Config(); break;                // конфигурация формы отображения
    case _F8: Model_Config( this ); break;
    case _Tab: DrawMode=(DrawMode&~3)|((DrawMode-1)&3); break;
    case _Enter:
    case _Blank: DrawMode^=4; break;    // прорисовка корпуса или треугольников
    case _Del: if( ScanStatus()&SHIFT ) // восстановление исходного отображения
    { DrawMode=0x3; Initial();          // исходный вариант цветовой раскраски
    } Distance=-2.4*sqrt(sqr(Length)+sqr(Breadth*2)+sqr(Draught*4));//дальность
      eye=(Vector){45,-15,0},look=(Vector){-5,0,0}; break;// векторы ориентации
    case _Esc:
     if( MessageBox( hWnd,UtA("Завершение вычислений")," Vessel :: Ship",
       MB_APPLMODAL|MB_ICONQUESTION|MB_OKCANCEL|MB_DEFBUTTON1 )==IDOK )Close();
            return true;                               // ~Window(); ~Hull()
   default: return View::KeyBoard( Keyb );             // и направления взгляда
  } Draw(); return true;                               // иначе команда принята
}
bool Field::KeyBoard( byte Keyb )
{ const static char                                     // Краткая подсказка по
     *Id[]={ "Sea",":","Вычислительный эксперимент",    // настройкам всего
                       "штормового маневрирования",     // вычислительного
                       "корабля в открытом море",0 },   // эксперимента
     *Cmds[]={ "F1 -"," краткая справка",
               "F4  ","настройка волнового режима",
               "F8  ","штормовая гидромеханика",0 };    // Activate();
  switch( Keyb )                                        // .. справка, что выше
  { case _F1: Help( Id,Cmds,Plus,-1,-1 ); break;
    case _F4: Config(); break;           // режимы вычислительного эксперимента
    case _F8: Model_Config( this ); break;
    case _Tab: Exp.draw+=ScanStatus()&SHIFT ? -1:1; break;  // триангуляционная
    case _Blank: Exp.view+=ScanStatus()&SHIFT ? -1:1; break; // сетка-раскраска
    case _Del: if( ScanStatus()&SHIFT )
               { Exp.draw=Exp.view=0;  // восстановление волновых закрасок
                 Original( false );    // возврат вычислений в начальный момент
               } Distance=-0.8*Long;   // ... от точки взгляда до места обзора
               eye=(Vector){ 150,-16.4,0 },look=(Vector){ 0,-10,0 }; break;
    case _Esc:
     if( MessageBox( 0,UtA("Завершение эксперимента")," Storm :: Sea",
       MB_APPLMODAL|MB_ICONQUESTION|MB_OKCANCEL|MB_DEFBUTTON1 )==IDOK )Close();
            return true;
   default: return View::KeyBoard( Keyb );
  } Draw(); return true;                 // команда принята к исполнению
}
static bool Hull_and_Waves_Draw()        // вся графика исполняется параллельно
{ static int recurse=0;                  // по специальному таймеру
  if( !recurse++ )
  { Vessel->Draw();    // сначала пароход идёт сам по себе в собственном окошке
    Storm->Draw();     // затем картинка штормового волнения вместе с пароходом
    recurse=0;         // при возникновении рекурсии новые рисунки пропускаются
  } Vessel->wPrint();  // ... и геометрические таблички -> на текстовую консоль
  return false;
}
//!    Главная процедура запускает процессы реального времени
///                     и зацикливается на опросах клавиатуры
//#include <Fenv.h>
//#include <Float.h>
//#include <locale.h>

int main() // int ans, char **av )
{//            _clearfp();
 //unsigned CW=_control87
 //(_EM_OVERFLOW|_EM_UNDERFLOW|_EM_ZERODIVIDE|_EM_INVALID|_EM_DENORMAL,_MCW_EM);
 //feclear{raise}except( FE_ALL_EXCEPT );
 //    setlocale( LC_ALL,".UTF8" );
 //    setlocale( LC_CTYPE,".UTF8" );
  textsize( 92,36 ),
  texttitle( Title ),
  textcolor( WHITE ),cprint( 2,1,Title ),
  textcolor( LIGHTCYAN ),printf( SubTitle ),
  textcolor( CYAN ),printf( CopyRight ),textcolor( LIGHTGREEN ),cprint( 1,27,
    " C  [м] - положение мгновенного центра величины (подводного объёма)\n"
    " mC (δ) - матрица ориентации корпуса (корабельные оси в морском базисе)\n"
    " W  [м] - корабельные координаты центра площади действующей ватерлинии\n"
    " vW (δ) - интегральная нормаль к действию гидростатических сил всплытия\n"
    " vS [H/ρ] - накопительный вектор сил от перепадов уровня моря\n"
    " vR [H·м/ρ] - поворотный момент от криво-наклонной ватерлинии\n"
    " mW [м⁴]4 - моменты инерции площади действующей ватерлинии\n"
    " mM [м⁵]5 - объёмные моменты инерции погруженного корпуса\n"
    " inMass   - исходные моменты инерции корпуса\n" );
  //
  // конструктор выбора корпуса - открывает и настраивает графику с чтением
  // из файла и формирование представления корпуса Vessel в оперативной памяти
  // с построением независимой прорисовки 3D корпуса под мышкой и таймером
  //
 Hull Ship;
  //
  //   графическая среда для построения и визуализации групповых
  //   трохоидальных структур штормового волнения всей акватории
  //
 Field Sea( 800,720,     // Long, Wide - длина и ширина штормовой акватории [м]
                0.5,     //.25 TimeStep заданный шаг времени волновых полей [c]
          64,0.9,-165,   //+10 ветровые волны с обрушающимися гребнями  [м,%,°]
         100,0.44,160,   //-20 свежая морская зыбь неподалёку прошедших штормов
         160,0.2,-130 ); //+50 пологие реликтовые волны от удаленных ураганов

//Start_Experiment();    ///   !!! - считывание конфигурационного файла !!!

  Sea.Original( true );   // конструктор перенастройки волнения и гидромеханики
                          //  с подготовкой корпуса для начальной инициализации
  Ship.wPrint( true );    // описатели парохода на экране-консоли и в протоколе
  logWave();              // изначальные характеристики волн для протокола

  Sea.Window::KeyBoard( AllKeyb ); // самый нижний уровень виртуальной рекурсии
  Ship.Window::KeyBoard( AllKeyb ); // доступен при прямом обращении в Window

  Ship.Above();           // установка активности окна с прорисовками корабля

//Ship.Initial()          // установка главных осей с исходными геометрическими
//    .Floating( false ); // расчётами по корпусу, без графической визуализации

   Sea.SetTimer( 500 );   // вычисления по волнению и механике корабля (½ сек)
  Ship.SetTimer( 1000,Hull_and_Waves_Draw );     // изображение корабля и моря,
                 // часы и прочая оперативная информация справа в нижней строке

//#pragma omp parallel //sections
//{
//#pragma omp master
//{
//#pragma omp section
//{
//do{ Sea.Simulation(); WaitTime( 100 ); }while( Ship.asYet() && Sea.asYet() );

//#pragma omp task
{
  do                                            // -=+*&#
  { static int i=0; cprint( 1,22,"%c",( "0123456789ABCDEF" )[++i%=16] );
    WaitTime( 250 );    // приостановка без снижения производительности Windows
  //  Sleep( 25 );      // или варианты полной задержки по блокирующему таймеру
  //  while( Ship.GetKey()||Sea.GetKey() ); // где-то в прерываниях недовыборка
  } while( Ship.Ready() && Sea.Ready() );
} // task
//} // master
//} // parallel
//#pragma omp taskwait

  if( Ship.Ready() )Ship.Close();
  if( Sea.Ready() )Sea.Close();
  textcolor( LIGHTMAGENTA ); cprint( 1,24,"\n << успешное завершение >> " );
  if( VIL )
  { fprintf( VIL,"\n\n <<%s успешное завершение >>\n",DtoA(Sea.Trun/3600,3) );
    ftruncate( fileno(VIL),ftell( VIL ) ); fclose( VIL ); VIL=0; // отсечение
  } return EXIT_SUCCESS;
}
//  Блок текущей консольной текстовой информации о штормовом море и корабле
//
void logWave()
{ wavePrint(); if(!VIL )return; // процедура печати используется для обновления
  logTime();                    //  отметка момента времени опытовых испытаний
  Field &S=*Storm;              //  при изменении параметров волн Field::Config
  fprintf( VIL,"\n" );
//fprintf( VIL,"\n Время: %s { δt=%.1g\"/%.1f }[%+d]\n",
//         DtoA( S.Trun/3600.0,-3 ),S.Tstep,S.tKrat,S.Kt );
  for( int y=0; y<=2; y++ )
  { Waves &W = !y?S.Wind:(y==1?S.Swell:S.Surge);
    fprintf( VIL,"   %s: "+(2-y),W.Title );
    fprintf( VIL,!y
  ? "λ=%3.0f м, τ=%4.1f\", ζ=%4.2f м, α=%4.2f, C=%4.1f м/с, A=%3.0f°, δS=%.1f м [%d·%d]\n"
  : "  %3.0f м,   %4.1f\",   %4.2f м,  =%4.2f,   %4.1f м/с,   %3.0f°,    %.1f м [%d·%d]\n",
     W.Length,W.Length/W.Cw,W.Height,hW*W.Height/W.Length,
     W.Cw,(_Pi-atan2(W.x.y,W.x.x))*_Rd,W.Ds,W.Mx,W.My );
  }
  fprintf( VIL,"  ⇒ %s (%s)\n",ModelWave[S.Exp.wave],S.Exp.peak
         ? "трохоидальный процесс":"поверхностный поток" );
}
void wavePrint()                // процедура печати используется для обновления
{ Field &S=*Storm;              //  при изменении параметров волн Field::Config
  for( int y=10; y<=12; y++ )
  { Waves &W = y==10 ? S.Wind : ( y==11 ? S.Swell : S.Surge );
    textcolor( LIGHTCYAN ); cprint( 2,y,"%-5s: ",W.Title );
    textcolor( LIGHTGRAY ); cprint( 9,y,y==10
  ? "L=%3.0f м, T=%4.1f\", A=%4.2f м, C=%4.1f м/с, Dir=%3.0f°, Ds=%.1f м [%d·%d] = {%.0f·%.0f} м\n"
  : "  %3.0f м,   %4.1f\",   %4.2f м,   %4.1f м/с,     %3.0f°,    %.1f м [%d·%d] \n",
     W.Length,W.Length/W.Cw,W.Height,W.Cw,(_Pi-atan2(W.x.y,W.x.x))*_Rd,
     W.Ds,W.Mx,W.My,S.Long,S.Wide );
} }
//inline void printB(unsigned B){ if(B>>1)printB(B>>1); putch((B&1)?'1':'0'); }

Hull& Hull::wPrint( bool log ) // информация по смоченному корпусу на текстовой
{ Field &F=*Storm;            //             консоли и в протоколе эксперимента
  textcolor( YELLOW ),       // текстовые данные о состоянии и динамике корабля
  cprint( 1,14," Time %s + %3.2f\"      \n"
               " Speed %3.1fуз(%4.2f=%3.1fL)  \n"
               " Volume %1.0f << %1.0f  \n"
               " Surface %1.0f << %1.0f  \n"
               " Floatable %1.0f << %1.0f  \n"
               " μCenter %4.1f >> %4.1f -- %3.1f Gravity.z  \n"
               "       h %4.2f >> %4.2f -- μM %3.1f >> %3.1f   ",
          DtoA(F.Kt*F.Tstep/F.tKrat/3600,3),F.Tstep,Speed*3600/_Mile,
          Speed/sqrt(_g*Length),sqr(Speed)*_Pd/_g/Length,Volume,iV,
          Surface,iS,Floatage,iF, Metacenter.x,vM.x,Gravity.z, hX,vM.z,
          Metacenter.y,vM.y ),
  cprint( 80,8,"Statum{ %X } ",Statum ), // printB( Statum );
  textcolor( WHITE ),cprint( 28,14,"C:{%6.1f,%.1f,%-6.1f}  ",vB.x,vB.y,vB.z );
#if 0
  cprint( 55,14,"mC:| %5.2f %-5.2f %-5.2f | = %s  ",x.x,x.y,x.z,DtoA(asin((z.y-y.z)/2)*_Rd,1) );
  cprint( 58,15,   "| %5.2f %-5.2f %-5.2f | = %s  ",y.x,y.y,y.z,DtoA(asin((x.z-z.x)/2)*_Rd,1) );
  cprint( 58,16,   "| %5.2f %-5.2f %-5.2f | = %s  ",z.x,z.y,z.z,DtoA(asin((y.x-x.y)/2)*_Rd,1) );
#else
  cprint( 55,14,"mC:| %5.2f %-5.2f %-5.2f | X %s  ",x.x,x.y,x.z,DtoA(atan2(-z.y,z.z )*_Rd,-1)),
  cprint( 58,15,   "| %5.2f %-5.2f %-5.2f | Y %s  ",y.x,y.y,y.z,DtoA(atan2( x.z,x.x )*_Rd,-1)),
  cprint( 58,16,   "| %5.2f %-5.2f %-5.2f | Z %s  ",z.x,z.y,z.z,DtoA(atan2(-x.y,x.x )*_Rd,-1));
#endif
  textcolor( LIGHTCYAN ),
  cprint( 28,15, "W:{%6.1f,%.1f,%-6.1f}  ",vF.x,vF.y,vF.z ),
  cprint( 27,16,"vW:{%6.1f,%.1f,%-6.1f}  ",vW.x,vW.y,vW.z ),
  textcolor( LIGHTGREEN,BLUE ),
  cprint( 27,17,"mW:| %7.0f  %-9.1f | ",mW.x.x,mW.x.y ),
  cprint( 30,18,   "| %7.1f  %-9.0f | ",mW.y.x,mW.y.y ),
//cprint( 55,17,"mM:| %7.0f  %-8.1f  %-9.1f | ",mM.x.x,mM.x.y,mM.x.z ),
//cprint( 58,18,   "| %7.1f  %-8.0f  %-9.1f | ",mM.y.x,mM.y.y,mM.y.z ),
//cprint( 58,19,   "| %7.1f  %-8.1f  %-9.0f | ",mM.z.x,mM.z.y,mM.z.z ),
  textcolor( LIGHTGRAY,BLACK ),
  cprint( 51,20,"inMass:"
                "| %7.0f  %-8.1f  %-9.1f | ",inMass.x.x,inMass.x.y,inMass.x.z ),
  cprint( 58,21,"| %7.1f  %-8.0f  %-9.1f | ",inMass.y.x,inMass.y.y,inMass.y.z ),
  cprint( 58,22,"| %7.1f  %-8.1f  %-9.0f | ",inMass.z.x,inMass.z.y,inMass.z.z ),
  textcolor( GREEN );
  if( Statum )cprint( 55,23,"vS:{%6.1f,%.1f,%-6.1f}  ",vS.x,vS.y,vS.z ),
              cprint( 55,24,"vR:{%6.1f,%.1f,%-6.1f}  ",vR.x,vR.y,vR.z );
  if( log )
  { textcolor( WHITE ); cprint( 1,7,"  >>> %s \n"
      "  >>> { L=%g, B=%g, T=%g, Ψ=%s\\δd=%.0fсм }^%g  K(а.%d<шп[%d]>%d.ф)"
//    " гидромеханика{ %X }  ",
      ,ShipName,Length,Breadth,Draught,DtoA(Trim*_Rd,2),asin(Trim)*Length*50,
      Ofs.z,Stern.length,Nframes,Stem.length,Statum );
    if( VIL )
    { logTime(); fprintf( VIL,"\n  ⇒ %s \n"
      "  ⇒ { L=%g, B=%g, T=%g, Ψ=%s\\δd=%.0fсм }^%g  К(а.%d<шп[%d]>%d.ф)\n"
      "  ⇒ { δ=%.2f, W=%.1f m³, S=%.1f m², F=%.1f m² }\n"
      "  ⇒ С{ x=%.1f, z=%.2f }, zG=%.2f, r=%.2f, h=%.2f [м]",
      ShipName,Length,Breadth,Draught,DtoA(Trim*_Rd,2),asin(Trim)*Length*50,
      Ofs.z,Stern.length,Nframes,Stem.length,Volume/Length/Breadth/Draught,
      Volume,Surface,Floatage,Buoyancy.x,Buoyancy.z,Gravity.z,
                              Metacenter.z-Buoyancy.z,hX );
      if( F.Kt<2 )logHydro(),logMdemp(),logAdemp(); fprintf(VIL,"\n");
    }
  } return *this;
}
