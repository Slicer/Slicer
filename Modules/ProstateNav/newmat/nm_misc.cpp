//$$ nm_misc.cpp                          Miscellaneous programs

#define WANT_MATH

#include "include.h"

#include "newmatap.h"

#ifdef use_namespace
namespace NEWMAT {
#endif


#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,21); ++ExeCount; }
#else
#define REPORT {}
#endif

ReturnMatrix Helmert(int n, bool full)
{
   REPORT
   Tracer et("Helmert ");
   if (n <= 0) Throw(ProgramException("Dimension <= 0 "));
   Matrix H;
   
   if (full) H.resize(n,n); else H.resize(n-1,n);
   H = 0.0;
   for (int i = 1; i < n; ++i)
   {
      Real f = 1.0 / sqrt((Real)i * (i+1));
      H.submatrix(i,i,1,i) = -f; H(i,i+1) = f * i;
   }
   if (full) { H.row(n) = 1.0 / sqrt((Real)n); }
   H.release(); return H.for_return();
} 



// Multiply X by n-1 x n matrix to give n-1 contrasts
// Return a ColumnVector
ReturnMatrix Helmert(const ColumnVector& X, bool full)
{
   REPORT
   Tracer et("Helmert * CV");
   int n = X.nrows();
   if (n == 0) Throw(ProgramException("X Vector of length 0", X));
   Real sum = 0.0; ColumnVector Y;
   if (full) Y.resize(n); else Y.resize(n-1);
   for (int i = 1; i < n; ++i)
      { sum += X(i); Y(i) = (i * X(i+1) - sum) / sqrt((Real)i * (i+1)); }
   if (full) { sum += X(n); Y(n) = sum / sqrt((Real)n); }
   Y.release(); return Y.for_return();
} 

// same as above for X a ColumnVector, length n, element j = 1; otherwise 0
ReturnMatrix Helmert(int n, int j, bool full)
{
   REPORT
   Tracer et("Helmert:single element ");
   if (n <= 0) Throw(ProgramException("X Vector of length <= 0"));
   if (j > n || j <= 0)
      Throw(ProgramException("Out of range element number "));
   ColumnVector Y; if (full) Y.resize(n); else Y.resize(n-1);
   Y = 0.0;
   if (j > 1) Y(j-1) = sqrt((Real)(j-1) / (Real)j);
   for (int i = j; i < n; ++i) Y(i) = - 1.0 / sqrt((Real)i * (i+1));
   if (full) Y(n) = 1.0 / sqrt((Real)n);
   Y.release(); return Y.for_return();
} 

ReturnMatrix Helmert_transpose(const ColumnVector& Y, bool full)
{
   REPORT
   Tracer et("Helmert_transpose * CV ");
   int n = Y.nrows(); Real sum;
   if (full) sum = Y(n) / sqrt((Real)n); else { sum = 0.0; ++n; }
   ColumnVector X(n);
   for (int i = n-1; i > 0; --i)
   {
      Real Yi = Y(i) / sqrt((Real)i * (i+1));
      X(i+1) = i * Yi + sum; sum -= Yi;
   }
   X(1) = sum;
   X.release(); return X.for_return();
}

// same as above but want only j-th element
Real Helmert_transpose(const ColumnVector& Y, int j, bool full)
{
   REPORT
   Tracer et("Helmert_transpose:single element ");
   int n = Y.nrows(); Real sum;
   if (full) sum = Y(n) / sqrt((Real)n); else { sum = 0.0; ++n; }
   if (j > n || j <= 0) Throw(IndexException(j, Y));
   for (int i = n-1; i > 0; --i)
   {
      Real Yi = Y(i) / sqrt((Real)i * (i+1));
      if (i+1 == j) return i * Yi + sum;
      sum -= Yi;
   }
   return sum;
}

ReturnMatrix Helmert(const Matrix& X, bool full)
{
   REPORT
   Tracer et("Helmert * Matrix");
   int m = X.nrows(); int n = X.ncols();
   if (m == 0) Throw(ProgramException("Matrix has 0 rows ", X));
   Matrix Y;
   if (full) Y.resize(m,n); else Y.resize(m-1, n);
   for (int j = 1; j <= n; ++j)
   {
      ColumnVector CV = X.Column(j);
      Y.Column(j) = Helmert(CV, full);
   }
   Y.release(); return Y.for_return();
}

ReturnMatrix Helmert_transpose(const Matrix& Y, bool full)
{
   REPORT
   Tracer et("Helmert_transpose * Matrix ");
   int m = Y.nrows(); int n = Y.ncols(); if (!full) ++m;
   Matrix X(m, n);
   for (int j = 1; j <= n; ++j)
   {
      ColumnVector CV = Y.Column(j);
      X.Column(j) = Helmert_transpose(CV, full);
   }
   X.release(); return X.for_return();
}




#ifdef use_namespace
}
#endif
