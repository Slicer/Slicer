//$$ newmat3.cpp        Matrix get and restore rows and columns

// Copyright (C) 1991,2,3,4: R B Davies

//#define WANT_STREAM

#include "include.h"

#include "newmat.h"
#include "newmatrc.h"

#ifdef use_namespace
namespace NEWMAT {
#endif


#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,3); ++ExeCount; }
#else
#define REPORT {}
#endif

//#define MONITOR(what,storage,store)
//   { cout << what << " " << storage << " at " << (long)store << "\n"; }

#define MONITOR(what,store,storage) {}


// Control bits codes for GetRow, GetCol, RestoreRow, RestoreCol
//
// LoadOnEntry:
//    Load data into MatrixRow or Col dummy array under GetRow or GetCol
// StoreOnExit:
//    Restore data to original matrix under RestoreRow or RestoreCol
// DirectPart:
//    Load or restore only part directly stored; must be set with StoreOnExit
//    Still have decide how to handle this with symmetric
// StoreHere:
//    used in columns only - store data at supplied storage address;
//    used for GetCol, NextCol & RestoreCol. No need to fill out zeros
// HaveStore:
//    dummy array has been assigned (internal use only).

// For symmetric matrices, treat columns as rows unless StoreHere is set;
// then stick to columns as this will give better performance for doing
// inverses

// How components are used:

// Use rows wherever possible in preference to columns

// Columns without StoreHere are used in in-exact transpose, sum column,
// multiply a column vector, and maybe in future access to column,
// additional multiply functions, add transpose

// Columns with StoreHere are used in exact transpose (not symmetric matrices
// or vectors, load only)

// Columns with MatrixColX (Store to full column) are used in inverse and solve

// Functions required for each matrix class

// GetRow(MatrixRowCol& mrc)
// GetCol(MatrixRowCol& mrc)
// GetCol(MatrixColX& mrc)
// RestoreRow(MatrixRowCol& mrc)
// RestoreCol(MatrixRowCol& mrc)
// RestoreCol(MatrixColX& mrc)
// NextRow(MatrixRowCol& mrc)
// NextCol(MatrixRowCol& mrc)
// NextCol(MatrixColX& mrc)

// The Restore routines assume StoreOnExit has already been checked
// Defaults for the Next routines are given below
// Assume cannot have both !DirectPart && StoreHere for MatrixRowCol routines


// Default NextRow and NextCol:
// will work as a default but need to override NextRow for efficiency

void GeneralMatrix::NextRow(MatrixRowCol& mrc)
{
   REPORT
   if (+(mrc.cw*StoreOnExit)) { REPORT this->RestoreRow(mrc); }
   mrc.rowcol++;
   if (mrc.rowcol<nrows_val) { REPORT this->GetRow(mrc); }
   else { REPORT mrc.cw -= StoreOnExit; }
}

void GeneralMatrix::NextCol(MatrixRowCol& mrc)
{
   REPORT                                                // 423
   if (+(mrc.cw*StoreOnExit)) { REPORT this->RestoreCol(mrc); }
   mrc.rowcol++;
   if (mrc.rowcol<ncols_val) { REPORT this->GetCol(mrc); }
   else { REPORT mrc.cw -= StoreOnExit; }
}

void GeneralMatrix::NextCol(MatrixColX& mrc)
{
   REPORT                                                // 423
   if (+(mrc.cw*StoreOnExit)) { REPORT this->RestoreCol(mrc); }
   mrc.rowcol++;
   if (mrc.rowcol<ncols_val) { REPORT this->GetCol(mrc); }
   else { REPORT mrc.cw -= StoreOnExit; }
}


// routines for matrix

void Matrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=0; mrc.storage=mrc.length=ncols_val;
   mrc.data=store+mrc.rowcol*ncols_val;
}


void Matrix::GetCol(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=0; mrc.storage=mrc.length=nrows_val;
   if ( ncols_val==1 && !(mrc.cw*StoreHere) )      // ColumnVector
      { REPORT mrc.data=store; }
   else
   {
      Real* ColCopy;
      if ( !(mrc.cw*(HaveStore+StoreHere)) )
      {
         REPORT
         ColCopy = new Real [nrows_val]; MatrixErrorNoSpace(ColCopy);
         MONITOR_REAL_NEW("Make (MatGetCol)",nrows_val,ColCopy)
         mrc.data = ColCopy; mrc.cw += HaveStore;
      }
      else { REPORT ColCopy = mrc.data; }
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* Mstore = store+mrc.rowcol; int i=nrows_val;
         //while (i--) { *ColCopy++ = *Mstore; Mstore+=ncols_val; }
         if (i) for (;;)
            { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore+=ncols_val; }
      }
   }
}

