#define WANT_STREAM
#define WANT_TIME

#include "include.h"

#include "newmat.h"

#include "tmt.h"

#ifdef use_namespace
//using namespace NEWMAT;
namespace NEWMAT {
#endif


/**************************** test program ******************************/


class PrintCounter
{
   int count;
   const char* s;
public:
   ~PrintCounter();
   PrintCounter(const char * sx) : count(0), s(sx) {}
   void operator++() { count++; }
};

PrintCounter PCZ("Number of non-zero matrices (should be 1) = ");
PrintCounter PCN("Number of matrices tested                 = ");

PrintCounter::~PrintCounter()
{ cout << s << count << "\n"; }


void Print(const Matrix& X)
{
   ++PCN;
   cout << "\nMatrix type: " << X.Type().Value() << " (";
   cout << X.Nrows() << ", ";
   cout << X.Ncols() << ")\n\n";
   if (X.IsZero()) { cout << "All elements are zero\n" << flush; return; }
   int nr=X.Nrows(); int nc=X.Ncols();
   for (int i=1; i<=nr; i++)
   {
      for (int j=1; j<=nc; j++)  cout << X(i,j) << "\t";
      cout << "\n";
   }
   cout << flush; ++PCZ;
}

void Print(const UpperTriangularMatrix& X)
{
   ++PCN;
   cout << "\nMatrix type: " << X.Type().Value() << " (";
   cout << X.Nrows() << ", ";
   cout << X.Ncols() << ")\n\n";
   if (X.IsZero()) { cout << "All elements are zero\n" << flush; return; }
   int nr=X.Nrows(); int nc=X.Ncols();
   for (int i=1; i<=nr; i++)
   {
      int j;
      for (j=1; j<i; j++) cout << "\t";
      for (j=i; j<=nc; j++)  cout << X(i,j) << "\t";
      cout << "\n";
   }
   cout << flush; ++PCZ;
}

void Print(const DiagonalMatrix& X)
{
   ++PCN;
   cout << "\nMatrix type: " << X.Type().Value() << " (";
   cout << X.Nrows() << ", ";
   cout << X.Ncols() << ")\n\n";
   if (X.IsZero()) { cout << "All elements are zero\n" << flush; return; }
   int nr=X.Nrows(); int nc=X.Ncols();
   for (int i=1; i<=nr; i++)
   {
      for (int j=1; j<i; j++) cout << "\t";
      if (i<=nc) cout << X(i,i) << "\t";
      cout << "\n";
   }
   cout << flush; ++PCZ;
}

void Print(const SymmetricMatrix& X)
{
   ++PCN;
   cout << "\nMatrix type: " << X.Type().Value() << " (";
   cout << X.Nrows() << ", ";
   cout << X.Ncols() << ")\n\n";
   if (X.IsZero()) { cout << "All elements are zero\n" << flush; return; }
   int nr=X.Nrows(); int nc=X.Ncols();
   for (int i=1; i<=nr; i++)
   {
      int j;
      for (j=1; j<i; j++) cout << X(j,i) << "\t";
      for (j=i; j<=nc; j++)  cout << X(i,j) << "\t";
      cout << "\n";
   }
   cout << flush; ++PCZ;
}

void Print(const LowerTriangularMatrix& X)
{
   ++PCN;
   cout << "\nMatrix type: " << X.Type().Value() << " (";
   cout << X.Nrows() << ", ";
   cout << X.Ncols() << ")\n\n";
   if (X.IsZero()) { cout << "All elements are zero\n" << flush; return; }
   int nr=X.Nrows();
   for (int i=1; i<=nr; i++)
   {
      for (int j=1; j<=i; j++) cout << X(i,j) << "\t";
      cout << "\n";
   }
   cout << flush; ++PCZ;
}


void Clean(Matrix& A, Real c)
{
   int nr = A.Nrows(); int nc = A.Ncols();
   for (int i=1; i<=nr; i++)
   {
      for (int j=1; j<=nc; j++)
      { Real a = A(i,j); if ((a < c) && (a > -c)) A(i,j) = 0.0; }
   }
}

void Clean(DiagonalMatrix& A, Real c)
{
   int nr = A.Nrows();
   for (int i=1; i<=nr; i++)
   { Real a = A(i,i); if ((a < c) && (a > -c)) A(i,i) = 0.0; }
}

void PentiumCheck(Real N, Real D)
{
   Real R = N / D;
   R = R * D - N;
   if ( R > 1 || R < -1)
      cout << "Pentium error detected: % error = " << 100 * R / N << "\n";
}

// random number generator class
// See newran03 documentation for details

MultWithCarry::MultWithCarry(double s)
{
   if (s>=1.0 || s<=0.0)
      Throw(Logic_error("MultWithCarry: seed out of range"));
   x = (unsigned long)(s * 4294967296.0);
   crry = 1234567;
}


// carry out 32bit * 32bit multiply in software

void MultWithCarry::NextValue()
{
   unsigned long  mult = 2083801278;
   unsigned long  m_hi = mult >> 16;
   unsigned long  m_lo = mult & 0xFFFF;

   unsigned long  x_hi = x >> 16;
   unsigned long  x_lo = x & 0xFFFF;

   unsigned long  c_hi = crry >> 16;
   unsigned long  c_lo = crry & 0xFFFF;

   x = x_lo * m_lo + c_lo;
   unsigned long axc = x_lo * m_hi + x_hi * m_lo + c_hi + (x >> 16);
   crry = x_hi * m_hi + (axc >> 16);

   x = (x & 0xFFFF) + (axc << 16);

}

Real MultWithCarry::Next() { NextValue(); return ((double)x + 0.5) / 4294967296.0; }

// fill a matrix with values from the MultWithCarry random number generator
void FillWithValues(MultWithCarry&MWC, Matrix& M)
{
   int nr = M.nrows();
   int nc = M.ncols();
   for (int i = 1; i <= nr; ++i) for (int j = 1; j <= nc; ++ j)
   M(i, j) = MWC.Next();
}
   

#ifdef use_namespace
}
using namespace NEWMAT;
#endif


