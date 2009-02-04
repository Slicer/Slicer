//$$ newmatex.cpp                    Exception handler

// Copyright (C) 1992,3,4,7: R B Davies

#define WANT_STREAM                  // include.h will get stream fns

#include "include.h"                 // include standard files
#include "newmat.h"

#ifdef use_namespace
namespace NEWMAT {
#endif

unsigned long OverflowException::Select;
unsigned long SingularException::Select;
unsigned long NPDException::Select;
unsigned long ConvergenceException::Select;
unsigned long ProgramException::Select;
unsigned long IndexException::Select;
unsigned long VectorException::Select;
unsigned long NotSquareException::Select;
unsigned long SubMatrixDimensionException::Select;
unsigned long IncompatibleDimensionsException::Select;
unsigned long NotDefinedException::Select;
unsigned long CannotBuildException::Select;
unsigned long InternalException::Select;



static void MatrixDetails(const GeneralMatrix& A)
// write matrix details to Exception buffer
{
   MatrixBandWidth bw = A.bandwidth();
   int ubw = bw.upper_val; int lbw = bw.lower_val;
   BaseException::AddMessage("MatrixType = ");
   BaseException::AddMessage(A.Type().Value());
   BaseException::AddMessage("  # Rows = "); BaseException::AddInt(A.Nrows());
   BaseException::AddMessage("; # Cols = "); BaseException::AddInt(A.Ncols());
   if (lbw >=0)
   {
      BaseException::AddMessage("; lower BW = ");
      BaseException::AddInt(lbw);
   }
   if (ubw >=0)
   {
      BaseException::AddMessage("; upper BW = ");
      BaseException::AddInt(ubw);
   }
   BaseException::AddMessage("\n");
}

NPDException::NPDException(const GeneralMatrix& A)
   : Runtime_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: matrix not positive definite\n\n");
   MatrixDetails(A);
   Tracer::AddTrace();
}

SingularException::SingularException(const GeneralMatrix& A)
   : Runtime_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: matrix is singular\n\n");
   MatrixDetails(A);
   Tracer::AddTrace();
}

ConvergenceException::ConvergenceException(const GeneralMatrix& A)
   : Runtime_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: process fails to converge\n\n");
   MatrixDetails(A);
   Tracer::AddTrace();
}

ConvergenceException::ConvergenceException(const char* c) : Runtime_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: ");
   AddMessage(c); AddMessage("\n\n");
   if (c) Tracer::AddTrace();
}

OverflowException::OverflowException(const char* c) : Runtime_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: ");
   AddMessage(c); AddMessage("\n\n");
   if (c) Tracer::AddTrace();
}

ProgramException::ProgramException(const char* c) : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: ");
   AddMessage(c); AddMessage("\n\n");
   if (c) Tracer::AddTrace();
}

ProgramException::ProgramException(const char* c, const GeneralMatrix& A)
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: ");
   AddMessage(c); AddMessage("\n\n");
   MatrixDetails(A);
   if (c) Tracer::AddTrace();
}

ProgramException::ProgramException(const char* c, const GeneralMatrix& A,
   const GeneralMatrix& B) : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: ");
   AddMessage(c); AddMessage("\n\n");
   MatrixDetails(A); MatrixDetails(B);
   if (c) Tracer::AddTrace();
}

ProgramException::ProgramException(const char* c, MatrixType a, MatrixType b)
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: ");
   AddMessage(c); AddMessage("\nMatrixTypes = ");
   AddMessage(a.Value()); AddMessage("; ");
   AddMessage(b.Value()); AddMessage("\n\n");
   if (c) Tracer::AddTrace();
}

VectorException::VectorException() : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: cannot convert matrix to vector\n\n");
   Tracer::AddTrace();
}

VectorException::VectorException(const GeneralMatrix& A)
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: cannot convert matrix to vector\n\n");
   MatrixDetails(A);
   Tracer::AddTrace();
}

NotSquareException::NotSquareException(const GeneralMatrix& A)
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: matrix is not square\n\n");
   MatrixDetails(A);
   Tracer::AddTrace();
}