void Matrix::GetCol(MatrixColX& mrc)
{
   REPORT
   mrc.skip=0; mrc.storage=nrows_val; mrc.length=nrows_val;
   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT  Real* ColCopy = mrc.data;
      Real* Mstore = store+mrc.rowcol; int i=nrows_val;
      //while (i--) { *ColCopy++ = *Mstore; Mstore+=ncols_val; }
      if (i) for (;;)
          { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore+=ncols_val; }
   }
}

void Matrix::RestoreCol(MatrixRowCol& mrc)
{
   // always check StoreOnExit before calling RestoreCol
   REPORT                                   // 429
   if (+(mrc.cw*HaveStore))
   {
      REPORT                                // 426
      Real* Mstore = store+mrc.rowcol; int i=nrows_val;
      Real* Cstore = mrc.data;
      // while (i--) { *Mstore = *Cstore++; Mstore+=ncols_val; }
      if (i) for (;;)
          { *Mstore = *Cstore++; if (!(--i)) break; Mstore+=ncols_val; }
   }
}

void Matrix::RestoreCol(MatrixColX& mrc)
{
   REPORT
   Real* Mstore = store+mrc.rowcol; int i=nrows_val; Real* Cstore = mrc.data;
   // while (i--) { *Mstore = *Cstore++; Mstore+=ncols_val; }
   if (i) for (;;)
      { *Mstore = *Cstore++; if (!(--i)) break; Mstore+=ncols_val; }
}

void Matrix::NextRow(MatrixRowCol& mrc) { REPORT mrc.IncrMat(); }  // 1808

void Matrix::NextCol(MatrixRowCol& mrc)
{
   REPORT                                        // 632
   if (+(mrc.cw*StoreOnExit)) { REPORT RestoreCol(mrc); }
   mrc.rowcol++;
   if (mrc.rowcol<ncols_val)
   {
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* ColCopy = mrc.data;
         Real* Mstore = store+mrc.rowcol; int i=nrows_val;
         //while (i--) { *ColCopy++ = *Mstore; Mstore+=ncols_val; }
         if (i) for (;;)
            { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore+=ncols_val; }
      }
   }
   else { REPORT mrc.cw -= StoreOnExit; }
}

void Matrix::NextCol(MatrixColX& mrc)
{
   REPORT
   if (+(mrc.cw*StoreOnExit)) { REPORT RestoreCol(mrc); }
   mrc.rowcol++;
   if (mrc.rowcol<ncols_val)
   {
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* ColCopy = mrc.data;
         Real* Mstore = store+mrc.rowcol; int i=nrows_val;
         // while (i--) { *ColCopy++ = *Mstore; Mstore+=ncols_val; }
         if (i) for (;;)
            { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore+=ncols_val; }
      }
   }
   else { REPORT mrc.cw -= StoreOnExit; }
}

// routines for diagonal matrix

void DiagonalMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=mrc.rowcol; mrc.storage=1;
   mrc.data=store+mrc.skip; mrc.length=ncols_val;
}

void DiagonalMatrix::GetCol(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=mrc.rowcol; mrc.storage=1; mrc.length=nrows_val;
   if (+(mrc.cw*StoreHere))              // should not happen
      Throw(InternalException("DiagonalMatrix::GetCol(MatrixRowCol&)"));
   else  { REPORT mrc.data=store+mrc.skip; }
                                                      // not accessed
}

void DiagonalMatrix::GetCol(MatrixColX& mrc)
{
   REPORT
   mrc.skip=mrc.rowcol; mrc.storage=1; mrc.length=nrows_val;
   mrc.data = mrc.store+mrc.skip;
   *(mrc.data)=*(store+mrc.skip);
}

void DiagonalMatrix::NextRow(MatrixRowCol& mrc) { REPORT mrc.IncrDiag(); }
                        // 800

void DiagonalMatrix::NextCol(MatrixRowCol& mrc) { REPORT mrc.IncrDiag(); }
                        // not accessed

