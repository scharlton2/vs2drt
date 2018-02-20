    SUBROUTINE VTSETUPSOL_PARALLEL
    !*******
    !VTSETUPSOL
    !*******
    !
    !    ROUTINE TO ASSEMBLE MATRIX EQUATIONS FOR ADVECTION-DISPERSION
    !    EQUATIONS AND TO CALL MATRIX SOLVER.
    !
    use press
    use rspac
    use kcon
    use mprop
    use dumm
    use disch
    use equats
    use jtxx
    use trxx
    use trxy2
    use rpropsh
    use scon
    use trxv
    use temp
    use pit
    use ptet
    use tempcc
    use COMPNAM
    use react
    use gmres1
    use, intrinsic :: iso_fortran_env, only : stdin=>input_unit, &
        stdout=>output_unit, &
        stderr=>error_unit
    IMPLICIT DOUBLE PRECISION (A-H,P-Z)

    COMMON/ISPAC/NLY,NLYY,NXR,NXRR,NNODES,Nsol,Nodesol
    COMMON/JCON/JSTOP,JFLAG,jflag1
    COMMON/TCON/STIM,DSMAX,KTIM,NIT,NIT1,KP,NIT3
    LOGICAL TRANS,TRANS1,TRANS2,SSTATE
    COMMON/TRXY/EPS1,EPS2,EPS3,TRANS,TRANS1,TRANS2,SSTATE,MB9(99),NMB9
    LOGICAL RAD,BCIT,ETSIM,SEEP,ITSTOP,CIS,CIT,GRAV
    COMMON/LOG1/RAD,BCIT,ETSIM,SEEP,ITSTOP,CIS,CIT,GRAV
    integer hydraulicFunctionType
    common/functiontype/ hydraulicFunctionType
    COMMON/TCON1/NIS,NIS1,NIS3
    COMMON/SCON1/ITESTS
    COMMON/JCONF/JFLAG2
    logical solved, pmgmres_ilu_cr
    !
    !...........................................................................
    !
    do 60 M=1,Nsol
        NIS1=0

        IF(jflag2.EQ.1) THEN
            DO 10 N=1,NNODES
                AOC(N)=0.0D0
                BOC(N)=0.0D0
                COC(N)=0.0D0
                DOC(N)=0.0D0
                EOC(N)=0.0D0
