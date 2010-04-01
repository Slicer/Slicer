//$$ newmat5.cpp         Transpose, evaluate etc

// Copyright (C) 1991,2,3,4: R B Davies

//#define WANT_STREAM

#include "include.h"

#include "newmat.h"
#include "newmatrc.h"

#ifdef use_namespace
namespace NEWMAT {
#endif


#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,5); ++ExeCount; }
#else
#define REPORT {}
#endif


/************************ carry out operations ******************************/


GeneralMatrix* GeneralMatrix::Transpose(TransposedMatrix* tm, MatrixType mt)
{
   GeneralMatrix* gm1;

   if (Compare(Type().t(),mt))
   {
      REPORT
      gm1 = mt.New(ncols_val,nrows_val,tm);
      for (int i=0; i<ncols_val; i++)
      {
         MatrixRow mr(gm1, StoreOnExit+DirectPart, i);
         MatrixCol mc(this, mr.Data(), LoadOnEntry, i);
      }
   }
   else
   {
      REPORT
      gm1 = mt.New(ncols_val,nrows_val,tm);
      MatrixRow mr(this, LoadOnEntry);
      MatrixCol mc(gm1, StoreOnExit+DirectPart);
      int i = nrows_val;
      while (i--) { mc.Copy(mr); mr.Next(); mc.Next(); }
   }
   tDelete(); gm1->ReleaseAndDelete(); return gm1;
}

GeneralMatrix* SymmetricMatrix::Transpose(TransposedMatrix*, MatrixType mt)
{ REPORT  return Evaluate(mt); }


GeneralMatrix* DiagonalMatrix::Transpose(TransposedMatrix*, MatrixType mt)
{ REPORT return Evaluate(mt); }

GeneralMatrix* ColumnVector::Transpose(TransposedMatrix*, MatrixType mt)
{
   REPORT
   GeneralMatrix* gmx = new RowVector; MatrixErrorNoSpace(gmx);
   gmx->nrows_val = 1; gmx->ncols_val = gmx->storage = storage;
   return BorrowStore(gmx,mt);
}

GeneralMatrix* RowVector::Transpose(TransposedMatrix*, MatrixType mt)
{
   REPORT
   GeneralMatrix* gmx = new ColumnVector; MatrixErrorNoSpace(gmx);
   gmx->ncols_val = 1; gmx->nrows_val = gmx->storage = storage;
   return BorrowStore(gmx,mt);
}

GeneralMatrix* IdentityMatrix::Transpose(TransposedMatrix*, MatrixType mt)
{ REPORT return Evaluate(mt); }

GeneralMatrix* GeneralMatrix::Evaluate(MatrixType mt)
{
   if (Compare(this->Type(),mt)) { REPORT return this; }
   REPORT
   GeneralMatrix* gmx = mt.New(nrows_val,ncols_val,this);
   MatrixRow mr(this, LoadOnEntry);
   MatrixRow mrx(gmx, StoreOnExit+DirectPart);
   int i=nrows_val;
   while (i--) { mrx.Copy(mr); mrx.Next(); mr.Next(); }
   tDelete(); gmx->ReleaseAndDelete(); return gmx;
}

GeneralMatrix* CroutMatrix::Evaluate(MatrixType mt)
{
   if (Compare(this->Type(),mt)) { REPORT return this; }
   REPORT
   Tracer et("CroutMatrix::Evaluate");
   bool dummy = true;
   if (dummy) Throw(ProgramException("Illegal use of CroutMatrix", *this));
   return this;
}

GeneralMatrix* GenericMatrix::Evaluate(MatrixType mt)
   { REPORT  return gm->Evaluate(mt); }

GeneralMatrix* ShiftedMatrix::Evaluate(MatrixType mt)
{
   gm=((BaseMatrix*&)bm)->Evaluate();
   int nr=gm->Nrows(); int nc=gm->Ncols();
   Compare(gm->Type().AddEqualEl(),mt);
   if (!(mt==gm->Type()))
   {
      REPORT
      GeneralMatrix* gmx = mt.New(nr,nc,this);
      MatrixRow mr(gm, LoadOnEntry);
      MatrixRow mrx(gmx, StoreOnExit+DirectPart);
      while (nr--) { mrx.Add(mr,f); mrx.Next(); mr.Next(); }
      gmx->ReleaseAndDelete(); gm->tDelete();
      return gmx;
   }
   else if (gm->reuse())
   {
      REPORT gm->Add(f);
      return gm;
   }
   else
   {
      REPORT GeneralMatrix* gmy = gm->Type().New(nr,nc,this);
      gmy->ReleaseAndDelete(); gmy->Add(gm,f);
      return gmy;
   }
}

GeneralMatrix* NegShiftedMatrix::Evaluate(MatrixType mt)
{
   gm=((BaseMatrix*&)bm)->Evaluate();
   int nr=gm->Nrows(); int nc=gm->Ncols();
   Compare(gm->Type().AddEqualEl(),mt);
   if (!(mt==gm->Type()))
   {
      REPORT
      GeneralMatrix* gmx = mt.New(nr,nc,this);
      MatrixRow mr(gm, LoadOnEntry);
      MatrixRow mrx(gmx, StoreOnExit+DirectPart);
      while (nr--) { mrx.NegAdd(mr,f); mrx.Next(); mr.Next(); }
      gmx->ReleaseAndDelete(); gm->tDelete();
      return gmx;
   }
   else if (gm->reuse())
   {
      REPORT gm->NegAdd(f);
      return gm;
   }
   else
   {
      REPORT GeneralMatrix* gmy = gm->Type().New(nr,nc,this);
      gmy->ReleaseAndDelete(); gmy->NegAdd(gm,f);
      return gmy;
   }
}

GeneralMatrix* ScaledMatrix::Evaluate(MatrixType mt)
{
   gm=((BaseMatrix*&)bm)->Evaluate();
   int nr=gm->Nrows(); int nc=gm->Ncols();
   if (Compare(gm->Type(),mt))
   {
      if (gm->reuse())
      {
         REPORT gm->Multiply(f);
         return gm;
      }
      else
      {
         REPORT GeneralMatrix* gmx = gm->Type().New(nr,nc,this);
         gmx->ReleaseAndDelete(); gmx->Multiply(gm,f);
         return gmx;
      }
   }
   else
   {
      REPORT
      GeneralMatrix* gmx = mt.New(nr,nc,this);
      MatrixRow mr(gm, LoadOnEntry);
      MatrixRow mrx(gmx, StoreOnExit+DirectPart);
      while (nr--) { mrx.Multiply(mr,f); mrx.Next(); mr.Next(); }
      gmx->ReleaseAndDelete(); gm->tDelete();
      return gmx;
   }
}

GeneralMatrix* NegatedMatrix::Evaluate(MatrixType mt)
{
   gm=((BaseMatrix*&)bm)->Evaluate();
   int nr=gm->Nrows(); int nc=gm->Ncols();
   if (Compare(gm->Type(),mt))
   {
      if (gm->reuse())
      {
         REPORT gm->Negate();
         return gm;
      }
      else
      {
         REPORT
         GeneralMatrix* gmx = gm->Type().New(nr,nc,this);
         gmx->ReleaseAndDelete(); gmx->Negate(gm);
         return gmx;
      }
   }
   else
   {
      REPORT
      GeneralMatrix* gmx = mt.New(nr,nc,this);
      MatrixRow mr(gm, LoadOnEntry);
      MatrixRow mrx(gmx, StoreOnExit+DirectPart);
      while (nr--) { mrx.Negate(mr); mrx.Next(); mr.Next(); }
      gmx->ReleaseAndDelete(); gm->tDelete();
      return gmx;
   }
}

GeneralMatrix* ReversedMatrix::Evaluate(MatrixType mt)
{
   gm=((BaseMatrix*&)bm)->Evaluate(); GeneralMatrix* gmx;

   if ((gm->Type()).is_band() && ! (gm->Type()).is_diagonal())
   {
      gm->tDelete();
      Throw(NotDefinedException("Reverse", "band matrices"));
   }

   if (gm->reuse()) { REPORT gm->ReverseElements(); gmx = gm; }
   else
   {
      REPORT
      gmx = gm->Type().New(gm->Nrows(), gm->Ncols(), this);
      gmx->ReverseElements(gm); gmx->ReleaseAndDelete();
   }
   return gmx->Evaluate(mt); // target matrix is different type?

}

GeneralMatrix* TransposedMatrix::Evaluate(MatrixType mt)
{
   REPORT
   gm=((BaseMatrix*&)bm)->Evaluate();
   Compare(gm->Type().t(),mt);
   GeneralMatrix* gmx=gm->Transpose(this, mt);
   return gmx;
}

GeneralMatrix* RowedMatrix::Evaluate(MatrixType mt)
{
   gm = ((BaseMatrix*&)bm)->Evaluate();
   GeneralMatrix* gmx = new RowVector; MatrixErrorNoSpace(gmx);
   gmx->nrows_val = 1; gmx->ncols_val = gmx->storage = gm->storage;
   return gm->BorrowStore(gmx,mt);
}

GeneralMatrix* ColedMatrix::Evaluate(MatrixType mt)
{
   gm = ((BaseMatrix*&)bm)->Evaluate();
   GeneralMatrix* gmx = new ColumnVector; MatrixErrorNoSpace(gmx);
   gmx->ncols_val = 1; gmx->nrows_val = gmx->storage = gm->storage;
   return gm->BorrowStore(gmx,mt);
}

GeneralMatrix* DiagedMatrix::Evaluate(MatrixType mt)
{
   gm = ((BaseMatrix*&)bm)->Evaluate();
   GeneralMatrix* gmx = new DiagonalMatrix; MatrixErrorNoSpace(gmx);
   gmx->nrows_val = gmx->ncols_val = gmx->storage = gm->storage;
   return gm->BorrowStore(gmx,mt);
}

GeneralMatrix* MatedMatrix::Evaluate(MatrixType mt)
{
   Tracer tr("MatedMatrix::Evaluate");
   gm = ((BaseMatrix*&)bm)->Evaluate();
   GeneralMatrix* gmx = new Matrix; MatrixErrorNoSpace(gmx);
   gmx->nrows_val = nr; gmx->ncols_val = nc; gmx->storage = gm->storage;
   if (nr*nc != gmx->storage)
      Throw(IncompatibleDimensionsException());
   return gm->BorrowStore(gmx,mt);
}

GeneralMatrix* GetSubMatrix::Evaluate(MatrixType mt)
{
   REPORT
   Tracer tr("SubMatrix(evaluate)");
   gm = ((BaseMatrix*&)bm)->Evaluate();
   if (row_number < 0) row_number = gm->Nrows();
   if (col_number < 0) col_number = gm->Ncols();
   if (row_skip+row_number > gm->Nrows() || col_skip+col_number > gm->Ncols())
   {
      gm->tDelete();
      Throw(SubMatrixDimensionException());
   }
   if (IsSym) Compare(gm->Type().ssub(), mt);
   else Compare(gm->Type().sub(), mt);
   GeneralMatrix* gmx = mt.New(row_number, col_number, this);
   int i = row_number;
   MatrixRow mr(gm, LoadOnEntry, row_skip); 
   MatrixRow mrx(gmx, StoreOnExit+DirectPart);
   MatrixRowCol sub;
   while (i--)
   {
      mr.SubRowCol(sub, col_skip, col_number);   // put values in sub
      mrx.Copy(sub); mrx.Next(); mr.Next();
   }
   gmx->ReleaseAndDelete(); gm->tDelete();
   return gmx;
}


GeneralMatrix* ReturnMatrix::Evaluate(MatrixType mt)
{
   return gm->Evaluate(mt);
}


void GeneralMatrix::Add(GeneralMatrix* gm1, Real f)
{
   REPORT
   Real* s1=gm1->store; Real* s=store; int i=(storage >> 2);
   while (i--)
   { *s++ = *s1++ + f; *s++ = *s1++ + f; *s++ = *s1++ + f; *s++ = *s1++ + f; }
   i = storage & 3; while (i--) *s++ = *s1++ + f;
}
   
void GeneralMatrix::Add(Real f)
{
   REPORT
   Real* s=store; int i=(storage >> 2);
   while (i--) { *s++ += f; *s++ += f; *s++ += f; *s++ += f; }
   i = storage & 3; while (i--) *s++ += f;
}
   
void GeneralMatrix::NegAdd(GeneralMatrix* gm1, Real f)
{
   REPORT
   Real* s1=gm1->store; Real* s=store; int i=(storage >> 2);
   while (i--)
   { *s++ = f - *s1++; *s++ = f - *s1++; *s++ = f - *s1++; *s++ = f - *s1++; }
   i = storage & 3; while (i--) *s++ = f - *s1++;
}
   
void GeneralMatrix::NegAdd(Real f)
{
   REPORT
   Real* s=store; int i=(storage >> 2);
   while (i--)
   {
      *s = f - *s; s++; *s = f - *s; s++;
      *s = f - *s; s++; *s = f - *s; s++;
   }
   i = storage & 3; while (i--)  { *s = f - *s; s++; }
}
   
void GeneralMatrix::Negate(GeneralMatrix* gm1)
{
   // change sign of elements
   REPORT
   Real* s1=gm1->store; Real* s=store; int i=(storage >> 2);
   while (i--)
   { *s++ = -(*s1++); *s++ = -(*s1++); *s++ = -(*s1++); *s++ = -(*s1++); }
   i = storage & 3; while(i--) *s++ = -(*s1++);
}
   
void GeneralMatrix::Negate()
{
   REPORT
   Real* s=store; int i=(storage >> 2);
   while (i--)
   { *s = -(*s); s++; *s = -(*s); s++; *s = -(*s); s++; *s = -(*s); s++; }
   i = storage & 3; while(i--) { *s = -(*s); s++; }
}
   
void GeneralMatrix::Multiply(GeneralMatrix* gm1, Real f)
{
   REPORT
   Real* s1=gm1->store; Real* s=store;  int i=(storage >> 2);
   while (i--)
   { *s++ = *s1++ * f; *s++ = *s1++ * f; *s++ = *s1++ * f; *s++ = *s1++ * f; }
   i = storage & 3; while (i--) *s++ = *s1++ * f;
}
   
void GeneralMatrix::Multiply(Real f)
{
   REPORT
   Real* s=store; int i=(storage >> 2);
   while (i--) { *s++ *= f; *s++ *= f; *s++ *= f; *s++ *= f; }
   i = storage & 3; while (i--) *s++ *= f;
}
   

/************************ MatrixInput routines ****************************/

// int MatrixInput::n;          // number values still to be read
// Real* MatrixInput::r;        // pointer to next location to be read to

MatrixInput MatrixInput::operator<<(double f)
{
   REPORT
   Tracer et("MatrixInput");
   if (n<=0) Throw(ProgramException("List of values too long"));
   *r = (Real)f; int n1 = n-1; n=0;   // n=0 so we won't trigger exception
   return MatrixInput(n1, r+1);
}


MatrixInput GeneralMatrix::operator<<(double f)
{
   REPORT
   Tracer et("MatrixInput");
   int n = Storage();
   if (n<=0) Throw(ProgramException("Loading data to zero length matrix"));
   Real* r; r = Store(); *r = (Real)f; n--;
   return MatrixInput(n, r+1);
}

MatrixInput GetSubMatrix::operator<<(double f)
{
   REPORT
   Tracer et("MatrixInput (GetSubMatrix)");
   SetUpLHS();
   if (row_number != 1 || col_skip != 0 || col_number != gm->Ncols())
   {
      Throw(ProgramException("MatrixInput requires complete rows"));
   }
   MatrixRow mr(gm, DirectPart, row_skip);  // to pick up location and length
   int n = mr.Storage();
   if (n<=0)
   {
      Throw(ProgramException("Loading data to zero length row"));
   }
   Real* r; r = mr.Data(); *r = (Real)f; n--;
   if (+(mr.cw*HaveStore))
   {
      Throw(ProgramException("Fails with this matrix type"));
   }
   return MatrixInput(n, r+1);
}

MatrixInput MatrixInput::operator<<(float f)
{
   REPORT
   Tracer et("MatrixInput");
   if (n<=0) Throw(ProgramException("List of values too long"));
   *r = (Real)f; int n1 = n-1; n=0;   // n=0 so we won't trigger exception
   return MatrixInput(n1, r+1);
}


MatrixInput GeneralMatrix::operator<<(float f)
{
   REPORT
   Tracer et("MatrixInput");
   int n = Storage();
   if (n<=0) Throw(ProgramException("Loading data to zero length matrix"));
   Real* r; r = Store(); *r = (Real)f; n--;
   return MatrixInput(n, r+1);
}

MatrixInput GetSubMatrix::operator<<(float f)
{
   REPORT
   Tracer et("MatrixInput (GetSubMatrix)");
   SetUpLHS();
   if (row_number != 1 || col_skip != 0 || col_number != gm->Ncols())
   {
      Throw(ProgramException("MatrixInput requires complete rows"));
   }
   MatrixRow mr(gm, DirectPart, row_skip);  // to pick up location and length
   int n = mr.Storage();
   if (n<=0)
   {
      Throw(ProgramException("Loading data to zero length row"));
   }
   Real* r; r = mr.Data(); *r = (Real)f; n--;
   if (+(mr.cw*HaveStore))
   {
      Throw(ProgramException("Fails with this matrix type"));
   }
   return MatrixInput(n, r+1);
}
MatrixInput::~MatrixInput()
{
   REPORT
   Tracer et("MatrixInput");
   if (n!=0) Throw(ProgramException("A list of values was too short"));
}

MatrixInput BandMatrix::operator<<(double)
{
   Tracer et("MatrixInput");
   bool dummy = true;
   if (dummy)                                   // get rid of warning message
      Throw(ProgramException("Cannot use list read with a BandMatrix"));
   return MatrixInput(0, 0);
}

MatrixInput BandMatrix::operator<<(float)
{
   Tracer et("MatrixInput");
   bool dummy = true;
   if (dummy)                                   // get rid of warning message
      Throw(ProgramException("Cannot use list read with a BandMatrix"));
   return MatrixInput(0, 0);
}

void BandMatrix::operator<<(const double*)
{ Throw(ProgramException("Cannot use array read with a BandMatrix")); }

void BandMatrix::operator<<(const float*)
{ Throw(ProgramException("Cannot use array read with a BandMatrix")); }

void BandMatrix::operator<<(const int*)
{ Throw(ProgramException("Cannot use array read with a BandMatrix")); }

void SymmetricBandMatrix::operator<<(const double*)
{ Throw(ProgramException("Cannot use array read with a BandMatrix")); }

void SymmetricBandMatrix::operator<<(const float*)
{ Throw(ProgramException("Cannot use array read with a BandMatrix")); }

void SymmetricBandMatrix::operator<<(const int*)
{ Throw(ProgramException("Cannot use array read with a BandMatrix")); }

// ************************* Reverse order of elements ***********************

void GeneralMatrix::ReverseElements(GeneralMatrix* gm)
{
   // reversing into a new matrix
   REPORT
   int n = Storage(); Real* rx = Store() + n; Real* x = gm->Store();
   while (n--) *(--rx) = *(x++);
}

void GeneralMatrix::ReverseElements()
{
   // reversing in place
   REPORT
   int n = Storage(); Real* x = Store(); Real* rx = x + n;
   n /= 2;
   while (n--) { Real t = *(--rx); *rx = *x; *(x++) = t; }
}


#ifdef use_namespace
}
#endif

