/*     Unicode-UTF-8 - Russian Windows-1251 + OEM-866
//     ��������� ���������� ���������/�������������� ��������� �����
//       -- ���������/�������� ����������
//       -- ����������� ���������� ������ �����
//       -- �������� ������ �������� � ����� � ������
//       -- ���������� �������� ������� (������)
//       -- ��������������� <CR><LF> ��� ������ <LF>
*/
#include <StdIO.h>
#include <Windows.h>
#define Blength 65532u            // 2^16-4 ����������� ����� �������� �������
//
//  ������ ��������� ���� ����������� ����� � �������� �������� � ������
//
static int strcut( char* S )
{ int len=strlen( S ); S+=len; while(len>0 && *--S<=' ')--len,*S=0; return len;
}
#if 0                             //
static char *getString( FILE *F ) // ������� ��� ������� <CR> �������� �������
{ static char *S; static int L=0; int c,k=0;       // ������ ��������� ������
  if( !L )S=(char*)malloc( L=Blength ); S[0]=0;    //      ������������ �����
  if( feof( F ) )return NULL;
  do if( (c=fgetc( F ))!='\r' )                    // ����� ������ ����� ����
     if(  c=='\n' || c==EOF )break; else           // �� � ����� ������� ������
       { if( k>=L )S=(char*)realloc( S,L+=Blength ); S[k++]=c; }
  while( !feof( F ) );
  S[k]=0;
  return S;
}
#else
static int ic=0;
static char *getString( FILE *F )
{ static char *S; static int L=0; int k=0;           // ������ ��������� ������
  if( !L )S=(char*)malloc( L=Blength ); S[0]=0;      //      ������������ �����
  if( feof( F ) || ic==EOF )return NULL;
  for(;;)
  { if( !ic )ic=fgetc( F );                 // � ��������� �� ���������� ������
    if( ic==EOF ){ ic=0; break; }           // ����� ����� ��� �������� �������
    if( ic=='\r' ){ ic=fgetc(F); if( ic=='\n' )ic=0; break; } // �������������
    if( ic=='\n' ){ ic=fgetc(F); if( ic=='\r' )ic=0; break; } //  ���� ��������
    if( k>=L )S=(char*)realloc( S,L+=Blength+4 ); S[k++]=ic; ic=0; // ��� �����
  } S[k]=0; strcut( S ); return S;
}
#endif
//      ��������� ����������� ������ � ����
//
static char *OutPut( int l )            // !! ��������� ��������������� �����
{ static char *OutBuf=NULL;             //    ��� ������� � ��������� �������
  static long OutLen=0,OutCur=0;        // ���������� �������� �����
  if( !l )OutCur=0; else OutCur+=l;     // ������ ����������� ����� ������
  if( OutLen-OutCur<=Blength )          // ������� ��������� ������� ����������
      OutBuf=(char*)realloc( OutBuf,OutLen=(OutLen/Blength+2)*Blength );
  return OutBuf+OutCur;
}
//                                               ������� ��������� �� ���������
static void Help( void )
{ printf( "\n?Use: Ftext name(s) [ -S0 ][ -T-8 ][ -R ][ -B ]" // [ -L72 ][ -C ]
          "  (c)92-24 V.Khramushi\n"
          "     /S�n    empty strings [+max-sub]\n"
          "     /T�n    tabulations   [+set-del]\n"
          "     /R      exclude <CR> symbol\n"
          "     /B      exclude blank and <CR>\n" ); exit( 2 );
/*	  "	/C[s|l]	cyrillic words [small|large]\n"
	  "	/Ln	maxima length of strings\n" */
}
int main( int Ac, char **Av )
{
 BOOL R=TRUE,                // ���� FALSE, �� -  �������� ����� ��� <CR>
      B=TRUE;                // ��������� ������ �������� � ����� � ������
 int  T=0,                   // ��������� ���� ����������
      E=0,                   // ���������� ���������� ������ �����
//    C=0,c=0,U=0,           // ������������� ������� ����: L=Large; S=Small
      Kn,i,k,l; //,NF,nF=1;  // ���������� ������
 FILE *Fi;                   // ���� ������ ���� �� ������ � ����� - ��� ������
 WIN32_FIND_DATA FileData;   // ����-��������� ����������� �����
 HANDLE hSearch;             // ������ �� �������� ������ �� ����������
 DWORD  dwAttrs=0,           // ������� ���������� ������
        FileLength=0;        // ����� �����
 char *inS,*S; //,*s;        // -- � ����� �� ����������
//
//      �������� ���������� ����������
//
//SetConsoleCP( ANSI_CHARSET );                         // OEM_CHARSET=866
  SetConsoleOutputCP( OEM_CHARSET ); if( Ac<2 )Help(); // ANSI_CHARSET=1251
//
//      ����� ������ �� ������ ����������
//
  for( l=k=1; k<Ac; )
  { if( Av[k][0]=='-' || Av[k][0]=='/' )
    { switch( Av[k][1]|0x20 )
      { case 'r': R=FALSE; break;
        case 'b': B=FALSE; break;
        case 't': T=8;
                  sscanf( Av[k]+2,"%d",&T ); if( T==0 )T=-1;         break;
        case 's': sscanf( Av[k]+2,"%d",&E );                         break;
       default  : printf( "\n\7?Incorrect switch: /%c\n",Av[k][1] ); Help();
      }
    } else l++; k++; if( l<k )Av[l]=Av[k];
  } Ac=l;
//
//      ������ ���������� � ������� ������
//
  char St[MAX_PATH*2];                      // ������ ��������� ��� Far-manager
   AnsiToOem( Av[Kn],St ); printf( "%s[%d] /S%+d /T%+d /R<%s> /B<%s>\n",
                               St,Ac,E,T,!R?"cr":"-cr+lf",!B?"clear":"pass" );
/* if( E  )printf( "/S%+d ",E );
   if( T  )printf( "/T%+d ",T );
   if( !R )printf( "/R " );
   if( !B )printf( "/B " );
           printf( "\n" );
*/
//      ������ ����� �� ������ ���� � ����������
//
  for( Kn=1; Kn<Ac; Kn++ )                   // �� ������ �� ������� ����������
  {
    BOOL done,fFinished=FALSE;
    AnsiToOem( Av[Kn],St );
    printf( ">%3d: %-24s",Kn,St );           // ���������� ����� ������ ������
//
//                  ���������� ������ ���� �� ����� ������ � ����������� ������
//
    if( (hSearch=FindFirstFile( Av[Kn],&FileData ))==INVALID_HANDLE_VALUE )
      continue;
//
//               ������ ����� �� ��������� ������ ������ �� �������� ����������
//
    while( !fFinished )                     // ���������� ��������� ����� �����
    { dwAttrs = GetFileAttributes( FileData.cFileName );
      done = !( dwAttrs&FILE_ATTRIBUTE_READONLY );          // ������ �� ������
//    AnsiToOem( FileData.cFileName,St );
//    printf( "%12s %8ld Read %s ",St,
      printf( " %8ld Read %s ",FileLength=( FileData.nFileSizeHigh*MAXDWORD )
                               + FileData.nFileSizeLow,done?"Write":"Only" );
      if( done && FileLength )
      { if( (Fi=fopen( FileData.cFileName,"rt" ))!=NULL )
        { int Len,m,e=0,t=abs( T );
          long ni=0,no=0; S=OutPut( 0 ); ic=0;      //! ��������� ������ ������
          while( (inS=getString( Fi ))!=NULL )
          { //Len=0;
            //if( ni ){ if( R )S[Len++]=13; S[Len++]=10; }  // �� ������ ������
            Len=strlen( inS );
            if( !t ){ strcpy( S,inS ); l=Len; } else // � ��� ������
            for( l=k=0; k<Len; k++ )                // ��������� ��� ����������
               { if( (S[l]=inS[k])!=9 )l++; else do S[l]=' '; while( ++l%t ); }
            S[l]=0;                              // ��������� �������� ��������
            Len=strcut( S );
//            s=S+Len-1;                    //
//            while( Len>0 && *s<=' ' && *s!=12 ){ *s=0; --s; --Len; }
//            if( Len>1 && *s==12 )
//            { while( Len>0 && *s<=' ' ){ *s=0; --s; --Len; } S[Len++]=12;
//            }
            if( Len )e=0; else ++e;          // ������ � ��������� ������ �����
            if( !e || !E || ( E>0 && e<=E )     // + ���������� ������ �����
                         || ( E<0 && e>-E ) )   // - ����� �� ��������� �����
            { if( !B )                          // ��������� ��� ������ �������
              { for( i=k=l=0;; )
                if( S[l]>' ' || S[l]==30 && l<Len )S[k++]=S[l++]; else
                { if( k==i+2 && S[i+1]=='0'          // +0 � -0 ���������� �� 0
                    && (S[i]=='-' || S[i]=='+' ) )S[i++]='0',S[i++]=' '; // +=2
                  else
                  if( k>0 )
                  { int j,D=0;                         // ����� ����� � �������
                    for( j=i; j<k && D<2; j++ )if( S[j]=='.' )++D;// ���� �����
                    if( D==1 && k>i+1 )     // ���� �� ���� ����� ��� ������
                    { for( j=i; j<k; j++ )  // ����� � ������ � ������ � ������
                      { if( j==i && (S[i]=='+' || S[i]=='-') )continue;
                        if( (S[j]<'0' || S[j]>'9') && S[j]!='.' ){ D=0; break; }
                      }
                      if( D )
                      { while( S[k-1]=='0' )k--; // ��� �������� ���� ���������
                        if( S[k-1]=='.' ) // ����� ��������� � ���������� ����
                        { if( k-1==i )S[i]='0'; else k--;
                        }
                      }
                    } S[k++]=' '; i=k;         // ��� ����� ������������ ������
                  }
                  if( l>=Len )break;
                  while( S[l]<=' ' && l<Len )l++;         // � ���������� �����
                }
                S[k++]=0; Len=strcut( S );
              } else                       // ����� ������ �� �� �����������
              if( T>1 )                    // ��������� ����������� �����������
              { for( m=l=k=0; l<Len; k++,l++ )
                { if( l%t==0 ){ if( m>1 ){ S[k-=m]=9; k++; } m=0; }
                  if( k<l )S[k]=S[l];
                  if( S[l]==' ' )m++; else m=0;
                } Len=k;
              }
              if( !feof( Fi ) || ic==EOF ){ if( R )S[Len++]=13; S[Len++]=10; }
              S=OutPut( Len ); ni++; no+=Len;
            }
          } fclose( Fi );
            S=OutPut( 0 );              // ��������� ������ � ������ ��� ������
          if( (Fi=fopen( FileData.cFileName,"wb" ))!=NULL )fwrite( S,1,no,Fi );
               fclose( Fi );          printf( " %6lu/%-4lu",no,ni );
        } else printf( "Shut" );
      }        printf( "\n" );

      if( !FindNextFile( hSearch,&FileData ) )
      { if( GetLastError() == ERROR_NO_MORE_FILES )
        { //printf( "No more %s files. Search completed.",Av[Kn] );
            fFinished = TRUE;
        }   else
        {   printf("Couldn't find next file."); return 7;
        }
      }
    } FindClose( hSearch );                         // ��������� ������� ������
  } return 0;
}
#if 0
/* ����� ������ � ��������� ���������.

��������� ������ �������� ��� ��������� ����� �� ������� ���������� � �����
���������� � ������ \TEXTRO, ������� �������� �������� CreateDirectory. ���
�������������, ������ � ����� ���������� ������������� ������� "������ ������".

����� ������ .TXT � ������� ���������� ������������ ��������� FindFirstFile �
FindNextFile. ������ ���� .TXT ���������� � ���������� \TEXTRO. ����� ����,
��� ���� ����������, ������� GetFileAttributes ���������, �������� �� ����
������ ��� ������. ���� �������� "������ ������" �� �����������, �� ����������
������������� ���� ������� �������� SetFileAttributes.

����� ����������� ������ .TXT, ����� ������ ����������� �������� FindClose.
*/
#include <windows.h>
#include <stdio.h>
void main_block()
{
WIN32_FIND_DATA FileData;
HANDLE hSearch;
DWORD dwAttrs;
char szDirPath[] = "c:\\TEXTRO\\";
char szNewPath[MAX_PATH];
char szHome[MAX_PATH];

BOOL fFinished = FALSE;

if( !CreateDirectory( szDirPath,NULL ) )          // ������ ����� ����������.
{   printf("Couldn't create new directory."); return;
}
// �������� ����� ������ .TXT � ������� ����������.

hSearch = FindFirstFile("*.txt", &FileData);

if( hSearch == INVALID_HANDLE_VALUE )
{   printf( "No .TXT files found." ); return;
}
// �������� ������ ���� .TXT � ����� ���������� � �������� ���
// ������� �� "������ ������", ���� �� ��� �� ����������.

while (!fFinished)
{   lstrcpy(szNewPath, szDirPath);
    lstrcat(szNewPath, FileData.cFileName);
    if( CopyFile( FileData.cFileName, szNewPath, FALSE ) )
    {   dwAttrs = GetFileAttributes(FileData.cFileName);
        if( !( dwAttrs & FILE_ATTRIBUTE_READONLY ) )
        {   SetFileAttributes( szNewPath,dwAttrs | FILE_ATTRIBUTE_READONLY );
        }
    }   else
    {   printf("Couldn't copy file.");
        return;
    }
    if( !FindNextFile( hSearch,&FileData ) )
    {   if( GetLastError() == ERROR_NO_MORE_FILES )
        {   MessageBox(hwnd, "No more .TXT files.","Search completed.", MB_OK);
            fFinished = TRUE;
        }   else
        {   printf("Couldn't find next file.");
            return;
        }
    }
}
FindClose(hSearch); // ��������� ����� ������.
}
#endif
