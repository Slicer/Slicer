
//#define WANT_STREAM
#define WANT_MATH


#include "include.h"

#include "newmatap.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif


/**************************** test program ******************************/


// slow sort program

static void SimpleSortDescending(Real* first, const int length)
{
   int i = length;
   while (--i)
   {
      Real x = *first; Real* f = first; Real* g = f;
      int j = i;
      while (j--) if (x < *(++f)) { g = f; x = *g; }
      *g = *first; *first++ = x;
   }
}

static void TestSort(int n)
{
   // make some data
   RowVector X(n);
   int i;
   for (i = 1; i <= n; i++)
      X(i) = sin((Real)i) + 0.3 * cos(i/5.0) - 0.6 * sin(i/7.0) + 0.2 * sin(2.0 * i);
   RowVector X_Sorted = X; SimpleSortDescending(X_Sorted.Store(), n);
   RowVector A = X + X.Reverse(); SimpleSortDescending(A.Store(), n);

   // test descending sort

   RowVector Y = X; SortDescending(Y); Y -= X_Sorted; Print(Y);
   Y = X_Sorted; SortDescending(Y); Y -= X_Sorted; Print(Y);
   Y = X_Sorted.Reverse(); SortDescending(Y); Y -= X_Sorted; Print(Y);
   Y = X + X.Reverse(); SortDescending(Y); Y -= A; Print(Y);

   // test ascending sort

   Y = X; SortAscending(Y); Y -= X_Sorted.Reverse(); Print(Y);
   Y = X_Sorted; SortAscending(Y); Y -= X_Sorted.Reverse(); Print(Y);
   Y = X_Sorted.Reverse(); SortAscending(Y); Y -= X_Sorted.Reverse(); Print(Y);
   Y = X + X.Reverse(); SortAscending(Y); Y -= A.Reverse(); Print(Y);
}


void trymat6()
{
   Tracer et("Sixth test of Matrix package");
   Tracer::PrintTrace();

   int i,j;


   DiagonalMatrix D(6);
   UpperTriangularMatrix U(6);
   for (i=1;i<=6;i++) { for (j=i;j<=6;j++) U(i,j)=i*i*i-50; D(i,i)=i*i+i-10; }
   LowerTriangularMatrix L=(U*3.0).t();
   SymmetricMatrix S(6);
   for (i=1;i<=6;i++) for (j=i;j<=6;j++) S(i,j)=i*i+2.0+j;
   Matrix MD=D; Matrix ML=L; Matrix MU=U; Matrix MS=S;
   Matrix M(6,6);
   for (i=1;i<=6;i++) for (j=1;j<=6;j++) M(i,j)=i*j+i*i-10.0;  
   {
      Tracer et1("Stage 1");
      Print(Matrix(MS+(-MS)));
      Print(Matrix((S+M)-(MS+M)));
      Print(Matrix((M+U)-(M+MU)));
      Print(Matrix((M+L)-(M+ML)));
   }
   {
      Tracer et1("Stage 2");
      Print(Matrix((M+D)-(M+MD)));
      Print(Matrix((U+D)-(MU+MD)));
      Print(Matrix((D+L)-(ML+MD)));
      Print(Matrix((-U+D)+MU-MD));
      Print(Matrix((-L+D)+ML-MD));
   }
   {
      Tracer et1("Stage 3 - concatenate");
      RowVector A(5);
      A << 1 << 2 << 3 << 4 << 5;
      RowVector B(5);
      B << 3 << 1 << 4 << 1 << 5;
      Matrix C(3,5);
      C <<  2 <<  3 <<  5 <<  7 << 11
        << 13 << 17 << 19 << 23 << 29
        << 31 << 37 << 41 << 43 << 47;
      Matrix X1 = A & B & C;
      Matrix X2 = (A.t() | B.t() | C.t()).t();
      Matrix X3(5,5);
      X3.Row(1)=A; X3.Row(2)=B; X3.Rows(3,5)=C;
      Print(Matrix(X1-X2));
      Print(Matrix(X1-X3));
      LowerTriangularMatrix LT1; LT1 << (A & B & C);
      UpperTriangularMatrix UT1; UT1 << (A.t() | B.t() | C.t());
      Print(LowerTriangularMatrix(LT1-UT1.t()));
      DiagonalMatrix D1; D1 << (A.t() | B.t() | C.t());
      ColumnVector At = A.t();
      ColumnVector Bt = B.t();
      Matrix Ct = C.t();
      LowerTriangularMatrix LT2; LT2 << (At | Bt | Ct);
      UpperTriangularMatrix UT2; UT2 << (At.t() & Bt.t() & Ct.t());
      Matrix ABt = At | Bt;
      DiagonalMatrix D2; D2 << (ABt | Ct);
      Print(LowerTriangularMatrix(LT2-UT2.t()));
      Print(DiagonalMatrix(D1-D2));
      Print(Matrix(LT1+UT2-D2-X1));
      Matrix M1 = LT1 | UT2; Matrix M2 = UT1 & LT2;
      Print(Matrix(M1-M2.t()));
      M1 = UT2 | LT1; M2 = LT2 & UT1;
      Print(Matrix(M1-M2.t()));
      M1 = (LT1 | UT2) & (UT2 | LT1);
      M2 = (UT1 & LT2) | (LT2 & UT1);
      Print(Matrix(M1-M2.t()));
      SymmetricMatrix SM1; SM1 << (M1 + M1.t());
      SymmetricMatrix SM2; SM2 << ((SM1 | M1) & (M1.t() | SM1));
      Matrix M3(20,20);
      M3.SubMatrix(1,10,1,10) = SM1;
      M3.SubMatrix(1,10,11,20) = M1;
      M3.SubMatrix(11,20,1,10) = M2;
      M3.SubMatrix(11,20,11,20) = SM1;
      Print(Matrix(M3-SM2));

      SymmetricMatrix SM(15); SM = 0; SM.SymSubMatrix(1,10) = SM1;
      M3.ReSize(15,15); M3 = 0; M3.SubMatrix(1,10,1,10) = SM1;
      M3 -= SM; Print(M3);
      SM = 0; SM.SymSubMatrix(6,15) = SM1;
      M3.ReSize(15,15); M3 = 0; M3.SubMatrix(6,15,6,15) = SM1;
      M3 = M3.t() - SM; Print(M3);
   }
   {
      Tracer et1("Stage 4 - sort");
      TestSort(1); TestSort(2); TestSort(3); TestSort(4);
      TestSort(15); TestSort(16); TestSort(17); TestSort(18);
      TestSort(99); TestSort(100); TestSort(101);
   }


//   cout << "\nEnd of sixth test\n";
}