//*************************** main program **********************************

void TestTypeAdd();                            // test +
void TestTypeMult();                           // test *
void TestTypeConcat();                         // test |
void TestTypeSP();                             // test SP
void TestTypeKP();                             // test KP
void TestTypeOrder();                          // test >=


int main()
{
   time_lapse tl;      // measure program run time
   Real* s1; Real* s2; Real* s3; Real* s4;
   cout << "\nBegin test\n";   // Forces cout to allocate memory at beginning
   cout << "Now print a real number: " << 3.14159265 << endl;
   // Throw exception to set up exception buffer
#ifndef DisableExceptions
   Try { Throw(BaseException("Just a dummy\n")); }
   CatchAll {}
#else
   cout << "Not doing exceptions\n";
#endif
   { Matrix A1(40,200); s1 = A1.Store(); }
   { Matrix A1(1,1); s3 = A1.Store(); }
   {
      Tracer et("Matrix test program");

      Matrix A(25,150);
      {
         int i;
         RowVector A(8);
         for (i=1;i<=7;i++) A(i)=0.0; A(8)=1.0;
         Print(A);
      }
      cout << "\n";

      TestTypeAdd(); TestTypeMult(); TestTypeConcat();
      TestTypeSP(); TestTypeKP(); TestTypeOrder();

      Try { 
         trymat1();
         trymat2();
         trymat3();
         trymat4();
         trymat5();
         trymat6();
         trymat7();
         trymat8();
         trymat9();
         trymata();
         trymatb();
         trymatc();
         trymatd();
         trymate();
         trymatf();
         trymatg();
         trymath();
         trymati();
         trymatj();
         trymatk();
         trymatl();
         trymatm();

         cout << "\nEnd of tests\n";
      }
      CatchAll
      {
         cout << "\nTest program fails - exception generated\n\n";
         cout << BaseException::what();
      }


   }

   { Matrix A1(40,200); s2 = A1.Store(); }
   cout << "\n(The following memory checks are probably not valid with all\n";
   cout << "compilers - see documentation)\n";
   cout << "\nChecking for lost memory (large block): "
      << (unsigned long)s1 << " " << (unsigned long)s2 << " ";
   if (s1 != s2) cout << " - see section 2.8\n\n"; else cout << " - ok\n\n";
   { Matrix A1(1,1); s4 = A1.Store(); }
   cout << "\nChecking for lost memory (small block): "
      << (unsigned long)s3 << " " << (unsigned long)s4 << " ";
   if (s3 != s4) cout << " - see section 2.8\n\n"; else cout << " - ok\n\n";

   // check for Pentium bug
   PentiumCheck(4195835L,3145727L);
   PentiumCheck(5244795L,3932159L);

#ifdef DO_FREE_CHECK
   FreeCheck::Status();
#endif
   return 0;
}




