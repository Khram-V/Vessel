
const double EPS = 1e-9;

struct pt  { double x,y; };
struct line{ double a,b,c;
  line( pt p,pt q ){ a=p.y-q.y; b=q.x-p.x; c= -a*p.x-b*p.y; norm(); }
  void norm(){ double z=sqrt( a*a+b*b ); if( abs( z )>EPS )a/=z,b/=z,c/= z; }
  double dist( pt p ) const { return a*p.x + b*p.y + c; }
};
#define det( a,b,c,d )( a*d-b*c )
double det( double a,double b,double c,double d ){ return a*d - b*c; }

bool intersect( line m,line n, pt &res )
{ double zn=det( m.a,m.b,n.a,n.b ); if( abs( zn )<EPS )return false;
     res.x=-det( m.c,m.b,n.c,n.b )/zn;
     res.y=-det( m.a,m.c,n.a,n.c )/zn; return true;
}
bool parallel( line m,line n )
{ return abs( det( m.a,m.b,n.a,n.b ) )<EPS;
}
bool equivalent( line m,line n )
{ return abs( det( m.a,m.b,n.a,n.b ) )<EPS
      && abs( det( m.a,m.c,n.a,n.c ) )<EPS
      && abs( det( m.b,m.c,n.b,n.c ) )<EPS;
}