//
//   Window - минимальная подборка для мышки в окне с кнопочками
//
#include "Window.h"

Window::Window(): F( (Field){ 0,0,1,1 } ),f( (field){ 0,0,0,0,0 } )
{ _x=b.left,_y=b.top,x=b.right,y=b.bottom; Focus();// конструктор приустановкой
}
bool Window::Is( int X,int Y )
{ return X>=b.left && X<=b.right && Y<=b.bottom && Y>=b.top;
}
Field Window::Focus()
   { field t=f; Tv_place( &t );
     Field T=F; Tv_place( 0,&T ); b=Tv_port; return T;
   }
Course Window::Console( int Info( int,int,bool ) ) // Info=>0 - отключение
{ Course Ans;                                      //  true - c нажатой кнопкой
  for( ;; )                                        //  false - ожидание реакции
  {   Ans=Tv_getc();                               //   ret=-1 - редактирование
  for_Repeat:
    if( Ans<_MouseMoved || Ans>_MouseReleased )return Ans; // метка для мышки
    Tv_GetCursor( x,y );                                  // == чужая команда
    if( Info( x,y,Ans==_MousePressed ) ) //&& Tv_MouseState&_MouseLeftPress ) )
    { int X,ix,lx=b.right-b.left,                       // Команда в поле карты
          Y,iy,ly=b.bottom-b.top;
      if( Ans==_MousePressed )           //&& Tv_MouseState&_MouseLeftPress )
      { ReScale: _x=x,_y=y; ix=iy=0;                    // Выбор области
        while( (Ans=Tv_getc())!=_MouseReleased )        // для масштабирования
        { Tv_rect_x( _x,_y,x,y );                       //
          Tv_GetCursor( x,y ); X=x,Y=y;
          if( !ix ){ if( x>b.right )ix=1,x-=lx; else if( x<b.left )ix=-1,x+=lx; }
              else   if( ix<0 && x>=b.right || ix>0 && x<=b.left )x+=ix*lx,ix=0;
          if( !iy ){ if( y>b.bottom )iy=1,y-=ly; else if( y<b.top )iy=-1,y+=ly; }
              else   if( iy<0 && y>=b.bottom || iy>0 && y<=b.top )y+=iy*ly,iy=0;
          if( ix>0 && x>_x )x=_x-1; else if( ix<0 && x<=_x )x=_x+1;
          if( iy>0 && y>_y )y=_y-1; else if( iy<0 && y<=_y )y=_y+1;
          if( x!=X || y!=Y )Tv_SetCursor( x,y );        // Check( x,y );
          Tv_rect_x( _x,_y,x=minmax( _x-lx,x+ix*lx,_x+lx ),
                           y=minmax( _y-ly,y+iy*ly,_y+ly ) );
          Info( x,y,true );                             // тянем за уголок
        }
        if( _x==x && _y==y )return _MousePressed;       // пока без перехлеста
//      if( _y>y ){ Y=y; y=_y; _y=Y; }                  //  через Гринвич или
//      if( _x>x ){ X=x; x=_x; _x=X; }                  //   меридиан смены дат
  ReSelect:
        while( (Ans=Tv_getc())!=_MousePressed )
        { if( Ans<_MouseMoved || Ans>_MouseReleased )break; // к выходу
          Tv_GetCursor( X,Y );         // Check( X,Y ); // ожидание повторной
                  Info( X,Y,false );                    //  реакции мышки
        }
        Tv_rect_x( _x,_y,x,y );                         // - поставить рамку
        if( Ans<_MouseMoved || Ans>_MouseReleased )return Ans; //   на выход
        if( ix>0 && X<_x || ix<0 && X>_x )X += ix*lx;   // подстройка позиции
        if( iy>0 && Y<_y || iy<0 && Y>_y )Y += iy*ly;   //  на внешний размер
        if( ( (X-x)^(X-_x) )<0
         && ( (Y-y)^(Y-_y) )<0 )           //&& Tv_MouseState&_MouseLeftPress )
        { Ans=Tv_getc();
          if( Ans==_MouseMoved )                        // Перемещение рамки
          {
//          Tv_GetCursor( _X,_Y );                      // Info( 0,0,false );
//          Tv_rect_x( _x+=(_X-X),_y+=(_Y-Y),x+=(_X-X),y+=(_Y-Y) );
            Tv_rect_x( _x,_y,x,y );                     // временно снять рамку
            Tv_SetCursor( X=_x,Y=_y );
            Tv_CursorDisable();
            while( (Ans=Tv_getc())==_MouseMoved )
            { Tv_rect_x( _x,_y,x,y );                   // гасится старая рамка
              Tv_GetCursor( _x,_y ); x+=_x-X,y+=_y-Y;   // относительно начала
              Info( X=_x,Y=_y,true );                   // рамка на карте
              if( !ix )
              { if( _x<b.left  )_x+=lx,x+=lx,ix=1;  else if( x>b.right )ix=1;
                if( _x>b.right )_x-=lx,x-=lx,ix=-1; else if( x<b.left )ix=-1;
              } else if( ix==1  ){ if( _x>b.right )ix=0,_x-=lx,x-=lx; else if( x<b.right )ix=0; }
                else if( ix==-1 ){ if( _x<b.left  )ix=0,_x+=lx,x+=lx; else if( x>b.left  )ix=0; }
              if( !iy )
              { if( _y<b.top    )_y+=ly,y+=ly,iy=1;  else if( y>b.bottom )iy=1;
                if( _y>b.bottom )_y-=ly,y-=ly,iy=-1; else if( y<b.top    )iy=-1;
              } else if( iy==1  ){ if( _y>b.bottom )iy=0,_y-=ly,y-=ly; else if( y<b.bottom )iy=0; }
                else if( iy==-1 ){ if( _y<b.top    )iy=0,_y+=ly,y+=ly; else if( y>b.top    )iy=0; }
              if( _x!=X || _y!=Y )Tv_SetCursor( X=_x,Y=_y );
                                  Tv_rect_x( _x,_y,x,y ); // снова ставим рамку
            }                     Tv_CursorEnable();
            goto ReSelect;
          } else
          { if( Ans!=_MouseReleased ){ x=X; y=Y; goto ReScale; }
            return (Course)-1;    //! -1 Выход с командой на перередактирование
          }
        } goto for_Repeat;
      }
    } else
    if( Ans==_MousePressed )                                  // Выход вне поля
    { if( (Ans=Tv_getc())==_MouseReleased )return _MousePressed;
      goto for_Repeat;
    }
  }
}
void Button::Place( int xt, int yt )
{ Tv.Height( 8 );
  if( xt<0 ){ f.Lx=-xt-Tv.mX; f.Jx=xt+Tv.StrLen( _sign )+4; } // отсчет слева
      else  { f.Jx=-xt; f.Lx=Tv.StrLen( _sign )-Tv.mX;
            } f.Ly=-yt; f.Jy=yt+10-Tv.mY;
              Tv_place( &f ); b=Tv_port;       // считывание рамки кнопочки и
              Tv_place( &f ); Tv.Height( 0 );  // восстановление исходного поля
}
void Button::Initial( const char* sign, int xt,int yt )
   { if( _sign )free( _sign ); _sign=strdup( sign ); Place( xt,yt );
   }
void Button::Draw()
{ if( _sign ){ Tv.Height( 8 ); int c=color( BLUE );
               Tv.Text( South_East,b.left,b.top,_sign );
               rectangle( b.left,b.top,b.right,b.bottom );
               Tv.Height( 0 ); color( c ); }
}
//void Window::Check( int &X, int &Y )
//   { bool Ans=false;                              //
//     if( Y<b.top    ){ Ans=true; Y=b.top; } else  // Удержание мышки в
//     if( Y>b.bottom ){ Ans=true; Y=b.bottom; }    //  границах заданного поля
//     if( X<b.left   ){ Ans=true; X=b.left; } else //
//     if( X>b.right  ){ Ans=true; X=b.right; }
//     if( Ans )Tv_SetCursor( X,Y );
//   }
