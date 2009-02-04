
//#define WANT_STREAM
#define WANT_MATH

#include "include.h"

#include "newmatap.h"

//#include "newmatio.h"

#include "tmt.h"

#ifdef use_namespace
using namespace NEWMAT;
#endif



static void SlowFT(const ColumnVector& a, const ColumnVector&b,
   ColumnVector& x, ColumnVector& y)
{
   int n = a.Nrows();
   x.ReSize(n); y.ReSize(n);
   Real f = 6.2831853071795864769/n;
   for (int j=1; j<=n; j++)
   {
      Real sumx = 0.0; Real sumy = 0.0;
      for (int k=1; k<=n; k++)
      {
         Real theta = - (j-1) * (k-1) * f;
         Real c = cos(theta); Real s = sin(theta);
         sumx += c * a(k) - s * b(k); sumy += s * a(k) + c * b(k);
      }
      x(j) = sumx; y(j) = sumy;
   }
}

static void SlowDTT_II(const ColumnVector& a, ColumnVector& c, ColumnVector& s)
{
   int n = a.Nrows(); c.ReSize(n); s.ReSize(n);
   Real f = 6.2831853071795864769 / (4*n);
   int k;

   for (k=1; k<=n; k++)
   {
      Real sum = 0.0;
      const int k1 = k-1;              // otherwise Visual C++ 5 fails
      for (int j=1; j<=n; j++) sum += cos(k1 * (2*j-1) * f) * a(j);
      c(k) = sum;
   }

   for (k=1; k<=n; k++)
   {
      Real sum = 0.0;
      for (int j=1; j<=n; j++) sum += sin(k * (2*j-1) * f) * a(j);
      s(k) = sum;
   }
}

static void SlowDTT(const ColumnVector& a, ColumnVector& c, ColumnVector& s)
{
   int n1 = a.Nrows(); int n = n1 - 1;
   c.ReSize(n1); s.ReSize(n1);
   Real f = 6.2831853071795864769 / (2*n);
   int k;

   int sign = 1;
   for (k=1; k<=n1; k++)
   {
      Real sum = 0.0;
      for (int j=2; j<=n; j++) sum += cos((j-1) * (k-1) * f) * a(j);
      c(k) = sum + (a(1) + sign * a(n1)) / 2.0;
      sign = -sign;
   }

   for (k=2; k<=n; k++)
   {
      Real sum = 0.0;
      for (int j=2; j<=n; j++) sum += sin((j-1) * (k-1) * f) * a(j);
      s(k) = sum;
   }
   s(1) = s(n1) = 0;
}

void SlowFT2(const Matrix& U, const Matrix& V, Matrix& X, Matrix& Y)
{
   Tracer trace("SlowFT2");
   int m = U.Nrows(); int n = U.Ncols();
   if (m != V.Nrows() || n != V.Ncols() || m == 0 || n == 0)
      Throw(ProgramException("Matrix dimensions unequal or zero", U, V));
   X.ReSize(U); Y.ReSize(V);
   const Real pi2 = atan(1.0) * 8.0;
   for (int i = 0; i < m; ++i) for (int j = 0; j < n; ++j)
   {
      Real sumr = 0.0, sumi = 0.0;
      for (int k = 0; k < m; ++k) for (int l = 0; l < n; ++l)
      {
         Real a = -pi2 * ( (Real)k * (Real)i / (Real)m
            + (Real)j * (Real)l / (Real)n );
         Real cs = cos(a); Real sn = sin(a);
         sumr += cs * U(k+1,l+1) - sn * V(k+1,l+1);
         sumi += cs * V(k+1,l+1) + sn * U(k+1,l+1);
      }
      X(i+1,j+1) = sumr; Y(i+1,j+1) = sumi;
   }
}


