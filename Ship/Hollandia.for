C
C     Программа по тематике Проектирования судов кафедры Теории корабля
C       Ленинградской корабелки, из работ Леонида Сергеевича Артюшкова,
C       исполнена студентом-дипломником Дмитрием Азаровским (Германия)
C
      real IE,K2,KS,KV,L,LPP
      dimension RT(450)
      open(1,file='Hollandia.dat')
      open(6,file='Hollandia.out')
      read (1,1) L,  LPP, B,   TF,  T,   CB,
     2           CM, CWP, CST, XB,  ABT, HB,
     3           AT, SAP, CBT, DPU, K2,  KS,
     4           RO, KV,  IE,  S,   CA
      read (1,2) N1,N2,N3,NP1,NP2,NP3,NS
      call TK01RU(L,LPP,B,TF,T,CB,CM,CWP,CST,XB,ABT,HB,AT,SAP,CBT,
     *            DPU,K2,KS,RO,KV,IE,S,CA,RT,N1,N2,N3,NS,NP1,NP2,NP3)
    1 format( 6F10.3 )
    2 format( 3I5/3I2/I3 )
      stop
      end
C
C     Программа составлена по заданию Леонида Сергеевича Артюшкова
C
      subroutine TK01RU( L,LPP,B,TF,T,CB,CM,CWP,CST,XB,ABT,HB,AT,
     *     SAP,CBT,DPU,K2,KS,RO,KV,IE,S,CA,RT,N1,N2,N3,NS,NP1,NP2,NP3 )

      real IE,K1,K2,KS,KV,L,LB,LCB,LD,LE,LM,LPMB,LPP,LR,LR1,LT,M1,M2,M60
      dimension VS(450),CTR(450),CBB(450),CF(450),RV(450),RW(450),
     *          RAP(450),RTR(450),RA(450),RF(450),RB(450),RT(450)
      print 6

      if( NS .ne. 60 )goto 105
C
C                Вычисление  CM  и  CWP
C
      DEL=CB*L*B*T
      LB=L/B
      BT=B/T
      CM=.08*CB+.93
      CP=CB/CM
      XB=20.*(CB-.675)
      CB2=CB*CB
      XB1=0.0
      if( CB.le.7. ) then
        LPMB=LPP*(3.402-11.55*CB+9.8*CB2)
        A1=-1.3201+6.3692*CB-5.5586*CB2
        A2=.3973-1.3116*CB+1.0823*CB2
        M60=-.1548+.7136*CB-.6327*CB2
        C11=-.0908+.61*CB-.7596*CB2
      else
        LPMB=LPP*(1.81*CB-1.148)
        A1=.9803-.6424*CB-.2368*CB2
        A2=.0531-.1172*CB+.0786*CB2
        M60=.2297-.6321*CB+.5051*CB2
        C11=-.5466+1.5372*CB-1.1539*CB2
      end if
      LE=LPP*(A1-A2*XB1)
      LR1=LPP-LPMB-LE
      CPR=(CP*LPP-LPMB-M60*LE*XB1-C11*LE)/(LE+LR1)
      CPE=C11+CPR+M60*XB1
      CPF=2.*(CPE*LE+LPP*.5-LE)/LPP
      CPA=2.*(CPR*LR1+LPP*.5-LR1)/LPP
      CWF=.09057+.9191*CPF
      CWA=.94008-1.04496*CPA+1.25341*CPA*CPA
      CWP=(CWA+CWF)/2.
      if( S.LT..000001 )S=DEL**(2./3.)*(3.432+.305*LB+.443*BT-.643*CB)
      LPMB=LPMB/LPP
      LE=LE/LPP
      LR1=LR1/LPP
  105 continue
c
      if( NS.ne.60 ) then
        DEL=CB*L*B*T
        LB=L/B
        BT=B/T
        if( S.LT.0.000001 )S = L*( 2.0*T+B )*sqrt( CM )
     *   * (.453+.4425*CB-.2862*CM-.003467*BT+.3696*CWP) + 2.38*ABT/CB
        CP=CB/CM
      end if
c
      G=9.8106
      TL=T/L
      LT=L/T
      TB=T/B
      BL=B/L
      TFL=TF/L
      PI=3.1415926
      LD=L**3/DEL
C
      if( NP1.ne.0 ) then
        print 50,L,LPP,B,TF,T
        print 52,CB,CM,CWP,CST,DEL,XB
        print 54,ABT,HB,AT,SAP,CBT,DPU
        print 56,K2,KS,RO,KV
        print 58,N1,N2,N3,NP1,NP2,NP3,NS
      end if
