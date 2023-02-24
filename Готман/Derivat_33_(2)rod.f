c
c  Действие пятое.
c  Для программы Derivat_33_(2)rod.f подготавливаются массивы форштевня
c  MASSIV_FH и ахтерштевня MASSIV_AH с чертежей в Excel
c
c  По программе Derivat_33_(2)rod.f получаются производные в оконечностях
c  всех ватерлиний и записываются в файл RODINA_DERS2(33)
c
c  Пояснение.
c  В данном случае расчет выполняется по 33 ватерлиниям.
c  Число 2 в скобках в названиях программ указывает количество производных
c  в интеграле Мичелла, которые принимаются в расчет.
c  Проверка числа принимаемых в расчет производных на каждой ватерлинии
c  показала, что увеличение числа производных до 16 практически
c  не уточняет результата.
c
c                                      ©2010-06-18 Ада Шоломовна Готман
c
      PROGRAMM TOTAL DERIVATIVES
      REAL A,AH,B,D,DL,FH,X,RB,RB1,SB,SB1,SB2,ZNB,ZNS,
     *     Y,Y1,Y2,Y3,Y4,Y5,Y6,Y7,Y8,Y9,Y10,
     *     YK1,YK2,YK3,YK4,YK5,YK6,YK7,YK8,YK9,YK10,
     *     TB,TB1,TB2,TB3,RS,RS1,SS,SS1,SS2,TS,TS1,TS2,TS3
      INTEGER   I,I1,K2,IR,IW,N,M,K1,J1
      DIMENSION A(33,9),D(297),AH(33),FH(33)
C     "         NUMBER OF WATERLINE M=21, NUMBER OF POINTS
C     *         ON A WATERLINE L=111, NUMBER PF POINTS IN MASSIV
C     *         ORDINATES+KOEF OF VERSIERA K2=25"
      IR=5
      IW=6
      OPEN(IW,FILE='RODINA_DERS2(33).DAT')
      B=11.6                            ! ширина по ватерлинии
      DL=90                             ! длина корпуса
c-    BL=B/DL
      K2=297                            ! это NxM = 9х33
C     NM=25
      N=9                               !?? загадочный отсчет внутреннего цикла
C     NS=525
      M=33                              ! количество слоёв ватерлиний
      OPEN(IR,FILE='COEFF_FIN.DAT')
      READ(IR,101)(D(I),I=1,K2)         ! =297
      OPEN(IR,FILE='MASSIV_FH.DAT')
      READ(IR,101)(FH(I),I=1,M)         ! =33 абсциссы форштевня ~+10
      OPEN(IR,FILE='MASSIV_AH.DAT')
      READ(IR,101)(AH(I),I=1,M)         ! =33 абсциссы ахтерштевня ~-10
      DO 4 J1=1,M                       ! M=33 - по ватерлиниям
      DO 3 I1=1,N                       ! N=9 - загадочная девятка
      K1=I1+N*(J1-1)                    ! по 9 параметров на каждую ватерлинию
      A(J1,I1)=D(K1)                    ! ( 33x9 )
    3 CONTINUE
    4 CONTINUE
      DO 1 I=1,M                        ! форштевень
      X=FH(I)                           ! абсцисса
C     DO 2 J=1,N
C     K=J+(I-1)*DX
C     C(J1,I1)=D(K)
      RB = A(I,1)*X+A(I,2)
      RB1= A(I,1)
      SB = A(I,3)*X**2+A(I,4)*X+A(I,5)
      SB1= 2.*A(I,3)*X+A(I,4)
      SB2= 2.*A(I,3)
      TB = A(I,6)*X**3+A(I,7)*X**2+A(I,8)*X+A(I,9)
      TB1= 3.*A(I,6)*X**2+2.*A(I,7)*X+A(I,8)
      TB2= 6.*A(I,6)*X+2.*A(I,7)
      TB3= 6.*A(I,6)
