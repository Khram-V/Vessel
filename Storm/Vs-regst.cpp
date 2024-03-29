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

bool logTime(){ const Field &S=*Storm; if( !VIL || S.Kt<2 )return false; else
 return 0<fprintf( VIL,"\n%9s/%-4d",DtoA(Storm->Trun/3600,-3),Storm->Kt ); }
void logMeta(){ if( VIL ){ const Hull &S=*Vessel;
 fprintf(VIL,"\n  ⇒ Гидростатика: С{ x=%.1f, z=%.2f }, zG=%.2f, r=%.2f, h=%.2f"
 + logTime(), S.Buoyancy.x,S.Buoyancy.z,S.Gravity.z,
              S.Metacenter.z-S.Buoyancy.z,S.hX ); } }
void logHydro(){ if( VIL ){ const byte St=Vessel->Statum;
     fprintf(VIL,"\n  ⇒ Гидромеханика[%d]: %s"+logTime(),St,Model[St]); } }
void logDamp(){ if( VIL ){ _Vector F=Vessel->muF,M=Vessel->muM;
fprintf( VIL,"\n  ⇒ Демпфирование: μV{ ξ=%4.2lf, η=%4.2lf, ζ=%4.2lf };"
  " μω{ θ=%4.2lf, ψ=%4.2lf, χ=%4.2lf }, Kv=%0.2lg"+logTime(),
        F.x,F.y,F.z,M.x,M.y,M.z,Vessel->Kv ); } }
void Model_Config( Window* Win ){ byte &St=Vessel->Statum,ans=St;
  Mlist Menu[]={ {1,0,"  Выбор модели гидромеханики корабля"},{2,45,Model[0]},
             {1,45,Model[1]},{1,45,Model[2]},{1,45,Model[3]},{1,45,Model[4]} };
  TextMenu T( Mlist( Menu ),Win,47,1 );
  if( (ans=T.Answer( ans+1 )-1)==_Esc-1 )ans=St;
  if( ans!=St ){ St=ans; logHydro(); }
}
//  блок протокола по экстремальным событиям в параметрах качки корпуса корабля
//
static bool shortEx=false;
static bool extFix( Real *W, Real c ){ bool ret=false; c=e5( c );
  if( W[0]!=W[1] )if( W[1]==c || (c-W[1])*(W[1]-W[0])<0.0 ) // экстремум вперёд
  { if( fabs( c )>fabs( W[2] ) ){ W[2]=c; ret=true; } else ret=!shortEx;
  } W[0]=W[1]; W[1]=c; return ret;
}
static void PtoG( char *s )
{ for( int i=0; s[i]; i++ )if( s[i]=='.' )Uset( s+i,0,"°" );        // градусы°
}
#include <unistd.h>
void logStop();
//!   регистрация экстремальных событий в ходе вычислительно эксперимента
//    смещение { ξ η ζ }м, углы поворота { ϑ ψ χ }°
//    〈•〉-экстремум;  〈·〉-сопутствующие отсчеты
//    〈П÷Л〉 Б-〈П÷Л〉на борт (π/30); Р-руль〈П÷Л〉 полборта (π/60);
//         М-помалу〈П÷Л〉=авторулевой (π/120)
//                                                              4 32 64 128
static Real wV[24],                                       // W{ x=8 y=16 z=2 }
           *wZ=wV+3,*wA=wV+6,*wM=wV+9,*wF=wV+12,*wC=wV+15,*wX=wV+18,*wY=wV+21;
