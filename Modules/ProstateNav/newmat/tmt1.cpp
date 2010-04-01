
#define WANT_STREAM



#include "include.h"

#include "newmat.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif


/**************************** test program ******************************/


void trymat1()
{
//   cout << "\nFirst test of Matrix package\n\n";
   Tracer et("First test of Matrix package");
   Tracer::PrintTrace();
   {
      Tracer et1("Stage 1");
      int i,j;

      LowerTriangularMatrix L(10);
      for (i=1;i<=10;i++) for (j=1;j<=i;j++) L(i,j)=2.0+i*i+j;
      SymmetricMatrix S(10);
      for (i=1;i<=10;i++) for (j=1;j<=i;j++) S(i,j)=i*j+1.0;
      SymmetricMatrix S1 = S / 2.0;
      S = S1 * 2.0;
      UpperTriangularMatrix U=L.t()*2.0;
      Print(LowerTriangularMatrix(L-U.t()*0.5));
      DiagonalMatrix D(10);
      for (i=1;i<=10;i++) D(i,i)=(i-4)*(i-5)*(i-6);
      Matrix M=(S+U-D+L)*(L+U-D+S);
      DiagonalMatrix DD=D*D;
      LowerTriangularMatrix LD=L*D;
      // expressions split for Turbo C
      Matrix M1 = S*L + U*L - D*L + L*L + 10.0;
      { M1 = M1 + S*U + U*U - D*U + L*U - S*D; }
      { M1 = M1 - U*D + DD - LD + S*S; }
      { M1 = M1 + U*S - D*S + L*S - 10.0; }
      M=M1-M;
      Print(M);
   }
   {
      Tracer et1("Stage 2");
      int i,j;

      LowerTriangularMatrix L(9);
      for (i=1;i<=9;i++) for (j=1;j<=i;j++) L(i,j)=1.0+j;
      UpperTriangularMatrix U1(9);
      for (j=1;j<=9;j++) for (i=1;i<=j;i++) U1(i,j)=1.0+i;
      LowerTriangularMatrix LX(9);
      for (i=1;i<=9;i++) for (j=1;j<=i;j++) LX(i,j)=1.0+i*i;
      UpperTriangularMatrix UX(9);
      for (j=1;j<=9;j++) for (i=1;i<=j;i++) UX(i,j)=1.0+j*j;
      {
         L=L+LX/0.5;   L=L-LX*3.0;   L=LX*2.0+L;
         U1=U1+UX*2.0; U1=U1-UX*3.0; U1=UX*2.0+U1;
      }


      SymmetricMatrix S(9);
      for (i=1;i<=9;i++) for (j=1;j<=i;j++) S(i,j)=i*i+j;
      {
         SymmetricMatrix S1 = S;
         S=S1+5.0;
         S=S-3.0;
      }

      DiagonalMatrix D(9);
      for (i=1;i<=9;i++) D(i,i)=S(i,i);
      UpperTriangularMatrix U=L.t()*2.0;
      {
         U1=U1*2.0 - U;  Print(U1);
         L=L*2.0-D; U=U-D;
      }
      Matrix M=U+L; S=S*2.0; M=S-M; Print(M);
   }
   {
      Tracer et1("Stage 3");
      int i,j;
      Matrix M(10,3), N(10,3);
      for (i = 1; i<=10; i++) for (j = 1; j<=3; j++)
         {  M(i,j) = 2*i-j; N(i,j) = i*j + 20; }
      Matrix MN = M + N, M1;

      M1 = M; M1 += N; M1 -= MN; Print(M1);
      M1 = M; M1 += M1; M1 = M1 - M * 2; Print(M1);
      M1 = M; M1 += N * 2; M1 -= (MN + N); Print(M1);
      M1 = M; M1 -= M1; Print(M1);
      M1 = M; M1 -= MN + M1; M1 += N + M; Print(M1);
      M1 = M; M1 -= 5; M1 -= M; M1 *= 0.2; M1 = M1 + 1; Print(M1);
      Matrix NT = N.t();
      M1 = M; M1 *= NT; M1 -= M * N.t(); Print(M1);
      M = M * M.t();
      DiagonalMatrix D(10); D = 2;
      M1 = M; M1 += D; M1 -= M; M1 = M1 - D; Print(M1);
      M1 = M; M1 -= D; M1 -= M; M1 = M1 + D; Print(M1);
      M1 = M; M1 *= D; M1 /= 2; M1 -= M; Print(M1);
      SymmetricMatrix SM; SM << M;
      // UpperTriangularMatrix SM; SM << M;
      SM += 10; M1 = SM - M; M1 /=10; M1 = M1 - 1; Print(M1);
   }
   {
      Tracer et1("Stage 4");
      int i,j;
      Matrix M(10,3), N(10,5);
      for (i = 1; i<=10; i++) for (j = 1; j<=3; j++) M(i,j) = 2*i-j;
      for (i = 1; i<=10; i++) for (j = 1; j<=5; j++) N(i,j) = i*j + 20;
      Matrix M1;
      M1 = M; M1 |= N; M1 &= N | M;
      M1 -= (M | N) & (N | M); Print(M1);
      M1 = M; M1 |= M1; M1 &= M1;
      M1 -= (M | M) & (M | M); Print(M1);

   }
   {
      Tracer et1("Stage 5");
      int i,j;
      BandMatrix BM1(10,2,3), BM2(10,4,1); Matrix M1(10,10), M2(10,10);
      for (i=1;i<=10;i++) for (j=1;j<=10;j++)
        { M1(i,j) = 0.5*i+j*j-50; M2(i,j) = (i*101 + j*103) % 13; }
      BM1.Inject(M1); BM2.Inject(M2);
      BandMatrix BM = BM1; BM += BM2;
      Matrix M1X = BM1; Matrix M2X = BM2; Matrix MX = BM;
      MX -= M1X + M2X; Print(MX);
      MX = BM1; MX += BM2; MX -= M1X; MX -= M2X; Print(MX);
      SymmetricBandMatrix SM1; SM1 << BM1 * BM1.t(); 
      SymmetricBandMatrix SM2; SM2 << BM2 * BM2.t();
      SM1 *= 5.5;
      M1X *= M1X.t(); M1X *= 5.5; M2X *= M2X.t();
      SM1 -= SM2; M1 = SM1 - M1X + M2X; Print(M1);
      M1 = BM1; BM1 *= SM1; M1 = M1 * SM1 - BM1; Print(M1); 
      M1 = BM1; BM1 -= SM1; M1 = M1 - SM1 - BM1; Print(M1); 
      M1 = BM1; BM1 += SM1; M1 = M1 + SM1 - BM1; Print(M1); 
      
   }
   {
      Tracer et1("Stage 6");
      int i,j;
      Matrix M(10,10), N(10,10);
      for (i = 1; i<=10; i++) for (j = 1; j<=10; j++)
         {  M(i,j) = 2*i-j; N(i,j) = i*j + 20; }
      GenericMatrix GM = M;
      GM += N; Matrix M1 = GM - N - M; Print(M1);
      DiagonalMatrix D(10); D = 3;
      GM = D; GM += N; GM += M; GM += D;
      M1 = D*2 - GM + M + N; Print(M1);
      GM = D; GM *= 4; GM += 16; GM /= 8; GM -= 2;
      GM -= D / 2; M1 = GM; Print(M1);
      GM = D; GM *= M; GM *= N; GM /= 3; M1 = M*N - GM; Print(M1);
      GM = D; GM |= M; GM &= N | D; M1 = GM - ((D | M) & (N | D));
      Print(M1);
      GM = M; M1 = M; GM += 5; GM *= 3; M *= 3; M += 15; M1 = GM - M;
      Print(M1);
      D.ReSize(10); for (i = 1; i<=10; i++) D(i) = i;
      M1 = D + 10; GM = D; GM += 10; M1 -= GM; Print(M1);
      GM = M; GM -= D; M1 = GM; GM = D; GM -= M; M1 += GM; Print(M1);
      GM = M; GM *= D; M1 = GM; GM = D; GM *= M.t();
      M1 -= GM.t(); Print(M1);
      GM = M; GM += 2 * GM; GM -= 3 * M; M1 = GM; Print(M1);
      GM = M; GM |= GM; GM -= (M | M); M1 = GM; Print(M1);
      GM = M; GM &= GM; GM -= (M & M); M1 = GM; Print(M1);
      M1 = M; M1 = (M1.t() & M.t()) - (M | M).t(); Print(M1);
      M1 = M; M1 = (M1.t() | M.t()) - (M & M).t(); Print(M1);

   }

   {
      Tracer et1("Stage 7");
      // test for bug in MS VC5
      int n = 3;
      int k; int j;
      Matrix A(n,n), B(n,n);

      //first version - MS VC++ 5 mis-compiles if optimisation is on
      for (k=1; k<=n; k++)
      {
         for (j = 1; j <= n; j++) A(k,j) = ((k-1) * (2*j-1));
      }

      //second version
      for (k=1; k<=n; k++)
      {
         const int k1 = k-1;          // otherwise Visual C++ 5 fails
         for (j = 1; j <= n; j++) B(k,j) = (k1 * (2*j-1));
      }

      if (A != B)
      {
         cout << "\nVisual C++ version 5 compiler error?";
         cout << "\nTurn off optimisation";
      }

      A -= B; Print(A);

   }

   {
      Tracer et1("Stage 8");
      // Cross product
      ColumnVector i(3); i << 1 << 0 << 0;
      ColumnVector j(3); j << 0 << 1 << 0;
      ColumnVector k(3); k << 0 << 0 << 1;
      ColumnVector X;
      X = CrossProduct(i,j) - k; Print(X);
      X = CrossProduct(j,k) - i; Print(X);
      X = CrossProduct(k,i) - j; Print(X);
      X = CrossProduct(j,i) + k; Print(X);
      X = CrossProduct(k,j) + i; Print(X);
      X = CrossProduct(i,k) + j; Print(X);
      X = CrossProduct(i,i); Print(X);
      X = CrossProduct(j,j); Print(X);
      X = CrossProduct(k,k); Print(X);

      ColumnVector A(3); A << 2.25 << 1.75 << -7.5;
      ColumnVector B(3); B << -0.5 << 4.75 << 3.25;
      ColumnVector C(3); C << 9.25 << 3.5  << 1.25;

      Real d0 = (A | B | C).Determinant();    // Vector triple product
      Real d1 = DotProduct(CrossProduct(A, B), C);
      Real d2 = DotProduct(CrossProduct(B, C), A);
      Real d3 = DotProduct(CrossProduct(C, A), B);
      X << (d1 - d0) << (d2 - d0) << (d3 - d0);
      Clean(X, 0.000000001); Print(X);

      X = CrossProduct(A, CrossProduct(B, C))
        + CrossProduct(B, CrossProduct(C, A))
        + CrossProduct(C, CrossProduct(A, B));
      Print(X);

      RowVector XT = CrossProduct(A.AsRow(), B.AsRow());
      XT -= CrossProduct(A, B).AsRow();
      Print(XT);
   }

   {
      Tracer et1("Stage 9");
      // More cross product
      int i, j;
      Matrix M(10,3), N(10,3);
      for (i = 1; i<=10; i++) for (j = 1; j<=3; j++)
         {  M(i,j) = 2*i-j; N(i,j) = i*j + 20; }

      Matrix CP1 = CrossProductRows(M, N);
      Matrix CP2(10,3);
      for (i = 1; i<=10; i++)
         CP2.Row(i) = CrossProduct(M.Row(i), N.Row(i));
      CP2 -= CP1; Print(CP2);

      CP2 = CrossProductColumns(M.t(), N.t());
      CP2 -= CP1.t(); Print(CP2);
   }

   {
      Tracer et1("Stage 10");
      // Make sure RNG works
      MultWithCarry mwc;
      ColumnVector cv(10);
      for (int i = 1; i <= 10; ++i) cv(i) = mwc.Next();
      cv *= 100.0;
      cv(1) -= 6.27874; 
      cv(2) -= 42.1718; 
      cv(3) -= 80.2854; 
      cv(4) -= 12.961;  
      cv(5) -= 17.7499; 
      cv(6) -= 13.2657; 
      cv(7) -= 50.4923; 
      cv(8) -= 26.095;  
      cv(9) -= 57.9147; 
      cv(10) -= 30.1778;        
      Clean(cv, 0.0001); Print(cv);
   }


//   cout << "\nEnd of first test\n";
}

