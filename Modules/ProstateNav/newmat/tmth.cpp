
//#define WANT_STREAM

#include "include.h"

#include "newmatap.h"
//#include "newmatio.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif

static int my_max(int p, int q) { return (p > q) ? p : q; }

static int my_min(int p, int q) { return (p < q) ? p : q; }


void BandFunctions(int l1, int u1, int l2, int u2)
{
   int i, j;
   BandMatrix BM1(20, l1, u1); BM1 = 999999.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= 20; ++j)
      if (i - j <= l1 && i - j >= -u1) BM1(i, j) = 100 * i + j;

   BandMatrix BM2 = BM1; Matrix M = BM2 - BM1; Print(M);

   BM2.ReSize(20, l2, u2); BM2 = 777777.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= 20; ++j)
      if (i - j <= l2 && i - j >= -u2) BM2(i, j) = (100 * i + j) * 11;

   BandMatrix BMA = BM1 + BM2, BMS = BM1 - BM2, BMSP = SP(BM1, BM2),
      BMM = BM1 * BM2, BMN = -BM1;

   Matrix M1(20,20); M1 = 0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= 20; ++j)
      if (i - j <= l1 && i - j >= -u1) M1(i, j) = 100 * i + j;

   Matrix M2(20,20); M2 = 0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= 20; ++j)
      if (i - j <= l2 && i - j >= -u2) M2(i, j) = (100 * i + j) * 11;

   Matrix MA = M1 + M2, MS = M1 - M2, MSP = SP(M1, M2), MM = M1 * M2, MN = -M1;
   MA -= BMA; MS -= BMS; MSP -= BMSP; MM -= BMM; MN -= BMN;
   Print(MA); Print(MS); Print(MSP); Print(MM); Print(MN);

   Matrix Test(7, 2);
   Test(1,1) = BM1.BandWidth().Lower() - l1;
   Test(1,2) = BM1.BandWidth().Upper() - u1;
   Test(2,1) = BM2.BandWidth().Lower() - l2;
   Test(2,2) = BM2.BandWidth().Upper() - u2;
   Test(3,1) = BMA.BandWidth().Lower() - my_max(l1,l2);
   Test(3,2) = BMA.BandWidth().Upper() - my_max(u1,u2);
   Test(4,1) = BMS.BandWidth().Lower() - my_max(l1,l2);
   Test(4,2) = BMS.BandWidth().Upper() - my_max(u1,u2);
   Test(5,1) = BMSP.BandWidth().Lower() - my_min(l1,l2);
   Test(5,2) = BMSP.BandWidth().Upper() - my_min(u1,u2);
   Test(6,1) = BMM.BandWidth().Lower() - (l1 + l2);
   Test(6,2) = BMM.BandWidth().Upper() - (u1 + u2);
   Test(7,1) = BMN.BandWidth().Lower() - l1;
   Test(7,2) = BMN.BandWidth().Upper() - u1;
   Print(Test);

   // test element function
   BandMatrix BM1E(20, l1, u1); BM1E = 999999.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= 20; ++j)
      if (i - j <= l1 && i - j >= -u1) BM1E.element(i-1, j-1) = 100 * i + j;
   BandMatrix BM2E = BM1E; BM2E.ReSize(BM2); BM2E = 777777.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= 20; ++j)
      if (i - j <= l2 && i - j >= -u2)
         BM2E.element(i-1, j-1) = (100 * i + j) * 11;
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

   // test element function with constant
   BM1E = 444444.04; BM2E = 555555.0;
   const BandMatrix BM1C = BM1, BM2C = BM2;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= 20; ++j)
      if (i - j <= l1 && i - j >= -u1)
         BM1E.element(i-1, j-1) = BM1C.element(i-1, j-1);
   for (i = 1; i <= 20; ++i) for (j = 1; j <= 20; ++j)
      if (i - j <= l2 && i - j >= -u2)
         BM2E.element(i-1, j-1) = BM2C.element(i-1, j-1);
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

   // test subscript function with constant
   BM1E = 444444.04; BM2E = 555555.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= 20; ++j)
      if (i - j <= l1 && i - j >= -u1) BM1E(i, j) = BM1C(i, j);
   for (i = 1; i <= 20; ++i) for (j = 1; j <= 20; ++j)
      if (i - j <= l2 && i - j >= -u2) BM2E(i, j) = BM2C(i, j);
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);
}

