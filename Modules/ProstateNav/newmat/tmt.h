// definition file for test programs

//#define DONT_DO_NRIC         // activate if running a bounds checker

#ifdef use_namespace
//using namespace NEWMAT;
namespace NEWMAT {
#endif

// print time between construction and destruction
class time_lapse
{
   double start_time;
public:
   time_lapse();
   ~time_lapse();
};

// random number generator

class MultWithCarry
{
   unsigned long x;
   unsigned long crry;

   void NextValue();

   void operator=(const MultWithCarry&) {}    // private so can't access

public:
   MultWithCarry(double s=0.46875);
   Real Next();
   ~MultWithCarry() {}
};

// fill a matrix with values from the MultWithCarry random number generator
void FillWithValues(MultWithCarry& MWC, Matrix& M);   


void Print(const Matrix& X);
void Print(const UpperTriangularMatrix& X);
void Print(const DiagonalMatrix& X);
void Print(const SymmetricMatrix& X);
void Print(const LowerTriangularMatrix& X);

void Clean(Matrix&, Real);
void Clean(DiagonalMatrix&, Real);

#ifdef use_namespace
}
using namespace NEWMAT;
#endif



void trymat1(); void trymat2(); void trymat3();
void trymat4(); void trymat5(); void trymat6();
void trymat7(); void trymat8(); void trymat9();
void trymata(); void trymatb(); void trymatc();
void trymatd(); void trymate(); void trymatf();
void trymatg(); void trymath(); void trymati();
void trymatj(); void trymatk(); void trymatl();
void trymatm();



// body file: tmt.cpp
// body file: tmt1.cpp
// body file: tmt2.cpp
// body file: tmt3.cpp
// body file: tmt4.cpp
// body file: tmt5.cpp
// body file: tmt6.cpp
// body file: tmt7.cpp
// body file: tmt8.cpp
// body file: tmt9.cpp
// body file: tmta.cpp
// body file: tmtb.cpp
// body file: tmtc.cpp
// body file: tmtd.cpp
// body file: tmte.cpp
// body file: tmtf.cpp
// body file: tmtg.cpp
// body file: tmth.cpp
// body file: tmti.cpp
// body file: tmtj.cpp
// body file: tmtk.cpp
// body file: tmtl.cpp
// body file: tmtm.cpp

