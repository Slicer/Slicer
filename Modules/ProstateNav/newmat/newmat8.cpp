//$$ newmat8.cpp         Advanced LU transform, scalar functions

// Copyright (C) 1991,2,3,4,8: R B Davies

#define WANT_MATH

#include "include.h"

#include "newmat.h"
#include "newmatrc.h"
#include "precisio.h"

#ifdef use_namespace
namespace NEWMAT {
#endif


#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,8); ++ExeCount; }
#else
#define REPORT {}
#endif


/************************** LU transformation ****************************/

void CroutMatrix::ludcmp()
// LU decomposition from Golub & Van Loan, algorithm 3.4.1, (the "outer
// product" version).
// This replaces the code derived from Numerical Recipes in C in previous
// versions of newmat and being row oriented runs much faster with large
// matrices.
{
   REPORT
   Tracer tr( "Crout(ludcmp)" ); sing = false;
   Real* akk = store;                    // runs down diagonal

   Real big = fabs(*akk); int mu = 0; Real* ai = akk; int k;

   for (k = 1; k < nrows_val; k++)
   {
      ai += nrows_val; const Real trybig = fabs(*ai);
      if (big < trybig) { big = trybig; mu = k; }
   }


   if (nrows_val) for (k = 0;;)
   {
      /*
      int mu1;
      {
         Real big = fabs(*akk); mu1 = k; Real* ai = akk; int i;

         for (i = k+1; i < nrows_val; i++)
         {
            ai += nrows_val; const Real trybig = fabs(*ai);
            if (big < trybig) { big = trybig; mu1 = i; }
         }
      }
      if (mu1 != mu) cout << k << " " << mu << " " << mu1 << endl;
      */

      indx[k] = mu;

      if (mu != k)                       //row swap
      {
         Real* a1 = store + nrows_val * k;
         Real* a2 = store + nrows_val * mu; d = !d;
         int j = nrows_val;
         while (j--) { const Real temp = *a1; *a1++ = *a2; *a2++ = temp; }
      }

      Real diag = *akk; big = 0; mu = k + 1;
      if (diag != 0)
      {
         ai = akk; int i = nrows_val - k - 1;
         while (i--)
         {
            ai += nrows_val; Real* al = ai;
            Real mult = *al / diag; *al = mult;
            int l = nrows_val - k - 1; Real* aj = akk;
            // work out the next pivot as part of this loop
            // this saves a column operation
            if (l-- != 0)
            {
               *(++al) -= (mult * *(++aj));
               const Real trybig = fabs(*al);
               if (big < trybig) { big = trybig; mu = nrows_val - i - 1; }
               while (l--) *(++al) -= (mult * *(++aj));
            }
         }
      }
      else sing = true;
      if (++k == nrows_val) break;          // so next line won't overflow
      akk += nrows_val + 1;
   }
}

void CroutMatrix::lubksb(Real* B, int mini)
{
   REPORT
   // this has been adapted from Numerical Recipes in C. The code has been
   // substantially streamlined, so I do not think much of the original
   // copyright remains. However there is not much opportunity for
   // variation in the code, so it is still similar to the NR code.
   // I follow the NR code in skipping over initial zeros in the B vector.

   Tracer tr("Crout(lubksb)");
   if (sing) Throw(SingularException(*this));
   int i, j, ii = nrows_val;       // ii initialised : B might be all zeros


   // scan for first non-zero in B
   for (i = 0; i < nrows_val; i++)
   {
      int ip = indx[i]; Real temp = B[ip]; B[ip] = B[i]; B[i] = temp;
      if (temp != 0.0) { ii = i; break; }
   }

   Real* bi; Real* ai;
   i = ii + 1;

   if (i < nrows_val)
   {
      bi = B + ii; ai = store + ii + i * nrows_val;
      for (;;)
      {
         int ip = indx[i]; Real sum = B[ip]; B[ip] = B[i];
         Real* aij = ai; Real* bj = bi; j = i - ii;
         while (j--) sum -= *aij++ * *bj++;
         B[i] = sum;
         if (++i == nrows_val) break;
         ai += nrows_val;
      }
   }

   ai = store + nrows_val * nrows_val;

   for (i = nrows_val - 1; i >= mini; i--)
   {
      Real* bj = B+i; ai -= nrows_val; Real* ajx = ai+i;
      Real sum = *bj; Real diag = *ajx;
      j = nrows_val - i; while(--j) sum -= *(++ajx) * *(++bj);
      B[i] = sum / diag;
   }
}

/****************************** scalar functions ****************************/

inline Real square(Real x) { return x*x; }

Real GeneralMatrix::sum_square() const
{
   REPORT
   Real sum = 0.0; int i = storage; Real* s = store;
   while (i--) sum += square(*s++);
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

Real GeneralMatrix::sum_absolute_value() const
{
   REPORT
   Real sum = 0.0; int i = storage; Real* s = store;
   while (i--) sum += fabs(*s++);
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

Real GeneralMatrix::sum() const
{
   REPORT
   Real sm = 0.0; int i = storage; Real* s = store;
   while (i--) sm += *s++;
   ((GeneralMatrix&)*this).tDelete(); return sm;
}

// maxima and minima

// There are three sets of routines
// maximum_absolute_value, minimum_absolute_value, maximum, minimum
// ... these find just the maxima and minima
// maximum_absolute_value1, minimum_absolute_value1, maximum1, minimum1
// ... these find the maxima and minima and their locations in a
//     one dimensional object
// maximum_absolute_value2, minimum_absolute_value2, maximum2, minimum2
// ... these find the maxima and minima and their locations in a
//     two dimensional object

// If the matrix has no values throw an exception

// If we do not want the location find the maximum or minimum on the
// array stored by GeneralMatrix
// This won't work for BandMatrices. We call ClearCorner for
// maximum_absolute_value but for the others use the absolute_minimum_value2
// version and discard the location.

// For one dimensional objects, when we want the location of the
// maximum or minimum, work with the array stored by GeneralMatrix

// For two dimensional objects where we want the location of the maximum or
// minimum proceed as follows:

// For rectangular matrices use the array stored by GeneralMatrix and
// deduce the location from the location in the GeneralMatrix

// For other two dimensional matrices use the Matrix Row routine to find the
// maximum or minimum for each row.

static void NullMatrixError(const GeneralMatrix* gm)
{
   ((GeneralMatrix&)*gm).tDelete();
   Throw(ProgramException("Maximum or minimum of null matrix"));
}

Real GeneralMatrix::maximum_absolute_value() const
{
   REPORT
   if (storage == 0) NullMatrixError(this);
   Real maxval = 0.0; int l = storage; Real* s = store;
   while (l--) { Real a = fabs(*s++); if (maxval < a) maxval = a; }
   ((GeneralMatrix&)*this).tDelete(); return maxval;
}

Real GeneralMatrix::maximum_absolute_value1(int& i) const
{
   REPORT
   if (storage == 0) NullMatrixError(this);
   Real maxval = 0.0; int l = storage; Real* s = store; int li = storage;
   while (l--)
      { Real a = fabs(*s++); if (maxval <= a) { maxval = a; li = l; }  }
   i = storage - li;
   ((GeneralMatrix&)*this).tDelete(); return maxval;
}

Real GeneralMatrix::minimum_absolute_value() const
{
   REPORT
   if (storage == 0) NullMatrixError(this);
   int l = storage - 1; Real* s = store; Real minval = fabs(*s++);
   while (l--) { Real a = fabs(*s++); if (minval > a) minval = a; }
   ((GeneralMatrix&)*this).tDelete(); return minval;
}

Real GeneralMatrix::minimum_absolute_value1(int& i) const
{
   REPORT
   if (storage == 0) NullMatrixError(this);
   int l = storage - 1; Real* s = store; Real minval = fabs(*s++); int li = l;
   while (l--)
      { Real a = fabs(*s++); if (minval >= a) { minval = a; li = l; }  }
   i = storage - li;
   ((GeneralMatrix&)*this).tDelete(); return minval;
}

Real GeneralMatrix::maximum() const
{
   REPORT
   if (storage == 0) NullMatrixError(this);
   int l = storage - 1; Real* s = store; Real maxval = *s++;
   while (l--) { Real a = *s++; if (maxval < a) maxval = a; }
   ((GeneralMatrix&)*this).tDelete(); return maxval;
}

Real GeneralMatrix::maximum1(int& i) const
{
   REPORT
   if (storage == 0) NullMatrixError(this);
   int l = storage - 1; Real* s = store; Real maxval = *s++; int li = l;
   while (l--) { Real a = *s++; if (maxval <= a) { maxval = a; li = l; } }
   i = storage - li;
   ((GeneralMatrix&)*this).tDelete(); return maxval;
}

Real GeneralMatrix::minimum() const
{
   REPORT
   if (storage == 0) NullMatrixError(this);
   int l = storage - 1; Real* s = store; Real minval = *s++;
   while (l--) { Real a = *s++; if (minval > a) minval = a; }
   ((GeneralMatrix&)*this).tDelete(); return minval;
}

Real GeneralMatrix::minimum1(int& i) const
{
   REPORT
   if (storage == 0) NullMatrixError(this);
   int l = storage - 1; Real* s = store; Real minval = *s++; int li = l;
   while (l--) { Real a = *s++; if (minval >= a) { minval = a; li = l; } }
   i = storage - li;
   ((GeneralMatrix&)*this).tDelete(); return minval;
}

Real GeneralMatrix::maximum_absolute_value2(int& i, int& j) const
{
   REPORT
   if (storage == 0) NullMatrixError(this);
   Real maxval = 0.0; int nr = Nrows();
   MatrixRow mr((GeneralMatrix*)this, LoadOnEntry+DirectPart);
   for (int r = 1; r <= nr; r++)
   {
      int c; maxval = mr.MaximumAbsoluteValue1(maxval, c);
      if (c > 0) { i = r; j = c; }
      mr.Next();
   }
   ((GeneralMatrix&)*this).tDelete(); return maxval;
}

Real GeneralMatrix::minimum_absolute_value2(int& i, int& j) const
{
   REPORT
   if (storage == 0)  NullMatrixError(this);
   Real minval = FloatingPointPrecision::Maximum(); int nr = Nrows();
   MatrixRow mr((GeneralMatrix*)this, LoadOnEntry+DirectPart);
   for (int r = 1; r <= nr; r++)
   {
      int c; minval = mr.MinimumAbsoluteValue1(minval, c);
      if (c > 0) { i = r; j = c; }
      mr.Next();
   }
   ((GeneralMatrix&)*this).tDelete(); return minval;
}

Real GeneralMatrix::maximum2(int& i, int& j) const
{
   REPORT
   if (storage == 0) NullMatrixError(this);
   Real maxval = -FloatingPointPrecision::Maximum(); int nr = Nrows();
   MatrixRow mr((GeneralMatrix*)this, LoadOnEntry+DirectPart);
   for (int r = 1; r <= nr; r++)
   {
      int c; maxval = mr.Maximum1(maxval, c);
      if (c > 0) { i = r; j = c; }
      mr.Next();
   }
   ((GeneralMatrix&)*this).tDelete(); return maxval;
}

Real GeneralMatrix::minimum2(int& i, int& j) const
{
   REPORT
   if (storage == 0) NullMatrixError(this);
   Real minval = FloatingPointPrecision::Maximum(); int nr = Nrows();
   MatrixRow mr((GeneralMatrix*)this, LoadOnEntry+DirectPart);
   for (int r = 1; r <= nr; r++)
   {
      int c; minval = mr.Minimum1(minval, c);
      if (c > 0) { i = r; j = c; }
      mr.Next();
   }
   ((GeneralMatrix&)*this).tDelete(); return minval;
}

Real Matrix::maximum_absolute_value2(int& i, int& j) const
{
   REPORT
   int k; Real m = GeneralMatrix::maximum_absolute_value1(k); k--;
   i = k / Ncols(); j = k - i * Ncols(); i++; j++;
   return m;
}

Real Matrix::minimum_absolute_value2(int& i, int& j) const
{
   REPORT
   int k; Real m = GeneralMatrix::minimum_absolute_value1(k); k--;
   i = k / Ncols(); j = k - i * Ncols(); i++; j++;
   return m;
}

Real Matrix::maximum2(int& i, int& j) const
{
   REPORT
   int k; Real m = GeneralMatrix::maximum1(k); k--;
   i = k / Ncols(); j = k - i * Ncols(); i++; j++;
   return m;
}

Real Matrix::minimum2(int& i, int& j) const
{
   REPORT
   int k; Real m = GeneralMatrix::minimum1(k); k--;
   i = k / Ncols(); j = k - i * Ncols(); i++; j++;
   return m;
}

Real SymmetricMatrix::sum_square() const
{
   REPORT
   Real sum1 = 0.0; Real sum2 = 0.0; Real* s = store; int nr = nrows_val;
   for (int i = 0; i<nr; i++)
   {
      int j = i;
      while (j--) sum2 += square(*s++);
      sum1 += square(*s++);
   }
   ((GeneralMatrix&)*this).tDelete(); return sum1 + 2.0 * sum2;
}

Real SymmetricMatrix::sum_absolute_value() const
{
   REPORT
   Real sum1 = 0.0; Real sum2 = 0.0; Real* s = store; int nr = nrows_val;
   for (int i = 0; i<nr; i++)
   {
      int j = i;
      while (j--) sum2 += fabs(*s++);
      sum1 += fabs(*s++);
   }
   ((GeneralMatrix&)*this).tDelete(); return sum1 + 2.0 * sum2;
}

Real IdentityMatrix::sum_absolute_value() const
   { REPORT  return fabs(trace()); }    // no need to do tDelete?

Real SymmetricMatrix::sum() const
{
   REPORT
   Real sum1 = 0.0; Real sum2 = 0.0; Real* s = store; int nr = nrows_val;
   for (int i = 0; i<nr; i++)
   {
      int j = i;
      while (j--) sum2 += *s++;
      sum1 += *s++;
   }
   ((GeneralMatrix&)*this).tDelete(); return sum1 + 2.0 * sum2;
}

Real IdentityMatrix::sum_square() const
{
   Real sum = *store * *store * nrows_val;
   ((GeneralMatrix&)*this).tDelete(); return sum;
}


Real BaseMatrix::sum_square() const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->sum_square(); return s;
}

Real BaseMatrix::norm_Frobenius() const
   { REPORT  return sqrt(sum_square()); }

Real BaseMatrix::sum_absolute_value() const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->sum_absolute_value(); return s;
}

Real BaseMatrix::sum() const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->sum(); return s;
}