void LowerBandFunctions(int l1, int l2)
{
   int i, j;
   LowerBandMatrix BM1(20, l1); BM1 = 999999.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) BM1(i, j) = 100 * i + j;

   LowerBandMatrix BM2 = BM1; Matrix M = BM2 - BM1; Print(M);

   BM2.ReSize(20, l2); BM2 = 777777.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) BM2(i, j) = (100 * i + j) * 11;

   LowerBandMatrix BMA = BM1 + BM2, BMS = BM1 - BM2, BMSP = SP(BM1, BM2),
      BMM = BM1 * BM2, BMN = -BM1;

   Matrix M1(20,20); M1 = 0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) M1(i, j) = 100 * i + j;

   Matrix M2(20,20); M2 = 0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) M2(i, j) = (100 * i + j) * 11;

   Matrix MA = M1 + M2, MS = M1 - M2, MSP = SP(M1, M2), MM = M1 * M2, MN = -M1;
   MA -= BMA; MS -= BMS; MSP -= BMSP; MM -= BMM; MN -= BMN;
   Print(MA); Print(MS); Print(MSP); Print(MM); Print(MN);

   Matrix Test(7, 2);
   Test(1,1) = BM1.BandWidth().Lower() - l1;
   Test(1,2) = BM1.BandWidth().Upper();
   Test(2,1) = BM2.BandWidth().Lower() - l2;
   Test(2,2) = BM2.BandWidth().Upper();
   Test(3,1) = BMA.BandWidth().Lower() - my_max(l1,l2);
   Test(3,2) = BMA.BandWidth().Upper();
   Test(4,1) = BMS.BandWidth().Lower() - my_max(l1,l2);
   Test(4,2) = BMS.BandWidth().Upper();
   Test(5,1) = BMSP.BandWidth().Lower() - my_min(l1,l2);
   Test(5,2) = BMSP.BandWidth().Upper();
   Test(6,1) = BMM.BandWidth().Lower() - (l1 + l2);
   Test(6,2) = BMM.BandWidth().Upper();
   Test(7,1) = BMN.BandWidth().Lower() - l1;
   Test(7,2) = BMN.BandWidth().Upper();
   Print(Test);

   // test element function
   LowerBandMatrix BM1E(20, l1); BM1E = 999999.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) BM1E.element(i-1, j-1) = 100 * i + j;
   LowerBandMatrix BM2E = BM1E; BM2E.ReSize(BM2); BM2E = 777777.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) BM2E.element(i-1, j-1) = (100 * i + j) * 11;
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

   // test element function with constant
   BM1E = 444444.04; BM2E = 555555.0;
   const LowerBandMatrix BM1C = BM1, BM2C = BM2;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) BM1E.element(i-1, j-1) = BM1C.element(i-1, j-1);
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) BM2E.element(i-1, j-1) = BM2C.element(i-1, j-1);
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

   // test subscript function with constant
   BM1E = 444444.04; BM2E = 555555.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) BM1E(i, j) = BM1C(i, j);
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) BM2E(i, j) = BM2C(i, j);
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);
}

