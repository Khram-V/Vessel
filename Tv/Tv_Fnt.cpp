//
//      Просмотр и изготовление шрифтов нового DESIGN-CAD 6.00
//
//  2007-03-04 - сделать вывод .chr или .vct для основной среды программирования
//
#include "OS/Tv_Graph.h"
#define Debug 0
#define Err2 "\n\7?\"%s\" Can`t open for read\n"
                       //
struct DCFont          // Структура векторного шрифта для Design-CAD 6.00
{                      //
 char  Ver,Fill;       // Основной признак и бит закраски
 int   Width,Height,   // Размеры для пропуска и высоты буквы
       Begin;          // Номер первого байта для первой таблицы
 char  Ident[48];      // Название шрифта
 short WL[256],        // Таблица ширины для каждой из букв
       AL[257],        // Таблица смещений для стартовых отрезков
       P[64000],       // Основные векторы для рисования букв
//_______________________ Данные определяют формат файла .VFN
       QL[256],        // Таблица количества отрезков в одной букве
       Vtyp;           // Тип входного файла: 5-.VCT; 6-.VFN.
 long  L;              // Длина файла -> Общее количество отрезков*4
 char* FName;          // Текущее имя файла
 void  Read_VFN();     // Чтения для Design CAD 5.00
 void  Read_VCT();     // Чтения для Design CAD 6.00
 void  Read_CHR();     // Чтения шрифта BGI версии 2.00
                       //--//
 void  Text_Information(); // Простое текстовое представление
 void  Graphics_Drawing(); // Изображение и командный монитор
                           //
};
void DCFont::Read_VFN()
{ int i,k;
 FILE *F;
  if( (F=fopen( FName,"rb" ))==0 )Break( Err2,FName );
  fseek( F,0,SEEK_END ); L=ftell( F )-(8+48+512+514);
  fseek( F,0,SEEK_SET );
    Ver=fgetc( F );
   Fill=fgetc( F );
  Width= get2( F );
  Height=get2( F );
  Begin =get2( F ); fread( Ident,1,48,F ); fseek( F,Begin,SEEK_SET );
                    fread( WL,2,256,F );
                    fread( AL,2,257,F );
  //
  //   Контроль входных данных до запуска процесса считывания
  //
  for( i=k=0; i<256; i++ )k+=QL[i]=AL[i+1]-AL[i];
  if( k!=(i=AL[256]) )
      Ghelp( "?Offset & Number of Line not Equal (%d^=%d)",i,k ),Tv_getc();
  if( i*4L != L )
      Ghelp( "?Lines Number Error: %dx4=%d ^= %ld ",i,i*4,L ),Tv_getc();
  //
  // Считывание основного потока отрезков для рисования букв
  //
  fread( P,1,(unsigned)L,F );
  fclose( F );
}                       //
#define Lw 1024         // Максимальная длина обрабатываемой строки
                        //