Real BaseMatrix::maximum_absolute_value() const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->maximum_absolute_value(); return s;
}

Real BaseMatrix::maximum_absolute_value1(int& i) const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->maximum_absolute_value1(i); return s;
}

Real BaseMatrix::maximum_absolute_value2(int& i, int& j) const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->maximum_absolute_value2(i, j); return s;
}

Real BaseMatrix::minimum_absolute_value() const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->minimum_absolute_value(); return s;
}

Real BaseMatrix::minimum_absolute_value1(int& i) const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->minimum_absolute_value1(i); return s;
}

Real BaseMatrix::minimum_absolute_value2(int& i, int& j) const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->minimum_absolute_value2(i, j); return s;
}

Real BaseMatrix::maximum() const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->maximum(); return s;
}

Real BaseMatrix::maximum1(int& i) const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->maximum1(i); return s;
}

Real BaseMatrix::maximum2(int& i, int& j) const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->maximum2(i, j); return s;
}

Real BaseMatrix::minimum() const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->minimum(); return s;
}

Real BaseMatrix::minimum1(int& i) const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->minimum1(i); return s;
}

Real BaseMatrix::minimum2(int& i, int& j) const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   Real s = gm->minimum2(i, j); return s;
}

Real dotproduct(const Matrix& A, const Matrix& B)
{
   REPORT
   int n = A.storage;
   if (n != B.storage)
   {
      Tracer tr("dotproduct");
      Throw(IncompatibleDimensionsException(A,B));
   }
   Real sum = 0.0; Real* a = A.store; Real* b = B.store;
   while (n--) sum += *a++ * *b++;
   return sum;
}