C
      if( NS .eq. 60 .and. NP2 .eq. 1 )print 110,
     &    LPMB,LE,LR1,A1,A2,M60,C11,CPR,CPE,CPF,CPA,XB1,CWF,CWA
C
      LCB=(XB*LPP/100.+.5*(L-LPP))*100./L
      if(4.*CP-1..LE..00001) goto 11
      LR=(1.-CP+.06*CP*LCB/(4.*CP-1.))*L
      if(TL.GE..05) C12=TL**.2228446
      if(TL.LT..05) C12=48.2*(TL-.02)**2.078+.479948
      if(TL.LT..02) C12=.479948
      C13=1.+.003*CST
      K1=C13*(.93+C12*(B/LR)**.92497*(.95-CP)**(-.521448)*(1.-CP+.0225*
     *LCB)**.6906)
      if( BL .gt. .25) C7=.5-.0625*LB
      if( BL .le. .25) C7=BL
      if( BL .le. .11) C7=.229577*BL**.33333
      if( IE .le. .000001)
     1  IE=1.+89.*EXP(-LB**.80856*(1.-CWP)**.30484*(1.-CP-.0225*LCB)**
     2  0.6367*(LR/B)**.34574*(100.*DEL/L**3)**.16302)
      C1=2223105.*C7**3.78613*TB**1.07961*(90.-IE)**(-1.37565)
      C3=.56*ABT**1.5/(B*T*(.31*SQRT(ABT)+TF-HB))
      C2=EXP(-1.89*SQRT(C3))
      C5=1.-.8*AT/(B*T*CM)
      if(LB.LT.12.)LM=1.446*CP-.03*LB
      if(LB.GE.12.)LM=1.446*CP-.36
      if(CP.LT.0.8)C14=8.07981*CP-13.8673*CP*CP+6.984388*CP**3
      if(CP.GE.0.8)C14=1.73014-.7067*CP
      M1=.0140407*LT-1.75254*DEL**(1./3.)/L-4.79323*BL-C14
      if(LD.GT.1727.)C15=0.0
      if(LD.LE.1727.)C15=-1.69385+(L/DEL**(1./3.)-8.)/2.36
      if(LD.LE.512.0)C15=-1.69385
      D1=-.9
      if(TF-1.5*HB.LE..00001) goto 12
      PB=.56*SQRT(ABT)/(TF-1.5*HB)
      if(TFL.LE..04) C4=TFL
      if(TFL.GT..04) C4=.04

      if( CA .ge. 0.9 ) then
        CA1 = 0.006*(L+100.)**(-0.16)
     &      - 0.00205+.003*SQRT(L/7.5)*CB**4*C2*(0.04-C4)
        if( KS .GT. 0.15e-3 )CA2=(.105*KS**(1./3.)-.005579)/L**(1./3.)
        if( KS .LE. 0.15e-3 )CA2=0.0
        CA=CA1+CA2
      end if
      if( NP3 .eq. 1 ) then
        print 10,LB,BL,LT,TL,BT,TB
        print 15,TFL,LD,LM,LCB,LR,S
        print 20,C1,C12,C13,C14,C15,C2
        print 25,C3,C4,C5,C7,CA1,CA2
        print 30,CA,CP,K1,M1,IE,PB
      endif
C
      if( NP1 .ne. 0 ) then
C
C       Таблица  1
C
        print 31
        print 32
        print 33
        print 34
        print 33
        print 32
        print 35
        print 32
        print 33
      end if
C
      NK=(N2-N1)/N3+1
      NV=N1-N3
      do 5 N=1,NK
         C6=0.0
         NV=NV+N3
         VS(N)=NV/10.0
         V=VS(N)*0.514
         V1=RO*V**2/2.0
         RE=V*L/KV
         CF(N)=.075/(ALOG10(RE)-2.)**2
         RF(N)=CF(N)*V1*S
         RV(N)=RF(N)*K1
         FN=V/SQRT(G*L)
         M2=C15*CP*CP*EXP(-.1*FN**(-2))
         RW(N)=C1*C2*C5*DEL*RO*G*EXP(M1*FN**D1+M2*COS(LM*FN**(-2)))
         FNI=V/SQRT(G*(TF-HB-.25*SQRT(ABT))+.15*V**2)
         if(ABT.LE..0001) RB(N)=0.0
         if(ABT.GT..0001) RB(N)=.11*EXP(-3.*PB**(-2))*FNI**3*ABT**1.5
     *   *RO*G/(1.+FNI**2)
         if(AT.LE..00001) goto 22
         FNT=V/SQRT(2.*G*AT/(B+B*CWP))
         if(FNT.LT.5.) C6=.2*(1.-.2*FNT)