void DiagonalMatrix::NextCol(MatrixColX& mrc)
{
   REPORT
   if (+(mrc.cw*StoreOnExit))
      { REPORT *(store+mrc.rowcol)=*(mrc.data); }
   mrc.IncrDiag();
   int t1 = +(mrc.cw*LoadOnEntry);
   if (t1 && mrc.rowcol < ncols_val)
      { REPORT *(mrc.data)=*(store+mrc.rowcol); }
}

// routines for upper triangular matrix

void UpperTriangularMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   int row = mrc.rowcol; mrc.skip=row; mrc.length=ncols_val;
   mrc.storage=ncols_val-row; mrc.data=store+(row*(2*ncols_val-row+1))/2;
}


void UpperTriangularMatrix::GetCol(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=0; int i=mrc.rowcol+1; mrc.storage=i;
   mrc.length=nrows_val; Real* ColCopy;
   if ( !(mrc.cw*(StoreHere+HaveStore)) )
   {
      REPORT                                              // not accessed
      ColCopy = new Real [nrows_val]; MatrixErrorNoSpace(ColCopy);
      MONITOR_REAL_NEW("Make (UT GetCol)",nrows_val,ColCopy)
      mrc.data = ColCopy; mrc.cw += HaveStore;
   }
   else { REPORT ColCopy = mrc.data; }
   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT
      Real* Mstore = store+mrc.rowcol; int j = ncols_val;
      // while (i--) { *ColCopy++ = *Mstore; Mstore += --j; }
      if (i) for (;;)
         { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += --j; }
   }
}

void UpperTriangularMatrix::GetCol(MatrixColX& mrc)
{
   REPORT
   mrc.skip=0; int i=mrc.rowcol+1; mrc.storage=i;
   mrc.length=nrows_val;
   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT
      Real* ColCopy = mrc.data;
      Real* Mstore = store+mrc.rowcol; int j = ncols_val;
      // while (i--) { *ColCopy++ = *Mstore; Mstore += --j; }
      if (i) for (;;)
         { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += --j; }
   }
}

void UpperTriangularMatrix::RestoreCol(MatrixRowCol& mrc)
{
  REPORT
  Real* Mstore = store+mrc.rowcol; int i=mrc.rowcol+1; int j = ncols_val;
  Real* Cstore = mrc.data;
  // while (i--) { *Mstore = *Cstore++; Mstore += --j; }
  if (i) for (;;)
     { *Mstore = *Cstore++; if (!(--i)) break; Mstore += --j; }
}

void UpperTriangularMatrix::NextRow(MatrixRowCol& mrc) { REPORT mrc.IncrUT(); }
                                                      // 722

// routines for lower triangular matrix

void LowerTriangularMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   int row=mrc.rowcol; mrc.skip=0; mrc.storage=row+1; mrc.length=ncols_val;
   mrc.data=store+(row*(row+1))/2;
}

void LowerTriangularMatrix::GetCol(MatrixRowCol& mrc)
{
   REPORT
   int col=mrc.rowcol; mrc.skip=col; mrc.length=nrows_val;
   int i=nrows_val-col; mrc.storage=i; Real* ColCopy;
   if ( +(mrc.cw*(StoreHere+HaveStore)) )
      { REPORT  ColCopy = mrc.data; }
   else
   {
      REPORT                                            // not accessed
      ColCopy = new Real [nrows_val]; MatrixErrorNoSpace(ColCopy);
      MONITOR_REAL_NEW("Make (LT GetCol)",nrows_val,ColCopy)
      mrc.cw += HaveStore; mrc.data = ColCopy;
   }

   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT
      Real* Mstore = store+(col*(col+3))/2;
      // while (i--) { *ColCopy++ = *Mstore; Mstore += ++col; }
      if (i) for (;;)
         { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += ++col; }
   }
}

void LowerTriangularMatrix::GetCol(MatrixColX& mrc)
{
   REPORT
   int col=mrc.rowcol; mrc.skip=col; mrc.length=nrows_val;
   int i=nrows_val-col; mrc.storage=i; mrc.data = mrc.store + col;

   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT  Real* ColCopy = mrc.data;
      Real* Mstore = store+(col*(col+3))/2;
      // while (i--) { *ColCopy++ = *Mstore; Mstore += ++col; }
      if (i) for (;;)
         { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += ++col; }
   }
}

