//$$ newmat1.cpp   Matrix type functions

// Copyright (C) 1991,2,3,4: R B Davies

//#define WANT_STREAM

#include "newmat.h"

#ifdef use_namespace
namespace NEWMAT {
#endif

#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,1); ++ExeCount; }
#else
#define REPORT {}
#endif


/************************* MatrixType functions *****************************/


// Skew needs more work <<<<<<<<<

// all operations to return MatrixTypes which correspond to valid types
// of matrices.
// Eg: if it has the Diagonal attribute, then it must also have
// Upper, Lower, Band, Square and Symmetric.


MatrixType MatrixType::operator*(const MatrixType& mt) const
{
   REPORT
   int a = attribute & mt.attribute & ~(Symmetric | Skew);
   a |= (a & Diagonal) * 63;                   // recognise diagonal
   return MatrixType(a);
}

MatrixType MatrixType::SP(const MatrixType& mt) const
// elementwise product
// Lower, Upper, Diag, Band if only one is
// Symmetric, Ones, Valid (and Real) if both are
// Need to include Lower & Upper => Diagonal
// Will need to include both Skew => Symmetric
{
   REPORT
   int a = ((attribute | mt.attribute) & ~(Symmetric + Skew + Valid + Ones))
      | (attribute & mt.attribute);
   if ((a & Lower) != 0  &&  (a & Upper) != 0) a |= Diagonal;
   if ((attribute & Skew) != 0)
   {
      if ((mt.attribute & Symmetric) != 0) a |= Skew;  
      if ((mt.attribute & Skew) != 0) { a &= ~Skew; a |= Symmetric; }
   }
   else if ((mt.attribute & Skew) != 0 && (attribute & Symmetric) != 0)
      a |= Skew;  
   a |= (a & Diagonal) * 63;                   // recognise diagonal
   return MatrixType(a);
}

MatrixType MatrixType::KP(const MatrixType& mt) const
// Kronecker product
// Lower, Upper, Diag, Symmetric, Band, Valid if both are
// Band if LHS is band & other is square 
// Ones is complicated so leave this out
{
   REPORT
   int a = (attribute & mt.attribute)  & ~Ones;
   if ((attribute & Band) != 0 && (mt.attribute & Square) != 0)
      a |= Band;
   //int a = ((attribute & mt.attribute) | (attribute & Band)) & ~Ones;

   return MatrixType(a);
}

MatrixType MatrixType::i() const               // type of inverse
{
   REPORT
   int a = attribute & ~(Band+LUDeco);
   a |= (a & Diagonal) * 63;                   // recognise diagonal
   return MatrixType(a);
}

MatrixType MatrixType::t() const
// swap lower and upper attributes
// assume Upper is in bit above Lower
{
   REPORT
   int a = attribute;
   a ^= (((a >> 1) ^ a) & Lower) * 3;
   return MatrixType(a);
}

MatrixType MatrixType::MultRHS() const
{
   REPORT
   // remove symmetric attribute unless diagonal
   return (attribute >= Dg) ? attribute : (attribute & ~Symmetric);
}

// this is used for deciding type of multiplication
bool Rectangular(MatrixType a, MatrixType b, MatrixType c)
{
   REPORT
   return
      ((a.attribute | b.attribute | c.attribute)
      & ~(MatrixType::Valid | MatrixType::Square)) == 0;
}

const char* MatrixType::value() const
{
// make a string with the name of matrix with the given attributes
   switch (attribute)
   {
   case Valid:                              REPORT return "Rect ";
   case Valid+Square:                       REPORT return "Squ  ";
   case Valid+Symmetric+Square:             REPORT return "Sym  ";
   case Valid+Skew+Square:                  REPORT return "Skew ";
   case Valid+Band+Square:                  REPORT return "Band ";
   case Valid+Symmetric+Band+Square:        REPORT return "SmBnd";
   case Valid+Skew+Band+Square:             REPORT return "SkBnd";
   case Valid+Upper+Square:                 REPORT return "UT   ";
   case Valid+Diagonal+Symmetric+Band+Upper+Lower+Square:
                                            REPORT return "Diag ";
   case Valid+Diagonal+Symmetric+Band+Upper+Lower+Ones+Square:
                                            REPORT return "Ident";
   case Valid+Band+Upper+Square:            REPORT return "UpBnd";
   case Valid+Lower+Square:                 REPORT return "LT   ";
   case Valid+Band+Lower+Square:            REPORT return "LwBnd";
   default:
      REPORT
      if (!(attribute & Valid))             return "UnSp ";
      if (attribute & LUDeco)
         return (attribute & Band) ?     "BndLU" : "Crout";
                                            return "?????";
   }
}


GeneralMatrix* MatrixType::New(int nr, int nc, BaseMatrix* bm) const
{
// make a new matrix with the given attributes

   Tracer tr("New"); GeneralMatrix* gm=0;   // initialised to keep gnu happy
   switch (attribute)
   {
   case Valid:
      REPORT
      if (nc==1) { gm = new ColumnVector(nr); break; }
      if (nr==1) { gm = new RowVector(nc); break; }
      gm = new Matrix(nr, nc); break;

   case Valid+Square:
      REPORT
      if (nc!=nr) { Throw(NotSquareException()); }
      gm = new SquareMatrix(nr); break;

   case Valid+Symmetric+Square:
      REPORT gm = new SymmetricMatrix(nr); break;

   case Valid+Band+Square:
      {
         REPORT
         MatrixBandWidth bw = bm->bandwidth();
         gm = new BandMatrix(nr,bw.lower_val,bw.upper_val); break;
      }

   case Valid+Symmetric+Band+Square:
      REPORT gm = new SymmetricBandMatrix(nr,bm->bandwidth().lower_val); break;

   case Valid+Upper+Square:
      REPORT gm = new UpperTriangularMatrix(nr); break;

   case Valid+Diagonal+Symmetric+Band+Upper+Lower+Square:
      REPORT gm = new DiagonalMatrix(nr); break;

   case Valid+Band+Upper+Square:
      REPORT gm = new UpperBandMatrix(nr,bm->bandwidth().upper_val); break;

   case Valid+Lower+Square:
      REPORT gm = new LowerTriangularMatrix(nr); break;

   case Valid+Band+Lower+Square:
      REPORT gm = new LowerBandMatrix(nr,bm->bandwidth().lower_val); break;

   case Valid+Diagonal+Symmetric+Band+Upper+Lower+Ones+Square:
      REPORT gm = new IdentityMatrix(nr); break;

   default:
      Throw(ProgramException("Invalid matrix type"));
   }
   
   MatrixErrorNoSpace(gm); gm->Protect(); return gm;
}


#ifdef use_namespace
}
#endif

