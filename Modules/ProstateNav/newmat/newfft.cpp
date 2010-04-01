//$ newfft.cpp

// This is originally by Sande and Gentleman in 1967! I have translated from
// Fortran into C and a little bit of C++.

// It takes about twice as long as fftw
// (http://theory.lcs.mit.edu/~fftw/homepage.html)
// but is much shorter than fftw  and so despite its age
// might represent a reasonable
// compromise between speed and complexity.
// If you really need the speed get fftw.


//    THIS SUBROUTINE WAS WRITTEN BY G.SANDE OF PRINCETON UNIVERSITY AND
//    W.M.GENTLMAN OF THE BELL TELEPHONE LAB.  IT WAS BROUGHT TO LONDON
//    BY DR. M.D. GODFREY AT THE IMPERIAL COLLEGE AND WAS ADAPTED FOR
//    BURROUGHS 6700 BY D. R. BRILLINGER AND J. PEMBERTON
//    IT REPRESENTS THE STATE OF THE ART OF COMPUTING COMPLETE FINITE
//    DISCRETE FOURIER TRANSFORMS AS OF NOV.1967.
//    OTHER PROGRAMS REQUIRED.
//                                 ONLY THOSE SUBROUTINES INCLUDED HERE.
//                      USAGE.
//       CALL AR1DFT(N,X,Y)
//            WHERE  N IS THE NUMBER OF POINTS IN THE SEQUENCE .
//                   X - IS A ONE-DIMENSIONAL ARRAY CONTAINING THE REAL
//                       PART OF THE SEQUENCE.
//                   Y - IS A ONE-DIMENSIONAL ARRAY CONTAINING THE
//                       IMAGINARY PART OF THE SEQUENCE.
//    THE TRANSFORM IS RETURNED IN X AND Y.
//            METHOD
//               FOR A GENERAL DISCUSSION OF THESE TRANSFORMS AND OF
//    THE FAST METHOD FOR COMPUTING THEM, SEE GENTLEMAN AND SANDE,
//    @FAST FOURIER TRANSFORMS - FOR FUN AND PROFIT,@ 1966 FALL JOINT
//    COMPUTER CONFERENCE.
//    THIS PROGRAM COMPUTES THIS FOR A COMPLEX SEQUENCE Z(T) OF LENGTH
//    N WHOSE ELEMENTS ARE STORED AT(X(I) , Y(I)) AND RETURNS THE
//    TRANSFORM COEFFICIENTS AT (X(I), Y(I)).
//        DESCRIPTION
//    AR1DFT IS A HIGHLY MODULAR ROUTINE CAPABLE OF COMPUTING IN PLACE
//    THE COMPLETE FINITE DISCRETE FOURIER TRANSFORM  OF A ONE-
//    DIMENSIONAL SEQUENCE OF RATHER GENERAL LENGTH N.
//       THE MAIN ROUTINE , AR1DFT ITSELF, FACTORS N. IT THEN CALLS ON
//    ON GR 1D FT TO COMPUTE THE ACTUAL TRANSFORMS, USING THESE FACTORS.
//    THIS GR 1D FT DOES, CALLING AT EACH STAGE ON THE APPROPRIATE KERN
//    EL R2FTK, R4FTK, R8FTK, R16FTK, R3FTK, R5FTK, OR RPFTK TO PERFORM
//    THE COMPUTATIONS FOR THIS PASS OVER THE SEQUENCE, DEPENDING ON
//    WHETHER THE CORRESPONDING FACTOR IS 2, 4, 8, 16, 3, 5, OR SOME
//    MORE GENERAL PRIME P. WHEN GR1DFT IS FINISHED THE TRANSFORM IS
//    COMPUTED, HOWEVER, THE RESULTS ARE STORED IN "DIGITS REVERSED"
//    ORDER. AR1DFT THEREFORE, CALLS UPON GR 1S FS TO SORT THEM OUT.
//    TO RETURN TO THE FACTORIZATION, SINGLETON HAS POINTED OUT THAT
//    THE TRANSFORMS ARE MORE EFFICIENT IF THE SAMPLE SIZE N, IS OF THE
//    FORM B*A**2 AND B CONSISTS OF A SINGLE FACTOR.  IN SUCH A CASE
//    IF WE PROCESS THE FACTORS IN THE ORDER ABA  THEN
//    THE REORDERING CAN BE DONE AS FAST IN PLACE, AS WITH SCRATCH
//    STORAGE.  BUT AS B BECOMES MORE COMPLICATED, THE COST OF THE DIGIT
//    REVERSING DUE TO B PART BECOMES VERY EXPENSIVE IF WE TRY TO DO IT
//    IN PLACE.  IN SUCH A CASE IT MIGHT BE BETTER TO USE EXTRA STORAGE
//    A ROUTINE TO DO THIS IS, HOWEVER, NOT INCLUDED HERE.
//    ANOTHER FEATURE INFLUENCING THE FACTORIZATION IS THAT FOR ANY FIXED
//    FACTOR N WE CAN PREPARE A SPECIAL KERNEL WHICH WILL COMPUTE
//    THAT STAGE OF THE TRANSFORM MORE EFFICIENTLY THAN WOULD A KERNEL
//    FOR GENERAL FACTORS, ESPECIALLY IF THE GENERAL KERNEL HAD TO BE
//    APPLIED SEVERAL TIMES. FOR EXAMPLE, FACTORS OF 4 ARE MORE
//    EFFICIENT THAN FACTORS OF 2, FACTORS OF 8 MORE EFFICIENT THAN 4,ETC
//    ON THE OTHER HAND DIMINISHING RETURNS RAPIDLY SET IN, ESPECIALLY
//    SINCE THE LENGTH OF THE KERNEL FOR A SPECIAL CASE IS ROUGHLY
//    PROPORTIONAL TO THE FACTOR IT DEALS WITH. HENCE THESE PROBABLY ARE
//    ALL THE KERNELS WE WISH TO HAVE.
//            RESTRICTIONS.
//    AN UNFORTUNATE FEATURE OF THE SORTING PROBLEM IS THAT THE MOST
//    EFFICIENT WAY TO DO IT IS WITH NESTED DO LOOPS, ONE FOR EACH
//    FACTOR. THIS PUTS A RESTRICTION ON N AS TO HOW MANY FACTORS IT
//    CAN HAVE.  CURRENTLY THE LIMIT IS 16, BUT THE LIMIT CAN BE READILY
//    RAISED IF NECESSARY.
//    A SECOND RESTRICTION OF THE PROGRAM IS THAT LOCAL STORAGE OF THE
//    THE ORDER P**2 IS REQUIRED BY THE GENERAL KERNEL RPFTK, SO SOME
//    LIMIT MUST BE SET ON P.  CURRENTLY THIS IS 19, BUT IT CAN BE INCRE
//    INCREASED BY TRIVIAL CHANGES.
//       OTHER COMMENTS.
//(1) THE ROUTINE IS ADAPTED TO CHECK WHETHER A GIVEN N WILL MEET THE
//    ABOVE FACTORING REQUIREMENTS AN, IF NOT, TO RETURN THE NEXT HIGHER
//    NUMBER, NX, SAY, WHICH WILL MEET THESE REQUIREMENTS.
//    THIS CAN BE ACCHIEVED BY   A STATEMENT OF THE FORM
//            CALL FACTR(N,X,Y).
//    IF A DIFFERENT N, SAY NX, IS RETURNED THEN THE TRANSFORMS COULD BE
//    OBTAINED BY EXTENDING THE SIZE OF THE X-ARRAY AND Y-ARRAY TO NX,
//    AND SETTING X(I) = Y(I) = 0., FOR I = N+1, NX.
//(2) IF THE SEQUENCE Z IS ONLY A REAL SEQUENCE, THEN THE IMAGINARY PART
//    Y(I)=0., THIS WILL RETURN THE COSINE TRANSFORM OF THE REAL SEQUENCE
//    IN X, AND THE SINE TRANSFORM IN Y.