10          CONTINUE
        END IF
        !
        !   INITIALIZE VARIABLES
        !
        do 50 it=1,itmax
            DO 20 I=2,NXRR
                N1=NLY*(I-1)
                DO 20 J=2,NLYY
                    N=N1+J
                    AS(N)=0.0D0
                    BS(N)=0.0D0
                    CS(N)=0.0D0
                    DS(N)=0.0D0
                    ES(N)=0.0D0
                    RHSS(N)=0.0D0
                    if(it.eq.1) then
                        CCOLD(M,N)=CC(M,N)
                        IF(NTYP(N).EQ.1) then
                            dum(n) = qs(n)
                            QS(N)=VSFLX1(N)
                        else
                            qs(n) = 0.0d0
                        end if
                    end if
                    TempC(N)=CC(M,N)
                    !      WRITE(6,*)'TempC Before ########### ',M
                    !     CALL VSOUTS(1,TempC(N))
                    IF(HX(N).NE.0.0D0) THEN
                        N2=JTEX(N)
                        !      RHO1=RHO(N)
                        !      RHO(N)=VTRHO(CC(N),N2)
                        !      RHO2=DABS(RHO1-RHO(N))
                        !      IF(RHO2.GT.RHOMAX)RHOMAX=RHO2
                        !      RET(N)=VTRET(CC(N),N2)
                        !      ret(n)=0.0D0
                        IM1=N-NLY
                        JM1=N-1
                        JP1=N+1
                        IP1=N+NLY
                        IP2=IP1-1
                        IM2=IM1+1
                        IM3=IM1-1
                        IP3=IP1+1
                        IF(RAD) THEN
                            AREAX=PI2*DELZ(J)*(RX(I)-0.5D0*DXR(I))
                            AREAX1=PI2*DELZ(J)*(RX(I)+0.5D0*DXR(I))
                            AREAZ=PI2*DXR(I)*RX(I)
                        ELSE
                            AREAX=DELZ(J)
                            AREAX1=AREAX
                            AREAZ=DXR(I)
                        END IF
                        VOL=AREAZ*DELZ(J)
                        AREAX=AREAX*0.5D0*(THETA(IM1)+THETA(N))
                        AREAX1=AREAX1*0.5D0*(THETA(IP1)+THETA(N))
                        AREAZ1=AREAZ*0.5D0*(THETA(JP1)+THETA(N))
                        AREAZ=AREAZ*0.5D0*(THETA(JM1)+THETA(N))
                        !
                        !   CALCULATE LHS OF MATRIX EQUATION
                        !
                        SS=THETA(N)*(P(N)-PXXX(N))*HK(N2,2)/HK(N2,3)
                        ES(N)=-DXS1(N)-DZS1(N)-DXS1(IP1)-DZS1(JP1)
                        !   &-VOL*(HT(N2,4)*(THETA(N)+RET(N)))
                        !
                        !  CHANGE ADDED 8-12-91 TO CORRECT STORAGE TERM
                        !
                        !      SS1=HT(N2,4)*(THETA(N)+RET(N))
                        !
                        !c******************
                        !  following change made 7-3-04 to correct dctheta/dt
                        !   calculation - see written notes
                        !
                        !******************
                        !      if(jflag1.ne.1.or.ntyp(n).ne.1) then
                        !       SS=THETA(N)+SS-THLST(N)
                        !     end if
                        !******************

                        !C#
                        AS(N)=DXS1(N)
                        BS(N)=DZS1(N)

                        CS(N)=DXS1(IP1)
                        DS(N)=DZS1(JP1)
                        !C#    TOP(n,j-1)
                        IF(HX(IM1).NE.0.0D0) THEN
                            !C#    A(N)=0.5D0*(DX1(N)*(RHO(N)+RHO(IM1))+DZ2(N)-DZ2(JP1))
                            IF(.NOT.CIS) THEN
                                IF(VX(N).GT.0.0D0) THEN
                                    AS(N)=AS(N)+AREAX*VX(N)
                                ELSE
                                    ES(N)=ES(N)+AREAX*VX(N)
                                END IF
                            ELSE
                                VV=AREAX*0.5D0*VX(N)
                                AS(N)=AS(N)+VV
                                ES(N)=ES(N)+VV
                            END IF
                            !C#
                            TEMPP=0.5D0*DXS2(N)
                            IF(HX(IM3).GT.0.0D0 .AND. HX(JM1).GT.0.0D0) THEN
                                IF(HX(IM2).GT.0.0D0 .AND. HX(JP1).GT.0.0D0) THEN
                                    BS(N)=BS(N)+TEMPP
                                    DS(N)=DS(N)-TEMPP
                                    RHSS(N)=RHSS(N)+TEMPP*(CC(M,IM2)-CC(M,IM3))
                                ELSE
                                    AS(N)=AS(N)-TEMPP
                                    BS(N)=BS(N)+TEMPP
                                    ES(N)=ES(N)-TEMPP
                                    RHSS(N)=RHSS(N)-TEMPP*CC(M,IM3)
                                END IF
                            ELSE
                                IF(HX(IM2).GT.0.0D0 .AND. HX(JP1).GT.0.0D0) THEN
                                    AS(N)=AS(N)+TEMPP
                                    DS(N)=DS(N)-TEMPP
                                    ES(N)=ES(N)+TEMPP
                                    RHSS(N)=RHSS(N)+TEMPP*CC(M,IM2)
                                END IF
                                !C#
                            END IF
                        END IF

                        !C#    left (n-1,j)
                        IF(HX(JM1).NE.0.0D0) THEN
                            !C#    B(N)=0.5D0*(DZ1(N)*(RHO(N)+RHO(JM1))+DX2(N)-DX2(IP1))
                            IF(.NOT.CIS) THEN
                                IF(VZ(N).GT.0.0D0) THEN
                                    BS(N)=BS(N)+AREAZ*VZ(N)
                                ELSE
                                    ES(N)=ES(N)+AREAZ*VZ(N)
                                END IF
                            ELSE
                                VV=0.5D0*AREAZ*VZ(N)
                                BS(N)=BS(N)+VV
                                ES(N)=ES(N)+VV
                            END IF
                            !C#
                            TEMPP=0.5D0*DZS2(N)
                            IF(HX(IP2).GT.0.0D0 .AND. HX(IP1).GT.0.0D0) THEN
                                IF(HX(IM3).GT.0.0D0 .AND. HX(IM1).GT.0.0D0) THEN
                                    AS(N)=AS(N)+TEMPP
                                    CS(N)=CS(N)-TEMPP
                                    RHSS(N)=RHSS(N)+TEMPP*(CC(M,IP2)-CC(M,IM3))
                                ELSE
                                    BS(N)=BS(N)+TEMPP
                                    CS(N)=CS(N)-TEMPP
                                    ES(N)=ES(N)+TEMPP
                                    RHSS(N)=RHSS(N)+TEMPP*CC(M,IP2)
                                END IF
                            ELSE
                                IF(HX(IM3).GT.0.0D0 .AND. HX(IM1).GT.0.0D0) THEN
                                    AS(N)=AS(N)+TEMPP
                                    BS(N)=BS(N)-TEMPP
                                    ES(N)=ES(N)-TEMPP
                                    RHSS(N)=RHSS(N)-TEMPP*CC(M,IM3)
                                END IF
                            END IF
                            !C#
                        END IF

                        !C#  Bottom (n,j+1)
                        IF(HX(IP1).NE.0.0D0) THEN
                            !C#    C(N)=0.5D0*(DX1(IP1)*(RHO(N)+RHO(IP1))-DZ2(N)+DZ2(JP1))
                            IF(.NOT.CIS) THEN
                                IF(VX(IP1).LT.0.0D0) THEN
                                    CS(N)=CS(N)-AREAX1*VX(IP1)
                                ELSE
                                    ES(N)=ES(N)-AREAX1*VX(IP1)
                                END IF
                            ELSE
                                VV=0.5D0*AREAX1*VX(IP1)
                                CS(N)=CS(N)-VV
                                ES(N)=ES(N)-VV
                            END IF
                            !C#
                            TEMPP=0.5D0*DXS2(IP1)
                            IF(HX(JP1).GT.0.0D0 .AND. HX(IP3).GT.0.0D0) THEN
                                IF(HX(IP2).GT.0.0D0 .AND. HX(JM1).GT.0.0D0) THEN
                                    BS(N)=BS(N)-TEMPP
                                    DS(N)=DS(N)+TEMPP
                                    RHSS(N)=RHSS(N)+TEMPP*(CC(M,IP2)-CC(M,IP3))
                                ELSE
                                    CS(N)=CS(N)-TEMPP
                                    DS(N)=DS(N)+TEMPP
                                    ES(N)=ES(N)-TEMPP
                                    RHSS(N)=RHSS(N)-TEMPP*CC(M,IP3)
                                END IF
                            ELSE
                                IF(HX(IP2).GT.0.0D0 .AND. HX(JM1).GT.0.0D0) THEN
                                    BS(N)=BS(N)-TEMPP
                                    CS(N)=CS(N)+TEMPP
                                    ES(N)=ES(N)+TEMPP
                                    RHSS(N)=RHSS(N)+TEMPP*CC(M,IP2)
                                END IF
                            END IF
                            !C#
                        END IF

                        !C#  right (n+1,j)
                        IF(HX(JP1).NE.0.0D0) THEN
                            !C#    D(N)=0.5D0*(DZ1(JP1)*(RHO(N)+RHO(JP1))-DX2(N)+DX2(IP1))
                            IF(.NOT.CIS) THEN
                                IF(VZ(JP1).LT.0.0D0) THEN
                                    DS(N)=DS(N)-AREAZ1*VZ(JP1)
                                ELSE
                                    ES(N)=ES(N)-AREAZ1*VZ(JP1)
                                END IF
                            ELSE
                                VV=0.5D0*AREAZ1*VZ(JP1)
                                DS(N)=DS(N)-VV
                                ES(N)=ES(N)-VV
                            END IF
                            !C#
                            TEMPP=0.5D0*DZS2(JP1)
                            IF(HX(IM2).GT.0.0D0 .AND. HX(IM1).GT.0.0D0) THEN
                                IF(HX(IP1).GT.0.0D0 .AND. HX(IP3).GT.0.0D0) THEN
                                    AS(N)=AS(N)-TEMPP
                                    CS(N)=CS(N)+TEMPP
                                    !     CALL VSOUTS(1,TempC(N))
                                    RHSS(N)=RHSS(N)+TEMPP*(CC(M,IM2)-CC(M,IP3))
                                ELSE
                                    AS(N)=AS(N)-TEMPP
                                    DS(N)=DS(N)+TEMPP
                                    ES(N)=ES(N)+TEMPP
                                    RHSS(N)=RHSS(N)+TEMPP*CC(M,IM2)
                                END IF
                            ELSE
                                IF(HX(IP1).GT.0.0D0 .AND. HX(IP3).GT.0.0D0) THEN
                                    CS(N)=CS(N)+TEMPP
                                    DS(N)=DS(N)-TEMPP
                                    ES(N)=ES(N)-TEMPP
                                    RHSS(N)=RHSS(N)-TEMPP*CC(M,IP3)
                                END IF
                            END IF
                            !C#
                        END IF
                        if (NPV.ge.0) then
                            IF(Q(N).LT.0.0D0 .AND. NTYP(N) .NE. 5) ES(N)=ES(N)+Q(N)
                        end if
                        IF(QQ(N).LT.0.0D0) ES(N)=ES(N)+QQ(N)
                        IF(QS(N).GT.0.0D0) ES(N)=ES(N)-QS(N)
                        !C
                        !C  CENTERED-IN-TIME DIFFERENCING CAN BE USED ONLY AFTER THE
                        !C  FIRST TIME STEP IN ANY RECHARGE PERIOD.
                        !C

                        IF(CIT.AND.JFLAG2.NE.1) THEN
                            AS(N)=0.5D0*AS(N)
                            BS(N)=0.5D0*BS(N)
                            CS(N)=0.5D0*CS(N)
                            DS(N)=0.5D0*DS(N)
                            ES(N)=0.5D0*ES(N)
                        END IF
                        ES(N)=ES(N)-VOL*(THETA(N)+SS)/DELT
                    END IF
