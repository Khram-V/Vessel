c
c  Действие третье.
c  По программе TOTAL_BEGIN_33_wl.for можно проверить  качество аппроксимации.
c  По этой программе получается 'ORD_APR.DAT, т.е. ординаты аппроксимированных
c  33-х ватерлиний. Для проверки качества аппроксимации  полученные ординаты
c  33-х аппроксимированных ватерлиний переводятся через лист ВОРД обратно
c  в таблицу по командам Insert_ Table_ Convert To Table указанием числа
c  ординат в ячейке Number of columns 21. Полученная таблица копируется
c  и вставляется в Excel, где наглядно видно её качество.
c
c  Действие четвёртое.
c  По программе TOTAL_COEF_FIN_33_wl.FOR по массиву SHIP_ORD_33_Wl.dat
c  получаются коэффициенты аппроксимирующих кривых всех 33 ватерлиний.
c
c                                      ©2010-06-18 Ада Шоломовна Готман
c
      PROGRAMM APPROXIMATION ! OF MASSIV OF WATERLINES
c
C **************************************************************************
C * Задать: 1) число точек на ватерлинии MN, 2) расстояние между точками DX,*
C * 3) массив MASSIV_ORD.DAT, 4) точность ординат заданных точек Е          *
C **************************************************************************
c             Примечание 1: поток* перенаправлен на порт-7="MASSIV_ORDIN.DAT"
c                        2: поток 8 создан для Действия 4=>"COEFF_FIN"[33x9]
c                               для которого раскомментарена запись в стр.229
c
      REAL E,F,G1,GT,T
      INTEGER I,IQ,K,IR,IW,NS,M,MN,MM,NM,L
      DIMENSION F(9),G1(21),T(21),GT(693)
      IR=5
      IW=6
      NS=693
      MN=21
      MM=33
      NM=9
      IQ=9
      E=0.18
      OPEN(IR,FILE='MASSIV_ORD_33_wl.DAT')
      OPEN(7,FILE='MASSIV_ORDIN.DAT')
      READ(IR,100)(GT(I),I=1,NS)
C     WRITE(6,100)(GT(I),I=1,NS)
      OPEN(IW,FILE='ORD_APR.DAT')
      OPEN(8,FILE='COEFF_FIN.DAT')
      DO 67 L=1,MM
C     WRITE(6,101)L
      DO 66 K=1,MN
      M=K+(L-1)*21
      G1(K)=GT(M)
C     WRITE(6,101)M
   66 CONTINUE
      CALL APR(G1,T,F)  ! ??? ошибка параметра Т =
   67 CONTINUE          ! Rank mismatch in argument 't' (scalar and rank-1)
c-536 FORMAT(F16.8)
  100 FORMAT(F16.8)
c-101 FORMAT(I)
      STOP
      END
c+++++
      SUBROUTINE APR( G,T,F )
      REAL S(27),X,DX,Y,Y0,Y1,C10,C20,C30,C40,Q,P,D1,R,QQQ,FFI,
     *     G,T,F,A,AA,AAA,BB,BBB,D,E,GGG,GGG1,YG1,YG2,YG3,Y2,Y3,YG,YGG
      INTEGER IQ,J,K,K1,K2,IR,IW,L,N,NM
      DIMENSION D(9,9),A(9),G(21),F(9),Y0(21),Y1(21),T(21),
     *          Y2(21),Y3(21),YGG(21),GGG(21),GGG1(21)
      IR=5
      IW=6
      IQ=9
      E=0.008
      DX=1
      NM=9
      N=9
      L=21
      IQ=9
      K=9
      K1=9
      K2=9
