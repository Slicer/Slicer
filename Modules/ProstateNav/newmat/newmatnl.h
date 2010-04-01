//$$ newmatnl.h           definition file for non-linear optimisation

// Copyright (C) 1993,4,5: R B Davies

#ifndef NEWMATNL_LIB
#define NEWMATNL_LIB 0

#include "newmat.h"

#ifdef use_namespace
namespace NEWMAT {
#endif



/*

This is a beginning of a series of classes for non-linear optimisation.

At present there are two classes. FindMaximum2 is the basic optimisation
strategy when one is doing an optimisation where one has first
derivatives and estimates of the second derivatives. Class
NonLinearLeastSquares is derived from FindMaximum2. This provides the
functions that calculate function values and derivatives.

A third class is now added. This is for doing maximum-likelihood when
you have first derviatives and something like the Fisher Information
matrix (eg the variance covariance matrix of the first derivatives or
minus the second derivatives - this matrix is assumed to be positive
definite).



   class FindMaximum2

Suppose T is the ColumnVector of parameters, F(T) the function we want
to maximise, D(T) the ColumnVector of derivatives of F with respect to
T, and S(T) the matrix of second derivatives.

Then the basic iteration is given a value of T, update it to

           T - S.i() * D

where .i() denotes inverse.

If F was quadratic this would give exactly the right answer (except it
might get a minimum rather than a maximum). Since F is not usually
quadratic, the simple procedure would be to recalculate S and D with the
new value of T and keep iterating until the process converges. This is
known as the method of conjugate gradients.

In practice, this method may not converge. FindMaximum2 considers an
iteration of the form

           T - x * S.i() * D

where x is a number. It tries x = 1 and uses the values of F and its
slope with respect to x at x = 0 and x = 1 to fit a cubic in x. It then
choses x to maximise the resulting function. This gives our new value of
T. The program checks that the value of F is getting better and carries
out a variety of strategies if it is not.

The program also has a second strategy. If the successive values of T
seem to be lying along a curve - eg we are following along a curved
ridge, the program will try to fit this ridge and project along it. This
does not work at present and is commented out.

FindMaximum2 has three virtual functions which need to be over-ridden by
a derived class.

   void Value(const ColumnVector& T, bool wg, Real& f, bool& oorg);

T is the column vector of parameters. The function returns the value of
the function to f, but may instead set oorg to true if the parameter
values are not valid. If wg is true it may also calculate and store the
second derivative information.

   bool NextPoint(ColumnVector& H, Real& d);

Using the value of T provided in the previous call of Value, find the
conjugate gradients adjustment to T, that is - S.i() * D. Also return

           d = D.t() * S.i() * D.

NextPoint should return true if it considers that the process has
converged (d very small) and false otherwise. The previous call of Value
will have set wg to true, so that S will be available.

   Real LastDerivative(const ColumnVector& H);

Return the scalar product of H and the vector of derivatives at the last
value of T.

The function Fit is the function that calls the iteration.

   void Fit(ColumnVector&, int);

The arguments are the trial parameter values as a ColumnVector and the
maximum number of iterations. The program calls a DataException if the
initial parameters are not valid and a ConvergenceException if the
process fails to converge.


   class NonLinearLeastSquares

This class is derived from FindMaximum2 and carries out a non-linear
least squares fit. It uses a QR decomposition to carry out the
operations required by FindMaximum2.

A prototype class R1_Col_I_D is provided. The user needs to derive a
class from this which includes functions the predicted value of each
observation its derivatives. An object from this class has to be
provided to class NonLinearLeastSquares.

Suppose we observe n normal random variables with the same unknown
variance and such the i-th one has expected value given by f(i,P)
where P is a column vector of unknown parameters and f is a known
function. We wish to estimate P.

First derive a class from R1_Col_I_D and override Real operator()(int i)
to give the value of the function f in terms of i and the ColumnVector
para defined in class R1_CoL_I_D. Also override ReturnMatrix
Derivatives() to give the derivates of f at para and the value of i
used in the preceeding call to operator(). Return the result as a
RowVector. Construct an object from this class. Suppose in what follows
it is called pred.

Now constuct a NonLinearLeastSquaresObject accessing pred and optionally
an iteration limit and an accuracy critierion.

   NonLinearLeastSquares NLLS(pred, 1000, 0.0001);

The accuracy critierion should be somewhat less than one and 0.0001 is
about the smallest sensible value.

Define a ColumnVector P containing a guess at the value of the unknown
parameter, and a ColumnVector Y containing the unknown data. Call

   NLLS.Fit(Y,P);

If the process converges, P will contain the estimates of the unknown
parameters. If it does not converge an exception will be generated.

The following member functions can be called after you have done a fit.

Real ResidualVariance() const;

The estimate of the variance of the observations.

void GetResiduals(ColumnVector& Z) const;

The residuals of the individual observations.

void GetStandardErrors(ColumnVector&);

The standard errors of the observations.

void GetCorrelations(SymmetricMatrix&);

The correlations of the observations.

void GetHatDiagonal(DiagonalMatrix&) const;

Forms a diagonal matrix of values between 0 and 1. If the i-th value is
larger than, say 0.2, then the i-th data value could have an undue
influence on your estimates.


*/

class FindMaximum2
{
   virtual void Value(const ColumnVector&, bool, Real&, bool&) = 0;
   virtual bool NextPoint(ColumnVector&, Real&) = 0;
   virtual Real LastDerivative(const ColumnVector&) = 0;
public:
   void Fit(ColumnVector&, int);
   virtual ~FindMaximum2() {}            // to keep gnu happy
};

class R1_Col_I_D
{
   // The prototype for a Real function of a ColumnVector and an
   // integer.
   // You need to derive your function from this one and put in your
   // function for operator() and Derivatives() at least.
   // You may also want to set up a constructor to enter in additional
   // parameter values (that will not vary during the solve).

protected:
   ColumnVector para;                 // Current x value

public:
   virtual bool IsValid() { return true; }
                                       // is the current x value OK
   virtual Real operator()(int i) = 0; // i-th function value at current para
   virtual void Set(const ColumnVector& X) { para = X; }
                                       // set current para
   bool IsValid(const ColumnVector& X)
      { Set(X); return IsValid(); }
                                       // set para, check OK
   Real operator()(int i, const ColumnVector& X)
      { Set(X); return operator()(i); }
                                       // set para, return value
   virtual ReturnMatrix Derivatives() = 0;
                                       // return derivatives as RowVector
   virtual ~R1_Col_I_D() {}            // to keep gnu happy
};


class NonLinearLeastSquares : public FindMaximum2
{
   // these replace the corresponding functions in FindMaximum2
   void Value(const ColumnVector&, bool, Real&, bool&);
   bool NextPoint(ColumnVector&, Real&);
   Real LastDerivative(const ColumnVector&);