20          CONTINUE
            !     WRITE(6,*)'TempC Before ########### ',M
            !     CALL VSOUTS(1,TempC(N))

            !
            !  BEGIN LOOP TO CALCULATE RHS AND CALL MATRIX SOLVER
            !

            DO 30 I=2,NXRR
                N1=NLY*(I-1)
                DO 30 J=2,NLYY
                    N=N1+J
                    IM1=N-NLY
                    JM1=N-1
                    JP1=N+1
                    IP1=N+NLY
                    IP2=IP1-1
                    IM2=IM1+1
                    IM3=IM1-1
                    IP3=IP1+1
                    IF(RAD) THEN
                        VOL=PI2*DELZ(J)*DXR(I)*RX(I)
                    ELSE
                        VOL=DELZ(J)*DXR(I)
                    END IF
                    N2=JTEX(N)

                    if (ntyp(n).eq.1) then
                        RHSS(N)=RHSS(N)-VOL*THETA(N)*CCOLD(M,N)/DELT-AS(N)*CC(M,IM1)&
                            -BS(N)*CC(M,JM1)-CS(N)*CC(M,IP1)-DS(N)*CC(M,JP1)-ES(N)*CC(M,N)
                    else
                        RHSS(N)=RHSS(N)-VOL*THLST(N)*CCOLD(M,N)/DELT-AS(N)*CC(M,IM1)&
                            -BS(N)*CC(M,JM1)-CS(N)*CC(M,IP1)-DS(N)*CC(M,JP1)-ES(N)*CC(M,N)
                    end if
                    !C#
                    IF (CIT.AND.JFLAG2.NE.1) RHSS(N)=RHSS(N)-AOC(N)*CCOLD(M,IM1)&
                        -BOC(N)*CCOLD(M,JM1)-COC(N)*CCOLD(M,IP1)-DOC(N)*CCOLD(M,JP1)&
                        -EOC(N)*CCOLD(M,N)
                    IF(QQ(N).GT.0.0D0 .and. ntyp(n).ne.1) then
                        RHSS(N)=RHSS(N)-QQ(N)*CSS(M,N)
                    endif

                    IF(QS(N).LT.0.0D0 .AND. NCTYP(N).EQ.0) then
                        if(cit.and.jflag2.ne.1) then
                            RHSS(N)=RHSS(N)+0.5d0*(QS(N)+dum(n))*CSS(M,N)
                        else
                            RHSS(N) = RHSS(N)+ QS(N)*CSS(M,N)
                        end if
                    end if
                    IF(QS(N).LE.0.0D0 .AND.NCTYP(N).EQ.2)RHSS(N)=RHSS(N)-CSS(M,N)

