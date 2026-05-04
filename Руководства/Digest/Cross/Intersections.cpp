using System.Windows;
namespace WpfAppCrossLines.MyCode
{ class Intersections
  { private Point Cross( double a1,double b1,double c1,double a2,double b2,double c2 )
    { /* решение: x = ( b1c2-c1b2 )/( a1b2-a2b1 )
                  y = ( a2c1-c2a1 )/( a1b2-a2b1 ) */
     Point pCross; pCross.X=( b1*c2-b2*c1 )/( a1*b2-a2*b1 );
                   pCross.Y=( a2*c1-a1*c2 )/( a1*b2-a2*b1 ); return pCross;
    }
    public bool LineLine( Point pA,Point pB,Point pC,Point pD, out Point pCross, out Info info )
    { info = new Info();
      /* Классическое уравнение прямой
         x - x1    y - y1
        ------- = -------
        x2 - x1   y2 - y1

       запишем в одну строку  x(y2-y1) + y(x1-x2) - x1y2 + y1x2 = 0
        a = (y2 - y1)
        b = (x1 - x2)
        c =(-x1y2 + y1x2)

       уравнение прямой с коэффициентами ax + by + c = 0
       если а b не равны 0, то

        a1x + b1y + c1 = 0
        a2x + b2y + c2 = 0

       решение
        x = (b1c2-c1b2)/(a1b2-a2b1)
        y = (a2c1-c2a1)/(a1b2-a2b1)

       Частный случай: прямая параллельна оси Х, тогда a = 0
            -- одна из прямых параллельна оси Y, тогда b = 0
       если и a=0 и b=0, то прямая не определена
      */
     double a1 =  pB.Y-pA.Y;
     double b1 =  pA.X-pB.X;
     double c1 = -pA.X*pB.Y + pA.Y*pB.X;

     double a2 =  pD.Y-pC.Y;
     double b2 =  pC.X-pD.X;
     double c2 = -pC.X*pD.Y + pC.Y*pD.X;

      /* До нахождения точки пересечения, определим состояние параллельности данных прямых
         Необходимым и достаточным условием параллельности двух прямых является:
           a1     b1
          ---- = ----
           a2     b2   в одну строчку a1*b2 - a2*b1 = 0
      */
      if( a1==0 && b1==0 )
      if( a2==0 && b2==0 ){ info.Id=10; info.Message="Обе прямые не определены";    return false; }
      if( a1==0 && b1==0 ){ info.Id=11; info.Message="Первая прямая не определена"; return false; }
      if( a2==0 && b2==0 ){ info.Id=12; info.Message="Вторая прямая не определена"; return false; }
      if( ( a1*b2-a2*b1 )==0 )
      {              info.Id=40; info.Message="Прямые параллельны";
        if( a1==0 ){ info.Id=41; info.Message="Прямые параллельны оси Х"; }
        if( b1==0 ){ info.Id=42; info.Message="Прямые параллельны оси Y"; }
        /* Прямые совпадают
           Необходимым и достаточным условием совпадения прямых является равенство:
           a1/a2 = b1/b2 = c1/c2
        */
        if( a1*b2==b1*a2 && a1*c2==a2*c1 && b1*c2==c1*b 2 )
          { info.Id=43; info.Message = "Прямые совпадают"; }
        return false;
      }

      pCross = Cross( a1,b1,c1,a2,b2,c2 ); ///  *** Прямые пересекаются ***

      /* Необходимым и достаточным условием перпендикулярности двух прямых является:
         a1a2 + b1b2 = 0
      */
      if( (a1*a2+b1*b2)==0 ){ info.Id=50; info.Message="Прямые перпендикулярны"; return true; }
      if( a1==0 ){ info.Id=60; info.Message="Первая прямая параллельна оси Х"; return true; }
      if( a2==0 ){ info.Id=61; info.Message="Вторая прямая параллельна оси Х"; return true; }
      if( b1==0 ){ info.Id=70; info.Message="Первая прямая параллельна оси Y"; return true; }
      if( b2==0 ){ info.Id=71; info.Message="Вторая прямая параллельна оси Y"; return true; }
                   info.Id=0;  info.Message="Общий случай"; return true;
    }
  }
  public class Info
  { public string Message;
    public int Id;
  }
}
