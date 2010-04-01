//#define WANT_STREAM

#include "include.h"
#include "newmatap.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif


/**************************** test program ******************************/


void trymat9()
{
   Tracer et("Ninth test of Matrix package");
   Tracer::PrintTrace();


   int i; int j;
   Matrix A(7,7); Matrix X(7,3);
   for (i=1;i<=7;i++) for (j=1;j<=7;j++) A(i,j)=i*i+j+((i==j) ? 1 : 0);
   for (i=1;i<=7;i++) for (j=1;j<=3;j++) X(i,j)=i-j;
   Matrix B = A.i(); DiagonalMatrix D(7); D=1.0;
   {
      Tracer et1("Stage 1");
      Matrix Q = B*A-D; Clean(Q, 0.000000001); Print(Q);
      Q=A; Q = Q.i() * X; Q = A*Q - X; Clean(Q, 0.000000001); Print(Q);
      Q=X; Q = A.i() * Q; Q = A*Q - X; Clean(Q, 0.000000001); Print(Q);
   }
   for (i=1;i<=7;i++) D(i,i)=i*i+1;
   DiagonalMatrix E(3); for (i=1;i<=3;i++) E(i,i)=i+23;
   {
      Tracer et1("Stage 2");
      Matrix DXE = D.i() * X * E;
      DXE = E.i() * DXE.t() * D - X.t(); Clean(DXE, 0.00000001); Print(DXE); 
      E=D; for (i=1;i<=7;i++) E(i,i)=i*3+1;
   }
   DiagonalMatrix F=D;
   {
      Tracer et1("Stage 3");
      F=E.i()*F; F=F*E-D; Clean(F,0.00000001); Print(F);
      F=E.i()*D; F=F*E-D; Clean(F,0.00000001); Print(F);
   }
   {
      Tracer et1("Stage 4");
      F=E; F=F.i()*D; F=F*E-D; Clean(F,0.00000001); Print(F);
   }
   {
      Tracer et1("Stage 5");
      // testing equal
      ColumnVector A(18), B(18);
      Matrix X(3,3);
      X << 3 << 5  << 7 << 5 << 8 << 2 << 7 << 2 << 9;
      SymmetricMatrix S; S << X;
      B(1) = S == X;         A(1) = true;
      B(2) = S == (X+1);     A(2) = false;
      B(3) = (S+2) == (X+2); A(3) = true;
      Matrix Y = X;
      B(4) = X == Y;         A(4) = true;
      B(5) = (X*2) == (Y*2); A(5) = true;
      Y(3,3) = 10;
      B(6) = X == Y;         A(6) = false;
      B(7) = (X*2) == (Y*2); A(7) = false;
      B(8) = S == Y;         A(8) = false;
      B(9) = S == S;         A(9) = true;
      Matrix Z = X.SubMatrix(1,2,2,3);
      B(10) = X == Z;        A(10) = false;
      GenericMatrix GS = S;
      GenericMatrix GX = X;
      GenericMatrix GY = Y;
      B(11) = GS == GX;      A(11) = true;
      B(12) = GS == GY;      A(12) = false;
      CroutMatrix CS = S;
      CroutMatrix CX = X;
      CroutMatrix CY = Y;
      B(13) = CS == CX;      A(13) = true;
      B(14) = CS == CY;      A(14) = false;
      B(15) = X == CX;       A(15) = false;
      B(16) = X == A;        A(16) = false;
      B(17) = X == (X | X);  A(17) = false;
      B(18) = CX == X;       A(18) = false;
      A = A - B; Print(A);
   }
   {
      Tracer et1("Stage 6");
      // testing equal
      ColumnVector A(22), B(22);
      BandMatrix X(6,2,1);
      X(1,1)=23; X(1,2)=21;
      X(2,1)=12; X(2,2)=17; X(2,3)=45;
      X(3,1)=35; X(3,2)=19; X(3,3)=24; X(3,4)=29;
                 X(4,2)=17; X(4,3)=11; X(4,4)=19; X(4,5)=35;
                            X(5,3)=10; X(5,4)=44; X(5,5)=23; X(5,6)=31;
                                       X(6,4)=49; X(6,5)=41; X(6,6)=17;
      SymmetricBandMatrix S1(6,2); S1.Inject(X);
      BandMatrix U(6,2,3); U = 0.0; U.Inject(X);
      B(1) = U == X;         A(1) = true;
      B(2) = U == (X*3);     A(2) = false;
      B(3) = (U*5) == (X*5); A(3) = true;
      Matrix Y = X;
      B(4) = X == Y;         A(4) = true;
      B(5) = (X*2) == (Y*2); A(5) = true;
      Y(6,6) = 10;
      B(6) = X == Y;         A(6) = false;
      B(7) = (X*2) == (Y*2); A(7) = false;
      B(8) = U == Y;         A(8) = false;
      B(9) = U == U;         A(9) = true;
      Matrix Z = X.SubMatrix(1,2,2,3);
      B(10) = X == Z;        A(10) = false;
      GenericMatrix GU = U;
      GenericMatrix GX = X;
      GenericMatrix GY = Y;
      B(11) = GU == GX;      A(11) = true;
      B(12) = GU == GY;      A(12) = false;
      X = X + X.t(); U = U + U.t();
      SymmetricBandMatrix S(6,2); S.Inject(X);
      Matrix D = S-X; Print(D);
      BandLUMatrix BS = S;
      BandLUMatrix BX = X;
      BandLUMatrix BU = U;
      CroutMatrix CX = X;
      B(13) = BS == BX;      A(13) = true;
      B(14) = BX == BU;      A(14) = false;
      B(15) = X == BX;       A(15) = false;
      B(16) = X != BX;       A(16) = true;
      B(17) = BX != BS;      A(17) = false;
      B(18) = (2*X) != (X*2);A(18) = false;
      B(19) = (X*2) != (X+2);A(19) = true;
      B(20) = BX == CX;      A(20) = false;
      B(21) = CX == BX;      A(21) = false;
      B(22) = BX == X;       A(22) = false;
      A = A - B; Print(A);
      DiagonalMatrix I(6); I=1.0;
      D = BS.i() * X - I;  Clean(D,0.00000001); Print(D);
      D = BX.i() * X - I;  Clean(D,0.00000001); Print(D);
      D = BU.i() * X - I;  Clean(D,0.00000001); Print(D);

      // test row wise load
      SymmetricBandMatrix X1(6,2);
      X1.Row(1) << 23;
      X1.Row(2) << 12 << 17;
      X1.Row(3) << 35 << 19 << 24;
      X1.Row(4)       << 17 << 11 << 19;
      X1.Row(5)             << 10 << 44 << 23;
      X1.Row(6)                   << 49 << 41 << 17;
      Matrix M = X1 - S1; Print(M);

      // check out submatrix
      SymmetricBandMatrix X2(20,3); X2 = 0.0;
      X2.SubMatrix(2,7,2,7) = X1; X2.SymSubMatrix(11,16) = 2 * X1;
      Matrix MX1 = X1;
      Matrix MX2(20,20); MX2 = 0;
      MX2.SymSubMatrix(2,7) = MX1; MX2.SubMatrix(11,16,11,16) = MX1 * 2;
      MX2 -= X2; Print(MX2);

      BandMatrix X4(20,3,3); X4 = 0.0;
      X4.SubMatrix(2,7,3,8) = X1; X4.SubMatrix(11,16,10,15) = 2 * X1;
      MX1 = X1;
      Matrix MX4(20,20); MX4 = 0;
      MX4.SubMatrix(2,7,3,8) = MX1; MX4.SubMatrix(11,16,10,15) = MX1 * 2;
      MX4 -= X4; Print(MX4);

      MX1 = X1.i() * X1 - IdentityMatrix(6);
      Clean(MX1,0.00000001); Print(MX1);

   }

   {
      Tracer et1("Stage 7");
      // testing equal
      ColumnVector A(12), B(12);
      BandMatrix X(6,2,1);
      X(1,1)=23; X(1,2)=21;
      X(2,1)=12; X(2,2)=17; X(2,3)=45;
      X(3,1)=35; X(3,2)=19; X(3,3)=24; X(3,4)=29;
                 X(4,2)=17; X(4,3)=11; X(4,4)=19; X(4,5)=35;
                            X(5,3)=10; X(5,4)=44; X(5,5)=23; X(5,6)=31;
                                       X(6,4)=49; X(6,5)=41; X(6,6)=17;
      Matrix Y = X;
      LinearEquationSolver LX = X;
      LinearEquationSolver LY = Y;
      CroutMatrix CX = X;
      CroutMatrix CY = Y;
      BandLUMatrix BX = X;
      B(1) = LX == CX;       A(1) = false;
      B(2) = LY == CY;       A(2) = true;
      B(3) = X == Y;         A(3) = true;
      B(4) = BX == LX;       A(4) = true;
      B(5) = CX == CY;       A(5) = true;
      B(6) = LX == LY;       A(6) = false;
      B(7) = BX == BX;       A(7) = true;
      B(8) = CX == CX;       A(8) = true;
      B(9) = LX == LX;       A(9) = true;
      B(10) = LY == LY;      A(10) = true;
      CroutMatrix CX1 = X.SubMatrix(1,4,1,4);
      B(11) = CX == CX1;     A(11) = false;
      BandLUMatrix BX1 = X.SymSubMatrix(1,4);    // error with SubMatrix
      B(12) = BX == BX1;     A(12) = false;
      A = A - B; Print(A);
      DiagonalMatrix I(6); I=1.0; Matrix D;
      D = LX.i() * X - I;  Clean(D,0.00000001); Print(D);
      D = LY.i() * X - I;  Clean(D,0.00000001); Print(D);
      I.ReSize(4); I = 1;
      D = CX1.i() * X.SymSubMatrix(1,4) - I;  Clean(D,0.00000001); Print(D);
      D = BX1.i() * X.SubMatrix(1,4,1,4) - I;  Clean(D,0.00000001); Print(D);
   }

   {
      Tracer et1("Stage 8");
      // test copying CroutMatrix and BandLUMatrix - see also tmtd.cpp 
      MultWithCarry MWC;
      SymmetricBandMatrix SBM(50, 10);
      for (int i = 1; i <= 50; ++i) for (int j = 1; j <= i; ++j)
         if (i - j <= 10) SBM(i, j) = MWC.Next();
      CroutMatrix CM = SBM; BandLUMatrix BM = SBM;
      CroutMatrix CM1 = CM; BandLUMatrix BM1; BM1 = BM;
      CM1.release(); BM1.release();
      CroutMatrix CM2; CM2 = CM1; BandLUMatrix BM2 = BM1;
      Matrix X = SBM.i(); Matrix Y = CM2.i() - X; Matrix Z = BM2.i() - X;
      Clean(Y,0.00000001); Print(Y); Clean(Z,0.00000001); Print(Z);
      X *= SBM; X -= IdentityMatrix(50); Clean(X,0.00000001); Print(X);
      LogAndSign x = log_determinant(SBM);
      LogAndSign y = log_determinant(CM2);
      LogAndSign z = log_determinant(BM2);
      RowVector D(4);
      D(1) = y.value() - x.value();
      D(2) = z.value() - x.value();
      D(3) = y.sign() - x.sign();
      D(4) = z.sign() - x.sign();
      Clean(D,0.00000001); Print(D);
   }

   {
      Tracer et1("Stage 9");
      // do it again odd matrix size 
      MultWithCarry MWC;
      SymmetricBandMatrix SBM(51, 10);
      for (int i = 1; i <= 51; ++i) for (int j = 1; j <= i; ++j)
         if (i - j <= 10) SBM(i, j) = MWC.Next();
      CroutMatrix CM = SBM; BandLUMatrix BM = SBM;
      CroutMatrix CM1 = CM; BandLUMatrix BM1; BM1 = BM;
      CM1.release(); BM1.release();
      CroutMatrix CM2; CM2 = CM1; BandLUMatrix BM2 = BM1;
      Matrix X = SBM.i(); Matrix Y = CM2.i() - X; Matrix Z = BM2.i() - X;
      Clean(Y,0.00000001); Print(Y); Clean(Z,0.00000001); Print(Z);
      X *= SBM; X -= IdentityMatrix(51); Clean(X,0.00000001); Print(X);
      LogAndSign x = log_determinant(SBM);
      LogAndSign y = log_determinant(CM2);
      LogAndSign z = log_determinant(BM2);
      RowVector D(4);
      D(1) = y.value() - x.value();
      D(2) = z.value() - x.value();
      D(3) = y.sign() - x.sign();
      D(4) = z.sign() - x.sign();
      Clean(D,0.00000001); Print(D);
   }
   
   
//   cout << "\nEnd of ninth test\n";
}