//************************ test type manipulation **************************/


// These functions may cause problems for Glockenspiel 2.0c; they are used
// only for testing so you can delete them


void TestTypeAdd()
{
   MatrixType list[13];
   list[0] = MatrixType::UT;
   list[1] = MatrixType::LT;
   list[2] = MatrixType::Rt;
   list[3] = MatrixType::Sq;
   list[4] = MatrixType::Sm;
   list[5] = MatrixType::Sk;
   list[6] = MatrixType::Dg;
   list[7] = MatrixType::Id;
   list[8] = MatrixType::BM;
   list[9] = MatrixType::UB;
   list[10] = MatrixType::LB;
   list[11] = MatrixType::SB;
   list[12] = MatrixType::KB;

   cout << "+     ";
   int i;
   for (i=0; i<MatrixType::nTypes(); i++) cout << list[i].Value() << " ";
   cout << "\n";
   for (i=0; i<MatrixType::nTypes(); i++)
        {
                cout << list[i].Value() << " ";
      for (int j=0; j<MatrixType::nTypes(); j++)
         cout << (list[j]+list[i]).Value() << " ";
      cout << "\n";
   }
   cout << "\n";
}

void TestTypeMult()
{
   MatrixType list[13];
   list[0] = MatrixType::UT;
   list[1] = MatrixType::LT;
   list[2] = MatrixType::Rt;
   list[3] = MatrixType::Sq;
   list[4] = MatrixType::Sm;
   list[5] = MatrixType::Sk;
   list[6] = MatrixType::Dg;
   list[7] = MatrixType::Id;
   list[8] = MatrixType::BM;
   list[9] = MatrixType::UB;
   list[10] = MatrixType::LB;
   list[11] = MatrixType::SB;
   list[12] = MatrixType::KB;

   cout << "*     ";
   int i;
   for (i=0; i<MatrixType::nTypes(); i++)
      cout << list[i].Value() << " ";
   cout << "\n";
   for (i=0; i<MatrixType::nTypes(); i++)
   {
                cout << list[i].Value() << " ";
      for (int j=0; j<MatrixType::nTypes(); j++)
         cout << (list[j]*list[i]).Value() << " ";
      cout << "\n";
   }
   cout << "\n";
}

void TestTypeConcat()
{
   MatrixType list[13];
   list[0] = MatrixType::UT;
   list[1] = MatrixType::LT;
   list[2] = MatrixType::Rt;
   list[3] = MatrixType::Sq;
   list[4] = MatrixType::Sm;
   list[5] = MatrixType::Sk;
   list[6] = MatrixType::Dg;
   list[7] = MatrixType::Id;
   list[8] = MatrixType::BM;
   list[9] = MatrixType::UB;
   list[10] = MatrixType::LB;
   list[11] = MatrixType::SB;
   list[12] = MatrixType::KB;

   cout << "|     ";
   int i;
   for (i=0; i<MatrixType::nTypes(); i++)
      cout << list[i].Value() << " ";
   cout << "\n";
   for (i=0; i<MatrixType::nTypes(); i++)
   {
                cout << list[i].Value() << " ";
      for (int j=0; j<MatrixType::nTypes(); j++)
         cout << (list[j] | list[i]).Value() << " ";
      cout << "\n";
   }
   cout << "\n";
}

