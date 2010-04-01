//$$ example.cpp                             Example of use of matrix package

#define WANT_STREAM                  // include.h will get stream fns
#define WANT_MATH                    // include.h will get math fns
                                     // newmatap.h will get include.h

#include "newmatap.h"                // need matrix applications

#include "newmatio.h"                // need matrix output routines

#ifdef use_namespace
using namespace NEWMAT;              // access NEWMAT namespace
#endif


// demonstration of matrix package on linear regression problem


void test1(Real* y, Real* x1, Real* x2, int nobs, int npred)
{
   cout << "\n\nTest 1 - traditional, bad\n";

   // traditional sum of squares and products method of calculation
   // but not adjusting means; maybe subject to round-off error

   // make matrix of predictor values with 1s into col 1 of matrix
   int npred1 = npred+1;        // number of cols including col of ones.
   Matrix X(nobs,npred1);
   X.column(1) = 1.0;

   // load x1 and x2 into X
   //    [use << rather than = when loading arrays]
   X.column(2) << x1;  X.column(3) << x2;

   // vector of Y values
   ColumnVector Y(nobs); Y << y;

   // form sum of squares and product matrix
   //    [use << rather than = for copying Matrix into SymmetricMatrix]
   SymmetricMatrix SSQ; SSQ << X.t() * X;

   // calculate estimate
   //    [bracket last two terms to force this multiplication first]
   //    [ .i() means inverse, but inverse is not explicity calculated]
   ColumnVector A = SSQ.i() * (X.t() * Y);

   // Get variances of estimates from diagonal elements of inverse of SSQ
   // get inverse of SSQ - we need it for finding D
   DiagonalMatrix D; D << SSQ.i();
   ColumnVector V = D.as_column();

   // Calculate fitted values and residuals
   ColumnVector Fitted = X * A;
   ColumnVector Residual = Y - Fitted;
   Real ResVar = sum_square(Residual) / (nobs-npred1);

   // Get diagonals of Hat matrix (an expensive way of doing this)
   DiagonalMatrix Hat;  Hat << X * (X.t() * X).i() * X.t();

   // print out answers
   cout << "\nEstimates and their standard errors\n\n";

   // make vector of standard errors
   ColumnVector SE(npred1);
   for (int i=1; i<=npred1; i++) SE(i) = sqrt(V(i)*ResVar);
   // use concatenation function to form matrix and use matrix print
   // to get two columns
   cout << setw(11) << setprecision(5) << (A | SE) << endl;

   cout << "\nObservations, fitted value, residual value, hat value\n";

   // use concatenation again; select only columns 2 to 3 of X
   cout << setw(9) << setprecision(3) <<
     (X.columns(2,3) | Y | Fitted | Residual | Hat.as_column());
   cout << "\n\n";
}

void test2(Real* y, Real* x1, Real* x2, int nobs, int npred)
{
   cout << "\n\nTest 2 - traditional, OK\n";

   // traditional sum of squares and products method of calculation
   // with subtraction of means - less subject to round-off error
   // than test1

   // make matrix of predictor values
   Matrix X(nobs,npred);

   // load x1 and x2 into X
   //    [use << rather than = when loading arrays]
   X.column(1) << x1;  X.column(2) << x2;

   // vector of Y values
   ColumnVector Y(nobs); Y << y;

   // make vector of 1s
   ColumnVector Ones(nobs); Ones = 1.0;

   // calculate means (averages) of x1 and x2 [ .t() takes transpose]
   RowVector M = X.sum_columns() / nobs;

   // and subtract means from x1 and x2
   Matrix XC(nobs,npred);
   XC = X - Ones * M;

   // do the same to Y [use sum to get sum of elements]
   ColumnVector YC(nobs);
   Real m = sum(Y) / nobs;  YC = Y - Ones * m;

   // form sum of squares and product matrix
   //    [use << rather than = for copying Matrix into SymmetricMatrix]
   SymmetricMatrix SSQ; SSQ << XC.t() * XC;

   // calculate estimate
   //    [bracket last two terms to force this multiplication first]
   //    [ .i() means inverse, but inverse is not explicity calculated]
   ColumnVector A = SSQ.i() * (XC.t() * YC);

   // calculate estimate of constant term
   //    [AsScalar converts 1x1 matrix to Real]
   Real a = m - (M * A).as_scalar();

   // Get variances of estimates from diagonal elements of inverse of SSQ
   //    [ we are taking inverse of SSQ - we need it for finding D ]
   Matrix ISSQ = SSQ.i(); DiagonalMatrix D; D << ISSQ;
   ColumnVector V = D.AsColumn();
   Real v = 1.0/nobs + (M * ISSQ * M.t()).as_scalar();
                                            // for calc variance of const

   // Calculate fitted values and residuals
   int npred1 = npred+1;
   ColumnVector Fitted = X * A + a;
   ColumnVector Residual = Y - Fitted;
   Real ResVar = sum_square(Residual) / (nobs-npred1);

   // Get diagonals of Hat matrix (an expensive way of doing this)
   Matrix X1(nobs,npred1); X1.column(1)<<Ones; X1.columns(2,npred1)<<X;
   DiagonalMatrix Hat;  Hat << X1 * (X1.t() * X1).i() * X1.t();

   // print out answers
   cout << "\nEstimates and their standard errors\n\n";
   cout.setf(ios::fixed, ios::floatfield);
   cout << setw(11) << setprecision(5)  << a << " ";
   cout << setw(11) << setprecision(5)  << sqrt(v*ResVar) << endl;
   // make vector of standard errors
   ColumnVector SE(npred);
   for (int i=1; i<=npred; i++) SE(i) = sqrt(V(i)*ResVar);
   // use concatenation function to form matrix and use matrix print
   // to get two columns
   cout << setw(11) << setprecision(5) << (A | SE) << endl;
   cout << "\nObservations, fitted value, residual value, hat value\n";
   cout << setw(9) << setprecision(3) <<
     (X | Y | Fitted | Residual | Hat.as_column());
   cout << "\n\n";
}