30          CONTINUE

            NIS1=NIS1+1
            !
            !   CALL MATRIX SOLVER
            !
            if (.not. use_gmres_solute) then
                CALL SLVSIPSOL
                DO 31 I=2,NXRR
                    N1=NLY*(I-1)
                    DO 31 J=2,NLYY
                        N=N1+J
                        CC(M,N)=TempC(N)
31              CONTINUE
            else
                !   installing gmress solver. No need for iterating on solute equation
                !    first step is to move coefficients into storate gmres storage
                !    arrays. We need to reorder nodes.
                !
                ITESTS = 1
                ia_gmr = 0
                ja_gmr = 0
                a_gmr = 0.0d0
                xis = 0.0d0
                rhs_gmr = 0.0d0
                n_order = 0
                nz_num = 0
                nly2 = nly - 2
                DO 300 I=2,NXRR
                    N1=NLY*(I-1)
                    DO 300 J=2,NLYY
                        N=N1+J
                        if(hx(n).eq.0.0d0.or.nctyp(n).eq.1) then
                            n_order = n_order + 1
                            nz_num = nz_num + 1
                            a_gmr(nz_num) = 1.0d0
                            !       ia_gmr(nz_num) = n_order
                            ia_gmr(n_order) = nz_num
                            ja_gmr(nz_num) = n_order
                            rhs_gmr(n_order) = 0.0d0
                            xis(n_order) = 0.0d0
                        else
                            n_order = n_order + 1
                            nz_num = nz_num + 1
                            a_gmr(nz_num) = es(n)
                            !       ia_gmr(nz_num) = n_order
                            ia_gmr(n_order) = nz_num
                            ja_gmr(nz_num) = n_order
                            rhs_gmr(n_order) = rhss(n)
                            xis(n_order) = 0.0d0
                            if(as(n).ne.0.0d0) then
                                nz_num = nz_num + 1
                                a_gmr(nz_num) = as(n)
                                !         ia_gmr(nz_num) = n_order
                                ja_gmr(nz_num) = n_order - nly2
                            end if
                            if(bs(n).ne.0.0d0) then
                                nz_num = nz_num +1
                                a_gmr(nz_num) = bs(n)
                                !         ia_gmr(nz_num) = n_order
                                ja_gmr(nz_num) = n_order - 1
                            end if
                            if(cs(n).ne.0.0d0) then
                                nz_num = nz_num +1
                                a_gmr(nz_num) = cs(n)
                                !         ia_gmr(nz_num) = n_order
                                ja_gmr(nz_num) = n_order +  nly2
                            end if
                            if(ds(n).ne.0.0d0) then
                                nz_num = nz_num +1
                                a_gmr(nz_num) = ds(n)
                                !         ia_gmr(nz_num) = n_order
                                ja_gmr(nz_num) = n_order + 1
                            end if
                        end if
