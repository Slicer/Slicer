
//#define WANT_STREAM

#include "include.h"

#include "newmatap.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif



// **************************** test program ******************************


void Transposer(const GenericMatrix& GM1, GenericMatrix&GM2)
   { GM2 = GM1.t(); }

// this is a routine in "Numerical Recipes in C" format
// if R is a row vector, C a column vector and D diagonal
// make matrix DCR

static void DCR(Real d[], Real c[], int m, Real r[], int n, Real **dcr)
{
   int i, j;
   for (i = 1; i <= m; i++) for (j = 1; j <= n; j++)
   dcr[i][j] = d[i] * c[i] * r[j];
}

ReturnMatrix TestReturn(const GeneralMatrix& gm) { return gm; }

void trymat8()
{
//   cout << "\nEighth test of Matrix package\n";
   Tracer et("Eighth test of Matrix package");
   Tracer::PrintTrace();

   int i;


   DiagonalMatrix D(6);
   for (i=1;i<=6;i++)  D(i,i)=i*i+i-10;
   DiagonalMatrix D2=D;
   Matrix MD=D;

   DiagonalMatrix D1(6); for (i=1;i<=6;i++) D1(i,i)=-100+i*i*i;
   Matrix MD1=D1;
   Print(Matrix(D*D1-MD*MD1));
   Print(Matrix((-D)*D1+MD*MD1));
   Print(Matrix(D*(-D1)+MD*MD1));
   DiagonalMatrix DX=D;
   {
      Tracer et1("Stage 1");
      DX=(DX+D1)*DX; Print(Matrix(DX-(MD+MD1)*MD));
      DX=D;
      DX=-DX*DX+(DX-(-D1))*((-D1)+DX);
      // Matrix MX = Matrix(MD1);
      // MD1=DX+(MX.t())*(MX.t()); Print(MD1);
      MD1=DX+(Matrix(MD1).t())*(Matrix(MD1).t()); Print(MD1);
      DX=D; DX=DX; DX=D2-DX; Print(DiagonalMatrix(DX));
      DX=D;
   }
   {
      Tracer et1("Stage 2");
      D.Release(2);
      D1=D; D2=D;
      Print(DiagonalMatrix(D1-DX));
      Print(DiagonalMatrix(D2-DX));
      MD1=1.0;
      Print(Matrix(MD1-1.0));
   }
   {
      Tracer et1("Stage 3");
      //GenericMatrix
      LowerTriangularMatrix LT(4);
      LT << 1 << 2 << 3 << 4 << 5 << 6  << 7 << 8 << 9 << 10;
      UpperTriangularMatrix UT = LT.t() * 2.0;
      GenericMatrix GM1 = LT;
      LowerTriangularMatrix LT1 = GM1-LT; Print(LT1);
      GenericMatrix GM2 = GM1; LT1 = GM2; LT1 = LT1-LT; Print(LT1);
      GM2 = GM1; LT1 = GM2; LT1 = LT1-LT; Print(LT1);
      GM2 = GM1*2; LT1 = GM2; LT1 = LT1-LT*2; Print(LT1);
      GM1.Release();
      GM1=GM1; LT1=GM1-LT; Print(LT1); LT1=GM1-LT; Print(LT1);
      GM1.Release();
      GM1=GM1*4; LT1=GM1-LT*4; Print(LT1);
      LT1=GM1-LT*4; Print(LT1); GM1.CleanUp();
      GM1=LT; GM2=UT; GM1=GM1*GM2; Matrix M=GM1; M=M-LT*UT; Print(M);
      Transposer(LT,GM2); LT1 = LT - GM2.t(); Print(LT1);
      GM1=LT; Transposer(GM1,GM2); LT1 = LT - GM2.t(); Print(LT1);
      GM1 = LT; GM1 = GM1 + GM1; LT1 = LT*2-GM1; Print(LT1);
      DiagonalMatrix D; D << LT; GM1 = D; LT1 = GM1; LT1 -= D; Print(LT1);
      UpperTriangularMatrix UT1 = GM1; UT1 -= D; Print(UT1);
   }
   {
      Tracer et1("Stage 4");
      // Another test of SVD
      Matrix M(12,12); M = 0;
      M(1,1) = M(2,2) = M(4,4) = M(6,6) =
         M(7,7) = M(8,8) = M(10,10) = M(12,12) = -1;
      M(1,6) = M(1,12) = -5.601594;
      M(3,6) = M(3,12) = -0.000165;
      M(7,6) = M(7,12) = -0.008294;
      DiagonalMatrix D;
      SVD(M,D);
      SortDescending(D);
      // answer given by matlab
      DiagonalMatrix DX(12);
      DX(1) = 8.0461;
      DX(2) = DX(3) = DX(4) = DX(5) = DX(6) = DX(7) = 1;
      DX(8) = 0.1243;
      DX(9) = DX(10) = DX(11) = DX(12) = 0;
      D -= DX; Clean(D,0.0001); Print(D);
   }
#ifndef DONT_DO_NRIC
   {
      Tracer et1("Stage 5");
      // test numerical recipes in C interface
      DiagonalMatrix D(10);
      D << 1 << 4 << 6 << 2 << 1 << 6 << 4 << 7 << 3 << 1;
      ColumnVector C(10);
      C << 3 << 7 << 5 << 1 << 4 << 2 << 3 << 9 << 1 << 3;
      RowVector R(6);
      R << 2 << 3 << 5 << 7 << 11 << 13;
      nricMatrix M(10, 6);
      DCR( D.nric(), C.nric(), 10, R.nric(), 6, M.nric() );
      M -= D * C * R;  Print(M);

      D.ReSize(5);
      D << 1.25 << 4.75 << 9.5 << 1.25 << 3.75;
      C.ReSize(5);
      C << 1.5 << 7.5 << 4.25 << 0.0 << 7.25;
      R.ReSize(9);
      R << 2.5 << 3.25 << 5.5 << 7 << 11.25 << 13.5 << 0.0 << 1.5 << 3.5;
      Matrix MX = D * C * R;
      M.ReSize(MX);
      DCR( D.nric(), C.nric(), 5, R.nric(), 9, M.nric() );
      M -= MX;  Print(M);
      
      // test swap
      nricMatrix A(3,4); nricMatrix B(4,5);
      A.Row(1) << 2 << 7 << 3 << 6;
      A.Row(2) << 6 << 2 << 5 << 9;
      A.Row(3) << 1 << 0 << 1 << 6;
      B.Row(1) << 2 << 8 << 4 << 5 << 3;
      B.Row(2) << 1 << 7 << 5 << 3 << 9;
      B.Row(3) << 7 << 8 << 2 << 1 << 6;
      B.Row(4) << 5 << 2 << 9 << 0 << 9;
      nricMatrix A1(1,2); nricMatrix B1;
      nricMatrix X(3,5); Matrix X1 = A * B;
      swap(A, A1); swap(B1, B);
      for (int i = 1; i <= 3; ++i) for (int j = 1; j <= 5; ++j)
      {
         X.nric()[i][j] = 0.0;
         for (int k = 1; k <= 4; ++k)
            X.nric()[i][j] += A1.nric()[i][k] * B1.nric()[k][j];
      }
      X1 -= X; Print(X1); 
   }
#endif
   {
      Tracer et1("Stage 6");
      // test dotproduct
      DiagonalMatrix test(5); test = 1;
      ColumnVector C(10);
      C << 3 << 7 << 5 << 1 << 4 << 2 << 3 << 9 << 1 << 3;
      RowVector R(10);
      R << 2 << 3 << 5 << 7 << 11 << 13 << -3 << -4 << 2 << 4;
      test(1) = (R * C).AsScalar() - DotProduct(C, R);
      test(2) = C.SumSquare() - DotProduct(C, C);
      test(3) = 6.0 * (C.t() * R.t()).AsScalar() - DotProduct(2.0 * C, 3.0 * R);
      Matrix MC = C.AsMatrix(2,5), MR = R.AsMatrix(5,2);
      test(4) = DotProduct(MC, MR) - (R * C).AsScalar();
      UpperTriangularMatrix UT(5);
      UT << 3 << 5 << 2 << 1 << 7
              << 1 << 1 << 8 << 2
                   << 7 << 0 << 1
                        << 3 << 5
                             << 6;
      LowerTriangularMatrix LT(5);
      LT << 5
         << 2 << 3
         << 1 << 0 << 7
         << 9 << 8 << 1 << 2
         << 0 << 2 << 1 << 9 << 2;
      test(5) = DotProduct(UT, LT) - Sum(SP(UT, LT));
      Print(test);
      // check row-wise load;
      LowerTriangularMatrix LT1(5);
      LT1.Row(1) << 5;
      LT1.Row(2) << 2   << 3;
      LT1.Row(3) << 1   << 0   << 7;
      LT1.Row(4) << 9   << 8   << 1   << 2;
      LT1.Row(5) << 0   << 2   << 1   << 9   << 2;
      Matrix M = LT1 - LT; Print(M);
      // check solution with identity matrix
      IdentityMatrix IM(5); IM *= 2;
      LinearEquationSolver LES1(IM);
      LowerTriangularMatrix LTX = LES1.i() * LT;
      M = LTX * 2 - LT; Print(M);
      DiagonalMatrix D = IM;
      LinearEquationSolver LES2(IM);
      LTX = LES2.i() * LT;
      M = LTX * 2 - LT; Print(M);
      UpperTriangularMatrix UTX = LES1.i() * UT;
      M = UTX * 2 - UT; Print(M);
      UTX = LES2.i() * UT;
      M = UTX * 2 - UT; Print(M);
   }

   {
      Tracer et1("Stage 7");
      // Some more GenericMatrix stuff with *= |= &=
      // but don't any additional checks
      BandMatrix BM1(6,2,3);
      BM1.Row(1) << 3 << 8 << 4 << 1;
      BM1.Row(2) << 5 << 1 << 9 << 7 << 2;
      BM1.Row(3) << 1 << 0 << 6 << 3 << 1 << 3;
      BM1.Row(4)      << 4 << 2 << 5 << 2 << 4;
      BM1.Row(5)           << 3 << 3 << 9 << 1;
      BM1.Row(6)                << 4 << 2 << 9;
      BandMatrix BM2(6,1,1);
      BM2.Row(1) << 2.5 << 7.5;
      BM2.Row(2) << 1.5 << 3.0 << 8.5;
      BM2.Row(3)        << 6.0 << 6.5 << 7.0;
      BM2.Row(4)               << 2.5 << 2.0 << 8.0;
      BM2.Row(5)                      << 0.5 << 4.5 << 3.5;
      BM2.Row(6)                             << 9.5 << 7.5;
      Matrix RM1 = BM1, RM2 = BM2;
      Matrix X;
      GenericMatrix GRM1 = RM1, GBM1 = BM1, GRM2 = RM2, GBM2 = BM2;
      Matrix Z(6,0); Z = 5; Print(Z);
      GRM1 |= Z; GBM1 |= Z; GRM2 &= Z.t(); GBM2 &= Z.t();
      X = GRM1 - BM1; Print(X); X = GBM1 - BM1; Print(X);
      X = GRM2 - BM2; Print(X); X = GBM2 - BM2; Print(X);

      GRM1 = RM1; GBM1 = BM1; GRM2 = RM2; GBM2 = BM2;
      GRM1 *= GRM2; GBM1 *= GBM2;
      X = GRM1 - BM1 * BM2; Print(X);
      X = RM1 * RM2 - GBM1; Print(X);

      GRM1 = RM1; GBM1 = BM1; GRM2 = RM2; GBM2 = BM2;
      GRM1 *= GBM2; GBM1 *= GRM2;          // Bs and Rs swapped on LHS
      X = GRM1 - BM1 * BM2; Print(X);
      X = RM1 * RM2 - GBM1; Print(X);

      X = BM1.t(); BandMatrix BM1X = BM1.t();
      GRM1 = RM1; X -= GRM1.t(); Print(X); X = BM1X - BM1.t(); Print(X);

      // check that linear equation solver works with Identity Matrix
      IdentityMatrix IM(6); IM *= 2;
      GBM1 = BM1; GBM1 *= 4; GRM1 = RM1; GRM1 *= 4;
      DiagonalMatrix D = IM;
      LinearEquationSolver LES1(D);
      BandMatrix BX;
      BX = LES1.i() * GBM1; BX -= BM1 * 2; X = BX; Print(X);
      LinearEquationSolver LES2(IM);
      BX = LES2.i() * GBM1; BX -= BM1 * 2; X = BX; Print(X);
      BX = D.i() * GBM1; BX -= BM1 * 2; X = BX; Print(X);
      BX = IM.i() * GBM1; BX -= BM1 * 2; X = BX; Print(X);
      BX = IM.i(); BX *= GBM1; BX -= BM1 * 2; X = BX; Print(X);

      // try symmetric band matrices
      SymmetricBandMatrix SBM; SBM << SP(BM1, BM1.t());
      SBM << IM.i() * SBM;
      X = 2 * SBM - SP(RM1, RM1.t()); Print(X);

      // Do this again with more general D
      D << 2.5 << 7.5 << 2 << 5 << 4.5 << 7.5;
      BX = D.i() * BM1; X = BX - D.i() * RM1;
      Clean(X,0.00000001); Print(X);
      BX = D.i(); BX *= BM1; X = BX - D.i() * RM1;
      Clean(X,0.00000001); Print(X);
      SBM << SP(BM1, BM1.t());
      BX = D.i() * SBM; X = BX - D.i() * SP(RM1, RM1.t());
      Clean(X,0.00000001); Print(X);

      // test return
      BX = TestReturn(BM1); X = BX - BM1;
      if (BX.BandWidth() != BM1.BandWidth()) X = 5;
      Print(X);
   }

//   cout << "\nEnd of eighth test\n";
}