static void test(int n)
{
   Tracer et("Test FFT");
   MultWithCarry mwc;

   ColumnVector A(n), B(n), X, Y;
   for (int i=0; i<n; i++)
      { A.element(i) = mwc.Next(); B.element(i) = mwc.Next(); }
   FFT(A, B, X, Y); FFTI(X, Y, X, Y);
   X = X - A; Y = Y - B;
   Clean(X,0.000000001); Clean(Y,0.000000001); Print(X); Print(Y);
}

static void test1(int n)
{
   Tracer et("Test RealFFT");
   MultWithCarry mwc;

   ColumnVector A(n), B(n), X, Y;
   for (int i=0; i<n; i++) A.element(i) = mwc.Next();
   B = 0.0;
   FFT(A, B, X, Y);
   B.CleanUp();                 // release some space
   int n2 = n/2+1;
   ColumnVector X1,Y1,X2,Y2;
   RealFFT(A, X1, Y1);
   X2 = X1 - X.Rows(1,n2); Y2 = Y1 - Y.Rows(1,n2);
   Clean(X2,0.000000001); Clean(Y2,0.000000001); Print(X2); Print(Y2);
   X2.CleanUp(); Y2.CleanUp();  // release some more space
   RealFFTI(X1,Y1,B);
   B = A - B;
   Clean(B,0.000000001); Print(B);
}

static void test2(int n)
{
   Tracer et("cf FFT and slow FT");
   MultWithCarry mwc;

   ColumnVector A(n), B(n), X, Y, X1, Y1;
   for (int i=0; i<n; i++)
      { A.element(i) = mwc.Next(); B.element(i) = mwc.Next(); }
   FFT(A, B, X, Y);
   SlowFT(A, B, X1, Y1);
   X = X - X1; Y = Y - Y1;
   Clean(X,0.000000001); Clean(Y,0.000000001); Print(X); Print(Y);
}

static void test3(int n)
{
   Tracer et("cf slow and fast DCT_II and DST_II");
   MultWithCarry mwc;

   ColumnVector A(n), X, Y, X1, Y1;
   for (int i=0; i<n; i++) A.element(i) = mwc.Next();
   DCT_II(A, X); DST_II(A, Y);
   SlowDTT_II(A, X1, Y1);
   X -= X1; Y -= Y1;
   Clean(X,0.000000001); Clean(Y,0.000000001); Print(X); Print(Y);
}

static void test4(int n)
{
   Tracer et("Test DCT_II");
   MultWithCarry mwc;

   ColumnVector A1(n);
   for (int i=0; i<n; i++) A1.element(i) = mwc.Next();
   // do DCT II by ordinary FFT
   ColumnVector P(2*n), Q(2*n);
   P = 0.0; Q = 0.0; P.Rows(1,n) = A1;
   FFT(P, Q, P, Q);
   ColumnVector B1(n);
   for (int k=0; k<n; k++)
   {
      Real arg = k * 6.2831853071795864769 / (4 * n);
      B1(k+1) = P(k+1) * cos(arg) + Q(k+1) * sin(arg);
   }
   // use DCT_II routine
   ColumnVector B2;
   DCT_II(A1,B2);
   // test inverse
   ColumnVector A2;
   DCT_II_inverse(B2,A2);
   A1 -= A2; B1 -= B2;
   Clean(A1,0.000000001); Clean(B1,0.000000001); Print(A1); Print(B1);
}

static void test5(int n)
{
   Tracer et("Test DST_II");
   MultWithCarry mwc;

   ColumnVector A1(n);
   for (int i=0; i<n; i++) A1.element(i) = mwc.Next();
   // do DST II by ordinary FFT
   ColumnVector P(2*n), Q(2*n);
   P = 0.0; Q = 0.0; P.Rows(1,n) = A1;
   FFT(P, Q, P, Q);
   ColumnVector B1(n);
   for (int k=1; k<=n; k++)
   {
      Real arg = k * 6.2831853071795864769 / (4 * n);
      B1(k) = P(k+1) * sin(arg) - Q(k+1) * cos(arg);
   }
   // use DST_II routine
   ColumnVector B2;
   DST_II(A1,B2);
   // test inverse
   ColumnVector A2;
   DST_II_inverse(B2,A2);
   A1 -= A2; B1 -= B2;
   Clean(A1,0.000000001); Clean(B1,0.000000001); Print(A1); Print(B1);
}