void LowerTriangularMatrix::RestoreCol(MatrixRowCol& mrc)
{
   REPORT
   int col=mrc.rowcol; Real* Cstore = mrc.data;
   Real* Mstore = store+(col*(col+3))/2; int i=nrows_val-col;
   //while (i--) { *Mstore = *Cstore++; Mstore += ++col; }
   if (i) for (;;)
      { *Mstore = *Cstore++; if (!(--i)) break; Mstore += ++col; }
}

void LowerTriangularMatrix::NextRow(MatrixRowCol& mrc) { REPORT mrc.IncrLT(); }
                                                         //712

// routines for symmetric matrix

void SymmetricMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT                                                //571
   mrc.skip=0; int row=mrc.rowcol; mrc.length=ncols_val;
   if (+(mrc.cw*DirectPart))
      { REPORT mrc.storage=row+1; mrc.data=store+(row*(row+1))/2; }
   else
   {
      // do not allow StoreOnExit and !DirectPart
      if (+(mrc.cw*StoreOnExit))
         Throw(InternalException("SymmetricMatrix::GetRow(MatrixRowCol&)"));
      mrc.storage=ncols_val; Real* RowCopy;
      if (!(mrc.cw*HaveStore))
      {
         REPORT
         RowCopy = new Real [ncols_val]; MatrixErrorNoSpace(RowCopy);
         MONITOR_REAL_NEW("Make (SymGetRow)",ncols_val,RowCopy)
         mrc.cw += HaveStore; mrc.data = RowCopy;
      }
      else { REPORT RowCopy = mrc.data; }
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT                                         // 544
         Real* Mstore = store+(row*(row+1))/2; int i = row;
         while (i--) *RowCopy++ = *Mstore++;
         i = ncols_val-row;
         // while (i--) { *RowCopy++ = *Mstore; Mstore += ++row; }
         if (i) for (;;)
            { *RowCopy++ = *Mstore; if (!(--i)) break; Mstore += ++row; }
      }
   }
}

void SymmetricMatrix::GetCol(MatrixRowCol& mrc)
{
   // do not allow StoreHere
   if (+(mrc.cw*StoreHere))
      Throw(InternalException("SymmetricMatrix::GetCol(MatrixRowCol&)"));

   int col=mrc.rowcol; mrc.length=nrows_val;
   REPORT
   mrc.skip=0;
   if (+(mrc.cw*DirectPart))    // actually get row ??
      { REPORT mrc.storage=col+1; mrc.data=store+(col*(col+1))/2; }
   else
   {
      // do not allow StoreOnExit and !DirectPart
      if (+(mrc.cw*StoreOnExit))
         Throw(InternalException("SymmetricMatrix::GetCol(MatrixRowCol&)"));

      mrc.storage=ncols_val; Real* ColCopy;
      if ( +(mrc.cw*HaveStore)) { REPORT ColCopy = mrc.data; }
      else
      {
         REPORT                                      // not accessed
         ColCopy = new Real [ncols_val]; MatrixErrorNoSpace(ColCopy);
         MONITOR_REAL_NEW("Make (SymGetCol)",ncols_val,ColCopy)
         mrc.cw += HaveStore; mrc.data = ColCopy;
      }
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* Mstore = store+(col*(col+1))/2; int i = col;
         while (i--) *ColCopy++ = *Mstore++;
         i = ncols_val-col;
         // while (i--) { *ColCopy++ = *Mstore; Mstore += ++col; }
         if (i) for (;;)
            { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += ++col; }
      }
   }
}

void SymmetricMatrix::GetCol(MatrixColX& mrc)
{
   int col=mrc.rowcol; mrc.length=nrows_val;
   if (+(mrc.cw*DirectPart))
   {
      REPORT
      mrc.skip=col; int i=nrows_val-col; mrc.storage=i;
      mrc.data = mrc.store+col;
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT                           // not accessed
         Real* ColCopy = mrc.data;
         Real* Mstore = store+(col*(col+3))/2;
         // while (i--) { *ColCopy++ = *Mstore; Mstore += ++col; }
         if (i) for (;;)
            { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += ++col; }
      }
   }
   else
   {
      REPORT
      // do not allow StoreOnExit and !DirectPart
      if (+(mrc.cw*StoreOnExit))
         Throw(InternalException("SymmetricMatrix::GetCol(MatrixColX&)"));

      mrc.skip=0; mrc.storage=ncols_val;
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* ColCopy = mrc.data;
         Real* Mstore = store+(col*(col+1))/2; int i = col;
         while (i--) *ColCopy++ = *Mstore++;
         i = ncols_val-col;
         // while (i--) { *ColCopy++ = *Mstore; Mstore += ++col; }
         if (i) for (;;)
            { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += ++col; }
      }
   }
}

