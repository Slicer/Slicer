// simple example 2
// Generate a Hilbert matrix and work out its eigenvalues
// and eigenvectors; check result by multiplying out.

// The Hilbert matrix is notoriously ill-conditioned (difficult to invert).
// In this example, I calculate the eigenvalues of a 7 x 7 Hilbert matrix.

// The dimensions of this matrix are not large enough for there to be numerical
// problems but we will be able to see that wide range of values of the
// eigenvalues. 

#define WANT_STREAM            // include iostream and iomanipulators

#include "newmatap.h"          // newmat headers including advanced functions
#include "newmatio.h"          // newmat headers including output functions

#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif


int my_main()                  // called by main()
{
   Tracer tr("my_main ");      // for tracking exceptions
   
   int n = 7;                 // this is the order we will work with
   int i, j;

   // declare a matrix
   SymmetricMatrix H(n);
   
   // load values for Hilbert matrix
   for (i = 1; i <= n; ++i) for (j = 1; j <= i; ++j)
      H(i, j) = 1.0 / (i + j - 1);

   // print the matrix
   cout << "SymmetricMatrix H" << endl;
   cout << setw(10) << setprecision(7) << H << endl;
   
   // calculate its eigenvalues and eigenvectors and print them
   Matrix U; DiagonalMatrix D;
   eigenvalues(H, D, U);
   cout << "Eigenvalues of H" << endl;
   cout << setw(17) << setprecision(14) << D.as_column() << endl;
   cout << "Eigenvector matrix, U" << endl;
   cout << setw(10) << setprecision(7) << U << endl;

   // check orthogonality
   cout << "U * U.t() (should be near identity)" << endl;   
   cout << setw(10) << setprecision(7) << (U * U.t()) << endl;
   
   // check decomposition
   cout << "U * D * U.t() (should be near H)" << endl;   
   cout << setw(10) << setprecision(7) << (U * D * U.t()) << endl;
   
   return 0;
}


// call my_main() - use this to catch exceptions
// use macros for exception names for compatibility with simuated exceptions
int main()
{
   Try  { return my_main(); }
   Catch(BaseException) { cout << BaseException::what() << "\n"; }
   CatchAll { cout << "\nProgram fails - exception generated\n\n"; }
   return 0;
}


