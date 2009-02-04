
//#define WANT_STREAM

#include "include.h"

#include "newmat.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif


// **************************** test program ******************************



ReturnMatrix Returner0(const GenericMatrix& GM)
{ Matrix M = GM; M.Release(); return M; }

ReturnMatrix Returner1(const GenericMatrix& GM)
{ Matrix M = GM+1; M.Release(); return M; }

ReturnMatrix Returner2(const GenericMatrix& GM)
{ UpperBandMatrix M = GM*2; M.Release(); return M; }

ReturnMatrix Returner3(const GenericMatrix& GM)
{ LowerBandMatrix M = GM*3; M.Release(); return M; }

ReturnMatrix Returner4(const GenericMatrix& GM)
{ SymmetricMatrix M = GM+4; M.Release(); return M; }

ReturnMatrix Returner5(const GenericMatrix& GM)
{ SymmetricBandMatrix M = GM*5; M.Release(); return M; }

ReturnMatrix Returner6(const GenericMatrix& GM)
{ BandMatrix M = GM*6; M.Release(); return M; }

ReturnMatrix Returner7(const GenericMatrix& GM)
{ DiagonalMatrix M = GM*7; M.Release(); return M; }

void trymat5()
{
//   cout << "\nFifth test of Matrix package\n";
   Tracer et("Fifth test of Matrix package");
   Tracer::PrintTrace();

   int i,j;

   Matrix A(5,6);
   for (i=1;i<=5;i++) for (j=1;j<=6;j++) A(i,j)=1+i*j+i*i+j*j;
   ColumnVector CV(6);
   for (i=1;i<=6;i++) CV(i)=i*i+3;
   ColumnVector CV2(5); for (i=1;i<=5;i++) CV2(i)=1.0;
   ColumnVector CV1=CV;

   {
      CV=A*CV;
      RowVector RV=CV.t(); // RowVector RV; RV=CV.t();
      RV=RV-1.0;
      CV=(RV*A).t()+A.t()*CV2; CV1=(A.t()*A)*CV1 - CV;
      Print(CV1);
   }

   CV1.ReSize(6);
   CV2.ReSize(6);
   CV.ReSize(6);
   for (i=1;i<=6;i++) { CV1(i)=i*3+1; CV2(i)=10-i; CV(i)=11+i*2; }
   ColumnVector CX=CV2-CV; { CX=CX+CV1; Print(CX); }
   Print(ColumnVector(CV1+CV2-CV));
   RowVector RV=CV.t(); RowVector RV1=CV1.t();
   RowVector R=RV-RV1; Print(RowVector(R-CV2.t()));

// test loading of list

   RV.ReSize(10);
   for (i=1;i<=10;i++) RV(i) = i*i;
   RV1.ReSize(10);
   RV1 << 1 << 4 << 9 << 16 << 25 << 36 << 49 << 64 << 81 << 100; // << 121;
   Print(RowVector(RV-RV1));

   et.ReName("Fifth test of Matrix package - almost at end");

   Matrix X(2,3);
   X << 11 << 12 << 13
     << 21 << 22 << 23;

   Matrix Y = X.t();                 // check simple transpose

   X(1,1) -= 11; X(1,2) -= 12; X(1,3) -= 13;
   X(2,1) -= 21; X(2,2) -= 22; X(2,3) -= 23;
   Print(X);

   Y(1,1) -= 11; Y(2,1) -= 12; Y(3,1) -= 13;
   Y(1,2) -= 21; Y(2,2) -= 22; Y(3,2) -= 23;
   Print(Y);

   et.ReName("Fifth test of Matrix package - at end");

   RV = Returner1(RV)-1; Print(RowVector(RV-RV1));
   CV1 = Returner1(RV.t())-1; Print(ColumnVector(RV.t()-CV1));
#ifndef DONT_DO_NRIC
   nricMatrix AA = A;
   X = Returner1(AA)-A-1; Print(X);
#endif
   UpperTriangularMatrix UT(31);
   for (i=1; i<=31; i++) for (j=i; j<=31; j++) UT(i,j) = i+j+(i-j)*(i-2*j);
   UpperBandMatrix UB(31,5); UB.Inject(UT);
   LowerTriangularMatrix LT = UT.t();
   LowerBandMatrix LB(31,5); LB.Inject(LT);
   A = Returner0(UB)-LB.t(); Print(A);
   A = Returner2(UB).t()-LB*2; Print(A);
   A = Returner3(LB).t()-UB*3; Print(A);
   SymmetricMatrix SM; SM << (UT+LT);
   A = Returner4(SM)-UT-LT-4; Print(A);
   SymmetricBandMatrix SB(31,5); SB.Inject(SM);
   A = Returner5(SB)/5-UB-LB; Print(A);
   BandMatrix B = UB+LB*LB; A = LB;
   A = Returner6(B)/6 - UB - A*A; Print(A);
   DiagonalMatrix D; D << UT;
   D << (Returner7(D)/7 - UT); Print(D);

//   cout << "\nEnd of fifth test\n";
}
