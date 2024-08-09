//!
//! проект Aurora - штормовой и экстремальный ход корабля
//   ┌───────────────────────────────────────────────╖
//   │  ↓  корпус ─> волны ─> графика                ║
//   │  ↓  статика ─> динамика ─> реакции            ║
//   │  ↨  кинематика ─> излучение ─> гидромеханика  ║
//   ╞═══════════════════════════════════════════════╝
//   └── ©2018-23 Санкт-Петербург — Южно-Сахалинск — Могилев — Друть — ‏יְרוּשָׁלַיִם
//
#include "Aurora.h"

bool logTime( bool next )// запрос корректности текущего времени, или шаг назад
{ const Field &S=*Storm;
  if( !VIL || S.Kt<2 )return false; else
  { Real T=S.Trun; if( !next )T -= Ts; // S.Tstep/S.tKrat;
    return 0<fprintf( VIL,T<60?"\n   %-6s\\%-4d":"\n%-9s/%-4d",
                      DtoA( T/3600,-3 ),S.Kt-(!next) );
} }
void logMeta(){ if( VIL ){ const Hull &S=*Vessel;
 fprintf(VIL,"\n  ⇒ Гидростатика: С{ x=%.1f, z=%.2f }, zG=%.2f, r=%.2f, h=%.2f"
  + logTime(), S.Buoyancy.x,S.Buoyancy.z,S.Gravity.z,
               S.Metacenter.z-S.Buoyancy.z,S.hX ); } }
void logHydro(){ if( VIL ){ const byte St=Vessel->Statum;
     fprintf(VIL,"\n  ⇒ Гидромеханика[%d]: %s"+logTime(),St,Model[St]); } }
void logDamp()
{ Hull &H=*Vessel; H.DampInit(); textcolor( BROWN );
  print(56,22,"μ√:│ ξ%.1f∫%.2f η%.1f∫%.2f ζ%.1f∫%.2f │ ",H.muF.x,H.nF.x,H.muF.y,H.nF.y,H.muF.z,H.nF.z );
  print(57,23, "ω:│ ϑ%.1f∫%.2f ψ%.1f∫%.2f χ%.1f∫%.2f │ ",H.muM.x,H.nM.x,H.muM.y,H.nM.y,H.muM.z,H.nM.z );
  if( VIL )
  { fprintf( VIL,"\n  ⇒ Демпфирование: μV{ ξ=%4.2lf, η=%4.2lf, ζ=%4.2lf };"
    " μω{ θ=%4.2lf, ψ=%4.2lf, χ=%4.2lf }, Kv=%0.2lg"+logTime(),
        H.muF.x,H.muF.y,H.muF.z,H.muM.x,H.muM.y,H.muM.z,H.Kv );
} }
void Model_Config( Window* Win ){ byte &St=Vessel->Statum,ans=St;
  Mlist Menu[]={ {1,0,"  Выбор модели гидромеханики корабля"},{2,45,Model[0]},
             {1,45,Model[1]},{1,45,Model[2]},{1,45,Model[3]},{1,45,Model[4]} };
  TextMenu T( Mlist( Menu ),Win,47,1 );
  if( (ans=T.Answer( ans+1 )-1)==_Esc-1 )ans=St;
  if( ans!=St ){ St=ans; logHydro(); }
}
//  блок протокола по экстремальным событиям в параметрах качки корпуса корабля
//!                                       максимум всегда на предыдущем отсчёте
static bool shortEx=false;
static bool extFix( Real *W, Real &c )
{ bool ret=false; e6( c );
  if( c!=W[1] )                       // сопоставление с предыдущей величиной
  if( (c-W[1])*(W[1]-W[0])<=0.0 )     // экстремум на предыдущем отсчете W[1]
  { ret=!shortEx;                     // будет включен по полному списку всегда
    if( c>W[1] ){ if( W[1]<W[2] ){ W[2]=W[1]; ret=true; } } else // минимум [2]
    if( c<W[1] ){ if( W[1]>W[3] ){ W[3]=W[1]; ret=true; } }      // максимум[3]
  } W[4]+=c; W[0]=W[1]; W[1]=c; c=W[0]; return ret;
}
static void PtoG( char *s )
{ for( int i=0; s[i]; i++ )if( s[i]=='.' )Uset( s+i,0,"°" );        // градусы°
}
#include <unistd.h>
//!   регистрация экстремальных событий в ходе вычислительно эксперимента
//    Текущая величинах[0],prev[1], min[2],max[3], SUM[4]
//    смещение { ξ η ζ }м, углы поворота { ϑ ψ χ }°
//    〈•〉-экстремум;  〈·〉-сопутствующие отсчеты
//    〈П÷Л〉 Б-〈П÷Л〉на борт (π/30); Р-руль〈П÷Л〉 полборта (π/60);
//                           М-помалу〈П÷Л〉=авторулевой (π/120)
//                                                               4 32 64 128
static Real wV[40],                                        // W{ x=8 y=16 z=2 }
          *wZ=wV+5,*wA=wV+10,*wM=wV+15,*wF=wV+20,*wC=wV+25,*wX=wV+30,*wY=wV+35;
