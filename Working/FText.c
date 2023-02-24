//!     Unicode-UTF-8 - Russian Windows-1251 + OEM-866
//!     ��������� ���������� ���������/�������������� ��������� �����
//       -- ���������/�������� ����������
//       -- ����������� ���������� ������ �����
//       -- ���������� �������� ������� (������)
//
#include <StdIO.h>
#include <Windows.h>
                                  //
#define Blength 65532u            // 2^16-4 ����������� ����� �������� �������
                                  //
static char *getString( FILE *F )
{ static char *S; static int L=0; int c,k=0;       // ������ ��������� ������
  if( !L )S=(char*)malloc( L=Blength ); S[0]=0;    //      ������������ �����
  while( !feof( F ) )
   if( (c=fgetc( F ))!='\r' )                      // ����� ������ ����� ����
   if(  c=='\n' || c==EOF )break; else             // �� � ����� ������� ������
    { if( k>=L )S=(char*)realloc( S,L+=Blength ); S[k++]=c; } S[k]=0; return S;
}
//      ��������� ����������� ������ � ����
//
static char *OutPut( int l )           // !! ��������� ��������������� �����
{                                      //    ��� ������� � ��������� �������
 static char *OutBuf=NULL;             // ���������� �������� �����
 static DWORD OutLen=0,OutCur=0;       // ������ ����������� ����� ������
  if( !l )OutCur=0; else OutCur+=l;    // ������� ��������� ������� ����������
  if( OutLen-OutCur<=Blength )
      OutBuf=(char*)realloc( OutBuf,OutLen=(OutLen/Blength+2)*Blength );
  return OutBuf+OutCur;
}
//      ������� ��������� �� ���������
//
static void Help( void )
{ printf( "\n?Use: Ftext name(s) [ -S0 ][ -T-8 ][ -R ]"
          "  (c)92-20 Khram\n"
          "     /S�n    empty strings [+max-sub]\n"
          "     /T�n    tabulations   [+set-del]\n"
          "     /R      exclude <CR> symbol\n" ); exit( 2 );
}
int main( int Ac, char **Av )
{
 int  T=0,                   // ��������� ���� ����������
      E=0,                   // ���������� ���������� ������ �����
      R=1,                   // ��������� CR � LF
//    C=0,c=0,U=0,           // ������������� ������� ����: L=Large; S=Small
      Kn,k,l; //,NF,nF=1;    // ���������� ������
 FILE *Fi;                   // ���� ������ ���� �� ������ � ����� - ��� ������
 WIN32_FIND_DATA FileData;   // ����-��������� ����������� �����
 HANDLE hSearch;             // ������ �� �������� ������ �� ����������
 DWORD  dwAttrs=0,           // ������� ���������� ������
        FileLength=0;        // ����� �����
 char *inS,*S,*s;            // -- � ����� �� ����������
//
//      �������� ���������� ����������
//
//SetConsoleCP( ANSI_CHARSET );                            // OEM_CHARSET=866
  SetConsoleOutputCP( OEM_CHARSET ); if( Ac<2 )Help();    // ANSI_CHARSET=1251
//
//      ����� ������ �� ������ ����������
//
  for( l=k=1; k<Ac; )
  { if( Av[k][0]=='-' || Av[k][0]=='/' )
    { switch( Av[k][1]|0x20 )
      { case 'r': R=0; break;
        case 't': T=8;
                  sscanf( Av[k]+2,"%d",&T ); if( T==0 )T=-1;           break;
        case 's': sscanf( Av[k]+2,"%d",&E );                           break;
       default  : printf( "\n\7?Incorrect switch: /%c\n",Av[k][1] );
                  Help();
      }
    } else l++; k++; if( l<k )Av[l]=Av[k];
  } Ac=l;
//
//      ������ ���������� � ������� ������
//
  char St[MAX_PATH];                        // ������ ��������� ��� Far-manager
   AnsiToOem( Av[Kn],St ); printf( "%s[%d] ",St,Ac );
   if( E>0 )printf( "/S%+d ",E );
   if( T   )printf( "/T%+d ",T );
   if( !R  )printf( "/R " );
            printf ( "\n" );
//
//      ������ ����� �� ������ ���� � ����������
//
  for( Kn=1; Kn<Ac; Kn++ )                  // �� ������ �� ������� ����������
  {
    BOOL done,fFinished=FALSE;
    AnsiToOem( Av[Kn],St );
    printf( ">%3d: %-24s",Kn,St );          // ���������� ����� ������ ������
//
//      ���������� ������ ���� �� ����� ������ � ����������� ������
//
    if( (hSearch=FindFirstFile( Av[Kn],&FileData ))==INVALID_HANDLE_VALUE )
      continue;
//
//      ������ ����� �� ��������� ������ ������ �� �������� ����������
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
          long ni=0,no=0; S=OutPut( 0 );        //! ��������� ������ ������
          while( !feof( Fi ) )
          { Len=strlen( inS=getString( Fi ) );
            if( !t ){ strcpy( S,inS ); l=Len; } // � ��� ������
            else for( l=k=0; k<Len; k++ )       // ��������� ��� ����������
            { if( (S[l]=inS[k])!=9 )l++; else do S[l]=' '; while( ++l%t );
            }                             //
            S[Len=l]=0;                   // ��������� �������� ��������
            s=S+Len-1;                    //
            while( Len>0 && *s<=' ' && *s!=12 ){ *s=0; --s; --Len; }
            if( Len>1 && *s==12 )
            { while( Len>0 && *s<=' ' ){ *s=0; --s; --Len; } S[Len++]=12;
            }                           //
            if( Len )e=0; else ++e;     // ������ � ��������� ������ �����
                                        //
            if( !e || !E || ( E>0 && e<=E )
                         || ( E<0 && e>-E ) )
            { if( T>1 )                 // ��������� ����������� �����������
              { for( m=l=k=0; l<Len; k++,l++ )
                { if( l%t==0 ){ if( m>1 ){ S[k-=m]=9; k++; } m=0; }
                  if( k<l )S[k]=S[l];
                  if( S[l]==' ' )m++; else m=0;
                } Len=k;
              }                        // 0x0D
              if( !feof( Fi ) ){ if( R )S[Len++]=13; S[Len++]=10; }
              S=OutPut( Len ); ni++; no+=Len;
            }
          } fclose( Fi );          S=OutPut( 0 ); // ����� ������ ��� ������

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
    } FindClose( hSearch ); // ��������� ������� ������
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
