
//#define WANT_STREAM

#include "include.h"

#include "newmatap.h"
//#include "newmatio.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif


void trymatj()
{
   Tracer et("Nineteenth test of Matrix package");
   Tracer::PrintTrace();
   // testing elementwise (SP) products

   {
      Tracer et1("Stage 1");
      Matrix A(13,7), B(13,7), C(13,7);
      int i,j;
      for (i=1;i<=13;i++) for (j=1; j<=7; j++)
      {
          Real a = (i+j*j)/2, b = (i*j-i/4);
          A(i,j)=a; B(i,j)=b; C(i,j)=a*b;
      }
      // Where complete matrix routine can be used
      Matrix X = SP(A,B)-C; Print(X);
      X = SP(A,B+1.0)-A-C; Print(X);
      X = SP(A-1,B)+B-C; Print(X);
      X = SP(A-1,B+1)+B-A-C+1; Print(X);
      // Where row-wise routine will be used
      A = A.Rows(7,13); B = B.Rows(7,13); C = C.Rows(7,13);
      LowerTriangularMatrix LTA; LTA << A;
      UpperTriangularMatrix UTB; UTB << B;
      DiagonalMatrix DC; DC << C;
      X = SP(LTA,UTB) - DC; Print(X);
      X = SP(LTA*2,UTB) - DC*2; Print(X);
      X = SP(LTA, UTB /2) - DC/2; Print(X);
      X = SP(LTA/2, UTB*2) - DC; Print(X);
      DiagonalMatrix DX;
      DX << SP(A,B); DX << (DX-C); Print(DX);
      DX << SP(A*4,B); DX << (DX-C*4); Print(DX);
      DX << SP(A,B*2); DX << (DX-C*2); Print(DX);
      DX << SP(A/4,B/4); DX << (DX-C/16); Print(DX);
      LowerTriangularMatrix LX;
      LX = SP(LTA,B); LX << (LX-C); Print(LX);
      LX = SP(LTA*3,B); LX << (LX-C*3); Print(LX);
      LX = SP(LTA,B*5); LX << (LX-C*5); Print(LX);
      LX = SP(-LTA,-B); LX << (LX-C); Print(LX);
   }
   {
      // Symmetric Matrices
      Tracer et1("Stage 2");
      SymmetricMatrix A(25), B(25), C(25);
      int i,j;
      for (i=1;i<=25;i++) for (j=i;j<=25;j++)
      {
         Real a = i*j +i - j + 3;
         Real b = i * i + j;
         A(i,j)=a; B(i,j)=b; C(i,j)=a*b;
      }
      UpperTriangularMatrix UT;
      UT << SP(A,B); UT << (UT - C); Print(UT);
      Matrix MA = A, X;
      X = SP(MA,B)-C; Print(X);
      X = SP(A,B)-C; Print(X);
      SymmetricBandMatrix BA(25,5), BB(25,5), BC(25,5);
      BA.Inject(A); BB.Inject(B); BC.Inject(C);
      X = SP(BA,BB)-BC; Print(X);
      X = SP(BA*7,BB)-BC*7; Print(X);
      X = SP(BA,BB/8)-BC/8; Print(X);
      X = SP(BA*16,BB/16)-BC; Print(X);
      X = SP(BA,BB); X=X-BC; Print(X);
      X = SP(BA*2, BB/2)-BC; Print(X);
      X = SP(BA, BB/2)-BC/2; Print(X);
      X = SP(BA*2, BB)-BC*2; Print(X);
   }
   {
      // Band matrices
      Tracer et1("Stage 3");
      Matrix A(19,19), B(19,19), C(19,19);
      int i,j;
      for (i=1;i<=19;i++) for (j=1;j<=19;j++)
      {
         Real a = i*j +i - 1.5*j + 3;
         Real b = i * i + j;
         A(i,j)=a; B(i,j)=b; C(i,j)=a*b;
      }
      BandMatrix BA(19,10,7), BB(19,8,15), BC(19,8,7);
      BA.Inject(A); BB.Inject(B); BC.Inject(C);
      Matrix X; BandMatrix BX; ColumnVector BW(2);
      X = SP(BA,BB); X=X-BC; Print(X);
      X = SP(BA/8,BB); X=X-BC/8; Print(X);
      X = SP(BA,BB*17); X=X-BC*17; Print(X);
      X = SP(BA/4,BB*7); X=X-BC*7/4; Print(X);
      X = SP(BA,BB)-BC; Print(X);
      X = SP(BA/8,BB)-BC/8; Print(X);
      X = SP(BA,BB*17)-BC*17; Print(X);
      X = SP(BA/4,BB*7)-BC*7/4; Print(X);
      BX = SP(BA,BB);
      BW(1)=BX.upper_val-7; BW(2)=BX.lower_val-8; Print(BW);

      BA.ReSize(19,7,10); BB.ReSize(19,15,8);
      BC.ReSize(19,7,8);
      BA.Inject(A); BB.Inject(B); BC.Inject(C);

      X = SP(BA,BB); X=X-BC; Print(X);
      X = SP(BA/8,BB); X=X-BC/8; Print(X);
      X = SP(BA,BB*17); X=X-BC*17; Print(X);
      X = SP(BA/4,BB*7); X=X-BC*7/4; Print(X);
      X = SP(BA,BB)-BC; Print(X);
      X = SP(BA/8,BB)-BC/8; Print(X);
      X = SP(BA,BB*17)-BC*17; Print(X);
      X = SP(BA/4,BB*7)-BC*7/4; Print(X);
      BX = SP(BA,BB);
      BW(1)=BX.upper_val-8; BW(2)=BX.lower_val-7; Print(BW);
   }
   {
      // SymmetricBandMatrices
      Tracer et1("Stage 4");
      Matrix A(7,7), B(7,7);
      int i,j;
      for (i=1;i<=7;i++) for (j=1;j<=7;j++)
      {
         Real a = i*j +i - 1.5*j + 3;
         Real b = i * i + j;
         A(i,j)=a; B(i,j)=b;
      }
      BandMatrix BA(7,2,4), BB(7,3,1), BC(7,2,1);
      BA.Inject(A);
      SymmetricBandMatrix SB(7,3);
      SymmetricMatrix S; S << (B+B.t());
      SB.Inject(S); A = BA; S = SB;
      Matrix X;  
      X = SP(BA,SB); X=X-SP(A,S); Print(X);
      X = SP(BA*2,SB); X=X-SP(A,S*2); Print(X);
      X = SP(BA,SB/4); X=X-SP(A/4,S); Print(X);
      X = SP(BA*4,SB/4); X=X-SP(A,S); Print(X);
      X = SP(BA,SB)-SP(A,S); Print(X);
      X = SP(BA*2,SB)-SP(A,S*2); Print(X);
      X = SP(BA,SB/4)-SP(A/4,S); Print(X);
      X = SP(BA*4,SB/4)-SP(A,S); Print(X);
   }

}


