//$$ newmat4.cpp       Constructors, resize, basic utilities

// Copyright (C) 1991,2,3,4,8,9: R B Davies

//#define WANT_STREAM

#include "include.h"

#include "newmat.h"
#include "newmatrc.h"

#ifdef use_namespace
namespace NEWMAT {
#endif



#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,4); ++ExeCount; }
#else
#define REPORT {}
#endif


#define DO_SEARCH                   // search for LHS of = in RHS

// ************************* general utilities *************************/

static int tristore(int n)                    // elements in triangular matrix
{ return (n*(n+1))/2; }


// **************************** constructors ***************************/

GeneralMatrix::GeneralMatrix()
{ store=0; storage=0; nrows_val=0; ncols_val=0; tag_val=-1; }

GeneralMatrix::GeneralMatrix(ArrayLengthSpecifier s)
{
   REPORT
   storage=s.Value(); tag_val=-1;
   if (storage)
   {
      store = new Real [storage]; MatrixErrorNoSpace(store);
      MONITOR_REAL_NEW("Make (GenMatrix)",storage,store)
   }
   else store = 0;
}

Matrix::Matrix(int m, int n) : GeneralMatrix(m*n)
{ REPORT nrows_val=m; ncols_val=n; }

SquareMatrix::SquareMatrix(ArrayLengthSpecifier n)
   : Matrix(n.Value(),n.Value())
{ REPORT }

SymmetricMatrix::SymmetricMatrix(ArrayLengthSpecifier n)
   : GeneralMatrix(tristore(n.Value()))
{ REPORT nrows_val=n.Value(); ncols_val=n.Value(); }

UpperTriangularMatrix::UpperTriangularMatrix(ArrayLengthSpecifier n)
   : GeneralMatrix(tristore(n.Value()))
{ REPORT nrows_val=n.Value(); ncols_val=n.Value(); }

LowerTriangularMatrix::LowerTriangularMatrix(ArrayLengthSpecifier n)
   : GeneralMatrix(tristore(n.Value()))
{ REPORT nrows_val=n.Value(); ncols_val=n.Value(); }

DiagonalMatrix::DiagonalMatrix(ArrayLengthSpecifier m) : GeneralMatrix(m)
{ REPORT nrows_val=m.Value(); ncols_val=m.Value(); }

Matrix::Matrix(const BaseMatrix& M)
{
   REPORT // CheckConversion(M);
   // MatrixConversionCheck mcc;
   GeneralMatrix* gmx=((BaseMatrix&)M).Evaluate(MatrixType::Rt);
   GetMatrix(gmx);
}

SquareMatrix::SquareMatrix(const BaseMatrix& M) : Matrix(M)
{
   REPORT
   if (ncols_val != nrows_val)
   {
      Tracer tr("SquareMatrix");
      Throw(NotSquareException(*this));
   }
}


SquareMatrix::SquareMatrix(const Matrix& gm)
{
   REPORT
   if (gm.ncols_val != gm.nrows_val)
   {
      Tracer tr("SquareMatrix(Matrix)");
      Throw(NotSquareException(gm));
   }
   GetMatrix(&gm);
}


RowVector::RowVector(const BaseMatrix& M) : Matrix(M)
{
   REPORT
   if (nrows_val!=1)
   {
      Tracer tr("RowVector");
      Throw(VectorException(*this));
   }
}

ColumnVector::ColumnVector(const BaseMatrix& M) : Matrix(M)
{
   REPORT
   if (ncols_val!=1)
   {
      Tracer tr("ColumnVector");
      Throw(VectorException(*this));
   }
}

SymmetricMatrix::SymmetricMatrix(const BaseMatrix& M)
{
   REPORT  // CheckConversion(M);
   // MatrixConversionCheck mcc;
   GeneralMatrix* gmx=((BaseMatrix&)M).Evaluate(MatrixType::Sm);
   GetMatrix(gmx);
}

UpperTriangularMatrix::UpperTriangularMatrix(const BaseMatrix& M)
{
   REPORT // CheckConversion(M);
   // MatrixConversionCheck mcc;
   GeneralMatrix* gmx=((BaseMatrix&)M).Evaluate(MatrixType::UT);
   GetMatrix(gmx);
}

LowerTriangularMatrix::LowerTriangularMatrix(const BaseMatrix& M)
{
   REPORT // CheckConversion(M);
   // MatrixConversionCheck mcc;
   GeneralMatrix* gmx=((BaseMatrix&)M).Evaluate(MatrixType::LT);
   GetMatrix(gmx);
}

DiagonalMatrix::DiagonalMatrix(const BaseMatrix& M)
{
   REPORT //CheckConversion(M);
   // MatrixConversionCheck mcc;
   GeneralMatrix* gmx=((BaseMatrix&)M).Evaluate(MatrixType::Dg);
   GetMatrix(gmx);
}

IdentityMatrix::IdentityMatrix(const BaseMatrix& M)
{
   REPORT //CheckConversion(M);
   // MatrixConversionCheck mcc;
   GeneralMatrix* gmx=((BaseMatrix&)M).Evaluate(MatrixType::Id);
   GetMatrix(gmx);
}

GeneralMatrix::~GeneralMatrix()
{
   if (store)
   {
      MONITOR_REAL_DELETE("Free (GenMatrix)",storage,store)
      delete [] store;
   }
}

CroutMatrix::CroutMatrix(const BaseMatrix& m)
{
   REPORT
   Tracer tr("CroutMatrix");
   indx = 0;                     // in case of exception at next line
   GeneralMatrix* gm = ((BaseMatrix&)m).Evaluate();
   if (gm->nrows_val!=gm->ncols_val)
      { gm->tDelete(); Throw(NotSquareException(*gm)); }
   if (gm->type() == MatrixType::Ct)
      { REPORT  ((CroutMatrix*)gm)->get_aux(*this); GetMatrix(gm); }
   else
   {
      REPORT
      GeneralMatrix* gm1 = gm->Evaluate(MatrixType::Rt);
      GetMatrix(gm1);
      d=true; sing=false;
      indx=new int [nrows_val]; MatrixErrorNoSpace(indx);
      MONITOR_INT_NEW("Index (CroutMat)",nrows_val,indx)
      ludcmp();
   }
}

// could we use SetParameters instead of this
void CroutMatrix::get_aux(CroutMatrix& X)
{
   X.d = d; X.sing = sing;
   if (tag_val == 0 || tag_val == 1) // reuse the array 
      { REPORT  X.indx = indx; indx = 0; d = true; sing = true; return; }
   else if (nrows_val == 0)
      { REPORT indx = 0; d = true; sing = true; return; }
   else                              // copy the array
   { 
      REPORT
      Tracer tr("CroutMatrix::get_aux");
      int *ix = new int [nrows_val]; MatrixErrorNoSpace(ix);
      MONITOR_INT_NEW("Index (CM::get_aux)", nrows_val, ix)
      int n = nrows_val; int* i = ix; int* j = indx;
      while(n--) *i++ = *j++;
      X.indx = ix;
   }
}

CroutMatrix::CroutMatrix(const CroutMatrix& gm)
{
   REPORT
   Tracer tr("CroutMatrix(const CroutMatrix&)");
   ((CroutMatrix&)gm).get_aux(*this);
   GetMatrix(&gm);
}

CroutMatrix::~CroutMatrix()
{
   MONITOR_INT_DELETE("Index (CroutMat)",nrows_val,indx)
   delete [] indx;
}

//ReturnMatrix::ReturnMatrix(GeneralMatrix& gmx)
//{
//   REPORT
//   gm = gmx.Image(); gm->ReleaseAndDelete();
//}


GeneralMatrix::operator ReturnMatrix() const
{
   REPORT
   GeneralMatrix* gm = Image(); gm->ReleaseAndDelete();
   return ReturnMatrix(gm);
}



ReturnMatrix GeneralMatrix::for_return() const
{
   REPORT
   GeneralMatrix* gm = Image(); gm->ReleaseAndDelete();
   return ReturnMatrix(gm);
}

// ************ Constructors for use with NR in C++ interface ***********

#ifdef SETUP_C_SUBSCRIPTS

Matrix::Matrix(Real a, int m, int n) : GeneralMatrix(m * n)
   { REPORT nrows_val=m; ncols_val=n; operator=(a); }
   
Matrix::Matrix(const Real* a, int m, int n) : GeneralMatrix(m * n)
   { REPORT nrows_val=m; ncols_val=n; *this << a; }

#endif



// ************************** resize matrices ***************************/

void GeneralMatrix::resize(int nr, int nc, int s)
{
   REPORT
   if (store)
   {
      MONITOR_REAL_DELETE("Free (ReDimensi)",storage,store)
      delete [] store;
   }
   storage=s; nrows_val=nr; ncols_val=nc; tag_val=-1;
   if (s)
   {
      store = new Real [storage]; MatrixErrorNoSpace(store);
      MONITOR_REAL_NEW("Make (ReDimensi)",storage,store)
   }
   else store = 0;
}

void Matrix::resize(int nr, int nc)
{ REPORT GeneralMatrix::resize(nr,nc,nr*nc); }

void SquareMatrix::resize(int n)
{ REPORT GeneralMatrix::resize(n,n,n*n); }

void SquareMatrix::resize(int nr, int nc)
{
   REPORT
   Tracer tr("SquareMatrix::resize");
   if (nc != nr) Throw(NotSquareException(*this));
   GeneralMatrix::resize(nr,nc,nr*nc);
}

void SymmetricMatrix::resize(int nr)
{ REPORT GeneralMatrix::resize(nr,nr,tristore(nr)); }

void UpperTriangularMatrix::resize(int nr)
{ REPORT GeneralMatrix::resize(nr,nr,tristore(nr)); }

void LowerTriangularMatrix::resize(int nr)
{ REPORT GeneralMatrix::resize(nr,nr,tristore(nr)); }

void DiagonalMatrix::resize(int nr)
{ REPORT GeneralMatrix::resize(nr,nr,nr); }

void RowVector::resize(int nc)
{ REPORT GeneralMatrix::resize(1,nc,nc); }

void ColumnVector::resize(int nr)
{ REPORT GeneralMatrix::resize(nr,1,nr); }

void RowVector::resize(int nr, int nc)
{
   Tracer tr("RowVector::resize");
   if (nr != 1) Throw(VectorException(*this));
   REPORT GeneralMatrix::resize(1,nc,nc);
}

void ColumnVector::resize(int nr, int nc)
{
   Tracer tr("ColumnVector::resize");
   if (nc != 1) Throw(VectorException(*this));
   REPORT GeneralMatrix::resize(nr,1,nr);
}

void IdentityMatrix::resize(int nr)
{ REPORT GeneralMatrix::resize(nr,nr,1); *store = 1; }


void Matrix::resize(const GeneralMatrix& A)
{ REPORT  resize(A.Nrows(), A.Ncols()); }

void SquareMatrix::resize(const GeneralMatrix& A)
{
   REPORT
   int n = A.Nrows();
   if (n != A.Ncols())
   {
      Tracer tr("SquareMatrix::resize(GM)");
      Throw(NotSquareException(*this));
   }
   resize(n);
}

void nricMatrix::resize(const GeneralMatrix& A)
{ REPORT  resize(A.Nrows(), A.Ncols()); }

void ColumnVector::resize(const GeneralMatrix& A)
{ REPORT  resize(A.Nrows(), A.Ncols()); }

void RowVector::resize(const GeneralMatrix& A)
{ REPORT  resize(A.Nrows(), A.Ncols()); }

void SymmetricMatrix::resize(const GeneralMatrix& A)
{
   REPORT
   int n = A.Nrows();
   if (n != A.Ncols())
   {
      Tracer tr("SymmetricMatrix::resize(GM)");
      Throw(NotSquareException(*this));
   }
   resize(n);
}

void DiagonalMatrix::resize(const GeneralMatrix& A)
{
   REPORT
   int n = A.Nrows();
   if (n != A.Ncols())
   {
      Tracer tr("DiagonalMatrix::resize(GM)");
      Throw(NotSquareException(*this));
   }
   resize(n);
}

void UpperTriangularMatrix::resize(const GeneralMatrix& A)
{
   REPORT
   int n = A.Nrows();
   if (n != A.Ncols())
   {
      Tracer tr("UpperTriangularMatrix::resize(GM)");
      Throw(NotSquareException(*this));
   }
   resize(n);
}

void LowerTriangularMatrix::resize(const GeneralMatrix& A)
{
   REPORT
   int n = A.Nrows();
   if (n != A.Ncols())
   {
      Tracer tr("LowerTriangularMatrix::resize(GM)");
      Throw(NotSquareException(*this));
   }
   resize(n);
}

void IdentityMatrix::resize(const GeneralMatrix& A)
{
   REPORT
   int n = A.Nrows();
   if (n != A.Ncols())
   {
      Tracer tr("IdentityMatrix::resize(GM)");
      Throw(NotSquareException(*this));
   }
   resize(n);
}

void GeneralMatrix::resize(const GeneralMatrix&)
{
   REPORT
   Tracer tr("GeneralMatrix::resize(GM)");
   Throw(NotDefinedException("resize", "this type of matrix"));
}

//*********************** resize_keep *******************************

void Matrix::resize_keep(int nr, int nc)
{
   Tracer tr("Matrix::resize_keep");
   if (nr == nrows_val && nc == ncols_val) { REPORT return; }
   
   if (nr <= nrows_val && nc <= ncols_val)
   {
      REPORT
      Matrix X = submatrix(1,nr,1,nc);
      swap(X);
   }
   else if (nr >= nrows_val && nc >= ncols_val)
   {
      REPORT
      Matrix X(nr, nc); X = 0;
      X.submatrix(1,nrows_val,1,ncols_val) = *this;
      swap(X);
   }
   else
   {
      REPORT
      Matrix X(nr, nc); X = 0;
      if (nr > nrows_val) nr = nrows_val;
      if (nc > ncols_val) nc = ncols_val;
      X.submatrix(1,nr,1,nc) = submatrix(1,nr,1,nc);
      swap(X);
   }
} 

void SquareMatrix::resize_keep(int nr)
{
   Tracer tr("SquareMatrix::resize_keep");
   if (nr < nrows_val)
   {
      REPORT
      SquareMatrix X = sym_submatrix(1,nr);
      swap(X);
   }
   else if (nr > nrows_val)
   {
      REPORT
      SquareMatrix X(nr); X = 0;
      X.sym_submatrix(1,nrows_val) = *this;
      swap(X);
   }
}

void SquareMatrix::resize_keep(int nr, int nc)
{
   Tracer tr("SquareMatrix::resize_keep 2");
   REPORT
   if (nr != nc) Throw(NotSquareException(*this));
   resize_keep(nr);
}
 

void SymmetricMatrix::resize_keep(int nr)
{
   Tracer tr("SymmetricMatrix::resize_keep");
   if (nr < nrows_val)
   {
      REPORT
      SymmetricMatrix X = sym_submatrix(1,nr);
      swap(X);
   }
   else if (nr > nrows_val)
   {
      REPORT
      SymmetricMatrix X(nr); X = 0;
      X.sym_submatrix(1,nrows_val) = *this;
      swap(X);
   }
} 

void UpperTriangularMatrix::resize_keep(int nr)
{
   Tracer tr("UpperTriangularMatrix::resize_keep");
   if (nr < nrows_val)
   {
      REPORT
      UpperTriangularMatrix X = sym_submatrix(1,nr);
      swap(X);
   }
   else if (nr > nrows_val)
   {
      REPORT
      UpperTriangularMatrix X(nr); X = 0;
      X.sym_submatrix(1,nrows_val) = *this;
      swap(X);
   }
} 

void LowerTriangularMatrix::resize_keep(int nr)
{
   Tracer tr("LowerTriangularMatrix::resize_keep");
   if (nr < nrows_val)
   {
      REPORT
      LowerTriangularMatrix X = sym_submatrix(1,nr);
      swap(X);
   }
   else if (nr > nrows_val)
   {
      REPORT
      LowerTriangularMatrix X(nr); X = 0;
      X.sym_submatrix(1,nrows_val) = *this;
      swap(X);
   }
} 

void DiagonalMatrix::resize_keep(int nr)
{
   Tracer tr("DiagonalMatrix::resize_keep");
   if (nr < nrows_val)
   {
      REPORT
      DiagonalMatrix X = sym_submatrix(1,nr);
      swap(X);
   }
   else if (nr > nrows_val)
   {
      REPORT
      DiagonalMatrix X(nr); X = 0;
      X.sym_submatrix(1,nrows_val) = *this;
      swap(X);
   }
} 

void RowVector::resize_keep(int nc)
{
   Tracer tr("RowVector::resize_keep");
   if (nc < ncols_val)
   {
      REPORT
      RowVector X = columns(1,nc);
      swap(X);
   }
   else if (nc > ncols_val)
   {
      REPORT
      RowVector X(nc); X = 0;
      X.columns(1,ncols_val) = *this;
      swap(X);
   }
}

void RowVector::resize_keep(int nr, int nc)
{
   Tracer tr("RowVector::resize_keep 2");
   REPORT
   if (nr != 1) Throw(VectorException(*this));
   resize_keep(nc);
}

void ColumnVector::resize_keep(int nr)
{
   Tracer tr("ColumnVector::resize_keep");
   if (nr < nrows_val)
   {
      REPORT
      ColumnVector X = rows(1,nr);
      swap(X);
   }
   else if (nr > nrows_val)
   {
      REPORT
      ColumnVector X(nr); X = 0;
      X.rows(1,nrows_val) = *this;
      swap(X);
   }
} 

void ColumnVector::resize_keep(int nr, int nc)
{
   Tracer tr("ColumnVector::resize_keep 2");
   REPORT
   if (nc != 1) Throw(VectorException(*this));
   resize_keep(nr);
}


/*
void GeneralMatrix::resizeForAdd(const GeneralMatrix& A, const GeneralMatrix&)
{ REPORT resize(A); }

void GeneralMatrix::resizeForSP(const GeneralMatrix& A, const GeneralMatrix&)
{ REPORT resize(A); }


// ************************* SameStorageType ******************************

// SameStorageType checks A and B have same storage type including bandwidth
// It does not check same dimensions since we assume this is already done

bool GeneralMatrix::SameStorageType(const GeneralMatrix& A) const
{
   REPORT
   return type() == A.type();
}
*/

// ******************* manipulate types, storage **************************/

int GeneralMatrix::search(const BaseMatrix* s) const
{ REPORT return (s==this) ? 1 : 0; }

int GenericMatrix::search(const BaseMatrix* s) const
{ REPORT return gm->search(s); }

int MultipliedMatrix::search(const BaseMatrix* s) const
{ REPORT return bm1->search(s) + bm2->search(s); }

int ShiftedMatrix::search(const BaseMatrix* s) const
{ REPORT return bm->search(s); }

int NegatedMatrix::search(const BaseMatrix* s) const
{ REPORT return bm->search(s); }

int ReturnMatrix::search(const BaseMatrix* s) const
{ REPORT return (s==gm) ? 1 : 0; }

MatrixType Matrix::type() const { return MatrixType::Rt; }
MatrixType SquareMatrix::type() const { return MatrixType::Sq; }
MatrixType SymmetricMatrix::type() const { return MatrixType::Sm; }
MatrixType UpperTriangularMatrix::type() const { return MatrixType::UT; }
MatrixType LowerTriangularMatrix::type() const { return MatrixType::LT; }
MatrixType DiagonalMatrix::type() const { return MatrixType::Dg; }
MatrixType RowVector::type() const { return MatrixType::RV; }
MatrixType ColumnVector::type() const { return MatrixType::CV; }
MatrixType CroutMatrix::type() const { return MatrixType::Ct; }
MatrixType BandMatrix::type() const { return MatrixType::BM; }
MatrixType UpperBandMatrix::type() const { return MatrixType::UB; }
MatrixType LowerBandMatrix::type() const { return MatrixType::LB; }
MatrixType SymmetricBandMatrix::type() const { return MatrixType::SB; }

MatrixType IdentityMatrix::type() const { return MatrixType::Id; }



MatrixBandWidth BaseMatrix::bandwidth() const { REPORT return -1; }
MatrixBandWidth DiagonalMatrix::bandwidth() const { REPORT return 0; }
MatrixBandWidth IdentityMatrix::bandwidth() const { REPORT return 0; }

MatrixBandWidth UpperTriangularMatrix::bandwidth() const
   { REPORT return MatrixBandWidth(0,-1); }

MatrixBandWidth LowerTriangularMatrix::bandwidth() const
   { REPORT return MatrixBandWidth(-1,0); }

MatrixBandWidth BandMatrix::bandwidth() const
   { REPORT return MatrixBandWidth(lower_val,upper_val); }

MatrixBandWidth BandLUMatrix::bandwidth() const
   { REPORT return MatrixBandWidth(m1,m2); }
   
MatrixBandWidth GenericMatrix::bandwidth()const
   { REPORT return gm->bandwidth(); }

MatrixBandWidth AddedMatrix::bandwidth() const
   { REPORT return gm1->bandwidth() + gm2->bandwidth(); }

MatrixBandWidth SPMatrix::bandwidth() const
   { REPORT return gm1->bandwidth().minimum(gm2->bandwidth()); }

MatrixBandWidth KPMatrix::bandwidth() const
{
   int lower, upper;
   MatrixBandWidth bw1 = gm1->bandwidth(), bw2 = gm2->bandwidth();
   if (bw1.Lower() < 0)
   {
      if (bw2.Lower() < 0) { REPORT lower = -1; }
      else { REPORT lower = bw2.Lower() + (gm1->Nrows() - 1) * gm2->Nrows(); }
   }
   else
   {
      if (bw2.Lower() < 0)
         { REPORT lower = (1 + bw1.Lower()) * gm2->Nrows() - 1; }
      else { REPORT lower = bw2.Lower() + bw1.Lower() * gm2->Nrows(); }
   }
   if (bw1.Upper() < 0)
   {
      if (bw2.Upper() < 0) { REPORT upper = -1; }
      else { REPORT upper = bw2.Upper() + (gm1->Nrows() - 1) * gm2->Nrows(); }
   }
   else
   {
      if (bw2.Upper() < 0)
         { REPORT upper = (1 + bw1.Upper()) * gm2->Nrows() - 1; }
      else { REPORT upper = bw2.Upper() + bw1.Upper() * gm2->Nrows(); }
   }
   return MatrixBandWidth(lower, upper);
}

MatrixBandWidth MultipliedMatrix::bandwidth() const
{ REPORT return gm1->bandwidth() * gm2->bandwidth(); }

MatrixBandWidth ConcatenatedMatrix::bandwidth() const { REPORT return -1; }

MatrixBandWidth SolvedMatrix::bandwidth() const
{
   if (+gm1->type() & MatrixType::Diagonal)
      { REPORT return gm2->bandwidth(); }
   else { REPORT return -1; }
}

MatrixBandWidth ScaledMatrix::bandwidth() const
   { REPORT return gm->bandwidth(); }

MatrixBandWidth NegatedMatrix::bandwidth() const
   { REPORT return gm->bandwidth(); }

MatrixBandWidth TransposedMatrix::bandwidth() const
   { REPORT return gm->bandwidth().t(); }

MatrixBandWidth InvertedMatrix::bandwidth() const
{
   if (+gm->type() & MatrixType::Diagonal)
      { REPORT return MatrixBandWidth(0,0); }
   else { REPORT return -1; }
}

MatrixBandWidth RowedMatrix::bandwidth() const { REPORT return -1; }
MatrixBandWidth ColedMatrix::bandwidth() const { REPORT return -1; }
MatrixBandWidth DiagedMatrix::bandwidth() const { REPORT return 0; }
MatrixBandWidth MatedMatrix::bandwidth() const { REPORT return -1; }
MatrixBandWidth ReturnMatrix::bandwidth() const
   { REPORT return gm->bandwidth(); }

MatrixBandWidth GetSubMatrix::bandwidth() const
{

   if (row_skip==col_skip && row_number==col_number)
      { REPORT return gm->bandwidth(); }
   else { REPORT return MatrixBandWidth(-1); }
}

// ********************** the memory managment tools **********************/

//  Rules regarding tDelete, reuse, GetStore, BorrowStore
//    All matrices processed during expression evaluation must be subject
//    to exactly one of reuse(), tDelete(), GetStore() or BorrowStore().
//    If reuse returns true the matrix must be reused.
//    GetMatrix(gm) always calls gm->GetStore()
//    gm->Evaluate obeys rules
//    bm->Evaluate obeys rules for matrices in bm structure

//  Meaning of tag_val
//    tag_val = -1          memory cannot be reused (default situation)
//    tag_val = -2          memory has been borrowed from another matrix
//                               (don't change values)
//    tag_val = i > 0       delete or reuse memory after i operations
//    tag_val = 0           like value 1 but matrix was created by new
//                               so delete it

void GeneralMatrix::tDelete()
{
   if (tag_val<0)
   {
      if (tag_val<-1) { REPORT store = 0; delete this; return; }  // borrowed
      else { REPORT return; }   // not a temporary matrix - leave alone
   }
   if (tag_val==1)
   {
      if (store)
      {
         REPORT  MONITOR_REAL_DELETE("Free   (tDelete)",storage,store)
         delete [] store;
      }
      MiniCleanUp(); return;                           // CleanUp
   }
   if (tag_val==0) { REPORT delete this; return; }

   REPORT tag_val--; return;
}

void newmat_block_copy(int n, Real* from, Real* to)
{
   REPORT
   int i = (n >> 3);
   while (i--)
   {
      *to++ = *from++; *to++ = *from++; *to++ = *from++; *to++ = *from++;
      *to++ = *from++; *to++ = *from++; *to++ = *from++; *to++ = *from++;
   }
   i = n & 7; while (i--) *to++ = *from++;
}

bool GeneralMatrix::reuse()
{
   if (tag_val < -1)                 // borrowed storage
   {
      if (storage)
      {
         REPORT
         Real* s = new Real [storage]; MatrixErrorNoSpace(s);
         MONITOR_REAL_NEW("Make     (reuse)",storage,s)
         newmat_block_copy(storage, store, s); store = s;
      }
      else { REPORT MiniCleanUp(); }                // CleanUp
      tag_val = 0; return true;
   }
   if (tag_val < 0 ) { REPORT return false; }
   if (tag_val <= 1 )  { REPORT return true; }
   REPORT tag_val--; return false;
}

Real* GeneralMatrix::GetStore()
{
   if (tag_val<0 || tag_val>1)
   {
      Real* s;
      if (storage)
      {
         s = new Real [storage]; MatrixErrorNoSpace(s);
         MONITOR_REAL_NEW("Make  (GetStore)",storage,s)
         newmat_block_copy(storage, store, s);
      }
      else s = 0;
      if (tag_val > 1) { REPORT tag_val--; }
      else if (tag_val < -1) { REPORT store = 0; delete this; } // borrowed store
      else { REPORT }
      return s;
   }
   Real* s = store;                             // cleanup - done later
   if (tag_val==0) { REPORT store = 0; delete this; }
   else { REPORT  MiniCleanUp(); }
   return s;
}

void GeneralMatrix::GetMatrix(const GeneralMatrix* gmx)
{
   REPORT  tag_val=-1; nrows_val=gmx->Nrows(); ncols_val=gmx->Ncols();
   storage=gmx->storage; SetParameters(gmx);
   store=((GeneralMatrix*)gmx)->GetStore();
}

GeneralMatrix* GeneralMatrix::BorrowStore(GeneralMatrix* gmx, MatrixType mt)
// Copy storage of *this to storage of *gmx. Then convert to type mt.
// If mt == 0 just let *gmx point to storage of *this if tag_val==-1.
{
   if (!mt)
   {
      if (tag_val == -1) { REPORT gmx->tag_val = -2; gmx->store = store; }
      else { REPORT gmx->tag_val = 0; gmx->store = GetStore(); }
   }
   else if (Compare(gmx->type(),mt))
   { REPORT  gmx->tag_val = 0; gmx->store = GetStore(); }
   else
   {
      REPORT gmx->tag_val = -2; gmx->store = store;
      gmx = gmx->Evaluate(mt); gmx->tag_val = 0; tDelete();
   }

   return gmx;
}

void GeneralMatrix::Eq(const BaseMatrix& X, MatrixType mt)
// Count number of references to this in X.
// If zero delete storage in this;
// otherwise tag this to show when storage can be deleted
// evaluate X and copy to this
{
#ifdef DO_SEARCH
   int counter=X.search(this);
   if (counter==0)
   {
      REPORT
      if (store)
      {
         MONITOR_REAL_DELETE("Free (operator=)",storage,store)
         REPORT delete [] store; storage = 0; store = 0;
      }
   }
   else { REPORT Release(counter); }
   GeneralMatrix* gmx = ((BaseMatrix&)X).Evaluate(mt);
   if (gmx!=this) { REPORT GetMatrix(gmx); }
   else { REPORT }
   Protect();
#else
   GeneralMatrix* gmx = ((BaseMatrix&)X).Evaluate(mt);
   if (gmx!=this)
   {
      REPORT
      if (store)
      {
         MONITOR_REAL_DELETE("Free (operator=)",storage,store)
         REPORT delete [] store; storage = 0; store = 0;
      }
      GetMatrix(gmx);
   }
   else { REPORT }
   Protect();
#endif
}

// version with no conversion
void GeneralMatrix::Eq(const GeneralMatrix& X)
{
   GeneralMatrix* gmx = (GeneralMatrix*)&X;
   if (gmx!=this)
   {
      REPORT
      if (store)
      {
         MONITOR_REAL_DELETE("Free (operator=)",storage,store)
         REPORT delete [] store; storage = 0; store = 0;
      }
      GetMatrix(gmx);
   }
   else { REPORT }
   Protect();
}

// version to work with operator<<
void GeneralMatrix::Eq(const BaseMatrix& X, MatrixType mt, bool ldok)
{
   REPORT
   if (ldok) mt.SetDataLossOK();
   Eq(X, mt);
}

void GeneralMatrix::Eq2(const BaseMatrix& X, MatrixType mt)
// a cut down version of Eq for use with += etc.
// we know BaseMatrix points to two GeneralMatrix objects,
// the first being this (may be the same).
// we know tag_val has been set correctly in each.
{
   GeneralMatrix* gmx = ((BaseMatrix&)X).Evaluate(mt);
   if (gmx!=this) { REPORT GetMatrix(gmx); }  // simplify GetMatrix ?
   else { REPORT }
   Protect();
}

void GeneralMatrix::inject(const GeneralMatrix& X)
// copy stored values of X; otherwise leave els of *this unchanged
{
   REPORT
   Tracer tr("inject");
   if (nrows_val != X.nrows_val || ncols_val != X.ncols_val)
      Throw(IncompatibleDimensionsException());
   MatrixRow mr((GeneralMatrix*)&X, LoadOnEntry);
   MatrixRow mrx(this, LoadOnEntry+StoreOnExit+DirectPart);
   int i=nrows_val;
   while (i--) { mrx.Inject(mr); mrx.Next(); mr.Next(); }
}

// ************* checking for data loss during conversion *******************/

bool Compare(const MatrixType& source, MatrixType& destination)
{
   if (!destination) { destination=source; return true; }
   if (destination==source) return true;
   if (!destination.DataLossOK && !(destination>=source))
      Throw(ProgramException("Illegal Conversion", source, destination));
   return false;
}

// ************* Make a copy of a matrix on the heap *********************/

GeneralMatrix* Matrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new Matrix(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* SquareMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new SquareMatrix(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* SymmetricMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new SymmetricMatrix(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* UpperTriangularMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new UpperTriangularMatrix(*this);
   MatrixErrorNoSpace(gm); return gm;
}

GeneralMatrix* LowerTriangularMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new LowerTriangularMatrix(*this);
   MatrixErrorNoSpace(gm); return gm;
}

GeneralMatrix* DiagonalMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new DiagonalMatrix(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* RowVector::Image() const
{
   REPORT
   GeneralMatrix* gm = new RowVector(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* ColumnVector::Image() const
{
   REPORT
   GeneralMatrix* gm = new ColumnVector(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* nricMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new nricMatrix(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* IdentityMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new IdentityMatrix(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* CroutMatrix::Image() const
{
   REPORT
   GeneralMatrix* gm = new CroutMatrix(*this); MatrixErrorNoSpace(gm);
   return gm;
}

GeneralMatrix* GeneralMatrix::Image() const
{
   bool dummy = true;
   if (dummy)                                   // get rid of warning message
      Throw(InternalException("Cannot apply Image to this matrix type"));
   return 0;
}


// *********************** nricMatrix routines *****************************/

void nricMatrix::MakeRowPointer()
{
   REPORT
   if (nrows_val > 0)
   {
      row_pointer = new Real* [nrows_val]; MatrixErrorNoSpace(row_pointer);
      Real* s = Store() - 1; int i = nrows_val; Real** rp = row_pointer;
      if (i) for (;;) { *rp++ = s; if (!(--i)) break; s+=ncols_val; }
   }
   else row_pointer = 0;
}

void nricMatrix::DeleteRowPointer()
   { REPORT if (nrows_val) delete [] row_pointer; }

void GeneralMatrix::CheckStore() const
{
   REPORT
   if (!store)
      Throw(ProgramException("NRIC accessing matrix with unset dimensions"));
}


// *************************** CleanUp routines *****************************/

void GeneralMatrix::cleanup()
{
   // set matrix dimensions to zero, delete storage
   REPORT
   if (store && storage)
   {
      MONITOR_REAL_DELETE("Free (cleanup)    ",storage,store)
      REPORT delete [] store;
   }
   store=0; storage=0; nrows_val=0; ncols_val=0; tag_val = -1;
}

void nricMatrix::cleanup()
   { REPORT DeleteRowPointer(); GeneralMatrix::cleanup(); }

void nricMatrix::MiniCleanUp()
   { REPORT DeleteRowPointer(); GeneralMatrix::MiniCleanUp(); }

void RowVector::cleanup()
   { REPORT GeneralMatrix::cleanup(); nrows_val=1; }

void ColumnVector::cleanup()
   { REPORT GeneralMatrix::cleanup(); ncols_val=1; }

void CroutMatrix::cleanup()
{
   REPORT
   if (nrows_val) delete [] indx;
   GeneralMatrix::cleanup();
}

void CroutMatrix::MiniCleanUp()
{
   REPORT
   if (nrows_val) delete [] indx;
   GeneralMatrix::MiniCleanUp();
}

void BandLUMatrix::cleanup()
{
   REPORT
   if (nrows_val) delete [] indx;
   if (storage2) delete [] store2;
   GeneralMatrix::cleanup();
}

void BandLUMatrix::MiniCleanUp()
{
   REPORT
   if (nrows_val) delete [] indx;
   if (storage2) delete [] store2;
   GeneralMatrix::MiniCleanUp();
}

// ************************ simple integer array class ***********************

// construct a new array of length xn. Check that xn is non-negative and
// that space is available

SimpleIntArray::SimpleIntArray(int xn) : n(xn)
{
   if (n < 0) Throw(Logic_error("invalid array length"));
   else if (n == 0) { REPORT  a = 0; }
   else { REPORT  a = new int [n]; if (!a) Throw(Bad_alloc()); }
}

// destroy an array - return its space to memory

SimpleIntArray::~SimpleIntArray() { REPORT  if (a) delete [] a; }

// access an element of an array; return a "reference" so elements
// can be modified.
// check index is within range
// in this array class the index runs from 0 to n-1

int& SimpleIntArray::operator[](int i)
{
   REPORT
   if (i < 0 || i >= n) Throw(Logic_error("array index out of range"));
   return a[i];
}

// same thing again but for arrays declared constant so we can't
// modify its elements

int SimpleIntArray::operator[](int i) const
{
   REPORT
   if (i < 0 || i >= n) Throw(Logic_error("array index out of range"));
   return a[i];
}

// set all the elements equal to a given value

void SimpleIntArray::operator=(int ai)
   { REPORT  for (int i = 0; i < n; i++) a[i] = ai; }

// set the elements equal to those of another array.
// now allow length to be changed

void SimpleIntArray::operator=(const SimpleIntArray& b)
{
   REPORT
   if (b.n != n) resize(b.n);
   for (int i = 0; i < n; i++) a[i] = b.a[i];
}

// construct a new array equal to an existing array
// check that space is available

SimpleIntArray::SimpleIntArray(const SimpleIntArray& b) : n(b.n)
{
   if (n == 0) { REPORT  a = 0; }
   else
   {
      REPORT
      a = new int [n]; if (!a) Throw(Bad_alloc());
      for (int i = 0; i < n; i++) a[i] = b.a[i];
   }
}

// change the size of an array; optionally copy data from old array to
// new array

void SimpleIntArray::resize(int n1, bool keep)
{
   if (n1 == n) { REPORT  return; }
   else if (n1 == 0) { REPORT  n = 0; delete [] a; a = 0; }
   else if (n == 0)
   {
      REPORT
      a = new int [n1]; if (!a) Throw(Bad_alloc());
      n = n1;
      if (keep) operator=(0);
   }
   else
   {
      int* a1 = a;
      if (keep)
      {
         REPORT
         int i;
         a = new int [n1]; if (!a) Throw(Bad_alloc());
         if (n > n1) n = n1;
         else for (i = n; i < n1; i++) a[i] = 0;
         for (i = 0; i < n; i++) a[i] = a1[i];
         n = n1; delete [] a1;
      }
      else
      {
         REPORT  n = n1; delete [] a1;
         a = new int [n]; if (!a) Throw(Bad_alloc());
      }
   }
}

//************************** swap values ********************************

// swap values

void GeneralMatrix::swap(GeneralMatrix& gm)
{
   REPORT
   int t;
   t = tag_val; tag_val = gm.tag_val; gm.tag_val = t;
   t = nrows_val; nrows_val = gm.nrows_val; gm.nrows_val = t;
   t = ncols_val; ncols_val = gm.ncols_val; gm.ncols_val = t;
   t = storage; storage = gm.storage; gm.storage = t;
   Real* s = store; store = gm.store; gm.store = s;
}
   
void nricMatrix::swap(nricMatrix& gm)
{
   REPORT
   GeneralMatrix::swap((GeneralMatrix&)gm);
   Real** rp = row_pointer; row_pointer = gm.row_pointer; gm.row_pointer = rp;
}

void CroutMatrix::swap(CroutMatrix& gm)
{
   REPORT
   GeneralMatrix::swap((GeneralMatrix&)gm);
   int* i = indx; indx = gm.indx; gm.indx = i;
   bool b;
   b = d; d = gm.d; gm.d = b;
   b = sing; sing = gm.sing; gm.sing = b;
}

void BandMatrix::swap(BandMatrix& gm)
{
   REPORT
   GeneralMatrix::swap((GeneralMatrix&)gm);
   int i;
   i = lower_val; lower_val = gm.lower_val; gm.lower_val = i;
   i = upper_val; upper_val = gm.upper_val; gm.upper_val = i;
}

void SymmetricBandMatrix::swap(SymmetricBandMatrix& gm)
{
   REPORT
   GeneralMatrix::swap((GeneralMatrix&)gm);
   int i;
   i = lower_val; lower_val = gm.lower_val; gm.lower_val = i;
}

void BandLUMatrix::swap(BandLUMatrix& gm)
{
   REPORT
   GeneralMatrix::swap((GeneralMatrix&)gm);
   int* i = indx; indx = gm.indx; gm.indx = i;
   bool b;
   b = d; d = gm.d; gm.d = b;
   b = sing; sing = gm.sing; gm.sing = b;
   int m;
   m = storage2; storage2 = gm.storage2; gm.storage2 = m;
   m = m1; m1 = gm.m1; gm.m1 = m;
   m = m2; m2 = gm.m2; gm.m2 = m;
   Real* s = store2; store2 = gm.store2; gm.store2 = s;
}

void GenericMatrix::swap(GenericMatrix& x)
{
   REPORT
   GeneralMatrix* tgm = gm; gm = x.gm; x.gm = tgm;
}

// ********************** C subscript classes ****************************

RealStarStar::RealStarStar(Matrix& A)
{
   REPORT
   Tracer tr("RealStarStar");
   int n = A.ncols();
   int m = A.nrows();
   a = new Real*[m];
   MatrixErrorNoSpace(a);
   Real* d = A.data();
   for (int i = 0; i < m; ++i) a[i] = d + i * n;
} 

ConstRealStarStar::ConstRealStarStar(const Matrix& A)
{
   REPORT
   Tracer tr("ConstRealStarStar");
   int n = A.ncols();
   int m = A.nrows();
   a = new const Real*[m];
   MatrixErrorNoSpace(a);
   const Real* d = A.data();
   for (int i = 0; i < m; ++i) a[i] = d + i * n;
} 



#ifdef use_namespace
}
#endif

