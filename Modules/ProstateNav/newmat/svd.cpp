//$$svd.cpp                           singular value decomposition

// Copyright (C) 1991,2,3,4,5: R B Davies
// Updated 17 July, 1995

#define WANT_MATH

#include "include.h"
#include "newmatap.h"
#include "newmatrm.h"
#include "precisio.h"

#ifdef use_namespace
namespace NEWMAT {
#endif

#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,15); ++ExeCount; }
#else
#define REPORT {}
#endif




void SVD(const Matrix& A, DiagonalMatrix& Q, Matrix& U, Matrix& V,
   bool withU, bool withV)
// from Wilkinson and Reinsch: "Handbook of Automatic Computation"
{
   REPORT
   Tracer trace("SVD");
   Real eps = FloatingPointPrecision::Epsilon();
   Real tol = FloatingPointPrecision::Minimum()/eps;

   int m = A.Nrows(); int n = A.Ncols();
   if (m<n)
      Throw(ProgramException("Want no. Rows >= no. Cols", A));
   if (withV && &U == &V)
      Throw(ProgramException("Need different matrices for U and V", U, V));
   U = A; Real g = 0.0; Real f,h; Real x = 0.0; int i;
   RowVector E(n); RectMatrixRow EI(E,0); Q.ReSize(n);
   RectMatrixCol UCI(U,0); RectMatrixRow URI(U,0,1,n-1);

   if (n) for (i=0;;)
   {
      EI.First() = g; Real ei = g; EI.Right(); Real s = UCI.SumSquare();
      if (s<tol) { REPORT Q.element(i) = 0.0; }
      else
      {
         REPORT
         f = UCI.First(); g = -sign(sqrt(s), f); h = f*g-s; UCI.First() = f-g;
         Q.element(i) = g; RectMatrixCol UCJ = UCI; int j=n-i;
         while (--j) { UCJ.Right(); UCJ.AddScaled(UCI, (UCI*UCJ)/h); }
      }

      s = URI.SumSquare();
      if (s<tol) { REPORT g = 0.0; }
      else
      {
         REPORT
         f = URI.First(); g = -sign(sqrt(s), f); URI.First() = f-g;
         EI.Divide(URI,f*g-s); RectMatrixRow URJ = URI; int j=m-i;
         while (--j) { URJ.Down(); URJ.AddScaled(EI, URI*URJ); }
      }

      Real y = fabs(Q.element(i)) + fabs(ei); if (x<y) { REPORT x = y; }
      if (++i == n) { REPORT break; }
      UCI.DownDiag(); URI.DownDiag();
   }

   if (withV)
   {
      REPORT
      V.ReSize(n,n); V = 0.0; RectMatrixCol VCI(V,n-1,n-1,1);
      if (n) { VCI.First() = 1.0; g=E.element(n-1); if (n!=1) URI.UpDiag(); }
      for (i=n-2; i>=0; i--)
      {
         VCI.Left();
         if (g!=0.0)
         {
            VCI.Divide(URI, URI.First()*g); int j = n-i;
            RectMatrixCol VCJ = VCI;
            while (--j) { VCJ.Right(); VCJ.AddScaled( VCI, (URI*VCJ) ); }
         }
         VCI.Zero(); VCI.Up(); VCI.First() = 1.0; g=E.element(i);
         if (i==0) break;
         URI.UpDiag();
      }
   }

   if (withU)
   {
      REPORT
      for (i=n-1; i>=0; i--)
      {
         g = Q.element(i); URI.Reset(U,i,i+1,n-i-1); URI.Zero();
         if (g!=0.0)
         {
            h=UCI.First()*g; int j=n-i; RectMatrixCol UCJ = UCI;
            while (--j)
            {
               UCJ.Right(); UCI.Down(); UCJ.Down(); Real s = UCI*UCJ;
               UCI.Up(); UCJ.Up(); UCJ.AddScaled(UCI,s/h);
            }
            UCI.Divide(g);
         }
         else UCI.Zero();
         UCI.First() += 1.0;
         if (i==0) break;
         UCI.UpDiag();
      }
   }

   eps *= x;
   for (int k=n-1; k>=0; k--)
   {
      Real z = -FloatingPointPrecision::Maximum(); // to keep Gnu happy
      Real y; int limit = 50; int l = 0;
      while (limit--)
      {
         Real c, s; int i; int l1=k; bool tfc=false;
         for (l=k; l>=0; l--)
         {
//          if (fabs(E.element(l))<=eps) goto test_f_convergence;
            if (fabs(E.element(l))<=eps) { REPORT tfc=true; break; }
            if (fabs(Q.element(l-1))<=eps) { REPORT l1=l; break; }
            REPORT
         }
         if (!tfc)
         {
            REPORT
            l=l1; l1=l-1; s = -1.0; c = 0.0;
            for (i=l; i<=k; i++)
            {
               f = - s * E.element(i); E.element(i) *= c;
//             if (fabs(f)<=eps) goto test_f_convergence;
               if (fabs(f)<=eps) { REPORT break; }
               g = Q.element(i); h = pythag(g,f,c,s); Q.element(i) = h;
               if (withU)
               {
                  REPORT
                  RectMatrixCol UCI(U,i); RectMatrixCol UCJ(U,l1);
                  ComplexScale(UCJ, UCI, c, s);
               }
            }
         }
//       test_f_convergence: z = Q.element(k); if (l==k) goto convergence;
         z = Q.element(k);  if (l==k) { REPORT break; }

         x = Q.element(l); y = Q.element(k-1);
         g = E.element(k-1); h = E.element(k);
         f = ((y-z)*(y+z) + (g-h)*(g+h)) / (2*h*y);
         if (f>1)         { REPORT g = f * sqrt(1 + square(1/f)); }
         else if (f<-1)   { REPORT g = -f * sqrt(1 + square(1/f)); }
         else             { REPORT g = sqrt(f*f + 1); }
            { REPORT f = ((x-z)*(x+z) + h*(y / ((f<0.0) ? f-g : f+g)-h)) / x; }

         c = 1.0; s = 1.0;
         for (i=l+1; i<=k; i++)
         {
            g = E.element(i); y = Q.element(i); h = s*g; g *= c;
            z = pythag(f,h,c,s); E.element(i-1) = z;
            f = x*c + g*s; g = -x*s + g*c; h = y*s; y *= c;
            if (withV)
            {
               REPORT
               RectMatrixCol VCI(V,i); RectMatrixCol VCJ(V,i-1);
               ComplexScale(VCI, VCJ, c, s);
            }
            z = pythag(f,h,c,s); Q.element(i-1) = z;
            f = c*g + s*y; x = -s*g + c*y;
            if (withU)
            {
               REPORT
               RectMatrixCol UCI(U,i); RectMatrixCol UCJ(U,i-1);
               ComplexScale(UCI, UCJ, c, s);
            }
         }
         E.element(l) = 0.0; E.element(k) = f; Q.element(k) = x;
      }
      if (l!=k) { Throw(ConvergenceException(A)); }
// convergence:
      if (z < 0.0)
      {
         REPORT
         Q.element(k) = -z;
         if (withV) { RectMatrixCol VCI(V,k); VCI.Negate(); }
      }
   }
   if (withU & withV) SortSV(Q, U, V);
   else if (withU) SortSV(Q, U);
   else if (withV) SortSV(Q, V);
   else sort_descending(Q);
}

void SVD(const Matrix& A, DiagonalMatrix& D)
{ REPORT Matrix U; SVD(A, D, U, U, false, false); }



#ifdef use_namespace
}
#endif