void UpperBandFunctions(int u1, int u2)
{
   int i, j;
   UpperBandMatrix BM1(20, u1); BM1 = 999999.0;
   for (i = 1; i <= 20; ++i) for (j = i; j <= 20; ++j)
      if (i - j >= -u1) BM1(i, j) = 100 * i + j;

   UpperBandMatrix BM2 = BM1; Matrix M = BM2 - BM1; Print(M);

   BM2.ReSize(20, u2); BM2 = 777777.0;
   for (i = 1; i <= 20; ++i) for (j = i; j <= 20; ++j)
      if (i - j >= -u2) BM2(i, j) = (100 * i + j) * 11;

   UpperBandMatrix BMA = BM1 + BM2, BMS = BM1 - BM2, BMSP = SP(BM1, BM2),
      BMM = BM1 * BM2, BMN = -BM1;

   Matrix M1(20,20); M1 = 0;
   for (i = 1; i <= 20; ++i) for (j = i; j <= 20; ++j)
      if (i - j >= -u1) M1(i, j) = 100 * i + j;

   Matrix M2(20,20); M2 = 0;
   for (i = 1; i <= 20; ++i) for (j = i; j <= 20; ++j)
      if (i - j >= -u2) M2(i, j) = (100 * i + j) * 11;

   Matrix MA = M1 + M2, MS = M1 - M2, MSP = SP(M1, M2), MM = M1 * M2, MN = -M1;
   MA -= BMA; MS -= BMS; MSP -= BMSP; MM -= BMM; MN -= BMN;
   Print(MA); Print(MS); Print(MSP); Print(MM); Print(MN);

   Matrix Test(7, 2);
   Test(1,1) = BM1.BandWidth().Lower();
   Test(1,2) = BM1.BandWidth().Upper() - u1;
   Test(2,1) = BM2.BandWidth().Lower();
   Test(2,2) = BM2.BandWidth().Upper() - u2;
   Test(3,1) = BMA.BandWidth().Lower();
   Test(3,2) = BMA.BandWidth().Upper() - my_max(u1,u2);
   Test(4,1) = BMS.BandWidth().Lower();
   Test(4,2) = BMS.BandWidth().Upper() - my_max(u1,u2);
   Test(5,1) = BMSP.BandWidth().Lower();
   Test(5,2) = BMSP.BandWidth().Upper() - my_min(u1,u2);
   Test(6,1) = BMM.BandWidth().Lower();
   Test(6,2) = BMM.BandWidth().Upper() - (u1 + u2);
   Test(7,1) = BMN.BandWidth().Lower();
   Test(7,2) = BMN.BandWidth().Upper() - u1;
   Print(Test);

   // test element function
   UpperBandMatrix BM1E(20, u1); BM1E = 999999.0;
   for (i = 1; i <= 20; ++i) for (j = i; j <= 20; ++j)
      if (i - j >= -u1) BM1E.element(i-1, j-1) = 100 * i + j;
   UpperBandMatrix BM2E = BM1E; BM2E.ReSize(BM2); BM2E = 777777.0;
   for (i = 1; i <= 20; ++i) for (j = i; j <= 20; ++j)
      if (i - j >= -u2) BM2E.element(i-1, j-1) = (100 * i + j) * 11;
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

   // test element function with constant
   BM1E = 444444.04; BM2E = 555555.0;
   const UpperBandMatrix BM1C = BM1, BM2C = BM2;
   for (i = 1; i <= 20; ++i) for (j = i; j <= 20; ++j)
      if (i - j >= -u1) BM1E.element(i-1, j-1) = BM1C.element(i-1, j-1);
   for (i = 1; i <= 20; ++i) for (j = i; j <= 20; ++j)
      if (i - j >= -u2) BM2E.element(i-1, j-1) = BM2C.element(i-1, j-1);
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

   // test subscript function with constant
   BM1E = 444444.04; BM2E = 555555.0;
   for (i = 1; i <= 20; ++i) for (j = i; j <= 20; ++j)
      if (i - j >= -u1) BM1E(i, j) = BM1C(i, j);
   for (i = 1; i <= 20; ++i) for (j = i; j <= 20; ++j)
      if (i - j >= -u2) BM2E(i, j) = BM2C(i, j);
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);
}

