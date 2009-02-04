//$$ fft.cpp                         Fast fourier transform

// Copyright (C) 1991,2,3,4,8: R B Davies


#define WANT_MATH
// #define WANT_STREAM

#include "include.h"

#include "newmatap.h"

// #include "newmatio.h"

#ifdef use_namespace
namespace NEWMAT {
#endif

#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,19); ++ExeCount; }
#else
#define REPORT {}
#endif

static void cossin(int n, int d, Real& c, Real& s)
// calculate cos(twopi*n/d) and sin(twopi*n/d)
// minimise roundoff error
{
   REPORT
   long n4 = n * 4; int sector = (int)floor( (Real)n4 / (Real)d + 0.5 );
   n4 -= sector * d;
   if (sector < 0) { REPORT sector = 3 - (3 - sector) % 4; }
   else  { REPORT sector %= 4; }
   Real ratio = 1.5707963267948966192 * (Real)n4 / (Real)d;

   switch (sector)
   {
   case 0: REPORT c =  cos(ratio); s =  sin(ratio); break;
   case 1: REPORT c = -sin(ratio); s =  cos(ratio); break;
   case 2: REPORT c = -cos(ratio); s = -sin(ratio); break;
   case 3: REPORT c =  sin(ratio); s = -cos(ratio); break;
   }
}

static void fftstep(ColumnVector& A, ColumnVector& B, ColumnVector& X,
   ColumnVector& Y, int after, int now, int before)
{
   REPORT
   Tracer trace("FFT(step)");
   // const Real twopi = 6.2831853071795864769;
   const int gamma = after * before;  const int delta = now * after;
   // const Real angle = twopi / delta;  Real temp;
   // Real r_omega = cos(angle);  Real i_omega = -sin(angle);
   Real r_arg = 1.0;  Real i_arg = 0.0;
   Real* x = X.Store();  Real* y = Y.Store();   // pointers to array storage
   const int m = A.Nrows() - gamma;

   for (int j = 0; j < now; j++)
   {
      Real* a = A.Store(); Real* b = B.Store(); // pointers to array storage
      Real* x1 = x; Real* y1 = y; x += after; y += after;
      for (int ia = 0; ia < after; ia++)
      {
         // generate sins & cosines explicitly rather than iteratively
         // for more accuracy; but slower
         cossin(-(j*after+ia), delta, r_arg, i_arg);

         Real* a1 = a++; Real* b1 = b++; Real* x2 = x1++; Real* y2 = y1++;
         if (now==2)
         {
            REPORT int ib = before;
            if (ib) for (;;)
            {
               REPORT
               Real* a2 = m + a1; Real* b2 = m + b1; a1 += after; b1 += after;
               Real r_value = *a2; Real i_value = *b2;
               *x2 = r_value * r_arg - i_value * i_arg + *(a2-gamma);
               *y2 = r_value * i_arg + i_value * r_arg + *(b2-gamma);
               if (!(--ib)) break;
               x2 += delta; y2 += delta;
            }
         }
         else
         {
            REPORT int ib = before;
            if (ib) for (;;)
            {
               REPORT
               Real* a2 = m + a1; Real* b2 = m + b1; a1 += after; b1 += after;
               Real r_value = *a2; Real i_value = *b2;
               int in = now-1; while (in--)
               {
                  // it should be possible to make this faster
                  // hand code for now = 2,3,4,5,8
                  // use symmetry to halve number of operations
                  a2 -= gamma; b2 -= gamma;  Real temp = r_value;
                  r_value = r_value * r_arg - i_value * i_arg + *a2;
                  i_value = temp    * i_arg + i_value * r_arg + *b2;
               }
               *x2 = r_value; *y2 = i_value;
               if (!(--ib)) break;
               x2 += delta; y2 += delta;
            }
         }

         // temp = r_arg;
         // r_arg = r_arg * r_omega - i_arg * i_omega;
         // i_arg = temp  * i_omega + i_arg * r_omega;

      }
   }
}


void FFTI(const ColumnVector& U, const ColumnVector& V,
   ColumnVector& X, ColumnVector& Y)
{
   // Inverse transform
   Tracer trace("FFTI");
   REPORT
   FFT(U,-V,X,Y);
   const Real n = X.Nrows(); X /= n; Y /= (-n);
}

void RealFFT(const ColumnVector& U, ColumnVector& X, ColumnVector& Y)
{
   // Fourier transform of a real series
   Tracer trace("RealFFT");
   REPORT
   const int n = U.Nrows();                     // length of arrays
   const int n2 = n / 2;
   if (n != 2 * n2)
      Throw(ProgramException("Vector length not multiple of 2", U));
   ColumnVector A(n2), B(n2);
   Real* a = A.Store(); Real* b = B.Store(); Real* u = U.Store(); int i = n2;
   while (i--) { *a++ = *u++; *b++ = *u++; }
   FFT(A,B,A,B);
   int n21 = n2 + 1;
   X.resize(n21); Y.resize(n21);
   i = n2 - 1;
   a = A.Store(); b = B.Store();              // first els of A and B
   Real* an = a + i; Real* bn = b + i;        // last els of A and B
   Real* x = X.Store(); Real* y = Y.Store();  // first els of X and Y
   Real* xn = x + n2; Real* yn = y + n2;      // last els of X and Y

   *x++ = *a + *b; *y++ = 0.0;                // first complex element
   *xn-- = *a++ - *b++; *yn-- = 0.0;          // last complex element

   int j = -1; i = n2/2;
   while (i--)
   {
      Real c,s; cossin(j--,n,c,s);
      Real am = *a - *an; Real ap = *a++ + *an--;
      Real bm = *b - *bn; Real bp = *b++ + *bn--;
      Real samcbp = s * am + c * bp; Real sbpcam = s * bp - c * am;
      *x++  =  0.5 * ( ap + samcbp); *y++  =  0.5 * ( bm + sbpcam);
      *xn-- =  0.5 * ( ap - samcbp); *yn-- =  0.5 * (-bm + sbpcam);
   }
}

void RealFFTI(const ColumnVector& A, const ColumnVector& B, ColumnVector& U)
{
   // inverse of a Fourier transform of a real series
   Tracer trace("RealFFTI");
   REPORT
   const int n21 = A.Nrows();                     // length of arrays
   if (n21 != B.Nrows() || n21 == 0)
      Throw(ProgramException("Vector lengths unequal or zero", A, B));
   const int n2 = n21 - 1;  const int n = 2 * n2;  int i = n2 - 1;

   ColumnVector X(n2), Y(n2);
   Real* a = A.Store(); Real* b = B.Store();  // first els of A and B
   Real* an = a + n2;   Real* bn = b + n2;    // last els of A and B
   Real* x = X.Store(); Real* y = Y.Store();  // first els of X and Y
   Real* xn = x + i;    Real* yn = y + i;     // last els of X and Y

   Real hn = 0.5 / n2;
   *x++  = hn * (*a + *an);  *y++  = - hn * (*a - *an);
   a++; an--; b++; bn--;
   int j = -1;  i = n2/2;
   while (i--)
   {
      Real c,s; cossin(j--,n,c,s);
      Real am = *a - *an; Real ap = *a++ + *an--;
      Real bm = *b - *bn; Real bp = *b++ + *bn--;
      Real samcbp = s * am - c * bp; Real sbpcam = s * bp + c * am;
      *x++  =  hn * ( ap + samcbp); *y++  =  - hn * ( bm + sbpcam);
      *xn-- =  hn * ( ap - samcbp); *yn-- =  - hn * (-bm + sbpcam);
   }
   FFT(X,Y,X,Y);             // have done inverting elsewhere
   U.resize(n); i = n2;
   x = X.Store(); y = Y.Store(); Real* u = U.Store();
   while (i--) { *u++ = *x++; *u++ = - *y++; }
}

void FFT(const ColumnVector& U, const ColumnVector& V,
   ColumnVector& X, ColumnVector& Y)
{
   // from Carl de Boor (1980), Siam J Sci Stat Comput, 1 173-8
   // but first try Sande and Gentleman
   Tracer trace("FFT");
   REPORT
   const int n = U.Nrows();                     // length of arrays
   if (n != V.Nrows() || n == 0)
      Throw(ProgramException("Vector lengths unequal or zero", U, V));
   if (n == 1) { REPORT X = U; Y = V; return; }

   // see if we can use the newfft routine
   if (!FFT_Controller::OnlyOldFFT && FFT_Controller::CanFactor(n))
   {
      REPORT
      X = U; Y = V;
      if ( FFT_Controller::ar_1d_ft(n,X.Store(),Y.Store()) ) return;
   }

   ColumnVector B = V;
   ColumnVector A = U;
   X.resize(n); Y.resize(n);
   const int nextmx = 8;
   int prime[8] = { 2,3,5,7,11,13,17,19 };
   int after = 1; int before = n; int next = 0; bool inzee = true;
   int now = 0; int b1;             // initialised to keep gnu happy

   do
   {
      for (;;)
      {
         if (next < nextmx) { REPORT now = prime[next]; }
         b1 = before / now;  if (b1 * now == before) { REPORT break; }
         next++; now += 2;
      }
      before = b1;

      if (inzee) { REPORT fftstep(A, B, X, Y, after, now, before); }
      else { REPORT fftstep(X, Y, A, B, after, now, before); }

      inzee = !inzee; after *= now;
   }
   while (before != 1);

   if (inzee) { REPORT A.release(); X = A; B.release(); Y = B; }
}

// Trigonometric transforms
// see Charles Van Loan (1992) "Computational frameworks for the fast
// Fourier transform" published by SIAM; section 4.4.

void DCT_II(const ColumnVector& U, ColumnVector& V)
{
   // Discrete cosine transform, type II, of a real series
   Tracer trace("DCT_II");
   REPORT
   const int n = U.Nrows();                     // length of arrays
   const int n2 = n / 2; const int n4 = n * 4;
   if (n != 2 * n2)
      Throw(ProgramException("Vector length not multiple of 2", U));
   ColumnVector A(n);
   Real* a = A.Store(); Real* b = a + n; Real* u = U.Store();
   int i = n2;
   while (i--) { *a++ = *u++; *(--b) = *u++; }
   ColumnVector X, Y;
   RealFFT(A, X, Y); A.cleanup();
   V.resize(n);
   Real* x = X.Store(); Real* y = Y.Store();
   Real* v = V.Store(); Real* w = v + n;
   *v = *x;
   int k = 0; i = n2;
   while (i--)
   {
      Real c, s; cossin(++k, n4, c, s);
      Real xi = *(++x); Real yi = *(++y);
      *(++v) = xi * c + yi * s; *(--w) = xi * s - yi * c;
   }
}

void DCT_II_inverse(const ColumnVector& V, ColumnVector& U)
{
   // Inverse of discrete cosine transform, type II
   Tracer trace("DCT_II_inverse");
   REPORT
   const int n = V.Nrows();                     // length of array
   const int n2 = n / 2; const int n4 = n * 4; const int n21 = n2 + 1;
   if (n != 2 * n2)
      Throw(ProgramException("Vector length not multiple of 2", V));
   ColumnVector X(n21), Y(n21);
   Real* x = X.Store(); Real* y = Y.Store();
   Real* v = V.Store(); Real* w = v + n;
   *x = *v; *y = 0.0;
   int i = n2; int k = 0;
   while (i--)
   {
      Real c, s; cossin(++k, n4, c, s);
      Real vi = *(++v); Real wi = *(--w);
      *(++x) = vi * c + wi * s; *(++y) = vi * s - wi * c;
   }
   ColumnVector A; RealFFTI(X, Y, A);
   X.cleanup(); Y.cleanup(); U.resize(n);
   Real* a = A.Store(); Real* b = a + n; Real* u = U.Store();
   i = n2;
   while (i--) { *u++ = *a++; *u++ = *(--b); }
}

void DST_II(const ColumnVector& U, ColumnVector& V)
{
   // Discrete sine transform, type II, of a real series
   Tracer trace("DST_II");
   REPORT
   const int n = U.Nrows();                     // length of arrays
   const int n2 = n / 2; const int n4 = n * 4;
   if (n != 2 * n2)
      Throw(ProgramException("Vector length not multiple of 2", U));
   ColumnVector A(n);
   Real* a = A.Store(); Real* b = a + n; Real* u = U.Store();
   int i = n2;
   while (i--) { *a++ = *u++; *(--b) = -(*u++); }
   ColumnVector X, Y;
   RealFFT(A, X, Y); A.cleanup();
   V.resize(n);
   Real* x = X.Store(); Real* y = Y.Store();
   Real* v = V.Store(); Real* w = v + n;
   *(--w) = *x;
   int k = 0; i = n2;
   while (i--)
   {
      Real c, s; cossin(++k, n4, c, s);
      Real xi = *(++x); Real yi = *(++y);
      *v++ = xi * s - yi * c; *(--w) = xi * c + yi * s;
   }
}

void DST_II_inverse(const ColumnVector& V, ColumnVector& U)
{
   // Inverse of discrete sine transform, type II
   Tracer trace("DST_II_inverse");
   REPORT
   const int n = V.Nrows();                     // length of array
   const int n2 = n / 2; const int n4 = n * 4; const int n21 = n2 + 1;
   if (n != 2 * n2)
      Throw(ProgramException("Vector length not multiple of 2", V));
   ColumnVector X(n21), Y(n21);
   Real* x = X.Store(); Real* y = Y.Store();
   Real* v = V.Store(); Real* w = v + n;
   *x = *(--w); *y = 0.0;
   int i = n2; int k = 0;
   while (i--)
   {
      Real c, s; cossin(++k, n4, c, s);
      Real vi = *v++; Real wi = *(--w);
      *(++x) = vi * s + wi * c; *(++y) = - vi * c + wi * s;
   }
   ColumnVector A; RealFFTI(X, Y, A);
   X.cleanup(); Y.cleanup(); U.resize(n);
   Real* a = A.Store(); Real* b = a + n; Real* u = U.Store();
   i = n2;
   while (i--) { *u++ = *a++; *u++ = -(*(--b)); }
}

void DCT_inverse(const ColumnVector& V, ColumnVector& U)
{
   // Inverse of discrete cosine transform, type I
   Tracer trace("DCT_inverse");
   REPORT
   const int n = V.Nrows()-1;                     // length of transform
   const int n2 = n / 2; const int n21 = n2 + 1;
   if (n != 2 * n2)
      Throw(ProgramException("Vector length not multiple of 2", V));
   ColumnVector X(n21), Y(n21);
   Real* x = X.Store(); Real* y = Y.Store(); Real* v = V.Store();
   Real vi = *v++; *x++ = vi; *y++ = 0.0;
   Real sum1 = vi / 2.0; Real sum2 = sum1; vi = *v++;
   int i = n2-1;
   while (i--)
   {
      Real vi2 = *v++; sum1 += vi2 + vi; sum2 += vi2 - vi;
      *x++ = vi2; vi2 = *v++; *y++ = vi - vi2; vi = vi2;
   }
   sum1 += vi; sum2 -= vi;
   vi = *v; *x = vi; *y = 0.0; vi /= 2.0; sum1 += vi; sum2 += vi;
   ColumnVector A; RealFFTI(X, Y, A);
   X.cleanup(); Y.cleanup(); U.resize(n+1);
   Real* a = A.Store(); Real* b = a + n; Real* u = U.Store(); v = u + n;
   i = n2; int k = 0; *u++ = sum1 / n2; *v-- = sum2 / n2;
   while (i--)
   {
      Real s = sin(1.5707963267948966192 * (++k) / n2);
      Real ai = *(++a); Real bi = *(--b);
      Real bz = (ai - bi) / 4 / s; Real az = (ai + bi) / 2;
      *u++ = az - bz; *v-- = az + bz;
   }
}

void DCT(const ColumnVector& U, ColumnVector& V)
{
   // Discrete cosine transform, type I
   Tracer trace("DCT");
   REPORT
   DCT_inverse(U, V);
   V *= (V.Nrows()-1)/2;
}

void DST_inverse(const ColumnVector& V, ColumnVector& U)
{
   // Inverse of discrete sine transform, type I
   Tracer trace("DST_inverse");
   REPORT
   const int n = V.Nrows()-1;                     // length of transform
   const int n2 = n / 2; const int n21 = n2 + 1;
   if (n != 2 * n2)
      Throw(ProgramException("Vector length not multiple of 2", V));
   ColumnVector X(n21), Y(n21);
   Real* x = X.Store(); Real* y = Y.Store(); Real* v = V.Store();
   Real vi = *(++v); *x++ = 2 * vi; *y++ = 0.0;
   int i = n2-1;
   while (i--) { *y++ = *(++v); Real vi2 = *(++v); *x++ = vi2 - vi; vi = vi2; }
   *x = -2 * vi; *y = 0.0;
   ColumnVector A; RealFFTI(X, Y, A);
   X.cleanup(); Y.cleanup(); U.resize(n+1);
   Real* a = A.Store(); Real* b = a + n; Real* u = U.Store(); v = u + n;
   i = n2; int k = 0; *u++ = 0.0; *v-- = 0.0;
   while (i--)
   {
      Real s = sin(1.5707963267948966192 * (++k) / n2);
      Real ai = *(++a); Real bi = *(--b);
      Real az = (ai + bi) / 4 / s; Real bz = (ai - bi) / 2;
      *u++ = az - bz; *v-- = az + bz;
   }
}

void DST(const ColumnVector& U, ColumnVector& V)
{
   // Discrete sine transform, type I
   Tracer trace("DST");
   REPORT
   DST_inverse(U, V);
   V *= (V.Nrows()-1)/2;
}

// Two dimensional FFT
void FFT2(const Matrix& U, const Matrix& V, Matrix& X, Matrix& Y)
{
   Tracer trace("FFT2");
   REPORT
   int m = U.Nrows(); int n = U.Ncols();
   if (m != V.Nrows() || n != V.Ncols() || m == 0 || n == 0)
      Throw(ProgramException("Matrix dimensions unequal or zero", U, V));
   X = U; Y = V;
   int i; ColumnVector CVR; ColumnVector CVI;
   for (i = 1; i <= m; ++i)
   {
      FFT(X.Row(i).t(), Y.Row(i).t(), CVR, CVI);
      X.Row(i) = CVR.t(); Y.Row(i) = CVI.t();
   }
   for (i = 1; i <= n; ++i)
   {
      FFT(X.Column(i), Y.Column(i), CVR, CVI);
      X.Column(i) = CVR; Y.Column(i) = CVI;
   }
}

void FFT2I(const Matrix& U, const Matrix& V, Matrix& X, Matrix& Y)
{
   // Inverse transform
   Tracer trace("FFT2I");
   REPORT
   FFT2(U,-V,X,Y);
   const Real n = X.Nrows() * X.Ncols(); X /= n; Y /= (-n);
}


#ifdef use_namespace
}
#endif


