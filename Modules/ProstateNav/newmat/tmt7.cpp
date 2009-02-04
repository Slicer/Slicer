
//#define WANT_STREAM

#include "include.h"

#include "newmat.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif

// C matrix mulitply - for testing RealStarStar

void c_matrix_multiply(int p, int q, int r,
   const Real** a, const Real** b, Real** c)
{
   for (int i = 0; i < p; ++i) for (int k = 0; k < r; ++k)
   {
      Real sum = 0.0;
      for (int j = 0; j < q; ++j) sum += a[i][j] * b[j][k];
      c[i][k] = sum;
   }
}
   


void trymat7()
{
//   cout << "\nSeventh test of Matrix package\n";
   Tracer et("Seventh test of Matrix package");
   Tracer::PrintTrace();

   int i,j;


   DiagonalMatrix D(6);
   UpperTriangularMatrix U(6);
   for (i=1;i<=6;i++) { for (j=i;j<=6;j++) U(i,j)=i*i*j-50; D(i,i)=i*i+i-10; }
   LowerTriangularMatrix L=(U*3.0).t();
   SymmetricMatrix S(6);
   for (i=1;i<=6;i++) for (j=i;j<=6;j++) S(i,j)=i*i+2.0+j;
   Matrix MD=D; Matrix ML=L; Matrix MU=U;
   Matrix MS=S;
   Matrix M(6,6);
   for (i=1;i<=6;i++) for (j=1;j<=6;j++) M(i,j)=i*j+i*i-10.0;  
   {
      Tracer et1("Stage 1");
      Print(Matrix((S-M)-(MS-M)));
      Print(Matrix((-M-S)+(MS+M)));
      Print(Matrix((U-M)-(MU-M)));
   }
   {
      Tracer et1("Stage 2");
      Print(Matrix((L-M)+(M-ML)));
      Print(Matrix((D-M)+(M-MD)));
      Print(Matrix((D-S)+(MS-MD)));
      Print(Matrix((D-L)+(ML-MD)));
   }

   { M=MU.t(); }
   LowerTriangularMatrix LY=D.i()*U.t();
   {
      Tracer et1("Stage 3");
      MS=D*LY-M; Clean(MS,0.00000001); Print(MS);
      L=U.t();
      LY=D.i()*L; MS=D*LY-M; Clean(MS,0.00000001); Print(MS);
   }
   {
      Tracer et1("Stage 4");
      UpperTriangularMatrix UT(11);
      int i, j;
      for (i=1;i<=11;i++) for (j=i;j<=11;j++) UT(i,j)=i*i+j*3;
      GenericMatrix GM; Matrix X;
      UpperBandMatrix UB(11,3); UB.Inject(UT); UT = UB;
      UpperBandMatrix UB2 = UB / 8;
      GM = UB2-UT/8; X = GM; Print(X);
      SymmetricBandMatrix SB(11,4); SB << (UB + UB.t());
      X = SB - UT - UT.t(); Print(X);
      BandMatrix B = UB + UB.t()*2;
      DiagonalMatrix D; D << B;
      X.ReSize(1,1); X(1,1) = Trace(B)-Sum(D); Print(X);
      X = SB + 5; Matrix X1=X; X = SP(UB,X); Matrix X2 =UB;
      X1 = (X1.AsDiagonal() * X2.AsDiagonal()).AsRow()-X.AsColumn().t();
      Print(X1);
      X1=SB.t(); X2 = B.t(); X = SB.i() * B - X1.i() * X2.t();
      Clean(X,0.00000001); Print(X);
      X = SB.i(); X = X * B - X1.i() * X2.t();
      Clean(X,0.00000001); Print(X);
      D = 1; X = SB.i() * SB - D; Clean(X,0.00000001); Print(X);
      ColumnVector CV(11);
      CV << 2 << 6 <<3 << 8 << -4 << 17.5 << 2 << 1 << -2 << 5 << 3.75;
      D << 2 << 6 <<3 << 8 << -4 << 17.5 << 2 << 1 << -2 << 5 << 3.75;
      X = CV.AsDiagonal(); X = X-D; Print(X);
      SymmetricBandMatrix SB1(11,7); SB1 = 5; 
      SymmetricBandMatrix SB2 = SB1 + D;
      X.ReSize(11,11); X=0;
      for (i=1;i<=11;i++) for (j=1;j<=11;j++)
      {
         if (abs(i-j)<=7) X(i,j)=5;
         if (i==j) X(i,j)+=CV(i);
      }
      SymmetricMatrix SM; SM.ReSize(11);
      SM=SB; SB = SB+SB2; X1 = SM+X-SB; Print(X1);
      SB2=0; X2=SB2; X1=SB; Print(X2);
      for (i=1;i<=11;i++) SB2.Column(i)<<SB.Column(i);
      X1=X1-SB2; Print(X1);
      X = SB; SB2.ReSize(11,4); SB2 = SB*5; SB2 = SB + SB2;
      X1 = X*6 - SB2; Print(X1);
      X1 = SP(SB,SB2/3); X1=X1-SP(X,X*2); Print(X1);
      X1 = SP(SB2/6,X*2); X1=X1-SP(X*2,X); Print(X1);
   }

   {
      // test the simple integer array class
      Tracer et("Stage 5");
      ColumnVector Test(10); Test = 0.0;
      int i;
      SimpleIntArray A(100);
      for (i = 0; i < 100; i++) A[i] = i*i+1;
      SimpleIntArray B(100), C(50), D;
      B = A; A.ReSize(50, true); C = A; A.ReSize(150, true); D = A;
      for (i = 0; i < 100; i++) if (B[i] != i*i+1) Test(1)=1;
      for (i = 0; i < 50; i++) if (C[i] != i*i+1) Test(2)=1;
      for (i = 0; i < 50; i++) if (D[i] != i*i+1) Test(3)=1;
      for (i = 50; i < 150; i++) if (D[i] != 0) Test(3)=1;
      A.resize(75); A = A.size();
      for (i = 0; i < 75; i++) if (A[i] != 75) Test(4)=1;
      A.resize(25); A = A.size();
      for (i = 0; i < 25; i++) if (A[i] != 25) Test(5)=1;
      A.ReSize(25); A = 23;
      for (i = 0; i < 25; i++) if (A[i] != 23) Test(6)=1;
      A.ReSize(0); A.ReSize(15); A = A.Size();
      for (i = 0; i < 15; i++) if (A[i] != 15) Test(7)=1;
      const SimpleIntArray E = B;
      for (i = 0; i < 100; i++) if (E[i] != i*i+1) Test(8)=1;
      SimpleIntArray F; F.resize_keep(5);
      for (i = 0; i < 5; i++) if (F[i] != 0) Test(9)=1;
      Print(Test);
   }
   
   {
      // testing RealStarStar
      Tracer et("Stage 6");
      MultWithCarry MWC;
      
      Matrix A(10, 12), B(12, 15), C(10, 15);
      FillWithValues(MWC, A); FillWithValues(MWC, B);
      ConstRealStarStar a(A);
      ConstRealStarStar b(B);
      RealStarStar c(C);
      c_matrix_multiply(10,12,15,a,b,c);
      Matrix X = C - A * B; Clean(X,0.00000001); Print(X);
      A.ReSize(11, 10); B.ReSize(10,8); C.ReSize(11,8);
      FillWithValues(MWC, A); FillWithValues(MWC, B);
      C = -1;
      c_matrix_multiply(11,10,8,
         ConstRealStarStar(A),ConstRealStarStar(B),RealStarStar(C));
      X = C - A * B; Clean(X,0.00000001); Print(X);
   }
   
   {
      // testing resize_keep
      Tracer et("Stage 7");
      Matrix X, Y;
      MultWithCarry MWC;
      
      X.resize(20,35); FillWithValues(MWC, X);
      Matrix M(20,35); M = X;
      X = M.submatrix(1,15,1,25);
      M.resize_keep(15,25); Y  = X - M; Print(Y);
      M.resize_keep(15,25); Y  = X - M; Print(Y);
      Y.resize(29,27); Y = 0; Y.submatrix(1,15,1,25) = X;
      M.resize_keep(29,27); Y -= M; Print(Y);
      M.resize_keep(0,5);  M.resize_keep(10,10); Print(M);
      M.resize_keep(15,0); M.resize_keep(10,10); Print(M);
      
      X.resize(20,35); FillWithValues(MWC, X);
      M = X;
      M.resize_keep(38,17);
      Y.resize(38,17); Y = 0;
      Y.submatrix(1,20,1,17) = X.submatrix(1,20,1,17);
      Y -= M; Print(Y);
            
      X.resize(40,12); FillWithValues(MWC, X);
      M = X;
      M.resize_keep(38,17);
      Y.resize(38,17); Y = 0;
      Y.submatrix(1,38,1,12) = X.submatrix(1,38,1,12);
      Y -= M; Print(Y);

#ifndef DONT_DO_NRIC

      X.resize(20,35); FillWithValues(MWC, X);
      nricMatrix nM(20,35); nM = X;
      X = nM.submatrix(1,15,1,25);
      nM.resize_keep(15,25); Y  = X - nM; Print(Y);
      nM.resize_keep(15,25); Y  = X - nM; Print(Y);
      Y.resize(29,27); Y = 0; Y.submatrix(1,15,1,25) = X;
      nM.resize_keep(29,27); Y -= nM; Print(Y);
      nM.resize_keep(0,5);  nM.resize_keep(10,10); Print(nM);
      nM.resize_keep(15,0); nM.resize_keep(10,10); Print(nM);
      
      X.resize(20,35); FillWithValues(MWC, X);
      nM = X;
      nM.resize_keep(38,17);
      Y.resize(38,17); Y = 0;
      Y.submatrix(1,20,1,17) = X.submatrix(1,20,1,17);
      Y -= nM; Print(Y);
            
      X.resize(40,12); FillWithValues(MWC, X);
      nM = X;
      nM.resize_keep(38,17);
      Y.resize(38,17); Y = 0;
      Y.submatrix(1,38,1,12) = X.submatrix(1,38,1,12);
      Y -= nM; Print(Y);      

#endif      
      
      X.resize(20,20); FillWithValues(MWC, X);
      SquareMatrix SQM(20); SQM << X;
      X = SQM.sym_submatrix(1,13);
      SQM.resize_keep(13); Y  = X - SQM; Print(Y);
      SQM.resize_keep(13); Y  = X - SQM; Print(Y);
      Y.resize(23,23); Y = 0; Y.sym_submatrix(1,13) = X;
      SQM.resize_keep(23,23); Y -= SQM; Print(Y);
      SQM.resize_keep(0); SQM.resize_keep(50); Print(SQM);
      
      X.resize(20,20); FillWithValues(MWC, X);
      SymmetricMatrix SM(20); SM << X;
      X = SM.sym_submatrix(1,13);
      SM.resize_keep(13); Y  = X - SM; Print(Y);
      SM.resize_keep(13); Y  = X - SM; Print(Y);
      Y.resize(23,23); Y = 0; Y.sym_submatrix(1,13) = X;
      SM.resize_keep(23); Y -= SM; Print(Y);
      SM.resize_keep(0); SM.resize_keep(50); Print(SM);
      
      X.resize(20,20); FillWithValues(MWC, X);
      LowerTriangularMatrix LT(20); LT << X;
      X = LT.sym_submatrix(1,13);
      LT.resize_keep(13); Y  = X - LT; Print(Y);
      LT.resize_keep(13); Y  = X - LT; Print(Y);
      Y.resize(23,23); Y = 0; Y.sym_submatrix(1,13) = X;
      LT.resize_keep(23); Y -= LT; Print(Y);
      LT.resize_keep(0); LT.resize_keep(50); Print(LT);
      
      X.resize(20,20); FillWithValues(MWC, X);
      UpperTriangularMatrix UT(20); UT << X;
      X = UT.sym_submatrix(1,13);
      UT.resize_keep(13); Y  = X - UT; Print(Y);
      UT.resize_keep(13); Y  = X - UT; Print(Y);
      Y.resize(23,23); Y = 0; Y.sym_submatrix(1,13) = X;
      UT.resize_keep(23); Y -= UT; Print(Y);
      UT.resize_keep(0); UT.resize_keep(50); Print(UT);
      
      X.resize(20,20); FillWithValues(MWC, X);
      DiagonalMatrix DM(20); DM << X;
      X = DM.sym_submatrix(1,13);
      DM.resize_keep(13); Y  = X - DM; Print(Y);
      DM.resize_keep(13); Y  = X - DM; Print(Y);
      Y.resize(23,23); Y = 0; Y.sym_submatrix(1,13) = X;
      DM.resize_keep(23); Y -= DM; Print(Y);
      DM.resize_keep(0); DM.resize_keep(50); Print(DM);
      
      X.resize(1,20); FillWithValues(MWC, X);
      RowVector RV(20); RV << X;
      X = RV.columns(1,13);
      RV.resize_keep(13); Y  = X - RV; Print(Y);
      RV.resize_keep(13); Y  = X - RV; Print(Y);
      Y.resize(1,23); Y = 0; Y.columns(1,13) = X;
      RV.resize_keep(1,23); Y -= RV; Print(Y);
      RV.resize_keep(0); RV.resize_keep(50); Print(RV);
      
      X.resize(20,1); FillWithValues(MWC, X);
      ColumnVector CV(20); CV << X;
      X = CV.rows(1,13);
      CV.resize_keep(13); Y  = X - CV; Print(Y);
      CV.resize_keep(13); Y  = X - CV; Print(Y);
      Y.resize(23,1); Y = 0; Y.rows(1,13) = X;
      CV.resize_keep(23,1); Y -= CV; Print(Y);
      CV.resize_keep(0); CV.resize_keep(50); Print(CV);
      
      
   }    


//   cout << "\nEnd of seventh test\n";
}
