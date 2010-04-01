
//#define WANT_STREAM
#define WANT_MATH

#include "include.h"

#include "newmatap.h"
//#include "newmatio.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif

// check D is sorted
void CheckIsSorted(const DiagonalMatrix& D, bool ascending = false)
{
   DiagonalMatrix D1 = D;
   if (ascending) SortAscending(D1); else SortDescending(D1);
   D1 -= D; Print(D1);
}



void trymate()
{


   Tracer et("Fourteenth test of Matrix package");
   Tracer::PrintTrace();

   {
      Tracer et1("Stage 1");
      Matrix A(8,5);
      {
         Real   a[] = { 22, 10,  2,  3,  7,
                        14,  7, 10,  0,  8,
                        -1, 13, -1,-11,  3,
                        -3, -2, 13, -2,  4,
                         9,  8,  1, -2,  4,
                         9,  1, -7,  5, -1,
                         2, -6,  6,  5,  1,
                         4,  5,  0, -2,  2 };
         int   ai[] = { 22, 10,  2,  3,  7,
                        14,  7, 10,  0,  8,
                        -1, 13, -1,-11,  3,
                        -3, -2, 13, -2,  4,
                         9,  8,  1, -2,  4,
                         9,  1, -7,  5, -1,
                         2, -6,  6,  5,  1,
                         4,  5,  0, -2,  2 };
         A << a;

         Matrix AI(8,5);
         AI << ai; AI -= A; Print(AI);
         int b[] = { 13, -1,-11,
                     -2, 13, -2,
                      8,  1, -2,
                      1, -7,  5 };
         Matrix B(8, 5); B = 23;
         B.SubMatrix(3,6,2,4) << b;
         AI = A;
         AI.Rows(1,2) = 23; AI.Rows(7,8) = 23;
         AI.Column(1) = 23; AI.Column(5) = 23;
         AI -= B; Print(AI);
      }
      DiagonalMatrix D; Matrix U; Matrix V;
      int anc = A.Ncols(); IdentityMatrix I(anc);
      SymmetricMatrix S1; S1 << A.t() * A;
      SymmetricMatrix S2; S2 << A * A.t();
      Real zero = 0.0; SVD(A+zero,D,U,V); CheckIsSorted(D);
      DiagonalMatrix D1; SVD(A,D1); CheckIsSorted(D1);
      Print(DiagonalMatrix(D-D1));
      Matrix W;
      SVD(A, D1, W, W, true, false); D1 -= D; W -= U;
      Clean(W,0.000000001); Print(W); Clean(D1,0.000000001); Print(D1);
      Matrix WX;
      SVD(A, D1, WX, W, false, true); D1 -= D; W -= V;
      Clean(W,0.000000001); Print(W); Clean(D1,0.000000001); Print(D1);
      Matrix SU = U.t() * U - I; Clean(SU,0.000000001); Print(SU);
      Matrix SV = V.t() * V - I; Clean(SV,0.000000001); Print(SV);
      Matrix B = U * D * V.t() - A; Clean(B,0.000000001); Print(B);
      D1=0.0;  SVD(A,D1,A); CheckIsSorted(D1); Print(Matrix(A-U));
      D(1) -= sqrt(1248.0); D(2) -= 20; D(3) -= sqrt(384.0);
      Clean(D,0.000000001); Print(D);

      Jacobi(S1, D, V);  CheckIsSorted(D, true);
      V = S1 - V * D * V.t(); Clean(V,0.000000001); Print(V);
      D = D.Reverse(); D(1)-=1248; D(2)-=400; D(3)-=384;
      Clean(D,0.000000001); Print(D);

      Jacobi(S1, D);  CheckIsSorted(D, true);
      D = D.Reverse(); D(1)-=1248; D(2)-=400; D(3)-=384;
      Clean(D,0.000000001); Print(D);

      SymmetricMatrix JW(5);
      Jacobi(S1, D, JW);  CheckIsSorted(D, true);
      D = D.Reverse(); D(1)-=1248; D(2)-=400; D(3)-=384;
      Clean(D,0.000000001); Print(D);

      Jacobi(S2, D, V);  CheckIsSorted(D, true);
      V = S2 - V * D * V.t(); Clean(V,0.000000001); Print(V);
      D = D.Reverse(); D(1)-=1248; D(2)-=400; D(3)-=384;
      Clean(D,0.000000001); Print(D);

      EigenValues(S1, D, V); CheckIsSorted(D, true);
      V = S1 - V * D * V.t(); Clean(V,0.000000001); Print(V);
      D(5)-=1248; D(4)-=400; D(3)-=384;
      Clean(D,0.000000001); Print(D);

      EigenValues(S2, D, V); CheckIsSorted(D, true);
      V = S2 - V * D * V.t(); Clean(V,0.000000001); Print(V);
      D(8)-=1248; D(7)-=400; D(6)-=384;
      Clean(D,0.000000001); Print(D);

      EigenValues(S1, D); CheckIsSorted(D, true);
      D(5)-=1248; D(4)-=400; D(3)-=384;
      Clean(D,0.000000001); Print(D);

      SymmetricMatrix EW(S2);
      EigenValues(S2, D, EW); CheckIsSorted(D, true);
      D(8)-=1248; D(7)-=400; D(6)-=384;
      Clean(D,0.000000001); Print(D);

   }

   {
      Tracer et1("Stage 2");
      Matrix A(20,21);
      int i,j;
      for (i=1; i<=20; i++) for (j=1; j<=21; j++)
      { if (i>j) A(i,j) = 0; else if (i==j) A(i,j) = 21-i; else A(i,j) = -1; }
      A = A.t();
      SymmetricMatrix S1; S1 << A.t() * A;
      SymmetricMatrix S2; S2 << A * A.t();
      DiagonalMatrix D; Matrix U; Matrix V;
      DiagonalMatrix I(A.Ncols());
      I=1.0;
      SVD(A,D,U,V); CheckIsSorted(D);
      Matrix SU = U.t() * U - I;    Clean(SU,0.000000001); Print(SU);
      Matrix SV = V.t() * V - I;    Clean(SV,0.000000001); Print(SV);
      Matrix B = U * D * V.t() - A; Clean(B,0.000000001);  Print(B);
      for (i=1; i<=20; i++)  D(i) -= sqrt((22.0-i)*(21.0-i));
      Clean(D,0.000000001); Print(D);
      Jacobi(S1, D, V); CheckIsSorted(D, true);
      V = S1 - V * D * V.t(); Clean(V,0.000000001); Print(V);
      D = D.Reverse();
      for (i=1; i<=20; i++)  D(i) -= (22-i)*(21-i);
      Clean(D,0.000000001); Print(D);
      Jacobi(S2, D, V); CheckIsSorted(D, true);
      V = S2 - V * D * V.t(); Clean(V,0.000000001); Print(V);
      D = D.Reverse();
      for (i=1; i<=20; i++)  D(i) -= (22-i)*(21-i);
      Clean(D,0.000000001); Print(D);

      EigenValues(S1, D, V); CheckIsSorted(D, true);
      V = S1 - V * D * V.t(); Clean(V,0.000000001); Print(V);
      for (i=1; i<=20; i++)  D(i) -= (i+1)*i;
      Clean(D,0.000000001); Print(D);
      EigenValues(S2, D, V); CheckIsSorted(D, true);
      V = S2 - V * D * V.t(); Clean(V,0.000000001); Print(V);
      for (i=2; i<=21; i++)  D(i) -= (i-1)*i;
      Clean(D,0.000000001); Print(D);

      EigenValues(S1, D); CheckIsSorted(D, true);
      for (i=1; i<=20; i++)  D(i) -= (i+1)*i;
      Clean(D,0.000000001); Print(D);
      EigenValues(S2, D); CheckIsSorted(D, true);
      for (i=2; i<=21; i++)  D(i) -= (i-1)*i;
      Clean(D,0.000000001); Print(D);
   }

   {
      Tracer et1("Stage 3");
      Matrix A(30,30);
      int i,j;
      for (i=1; i<=30; i++) for (j=1; j<=30; j++)
      { if (i>j) A(i,j) = 0; else if (i==j) A(i,j) = 1; else A(i,j) = -1; }
      Real d1 = A.LogDeterminant().Value();
      DiagonalMatrix D; Matrix U; Matrix V;
      DiagonalMatrix I(A.Ncols());
      I=1.0;
      SVD(A,D,U,V); CheckIsSorted(D);
      Matrix SU = U.t() * U - I; Clean(SU,0.000000001); Print(SU);
      Matrix SV = V.t() * V - I; Clean(SV,0.000000001); Print(SV);
      Real d2 = D.LogDeterminant().Value();
      Matrix B = U * D * V.t() - A; Clean(B,0.000000001); Print(B);
      Real d3 = D.LogDeterminant().Value();
      ColumnVector Test(3);
      Test(1) = d1 - 1; Test(2) = d2 - 1; Test(3) = d3 - 1;
      Clean(Test,0.00000001); Print(Test); // only 8 decimal figures
      A.ReSize(2,2);
      Real a = 1.5; Real b = 2; Real c = 2 * (a*a + b*b);
      A << a << b << a << b;
      I.ReSize(2); I=1;
      SVD(A,D,U,V); CheckIsSorted(D);
      SU = U.t() * U - I; Clean(SU,0.000000001); Print(SU);
      SV = V.t() * V - I; Clean(SV,0.000000001); Print(SV);
      B = U * D * V.t() - A; Clean(B,0.000000001); Print(B);
      D = D*D; SortDescending(D);
      DiagonalMatrix D50(2); D50 << c << 0; D = D - D50;
      Clean(D,0.000000001);
      Print(D);
      A << a << a << b << b;
      SVD(A,D,U,V); CheckIsSorted(D);
      SU = U.t() * U - I; Clean(SU,0.000000001); Print(SU);
      SV = V.t() * V - I; Clean(SV,0.000000001); Print(SV);
      B = U * D * V.t() - A; Clean(B,0.000000001); Print(B);
      D = D*D; SortDescending(D);
      D = D - D50;
      Clean(D,0.000000001);
      Print(D);
   }

   {
      Tracer et1("Stage 4");

      // test for bug found by Olof Runborg,
      // Department of Numerical Analysis and Computer Science (NADA),
      // KTH, Stockholm

      Matrix A(22,20);

      A = 0;

      int a=1;

      A(a+0,a+2) = 1;     A(a+0,a+18) = -1;
      A(a+1,a+9) = 1;     A(a+1,a+12) = -1;
      A(a+2,a+11) = 1;    A(a+2,a+12) = -1;
      A(a+3,a+10) = 1;    A(a+3,a+19) = -1;
      A(a+4,a+16) = 1;    A(a+4,a+19) = -1;
      A(a+5,a+17) = 1;    A(a+5,a+18) = -1;
      A(a+6,a+10) = 1;    A(a+6,a+4) = -1;
      A(a+7,a+3) = 1;     A(a+7,a+2) = -1;
      A(a+8,a+14) = 1;    A(a+8,a+15) = -1;
      A(a+9,a+13) = 1;    A(a+9,a+16) = -1;
      A(a+10,a+8) = 1;    A(a+10,a+9) = -1;
      A(a+11,a+1) = 1;    A(a+11,a+15) = -1;
      A(a+12,a+16) = 1;   A(a+12,a+4) = -1;
      A(a+13,a+6) = 1;    A(a+13,a+9) = -1;
      A(a+14,a+5) = 1;    A(a+14,a+4) = -1;
      A(a+15,a+0) = 1;    A(a+15,a+1) = -1;
      A(a+16,a+14) = 1;   A(a+16,a+0) = -1;
      A(a+17,a+7) = 1;    A(a+17,a+6) = -1;
      A(a+18,a+13) = 1;   A(a+18,a+5) = -1;
      A(a+19,a+7) = 1;    A(a+19,a+8) = -1;
      A(a+20,a+17) = 1;   A(a+20,a+3) = -1;
      A(a+21,a+6) = 1;    A(a+21,a+11) = -1;


      Matrix U, V; DiagonalMatrix S;

      SVD(A, S, U, V, true, true); CheckIsSorted(S);

      DiagonalMatrix D(20); D = 1;

      Matrix tmp = U.t() * U - D;
      Clean(tmp,0.000000001); Print(tmp);

      tmp = V.t() * V - D;
      Clean(tmp,0.000000001); Print(tmp);

      tmp = U * S * V.t() - A ;
      Clean(tmp,0.000000001); Print(tmp);

   }

   {
      Tracer et1("Stage 5");
      Matrix A(10,10);

      A.Row(1)  <<  1.00 <<  0.07 <<  0.05 <<  0.00 <<  0.06
                <<  0.09 <<  0.03 <<  0.02 <<  0.02 << -0.03;
      A.Row(2)  <<  0.07 <<  1.00 <<  0.05 <<  0.05 << -0.03
                <<  0.07 <<  0.00 <<  0.07 <<  0.00 <<  0.02;
      A.Row(3)  <<  0.05 <<  0.05 <<  1.00 <<  0.05 <<  0.02
                <<  0.01 << -0.05 <<  0.04 <<  0.05 << -0.03;
      A.Row(4)  <<  0.00 <<  0.05 <<  0.05 <<  1.00 << -0.05
                <<  0.04 <<  0.01 <<  0.02 << -0.05 <<  0.00;
      A.Row(5)  <<  0.06 << -0.03 <<  0.02 << -0.05 <<  1.00
                << -0.03 <<  0.02 << -0.02 <<  0.04 <<  0.00;
      A.Row(6)  <<  0.09 <<  0.07 <<  0.01 <<  0.04 << -0.03
                <<  1.00 << -0.06 <<  0.08 << -0.02 << -0.10;
      A.Row(7)  <<  0.03 <<  0.00 << -0.05 <<  0.01 <<  0.02
                << -0.06 <<  1.00 <<  0.09 <<  0.12 << -0.03;
      A.Row(8)  <<  0.02 <<  0.07 <<  0.04 <<  0.02 << -0.02
                <<  0.08 <<  0.09 <<  1.00 <<  0.00 << -0.02;
      A.Row(9)  <<  0.02 <<  0.00 <<  0.05 << -0.05 <<  0.04
                << -0.02 <<  0.12 <<  0.00 <<  1.00 <<  0.02;
      A.Row(10) << -0.03 <<  0.02 << -0.03 <<  0.00 <<  0.00
                << -0.10 << -0.03 << -0.02 <<  0.02 <<  1.00;

      SymmetricMatrix AS; AS << A;
      Matrix V; DiagonalMatrix D, D1;
      ColumnVector Check(6);
      EigenValues(AS,D,V); CheckIsSorted(D, true);
      Check(1) = MaximumAbsoluteValue(A - V * D * V.t());
      DiagonalMatrix I(10); I = 1;
      Check(2) = MaximumAbsoluteValue(V * V.t() - I);
      Check(3) = MaximumAbsoluteValue(V.t() * V - I);

      EigenValues(AS, D1); CheckIsSorted(D1, true);
      D -= D1;
      Clean(D,0.000000001); Print(D);

      Jacobi(AS,D,V);
      Check(4) = MaximumAbsoluteValue(A - V * D * V.t());
      Check(5) = MaximumAbsoluteValue(V * V.t() - I);
      Check(6) = MaximumAbsoluteValue(V.t() * V - I);

      SortAscending(D);
      D -= D1;
      Clean(D,0.000000001); Print(D);

      Clean(Check,0.000000001); Print(Check);

      // Check loading rows

      SymmetricMatrix B(10);

      B.Row(1)  <<  1.00;
      B.Row(2)  <<  0.07 <<  1.00;
      B.Row(3)  <<  0.05 <<  0.05 <<  1.00;
      B.Row(4)  <<  0.00 <<  0.05 <<  0.05 <<  1.00;
      B.Row(5)  <<  0.06 << -0.03 <<  0.02 << -0.05 <<  1.00;
      B.Row(6)  <<  0.09 <<  0.07 <<  0.01 <<  0.04 << -0.03
                <<  1.00;
      B.Row(7)  <<  0.03 <<  0.00 << -0.05 <<  0.01 <<  0.02
                << -0.06 <<  1.00;
      B.Row(8)  <<  0.02 <<  0.07 <<  0.04 <<  0.02 << -0.02
                <<  0.08 <<  0.09 <<  1.00;
      B.Row(9)  <<  0.02 <<  0.00 <<  0.05 << -0.05 <<  0.04
                << -0.02 <<  0.12 <<  0.00 <<  1.00;
      B.Row(10) << -0.03 <<  0.02 << -0.03 <<  0.00 <<  0.00
                << -0.10 << -0.03 << -0.02 <<  0.02 <<  1.00;

      B -= AS; Print(B);

   }

   {
      Tracer et1("Stage 6");
      // badly scaled matrix
      Matrix A(9,9);

      A.Row(1) << 1.13324e+012 << 3.68788e+011 << 3.35163e+009
               << 3.50193e+011 << 1.25335e+011 << 1.02212e+009
               << 3.16602e+009 << 1.02418e+009 << 9.42959e+006;
      A.Row(2) << 3.68788e+011 << 1.67128e+011 << 1.27449e+009
               << 1.25335e+011 << 6.05413e+010 << 4.34573e+008
               << 1.02418e+009 << 4.69192e+008 << 3.61098e+006;
      A.Row(3) << 3.35163e+009 << 1.27449e+009 << 1.25571e+007
               << 1.02212e+009 << 4.34573e+008 << 3.69769e+006
               << 9.42959e+006 << 3.61098e+006 << 3.59450e+004;
      A.Row(4) << 3.50193e+011 << 1.25335e+011 << 1.02212e+009
               << 1.43514e+011 << 5.42310e+010 << 4.15822e+008
               << 1.23068e+009 << 4.31545e+008 << 3.58714e+006;
      A.Row(5) << 1.25335e+011 << 6.05413e+010 << 4.34573e+008
               << 5.42310e+010 << 2.76601e+010 << 1.89102e+008
               << 4.31545e+008 << 2.09778e+008 << 1.51083e+006;
      A.Row(6) << 1.02212e+009 << 4.34573e+008 << 3.69769e+006
               << 4.15822e+008 << 1.89102e+008 << 1.47143e+006
               << 3.58714e+006 << 1.51083e+006 << 1.30165e+004;
      A.Row(7) << 3.16602e+009 << 1.02418e+009 << 9.42959e+006
               << 1.23068e+009 << 4.31545e+008 << 3.58714e+006
               << 1.12335e+007 << 3.54778e+006 << 3.34311e+004;
      A.Row(8) << 1.02418e+009 << 4.69192e+008 << 3.61098e+006
               << 4.31545e+008 << 2.09778e+008 << 1.51083e+006
               << 3.54778e+006 << 1.62552e+006 << 1.25885e+004;
      A.Row(9) << 9.42959e+006 << 3.61098e+006 << 3.59450e+004
               << 3.58714e+006 << 1.51083e+006 << 1.30165e+004
               << 3.34311e+004 << 1.25885e+004 << 1.28000e+002;


      SymmetricMatrix AS; AS << A;
      Matrix V; DiagonalMatrix D, D1;
      ColumnVector Check(6);
      EigenValues(AS,D,V); CheckIsSorted(D, true);
      Check(1) = MaximumAbsoluteValue(A - V * D * V.t()) / 100000;
      DiagonalMatrix I(9); I = 1;
      Check(2) = MaximumAbsoluteValue(V * V.t() - I);
      Check(3) = MaximumAbsoluteValue(V.t() * V - I);

      EigenValues(AS, D1);
      D -= D1;
      Clean(D,0.001); Print(D);

      Jacobi(AS,D,V);
      Check(4) = MaximumAbsoluteValue(A - V * D * V.t()) / 100000;
      Check(5) = MaximumAbsoluteValue(V * V.t() - I);
      Check(6) = MaximumAbsoluteValue(V.t() * V - I);

      SortAscending(D);
      D -= D1;
      Clean(D,0.001); Print(D);

      Clean(Check,0.0000001); Print(Check);
   }

   {
      Tracer et1("Stage 7");
      // matrix with all singular values close to 1
      Matrix A(8,8);
      A.Row(1)<<-0.4343<<-0.0445<<-0.4582<<-0.1612<<-0.3191<<-0.6784<<0.1068<<0;
      A.Row(2)<<0.5791<<0.5517<<0.2575<<-0.1055<<-0.0437<<-0.5282<<0.0442<<0;
      A.Row(3)<<0.5709<<-0.5179<<-0.3275<<0.2598<<-0.196<<-0.1451<<-0.4143<<0;
      A.Row(4)<<0.2785<<-0.5258<<0.1251<<-0.4382<<0.0514<<-0.0446<<0.6586<<0;
      A.Row(5)<<0.2654<<0.3736<<-0.7436<<-0.0122<<0.0376<<0.3465<<0.3397<<0;
      A.Row(6)<<0.0173<<-0.0056<<-0.1903<<-0.7027<<0.4863<<-0.0199<<-0.4825<<0;
      A.Row(7)<<0.0434<<0.0966<<0.1083<<-0.4576<<-0.7857<<0.3425<<-0.1818<<0;
      A.Row(8)<<0.0<<0.0<<0.0<<0.0<<0.0<<0.0<<0.0<<-1.0;
      Matrix U,V; DiagonalMatrix D;
      SVD(A,D,U,V); CheckIsSorted(D);
      Matrix B = U * D * V.t() - A; Clean(B,0.000000001); Print(B);
      DiagonalMatrix I(8); I = 1; D -= I; Clean(D,0.0001); Print(D);
      U *= U.t(); U -= I; Clean(U,0.000000001); Print(U);
      V *= V.t(); V -= I; Clean(V,0.000000001); Print(V);

   }

   {
      Tracer et1("Stage 8");
      // check SortSV functions

      Matrix A(15, 10);
      int i, j;
      for (i = 1; i <= 15; ++i) for (j = 1; j <= 10; ++j)
         A(i, j) = i + j / 1000.0;
      DiagonalMatrix D(10);
      D << 0.2 << 0.5 << 0.1 << 0.7 << 0.8 << 0.3 << 0.4 << 0.7 << 0.9 << 0.6;
      Matrix U = A; Matrix V = 10 - 2 * A;
      Matrix Prod = U * D * V.t();

      DiagonalMatrix D2 = D; SortDescending(D2);
      DiagonalMatrix D1 = D; SortSV(D1, U, V); Matrix X = D1 - D2; Print(X);
      X = Prod - U * D1 * V.t(); Clean(X,0.000000001); Print(X);
      U = A; V = 10 - 2 * A;
      D1 = D; SortSV(D1, U); X = D1 - D2; Print(X);
      D1 = D; SortSV(D1, V); X = D1 - D2; Print(X);
      X = Prod - U * D1 * V.t(); Clean(X,0.000000001); Print(X);

      D2 = D; SortAscending(D2);
      U = A; V = 10 - 2 * A;
      D1 = D; SortSV(D1, U, V, true); X = D1 - D2; Print(X);
      X = Prod - U * D1 * V.t(); Clean(X,0.000000001); Print(X);
      U = A; V = 10 - 2 * A;
      D1 = D; SortSV(D1, U, true); X = D1 - D2; Print(X);
      D1 = D; SortSV(D1, V, true); X = D1 - D2; Print(X);
      X = Prod - U * D1 * V.t(); Clean(X,0.000000001); Print(X);
   }

   {
      Tracer et1("Stage 9");
      // Tom William's example
      Matrix A(10,10);
      Matrix U;
      Matrix V;
      DiagonalMatrix Sigma;
      Real myVals[] =
      {
         1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
         1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
         1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
         1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
         1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
         1,    1,    1,    1,    1,    1,    1,    1,    1,    0,
         1,    1,    1,    1,    1,    1,    1,    1,    1,    0,
         1,    1,    1,    1,    1,    1,    1,    1,    0,    0,
         1,    1,    1,    1,    1,    1,    1,    0,    0,    0,
         1,    1,    1,    1,    1,    0,    0,    0,    0,    0,
      };

      A << myVals;
      SVD(A, Sigma, U, V); CheckIsSorted(Sigma);
      A -= U * Sigma * V.t();
      Clean(A, 0.000000001); Print(A);
   }



}
