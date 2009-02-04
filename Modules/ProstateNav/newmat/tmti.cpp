
//#define WANT_STREAM

#include "include.h"

#include "newmatap.h"
//#include "newmatio.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif


void WillNotConverge()
{
   Matrix A(10,10);
   Throw(ConvergenceException(A));
}

void ReSizeMatrix(Matrix& A)
// for seeing if we can redimension a vector as a matrix
{ A.ReSize(4,5); }

void trymati()
{
#ifndef DisableExceptions
   Tracer et("Eighteenth test of Matrix package");
   Matrix RUStillThere(10,20); RUStillThere = 1553;
   Tracer::PrintTrace();

   ColumnVector checks(23); checks = 1.0; checks(1) = 0.0;

   Try { WillNotConverge(); }
   Catch(ConvergenceException) { checks(2) = 0; }
   CatchAll { checks(1) = 1; }


   Try { Matrix M(10,10); SymmetricMatrix S = M; }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(ProgramException) { checks(3) = 0; }
   CatchAll { checks(1) = 1; }


   Try { Matrix M(10,10); M(10,11) = 2.0; }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(IndexException) { checks(4) = 0; }
   CatchAll { checks(1) = 1; }

   Try { Matrix M(10,10); M = 0.0; M = M.i(); }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(ProgramException) { checks(1) = 1; }
   Catch(SingularException) { checks(5) = 0; }
   Catch(Bad_alloc) { checks(1) = 1; }
   CatchAndThrow;

   Try { ColumnVector A(30), B(50);  A = 5; B = 3; FFT(A,B,A,B); }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(ProgramException) { checks(6) = 0; }
   CatchAll { checks(1) = 1; }

   Try
   {
      ColumnVector A(30); A = 5; Matrix At = A.t();
      DiagonalMatrix D;
      SVD(At,D);
   }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(Logic_error) { checks(6) = 0; }
   Catch(Bad_alloc) { checks(1) = 1; }
   CatchAndThrow;

   Try { BandMatrix X(10,3,4); X(1,10) = 4.0; }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(IndexException) { checks(7) = 0; }
   CatchAll { checks(1) = 1; }

   Try
   {
      SymmetricMatrix S(10); S = 5;
      LowerTriangularMatrix L = Cholesky(S);
   }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(ProgramException) { checks(1) = 1; }
   Catch(NPDException) { checks(8) = 0; }
   Catch(Bad_alloc) { checks(1) = 1; }
   CatchAndThrow;

   Try { BandMatrix M(10,3,5); M = 0.0; Matrix XM = M.i(); }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(ProgramException) { checks(1) = 1; }
   Catch(SingularException) { checks(9) = 0; }
   Catch(Bad_alloc) { checks(1) = 1; }
   CatchAndThrow;

   Try { ColumnVector X(10); ColumnVector Y; X = 5; X = X - Y; }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(IncompatibleDimensionsException) { checks(10) = 0; }
   Catch(Bad_alloc) { checks(1) = 1; }
   CatchAndThrow;

   Try
   {
      UpperTriangularMatrix U(3); RowVector RV(3); RV = 10;
      U.Row(2) = RV;
   }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(ProgramException) { checks(11) = 0; }
   Catch(Bad_alloc) { checks(1) = 1; }
   CatchAndThrow;

   Try { DiagonalMatrix D(3); D << 12 << 13 << 14 << 15; }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(ProgramException) { checks(12) = 0; }
   CatchAndThrow;

   Try { ColumnVector D(3); D << 12 << 13; D << 1 << 2 << 3; }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(ProgramException) { checks(13) = 0; }
   CatchAndThrow;


   Try {  { ColumnVector D(3); D << 12 << 13; }  }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(ProgramException) { checks(14) = 0; }
   CatchAndThrow;

   Try { ColumnVector CV; ReSizeMatrix(CV); }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(VectorException) { checks(15) = 0; }
   CatchAndThrow;

   Try { RowVector RV(20); ReSizeMatrix(RV); }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(VectorException) { checks(16) = 0; }
   CatchAndThrow;

   Try
   {
      UpperTriangularMatrix U(10); U = 5;
      DiagonalMatrix D(10); D = 2;
      D += U;                 // illegal conversion
   }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(ProgramException) { checks(17) = 0; }
   CatchAndThrow;

   Try { Matrix A(2,3), B(2,3); if (A < B) A = B; }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(NotDefinedException) { checks(18) = 0; }
   CatchAndThrow;

   Try { SymmetricBandMatrix A(3,1); A = 1; A = A.Reverse(); }
   Catch(ConvergenceException) { checks(1) = 1; }
   Catch(InternalException) { checks(1) = 1; }
   Catch(NotDefinedException) { checks(19) = 0; }
   CatchAndThrow;

   Try
   {
      Matrix A(5,5); A = 1.0;
      UpperTriangularMatrix B(10);
      B.SubMatrix(3,7,3,7) = A;
   }
   Catch(ProgramException) { checks(20) = 0; }
   CatchAndThrow;

   Try {  { RowVector D(1); D << 12 << 13; }  }
   Catch(InternalException) { checks(1) = 1; }
   Catch(ProgramException) { checks(21) = 0; }
   CatchAndThrow;

   Try {  { RowVector D(0); D << 12; }  }
   Catch(InternalException) { checks(1) = 1; }
   Catch(ProgramException) { checks(22) = 0; }
   CatchAndThrow;

   Try { Matrix M(10,10); Matrix XM(3,3); M = 0.0; XM = M.i(); }
   Catch(SingularException) { checks(23) = 0; }
   CatchAll { checks(1) = 1; }

   Print(checks);
   Matrix RUStillThere1(10,20); RUStillThere1 = 1553;
   RUStillThere = RUStillThere - RUStillThere1;
   Print(RUStillThere);
#endif

}