void SymmetricBandFunctions(int l1, int l2)
{
   int i, j;
   SymmetricBandMatrix BM1(20, l1); BM1 = 999999.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) BM1(i, j) = 100 * i + j;

   SymmetricBandMatrix BM2 = BM1; Matrix M = BM2 - BM1; Print(M);

   BM2.ReSize(20, l2); BM2 = 777777.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) BM2(i, j) = (100 * i + j) * 11;

   SymmetricBandMatrix BMA = BM1 + BM2, BMS = BM1 - BM2, BMSP = SP(BM1, BM2),
      BMN = -BM1;
   BandMatrix BMM = BM1 * BM2;

   SymmetricMatrix M1(20); M1 = 0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) M1(i, j) = 100 * i + j;

   SymmetricMatrix M2(20); M2 = 0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) M2(i, j) = (100 * i + j) * 11;

   SymmetricMatrix MA = M1 + M2, MS = M1 - M2, MSP = SP(M1, M2), MN = -M1;
   Matrix MM = M1 * M2;
   MA -= BMA; MS -= BMS; MSP -= BMSP; MM -= BMM; MN -= BMN;
   Print(MA); Print(MS); Print(MSP); Print(MM); Print(MN);

   Matrix Test(7, 2);
   Test(1,1) = BM1.BandWidth().Lower() - l1;
   Test(1,2) = BM1.BandWidth().Upper() - l1;
   Test(2,1) = BM2.BandWidth().Lower() - l2;
   Test(2,2) = BM2.BandWidth().Upper() - l2;
   Test(3,1) = BMA.BandWidth().Lower() - my_max(l1,l2);
   Test(3,2) = BMA.BandWidth().Upper() - my_max(l1,l2);
   Test(4,1) = BMS.BandWidth().Lower() - my_max(l1,l2);
   Test(4,2) = BMS.BandWidth().Upper() - my_max(l1,l2);
   Test(5,1) = BMSP.BandWidth().Lower() - my_min(l1,l2);
   Test(5,2) = BMSP.BandWidth().Upper() - my_min(l1,l2);
   Test(6,1) = BMM.BandWidth().Lower() - (l1 + l2);
   Test(6,2) = BMM.BandWidth().Upper() - (l1 + l2);
   Test(7,1) = BMN.BandWidth().Lower() - l1;
   Test(7,2) = BMN.BandWidth().Upper() - l1;
   Print(Test);

   // test element function
   SymmetricBandMatrix BM1E(20, l1); BM1E = 999999.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) BM1E.element(i-1, j-1) = 100 * i + j;
   SymmetricBandMatrix BM2E = BM1E; BM2E.ReSize(BM2); BM2E = 777777.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) BM2E.element(i-1, j-1) = (100 * i + j) * 11;
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

   // reverse subscripts
   BM1E = 999999.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) BM1E.element(j-1, i-1) = 100 * i + j;
   BM2E = 777777.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) BM2E.element(j-1, i-1) = (100 * i + j) * 11;
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

   // test element function with constant
   BM1E = 444444.04; BM2E = 555555.0;
   const SymmetricBandMatrix BM1C = BM1, BM2C = BM2;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) BM1E.element(i-1, j-1) = BM1C.element(i-1, j-1);
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) BM2E.element(i-1, j-1) = BM2C.element(i-1, j-1);
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

   // reverse subscripts
   BM1E = 444444.0; BM2E = 555555.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) BM1E.element(j-1, i-1) = BM1C.element(j-1, i-1);
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) BM2E.element(j-1, i-1) = BM2C.element(j-1, i-1);
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

   // test subscript function with constant
   BM1E = 444444.0; BM2E = 555555.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) BM1E(i, j) = BM1C(i, j);
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) BM2E(i, j) = BM2C(i, j);
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

   // reverse subscripts
   BM1E = 444444.0; BM2E = 555555.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) BM1E(j, i) = BM1C(j, i);
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) BM2E(j, i) = BM2C(j, i);
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

   // partly reverse subscripts
   BM1E = 444444.0; BM2E = 555555.0;
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l1) BM1E(j, i) = BM1C(i, j);
   for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      if (i - j <= l2) BM2E(j, i) = BM2C(i, j);
   M1 = BM1E - BM1; Print(M1);
   M2 = BM2E - BM2; Print(M2);

}



