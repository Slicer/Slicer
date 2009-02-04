
#define WANT_STREAM

#include "include.h"

#include "newmatap.h"
#include "newmatio.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif


static inline int my_min(int x, int y) { return x < y ? x : y; }
static inline int my_max(int x, int y) { return x > y ? x : y; }


#ifdef SETUP_C_SUBSCRIPTS

void trymatk()
{
   Tracer et("Twentieth test of Matrix package");
   Tracer::PrintTrace();
   // test C subscript package


   int i,j; Matrix X, Y;

   cout << "Matrix\n";
   Matrix A(15,35), B(15, 35);
   for (i=0; i<15; i++) for (j=0; j<35; j++)
      { A[i][j] = i+100*j; B(i+1,j+1) = i+100*j; }
   X = A - B; Print(X); Y = X;
   for (i=0; i<15; i++) for (j=0; j<35; j++)
   {
      X.element(i,j) = A.element(i,j) - B[i][j];
      Y.element(i,j) = ((const Matrix&)A)[i][j] - B[i][j];
   }
   Print(X); Print(Y);
   A.CleanUp(); B.CleanUp();

   cout << "UpperTriangularMatrix\n";
   UpperTriangularMatrix A1(15), B1(15);
   for (i=0; i<15; i++) for (j=i; j<15; j++)
      { A1[i][j] = i+100*j; B1(i+1,j+1) = i+100*j; }
   X = A1 - B1; Print(X); Y = X;
   for (i=0; i<15; i++) for (j=i; j<15; j++)
   {
      X.element(i,j) = A1.element(i,j) - B1[i][j];
      Y.element(i,j) = ((const UpperTriangularMatrix&)A1)[i][j] - B1[i][j];
   }
   Print(X); Print(Y);
   A1.CleanUp(); B1.CleanUp();

   cout << "LowerTriangularMatrix\n";
   LowerTriangularMatrix A2(35), B2(35); 
   for (i=0; i<35; i++) for (j=0; j<=i; j++)
      { A2[i][j] = i+100*j; B2(i+1,j+1) = i+100*j; }
   X = A2 - B2; Print(X); Y = X;
   for (i=0; i<35; i++) for (j=0; j<=i; j++)
   {
      X.element(i,j) = A2.element(i,j) - B2[i][j];
      Y.element(i,j) = ((const LowerTriangularMatrix&)A2)[i][j] - B2[i][j];
   }
   Print(X); Print(Y);
   A2.CleanUp(); B2.CleanUp();

   cout << "SymmetricMatrix\n";
   SymmetricMatrix A3(10), B3(10);
   for (i=0; i<10; i++) for (j=0; j<=i; j++)
      { A3[i][j] = i+100*j; B3(i+1,j+1) = i+100*j; }
   X = A3 - B3; Print(X); Y = X;
   for (i=0; i<10; i++) for (j=0; j<=i; j++)
   {
      X.element(i,j) = A3.element(i,j) - B3[i][j];
      Y.element(i,j) = ((const SymmetricMatrix&)A3)[i][j] - B3[i][j];
   }
   Print(X); Print(Y);
   A3.CleanUp(); B3.CleanUp();

   cout << "DiagonalMatrix\n";
   DiagonalMatrix A4(10), B4(10);
   for (i=0; i<10; i++)
      { A4[i] = i+100; B4(i+1) = i+100; }
   X = A4 - B4; Print(X); Y = X;
   for (i=0; i<10; i++)
   {
      X.element(i,i) = A4.element(i) - B4[i];
      Y.element(i,i) = ((const DiagonalMatrix&)A4)[i] - B4[i];
   }
   Print(X); Print(Y);
   A4.CleanUp(); B4.CleanUp();

   cout << "RowVector\n";
   RowVector A5(10), B5(10);
   for (i=0; i<10; i++)
      { A5[i] = i+100; B5(i+1) = i+100; }
   X = A5 - B5; Print(X); Y = X;
   for (i=0; i<10; i++)
   {
      X.element(0,i) = A5.element(i) - B5[i];
      Y.element(0,i) = ((const RowVector&)A5)[i] - B5[i];
   }
   Print(X); Print(Y);
   A5.CleanUp(); B5.CleanUp();

   cout << "ColumnVector\n";
   ColumnVector A6(10), B6(10);
   for (i=0; i<10; i++)
      { A6[i] = i+100; B6(i+1) = i+100; }
   X = A6 - B6; Print(X); Y = X;
   for (i=0; i<10; i++)
   {
      X.element(i,0) = A6.element(i) - B6[i];
      Y.element(i,0) = ((const ColumnVector&)A6)[i] - B6[i];
   }
   Print(X); Print(Y);
   A6.CleanUp(); B6.CleanUp();

   cout << "BandMatrix\n";
   BandMatrix A7(55,10, 5), B7(55, 10, 5);
   for (i=0; i<55; i++) for (j=my_max(0,i-10); j<=my_min(54,i+5); j++)
      { A7[i][j] = i+100*j; B7(i+1,j+1) = i+100*j; }
   X = A7 - B7; Print(X); Y = X;
   for (i=0; i<55; i++) for (j=my_max(0,i-10); j<=my_min(54,i+5); j++)
   {
      X.element(i,j) = A7.element(i,j) - B7[i][j];
      Y.element(i,j) = ((const BandMatrix&)A7)[i][j] - B7[i][j];
   }
   Print(X); Print(Y);
   A7.CleanUp(); B7.CleanUp();

   cout << "UpperBandMatrix\n";
   UpperBandMatrix A8(80,15), B8(80,15);
   for (i=0; i<80; i++) for (j=i; j<=my_min(79,i+15); j++)
      { A8[i][j] = i+100*j; B8(i+1,j+1) = i+100*j; }
   X = A8 - B8; Print(X); Y = X;
   for (i=0; i<80; i++) for (j=i; j<=my_min(79,i+15); j++)
   {
      X.element(i,j) = A8.element(i,j) - B8[i][j];
      Y.element(i,j) = ((const UpperBandMatrix&)A8)[i][j] - B8[i][j];
   }
   Print(X); Print(Y);
   A8.CleanUp(); B8.CleanUp();

   cout << "LowerBandMatrix\n";
   LowerBandMatrix A9(75,27), B9(75,27);
   for (i=0; i<75; i++) for (j=my_max(0,i-27); j<=i; j++)
      { A9[i][j] = i+100*j; B9(i+1,j+1) = i+100*j; }
   X = A9 - B9; Print(X); Y = X;
   for (i=0; i<75; i++) for (j=my_max(0,i-27); j<=i; j++)
   {
      X.element(i,j) = A9.element(i,j) - B9[i][j];
      Y.element(i,j) = ((const LowerBandMatrix&)A9)[i][j] - B9[i][j];
   }
   Print(X); Print(Y);
   A9.CleanUp(); B9.CleanUp();

   cout << "SymmetricBandMatrix\n";
   SymmetricBandMatrix Aa(69,15), Ba(69,15);
   for (i=0; i<69; i++) for (j=my_max(0,i-15); j<=i; j++)
      { Aa[i][j] = i+100*j; Ba(i+1,j+1) = i+100*j; }
   X = Aa - Ba; Print(X); Y = X;
   for (i=0; i<69; i++) for (j=my_max(0,i-15); j<=i; j++)
   {
      X.element(i,j) = Aa.element(i,j) - Ba[i][j];
      Y.element(i,j) = ((const SymmetricBandMatrix&)Aa)[i][j] - Ba[i][j];
   }
   Print(X); Print(Y);
   Aa.CleanUp(); Ba.CleanUp();
   
   // test special constructors used in Numerical Recipes for C++
   Real a[] = {1.2, 5.6, 7.9, 3.8, 4.5, 1.3,
               5.2, 9.9, 2.1, 4.7, 0.0, 1.6 };
   ColumnVector CV0(12); CV0 << a;
   ColumnVector CV1(a, 12); CV1 -= CV0; Print(CV1);
   RowVector RV1(a, 12); RV1 -= CV0.AsRow(); Print(RV1);
   Matrix RM1(a, 3, 4); RM1 -= CV0.AsMatrix(3,4); Print(RM1);
   ColumnVector CV2(5.75, 15); CV2 -= 5.75; Print(CV2);
   RowVector RV2(2.75, 15); RV2 -= 2.75; Print(RV2);
   Matrix RM2(-3.75, 6, 4); RM2 += 3.75; Print(RM2);


}


#else

void trymatk()
{
   Tracer et("Twentieth test of Matrix package");
   Tracer::PrintTrace();
   // test C subscript package
   cout << "C subscripts not enabled, not tested\n\n";
}

#endif
