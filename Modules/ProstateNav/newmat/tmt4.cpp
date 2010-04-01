
//#define WANT_STREAM


#include "include.h"

#include "newmat.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif


/**************************** test program ******************************/


void trymat4()
{
//   cout << "\nFourth test of Matrix package\n";
   Tracer et("Fourth test of Matrix package");
   Tracer::PrintTrace();


   {
      Tracer et1("Stage 1");
      int i, j;
      Matrix M(10,10);
      UpperTriangularMatrix U(10);
      for (i=1;i<=10;i++) for (j=1;j<=10;j++) M(i,j) = 100*i+j;
      U << -M;
      Matrix X1 = M.Rows(2,4);
      Matrix Y1 = U.t().Rows(2,4);
      Matrix X = U; { Print(Matrix(X.Columns(2,4).t()-Y1)); }
      RowVector RV = M.Row(5);
      {
         X.ReSize(3,10);
         X.Row(1) << M.Row(2); X.Row(2) << M.Row(3); X.Row(3) << M.Row(4);
         Print(Matrix(X-X1));
      }
      {
         UpperTriangularMatrix V = U.SymSubMatrix(3,5);
         Matrix MV = U.SubMatrix(3,5,3,5); { Print(Matrix(MV-V)); }
         Matrix X2 = M.t().Columns(2,4); { Print(Matrix(X2-X1.t())); }
         Matrix Y2 = U.Columns(2,4); { Print(Matrix(Y2-Y1.t())); }
         ColumnVector CV = M.t().Column(5); { Print(ColumnVector(CV-RV.t())); }
         X.ReSize(10,3); M = M.t();
         X.Column(1) << M.Column(2); X.Column(2) << M.Column(3);
         X.Column(3) << M.Column(4);
         Print(Matrix(X-X2));
      }
   }

   {
      Tracer et1("Stage 2");
      int i, j;
      Matrix M; Matrix X; M.ReSize(5,8);
      for (i=1;i<=5;i++) for (j=1;j<=8;j++) M(i,j) = 100*i+j;
      {
         X = M.Columns(5,8); M.Columns(5,8) << M.Columns(1,4);
             M.Columns(1,4) << X;
         X = M.Columns(3,4); M.Columns(3,4) << M.Columns(1,2);
             M.Columns(1,2) << X;
         X = M.Columns(7,8); M.Columns(7,8) << M.Columns(5,6);
             M.Columns(5,6) << X;
      }
      {
         X = M.Column(2); M.Column(2) = M.Column(1); M.Column(1) = X;
         X = M.Column(4); M.Column(4) = M.Column(3); M.Column(3) = X;
         X = M.Column(6); M.Column(6) = M.Column(5); M.Column(5) = X;
         X = M.Column(8); M.Column(8) = M.Column(7); M.Column(7) = X;
         X.ReSize(5,8);
      }
      for (i=1;i<=5;i++) for (j=1;j<=8;j++) X(i,9-j) = 100*i+j;
      Print(Matrix(X-M));
   }
   {
      Tracer et1("Stage 3");
      // try submatrices of zero dimension
      int i, j;
      Matrix A(4,5); Matrix B, C;
      for (i=1; i<=4; i++) for (j=1; j<=5; j++)
         A(i,j) = 100+i*10+j;
      B = A + 100;
      C = A | B.Columns(4,3); Print(Matrix(A - C));
      C = A | B.Columns(1,0); Print(Matrix(A - C));
      C = A | B.Columns(6,5); Print(Matrix(A - C));
      C = A & B.Rows(2,1); Print(Matrix(A - C));
   }
   {
      Tracer et1("Stage 4");
      BandMatrix BM(5,3,2);
      BM(1,1) = 1; BM(1,2) = 2; BM(1,3) = 3;
      BM(2,1) = 4; BM(2,2) = 5; BM(2,3) = 6; BM(2,4) = 7;
      BM(3,1) = 8; BM(3,2) = 9; BM(3,3) =10; BM(3,4) =11; BM(3,5) =12;
      BM(4,1) =13; BM(4,2) =14; BM(4,3) =15; BM(4,4) =16; BM(4,5) =17;
                   BM(5,2) =18; BM(5,3) =19; BM(5,4) =20; BM(5,5) =21;
      SymmetricBandMatrix SM(5,3);
      SM.Inject(BandMatrix(BM + BM.t()));
      Matrix A = BM + 1;
      Matrix M = A + A.t() - 2;
      Matrix C = A.i() * BM;
      C = A * C - BM; Clean(C, 0.000000001); Print(C);
      C = A.i() * SM;
      C = A * C - M; Clean(C, 0.000000001); Print(C);

      // check row-wise load
      BandMatrix BM1(5,3,2);
      BM1.Row(1) <<  1 <<  2 <<  3;
      BM1.Row(2) <<  4 <<  5 <<  6 <<  7;
      BM1.Row(3) <<  8 <<  9 << 10 << 11 << 12;
      BM1.Row(4) << 13 << 14 << 15 << 16 << 17;
      BM1.Row(5)       << 18 << 19 << 20 << 21;
      Matrix M1 = BM1 - BM; Print(M1);
   }
   {
      Tracer et1("Stage 5");
      Matrix X(4,4);
      X << 1 << 2 << 3 << 4
        << 5 << 6 << 7 << 8
        << 9 <<10 <<11 <<12
        <<13 <<14 <<15 <<16;
      Matrix Y(4,0);
      Y = X | Y;
      X -= Y; Print(X);

      DiagonalMatrix D(1);
      D << 23;                       // matrix input with just one value
      D(1) -= 23; Print(D);

   }
   {
      Tracer et1("Stage 6");
      Matrix h (2,2);
      h << 1.0 << 2.0 << 0.0 << 1.0 ;
      RowVector c(2);
      c << 0.0 << 1.0;
      h -= c & c;
      h -= c.t().Reverse() | c.Reverse().t();
      Print(h);
   }
   {
      Tracer et1("Stage 7");
      // Check row-wise input for diagonal matrix
      DiagonalMatrix D(4);
      D << 18 << 23 << 31 << 17;
      DiagonalMatrix D1(4);
      D1.Row(1) << 18; D1.Row(2) << 23; D1.Row(3) << 31; D1.Row(4) << 17;
      D1 -= D; Print(D1);
      D1(1) = 18; D1(2) = 23; D1(3) = 31; D1(4) = 17;
      D1 -= D; Print(D1);
   }
   
   {
      Tracer et1("Stage 8");
      // test swap functions
      MultWithCarry MWC;
      Matrix A(3,4); Matrix B(5,6);
      FillWithValues(MWC, A); FillWithValues(MWC, B);
      Matrix A1 = A; Matrix B1 = B; A.Release(); B.Release(2);
      swap(A, B);
      int a = A.size() - B1.size(), b = B.size() - A1.size();
      Matrix D = A - B1; Print(D);
      D = B - A1; Print(D);
      Print(B);   // should be zero because of release
      D = A - B1; Print(D);
      Print(A);   // now should be zero
      D.ReSize(1,2); D(1,1) = a; D(1,2) = b; Print(D);
      
      A.ReSize(20,20); FillWithValues(MWC, A);
     
      UpperTriangularMatrix UA; UA << A; UpperTriangularMatrix UA1 = UA;
      UpperTriangularMatrix UB;
      swap(UA, UB); Print(UA); UB -= UA1; Print(UB);
      
      LowerTriangularMatrix LA; LA << A; LowerTriangularMatrix LA1 = LA;
      LowerTriangularMatrix LB;
      swap(LB, LA); Print(LA); LB -= LA1; Print(LB);

      SymmetricMatrix SA; SA << A; SymmetricMatrix SA1 = SA;
      SymmetricMatrix SB;
      swap(SA, SB); Print(SA); SB -= SA1; Print(SB);
      
      DiagonalMatrix DA; DA << A; DiagonalMatrix DA1 = DA;
      DiagonalMatrix DB;
      swap(DB, DA); Print(DA); DB -= DA1; Print(DB);
      
      RowVector RVA = A.Row(1); RowVector RVA1 = RVA;
      RowVector RVB;
      swap(RVB, RVA); Print(RVA); RVB -= RVA1; Print(RVB);
      
      ColumnVector CVA = A.Column(1); ColumnVector CVA1 = CVA;
      ColumnVector CVB;
      swap(CVA, CVB); Print(CVA); CVB -= CVA1; Print(CVB);
      
      BandMatrix BA(20, 7, 4); BA.Inject(A); BandMatrix BA1 = BA;
      BandMatrix BB;
      swap(BA, BB); D = BA; Print(D); BB -= BA1; D = BB; Print(D);
      
      LowerBandMatrix LBA(20, 6); LBA.Inject(A); LowerBandMatrix LBA1 = LBA;
      LowerBandMatrix LBB;
      swap(LBB, LBA); D = LBA; Print(D); LBB -= LBA1; D = LBB; Print(D);
      
      UpperBandMatrix UBA(20, 9); UBA.Inject(A); UpperBandMatrix UBA1 = UBA;
      UpperBandMatrix UBB;
      swap(UBA, UBB); D = UBA; Print(D); UBB -= UBA1; D = UBB; Print(D);
      
      SymmetricBandMatrix SBA(20, 4); SBA.Inject(A);
      SymmetricBandMatrix SBA1 = SBA;
      SymmetricBandMatrix SBB;
      
      swap(SBB, SBA); D = SBA; Print(D);
      SBB -= SBA1; D = SBB; Print(D);
      
      B.ReSize(10,10); FillWithValues(MWC, B);
      
      CroutMatrix CA = A; IdentityMatrix IA(20);
      CroutMatrix CB = B; IdentityMatrix IB(10);
      swap(CA, CB); swap(IA, IB);
      D = CA.i() * B - IA; Clean(D,0.00000001); Print(D);
      D = CB.i() * A - IB; Clean(D,0.00000001); Print(D);
      
      BA.ReSize(20, 5, 7); BA.Inject(A); BandLUMatrix BLUA = BA;
      BB.ReSize(10, 3, 4); BB.Inject(B); BandLUMatrix BLUB = BB;
      swap(BLUA, BLUB);
      D = BLUA.i() * BB - IA; Clean(D,0.00000001); Print(D);
      D = BLUB.i() * BA - IB; Clean(D,0.00000001); Print(D);

      
      SBA.ReSize(20, 5); SBA.Inject(A); BandLUMatrix SBLUA = SBA;
      SBB.ReSize(10, 3); SBB.Inject(B); BandLUMatrix SBLUB = SBB;
      swap(SBLUA, SBLUB);
      D = SBLUA.i() * SBB - IA; Clean(D,0.00000001); Print(D);
      D = SBLUB.i() * SBA - IB; Clean(D,0.00000001); Print(D);
      
      UA << A;
      GenericMatrix GUA = UA; GenericMatrix GB = B; swap(GUA, GB);
      D = GB - UA; Print(D); D = B - GUA; Print(D);
      
   }

//   cout << "\nEnd of fourth test\n";
}

