//$$ cholesky.cpp                     cholesky decomposition

// Copyright (C) 1991,2,3,4: R B Davies

#define WANT_MATH
//#define WANT_STREAM

#include "include.h"

#include "newmat.h"
#include "newmatrm.h"

#ifdef use_namespace
namespace NEWMAT {
#endif

#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,14); ++ExeCount; }
#else
#define REPORT {}
#endif

/********* Cholesky decomposition of a positive definite matrix *************/

// Suppose S is symmetrix and positive definite. Then there exists a unique
// lower triangular matrix L such that L L.t() = S;


ReturnMatrix Cholesky(const SymmetricMatrix& S)
{
   REPORT
   Tracer trace("Cholesky");
   int nr = S.Nrows();
   LowerTriangularMatrix T(nr);
   Real* s = S.Store(); Real* t = T.Store(); Real* ti = t;
   for (int i=0; i<nr; i++)
   {
      Real* tj = t; Real sum; int k;
      for (int j=0; j<i; j++)
      {
         Real* tk = ti; sum = 0.0; k = j;
         while (k--) { sum += *tj++ * *tk++; }
         *tk = (*s++ - sum) / *tj++;
      }
      sum = 0.0; k = i;
      while (k--) { sum += square(*ti++); }
      Real d = *s++ - sum;
      if (d<=0.0)  Throw(NPDException(S));
      *ti++ = sqrt(d);
   }
   T.release(); return T.for_return();
}

ReturnMatrix Cholesky(const SymmetricBandMatrix& S)
{
   REPORT
   Tracer trace("Band-Cholesky");
   int nr = S.Nrows(); int m = S.lower_val;
   LowerBandMatrix T(nr,m);
   Real* s = S.Store(); Real* t = T.Store(); Real* ti = t;

   for (int i=0; i<nr; i++)
   {
      Real* tj = t; Real sum; int l;
      if (i<m) { REPORT l = m-i; s += l; ti += l; l = i; }
      else { REPORT t += (m+1); l = m; }

      for (int j=0; j<l; j++)
      {
         Real* tk = ti; sum = 0.0; int k = j; tj += (m-j);
         while (k--) { sum += *tj++ * *tk++; }
         *tk = (*s++ - sum) / *tj++;
      }
      sum = 0.0;
      while (l--) { sum += square(*ti++); }
      Real d = *s++ - sum;
      if (d<=0.0)  Throw(NPDException(S));
      *ti++ = sqrt(d);
   }

   T.release(); return T.for_return();
}




// Contributed by Nick Bennett of Schlumberger-Doll Research; modified by RBD

// The enclosed routines can be used to update the Cholesky decomposition of
// a positive definite symmetric matrix.  A good reference for this routines
// can be found in
// LINPACK User's Guide, Chapter 10, Dongarra et. al., SIAM, Philadelphia, 1979

// produces the Cholesky decomposition of A + x.t() * x where A = chol.t() * chol
void update_Cholesky(UpperTriangularMatrix &chol, RowVector x)
{
   int nc = chol.Nrows();
   ColumnVector cGivens(nc); cGivens = 0.0;
   ColumnVector sGivens(nc); sGivens = 0.0;
        
   for(int j = 1; j <= nc; ++j) // process the jth column of chol
   {
      // apply the previous Givens rotations k = 1,...,j-1 to column j
      for(int k = 1; k < j; ++k)
         GivensRotation(cGivens(k), sGivens(k), chol(k,j), x(j));

      // determine the jth Given's rotation
      pythag(chol(j,j), x(j), cGivens(j), sGivens(j));

      // apply the jth Given's rotation
      {
         Real tmp0 = cGivens(j) * chol(j,j) + sGivens(j) * x(j);
         chol(j,j) = tmp0; x(j) = 0.0;
      }

   }

}


// produces the Cholesky decomposition of A - x.t() * x where A = chol.t() * chol
void downdate_Cholesky(UpperTriangularMatrix &chol, RowVector x)
{
   int nRC = chol.Nrows();
        
   // solve R^T a = x
   LowerTriangularMatrix L = chol.t();
   ColumnVector a(nRC); a = 0.0;
   int i, j;
        
   for (i = 1; i <= nRC; ++i)
   {
      // accumulate subtr sum
      Real subtrsum = 0.0;
      for(int k = 1; k < i; ++k) subtrsum += a(k) * L(i,k);

      a(i) = (x(i) - subtrsum) / L(i,i);
   }

   // test that l2 norm of a is < 1
   Real squareNormA = a.SumSquare();
   if (squareNormA >= 1.0)
      Throw(ProgramException("downdate_Cholesky() fails", chol));

   Real alpha = sqrt(1.0 - squareNormA);

   // compute and apply Givens rotations to the vector a
   ColumnVector cGivens(nRC);  cGivens = 0.0;
   ColumnVector sGivens(nRC);  sGivens = 0.0;
   for(i = nRC; i >= 1; i--)
      alpha = pythag(alpha, a(i), cGivens(i), sGivens(i));

   // apply Givens rotations to the jth column of chol
   ColumnVector xtilde(nRC); xtilde = 0.0;
   for(j = nRC; j >= 1; j--)
   {
      // only the first j rotations have an affect on chol,0
      for(int k = j; k >= 1; k--)
         GivensRotation(cGivens(k), -sGivens(k), chol(k,j), xtilde(j));
   }
}



// produces the Cholesky decomposition of EAE where A = chol.t() * chol
// and E produces a RIGHT circular shift of the rows and columns from
// 1,...,k-1,k,k+1,...l,l+1,...,p to
// 1,...,k-1,l,k,k+1,...l-1,l+1,...p
void right_circular_update_Cholesky(UpperTriangularMatrix &chol, int k, int l)
{
   int nRC = chol.Nrows();
   int i, j;
        
   // I. compute shift of column l to the kth position
   Matrix cholCopy = chol;
   // a. grab column l
   ColumnVector columnL = cholCopy.Column(l);
   // b. shift columns k,...l-1 to the RIGHT
   for(j = l-1; j >= k; --j)
      cholCopy.Column(j+1) = cholCopy.Column(j);
   // c. copy the top k-1 elements of columnL into the kth column of cholCopy
   cholCopy.Column(k) = 0.0;
   for(i = 1; i < k; ++i) cholCopy(i,k) = columnL(i);

    // II. determine the l-k Given's rotations
   int nGivens = l-k;
   ColumnVector cGivens(nGivens); cGivens = 0.0;
   ColumnVector sGivens(nGivens); sGivens = 0.0;
   for(i = l; i > k; i--)
   {
      int givensIndex = l-i+1;
      columnL(i-1) = pythag(columnL(i-1), columnL(i),
         cGivens(givensIndex), sGivens(givensIndex));
      columnL(i) = 0.0;
   }
   // the kth entry of columnL is the new diagonal element in column k of cholCopy
   cholCopy(k,k) = columnL(k);
        
   // III. apply these Given's rotations to subsequent columns
   // for columns k+1,...,l-1 we only need to apply the last nGivens-(j-k) rotations
   for(j = k+1; j <= nRC; ++j)
   {
      ColumnVector columnJ = cholCopy.Column(j);
      int imin = nGivens - (j-k) + 1; if (imin < 1) imin = 1;
      for(int gIndex = imin; gIndex <= nGivens; ++gIndex)
      {
         // apply gIndex Given's rotation
         int topRowIndex = k + nGivens - gIndex;
         GivensRotationR(cGivens(gIndex), sGivens(gIndex),
            columnJ(topRowIndex), columnJ(topRowIndex+1));
      }
      cholCopy.Column(j) = columnJ;
   }

   chol << cholCopy;
}



// produces the Cholesky decomposition of EAE where A = chol.t() * chol
// and E produces a LEFT circular shift of the rows and columns from
// 1,...,k-1,k,k+1,...l,l+1,...,p to
// 1,...,k-1,k+1,...l,k,l+1,...,p to
void left_circular_update_Cholesky(UpperTriangularMatrix &chol, int k, int l)
{
   int nRC = chol.Nrows();
   int i, j;

   // I. compute shift of column k to the lth position
   Matrix cholCopy = chol;
   // a. grab column k
   ColumnVector columnK = cholCopy.Column(k);
   // b. shift columns k+1,...l to the LEFT
   for(j = k+1; j <= l; ++j)
      cholCopy.Column(j-1) = cholCopy.Column(j);
   // c. copy the elements of columnK into the lth column of cholCopy
   cholCopy.Column(l) = 0.0;
   for(i = 1; i <= k; ++i)
      cholCopy(i,l) = columnK(i);

   // II. apply and compute Given's rotations
   int nGivens = l-k;
   ColumnVector cGivens(nGivens); cGivens = 0.0;
   ColumnVector sGivens(nGivens); sGivens = 0.0;
   for(j = k; j <= nRC; ++j)
   {
      ColumnVector columnJ = cholCopy.Column(j);

      // apply the previous Givens rotations to columnJ
      int imax = j - k; if (imax > nGivens) imax = nGivens;
      for(int i = 1; i <= imax; ++i)
      {
         int gIndex = i;
         int topRowIndex = k + i - 1;
         GivensRotationR(cGivens(gIndex), sGivens(gIndex),
            columnJ(topRowIndex), columnJ(topRowIndex+1));
      }

      // compute a new Given's rotation when j < l
      if(j < l)
      {
         int gIndex = j-k+1;
         columnJ(j) = pythag(columnJ(j), columnJ(j+1), cGivens(gIndex),
            sGivens(gIndex));
         columnJ(j+1) = 0.0;
      }

      cholCopy.Column(j) = columnJ;
   }

   chol << cholCopy;
        
}




#ifdef use_namespace
}
#endif