c-    K3=27
      S(1)=0
      S(2)=0.
      S(3)=0.
      S(4)=0.
      S(5)=0.
      S(6)=0.
      S(7)=0.
      S(8)=0.
      S(9)=0.
      S(10)=0.
      S(11)=0.
      S(12)=0.
      S(13)=0.
      S(14)=0.
      S(15)=0.
      S(16)=0.
      S(17)=0.
      S(18)=0.
      S(19)=0.
      S(20)=0.
      S(21)=0.
      S(22)=0.
      S(23)=0.
      S(24)=0.
      S(25)=0.
      S(26)=0.
      S(27)=0.
      DO 30 K=1,L
      X=10-(K-1)*DX
      Y=G(K)
      S(1)=S(1)+(Y**4)*X*X
      S(2)=S(2)+(Y**4)*X
      S(3)=S(3)+(Y**3)*X**3
      S(4)=S(4)+(Y**3)*X*X
      S(5)=S(5)+(Y**3)*X
      S(6)=S(6)+(Y**2)*X**4
      S(7)=S(7)+(Y**2)*X**3
      S(8)=S(8)+(Y**2)*X*X
      S(9)=S(9)+(Y**2)*X
      S(10)=S(10)+Y**4
      S(11)=S(11)+Y**3
      S(12)=S(12)+Y**2
      S(13)=S(13)+Y*X**5
      S(14)=S(14)+Y*X**4
      S(15)=S(15)+Y*X**3
      S(16)=S(16)+Y*X**2
      S(17)=S(17)+Y*X
      S(18)=S(18)+Y
      S(19)=S(19)+X**6
      S(20)=S(20)+X**5
      S(21)=S(21)+X**4
      S(22)=S(22)+X**3
      S(23)=S(23)+X*X
      S(24)=S(24)+X
      S(25)=S(25)+Y**5
      S(26)=S(26)+(Y**5)*X
      S(27)=L
   30 CONTINUE
C     WRITE(*,503)(S(I),I=1,K3)
C     WRITE(6,503)(S(I),I=1,K3)
      DO 21 J=1,9
      D(1,J)=S(J)
   21 CONTINUE
      D(2,1)=S(2)
      D(2,2)=S(10)
      D(2,3)=S(4)
      D(2,4)=S(5)
      D(2,5)=S(11)
      D(2,6)=S(7)
      D(2,7)=S(8)
      D(2,8)=S(9)
      D(2,9)=S(12)
      D(3,1)=S(3)
      D(3,2)=S(4)
      D(3,3)=S(6)
      D(3,4)=S(7)
      D(3,5)=S(8)
      D(3,6)=S(13)
      D(3,7)=S(14)
      D(3,8)=S(15)
      D(3,9)=S(16)
      D(4,1)=S(4)
      D(4,2)=S(5)
      D(4,3)=S(7)
      D(4,4)=S(8)
      D(4,5)=S(9)
      D(4,6)=S(14)
      D(4,7)=S(15)
      D(4,8)=S(16)
      D(4,9)=S(17)
      D(5,1)=S(5)
      D(5,2)=S(11)
      D(5,3)=S(8)
      D(5,4)=S(9)
      D(5,5)=S(12)
      D(5,6)=S(15)
      D(5,7)=S(16)
      D(5,8)=S(17)
      D(5,9)=S(18)
      D(6,1)=S(6)
      D(6,2)=S(7)
      D(6,3)=S(13)
      D(6,4)=S(14)
      D(6,5)=S(15)
      D(6,6)=S(19)
      D(6,7)=S(20)
      D(6,8)=S(21)
      D(6,9)=S(22)
      D(7,1)=S(7)
      D(7,2)=S(8)
      D(7,3)=S(14)
      D(7,4)=S(15)
      D(7,5)=S(16)
      D(7,6)=S(20)
      D(7,7)=S(21)
      D(7,8)=S(22)
      D(7,9)=S(23)
      D(8,1)=S(8)
      D(8,2)=S(9)
      D(8,3)=S(15)
      D(8,4)=S(16)
      D(8,5)=S(17)
      D(8,6)=S(21)
      D(8,7)=S(22)
      D(8,8)=S(23)
      D(8,9)=S(24)
      D(9,1)=S(9)
      D(9,2)=S(12)
      D(9,3)=S(16)
      D(9,4)=S(17)
      D(9,5)=S(18)
      D(9,6)=S(22)
      D(9,7)=S(23)
      D(9,8)=S(24)
      D(9,9)=S(27)
      A(1)=-S(26)
      A(2)=-S(25)
      A(3)=-S(1)
      A(4)=-S(2)
      A(5)=-S(10)
      A(6)=-S(3)
      A(7)=-S(4)
      A(8)=-S(5)
      A(9)=-S(11)
      CALL A0J(NM,N,D,A)
      DO 81 K=1,9
      F(K)=A(K)