static int Stream( FILE *F, char* S, char* &s )
{ while( *s<=' ' )
  { if( *s )++s; else
    { fgets( S,Lw,F ); s=S;
      while( *s<=' ' )
        { if( *s==0 ){ Tv_bell(); return 0; } s++;
  } } } char* z=s; while( *s>' ' )s++; *s=0; s++; return atoi( z );
}
void DCFont::Read_VCT()
{ int i,j,k;
 short *p;
 char S[Lw],*s;
 FILE *F;
  if( (F=fopen( FName,"rt" ))==0 )Break( Err2,FName );
  for( L=0;;)
  { fgets( S,Lw,F ); if( *S<=' ' )break; else i=*S;
    fgets( S,Lw,F ); sscanf( S,"%d%d",&k,&j ); WL[i]=j; L+=QL[i]=k; k*=2;
          *S=0; s=S;
    while( k-->0 )Stream( F,S,s );
  } rewind( F ); L*=4;
  for( i=0; i<256; i++ )AL[i+1]=AL[i]+QL[i];
  for( ;; )
  { fgets( S,Lw,F ); if( *S<=' ' )break; else i=*S;
    fgets( S,Lw,F ); k=QL[i]; p=P+AL[i]*2; *S=0; s=S;
    for( j=0; j<k; j++,p++ ){ p[0]=Stream( F,S,s );
                              p[k]=Stream( F,S,s ); }
  } Ver=3;        fgets( S,Lw,F );
  Begin=56; i=0; sscanf( S,"%d%d%d%d",&Width,&Height,&j,&i );
  if( i )Fill=1;  fgets( S,Lw,F );
  for( i=0; i<45 && S[i]>=' '; i++ )Ident[i]=S[i];
  ((unsigned short*)Ident)[23]=0x3F80;
  fclose( F );
}
#undef Lw
#if Debug
void DCFont::Text_Information( void )
{
 int i,j,k;
 short *pY,*pX;
    printf( "\n: Sign=%d, [%dx%d] %s(%d) \"%s\" %02X%02X\n:\n",
        (int)Ver,Width,Height,Fill?"Filled ":"",Begin,Ident,
        (int)(Ident[46]),(int)(Ident[47]) );
    printf( ": << Wide for Each Letters >>\n" );
  for( i=0; i<256; i++ )
  { printf( " %c",i<32?'.':i );
    printf( WL[i]?"%3d":"   ",WL[i] );
  } k=0;
    printf( ":\n: << Number of Lines for Letter Drawings >>\n" );
  for( i=0; i<256; i++ )
  { printf( " %c",i<32?'.':i ); j=QL[i];
    printf( j?"%3d":"   ",j );
  }
  for( i=k=0; i<256 && k<3; i++ )if( WL[i] )
  { j=QL[i];
    printf( "\n<< %c >>(%d)\n",i,j ); ++k;
    pX=P+AL[i]*2;
    pY=pX+j;
    while( j>0 ){ printf( "(%d,%d)",*pX,*pY ); ++pX; ++pY; j--; }
  } i=getchar();
}
#endif
void DCFont::Graphics_Drawing()
{
 int i,j,k,m,Yy,Xx;
 short *pY,*pX;
 char str[256];
// const char *Msg[7]={ Ident," !\"#$%&'()*+,-./0123456789:;<=>? ",
//                            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ ",
//                            "`abcdefghijklmnopqrstuvwxyz{|}~ ",
//                            "АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ ",
//                            "абвгдежзийклмнопрстуфхцчшщъыьэюя ",
//                            "ЁёЄєЇїЎў°•·±№¤¦ "
//             };
  if( ( Tv_getk()&SHIFT )==0 )clear();
  color( DARKGRAY );
//if( Msg[3][0]=='А' )
//for( m=3; m<7; m++ )CharToOemBuff( Msg[m],Msg[m],strlen( Msg[m] ) );
  Yy=Height+24;
  for( m=0; m<8 && Yy<Tv.mY-3; m++ )
  { //str=string( m>0?W2D( Msg[m] ):Msg[0] );
    if( !m )strcpy( str,Ident ); else
    { for( i=0; i<32; i++ )str[i]=m*32+i; str[32]=0; }
    Xx=6;
    i=0;
    while( (j=str[i++])!=0 && Xx<Tv.mX-6 )
    { int Y,X,y=-1,x=-1; k=QL[j]; pX=P+AL[j]*2; pY=pX+k;
      while( k-- > 0 )
      {  Y=*pY++;  X=*pX++;
         if( x<0  && x==y )moveto( Xx+X,Yy-Y ); else
         if( X>=0 || X!=Y )lineto( Xx+X,Yy-Y ); x=X; y=Y;
      } k=WL[j]; Xx+=k?k:Width;
    }   Yy+=(Height*3)/2; if( !m ){ Yy+=Height/2; color( CYAN ); }
  }
  Thelp( " Sign:%d, [%dx%d]:%ld %s(%d)  \"%s\"  %02X%02X",
       (int)Ver,Width,Height,L,Fill?"Filled ":"",Begin,Ident,
       (int)(Ident[46]),(int)(Ident[47]) );
 Mlist Menu[]={ { 0,45,"Id: %s",Ident }
              , { 1,3,"  [%3d",&Width },{ 0,3,"x%d",&Height }
              , { 0,0,"] " },{ 0,8 }  // Признак Fill No Fill
              , { 0,0,"     Write to: " },{ 0,1,".VFN " },{ 0,1,".VCT " }
              , { 1,45,"File: %s",W2D( FName ) }
              };
  color( CYAN ); i=0;
  do
  { Menu[4].Msg=(string)(Fill?" Fill":" No Fill");
    for( j=0; j<45; j++ )
    if( FName[j]<=' ' || FName[j]=='.' ){ FName[j]=0; break; }
    switch( i=Tmenu( Mlist( Menu ),1,-1,i ) )
    { case 4: if( Fill )Fill=0; else Fill=1; break;
      case 6:                                //
      { FILE *F;                             // Запись шрифта
        strcat( FName,".vfn" );              // в формате .VFN
        if( (F=fopen( FName,"wb" ))!=0 )     //
        { //fwrite( &Ver,1,(unsigned)L+8+48+512+514,F );
           fputc( Ver,F );
           fputc( Fill,F );
            put2( Width,F );
            put2( Height,F ); // Begin=56;
            put2( Begin,F );
          fwrite( Ident,1,48,F );
          fwrite( WL,2,256,F );
          fwrite( AL,2,257,F );
          fwrite( P,1,(unsigned)L,F );
          fclose( F );
        } else Tv_bell();
      } break;
      case 7:
      { FILE *F;
       int i,j,k,l;
       short *p;                             // Запись текстового
        strcat( FName,".vct" );              // шрифта Design-CAD v5.
        if( (F=fopen( FName,"wt" ))!=0 )     // (в формате .VCT)
        { for( i=0; i<256; i++ )
          if( QL[i]>0 )
          { fprintf( F,"%c\n%d %d\n",i,QL[i],WL[i] ); p=P+AL[i]*2; k=QL[i];
            for( j=l=0; j<k; j++,p++ )
            { l+=fprintf( F," %d %d",p[0],p[k] );
              if( l>72 && j<k-1 ){ fprintf( F,"\n" ); l=0; }
            } fprintf( F,"\n" );
          }   fprintf( F,"\n%d %d %d %d\n%s\n",Width,Height,Width/5,Fill,Ident );
               fclose( F );
        } else Tv_bell();
      }
    }
  } while( i!=_Esc ); //Tv_close();
}
void FNT_View( char* _fName, const int Typ )
{
 DCFont *V;
  V=(DCFont*)Allocate( sizeof( DCFont ) );
           memset( V,0,sizeof( DCFont ) ); V->FName=_fName; V->Vtyp=Typ;
  Tv_Graphics_Start();
  setwindowtitle( "Tv - CHR-VCT-VFN - fonts" );
  bkcolor( WHITE );
  if( Typ==2 )V->Read_CHR();
  if( Typ==3 )V->Read_VCT();
  if( Typ==4 )V->Read_VFN();

#if Debug
      V->Text_Information();
#endif
      V->Graphics_Drawing(); exit( 0 );
}
//
//=======================================================================
//
//  Copyright (c) 1988,89 Borland International
//
#define Minor_Version   0
#define Major_Version   1
#define Prefix_Size     0x80    // 128
#define SIGNATURE       '+'

