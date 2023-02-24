//
//!   Контекстно-связанный интерфейс с экранными окнами для графики OpenGL
//    формальные построения и динамические переустановки текстовых страниц
//            и графических фрагментов внутри главного окна Window
//
//!   блок контекстно-зависимых (дружественных) процедур управления фрагментами
//                  возможно нужна единая ссылка к чистому выходу из прерываний
Place& Place::Activate( bool act )        // активизация графического контекста
{ wglMakeCurrent( Site->hDC,Site->hRC );          // предустановленной страницы
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
  } return *this;
}
//!    Начальная установка базового шрифта, связанного с текущим окном OpenGL
//
Place& Place::AlfaBit( unsigned char *DispCCCP )     /// старый советский растр
{ if( !Fnt )Fnt=(Font*)calloc( sizeof(Font),1 );    // ставится локальный адрес
  if( !( Fnt->Bit=DispCCCP ) )return Alfabet();    // сходу не линкуется => TTF
  Fnt->W=(Fnt->Bit)[0]+1,Fnt->H=(Fnt->Bit)[1]+2;
//wglMakeCurrent( Site->hDC,Site->hRC );
  if( Fnt->Base )glDeleteLists( Fnt->Base,ListsGroup ); Fnt->Base=0;
  if( Fnt->hF )DeleteObject( Fnt->hF ); Fnt->hF=0; return *this;
}
//!  Windows TrueType шрифт Courier New-18 устанавливается по умолчанию
//
Place& Place::Alfabet( int h, const char *fn, int weight, bool italic )
{ //wglMakeCurrent( Site->hDC,Site->hRC );
  if( !Fnt )Fnt=(Font*)calloc( sizeof(Font),1 );    // ставится локальный адрес
  if( Fnt->Base )glDeleteLists( Fnt->Base,ListsGroup ); Fnt->Base=0;
  if( Fnt->hF )DeleteObject( Fnt->hF );                 Fnt->Bit=NULL;
  LOGFONT logfont={};                             // Setup Font characteristics
   logfont.lfHeight       = h<6?Site->ScreenHeight/48:h; //  высота шрифта = 1?
// logfont.lfWidth        = 0;
   logfont.lfEscapement   = GM_COMPATIBLE;         //   GM_ADVANCED - 0
// logfont.lfOrientation  = 0;
   logfont.lfWeight       = weight;  //FW_DONTCARE+THIN,ULTRALIGHT,LIGHT,NORMAL
   logfont.lfItalic       = italic;  //    MEDIUM,SEMIBOLD,BOLD,EXTRABOLD,HEAVY
// logfont.lfUnderline    = false;
// logfont.lfStrikeOut    = false;
   logfont.lfCharSet      = RUSSIAN_CHARSET;    // DEFAULT_CHARSET ANSI_CHARSET
   logfont.lfOutPrecision = OUT_STRING_PRECIS;        // OUT_TT_PRECIS
   logfont.lfClipPrecision= CLIP_DEFAULT_PRECIS;      // OUT_STROKE_PRECIS
   logfont.lfQuality      = DEFAULT_QUALITY;          // PROOF_QUALITY
   logfont.lfPitchAndFamily=FF_MODERN|FIXED_PITCH;    // FF_ROMAN DEFAULT_PITCH
   strcpy( logfont.lfFaceName,fn?fn:"Courier New");   // имя шрифта 32 байта
   Fnt->hF = CreateFontIndirect( &logfont );            // Create the font
  HFONT oFont=(HFONT)SelectObject( Site->hDC,Fnt->hF ); // and display list
   Fnt->Base=glGenLists( ListsGroup );
   if( !wglUseFontBitmaps( Site->hDC,0,ListsGroup,Fnt->Base ) ) //! странная ??
        wglUseFontBitmaps( Site->hDC,0,ListsGroup,Fnt->Base );  //! проблема ??
// { glDeleteLists( Fnt->Base,ListsGroup==ListsGroup );
//   Fnt->Base=0; FontWidth=FontHeight=1 } else
   { TEXTMETRIC T; GetTextMetrics( Site->hDC,&T );
     Fnt->W=T.tmAveCharWidth;     // (AlfaRect( "_Ё°y" ).cx+3)/4;
     Fnt->H=logfont.lfHeight;     // T.tmAscent; // tmHeight+tmExternalLeading;
   }
   if( oFont )SelectObject( Site->hDC,oFont ); return *this;
}
//   Определение размеров изображения текст !!! необходим многострочный вариант
//
SIZE Place::AlfaRect( const char *s, bool ANSI )  // предварительно формируется
{ SIZE F={ ANSI?(long)strlen( s ):Ulen( s ),0 };  // в буфере или ANSI-строчка
  if( AF.Bit )F.cx*=AF.W,F.cy=AF.H; else
  { glListBase( AF.Base ); char *str=(char*)s;
   HFONT oFont=(HFONT)SelectObject( Site->hDC,AF.hF );
    if( !ANSI )str=UlA( strdup( s ),false );     // сброс UTF-8 в ANSI на месте
    GetTextExtentPoint32( Site->hDC,str,F.cx,&F );
    if( oFont )SelectObject( Site->hDC,oFont );
    if( !ANSI )free( str );
  } return F;
}