// Do not need RestoreRow because we do not allow !DirectPart && StoreOnExit

void SymmetricMatrix::RestoreCol(MatrixColX& mrc)
{
   REPORT
   // Really do restore column
   int col=mrc.rowcol; Real* Cstore = mrc.data;
   Real* Mstore = store+(col*(col+3))/2; int i = nrows_val-col;
   // while (i--) { *Mstore = *Cstore++; Mstore+= ++col; }
   if (i) for (;;)
      { *Mstore = *Cstore++; if (!(--i)) break; Mstore+= ++col; }

}

// routines for row vector

void RowVector::GetCol(MatrixRowCol& mrc)
{
   REPORT
   // do not allow StoreHere
   if (+(mrc.cw*StoreHere))
      Throw(InternalException("RowVector::GetCol(MatrixRowCol&)"));

   mrc.skip=0; mrc.storage=1; mrc.length=nrows_val;
   mrc.data = store+mrc.rowcol;

}

void RowVector::GetCol(MatrixColX& mrc)
{
   REPORT
   mrc.skip=0; mrc.storage=1; mrc.length=nrows_val;
   if (mrc.cw >= LoadOnEntry)
      { REPORT *(mrc.data) = *(store+mrc.rowcol); }

}

void RowVector::NextCol(MatrixRowCol& mrc)
{ REPORT mrc.rowcol++; mrc.data++; }

void RowVector::NextCol(MatrixColX& mrc)
{
   if (+(mrc.cw*StoreOnExit)) { REPORT *(store+mrc.rowcol)=*(mrc.data); }

   mrc.rowcol++;
   if (mrc.rowcol < ncols_val)
   {
      if (+(mrc.cw*LoadOnEntry)) { REPORT *(mrc.data)=*(store+mrc.rowcol); }
   }
   else { REPORT mrc.cw -= StoreOnExit; }
}

void RowVector::RestoreCol(MatrixColX& mrc)
   { REPORT *(store+mrc.rowcol)=*(mrc.data); }           // not accessed


// routines for band matrices

void BandMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   int r = mrc.rowcol; int w = lower_val+1+upper_val; mrc.length=ncols_val;
   int s = r-lower_val;
   if (s<0) { mrc.data = store+(r*w-s); w += s; s = 0; }
   else mrc.data = store+r*w;
   mrc.skip = s; s += w-ncols_val; if (s>0) w -= s; mrc.storage = w;
}

// should make special versions of this for upper and lower band matrices

void BandMatrix::NextRow(MatrixRowCol& mrc)
{
   REPORT
   int r = ++mrc.rowcol;
   if (r<=lower_val) { mrc.storage++; mrc.data += lower_val+upper_val; }
   else  { mrc.skip++; mrc.data += lower_val+upper_val+1; }
   if (r>=ncols_val-upper_val) mrc.storage--;
}

void BandMatrix::GetCol(MatrixRowCol& mrc)
{
   REPORT
   int c = mrc.rowcol; int n = lower_val+upper_val; int w = n+1;
   mrc.length=nrows_val; Real* ColCopy;
   int b; int s = c-upper_val;
   if (s<=0) { w += s; s = 0; b = c+lower_val; } else b = s*w+n;
   mrc.skip = s; s += w-nrows_val; if (s>0) w -= s; mrc.storage = w;
   if ( +(mrc.cw*(StoreHere+HaveStore)) )
      { REPORT ColCopy = mrc.data; }
   else
   {
      REPORT
      ColCopy = new Real [n+1]; MatrixErrorNoSpace(ColCopy);
      MONITOR_REAL_NEW("Make (BMGetCol)",n+1,ColCopy)
      mrc.cw += HaveStore; mrc.data = ColCopy;
   }

   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT
      Real* Mstore = store+b;
      // while (w--) { *ColCopy++ = *Mstore; Mstore+=n; }
      if (w) for (;;)
         { *ColCopy++ = *Mstore; if (!(--w)) break; Mstore+=n; }
   }
}

