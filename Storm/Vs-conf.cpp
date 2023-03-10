//
//      Настройки вычислительного эксперимента
//              с визуализацией вычислительных процессов
//
//                                                   2019-02-07 Старый Петергоф
//
#include "Vessel.h"
/* Statum: Выбор и установка моделей гидромеханики корабля в штормовом море
              --   0 - кинематическое позиционирование(не скольжение) корпуса
              --       над волнением по хордам четырёх точек склона, иначе:
              --   1 - объёмное гравитационное всплытие: борт-киль-вертикаль
              --   2 - гидростатическое давление в волне на смоченной обшивке
              --   3 ÷ ≈ вихреисточники обтекания корпуса набегающим потоком
              --   4 - компоненты скорости на обшивке под волновыми гребнями */
const char
 *Model[]={ "Кинематика качки на 4-х точках по склону волн",   // ⌠
            "Объёмная плавучесть переменного водоизмещения",   // │
            "Давление смоченных элементов бортовой обшивки",   // │
            "Давление на встречном потоке без внешних волн",   // │
            "Давление с волновыми и ходовыми потоками воды" }; // ⌡
byte
Model_Config( byte Statum, Window* Win )
{ byte ans=Statum;
  Mlist Menu[]={ {1,0,"   Выбор модели гидромеханики корабля"},{2,45,Model[0]},
             {1,45,Model[1]},{1,45,Model[2]},{1,45,Model[3]},{1,45,Model[4]} };
  TextMenu T( Mlist( Menu ),Win,47,1 );
  do{ if( (ans=T.Answer( ans+1 )-1)==_Esc-1 )ans=Statum; }while( ans>4 );
  return ans;
}
/*     объекты и производные операции с корпусом на волнении
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
  Mlist Menu[]={ { 1,0,"   Гидростатика и динамика корпуса" }
        , { 2,34 }           // признаки вариантов прорисовки корпуса
        , { 1,34 }           // закраска или контуры триангуляционного покрытия
        , { 1,34 }           // варианты гидромеханики корабля и морских волн
        , { 1,4,"Метацентрическая высота:   %4.1lf",&hX },{ 0,0," м" }
        , { 1,4,"Демпфирование-X %4.2lf",&DampF.x},{0,4," Y %4.2lf",&DampF.y }
                                                  ,{0,4," Z %4.2lf",&DampF.z }
        , { 1,4," Борт %4.2lf",&DampM.x }, { 0,4," Киль %4.2lf",    &DampM.y }
                                         , { 0,4," Рыскание %4.2lf",&DampM.z }
        , { 1,4,"Практическая осадка:    %5.2lf",&D },    { 0,0," м" }
        , { 1,5,"Кинематические графики  %5.2lf",&sTime },{ 0,0," мин" }
        };
  TextMenu T( Mlist(Menu),this,1,1 ); int ans=-1; Real h=hX;
  do
  { Menu[1].Msg=(char*)HView[DrawMode&3]; // выбор метода прорисовки корпуса
    Menu[2].Msg=DrawMode&4 ? "Контуры рёбер триангуляционного покрытия"
                           : "Закрашиваемая поверхность корабельной обшивки";
    Menu[3].Msg=Model[Statum];
    if( (ans=T.Answer( ans ))==2 )DrawMode^=4; else
    if( ans==1 )DrawMode=(DrawMode&~3)|((DrawMode-1)&3); else
    if( ans==3 )Statum=Model_Config( Statum,this );
    if( h!=hX )// сдвиг центра масс и пересчет тензора инерции под новую высоту
    { Vector G=Gravity; Gravity.z+=h-hX; h=hX; inMass=Steiner(inMass,G,Gravity);
    }
    if( D!=Draught )                  // предварительная очистка волнового поля
    { if( !Read( FileName,D ) )       // главные оси и геометрические параметры
           Break( "Ошибка повторного чтения корпуса %s",FileName );
     int sKt=Storm->Kt; Storm->Original( false ).Kt=0;
      mM=0.0; Initial().Floating().wPrint( true ); Storm->Kt=sKt;
    }
    sT=max( 0.5,sTime )*60; sTime=sT/60.0;
  } while( ans!=_Esc ); return *this;
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
    { case  1: (++Exp.wave)%=3; break;         // Initial() модель волнения
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
  } while( ans!=_Esc ); waveLog(); return *this;
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
{ int l=0; while( *s && *s<=' ' )++s;
  if( !memcmp( s,"см",l=strlen("см") ) )M/=100.0; else // сантиметры
  if( !memcmp( s,"дм",l=strlen("дм") ) )M/=10.0; else // дециметры
  if(  memcmp( s, "м",l=strlen("м") ) )return 0; return s+l;
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
      if( !memcmp( s,"узл",strlen("узл") ) )cSp*=_Mile/3600.0; else
      if( memcmp( s,"м/с",strlen("м/с") ) )cSp*=W;             // cSp => Speed
      if( cSp/W>0.8 )cSp=0.8*W;
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
        { if( strpbrk( s,"°'\"") ){ AtoD( s,Trim ); Trim*=_dR; } else
          { Trim=strtod( s,&s ); AtoM( s,Trim ); Trim=asin(-2*Trim/Length ); }
          if( sin( fabs( Trim )*Length/2.0>Draught ) )       // ограничение
            Trim=asin( 2*copysign( Draught,Trim )/Length );  // макс.дифферента
        }
        if( z && *z )          //! выбор модели штормовой гидромеханики корабля
        if( strcut( s=z ) )Statum=byte( minmax( 0,atoi( s ),4 ) );
      }
      if( D>0 && D!=Draught )     // изменение конструктивной осадки парохода
      if( !Read( FileName,D ) )Break( "Ошибка смены осадки: %s",FileName );
  } } return *this;
}
//    настройка вычислительной акватории и условий генерации штормовых волн
//
Field& Field::Get( char *s )
{ char *z; Real &VL=Vessel->Length;
  if( z=strchr( s,',' ) )*z++=0;   // ограничение поля новой числовой величины
  if( strcut( s ) )        //! уточнение размерений числовой опытовой акватории
  { Real L=strtod( s,&s );
    Real W=strtod( s,&s );
    if( AtoM( s,L ) ){ s=AtoM( s,W ); if( L>0 )Long=L; if( W>0 )Wide=W; } else
    { if( L>0 )Long=L*VL;                         // вариант размеров бассейна
      if( W>0 )Wide=W*VL;                         // относительно длины корабля
    } if( Long<VL*4 )Long=VL*4;                   // контроль слишком малых
      if( Wide<VL*3 )Wide=VL*3;                   // размеров акватории
  }
  if( z && *z )                   //! выбор режимов генерации морского волнения
  { if( z=strchr( s=z,',' ) )*z++=0;
    if( strcut( s ) )
    { int i=atoi( s );  if( i>=0&&i<=3 )Exp.wave=byte(i);
      if( z && *z )  //! уточнение к простым полным потокам или трохоидальности
      if( strcut( s=z ) ){ int i=atoi( s ); if( i>=0&&i<=4 )Exp.peak=byte(i); }
  } } return *this;
}
//    переопределение характеристик групповой структуры штормового волнения
//
Waves&
Waves::Get( char *s, Real &L, Real &H, Real &D )  // характеристики пакета волн
{  /* «Wave», «Swell», «Surge» или «Волна», «Зыбь» и «Вал» – исходные величины:
      длина [Δ∨м|m∨' "] –  в долях длины парохода; метрах или секундах периода,
      направление [☼∨°] –     в румбах или в градусах от меридиана из картушки,
      высота [Δ∨м] –       в долях от высоты обрушающегося гребня или в метрах,
      номер – отсчёт гребня волны от обнулённого начала групповой структуры. */
 char *z;                         // числовые величины разделяются запятыми
  if( *s++ != ':' )return *this;  // небольшой дополнительный контроль ключа":"
  if( z=strchr( s,',' ) )*z++=0;  //! ограничение поля новой числовой величины
  if( strcut( s ) )               // волнение может определяться в периодах °'"
  { if( wcspbrk( U2W(s),L"°'\"") ){ s=AtoD( s,L ); L*=3600; L*=_g*L/_Pd; } else
    { L=strtod( s,&s );if( !AtoM(s,L) )L*=Vessel->Length; // wcschr(U2W(s),L'м')
  } }                             // длина волны в метрах \ долях длины корабля
  if( z && *z )                   //! интенсивность или высота волны в метрах
  { if( z=strchr( s=z,',' ) )*z++=0; // относительно обрушающегося гребня
    if( strcut( s ) )               // интенсивность (высота) морского волнения
    { H=strtod( s,&s ); if( AtoM( s,H ) )H*=hW/L; if( H>1 )H=1; }
    if( z && *z )                  //! направление распространения гребней волн
    { if( z=strchr( s=z,',' ) )*z++=0;     // -- указывается курсом из картушки
      while( *s && *s<=' ' )s++;      // приведение к первому значимому символу
      if( strcut( s ) )   // географическое навигационное измерение в градусах°
      { if( strpbrk( s,"NOSW" ) )s=RtoD( s,D ); else s=AtoD( s,D ); // и румбах
      }
//    if( z && *z ){ if( strcut( s=z ) ){} } // к номеру исходного гребня волны
    }
  } return *this;
}