#define WANT_STREAM

#define WANT_MATH

#include "newmatap.h"

#ifdef use_namespace
namespace NEWMAT {
#endif

#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,20); ++ExeCount; }
#else
#define REPORT {}
#endif

inline Real square(Real x) { return x*x; }
inline int square(int x) { return x*x; }

static void GR_1D_FS (int PTS, int N_SYM, int N_UN_SYM,
   const SimpleIntArray& SYM, int P_SYM, const SimpleIntArray& UN_SYM,
   Real* X, Real* Y);
static void GR_1D_FT (int N, int N_FACTOR, const SimpleIntArray& FACTOR,
   Real* X, Real* Y);
static void R_P_FTK (int N, int M, int P, Real* X, Real* Y);
static void R_2_FTK (int N, int M, Real* X0, Real* Y0, Real* X1, Real* Y1);
static void R_3_FTK (int N, int M, Real* X0, Real* Y0, Real* X1, Real* Y1,
   Real* X2, Real* Y2);
static void R_4_FTK (int N, int M,
   Real* X0, Real* Y0, Real* X1, Real* Y1,
   Real* X2, Real* Y2, Real* X3, Real* Y3);
static void R_5_FTK (int N, int M,
   Real* X0, Real* Y0, Real* X1, Real* Y1, Real* X2, Real* Y2,
   Real* X3, Real* Y3, Real* X4, Real* Y4);
static void R_8_FTK (int N, int M,
   Real* X0, Real* Y0, Real* X1, Real* Y1,
   Real* X2, Real* Y2, Real* X3, Real* Y3,
   Real* X4, Real* Y4, Real* X5, Real* Y5,
   Real* X6, Real* Y6, Real* X7, Real* Y7);
static void R_16_FTK (int N, int M,
   Real* X0, Real* Y0, Real* X1, Real* Y1,
   Real* X2, Real* Y2, Real* X3, Real* Y3,
   Real* X4, Real* Y4, Real* X5, Real* Y5,
   Real* X6, Real* Y6, Real* X7, Real* Y7,
   Real* X8, Real* Y8, Real* X9, Real* Y9,
   Real* X10, Real* Y10, Real* X11, Real* Y11,
   Real* X12, Real* Y12, Real* X13, Real* Y13,
   Real* X14, Real* Y14, Real* X15, Real* Y15);
static int BitReverse(int x, int prod, int n, const SimpleIntArray& f);


bool FFT_Controller::ar_1d_ft (int PTS, Real* X, Real *Y)
{
//    ARBITRARY RADIX ONE DIMENSIONAL FOURIER TRANSFORM

   REPORT

   int  F,J,N,NF,P,PMAX,P_SYM,P_TWO,Q,R,TWO_GRP;

   // NP is maximum number of squared factors allows PTS up to 2**32 at least
   // NQ is number of not-squared factors - increase if we increase PMAX
   const int NP = 16, NQ = 10;
   SimpleIntArray PP(NP), QQ(NQ);

   TWO_GRP=16; PMAX=19;

   // PMAX is the maximum factor size
   // TWO_GRP is the maximum power of 2 handled as a single factor
   // Doesn't take advantage of combining powers of 2 when calculating
   // number of factors

   if (PTS<=1) return true;
   N=PTS; P_SYM=1; F=2; P=0; Q=0;

   // P counts the number of squared factors
   // Q counts the number of the rest
   // R = 0 for no non-squared factors; 1 otherwise

   // FACTOR holds all the factors - non-squared ones in the middle
   //   - length is 2*P+Q
   // SYM also holds all the factors but with the non-squared ones
   //   multiplied together - length is 2*P+R
   // PP holds the values of the squared factors - length is P
   // QQ holds the values of the rest - length is Q

   // P_SYM holds the product of the squared factors

   // find the factors - load into PP and QQ
   while (N > 1)
   {
      bool fail = true;
      for (J=F; J<=PMAX; J++)
         if (N % J == 0) { fail = false; F=J; break; }
      if (fail || P >= NP || Q >= NQ) return false; // can't factor
      N /= F;
      if (N % F != 0) QQ[Q++] = F;
      else { N /= F; PP[P++] = F; P_SYM *= F; }
   }

   R = (Q == 0) ? 0 : 1;  // R = 0 if no not-squared factors, 1 otherwise

   NF = 2*P + Q;
   SimpleIntArray FACTOR(NF + 1), SYM(2*P + R);
   FACTOR[NF] = 0;                // we need this in the "combine powers of 2"

   // load into SYM and FACTOR
   for (J=0; J<P; J++)
      { SYM[J]=FACTOR[J]=PP[P-1-J]; FACTOR[P+Q+J]=SYM[P+R+J]=PP[J]; }

   if (Q>0)
   {
      REPORT
      for (J=0; J<Q; J++) FACTOR[P+J]=QQ[J];
      SYM[P]=PTS/square(P_SYM);
   }

   // combine powers of 2
   P_TWO = 1;
   for (J=0; J < NF; J++)
   {
      if (FACTOR[J]!=2) continue;
      P_TWO=P_TWO*2; FACTOR[J]=1;
      if (P_TWO<TWO_GRP && FACTOR[J+1]==2) continue;
      FACTOR[J]=P_TWO; P_TWO=1;
   }

   if (P==0) R=0;
   if (Q<=1) Q=0;

   // do the analysis
   GR_1D_FT(PTS,NF,FACTOR,X,Y);                 // the transform
   GR_1D_FS(PTS,2*P+R,Q,SYM,P_SYM,QQ,X,Y);      // the reshuffling

   return true;

}

static void GR_1D_FS (int PTS, int N_SYM, int N_UN_SYM,
   const SimpleIntArray& SYM, int P_SYM, const SimpleIntArray& UN_SYM,
   Real* X, Real* Y)
{
//    GENERAL RADIX ONE DIMENSIONAL FOURIER SORT

// PTS = number of points
// N_SYM = length of SYM
// N_UN_SYM = length of UN_SYM
// SYM: squared factors + product of non-squared factors + squared factors
// P_SYM = product of squared factors (each included only once)
// UN_SYM: not-squared factors

   REPORT

   Real T;
   int  JJ,KK,P_UN_SYM;

   // I have replaced the multiple for-loop used by Sande-Gentleman code
   // by the following code which does not limit the number of factors

   if (N_SYM > 0)
   {
      REPORT
      SimpleIntArray U(N_SYM);
      for(MultiRadixCounter MRC(N_SYM, SYM, U); !MRC.Finish(); ++MRC)
      {
         if (MRC.Swap())
         {
            int P = MRC.Reverse(); int JJ = MRC.Counter(); Real T;
            T=X[JJ]; X[JJ]=X[P]; X[P]=T; T=Y[JJ]; Y[JJ]=Y[P]; Y[P]=T;
         }
      }
   }

   int J,JL,K,L,M,MS;

   // UN_SYM contains the non-squared factors
   // I have replaced the Sande-Gentleman code as it runs into
   // integer overflow problems
   // My code (and theirs) would be improved by using a bit array
   // as suggested by Van Loan

   if (N_UN_SYM==0) { REPORT return; }
   P_UN_SYM=PTS/square(P_SYM); JL=(P_UN_SYM-3)*P_SYM; MS=P_UN_SYM*P_SYM;

   for (J = P_SYM; J<=JL; J+=P_SYM)
   {
      K=J;
      do K = P_SYM * BitReverse(K / P_SYM, P_UN_SYM, N_UN_SYM, UN_SYM);
      while (K<J);

      if (K!=J)
      {
         REPORT
         for (L=0; L<P_SYM; L++) for (M=L; M<PTS; M+=MS)
         {
            JJ=M+J; KK=M+K;
            T=X[JJ]; X[JJ]=X[KK]; X[KK]=T; T=Y[JJ]; Y[JJ]=Y[KK]; Y[KK]=T;
         }
      }
   }

   return;
}

static void GR_1D_FT (int N, int N_FACTOR, const SimpleIntArray& FACTOR,
   Real* X, Real* Y)
{
//    GENERAL RADIX ONE DIMENSIONAL FOURIER TRANSFORM;

   REPORT

   int  M = N;

   for (int i = 0; i < N_FACTOR; i++)
   {
      int P = FACTOR[i]; M /= P;

      switch(P)
      {
      case 1: REPORT break;
      case 2: REPORT R_2_FTK (N,M,X,Y,X+M,Y+M); break;
      case 3: REPORT R_3_FTK (N,M,X,Y,X+M,Y+M,X+2*M,Y+2*M); break;
      case 4: REPORT R_4_FTK (N,M,X,Y,X+M,Y+M,X+2*M,Y+2*M,X+3*M,Y+3*M); break;
      case 5:
         REPORT
         R_5_FTK (N,M,X,Y,X+M,Y+M,X+2*M,Y+2*M,X+3*M,Y+3*M,X+4*M,Y+4*M);
         break;
      case 8:
         REPORT
         R_8_FTK (N,M,X,Y,X+M,Y+M,X+2*M,Y+2*M,
            X+3*M,Y+3*M,X+4*M,Y+4*M,X+5*M,Y+5*M,
            X+6*M,Y+6*M,X+7*M,Y+7*M);
         break;
      case 16:
         REPORT
         R_16_FTK (N,M,X,Y,X+M,Y+M,X+2*M,Y+2*M,
            X+3*M,Y+3*M,X+4*M,Y+4*M,X+5*M,Y+5*M,
            X+6*M,Y+6*M,X+7*M,Y+7*M,X+8*M,Y+8*M,
            X+9*M,Y+9*M,X+10*M,Y+10*M,X+11*M,Y+11*M,
            X+12*M,Y+12*M,X+13*M,Y+13*M,X+14*M,Y+14*M,
            X+15*M,Y+15*M);
         break;
      default: REPORT R_P_FTK (N,M,P,X,Y); break;
      }
   }

}

static void R_P_FTK (int N, int M, int P, Real* X, Real* Y)
//    RADIX PRIME FOURIER TRANSFORM KERNEL;
// X and Y are treated as M * P matrices with Fortran storage
{
   REPORT
   bool NO_FOLD,ZERO;
   Real ANGLE,IS,IU,RS,RU,T,TWOPI,XT,YT;
   int  J,JJ,K0,K,M_OVER_2,MP,PM,PP,U,V;

   Real AA [9][9], BB [9][9];
   Real A [18], B [18], C [18], S [18];
   Real IA [9], IB [9], RA [9], RB [9];

   TWOPI=8.0*atan(1.0);
   M_OVER_2=M/2+1; MP=M*P; PP=P/2; PM=P-1;

   for (U=0; U<PP; U++)
   {
      ANGLE=TWOPI*Real(U+1)/Real(P);
      JJ=P-U-2;
      A[U]=cos(ANGLE); B[U]=sin(ANGLE);
      A[JJ]=A[U]; B[JJ]= -B[U];
   }

   for (U=1; U<=PP; U++)
   {
      for (V=1; V<=PP; V++)
         { JJ=U*V-U*V/P*P; AA[V-1][U-1]=A[JJ-1]; BB[V-1][U-1]=B[JJ-1]; }
   }

   for (J=0; J<M_OVER_2; J++)
   {
      NO_FOLD = (J==0 || 2*J==M);
      K0=J;
      ANGLE=TWOPI*Real(J)/Real(MP); ZERO=ANGLE==0.0;
      C[0]=cos(ANGLE); S[0]=sin(ANGLE);
      for (U=1; U<PM; U++)
      {
         C[U]=C[U-1]*C[0]-S[U-1]*S[0];
         S[U]=S[U-1]*C[0]+C[U-1]*S[0];
      }
      goto L700;
   L500:
      REPORT
      if (NO_FOLD) { REPORT goto L1500; }
      REPORT
      NO_FOLD=true; K0=M-J;
      for (U=0; U<PM; U++)
         { T=C[U]*A[U]+S[U]*B[U]; S[U]= -S[U]*A[U]+C[U]*B[U]; C[U]=T; }
   L700:
      REPORT
      for (K=K0; K<N; K+=MP)
      {
         XT=X[K]; YT=Y[K];
         for (U=1; U<=PP; U++)
         {
            RA[U-1]=XT; IA[U-1]=YT;
            RB[U-1]=0.0; IB[U-1]=0.0;
         }
         for (U=1; U<=PP; U++)
         {
            JJ=P-U;
            RS=X[K+M*U]+X[K+M*JJ]; IS=Y[K+M*U]+Y[K+M*JJ];
            RU=X[K+M*U]-X[K+M*JJ]; IU=Y[K+M*U]-Y[K+M*JJ];
            XT=XT+RS; YT=YT+IS;
            for (V=0; V<PP; V++)
            {
               RA[V]=RA[V]+RS*AA[V][U-1]; IA[V]=IA[V]+IS*AA[V][U-1];
               RB[V]=RB[V]+RU*BB[V][U-1]; IB[V]=IB[V]+IU*BB[V][U-1];
            }
         }
         X[K]=XT; Y[K]=YT;
         for (U=1; U<=PP; U++)
         {
            if (!ZERO)
            {
               REPORT
               XT=RA[U-1]+IB[U-1]; YT=IA[U-1]-RB[U-1];
               X[K+M*U]=XT*C[U-1]+YT*S[U-1]; Y[K+M*U]=YT*C[U-1]-XT*S[U-1];
               JJ=P-U;
               XT=RA[U-1]-IB[U-1]; YT=IA[U-1]+RB[U-1];
               X[K+M*JJ]=XT*C[JJ-1]+YT*S[JJ-1];
               Y[K+M*JJ]=YT*C[JJ-1]-XT*S[JJ-1];
            }
            else
            {
               REPORT
               X[K+M*U]=RA[U-1]+IB[U-1]; Y[K+M*U]=IA[U-1]-RB[U-1];
               JJ=P-U;
               X[K+M*JJ]=RA[U-1]-IB[U-1]; Y[K+M*JJ]=IA[U-1]+RB[U-1];
            }
         }
      }
      goto L500;
L1500: ;
   }
   return;
}

static void R_2_FTK (int N, int M, Real* X0, Real* Y0, Real* X1, Real* Y1)
//    RADIX TWO FOURIER TRANSFORM KERNEL;
{
   REPORT
   bool NO_FOLD,ZERO;
   int  J,K,K0,M2,M_OVER_2;
   Real ANGLE,C,IS,IU,RS,RU,S,TWOPI;

   M2=M*2; M_OVER_2=M/2+1;
   TWOPI=8.0*atan(1.0);

   for (J=0; J<M_OVER_2; J++)
   {
      NO_FOLD = (J==0 || 2*J==M);
      K0=J;
      ANGLE=TWOPI*Real(J)/Real(M2); ZERO=ANGLE==0.0;
      C=cos(ANGLE); S=sin(ANGLE);
      goto L200;
   L100:
      REPORT
      if (NO_FOLD) { REPORT goto L600; }
      REPORT
      NO_FOLD=true; K0=M-J; C= -C;
   L200:
      REPORT
      for (K=K0; K<N; K+=M2)
      {
         RS=X0[K]+X1[K]; IS=Y0[K]+Y1[K];
         RU=X0[K]-X1[K]; IU=Y0[K]-Y1[K];
         X0[K]=RS; Y0[K]=IS;
         if (!ZERO) { X1[K]=RU*C+IU*S; Y1[K]=IU*C-RU*S; }
         else { X1[K]=RU; Y1[K]=IU; }
      }
      goto L100;
   L600: ;
   }

   return;
}

static void R_3_FTK (int N, int M, Real* X0, Real* Y0, Real* X1, Real* Y1,
   Real* X2, Real* Y2)
//    RADIX THREE FOURIER TRANSFORM KERNEL
{
   REPORT
   bool NO_FOLD,ZERO;
   int  J,K,K0,M3,M_OVER_2;
   Real ANGLE,A,B,C1,C2,S1,S2,T,TWOPI;
   Real I0,I1,I2,IA,IB,IS,R0,R1,R2,RA,RB,RS;

   M3=M*3; M_OVER_2=M/2+1; TWOPI=8.0*atan(1.0);
   A=cos(TWOPI/3.0); B=sin(TWOPI/3.0);

   for (J=0; J<M_OVER_2; J++)
   {
      NO_FOLD = (J==0 || 2*J==M);
      K0=J;
      ANGLE=TWOPI*Real(J)/Real(M3); ZERO=ANGLE==0.0;
      C1=cos(ANGLE); S1=sin(ANGLE);
      C2=C1*C1-S1*S1; S2=S1*C1+C1*S1;
      goto L200;
   L100:
      REPORT
      if (NO_FOLD) { REPORT goto L600; }
      REPORT
      NO_FOLD=true; K0=M-J;
      T=C1*A+S1*B; S1=C1*B-S1*A; C1=T;
      T=C2*A-S2*B; S2= -C2*B-S2*A; C2=T;
   L200:
      REPORT
      for (K=K0; K<N; K+=M3)
      {
         R0 = X0[K]; I0 = Y0[K];
         RS=X1[K]+X2[K]; IS=Y1[K]+Y2[K];
         X0[K]=R0+RS; Y0[K]=I0+IS;
         RA=R0+RS*A; IA=I0+IS*A;
         RB=(X1[K]-X2[K])*B; IB=(Y1[K]-Y2[K])*B;
         if (!ZERO)
         {
            REPORT
            R1=RA+IB; I1=IA-RB; R2=RA-IB; I2=IA+RB;
            X1[K]=R1*C1+I1*S1; Y1[K]=I1*C1-R1*S1;
            X2[K]=R2*C2+I2*S2; Y2[K]=I2*C2-R2*S2;
         }
         else { REPORT X1[K]=RA+IB; Y1[K]=IA-RB; X2[K]=RA-IB; Y2[K]=IA+RB; }
      }
      goto L100;
   L600: ;
   }

   return;
}

static void R_4_FTK (int N, int M,
   Real* X0, Real* Y0, Real* X1, Real* Y1,
   Real* X2, Real* Y2, Real* X3, Real* Y3)
//    RADIX FOUR FOURIER TRANSFORM KERNEL
{
   REPORT
   bool NO_FOLD,ZERO;
   int  J,K,K0,M4,M_OVER_2;
   Real ANGLE,C1,C2,C3,S1,S2,S3,T,TWOPI;
   Real I1,I2,I3,IS0,IS1,IU0,IU1,R1,R2,R3,RS0,RS1,RU0,RU1;

   M4=M*4; M_OVER_2=M/2+1;
   TWOPI=8.0*atan(1.0);

   for (J=0; J<M_OVER_2; J++)
   {
      NO_FOLD = (J==0 || 2*J==M);
      K0=J;
      ANGLE=TWOPI*Real(J)/Real(M4); ZERO=ANGLE==0.0;
      C1=cos(ANGLE); S1=sin(ANGLE);
      C2=C1*C1-S1*S1; S2=S1*C1+C1*S1;
      C3=C2*C1-S2*S1; S3=S2*C1+C2*S1;
      goto L200;
   L100:
      REPORT
      if (NO_FOLD) { REPORT goto L600; }
      REPORT
      NO_FOLD=true; K0=M-J;
      T=C1; C1=S1; S1=T;
      C2= -C2;
      T=C3; C3= -S3; S3= -T;
   L200:
      REPORT
      for (K=K0; K<N; K+=M4)
      {
         RS0=X0[K]+X2[K]; IS0=Y0[K]+Y2[K];
         RU0=X0[K]-X2[K]; IU0=Y0[K]-Y2[K];
         RS1=X1[K]+X3[K]; IS1=Y1[K]+Y3[K];
         RU1=X1[K]-X3[K]; IU1=Y1[K]-Y3[K];
         X0[K]=RS0+RS1; Y0[K]=IS0+IS1;
         if (!ZERO)
         {
            REPORT
            R1=RU0+IU1; I1=IU0-RU1;
            R2=RS0-RS1; I2=IS0-IS1;
            R3=RU0-IU1; I3=IU0+RU1;
            X2[K]=R1*C1+I1*S1; Y2[K]=I1*C1-R1*S1;
            X1[K]=R2*C2+I2*S2; Y1[K]=I2*C2-R2*S2;
            X3[K]=R3*C3+I3*S3; Y3[K]=I3*C3-R3*S3;
         }
         else
         {
            REPORT
            X2[K]=RU0+IU1; Y2[K]=IU0-RU1;
            X1[K]=RS0-RS1; Y1[K]=IS0-IS1;
            X3[K]=RU0-IU1; Y3[K]=IU0+RU1;
         }
      }
      goto L100;
   L600: ;
   }

   return;
}

static void R_5_FTK (int N, int M,
   Real* X0, Real* Y0, Real* X1, Real* Y1, Real* X2, Real* Y2,
   Real* X3, Real* Y3, Real* X4, Real* Y4)
//    RADIX FIVE FOURIER TRANSFORM KERNEL

{
   REPORT
   bool NO_FOLD,ZERO;
   int  J,K,K0,M5,M_OVER_2;
   Real ANGLE,A1,A2,B1,B2,C1,C2,C3,C4,S1,S2,S3,S4,T,TWOPI;
   Real R0,R1,R2,R3,R4,RA1,RA2,RB1,RB2,RS1,RS2,RU1,RU2;
   Real I0,I1,I2,I3,I4,IA1,IA2,IB1,IB2,IS1,IS2,IU1,IU2;

   M5=M*5; M_OVER_2=M/2+1;
   TWOPI=8.0*atan(1.0);
   A1=cos(TWOPI/5.0); B1=sin(TWOPI/5.0);
   A2=cos(2.0*TWOPI/5.0); B2=sin(2.0*TWOPI/5.0);

   for (J=0; J<M_OVER_2; J++)
   {
      NO_FOLD = (J==0 || 2*J==M);
      K0=J;
      ANGLE=TWOPI*Real(J)/Real(M5); ZERO=ANGLE==0.0;
      C1=cos(ANGLE); S1=sin(ANGLE);
      C2=C1*C1-S1*S1; S2=S1*C1+C1*S1;
      C3=C2*C1-S2*S1; S3=S2*C1+C2*S1;
      C4=C2*C2-S2*S2; S4=S2*C2+C2*S2;
      goto L200;
   L100:
      REPORT
      if (NO_FOLD) { REPORT goto L600; }
      REPORT
      NO_FOLD=true; K0=M-J;
      T=C1*A1+S1*B1; S1=C1*B1-S1*A1; C1=T;
      T=C2*A2+S2*B2; S2=C2*B2-S2*A2; C2=T;
      T=C3*A2-S3*B2; S3= -C3*B2-S3*A2; C3=T;
      T=C4*A1-S4*B1; S4= -C4*B1-S4*A1; C4=T;
   L200:
      REPORT
      for (K=K0; K<N; K+=M5)
      {
         R0=X0[K]; I0=Y0[K];
         RS1=X1[K]+X4[K]; IS1=Y1[K]+Y4[K];
         RU1=X1[K]-X4[K]; IU1=Y1[K]-Y4[K];
         RS2=X2[K]+X3[K]; IS2=Y2[K]+Y3[K];
         RU2=X2[K]-X3[K]; IU2=Y2[K]-Y3[K];
         X0[K]=R0+RS1+RS2; Y0[K]=I0+IS1+IS2;
         RA1=R0+RS1*A1+RS2*A2; IA1=I0+IS1*A1+IS2*A2;
         RA2=R0+RS1*A2+RS2*A1; IA2=I0+IS1*A2+IS2*A1;
         RB1=RU1*B1+RU2*B2; IB1=IU1*B1+IU2*B2;
         RB2=RU1*B2-RU2*B1; IB2=IU1*B2-IU2*B1;
         if (!ZERO)
         {
            REPORT
            R1=RA1+IB1; I1=IA1-RB1;
            R2=RA2+IB2; I2=IA2-RB2;
            R3=RA2-IB2; I3=IA2+RB2;
            R4=RA1-IB1; I4=IA1+RB1;
            X1[K]=R1*C1+I1*S1; Y1[K]=I1*C1-R1*S1;
            X2[K]=R2*C2+I2*S2; Y2[K]=I2*C2-R2*S2;
            X3[K]=R3*C3+I3*S3; Y3[K]=I3*C3-R3*S3;
            X4[K]=R4*C4+I4*S4; Y4[K]=I4*C4-R4*S4;
         }
         else
         {
            REPORT
            X1[K]=RA1+IB1; Y1[K]=IA1-RB1;
            X2[K]=RA2+IB2; Y2[K]=IA2-RB2;
            X3[K]=RA2-IB2; Y3[K]=IA2+RB2;
            X4[K]=RA1-IB1; Y4[K]=IA1+RB1;
         }
      }
      goto L100;
   L600: ;
   }

   return;
}

static void R_8_FTK (int N, int M,
   Real* X0, Real* Y0, Real* X1, Real* Y1,
   Real* X2, Real* Y2, Real* X3, Real* Y3,
   Real* X4, Real* Y4, Real* X5, Real* Y5,
   Real* X6, Real* Y6, Real* X7, Real* Y7)
//    RADIX EIGHT FOURIER TRANSFORM KERNEL
{
   REPORT
   bool NO_FOLD,ZERO;
   int  J,K,K0,M8,M_OVER_2;
   Real ANGLE,C1,C2,C3,C4,C5,C6,C7,E,S1,S2,S3,S4,S5,S6,S7,T,TWOPI;
   Real R1,R2,R3,R4,R5,R6,R7,RS0,RS1,RS2,RS3,RU0,RU1,RU2,RU3;
   Real I1,I2,I3,I4,I5,I6,I7,IS0,IS1,IS2,IS3,IU0,IU1,IU2,IU3;
   Real RSS0,RSS1,RSU0,RSU1,RUS0,RUS1,RUU0,RUU1;
   Real ISS0,ISS1,ISU0,ISU1,IUS0,IUS1,IUU0,IUU1;

   M8=M*8; M_OVER_2=M/2+1;
   TWOPI=8.0*atan(1.0); E=cos(TWOPI/8.0);

   for (J=0;J<M_OVER_2;J++)
   {
      NO_FOLD= (J==0 || 2*J==M);
      K0=J;
      ANGLE=TWOPI*Real(J)/Real(M8); ZERO=ANGLE==0.0;
      C1=cos(ANGLE); S1=sin(ANGLE);
      C2=C1*C1-S1*S1; S2=C1*S1+S1*C1;
      C3=C2*C1-S2*S1; S3=S2*C1+C2*S1;
      C4=C2*C2-S2*S2; S4=S2*C2+C2*S2;
      C5=C4*C1-S4*S1; S5=S4*C1+C4*S1;
      C6=C4*C2-S4*S2; S6=S4*C2+C4*S2;
      C7=C4*C3-S4*S3; S7=S4*C3+C4*S3;
      goto L200;
   L100:
      REPORT
      if (NO_FOLD) { REPORT goto L600; }
      REPORT
      NO_FOLD=true; K0=M-J;
      T=(C1+S1)*E; S1=(C1-S1)*E; C1=T;
      T=S2; S2=C2; C2=T;
      T=(-C3+S3)*E; S3=(C3+S3)*E; C3=T;
      C4= -C4;
      T= -(C5+S5)*E; S5=(-C5+S5)*E; C5=T;
      T= -S6; S6= -C6; C6=T;
      T=(C7-S7)*E; S7= -(C7+S7)*E; C7=T;
   L200:
      REPORT
      for (K=K0; K<N; K+=M8)
      {
         RS0=X0[K]+X4[K]; IS0=Y0[K]+Y4[K];
         RU0=X0[K]-X4[K]; IU0=Y0[K]-Y4[K];
         RS1=X1[K]+X5[K]; IS1=Y1[K]+Y5[K];
         RU1=X1[K]-X5[K]; IU1=Y1[K]-Y5[K];
         RS2=X2[K]+X6[K]; IS2=Y2[K]+Y6[K];
         RU2=X2[K]-X6[K]; IU2=Y2[K]-Y6[K];
         RS3=X3[K]+X7[K]; IS3=Y3[K]+Y7[K];
         RU3=X3[K]-X7[K]; IU3=Y3[K]-Y7[K];
         RSS0=RS0+RS2; ISS0=IS0+IS2;
         RSU0=RS0-RS2; ISU0=IS0-IS2;
         RSS1=RS1+RS3; ISS1=IS1+IS3;
         RSU1=RS1-RS3; ISU1=IS1-IS3;
         RUS0=RU0-IU2; IUS0=IU0+RU2;
         RUU0=RU0+IU2; IUU0=IU0-RU2;
         RUS1=RU1-IU3; IUS1=IU1+RU3;
         RUU1=RU1+IU3; IUU1=IU1-RU3;
         T=(RUS1+IUS1)*E; IUS1=(IUS1-RUS1)*E; RUS1=T;
         T=(RUU1+IUU1)*E; IUU1=(IUU1-RUU1)*E; RUU1=T;
         X0[K]=RSS0+RSS1; Y0[K]=ISS0+ISS1;
         if (!ZERO)
         {
            REPORT
            R1=RUU0+RUU1; I1=IUU0+IUU1;
            R2=RSU0+ISU1; I2=ISU0-RSU1;
            R3=RUS0+IUS1; I3=IUS0-RUS1;
            R4=RSS0-RSS1; I4=ISS0-ISS1;
            R5=RUU0-RUU1; I5=IUU0-IUU1;
            R6=RSU0-ISU1; I6=ISU0+RSU1;
            R7=RUS0-IUS1; I7=IUS0+RUS1;
            X4[K]=R1*C1+I1*S1; Y4[K]=I1*C1-R1*S1;
            X2[K]=R2*C2+I2*S2; Y2[K]=I2*C2-R2*S2;
            X6[K]=R3*C3+I3*S3; Y6[K]=I3*C3-R3*S3;
            X1[K]=R4*C4+I4*S4; Y1[K]=I4*C4-R4*S4;
            X5[K]=R5*C5+I5*S5; Y5[K]=I5*C5-R5*S5;
            X3[K]=R6*C6+I6*S6; Y3[K]=I6*C6-R6*S6;
            X7[K]=R7*C7+I7*S7; Y7[K]=I7*C7-R7*S7;
         }
         else
         {
            REPORT
            X4[K]=RUU0+RUU1; Y4[K]=IUU0+IUU1;
            X2[K]=RSU0+ISU1; Y2[K]=ISU0-RSU1;
            X6[K]=RUS0+IUS1; Y6[K]=IUS0-RUS1;
            X1[K]=RSS0-RSS1; Y1[K]=ISS0-ISS1;
            X5[K]=RUU0-RUU1; Y5[K]=IUU0-IUU1;
            X3[K]=RSU0-ISU1; Y3[K]=ISU0+RSU1;
            X7[K]=RUS0-IUS1; Y7[K]=IUS0+RUS1;
         }
      }
      goto L100;
   L600: ;
   }

   return;
}

static void R_16_FTK (int N, int M,
   Real* X0, Real* Y0, Real* X1, Real* Y1,
   Real* X2, Real* Y2, Real* X3, Real* Y3,
   Real* X4, Real* Y4, Real* X5, Real* Y5,
   Real* X6, Real* Y6, Real* X7, Real* Y7,
   Real* X8, Real* Y8, Real* X9, Real* Y9,
   Real* X10, Real* Y10, Real* X11, Real* Y11,
   Real* X12, Real* Y12, Real* X13, Real* Y13,
   Real* X14, Real* Y14, Real* X15, Real* Y15)
//    RADIX SIXTEEN FOURIER TRANSFORM KERNEL
{
   REPORT
   bool NO_FOLD,ZERO;
   int  J,K,K0,M16,M_OVER_2;
   Real ANGLE,EI1,ER1,E2,EI3,ER3,EI5,ER5,T,TWOPI;
   Real RS0,RS1,RS2,RS3,RS4,RS5,RS6,RS7;
   Real IS0,IS1,IS2,IS3,IS4,IS5,IS6,IS7;
   Real RU0,RU1,RU2,RU3,RU4,RU5,RU6,RU7;
   Real IU0,IU1,IU2,IU3,IU4,IU5,IU6,IU7;
   Real RUS0,RUS1,RUS2,RUS3,RUU0,RUU1,RUU2,RUU3;
   Real ISS0,ISS1,ISS2,ISS3,ISU0,ISU1,ISU2,ISU3;
   Real RSS0,RSS1,RSS2,RSS3,RSU0,RSU1,RSU2,RSU3;
   Real IUS0,IUS1,IUS2,IUS3,IUU0,IUU1,IUU2,IUU3;
   Real RSSS0,RSSS1,RSSU0,RSSU1,RSUS0,RSUS1,RSUU0,RSUU1;
   Real ISSS0,ISSS1,ISSU0,ISSU1,ISUS0,ISUS1,ISUU0,ISUU1;
   Real RUSS0,RUSS1,RUSU0,RUSU1,RUUS0,RUUS1,RUUU0,RUUU1;
   Real IUSS0,IUSS1,IUSU0,IUSU1,IUUS0,IUUS1,IUUU0,IUUU1;
   Real R1,R2,R3,R4,R5,R6,R7,R8,R9,R10,R11,R12,R13,R14,R15;
   Real I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14,I15;
   Real C1,C2,C3,C4,C5,C6,C7,C8,C9,C10,C11,C12,C13,C14,C15;
   Real S1,S2,S3,S4,S5,S6,S7,S8,S9,S10,S11,S12,S13,S14,S15;

   M16=M*16; M_OVER_2=M/2+1;
   TWOPI=8.0*atan(1.0);
   ER1=cos(TWOPI/16.0); EI1=sin(TWOPI/16.0);
   E2=cos(TWOPI/8.0);
   ER3=cos(3.0*TWOPI/16.0); EI3=sin(3.0*TWOPI/16.0);
   ER5=cos(5.0*TWOPI/16.0); EI5=sin(5.0*TWOPI/16.0);

   for (J=0; J<M_OVER_2; J++)
   {
      NO_FOLD = (J==0 || 2*J==M);
      K0=J;
      ANGLE=TWOPI*Real(J)/Real(M16);
      ZERO=ANGLE==0.0;
      C1=cos(ANGLE); S1=sin(ANGLE);
      C2=C1*C1-S1*S1; S2=C1*S1+S1*C1;
      C3=C2*C1-S2*S1; S3=S2*C1+C2*S1;
      C4=C2*C2-S2*S2; S4=S2*C2+C2*S2;
      C5=C4*C1-S4*S1; S5=S4*C1+C4*S1;
      C6=C4*C2-S4*S2; S6=S4*C2+C4*S2;
      C7=C4*C3-S4*S3; S7=S4*C3+C4*S3;
      C8=C4*C4-S4*S4; S8=C4*S4+S4*C4;
      C9=C8*C1-S8*S1; S9=S8*C1+C8*S1;
      C10=C8*C2-S8*S2; S10=S8*C2+C8*S2;
      C11=C8*C3-S8*S3; S11=S8*C3+C8*S3;
      C12=C8*C4-S8*S4; S12=S8*C4+C8*S4;
      C13=C8*C5-S8*S5; S13=S8*C5+C8*S5;
      C14=C8*C6-S8*S6; S14=S8*C6+C8*S6;
      C15=C8*C7-S8*S7; S15=S8*C7+C8*S7;
      goto L200;
   L100:
      REPORT
      if (NO_FOLD) { REPORT goto L600; }
      REPORT
      NO_FOLD=true; K0=M-J;
      T=C1*ER1+S1*EI1; S1= -S1*ER1+C1*EI1; C1=T;
      T=(C2+S2)*E2; S2=(C2-S2)*E2; C2=T;
      T=C3*ER3+S3*EI3; S3= -S3*ER3+C3*EI3; C3=T;
      T=S4; S4=C4; C4=T;
      T=S5*ER1-C5*EI1; S5=C5*ER1+S5*EI1; C5=T;
      T=(-C6+S6)*E2; S6=(C6+S6)*E2; C6=T;
      T=S7*ER3-C7*EI3; S7=C7*ER3+S7*EI3; C7=T;
      C8= -C8;
      T= -(C9*ER1+S9*EI1); S9=S9*ER1-C9*EI1; C9=T;
      T= -(C10+S10)*E2; S10=(-C10+S10)*E2; C10=T;
      T= -(C11*ER3+S11*EI3); S11=S11*ER3-C11*EI3; C11=T;
      T= -S12; S12= -C12; C12=T;
      T= -S13*ER1+C13*EI1; S13= -(C13*ER1+S13*EI1); C13=T;
      T=(C14-S14)*E2; S14= -(C14+S14)*E2; C14=T;
      T= -S15*ER3+C15*EI3; S15= -(C15*ER3+S15*EI3); C15=T;
   L200:
      REPORT
      for (K=K0; K<N; K+=M16)
      {
         RS0=X0[K]+X8[K]; IS0=Y0[K]+Y8[K];
         RU0=X0[K]-X8[K]; IU0=Y0[K]-Y8[K];
         RS1=X1[K]+X9[K]; IS1=Y1[K]+Y9[K];
         RU1=X1[K]-X9[K]; IU1=Y1[K]-Y9[K];
         RS2=X2[K]+X10[K]; IS2=Y2[K]+Y10[K];
         RU2=X2[K]-X10[K]; IU2=Y2[K]-Y10[K];
         RS3=X3[K]+X11[K]; IS3=Y3[K]+Y11[K];
         RU3=X3[K]-X11[K]; IU3=Y3[K]-Y11[K];
         RS4=X4[K]+X12[K]; IS4=Y4[K]+Y12[K];
         RU4=X4[K]-X12[K]; IU4=Y4[K]-Y12[K];
         RS5=X5[K]+X13[K]; IS5=Y5[K]+Y13[K];
         RU5=X5[K]-X13[K]; IU5=Y5[K]-Y13[K];
         RS6=X6[K]+X14[K]; IS6=Y6[K]+Y14[K];
         RU6=X6[K]-X14[K]; IU6=Y6[K]-Y14[K];
         RS7=X7[K]+X15[K]; IS7=Y7[K]+Y15[K];
         RU7=X7[K]-X15[K]; IU7=Y7[K]-Y15[K];
         RSS0=RS0+RS4; ISS0=IS0+IS4;
         RSS1=RS1+RS5; ISS1=IS1+IS5;
         RSS2=RS2+RS6; ISS2=IS2+IS6;
         RSS3=RS3+RS7; ISS3=IS3+IS7;
         RSU0=RS0-RS4; ISU0=IS0-IS4;
         RSU1=RS1-RS5; ISU1=IS1-IS5;
         RSU2=RS2-RS6; ISU2=IS2-IS6;
         RSU3=RS3-RS7; ISU3=IS3-IS7;
         RUS0=RU0-IU4; IUS0=IU0+RU4;
         RUS1=RU1-IU5; IUS1=IU1+RU5;
         RUS2=RU2-IU6; IUS2=IU2+RU6;
         RUS3=RU3-IU7; IUS3=IU3+RU7;
         RUU0=RU0+IU4; IUU0=IU0-RU4;
         RUU1=RU1+IU5; IUU1=IU1-RU5;
         RUU2=RU2+IU6; IUU2=IU2-RU6;
         RUU3=RU3+IU7; IUU3=IU3-RU7;
         T=(RSU1+ISU1)*E2; ISU1=(ISU1-RSU1)*E2; RSU1=T;
         T=(RSU3+ISU3)*E2; ISU3=(ISU3-RSU3)*E2; RSU3=T;
         T=RUS1*ER3+IUS1*EI3; IUS1=IUS1*ER3-RUS1*EI3; RUS1=T;
         T=(RUS2+IUS2)*E2; IUS2=(IUS2-RUS2)*E2; RUS2=T;
         T=RUS3*ER5+IUS3*EI5; IUS3=IUS3*ER5-RUS3*EI5; RUS3=T;
         T=RUU1*ER1+IUU1*EI1; IUU1=IUU1*ER1-RUU1*EI1; RUU1=T;
         T=(RUU2+IUU2)*E2; IUU2=(IUU2-RUU2)*E2; RUU2=T;
         T=RUU3*ER3+IUU3*EI3; IUU3=IUU3*ER3-RUU3*EI3; RUU3=T;
         RSSS0=RSS0+RSS2; ISSS0=ISS0+ISS2;
         RSSS1=RSS1+RSS3; ISSS1=ISS1+ISS3;
         RSSU0=RSS0-RSS2; ISSU0=ISS0-ISS2;
         RSSU1=RSS1-RSS3; ISSU1=ISS1-ISS3;
         RSUS0=RSU0-ISU2; ISUS0=ISU0+RSU2;
         RSUS1=RSU1-ISU3; ISUS1=ISU1+RSU3;
         RSUU0=RSU0+ISU2; ISUU0=ISU0-RSU2;
         RSUU1=RSU1+ISU3; ISUU1=ISU1-RSU3;
         RUSS0=RUS0-IUS2; IUSS0=IUS0+RUS2;
         RUSS1=RUS1-IUS3; IUSS1=IUS1+RUS3;
         RUSU0=RUS0+IUS2; IUSU0=IUS0-RUS2;
         RUSU1=RUS1+IUS3; IUSU1=IUS1-RUS3;
         RUUS0=RUU0+RUU2; IUUS0=IUU0+IUU2;
         RUUS1=RUU1+RUU3; IUUS1=IUU1+IUU3;
         RUUU0=RUU0-RUU2; IUUU0=IUU0-IUU2;
         RUUU1=RUU1-RUU3; IUUU1=IUU1-IUU3;
         X0[K]=RSSS0+RSSS1; Y0[K]=ISSS0+ISSS1;
         if (!ZERO)
         {
            REPORT
            R1=RUUS0+RUUS1; I1=IUUS0+IUUS1;
            R2=RSUU0+RSUU1; I2=ISUU0+ISUU1;
            R3=RUSU0+RUSU1; I3=IUSU0+IUSU1;
            R4=RSSU0+ISSU1; I4=ISSU0-RSSU1;
            R5=RUUU0+IUUU1; I5=IUUU0-RUUU1;
            R6=RSUS0+ISUS1; I6=ISUS0-RSUS1;
            R7=RUSS0+IUSS1; I7=IUSS0-RUSS1;
            R8=RSSS0-RSSS1; I8=ISSS0-ISSS1;
            R9=RUUS0-RUUS1; I9=IUUS0-IUUS1;
            R10=RSUU0-RSUU1; I10=ISUU0-ISUU1;
            R11=RUSU0-RUSU1; I11=IUSU0-IUSU1;
            R12=RSSU0-ISSU1; I12=ISSU0+RSSU1;
            R13=RUUU0-IUUU1; I13=IUUU0+RUUU1;
            R14=RSUS0-ISUS1; I14=ISUS0+RSUS1;
            R15=RUSS0-IUSS1; I15=IUSS0+RUSS1;
            X8[K]=R1*C1+I1*S1; Y8[K]=I1*C1-R1*S1;
            X4[K]=R2*C2+I2*S2; Y4[K]=I2*C2-R2*S2;
            X12[K]=R3*C3+I3*S3; Y12[K]=I3*C3-R3*S3;
            X2[K]=R4*C4+I4*S4; Y2[K]=I4*C4-R4*S4;
            X10[K]=R5*C5+I5*S5; Y10[K]=I5*C5-R5*S5;
            X6[K]=R6*C6+I6*S6; Y6[K]=I6*C6-R6*S6;
            X14[K]=R7*C7+I7*S7; Y14[K]=I7*C7-R7*S7;
            X1[K]=R8*C8+I8*S8; Y1[K]=I8*C8-R8*S8;
            X9[K]=R9*C9+I9*S9; Y9[K]=I9*C9-R9*S9;
            X5[K]=R10*C10+I10*S10; Y5[K]=I10*C10-R10*S10;
            X13[K]=R11*C11+I11*S11; Y13[K]=I11*C11-R11*S11;
            X3[K]=R12*C12+I12*S12; Y3[K]=I12*C12-R12*S12;
            X11[K]=R13*C13+I13*S13; Y11[K]=I13*C13-R13*S13;
            X7[K]=R14*C14+I14*S14; Y7[K]=I14*C14-R14*S14;
            X15[K]=R15*C15+I15*S15; Y15[K]=I15*C15-R15*S15;
         }
         else
         {
            REPORT
            X8[K]=RUUS0+RUUS1; Y8[K]=IUUS0+IUUS1;
            X4[K]=RSUU0+RSUU1; Y4[K]=ISUU0+ISUU1;
            X12[K]=RUSU0+RUSU1; Y12[K]=IUSU0+IUSU1;
            X2[K]=RSSU0+ISSU1; Y2[K]=ISSU0-RSSU1;
            X10[K]=RUUU0+IUUU1; Y10[K]=IUUU0-RUUU1;
            X6[K]=RSUS0+ISUS1; Y6[K]=ISUS0-RSUS1;
            X14[K]=RUSS0+IUSS1; Y14[K]=IUSS0-RUSS1;
            X1[K]=RSSS0-RSSS1; Y1[K]=ISSS0-ISSS1;
            X9[K]=RUUS0-RUUS1; Y9[K]=IUUS0-IUUS1;
            X5[K]=RSUU0-RSUU1; Y5[K]=ISUU0-ISUU1;
            X13[K]=RUSU0-RUSU1; Y13[K]=IUSU0-IUSU1;
            X3[K]=RSSU0-ISSU1; Y3[K]=ISSU0+RSSU1;
            X11[K]=RUUU0-IUUU1; Y11[K]=IUUU0+RUUU1;
            X7[K]=RSUS0-ISUS1; Y7[K]=ISUS0+RSUS1;
            X15[K]=RUSS0-IUSS1; Y15[K]=IUSS0+RUSS1;
         }
      }
      goto L100;
   L600: ;
   }

   return;
}

// can the number of points be factorised sufficiently
// for the fft to run

bool FFT_Controller::CanFactor(int PTS)
{
   REPORT
   const int NP = 16, NQ = 10, PMAX=19;

   if (PTS<=1) { REPORT return true; }

   int N = PTS, F = 2, P = 0, Q = 0;

   while (N > 1)
   {
      bool fail = true;
      for (int J = F; J <= PMAX; J++)
         if (N % J == 0) { fail = false; F=J; break; }
      if (fail || P >= NP || Q >= NQ) { REPORT return false; }
      N /= F;
      if (N % F != 0) Q++; else { N /= F; P++; }
   }

   return true;    // can factorise

}

bool FFT_Controller::OnlyOldFFT;         // static variable

// **************************** multi radix counter **********************

MultiRadixCounter::MultiRadixCounter(int nx, const SimpleIntArray& rx,
   SimpleIntArray& vx)
   : Radix(rx), Value(vx), n(nx), reverse(0),
      product(1), counter(0), finish(false)
{
   REPORT for (int k = 0; k < n; k++) { Value[k] = 0; product *= Radix[k]; }
}

void MultiRadixCounter::operator++()
{
   REPORT
   counter++; int p = product;
   for (int k = 0; k < n; k++)
   {
      Value[k]++; int p1 = p / Radix[k]; reverse += p1;
      if (Value[k] == Radix[k]) { REPORT Value[k] = 0; reverse -= p; p = p1; }
      else { REPORT return; }
   }
   finish = true;
}


static int BitReverse(int x, int prod, int n, const SimpleIntArray& f)
{
   // x = c[0]+f[0]*(c[1]+f[1]*(c[2]+...
   // return c[n-1]+f[n-1]*(c[n-2]+f[n-2]*(c[n-3]+...
   // prod is the product of the f[i]
   // n is the number of f[i] (don't assume f has the correct length)

   REPORT
   const int* d = f.Data() + n; int sum = 0; int q = 1;
   while (n--)
   {
      prod /= *(--d);
      int c = x / prod; x-= c * prod;
      sum += q * c; q *= *d;
   }
   return sum;
}


#ifdef use_namespace
}
#endif