void BandMatrix::GetCol(MatrixColX& mrc)
{
   REPORT
   int c = mrc.rowcol; int n = lower_val+upper_val; int w = n+1;
   mrc.length=nrows_val; int b; int s = c-upper_val;
   if (s<=0) { w += s; s = 0; b = c+lower_val; } else b = s*w+n;
   mrc.skip = s; s += w-nrows_val; if (s>0) w -= s; mrc.storage = w;
   mrc.data = mrc.store+mrc.skip;

   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT
      Real* ColCopy = mrc.data; Real* Mstore = store+b;
      // while (w--) { *ColCopy++ = *Mstore; Mstore+=n; }
      if (w) for (;;)
         { *ColCopy++ = *Mstore; if (!(--w)) break; Mstore+=n; }
   }
}

void BandMatrix::RestoreCol(MatrixRowCol& mrc)
{
   REPORT
   int c = mrc.rowcol; int n = lower_val+upper_val; int s = c-upper_val;
   Real* Mstore = store + ((s<=0) ? c+lower_val : s*n+s+n);
   Real* Cstore = mrc.data;
   int w = mrc.storage;
   // while (w--) { *Mstore = *Cstore++; Mstore += n; }
   if (w) for (;;)
      { *Mstore = *Cstore++; if (!(--w)) break; Mstore += n; }
}

// routines for symmetric band matrix

void SymmetricBandMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   int r=mrc.rowcol; int s = r-lower_val; int w1 = lower_val+1; int o = r*w1;
   mrc.length = ncols_val;
   if (s<0) { w1 += s; o -= s; s = 0; }
   mrc.skip = s;

   if (+(mrc.cw*DirectPart))
      { REPORT  mrc.data = store+o; mrc.storage = w1; }
   else
   {
      // do not allow StoreOnExit and !DirectPart
      if (+(mrc.cw*StoreOnExit))
         Throw(InternalException("SymmetricBandMatrix::GetRow(MatrixRowCol&)"));
      int w = w1+lower_val; s += w-ncols_val; Real* RowCopy;
      if (s>0) w -= s; mrc.storage = w; int w2 = w-w1;
      if (!(mrc.cw*HaveStore))
      {
         REPORT
         RowCopy = new Real [2*lower_val+1]; MatrixErrorNoSpace(RowCopy);
         MONITOR_REAL_NEW("Make (SmBGetRow)",2*lower_val+1,RowCopy)
         mrc.cw += HaveStore; mrc.data = RowCopy;
      }
      else { REPORT  RowCopy = mrc.data; }

      if (+(mrc.cw*LoadOnEntry) && ncols_val > 0)
      {
         REPORT
         Real* Mstore = store+o;
         while (w1--) *RowCopy++ = *Mstore++;
         Mstore--;
         while (w2--) { Mstore += lower_val; *RowCopy++ = *Mstore; }
      }
   }
}

void SymmetricBandMatrix::GetCol(MatrixRowCol& mrc)
{
   // do not allow StoreHere
   if (+(mrc.cw*StoreHere))
      Throw(InternalException("SymmetricBandMatrix::GetCol(MatrixRowCol&)"));

   int c=mrc.rowcol; int w1 = lower_val+1; mrc.length=nrows_val;
   REPORT
   int s = c-lower_val; int o = c*w1;
   if (s<0) { w1 += s; o -= s; s = 0; }
   mrc.skip = s;

   if (+(mrc.cw*DirectPart))
   { REPORT  mrc.data = store+o; mrc.storage = w1; }
   else
   {
      // do not allow StoreOnExit and !DirectPart
      if (+(mrc.cw*StoreOnExit))
         Throw(InternalException("SymmetricBandMatrix::GetCol(MatrixRowCol&)"));
      int w = w1+lower_val; s += w-ncols_val; Real* ColCopy;
      if (s>0) w -= s; mrc.storage = w; int w2 = w-w1;

      if ( +(mrc.cw*HaveStore) ) { REPORT ColCopy = mrc.data; }
      else
      {
         REPORT ColCopy = new Real [2*lower_val+1]; MatrixErrorNoSpace(ColCopy);
         MONITOR_REAL_NEW("Make (SmBGetCol)",2*lower_val+1,ColCopy)
         mrc.cw += HaveStore; mrc.data = ColCopy;
      }

      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* Mstore = store+o;
         while (w1--) *ColCopy++ = *Mstore++;
         Mstore--;
         while (w2--) { Mstore += lower_val; *ColCopy++ = *Mstore; }
      }
   }
}

