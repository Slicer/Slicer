#define WANT_STREAM

#include "newmatap.h"
#include "newmatio.h"              // to help namespace with VC++ 5

#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif

//#include <except.h>             // if you want to use set_terminate

/**************************** test exceptions ******************************/



int main()
{
   // activate the next expression if you want to use compiler supported
   // exceptions and you want Terminate to catch uncaught exceptions
   // set_terminate(Terminate);
   Real* s1; Real* s2; Real* s3; Real* s4;
   // Forces cout to allocate memory at beginning
   cout << "\nThis tests the exception system, so you will get\n" <<
      "a long list of error messages\n\n";
   cout << "\nPrint a real number (may help lost memory test): "
      << 3.14159265 << "\n";
   // Throw exception to set up exception buffer
   Try { Throw(BaseException("Just a dummy\n")); }
   CatchAll {};
   { Matrix A1(40,200); s1 = A1.data(); }
   { Matrix A1(1,1); s3 = A1.data(); }
   {
      Tracer et("Test");

      Try
      {
         Tracer et("Try block");



         cout << "-----------------------------------------\n\n";
         Matrix A(2,3), B(4,5); A = 1; B = 2;
         cout << "Incompatible dimensions\n";
         et.ReName("Block A");
         Try { Matrix C = A + B; }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Bad index\n";
         et.ReName("Block B");
         Try { Real f = A(3,3); cout << f << endl; }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Illegal conversion\n";
         et.ReName("Block C");
         Try { UpperTriangularMatrix U = A; }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Invert non-square matrix - 1\n";
         et.ReName("Block D");
         Try { CroutMatrix X = A; }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Invert non-square matrix - 2\n";
         et.ReName("Block E");
         Try { Matrix X = A.i(); }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Non 1x1 matrix to scalar\n";
         et.ReName("Block F");
         Try { Real f = A.as_scalar(); cout << f << endl; }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Matrix to vector\n";
         et.ReName("Block G");
         Try { ColumnVector CV = A;}
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Invert singular matrix\n";
         et.ReName("Block H");
         Try { Matrix X(2,2); X<<1<<2<<2<<4; X = X.i(); }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "SubMatrix error\n";
         et.ReName("Block I");
         Try { Matrix X = A.Row(3); }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "SubMatrix error\n";
         et.ReName("Block J");
         Try { Matrix X = A.Row(0); }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Cholesky error\n";
         et.ReName("Block K");
         Try
         {
            SymmetricMatrix SM(50); SM = 10;
            LowerTriangularMatrix L = Cholesky(SM);
         }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Inequality error\n";
         et.ReName("Block L");
         Try
         {
            Matrix A(10,10), B(10,10); A = 10; B = 20;
            if ( A < B) A = B;
         }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Maximum of empty matrix\n";
         et.ReName("Block M");
         Try
         {
            Matrix A(10,20); A = 5; Matrix B=A.Rows(6,5);
            maximum_absolute_value(B);
         }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Incorrectly ReSizing band matrix\n";
         et.ReName("Block N");
         Try
         {
            BandMatrix A(20,5,3); A = 5; UpperBandMatrix B;
            B.resize(A);
         }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Incorrectly resizing symmetric band matrix\n";
         et.ReName("Block M");
         Try
         {
            BandMatrix A(20,5,3); A = 5; SymmetricBandMatrix B;
            B.ReSize(A);
         }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "ReSize CroutMatrix\n";
         et.ReName("Block O");
         Try
         {
            Matrix A(3,3); A = 0; A(1,1) = A(2,2) = A(3,3) = 1;
            CroutMatrix B = A;
            B.resize(A);
         }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Manipulate CroutMatrix\n";
         et.ReName("Block P");
         Try
         {
            Matrix A(3,3); A = 0; A(1,1) = A(2,2) = A(3,3) = 1;
            CroutMatrix B = A;
            Matrix C = B;
         }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";

         cout << "Manipulate BandLUMatrix\n";
         et.ReName("Block Q");
         Try
         {
            SymmetricBandMatrix A(3,1); A = 0; A(1,1) = A(2,2) = A(3,3) = 1;
            BandLUMatrix B = A;
            Matrix C = B;
         }
         CatchAll { cout << BaseException::what() << endl; }
         cout << "-----------------------------------------\n\n";


      }
      CatchAll { cout << "\nException generated in test program\n\n"; }
   }

   cout << "\nEnd test\n";
   { Matrix A1(40,200); s2 = A1.data(); }
   cout << "\n(The following memory checks are probably not valid with all\n";
   cout << "compilers - see documentation)\n";
   cout << "\nChecking for lost memory (large block): "
      << (unsigned long)s1 << " " << (unsigned long)s2 << " ";
   if (s1 != s2) cout << " - see section 2.8\n"; else cout << " - ok\n";
   { Matrix A1(1,1); s4 = A1.data(); }
   cout << "\nChecking for lost memory (small block): "
      << (unsigned long)s3 << " " << (unsigned long)s4 << " ";
   if (s3 != s4) cout << " - see section 2.8\n\n"; else cout << " - ok\n\n";


#ifdef DO_FREE_CHECK
   FreeCheck::Status();
#endif

//   Throw(Runtime_error("Exception outside try block"));

   return 0;
}
