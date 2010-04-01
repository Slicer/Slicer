// This is an example of a non-linear least squares fit. The example
// is from "Nonlinear estimation" by Gavin Ross (Springer,1990), p 63.
// There are better ways of doing the fit in this case so this
// example is just an example.

// The model is E(y) = a + b exp(-kx) and there are 6 data points.

#define WANT_STREAM
#define WANT_MATH
#include "newmatnl.h"
#include "newmatio.h"

#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif


// first define the class describing the predictor function

class Model_3pe : public R1_Col_I_D
{
   ColumnVector x_values;         // the values of "x"
   RowVector deriv;               // values of derivatives
public:
   Model_3pe(const ColumnVector& X_Values)
      : x_values(X_Values) { deriv.resize(3); }
                                                                                         // load X data
   Real operator()(int);
   bool IsValid() { return para(3)>0; }
                                  // require "k" > 0
   ReturnMatrix Derivatives() { return deriv; }
};

Real Model_3pe::operator()(int i)
{
   Real a = para(1); Real b = para(2); Real k = para(3);
   Real xvi = x_values(i);
   Real e = exp(-k * xvi);
   deriv(1) = 1.0;                    // calculate derivatives
   deriv(2) = e;
   deriv(3) = - b * e * xvi;
   return a + b * e;                  // function value
}


int my_main()
{
   {
      // Get the data
      ColumnVector X(6);
      ColumnVector Y(6);
      X << 1   << 2   <<  3   <<  4   <<  6   <<  8;
      Y << 3.2 << 7.9 << 11.1 << 14.5 << 16.7 << 18.3;


      // Do the fit
      Model_3pe model(X);                // the model object
      NonLinearLeastSquares NLLS(model); // the non-linear least squares
                                         // object
      ColumnVector Para(3);              // for the parameters
      Para << 9 << -6 << .5;             // trial values of parameters
      cout << "Fitting parameters\n";
      NLLS.Fit(Y,Para);                  // do the fit

      // Inspect the results
      ColumnVector SE;                   // for the standard errors
      NLLS.GetStandardErrors(SE);
      cout << "\n\nEstimates and standard errors\n" <<
         setw(10) << setprecision(2) << (Para | SE) << endl;
      Real ResidualSD = sqrt(NLLS.ResidualVariance());
      cout << "\nResidual s.d. = " << setw(10) << setprecision(2) <<
         ResidualSD << endl;
      SymmetricMatrix Correlations;
      NLLS.GetCorrelations(Correlations);
      cout << "\nCorrelationMatrix\n" <<
         setw(10) << setprecision(2) << Correlations << endl;
      ColumnVector Residuals;
      NLLS.GetResiduals(Residuals);
      DiagonalMatrix Hat;
      NLLS.GetHatDiagonal(Hat);
      cout << "\nX, Y, Residual, Hat\n" << setw(10) << setprecision(2) <<
         (X | Y | Residuals | Hat.as_column()) << endl;
      // recover var/cov matrix
      SymmetricMatrix D;
      D << SE.as_diagonal() * Correlations * SE.as_diagonal();
      cout << "\nVar/cov\n" << setw(14) << setprecision(4) << D << endl;
   }

#ifdef DO_FREE_CHECK
   FreeCheck::Status();
#endif
 
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