void SymmetricBandMatrix::GetCol(MatrixColX& mrc)
{
   int c=mrc.rowcol; int w1 = lower_val+1; mrc.length=nrows_val;
   if (+(mrc.cw*DirectPart))
   {
      REPORT
      int b = c*w1+lower_val;
      mrc.skip = c; c += w1-nrows_val; w1 -= c; mrc.storage = w1;
      Real* ColCopy = mrc.data = mrc.store+mrc.skip;
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* Mstore = store+b;
         // while (w1--) { *ColCopy++ = *Mstore; Mstore += lower; }
         if (w1) for (;;)
            { *ColCopy++ = *Mstore; if (!(--w1)) break; Mstore += lower_val; }
      }
   }
   else
   {
      REPORT
      // do not allow StoreOnExit and !DirectPart
      if (+(mrc.cw*StoreOnExit))
         Throw(InternalException("SymmetricBandMatrix::GetCol(MatrixColX&)"));
      int s = c-lower_val; int o = c*w1;
      if (s<0) { w1 += s; o -= s; s = 0; }
      mrc.skip = s;

      int w = w1+lower_val; s += w-ncols_val;
      if (s>0) w -= s; mrc.storage = w; int w2 = w-w1;

      Real* ColCopy = mrc.data = mrc.store+mrc.skip;

      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* Mstore = store+o;
         while (w1--) *ColCopy++ = *Mstore++;
         Mstore--;
         while (w2--) { Mstore += lower_val; *ColCopy++ = *Mstore; }
      }

   }
}

void SymmetricBandMatrix::RestoreCol(MatrixColX& mrc)
{
   REPORT
   int c = mrc.rowcol;
   Real* Mstore = store + c*lower_val+c+lower_val;
   Real* Cstore = mrc.data; int w = mrc.storage;
   // while (w--) { *Mstore = *Cstore++; Mstore += lower_val; }
   if (w) for (;;)
      { *Mstore = *Cstore++; if (!(--w)) break; Mstore += lower_val; }
}

// routines for identity matrix

void IdentityMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=mrc.rowcol; mrc.storage=1; mrc.data=store; mrc.length=ncols_val;
}

void IdentityMatrix::GetCol(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=mrc.rowcol; mrc.storage=1; mrc.length=nrows_val;
   if (+(mrc.cw*StoreHere))              // should not happen
      Throw(InternalException("IdentityMatrix::GetCol(MatrixRowCol&)"));
   else  { REPORT mrc.data=store; }
}

void IdentityMatrix::GetCol(MatrixColX& mrc)
{
   REPORT
   mrc.skip=mrc.rowcol; mrc.storage=1; mrc.length=nrows_val;
   mrc.data = mrc.store+mrc.skip; *(mrc.data)=*store;
}

void IdentityMatrix::NextRow(MatrixRowCol& mrc) { REPORT mrc.IncrId(); }

void IdentityMatrix::NextCol(MatrixRowCol& mrc) { REPORT mrc.IncrId(); }

void IdentityMatrix::NextCol(MatrixColX& mrc)
{
   REPORT
   if (+(mrc.cw*StoreOnExit)) { REPORT *store=*(mrc.data); }
   mrc.IncrDiag();            // must increase mrc.data so need IncrDiag
   int t1 = +(mrc.cw*LoadOnEntry);
   if (t1 && mrc.rowcol < ncols_val) { REPORT *(mrc.data)=*store; }
}




// *************************** destructors *******************************

MatrixRowCol::~MatrixRowCol()
{
   if (+(cw*HaveStore))
   {
      MONITOR_REAL_DELETE("Free    (RowCol)",-1,data)  // do not know length
      delete [] data;
   }
}

MatrixRow::~MatrixRow() { if (+(cw*StoreOnExit)) gm->RestoreRow(*this); }

MatrixCol::~MatrixCol() { if (+(cw*StoreOnExit)) gm->RestoreCol(*this); }

MatrixColX::~MatrixColX() { if (+(cw*StoreOnExit)) gm->RestoreCol(*this); }

#ifdef use_namespace
}
#endif