C     WRITE(6,102)RB,RB1
C     WRITE(6,103)SB,SB1,SB2
C     WRITE(6,104)TB,TB1,TB2,TB3
      Y=0
      ZNB=3.*Y**2+2.*RB*Y+SB
      Y1 =-(RB1*Y**2+SB1*Y+TB1)/ZNB
      Y2 =-(6.*Y*Y1*Y1+4.*RB1*Y*Y1+2.*RB*Y1*Y1+
     *    SB2*Y+2.*SB1*Y1+TB2)/ZNB
      Y3 =-(6.*Y1**3+18.*Y*Y1*Y2+6.*RB1*Y1**2+6.*RB1*Y*Y2+
     *    6.*RB*Y1*Y2+3.*SB2*Y1+3.*SB1*Y2+TB3)/ZNB
      Y4=-(36.*Y1**2*Y2+18.*Y*Y2**2+24.*Y*Y1*Y3+24.*RB1*Y1*Y2+
     *    8.*RB1*Y*Y3+6.*RB*Y2**2+8.*RB*Y1*Y3+
     *    6.*SB2*Y2+4.*SB1*Y3)/ZNB
      Y5=-(90.*Y1*Y2**2+60.*Y1*Y1*Y3+60.*Y*Y2*Y3+30.*Y*Y1*Y4+
     *    30.*RB1*Y2**2+40.*RB1*Y1*Y3+10.*RB1*Y*Y4+20.*RB*Y2*Y3+
     *    10.*RB*Y1*Y4+10.*SB2*Y3+5.*SB1*Y4)/ZNB
      Y6=-(90.*Y2**3+360.*Y1*Y2*Y3+90.*Y1**2*Y4+60.*Y*Y3**2+
     *    90.*Y*Y2*Y4+36.*Y*Y1*Y5+120.*RB1*Y2*Y3+60.*RB1*Y1*Y4+
     *    12.*RB1*Y*Y5+20.*RB*Y3**2+30.*RB*Y2*Y4+12.*RB*Y1*Y5+
     *    15.*SB2*Y4+6.*SB1*Y5)/ZNB
      Y7=-(630.*Y2**2*Y3+420.*Y1*Y3**2+630.*Y1*Y2*Y4+
     *    126.*Y1**2*Y5+210.*Y*Y3*Y4+126.*Y*Y2*Y5+42.*Y*Y1*Y6+
     *    140.*RB1*Y3**2+210.*RB1*Y2*Y4+84.*RB1*Y1*Y5+14.*RB1*Y*Y6+
     *    70.*RB*Y3*Y4+42.*RB*Y2*Y5+14.*RB*Y1*Y6+
     *    21.*SB2*Y5+7.*SB1*Y6)/ZNB
      Y8=-(1680.*Y2*Y3**2+1260.*Y2**2*Y4+1680.*Y1*Y3*Y4+
     *    1008.*Y1*Y2*Y5+168.*Y1**2*Y6+210.*Y*Y4**2+336.*Y*Y3*Y5+
     *    168.*Y*Y2*Y6+48.*Y*Y1*Y7+360.*RB1*Y3*Y4+336.*RB1*Y2*Y5+
     *    112.*RB1*Y1*Y6+16.*RB1*Y*Y7+70.*RB*Y4**2+112.*RB*Y3*Y5+
     *    56.*RB*Y2*Y6+16.*RB*Y1*Y7+28.*SB2*Y6+8.*SB1*Y7)/ZNB
      Y9=-(1680.*Y3**3+7560.*Y2*Y3*Y4+2268.*Y2*Y2*Y5+1890.*Y1*Y4**2+
     *    3024*Y1*Y3*Y5+1512.*Y1*Y2*Y6+216.*Y1*Y1*Y7+756.*Y*Y4*Y5+
     *    504.*Y4*Y6+216.*Y*Y2*Y7+54.*Y*Y1*Y8+630.*RB1*Y4*Y4+
     *    1008.*RB1*Y3*Y5+504.*RB1*Y2*Y4+144.*RB1*Y1*Y7+
     *    18.*RB1*Y*Y8+252.*RB*Y4*Y5+168.*RB*Y3*Y6+72.*RB*Y2*Y7+
     *    18.*RB*Y1*Y8+36.*SB2*Y7+9.*SB1*Y8)/ZNB
      Y10=-(12600.*Y3*Y3*Y4+9450.*Y2*Y4*Y4+15120.*Y2*Y3*Y5+
     *    3780.*Y2*Y2*Y6+7560.*Y1*Y4*Y5+5040.*Y1*Y3*Y6+
     *    2160.*Y1*Y2*Y7+270.*Y1*Y1*Y8+756.*Y*Y5*Y5+
     *    1260*Y*Y4*Y6+720.*Y*Y3*Y7+270.*Y*Y2*Y8+60.*Y*Y1*Y9+
     *    2520.*RB1*Y4*Y5+1680.*RB1*Y3*Y6+2160.*RB1*Y2*Y7+
     *    180.*RB1*Y1*Y8+20.*RB1*Y*Y9+252*RB*Y5*Y5+420*RB*Y4*Y6+
     *    240.*RB*Y3*Y7+90.*RB*Y2*Y8+20.*RB*Y1*Y9+45.*SB2*Y8+
     *    10.*SB1*Y9)/ZNB
      WRITE(6,105)Y1,Y2
      Y=0
      X=AH(I)
      RS=A(I,1)*X+A(I,2)
      RS1=A(I,1)
      SS=A(I,3)*X**2+A(I,4)*X+A(I,5)
      SS1=2.*A(I,3)*X+A(I,4)
      SS2=2.*A(I,3)
      TS=A(I,6)*X**3+A(I,7)*X**2+A(I,8)*X+A(I,9)
      TS1=3.*A(I,6)*X**2+2.*A(I,7)*X+A(I,8)
      TS2=6.*A(I,6)*X+2.*A(I,7)
      TS3=6.*A(I,6)
