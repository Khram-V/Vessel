/**                                                                        ¹²³ⁿ
 *      Подборка геометрических фигур с собственным пространственным базисом
 *      (c)2020, -- ‏יְרוּשָׁלַיִם
 */
#include "..\Window\View.h"

struct Model:Base
{ Model():Base(){};
  Model& operator =( _Real Scale ){ Base::operator=(Scale);  return *this; }
  Model& operator =( _Point Rc   ){ Point::operator=(Rc);    return *this; }
  Model& operator+=( _Vector dR  ){ Point::operator+=(dR);   return *this; }
  Model& cubic( bool paint_Model=false ); // n>=2 - параллели; m>=3 - меридианы
  Model& torus ( _Real r,_Real R, int=33,int=24, bool=false );   // минимум:
  Model& sphere(                  int=33,int=24, bool=false );   // n=2, m=3
  Model& dipole( _Real acl=0.0,                  bool=false );   // {-1<s<+oo}
  operator Real(){ return (x*y)%z; }          //... определитель — объем триады
};