void test3(Real* y, Real* x1, Real* x2, int nobs, int npred)
{
   cout << "\n\nTest 3 - Cholesky\n";

   // traditional sum of squares and products method of calculation
   // with subtraction of means - using Cholesky decomposition

   Matrix X(nobs,npred);
   X.column(1) << x1;  X.column(2) << x2;
   ColumnVector Y(nobs); Y << y;
   RowVector M = X.sum_columns() / nobs;
   Matrix XC(nobs,npred);
   ColumnVector Ones(nobs); Ones = 1.0;
   XC = X - Ones * M;
   ColumnVector YC(nobs);
   Real m = sum(Y) / nobs;  YC = Y - Ones * m;
   SymmetricMatrix SSQ; SSQ << XC.t() * XC;

   // Cholesky decomposition of SSQ
   LowerTriangularMatrix L = Cholesky(SSQ);

   // calculate estimate
   ColumnVector A = L.t().i() * (L.i() * (XC.t() * YC));

   // calculate estimate of constant term
   Real a = m - (M * A).AsScalar();

   // Get variances of estimates from diagonal elements of invoice of SSQ
   DiagonalMatrix D; D << L.t().i() * L.i();
   ColumnVector V = D.as_column();
   Real v = 1.0/nobs + sum_square(L.i() * M.t());

   // Calculate fitted values and residuals
   int npred1 = npred+1;
   ColumnVector Fitted = X * A + a;
   ColumnVector Residual = Y - Fitted;
   Real ResVar = sum_square(Residual) / (nobs-npred1);

   // Get diagonals of Hat matrix (an expensive way of doing this)
   Matrix X1(nobs,npred1); X1.column(1)<<Ones; X1.columns(2,npred1)<<X;
   DiagonalMatrix Hat;  Hat << X1 * (X1.t() * X1).i() * X1.t();

   // print out answers
   cout << "\nEstimates and their standard errors\n\n";
   cout.setf(ios::fixed, ios::floatfield);
   cout << setw(11) << setprecision(5)  << a << " ";
   cout << setw(11) << setprecision(5)  << sqrt(v*ResVar) << endl;
   ColumnVector SE(npred);
   for (int i=1; i<=npred; i++) SE(i) = sqrt(V(i)*ResVar);
   cout << setw(11) << setprecision(5) << (A | SE) << endl;
   cout << "\nObservations, fitted value, residual value, hat value\n";
   cout << setw(9) << setprecision(3) <<
      (X | Y | Fitted | Residual | Hat.as_column());
   cout << "\n\n";
}