   Matrix X;                         // the things we need to do the
   ColumnVector Y;                   // QR triangularisation
   UpperTriangularMatrix U;          // see the write-up in newmata.txt
   ColumnVector M;
   Real errorvar, criterion;
   int n_obs, n_param;
   const ColumnVector* DataPointer;
   RowVector Derivs;
   SymmetricMatrix Covariance;
   DiagonalMatrix SE;
   R1_Col_I_D& Pred;                 // Reference to predictor object
   int Lim;                          // maximum number of iterations

public:
   NonLinearLeastSquares(R1_Col_I_D& pred, int lim=1000, Real crit=0.0001)
      : criterion(crit), Pred(pred), Lim(lim) {}
   void Fit(const ColumnVector&, ColumnVector&);
   Real ResidualVariance() const { return errorvar; }
   void GetResiduals(ColumnVector& Z) const { Z = Y; }
   void GetStandardErrors(ColumnVector&);
   void GetCorrelations(SymmetricMatrix&);
   void GetHatDiagonal(DiagonalMatrix&) const;

private:
   void MakeCovariance();
};


// The next class is the prototype class for calculating the
// log-likelihood.
// I assume first derivatives are available and something like the 
// Fisher Information or variance/covariance matrix of the first
// derivatives or minus the matrix of second derivatives is
// available. This matrix must be positive definite.

class LL_D_FI
{
protected:
   ColumnVector para;                  // current parameter values
   bool wg;                         // true if FI matrix wanted

public:
   virtual void Set(const ColumnVector& X) { para = X; }
                                       // set parameter values
   virtual void WG(bool wgx) { wg = wgx; }
                                       // set wg

   virtual bool IsValid() { return true; }
                                       // return true is para is OK
   bool IsValid(const ColumnVector& X, bool wgx=true)
      { Set(X); WG(wgx); return IsValid(); }

   virtual Real LogLikelihood() = 0;   // return the loglikelihhod
   Real LogLikelihood(const ColumnVector& X, bool wgx=true)
      { Set(X); WG(wgx); return LogLikelihood(); }

   virtual ReturnMatrix Derivatives() = 0;
                                       // column vector of derivatives
   virtual ReturnMatrix FI() = 0;      // Fisher Information matrix
   virtual ~LL_D_FI() {}               // to keep gnu happy
};

// This is the class for doing the maximum likelihood estimation

class MLE_D_FI : public FindMaximum2
{
   // these replace the corresponding functions in FindMaximum2
   void Value(const ColumnVector&, bool, Real&, bool&);
   bool NextPoint(ColumnVector&, Real&);
   Real LastDerivative(const ColumnVector&);

   // the things we need for the analysis
   LL_D_FI& LL;                        // reference to log-likelihood
   int Lim;                            // maximum number of iterations
   Real Criterion;                     // convergence criterion
   ColumnVector Derivs;                // for the derivatives
   LowerTriangularMatrix LT;           // Cholesky decomposition of FI
   SymmetricMatrix Covariance;
   DiagonalMatrix SE;

public:
   MLE_D_FI(LL_D_FI& ll, int lim=1000, Real criterion=0.0001)
      : LL(ll), Lim(lim), Criterion(criterion) {}
   void Fit(ColumnVector& Parameters);
   void GetStandardErrors(ColumnVector&);
   void GetCorrelations(SymmetricMatrix&);

private:
   void MakeCovariance();
};


#ifdef use_namespace
}
#endif



#endif

// body file: newmatnl.cpp




