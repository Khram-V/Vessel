//
//                                                     /май.2003/  85.04.02
//  Расчет волнового сопротивления условно тонкого судна по теории Мичелла.
//
//                         ++ ЛКИ: Александр Николаевич Шебалов
//                         ≈≈      (Владимир Атанасов\Йорданов)
//                         ++      Ада Шоломовна Готман
#include "Hull.h"

const int  Nr = 91;        // Количество отсчетов на кривой сопротивления
const Real mFn=0.8,        // Предельно большое число Фруда в циклах расчетов
           dFn=(mFn-0.1)/(Nr-1), // шаг к расчётному количеству скоростей Фруда
           Ro=1.025;             //  кг/дм³ = тонн/м³

static Window *mcWin=0;    // Графическое окно для вывода текстовых результатов
static Plane *mHull=0,     // Окно проекции корпус
             *mLine=0,     // Ватерлинии
             *mResw=0,     // Кривые волнового сопротивления
             *mWave=0;     // Интенсивности излучения корабельных волн
static Plaze *PL=NULL;     // корпус корабля для расчётов волнообразования
static Place *MPL=0;       // окошечко с подписью для мышки
static int   W=1280,H=600, // Исходный размер графического окна
             Type=0;       //  0 - мой Michel; 1 - старый вариант Шебалова
                           //  2 - Ада Шоломовна Готман - наилучший алгоритм
                           // -1 - расчет занесен в числовые массивы
static int  Nb=0;          // количество графиков
static Real Hb=1e-6,       // и масштаб для корабельного волнообразования
         **BorWave=NULL;   // 0-чисто; (-)запрос расчета; (+)только картинка
                           // собственно график зарождения волны вдоль борта
void Ada_Sholomovna_Gotman
(     Real **Hull,  // Корпус
     _Real Froud,   // Число Фруда
     _Real Length,  // Длина между перпендикулярами                      [м]
     _Real Breadth, // Ширина (на всякий случай)                         [м]
     _Real Depth,   // Осадка (под дробление по ватерлиниям)             [м]
     _Real Volume,  // Водоизмещение                                    [м³]
     _Real Surface, // Смоченная поверхность корпуса                    [м²]
 const int Nx,      // Индексы размерности корпуса Hull по длине
 const int Nz,      //  и по осадке
     Real &RBasic,  // коэффициенты: главной части волнового сопротивления
     Real &RMichell, // ++ к собственно расчету на основе интеграла Мичелла
     Real &Residual  // ++ остаточное сопротивление с турбулентной вязкостью
);
Real WaveRes( Real **Hull,    // Корпус
              Real Froud,     // Число Фруда
              Real Length,    // Длина
              Real Breadth,   // Ширина
              Real Depth,     // Осадка
              Real Surface,   // Смоченная поверхность корпуса
               int Nx,        // Индексы по длине
               int Nz         //  и по осадке
            );