void test4(Real* y, Real* x1, Real* x2, int nobs, int npred)
{
   cout << "\n\nTest 4 - QR triangularisation\n";

   // QR triangularisation method
 
   // load data - 1s into col 1 of matrix
   int npred1 = npred+1;
   Matrix X(nobs,npred1); ColumnVector Y(nobs);
   X.column(1) = 1.0;  X.column(2) << x1;  X.column(3) << x2;  Y << y;

   // do Householder triangularisation
   // no need to deal with constant term separately
   Matrix X1 = X;                 // Want copy of matrix
   ColumnVector Y1 = Y;
   UpperTriangularMatrix U; ColumnVector M;
   QRZ(X1, U); QRZ(X1, Y1, M);    // Y1 now contains resids
   ColumnVector A = U.i() * M;
   ColumnVector Fitted = X * A;
   Real ResVar = sum_square(Y1) / (nobs-npred1);

   // get variances of estimates
   U = U.i(); DiagonalMatrix D; D << U * U.t();

   // Get diagonals of Hat matrix
   DiagonalMatrix Hat;  Hat << X1 * X1.t();

   // print out answers
   cout << "\nEstimates and their standard errors\n\n";
   ColumnVector SE(npred1);
   for (int i=1; i<=npred1; i++) SE(i) = sqrt(D(i)*ResVar);
   cout << setw(11) << setprecision(5) << (A | SE) << endl;
   cout << "\nObservations, fitted value, residual value, hat value\n";
   cout << setw(9) << setprecision(3) << 
      (X.columns(2,3) | Y | Fitted | Y1 | Hat.as_column());
   cout << "\n\n";
}

void test5(Real* y, Real* x1, Real* x2, int nobs, int npred)
{
   cout << "\n\nTest 5 - singular value\n";

   // Singular value decomposition method
 
   // load data - 1s into col 1 of matrix
   int npred1 = npred+1;
   Matrix X(nobs,npred1); ColumnVector Y(nobs);
   X.column(1) = 1.0;  X.column(2) << x1;  X.column(3) << x2;  Y << y;

   // do SVD
   Matrix U, V; DiagonalMatrix D;
   SVD(X,D,U,V);                              // X = U * D * V.t()
   ColumnVector Fitted = U.t() * Y;
   ColumnVector A = V * ( D.i() * Fitted );
   Fitted = U * Fitted;
   ColumnVector Residual = Y - Fitted;
   Real ResVar = sum_square(Residual) / (nobs-npred1);

   // get variances of estimates
   D << V * (D * D).i() * V.t();

   // Get diagonals of Hat matrix
   DiagonalMatrix Hat;  Hat << U * U.t();

   // print out answers
   cout << "\nEstimates and their standard errors\n\n";
   ColumnVector SE(npred1);
   for (int i=1; i<=npred1; i++) SE(i) = sqrt(D(i)*ResVar);
   cout << setw(11) << setprecision(5) << (A | SE) << endl;
   cout << "\nObservations, fitted value, residual value, hat value\n";
   cout << setw(9) << setprecision(3) << 
      (X.columns(2,3) | Y | Fitted | Residual | Hat.as_column());
   cout << "\n\n";
}

int main()
{
   cout << "\nDemonstration of Matrix package\n";
   cout << "\nPrint a real number (may help lost memory test): " << 3.14159265 << "\n";

   // Test for any memory not deallocated after running this program
   Real* s1; { ColumnVector A(8000); s1 = A.data(); }

   {
      // the data

      Real y[]  = { 8.3, 5.5, 8.0, 8.5, 5.7, 4.4, 6.3, 7.9, 9.1 };
      Real x1[] = { 2.4, 1.8, 2.4, 3.0, 2.0, 1.2, 2.0, 2.7, 3.6 };
      Real x2[] = { 1.7, 0.9, 1.6, 1.9, 0.5, 0.6, 1.1, 1.0, 0.5 };


      int nobs = 9;                           // number of observations
      int npred = 2;                          // number of predictor values

      // we want to find the values of a,a1,a2 to give the best
      // fit of y[i] with a0 + a1*x1[i] + a2*x2[i]
      // Also print diagonal elements of hat matrix, X*(X.t()*X).i()*X.t()

      // this example demonstrates five methods of calculation

      Try
      {
         test1(y, x1, x2, nobs, npred);
         test2(y, x1, x2, nobs, npred);
         test3(y, x1, x2, nobs, npred);
         test4(y, x1, x2, nobs, npred);
         test5(y, x1, x2, nobs, npred);
      }
      CatchAll { cout << BaseException::what(); }
   }

#ifdef DO_FREE_CHECK
   FreeCheck::Status();
#endif
   Real* s2; { ColumnVector A(8000); s2 = A.data(); }
   cout << "\n\nThe following test does not work with all compilers - see documentation\n";
   cout << "Checking for lost memory: "
      << (unsigned long)s1 << " " << (unsigned long)s2 << " ";
   if (s1 != s2) cout << " - see section 2.8\n"; else cout << " - ok\n";

   return 0;

}

