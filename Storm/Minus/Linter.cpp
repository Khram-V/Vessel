static int find( Real *A,_Real Ar,int N )   // ƒвоичный поиск ближайшего левого
{ int k,i=0; bool d=A[--N]>A[0];            //  индекса дл€ заданного аргумента
   if( Ar<=A[0] )return d?0:N;              //   с учетом знака его прироста
   if( Ar>=A[N] )return d?N:0;
  while( N-i>1 ){ k=(N+i)/2; if( d ){ if( Ar<A[k] )N=k; else i=k; }
                               else { if( Ar>A[k] )N=k; else i=k; } } return i;
}
class Linter{ Real *_Y,*_X; const int len; public:
  Linter( int L ): len( L )
          { _Y=(_X=(Real*)malloc( L*sizeof( Real )*2 ) )+L; }
 ~Linter(){ free( _X ); }
  Real& operator[]( int k ){ return _Y[minmax( 0,k,len-1 )]; }
  Real& operator()( int k ){ return _X[minmax( 0,k,len-1 )]; }
  Real operator()( _Real A )
  { if( len<2 )return len?_Y[0]:0.0; int k=find( _X,A,len );
    const Real *x=_X+k,*y=_Y+k,a=A-x[0],h=x[1]-x[0];
    if( !a )return y[0];
    if( !h )return (y[0]+y[1])/2.0;
            return  y[0]+a*(y[1]-y[0])/h;
  }
};