300             continue
                ia_gmr(n_order+1) = nz_num + 1
                itmax1 = itmax/10
                !      mr = n_order - 1
                !      mr = 200
                mr = MIN0(20,n_order-1)
                solved = pmgmres_ilu_cr ( n_order, nz_num, ia_gmr, ja_gmr, a_gmr, xis, rhs_gmr, itmax1, mr, &
                    eps3, eps3 )
                if(solved) then
                    n_order = 0
                    DO 301 I=2,NXRR
                        N1=NLY*(I-1)
                        DO 301 J=2,NLYY
                            N=N1+J
                            n_order = n_order + 1
                            if(hx(n).ne.0.0d0.and.nctyp(n).ne.1) then
                                cc(m,n) = cc(m,n) + xis(n_order)
                            end if
301                 continue
                    ITESTS = 0
                    write(stderr,*) "    ", compname(m)
                else
                    JSTOP=10
                    JFLAG=1
                    write(stderr,*) 'ERROR: MAXIMUM NUMBER OF ITERATIONS EXCEEDED FOR SOLUTE'&
                        ,' TRANSPORT EQUATION '
                    WRITE(6,4000)
                    RETURN
                endif
            endif


            !C      WRITE(6,*)'TempC After ########### ',M
            !      CALL VSOUTS(1,TempC(N))
            IF(ITESTS.EQ.0) THEN
                IF (CIT) THEN
                    DO 40 I=2,NXRR
                        N1=NLY*(I-1)
                        DO 40 J=2,NLYY
                            N=N1+J
                            IF(HX(N).EQ.0.0D0) GO TO 40
                            if(nctyp(n).ne.1) then
                                AOC(N)=AS(N)
                                BOC(N)=BS(N)
                                COC(N)=CS(N)
                                DOC(N)=DS(N)
                            end if
                            IF(RAD) THEN
                                AREAZ=PI2*DXR(I)*RX(I)
                            ELSE
                                AREAZ=DXR(I)
                            END IF
                            VOL=AREAZ*DELZ(J)
                            N2=JTEX(N)
                            SS=theta(n)*(P(N)-PXXX(N))*HK(N2,2)/HK(N2,3)
                            !
                            !  CHANGE 8-12-91 FOR STORAGE
                            !
                            !      SS1=HT(N2,4)*(THETA(N)+RET(N))
                            !
                            !****************
                            !c  following change made 7-3-04 to correct dctheta/dt
                            !   calculation
                            !
                            !****************
                            !      if(jflag1.ne.1.or.ntyp(n).ne.1) then
                            !       SS=THETA(N)+SS-THLST(N)
                            !      end if
                            !****************
                            EOC(N)=ES(N)+VOL*(THETA(N)+SS)/DELT
                            IF(JFLAG1.EQ.1) THEN
                                if(nctyp(n).ne.1) then
                                    AOC(N)=0.5D0*AOC(N)
                                    BOC(N)=0.5D0*BOC(N)
                                    COC(N)=0.5D0*COC(N)
                                    DOC(N)=0.5D0*DOC(N)
                                end if
                                EOC(N)=0.5D0*EOC(N)
                            END IF
40                  CONTINUE
                END IF
                if (it > itmax/2) write(stderr,*) '***Solute iterations: ', it, compname(m)
                go to 60
            END IF

50      CONTINUE        
        WRITE(6,4000)
        IF (.NOT.ITSTOP) GO TO 60
        JSTOP=10
        JFLAG=1
        write(stderr,*) 'ERROR: MAXIMUM NUMBER OF ITERATIONS EXCEEDED FOR SOLUTE'&
        ,' TRANSPORT EQUATION '
        WRITE(6,4000)
        RETURN
60  CONTINUE 
    RETURN
4000 FORMAT('MAXIMUM NUMBER OF ITERATIONS EXCEEDED FOR SOLUTE '&
    ,' TRANSPORT EQUATION')
4010 FORMAT(' Simulation terminated')
    END SUBROUTINE VTSETUPSOL_PARALLEL