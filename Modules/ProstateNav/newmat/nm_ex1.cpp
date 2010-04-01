// simple example 1
// invert a 4 x 4 matrix then check the result

#define WANT_STREAM       // include iostream and iomanipulators

#include "newmatap.h"     // newmat advanced functions
                          // should not be required for this example
                          // included because it seems to help MS VC6
                          // when you have namespace turned on
                           
#include "newmatio.h"     // newmat headers including output functions

#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif


int my_main()                  // called by main()
{
   Tracer tr("my_main ");      // for tracking exceptions

   // declare a matrix
   Matrix X(4,4);
   
   // load values row by row
   X.row(1) <<  3.7 << -2.1 <<  7.4 << -1.0;
   X.row(2) <<  4.1 <<  0.0 <<  3.9 <<  4.0;
   X.row(3) << -2.5 <<  1.9 << -0.4 <<  7.3;
   X.row(4) <<  1.5 <<  9.8 << -2.1 <<  1.1;

   // print the matrix
   cout << "Matrix X" << endl;
   cout << setw(15) << setprecision(8) << X << endl;
   
   // calculate its inverse and print it
   Matrix Y = X.i();
   cout << "Inverse of X" << endl;
   cout << setw(15) << setprecision(8) << Y << endl;
   
   // multiply X by its inverse and print the result (should be near identity)
   cout << "X * inverse of X" << endl;
   cout << setw(15) << setprecision(8) << (X * Y) << endl;
   
   return 0;
}


// call my_main() - use this to catch exceptions
// use macros for exception names for compatibility with simulated exceptions
int main()
{
   Try  { return my_main(); }
   Catch(BaseException) { cout << BaseException::what() << "\n"; }
   CatchAll { cout << "\nProgram fails - exception generated\n\n"; }
   return 0;
}


