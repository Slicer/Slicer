
#define WANT_STREAM

#define WANT_MATH

#include "newmat.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif



// test maxima and minima

Real TestMax(const GenericMatrix& GM)
{
   Matrix M = GM;
   ColumnVector CV = GM.AsColumn();

   int c, i, j, k; int n = CV.Nrows(), nr = M.Nrows(), nc = M.Ncols();
   Real x1, x2, x3;

   MatrixBandWidth mbw = GM.BandWidth();
   int u = mbw.Upper(); int l = mbw.Lower();
   bool IsBandMatrix = u > 0 && l > 0 && !(u == 0 && l == 0);

   x1 = GM.MaximumAbsoluteValue();
   x2 = GM.MaximumAbsoluteValue1(i);
   if (fabs(CV(i)) != x2) return 1.1;
   x3 = GM.MaximumAbsoluteValue2(i,j);
   if (fabs(M(i,j)) != x3) return 1.2;
   if (x3 != x1) return 1.3;
   if (x2 != x1) return 1.4;
   c = 0;
   for (k = 1; k <= n; k++)
   {
      Real cvk = fabs(CV(k));
      if (x1 <= cvk) { if (x1 < cvk) return 1.5; else c++; }
   }
   if (c == 0) return 1.6;


   x1 = GM.MinimumAbsoluteValue();
   x2 = GM.MinimumAbsoluteValue1(i);
   if (fabs(CV(i)) != x2) return 2.1;
   x3 = GM.MinimumAbsoluteValue2(i,j);
   if (fabs(M(i,j)) != x3) return 2.2;
   if (x3 != x1) return 2.3;
   if (x2 != CV.MinimumAbsoluteValue()) return 2.4;
   c = 0;
   if (IsBandMatrix)
   {
      for (i = 1; i <= nr; i++) for (j = 1; j <= nc; j++)
         if (i - j <= l && j - i <= u)
      {
         Real mij = fabs(M(i,j));
         if (x1 >= mij) { if (x1 > mij) return 2.51; else c++; }
      }
   }
   else
   {
      for (k = 1; k <= n; k++)
      {
         Real cvk = fabs(CV(k));
         if (x1 >= cvk) { if (x1 > cvk) return 2.52; else c++; }
      }
   }
   if (c == 0) return 2.6;

   x1 = GM.Maximum();
   x2 = GM.Maximum1(i);
   if (CV(i) != x2) return 3.1;
   x3 = GM.Maximum2(i,j);
   if (M(i,j) != x3) return 3.2;
   if (x3 != x1) return 3.3;
   if (x2 != CV.Maximum()) return 3.4;
   c = 0;
   if (IsBandMatrix)
   {
      for (i = 1; i <= nr; i++) for (j = 1; j <= nc; j++)
         if (i - j <= l && j - i <= u)
      {
         Real mij = M(i,j);
         if (x1 <= mij) { if (x1 < mij) return 3.51; else c++; }
      }
   }
   else
   {
      for (k = 1; k <= n; k++)
      {
         Real cvk = CV(k);
         if (x1 <= cvk) { if (x1 < cvk) return 3.52; else c++; }
      }
   }
   if (c == 0) return 3.6;

   x1 = GM.Minimum();
   x2 = GM.Minimum1(i);
   if (CV(i) != x2) return 4.1;
   x3 = GM.Minimum2(i,j);
   if (M(i,j) != x3) return 4.2;
   if (x3 != x1) return 4.3;
   if (x2 != CV.Minimum()) return 4.4;
   c = 0;
   if (IsBandMatrix)
   {
      for (i = 1; i <= nr; i++) for (j = 1; j <= nc; j++)
         if (i - j <= l && j - i <= u)
      {
         Real mij = M(i,j);
         if (x1 >= mij) { if (x1 > mij) return 4.51; else c++; }
      }
   }
   else
   {
      for (k = 1; k <= n; k++)
      {
         Real cvk = CV(k);
         if (x1 >= cvk) { if (x1 > cvk) return 4.52; else c++; }
      }
   }
   if (c == 0) return 4.6;

   return 0;

}


void trymatl()
{
   Tracer et("Twenty first test of Matrix package");
   Tracer::PrintTrace();

   Matrix M(4, 4);
   M <<  1.5 <<  1.0 << -4.0 <<  4.5
     <<  3.5 <<  7.0 <<  2.0 << -1.0
     << -1.5 <<  7.5 << -6.0 <<  5.0
     <<  0.5 << -8.0 <<  5.5 <<  4.0;
   UpperTriangularMatrix UM;  UM << M;
   LowerTriangularMatrix LM;  LM << -M;
   SymmetricMatrix SM; SM << (UM + UM.t());
   BandMatrix BM(4, 1, 2);  BM.Inject(M);
   DiagonalMatrix DM; DM << M;
   SymmetricBandMatrix SBM(4,1); SBM.Inject(M);
   RowVector Test(28); int k = 0;

   // tests for different shapes
   Test(++k) = TestMax(GenericMatrix(M));
   Test(++k) = TestMax(GenericMatrix(UM));
   Test(++k) = TestMax(GenericMatrix(LM));
   Test(++k) = TestMax(GenericMatrix(SM));
   Test(++k) = TestMax(GenericMatrix(DM));
   Test(++k) = TestMax(GenericMatrix(BM));
   Test(++k) = TestMax(GenericMatrix(SBM));

   // tests for constant matrices - don't put non-zeros in corners of BM
   Matrix MX(4,4);
   MX = 1; Test(++k) = TestMax(GenericMatrix(MX));
   BM.Inject(MX); Test(++k) = TestMax(GenericMatrix(BM));
   MX = 0; Test(++k) = TestMax(GenericMatrix(MX));
   BM.Inject(MX); Test(++k) = TestMax(GenericMatrix(BM));
   MX = -1; Test(++k) = TestMax(GenericMatrix(MX));
   BM.Inject(MX); Test(++k) = TestMax(GenericMatrix(BM));

   // test for non-square
   MX = M | (2 * M).t(); Test(++k) = TestMax(GenericMatrix(MX));

   // test on row and column vector
   RowVector RV(6);
   RV << 1 << 3 << -5 << 2 << -4 << 3;
   Test(++k) = TestMax(GenericMatrix(RV));
   Test(++k) = TestMax(GenericMatrix(RV.t()));

   // test for function form
   Test(++k) = (MaximumAbsoluteValue(RV) - 5);
   Test(++k) = (MinimumAbsoluteValue(RV) - 1);
   Test(++k) = (Maximum(RV) - 3);
   Test(++k) = (Minimum(RV) + 5);
   Test(++k) = (MaximumAbsoluteValue(-RV) - 5);
   Test(++k) = (MinimumAbsoluteValue(-RV) - 1);
   Test(++k) = (Maximum(-RV) - 5);
   Test(++k) = (Minimum(-RV) + 3);

   // test formulae
   RowVector RV2(6);
   RV2 << 2 << 8 << 1 << 9 << 3 << -1;
   Test(++k) = (MaximumAbsoluteValue(RV+RV2) - 11);
   Test(++k) = (MinimumAbsoluteValue(RV+RV2) - 1);
   Test(++k) = (Maximum(RV+RV2) - 11);
   Test(++k) = (Minimum(RV+RV2) + 4);


   Print(Test);
}