void logStop()
{ if( !VIL )return;
  fprintf( VIL,"\n\n << успешное завершение >>" ); logTime();
  if( Vessel->Educt&0xFF )fprintf( VIL,
       "  ⇒ ±ξ%+4.2f χ%+4.2f ζ%+4.2f ϑ%+4.2f ψ%+4.2f « %+4.2f %+4.2f %+4.2f »",
                wV[2],-wC[2]*_Rd,wZ[2],wX[2]*_Rd,wY[2]*_Rd,wA[2],wM[2],wF[2] );
  fprintf( VIL,"\n" ); ftruncate( fileno(VIL),ftell(VIL) ); fclose(VIL); VIL=0;
}
Hull& Hull::Protocol()
{ if( VIL && Educt&0xFF )
  { Field &S=*Storm; Vector Dir=Swing[-1];         // в третьей точке экстремум
    if( S.Kt<2 )                                   // ... { ξ η ζ }м,{ ϑ ψ χ }°
    { fprintf( VIL,
      "\n  ⇒ Время Kt      скорость,узл. курс±рыскание руль"
      " Z миделя∫волн бортовая килевая « корма мидель нос »/g" );
      memset( wV,0,sizeof( wV ) );
    } else
    { byte ev=0;
     Real w=Course+Dir.z,a,m,f,v=(cSp-Speed)*3600.0/_Mile, // +отличие скорости хода в узлах
          w1=Route[-1].z,w2=Route[-2].z,w3=Route[-3].z,
          s1=Length*sin( Swing[-1].y )/2, s2=Length*sin( Swing[-2].y )/2,
          s3=Length*sin( Swing[-3].y )/2, d=S.Tstep/S.tKrat; d=_g*d*d;
      m = ( w1 - 2*w2 + w3 )/d;          // расчетный шаг в циклах эксперимента
      a = ( w1+s1 -2*(w2+s2) + w3+s3 )/d;
      f = ( w1-s1 -2*(w2-s2) + w3-s3 )/d;
      shortEx = (Educt&0x200)!=0;    // режим записи только превышающих величин
      if( extFix( wV,v )       )if( Educt&1  )ev|=1;   // потеря скорости хода
      if( extFix( wC,angle(w)) )if( Educt&2  )ev|=2;   // рыскание
      if( extFix( wZ,Z )       )if( Educt&4  )ev|=4;   // вертикальная,
      if( extFix( wX,Dir.x )   )if( Educt&8  )ev|=8;   // бортовая и
      if( extFix( wY,Dir.y )   )if( Educt&16 )ev|=16;  // килевая качка
      if( extFix( wA,a )       )if( Educt&32 )ev|=32;  // ускорение ахтерштевня
      if( extFix( wM,m )       )if( Educt&64 )ev|=64;  // ускорение на миделе
      if( extFix( wF,f )       )if( Educt&128)ev|=128; // и вблизи форштевня
      if( ev )
      { static char str[64]; int i; logTime();  // сначала время, отсчеты шагов
        sprintf( str,"  ⇒ %s±ξ%5.1f%-+5.1f %s±χ%6.1f%-+6.1f",
                 ev&1?"•":"·",Speed*3600.0/_Mile,v,
                 ev&2?"•":"·",Course*_Rd,-w*_Rd ); PtoG( str+24 );
        fprintf( VIL,str );// Б-〈П÷Л〉на борт; Р-руль〈П÷Л〉 полборта; М-помалу〈П÷Л〉
        if( fabs( w )<_Pi/32.0 )fprintf( VIL,"↨ " ); else
        { fprintf( VIL,w>0?"п":"л" );
          fprintf( VIL, dCs>_Pi/59?"Б" : dCs<_Pi/61?"M":"P" );
        }
        sprintf( str," %sζ%+4.1f∫%-+4.1f ",     // вертикальная качка и уровень
                 ev&4?"•":"·",Z,S.Value( out( Zero ) ) ); i=Ulen( str );
        fprintf( VIL,str ); while( ++i<15 )fprintf( VIL,"_" );
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
    textcolor( LIGHTCYAN ); cprint( 2,y,"%-5s: ",W.Title );
    textcolor( LIGHTGRAY ); cprint( 9,y,y==10
  ? "L=%3.0f м, T=%4.1f\", A=%4.2f м, C=%4.1f м/с, Dir=%3.0f°, Ds=%.1f м [%d·%d] = {%.0f·%.0f} м\n"
  : "  %3.0f м,   %4.1f\",   %4.2f м,   %4.1f м/с,     %3.0f°,    %.1f м [%d·%d] \n",
     W.Length,W.Length/W.Cw,W.Height,W.Cw,(_Pi-atan2(W.x.y,W.x.x))*_Rd,
     W.Ds,W.Mx,W.My,S.Long,S.Wide );
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
  : "  %3.0f м,   %4.1f\",   %4.2f м,  =%4.2f,   %4.1f м/с,   %3.0f°,    %.1f м [%d·%d]\n",
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
  cprint( 1,14," Time %s + %3.2f\"      \n"
               " Speed %3.1fуз(%4.2f=%3.1fL)  \n"
               " Volume %1.0f << %1.0f  \n"
               " Surface %1.0f << %1.0f  \n"
               " Floatable %1.0f << %1.0f  \n"
               " μCenter %4.1f >> %4.1f -- %3.1f Gravity.z  \n"
               "       h %4.2f >> %4.2f -- μM %3.1f >> %3.1f   ",
        DtoA(F.Kt*F.Tstep/F.tKrat/3600,3),F.Tstep,Speed*3600/_Mile,
        Speed/sqrt(_g*Length),sqr(Speed)*_Pd/_g/Length,Volume,iV,Surface,iS,
        Floatage,iF,Metacenter.x,vM.x,Gravity.z,hX,vM.z,Metacenter.y,vM.y ),
  cprint( 80,8,"Statum{ %X } ",Statum ), // printB( Statum );
  textcolor( WHITE ),cprint( 28,14,"C:{%6.1f,%.1f,%-6.1f}  ",vB.x,vB.y,vB.z );
  cprint( 55,14,"mC:| %5.2f %-5.2f %-5.2f | X %s  ",x.x,x.y,x.z,DtoA(atan2(-z.y,z.z)*_Rd,-1)),
  cprint( 58,15,   "| %5.2f %-5.2f %-5.2f | Y %s  ",y.x,y.y,y.z,DtoA(atan2( x.z,x.x)*_Rd,-1)),
  cprint( 58,16,   "| %5.2f %-5.2f %-5.2f | Z %s  ",z.x,z.y,z.z,DtoA(atan2(-x.y,x.x)*_Rd,-1));
  textcolor( LIGHTCYAN ),
  cprint( 27,15,"vF:{%6.1f,%.1f,%-6.1f}  ", vF.x,vF.y,vF.z ),
  cprint( 27,16,"vW:{%6.1f,%.1f,%-6.1f}  ", vW.x,vW.y,vW.z ),
  cprint( 27,17,"vD:{%6.1f,%.1f,%-6.1f}  ", vD.x,vD.y,vD.z ),
//cprint( 27,17,"vS:{%6.1f,%.1f,%-5.1f }  ",vS.x,vS.y,vS.z ),
  textcolor( LIGHTGREEN,BLUE ),
  cprint( 55,17,"mW:| %7.0f  %-9.1f | ",mW.x.x,mW.x.y ),
  cprint( 58,18,   "| %7.1f  %-9.0f | ",mW.y.x,mW.y.y ),
//cprint( 55,17,"mM:| %7.0f  %-8.1f  %-9.1f | ",mM.x.x,mM.x.y,mM.x.z ),
//cprint( 58,18,   "| %7.1f  %-8.0f  %-9.1f | ",mM.y.x,mM.y.y,mM.y.z ),
//cprint( 58,19,   "| %7.1f  %-8.1f  %-9.0f | ",mM.z.x,mM.z.y,mM.z.z ),
  textcolor( LIGHTGRAY,BLACK ),
  cprint( 51,19,"inMass:"
                "| %7.0f  %-8.1f  %-9.1f | ",inMass.x.x,inMass.x.y,inMass.x.z),
  cprint( 58,20,"| %7.1f  %-8.0f  %-9.1f | ",inMass.y.x,inMass.y.y,inMass.y.z),
  cprint( 58,21,"| %7.1f  %-8.1f  %-9.0f | ",inMass.z.x,inMass.z.y,inMass.z.z),
  textcolor( GREEN );
  if( Statum )cprint( 55,22,"vS:{%6.1f,%.1f,%-6.1f}  ",vS.x,vS.y,vS.z ),
              cprint( 55,23,"vR:{%6.1f,%.1f,%-6.1f}  ",vR.x,vR.y,vR.z );
  if( log )
  { textcolor( WHITE ); cprint( 1,7,"  >>> %s \n"
      "  >>> { L=%g, B=%g, T=%g, Ψ=%s\\δd=%.0fсм }^%g  №〈A.%d<%+d+>%d.Ф 〉 ",
      ShipName,Length,Breadth,Draught,DtoA(Trim*_Rd,2),asin(Trim)*Length*50,
      Ofs.z-Draught,Stern.len,Nframes,Stem.len );
                textcolor( CYAN ); cprint( 1,38,fname( fext( FileName ) ) );
    if( !VIL ){ textcolor( MAGENTA ); printf( " <= без протокола" ); } else
    if( Educt&0xFF )
    { textcolor( GREEN ); printf( " <=%s: %s%s; %s%s%s{%s%s%s}",
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
