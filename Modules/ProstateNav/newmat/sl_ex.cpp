// This is an example of the use of solution to find the cube root of 
// the integers -10 to 10

// you will need to compile and link solution.cpp and except.cpp

#define WANT_STREAM
#define WANT_MATH

#include "include.h"
#include "solution.h"

#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif


// the cube class

class Cube : public R1_R1
{ Real operator()() { return x*x*x; } };


int my_main()
{
   // construct the Cube object
   Cube cube;
   // and then the solve object
   OneDimSolve cube_root(cube);
   // Now do the solves
   for (int i=-10; i<=10; i++)
      cout << i << "   "  << cube_root.Solve(i,0,1.5) << endl;
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