c        if(FNT.GE.5.) C6=0.0
   22    RTR(N)=V1*AT*C6
         RA(N)=V1*S*CA
         RAP(N)=CF(N)*SAP*V1*K2
         RT(N)=RV(N)+RAP(N)+RW(N)+RB(N)+RTR(N)+RA(N)
         if(NP1.EQ.0)  goto  5
         PE=RT(N)*V
         CV=RV(N)/V1/S
         CW=RW(N)/V1/S
         CT=RT(N)/V1/S
         CR=(RT(N)-RA(N))/V1/S-CF(N)
         CTR(N)=RTR(N)/V1/S
         CBB(N)=RB(N)/V1/S
         if(RT(N).GT.99999999.) goto 23
         if(RT(N).LT.1000.) goto 24
         print 45,VS(N),V,FN,RE,CV,CW,CT,CR,V1,RT(N),PE
         goto  5
   23    print 43,VS(N),V,FN,RE,CV,CW,CT,CR,V1,RT(N),PE
         goto  5
   24    print 44,VS(N),V,FN,RE,CV,CW,CT,CR,V1,RT(N),PE
    5 continue
      if( NP1 .ne. 0 ) then
        print 33
        print 32
      end if
      if( NP1 .lt. 2 )return
C
C                   Таблица 2
C
      print 31
      print 32
      print 33
      print 36
      print 33
      print 32
      print 37
      print 32
C
      print 33
      DO 9 N=1,NK
        if(RF(N).LT.1000.) print 47,VS(N),CTR(N),CBB(N),CF(N),RV(N),
     *     RW(N),RF(N),RB(N),RAP(N),RTR(N),RA(N)
        if(RF(N).GE.1000.)
     *     print 46,VS(N),CTR(N),CBB(N),CF(N),RV(N),RW(N),RF(N),RB(N),
     *     RAP(N),RTR(N),RA(N)
    9 continue
      print 33
      print 32
      return
   11 print 40
      return
   12 print 41
      return
