//$$ newmatnl.cpp         Non-linear optimisation

// Copyright (C) 1993,4,5,6: R B Davies


#define WANT_MATH
#define WANT_STREAM

#include "newmatap.h"
#include "newmatnl.h"

#ifdef use_namespace
namespace NEWMAT {
#endif



void FindMaximum2::Fit(ColumnVector& Theta, int n_it)
{
   Tracer tr("FindMaximum2::Fit");
   enum State {Start, Restart, Continue, Interpolate, Extrapolate,
      Fail, Convergence};
   State TheState = Start;
   Real z,w,x,x2,g,l1,l2,l3,d1,d2=0,d3;
   ColumnVector Theta1, Theta2, Theta3;
   int np = Theta.Nrows();
   ColumnVector H1(np), H3, HP(np), K, K1(np);
   bool oorg, conv;
   int counter = 0;
   Theta1 = Theta; HP = 0.0; g = 0.0;

   // This is really a set of gotos and labels, but they do not work
   // correctly in AT&T C++ and Sun 4.01 C++.

   for(;;)
   {
      switch (TheState)
      {
      case Start:
         tr.ReName("FindMaximum2::Fit/Start");
         Value(Theta1, true, l1, oorg);
         if (oorg) Throw(ProgramException("invalid starting value\n"));

      case Restart:
         tr.ReName("FindMaximum2::Fit/ReStart");
         conv = NextPoint(H1, d1);
         if (conv) { TheState = Convergence; break; }
         if (counter++ > n_it) { TheState = Fail; break; }

         z = 1.0 / sqrt(d1);
         H3 = H1 * z; K = (H3 - HP) * g; HP = H3;
         g = 0.0;                     // de-activate to use curved projection
         if ( g == 0.0 ) K1 = 0.0; else K1 = K * 0.2 + K1 * 0.6;
         // (K - K1) * alpha + K1 * (1 - alpha)
         //     = K * alpha + K1 * (1 - 2 * alpha)
         K = K1 * d1; g = z;

      case Continue:
         tr.ReName("FindMaximum2::Fit/Continue");
         Theta2 = Theta1 + H1 + K;
         Value(Theta2, false, l2, oorg);
         if (counter++ > n_it) { TheState = Fail; break; }
         if (oorg)
         {
            H1 *= 0.5; K *= 0.25; d1 *= 0.5; g *= 2.0;
            TheState =  Continue; break;
         }
         d2 = LastDerivative(H1 + K * 2.0);

      case Interpolate:
         tr.ReName("FindMaximum2::Fit/Interpolate");
         z = d1 + d2 - 3.0 * (l2 - l1);
         w = z * z - d1 * d2;
         if (w < 0.0) { TheState = Extrapolate; break; }
         w = z + sqrt(w);
         if (1.5 * w + d1 < 0.0)
            { TheState = Extrapolate; break; }
         if (d2 > 0.0 && l2 > l1 && w > 0.0)
            { TheState = Extrapolate; break; }
         x = d1 / (w + d1); x2 = x * x; g /= x;
         Theta3 = Theta1 + H1 * x + K * x2;
         Value(Theta3, true, l3, oorg);
         if (counter++ > n_it) { TheState = Fail; break; }
         if (oorg)
         {
            if (x <= 1.0)
               { x *= 0.5; x2 = x*x; g *= 2.0; d1 *= x; H1 *= x; K *= x2; }
            else
            {
               x = 0.5 * (x-1.0); x2 = x*x; Theta1 = Theta2;
               H1 = (H1 + K * 2.0) * x;
               K *= x2; g = 0.0; d1 = x * d2; l1 = l2;
            }
            TheState = Continue; break;
         }

         if (l3 >= l1 && l3 >= l2)
            { Theta1 = Theta3; l1 = l3; TheState =  Restart; break; }

         d3 = LastDerivative(H1 + K * 2.0);
         if (l1 > l2)
            { H1 *= x; K *= x2; Theta2 = Theta3; d1 *= x; d2 = d3*x; }
         else
         {
            Theta1 = Theta2; Theta2 = Theta3;
            x -= 1.0; x2 = x*x; g = 0.0; H1 = (H1 + K * 2.0) * x;
            K *= x2; l1 = l2; l2 = l3; d1 = x*d2; d2 = x*d3;
            if (d1 <= 0.0) { TheState = Start; break; }
         }
         TheState =  Interpolate; break;

      case Extrapolate:
         tr.ReName("FindMaximum2::Fit/Extrapolate");
         Theta1 = Theta2; g = 0.0; K *= 4.0; H1 = (H1 * 2.0 + K);
         d1 = 2.0 * d2; l1 = l2;
         TheState = Continue; break;

      case Fail:
         Throw(ConvergenceException(Theta));

      case Convergence:
         Theta = Theta1; return;
      }
   }
}



void NonLinearLeastSquares::Value
   (const ColumnVector& Parameters, bool, Real& v, bool& oorg)
{
   Tracer tr("NonLinearLeastSquares::Value");
   Y.resize(n_obs); X.resize(n_obs,n_param);
   // put the fitted values in Y, the derivatives in X.
   Pred.Set(Parameters);
   if (!Pred.IsValid()) { oorg=true; return; }
   for (int i=1; i<=n_obs; i++)
   {
      Y(i) = Pred(i);
      X.Row(i) = Pred.Derivatives();
   }
   if (!Pred.IsValid()) { oorg=true; return; }  // check afterwards as well
   Y = *DataPointer - Y; Real ssq = Y.SumSquare();
   errorvar =  ssq / (n_obs - n_param);
   cout << endl;
   cout << setw(15) << setprecision(10) << " " << errorvar;
   Derivs = Y.t() * X;          // get the derivative and stash it
   oorg = false; v = -0.5 * ssq;
}

bool NonLinearLeastSquares::NextPoint(ColumnVector& Adj, Real& test)
{
   Tracer tr("NonLinearLeastSquares::NextPoint");
   QRZ(X, U); QRZ(X, Y, M);     // do the QR decomposition
   test = M.SumSquare();
   cout << " " << setw(15) << setprecision(10)
      << test << " " << Y.SumSquare() / (n_obs - n_param);
   Adj = U.i() * M;
   if (test < errorvar * criterion) return true;
   else return false;
}

Real NonLinearLeastSquares::LastDerivative(const ColumnVector& H)
{ return (Derivs * H).AsScalar(); }

void NonLinearLeastSquares::Fit(const ColumnVector& Data,
   ColumnVector& Parameters)
{
   Tracer tr("NonLinearLeastSquares::Fit");
   n_param = Parameters.Nrows(); n_obs = Data.Nrows();
   DataPointer = &Data;
   FindMaximum2::Fit(Parameters, Lim);
   cout << "\nConverged" << endl;
}

void NonLinearLeastSquares::MakeCovariance()
{
   if (Covariance.Nrows()==0)
   {
      UpperTriangularMatrix UI = U.i();
      Covariance << UI * UI.t() * errorvar;
      SE << Covariance;                 // get diagonals
      for (int i = 1; i<=n_param; i++) SE(i) = sqrt(SE(i));
   }
}

void NonLinearLeastSquares::GetStandardErrors(ColumnVector& SEX)
   { MakeCovariance(); SEX = SE.AsColumn(); }

void NonLinearLeastSquares::GetCorrelations(SymmetricMatrix& Corr)
   { MakeCovariance(); Corr << SE.i() * Covariance * SE.i(); }

void NonLinearLeastSquares::GetHatDiagonal(DiagonalMatrix& Hat) const
{
   Hat.resize(n_obs);
   for (int i = 1; i<=n_obs; i++) Hat(i) = X.Row(i).SumSquare();
}


// the MLE_D_FI routines

void MLE_D_FI::Value
   (const ColumnVector& Parameters, bool wg, Real& v, bool& oorg)
{
   Tracer tr("MLE_D_FI::Value");
   if (!LL.IsValid(Parameters,wg)) { oorg=true; return; }
   v = LL.LogLikelihood();
   if (!LL.IsValid()) { oorg=true; return; }     // check validity again
   cout << endl;
   cout << setw(20) << setprecision(10) << v;
   oorg = false;
   Derivs = LL.Derivatives();                    // Get derivatives
}

bool MLE_D_FI::NextPoint(ColumnVector& Adj, Real& test)
{
   Tracer tr("MLE_D_FI::NextPoint");
   SymmetricMatrix FI = LL.FI();
   LT = Cholesky(FI);
   ColumnVector Adj1 = LT.i() * Derivs;
   Adj = LT.t().i() * Adj1;
   test = SumSquare(Adj1);
   cout << "   " << setw(20) << setprecision(10) << test;
   return (test < Criterion);
}

Real MLE_D_FI::LastDerivative(const ColumnVector& H)
{ return (Derivs.t() * H).AsScalar(); }

void MLE_D_FI::Fit(ColumnVector& Parameters)
{
   Tracer tr("MLE_D_FI::Fit");
   FindMaximum2::Fit(Parameters,Lim);
   cout << "\nConverged" << endl;
}
  
void MLE_D_FI::MakeCovariance()
{
   if (Covariance.Nrows()==0)
   {
      LowerTriangularMatrix LTI = LT.i();
      Covariance << LTI.t() * LTI;
      SE << Covariance;                // get diagonal
      int n = Covariance.Nrows();
      for (int i=1; i <= n; i++) SE(i) = sqrt(SE(i));
   }
}

void MLE_D_FI::GetStandardErrors(ColumnVector& SEX)
{ MakeCovariance(); SEX = SE.AsColumn(); }
   
void MLE_D_FI::GetCorrelations(SymmetricMatrix& Corr)
{ MakeCovariance(); Corr << SE.i() * Covariance * SE.i(); }



#ifdef use_namespace
}
#endif