static void test6(int n)
{
   Tracer et("Test DST");
   MultWithCarry mwc;

   ColumnVector A1(n+1);
   A1(1) = A1(n+1) = 0;
   for (int i=1; i<n; i++) A1.element(i) = mwc.Next();

   // do DST by ordinary FFT
   ColumnVector P(2*n), Q(2*n); P = 0.0; Q = 0.0; P.Rows(1,n+1) = A1;
   FFT(P, Q, P, Q);
   ColumnVector B1 = -Q.Rows(1,n+1);
   // use DST routine
   ColumnVector B2;
   DST(A1,B2);
   // test inverse
   ColumnVector A2;
   DST_inverse(B2,A2);
   A1 -= A2; B1 -= B2;
   Clean(A1,0.000000001); Clean(B1,0.000000001); Print(A1); Print(B1);
}



static void test7(int n)
{
   Tracer et("Test DCT");
   MultWithCarry mwc;

   ColumnVector A1(n+1);
   for (int i=0; i<=n; i++) A1.element(i) = mwc.Next();

   // do DCT by ordinary FFT
   ColumnVector P(2*n), Q(2*n); P = 0.0; Q = 0.0; P.Rows(1,n+1) = A1;
   P(1) /= 2.0; P(n+1) /= 2.0;
   FFT(P, Q, P, Q);
   ColumnVector B1 = P.Rows(1,n+1);
   // use DCT routine
   ColumnVector B2;
   DCT(A1,B2);
   // test inverse
   ColumnVector A2;
   DCT_inverse(B2,A2);
   A1 -= A2; B1 -= B2;
   Clean(A1,0.000000001); Clean(B1,0.000000001); Print(A1); Print(B1);
}

static void test8(int n)
{
   Tracer et("cf slow and fast DCT and DST");
   MultWithCarry mwc;

   ColumnVector A(n+1), X, Y, X1, Y1;
   for (int i=0; i<=n; i++) A.element(i) = mwc.Next();

   DCT(A, X); DST(A, Y);
   SlowDTT(A, X1, Y1);
   X -= X1; Y -= Y1;
   Clean(X,0.000000001); Clean(Y,0.000000001); Print(X); Print(Y);
}

static void test9(int m, int n)
{
   Tracer et("cf FFT2 and slow FT2");
   MultWithCarry mwc;

   Matrix A(m,n), B(m,n), X, Y, X1, Y1;
   for (int i=0; i<m; i++) for (int j=0; j<n; j++)
      { A.element(i,j) = mwc.Next(); B.element(i,j) = mwc.Next(); }
   FFT2(A, B, X, Y);
   SlowFT2(A, B, X1, Y1);
   X = X - X1; Y = Y - Y1;
   Clean(X,0.000000001); Clean(Y,0.000000001); Print(X); Print(Y);
   FFT2I(X1, Y1, X1, Y1);
   X1 -= A; Y1 -= B;
   Clean(X1,0.000000001); Clean(Y1,0.000000001); Print(X1); Print(Y1);   
}




