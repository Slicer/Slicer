
#define WANT_STREAM
#define WANT_MATH
#define WANT_FSTREAM

#include "newmatap.h"
#include "newmatio.h"
#include "newmatnl.h"

#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif

// This is a demonstration of a special case of the Garch model
// Observe two series X and Y of length n
// and suppose
//    Y(i) = beta * X(i) + epsilon(i)
// where epsilon(i) is normally distributed with zero mean and variance =
//    h(i) = alpha0 + alpha1 * square(epsilon(i-1)) + beta1 * h(i-1).
// Then this program is supposed to estimate beta, alpha0, alpha1, beta1
// The Garch model is supposed to model something like an instability
// in the stock or options market following an unexpected result.
// alpha1 determines the size of the instability and beta1 determines how
// quickly is dies away.
// We should, at least, have an X of several columns and beta as a vector

inline Real square(Real x) { return x*x; }

// the class that defines the GARCH log-likelihood

class GARCH11_LL : public LL_D_FI
{
   ColumnVector Y;                 // Y values
   ColumnVector X;                 // X values
   ColumnVector D;                 // derivatives of loglikelihood
   SymmetricMatrix D2;             // - approximate second derivatives
   int n;                          // number of observations
   Real beta, alpha0, alpha1, beta1;
                                   // the parameters

public:

   GARCH11_LL(const ColumnVector& y, const ColumnVector& x)
      : Y(y), X(x), n(y.nrows()) {}
                                   // constructor - load Y and X values

   void Set(const ColumnVector& p) // set parameter values
   {
      para = p;
      beta = para(1); alpha0 = para(2);
      alpha1 = para(3); beta1 = para(4);
   }

   bool IsValid();                 // are parameters valid
   Real LogLikelihood();           // return the loglikelihood
   ReturnMatrix Derivatives();     // derivatives of log-likelihood
   ReturnMatrix FI();              // Fisher Information matrix
};

bool GARCH11_LL::IsValid()
{ return alpha0>0 && alpha1>0 && beta1>0 && (alpha1+beta1)<1.0; }

Real GARCH11_LL::LogLikelihood()
{
//   cout << endl << "                           ";
//   cout << setw(10) << setprecision(5) << beta;
//   cout << setw(10) << setprecision(5) << alpha0;
//   cout << setw(10) << setprecision(5) << alpha1;
//   cout << setw(10) << setprecision(5) << beta1;
//   cout << endl;
   ColumnVector H(n);              // residual variances
   ColumnVector U = Y - X * beta;  // the residuals
   ColumnVector LH(n);     // derivative of log-likelihood wrt H
                           // each row corresponds to one observation
   LH(1)=0;
   Matrix Hderiv(n,4);     // rectangular matrix of derivatives
                           // of H wrt parameters
                           // each row corresponds to one observation
                           // each column to one of the parameters

   // Regard Y(1) as fixed and don't include in likelihood
   // then put in an expected value of H(1) in place of actual value
   //   which we don't know. Use
   // E{H(i)} = alpha0 + alpha1 * E{square(epsilon(i-1))} + beta1 * E{H(i-1)}
   // and  E{square(epsilon(i-1))} = E{H(i-1)} = E{H(i)}
   Real denom = (1-alpha1-beta1);
   H(1) = alpha0/denom;    // the expected value of H
   Hderiv(1,1) = 0;
   Hderiv(1,2) = 1.0 / denom;
   Hderiv(1,3) = alpha0 / square(denom);
   Hderiv(1,4) = Hderiv(1,3);
   Real LL = 0.0;          // the log likelihood
   Real sum1 = 0;          // for forming derivative wrt beta
   Real sum2 = 0;          // for forming second derivative wrt beta
   for (int i=2; i<=n; i++)
   {
      Real u1 = U(i-1); Real h1 = H(i-1);
      Real h = alpha0 + alpha1*square(u1) + beta1*h1; // variance of this obsv.
      H(i) = h; Real u = U(i);
      LL += log(h) + square(u) / h;        // -2 * log likelihood
      // Hderiv are derivatives of h with respect to the parameters
      // need to allow for h1 depending on parameters
      Hderiv(i,1) = -2*u1*alpha1*X(i-1) + beta1*Hderiv(i-1,1);  // beta
      Hderiv(i,2) = 1 + beta1*Hderiv(i-1,2);                    // alpha0
      Hderiv(i,3) = square(u1) + beta1*Hderiv(i-1,3);           // alpha1
      Hderiv(i,4) = h1 + beta1*Hderiv(i-1,4);                   // beta1
      LH(i) = -0.5 * (1/h - square(u/h));
      sum1 += u * X(i)/ h;
      sum2 += square(X(i)) / h;
   }
   D = Hderiv.t()*LH;         // derivatives of likelihood wrt parameters
   D(1) += sum1;              // add on deriv wrt beta from square(u) term
//   cout << setw(10) << setprecision(5) << D << endl;

   // do minus expected value of second derivatives
   if (wg)                    // do only if second derivatives wanted
   {
      Hderiv.row(1) = 0.0;
      Hderiv = H.as_diagonal().i() * Hderiv;
      D2 << Hderiv.t() * Hderiv;  D2 = D2 / 2.0;
      D2(1,1) += sum2;
//      cout << setw(10) << setprecision(5) << D2 << endl;
//      DiagonalMatrix DX; EigenValues(D2,DX);
//      cout << setw(10) << setprecision(5) << DX << endl;

   }
   return -0.5 * LL;
}

ReturnMatrix GARCH11_LL::Derivatives()
{ return D; }

ReturnMatrix GARCH11_LL::FI()
{
   if (!wg) cout << endl << "unexpected call of FI" << endl;
   return D2;
}


int my_main()
{
   // get data
   ifstream fin("garch.dat");
   if (!fin) { cout << "cannot find garch.dat\n"; exit(1); }
   int n; fin >> n;            // series length
   // Y contains the dependant variable, X the predictor variable
   ColumnVector Y(n), X(n);
   int i;
   for (i=1; i<=n; i++) fin >> Y(i) >> X(i);
   cout << "Read " << n << " data points - begin fit\n\n";
   // now do the fit
   ColumnVector H(n);
   GARCH11_LL garch11(Y,X);                  // loglikehood "object"
   MLE_D_FI mle_d_fi(garch11,100,0.0001);    // mle "object"
   ColumnVector Para(4);                     // to hold the parameters
   Para << 0.0 << 0.1 << 0.1 << 0.1;         // starting values
      // (Should change starting values to a more intelligent formula)
   mle_d_fi.Fit(Para);                       // do the fit
   ColumnVector SE;
   mle_d_fi.GetStandardErrors(SE);
   cout << "\n\n";
   cout << "estimates and standard errors\n";
   cout << setw(15) << setprecision(5) << (Para | SE) << endl << endl;
   SymmetricMatrix Corr;
   mle_d_fi.GetCorrelations(Corr);
   cout << "correlation matrix\n";
   cout << setw(10) << setprecision(2) << Corr << endl << endl;
   cout << "inverse of correlation matrix\n";
   cout << setw(10) << setprecision(2) << Corr.i() << endl << endl;
   return 0;
}


// call my_main() - use this to catch exceptions
int main()
{
   Try
   {
      return my_main();
   }
   Catch(BaseException)
   {
      cout << BaseException::what() << "\n";
   }
   CatchAll
   {
      cout << "\nProgram fails - exception generated\n\n"; 
   }
   return 0;
}


