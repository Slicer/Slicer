
#define WANT_STREAM

#define WANT_MATH

#include "newmat.h"
#include "newmatio.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif



// test Kronecker Product


void trymatm()
{
   Tracer et("Twenty second test of Matrix package");
   Tracer::PrintTrace();

   {
      Tracer et1("Stage 1");


      Matrix A(2,3);
      A << 3 << 5 << 2
        << 4 << 1 << 6;

      Matrix B(4,3);
      B <<  7 <<  2 <<  9
        <<  1 <<  3 <<  6
        <<  4 << 10 <<  5
        << 11 <<  8 << 12;

      Matrix C(8, 9);

      C.Row(1) << 21 <<  6 << 27  << 35 << 10 << 45  << 14 <<  4 << 18;
      C.Row(2) <<  3 <<  9 << 18  <<  5 << 15 << 30  <<  2 <<  6 << 12;
      C.Row(3) << 12 << 30 << 15  << 20 << 50 << 25  <<  8 << 20 << 10;
      C.Row(4) << 33 << 24 << 36  << 55 << 40 << 60  << 22 << 16 << 24;

      C.Row(5) << 28 <<  8 << 36  <<  7 <<  2 <<  9  << 42 << 12 << 54;
      C.Row(6) <<  4 << 12 << 24  <<  1 <<  3 <<  6  <<  6 << 18 << 36;
      C.Row(7) << 16 << 40 << 20  <<  4 << 10 <<  5  << 24 << 60 << 30;
      C.Row(8) << 44 << 32 << 48  << 11 <<  8 << 12  << 66 << 48 << 72;

      Matrix AB = KP(A,B) - C; Print(AB);

      IdentityMatrix I1(10); IdentityMatrix I2(15); I2 *= 2;
      DiagonalMatrix D = KP(I1, I2) - IdentityMatrix(150) * 2;
      Print(D);
   }

   {
      Tracer et1("Stage 2");

      UpperTriangularMatrix A(3);
      A << 3 << 8 << 5
             << 7 << 2
                  << 4;
      UpperTriangularMatrix B(4);
      B << 4 << 1 << 7 << 2
             << 3 << 9 << 8
                  << 1 << 5
                       << 6;

      UpperTriangularMatrix C(12);

      C.Row(1) <<12<< 3<<21<< 6 <<32<< 8<<56<<16 <<20<< 5<<35<<10;
      C.Row(2)     << 9<<27<<24 << 0<<24<<72<<64 << 0<<15<<45<<40;
      C.Row(3)         << 3<<15 << 0<< 0<< 8<<40 << 0<< 0<< 5<<25;
      C.Row(4)             <<18 << 0<< 0<< 0<<48 << 0<< 0<< 0<<30;

      C.Row(5)                  <<28<< 7<<49<<14 << 8<< 2<<14<< 4;
      C.Row(6)                      <<21<<63<<56 << 0<< 6<<18<<16;
      C.Row(7)                          << 7<<35 << 0<< 0<< 2<<10;
      C.Row(8)                              <<42 << 0<< 0<< 0<<12;

      C.Row(9)                                   <<16<< 4<<28<< 8;
      C.Row(10)                                      <<12<<36<<32;
      C.Row(11)                                          << 4<<20;
      C.Row(12)                                              <<24;


      UpperTriangularMatrix AB = KP(A,B) - C; Print(AB);

      LowerTriangularMatrix BT = B.t(); Matrix N(12,12);

      N.Row(1) <<12 << 0<< 0<< 0 <<32<< 0<< 0<< 0 <<20<< 0<< 0<< 0;
      N.Row(2) << 3 << 9<< 0<< 0 << 8<<24<< 0<< 0 << 5<<15<< 0<< 0;
      N.Row(3) <<21 <<27<< 3<< 0 <<56<<72<< 8<< 0 <<35<<45<< 5<< 0;
      N.Row(4) << 6 <<24<<15<<18 <<16<<64<<40<<48 <<10<<40<<25<<30;

      N.Row(5) << 0 << 0<< 0<< 0 <<28<< 0<< 0<< 0 << 8<< 0<< 0<< 0;
      N.Row(6) << 0 << 0<< 0<< 0 << 7<<21<< 0<< 0 << 2<< 6<< 0<< 0;
      N.Row(7) << 0 << 0<< 0<< 0 <<49<<63<< 7<< 0 <<14<<18<< 2<< 0;
      N.Row(8) << 0 << 0<< 0<< 0 <<14<<56<<35<<42 << 4<<16<<10<<12;

      N.Row(9) << 0 << 0<< 0<< 0 << 0<< 0<< 0<< 0 <<16<< 0<< 0<< 0;
      N.Row(10)<< 0 << 0<< 0<< 0 << 0<< 0<< 0<< 0 << 4<<12<< 0<< 0;
      N.Row(11)<< 0 << 0<< 0<< 0 << 0<< 0<< 0<< 0 <<28<<36<< 4<< 0;
      N.Row(12)<< 0 << 0<< 0<< 0 << 0<< 0<< 0<< 0 << 8<<32<<20<<24;

      Matrix N1 = KP(A, BT); N1 -= N; Print(N1);
      AB << KP(A, BT); AB << (AB - N); Print(AB);
      BT << KP(A, BT); BT << (BT - N); Print(BT);

      LowerTriangularMatrix AT = A.t();
      N1 = KP(AT, B); N1 -= N.t(); Print(N1);
      AB << KP(AT, B); AB << (AB - N.t()); Print(AB);
      BT << KP(AT, B); BT << (BT - N.t()); Print(BT);
   }

   {
      Tracer et1("Stage 3");

      BandMatrix BMA(6,2,3);
      BMA.Row(1) << 5.25 << 4.75 << 2.25 << 1.75;
      BMA.Row(2) << 1.25 << 9.75 << 4.50 << 0.25 << 1.50;
      BMA.Row(3) << 7.75 << 1.50 << 3.00 << 4.25 << 0.50 << 5.50;
      BMA.Row(4) << 2.75 << 9.00 << 8.00 << 3.25 << 3.50;
      BMA.Row(5) << 8.75 << 6.25 << 5.00 << 5.75;
      BMA.Row(6) << 3.75 << 6.75 << 6.00;

      Matrix A = BMA;

      BandMatrix BMB(4,2,1);
      BMB.Row(1) << 4.5 << 9.5;
      BMB.Row(2) << 1.5 << 6.0 << 2.0;
      BMB.Row(3) << 0.5 << 2.5 << 8.5 << 7.5;
      BMB.Row(4) << 3.0 << 4.0 << 6.5;

      SquareMatrix B = BMB;

      BandMatrix BMC = KP(BMA, BMB);
      BandMatrix BMC1 = KP(BMA, B);
      Matrix C2 = KP(A, BMB);
      Matrix C = KP(A, B);

      Matrix M = C - BMC; Print(M);
      M = C - BMC1; Print(M);
      M = C - C2; Print(M);

      RowVector X(4);
      X(1) = BMC.BandWidth().Lower() - 10;
      X(2) = BMC.BandWidth().Upper() - 13;
      X(3) = BMC1.BandWidth().Lower() - 11;
      X(4) = BMC1.BandWidth().Upper() - 15;
      Print(X);

      UpperTriangularMatrix UT;  UT << KP(BMA, BMB);
      UpperTriangularMatrix UT1; UT1 << (C - UT); Print(UT1);
      LowerTriangularMatrix LT;  LT << KP(BMA, BMB);
      LowerTriangularMatrix LT1; LT1 << (C - LT); Print(LT1);
   }

   {
      Tracer et1("Stage 4");

      SymmetricMatrix SM1(4);
      SM1.Row(1) << 2;
      SM1.Row(2) << 4 << 5;
      SM1.Row(3) << 9 << 2 << 1;
      SM1.Row(4) << 3 << 6 << 8 << 2;

      SymmetricMatrix SM2(3);
      SM2.Row(1) <<  3;
      SM2.Row(2) << -7 << -6;
      SM2.Row(3) <<  4 << -2 << -1;

      SymmetricMatrix SM = KP(SM1, SM2);
      Matrix M1 = SM1; Matrix M2 = SM2;
      Matrix M = KP(SM1, SM2); M -= SM; Print(M);
      M = KP(SM1, SM2) - SM; Print(M);
      M = KP(M1, SM2) - SM; Print(M);
      M = KP(SM1, M2) - SM; Print(M);
      M = KP(M1, M2); M -= SM; Print(M);
   }

   {
      Tracer et1("Stage 5");

      Matrix A(2,3);
      A << 3 << 5 << 2
        << 4 << 1 << 6;

      Matrix B(3,4);
      B <<  7 <<  2 <<  9 << 11
        <<  1 <<  3 <<  6 <<  8
        <<  4 << 10 <<  5 << 12;

      RowVector C(2); C << 3 << 7;
      ColumnVector D(4); D << 0 << 5 << 13 << 11;

      Matrix M = KP(C * A, B * D) - KP(C, B) * KP(A, D); Print(M);
   }

   {
      Tracer et1("Stage 6");

      RowVector A(3), B(5), C(15);
      A << 5 << 2 << 4;
      B << 3 << 2 << 0 << 1 << 6;
      C << 15 << 10 << 0 << 5 << 30
        <<  6 <<  4 << 0 << 2 << 12
        << 12 <<  8 << 0 << 4 << 24;
      Matrix N = KP(A, B) - C;    Print(N);
      N = KP(A.t(), B.t()) - C.t();    Print(N);
      N = KP(A.AsDiagonal(), B.AsDiagonal()) - C.AsDiagonal();    Print(N);
   }

   {
      Tracer et1("Stage 7");
      IdentityMatrix I(3);
      ColumnVector CV(4); CV << 4 << 3 << 1 << 7;
      Matrix A = KP(I, CV) + 5;
      Matrix B(3,12);
      B.Row(1) << 9 << 8 << 6 << 12 << 5 << 5 << 5 << 5 << 5 << 5 << 5 << 5;
      B.Row(2) << 5 << 5 << 5 << 5 << 9 << 8 << 6 << 12 << 5 << 5 << 5 << 5;
      B.Row(3) << 5 << 5 << 5 << 5 << 5 << 5 << 5 << 5 << 9 << 8 << 6 << 12;
      B -= A.t(); Print(B);

   }

   {
      Tracer et1("Stage 8");          // SquareMatrix
      Matrix A(2,3), B(3,2);
      A << 2 << 6 << 7
        << 4 << 3 << 9;
      B << 1 << 3
        << 4 << 8
        << 0 << 6;
      SquareMatrix AB = A * B;
      Matrix M = (B.t() * A.t()).t(); M -= AB; Print(M);
      AB = B * A;
      M = (A.t() * B.t()).t(); M -= AB; Print(M);
      AB.ReSize(5,5); AB = 0;
      AB.SubMatrix(1,2,1,3) = A; AB.SubMatrix(4,5,3,5) = A;
      AB.SubMatrix(1,3,4,5) = B; AB.SubMatrix(3,5,1,2) = B;
      SquareMatrix C(5);
      C.Row(1) << 2 << 6 << 7 << 1 << 3;
      C.Row(2) << 4 << 3 << 9 << 4 << 8;
      C.Row(3) << 1 << 3 << 0 << 0 << 6;
      C.Row(4) << 4 << 8 << 2 << 6 << 7;
      C.Row(5) << 0 << 6 << 4 << 3 << 9;
      C -= AB; Print(C);
      AB = A.SymSubMatrix(1,2);
      AB = (AB | AB) & (AB | AB);
      C.ReSize(4);
      C.Row(1) << 2 << 6 << 2 << 6;
      C.Row(2) << 4 << 3 << 4 << 3;
      C.Row(3) << 2 << 6 << 2 << 6;
      C.Row(4) << 4 << 3 << 4 << 3;
      M = AB;
      C -= M; Print(C);
      C << M; C += -M; Print(C);
      
   }


}