NotSquareException::NotSquareException()
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: matrix is not square\n\n");
   Tracer::AddTrace();
}

SubMatrixDimensionException::SubMatrixDimensionException()
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: incompatible submatrix dimension\n\n");
   Tracer::AddTrace();
}

IncompatibleDimensionsException::IncompatibleDimensionsException()
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: incompatible dimensions\n\n");
   Tracer::AddTrace();
}

IncompatibleDimensionsException::IncompatibleDimensionsException
   (const GeneralMatrix& A, const GeneralMatrix& B)
      : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: incompatible dimensions\n\n");
   MatrixDetails(A); MatrixDetails(B);
   Tracer::AddTrace();
}

IncompatibleDimensionsException::IncompatibleDimensionsException
   (const GeneralMatrix& A)
      : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: incompatible dimensions\n\n");
   MatrixDetails(A);
   Tracer::AddTrace();
}

NotDefinedException::NotDefinedException(const char* op, const char* matrix)
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: ");
   AddMessage(op);
   AddMessage(" not defined for ");
   AddMessage(matrix);
   AddMessage("\n\n");
   Tracer::AddTrace();
}

CannotBuildException::CannotBuildException(const char* matrix)
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: cannot build matrix type ");
   AddMessage(matrix); AddMessage("\n\n");
   Tracer::AddTrace();
}

IndexException::IndexException(int i, const GeneralMatrix& A)
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: index error: requested index = ");
   AddInt(i); AddMessage("\n\n");
   MatrixDetails(A);
   Tracer::AddTrace();
}

IndexException::IndexException(int i, int j, const GeneralMatrix& A)
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: index error: requested indices = ");
   AddInt(i); AddMessage(", "); AddInt(j);
   AddMessage("\n\n");
   MatrixDetails(A);
   Tracer::AddTrace();
}


IndexException::IndexException(int i, const GeneralMatrix& A, bool)
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("detected by Newmat: element error: requested index (wrt 0) = ");
   AddInt(i);
   AddMessage("\n\n");
   MatrixDetails(A);
   Tracer::AddTrace();
}

IndexException::IndexException(int i, int j, const GeneralMatrix& A, bool)
   : Logic_error()
{
   Select = BaseException::Select;
   AddMessage(
      "detected by Newmat: element error: requested indices (wrt 0) = ");
   AddInt(i); AddMessage(", "); AddInt(j);
   AddMessage("\n\n");
   MatrixDetails(A);
   Tracer::AddTrace();
}

InternalException::InternalException(const char* c) : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("internal error detected by Newmat: please inform author\n");
   AddMessage(c); AddMessage("\n\n");
   Tracer::AddTrace();
}




/************************* ExeCounter functions *****************************/

#ifdef DO_REPORT

int ExeCounter::nreports;                      // will be set to zero

ExeCounter::ExeCounter(int xl, int xf) : line(xl), fileid(xf), nexe(0) {}

ExeCounter::~ExeCounter()
{
   nreports++;
   cout << "REPORT  " << setw(6) << nreports << "  "
      << setw(6) << fileid << "  " << setw(6) << line
      << "  " << setw(6) << nexe << "\n";
}

#endif

/**************************** error handler *******************************/

void MatrixErrorNoSpace(const void* v) { if (!v) Throw(Bad_alloc()); }
// throw exception if v is null




/************************* miscellanous errors ***************************/


void CroutMatrix::GetRow(MatrixRowCol&)
   { Throw(NotDefinedException("GetRow","Crout")); }
void CroutMatrix::GetCol(MatrixRowCol&)
   { Throw(NotDefinedException("GetCol","Crout")); }
void BandLUMatrix::GetRow(MatrixRowCol&)
   { Throw(NotDefinedException("GetRow","BandLUMatrix")); }
void BandLUMatrix::GetCol(MatrixRowCol&)
   { Throw(NotDefinedException("GetCol","BandLUMatrix")); }
void BaseMatrix::IEQND() const
   { Throw(NotDefinedException("inequalities", "matrices")); }


#ifdef use_namespace
}
#endif

