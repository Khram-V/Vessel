#include "Tv_graph.h"
//
//      Name    3 строки расширенного названия, и признак движения
//      Text    Парное описание основных команд
//      Plus    Парное описание дополнительных команд
//              Информационные блоки закрываются нулевым адресом
//
Course Help( const char *Name[], const char** Text, const char** Plus )
{
 int i=strlen( Name[0] ),n=0,h=0,t=0,p=0,l=29,b=Tv.BkColor,c=Tv.Color,k;
 Course Ans;
  if( Name )for( ; Name[n] && Name[n][0]; n++ );
  if( Text )for( ; Text[t]; t++ ); if( t )h++;
  if( Plus )for( ; Plus[p]; p++ ); if( p )h++; h+=n+(p+1)/2+(t+1)/2+1; //2;
  for( c=1; c<=2; c++ )
   if( (k=i+strlen( Name[c] ))>l )l=k; l+=2; bkcolor( BLUE );
  do
  { static int x=0,y=-1; Twindow( x,y,l,h,1 );
    color( YELLOW );     Tgo( 2,2 );
    for( k=0; k<n; k++ )
    { Tprintf( W2D( Name[k] ) ); Tgo( i+2,k+2 ); color( WHITE );
    }
    for( k=0; k<t; k++ )
    { if( !(k&1) ){ color( WHITE );     Tgo( 2,n+k/2+2 ); }
               else color( LIGHTCYAN ); Tprintf( W2D( Text[k] ) );
    }
    for( k=0; k<p; k++ )
    { if( !(k&1) ){ color( LIGHTGREEN ); Tgo( 2,n+k/2+(t+1)/2+(t>0)+2 ); }
               else color( LIGHTGRAY );  Tprintf( W2D( Plus[k] ) );
    }
    if( n||t||p )
    { Bfont C( _Small_font,11 ); color( YELLOW );
      C.Text( North_West,Tport.right-10,Tport.bottom-1,
                    W2D( "(c)1975-2017, В.Н.Храмушин." ) );
    }
    do{ Ans=Tv_getc(); } while ( Ans>=_MouseMoved && Ans<=_MouseReleased );
    Tback();
//  if( !Name[n] )break;
    if( Ans==_Esc || Ans==_Enter )Ans=Center; else
    if( Ans<_Enter ){ if( Ans&North )--y; if( Ans&East  )x+=2;
                      if( Ans&South )++y; if( Ans&West  )x-=2;
                    }
  } while( Ans && Ans<_Enter ); color( c ); bkcolor( b );
    return Course( Ans );
}
#if 0
// extern char* Tv_Help_FileName;
//  Tv_Help_FileName="ANI.f1h";
//Course Tv_Help( const char Part[] );
 static char *Name[]={ "Time  ","",
                                "",
                                "",0 },
             *Text[]={ "F1 ","",
                       "F2 ","",
                       "F2 ","",
                       "F2 ","",
                       "F2 ","",
                       "F2 ","",
                       "F8 ","",0 },
             *Plus[]={ "","",
                       "","",
                       "","",0, };
//
// Считывание разделов помощи из специального файла Exename.f1h
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Признаком начала блока описания является точка в первой строке,
// за которой следует ключевое слово (или выражение). Признаком начала
// и продолжения подраздела (внутри блока описания) является вертикальная
// палочка '|', которая разделяет строку на две части - русскую|анлийскую
//
// Во избежание зацикливания при считывании данных, произведено
// ограничение количества строк в подразделах:
//                      1. Name[6]  - расширенное название
//                      2. Text[16] - список основных команд
//                      3. Plus[16] - пояснения и комментарии
//
char * Tv_Help_FileName=NULL;
Course Tv_Help( const char Part[] )
{ int i;
 Course Ans=Center;
 char **Name=NULL,      // Расщширенное название процедуры
      **Text=NULL,      // Список основных команд
      **Plus=NULL,      // Дополнительный список и комментарии
      *s,*str=new char[132];
 FILE   *F = NULL;
  if( !str )goto _exit;
  if( !Tv_Help_FileName ){ strcpy( str,_av_[0] ); fext( str,"f1h" ); }
                      else strcpy( str,Tv_Help_FileName );
  if( !(F=Topen( str )) )goto _exit;            //
  while( fgets( str,132,F ) )                   // Поиск блока и запуск
  if( *str=='.' )                               // процедуры считывания
  if( !memcmp( str+1,Part,strlen( Part ) ) )    //
  {
    Name=(char**)Allocate( sizeof(char*)*8 );
    Name[0]=(char*)Part;
    for( i=1; fgets( str,132,F ); i++ )
    if( i>=7 || (s=strchr( str,'|' ))==NULL )break; else
    { *s=0; strcut( str ); Name[i]=strdup( str );
    }
    Text=(char**)Allocate( sizeof(char*)*33 );
    for( i=0; fgets( str,132,F ); i+=2 )
    if( i>=32 || !(s=strchr( str,'|' )) )break; else
    { *s=0;  strcut( str );
      if( (s=strchr( str,'\\' ))==NULL )break; else
      { *s=0; Text[i]  =strdup( str );
      }       Text[i+1]=strdup( s+1 );
    }
    Plus=(char**)Allocate( sizeof(char*)*33 );
    for( i=0; fgets( str,132,F ); i+=2 )
    if( i>=32 || (s=strchr( str,'|' ))==NULL )break; else
    { *s=0; strcut( str );
      if( (s=strchr( str,'\\' ))==NULL )break; else
      { *s=0; Plus[i]  =strdup( str );
      }       Plus[i+1]=strdup( s+1 );
    }
  } Ans=Help( Name,Text,Plus );
_exit:
  if( str )delete str;
  if( F )fclose( F );
  if( Name )
  { for( i=1; i<7;  i++ )if( Name[i] )free( Name[i] ); Allocate( 0,Name );
    for( i=0; i<32; i++ )if( Text[i] )free( Text[i] ); Allocate( 0,Text );
    for( i=0; i<32; i++ )if( Plus[i] )free( Plus[i] ); Allocate( 0,Plus );
  } return Ans;
}
#endif