C-------------
    6 format(//120('═'),//40X,
     1 'Расчет  полного ',
     2 ' сопротивления  судна')
   10 format(/40X,
     1 'Величины,  не  зависящие',
     2 '  от  скорости'//6X,
     3 'LB=',F10.7,6X,'BL=',F10.7,6X,'LT=',F10.7,6X,'TL=',F10.7,6X,
     4 'BT=',F10.7,6X,'TB=',F10.7)
   15 format(5X,'TFL=',F10.7,6X,'LD=',F10.5,6X,'LM=', F10.7,5X,
     &           'LCB=',F10.7,6X,'LR=',F10.6,7X,'S=',  F10.4 )
   20 format(6X,'C1=',F10.7,5X,'C12=',F10.7,5X,'C13=',F10.7,5X,
     1          'C14=',F10.7,5X,'C15=',F10.7,6X,'C2=', F10.7)
   25 format(6X,'C3=',F10.7,6X, 'C4=',F10.7,6X,'C5=', F10.7,6X,
     &           'C7=',F10.7,5X,'CA1=',F10.7,5X,'CA2=',F10.7 )
   30 format(6X,'CA=',F10.6,6X, 'CP=',F10.7,6X,'K1=', F10.7,6X,
     &           'M1=',F10.7,6X, 'IE=',F10.7,6X,'PB=', F10.7 )
   31 format( / )
   32 format( 1X,'├',117('─'),'┤')
   33 format( ' │       │',10(10X,'│') )
   34 format( ' │   Vs  │     V    │    Fn    │    Re    ',
     &        '│    Cv    │    Cw    │    Ct    │    Cr    ',
     &        '│    Vi    │    Rt    │    Pe    │' )
   35 format( ' │  узлы │    м/с   │',6('     -    │'),
     &        2('     Н    │'),'    Вт    │' )
   36 format( ' │   VS  │    Ctr   │    Cbb   │    Cf    ',
     &        '│    Rv    │    Rw    │    Rf    │    Rb    ',
     &        '│    Rap   │    Rtr   │    Ra    │' )
   37 format( ' │  узлы │',
     &     3('     -    │'),7('     Н    │'))
   40 format( //30X,'4.*CP-1.=0  деление на ноль ',
     &              'при вычислении LR' )
   41 format( //30X,'TF-1.5*HB=0 деление на ноль ',
     &              'при вычислении PB' )
   43 format( ' │ ',F5.2,' │  ',  F6.3,'  │ ',F8.6,
     &        ' │ ',E8.1,' │',4(1X,F8.6,' │'),3(1X,E8.1,' │') )
   44 format( ' │ ',F5.2,' │  ',  F6.3,'  │ ',F8.6,
     &        ' │ ',E8.1,' │',4(1X,F8.6,' │'),3(1X,F8.4,' │') )
   45 format( ' │ ',F5.2,' │  ',  F6.3,'  │ ',F8.6,
     &        ' │ ',E8.1,' │',4(1X,F8.6,' │'),3(1X,F8.0,' │') )
   46 format( ' │ ',F5.2,' │',3(1X,F8.6,' │'),7(1X,F8.0,' │') )
   47 format( ' │ ',F5.2,' │',3(1X,F8.6,' │'),3(1X,F8.0,' │'),
     &                        4(1X,F8.4,' │') )
   50 format(/49X,'Исходные  данные'//
     & ' Длина  по  ватерлинии',
     & 38(' .'),'  L =',F8.3,' м'/
     & ' Длина  между  перпендикулярами ',
     & 32(' .'),'  Lpp =',F8.3,' м'/
     & ' Расчетная  ширина ',40('. '),
     & ' B =',F8.3,' м'/' Осадка  носом ',42('. '),
     & 'TF =',F8.3,' м'/' Осадка  наибольшая',
     & 40('. '),' T =',F8.3,' м')
   52 format(/' Коэффициент  общей  полноты ',
     &  35('. '),'CB =',F6.3,
     & /' Коэффициент  полноты ',
     &  ' мидель-шпангоута',30('. '),'CM =',F6.3,
     & /' Коэффициент  полноты ',
     &  ' ватерлинии',32('. '),' CWP =',F6.3,
     & /' Параметр  формы  кормы',
     &  37('. '),' CST = ',F5.1,
     & /' Объемное  водоизмещение ',
     &  36('. '),' DEL =',F11.3,' м³',
     & /' Продольное  положение  центра ',
     &  ' величины  (в  процентах  ',
     &  'от  Lpp , относительно  1/2  Lpp )',
     &  '. . .   XB = ',F6.3)
   54 format(/' Поперечная  площадь ',
     &  ' сечения  бульба',30('. '),' ABT =',F7.3,' м²',
     & /' Отстояние  центра  тяжести ',
     &  ' сечения  бульба  от  линии ',
     &  ' киля',19('. '),'HB =',F7.3,' м',
     & /' Площадь  транца, погруженная  в ',
     &  ' воду  при  VS=0 ',24(' .'),' AT =',F7.3,' м²',
     & /' Площадь  смоченной  поверхности ',
     &  ' выступающих  частей',
     &    22('. '),' SAP =',F7.3,' м²',
     & /' Коэффициент  сопротивления ',
     &  ' носового  подруливающего ',
     &  ' устройства',16('. '),' CBT =',F7.3,
     & /' Диаметр  тоннеля ',
     &  ' носового  подруливающего ',
     &  ' устройства',21('. '),' DPU =',F7.3,' м')
   56 format(/' Коэффициент  влияния ',
     &  ' выступающих  частей ',28('. '),
     &  'K2 =',F7.3/' Шероховатость ',
     &  ' поверхности  ( средняя ',
     &  ' амплитуда  бугорков )',19('. '),
     &  'KS =',F7.4,' м'/' Плотность  воды ',
     &    41('. '),'RO =',F8.2,' кг/м³',
     & /' Кинематическая ',
     &  ' вязкость  воды',34('. '),'KV =',F11.8,' м²/с')
   58 format(/20X,'Параметры  цикла',10('. '),' N1=',
     & I3,5X,'N2=',I3,5X,'N3=',I3/20X,
     & 'Параметры  печати ',9('. '),' NP1=',I1,6X,
     & 'NP2=',I1,6X,'NP3=',I1//30X,'Параметр  NS=',I2)
  110 format(/30X,'Коэффициенты,  полученные ',
     & ' при  вычислении  CM  и  CWP'//,
     & 4X,'LPMB=',F10.6,6X,'LE=',F10.6,5X,'LR1=',F10.6,
     & 6X,'A1=',F10.6,6X,'A2=',F10.6,5X,'M60=',F10.6/5X,'C11=',F10.6,
     & 5X,'CPR=',F10.6,5X,'CPE=',F10.6,5X,'CPF=',F10.6,5X,'CPA=',F10.6,
     & 5X,'XB1=',F10.4/5X,'CWF=',F10.6,5X,'CWA=',F10.6)
      end