C     WRITE(*,101)F(K)
      WRITE(8,100)F(K)
   81 CONTINUE
      DO 20 J=1,L
      X=10-(J-1)*DX
      C10=1.00
      C20=F(1)*X+F(2)
      C30=F(3)*X*X+F(4)*X+F(5)
      C40=F(6)*X**3+F(7)*X*X+F(8)*X+F(9)
      Q=(C20**3)/(27*C10**3)-(C20*C30)/(6*C10*C10)+C40/C10/2
      P=(3*C10*C30-C20*C20)/9/C10/C10
      D1=Q*Q+P**3
      IF( D1.LT.0.0 )GOTO 90
      IF( D1.GE.0.0 )AA=-Q+SQRT(D1)
      IF( D1.GE.0.0 )BB=-Q-SQRT(D1)
      IF( D1.GE.0.0 .AND. AA.LT.0.0 )AAA=-ABS(AA)**(1./3.)
      IF( D1.GE.0.0 .AND. BB.LT.0.0 )BBB=-ABS(BB)**(1./3.)
      IF( D1.GE.0.0 .AND. AA.GE.0.0 )AAA=AA**(1./3.)
      IF( D1.GE.0.0 .AND. BB.GE.0.0 )BBB=BB**(1./3.)
      Y0(J)=AAA+BBB-C20/3/C10
C     GO TO 50
      IF(D1.GE.0)GOTO 70
   90 R=SQRT(ABS(P))
      IF(Q.LT.0.)R=-R
      QQQ=Q/R/R/R
      FFI=ACOS(QQQ)
   70 Y1(J)=-2*R*COS(FFI/3)-C20/3/C10
      Y2(J)=-C20/3/C10+2*R*COS(1.047197551-FFI/3)
      Y3(J)=-C20/3/C10+2*R*COS(1.047197551+FFI/3)
      YG1=ABS(Y1(J)-G(J))
      YG2=ABS(Y2(J)-G(J))
      YG3=ABS(Y3(J)-G(J))
      YG=ABS(Y0(J)-G(J))
      YGG(J)=MIN(YG1,YG2,YG3,YG)
      GGG(J)=YGG(J)
C     IF(YGG(J).GT.E)YGG(J)=E
      GGG(J)=YGG(J)
      GGG1(J)=GGG(J)+G(J)
   54 WRITE(7,100)GGG(J)
      WRITE(6,100)GGG1(J)
   20 CONTINUE
c- 40 FORMAT(2X,I3)
  100 FORMAT(F16.8)
      RETURN
      END
c+++++
      SUBROUTINE A0J(NM,N,D,A)
      REAL T,D,A                                    ! ,G
      INTEGER I,IS,IS1,J,L,N,N1,NM
      DIMENSION A(9),D(9,9)                        ! ,G(9)
      N1=N-1
      DO 1 IS=1,N1
      IS1=IS+1
      L=IS
      DO 2 I=IS1,N
      IF(ABS(D(I,IS)).GT.ABS(D(L,IS)))L=I
 2    CONTINUE
      IF(L.EQ.IS) GO TO 4
      DO 3 J=IS,N
      T=D(IS,J)
      D(IS,J)=D(L,J)
      D(L,J)=T
 3    CONTINUE
      A(IS)=A(IS)
      T=A(IS)
      A(IS)=A(L)
      A(L)=T
      IS1=IS+1
    4 CONTINUE
      DO 1 I=IS1,N
      T=D(I,IS)/D(IS,IS)
      A(I)=A(I)-T*A(IS)
      DO 1 J=IS1,N
      D(I,J)=D(I,J)-T*D(IS,J)
    1 CONTINUE
      A(N)=A(N)/D(N,N)
      I=N
    7 I=I-1
      T=0.0
      IS1=I+1
      DO 5 J=IS1,N
      T=T+D(I,J)*A(J)
    5 CONTINUE
      A(I)=(A(I)-T)/D(I,I)
C     WRITE(*,510)T
C     WRITE(6,510)T
c-510 FORMAT(3F16.12)
      IF(I-1.GT.0)goto 7
c-    IF(I-1) 6,6,7
    6 RETURN
      END