C     WRITE(6,102)RS,RS1
C     WRITE(6,103)SS,SS1,SS2
C     WRITE(6,104)TS,TS1,TS2,TS3
      ZNS=3.*Y**2+2.*RS*Y+SS
      YK1=-(RS1*Y**2+SS1*Y+TS1)/ZNS
      YK2=-(6.*Y*Y1*YK1+4.*RS1*Y*YK1+2.*RS*YK1*YK1+
     *     SS2*Y+2.*SS1*YK1+TS2)/ZNS
      YK3=-(6.*YK1**3+18.*Y*YK1*Y2+6.*RS1*YK1**2+6.*RS1*Y*YK2+
     *     3.*RS*Y1*Y2+6.*SS2*Y1+3.*SS1*Y2+TS3)/ZNS
           YK4=-(36.*YK1**2*YK2+18.*Y*YK2**2+24.*Y*YK1*YK3+
     *     24.*RS1*YK1*YK2+8.*RS1*Y*YK3+6.*RS*YK2**2+8.*RS*YK1*YK3+
     *     6.*SS2*Y2+4.*SS1*Y3)/ZNS
      YK5=-(90.*YK1*YK2**2+60.*Y1*YK1*YK3+60.*Y*YK2*YK3+
     *     30.*Y*YK1*YK4+
     *     30.*RS1*YK2**2+40.*RS1*YK1*YK3+10.*RS1*Y*YK4+
     *     20.*RS*YK2*YK3+10.*RS*YK1*YK4+10.*SS2*YK3+5.*SS1*YK4)/ZNS
           YK6=-(90.*YK2**3+360.*YK1*YK2*Y3+90.*YK1**2*YK4+
     *     60.*Y*YK3**2+
     *     90.*Y*YK2*YK4+36.*Y*YK1*YK5+120.*RS1*YK2*YK3+
     *     60.*RS1*YK1*YK4+
     *     12.*RS1*Y*YK5+20.*RS*YK3**2+30*RS*YK2*YK4+12*RS*YK1*YK5+
     *     15.*SS2*YK4+6.*SS1*YK5)/ZNS
      YK7=-(630.*YK2**2*YK3+420.*YK1*YK3**2+630.*YK1*YK2*YK4+
     *     126.*YK1**2*YK5+210.*Y*YK3*YK4+126*Y*YK2*YK5+42*Y*YK1*YK6+
     *     140*RS1*YK3**2+210*RS1*YK2*YK4+84*RS1*YK1*YK5+14*RS1*Y*YK6+
     *     70.*RS*YK3*YK4+42.*RS*YK2*YK5+14.*RS*YK1*YK6+
     *     21.*SS2*YK5+7.*SS1*YK6)/ZNS
      YK8=-(1680*YK2*YK3**2+1260*YK2**2*YK4+1680*YK1*YK3*YK4+
     *     1008*YK1*YK2*YK5+168.*YK1**2*YK6+210.*Y*YK4**2+
     *     336.*Y*YK3*YK5+
     *     168.*Y*YK2*YK6+48.*Y*YK1*YK7+360.*RS1*YK3*YK4+
     *     336.*RS1*YK2*YK5+
     *     112.*RS1*YK1*YK6+16.*RS1*Y*YK7+70.*RS*YK4**2+
     *     112.*RS*YK3*YK5+
     *     56.*RS*YK2*YK6+16.*RS*YK1*YK7+28.*SS2*YK6+8.*SS1*YK7)/ZNS
      YK9=-(1680.*YK3**3+7560.*YK2*YK3*Y4+2268.*YK2*YK2*YK5+
     *     1890.*YK1*YK4**2+3024.*YK1*YK3*YK5+1512.*YK1*YK2*YK6+
     *     216.*YK1*YK1*YK7+756.*Y*YK4*YK5+504.*Y*YK3*YK6+
     *     216.*Y*YK2*YK7+54.*Y*YK1*YK8+630.*RS1*YK4*YK4+
     *     1008.*RS1*YK3*YK5+504.*RS1*YK2*YK4+144.*RS1*YK1*YK7+
     *     18.*RS1*Y*YK8+252.*RS*YK4*YK5+168.*RS*YK3*YK6+
     *     72.*RS*YK2*YK7+18.*RS*YK1*YK8+36.*SS2*YK7+9.*SS1*YK8)/ZNS
      YK10=-(12600.*YK3*YK3*YK4+9450.*YK2*YK4*YK4+15120.*YK2*YK3*YK5+
     *    3780.*YK2*YK2*YK6+7560.*YK1*YK4*YK5+5040.*YK1*YK3*YK6+
     *    2160.*YK1*YK2*YK7+270.*YK1*YK1*YK8+756.*Y*YK5*YK5+
     *    1260.*Y*YK4*YK6+720.*Y*YK3*YK7+270.*Y*YK2*YK8+60.*Y*YK1*YK9+
     *    2520.*RS1*YK4*YK5+1680.*RS1*YK3*YK6+2160.*RS1*YK2*YK7+
     *    180.*RS1*YK1*YK8+20.*RS1*Y*YK9+252*RS*YK5*YK5+420*RS*YK4*YK6+
     *    240.*RS*YK3*YK7+90.*RS*YK2*YK8+20.*RS*YK1*YK9+45.*SS2*YK8+
     *    10.*SS1*YK9)/ZNS
      WRITE(6,105)YK1,YK2
C     WRITE(6,509)
C    2 CONTINUE
    1 CONTINUE
c-509 FORMAT(2X,60('*'))
c- 40 FORMAT(I4)
c-100 FORMAT(2G16.8)
  101 FORMAT(F16.8)
c-102 FORMAT(/2X,2F16.8)
c-103 FORMAT(2X,3F16.8)
c-104 FORMAT(2X,4E16.8/)
c-106 FORMAT(X,3E16.8/2X,3E16.8/2X,3E16.8/2X,3E16.8)
  105 FORMAT(2X,E16.8)
      STOP
      END
