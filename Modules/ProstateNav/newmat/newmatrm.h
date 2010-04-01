//$$newmatrm.h                            rectangular matrix operations

// Copyright (C) 1991,2,3,4: R B Davies

#ifndef NEWMATRM_LIB
#define NEWMATRM_LIB 0

#ifdef use_namespace
namespace NEWMAT {
#endif

// operations on rectangular matrices

class RectMatrixCol;

class RectMatrixRowCol
// a class for accessing rows and columns of rectangular matrices
{
protected:
#ifdef use_namespace              // to make namespace work
public:
#endif
   Real* store;                   // pointer to storage
   int n;                         // number of elements
   int spacing;                   // space between elements
   int shift;                     // space between cols or rows
   RectMatrixRowCol(Real* st, int nx, int sp, int sh)
      : store(st), n(nx), spacing(sp), shift(sh) {}
   void Reset(Real* st, int nx, int sp, int sh)
      { store=st; n=nx; spacing=sp; shift=sh; }
public:
   Real operator*(const RectMatrixRowCol&) const;         // dot product
   void AddScaled(const RectMatrixRowCol&, Real);         // add scaled
   void Divide(const RectMatrixRowCol&, Real);            // scaling
   void Divide(Real);                                     // scaling
   void Negate();                                         // change sign
   void Zero();                                           // zero row col
   Real& operator[](int i) { return *(store+i*spacing); } // element
   Real SumSquare() const;                                // sum of squares
   Real& First() { return *store; }                       // get first element
   void DownDiag() { store += (shift+spacing); n--; }
   void UpDiag() { store -= (shift+spacing); n++; }
   friend void ComplexScale(RectMatrixCol&, RectMatrixCol&, Real, Real);
   friend void Rotate(RectMatrixCol&, RectMatrixCol&, Real, Real);
   FREE_CHECK(RectMatrixRowCol)
};

class RectMatrixRow : public RectMatrixRowCol
{
public:
   RectMatrixRow(const Matrix&, int, int, int);
   RectMatrixRow(const Matrix&, int);
   void Reset(const Matrix&, int, int, int);
   void Reset(const Matrix&, int);
   Real& operator[](int i) { return *(store+i); }
   void Down() { store += shift; }
   void Right() { store++; n--; }
   void Up() { store -= shift; }
   void Left() { store--; n++; }
   FREE_CHECK(RectMatrixRow)
};

class RectMatrixCol : public RectMatrixRowCol
{
public:
   RectMatrixCol(const Matrix&, int, int, int);
   RectMatrixCol(const Matrix&, int);
   void Reset(const Matrix&, int, int, int);
   void Reset(const Matrix&, int);
   void Down() { store += spacing; n--; }
   void Right() { store++; }
   void Up() { store -= spacing; n++; }
   void Left() { store--; }
   friend void ComplexScale(RectMatrixCol&, RectMatrixCol&, Real, Real);
   friend void Rotate(RectMatrixCol&, RectMatrixCol&, Real, Real);
   FREE_CHECK(RectMatrixCol)
};

class RectMatrixDiag : public RectMatrixRowCol
{
public:
   RectMatrixDiag(const DiagonalMatrix& D)
      : RectMatrixRowCol(D.Store(), D.Nrows(), 1, 1) {}
   Real& operator[](int i) { return *(store+i); }
   void DownDiag() { store++; n--; }
   void UpDiag() { store--; n++; }
   FREE_CHECK(RectMatrixDiag)
};




inline RectMatrixRow::RectMatrixRow
   (const Matrix& M, int row, int skip, int length)
   : RectMatrixRowCol( M.Store()+row*M.Ncols()+skip, length, 1, M.Ncols() ) {}

inline RectMatrixRow::RectMatrixRow (const Matrix& M, int row)
   : RectMatrixRowCol( M.Store()+row*M.Ncols(), M.Ncols(), 1, M.Ncols() ) {}

inline RectMatrixCol::RectMatrixCol
   (const Matrix& M, int skip, int col, int length)
   : RectMatrixRowCol( M.Store()+col+skip*M.Ncols(), length, M.Ncols(), 1 ) {}

inline RectMatrixCol::RectMatrixCol (const Matrix& M, int col)
   : RectMatrixRowCol( M.Store()+col, M.Nrows(), M.Ncols(), 1 ) {}

inline Real square(Real x) { return x*x; }
inline Real sign(Real x, Real y)
   { return (y>=0) ? x : -x; }                    // assume x >=0


// Misc numerical things

Real pythag(Real f, Real g, Real& c, Real& s);

inline void GivensRotation(Real cGivens, Real sGivens, Real& x, Real& y)
{
   // allow for possibility &x = &y
   Real tmp0 = cGivens * x + sGivens * y;
   Real tmp1 = -sGivens * x + cGivens * y;
   x = tmp0; y = tmp1;
}
   
inline void GivensRotationR(Real cGivens, Real sGivens, Real& x, Real& y)
{
   // also change sign of y
   // allow for possibility &x = &y
   Real tmp0 = cGivens * x + sGivens * y;
   Real tmp1 = sGivens * x - cGivens * y;
   x = tmp0; y = tmp1;
}   





#ifdef use_namespace
}
#endif

#endif

// body file: newmatrm.cpp