void trymatf()
{
   Tracer et("Fifteenth test of Matrix package");
   Tracer::PrintTrace();

   int i;
   ColumnVector A(12), B(12);
   for (i = 1; i <=12; i++)
   {
      Real i1 = i - 1;
      A(i) = .7
           + .2 * cos(6.2831853071795864769 * 4.0 * i1 / 12)
           + .3 * sin(6.2831853071795864769 * 3.0 * i1 / 12);
      B(i) = .9
           + .5 * sin(6.2831853071795864769 * 2.0 * i1 / 12)
           + .4 * cos(6.2831853071795864769 * 1.0 * i1 / 12);
   }
   FFT(A, B, A, B);
   A(1) -= 8.4; A(3) -= 3.0; A(5) -= 1.2; A(9) -= 1.2; A(11) += 3.0;
   B(1) -= 10.8; B(2) -= 2.4; B(4) += 1.8; B(10) -= 1.8; B(12) -= 2.4;
   Clean(A,0.000000001); Clean(B,0.000000001); Print(A); Print(B);


   // test FFT
   test(2048); test(2000); test(27*81); test(2310); test(49*49);
   test(13*13*13); test(43*47);
   test(16*16*3); test(16*16*5); test(16*16*7);
   test(8*8*5);

   // test realFFT
   test1(2); test1(98); test1(22); test1(100);
   test1(2048); test1(2000); test1(35*35*2);

   // compare FFT and slowFFT
   test2(1); test2(13); test2(12); test2(9); test2(16); test2(30); test2(42);
   test2(24); test2(8); test2(40); test2(48); test2(4); test2(3); test2(5);
   test2(32); test2(2);

   // compare DCT_II, DST_II and slow versions
   test3(2); test3(26); test3(32); test3(18);

   // test DCT_II and DST_II
   test4(2); test5(2);
   test4(202); test5(202);
   test4(1000); test5(1000);

   // test DST and DCT
   test6(2); test7(2);
   test6(274); test7(274);
   test6(1000); test7(1000);

   // compare DCT, DST and slow versions
   test8(2); test8(26); test8(32); test8(18);

   // compare FFT2 with slow version
   test9(1,16); test9(16,1);
   test9(4,3); test9(4,4); test9(4,5); test9(5,3);

   
   // now do the same thing all over again forcing use of old FFT
   FFT_Controller::OnlyOldFFT = true;

   for (i = 1; i <=12; i++)
   {
      Real i1 = i - 1;
      A(i) = .7
           + .2 * cos(6.2831853071795864769 * 4.0 * i1 / 12)
           + .3 * sin(6.2831853071795864769 * 3.0 * i1 / 12);
      B(i) = .9
           + .5 * sin(6.2831853071795864769 * 2.0 * i1 / 12)
           + .4 * cos(6.2831853071795864769 * 1.0 * i1 / 12);
   }
   FFT(A, B, A, B);
   A(1) -= 8.4; A(3) -= 3.0; A(5) -= 1.2; A(9) -= 1.2; A(11) += 3.0;
   B(1) -= 10.8; B(2) -= 2.4; B(4) += 1.8; B(10) -= 1.8; B(12) -= 2.4;
   Clean(A,0.000000001); Clean(B,0.000000001); Print(A); Print(B);


   // test FFT
   test(2048); test(2000); test(27*81); test(2310); test(49*49);
   test(13*13*13); test(43*47);
   test(16*16*3); test(16*16*5); test(16*16*7);
   test(8*8*5);

   // test realFFT
   test1(2); test1(98); test1(22); test1(100);
   test1(2048); test1(2000); test1(35*35*2);

   // compare FFT and slowFFT
   test2(1); test2(13); test2(12); test2(9); test2(16); test2(30); test2(42);
   test2(24); test2(8); test2(40); test2(48); test2(4); test2(3); test2(5);
   test2(32); test2(2);

   // compare DCT_II, DST_II and slow versions
   test3(2); test3(26); test3(32); test3(18);

   // test DCT_II and DST_II
   test4(2); test5(2);
   test4(202); test5(202);
   test4(1000); test5(1000);

   // test DST and DCT
   test6(2); test7(2);
   test6(274); test7(274);
   test6(1000); test7(1000);

   // compare DCT, DST and slow versions
   test8(2); test8(26); test8(32); test8(18);
   
   // compare FFT2 with slow version
   // don't redo these

   FFT_Controller::OnlyOldFFT = false;



}
