//
//-     + find - для поиска ближайшего слева индекса в регулярном (±) массиве
   {A=fmod(A,360);return A>180?A-360:(A<-180?A+360:A);} // контроль направления
// двоичный поиск ближайшего левого индекса в поступательной последовательности
//                                                  с учетом знака его прироста
template <class real> int find( const real *A, const real &Ar, int N )
{ int k,i=0; bool d=A[--N]>A[0];
   if( d ){ if( Ar<=A[0] )return 0; else if( Ar>=A[N] )return N-1; }
     else { if( Ar>=A[0] )return 0; else if( Ar<=A[N] )return N-1; }
  while( N-i>1 ){ k=(N+i)/2; if( d ){ if( Ar<A[k] )N=k; else i=k; }
                               else { if( Ar>A[k] )N=k; else i=k; } } return i;
}
template int find<double>( const double *A, const double &Ar, int N );
template int find<float>( const float *A, const float &Ar, int N );
//
    if( *rs=='-' )sign=1;                         // знак отделяется от разбора