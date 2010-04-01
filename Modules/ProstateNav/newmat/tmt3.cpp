
//#define WANT_STREAM

#include "include.h"

#include "newmat.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif


/**************************** test program ******************************/

void trymat3()
{
   Tracer et("Third test of Matrix package");
   Tracer::PrintTrace();

   {
      Tracer et1("Stage 1");
      int i,j;
      SymmetricMatrix S(7);
      for (i=1;i<=7;i++) for (j=1;j<=i;j++) S(i,j)=i*i+j;
                S=-S+2.0;

      DiagonalMatrix D(7);
      for (i=1;i<=7;i++) D(i,i)=S(i,i);

      Matrix M4(7,7); { M4=D+(D+4.0); M4=M4-D*2.0;  M4=M4-4.0; Print(M4); }
      SymmetricMatrix S2=D; Matrix M2=S2;  { M2=-D+M2; Print(M2); }
      UpperTriangularMatrix U2=D; { M2=U2; M2=D-M2; Print(M2); }
      LowerTriangularMatrix L2=D; { M2=L2; M2=D-M2; Print(M2); }
      M2=D; M2=M2-D; Print(M2);
      for (i=1;i<=7;i++) for (j=1;j<=i;j++) L2(i,j)=2.0-i*i-j;
      U2=L2.t(); D=D.t(); S=S.t();
      M4=(L2-1.0)+(U2+1.0)-D-S; Print(M4);
      M4=(-L2+1.0)+(-U2-1.0)+D+S; Print(M4);
   }

   {
      Tracer et1("Stage 2");
      int i,j;
      DiagonalMatrix D(6);
      for (i=1;i<=6;i++) D(i,i)=i*3.0+i*i+2.0;
      UpperTriangularMatrix U2(7); LowerTriangularMatrix L2(7);
      for (i=1;i<=7;i++) for (j=1;j<=i;j++) L2(i,j)=2.0-i*i+j;
                { U2=L2.t(); }
      DiagonalMatrix D1(7); for (i=1;i<=7;i++) D1(i,i)=(i-2)*(i-4);
      Matrix M2(6,7);
      for (i=1;i<=6;i++) for (j=1;j<=7;j++) M2(i,j)=2.0+i*j+i*i+2.0*j*j;
      Matrix MD=D; SymmetricMatrix MD1(1); MD1=D1;
      Matrix MX=MD*M2*MD1 - D*(M2*D1); Print(MX);
      MX=MD*M2*MD1 - (D*M2)*D1; Print(MX);
      {
         D.ReSize(7); for (i=1;i<=7;i++) D(i,i)=i*3.0+i*i+2.0;
         LowerTriangularMatrix LD(1); LD=D;
         UpperTriangularMatrix UD(1); UD=D;
         M2=U2; M2=LD*M2*MD1 - D*(U2*D1); Print(M2);
         M2=U2; M2=UD*M2*MD1 - (D*U2)*D1; Print(M2);
         M2=L2; M2=LD*M2*MD1 - D*(L2*D1); Print(M2);
         M2=L2; M2=UD*M2*MD1 - (D*L2)*D1; Print(M2);
      }
   }

   {
      Tracer et1("Stage 3");
      // test inverse * scalar
      DiagonalMatrix D(6);
      for (int i=1;i<=6;i++) D(i)=i*i;
      DiagonalMatrix E = D.i() * 4.0;
      DiagonalMatrix I(6); I = 1.0;
      E=D*E-I*4.0; Print(E);
      E = D.i() / 0.25; E=D*E-I*4.0; Print(E);
   }
   {
      Tracer et1("Stage 4");
      Matrix sigma(3,3); Matrix sigmaI(3,3);
      sigma = 0; sigma(1,1) = 1.0; sigma(2,2) = 1.0; sigma(3,3) = 1.0;
      sigmaI = sigma.i();
      sigmaI -= sigma;  Clean(sigmaI, 0.000000001); Print(sigmaI);
   }
   {
      Tracer et1("Stage 5");
      Matrix X(5,5); DiagonalMatrix DM(5);
      for (int i=1; i<=5; i++) for (int j=1; j<=5; j++)
         X(i,j) = (23*i+59*j) % 43;
      DM << 1 << 8 << -7 << 2 << 3;
      Matrix Y = X.i() * DM; Y = X * Y - DM;
      Clean(Y, 0.000000001); Print(Y);
   }
   {
      Tracer et1("Stage 6");          // test reverse function
      ColumnVector CV(10), RCV(10);
      CV  <<  2 << 7 << 1  << 6 << -3 <<  1 << 8 << -4 << 0 << 17;
      RCV << 17 << 0 << -4 << 8 << 1  << -3 << 6 <<  1 << 7 << 2;
      ColumnVector X = CV - RCV.Reverse(); Print(X);
      RowVector Y = CV.t() - RCV.t().Reverse(); Print(Y);
      DiagonalMatrix D = CV.AsDiagonal() - RCV.AsDiagonal().Reverse();
      Print(D);
      X = CV & CV.Rows(1,9).Reverse();
      ColumnVector Z(19);
      Z.Rows(1,10) = RCV.Reverse(); Z.Rows(11,19) = RCV.Rows(2,10);
      X -= Z; Print(X); Z -= Z.Reverse(); Print(Z);
      Matrix A(3,3); A << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
      Matrix B(3,3); B << 9 << 8 << 7 << 6 << 5 << 4 << 3 << 2 << 1;
      Matrix Diff = A - B.Reverse(); Print(Diff);
      Diff = (-A).Reverse() + B; Print(Diff);
      UpperTriangularMatrix U;
      U << A.Reverse(); Diff = U; U << B; Diff -= U; Print(Diff);
      U << (-A).Reverse(); Diff = U; U << B; Diff += U; Print(Diff);
   }
   {
      Tracer et1("Stage 7");           // test IsSingular function
      ColumnVector XX(4);
      Matrix A(3,3);
      A = 0;
      CroutMatrix B1 = A;
      XX(1) = B1.IsSingular() ? 0 : 1;
      A << 1 << 3 << 6
        << 7 << 11 << 13
        << 2 << 4  << 1;
      CroutMatrix B2(A);
      XX(2) = B2.IsSingular() ? 1 : 0;
      BandMatrix C(3,1,1); C.Inject(A);
      BandLUMatrix B3(C);
      XX(3) = B3.IsSingular() ? 1 : 0;
      C = 0;
      BandLUMatrix B4(C);
      XX(4) = B4.IsSingular() ? 0 : 1;
      Print(XX);
   }
   {
      Tracer et1("Stage 8");           // inverse with vector of 0s
      Matrix A(3,3); Matrix Z(3,3); ColumnVector X(6);
      A <<  1 <<  3 <<  6
        <<  7 << 11 << 13
        <<  2 <<  4 <<  1;
      Z = 0;
      Matrix B = (A | Z) & (Z | A);   // 6 * 6 matrix
      X = 0.0;
      X = B.i() * X;
      Print(X);
      // also check inverse with non-zero Y
      Matrix Y(3,3);
      Y << 0.0 << 1.0 << 1.0
        << 5.0 << 0.0 << 5.0
        << 3.0 << 3.0 << 0.0;
      Matrix YY = Y & Y;        // stack Y matrices
      YY = B.i() * YY;
      Matrix Y1 = A.i() * Y;
      YY -= Y1 & Y1; Clean(YY, 0.000000001); Print(YY);
      Y1 = A * Y1 - Y; Clean(Y1, 0.000000001); Print(Y1);
   }


}