Real Matrix::trace() const
{
   REPORT
   Tracer tr("trace");
   int i = nrows_val; int d = i+1;
   if (i != ncols_val) Throw(NotSquareException(*this));
   Real sum = 0.0; Real* s = store;
//   while (i--) { sum += *s; s += d; }
   if (i) for (;;) { sum += *s; if (!(--i)) break; s += d; }
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

Real DiagonalMatrix::trace() const
{
   REPORT
   int i = nrows_val; Real sum = 0.0; Real* s = store;
   while (i--) sum += *s++;
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

Real SymmetricMatrix::trace() const
{
   REPORT
   int i = nrows_val; Real sum = 0.0; Real* s = store; int j = 2;
   // while (i--) { sum += *s; s += j++; }
   if (i) for (;;) { sum += *s; if (!(--i)) break; s += j++; }
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

Real LowerTriangularMatrix::trace() const
{
   REPORT
   int i = nrows_val; Real sum = 0.0; Real* s = store; int j = 2;
   // while (i--) { sum += *s; s += j++; }
   if (i) for (;;) { sum += *s; if (!(--i)) break; s += j++; }
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

Real UpperTriangularMatrix::trace() const
{
   REPORT
   int i = nrows_val; Real sum = 0.0; Real* s = store;
   while (i) { sum += *s; s += i--; }             // won t cause a problem
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

Real BandMatrix::trace() const
{
   REPORT
   int i = nrows_val; int w = lower_val+upper_val+1;
   Real sum = 0.0; Real* s = store+lower_val;
   // while (i--) { sum += *s; s += w; }
   if (i) for (;;) { sum += *s; if (!(--i)) break; s += w; }
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

Real SymmetricBandMatrix::trace() const
{
   REPORT
   int i = nrows_val; int w = lower_val+1;
   Real sum = 0.0; Real* s = store+lower_val;
   // while (i--) { sum += *s; s += w; }
   if (i) for (;;) { sum += *s; if (!(--i)) break; s += w; }
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

Real IdentityMatrix::trace() const
{
   Real sum = *store * nrows_val;
   ((GeneralMatrix&)*this).tDelete(); return sum;
}


Real BaseMatrix::trace() const
{
   REPORT
   MatrixType Diag = MatrixType::Dg; Diag.SetDataLossOK();
   GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate(Diag);
   Real sum = gm->trace(); return sum;
}

void LogAndSign::operator*=(Real x)
{
   if (x > 0.0) { log_val += log(x); }
   else if (x < 0.0) { log_val += log(-x); sign_val = -sign_val; }
   else sign_val = 0;
}

void LogAndSign::pow_eq(int k)
{
   if (sign_val)
   {
      log_val *= k;
      if ( (k & 1) == 0 ) sign_val = 1;
   }
}

Real LogAndSign::value() const
{
   Tracer et("LogAndSign::value");
   if (log_val >= FloatingPointPrecision::LnMaximum())
      Throw(OverflowException("Overflow in exponential"));
   return sign_val * exp(log_val);
}

LogAndSign::LogAndSign(Real f)
{
   if (f == 0.0) { log_val = 0.0; sign_val = 0; return; }
   else if (f < 0.0) { sign_val = -1; f = -f; }
   else sign_val = 1;
   log_val = log(f);
}

LogAndSign DiagonalMatrix::log_determinant() const
{
   REPORT
   int i = nrows_val; LogAndSign sum; Real* s = store;
   while (i--) sum *= *s++;
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

LogAndSign LowerTriangularMatrix::log_determinant() const
{
   REPORT
   int i = nrows_val; LogAndSign sum; Real* s = store; int j = 2;
   // while (i--) { sum *= *s; s += j++; }
   if (i) for(;;) { sum *= *s; if (!(--i)) break; s += j++; }
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

LogAndSign UpperTriangularMatrix::log_determinant() const
{
   REPORT
   int i = nrows_val; LogAndSign sum; Real* s = store;
   while (i) { sum *= *s; s += i--; }
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

LogAndSign IdentityMatrix::log_determinant() const
{
   REPORT
   int i = nrows_val; LogAndSign sum;
   if (i > 0) { sum = *store; sum.PowEq(i); }
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

LogAndSign BaseMatrix::log_determinant() const
{
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   LogAndSign sum = gm->log_determinant(); return sum;
}

LogAndSign GeneralMatrix::log_determinant() const
{
   REPORT
   Tracer tr("log_determinant");
   if (nrows_val != ncols_val) Throw(NotSquareException(*this));
   CroutMatrix C(*this); return C.log_determinant();
}

LogAndSign CroutMatrix::log_determinant() const
{
   REPORT
   if (sing) return 0.0;
   int i = nrows_val; int dd = i+1; LogAndSign sum; Real* s = store;
   if (i) for(;;)
   {
      sum *= *s;
      if (!(--i)) break;
      s += dd;
   }
   if (!d) sum.ChangeSign(); return sum;

}

Real BaseMatrix::determinant() const
{
   REPORT
   Tracer tr("determinant");
   REPORT GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   LogAndSign ld = gm->log_determinant();
   return ld.Value();
}

LinearEquationSolver::LinearEquationSolver(const BaseMatrix& bm)
{
   gm = ( ((BaseMatrix&)bm).Evaluate() )->MakeSolver();
   if (gm==&bm) { REPORT  gm = gm->Image(); }
   // want a copy if  *gm is actually bm
   else { REPORT  gm->Protect(); }
}

ReturnMatrix BaseMatrix::sum_square_rows() const
{
   REPORT
   GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   int nr = gm->nrows();
   ColumnVector ssq(nr);
   if (gm->size() == 0) { REPORT ssq = 0.0; }
   else
   {
      MatrixRow mr(gm, LoadOnEntry);
      for (int i = 1; i <= nr; ++i)
      {
         Real sum = 0.0;
         int s = mr.Storage();
         Real* in = mr.Data();
         while (s--) sum += square(*in++);
         ssq(i) = sum;   
         mr.Next();
      }
   }
   gm->tDelete();
   ssq.release(); return ssq.for_return();
}

ReturnMatrix BaseMatrix::sum_square_columns() const
{
   REPORT
   GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   int nr = gm->nrows(); int nc = gm->ncols();
   RowVector ssq(nc); ssq = 0.0;
   if (gm->size() != 0)
   {
      MatrixRow mr(gm, LoadOnEntry);
      for (int i = 1; i <= nr; ++i)
      {
         int s = mr.Storage();
         Real* in = mr.Data(); Real* out = ssq.data() + mr.Skip();
         while (s--) *out++ += square(*in++);
         mr.Next();
      }
   }
   gm->tDelete();
   ssq.release(); return ssq.for_return();
}

ReturnMatrix BaseMatrix::sum_rows() const
{
   REPORT
   GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   int nr = gm->nrows();
   ColumnVector sum_vec(nr);
   if (gm->size() == 0) { REPORT sum_vec = 0.0; }
   else
   {
      MatrixRow mr(gm, LoadOnEntry);
      for (int i = 1; i <= nr; ++i)
      {
         Real sum = 0.0;
         int s = mr.Storage();
         Real* in = mr.Data();
         while (s--) sum += *in++;
         sum_vec(i) = sum;   
         mr.Next();
      }
   }
   gm->tDelete();
   sum_vec.release(); return sum_vec.for_return();
}

ReturnMatrix BaseMatrix::sum_columns() const
{
   REPORT
   GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   int nr = gm->nrows(); int nc = gm->ncols();
   RowVector sum_vec(nc); sum_vec = 0.0;
   if (gm->size() != 0)
   {
      MatrixRow mr(gm, LoadOnEntry);
      for (int i = 1; i <= nr; ++i)
      {
         int s = mr.Storage();
         Real* in = mr.Data(); Real* out = sum_vec.data() + mr.Skip();
         while (s--) *out++ += *in++;
         mr.Next();
      }
   }
   gm->tDelete();
   sum_vec.release(); return sum_vec.for_return();
}


#ifdef use_namespace
}
#endif