static long KS=0;                             // счетчик записей под осреднение
static bool mV=false;                         // признак минимума скорости хода
void logStop()
{ if( !VIL )return; logTime(); fprintf( VIL,"  ⇒ << успешное завершение >>" );
  if( KS>0 && Vessel->Educt&0xFF )
  { Real RS=Real( KS )/_Rd; fprintf( VIL,"\n\t"
     "скорость хода      ±ξ[м/с]%6.2f < %+4.2f > %-+4.2f\n\t"
     "рыскание на курсе  ±χ[°] %7.2f < %+4.2f > %-+4.2f\n\t"
     "вертикальная качка ±ζ[м] %7.2f < %+4.2f > %-+4.2f\n\t"
     "бортовая качка     ±ϑ[°] %7.2f < %+4.2f > %-+4.2f\n\t"
     "килевая качка      ±ψ[°] %7.2f < %+4.2f > %-+4.2f\n\t"
     "ускорения по корме {a/g} %+7.2f < %+4.2f > %-+5.2f\n\t"
     "[м/с²/g] на миделе {m/g} %+7.2f < %+4.2f > %-+5.2f\n\t"
     "  — —  у форштевня {f/g} %+7.2f < %+4.2f > %-+5.2f",
      wV[2],     wV[4]/KS, wV[3],
     -wC[2]*_Rd,-wC[4]/RS,-wC[3]*_Rd,
      wZ[2],     wZ[4]/KS, wZ[3],
      wX[2]*_Rd, wX[4]/RS, wX[3]*_Rd,
      wY[2]*_Rd, wY[4]/RS, wY[3]*_Rd,
      wA[2],wA[4]/KS,wA[3],wM[2],wM[4]/KS,wM[3],wF[2],wF[4]/KS,wF[3] );
  } fprintf( VIL,"\n" ); ftruncate(fileno(VIL),ftell(VIL)); fclose(VIL); VIL=0;
}
Hull& Hull::Protocol()
{ if( VIL && Educt&0xFF )
  { Field &S=*Storm; Vector Dir=Head[-1];          // в третьей точке экстремум
    if( S.Kt<2 )                                   // ... { ξ η ζ }м,{ ϑ ψ χ }°
    { fprintf( VIL,
      "\n  ⇒ Время\\Kt    ⇒ Скорость±δ,узл курс±рыскание руль"
      " Z миделя∫волн бортовая килевая « корма мидель нос »/g" ); KS=0;
      memset( wV,0,sizeof( wV ) );                                mV=false;
    } else { byte ev=0;                       // +отличие скорости хода в узлах
     Real w=Course+Dir.z,a=0,m=0,f=0,V=Speed*3600.0/_Mile; //,v=cSp*3600.0/_Mile-V;
      KS++;
      if( S.Kt>2 )
      { Real w1=Route[-1].z,w2=Route[-2].z,w3=Route[-3].z,
             s1=Length*sin( Head[-1].y )/2, s2=Length*sin( Head[-2].y )/2,
             s3=Length*sin( Head[-3].y )/2, d=Ts; d=_g*d*d;
        m = ( w1 - 2*w2 + w3 )/d;        // расчетный шаг в циклах эксперимента
        a = ( w1+s1 -2*(w2+s2) + w3+s3 )/d;
        f = ( w1-s1 -2*(w2-s2) + w3-s3 )/d;
      } shortEx = (Educt&0x200)!=0;  // режим записи только превышающих величин
      if( extFix( wV,V )       )if( Educt&1  )ev|=1;   // потеря скорости хода
      if( extFix( wC,angle(w)) )if( Educt&2  )ev|=2;   // рыскание
      if( extFix( wZ,Z )       )if( Educt&4  )ev|=4;   // вертикальная,
      if( extFix( wX,Dir.x )   )if( Educt&8  )ev|=8;   // бортовая и
      if( extFix( wY,Dir.y )   )if( Educt&16 )ev|=16;  // килевая качка
      if( extFix( wA,a )       )if( Educt&32 )ev|=32;  // ускорение ахтерштевня
      if( extFix( wM,m )       )if( Educt&64 )ev|=64;  // ускорение на миделе
      if( extFix( wF,f )       )if( Educt&128)ev|=128; // и вблизи форштевня
      if( !mV && V && ev&1 )wV[4]=(wV[2]=V)*S.Kt,mV=true; // усреднение разгона
      if( ev )
      { static char str[64]; int i; logTime( false ); // сначала время, отсчеты
        sprintf( str,"  ⇒ %sV±ξ%5.1f%-+5.1f %s±χ%6.1f%-+6.1f",
           ev&1?"•":"·",V,cSp*3600.0/_Mile-V,
           ev&2?"•":"·", Course*_Rd,-w*_Rd ); PtoG( str+24 );
        fprintf( VIL,str );// Б-〈П÷Л〉на борт; Р-руль〈П÷Л〉полборта; М-помалу〈П÷Л〉
        if( fabs( w )<_Pi/32.0 )fprintf( VIL,"↨ " ); else
        { fprintf( VIL,w>0?"п":"л" );
          fprintf( VIL,dCs>_Pi/59?"Б" : dCs<_Pi/61?"M":"P" );
        }
        sprintf( str," %sζ%+4.1f∫%-+4.1f ",     // вертикальная качка и уровень
                 ev&4?"•":"·",Z,S.Value( out( Zero ) ) ); i=Ulen( str );
        fprintf( VIL,str ); while( ++i<15 )fprintf( VIL," " );    // ++ пробелы
        sprintf( str," %sϑ%-+6.1f %sψ%-+6.1f ",                   // углы качки
                 ev&8?"•":"·",Dir.x*_Rd,ev&16?"•":"·",Dir.y*_Rd ); PtoG( str );
        fprintf( VIL,"%s« %s%+4.1f %s%+4.1f %s%-+5.1f »",str,
                 ev&32?"•":"·",a,ev&64?"•":"·",m,ev&128?"•":"·",f );
  } } } return *this;
}
//  Блок текущей консольной текстовой информации о штормовом море и корабле
//
void wavePrint()                // процедура печати используется для обновления
{ Field &S=*Storm;              //  при изменении параметров волн Field::Config
  for( int y=10; y<=12; y++ )
  { Waves &W = y==10 ? S.Wind : ( y==11 ? S.Swell : S.Surge );
    textcolor( LIGHTCYAN ); print( 2,y,"%-5s: ",W.Title );
    textcolor( LIGHTGRAY ); print( 9,y,y==10
  ? "λ=%3.0f м, τ=%4.1f\", ζ=%4.1f/%4.2g м, C=%4.1f м/с, A=%3.0f°, δS=%.1f м [%d·%d]={%.0f·%.0f} м\n"
  : "  %3.0f м,   %4.1f\",   %4.1f/%4.2g м,   %4.1f м/с,   %3.0f°,    %.1f м [%d·%d] \n",
     W.Length,W.Length/W.Cw,W.Height,hW*W.Height/W.Length,
     W.Cw,(_Pi-atan2(W.x.y,W.x.x))*_Rd,W.Ds,W.Mx,W.My,S.Long,S.Wide );
} }
void logWave()
{ wavePrint(); if(!VIL )return; // процедура печати используется для обновления
 Field &S=*Storm; logTime();    //  отметка момента времени опытовых испытаний
  fprintf( VIL,"\n" );          //  при изменении параметров волн Field::Config
  for( int y=0; y<=2; y++ )
  { Waves &W = !y?S.Wind:(y==1?S.Swell:S.Surge);
    fprintf( VIL,"   %s: "+(2-y),W.Title );
    fprintf( VIL,!y
  ? "λ=%3.0f м, τ=%4.1f\", ζ=%4.2f м, α=%4.2f, C=%4.1f м/с, A=%3.0f°, δS=%.1f м [%d·%d]\n"
  : "  %3.0f м,   %4.1f\",   %4.2f м,   %4.2f,   %4.1f м/с,   %3.0f°,    %.1f м [%d·%d]\n",
     W.Length,W.Length/W.Cw,W.Height,hW*W.Height/W.Length,
     W.Cw,(_Pi-atan2(W.x.y,W.x.x))*_Rd,W.Ds,W.Mx,W.My );
  }
  fprintf( VIL,"  ⇒ %s (%s)\n",ModelWave[S.Exp.wave],S.Exp.peak
             ? "трохоидальный процесс" : "поверхностный поток" );
}
//inline void printB(unsigned B){ if(B>>1)printB(B>>1); putch((B&1)?'1':'0'); }

