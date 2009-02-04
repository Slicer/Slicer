//$$evalue.cpp                           eigen-value decomposition

// Copyright (C) 1991,2,3,4: R B Davies

#define WANT_MATH

#include "include.h"
#include "newmatap.h"
#include "newmatrm.h"
#include "precisio.h"

#ifdef use_namespace
namespace NEWMAT {
#endif

#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,17); ++ExeCount; }
#else
#define REPORT {}
#endif



static void tred2(const SymmetricMatrix& A, DiagonalMatrix& D,
   DiagonalMatrix& E, Matrix& Z)
{
   Tracer et("Evalue(tred2)");
   REPORT
   Real tol =
      FloatingPointPrecision::Minimum()/FloatingPointPrecision::Epsilon();
   int n = A.Nrows(); Z.resize(n,n); Z.Inject(A);
   D.resize(n); E.resize(n);
   Real* z = Z.Store(); int i;

   for (i=n-1; i > 0; i--)                   // i=0 is excluded
   {
      Real f = Z.element(i,i-1); Real g = 0.0;
      int k = i-1; Real* zik = z + i*n;
      while (k--) g += square(*zik++);
      Real h = g + square(f);
      if (g <= tol) { REPORT E.element(i) = f; h = 0.0; }
      else
      {
         REPORT
         g = sign(-sqrt(h), f); E.element(i) = g; h -= f*g;
         Z.element(i,i-1) = f-g; f = 0.0;
         Real* zji = z + i; Real* zij = z + i*n; Real* ej = E.Store();
         int j;
         for (j=0; j<i; j++)
         {
            *zji = (*zij++)/h; g = 0.0;
            Real* zjk = z + j*n; zik = z + i*n;
            k = j; while (k--) g += *zjk++ * (*zik++);
            k = i-j;
            if (k) for(;;)
               { g += *zjk * (*zik++); if (!(--k)) break; zjk += n; }
            *ej++ = g/h; f += g * (*zji); zji += n;
         }
         Real hh = f / (h + h); zij = z + i*n; ej = E.Store();
         for (j=0; j<i; j++)
         {
            f = *zij++; g = *ej - hh * f; *ej++ = g;
            Real* zjk = z + j*n; Real* zik = z + i*n;
            Real* ek = E.Store(); k = j+1;
            while (k--)  *zjk++ -= ( f*(*ek++) + g*(*zik++) ); 
         }
      }
      D.element(i) = h;
   }

   D.element(0) = 0.0; E.element(0) = 0.0;
   for (i=0; i<n; i++)
   {
      if (D.element(i) != 0.0)
      {
         REPORT
         for (int j=0; j<i; j++)
         {
            Real g = 0.0;
            Real* zik = z + i*n; Real* zkj = z + j;
            int k = i;
            if (k) for (;;)
               { g += *zik++ * (*zkj); if (!(--k)) break; zkj += n; }
            Real* zki = z + i; zkj = z + j;
            k = i;
            if (k) for (;;)
               { *zkj -= g * (*zki); if (!(--k)) break; zkj += n; zki += n; }
         }
      }
      Real* zij = z + i*n; Real* zji = z + i;
      int j = i;
      if (j) for (;;)
         { *zij++ = 0.0; *zji = 0.0; if (!(--j)) break; zji += n; }
      D.element(i) = *zij; *zij = 1.0;
   }
}

static void tql2(DiagonalMatrix& D, DiagonalMatrix& E, Matrix& Z)
{
   Tracer et("Evalue(tql2)");
   REPORT
   Real eps = FloatingPointPrecision::Epsilon();
   int n = D.Nrows(); Real* z = Z.Store(); int l;
   for (l=1; l<n; l++) E.element(l-1) = E.element(l);
   Real b = 0.0; Real f = 0.0; E.element(n-1) = 0.0;
   for (l=0; l<n; l++)
   {
      int i,j;
      Real& dl = D.element(l); Real& el = E.element(l);
      Real h = eps * ( fabs(dl) + fabs(el) );
      if (b < h) { REPORT b = h; }
      int m;
      for (m=l; m<n; m++) if (fabs(E.element(m)) <= b) break;
      bool test = false;
      for (j=0; j<30; j++)
      {
         if (m==l) { REPORT test = true; break; }
         Real& dl1 = D.element(l+1);
         Real g = dl; Real p = (dl1-g) / (2.0*el); Real r = sqrt(p*p + 1.0);
         dl = el / (p < 0.0 ? p-r : p+r); Real h = g - dl; f += h;
         Real* dlx = &dl1; i = n-l-1; while (i--) *dlx++ -= h;

         p = D.element(m); Real c = 1.0; Real s = 0.0;
         for (i=m-1; i>=l; i--)
         {
            Real ei = E.element(i); Real di = D.element(i);
            Real& ei1 = E.element(i+1);
            g = c * ei; h = c * p;
            if ( fabs(p) >= fabs(ei))
            {
               REPORT
               c = ei / p; r = sqrt(c*c + 1.0);
               ei1 = s*p*r; s = c/r; c = 1.0/r;
            }
            else
            {
               REPORT
               c = p / ei; r = sqrt(c*c + 1.0);
               ei1 = s * ei * r; s = 1.0/r; c /= r;
            }
            p = c * di - s*g; D.element(i+1) = h + s * (c*g + s*di);

            Real* zki = z + i; Real* zki1 = zki + 1; int k = n;
            if (k) for (;;)
            {
               REPORT
               h = *zki1; *zki1 = s*(*zki) + c*h; *zki = c*(*zki) - s*h;
               if (!(--k)) break;
               zki += n; zki1 += n;
            }
         }
         el = s*p; dl = c*p;
         if (fabs(el) <= b) { REPORT; test = true; break; }
      }
      if (!test) Throw ( ConvergenceException(D) );
      dl += f;
   }
/*
   for (int i=0; i<n; i++)
   {
      int k = i; Real p = D.element(i);
      for (int j=i+1; j<n; j++)
         { if (D.element(j) < p) { k = j; p = D.element(j); } }
      if (k != i)
      {
         D.element(k) = D.element(i); D.element(i) = p; int j = n;
         Real* zji = z + i; Real* zjk = z + k;
         if (j) for(;;)
         {
            p = *zji; *zji = *zjk; *zjk = p;
            if (!(--j)) break;
            zji += n; zjk += n;
         }
      }
   }
*/
}

static void tred3(const SymmetricMatrix& X, DiagonalMatrix& D,
   DiagonalMatrix& E, SymmetricMatrix& A)
{
   Tracer et("Evalue(tred3)");
   REPORT
   Real tol =
      FloatingPointPrecision::Minimum()/FloatingPointPrecision::Epsilon();
   int n = X.Nrows(); A = X; D.resize(n); E.resize(n);
   Real* ei = E.Store() + n;
   for (int i = n-1; i >= 0; i--)
   {
      Real h = 0.0; Real f = - FloatingPointPrecision::Maximum();
      Real* d = D.Store(); Real* a = A.Store() + (i*(i+1))/2; int k = i;
      while (k--) { f = *a++; *d++ = f; h += square(f); }
      if (h <= tol) { REPORT *(--ei) = 0.0; h = 0.0; }
      else
      {
         REPORT
         Real g = sign(-sqrt(h), f); *(--ei) = g; h -= f*g;
         f -= g; *(d-1) = f; *(a-1) = f; f = 0.0;
         Real* dj = D.Store(); Real* ej = E.Store(); int j;
         for (j = 0; j < i; j++)
         {
            Real* dk = D.Store(); Real* ak = A.Store()+(j*(j+1))/2;
            Real g = 0.0; k = j;
            while (k--)  g += *ak++ * *dk++;
            k = i-j; int l = j; 
            if (k) for (;;) { g += *ak * *dk++; if (!(--k)) break; ak += ++l; }
            g /= h; *ej++ = g; f += g * *dj++;
         }  
         Real hh = f / (2 * h); Real* ak = A.Store();
         dj = D.Store(); ej = E.Store();
         for (j = 0; j < i; j++)
         {
            f = *dj++; g = *ej - hh * f; *ej++ = g;
            Real* dk = D.Store(); Real* ek = E.Store(); k = j+1;
            while (k--) { *ak++ -= (f * *ek++ + g * *dk++); }
         }
      }
      *d = *a; *a = h;
   }
}

static void tql1(DiagonalMatrix& D, DiagonalMatrix& E)
{
   Tracer et("Evalue(tql1)");
   REPORT
   Real eps = FloatingPointPrecision::Epsilon();
   int n = D.Nrows(); int l;
   for (l=1; l<n; l++) E.element(l-1) = E.element(l);
   Real b = 0.0; Real f = 0.0; E.element(n-1) = 0.0;
   for (l=0; l<n; l++)
   {
      int i,j;
      Real& dl = D.element(l); Real& el = E.element(l);
      Real h = eps * ( fabs(dl) + fabs(el) );
      if (b < h) b = h;
      int m;
      for (m=l; m<n; m++) if (fabs(E.element(m)) <= b) break;
      bool test = false;
      for (j=0; j<30; j++)
      {
         if (m==l) { REPORT test = true; break; }
         Real& dl1 = D.element(l+1);
         Real g = dl; Real p = (dl1-g) / (2.0*el); Real r = sqrt(p*p + 1.0);
         dl = el / (p < 0.0 ? p-r : p+r); Real h = g - dl; f += h;
         Real* dlx = &dl1; i = n-l-1; while (i--) *dlx++ -= h;

         p = D.element(m); Real c = 1.0; Real s = 0.0;
         for (i=m-1; i>=l; i--)
         {
            Real ei = E.element(i); Real di = D.element(i);
            Real& ei1 = E.element(i+1);
            g = c * ei; h = c * p;
            if ( fabs(p) >= fabs(ei))
            {
               REPORT
               c = ei / p; r = sqrt(c*c + 1.0);
               ei1 = s*p*r; s = c/r; c = 1.0/r;
            }
            else
            {
               REPORT
               c = p / ei; r = sqrt(c*c + 1.0);
               ei1 = s * ei * r; s = 1.0/r; c /= r;
            }
            p = c * di - s*g; D.element(i+1) = h + s * (c*g + s*di);
         }
         el = s*p; dl = c*p;
         if (fabs(el) <= b) { REPORT test = true; break; }
      }
      if (!test) Throw ( ConvergenceException(D) );
      Real p = dl + f;
      test = false;
      for (i=l; i>0; i--)
      {
         if (p < D.element(i-1)) { REPORT D.element(i) = D.element(i-1); }
         else { REPORT test = true; break; }
      }
      if (!test) i=0;
      D.element(i) = p;
   }
}

void eigenvalues(const SymmetricMatrix& A, DiagonalMatrix& D, Matrix& Z)
{ REPORT DiagonalMatrix E; tred2(A, D, E, Z); tql2(D, E, Z); SortSV(D,Z,true); }

void eigenvalues(const SymmetricMatrix& X, DiagonalMatrix& D)
{ REPORT DiagonalMatrix E; SymmetricMatrix A; tred3(X,D,E,A); tql1(D,E); }

void eigenvalues(const SymmetricMatrix& X, DiagonalMatrix& D,
   SymmetricMatrix& A)
{ REPORT DiagonalMatrix E; tred3(X,D,E,A); tql1(D,E); }


#ifdef use_namespace
}
#endif

