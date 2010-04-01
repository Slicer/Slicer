//$$ newmat2.cpp      Matrix row and column operations

// Copyright (C) 1991,2,3,4: R B Davies

#define WANT_MATH

#include "include.h"

#include "newmat.h"
#include "newmatrc.h"

#ifdef use_namespace
namespace NEWMAT {
#endif


#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,2); ++ExeCount; }
#else
#define REPORT {}
#endif

//#define MONITOR(what,storage,store) { cout << what << " " << storage << " at " << (long)store << "\n"; }

#define MONITOR(what,store,storage) {}

/************************** Matrix Row/Col functions ************************/

void MatrixRowCol::Add(const MatrixRowCol& mrc)
{
   // THIS += mrc
   REPORT
   int f = mrc.skip; int l = f + mrc.storage; int lx = skip + storage;
   if (f < skip) f = skip; if (l > lx) l = lx; l -= f;
   if (l<=0) return;
   Real* elx=data+(f-skip); Real* el=mrc.data+(f-mrc.skip);
   while (l--) *elx++ += *el++;
}

void MatrixRowCol::AddScaled(const MatrixRowCol& mrc, Real x)
{
   REPORT
   // THIS += (mrc * x)
   int f = mrc.skip; int l = f + mrc.storage; int lx = skip + storage;
   if (f < skip) f = skip; if (l > lx) l = lx; l -= f;
   if (l<=0) return;
   Real* elx=data+(f-skip); Real* el=mrc.data+(f-mrc.skip);
   while (l--) *elx++ += *el++ * x;
}

void MatrixRowCol::Sub(const MatrixRowCol& mrc)
{
   REPORT
   // THIS -= mrc
   int f = mrc.skip; int l = f + mrc.storage; int lx = skip + storage;
   if (f < skip) f = skip; if (l > lx) l = lx; l -= f;
   if (l<=0) return;
   Real* elx=data+(f-skip); Real* el=mrc.data+(f-mrc.skip);
   while (l--) *elx++ -= *el++;
}

void MatrixRowCol::Inject(const MatrixRowCol& mrc)
// copy stored elements only
{
   REPORT
   int f = mrc.skip; int l = f + mrc.storage; int lx = skip + storage;
   if (f < skip) f = skip; if (l > lx) l = lx; l -= f;
   if (l<=0) return;
   Real* elx=data+(f-skip); Real* ely=mrc.data+(f-mrc.skip);
   while (l--) *elx++ = *ely++;
}

Real DotProd(const MatrixRowCol& mrc1, const MatrixRowCol& mrc2)
{
   REPORT                                         // not accessed
   int f = mrc1.skip; int f2 = mrc2.skip;
   int l = f + mrc1.storage; int l2 = f2 + mrc2.storage;
   if (f < f2) f = f2; if (l > l2) l = l2; l -= f;
   if (l<=0) return 0.0;

   Real* el1=mrc1.data+(f-mrc1.skip); Real* el2=mrc2.data+(f-mrc2.skip);
   Real sum = 0.0;
   while (l--) sum += *el1++ * *el2++;
   return sum;
}

void MatrixRowCol::Add(const MatrixRowCol& mrc1, const MatrixRowCol& mrc2)
{
   // THIS = mrc1 + mrc2
   int f = skip; int l = skip + storage;
   int f1 = mrc1.skip; int l1 = f1 + mrc1.storage;
   if (f1<f) f1=f; if (l1>l) l1=l;
   int f2 = mrc2.skip; int l2 = f2 + mrc2.storage;
   if (f2<f) f2=f; if (l2>l) l2=l;
   Real* el = data + (f-skip);
   Real* el1 = mrc1.data+(f1-mrc1.skip); Real* el2 = mrc2.data+(f2-mrc2.skip);
   if (f1<f2)
   {
      int i = f1-f; while (i--) *el++ = 0.0;
      if (l1<=f2)                              // disjoint
      {
         REPORT                                // not accessed
         i = l1-f1;     while (i--) *el++ = *el1++;
         i = f2-l1;     while (i--) *el++ = 0.0;
         i = l2-f2;     while (i--) *el++ = *el2++;
         i = l-l2;      while (i--) *el++ = 0.0;
      }
      else
      {
         i = f2-f1;    while (i--) *el++ = *el1++;
         if (l1<=l2)
         {
            REPORT
            i = l1-f2; while (i--) *el++ = *el1++ + *el2++;
            i = l2-l1; while (i--) *el++ = *el2++;
            i = l-l2;  while (i--) *el++ = 0.0;
         }
         else
         {
            REPORT
            i = l2-f2; while (i--) *el++ = *el1++ + *el2++;
            i = l1-l2; while (i--) *el++ = *el1++;
            i = l-l1;  while (i--) *el++ = 0.0;
         }
      }
   }
   else
   {
      int i = f2-f; while (i--) *el++ = 0.0;
      if (l2<=f1)                              // disjoint
      {
         REPORT                                // not accessed
         i = l2-f2;     while (i--) *el++ = *el2++;
         i = f1-l2;     while (i--) *el++ = 0.0;
         i = l1-f1;     while (i--) *el++ = *el1++;
         i = l-l1;      while (i--) *el++ = 0.0;
      }
      else
      {
         i = f1-f2;    while (i--) *el++ = *el2++;
         if (l2<=l1)
         {
            REPORT
            i = l2-f1; while (i--) *el++ = *el1++ + *el2++;
            i = l1-l2; while (i--) *el++ = *el1++;
            i = l-l1;  while (i--) *el++ = 0.0;
         }
         else
         {
            REPORT
            i = l1-f1; while (i--) *el++ = *el1++ + *el2++;
            i = l2-l1; while (i--) *el++ = *el2++;
            i = l-l2;  while (i--) *el++ = 0.0;
         }
      }
   }
}

void MatrixRowCol::Sub(const MatrixRowCol& mrc1, const MatrixRowCol& mrc2)
{
   // THIS = mrc1 - mrc2
   int f = skip; int l = skip + storage;
   int f1 = mrc1.skip; int l1 = f1 + mrc1.storage;
   if (f1<f) f1=f; if (l1>l) l1=l;
   int f2 = mrc2.skip; int l2 = f2 + mrc2.storage;
   if (f2<f) f2=f; if (l2>l) l2=l;
   Real* el = data + (f-skip);
   Real* el1 = mrc1.data+(f1-mrc1.skip); Real* el2 = mrc2.data+(f2-mrc2.skip);
   if (f1<f2)
   {
      int i = f1-f; while (i--) *el++ = 0.0;
      if (l1<=f2)                              // disjoint
      {
         REPORT                                // not accessed
         i = l1-f1;     while (i--) *el++ = *el1++;
         i = f2-l1;     while (i--) *el++ = 0.0;
         i = l2-f2;     while (i--) *el++ = - *el2++;
         i = l-l2;      while (i--) *el++ = 0.0;
      }
      else
      {
         i = f2-f1;    while (i--) *el++ = *el1++;
         if (l1<=l2)
         {
            REPORT
            i = l1-f2; while (i--) *el++ = *el1++ - *el2++;
            i = l2-l1; while (i--) *el++ = - *el2++;
            i = l-l2;  while (i--) *el++ = 0.0;
         }
         else
         {
            REPORT
            i = l2-f2; while (i--) *el++ = *el1++ - *el2++;
            i = l1-l2; while (i--) *el++ = *el1++;
            i = l-l1;  while (i--) *el++ = 0.0;
         }
      }
   }
   else
   {
      int i = f2-f; while (i--) *el++ = 0.0;
      if (l2<=f1)                              // disjoint
      {
         REPORT                                // not accessed
         i = l2-f2;     while (i--) *el++ = - *el2++;
         i = f1-l2;     while (i--) *el++ = 0.0;
         i = l1-f1;     while (i--) *el++ = *el1++;
         i = l-l1;      while (i--) *el++ = 0.0;
      }
      else
      {
         i = f1-f2;    while (i--) *el++ = - *el2++;
         if (l2<=l1)
         {
            REPORT
            i = l2-f1; while (i--) *el++ = *el1++ - *el2++;
            i = l1-l2; while (i--) *el++ = *el1++;
            i = l-l1;  while (i--) *el++ = 0.0;
         }
         else
         {
            REPORT
            i = l1-f1; while (i--) *el++ = *el1++ - *el2++;
            i = l2-l1; while (i--) *el++ = - *el2++;
            i = l-l2;  while (i--) *el++ = 0.0;
         }
      }
   }
}


void MatrixRowCol::Add(const MatrixRowCol& mrc1, Real x)
{
   // THIS = mrc1 + x
   REPORT
   if (!storage) return;
   int f = mrc1.skip; int l = f + mrc1.storage; int lx = skip + storage;
   if (f < skip) { f = skip; if (l < f) l = f; }
   if (l > lx) { l = lx; if (f > lx) f = lx; }

   Real* elx = data; Real* ely = mrc1.data+(f-mrc1.skip);

   int l1 = f-skip;  while (l1--) *elx++ = x;
       l1 = l-f;     while (l1--) *elx++ = *ely++ + x;
       lx -= l;      while (lx--) *elx++ = x;
}

void MatrixRowCol::NegAdd(const MatrixRowCol& mrc1, Real x)
{
   // THIS = x - mrc1
   REPORT
   if (!storage) return;
   int f = mrc1.skip; int l = f + mrc1.storage; int lx = skip + storage;
   if (f < skip) { f = skip; if (l < f) l = f; }
   if (l > lx) { l = lx; if (f > lx) f = lx; }

   Real* elx = data; Real* ely = mrc1.data+(f-mrc1.skip);

   int l1 = f-skip;  while (l1--) *elx++ = x;
       l1 = l-f;     while (l1--) *elx++ = x - *ely++;
       lx -= l;      while (lx--) *elx++ = x;
}

void MatrixRowCol::RevSub(const MatrixRowCol& mrc1)
{
   // THIS = mrc - THIS
   REPORT
   if (!storage) return;
   int f = mrc1.skip; int l = f + mrc1.storage; int lx = skip + storage;
   if (f < skip) { f = skip; if (l < f) l = f; }
   if (l > lx) { l = lx; if (f > lx) f = lx; }

   Real* elx = data; Real* ely = mrc1.data+(f-mrc1.skip);

   int l1 = f-skip;  while (l1--) { *elx = - *elx; elx++; }
       l1 = l-f;     while (l1--) { *elx = *ely++ - *elx; elx++; }
       lx -= l;      while (lx--) { *elx = - *elx; elx++; }
}

void MatrixRowCol::ConCat(const MatrixRowCol& mrc1, const MatrixRowCol& mrc2)
{
   // THIS = mrc1 | mrc2
   REPORT
   int f1 = mrc1.skip; int l1 = f1 + mrc1.storage; int lx = skip + storage;
   if (f1 < skip) { f1 = skip; if (l1 < f1) l1 = f1; }
   if (l1 > lx) { l1 = lx; if (f1 > lx) f1 = lx; }

   Real* elx = data;

   int i = f1-skip;  while (i--) *elx++ =0.0;
   i = l1-f1;
   if (i)                       // in case f1 would take ely out of range
      { Real* ely = mrc1.data+(f1-mrc1.skip);  while (i--) *elx++ = *ely++; }

   int f2 = mrc2.skip; int l2 = f2 + mrc2.storage; i = mrc1.length;
   int skipx = l1 - i; lx -= i; // addresses rel to second seg, maybe -ve
   if (f2 < skipx) { f2 = skipx; if (l2 < f2) l2 = f2; }
   if (l2 > lx) { l2 = lx; if (f2 > lx) f2 = lx; }

   i = f2-skipx; while (i--) *elx++ = 0.0;
   i = l2-f2;
   if (i)                       // in case f2 would take ely out of range
      { Real* ely = mrc2.data+(f2-mrc2.skip); while (i--) *elx++ = *ely++; }
   lx -= l2;     while (lx--) *elx++ = 0.0;
}

void MatrixRowCol::Multiply(const MatrixRowCol& mrc1)
// element by element multiply into
{
   REPORT
   if (!storage) return;
   int f = mrc1.skip; int l = f + mrc1.storage; int lx = skip + storage;
   if (f < skip) { f = skip; if (l < f) l = f; }
   if (l > lx) { l = lx; if (f > lx) f = lx; }

   Real* elx = data; Real* ely = mrc1.data+(f-mrc1.skip);

   int l1 = f-skip;  while (l1--) *elx++ = 0;
       l1 = l-f;     while (l1--) *elx++ *= *ely++;
       lx -= l;      while (lx--) *elx++ = 0;
}

void MatrixRowCol::Multiply(const MatrixRowCol& mrc1, const MatrixRowCol& mrc2)
// element by element multiply
{
   int f = skip; int l = skip + storage;
   int f1 = mrc1.skip; int l1 = f1 + mrc1.storage;
   if (f1<f) f1=f; if (l1>l) l1=l;
   int f2 = mrc2.skip; int l2 = f2 + mrc2.storage;
   if (f2<f) f2=f; if (l2>l) l2=l;
   Real* el = data + (f-skip); int i;
   if (f1<f2) f1 = f2; if (l1>l2) l1 = l2;
   if (l1<=f1) { REPORT i = l-f; while (i--) *el++ = 0.0; }  // disjoint
   else
   {
      REPORT
      Real* el1 = mrc1.data+(f1-mrc1.skip);
      Real* el2 = mrc2.data+(f1-mrc2.skip);
      i = f1-f ;    while (i--) *el++ = 0.0;
      i = l1-f1;    while (i--) *el++ = *el1++ * *el2++;
      i = l-l1;     while (i--) *el++ = 0.0;
   }
}

void MatrixRowCol::KP(const MatrixRowCol& mrc1, const MatrixRowCol& mrc2)
// row for Kronecker product
{
   int f = skip; int s = storage; Real* el = data; int i;

   i = mrc1.skip * mrc2.length;
   if (i > f)
   {
      i -= f; f = 0; if (i > s) { i = s; s = 0; }  else s -= i;
      while (i--) *el++ = 0.0;
      if (s == 0) return;
   }
   else f -= i;

   i = mrc1.storage; Real* el1 = mrc1.data;
   int mrc2_skip = mrc2.skip; int mrc2_storage = mrc2.storage;
   int mrc2_length = mrc2.length;
   int mrc2_remain = mrc2_length - mrc2_skip - mrc2_storage;
   while (i--)
   {
      int j; Real* el2 = mrc2.data; Real vel1 = *el1;
      if (f == 0 && mrc2_length <= s)
      {
         j = mrc2_skip; s -= j;    while (j--) *el++ = 0.0;
         j = mrc2_storage; s -= j; while (j--) *el++ = vel1 * *el2++;
         j = mrc2_remain; s -= j;  while (j--) *el++ = 0.0;
      }
      else if (f >= mrc2_length) f -= mrc2_length;
      else
      {
         j = mrc2_skip;
         if (j > f)
         {
            j -= f; f = 0; if (j > s) { j = s; s = 0; } else s -= j;
            while (j--) *el++ = 0.0;
         }
         else f -= j;

         j = mrc2_storage;
         if (j > f)
         {
            j -= f; el2 += f; f = 0; if (j > s) { j = s; s = 0; } else s -= j;
            while (j--) *el++ = vel1 * *el2++;
         }
         else f -= j;

         j = mrc2_remain;
         if (j > f)
         {
            j -= f; f = 0; if (j > s) { j = s; s = 0; } else s -= j;
            while (j--) *el++ = 0.0;
         }
         else f -= j;
      }
      if (s == 0) return;
      ++el1;
   }

   i = (mrc1.length - mrc1.skip - mrc1.storage) * mrc2.length;
   if (i > f)
   {
      i -= f; if (i > s) i = s;
      while (i--) *el++ = 0.0;
   }
}


void MatrixRowCol::Copy(const MatrixRowCol& mrc1)
{
   // THIS = mrc1
   REPORT
   if (!storage) return;
   int f = mrc1.skip; int l = f + mrc1.storage; int lx = skip + storage;
   if (f < skip) { f = skip; if (l < f) l = f; }
   if (l > lx) { l = lx; if (f > lx) f = lx; }

   Real* elx = data; Real* ely = 0;

   if (l-f) ely = mrc1.data+(f-mrc1.skip);

   int l1 = f-skip;  while (l1--) *elx++ = 0.0;
       l1 = l-f;     while (l1--) *elx++ = *ely++;
       lx -= l;      while (lx--) *elx++ = 0.0;
}

void MatrixRowCol::CopyCheck(const MatrixRowCol& mrc1)
// Throw an exception if this would lead to a loss of data
{
   REPORT
   if (!storage) return;
   int f = mrc1.skip; int l = f + mrc1.storage; int lx = skip + storage;
   if (f < skip || l > lx) Throw(ProgramException("Illegal Conversion"));

   Real* elx = data; Real* ely = mrc1.data+(f-mrc1.skip);

   int l1 = f-skip;  while (l1--) *elx++ = 0.0;
       l1 = l-f;     while (l1--) *elx++ = *ely++;
       lx -= l;      while (lx--) *elx++ = 0.0;
}

void MatrixRowCol::Check(const MatrixRowCol& mrc1)
// Throw an exception if +=, -=, copy etc would lead to a loss of data
{
   REPORT
   int f = mrc1.skip; int l = f + mrc1.storage; int lx = skip + storage;
   if (f < skip || l > lx) Throw(ProgramException("Illegal Conversion"));
}

void MatrixRowCol::Check()
// Throw an exception if +=, -= of constant would lead to a loss of data
// that is: check full row is present
// may not be appropriate for symmetric matrices
{
   REPORT
   if (skip!=0 || storage!=length)
      Throw(ProgramException("Illegal Conversion"));
}

void MatrixRowCol::Negate(const MatrixRowCol& mrc1)
{
   // THIS = -mrc1
   REPORT
   if (!storage) return;
   int f = mrc1.skip; int l = f + mrc1.storage; int lx = skip + storage;
   if (f < skip) { f = skip; if (l < f) l = f; }
   if (l > lx) { l = lx; if (f > lx) f = lx; }

   Real* elx = data; Real* ely = mrc1.data+(f-mrc1.skip);

   int l1 = f-skip;  while (l1--) *elx++ = 0.0;
       l1 = l-f;     while (l1--) *elx++ = - *ely++;
       lx -= l;      while (lx--) *elx++ = 0.0;
}

void MatrixRowCol::Multiply(const MatrixRowCol& mrc1, Real s)
{
   // THIS = mrc1 * s
   REPORT
   if (!storage) return;
   int f = mrc1.skip; int l = f + mrc1.storage; int lx = skip + storage;
   if (f < skip) { f = skip; if (l < f) l = f; }
   if (l > lx) { l = lx; if (f > lx) f = lx; }

   Real* elx = data; Real* ely = mrc1.data+(f-mrc1.skip);

   int l1 = f-skip;  while (l1--) *elx++ = 0.0;
       l1 = l-f;     while (l1--) *elx++ = *ely++ * s;
       lx -= l;      while (lx--) *elx++ = 0.0;
}

void DiagonalMatrix::Solver(MatrixColX& mrc, const MatrixColX& mrc1)
{
   // mrc = mrc / mrc1   (elementwise)
   REPORT
   int f = mrc1.skip; int f0 = mrc.skip;
   int l = f + mrc1.storage; int lx = f0 + mrc.storage;
   if (f < f0) { f = f0; if (l < f) l = f; }
   if (l > lx) { l = lx; if (f > lx) f = lx; }

   Real* elx = mrc.data; Real* eld = store+f;

   int l1 = f-f0;    while (l1--) *elx++ = 0.0;
       l1 = l-f;     while (l1--) *elx++ /= *eld++;
       lx -= l;      while (lx--) *elx++ = 0.0;
   // Solver makes sure input and output point to same memory
}

void IdentityMatrix::Solver(MatrixColX& mrc, const MatrixColX& mrc1)
{
   // mrc = mrc / mrc1   (elementwise)
   REPORT
   int f = mrc1.skip; int f0 = mrc.skip;
   int l = f + mrc1.storage; int lx = f0 + mrc.storage;
   if (f < f0) { f = f0; if (l < f) l = f; }
   if (l > lx) { l = lx; if (f > lx) f = lx; }

   Real* elx = mrc.data; Real eldv = *store;

   int l1 = f-f0;    while (l1--) *elx++ = 0.0;
       l1 = l-f;     while (l1--) *elx++ /= eldv;
       lx -= l;      while (lx--) *elx++ = 0.0;
   // Solver makes sure input and output point to same memory
}

void MatrixRowCol::Copy(const double*& r)
{
   // THIS = *r
   REPORT
   Real* elx = data; const double* ely = r+skip; r += length;
   int l = storage; while (l--) *elx++ = (Real)*ely++;
}

void MatrixRowCol::Copy(const float*& r)
{
   // THIS = *r
   REPORT
   Real* elx = data; const float* ely = r+skip; r += length;
   int l = storage; while (l--) *elx++ = (Real)*ely++;
}

void MatrixRowCol::Copy(const int*& r)
{
   // THIS = *r
   REPORT
   Real* elx = data; const int* ely = r+skip; r += length;
   int l = storage; while (l--) *elx++ = (Real)*ely++;
}

void MatrixRowCol::Copy(Real r)
{
   // THIS = r
   REPORT  Real* elx = data; int l = storage; while (l--) *elx++ = r;
}

void MatrixRowCol::Zero()
{
   // THIS = 0
   REPORT  Real* elx = data; int l = storage; while (l--) *elx++ = 0;
}

void MatrixRowCol::Multiply(Real r)
{
   // THIS *= r
   REPORT  Real* elx = data; int l = storage; while (l--) *elx++ *= r;
}

void MatrixRowCol::Add(Real r)
{
   // THIS += r
   REPORT
   Real* elx = data; int l = storage; while (l--) *elx++ += r;
}

Real MatrixRowCol::SumAbsoluteValue()
{
   REPORT
   Real sum = 0.0; Real* elx = data; int l = storage;
   while (l--) sum += fabs(*elx++);
   return sum;
}

// max absolute value of r and elements of row/col
// we use <= or >= in all of these so we are sure of getting
// r reset at least once.
Real MatrixRowCol::MaximumAbsoluteValue1(Real r, int& i)
{
   REPORT
   Real* elx = data; int l = storage; int li = -1;
   while (l--) { Real f = fabs(*elx++); if (r <= f) { r = f; li = l; } }
   i = (li >= 0) ? storage - li + skip : 0;
   return r;
}

// min absolute value of r and elements of row/col
Real MatrixRowCol::MinimumAbsoluteValue1(Real r, int& i)
{
   REPORT
   Real* elx = data; int l = storage; int li = -1;
   while (l--) { Real f = fabs(*elx++); if (r >= f) { r = f; li = l; } }
   i = (li >= 0) ? storage - li + skip : 0;
   return r;
}

// max value of r and elements of row/col
Real MatrixRowCol::Maximum1(Real r, int& i)
{
   REPORT
   Real* elx = data; int l = storage; int li = -1;
   while (l--) { Real f = *elx++; if (r <= f) { r = f; li = l; } }
   i = (li >= 0) ? storage - li + skip : 0;
   return r;
}

// min value of r and elements of row/col
Real MatrixRowCol::Minimum1(Real r, int& i)
{
   REPORT
   Real* elx = data; int l = storage; int li = -1;
   while (l--) { Real f = *elx++; if (r >= f) { r = f; li = l; } }
   i = (li >= 0) ? storage - li + skip : 0;
   return r;
}

Real MatrixRowCol::Sum()
{
   REPORT
   Real sum = 0.0; Real* elx = data; int l = storage;
   while (l--) sum += *elx++;
   return sum;
}

void MatrixRowCol::SubRowCol(MatrixRowCol& mrc, int skip1, int l1) const
{
   mrc.length = l1;  int d = skip - skip1;
   if (d<0) { mrc.skip = 0; mrc.data = data - d; }
   else  { mrc.skip = d; mrc.data = data; }
   d = skip + storage - skip1;
   d = ((l1 < d) ? l1 : d) - mrc.skip;  mrc.storage = (d < 0) ? 0 : d;
   mrc.cw = 0;
}

#ifdef use_namespace
}
#endif

