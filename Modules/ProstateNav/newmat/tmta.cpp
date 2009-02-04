
//#define WANT_STREAM


#include "include.h"

#include "newmatap.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif


/**************************** test program ******************************/


static void process(const GeneralMatrix& A,
   const ColumnVector& X1, const ColumnVector& X2)
{
      Matrix B = A;
      LinearEquationSolver L(A);
      Matrix Y(4,2);
      Y.Column(1) << L.i() * X1; Y.Column(2) << L.i() * X2;
      Matrix Z(4,2); Z.Column(1) << X1; Z.Column(2) << X2;
      Z = B * Y - Z; Clean(Z,0.00000001); Print(Z);
}



void trymata()
{
//   cout << "\nTenth test of Matrix package\n";
   Tracer et("Tenth test of Matrix package");
   Tracer::PrintTrace();
   int i; int j;
   UpperTriangularMatrix U(8);
   for (i=1;i<=8;i++) for (j=i;j<=8;j++) U(i,j)=i+j*j+5;
   Matrix X(8,6);
   for (i=1;i<=8;i++) for (j=1;j<=6;j++) X(i,j)=i*j+1.0;
   Matrix Y = U.i()*X; Matrix MU=U;
   Y=Y-MU.i()*X; Clean(Y,0.00000001); Print(Y);
   Y = U.t().i()*X; Y=Y-MU.t().i()*X; Clean(Y,0.00000001); Print(Y);
   UpperTriangularMatrix UX(8);
   for (i=1;i<=8;i++) for (j=i;j<=8;j++) UX(i,j)=i+j+1;
   UX(4,4)=0; UX(4,5)=0;
   UpperTriangularMatrix UY = U.i() * UX;
   { X=UX; MU=U; Y=UY-MU.i()*X; Clean(Y,0.000000001); Print(Y); }
   LowerTriangularMatrix LY = U.t().i() * UX.t();
   { Y=LY-MU.i().t()*X.t(); Clean(Y,0.000000001); Print(Y); }
   DiagonalMatrix D(8); for (i=1;i<=8;i++) D(i,i)=i+1;
   { X=D.i()*MU; }
   { UY=U; UY=D.i()*UY; Y=UY-X; Clean(Y,0.00000001); Print(Y); }
   { UY=D.i()*U; Y=UY-X; Clean(Y,0.00000001); Print(Y); }
//   X=MU.t();
//   LY=D.i()*U.t(); Y=D*LY-X; Clean(Y,0.00000001); Print(Y);
//   LowerTriangularMatrix L=U.t();
//   LY=D.i()*L; Y=D*LY-X; Clean(Y,0.00000001); Print(Y);
   U.ReSize(8);
   for (i=1;i<=8;i++) for (j=i;j<=8;j++) U(i,j)=i+j*j+5;
   MU = U;
   MU = U.i() - MU.i(); Clean(MU,0.00000001); Print(MU);
   MU = U.t().i() - U.i().t(); Clean(MU,0.00000001); Print(MU);

   // test LINEQ
   {
      ColumnVector X1(4), X2(4);
      X1(1)=1; X1(2)=2; X1(3)=3; X1(4)=4;
      X2(1)=1; X2(2)=10; X2(3)=100; X2(4)=1000;


      Matrix A(4,4);
      A(1,1)=1; A(1,2)=3; A(1,3)=0; A(1,4)=0;
      A(2,1)=3; A(2,2)=2; A(2,3)=5; A(2,4)=0;
      A(3,1)=0; A(3,2)=5; A(3,3)=4; A(3,4)=1;
      A(4,1)=0; A(4,2)=0; A(4,3)=1; A(4,4)=3;
      process(A,X1,X2);

      BandMatrix B(4,1,1);  B.Inject(A);
      process(B,X1,X2);

      UpperTriangularMatrix U(4);
      U(1,1)=1; U(1,2)=2; U(1,3)=3; U(1,4)=4;
                      U(2,2)=8; U(2,3)=7; U(2,4)=6;
                          U(3,3)=2; U(3,4)=7;
                                    U(4,4)=1;
      process(U,X1,X2);

      // check rowwise load
      UpperTriangularMatrix U1(4);
      U1.Row(1) << 1 << 2 << 3 << 4;
      U1.Row(2)      << 8 << 7 << 6;
      U1.Row(3)           << 2 << 7;
      U1.Row(4)                << 1;

      U1 -= U;

      Print(U1);

      LowerTriangularMatrix L = U.t();
      process(L,X1,X2);
   }

   // test inversion of poorly conditioned matrix
   // a user complained this didn't work under OS9
   {
      Matrix M(4,4);

      M <<  8.613057e+00 <<  8.693985e+00 << -2.322050e-01  << 0.000000e+00
        <<  8.693985e+00 <<  8.793868e+00 << -2.346310e-01  << 0.000000e+00
        << -2.322050e-01 << -2.346310e-01 <<  6.264000e-03  << 0.000000e+00
        <<  0.000000e+00 <<  0.000000e+00 <<  0.000000e+00  << 3.282806e+03 ;
      Matrix MI = M.i();
      DiagonalMatrix I(4); I = 1;
      Matrix Diff = MI *  M - I;
      Clean(Diff,0.00000001); Print(Diff);
      // Alternatively do Cholesky
      SymmetricMatrix SM; SM << M;
      LowerTriangularMatrix LT = Cholesky(SM).i();
      MI = LT.t() * LT; Diff = MI *  M - I;
      Clean(Diff,0.00000001); Print(Diff);
   }

//   cout << "\nEnd of tenth test\n";
}
