
//#define WANT_STREAM


#include "include.h"

#include "newmat.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif


/**************************** test program ******************************/


void trymat2()
{
//   cout << "\nSecond test of Matrix package\n\n";
   Tracer et("Second test of Matrix package");
   Tracer::PrintTrace();

   int i,j;

   Matrix M(3,5);
   for (i=1; i<=3; i++) for (j=1; j<=5; j++) M(i,j) = 100*i + j;
   Matrix X(8,10);
   for (i=1; i<=8; i++) for (j=1; j<=10; j++) X(i,j) = 1000*i + 10*j;
   Matrix Y = X; Matrix Z = X;
   { X.SubMatrix(2,4,3,7) << M; }
   for (i=1; i<=3; i++) for (j=1; j<=5; j++) Y(i+1,j+2) = 100*i + j;
   Print(Matrix(X-Y));


   Real a[15]; Real* r = a;
   for (i=1; i<=3; i++) for (j=1; j<=5; j++) *r++ = 100*i + j;
   { Z.SubMatrix(2,4,3,7) << a; }
   Print(Matrix(Z-Y));

   { M=33; X.SubMatrix(2,4,3,7) << M; }
   { Z.SubMatrix(2,4,3,7) = 33; }
   Print(Matrix(Z-X));

   for (i=1; i<=8; i++) for (j=1; j<=10; j++) X(i,j) = 1000*i + 10*j;
   Y = X;
   UpperTriangularMatrix U(5);
   for (i=1; i<=5; i++) for (j=i; j<=5; j++) U(i,j) = 100*i + j;
   { X.SubMatrix(3,7,5,9) << U; }
   for (i=1; i<=5; i++) for (j=i; j<=5; j++) Y(i+2,j+4) = 100*i + j;
   for (i=1; i<=5; i++) for (j=1; j<i; j++) Y(i+2,j+4) = 0.0;
   Print(Matrix(X-Y));
   for (i=1; i<=8; i++) for (j=1; j<=10; j++) X(i,j) = 1000*i + 10*j;
   Y = X;
   for (i=1; i<=5; i++) for (j=i; j<=5; j++) U(i,j) = 100*i + j;
   { X.SubMatrix(3,7,5,9).Inject(U); }
   for (i=1; i<=5; i++) for (j=i; j<=5; j++) Y(i+2,j+4) = 100*i + j;
   Print(Matrix(X-Y));


   // test growing and shrinking a vector
   {
      ColumnVector V(100);
      for (i=1;i<=100;i++) V(i) = i*i+i;
      V = V.Rows(1,50);               // to get first 50 vlaues.

      {
         V.Release(); ColumnVector VX=V;
         V.ReSize(100); V = 0.0; V.Rows(1,50)=VX;
      }                               // V now length 100

      M=V; M=100;                     // to make sure V will hold its values
      for (i=1;i<=50;i++) V(i) -= i*i+i;
      Print(V);


           // test redimensioning vectors with two dimensions given
      ColumnVector CV1(10); CV1 = 10;
      ColumnVector CV2(5); CV2.ReSize(10,1); CV2 = 10;
      V = CV1-CV2; Print(V);

      RowVector RV1(20); RV1 = 100;
      RowVector RV2; RV2.ReSize(1,20); RV2 = 100;
      V = (RV1-RV2).t(); Print(V);

      X.ReSize(4,7);
      for (i=1; i<=4; i++) for (j=1; j<=7; j++) X(i,j) = 1000*i + 10*j;
      Real Ty = 10.5;
      Y = Ty * X;
      Z = 7.25 - Y;
      M = Z + X * 10.5 - 7.25;
      Print(M);
      Y = 2.5 * X;
      Z = 9.25 + Y;
      M = Z - X * 2.5 - 9.25;
      Print(M);
      U.ReSize(8);
      for (i=1; i<=8; i++) for (j=i; j<=8; j++) U(i,j) = 100*i + j;
      Y = 100 - U;
      M = Y + U - 100;
      Print(M);
   }

   {
      SymmetricMatrix S,T;

      S << (U + U.t());
      T = 100 - S; M = T + S - 100; Print(M);
      T = 100 - 2 * S; M = T + S * 2 - 100; Print(M);
      X = 100 - 2 * S; M = X + S * 2 - 100; Print(M);
      T = S; T = 100 - T; M = T + S - 100; Print(M);
   }

   // test new
   {
      ColumnVector CV1; RowVector RV1;
      Matrix* MX; MX = new Matrix; if (!MX) Throw(Bad_alloc("New fails "));
      MX->ReSize(10,20);
      for (i = 1; i <= 10; i++) for (j = 1; j <= 20; j++)
         (*MX)(i,j) = 100 * i + j;
      ColumnVector* CV = new ColumnVector(10);
      if (!CV) Throw(Bad_alloc("New fails "));
      *CV << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10;
      RowVector* RV =  new RowVector(CV->t() | (*CV + 10).t());
      if (!RV) Throw(Bad_alloc("New fails "));
      CV1 = ColumnVector(10); CV1 = 1; RV1 = RowVector(20); RV1 = 1;
      *MX -= 100 * *CV * RV1 + CV1 * *RV;
      Print(*MX);
      delete MX; delete CV; delete RV;
   }


   // test copying of vectors and matrices with no elements
   {
      ColumnVector dims(16);
      Matrix M1; Matrix M2 = M1; Print(M2);
      dims(1) = M2.Nrows(); dims(2) = M2.Ncols();
      dims(3) = (Real)(unsigned long)M2.Store(); dims(4) = M2.Storage();
      M2 = M1;
      dims(5) = M2.Nrows(); dims(6) = M2.Ncols();
      dims(7) = (Real)(unsigned long)M2.Store(); dims(8) = M2.Storage();
      M2.ReSize(10,20); M2.CleanUp();
      dims(9) = M2.Nrows(); dims(10) = M2.Ncols();
      dims(11) = (Real)(unsigned long)M2.Store(); dims(12) = M2.Storage();
      M2.ReSize(20,10); M2.ReSize(0,0);
      dims(13) = M2.Nrows(); dims(14) = M2.Ncols();
      dims(15) = (Real)(unsigned long)M2.Store(); dims(16) = M2.Storage();
      Print(dims);
   }

   {
      ColumnVector dims(16);
      ColumnVector M1; ColumnVector M2 = M1; Print(M2);
      dims(1) = M2.Nrows(); dims(2) = M2.Ncols()-1;
      dims(3) = (Real)(unsigned long)M2.Store(); dims(4) = M2.Storage();
      M2 = M1;
      dims(5) = M2.Nrows(); dims(6) = M2.Ncols()-1;
      dims(7) = (Real)(unsigned long)M2.Store(); dims(8) = M2.Storage();
      M2.ReSize(10); M2.CleanUp();
      dims(9) = M2.Nrows(); dims(10) = M2.Ncols()-1;
      dims(11) = (Real)(unsigned long)M2.Store(); dims(12) = M2.Storage();
      M2.ReSize(10); M2.ReSize(0);
      dims(13) = M2.Nrows(); dims(14) = M2.Ncols()-1;
      dims(15) = (Real)(unsigned long)M2.Store(); dims(16) = M2.Storage();
      Print(dims);
   }

   {
      ColumnVector dims(16);
      RowVector M1; RowVector M2 = M1; Print(M2);
      dims(1) = M2.Nrows()-1; dims(2) = M2.Ncols();
      dims(3) = (Real)(unsigned long)M2.Store(); dims(4) = M2.Storage();
      M2 = M1;
      dims(5) = M2.Nrows()-1; dims(6) = M2.Ncols();
      dims(7) = (Real)(unsigned long)M2.Store(); dims(8) = M2.Storage();
      M2.ReSize(10); M2.CleanUp();
      dims(9) = M2.Nrows()-1; dims(10) = M2.Ncols();
      dims(11) = (Real)(unsigned long)M2.Store(); dims(12) = M2.Storage();
      M2.ReSize(10); M2.ReSize(0);
      dims(13) = M2.Nrows()-1; dims(14) = M2.Ncols();
      dims(15) = (Real)(unsigned long)M2.Store(); dims(16) = M2.Storage();
      Print(dims);
   }

   // test identity matrix
   {
      Matrix M;
      IdentityMatrix I(10); DiagonalMatrix D(10); D = 1;
      M = I; M -= D; Print(M);
      D -= I; Print(D);
      ColumnVector X(8);
      D = 1;
      X(1) = Sum(D) - Sum(I);
      X(2) = SumAbsoluteValue(D) - SumAbsoluteValue(I);
      X(3) = SumSquare(D) - SumSquare(I);
      X(4) = Trace(D) - Trace(I);
      X(5) = Maximum(D) - Maximum(I);
      X(6) = Minimum(D) - Minimum(I);
      X(7) = LogDeterminant(D).LogValue() - LogDeterminant(I).LogValue();
      X(8) = LogDeterminant(D).Sign() - LogDeterminant(I).Sign();
      Clean(X,0.00000001); Print(X);

      for (i = 1; i <= 10; i++) for (j = 1; j <= 10; j++)
         M(i,j) = 100 * i + j;
      Matrix N;
      N = M * I - M; Print(N);
      N = I * M - M; Print(N);
      N = M * I.i() - M; Print(N);
      N = I.i() * M - M; Print(N);
      N = I.i(); N -= I; Print(N);
      N = I.t(); N -= I; Print(N);
      N = I.t(); N += (-I); Print(N); // <----------------
      D = I; N = D; D = 1; N -= D; Print(N);
      N = I; D = 1; N -= D; Print(N);
      N = M + 2 * IdentityMatrix(10); N -= (M + 2 * D); Print(N);

      I *= 4;

      D = 4;

      X.ReSize(14);
      X(1) = Sum(D) - Sum(I);
      X(2) = SumAbsoluteValue(D) - SumAbsoluteValue(I);
      X(3) = SumSquare(D) - SumSquare(I);
      X(4) = Trace(D) - Trace(I);
      X(5) = Maximum(D) - Maximum(I);
      X(6) = Minimum(D) - Minimum(I);
      X(7) = LogDeterminant(D).LogValue() - LogDeterminant(I).LogValue();  // <--
      X(8) = LogDeterminant(D).Sign() - LogDeterminant(I).Sign();
      int i,j;
      X(9) = I.Maximum1(i) - 4; X(10) = i-1;
      X(11) = I.Maximum2(i,j) - 4; X(12) = i-10; X(13) = j-10;
      X(14) = I.Nrows() - 10;
      Clean(X,0.00000001); Print(X);


      N = D.i();
      N += I / (-16);
      Print(N);
      N = M * I - 4 * M; Print(N);
      N = I * M - 4 * M; Print(N);
      N = M * I.i() - 0.25 * M; Print(N);
      N = I.i() * M - 0.25 * M; Print(N);
      N = I.i(); N -= I * 0.0625; Print(N);
      N = I.i(); N = N - 0.0625 * I; Print(N);
      N = I.t(); N -= I; Print(N);
      D = I * 2; N = D; D = 1; N -= 8 * D; Print(N);
      N = I * 2; N -= 8 * D; Print(N);
      N = 0.5 * I + M; N -= M; N -= 2.0 * D; Print(N);

      IdentityMatrix J(10); J = 8;
      D = 4;
      DiagonalMatrix E(10); E = 8;
      N = (I + J) - (D + E); Print(N);
      N = (5*I + 3*J) - (5*D + 3*E); Print(N);
      N = (-I + J) - (-D + E); Print(N);
      N = (I - J) - (D - E); Print(N);
      N = (I | J) - (D | E); Print(N);
      N = (I & J) - (D & E); Print(N);
      N = SP(I,J) - SP(D,E); Print(N);
      N = D.SubMatrix(2,5,3,8) - I.SubMatrix(2,5,3,8); Print(N);

      N = M; N.Inject(I); D << M; N -= (M + I); N += D; Print(N);
      D = 4;

      IdentityMatrix K = I.i()*7 - J.t()/4;
      N = D.i() * 7 - E / 4 - K; Print(N);
      K = I * J; N = K - D * E; Print(N);
      N = I * J; N -= D * E; Print(N);
      K = 5*I - 3*J;
      N = K - (5*D - 3*E); Print(N);
      K = I.i(); N = K - 0.0625 * I; Print(N);
      K = I.t(); N = K - I; Print(N);


      K.ReSize(20); D.ReSize(20); D = 1;
      D -= K; Print(D);

      I.ReSize(3); J.ReSize(3); K = I * J; N = K - I; Print(N);
      K << D; N = K - D; Print(N);
   }

   // test add integer
   {
      Matrix X(2,3);
      X << 5.25 << 7.75 << 1.25
        << 9.00 << 1.00 << 2.50;
      Matrix Y = X;
      X = 10 + X;
      X += (-10);
      X -= Y;
      Print(X);

      // also test f suffix
      X << 5.25f << 7.75f << 1.25f
        << 9.00f << 1.00f << 2.50f;
      X -= Y; Print(X);

   }




//   cout << "\nEnd of second test\n";
}
