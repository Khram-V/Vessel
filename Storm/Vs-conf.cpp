//
//      Настройки вычислительного эксперимента
//              с визуализацией вычислительных процессов
//
//                                                   2019-02-07 Старый Петергоф
//
#include "Aurora.h"
#include <ctype.h>
/* Statum: Выбор и установка моделей гидромеханики корабля в штормовом море
              --   0 - кинематическое позиционирование(не скольжение) корпуса
              --       над волнением по хордам четырёх точек склона, иначе:
              --   1 - объёмное гравитационное всплытие: борт-киль-вертикаль
              --   2 - гидростатическое давление в волне на смоченной обшивке
              --   3 ÷ ≈ вихреисточники обтекания корпуса набегающим потоком
              --   4 - компоненты скорости на обшивке под волновыми гребнями
           Объекты и производные операции с корпусом на волнении
              запрос выбора метода прорисовки корпуса динамически цепляется
              за активное окно Window с наложением собственной площадки Place
                   0 - ватерлиния строится при любом раскладе
                   1 - только шпангоуты и штевни
                   2 - подводная часть со шпангоутами над водой
                   3 - корпус прорисовывается целиком
                   4 - тоже, но в пустых триангуляционных гранях
       демпфирование визуально отсчитывается от нуля(без) на увеличение(тормоз)
*/
Hull& Hull::Config()
{ const char *HView[]={ "Полупрозрачная тень, гидростатические отметки",
                        "Контуры шпангоутов при поверхности ватерлинии",
                        "Смоченная обшивка под поверхностью ватерлинии",
                        "Обшивка корабельного корпуса и надводный борт" };
  Real D=Draught,sTime=sT/60;
  Mlist Menu[]={ { 1,0,"   Гидростатика и динамика корпуса" }          // 0
    , { 2,34 }        // варианты гидромеханики корабля и морских волн == 1
    , { 1,5,"Начальная метацентрическая высота:  h=%-4.1lf",&hX },{ 0,0," м" }     // 2,3
    , { 1,4,"Факторы демпфирования: x=%-4.2lf",&muF.x },
                            {0,4,"  y=%-4.2lf",&muF.y }, // 4,5
                            {0,4,"  z=%-4.2lf",&muF.z } // 6
    , { 1,4,"  бортовая: %-4.2lf",&muM.x},{0,4," килевая: %-4.2lf",&muM.y} // 7,8
                                        ,{0,4," рыскание: %-4.2lf",&muM.z} // 9
    , { 1,6,"Практическая осадка:    %6.2lf",&D },    { 0,0," м" }     // 10,11
    , { 1,34 }      // закраска или контуры триангуляционного покрытия    12
    , { 1,34 }      // признаки вариантов прорисовки корпуса              13
//  , { 1,7,"Протяжённость графиков во времени %7.2lf",&sTime },{ 0,0," мин" }   // 14,15
    , { 1,5,"Протяжённость кинематической выборки %-5.4lg",&sTime },{ 0,0,"мин" }   // 14,15
    };
  TextMenu T( Mlist(Menu),this,1,1 ); int ans=-1; Real h=hX;
  Vector DF=muF,DM=muM;
  do
  { Menu[13].Msg=(char*)HView[DrawMode&3]; // выбор метода прорисовки корпуса
    Menu[12].Msg=DrawMode&4 ? "Контуры рёбер триангуляционного покрытия"
                            : "Закрашиваемая поверхность корабельной обшивки";
    Menu[1].Msg=Model[Statum];
    if( (ans=T.Answer( ans ))==12 )DrawMode^=4; else
    if( ans==13 )DrawMode=(DrawMode&~3)|((DrawMode-1)&3); else
    if( ans==1 )Model_Config( this );
    if( h!=hX )// сдвиг центра масс и пересчет тензора инерции под новую высоту
    { Vector G=Gravity; Gravity.z+=h-hX; h=hX; inMass=Steiner(inMass,G,Gravity);
      logMeta();                      //! проверить !
    }
    if( D!=Draught )                  // предварительная очистка волнового поля
    { if( !Read( FileName,D ) )       // главные оси и геометрические параметры
           Break( "Ошибка повторного чтения корпуса %s",FileName );
    //int sKt=Storm->Kt;
      Storm->Original( false ).Kt=0;
      Initial().Floating(); //Storm->Kt=sKt;
      wPrint( true );
    }
    if( DM!=muM || DF!=muF ){ DM=muM; DF=muF; logDamp(); }
    DampInit();                                // факторы демпфирования сдвигов
    sT=max( 0.5,sTime )*60; sTime=sT/60.0;     // протяжённость графиков качки
  } while( ans!=_Esc );
  return *this;
}
//!    Ключи Exp.peak & .wave могут изменяться только здесь
//        так как для этого нужен контроль состава структур
//
Field& Field::Config() // Height = 1.134*Lw*Hw/_Pd/2.0;
{ static Real H1,h1,H2,h2,H3,h3,g1,g2,g3;
  Mlist Menu[]={ { 1,0,"Вычислительный эксперимент" }                      // 0
  , { 2,35 }  // маска режимов ведения вычислительного эксперимента           1
  , { 1,34 }  // трохоидальные волны или колебания уровня в полных потоках    2
  , { 1,34 }  // закраска или контуры триангуляционного покрытия              3
  , { 1,34 }  // освещенные бело-голубые склоны / выделение уровня            4
  , { 1,0,"Цуги:   длина  высота    направление" }                        //  5
  , { 1,4," Ветер:%4.0lf",&(Wind.Length) },{0,4," м  %4.2lf",&h1 }       // 6,7
  , { 0,4,"\\%-4.1lf",&(Wind.Height) },{0,4,"м  %4.0lf",&g1 },{ 0,0," град°" }
  , { 1,4," Зыбь: %4.0lf",&(Swell.Length) },{0,4," м  %4.2lf",&h2 }    // 11,12
  , { 0,4,"\\%-4.1lf",&(Swell.Height) },{0,4,"м  %4.0lf",&g2 },{ 0,0," град°" }
  , { 1,4," Вал:  %4.0lf",&(Surge.Length)},{0,4," м  %4.2lf",&h3}      // 16,17
  , { 0,4,"\\%-4.1lf",&(Surge.Height) },{0,4,"м  %4.0lf",&g3 },{ 0,0," град°" }
  , { 1,8,"Расстояние от корабля: %5.0lf",&Distance },{0,0," м"}       // 21,22
  };
  TextMenu T( Mlist(Menu),this,-2,-1 ); int ans=-1;
  do
  { switch( Exp.wave )
    { case 0: Menu[1].Msg="Чистое штилевое и безветренное море "; break;
      case 1: Menu[1].Msg="Математическая модель штормовых волн"; break;
      case 2: Menu[1].Msg="Трохоидальный вычислительный процесс"; /* break;
      case 3: Menu[1].Msg="Континуально-корпускулярные решения "; */ }
    switch( Exp.peak )
    { case 0: Menu[2].Msg="Поверхностные волны в полных потоках"; break;
      case 1: Menu[2].Msg="Высокие волны с дрейфовыми течениями"; }
    switch( Exp.draw )
    { case 0: Menu[3].Msg="Раскраска сине-зелёно-пенных склонов"; break;
      case 1: Menu[3].Msg="Сеточное покрытие в цветовой палитре"; break;
      case 2: Menu[3].Msg="Разреженная сетка, сто контуров волн"; break;
      case 3: Menu[3].Msg="Профили волнения по курсу и траверзу"; }
    switch( Exp.view )
    { case 0: Menu[4].Msg="Тёмная вода с освещёнными оттенками "; break;
      case 1: Menu[4].Msg="Светлая вода с выделением ватерлинии"; break;
      case 2: Menu[4].Msg="Тёмная вода + три слоя исходных волн"; break;
      case 3: Menu[4].Msg="Светлая вода с послойным разложением";
    }
    H1=h1=hW*Wind.Height/Wind.Length;   g1=(_Pi-atan2(Wind.x.y,Wind.x.x))*_Rd;
    H2=h2=hW*Swell.Height/Swell.Length; g2=(_Pi-atan2(Swell.x.y,Swell.x.x))*_Rd;
    H3=h3=hW*Surge.Height/Surge.Length; g3=(_Pi-atan2(Surge.x.y,Surge.x.x))*_Rd;
    switch( ans=T.Answer( ans ) )
    { case  1: Exp.wave++; Exp.wave%=3; break; // Initial() модель волнения
      case  2: Exp.peak^=true;  break;         // Initial() колебания или волны
      case  3: Exp.draw+=ScanStatus()&SHIFT?-1:1; break;
      case  4: Exp.view+=ScanStatus()&SHIFT?-1:1; break;
//    case  7: Wind.Height =h1*Wind.Length/hW;  break;
//    case 12: Swell.Height=h2*Swell.Length/hW; break;
//    case 17: Surge.Height=h3*Surge.Length/hW; break;
    }
    if( H1!=h1 )Wind.Height=h1*Wind.Length/hW;     // Wind .axiZ( _Pi-g1*_dR );
    if( H2!=h2 )Swell.Height=h2*Swell.Length/hW;   // Swell.axiZ( _Pi-g2*_dR );
    if( H3!=h3 )Surge.Height=h3*Surge.Length/hW;   // Surge.axiZ( _Pi-g3*_dR );
    if(  Wind.Height>Wind.Length/hW  ){ h1=1.0;  Wind.Height=Wind.Length/hW; }
    if( Swell.Height>Swell.Length/hW ){ h2=1.0; Swell.Height=Swell.Length/hW; }
    if( Surge.Height>Surge.Length/hW ){ h3=1.0; Surge.Height=Surge.Length/hW; }
     Wind.Initial( Wind.Length, hW*Wind.Height/Wind.Length,g1 );
    Swell.Initial( Swell.Length,hW*Swell.Height/Swell.Length,g2 );
    Surge.Initial( Surge.Length,hW*Surge.Height/Surge.Length,g3 );
    if( Exp.wave!=1 )Original( false );
    Vessel->DampInit();                     // на случай изменения шага времени
  } while( ans!=_Esc ); logWave();
  return *this;
}
//!    Конфигурационные и протокольные записи (*.vil vessel-initiation+logging)
//     считывание угловых величин в румбах навигационного компаса (розы ветров)
//
static char* RtoD( char *s, Real &D )
{ int i=0; const char *M[]={"N","O","S","W"};
  while( *s && *s<=' ' )++s; while( s[i]>='@' || s[i]=='"' )++i; s[i++]=0;
  if( i>1 )for( int k=0; k<32; k++ ) // поиск буквосочетания в выделенном слове
    if( !strcmp( Rmbs[k],s ) || ( !strcmp( M[k/8],s ) && !(k%8) ) )
      { D=Real( k )*11.25; return s+i; } return 0;   // здесь ноль - как ошибка
}
//     выборка метрического множителя для меры длины [см, дм или м]
//
static char *AtoM( char *s, Real &M )
{ int l=0; while( *s<=' ' )if( !*s )return 0; else ++s;
  if( !memcmp( s,"см",l=strlen("см") ) )M/=100.0; else // сантиметры
  if( !memcmp( s,"дм",l=strlen("дм") ) )M/=10.0; else // дециметры
  if(  memcmp( s, "м",l=strlen("м") ) )return 0; return s+l;
}
//     уточнение относительной величины в процентах или в промиллях
//
static char *AtoA( char *s, Real &A )
{ int l=0; while( *s<=' ' )if( !*s )return 0; else ++s;
  if( !memcmp( s,"‰",l=strlen("‰") ) )A/=1000.0; else        // промилли
  if( *s=='%' ){ l=1; A/=100.0; } else return 0; return s+l; // проценты
}
//   предустановка посадки с назначением курса и заданной скорости хода корабля
//
Hull& Hull::Get( char *s )
{ char *z; Real W,D=-1;           // числовые величины разделяются запятыми
  if( z=strchr( s,',' ) )*z++=0;  //! ограничение поля новой числовой величины
  if( strcut( s ) )               //  изначальный курс корабля в румбах или °'"
  { if( strpbrk( s,"NOSW" ) ){ RtoD( s,Course ); Course*=_dR; }
                        else { AtoD( s,Course ); Course*=_dR; } }
  if( z && *z )                   //! заданная скорость хода: Fn, узлы или м/с
  { if( z=strchr( s=z,',' ) )*z++=0;
    if( strcut( s ) )             // выборка величины назначаемой скорости хода
    { cSp=strtod( s,&s ); while( *s && *s<=' ' )++s;  // очистка левых пробелов
      W = sqrt( _g*Length );        // Фрудовский множитель к масштабу скорости
      if( !memcmp( s,"узл",strlen("узл") ) )cSp*=_Mile/3600.0; else  // в узлах
      if( memcmp( s,"м/с",strlen("м/с") ) ){ AtoA( s,cSp ); cSp*=W; } // и Фруд
      if( cSp/W>0.8 )cSp=0.8*W;                                 // cSp => Speed
    }
    if( z && *z )                   //! изменение или переназначение осадки
    { if( z=strchr( s=z,',' ) )*z++=0;
      if( strcut( s ) )             // изменение осадки или её переназначение
      { bool c=strpbrk( s,"+-" )!=0; D=strtod( s,&s ); AtoM( s,D );
        if( c )D+=Draught;          // отсрочка пересчитывания из-за getString
      }
      if( z && *z )            //! установка дифферента по динамической статике
      { if( z=strchr( s=z,',' ) )*z++=0;
        if( strcut( s ) )
        { if( strstr( s,"°" ) || strpbrk( s,"\"'") )
          { AtoD( s,Trim ); Trim*=_dR; } else       // сходу в угловой мере или
          { Trim=strtod( s,&s );                    // по окончанию в остатках
            if( !AtoM( s,Trim ) )                   // с дифферентом в метрах
            { AtoA( s,Trim ); Trim*=Draught;        // или относительно осадки
            } Trim=asin( 2.0*Trim/Length );         // к полудлине корпуса
          }
          if( sin( fabs( Trim )*Length/2.0>Draught ) )       // ограничение
            Trim=asin( 2*copysign( Draught,Trim )/Length );  // макс.дифферента
        }
        if( z && *z )  // считывание начальной/исходной метацентрической высоты
        { if( z=strchr( s=z,',' ) )*z++=0;
          if( strcut( s ) ){ hX=strtod( s,&s );
            if( wcspbrk( U2W( s ),L"м") )AtoM( s,hX ); // в абсолютных метрах
            else { AtoA( s,hX ); hX*=Breadth; }      // или относительно ширины
          }
          if( z && *z )        //! выбор модели штормовой гидромеханики корабля
          if( strcut( s=z ) )Statum=byte( minmax( 0,atoi( s ),4 ) );
      } }
      if( D>0 && D!=Draught )     // изменение конструктивной осадки парохода
      if( !Read( FileName,D ) )Break( "Ошибка смены осадки: %s",FileName );
  } } return Initial();
}
//    выборка факторов демпфирования поступательных и угловых колебаний корпуса
//
Hull& Hull::GetDam( char *s )
{ char *z;                      // тройки числовых величины разделяются запятой
  if( z=strchr( s,',' ) )*z++=0; //!   ограничение поля новой числовой величины
  sscanf( s,"%lg%lg%lg",&muF.x,&muF.y,&muF.z );      // пустой строке числе нет
  if( z )
  { if( z=strchr( s=z,',' ) )*z++=0; // к фактору компенсации парадокса Даламбера
    sscanf( s,"%lg%lg%lg",&muM.x,&muM.y,&muM.z );
    if( z ){ sscanf( s=z,"%lg",&Kv ); Kv=minmax( 0.0,Kv,1.0 ); }
  } DampInit(); return *this;
}
Hull& Hull::GetExp( char *s )  // выборка ключевых слов с произвольным порядком
{ bool All=false; Educt=0;     // и остаётся только то, что выбирается из файла
  if( strstr( s,"больш"))Educt|=0x200;  // протокол только наи<больш>их величин
  if( strstr( s,"всё" ) ){ Educt|=0xFF; All=true; } // <всё> станет исключением
  if( strstr( s,"ход" ) )Educt=All? Educt&~1 : Educt|1; // скорость <ход>а
  if( strstr( s,"рыск") )Educt=All? Educt&~2 : Educt|2; // <рыск>ание на курсе
  if( strstr( s,"верт") )Educt=All? Educt&~4 : Educt|4; // <верт>икальная качка
  if( strstr( s,"борт") )Educt=All? Educt&~8 : Educt|8; // <борт>овая качка
  if( strstr( s,"кил" ) )Educt=All? Educt&~16: Educt|16;// <кил>евая качка
  if( strstr( s,"корм") )Educt=All? Educt&~32: Educt|32; // ускорения в <корм>е
  if( strstr( s,"мид" ) )Educt=All? Educt&~64: Educt|64;  // --//-- на <мид>еле
  if( strstr( s,"нос" ) )Educt=All? Educt&~128:Educt|128;  // в <нос>овой части
 char *z=strpbrk( s,"\"'" );
  if( z )
  { Real c = z[0]=='\''?60:1;
    while( z>s && z[-1]==' ' )--z;
    while( z>s && isdigit( z[-1] ) )--z;
    if( isdigit( z[0] ) )sT=c*strtod( z,&z ); if( sT<12 )sT=12;        // 1 час
  } return *this;
}
//     настройка вычислительной акватории и условий генерации штормовых волн
//
Field& Field::Get( char *s )
{ char *z; Real &VL=Vessel->Length;
  if( z=strchr( s,',' ) )*z++=0;    // ограничение поля новой числовой величины
  if( strcut( s ) )        //! уточнение размерений числовой опытовой акватории
  { Real L=strtod( s,&s );
    Real W=strtod( s,&s );
    if( AtoM( s,L ) ){ s=AtoM( s,W ); if( L>0 )Long=L; if( W>0 )Wide=W; } else
      { AtoA( s,L ); if( L>0 )Long=L*VL;          // вариант размеров бассейна
        AtoA( s,W ); if( W>0 )Wide=W*VL;          // относительно длины корабля
    } if( Long<VL*4 )Long=VL*4;                   // контроль слишком малых
      if( Wide<VL*3 )Wide=VL*3;                   // размеров акватории
  }
  if( z && *z )                   //! выбор режимов генерации морского волнения
  { if( z=strchr( s=z,',' ) )*z++=0;
    if( strcut( s ) )
    { int i=atoi( s );  if( i>=0&&i<=3 )Exp.wave=byte(i);
      if( z && *z )//! уточнение к простым полным потокам или в трохоидальности
      if( strcut( s=z ) )
      { int i=atoi( s ); if( i>=0&&i<=4 )Exp.peak=byte(i);
    } }
  } return *this;
}
//    переопределение характеристик групповой структуры штормового волнения
//
Waves&
Waves::Get( char *s, Real &L, Real &H, Real &D )  // характеристики пакета волн
{ /* «Wave», «Swell», «Surge» или «Волна», «Зыбь» и «Вал» – исходные величины:
   [Δ%‰|м|'"] длина относительно корпуса корабля[Δ], в метрах[м] или период во времени['"]
   [Δ|%‰|м] высота относительно обрушающегося гребня[Δ], в процентах от осадки корпуса,
            или собственно в метрах в отношении удвоенного трохоидального радиуса [м]
   [°|☼] направление распространения пакета волн в градусах[°] или румбах [☼] компаса
   ≡⇒0   начальный отсчет номера/фазы трохоидальной волны: (+)-в ожидание (-)-пропуск,
       – отсчёт гребня волны от обнулённого начала групповой структуры. */
 char *z;                         // числовые величины разделяются запятыми
  if( *s++ != ':' )return *this;  // небольшой дополнительный контроль ключа":"
  if( z=strchr( s,',' ) )*z++=0;  //! ограничение поля новой числовой величины
  if( strcut( s ) )               // волнение может определяться в периодах °'"
  { if( wcspbrk( U2W(s),L"°'\"") ){ s=AtoD( s,L ); L*=3600; L*=_g*L/_Pd; } else
    { L=strtod( s,&s );
      if( !AtoM( s,L ) ){ AtoA( s,L ); L*=Vessel->Length; }
  } }                             // длина волны в метрах \ долях длины корабля
  if( z && *z )                   //! интенсивность или высота волны в метрах
  { if( z=strchr( s=z,',' ) )*z++=0; // относительно обрушающегося гребня
    if( strcut( s ) )               // интенсивность (высота) морского волнения
    { H=strtod( s,&s );
      if( AtoM( s,H ) )H*=hW/L; else        // высота волны в метрах, дм или см
      if( AtoA( s,H ) )H*=Vessel->Draught*hW/L;   // проценты-промили от осадки
      if( H>=1 )H=.999; // иначе это сразу доли крутизны волнового гребня [0-1[
    }
    if( z && *z )                  //! направление распространения гребней волн
    { if( z=strchr( s=z,',' ) )*z++=0;     // -- указывается курсом из картушки
      while( *s && *s<=' ' )s++;      // приведение к первому значимому символу
      if( strcut( s ) )   // географическое навигационное измерение в градусах°
      { if( strpbrk( s,"NOSW" ) )s=RtoD( s,D ); else s=AtoD( s,D ); // и румбах
      }
      if( z && *z ) // номер начальной фазы первого вступления волнового фронта
      { if( strcut( s=z ) )Phase=strtod( s,&s );
      }
    }
  } return *this;
}
//  уточнение коэффициентов углового и поступательного демпфирования
//
void Hull::DampInit()
{ nM=muM*(0.5*Ts); /** &/inMass */ nM.x = (1.0-exp( -nM.x ))/nM.x;
                                   nM.y = (1.0-exp( -nM.y ))/nM.y;
                                   nM.z = (1.0-exp( -nM.z ))/nM.z;
  nF=muF*(0.5*Ts); /** &/Volume */ nF.x = (1.0-exp( -nF.x ))/nF.x;
                                   nF.y = (1.0-exp( -nF.y ))/nF.y;
                                   nF.z = (1.0-exp( -nF.z ))/nF.z;
}