#define WaveResistance(_W )( _W/V ) // сопротивление относительно водоизмещения
#define WaveCoefficient(_W )( _W*2.0/Lwl/Fn/Fn/Ro/_g/S ) // коэффициент: Cw
//
//     Прорисовка расчетного корпуса
//         основанного на уточненной/расчетной таблице плазовых ординат
//
void Plaze::Drawing()
{ int i,k;
  mcWin->Activate().Clear();     //. теперь и мышка уж умеет сбивать активность
  //
  //! сначала верхнее окошко с волновым профилем
  //
  for( i=0; i<Nb; i++ )
  { Real &VelWave=BorWave[i][0];     // две первые точки в пропусках
    if( VelWave<0 )
    { Michell( VelWave=-VelWave,0 ); memcpy( BorWave[i]+1,Wx,Nx*sizeof(Real) );
      for( k=3; k<=Nx; k++ )if( Hb<BorWave[i][k] )Hb=BorWave[i][k];
  } }
  //! первым формируется и сохраняется окошко волнообразования
  //
  glColor4f( 0.85,1,0.9,1 );
  mWave->Set( 0,0,1,Hb*1.05 ).Clear( 0 );
  gl_BLUE,mWave->Print( 0,2,"%s \n%s ",mWave->iD,abs( Type )<2
    ?"Текущие проработки по книге «История штормовой мореходности»":abs(Type)<3
    ?"Академический алгоритм Ленинградской Корабелки по А.Н.Шебалову"
    :"Поверенные математические модели и алгоритмы Ады Шоломовны Готман" );
  gl_LIGHTRED; mWave->Print( 0,4,"hW[%d] = %.4g  ",Nb,Hb );
  for( i=0; i<Nb; i++ )
  { Real &VelWave=BorWave[i][0];          // пусть графики идут поверх надписей
    if( VelWave>0 )
    { gl_GREEN; glBegin( GL_LINE_STRIP );
      for( k=3; k<=Nx; k++ )glVertex2d( Real( k )/Nx,BorWave[i][k] ); glEnd();
    } gl_BLUE;
    Real X=mWave->wX( mResw->x( VelWave ) ),
         Y=BorWave[i][int(X*Nx+0.5)];
    mWave->Text( _North,X,Y,0.0,"%.2g",VelWave );
  }
  //! проекция полуширота теоретического чертежа корпуса корабля
  //
  glColor3f( 0.9,1,0.95 );                // Проекция полуширота для информации
  mLine->Set( 0.0,0.0,xS-xA,Breadth/2 ).Clear( false );
  gl_CYAN,mLine->Print( 0,1,"%s \nV=%1.0f \nS=%1.0f ",mLine->iD,V,S );
  for( k=0; k<Nz; k++ )                   // .. прорисовка расчётных ватерлиний
  { glBegin( GL_LINE_STRIP );             // ... в естественном масштабе
    for( i=0; i<Nx; i++)glVertex2d(i*dX,Y[k][i]); // по длине графического окна
    glEnd();
  }
  //! обводы в проекции корпус теоретического чертежа корабля
  //
  glColor3f( 0.9,1,0.95 );
  mHull->Set( -0.5*Breadth,Do,Breadth/2,Draught ).Clear(0); //.Show();
  for( i=0; i<Nx; i++ )                      // проекция корпус или - шпангоуты
  { if( !(i%(Nx-1)) )gl_RED; else gl_CYAN;   // Построение картинки с корпусом
    glBegin( GL_LINE_STRIP );                // .. интерполированные расчётные
    for( k=0; k<Nz; k++ )                    // ... теоретические шпангоуты
     glVertex2d( i>Nx/2?Y[k][i]:-Y[k][i],Do+Real( k )*dZ );
    glEnd();
  }
  //!     Здесь начинается подготовка собственно результатов расчета
  //!          волнового и остаточного сопротивления по Мичеллу
  //
 static Real Fn,W,      //
        Rv[Nr],mv=0.0,  // коэффициенты: главной части волнового сопротивления
        Rm[Nr],mm=0.0,  // ++ к собственно расчету на основе интеграла Мичелла
        Rr[Nr],mr=0.0;  // ++ остаточное сопротивление с турбулентной вязкостью
  mcWin->Save().Show(); //   сбрасываем картинки на экран и начинаем вычисления
  mResw->Set( 0.098,0.0,mFn,1.04 );      // экстремумов на кривой сопротивления
                            // -- как бы должны вписываться в промежуток [0..1]
  if( Type>0 )              // Type<0 - настоящие вычисления проведены ранее
  { mv=mm=mr=0.0;           // очистка признаков наличия результатов вычислений
    for( Fn=0.1,i=0; i<Nr; i++,Fn+=dFn ) //! Расчеты волнового сопротивления
    { switch( Type )
      { case 1: W=Michell( Fn ); break;   // контекстные вычисления и -внешние:
        case 2: W=WaveRes( Y,Fn,Lwl,Bwl,Draught,Surface,Nx,Nz ); break;
        case 3: Ada_Sholomovna_Gotman     // Ада Шоломовна Готман - Новосибирск
              ( Y,Fn,Lwl,Bwl,Draught,Volume,Surface,Nx,Nz,Rv[i],Rm[i],Rr[i] );
      }
      if( Type<3 )
      { if( (Rv[i]=WaveResistance ( W ) )>mv )mv=Rv[i];
        if( (Rm[i]=WaveCoefficient( W ) )>mm )mm=Rm[i];
      } else if( Type==3 )
      { if( Fn>0.85 )Rv[i]=0.0; else
         if( Rv[i]>mv )mv=Rv[i];// коэффициент главной части интегралла Мичелла
        if( Fn>0.72 )Rm[i]=0.0; else
         if( Rm[i]>mm )mm=Rm[i];  // собственно расчет сопротивления по Мичеллу
        if( Rr[i]>mr )mr=Rr[i];   // остаточное сопротивление с турбулентностью
      }                           // ,R[i]=W/(2.0*V/Fn/Fn/Surface/Ro/g/Length);
      if( !(i%(Nr/12)) )
      { glColor3f( 0.9,0.9,0.9 );     // серые отметки процессов вычислений
        glRectd( Fn-mFn/Nr,0.25,Fn,0.75 ); mResw->Show(); //,mcWin->ScanStatus();
      }
      if( !mcWin->Ready() )return;    // если в длинных расчетах закроется окно
    } Type=-Type;                     //! блокировка повторных перерасчётов
  }
  //! ...и варианты прорисовки кривых волнового и остаточного сопротивления
  //
  glColor4f( 0.98,1,1,1 );
  mResw->Clear( false );              // графики кривых волнового сопротивления
  glBegin( GL_LINES ); gl_CYAN;       //     масштабная шкала (временная схема)
  for( Fn=0.1; Fn<mFn; Fn+=0.1 )glVertex2d( Fn,0  ),glVertex2d( Fn,.0625 );
  gl_BLUE;                     glVertex2d( 0.5,0  ),glVertex2d( .5,.125 );
  for( Fn=0.1,i=1; i<Nr; i++,Fn+=dFn ) // Расчет кривых волнового сопротивления
  { if(mv&&Rv[i])gl_MAGENTA,glVertex2d(Fn,Rv[i-1]/mv),glVertex2d(Fn+dFn,Rv[i]/mv);
    if(mm&&Rm[i])gl_GREEN,  glVertex2d(Fn,Rm[i-1]/mm),glVertex2d(Fn+dFn,Rm[i]/mm);
    if(mr&&Rr[i])gl_BLUE,   glVertex2d(Fn,Rr[i-1]/mr),glVertex2d(Fn+dFn,Rr[i]/mr);
  } glEnd();
  for( i=0; i<Nb; i++ )
  { Real &VelWave=BorWave[i][0];
    if( VelWave>0 )                    // отметка Фруда для волны вдоль корпуса
    { gl_LIGHTGREEN; glBegin( GL_LINES );
      glVertex2d( VelWave,0 ),glVertex2d( VelWave,1 ); glEnd();
  } }
  //
  //   надписи на графическом поле кривых волнового сопротивления
  //
  gl_BLUE;
  for( Fn=0.1; Fn<mFn; Fn+=0.1 )
  mcWin->Text( Fn<0.15?_South_East:_South_West,Fn,0,0,"%3.1f",Fn );
  mResw->Print(-1,0,Type==-3?"Ада Готман":Type==-2?"А.Н.Шебалов"
                                         :"Штормовая мореходность");
  if( Type>-3 )gl_MAGENTA,mResw->Print( 2,1,"R/D=%0.3g Н/кг",mv ),
               gl_GREEN,  mResw->Print(  "\n Cw =%0.3g",mm );
      else     gl_MAGENTA,mResw->Print( 2,1,"%0.3g главная часть ",mv ),
               gl_GREEN,  mResw->Print(  "\n %0.3g Мичелл ",mm ),
               gl_BLUE,   mResw->Print(  "\n %0.3g остаточное ",mr );
  mcWin->Save().Refresh(); // наверняка можно сработать только над окном Window
}
///   Информационные процедуры для активного окна
///
static bool Mouse_in_Window( int x, int y )
{ if( ( Active&3 )!=3 || !mcWin->Ready() )return false;
 Plane *M; mcWin->Activate().Rest();     // восстановление прошлого изображения
  if( mLine->Is( x,y ) )M=mLine; else
  if( mResw->Is( x,y ) )M=mResw; else
  if( mHull->Is( x,y ) )M=mHull; else
  if( mWave->Is( x,y ) )M=mWave; else M=NULL;
  if( M==mResw )  // неплохо бы сделать статический контроль излишних зарисовок
  { Plaze &P=*PL; Real X=M->wX( x ),H; int k; P.Michell( X,false );
    if( !Nb )        // на чистом поле нужны экстремумы и новое масштабирование
    { for( H=1e-6,k=2; k<P.Nx; k++ )if( H<P.Wx[k] )H=P.Wx[k];
      mWave->Set( 0,0,1,H*1.05 );        // формируется окошко волнообразования
    } else mWave->Focus();
    glColor3f(.5,.5,1); glBegin( GL_LINE_STRIP );
    for( k=2; k<P.Nx; k++ )glVertex2d( Real( k )/P.Nx,P.Wx[k] ); glEnd();
    H=mWave->wX( mResw->x( X ) );
    mWave->Text( _North,H,P.Wx[int(H*P.Nx+0.5)],0.0,"%.2g",X ).Show();
    M->Focus();
    glBegin( GL_LINES ); glVertex2d( X,0.0 ),glVertex2d( X,1.0 ); glEnd();
    M->Show();
  }
  if( M ){ gl_LIGHTRED; MPL->Clear().Print( 0,1,"%s  -  %s %.2f, %s %.2f ",
                             M->iD,M->sX,M->wX(x),M->sZ,M->wZ(y) ).Show(); }
  return false;
}
static bool Mouse_Click( int State, int x, int y )
{ if( State==_MouseLeft )               //
  if( mResw->Is( x,y ) )                // мышка может сыпать двойными кликами
  { Real Fn=mResw->wX( x );             //
    for( int i=0;i<Nb;i++ )if( fabs(Fn-fabs(BorWave[i][0]))<dFn )return false;
    BorWave=(Real**)Allocate( Nb+1,(PL->Nx+1)*sizeof( Real),BorWave );
    BorWave[Nb++][0] = -Fn; PL->Drawing();
  } return false;
}
static bool FullDraw()
{ if( mcWin->Ready() )
  { mcWin->Activate();
    H=mcWin->Height;             // полный пересчет местоположения страничек на
    W=mcWin->Width;              // случай изменения размеров графического окна
    mResw->ax=mWave->ax=mLine->ax=0;
    mHull->ux=mWave->ux=mLine->ux=W;
    mHull->ax=mResw->ux=(2*W)/3; // -- здесь настройка размерений всех картинок
    mHull->az=mResw->az=24;
    mHull->uz=mResw->uz=mLine->az=mHull->az+(W-mHull->ax)*(Draught-Do)/Breadth;
    mLine->uz=mWave->az=mHull->uz+(W*Breadth)/Lmx/2;
    mWave->uz=H;
    MPL->Area( 0,1,51,-22 );       // информационная подстрочка
    PL->Drawing();
  } return false;
}
static void HelpWave()
{ const char *Name[]={ " Wave ","  Волнообразование и",
                                  "сопротивление движению",
                                  "корабля на тихой воде",0 },
             *Cmds[]={ "F1 ","     краткая справка",
                       "F7 ","по \"Штормовой мореходности\"",
                       "F8 ","из Корабелки + А.Н.Шебалов",
                       "F9 ","++ остаточное от А.Ш.Готман",0 },
             *Plus[]={ "(LMouse)  ","+ волновое излучение",
                       "<Space>   ","  сброс профилей волн",
                       "<Enter>   ","обновить изображение",
                       "<Tab>     ","  иная модель Мичелла",
                       "<Esc>/<ctrlC> ","   завершение",0
                     }; mcWin->Help( Name,Cmds,Plus,2 );
}
void Hull_Wave( const int _Type )
{ int ans;
 Type=_Type;
 Window Win(" Simple Wave Resistance of ship",0,0,W,H ); mcWin=&Win;  // единое
 Plane Hull("Корпус",    "Y","Z",mcWin ); mHull=&Hull;  // окно проекции корпус
 Plane Line("Полуширота","X","Y",mcWin ); mLine=&Line;  // проекция полуширота
 Plane Resw("Волновое сопротивление","Fn","Wr",mcWin ); mResw=&Resw; // Мичелл
 Plane Wave("Интенсивность излучения","X","Aw",mcWin ); mWave=&Wave; // и волны
 Place Mous( mcWin,PlaceAbove ); MPL=&Mous;    // красная строчка с информацией
 Plaze Ship(64,96,Draught);PL=&Ship;// Рабочая таблица плазовых ординат корпуса
                                    //  для простых или ускоренных перерасчётов
//V=PL.V/( Lwl*Draught*Bwl );                      == коэффициент общей полноты
  glClearColor( 0.95,1,0.9,1 );
  glInitial();
  Win.Activate().Alfabet( 18,"Times" )    // привязанная к текущему окну
     .Mouse( Mouse_in_Window )            // графическое окно волнообразования
     .Mouse( Mouse_Click )
     .Clear();
  Win.Draw( FullDraw ).Draw();            // обновление при вызовах из системы
MainLoop:   FullDraw();                   // единая процедура полной прорисовки
  switch( ans=Win.WaitKey() )             // остановка всех операций в ожидании
  { case _Esc: Win.Close(); break;        // откликов с клавиатуры
    case _F1 : HelpWave();  break;
    case _F7 : Type=1;      break;
    case _F8 : Type=2;      break;
    case _F9 : Type=3;      break;
    case _Tab: Type=1+(abs(Type)+1)%3; break; // перебор мат.моделей по Мичеллу
    case _Blank: Hb=Nb=0; break;
    case _Enter: Win.Clear();              // переразметка графических площадок
  }
  if( Win.Ready() )goto MainLoop;          // выход из процедуры только здесь
  W = Win.Width;                           // сохраняются размеры экрана
  H = Win.Height; Hb=Nb=0;                 // для последующих обращений
}
