#ifndef _CONIO_H_
 void textsize( int w,int h,int bh=0 ); // размеры экрана, укрытая длина буфера
 inline void texttitle( const char* title ){ SetConsoleTitle( title ); }
//int cprint( const char*,... );
#endif