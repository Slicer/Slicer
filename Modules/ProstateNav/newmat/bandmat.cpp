//$$ bandmat.cpp                     Band matrix definitions

// Copyright (C) 1991,2,3,4,9: R B Davies

#define WANT_MATH                    // include.h will get math fns

//#define WANT_STREAM

#include "include.h"

#include "newmat.h"
#include "newmatrc.h"

#ifdef use_namespace
namespace NEWMAT {
#endif



#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,10); ++ExeCount; }
#else
#define REPORT {}
#endif

static inline int my_min(int x, int y) { return x < y ? x : y; }
static inline int my_max(int x, int y) { return x > y ? x : y; }


BandMatrix::BandMatrix(const BaseMatrix& M)
{
   REPORT // CheckConversion(M);
   // MatrixConversionCheck mcc;
   GeneralMatrix* gmx=((BaseMatrix&)M).Evaluate(MatrixType::BM);
   GetMatrix(gmx); CornerClear();
}

void BandMatrix::SetParameters(const GeneralMatrix* gmx)
{
   REPORT
   MatrixBandWidth bw = gmx->bandwidth();
   lower_val = bw.lower_val; upper_val = bw.upper_val;
}

void BandMatrix::resize(int n, int lb, int ub)
{
   REPORT
   Tracer tr("BandMatrix::resize");
   if (lb<0 || ub<0) Throw(ProgramException("Undefined bandwidth"));
   lower_val = (lb<=n) ? lb : n-1; upper_val = (ub<=n) ? ub : n-1;
   GeneralMatrix::resize(n,n,n*(lower_val+1+upper_val)); CornerClear();
}

// SimpleAddOK shows when we can add etc two matrices by a simple vector add
// and when we can add one matrix into another
// *gm must be the same type as *this
// return 0 if simple add is OK
// return 1 if we can add into *gm only
// return 2 if we can add into *this only
// return 3 if we can't add either way
// For SP this will still be valid if we swap 1 and 2

short BandMatrix::SimpleAddOK(const GeneralMatrix* gm)
{
   const BandMatrix* bm = (const BandMatrix*)gm;
   if (bm->lower_val == lower_val && bm->upper_val == upper_val)
      { REPORT return 0; }
   else if (bm->lower_val >= lower_val && bm->upper_val >= upper_val)
      { REPORT return 1; }
   else if (bm->lower_val <= lower_val && bm->upper_val <= upper_val)
      { REPORT return 2; }
   else { REPORT return 3; }
}

short SymmetricBandMatrix::SimpleAddOK(const GeneralMatrix* gm)
{
   const SymmetricBandMatrix* bm = (const SymmetricBandMatrix*)gm;
   if (bm->lower_val == lower_val) { REPORT return 0; }
   else if (bm->lower_val > lower_val) { REPORT return 1; }
   else { REPORT return 2; }
}

void UpperBandMatrix::resize(int n, int lb, int ub)
{
   REPORT
   if (lb != 0)
   {
      Tracer tr("UpperBandMatrix::resize");
      Throw(ProgramException("UpperBandMatrix with non-zero lower band" ));
   }
   BandMatrix::resize(n, lb, ub);
}

void LowerBandMatrix::resize(int n, int lb, int ub)
{
   REPORT
   if (ub != 0)
   {
      Tracer tr("LowerBandMatrix::resize");
      Throw(ProgramException("LowerBandMatrix with non-zero upper band" ));
   }
   BandMatrix::resize(n, lb, ub);
}

void BandMatrix::resize(const GeneralMatrix& A)
{
   REPORT
   int n = A.Nrows();
   if (n != A.Ncols())
   {
      Tracer tr("BandMatrix::resize(GM)");
      Throw(NotSquareException(*this));
   }
   MatrixBandWidth mbw = A.bandwidth();
   resize(n, mbw.Lower(), mbw.Upper());
}
/*
bool BandMatrix::SameStorageType(const GeneralMatrix& A) const
{
   if (type() != A.type()) { REPORT return false; }
   REPORT
   return bandwidth() == A.bandwidth();
}

void BandMatrix::resizeForAdd(const GeneralMatrix& A, const GeneralMatrix& B)
{
   REPORT
   Tracer tr("BandMatrix::resizeForAdd");
   MatrixBandWidth A_BW = A.bandwidth(); MatrixBandWidth B_BW = B.bandwidth();
   if ((A_BW.Lower() < 0) | (A_BW.Upper() < 0) | (B_BW.Lower() < 0)
      | (A_BW.Upper() < 0))
         Throw(ProgramException("Can't resize to BandMatrix" ));
   // already know A and B are square
   resize(A.Nrows(), my_max(A_BW.Lower(), B_BW.Lower()),
      my_max(A_BW.Upper(), B_BW.Upper()));
}

void BandMatrix::resizeForSP(const GeneralMatrix& A, const GeneralMatrix& B)
{
   REPORT
   Tracer tr("BandMatrix::resizeForSP");
   MatrixBandWidth A_BW = A.bandwidth(); MatrixBandWidth B_BW = B.bandwidth();
   if ((A_BW.Lower() < 0) | (A_BW.Upper() < 0) | (B_BW.Lower() < 0)
      | (A_BW.Upper() < 0))
         Throw(ProgramException("Can't resize to BandMatrix" ));
   // already know A and B are square
   resize(A.Nrows(), my_min(A_BW.Lower(), B_BW.Lower()),
      my_min(A_BW.Upper(), B_BW.Upper()));
}
*/

void BandMatrix::operator=(const BaseMatrix& X)
{
   REPORT // CheckConversion(X);
   // MatrixConversionCheck mcc;
   Eq(X,MatrixType::BM); CornerClear();
}

void BandMatrix::CornerClear() const
{
   // set unused parts of BandMatrix to zero
   REPORT
   int i = lower_val; Real* s = store; int bw = lower_val + 1 + upper_val;
   while (i)
      { int j = i--; Real* sj = s; s += bw; while (j--) *sj++ = 0.0; }
   i = upper_val; s = store + storage;
   while (i)
      { int j = i--; Real* sj = s; s -= bw; while (j--) *(--sj) = 0.0; }
}

MatrixBandWidth MatrixBandWidth::operator+(const MatrixBandWidth& bw) const
{
   REPORT
   int l = bw.lower_val; int u = bw.upper_val;
   l = (lower_val < 0 || l < 0) ? -1 : (lower_val > l) ? lower_val : l;
   u = (upper_val < 0 || u < 0) ? -1 : (upper_val > u) ? upper_val : u;
   return MatrixBandWidth(l,u);
}

MatrixBandWidth MatrixBandWidth::operator*(const MatrixBandWidth& bw) const
{
   REPORT
   int l = bw.lower_val; int u = bw.upper_val;
   l = (lower_val < 0 || l < 0) ? -1 : lower_val+l;
   u = (upper_val < 0 || u < 0) ? -1 : upper_val+u;
   return MatrixBandWidth(l,u);
}

MatrixBandWidth MatrixBandWidth::minimum(const MatrixBandWidth& bw) const
{
   REPORT
   int l = bw.lower_val; int u = bw.upper_val;
   if ((lower_val >= 0) && ( (l < 0) || (l > lower_val) )) l = lower_val;
   if ((upper_val >= 0) && ( (u < 0) || (u > upper_val) )) u = upper_val;
   return MatrixBandWidth(l,u);
}

UpperBandMatrix::UpperBandMatrix(const BaseMatrix& M)
{
   REPORT // CheckConversion(M);
   // MatrixConversionCheck mcc;
   GeneralMatrix* gmx=((BaseMatrix&)M).Evaluate(MatrixType::UB);
   GetMatrix(gmx); CornerClear();
}

void UpperBandMatrix::operator=(const BaseMatrix& X)
{
   REPORT // CheckConversion(X);
   // MatrixConversionCheck mcc;
   Eq(X,MatrixType::UB); CornerClear();
}

LowerBandMatrix::LowerBandMatrix(const BaseMatrix& M)
{
   REPORT // CheckConversion(M);
   // MatrixConversionCheck mcc;
   GeneralMatrix* gmx=((BaseMatrix&)M).Evaluate(MatrixType::LB);
   GetMatrix(gmx); CornerClear();
}

void LowerBandMatrix::operator=(const BaseMatrix& X)
{
   REPORT // CheckConversion(X);
   // MatrixConversionCheck mcc;
   Eq(X,MatrixType::LB); CornerClear();
}

BandLUMatrix::BandLUMatrix(const BaseMatrix& m)
{
   REPORT
   Tracer tr("BandLUMatrix");
   storage2 = 0; store2 = 0; indx = 0; // in event of exception during build
   GeneralMatrix* gm = ((BaseMatrix&)m).Evaluate();
   if (gm->nrows() != gm->ncols())
      { gm->tDelete(); Throw(NotSquareException(*this)); }
   if (gm->type() == MatrixType::BC)
      { REPORT  ((BandLUMatrix*)gm)->get_aux(*this); GetMatrix(gm); }
   else
   {
      REPORT
      BandMatrix* gm1 = (BandMatrix*)(gm->Evaluate(MatrixType::BM));
      m1 = gm1->lower_val; m2 = gm1->upper_val;
      GetMatrix(gm1);
      d = true; sing = false;
      indx = new int [nrows_val]; MatrixErrorNoSpace(indx);
      MONITOR_INT_NEW("Index (BndLUMat)",nrows_val,indx)
      storage2 = nrows_val * m1;
      store2 = new Real [storage2]; MatrixErrorNoSpace(store2);
      MONITOR_REAL_NEW("Make (BandLUMat)",storage2,store2)
      ludcmp();
   }
}

GeneralMatrix* BandLUMatrix::Evaluate(MatrixType mt)
{
   if (Compare(this->Type(),mt)) { REPORT return this; }
   REPORT
   Tracer et("BandLUMatrix::Evaluate");
   bool dummy = true;
   if (dummy) Throw(ProgramException("Illegal use of BandLUMatrix", *this));
   return this;
}

// could we use SetParameters instead of this
void BandLUMatrix::get_aux(BandLUMatrix& X)
{
   X.d = d; X.sing = sing; X.storage2 = storage2; X.m1 = m1; X.m2 = m2;   
   if (tag_val == 0 || tag_val == 1) // reuse the array 
   {
      REPORT
      X.indx = indx; indx = 0;
      X.store2 = store2; store2 = 0;
      d = true; sing = true; storage2 = 0; m1 = 0; m2 = 0;
      return;
   }
   else if (nrows_val == 0)
   {
      REPORT
      indx = 0; store2 = 0; storage2 = 0;
      d = true; sing = true; m1 = m2 = 0;
      return;
   }
   else                              // copy the array
   {
      REPORT
      Tracer tr("BandLUMatrix::get_aux");
      int *ix = new int [nrows_val]; MatrixErrorNoSpace(ix);
      MONITOR_INT_NEW("Index (BLUM::get_aux)", nrows_val, ix)
      int n = nrows_val; int* i = ix; int* j = indx;
      while(n--) *i++ = *j++;
      X.indx = ix;
      Real *rx = new Real [storage2]; MatrixErrorNoSpace(indx);
      MONITOR_REAL_NEW("Index (BLUM::get_aux)", storage2, rx)
      newmat_block_copy(storage2, store2, rx);
      X.store2 = rx;
   }
}

BandLUMatrix::BandLUMatrix(const BandLUMatrix& gm)
{
   REPORT
   Tracer tr("BandLUMatrix(const BandLUMatrix&)");
   ((BandLUMatrix&)gm).get_aux(*this);
   GetMatrix(&gm);
}

void BandLUMatrix::operator=(const BandLUMatrix& gm)
{
   if (&gm == this) { REPORT tag_val = -1; return; }
   REPORT
   delete [] indx; indx = 0;
   delete [] store2; store2 = 0; storage2 = 0;
   ((BandLUMatrix&)gm).get_aux(*this);
   Eq(gm);
}
   







BandLUMatrix::~BandLUMatrix()
{
   REPORT
   MONITOR_INT_DELETE("Index (BndLUMat)",nrows_val,indx)
   MONITOR_REAL_DELETE("Delete (BndLUMt)",storage2,store2)
   delete [] indx; delete [] store2;
}

MatrixType BandLUMatrix::type() const { REPORT return MatrixType::BC; }


LogAndSign BandLUMatrix::log_determinant() const
{
   REPORT
   if (sing) return 0.0;
   Real* a = store; int w = m1+1+m2; LogAndSign sum; int i = nrows_val;
   // while (i--) { sum *= *a; a += w; }
   if (i) for (;;) { sum *= *a; if (!(--i)) break; a += w; }
   if (!d) sum.ChangeSign(); return sum;
}

GeneralMatrix* BandMatrix::MakeSolver()
{
   REPORT
   GeneralMatrix* gm = new BandLUMatrix(*this);
   MatrixErrorNoSpace(gm); gm->ReleaseAndDelete(); return gm;
}


void BandLUMatrix::ludcmp()
{
   REPORT
   Real* a = store2; int i = storage2;
   // clear store2 - so unused locations are always zero -
   // required by operator==
   while (i--) *a++ = 0.0;
   a = store;
   i = m1; int j = m2; int k; int n = nrows_val; int w = m1 + 1 + m2;
   while (i)
   {
      Real* ai = a + i;
      k = ++j; while (k--) *a++ = *ai++;
      k = i--; while (k--) *a++ = 0.0;
   }

   a = store; int l = m1;
   for (k=0; k<n; k++)
   {
      Real x = *a; i = k; Real* aj = a;
      if (l < n) l++;
      for (j=k+1; j<l; j++)
         { aj += w; if (fabs(x) < fabs(*aj)) { x = *aj; i = j; } }
      indx[k] = i;
      if (x==0) { sing = true; return; }
      if (i!=k)
      {
         d = !d; Real* ak = a; Real* ai = store + i * w; j = w;
         while (j--) { x = *ak; *ak++ = *ai; *ai++ = x; }
      }
      aj = a + w; Real* m = store2 + m1 * k;
      for (j=k+1; j<l; j++)
      {
         *m++ = x = *aj / *a; i = w; Real* ak = a;
         while (--i) { Real* aj1 = aj++; *aj1 = *aj - x * *(++ak); }
         *aj++ = 0.0;
      }
      a += w;
   }
}

void BandLUMatrix::lubksb(Real* B, int mini)
{
   REPORT
   Tracer tr("BandLUMatrix::lubksb");
   if (sing) Throw(SingularException(*this));
   int n = nrows_val; int l = m1; int w = m1 + 1 + m2;

   for (int k=0; k<n; k++)
   {
      int i = indx[k];
      if (i!=k) { Real x=B[k]; B[k]=B[i]; B[i]=x; }
      if (l<n) l++;
      Real* m = store2 + k*m1; Real* b = B+k; Real* bi = b;
      for (i=k+1; i<l; i++)  *(++bi) -= *m++ * *b;
   }

   l = -m1;
   for (int i = n-1; i>=mini; i--)
   {
      Real* b = B + i; Real* bk = b; Real x = *bk;
      Real* a = store + w*i; Real y = *a;
      int k = l+m1; while (k--) x -=  *(++a) * *(++bk);
      *b = x / y;
      if (l < m2) l++;
   }
}

void BandLUMatrix::Solver(MatrixColX& mcout, const MatrixColX& mcin)
{
   REPORT
   int i = mcin.skip; Real* el = mcin.data-i; Real* el1=el;
   while (i--) *el++ = 0.0;
   el += mcin.storage; i = nrows_val - mcin.skip - mcin.storage;
   while (i--) *el++ = 0.0;
   lubksb(el1, mcout.skip);
}

// Do we need check for entirely zero output?


void UpperBandMatrix::Solver(MatrixColX& mcout,
   const MatrixColX& mcin)
{
   REPORT
   int i = mcin.skip-mcout.skip; Real* elx = mcin.data-i;
   while (i-- > 0) *elx++ = 0.0;
   int nr = mcin.skip+mcin.storage;
   elx = mcin.data+mcin.storage; Real* el = elx;
   int j = mcout.skip+mcout.storage-nr; i = nr-mcout.skip;
   while (j-- > 0) *elx++ = 0.0;

   Real* Ael = store + (upper_val+1)*(i-1)+1; j = 0;
   if (i > 0) for(;;)
   {
      elx = el; Real sum = 0.0; int jx = j;
      while (jx--) sum += *(--Ael) * *(--elx);
      elx--; *elx = (*elx - sum) / *(--Ael);
      if (--i <= 0) break;
      if (j<upper_val) Ael -= upper_val - (++j); else el--;
   }
}

void LowerBandMatrix::Solver(MatrixColX& mcout,
   const MatrixColX& mcin)
{
   REPORT
   int i = mcin.skip-mcout.skip; Real* elx = mcin.data-i;
   while (i-- > 0) *elx++ = 0.0;
   int nc = mcin.skip; i = nc+mcin.storage; elx = mcin.data+mcin.storage;
   int nr = mcout.skip+mcout.storage; int j = nr-i; i = nr-nc;
   while (j-- > 0) *elx++ = 0.0;

   Real* el = mcin.data;
   Real* Ael = store + (lower_val+1)*nc + lower_val;
   j = 0;
   if (i > 0) for(;;)
   {
      elx = el; Real sum = 0.0; int jx = j;
      while (jx--) sum += *Ael++ * *elx++;
      *elx = (*elx - sum) / *Ael++;
      if (--i <= 0) break;
      if (j<lower_val) Ael += lower_val - (++j); else el++;
   }
}


LogAndSign BandMatrix::log_determinant() const
{
   REPORT
   BandLUMatrix C(*this); return C.log_determinant();
}

LogAndSign LowerBandMatrix::log_determinant() const
{
   REPORT
   int i = nrows_val; LogAndSign sum;
   Real* s = store + lower_val; int j = lower_val + 1;
//   while (i--) { sum *= *s; s += j; }
   if (i) for (;;) { sum *= *s; if (!(--i)) break; s += j; }
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

LogAndSign UpperBandMatrix::log_determinant() const
{
   REPORT
   int i = nrows_val; LogAndSign sum; Real* s = store; int j = upper_val + 1;
//   while (i--) { sum *= *s; s += j; }
   if (i) for (;;) { sum *= *s; if (!(--i)) break; s += j; }
   ((GeneralMatrix&)*this).tDelete(); return sum;
}

GeneralMatrix* SymmetricBandMatrix::MakeSolver()
{
   REPORT
   GeneralMatrix* gm = new BandLUMatrix(*this);
   MatrixErrorNoSpace(gm); gm->ReleaseAndDelete(); return gm;
}

SymmetricBandMatrix::SymmetricBandMatrix(const BaseMatrix& M)
{
   REPORT  // CheckConversion(M);
   // MatrixConversionCheck mcc;
   GeneralMatrix* gmx=((BaseMatrix&)M).Evaluate(MatrixType::SB);
   GetMatrix(gmx);
}

GeneralMatrix* SymmetricBandMatrix::Transpose(TransposedMatrix*, MatrixType mt)
{ REPORT  return Evaluate(mt); }

LogAndSign SymmetricBandMatrix::log_determinant() const
{
   REPORT
   BandLUMatrix C(*this); return C.log_determinant();
}

void SymmetricBandMatrix::SetParameters(const GeneralMatrix* gmx)
{ REPORT lower_val = gmx->bandwidth().lower_val; }

void SymmetricBandMatrix::resize(int n, int lb)
{
   REPORT
   Tracer tr("SymmetricBandMatrix::resize");
   if (lb<0) Throw(ProgramException("Undefined bandwidth"));
   lower_val = (lb<=n) ? lb : n-1;
   GeneralMatrix::resize(n,n,n*(lower_val+1));
}

void SymmetricBandMatrix::resize(const GeneralMatrix& A)
{
   REPORT
   int n = A.Nrows();
   if (n != A.Ncols())
   {
      Tracer tr("SymmetricBandMatrix::resize(GM)");
      Throw(NotSquareException(*this));
   }
   MatrixBandWidth mbw = A.bandwidth(); int b = mbw.Lower();
   if (b != mbw.Upper())
   {
      Tracer tr("SymmetricBandMatrix::resize(GM)");
      Throw(ProgramException("Upper and lower band-widths not equal"));
   }
   resize(n, b);
}
/*
bool SymmetricBandMatrix::SameStorageType(const GeneralMatrix& A) const
{
   if (type() != A.type()) { REPORT return false; }
   REPORT
   return bandwidth() == A.bandwidth();
}

void SymmetricBandMatrix::resizeForAdd(const GeneralMatrix& A,
   const GeneralMatrix& B)
{
   REPORT
   Tracer tr("SymmetricBandMatrix::resizeForAdd");
   MatrixBandWidth A_BW = A.bandwidth(); MatrixBandWidth B_BW = B.bandwidth();
   if ((A_BW.Lower() < 0) | (B_BW.Lower() < 0))
         Throw(ProgramException("Can't resize to SymmetricBandMatrix" ));
   // already know A and B are square
   resize(A.Nrows(), my_max(A_BW.Lower(), B_BW.Lower()));
}

void SymmetricBandMatrix::resizeForSP(const GeneralMatrix& A,
   const GeneralMatrix& B)
{
   REPORT
   Tracer tr("SymmetricBandMatrix::resizeForSP");
   MatrixBandWidth A_BW = A.bandwidth(); MatrixBandWidth B_BW = B.bandwidth();
   if ((A_BW.Lower() < 0) | (B_BW.Lower() < 0))
         Throw(ProgramException("Can't resize to SymmetricBandMatrix" ));
   // already know A and B are square
   resize(A.Nrows(), my_min(A_BW.Lower(), B_BW.Lower()));
}
*/

void SymmetricBandMatrix::operator=(const BaseMatrix& X)
{
   REPORT // CheckConversion(X);
   // MatrixConversionCheck mcc;
   Eq(X,MatrixType::SB);
}

void SymmetricBandMatrix::CornerClear() const
{
   // set unused parts of BandMatrix to zero
   REPORT
   int i = lower_val; Real* s = store; int bw = lower_val + 1;
   if (i) for(;;)
   {
      int j = i;
      Real* sj = s;
      while (j--) *sj++ = 0.0;
      if (!(--i)) break;
      s += bw;
   }
}

MatrixBandWidth SymmetricBandMatrix::bandwidth() const
   { REPORT return MatrixBandWidth(lower_val,lower_val); }

GeneralMatrix* BandMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new BandMatrix(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* UpperBandMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new UpperBandMatrix(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* LowerBandMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new LowerBandMatrix(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* SymmetricBandMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new SymmetricBandMatrix(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* BandLUMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new BandLUMatrix(*this); MatrixErrorNoSpace(gm);
   return gm;
}


inline Real square(Real x) { return x*x; }

Real SymmetricBandMatrix::sum_square() const
{
   REPORT
   CornerClear();
   Real sum1=0.0; Real sum2=0.0; Real* s=store; int i=nrows_val;
   int l=lower_val;
   while (i--)
      { int j = l; while (j--) sum2 += square(*s++); sum1 += square(*s++); }
   ((GeneralMatrix&)*this).tDelete(); return sum1 + 2.0 * sum2;
}

Real SymmetricBandMatrix::sum_absolute_value() const
{
   REPORT
   CornerClear();
   Real sum1=0.0; Real sum2=0.0; Real* s=store; int i=nrows_val;
   int l=lower_val;
   while (i--)
      { int j = l; while (j--) sum2 += fabs(*s++); sum1 += fabs(*s++); }
   ((GeneralMatrix&)*this).tDelete(); return sum1 + 2.0 * sum2;
}

Real SymmetricBandMatrix::sum() const
{
   REPORT
   CornerClear();
   Real sum1=0.0; Real sum2=0.0; Real* s=store; int i=nrows_val;
   int l=lower_val;
   while (i--)
      { int j = l; while (j--) sum2 += *s++; sum1 += *s++; }
   ((GeneralMatrix&)*this).tDelete(); return sum1 + 2.0 * sum2;
}





#ifdef use_namespace
}
#endif

