//
//!  Конструкторы и процедуры сохранения и перерисовки фонового изображения
//!    - все странички всегда связаны с одним из графических окон Window,
//!    - однако отсутствие ссылки Prev - показывает,
//!      что окно исключено из всех операций со списками окон
//     ( это площадка базового окна или фантомное отображение вне Window )
//
Place::Place( Window *Win, byte Mode ): Site( Win ),Signs( Mode ),Up( NULL ),
       pX( 0 ),pY( 0 ),Width( Win->Width ),Height( Win->Height ),
       MouseState( 0 ),xo( 0 ),yo(-1 ), // беспросветная унылость мышки/курсора
       chX(0),chY(0),bX(1),  // место и отступ строки текста от боковой границы
       extDraw( NULL ),      // свободное рисование без дополнительных настроек
       extPass( NULL ),      // адреса двух внешних\свободных и бесконтрольных
       extPush( NULL ),      // процедур для параллельного контроля хода мышки
            Ft( NULL ),      // шрифтовой блок, b при нуле включается DesignCAD
           Img( NULL ),      // фоновый растр с наложенной площадкой PlaceAbove
       isDraw( false ),      //  прорисовка сбрасывается по случаю незавершения
      isMouse( false )       //     предыдущей операции (во избежание рекурсии)
{ if( this!=(Place*)Win )    // обход неприкасаемого базового окна Window.Place
  { Place *S=(Place*)Win;    // Tw=S->Tw; Th=S->Th; Bit=NULL;
    while( S->Up )S=S->Up; S->Up=this;   // новая площадка набрасывается сверху
//  if( Site->Up==this )Site->Save();    // первый фрагмент сохраняет фон окна?
    glAct( Win );            // связь контекста OpenGL с требуемым окном Window
    chY=Height-AlfaHeight(); // буквенная позиция изначально будет сверху/слева
  }
}                                     // wglMakeCurrent( Site->hDC,Site->hRC );
//
//!   блок контекстно-зависимых (дружественных) процедур управления фрагментами
//                  возможно нужна единая ссылка к чистому выходу из прерываний
Place& Place::Activate( bool act )        // активизация графического контекста
{ glAct( Site );                                  // предустановленной страницы
  glViewport( pX,pY,Width,Height );               // установка  растрового поля
 //glScissor( pX,pY,Width,Height );               // обрезка внешнего окружения
  if( act )                                       // если без внешней настройки
  { glMatrixMode(GL_PROJECTION),glLoadIdentity(); // на размер фрагмента экрана
    if( Signs&PlaceOrtho )                        // ортогональная  отметка под
    { Real r=Real( Width )/Height;                // исходное ортонормированное
      if( r>1 )glOrtho( -r,r,-1,1,-1,1 );         // масштабирование  экрана во
          else glOrtho( -1,1,-1/r,1/r,-1,1 );     // внутренний 3D-размер[-1:1]
    } else     glOrtho( 0,Width,0,Height,-1,1 );  // +отсчет растра слева/снизу
    glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // единичная  матрица  модели
  } return *this;                                 //
}
bool Place::Draw()       // в виртуальной среде Draw доступен внутренний пролог
{ if( Site && extDraw )  // настройки OpenGL с контекстным эпилогом прорисовки
  { glContext S( Site ); // подготовка среды к внешнему исполнению с рекурсией
    if( S.Active )
    if( extDraw() )      // -- исполнение внешней независимой транзакции
      { if( this==Site )Save().Refresh(); else Show(); return false; }
  } return Site!=NULL;
}
//!  Внутренние процедуры для реализации виртуальных обращений с мышкой
//!
bool Place::Mouse( int x,int y ) // виртуальная функция динамически подменяется
{ if( extPass )return extPass( x,y ); else// локальная среда внешней транзакции
  if( Site->extPass )return Site->extPass( x+pX,y-pY-Height+Site->Height );
  return false;               // без внешних транзакций считать всё завершённым
}
bool Place::Mouse( int b, int x,int y )
{ if( extPush )return extPush( b,x,y ); else
  if( Site->extPush )return Site->extPush( b,x+pX,y-pY-Height+Site->Height );
  return false;
}
Place::~Place()  // освобождение площадки, шрифтов, картинки и всех точек входа
{ if( Site )                                 // +++ средняя площадка вышибается
  { if( Img ){ free( Img ); Img=NULL; }      // все связные объекты расчищаются
    if( Ft ){ free( Ft ); Ft=NULL; }         // отключение привязанного шрифта
//  AlfaBit( 0 );                            // здесь полный destructor шрифтам
    for( Place *S=(Place*)Site;S;S=S->Up )if( S->Up==this ){ S->Up=Up; break; }
    if( Site==this )Site=NULL; else;        // возврат к моменту создания Place
    if( Signs&PlaceAbove )Site->Refresh();  // обновление после удаления Place
  }
}