void TestTypeSP()
{
   MatrixType list[13];
   list[0] = MatrixType::UT;
   list[1] = MatrixType::LT;
   list[2] = MatrixType::Rt;
   list[3] = MatrixType::Sq;
   list[4] = MatrixType::Sm;
   list[5] = MatrixType::Sk;
   list[6] = MatrixType::Dg;
   list[7] = MatrixType::Id;
   list[8] = MatrixType::BM;
   list[9] = MatrixType::UB;
   list[10] = MatrixType::LB;
   list[11] = MatrixType::SB;
   list[12] = MatrixType::KB;

   cout << "SP    ";
   int i;
   for (i=0; i<MatrixType::nTypes(); i++)
                cout << list[i].Value() << " ";
   cout << "\n";
   for (i=0; i<MatrixType::nTypes(); i++)
   {
                cout << list[i].Value() << " ";
      for (int j=0; j<MatrixType::nTypes(); j++)
         cout << (list[j].SP(list[i])).Value() << " ";
      cout << "\n";
   }
   cout << "\n";
}

void TestTypeKP()
{
   MatrixType list[13];
   list[0] = MatrixType::UT;
   list[1] = MatrixType::LT;
   list[2] = MatrixType::Rt;
   list[3] = MatrixType::Sq;
   list[4] = MatrixType::Sm;
   list[5] = MatrixType::Sk;
   list[6] = MatrixType::Dg;
   list[7] = MatrixType::Id;
   list[8] = MatrixType::BM;
   list[9] = MatrixType::UB;
   list[10] = MatrixType::LB;
   list[11] = MatrixType::SB;
   list[12] = MatrixType::KB;

   cout << "KP    ";
   int i;
   for (i=0; i<MatrixType::nTypes(); i++)
                cout << list[i].Value() << " ";
   cout << "\n";
   for (i=0; i<MatrixType::nTypes(); i++)
   {
                cout << list[i].Value() << " ";
      for (int j=0; j<MatrixType::nTypes(); j++)
         cout << (list[j].KP(list[i])).Value() << " ";
      cout << "\n";
   }
   cout << "\n";
}

void TestTypeOrder()
{
   MatrixType list[13];
   list[0] = MatrixType::UT;
   list[1] = MatrixType::LT;
   list[2] = MatrixType::Rt;
   list[3] = MatrixType::Sq;
   list[4] = MatrixType::Sm;
   list[5] = MatrixType::Sk;
   list[6] = MatrixType::Dg;
   list[7] = MatrixType::Id;
   list[8] = MatrixType::BM;
   list[9] = MatrixType::UB;
   list[10] = MatrixType::LB;
   list[11] = MatrixType::SB;
   list[12] = MatrixType::KB;

   cout << ">=    ";
   int i;
   for (i = 0; i<MatrixType::nTypes(); i++)
      cout << list[i].Value() << " ";
   cout << "\n";
   for (i=0; i<MatrixType::nTypes(); i++)
   {
      cout << list[i].Value() << " ";
      for (int j=0; j<MatrixType::nTypes(); j++)
         cout << ((list[j]>=list[i]) ? "Yes   " : "No    ");
      cout << "\n";
   }
   cout << "\n";
}


//************** elapsed time class ****************

time_lapse::time_lapse()
{
   start_time = ((double)clock())/(double)CLOCKS_PER_SEC;
}

time_lapse::~time_lapse()
{
   double time = ((double)clock())/(double)CLOCKS_PER_SEC - start_time;
   cout << "Elapsed (processor) time = " << setprecision(2) << time << " seconds" << endl;
   cout << endl;
}







