
//#define WANT_STREAM

#include "include.h"

#include "newmat.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif


/**************************** test program ******************************/

// make sure matrices work as members of a class

class TestClass
{
   Matrix A;
   Matrix B;
public:
   TestClass();
   ReturnMatrix Sum();
};

TestClass::TestClass() : A(2,3)
{
   B.ReSize(2,3);
   A << 1 << 4
     << 4 << 1
     << 2 << 9;
   B << 8 << 5
     << 5 << 8
     << 7 << 0;
}

ReturnMatrix TestClass::Sum() { return Matrix(A + B).ForReturn(); }



void trymatb()
{
//   cout << "\nEleventh test of Matrix package\n";
   Tracer et("Eleventh test of Matrix package");
   Tracer::PrintTrace();
   int i; int j;
   RowVector RV; RV.ReSize(10);
   {
      Tracer et1("Stage 1");
      for (i=1;i<=10;i++) RV(i)=i*i-3;
      Matrix X(1,1); X(1,1) = .25;
      Print(RowVector(X.i() * RV - RV / .25));
//      Print(RowVector(X.i() * Matrix(RV) - RV / .25)); // != zortech, AT&T
      Print(RowVector(X.i() * RV - RV / .25));
   }
   LowerTriangularMatrix L(5); UpperTriangularMatrix U(5);
   for (i=1; i<=5; i++) for (j=1; j<=i; j++)
   { L(i,j) = i*i + j -2.0; U(j,i) = i*i*j+3; }
   DiagonalMatrix D(5);
   for (i=1; i<=5; i++) D(i,i) = i*i + i + 2;
   Matrix M1 = -L; Matrix M2 = L-U; Matrix M3 = U*3; Matrix M4 = U-L;
   Matrix M5 = M1 - D; M1 = D * (-3) - M3;
   {
      Tracer et1("Stage 2");
      Print(Matrix((M2-M4*2)+M5*3-M1));
      M1 = L.t(); Print(Matrix(M1.t()-L));
      M1 = U.t(); Print(Matrix(M1.t()-U));
   }
   {
      Tracer et1("Stage 3");
      SymmetricMatrix S(5);
      for (i=1; i<=5; i++) for (j=1; j<=i; j++) S(i,j) = i*j+i-j+5;
      M2 = S.i() * M4; M1 = S; M3=M1*M2-M4; Clean(M3,0.00000001); Print(M3);
      SymmetricMatrix T(5);
      for (i=1; i<=5; i++) for (j=1; j<=i; j++) T(i,j) = i*i*j*j+i-j+5-i*j;
      M1 = S.i() * T; M1 = S * M1; M1 = M1-T; Clean(M1,0.00000001); Print(M1);
      ColumnVector CV(5); for (i=1; i<=5; i++) CV(i) = i*i*i+10;
      M1 = CV * RV;
   }
   {
      Tracer et1("Stage 4");
      M4.ReSize(5,10);
      for (i=1; i<=5; i++) for (j=1; j<=10; j++) M4(i,j) = (i*i*i+10)*(j*j-3);
      Print(Matrix(M1-M4));
      M1 = L.t(); M2 = U.t(); M3 = L+U; Print(Matrix(M1-M3.t()+M2));
   }
//   UpperTriangularMatrix U2((const UpperTriangularMatrix&)U); // != zortech
   UpperTriangularMatrix U2((UpperTriangularMatrix&)U);
   {
      Tracer et1("Stage 5");
      Print(Matrix(U2-U));
      M2.ReSize(10,10);
      for (i=1; i<=10; i++) for (j=1; j<=10; j++) M2(i,j) = (i*i*i+10)*(j*j-3);
      D << M2; L << M2; U << M2;               // check copy into
      Print( Matrix( (D+M2)-(L+U) ) );
   }
   {
      Tracer et1("Stage 6");
      M1.ReSize(6,10);
      for (i=1; i<=6; i++) for (j=1; j<=10; j++)  M1(i,j) = 100*i + j;
      M2 = M1.SubMatrix(3,5,4,7);  M3.ReSize(3,4);
      for (i=3; i<=5; i++) for (j=4; j<=7; j++)   M3(i-2,j-3) = 100*i + j;
      Print(Matrix(M2-M3));
   }
   int a1,a2,a3,a4;
   {
      Tracer et1("Stage 7");
      int a1,a2,a3,a4;
      a1=4; a2=9; a3=4; a4=7;
      U.ReSize(10);
      for (i=1; i<=10; i++) for (j=i; j<=10; j++)  U(i,j) = 100*i + j;
      M2 = U.SubMatrix(a1,a2,a3,a4);
      M3.ReSize(a2-a1+1,a4-a3+1); M3=0.0;
      for (i=a1; i<=a2; i++) for (j=(i>a3) ? i : a3; j<=a4; j++)
         M3(i-a1+1,j-a3+1) = 100*i + j;
      Print(Matrix(M2-M3));
   }
   {
      Tracer et1("Stage 8");
      a1=3; a2=9; a3=2; a4=7;
      U.ReSize(10);
      for (i=1; i<=10; i++) for (j=i; j<=10; j++)  U(i,j) = 100*i + j;
      M2 = U.SubMatrix(a1,a2,a3,a4);
      M3.ReSize(a2-a1+1,a4-a3+1); M3=0.0;
      for (i=a1; i<=a2; i++) for (j=(i>a3) ? i : a3; j<=a4; j++)
         M3(i-a1+1,j-a3+1) = 100*i + j;
      Print(Matrix(M2-M3));
   }
   {
      Tracer et1("Stage 9");
      a1=4; a2=6; a3=2; a4=5;
      U.ReSize(10);
      for (i=1; i<=10; i++) for (j=i; j<=10; j++)  U(i,j) = 100*i + j;
      M2 = U.SubMatrix(a1,a2,a3,a4);
      M3.ReSize(a2-a1+1,a4-a3+1); M3=0.0;
      for (i=a1; i<=a2; i++) for (j=(i>a3) ? i : a3; j<=a4; j++)
         M3(i-a1+1,j-a3+1) = 100*i + j;
      Print(Matrix(M2-M3));
   }

   {
      Tracer et1("Stage 10");
      TestClass TC;
      Matrix M = TC.Sum() - 9;
      Print(M);
   }


//   cout << "\nEnd of eleventh test\n";
}
