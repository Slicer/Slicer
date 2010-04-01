//$$ newmat7.cpp     Invert, solve, binary operations

// Copyright (C) 1991,2,3,4: R B Davies

#include "include.h"

#include "newmat.h"
#include "newmatrc.h"

#ifdef use_namespace
namespace NEWMAT {
#endif


#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,7); ++ExeCount; }
#else
#define REPORT {}
#endif


//***************************** solve routines ******************************/

GeneralMatrix* GeneralMatrix::MakeSolver()
{
   REPORT
   GeneralMatrix* gm = new CroutMatrix(*this);
   MatrixErrorNoSpace(gm); gm->ReleaseAndDelete(); return gm;
}

GeneralMatrix* Matrix::MakeSolver()
{
   REPORT
   GeneralMatrix* gm = new CroutMatrix(*this);
   MatrixErrorNoSpace(gm); gm->ReleaseAndDelete(); return gm;
}

void CroutMatrix::Solver(MatrixColX& mcout, const MatrixColX& mcin)
{
   REPORT
   int i = mcin.skip; Real* el = mcin.data-i; Real* el1 = el;
   while (i--) *el++ = 0.0;
   el += mcin.storage; i = nrows_val - mcin.skip - mcin.storage;
   while (i--) *el++ = 0.0;
   lubksb(el1, mcout.skip);
}


// Do we need check for entirely zero output?

void UpperTriangularMatrix::Solver(MatrixColX& mcout,
   const MatrixColX& mcin)
{
   REPORT
   int i = mcin.skip-mcout.skip; Real* elx = mcin.data-i;
   while (i-- > 0) *elx++ = 0.0;
   int nr = mcin.skip+mcin.storage;
   elx = mcin.data+mcin.storage; Real* el = elx;
   int j = mcout.skip+mcout.storage-nr;
   int nc = ncols_val-nr; i = nr-mcout.skip;
   while (j-- > 0) *elx++ = 0.0;
   Real* Ael = store + (nr*(2*ncols_val-nr+1))/2; j = 0;
   while (i-- > 0)
   {
      elx = el; Real sum = 0.0; int jx = j++; Ael -= nc;
      while (jx--) sum += *(--Ael) * *(--elx);
      elx--; *elx = (*elx - sum) / *(--Ael);
   }
}

void LowerTriangularMatrix::Solver(MatrixColX& mcout,
   const MatrixColX& mcin)
{
   REPORT
   int i = mcin.skip-mcout.skip; Real* elx = mcin.data-i;
   while (i-- > 0) *elx++ = 0.0;
   int nc = mcin.skip; i = nc+mcin.storage; elx = mcin.data+mcin.storage;
   int nr = mcout.skip+mcout.storage; int j = nr-i; i = nr-nc;
   while (j-- > 0) *elx++ = 0.0;
   Real* el = mcin.data; Real* Ael = store + (nc*(nc+1))/2; j = 0;
   while (i-- > 0)
   {
      elx = el; Real sum = 0.0; int jx = j++; Ael += nc;
      while (jx--) sum += *Ael++ * *elx++;
      *elx = (*elx - sum) / *Ael++;
   }
}

//******************* carry out binary operations *************************/

static GeneralMatrix*
   GeneralMult(GeneralMatrix*,GeneralMatrix*,MultipliedMatrix*,MatrixType);
static GeneralMatrix*
   GeneralSolv(GeneralMatrix*,GeneralMatrix*,BaseMatrix*,MatrixType);
static GeneralMatrix*
   GeneralSolvI(GeneralMatrix*,BaseMatrix*,MatrixType);
static GeneralMatrix*
   GeneralKP(GeneralMatrix*,GeneralMatrix*,KPMatrix*,MatrixType);

GeneralMatrix* MultipliedMatrix::Evaluate(MatrixType mt)
{
   REPORT
   gm2 = ((BaseMatrix*&)bm2)->Evaluate();
   gm2 = gm2->Evaluate(gm2->type().MultRHS());     // no symmetric on RHS
   gm1=((BaseMatrix*&)bm1)->Evaluate();
   return GeneralMult(gm1, gm2, this, mt);
}

GeneralMatrix* SolvedMatrix::Evaluate(MatrixType mt)
{
   REPORT
   gm1=((BaseMatrix*&)bm1)->Evaluate();
   gm2=((BaseMatrix*&)bm2)->Evaluate();
   return GeneralSolv(gm1,gm2,this,mt);
}

GeneralMatrix* KPMatrix::Evaluate(MatrixType mt)
{
   REPORT
   gm1=((BaseMatrix*&)bm1)->Evaluate();
   gm2=((BaseMatrix*&)bm2)->Evaluate();
   return GeneralKP(gm1,gm2,this,mt);
}

// routines for adding or subtracting matrices of identical storage structure

static void Add(GeneralMatrix* gm, GeneralMatrix* gm1, GeneralMatrix* gm2)
{
   REPORT
   Real* s1=gm1->Store(); Real* s2=gm2->Store();
   Real* s=gm->Store(); int i=gm->Storage() >> 2;
   while (i--)
   {
       *s++ = *s1++ + *s2++; *s++ = *s1++ + *s2++;
       *s++ = *s1++ + *s2++; *s++ = *s1++ + *s2++;
   }
   i=gm->Storage() & 3; while (i--) *s++ = *s1++ + *s2++;
}

static void AddTo(GeneralMatrix* gm, const GeneralMatrix* gm2)
{
   REPORT
   const Real* s2=gm2->Store(); Real* s=gm->Store(); int i=gm->Storage() >> 2;
   while (i--)
   { *s++ += *s2++; *s++ += *s2++; *s++ += *s2++; *s++ += *s2++; }
   i=gm->Storage() & 3; while (i--) *s++ += *s2++;
}

void GeneralMatrix::PlusEqual(const GeneralMatrix& gm)
{
   REPORT
   if (nrows_val != gm.nrows_val || ncols_val != gm.ncols_val)
      Throw(IncompatibleDimensionsException(*this, gm));
   AddTo(this, &gm);
}

static void Subtract(GeneralMatrix* gm, GeneralMatrix* gm1, GeneralMatrix* gm2)
{
   REPORT
   Real* s1=gm1->Store(); Real* s2=gm2->Store();
   Real* s=gm->Store(); int i=gm->Storage() >> 2;
   while (i--)
   {
       *s++ = *s1++ - *s2++; *s++ = *s1++ - *s2++;
       *s++ = *s1++ - *s2++; *s++ = *s1++ - *s2++;
   }
   i=gm->Storage() & 3; while (i--) *s++ = *s1++ - *s2++;
}

static void SubtractFrom(GeneralMatrix* gm, const GeneralMatrix* gm2)
{
   REPORT
   const Real* s2=gm2->Store(); Real* s=gm->Store(); int i=gm->Storage() >> 2;
   while (i--)
   { *s++ -= *s2++; *s++ -= *s2++; *s++ -= *s2++; *s++ -= *s2++; }
   i=gm->Storage() & 3; while (i--) *s++ -= *s2++;
}

void GeneralMatrix::MinusEqual(const GeneralMatrix& gm)
{
   REPORT
   if (nrows_val != gm.nrows_val || ncols_val != gm.ncols_val)
      Throw(IncompatibleDimensionsException(*this, gm));
   SubtractFrom(this, &gm);
}

static void ReverseSubtract(GeneralMatrix* gm, const GeneralMatrix* gm2)
{
   REPORT
   const Real* s2=gm2->Store(); Real* s=gm->Store(); int i=gm->Storage() >> 2;
   while (i--)
   {
      *s = *s2++ - *s; s++; *s = *s2++ - *s; s++;
      *s = *s2++ - *s; s++; *s = *s2++ - *s; s++;
   }
   i=gm->Storage() & 3; while (i--) { *s = *s2++ - *s; s++; }
}

static void SP(GeneralMatrix* gm, GeneralMatrix* gm1, GeneralMatrix* gm2)
{
   REPORT
   Real* s1=gm1->Store(); Real* s2=gm2->Store();
   Real* s=gm->Store(); int i=gm->Storage() >> 2;
   while (i--)
   {
       *s++ = *s1++ * *s2++; *s++ = *s1++ * *s2++;
       *s++ = *s1++ * *s2++; *s++ = *s1++ * *s2++;
   }
   i=gm->Storage() & 3; while (i--) *s++ = *s1++ * *s2++;
}

static void SP(GeneralMatrix* gm, GeneralMatrix* gm2)
{
   REPORT
   Real* s2=gm2->Store(); Real* s=gm->Store(); int i=gm->Storage() >> 2;
   while (i--)
   { *s++ *= *s2++; *s++ *= *s2++; *s++ *= *s2++; *s++ *= *s2++; }
   i=gm->Storage() & 3; while (i--) *s++ *= *s2++;
}

// routines for adding or subtracting matrices of different storage structure

static void AddDS(GeneralMatrix* gm, GeneralMatrix* gm1, GeneralMatrix* gm2)
{
   REPORT
   int nr = gm->Nrows();
   MatrixRow mr1(gm1, LoadOnEntry); MatrixRow mr2(gm2, LoadOnEntry);
   MatrixRow mr(gm, StoreOnExit+DirectPart);
   while (nr--) { mr.Add(mr1,mr2); mr1.Next(); mr2.Next(); mr.Next(); }
}

static void AddDS(GeneralMatrix* gm, GeneralMatrix* gm2)
// Add into first argument
{
   REPORT
   int nr = gm->Nrows();
   MatrixRow mr(gm, StoreOnExit+LoadOnEntry+DirectPart);
   MatrixRow mr2(gm2, LoadOnEntry);
   while (nr--) { mr.Add(mr2); mr.Next(); mr2.Next(); }
}

static void SubtractDS
   (GeneralMatrix* gm, GeneralMatrix* gm1, GeneralMatrix* gm2)
{
   REPORT
   int nr = gm->Nrows();
   MatrixRow mr1(gm1, LoadOnEntry); MatrixRow mr2(gm2, LoadOnEntry);
   MatrixRow mr(gm, StoreOnExit+DirectPart);
   while (nr--) { mr.Sub(mr1,mr2); mr1.Next(); mr2.Next(); mr.Next(); }
}

static void SubtractDS(GeneralMatrix* gm, GeneralMatrix* gm2)
{
   REPORT
   int nr = gm->Nrows();
   MatrixRow mr(gm, LoadOnEntry+StoreOnExit+DirectPart);
   MatrixRow mr2(gm2, LoadOnEntry);
   while (nr--) { mr.Sub(mr2); mr.Next(); mr2.Next(); }
}

static void ReverseSubtractDS(GeneralMatrix* gm, GeneralMatrix* gm2)
{
   REPORT
   int nr = gm->Nrows();
   MatrixRow mr(gm, LoadOnEntry+StoreOnExit+DirectPart);
   MatrixRow mr2(gm2, LoadOnEntry);
   while (nr--) { mr.RevSub(mr2); mr2.Next(); mr.Next(); }
}

static void SPDS(GeneralMatrix* gm, GeneralMatrix* gm1, GeneralMatrix* gm2)
{
   REPORT
   int nr = gm->Nrows();
   MatrixRow mr1(gm1, LoadOnEntry); MatrixRow mr2(gm2, LoadOnEntry);
   MatrixRow mr(gm, StoreOnExit+DirectPart);
   while (nr--) { mr.Multiply(mr1,mr2); mr1.Next(); mr2.Next(); mr.Next(); }
}

static void SPDS(GeneralMatrix* gm, GeneralMatrix* gm2)
// SP into first argument
{
   REPORT
   int nr = gm->Nrows();
   MatrixRow mr(gm, StoreOnExit+LoadOnEntry+DirectPart);
   MatrixRow mr2(gm2, LoadOnEntry);
   while (nr--) { mr.Multiply(mr2); mr.Next(); mr2.Next(); }
}

static GeneralMatrix* GeneralMult1(GeneralMatrix* gm1, GeneralMatrix* gm2,
   MultipliedMatrix* mm, MatrixType mtx)
{
   REPORT
   Tracer tr("GeneralMult1");
   int nr=gm1->Nrows(); int nc=gm2->Ncols();
   if (gm1->Ncols() !=gm2->Nrows())
      Throw(IncompatibleDimensionsException(*gm1, *gm2));
   GeneralMatrix* gmx = mtx.New(nr,nc,mm);

   MatrixCol mcx(gmx, StoreOnExit+DirectPart);
   MatrixCol mc2(gm2, LoadOnEntry);
   while (nc--)
   {
      MatrixRow mr1(gm1, LoadOnEntry, mcx.Skip());
      Real* el = mcx.Data();                         // pointer to an element
      int n = mcx.Storage();
      while (n--) { *(el++) = DotProd(mr1,mc2); mr1.Next(); }
      mc2.Next(); mcx.Next();
   }
   gmx->ReleaseAndDelete(); gm1->tDelete(); gm2->tDelete(); return gmx;
}

static GeneralMatrix* GeneralMult2(GeneralMatrix* gm1, GeneralMatrix* gm2,
   MultipliedMatrix* mm, MatrixType mtx)
{
   // version that accesses by row only - not good for thin matrices
   // or column vectors in right hand term.
   REPORT
   Tracer tr("GeneralMult2");
   int nr=gm1->Nrows(); int nc=gm2->Ncols();
   if (gm1->Ncols() !=gm2->Nrows())
      Throw(IncompatibleDimensionsException(*gm1, *gm2));
   GeneralMatrix* gmx = mtx.New(nr,nc,mm);

   MatrixRow mrx(gmx, LoadOnEntry+StoreOnExit+DirectPart);
   MatrixRow mr1(gm1, LoadOnEntry);
   while (nr--)
   {
      MatrixRow mr2(gm2, LoadOnEntry, mr1.Skip());
      Real* el = mr1.Data();                         // pointer to an element
      int n = mr1.Storage();
      mrx.Zero();
      while (n--) { mrx.AddScaled(mr2, *el++); mr2.Next(); }
      mr1.Next(); mrx.Next();
   }
   gmx->ReleaseAndDelete(); gm1->tDelete(); gm2->tDelete(); return gmx;
}

static GeneralMatrix* mmMult(GeneralMatrix* gm1, GeneralMatrix* gm2)
{
   // matrix multiplication for type Matrix only
   REPORT
   Tracer tr("MatrixMult");

   int nr=gm1->Nrows(); int ncr=gm1->Ncols(); int nc=gm2->Ncols();
   if (ncr != gm2->Nrows()) Throw(IncompatibleDimensionsException(*gm1,*gm2));

   Matrix* gm = new Matrix(nr,nc); MatrixErrorNoSpace(gm);

   Real* s1=gm1->Store(); Real* s2=gm2->Store(); Real* s=gm->Store();

   if (ncr)
   {
      while (nr--)
      {
         Real* s2x = s2; int j = ncr;
         Real* sx = s; Real f = *s1++; int k = nc;
         while (k--) *sx++ = f * *s2x++;
         while (--j)
            { sx = s; f = *s1++; k = nc; while (k--) *sx++ += f * *s2x++; }
         s = sx;
      }
   }
   else *gm = 0.0;

   gm->ReleaseAndDelete(); gm1->tDelete(); gm2->tDelete(); return gm;
}

static GeneralMatrix* GeneralMult(GeneralMatrix* gm1, GeneralMatrix* gm2,
   MultipliedMatrix* mm, MatrixType mtx)
{
   if ( Rectangular(gm1->type(), gm2->type(), mtx))
   {
      REPORT
      return mmMult(gm1, gm2);
   }
   else
   {
      REPORT
      Compare(gm1->type() * gm2->type(),mtx);
      int nr = gm2->Nrows(); int nc = gm2->Ncols();
      if (nc <= 5 && nr > nc) { REPORT return GeneralMult1(gm1, gm2, mm, mtx); }
      else { REPORT return GeneralMult2(gm1, gm2, mm, mtx); }
   }
}

static GeneralMatrix* GeneralKP(GeneralMatrix* gm1, GeneralMatrix* gm2,
   KPMatrix* kp, MatrixType mtx)
{
   REPORT
   Tracer tr("GeneralKP");
   int nr1 = gm1->Nrows(); int nc1 = gm1->Ncols();
   int nr2 = gm2->Nrows(); int nc2 = gm2->Ncols();
   Compare((gm1->type()).KP(gm2->type()),mtx);
   GeneralMatrix* gmx = mtx.New(nr1*nr2, nc1*nc2, kp);
   MatrixRow mrx(gmx, LoadOnEntry+StoreOnExit+DirectPart);
   MatrixRow mr1(gm1, LoadOnEntry);
   for (int i = 1; i <= nr1; ++i)
   {
      MatrixRow mr2(gm2, LoadOnEntry);
      for (int j = 1; j <= nr2; ++j)
         { mrx.KP(mr1,mr2); mr2.Next(); mrx.Next(); }
      mr1.Next();
   }
   gmx->ReleaseAndDelete(); gm1->tDelete(); gm2->tDelete(); return gmx;
}

static GeneralMatrix* GeneralSolv(GeneralMatrix* gm1, GeneralMatrix* gm2,
   BaseMatrix* sm, MatrixType mtx)
{
   REPORT
   Tracer tr("GeneralSolv");
   Compare(gm1->type().i() * gm2->type(),mtx);
   int nr = gm1->Nrows();
   if (nr != gm1->Ncols()) Throw(NotSquareException(*gm1));
   int nc = gm2->Ncols();
   if (gm1->Ncols() != gm2->Nrows())
      Throw(IncompatibleDimensionsException(*gm1, *gm2));
   GeneralMatrix* gmx = mtx.New(nr,nc,sm); MatrixErrorNoSpace(gmx);
   Real* r = new Real [nr]; MatrixErrorNoSpace(r);
   MONITOR_REAL_NEW("Make   (GenSolv)",nr,r)
   GeneralMatrix* gms = gm1->MakeSolver();
   Try
   {

      MatrixColX mcx(gmx, r, StoreOnExit+DirectPart);   // copy to and from r
         // this must be inside Try so mcx is destroyed before gmx
      MatrixColX mc2(gm2, r, LoadOnEntry);
      while (nc--) { gms->Solver(mcx, mc2); mcx.Next(); mc2.Next(); }
   }
   CatchAll
   {
      if (gms) gms->tDelete();
      delete gmx;                   // <--------------------
      gm2->tDelete();
      MONITOR_REAL_DELETE("Delete (GenSolv)",nr,r)
                          // AT&T version 2.1 gives an internal error
      delete [] r;
      ReThrow;
   }
   gms->tDelete(); gmx->ReleaseAndDelete(); gm2->tDelete();
   MONITOR_REAL_DELETE("Delete (GenSolv)",nr,r)
                          // AT&T version 2.1 gives an internal error
   delete [] r;
   return gmx;
}

// version for inverses - gm2 is identity
static GeneralMatrix* GeneralSolvI(GeneralMatrix* gm1, BaseMatrix* sm,
   MatrixType mtx)
{
   REPORT
   Tracer tr("GeneralSolvI");
   Compare(gm1->type().i(),mtx);
   int nr = gm1->Nrows();
   if (nr != gm1->Ncols()) Throw(NotSquareException(*gm1));
   int nc = nr;
   // DiagonalMatrix I(nr); I = 1;
   IdentityMatrix I(nr);
   GeneralMatrix* gmx = mtx.New(nr,nc,sm); MatrixErrorNoSpace(gmx);
   Real* r = new Real [nr]; MatrixErrorNoSpace(r);
   MONITOR_REAL_NEW("Make   (GenSolvI)",nr,r)
   GeneralMatrix* gms = gm1->MakeSolver();
   Try
   {

      MatrixColX mcx(gmx, r, StoreOnExit+DirectPart);   // copy to and from r
         // this must be inside Try so mcx is destroyed before gmx
      MatrixColX mc2(&I, r, LoadOnEntry);
      while (nc--) { gms->Solver(mcx, mc2); mcx.Next(); mc2.Next(); }
   }
   CatchAll
   {
      if (gms) gms->tDelete();
      delete gmx;
      MONITOR_REAL_DELETE("Delete (GenSolvI)",nr,r)
                          // AT&T version 2.1 gives an internal error
      delete [] r;
      ReThrow;
   }
   gms->tDelete(); gmx->ReleaseAndDelete();
   MONITOR_REAL_DELETE("Delete (GenSolvI)",nr,r)
                          // AT&T version 2.1 gives an internal error
   delete [] r;
   return gmx;
}

GeneralMatrix* InvertedMatrix::Evaluate(MatrixType mtx)
{
   // Matrix Inversion - use solve routines
   Tracer tr("InvertedMatrix::Evaluate");
   REPORT
   gm=((BaseMatrix*&)bm)->Evaluate();
   return GeneralSolvI(gm,this,mtx);
}

//*************************** New versions ************************

GeneralMatrix* AddedMatrix::Evaluate(MatrixType mtd)
{
   REPORT
   Tracer tr("AddedMatrix::Evaluate");
   gm1=((BaseMatrix*&)bm1)->Evaluate(); gm2=((BaseMatrix*&)bm2)->Evaluate();
   int nr=gm1->Nrows(); int nc=gm1->Ncols();
   if (nr!=gm2->Nrows() || nc!=gm2->Ncols())
   {
      Try { Throw(IncompatibleDimensionsException(*gm1, *gm2)); }
      CatchAll
      {
         gm1->tDelete(); gm2->tDelete();
         ReThrow;
      }
   }
   MatrixType mt1 = gm1->type(), mt2 = gm2->type(); MatrixType mts = mt1 + mt2;
   if (!mtd) { REPORT mtd = mts; }
   else if (!(mtd.DataLossOK || mtd >= mts))
   {
      REPORT
      gm1->tDelete(); gm2->tDelete();
      Throw(ProgramException("Illegal Conversion", mts, mtd));
   }
   GeneralMatrix* gmx;
   bool c1 = (mtd == mt1), c2 = (mtd == mt2);
   if ( c1 && c2 && (gm1->SimpleAddOK(gm2) == 0) )
   {
      if (gm1->reuse()) { REPORT AddTo(gm1,gm2); gm2->tDelete(); gmx = gm1; }
      else if (gm2->reuse()) { REPORT AddTo(gm2,gm1); gmx = gm2; }
      else
      {
         REPORT
         // what if new throws an exception
         Try { gmx = mt1.New(nr,nc,this); }
         CatchAll
         {
            ReThrow;
         }
         gmx->ReleaseAndDelete(); Add(gmx,gm1,gm2);
      }
   }
   else
   {
      if (c1 && c2)
      {
         short SAO = gm1->SimpleAddOK(gm2);
         if (SAO & 1) { REPORT c1 = false; }
         if (SAO & 2) { REPORT c2 = false; }
      }
      if (c1 && gm1->reuse() )               // must have type test first
         { REPORT AddDS(gm1,gm2); gm2->tDelete(); gmx = gm1; }
      else if (c2 && gm2->reuse() )
         { REPORT AddDS(gm2,gm1); if (!c1) gm1->tDelete(); gmx = gm2; }
      else
      {
         REPORT
         Try { gmx = mtd.New(nr,nc,this); }
         CatchAll
         {
            if (!c1) gm1->tDelete(); if (!c2) gm2->tDelete();
            ReThrow;
         }
         AddDS(gmx,gm1,gm2);
         if (!c1) gm1->tDelete(); if (!c2) gm2->tDelete();
         gmx->ReleaseAndDelete();
      }
   }
   return gmx;
}

GeneralMatrix* SubtractedMatrix::Evaluate(MatrixType mtd)
{
   REPORT
   Tracer tr("SubtractedMatrix::Evaluate");
   gm1=((BaseMatrix*&)bm1)->Evaluate(); gm2=((BaseMatrix*&)bm2)->Evaluate();
   int nr=gm1->Nrows(); int nc=gm1->Ncols();
   if (nr!=gm2->Nrows() || nc!=gm2->Ncols())
   {
      Try { Throw(IncompatibleDimensionsException(*gm1, *gm2)); }
      CatchAll
      {
         gm1->tDelete(); gm2->tDelete();
         ReThrow;
      }
   }
   MatrixType mt1 = gm1->type(), mt2 = gm2->type(); MatrixType mts = mt1 + mt2;
   if (!mtd) { REPORT mtd = mts; }
   else if (!(mtd.DataLossOK || mtd >= mts))
   {
      gm1->tDelete(); gm2->tDelete();
      Throw(ProgramException("Illegal Conversion", mts, mtd));
   }
   GeneralMatrix* gmx;
   bool c1 = (mtd == mt1), c2 = (mtd == mt2);
   if ( c1 && c2 && (gm1->SimpleAddOK(gm2) == 0) )
   {
      if (gm1->reuse())
         { REPORT SubtractFrom(gm1,gm2); gm2->tDelete(); gmx = gm1; }
      else if (gm2->reuse()) { REPORT ReverseSubtract(gm2,gm1); gmx = gm2; }
      else
      {
         REPORT
         Try { gmx = mt1.New(nr,nc,this); }
         CatchAll
         {
            ReThrow;
         }
         gmx->ReleaseAndDelete(); Subtract(gmx,gm1,gm2);
      }
   }
   else
   {
      if (c1 && c2)
      {
         short SAO = gm1->SimpleAddOK(gm2);
         if (SAO & 1) { REPORT c1 = false; }
         if (SAO & 2) { REPORT c2 = false; }
      }
      if (c1 && gm1->reuse() )               // must have type test first
         { REPORT SubtractDS(gm1,gm2); gm2->tDelete(); gmx = gm1; }
      else if (c2 && gm2->reuse() )
      {
         REPORT ReverseSubtractDS(gm2,gm1);
         if (!c1) gm1->tDelete(); gmx = gm2;
      }
      else
      {
         REPORT
         // what if New throws and exception
         Try { gmx = mtd.New(nr,nc,this); }
         CatchAll
         {
            if (!c1) gm1->tDelete(); if (!c2) gm2->tDelete();
            ReThrow;
         }
         SubtractDS(gmx,gm1,gm2);
         if (!c1) gm1->tDelete(); if (!c2) gm2->tDelete();
         gmx->ReleaseAndDelete();
      }
   }
   return gmx;
}

GeneralMatrix* SPMatrix::Evaluate(MatrixType mtd)
{
   REPORT
   Tracer tr("SPMatrix::Evaluate");
   gm1=((BaseMatrix*&)bm1)->Evaluate(); gm2=((BaseMatrix*&)bm2)->Evaluate();
   int nr=gm1->Nrows(); int nc=gm1->Ncols();
   if (nr!=gm2->Nrows() || nc!=gm2->Ncols())
   {
      Try { Throw(IncompatibleDimensionsException(*gm1, *gm2)); }
      CatchAll
      {
         gm1->tDelete(); gm2->tDelete();
         ReThrow;
      }
   }
   MatrixType mt1 = gm1->type(), mt2 = gm2->type();
   MatrixType mts = mt1.SP(mt2);
   if (!mtd) { REPORT mtd = mts; }
   else if (!(mtd.DataLossOK || mtd >= mts))
   {
      gm1->tDelete(); gm2->tDelete();
      Throw(ProgramException("Illegal Conversion", mts, mtd));
   }
   GeneralMatrix* gmx;
   bool c1 = (mtd == mt1), c2 = (mtd == mt2);
   if ( c1 && c2 && (gm1->SimpleAddOK(gm2) == 0) )
   {
      if (gm1->reuse()) { REPORT SP(gm1,gm2); gm2->tDelete(); gmx = gm1; }
      else if (gm2->reuse()) { REPORT SP(gm2,gm1); gmx = gm2; }
      else
      {
         REPORT
         Try { gmx = mt1.New(nr,nc,this); }
         CatchAll
         {
            ReThrow;
         }
         gmx->ReleaseAndDelete(); SP(gmx,gm1,gm2);
      }
   }
   else
   {
      if (c1 && c2)
      {
         short SAO = gm1->SimpleAddOK(gm2);
         if (SAO & 1) { REPORT c2 = false; }    // c1 and c2 swapped
         if (SAO & 2) { REPORT c1 = false; }
      }
      if (c1 && gm1->reuse() )               // must have type test first
         { REPORT SPDS(gm1,gm2); gm2->tDelete(); gmx = gm1; }
      else if (c2 && gm2->reuse() )
         { REPORT SPDS(gm2,gm1); if (!c1) gm1->tDelete(); gmx = gm2; }
      else
      {
         REPORT
         // what if New throws and exception
         Try { gmx = mtd.New(nr,nc,this); }
         CatchAll
         {
            if (!c1) gm1->tDelete(); if (!c2) gm2->tDelete();
            ReThrow;
         }
         SPDS(gmx,gm1,gm2);
         if (!c1) gm1->tDelete(); if (!c2) gm2->tDelete();
         gmx->ReleaseAndDelete();
      }
   }
   return gmx;
}



//*************************** norm functions ****************************/

Real BaseMatrix::norm1() const
{
   // maximum of sum of absolute values of a column
   REPORT
   GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   int nc = gm->Ncols(); Real value = 0.0;
   MatrixCol mc(gm, LoadOnEntry);
   while (nc--)
      { Real v = mc.SumAbsoluteValue(); if (value < v) value = v; mc.Next(); }
   gm->tDelete(); return value;
}

Real BaseMatrix::norm_infinity() const
{
   // maximum of sum of absolute values of a row
   REPORT
   GeneralMatrix* gm = ((BaseMatrix&)*this).Evaluate();
   int nr = gm->Nrows(); Real value = 0.0;
   MatrixRow mr(gm, LoadOnEntry);
   while (nr--)
      { Real v = mr.SumAbsoluteValue(); if (value < v) value = v; mr.Next(); }
   gm->tDelete(); return value;
}

//********************** Concatenation and stacking *************************/

GeneralMatrix* ConcatenatedMatrix::Evaluate(MatrixType mtx)
{
   REPORT
   Tracer tr("Concatenate");
      gm2 = ((BaseMatrix*&)bm2)->Evaluate();
      gm1 = ((BaseMatrix*&)bm1)->Evaluate();
      Compare(gm1->type() | gm2->type(),mtx);
      int nr=gm1->Nrows(); int nc = gm1->Ncols() + gm2->Ncols();
      if (nr != gm2->Nrows())
         Throw(IncompatibleDimensionsException(*gm1, *gm2));
      GeneralMatrix* gmx = mtx.New(nr,nc,this);
      MatrixRow mr1(gm1, LoadOnEntry); MatrixRow mr2(gm2, LoadOnEntry);
      MatrixRow mr(gmx, StoreOnExit+DirectPart);
      while (nr--) { mr.ConCat(mr1,mr2); mr1.Next(); mr2.Next(); mr.Next(); }
      gmx->ReleaseAndDelete(); gm1->tDelete(); gm2->tDelete(); return gmx;
}

GeneralMatrix* StackedMatrix::Evaluate(MatrixType mtx)
{
   REPORT
   Tracer tr("Stack");
      gm2 = ((BaseMatrix*&)bm2)->Evaluate();
      gm1 = ((BaseMatrix*&)bm1)->Evaluate();
      Compare(gm1->type() & gm2->type(),mtx);
      int nc=gm1->Ncols();
      int nr1 = gm1->Nrows(); int nr2 = gm2->Nrows();
      if (nc != gm2->Ncols())
         Throw(IncompatibleDimensionsException(*gm1, *gm2));
      GeneralMatrix* gmx = mtx.New(nr1+nr2,nc,this);
      MatrixRow mr1(gm1, LoadOnEntry); MatrixRow mr2(gm2, LoadOnEntry);
      MatrixRow mr(gmx, StoreOnExit+DirectPart);
      while (nr1--) { mr.Copy(mr1); mr1.Next(); mr.Next(); }
      while (nr2--) { mr.Copy(mr2); mr2.Next(); mr.Next(); }
      gmx->ReleaseAndDelete(); gm1->tDelete(); gm2->tDelete(); return gmx;
}

// ************************* equality of matrices ******************** //

static bool RealEqual(Real* s1, Real* s2, int n)
{
   int i = n >> 2;
   while (i--)
   {
      if (*s1++ != *s2++) return false; if (*s1++ != *s2++) return false;
      if (*s1++ != *s2++) return false; if (*s1++ != *s2++) return false;
   }
   i = n & 3; while (i--) if (*s1++ != *s2++) return false;
   return true;
}

static bool intEqual(int* s1, int* s2, int n)
{
   int i = n >> 2;
   while (i--)
   {
      if (*s1++ != *s2++) return false; if (*s1++ != *s2++) return false;
      if (*s1++ != *s2++) return false; if (*s1++ != *s2++) return false;
   }
   i = n & 3; while (i--) if (*s1++ != *s2++) return false;
   return true;
}


bool operator==(const BaseMatrix& A, const BaseMatrix& B)
{
   Tracer tr("BaseMatrix ==");
   REPORT
   GeneralMatrix* gmA = ((BaseMatrix&)A).Evaluate();
   GeneralMatrix* gmB = ((BaseMatrix&)B).Evaluate();

   if (gmA == gmB)                            // same matrix
      { REPORT gmA->tDelete(); return true; }

   if ( gmA->Nrows() != gmB->Nrows() || gmA->Ncols() != gmB->Ncols() )
                                              // different dimensions
      { REPORT gmA->tDelete(); gmB->tDelete(); return false; }

   // check for CroutMatrix or BandLUMatrix
   MatrixType AType = gmA->type(); MatrixType BType = gmB->type();
   if (AType.CannotConvert() || BType.CannotConvert() )
   {
      REPORT
      bool bx = gmA->IsEqual(*gmB);
      gmA->tDelete(); gmB->tDelete();
      return bx;
   }

   // is matrix storage the same
   // will need to modify if further matrix structures are introduced
   if (AType == BType && gmA->bandwidth() == gmB->bandwidth())
   {                                          // compare store
      REPORT
      bool bx = RealEqual(gmA->Store(),gmB->Store(),gmA->Storage());
      gmA->tDelete(); gmB->tDelete();
      return bx;
   }

   // matrix storage different - just subtract
   REPORT  return is_zero(*gmA-*gmB);
}

bool operator==(const GeneralMatrix& A, const GeneralMatrix& B)
{
   Tracer tr("GeneralMatrix ==");
   // May or may not call tDeletes
   REPORT

   if (&A == &B)                              // same matrix
      { REPORT return true; }

   if ( A.Nrows() != B.Nrows() || A.Ncols() != B.Ncols() )
      { REPORT return false; }                // different dimensions

   // check for CroutMatrix or BandLUMatrix
   MatrixType AType = A.Type(); MatrixType BType = B.Type();
   if (AType.CannotConvert() || BType.CannotConvert() )
      { REPORT  return A.IsEqual(B); }

   // is matrix storage the same
   // will need to modify if further matrix structures are introduced
   if (AType == BType && A.bandwidth() == B.bandwidth())
      { REPORT return RealEqual(A.Store(),B.Store(),A.Storage()); }

   // matrix storage different - just subtract
   REPORT  return is_zero(A-B);
}

bool GeneralMatrix::is_zero() const
{
   REPORT
   Real* s=store; int i = storage >> 2;
   while (i--)
   {
      if (*s++) return false; if (*s++) return false;
      if (*s++) return false; if (*s++) return false;
   }
   i = storage & 3; while (i--) if (*s++) return false;
   return true;
}

bool is_zero(const BaseMatrix& A)
{
   Tracer tr("BaseMatrix::is_zero");
   REPORT
   GeneralMatrix* gm1 = 0; bool bx;
   Try { gm1=((BaseMatrix&)A).Evaluate(); bx = gm1->is_zero(); }
   CatchAll { if (gm1) gm1->tDelete(); ReThrow; }
   gm1->tDelete();
   return bx;
}

// IsEqual functions - insist matrices are of same type
// as well as equal values to be equal

bool GeneralMatrix::IsEqual(const GeneralMatrix& A) const
{
   Tracer tr("GeneralMatrix IsEqual");
   if (A.type() != type())                       // not same types
      { REPORT return false; }
   if (&A == this)                               // same matrix
      { REPORT  return true; }
   if (A.nrows_val != nrows_val || A.ncols_val != ncols_val)
                                                 // different dimensions
   { REPORT return false; }
   // is matrix storage the same - compare store
   REPORT
   return RealEqual(A.store,store,storage);
}

bool CroutMatrix::IsEqual(const GeneralMatrix& A) const
{
   Tracer tr("CroutMatrix IsEqual");
   if (A.type() != type())                       // not same types
      { REPORT return false; }
   if (&A == this)                               // same matrix
      { REPORT  return true; }
   if (A.nrows_val != nrows_val || A.ncols_val != ncols_val)
                                                 // different dimensions
   { REPORT return false; }
   // is matrix storage the same - compare store
   REPORT
   return RealEqual(A.store,store,storage)
      && intEqual(((CroutMatrix&)A).indx, indx, nrows_val);
}


bool BandLUMatrix::IsEqual(const GeneralMatrix& A) const
{
   Tracer tr("BandLUMatrix IsEqual");
   if (A.type() != type())                       // not same types
      { REPORT  return false; }
   if (&A == this)                               // same matrix
      { REPORT  return true; }
   if ( A.Nrows() != nrows_val || A.Ncols() != ncols_val
      || ((BandLUMatrix&)A).m1 != m1 || ((BandLUMatrix&)A).m2 != m2 )
                                                 // different dimensions
   { REPORT  return false; }

   // matrix storage the same - compare store
   REPORT
   return RealEqual(A.Store(),store,storage)
      && RealEqual(((BandLUMatrix&)A).store2,store2,storage2)
      && intEqual(((BandLUMatrix&)A).indx, indx, nrows_val);
}


// ************************* cross products ******************** //

inline void crossproduct_body(Real* a, Real* b, Real* c)
{
   c[0] = a[1] * b[2] - a[2] * b[1];
   c[1] = a[2] * b[0] - a[0] * b[2];
   c[2] = a[0] * b[1] - a[1] * b[0];
}

Matrix crossproduct(const Matrix& A, const Matrix& B)
{
   REPORT
   int ac = A.Ncols(); int ar = A.Nrows();
   int bc = B.Ncols(); int br = B.Nrows();
   Real* a = A.Store(); Real* b = B.Store();
   if (ac == 3)
   {
      if (bc != 3 || ar != 1 || br != 1)
         { Tracer et("crossproduct"); IncompatibleDimensionsException(A, B); }
      REPORT
      RowVector C(3);  Real* c = C.Store(); crossproduct_body(a, b, c);
      return (Matrix&)C;
   }
   else
   {
      if (ac != 1 || bc != 1 || ar != 3 || br != 3)
         { Tracer et("crossproduct"); IncompatibleDimensionsException(A, B); }
      REPORT
      ColumnVector C(3);  Real* c = C.Store(); crossproduct_body(a, b, c);
      return (Matrix&)C;
   }
}

ReturnMatrix crossproduct_rows(const Matrix& A, const Matrix& B)
{
   REPORT
   int n = A.Nrows();
   if (A.Ncols() != 3 || B.Ncols() != 3 || n != B.Nrows())
   {
      Tracer et("crossproduct_rows"); IncompatibleDimensionsException(A, B);
   }
   Matrix C(n, 3);
   Real* a = A.Store(); Real* b = B.Store(); Real* c = C.Store();
   if (n--)
   {
      for (;;)
      {
         crossproduct_body(a, b, c);
         if (!(n--)) break;
         a += 3; b += 3; c += 3;
      }
   }

   return C.ForReturn();
}

ReturnMatrix crossproduct_columns(const Matrix& A, const Matrix& B)
{
   REPORT
   int n = A.Ncols();
   if (A.Nrows() != 3 || B.Nrows() != 3 || n != B.Ncols())
   {
      Tracer et("crossproduct_columns");
      IncompatibleDimensionsException(A, B);
   }
   Matrix C(3, n);
   Real* a = A.Store(); Real* b = B.Store(); Real* c = C.Store();
   Real* an = a+n; Real* an2 = an+n;
   Real* bn = b+n; Real* bn2 = bn+n;
   Real* cn = c+n; Real* cn2 = cn+n;

   int i = n; 
   while (i--)
   {
      *c++   = *an    * *bn2   - *an2   * *bn;
      *cn++  = *an2++ * *b     - *a     * *bn2++;
      *cn2++ = *a++   * *bn++  - *an++  * *b++;
   }

   return C.ForReturn();
}


#ifdef use_namespace
}
#endif