void trymath()
{
//   cout << "\nSeventeenth test of Matrix package\n";
//   cout << "\n";
   Tracer et("Seventeenth test of Matrix package");
   Tracer::PrintTrace();


   {
      Tracer et1("Stage 1");
      int i, j;
      BandMatrix B(8,3,1);
      for (i=1; i<=8; i++) for (j=-3; j<=1; j++)
         { int k = i+j; if (k>0 && k<=8) B(i,k) = i + k/64.0; }

      IdentityMatrix I(8); BandMatrix B1; B1 = I;
      UpperTriangularMatrix UT = I; Print(Matrix(B1-UT));
      Print(Matrix(B * B - B * 2 + I - (B - I) * (B - I)));
      Matrix A = B; BandMatrix C; C = B;
      Print(Matrix(B * A - C * 2 + I - (A - I) * (B - I)));

      C.ReSize(8,2,2); C = 0.0; C.Inject(B);
      Matrix X = A.t();
      B1.ReSize(8,2,2); B1.Inject(X); Print(Matrix(C.t()-B1));

      Matrix BI = B.i(); A = A.i()-BI; Clean(A,0.000000001); Print(A);
      BandLUMatrix BLU = B.t();
      BI = BLU.i(); A = X.i()-BI; Clean(A,0.000000001); Print(A);
      X.ReSize(1,1);
      X(1,1) = BLU.LogDeterminant().Value()-B.LogDeterminant().Value();
      Clean(X,0.000000001); Print(X);

      UpperBandMatrix U; U << B; LowerBandMatrix L; L << B;
      DiagonalMatrix D; D << B;
      Print( Matrix(L + (U - D - B)) );



      for (i=1; i<=8; i++)  A.Column(i) << B.Column(i);
      Print(Matrix(A-B));
   }
   {
      Tracer et1("Stage 2");
      BandMatrix A(7,2,2);
      int i,j;
      for (i=1; i<=7; i++) for (j=1; j<=7; j++)
      {
         int k=i-j; if (k<0) k = -k;
         if (k==0) A(i,j)=6;
         else if (k==1) A(i,j) = -4;
         else if (k==2) A(i,j) = 1;
         A(1,1) = A(7,7) = 5;
      }
      DiagonalMatrix D(7); D = 0.0; A = A - D;
      BandLUMatrix B(A); Matrix M = A;
      ColumnVector V(6);
      V(1) = LogDeterminant(B).Value();
      V(2) = LogDeterminant(A).Value();
      V(3) = LogDeterminant(M).Value();
      V(4) = Determinant(B);
      V(5) = Determinant(A);
      V(6) = Determinant(M);
      V = V / 64 - 1; Clean(V,0.000000001); Print(V);
      ColumnVector X(7);

      Real a[] = {1,2,3,4,5,6,7};
      X << a;
      M = (M.i()*X).t() - (B.i()*X).t() * 2.0 + (A.i()*X).t();
      Clean(M,0.000000001); Print(M);


      BandMatrix P(80,2,5); ColumnVector CX(80);
      for (i=1; i<=80; i++) for (j=1; j<=80; j++)
      { int d = i-j; if (d<=2 && d>=-5) P(i,j) = i + j/100.0; }
      for (i=1; i<=80; i++)  CX(i) = i*100.0;
      Matrix MP = P;
      ColumnVector V1 = P.i() * CX; ColumnVector V2 = MP.i() * CX;
      V = V1 - V2; Clean(V,0.000000001); Print(V);

      V1 = P * V1; V2 = MP * V2; V = V1 - V2; Clean(V,0.000000001); Print(V);
      RowVector XX(1);
      XX = LogDeterminant(P).Value() / LogDeterminant(MP).Value() - 1.0;
      Clean(XX,0.000000001); Print(XX);

      LowerBandMatrix LP(80,5);
      for (i=1; i<=80; i++) for (j=1; j<=80; j++)
      { int d = i-j; if (d<=5 && d>=0) LP(i,j) = i + j/100.0; }
      MP = LP;
      XX.ReSize(4);
      XX(1) = LogDeterminant(LP).Value();
      XX(2) = LogDeterminant(MP).Value();
      V1 = LP.i() * CX; V2 = MP.i() * CX;
      V = V1 - V2; Clean(V,0.000000001); Print(V);

      UpperBandMatrix UP(80,4);
      for (i=1; i<=80; i++) for (j=1; j<=80; j++)
      { int d = i-j; if (d<=0 && d>=-4) UP(i,j) = i + j/100.0; }
      MP = UP;
      XX(3) = LogDeterminant(UP).Value();
      XX(4) = LogDeterminant(MP).Value();
      V1 = UP.i() * CX; V2 = MP.i() * CX;
      V = V1 - V2; Clean(V,0.000000001); Print(V);
      XX = XX / SumAbsoluteValue(XX) - .25; Clean(XX,0.000000001); Print(XX);
   }

   {
      Tracer et1("Stage 3");
      SymmetricBandMatrix SA(8,5);
      int i,j;
      for (i=1; i<=8; i++) for (j=1; j<=8; j++)
         if (i-j<=5 && 0<=i-j) SA(i,j) =i + j/128.0;
      DiagonalMatrix D(8); D = 10; SA = SA + D;

      Matrix MA1(8,8); Matrix MA2(8,8);
      for (i=1; i<=8; i++)
         { MA1.Column(i) << SA.Column(i); MA2.Row(i) << SA.Row(i); }
      Print(Matrix(MA1-MA2));

      D = 10; SA = SA.t() + D; MA1 = MA1 + D;
      Print(Matrix(MA1-SA));

      UpperBandMatrix UB(8,3); LowerBandMatrix LB(8,4);
      D << SA; UB << SA; LB << SA;
      SA = SA * 5.0; D = D * 5.0; LB = LB * 5.0; UB = UB * 5.0;
      BandMatrix B = LB - D + UB - SA; Print(Matrix(B));

      SymmetricBandMatrix A(7,2); A = 100.0;
      for (i=1; i<=7; i++) for (j=1; j<=7; j++)
      {
         int k=i-j;
         if (k==0) A(i,j)=6;
         else if (k==1) A(i,j) = -4;
         else if (k==2) A(i,j) = 1;
         A(1,1) = A(7,7) = 5;
      }
      BandLUMatrix C(A); Matrix M = A;
      ColumnVector X(8);
      X(1) = LogDeterminant(C).Value() - 64;
      X(2) = LogDeterminant(A).Value() - 64;
      X(3) = LogDeterminant(M).Value() - 64;
      X(4) = SumSquare(M) - SumSquare(A);
      X(5) = SumAbsoluteValue(M) - SumAbsoluteValue(A);
      X(6) = MaximumAbsoluteValue(M) - MaximumAbsoluteValue(A);
      X(7) = Trace(M) - Trace(A);
      X(8) = Sum(M) - Sum(A);
      Clean(X,0.000000001); Print(X);

      Real a[] = {1,2,3,4,5,6,7};
      X.ReSize(7);
      X << a;
      X = M.i()*X - C.i()*X * 2 + A.i()*X;
      Clean(X,0.000000001); Print(X);


      LB << A; UB << A; D << A;
      BandMatrix XA = LB + (UB - D);
      Print(Matrix(XA - A));

      for (i=1; i<=7; i++) for (j=1; j<=7; j++)
      {
         int k=i-j;
         if (k==0) A(i,j)=6;
         else if (k==1) A(i,j) = -4;
         else if (k==2) A(i,j) = 1;
         A(1,1) = A(7,7) = 5;
      }
      D = 1;

      M = LB.i() * LB - D; Clean(M,0.000000001); Print(M);
      M = UB.i() * UB - D; Clean(M,0.000000001); Print(M);
      M = XA.i() * XA - D; Clean(M,0.000000001); Print(M);
      Matrix MUB = UB; Matrix MLB = LB;
      M = LB.i() * UB - LB.i() * MUB; Clean(M,0.000000001); Print(M);
      M = UB.i() * LB - UB.i() * MLB; Clean(M,0.000000001); Print(M);
      M = LB.i() * UB - LB.i() * Matrix(UB); Clean(M,0.000000001); Print(M);
      M = UB.i() * LB - UB.i() * Matrix(LB); Clean(M,0.000000001); Print(M);
   }

   {
      // some tests about adding and subtracting band matrices of different
      // sizes - check bandwidth of results
      Tracer et1("Stage 4");

      BandFunctions(9, 3, 9, 3);   // equal
      BandFunctions(4, 7, 4, 7);   // equal
      BandFunctions(9, 3, 5, 8);   // neither < or >
      BandFunctions(5, 8, 9, 3);   // neither < or >
      BandFunctions(9, 8, 5, 3);   // >
      BandFunctions(3, 5, 8, 9);   // <

      LowerBandFunctions(9, 9);    // equal
      LowerBandFunctions(4, 4);    // equal
      LowerBandFunctions(9, 5);    // >
      LowerBandFunctions(3, 8);    // <

      UpperBandFunctions(3, 3);    // equal
      UpperBandFunctions(7, 7);    // equal
      UpperBandFunctions(8, 3);    // >
      UpperBandFunctions(5, 9);    // <

      SymmetricBandFunctions(9, 9);   // equal
      SymmetricBandFunctions(4, 4);   // equal
      SymmetricBandFunctions(9, 5);   // >
      SymmetricBandFunctions(3, 8);   // <

      DiagonalMatrix D(6); D << 2 << 3 << 4.5 << 1.25 << 9.5 << -5;
      BandMatrix BD = D;
      UpperBandMatrix UBD; UBD = D;
      LowerBandMatrix LBD; LBD = D;
      SymmetricBandMatrix SBD = D;
      Matrix X = BD - D; Print(X); X = UBD - D; Print(X);
      X = LBD - D; Print(X); X = SBD - D; Print(X);
      Matrix Test(9,2);
      Test(1,1) =  BD.BandWidth().Lower(); Test(1,2) =  BD.BandWidth().Upper();
      Test(2,1) = UBD.BandWidth().Lower(); Test(2,2) = UBD.BandWidth().Upper();
      Test(3,1) = LBD.BandWidth().Lower(); Test(3,2) = LBD.BandWidth().Upper();
      Test(4,1) = SBD.BandWidth().Lower(); Test(4,2) = SBD.BandWidth().Upper();

      IdentityMatrix I(10); I *= 5;
      BD = I; UBD = I; LBD = I; SBD = I;
      X = BD - I; Print(X); X = UBD - I; Print(X);
      X = LBD - I; Print(X); X = SBD - I; Print(X);
      Test(5,1) =  BD.BandWidth().Lower(); Test(5,2) =  BD.BandWidth().Upper();
      Test(6,1) = UBD.BandWidth().Lower(); Test(6,2) = UBD.BandWidth().Upper();
      Test(7,1) = LBD.BandWidth().Lower(); Test(7,2) = LBD.BandWidth().Upper();
      Test(8,1) = SBD.BandWidth().Lower(); Test(8,2) = SBD.BandWidth().Upper();

      RowVector RV = D.AsRow(); I.ReSize(6); BandMatrix BI = I; I = 1;
      BD =  RV.AsDiagonal() +  BI; X =  BD - D - I; Print(X);
      Test(9,1) =  BD.BandWidth().Lower(); Test(9,2) =  BD.BandWidth().Upper();

      Print(Test);
   }

   {
      // various element functions
      Tracer et1("Stage 5");

      int i, j;
      Matrix Count(1, 1); Count = 0;  // for counting errors
      Matrix M(20,30);
      for (i = 1; i <= 20; ++i) for (j = 1; j <= 30; ++j)
         M(i, j) = 100 * i + j;
      const Matrix CM = M;
      for (i = 1; i <= 20; ++i) for (j = 1; j <= 30; ++j)
      {
         if (M(i, j) != CM(i, j)) ++Count(1,1);
         if (M(i, j) != CM.element(i-1, j-1)) ++Count(1,1);
         if (M(i, j) != M.element(i-1, j-1)) ++Count(1,1);
      }

      UpperTriangularMatrix U(20);
      for (i = 1; i <= 20; ++i) for (j = i; j <= 20; ++j)
         U(i, j) = 100 * i + j;
      const UpperTriangularMatrix CU = U;
      for (i = 1; i <= 20; ++i) for (j = i; j <= 20; ++j)
      {
         if (U(i, j) != CU(i, j)) ++Count(1,1);
         if (U(i, j) != CU.element(i-1, j-1)) ++Count(1,1);
         if (U(i, j) != U.element(i-1, j-1)) ++Count(1,1);
      }

      LowerTriangularMatrix L(20);
      for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
         L(i, j) = 100 * i + j;
      const LowerTriangularMatrix CL = L;
      for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
      {
         if (L(i, j) != CL(i, j)) ++Count(1,1);
         if (L(i, j) != CL.element(i-1, j-1)) ++Count(1,1);
         if (L(i, j) != L.element(i-1, j-1)) ++Count(1,1);
      }

      SymmetricMatrix S(20);
      for (i = 1; i <= 20; ++i) for (j = 1; j <= i; ++j)
         S(i, j) = 100 * i + j;
      const SymmetricMatrix CS = S;
      for (i = 1; i <= 20; ++i) for (j = 1; j <= 20; ++j)
      {
         if (S(i, j) != CS(i, j)) ++Count(1,1);
         if (S(i, j) != CS.element(i-1, j-1)) ++Count(1,1);
         if (S(i, j) != S.element(i-1, j-1)) ++Count(1,1);
         if (S(i, j) != S(j, i)) ++Count(1,1);
         if (S(i, j) != CS(i, j)) ++Count(1,1);
         if (S(i, j) != CS.element(i-1, j-1)) ++Count(1,1);
         if (S(i, j) != S.element(i-1, j-1)) ++Count(1,1);
      }

      DiagonalMatrix D(20);
      for (i = 1; i <= 20; ++i) D(i) = 100 * i + i * i;
      const DiagonalMatrix CD = D;
      for (i = 1; i <= 20; ++i)
      {
         if (D(i, i) != CD(i, i)) ++Count(1,1);
         if (D(i, i) != CD.element(i-1, i-1)) ++Count(1,1);
         if (D(i, i) != D.element(i-1, i-1)) ++Count(1,1);
         if (D(i, i) != D(i)) ++Count(1,1);
         if (D(i) != CD(i)) ++Count(1,1);
         if (D(i) != CD.element(i-1)) ++Count(1,1);
         if (D(i) != D.element(i-1)) ++Count(1,1);
      }

      RowVector R(20);
      for (i = 1; i <= 20; ++i) R(i) = 100 * i + i * i;
      const RowVector CR = R;
      for (i = 1; i <= 20; ++i)
      {
         if (R(i) != CR(i)) ++Count(1,1);
         if (R(i) != CR.element(i-1)) ++Count(1,1);
         if (R(i) != R.element(i-1)) ++Count(1,1);
      }

      ColumnVector C(20);
      for (i = 1; i <= 20; ++i) C(i) = 100 * i + i * i;
      const ColumnVector CC = C;
      for (i = 1; i <= 20; ++i)
      {
         if (C(i) != CC(i)) ++Count(1,1);
         if (C(i) != CC.element(i-1)) ++Count(1,1);
         if (C(i) != C.element(i-1)) ++Count(1,1);
      }

      Print(Count);

   }

   {
      // resize to another matrix size
      Tracer et1("Stage 6");

      Matrix A(20, 30); A = 3;
      Matrix B(3, 4);
      B.ReSize(A); B = 6; B -= 2 * A; Print(B);

      A.ReSize(25,25); A = 12;

      UpperTriangularMatrix U(5);
      U.ReSize(A); U = 12; U << (U - A); Print(U);

      LowerTriangularMatrix L(5);
      L.ReSize(U); L = 12; L << (L - A); Print(L);

      DiagonalMatrix D(5);
      D.ReSize(U); D = 12; D << (D - A); Print(D);

      SymmetricMatrix S(5);
      S.ReSize(U); S = 12; S << (S - A); Print(S);

      IdentityMatrix I(5);
      I.ReSize(U); I = 12; D << (I - A); Print(D);

      A.ReSize(10, 1); A = 17;
      ColumnVector C(5); C.ReSize(A); C = 17; C -= A; Print(C);

      A.ReSize(1, 10); A = 15;
      RowVector R(5); R.ReSize(A); R = 15; R -= A; Print(R);

   }

   {
      // generic matrix and identity matrix
      Tracer et1("Stage 7");
      IdentityMatrix I(5);
      I *= 4;
      GenericMatrix GM = I;
      GM /= 2;
      DiagonalMatrix D = GM;
      Matrix A = GM + 10;
      A -= 10;
      A -= D;
      Print(A);
   }

   {
      // SP and upper and lower triangular matrices
      Tracer et1("Stage 8");
      UpperTriangularMatrix UT(4);
      UT << 3 << 7 << 3 << 9
              << 5 << 2 << 6
                   << 8 << 0
                        << 4;
      LowerTriangularMatrix LT; LT.ReSize(UT);
      LT << 2
         << 7 << 9
         << 2 << 8 << 6
         << 1 << 0 << 3 << 5;

      DiagonalMatrix D = SP(UT, LT);
      DiagonalMatrix D1(4);
      D1 << 6 << 45 << 48 << 20;
      D -= D1; Print(D);
      BandMatrix BM = SP(UT, LT);
      Matrix X = BM - D1; Print(X);
      RowVector RV(2);
      RV(1) = BM.BandWidth().Lower();
      RV(2) = BM.BandWidth().Upper();
      Print(RV);
   }
   
   {
      // Helmert multiplies
      Tracer et1("Stage 9");
      MultWithCarry MCW;
      int i, j;

      IdentityMatrix I(8);
      Matrix X = I;
      Matrix Y = Helmert_transpose(X);
      Matrix H = Helmert(9); H -= Y.t(); Clean(H,0.000000001); Print(H);
      Matrix Z = Helmert(Y) - I;
      Clean(Z,0.000000001); Print(Z);
      
      Matrix A(9, 8);
      for (i = 1; i <= 9; ++i) for (j = 1; j <= 8; ++j)
         A(i, j) = Helmert_transpose(X.column(j), i);
      A -= Y; Clean(A,0.000000001); Print(A); 
   
      X = I;
      Y = Helmert_transpose(X, true);
      H = Helmert(8, true); H -= Y.t(); Clean(H,0.000000001); Print(H);
      Z = Helmert(Y, true) - I;
      Clean(Z,0.000000001); Print(Z);
      
      A.resize(8, 8);
      for (i = 1; i <= 8; ++i) for (j = 1; j <= 8; ++j)
         A(i, j) = Helmert_transpose(X.column(j), i, true);
      A -= Y; Clean(A,0.000000001); Print(A); 



      I.ReSize(9);
      X = I;
      Y = Helmert(X, true);
      H = Helmert(9, true); H -= Y; Clean(H,0.000000001); Print(H);
      Z = Helmert_transpose(Y, true) - I;
      Clean(Z,0.000000001); Print(Z);
   
      A.ReSize(9, 9);
      for (i = 1; i <= 9; ++i) A.Column(i) = Helmert(9, i, true);
      A -= Y; Clean(A,0.000000001); Print(A);

      Y = Helmert(X);
      A.ReSize(8, 9);
      for (i = 1; i <= 9; ++i) A.Column(i) = Helmert(9, i);
      A -= Y; Clean(A,0.000000001); Print(A);
      
      ColumnVector Twos(100); Twos = 2;
      ColumnVector CV = Helmert(Twos); Clean(CV,0.000000001); Print(CV);
      
      X.resize(25,30);
      FillWithValues(MCW, X);
      Y = Helmert(X);
      Z = Helmert(X,true).rows(1,24) - Y;
      Clean(Z,0.000000001); Print(Z);
      Z = Helmert(X,true).row(25) - X.sum_columns() / 5.0;
      Clean(Z,0.000000001); Print(Z);
      
      I.resize(15);
      X = I;
      Z = Helmert_transpose(X, true) - Helmert(X, true).t();
      Clean(Z,0.000000001); Print(Z);
      I.resize(14); Y = I;
      Z = Helmert(X) - Helmert_transpose(Y).t();
      Clean(Z,0.000000001); Print(Z);
      
      
      
   }
      
      
      
      
      

//   cout << "\nEnd of Seventeenth test\n";
}