enum OP_CODES{ END_OF_CHAR, DO_SCAN, MOVE, DRAW };

typedef struct                  //
{ char  sig;                    // SIGNATURE byte
  short nchrs;                  // number of characters in file
  char  mystery;                // Currently Undefined
  char  first;                  // first character in file
  short cdefs;                  // offset to char definitions
  char  scan_flag;              // True if set is scanable
  char  org_to_cap;             // Height from origin to top of capitol
  char  org_to_base;            // Height from origin to baseline
  char  org_to_dec;             // Height from origin to bot of decender
  char  fntname[4];             // Four character name of font
  char  unused;                 // Currently undefined
} HEADER;                       //

typedef struct                  //
{ char  opcode;                 // Stroke opcode byte
  short x;                      // Relative offset x direction
  short y;                      // Relative offset y direction
} STROKE;                       //

typedef struct                  //
{ fixed header_size;            // Version 2.0 Header Format
  byte  font_name[4];           // Font Internal Name
  fixed font_size;              // Size in byte of file
  byte  font_major,font_minor;  // Driver Version Information
  byte  min_major, min_minor;   // BGI Revision Information
} FHEADER;                      //
//
//      DECODE: This routine decodes a single word in file to a
//      stroke record.
//
int decode( unsigned short *iptr, short *x, short *y ) // FUNCTION
{ struct xy { byte x,y; } d=*(xy*)iptr;
     *x = ((signed char)(d.x<<1))/2;
     *y = ((signed char)(d.y<<1))/2;
   return ( ( (d.x&0x80)!=0 ) ?2:0 )
        | ( ( (d.y&0x80)!=0 ) ?1:0 );
}
//      UNPACK: This routine decodes the file format
//      into a more reasonable internal structure
//
int unpack( char *buf, int index, STROKE **New ){ // FUNCTION
 STROKE   *po;
 unsigned short *pb;
 short num_ops=0, jx,jy, opcode,i,opc;
                                        //
  pb=(unsigned short*)(buf+index);      // Reset pointer to buffer
  do                                    // For each byte in buffer
  { num_ops++;                          // Count the operation
        opcode=decode( pb++,&jx,&jy );  // Decode the data record
  } while( opcode!=END_OF_CHAR );       // Exit loop at end of char
                                        //
  po=*New=(STROKE*)Allocate( num_ops*sizeof( STROKE ),*New );
  pb=(unsigned short*)(buf+index);      // Reset pointer to buffer
                                        //
  for( i=0; i<num_ops; ++i )            // For each opcode in buffer
  { opc = decode( pb++,&po->x,&po->y ); // Decode the data field
    po->opcode=opc;                     // Save the opcode
    po++;                               //
  } return( num_ops );                  // return OPS count
}                                       //
//
//=======================================================================
//
void DCFont::Read_CHR()
{                                  //
 STROKE  *sptr,*Strokes;           // Stroke Data Base
 HEADER  Header;                   // File Data Header
 long    length, current;          //
 short   Offset[256],i,j,k;        // Font data offsets
 char   *Font,                     // Pointer to font storage
        Prefix[Prefix_Size],       // File Prefix Holder
        *cptr;                     //
 FILE *F;
  if( (F=fopen( FName,"rb" ))==0 )Break( Err2,FName );
//
//      Read in and display the file prefix record.
//
  Strokes=0;                       //
  fread( Prefix,1,Prefix_Size,F ); // Read in the file prefix
    cptr=Prefix;                   // Begin at base of prefix
  while( 0x1A!=*cptr )++cptr;      // Move to EOF in prefix
               *cptr='\0';         // Cut prefix at EOF
  strcpy( Ident,cptr+3 )[4]=0;     //
//
//      Read in and display the font header record.
//
//fread( &Header,sizeof(HEADER),1,F ); // Read in the file prefix
  Header.sig=fgetc( F );        // SIGNATURE byte
  Header.nchrs=get2( F );       // number of characters in file
  Header.mystery=fgetc( F );    // Currently Undefined
  Header.first=fgetc( F );      // first character in file
  Header.cdefs=get2( F );       // offset to char definitions
  Header.scan_flag=fgetc( F );  // True if set is scanable
  Header.org_to_cap=fgetc( F ); // Height from origin to top of capitol
  Header.org_to_base=fgetc( F );// Height from origin to baseline
  Header.org_to_dec=fgetc( F ); // Height from origin to bot of decender
  fread( Header.fntname,4,1,F );// Four character name of font
  Header.unused=fgetc( F );     // Currently undefined
//
//      Read file offset table into memory.
//
  fread( &Offset[int(Header.first)],Header.nchrs,sizeof(short),F );
//
//      Load the character width table into memory.
//
  for( i=Header.first; i<Header.first+Header.nchrs; i++ )getc( F );
//
//      Determine the length of the stroke database.
//
  current = ftell( F );         // Current file location
  fseek( F,0,SEEK_END );        // Go to the end of the file
  length = ftell( F );          // Get the file length
  fseek( F,current,SEEK_SET );  // Restore old file location
//
//      Load the stroke database.
//
  Font=(char*)Allocate(fixed(length));  // Create space for font data
  fread( Font,fixed(length),1,F );      // Load the stroke data
  fclose( F );
//
//      Font is now loaded, display the internal data
//
  {
   int w,wc,wl,wn,wy=0,wx=0;
   short *p;
         wc=i=j=0; L=0; // L=Header.nchrs*4;
         p=P;
    for( i=Header.first; i<Header.first+Header.nchrs; i++ )
    if( Offset[i] || i==Header.first )
    { wl=unpack( Font,Offset[i],&Strokes ); sptr=Strokes;
      for( j=w=wn=0; j<wl; ++sptr,++j )
      { k=sptr->opcode;
        if( k!=END_OF_CHAR )wx=sptr->x;
        if( k!=DRAW || !j )w=1; else{ if( w==1 )wn+=2; wn++; w++; }
      } --wn;
      if( wn>1 )
      { WL[i]=wx;
        sptr=Strokes;
        for( j=w=wc=0; j<wl; ++j,++sptr )
        { k=sptr->opcode;
          if( !j || k!=DRAW ){ w=1; wx=sptr->x; wy=sptr->y; } else
          { if( w==1 )
            { if( wc ){ p[0]=p[wn]=-1; p++; } else wc=1;
              p[0] =wx;
              p[wn]=wy; p++;
            } p[0] =sptr->x;
              p[wn]=sptr->y; p++; w++;
        } }  QL[i]=wn;       p+=wn;
      } else QL[i]=0;
    }
    for( i=0; i<256; i++ )AL[i+1]=AL[i]+QL[i]; L=AL[256]*4;
    Ver=3;
    Fill=0;
    Begin=56;
    Width=WL[fixed( '1' )];
    Height=Header.org_to_cap-Header.org_to_base;
//  if( Header.fntname[0] )strncpy( Ident,Header.fntname,4 )[4]=0;
//                   else  strcpy( Ident,fname( FName ) )[0]&=~0x20;
    ((unsigned short*)Ident)[23]=0x3F80;
  } Allocate( 0,Font );
    Allocate( 0,Strokes );
}