Hull& Hull::wPrint( bool log ) // информация по смоченному корпусу на текстовой
{ Field &F=*Storm;            //             консоли и в протоколе эксперимента
  textcolor( YELLOW ),       // текстовые данные о состоянии и динамике корабля
  print( 1,14," Time%s +%.2g\"/%.3g \n"
               " Speed %3.1fуз(%4.2f=%3.1fL)  \n"
               " Volume %1.0f << %1.0f  \n"
               " Surface %1.0f << %1.0f  \n"
               " Floatable %1.0f << %1.0f  \n\n"
               " μCenter %4.1f >> %4.1f -- Gravity.z=%+3.1f  \n"
               "       h %4.2f >> %4.2f -- H %3.1f >> %3.1f   ",
        DtoA( F.Trun/3600,F.Trun>3600?2:(F.Trun>60?3:-3) ),
        TimeStep,tKrat,Speed*3600/_Mile,
        Speed/sqrt(_g*Length),sqr(Speed)*_Pd/_g/Length,Volume,iV,Surface,iS,
        Floatage,iF,Metacenter.x,vM.x,Gravity.z,hX,vM.z,Metacenter.y,vM.y ),
  print( 80,8,"Statum{ %X } ",Statum ), // printB( Statum );
  print( 28,13, "P:{%6.1f,%.1f,%-6.1f}  ",vP.x,vP.y,vP.z );
  textcolor( WHITE );
  print( 28,14, "C:{%6.1f,%.1f,%-6.1f}  ",vB.x,vB.y,vB.z );
  textcolor( LIGHTCYAN ),
  print( 27,15,"vF:{%6.1f,%.1f,%-6.1f}  ",vF.x,vF.y,vF.z ),
  print( 27,16,"vW:{%6.1f,%.1f,%-6.1f}  ",vW.x,vW.y,vW.z ),
  print( 27,17,"vD:{%6.1f,%.1f,%-6.1f}  ",vD.x,vD.y,vD.z );
  if( Statum )textcolor( GREEN ),
              print( 27,18,"vS:{%6.1f,%.1f,%-6.1f}  ",vS.x,vS.y,vS.z ),
              print( 27,19,"vR:{%6.1f,%.1f,%-6.1f}  ",vR.x,vR.y,vR.z );
  textcolor( LIGHTGREEN,BLUE ),
  print( 56,20,"mW:| %7.0f  %-9.1f | ",mW.x.x,mW.x.y ),
  print( 59,21,   "| %7.1f  %-9.0f | ",mW.y.x,mW.y.y ),
//print( 56,17,"mM:| %7.0f  %-8.1f  %-9.1f | ",mM.x.x,mM.x.y,mM.x.z ),
//print( 59,18,   "| %7.1f  %-8.0f  %-9.1f | ",mM.y.x,mM.y.y,mM.y.z ),
//print( 59,19,   "| %7.1f  %-8.1f  %-9.0f | ",mM.z.x,mM.z.y,mM.z.z ),
  textcolor( WHITE,BLACK );
  print( 56,14,"mC:| %5.2f %-5.2f %-5.2f | X %s  ",x.x,x.y,x.z,DtoA(atan2(-z.y,z.z)*_Rd,-1)),
  print( 59,15,   "| %5.2f %-5.2f %-5.2f | Y %s  ",y.x,y.y,y.z,DtoA(atan2( x.z,x.x)*_Rd,-1)),
  print( 59,16,   "| %5.2f %-5.2f %-5.2f | Z %s  ",z.x,z.y,z.z,DtoA(atan2(-x.y,x.x)*_Rd,-1));
  if( log )
  { char *s=DtoA( Trim*_Rd,2 ); while( *s==' ' )++s; logDamp();
    textcolor( WHITE ); print( 1,7,"  >>> %s \n"
      "  >>> { L=%g, B=%g, T=%g, Ψ=%s\\δd≈%.0fсм }∧%g  №〈A.%d<%+d+>%d.Ф 〉∨%g ",
         ShipName,Length,Breadth,Draught,s,asin( Trim )*Length*50,
         Ofs.z-Draught,Stern.len,Nframes,Stem.len,Kv );
    print(52,17,"inMass:"
                "│ %7.0f  %-8.1f  %-9.1f │ ",inMass.x.x,inMass.x.y,inMass.x.z),
    print(59,18,"│ %7.1f  %-8.0f  %-9.1f │ ",inMass.y.x,inMass.y.y,inMass.y.z),
    print(59,19,"│ %7.1f  %-8.1f  %-9.0f │ ",inMass.z.x,inMass.z.y,inMass.z.z);

    textcolor( CYAN ); print( 1,38,fname( fext( FileName ) ) );
    if( !VIL ){ textcolor( MAGENTA ); print( " <= без протокола" ); } else
    if( Educt&0xFF )
    { textcolor( GREEN ); print( " <=%s: %s%s; %s%s%s{%s%s%s}",
      Educt&0x200?"кратко":"полно",Educt&1?" ход,":"",Educt&2?" курс":"",
      Educt&4?" вертикаль,":"",Educt&8?" борт,":"",Educt&16?" киль ":"",
      Educt&32?"корма,":"",Educt&64?"мидель,":"",Educt&128?"нос":"" );
      logTime(); fprintf( VIL,"\n\n  ⇒ %s \n"
      "  ⇒ { L=%g, B=%g, T=%g, Ψ=%s\\δd=%.0fсм }^%g  №〈а.%d<шп[%d]>%d.ф〉\n"
      "  ⇒ { δ=%.2f, W=%.1f m³, S=%.1f m², F=%.1f m² }\n"
      "  ⇒ С{ x=%.1f, z=%.2f }, zG=%.2f, r=%.2f, h=%.2f [м]",
      ShipName,Length,Breadth,Draught,DtoA(Trim*_Rd,2),asin(Trim)*Length*50,
      Ofs.z-Draught,Stern.len,Nframes,Stem.len,Volume/Length/Breadth/Draught,
      Volume,Surface,Floatage,Buoyancy.x,Buoyancy.z,Gravity.z,
      Metacenter.z-Buoyancy.z,hX );
      if( F.Kt<2 )logDamp(),logHydro(); fprintf( VIL,"\n" );
  } } return *this;
}
