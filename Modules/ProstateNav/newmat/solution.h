//$$ solution.h                      // solve routines

#include "myexcept.h"

#ifdef use_namespace
namespace RBD_COMMON {
#endif


// Solve the equation f(x)=y for x where f is a monotone continuous
// function of x
// Essentially Brent s method

// You need to derive a class from R1_R1 and override "operator()"
// with the function you want to solve.
// Use an object from this class in OneDimSolve

class R1_R1
{
   // the prototype for a Real function of a Real variable
   // you need to derive your function from this one and put in your
   // function for operator() at least. You probably also want to set up a
   // constructor to put in additional parameter values (e.g. that will not
   // vary during a solve)

protected:
   Real x;                             // Current x value
   bool xSet;                          // true if a value assigned to x

public:
   Real minX, maxX;                    // range of value x
   bool minXinf, maxXinf;              // true if these are infinite
   R1_R1() : xSet(false), minXinf(true), maxXinf(true) {}
   virtual Real operator()() = 0;      // function value at current x
                                       // set current x
   virtual void Set(Real X);           // set x, check OK
   Real operator()(Real X) { Set(X); return operator()(); }
                                       // set x, return value
   virtual bool IsValid(Real X);
   operator Real();                    // implicit conversion
};

class SolutionException : public BaseException
{
public:
   static unsigned long Select;
   SolutionException(const char* a_what = 0);
};

class OneDimSolve
{
   R1_R1& function;                     // reference to the function
   Real accX;                           // accuracy in X direction
   Real accY;                           // accuracy in Y direction
   int lim;                             // maximum number of iterations

public:
   OneDimSolve(R1_R1& f, Real AccY = 0.0001, Real AccX = 0.0)
      : function(f), accX(AccX), accY(AccY) {}
                       // f is an R1_R1 function
   Real Solve(Real Y, Real X, Real Dev, int Lim=100);
                       // Solve for x in Y=f(x)
                       // X is the initial trial value of x
                       // X+Dev is the second trial value
                       // program returns a value of x such that
                       // |Y-f(x)| <= accY or |f.inv(Y)-x| <= accX

private:
   Real x[3], y[3];                         // Trial values of X and Y
   int L,C,U,Last;                          // Locations of trial values
   int vpol, hpol;                          // polarities
   Real YY;                                 // target value
   int i;
   void LookAt(int);                        // get new value of function
   bool Finish;                             // true if LookAt finds conv.
   bool Captured;                           // true when target surrounded
   void VFlip();
   void HFlip();
   void Flip();
   void State(int I, int J, int K);
   void Linear(int, int, int);
   void Quadratic(int, int, int);
};


#ifdef use_namespace
}
#endif

// body file: solution.cpp



