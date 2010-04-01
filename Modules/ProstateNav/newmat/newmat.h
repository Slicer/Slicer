//$$ newmat.h           definition file for new version of matrix package

// Copyright (C) 2004: R B Davies

#ifndef NEWMAT_LIB
#define NEWMAT_LIB 0

#include "include.h"

#include "myexcept.h"


#ifdef use_namespace
namespace NEWMAT { using namespace RBD_COMMON; }
namespace RBD_LIBRARIES { using namespace NEWMAT; }
namespace NEWMAT {
#endif

//#define DO_REPORT                     // to activate REPORT

#ifdef NO_LONG_NAMES
#define UpperTriangularMatrix UTMatrix
#define LowerTriangularMatrix LTMatrix
#define SymmetricMatrix SMatrix
#define DiagonalMatrix DMatrix
#define BandMatrix BMatrix
#define UpperBandMatrix UBMatrix
#define LowerBandMatrix LBMatrix
#define SymmetricBandMatrix SBMatrix
#define BandLUMatrix BLUMatrix
#endif

// ************************** general utilities ****************************/

class GeneralMatrix;

void MatrixErrorNoSpace(const void*);                 // no space handler

class LogAndSign
// Return from LogDeterminant function
//    - value of the log plus the sign (+, - or 0)
{
   Real log_val;
   int sign_val;
public:
   LogAndSign() { log_val=0.0; sign_val=1; }
   LogAndSign(Real);
   void operator*=(Real);
   void pow_eq(int k);  // raise to power of k
   void PowEq(int k) { pow_eq(k); }
   void ChangeSign() { sign_val = -sign_val; }
   void change_sign() { sign_val = -sign_val; }
   Real LogValue() const { return log_val; }
   Real log_value() const { return log_val; }
   int Sign() const { return sign_val; }
   int sign() const { return sign_val; }
   Real value() const;
   Real Value() const { return value(); }
   FREE_CHECK(LogAndSign)
};

// the following class is for counting the number of times a piece of code
// is executed. It is used for locating any code not executed by test
// routines. Use turbo GREP locate all places this code is called and
// check which ones are not accessed.
// Somewhat implementation dependent as it relies on "cout" still being
// present when ExeCounter objects are destructed.

#ifdef DO_REPORT

class ExeCounter
{
   int line;                                    // code line number
   int fileid;                                  // file identifier
   long nexe;                                   // number of executions
   static int nreports;                         // number of reports
public:
   ExeCounter(int,int);
   void operator++() { nexe++; }
   ~ExeCounter();                               // prints out reports
};

#endif


// ************************** class MatrixType *****************************/

// Is used for finding the type of a matrix resulting from the binary operations
// +, -, * and identifying what conversions are permissible.
// This class must be updated when new matrix types are added.

class GeneralMatrix;                            // defined later
class BaseMatrix;                               // defined later
class MatrixInput;                              // defined later

class MatrixType
{
public:
   enum Attribute {  Valid     = 1,
                     Diagonal  = 2,             // order of these is important
                     Symmetric = 4,
                     Band      = 8,
                     Lower     = 16,
                     Upper     = 32,
                     Square    = 64,
                     Skew      = 128,
                     LUDeco    = 256,
                     Ones      = 512 };

   enum            { US = 0,
                     UT = Valid + Upper + Square,
                     LT = Valid + Lower + Square,
                     Rt = Valid,
                     Sq = Valid + Square,
                     Sm = Valid + Symmetric + Square,
                     Sk = Valid + Skew + Square,
                     Dg = Valid + Diagonal + Band + Lower + Upper + Symmetric
                        + Square,
                     Id = Valid + Diagonal + Band + Lower + Upper + Symmetric
                        + Square + Ones,
                     RV = Valid,     //   do not separate out
                     CV = Valid,     //   vectors
                     BM = Valid + Band + Square,
                     UB = Valid + Band + Upper + Square,
                     LB = Valid + Band + Lower + Square,
                     SB = Valid + Band + Symmetric + Square,
                     KB = Valid + Band + Skew + Square,
                     Ct = Valid + LUDeco + Square,
                     BC = Valid + Band + LUDeco + Square,
                     Mask = ~Square
                   };


   static int nTypes() { return 13; }          // number of different types
                                               // exclude Ct, US, BC
public:
   int attribute;
   bool DataLossOK;                            // true if data loss is OK when
                                               // this represents a destination
public:
   MatrixType () : DataLossOK(false) {}
   MatrixType (int ii) : attribute(ii), DataLossOK(false) {}
   MatrixType (int ii, bool dlok) : attribute(ii), DataLossOK(dlok) {}
   MatrixType (const MatrixType& mt)
      : attribute(mt.attribute), DataLossOK(mt.DataLossOK) {}
   void operator=(const MatrixType& mt)
      { attribute = mt.attribute; DataLossOK = mt.DataLossOK; }
   void SetDataLossOK() { DataLossOK = true; }
   int operator+() const { return attribute; }
   MatrixType operator+(MatrixType mt) const
      { return MatrixType(attribute & mt.attribute); }
   MatrixType operator*(const MatrixType&) const;
   MatrixType SP(const MatrixType&) const;
   MatrixType KP(const MatrixType&) const;
   MatrixType operator|(const MatrixType& mt) const
      { return MatrixType(attribute & mt.attribute & Valid); }
   MatrixType operator&(const MatrixType& mt) const
      { return MatrixType(attribute & mt.attribute & Valid); }
   bool operator>=(MatrixType mt) const
      { return ( attribute & ~mt.attribute & Mask ) == 0; }
   bool operator<(MatrixType mt) const         // for MS Visual C++ 4
      { return ( attribute & ~mt.attribute & Mask ) != 0; }
   bool operator==(MatrixType tt) const
      { return (attribute == tt.attribute); }
   bool operator!=(MatrixType tt) const
      { return (attribute != tt.attribute); }
   bool operator!() const { return (attribute & Valid) == 0; }
   MatrixType i() const;                       // type of inverse
   MatrixType t() const;                       // type of transpose
   MatrixType AddEqualEl() const               // Add constant to matrix
      { return MatrixType(attribute & (Valid + Symmetric + Square)); }
   MatrixType MultRHS() const;                 // type for rhs of multiply
   MatrixType sub() const                      // type of submatrix
      { return MatrixType(attribute & Valid); }
   MatrixType ssub() const                     // type of sym submatrix
      { return MatrixType(attribute); }        // not for selection matrix
   GeneralMatrix* New() const;                 // new matrix of given type
   GeneralMatrix* New(int,int,BaseMatrix*) const;
                                               // new matrix of given type
   const char* value() const;                  // to print type
   const char* Value() const { return value(); }
   friend bool Rectangular(MatrixType a, MatrixType b, MatrixType c);
   friend bool Compare(const MatrixType&, MatrixType&);
                                               // compare and check conv.
   bool is_band() const { return (attribute & Band) != 0; }
   bool is_diagonal() const { return (attribute & Diagonal) != 0; }
   bool is_symmetric() const { return (attribute & Symmetric) != 0; }
   bool CannotConvert() const { return (attribute & LUDeco) != 0; }
                                               // used by operator== 
   FREE_CHECK(MatrixType)
};


// *********************** class MatrixBandWidth ***********************/

class MatrixBandWidth
{
public:
   int lower_val;
   int upper_val;
   MatrixBandWidth(const int l, const int u) : lower_val(l), upper_val(u) {}
   MatrixBandWidth(const int ii) : lower_val(ii), upper_val(ii) {}
   MatrixBandWidth operator+(const MatrixBandWidth&) const;
   MatrixBandWidth operator*(const MatrixBandWidth&) const;
   MatrixBandWidth minimum(const MatrixBandWidth&) const;
   MatrixBandWidth t() const { return MatrixBandWidth(upper_val,lower_val); }
   bool operator==(const MatrixBandWidth& bw) const
      { return (lower_val == bw.lower_val) && (upper_val == bw.upper_val); }
   bool operator!=(const MatrixBandWidth& bw) const { return !operator==(bw); }
   int Upper() const { return upper_val; }
   int upper() const { return upper_val; }
   int Lower() const { return lower_val; }
   int lower() const { return lower_val; }
   FREE_CHECK(MatrixBandWidth)
};


// ********************* Array length specifier ************************/

// This class is introduced to avoid constructors such as
//   ColumnVector(int)
// being used for conversions

class ArrayLengthSpecifier
{
   int v;
public:
   int Value() const { return v; }
   int value() const { return v; }
   ArrayLengthSpecifier(int l) : v(l) {}
};

// ************************* Matrix routines ***************************/


class MatrixRowCol;                             // defined later
class MatrixRow;
class MatrixCol;
class MatrixColX;

class GeneralMatrix;                            // defined later
class AddedMatrix;
class MultipliedMatrix;
class SubtractedMatrix;
class SPMatrix;
class KPMatrix;
class ConcatenatedMatrix;
class StackedMatrix;
class SolvedMatrix;
class ShiftedMatrix;
class NegShiftedMatrix;
class ScaledMatrix;
class TransposedMatrix;
class ReversedMatrix;
class NegatedMatrix;
class InvertedMatrix;
class RowedMatrix;
class ColedMatrix;
class DiagedMatrix;
class MatedMatrix;
class GetSubMatrix;
class ReturnMatrix;
class Matrix;
class SquareMatrix;
class nricMatrix;
class RowVector;
class ColumnVector;
class SymmetricMatrix;
class UpperTriangularMatrix;
class LowerTriangularMatrix;
class DiagonalMatrix;
class CroutMatrix;
class BandMatrix;
class LowerBandMatrix;
class UpperBandMatrix;
class SymmetricBandMatrix;
class LinearEquationSolver;
class GenericMatrix;


#define MatrixTypeUnSp 0
//static MatrixType MatrixTypeUnSp(MatrixType::US);
//                                              // AT&T needs this

class BaseMatrix : public Janitor               // base of all matrix classes
{
protected:
   virtual int search(const BaseMatrix*) const = 0;
                                                // count number of times matrix
                                                // is referred to

public:
   virtual GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp) = 0;
                                                // evaluate temporary
   // for old version of G++
   //   virtual GeneralMatrix* Evaluate(MatrixType mt) = 0;
   //   GeneralMatrix* Evaluate() { return Evaluate(MatrixTypeUnSp); }
   AddedMatrix operator+(const BaseMatrix&) const;    // results of operations
   MultipliedMatrix operator*(const BaseMatrix&) const;
   SubtractedMatrix operator-(const BaseMatrix&) const;
   ConcatenatedMatrix operator|(const BaseMatrix&) const;
   StackedMatrix operator&(const BaseMatrix&) const;
   ShiftedMatrix operator+(Real) const;
   ScaledMatrix operator*(Real) const;
   ScaledMatrix operator/(Real) const;
   ShiftedMatrix operator-(Real) const;
   TransposedMatrix t() const;
//   TransposedMatrix t;
   NegatedMatrix operator-() const;                   // change sign of elements
   ReversedMatrix reverse() const;
   ReversedMatrix Reverse() const;
   InvertedMatrix i() const;
//   InvertedMatrix i;
   RowedMatrix as_row() const;
   RowedMatrix AsRow() const;
   ColedMatrix as_column() const;
   ColedMatrix AsColumn() const;
   DiagedMatrix as_diagonal() const;
   DiagedMatrix AsDiagonal() const;
   MatedMatrix as_matrix(int,int) const;
   MatedMatrix AsMatrix(int m, int n) const;
   GetSubMatrix submatrix(int,int,int,int) const;
   GetSubMatrix SubMatrix(int fr, int lr, int fc, int lc) const;
   GetSubMatrix sym_submatrix(int,int) const;
   GetSubMatrix SymSubMatrix(int f, int l) const;
   GetSubMatrix row(int) const;
   GetSubMatrix rows(int,int) const;
   GetSubMatrix column(int) const;
   GetSubMatrix columns(int,int) const;
   GetSubMatrix Row(int f) const;
   GetSubMatrix Rows(int f, int l) const;
   GetSubMatrix Column(int f) const;
   GetSubMatrix Columns(int f, int l) const;
   Real as_scalar() const;                      // conversion of 1 x 1 matrix
   Real AsScalar() const;
   virtual LogAndSign log_determinant() const;
   LogAndSign LogDeterminant() const { return log_determinant(); }
   Real determinant() const;
   Real Determinant() const { return determinant(); }
   virtual Real sum_square() const;
   Real SumSquare() const { return sum_square(); }
   Real norm_Frobenius() const;
   Real norm_frobenius() const { return norm_Frobenius(); }
   Real NormFrobenius() const { return norm_Frobenius(); }
   virtual Real sum_absolute_value() const;
   Real SumAbsoluteValue() const { return sum_absolute_value(); }
   virtual Real sum() const;
   virtual Real Sum() const { return sum(); }
   virtual Real maximum_absolute_value() const;
   Real MaximumAbsoluteValue() const { return maximum_absolute_value(); }
   virtual Real maximum_absolute_value1(int& ii) const;
   Real MaximumAbsoluteValue1(int& ii) const
      { return maximum_absolute_value1(ii); }
   virtual Real maximum_absolute_value2(int& ii, int& jj) const;
   Real MaximumAbsoluteValue2(int& ii, int& jj) const
      { return maximum_absolute_value2(ii,jj); }
   virtual Real minimum_absolute_value() const;
   Real MinimumAbsoluteValue() const { return minimum_absolute_value(); }
   virtual Real minimum_absolute_value1(int& ii) const;
   Real MinimumAbsoluteValue1(int& ii) const
      { return minimum_absolute_value1(ii); }
   virtual Real minimum_absolute_value2(int& ii, int& jj) const;
   Real MinimumAbsoluteValue2(int& ii, int& jj) const
      { return minimum_absolute_value2(ii,jj); }
   virtual Real maximum() const;
   Real Maximum() const { return maximum(); }
   virtual Real maximum1(int& ii) const;
   Real Maximum1(int& ii) const { return maximum1(ii); }
   virtual Real maximum2(int& ii, int& jj) const;
   Real Maximum2(int& ii, int& jj) const { return maximum2(ii,jj); }
   virtual Real minimum() const;
   Real Minimum() const { return minimum(); }
   virtual Real minimum1(int& ii) const;
   Real Minimum1(int& ii) const { return minimum1(ii); }
   virtual Real minimum2(int& ii, int& jj) const;
   Real Minimum2(int& ii, int& jj) const { return minimum2(ii,jj); }
   virtual Real trace() const;
   Real Trace() const { return trace(); }
   Real norm1() const;
   Real Norm1() const { return norm1(); }
   Real norm_infinity() const;
   Real NormInfinity() const { return norm_infinity(); }
   virtual MatrixBandWidth bandwidth() const;  // bandwidths of band matrix
   virtual MatrixBandWidth BandWidth() const { return bandwidth(); }
   void IEQND() const;                         // called by ineq. ops
   ReturnMatrix sum_square_columns() const;
   ReturnMatrix sum_square_rows() const;
   ReturnMatrix sum_columns() const;
   ReturnMatrix sum_rows() const;
   virtual void cleanup() {}
   void CleanUp() { cleanup(); }

//   virtual ReturnMatrix Reverse() const;       // reverse order of elements
//protected:
//   BaseMatrix() : t(this), i(this) {}

   friend class GeneralMatrix;
   friend class Matrix;
   friend class SquareMatrix;
   friend class nricMatrix;
   friend class RowVector;
   friend class ColumnVector;
   friend class SymmetricMatrix;
   friend class UpperTriangularMatrix;
   friend class LowerTriangularMatrix;
   friend class DiagonalMatrix;
   friend class CroutMatrix;
   friend class BandMatrix;
   friend class LowerBandMatrix;
   friend class UpperBandMatrix;
   friend class SymmetricBandMatrix;
   friend class AddedMatrix;
   friend class MultipliedMatrix;
   friend class SubtractedMatrix;
   friend class SPMatrix;
   friend class KPMatrix;
   friend class ConcatenatedMatrix;
   friend class StackedMatrix;
   friend class SolvedMatrix;
   friend class ShiftedMatrix;
   friend class NegShiftedMatrix;
   friend class ScaledMatrix;
   friend class TransposedMatrix;
   friend class ReversedMatrix;
   friend class NegatedMatrix;
   friend class InvertedMatrix;
   friend class RowedMatrix;
   friend class ColedMatrix;
   friend class DiagedMatrix;
   friend class MatedMatrix;
   friend class GetSubMatrix;
   friend class ReturnMatrix;
   friend class LinearEquationSolver;
   friend class GenericMatrix;
   NEW_DELETE(BaseMatrix)
};


// ***************************** working classes **************************/

class GeneralMatrix : public BaseMatrix         // declarable matrix types
{
   virtual GeneralMatrix* Image() const;        // copy of matrix
protected:
   int tag_val;                               // shows whether can reuse
   int nrows_val, ncols_val;                    // dimensions
   int storage;                                 // total store required
   Real* store;                                 // point to store (0=not set)
   GeneralMatrix();                             // initialise with no store
   GeneralMatrix(ArrayLengthSpecifier);         // constructor getting store
   void Add(GeneralMatrix*, Real);              // sum of GM and Real
   void Add(Real);                              // add Real to this
   void NegAdd(GeneralMatrix*, Real);           // Real - GM
   void NegAdd(Real);                           // this = this - Real
   void Multiply(GeneralMatrix*, Real);         // product of GM and Real
   void Multiply(Real);                         // multiply this by Real
   void Negate(GeneralMatrix*);                 // change sign
   void Negate();                               // change sign
   void ReverseElements();                      // internal reverse of elements
   void ReverseElements(GeneralMatrix*);        // reverse order of elements
   void operator=(Real);                        // set matrix to constant
   Real* GetStore();                            // get store or copy
   GeneralMatrix* BorrowStore(GeneralMatrix*, MatrixType);
                                                // temporarily access store
   void GetMatrix(const GeneralMatrix*);        // used by = and initialise
   void Eq(const BaseMatrix&, MatrixType);      // used by =
   void Eq(const GeneralMatrix&);               // version with no conversion
   void Eq(const BaseMatrix&, MatrixType, bool);// used by <<
   void Eq2(const BaseMatrix&, MatrixType);     // cut down version of Eq
   int search(const BaseMatrix*) const;
   virtual GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void CheckConversion(const BaseMatrix&);     // check conversion OK
   void resize(int, int, int);                  // change dimensions
   virtual short SimpleAddOK(const GeneralMatrix* /*gm*/) { return 0; }
             // see bandmat.cpp for explanation
   virtual void MiniCleanUp()
      { store = 0; storage = 0; nrows_val = 0; ncols_val = 0; tag_val = -1;}
             // CleanUp when the data array has already been deleted
   void PlusEqual(const GeneralMatrix& gm);
   void MinusEqual(const GeneralMatrix& gm);
   void PlusEqual(Real f);
   void MinusEqual(Real f);
   void swap(GeneralMatrix& gm);                // swap values
public:
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   virtual MatrixType type() const = 0;         // type of a matrix
   MatrixType Type() const { return type(); }
   int Nrows() const { return nrows_val; }      // get dimensions
   int Ncols() const { return ncols_val; }
   int Storage() const { return storage; }
   Real* Store() const { return store; }
   // updated names
   int nrows() const { return nrows_val; }      // get dimensions
   int ncols() const { return ncols_val; }
   int size() const { return storage; }
   Real* data() { return store; }
   const Real* data() const { return store; }
   const Real* const_data() const { return store; }
   virtual ~GeneralMatrix();                    // delete store if set
   void tDelete();                              // delete if tag_val permits
   bool reuse();                                // true if tag_val allows reuse
   void protect() { tag_val=-1; }               // cannot delete or reuse
   void Protect() { tag_val=-1; }               // cannot delete or reuse
   int tag() const { return tag_val; }
   int Tag() const { return tag_val; }
   bool is_zero() const;                        // test matrix has all zeros
   bool IsZero() const { return is_zero(); }    // test matrix has all zeros
   void Release() { tag_val=1; }                // del store after next use
   void Release(int tt) { tag_val=tt; }           // del store after tt accesses
   void ReleaseAndDelete() { tag_val=0; }       // delete matrix after use
   void release() { tag_val=1; }                // del store after next use
   void release(int tt) { tag_val=tt; }           // del store after t accesses
   void release_and_delete() { tag_val=0; }     // delete matrix after use
   void operator<<(const double*);              // assignment from an array
   void operator<<(const float*);               // assignment from an array
   void operator<<(const int*);                 // assignment from an array
   void operator<<(const BaseMatrix& X)
      { Eq(X,this->type(),true); }              // = without checking type
   void inject(const GeneralMatrix&);           // copy stored els only
   void Inject(const GeneralMatrix& GM) { inject(GM); }
   void operator+=(const BaseMatrix&);
   void operator-=(const BaseMatrix&);
   void operator*=(const BaseMatrix&);
   void operator|=(const BaseMatrix&);
   void operator&=(const BaseMatrix&);
   void operator+=(Real);
   void operator-=(Real r) { operator+=(-r); }
   void operator*=(Real);
   void operator/=(Real r) { operator*=(1.0/r); }
   virtual GeneralMatrix* MakeSolver();         // for solving
   virtual void Solver(MatrixColX&, const MatrixColX&) {}
   virtual void GetRow(MatrixRowCol&) = 0;      // Get matrix row
   virtual void RestoreRow(MatrixRowCol&) {}    // Restore matrix row
   virtual void NextRow(MatrixRowCol&);         // Go to next row
   virtual void GetCol(MatrixRowCol&) = 0;      // Get matrix col
   virtual void GetCol(MatrixColX&) = 0;        // Get matrix col
   virtual void RestoreCol(MatrixRowCol&) {}    // Restore matrix col
   virtual void RestoreCol(MatrixColX&) {}      // Restore matrix col
   virtual void NextCol(MatrixRowCol&);         // Go to next col
   virtual void NextCol(MatrixColX&);           // Go to next col
   Real sum_square() const;
   Real sum_absolute_value() const;
   Real sum() const;
   Real maximum_absolute_value1(int& ii) const;
   Real minimum_absolute_value1(int& ii) const;
   Real maximum1(int& ii) const;
   Real minimum1(int& ii) const;
   Real maximum_absolute_value() const;
   Real maximum_absolute_value2(int& ii, int& jj) const;
   Real minimum_absolute_value() const;
   Real minimum_absolute_value2(int& ii, int& jj) const;
   Real maximum() const;
   Real maximum2(int& ii, int& jj) const;
   Real minimum() const;
   Real minimum2(int& ii, int& jj) const;
   LogAndSign log_determinant() const;
   virtual bool IsEqual(const GeneralMatrix&) const;
                                                // same type, same values
   void CheckStore() const;                     // check store is non-zero
   virtual void SetParameters(const GeneralMatrix*) {}
                                                // set parameters in GetMatrix
   operator ReturnMatrix() const;               // for building a ReturnMatrix
   ReturnMatrix for_return() const;
   ReturnMatrix ForReturn() const;
   //virtual bool SameStorageType(const GeneralMatrix& A) const;
   //virtual void ReSizeForAdd(const GeneralMatrix& A, const GeneralMatrix& B);
   //virtual void ReSizeForSP(const GeneralMatrix& A, const GeneralMatrix& B);
   virtual void resize(const GeneralMatrix& A);
   virtual void ReSize(const GeneralMatrix& A) { resize(A); }
   MatrixInput operator<<(double);                // for loading a list
   MatrixInput operator<<(float);                // for loading a list
   MatrixInput operator<<(int f);
//   ReturnMatrix Reverse() const;                // reverse order of elements
   void cleanup();                              // to clear store

   friend class Matrix;
   friend class SquareMatrix;
   friend class nricMatrix;
   friend class SymmetricMatrix;
   friend class UpperTriangularMatrix;
   friend class LowerTriangularMatrix;
   friend class DiagonalMatrix;
   friend class CroutMatrix;
   friend class RowVector;
   friend class ColumnVector;
   friend class BandMatrix;
   friend class LowerBandMatrix;
   friend class UpperBandMatrix;
   friend class SymmetricBandMatrix;
   friend class BaseMatrix;
   friend class AddedMatrix;
   friend class MultipliedMatrix;
   friend class SubtractedMatrix;
   friend class SPMatrix;
   friend class KPMatrix;
   friend class ConcatenatedMatrix;
   friend class StackedMatrix;
   friend class SolvedMatrix;
   friend class ShiftedMatrix;
   friend class NegShiftedMatrix;
   friend class ScaledMatrix;
   friend class TransposedMatrix;
   friend class ReversedMatrix;
   friend class NegatedMatrix;
   friend class InvertedMatrix;
   friend class RowedMatrix;
   friend class ColedMatrix;
   friend class DiagedMatrix;
   friend class MatedMatrix;
   friend class GetSubMatrix;
   friend class ReturnMatrix;
   friend class LinearEquationSolver;
   friend class GenericMatrix;
   NEW_DELETE(GeneralMatrix)
};



class Matrix : public GeneralMatrix             // usual rectangular matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   Matrix() {}
   ~Matrix() {}
   Matrix(int, int);                            // standard declaration
   Matrix(const BaseMatrix&);                   // evaluate BaseMatrix
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const Matrix& m) { Eq(m); }
   MatrixType type() const;
   Real& operator()(int, int);                  // access element
   Real& element(int, int);                     // access element
   Real operator()(int, int) const;            // access element
   Real element(int, int) const;               // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+m*ncols_val; }
   const Real* operator[](int m) const { return store+m*ncols_val; }
   // following for Numerical Recipes in C++
   Matrix(Real, int, int);
   Matrix(const Real*, int, int);
#endif
   Matrix(const Matrix& gm) { GetMatrix(&gm); }
   GeneralMatrix* MakeSolver();
   Real trace() const;
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&);
   void RestoreCol(MatrixColX&);
   void NextRow(MatrixRowCol&);
   void NextCol(MatrixRowCol&);
   void NextCol(MatrixColX&);
   virtual void resize(int,int);           // change dimensions
      // virtual so we will catch it being used in a vector called as a matrix
   virtual void resize_keep(int,int);
   virtual void ReSize(int m, int n) { resize(m, n); }
   void resize(const GeneralMatrix& A);
   void ReSize(const GeneralMatrix& A) { resize(A); }
   Real maximum_absolute_value2(int& ii, int& jj) const;
   Real minimum_absolute_value2(int& ii, int& jj) const;
   Real maximum2(int& ii, int& jj) const;
   Real minimum2(int& ii, int& jj) const;
   void operator+=(const Matrix& M) { PlusEqual(M); }
   void operator-=(const Matrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::Add(f); }
   void operator-=(Real f) { GeneralMatrix::Add(-f); }
   void swap(Matrix& gm) { GeneralMatrix::swap((GeneralMatrix&)gm); }
   friend Real dotproduct(const Matrix& A, const Matrix& B);
   NEW_DELETE(Matrix)
};

class SquareMatrix : public Matrix              // square matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   SquareMatrix() {}
   ~SquareMatrix() {}
   SquareMatrix(ArrayLengthSpecifier);          // standard declaration
   SquareMatrix(const BaseMatrix&);             // evaluate BaseMatrix
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const SquareMatrix& m) { Eq(m); }
   void operator=(const Matrix& m);
   MatrixType type() const;
   SquareMatrix(const SquareMatrix& gm) { GetMatrix(&gm); }
   SquareMatrix(const Matrix& gm);
   void resize(int);                            // change dimensions
   void ReSize(int m) { resize(m); }
   void resize_keep(int);
   void resize_keep(int,int);
   void resize(int,int);                        // change dimensions
   void ReSize(int m, int n) { resize(m, n); }
   void resize(const GeneralMatrix& A);
   void ReSize(const GeneralMatrix& A) { resize(A); }
   void operator+=(const Matrix& M) { PlusEqual(M); }
   void operator-=(const Matrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::Add(f); }
   void operator-=(Real f) { GeneralMatrix::Add(-f); }
   void swap(SquareMatrix& gm) { GeneralMatrix::swap((GeneralMatrix&)gm); }
   NEW_DELETE(SquareMatrix)
};

class nricMatrix : public Matrix                // for use with Numerical
                                                // Recipes in C
{
   GeneralMatrix* Image() const;                // copy of matrix
   Real** row_pointer;                          // points to rows
   void MakeRowPointer();                       // build rowpointer
   void DeleteRowPointer();
public:
   nricMatrix() {}
   nricMatrix(int m, int n)                     // standard declaration
      :  Matrix(m,n) { MakeRowPointer(); }
   nricMatrix(const BaseMatrix& bm)             // evaluate BaseMatrix
      :  Matrix(bm) { MakeRowPointer(); }
   void operator=(const BaseMatrix& bm)
      { DeleteRowPointer(); Matrix::operator=(bm); MakeRowPointer(); }
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const nricMatrix& m)
      { DeleteRowPointer(); Eq(m); MakeRowPointer(); }
   void operator<<(const BaseMatrix& X)
      { DeleteRowPointer(); Eq(X,this->type(),true); MakeRowPointer(); }
   nricMatrix(const nricMatrix& gm) { GetMatrix(&gm); MakeRowPointer(); }
   void resize(int m, int n)               // change dimensions
      { DeleteRowPointer(); Matrix::resize(m,n); MakeRowPointer(); }
   void resize_keep(int m, int n)               // change dimensions
      { DeleteRowPointer(); Matrix::resize_keep(m,n); MakeRowPointer(); }
   void ReSize(int m, int n)               // change dimensions
      { DeleteRowPointer(); Matrix::resize(m,n); MakeRowPointer(); }
   void resize(const GeneralMatrix& A);
   void ReSize(const GeneralMatrix& A) { resize(A); }
   ~nricMatrix() { DeleteRowPointer(); }
   Real** nric() const { CheckStore(); return row_pointer-1; }
   void cleanup();                                // to clear store
   void MiniCleanUp();
   void operator+=(const Matrix& M) { PlusEqual(M); }
   void operator-=(const Matrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::Add(f); }
   void operator-=(Real f) { GeneralMatrix::Add(-f); }
   void swap(nricMatrix& gm);
   NEW_DELETE(nricMatrix)
};

class SymmetricMatrix : public GeneralMatrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   SymmetricMatrix() {}
   ~SymmetricMatrix() {}
   SymmetricMatrix(ArrayLengthSpecifier);
   SymmetricMatrix(const BaseMatrix&);
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const SymmetricMatrix& m) { Eq(m); }
   Real& operator()(int, int);                  // access element
   Real& element(int, int);                     // access element
   Real operator()(int, int) const;             // access element
   Real element(int, int) const;                // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+(m*(m+1))/2; }
   const Real* operator[](int m) const { return store+(m*(m+1))/2; }
#endif
   MatrixType type() const;
   SymmetricMatrix(const SymmetricMatrix& gm) { GetMatrix(&gm); }
   Real sum_square() const;
   Real sum_absolute_value() const;
   Real sum() const;
   Real trace() const;
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&) {}
   void RestoreCol(MatrixColX&);
   GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void resize(int);                           // change dimensions
   void ReSize(int m) { resize(m); }
   void resize_keep(int);
   void resize(const GeneralMatrix& A);
   void ReSize(const GeneralMatrix& A) { resize(A); }
   void operator+=(const SymmetricMatrix& M) { PlusEqual(M); }
   void operator-=(const SymmetricMatrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::Add(f); }
   void operator-=(Real f) { GeneralMatrix::Add(-f); }
   void swap(SymmetricMatrix& gm) { GeneralMatrix::swap((GeneralMatrix&)gm); }
   NEW_DELETE(SymmetricMatrix)
};

class UpperTriangularMatrix : public GeneralMatrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   UpperTriangularMatrix() {}
   ~UpperTriangularMatrix() {}
   UpperTriangularMatrix(ArrayLengthSpecifier);
   void operator=(const BaseMatrix&);
   void operator=(const UpperTriangularMatrix& m) { Eq(m); }
   UpperTriangularMatrix(const BaseMatrix&);
   UpperTriangularMatrix(const UpperTriangularMatrix& gm) { GetMatrix(&gm); }
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   Real& operator()(int, int);                  // access element
   Real& element(int, int);                     // access element
   Real operator()(int, int) const;             // access element
   Real element(int, int) const;                // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+m*ncols_val-(m*(m+1))/2; }
   const Real* operator[](int m) const
      { return store+m*ncols_val-(m*(m+1))/2; }
#endif
   MatrixType type() const;
   GeneralMatrix* MakeSolver() { return this; } // for solving
   void Solver(MatrixColX&, const MatrixColX&);
   LogAndSign log_determinant() const;
   Real trace() const;
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&);
   void RestoreCol(MatrixColX& c) { RestoreCol((MatrixRowCol&)c); }
   void NextRow(MatrixRowCol&);
   void resize(int);                       // change dimensions
   void ReSize(int m) { resize(m); }
   void resize(const GeneralMatrix& A);
   void ReSize(const GeneralMatrix& A) { resize(A); }
   void resize_keep(int);
   MatrixBandWidth bandwidth() const;
   void operator+=(const UpperTriangularMatrix& M) { PlusEqual(M); }
   void operator-=(const UpperTriangularMatrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::operator+=(f); }
   void operator-=(Real f) { GeneralMatrix::operator-=(f); }
   void swap(UpperTriangularMatrix& gm)
      { GeneralMatrix::swap((GeneralMatrix&)gm); }
   NEW_DELETE(UpperTriangularMatrix)
};

class LowerTriangularMatrix : public GeneralMatrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   LowerTriangularMatrix() {}
   ~LowerTriangularMatrix() {}
   LowerTriangularMatrix(ArrayLengthSpecifier);
   LowerTriangularMatrix(const LowerTriangularMatrix& gm) { GetMatrix(&gm); }
   LowerTriangularMatrix(const BaseMatrix& M);
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const LowerTriangularMatrix& m) { Eq(m); }
   Real& operator()(int, int);                  // access element
   Real& element(int, int);                     // access element
   Real operator()(int, int) const;             // access element
   Real element(int, int) const;                // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+(m*(m+1))/2; }
   const Real* operator[](int m) const { return store+(m*(m+1))/2; }
#endif
   MatrixType type() const;
   GeneralMatrix* MakeSolver() { return this; } // for solving
   void Solver(MatrixColX&, const MatrixColX&);
   LogAndSign log_determinant() const;
   Real trace() const;
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&);
   void RestoreCol(MatrixColX& c) { RestoreCol((MatrixRowCol&)c); }
   void NextRow(MatrixRowCol&);
   void resize(int);                       // change dimensions
   void ReSize(int m) { resize(m); }
   void resize_keep(int);
   void resize(const GeneralMatrix& A);
   void ReSize(const GeneralMatrix& A) { resize(A); }
   MatrixBandWidth bandwidth() const;
   void operator+=(const LowerTriangularMatrix& M) { PlusEqual(M); }
   void operator-=(const LowerTriangularMatrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::operator+=(f); }
   void operator-=(Real f) { GeneralMatrix::operator-=(f); }
   void swap(LowerTriangularMatrix& gm)
      { GeneralMatrix::swap((GeneralMatrix&)gm); }
   NEW_DELETE(LowerTriangularMatrix)
};

class DiagonalMatrix : public GeneralMatrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   DiagonalMatrix() {}
   ~DiagonalMatrix() {}
   DiagonalMatrix(ArrayLengthSpecifier);
   DiagonalMatrix(const BaseMatrix&);
   DiagonalMatrix(const DiagonalMatrix& gm) { GetMatrix(&gm); }
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const DiagonalMatrix& m) { Eq(m); }
   Real& operator()(int, int);                  // access element
   Real& operator()(int);                       // access element
   Real operator()(int, int) const;             // access element
   Real operator()(int) const;
   Real& element(int, int);                     // access element
   Real& element(int);                          // access element
   Real element(int, int) const;                // access element
   Real element(int) const;                     // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real& operator[](int m) { return store[m]; }
   const Real& operator[](int m) const { return store[m]; }
#endif
   MatrixType type() const;

   LogAndSign log_determinant() const;
   Real trace() const;
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void NextRow(MatrixRowCol&);
   void NextCol(MatrixRowCol&);
   void NextCol(MatrixColX&);
   GeneralMatrix* MakeSolver() { return this; } // for solving
   void Solver(MatrixColX&, const MatrixColX&);
   GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void resize(int);                       // change dimensions
   void ReSize(int m) { resize(m); }
   void resize_keep(int);
   void resize(const GeneralMatrix& A);
   void ReSize(const GeneralMatrix& A) { resize(A); }
   Real* nric() const
      { CheckStore(); return store-1; }         // for use by NRIC
   MatrixBandWidth bandwidth() const;
//   ReturnMatrix Reverse() const;                // reverse order of elements
   void operator+=(const DiagonalMatrix& M) { PlusEqual(M); }
   void operator-=(const DiagonalMatrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::operator+=(f); }
   void operator-=(Real f) { GeneralMatrix::operator-=(f); }
   void swap(DiagonalMatrix& gm)
      { GeneralMatrix::swap((GeneralMatrix&)gm); }
   NEW_DELETE(DiagonalMatrix)
};

class RowVector : public Matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   RowVector() { nrows_val = 1; }
   ~RowVector() {}
   RowVector(ArrayLengthSpecifier n) : Matrix(1,n.Value()) {}
   RowVector(const BaseMatrix&);
   RowVector(const RowVector& gm) { GetMatrix(&gm); }
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const RowVector& m) { Eq(m); }
   Real& operator()(int);                       // access element
   Real& element(int);                          // access element
   Real operator()(int) const;                  // access element
   Real element(int) const;                     // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real& operator[](int m) { return store[m]; }
   const Real& operator[](int m) const { return store[m]; }
   // following for Numerical Recipes in C++
   RowVector(Real a, int n) : Matrix(a, 1, n) {}
   RowVector(const Real* a, int n) : Matrix(a, 1, n) {}
#endif
   MatrixType type() const;
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void NextCol(MatrixRowCol&);
   void NextCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&) {}
   void RestoreCol(MatrixColX& c);
   GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void resize(int);                       // change dimensions
   void ReSize(int m) { resize(m); }
   void resize_keep(int);
   void resize_keep(int,int);
   void resize(int,int);                   // in case access is matrix
   void ReSize(int m,int n) { resize(m, n); }
   void resize(const GeneralMatrix& A);
   void ReSize(const GeneralMatrix& A) { resize(A); }
   Real* nric() const
      { CheckStore(); return store-1; }         // for use by NRIC
   void cleanup();                              // to clear store
   void MiniCleanUp()
      { store = 0; storage = 0; nrows_val = 1; ncols_val = 0; tag_val = -1; }
   // friend ReturnMatrix GetMatrixRow(Matrix& A, int row);
   void operator+=(const Matrix& M) { PlusEqual(M); }
   void operator-=(const Matrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::Add(f); }
   void operator-=(Real f) { GeneralMatrix::Add(-f); }
   void swap(RowVector& gm)
      { GeneralMatrix::swap((GeneralMatrix&)gm); }
   NEW_DELETE(RowVector)
};

class ColumnVector : public Matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   ColumnVector() { ncols_val = 1; }
   ~ColumnVector() {}
   ColumnVector(ArrayLengthSpecifier n) : Matrix(n.Value(),1) {}
   ColumnVector(const BaseMatrix&);
   ColumnVector(const ColumnVector& gm) { GetMatrix(&gm); }
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const ColumnVector& m) { Eq(m); }
   Real& operator()(int);                       // access element
   Real& element(int);                          // access element
   Real operator()(int) const;                  // access element
   Real element(int) const;                     // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real& operator[](int m) { return store[m]; }
   const Real& operator[](int m) const { return store[m]; }
   // following for Numerical Recipes in C++
   ColumnVector(Real a, int m) : Matrix(a, m, 1) {}
   ColumnVector(const Real* a, int m) : Matrix(a, m, 1) {}
#endif
   MatrixType type() const;
   GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void resize(int);                       // change dimensions
   void ReSize(int m) { resize(m); }
   void resize_keep(int);
   void resize_keep(int,int);
   void resize(int,int);                   // in case access is matrix
   void ReSize(int m,int n) { resize(m, n); }
   void resize(const GeneralMatrix& A);
   void ReSize(const GeneralMatrix& A) { resize(A); }
   Real* nric() const
      { CheckStore(); return store-1; }         // for use by NRIC
   void cleanup();                              // to clear store
   void MiniCleanUp()
      { store = 0; storage = 0; nrows_val = 0; ncols_val = 1; tag_val = -1; }
//   ReturnMatrix Reverse() const;                // reverse order of elements
   void operator+=(const Matrix& M) { PlusEqual(M); }
   void operator-=(const Matrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::Add(f); }
   void operator-=(Real f) { GeneralMatrix::Add(-f); }
   void swap(ColumnVector& gm)
      { GeneralMatrix::swap((GeneralMatrix&)gm); }
   NEW_DELETE(ColumnVector)
};

class CroutMatrix : public GeneralMatrix        // for LU decomposition
{
   int* indx;
   bool d;                              // number of row swaps = even or odd
   bool sing;
   void ludcmp();
   void get_aux(CroutMatrix&);                  // for copying indx[] etc
   GeneralMatrix* Image() const;                // copy of matrix
public:
   CroutMatrix(const BaseMatrix&);
   CroutMatrix() : indx(0), d(true), sing(true) {}
   CroutMatrix(const CroutMatrix&);
   void operator=(const CroutMatrix&);
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixType type() const;
   void lubksb(Real*, int=0);
   ~CroutMatrix();
   GeneralMatrix* MakeSolver() { return this; } // for solving
   LogAndSign log_determinant() const;
   void Solver(MatrixColX&, const MatrixColX&);
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX& c) { GetCol((MatrixRowCol&)c); }
   void cleanup();                                // to clear store
   void MiniCleanUp();
   bool IsEqual(const GeneralMatrix&) const;
   bool is_singular() const { return sing; }
   bool IsSingular() const { return sing; }
   const int* const_data_indx() const { return indx; }
   bool even_exchanges() const { return d; }
   void swap(CroutMatrix& gm);
   NEW_DELETE(CroutMatrix)
};

// ***************************** band matrices ***************************/

class BandMatrix : public GeneralMatrix         // band matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
protected:
   void CornerClear() const;                    // set unused elements to zero
   short SimpleAddOK(const GeneralMatrix* gm);
public:
   int lower_val, upper_val;                            // band widths
   BandMatrix() { lower_val=0; upper_val=0; CornerClear(); }
   ~BandMatrix() {}
   BandMatrix(int n,int lb,int ub) { resize(n,lb,ub); CornerClear(); }
                                                // standard declaration
   BandMatrix(const BaseMatrix&);               // evaluate BaseMatrix
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const BandMatrix& m) { Eq(m); }
   MatrixType type() const;
   Real& operator()(int, int);                  // access element
   Real& element(int, int);                     // access element
   Real operator()(int, int) const;             // access element
   Real element(int, int) const;                // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+(upper_val+lower_val)*m+lower_val; }
   const Real* operator[](int m) const
      { return store+(upper_val+lower_val)*m+lower_val; }
#endif
   BandMatrix(const BandMatrix& gm) { GetMatrix(&gm); }
   LogAndSign log_determinant() const;
   GeneralMatrix* MakeSolver();
   Real trace() const;
   Real sum_square() const
      { CornerClear(); return GeneralMatrix::sum_square(); }
   Real sum_absolute_value() const
      { CornerClear(); return GeneralMatrix::sum_absolute_value(); }
   Real sum() const
      { CornerClear(); return GeneralMatrix::sum(); }
   Real maximum_absolute_value() const
      { CornerClear(); return GeneralMatrix::maximum_absolute_value(); }
   Real minimum_absolute_value() const
      { int ii, jj; return GeneralMatrix::minimum_absolute_value2(ii, jj); }  // FIXME: This can't possibly work right... ii and jj are not initialized.
   Real maximum() const { int ii, jj; return GeneralMatrix::maximum2(ii, jj); }
   Real minimum() const { int ii, jj; return GeneralMatrix::minimum2(ii, jj); }
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&);
   void RestoreCol(MatrixColX& c) { RestoreCol((MatrixRowCol&)c); }
   void NextRow(MatrixRowCol&);
   virtual void resize(int, int, int);             // change dimensions
   virtual void ReSize(int m, int n, int b) { resize(m, n, b); }
   void resize(const GeneralMatrix& A);
   void ReSize(const GeneralMatrix& A) { resize(A); }
   //bool SameStorageType(const GeneralMatrix& A) const;
   //void ReSizeForAdd(const GeneralMatrix& A, const GeneralMatrix& B);
   //void ReSizeForSP(const GeneralMatrix& A, const GeneralMatrix& B);
   MatrixBandWidth bandwidth() const;
   void SetParameters(const GeneralMatrix*);
   MatrixInput operator<<(double);                // will give error
   MatrixInput operator<<(float);                // will give error
   MatrixInput operator<<(int f);
   void operator<<(const double* r);              // will give error
   void operator<<(const float* r);              // will give error
   void operator<<(const int* r);               // will give error
      // the next is included because Zortech and Borland
      // cannot find the copy in GeneralMatrix
   void operator<<(const BaseMatrix& X) { GeneralMatrix::operator<<(X); }
   void swap(BandMatrix& gm);
   NEW_DELETE(BandMatrix)
};

class UpperBandMatrix : public BandMatrix       // upper band matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   UpperBandMatrix() {}
   ~UpperBandMatrix() {}
   UpperBandMatrix(int n, int ubw)              // standard declaration
      : BandMatrix(n, 0, ubw) {}
   UpperBandMatrix(const BaseMatrix&);          // evaluate BaseMatrix
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const UpperBandMatrix& m) { Eq(m); }
   MatrixType type() const;
   UpperBandMatrix(const UpperBandMatrix& gm) { GetMatrix(&gm); }
   GeneralMatrix* MakeSolver() { return this; }
   void Solver(MatrixColX&, const MatrixColX&);
   LogAndSign log_determinant() const;
   void resize(int, int, int);             // change dimensions
   void ReSize(int m, int n, int b) { resize(m, n, b); }
   void resize(int n,int ubw)              // change dimensions
      { BandMatrix::resize(n,0,ubw); }
   void ReSize(int n,int ubw)              // change dimensions
      { BandMatrix::resize(n,0,ubw); }
   void resize(const GeneralMatrix& A) { BandMatrix::resize(A); }
   void ReSize(const GeneralMatrix& A) { BandMatrix::resize(A); }
   Real& operator()(int, int);
   Real operator()(int, int) const;
   Real& element(int, int);
   Real element(int, int) const;
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+upper_val*m; }
   const Real* operator[](int m) const { return store+upper_val*m; }
#endif
   void swap(UpperBandMatrix& gm)
      { BandMatrix::swap((BandMatrix&)gm); }
   NEW_DELETE(UpperBandMatrix)
};

class LowerBandMatrix : public BandMatrix       // upper band matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   LowerBandMatrix() {}
   ~LowerBandMatrix() {}
   LowerBandMatrix(int n, int lbw)              // standard declaration
      : BandMatrix(n, lbw, 0) {}
   LowerBandMatrix(const BaseMatrix&);          // evaluate BaseMatrix
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const LowerBandMatrix& m) { Eq(m); }
   MatrixType type() const;
   LowerBandMatrix(const LowerBandMatrix& gm) { GetMatrix(&gm); }
   GeneralMatrix* MakeSolver() { return this; }
   void Solver(MatrixColX&, const MatrixColX&);
   LogAndSign log_determinant() const;
   void resize(int, int, int);             // change dimensions
   void ReSize(int m, int n, int b) { resize(m, n, b); }
   void resize(int n,int lbw)             // change dimensions
      { BandMatrix::resize(n,lbw,0); }
   void ReSize(int n,int lbw)             // change dimensions
      { BandMatrix::resize(n,lbw,0); }
   void resize(const GeneralMatrix& A) { BandMatrix::resize(A); }
   void ReSize(const GeneralMatrix& A) { BandMatrix::resize(A); }
   Real& operator()(int, int);
   Real operator()(int, int) const;
   Real& element(int, int);
   Real element(int, int) const;
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+lower_val*(m+1); }
   const Real* operator[](int m) const { return store+lower_val*(m+1); }
#endif
   void swap(LowerBandMatrix& gm)
      { BandMatrix::swap((BandMatrix&)gm); }
   NEW_DELETE(LowerBandMatrix)
};

class SymmetricBandMatrix : public GeneralMatrix
{
   GeneralMatrix* Image() const;                // copy of matrix
   void CornerClear() const;                    // set unused elements to zero
   short SimpleAddOK(const GeneralMatrix* gm);
public:
   int lower_val;                                   // lower band width
   SymmetricBandMatrix() { lower_val=0; CornerClear(); }
   ~SymmetricBandMatrix() {}
   SymmetricBandMatrix(int n, int lb) { resize(n,lb); CornerClear(); }
   SymmetricBandMatrix(const BaseMatrix&);
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const SymmetricBandMatrix& m) { Eq(m); }
   Real& operator()(int, int);                  // access element
   Real& element(int, int);                     // access element
   Real operator()(int, int) const;             // access element
   Real element(int, int) const;                // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+lower_val*(m+1); }
   const Real* operator[](int m) const { return store+lower_val*(m+1); }
#endif
   MatrixType type() const;
   SymmetricBandMatrix(const SymmetricBandMatrix& gm) { GetMatrix(&gm); }
   GeneralMatrix* MakeSolver();
   Real sum_square() const;
   Real sum_absolute_value() const;
   Real sum() const;
   Real maximum_absolute_value() const
      { CornerClear(); return GeneralMatrix::maximum_absolute_value(); }
   Real minimum_absolute_value() const
      { int ii, jj; return GeneralMatrix::minimum_absolute_value2(ii, jj); }
   Real maximum() const { int ii, jj; return GeneralMatrix::maximum2(ii, jj); }
   Real minimum() const { int ii, jj; return GeneralMatrix::minimum2(ii, jj); }
   Real trace() const;
   LogAndSign log_determinant() const;
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&) {}
   void RestoreCol(MatrixColX&);
   GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void resize(int,int);                       // change dimensions
   void ReSize(int m,int b) { resize(m, b); }
   void resize(const GeneralMatrix& A);
   void ReSize(const GeneralMatrix& A) { resize(A); }
   //bool SameStorageType(const GeneralMatrix& A) const;
   //void ReSizeForAdd(const GeneralMatrix& A, const GeneralMatrix& B);
   //void ReSizeForSP(const GeneralMatrix& A, const GeneralMatrix& B);
   MatrixBandWidth bandwidth() const;
   void SetParameters(const GeneralMatrix*);
   void operator<<(const double* r);              // will give error
   void operator<<(const float* r);              // will give error
   void operator<<(const int* r);               // will give error
   void operator<<(const BaseMatrix& X) { GeneralMatrix::operator<<(X); }
   void swap(SymmetricBandMatrix& gm);
   NEW_DELETE(SymmetricBandMatrix)
};

class BandLUMatrix : public GeneralMatrix
// for LU decomposition of band matrix
{
   int* indx;
   bool d;
   bool sing;                                   // true if singular
   Real* store2;
   int storage2;
   int m1,m2;                                   // lower and upper
   void ludcmp();
   void get_aux(BandLUMatrix&);                 // for copying indx[] etc
   GeneralMatrix* Image() const;                // copy of matrix
public:
   BandLUMatrix(const BaseMatrix&);
   BandLUMatrix()
     : indx(0), d(true), sing(true), store2(0), storage2(0), m1(0), m2(0) {}
   BandLUMatrix(const BandLUMatrix&);
   void operator=(const BandLUMatrix&);
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixType type() const;
   void lubksb(Real*, int=0);
   ~BandLUMatrix();
   GeneralMatrix* MakeSolver() { return this; } // for solving
   LogAndSign log_determinant() const;
   void Solver(MatrixColX&, const MatrixColX&);
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX& c) { GetCol((MatrixRowCol&)c); }
   void cleanup();                                // to clear store
   void MiniCleanUp();
   bool IsEqual(const GeneralMatrix&) const;
   bool is_singular() const { return sing; }
   bool IsSingular() const { return sing; }
   const Real* const_data2() const { return store2; }
   int size2() const { return storage2; }
   const int* const_data_indx() const { return indx; }
   bool even_exchanges() const { return d; }
   MatrixBandWidth bandwidth() const;
   void swap(BandLUMatrix& gm);
   NEW_DELETE(BandLUMatrix)
};

// ************************** special matrices ****************************

class IdentityMatrix : public GeneralMatrix
{
   GeneralMatrix* Image() const;          // copy of matrix
public:
   IdentityMatrix() {}
   ~IdentityMatrix() {}
   IdentityMatrix(ArrayLengthSpecifier n) : GeneralMatrix(1)
      { nrows_val = ncols_val = n.Value(); *store = 1; }
   IdentityMatrix(const IdentityMatrix& gm) { GetMatrix(&gm); }
   IdentityMatrix(const BaseMatrix&);
   void operator=(const BaseMatrix&);
   void operator=(const IdentityMatrix& m) { Eq(m); }
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   MatrixType type() const;

   LogAndSign log_determinant() const;
   Real trace() const;
   Real sum_square() const;
   Real sum_absolute_value() const;
   Real sum() const { return trace(); }
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void NextRow(MatrixRowCol&);
   void NextCol(MatrixRowCol&);
   void NextCol(MatrixColX&);
   GeneralMatrix* MakeSolver() { return this; } // for solving
   void Solver(MatrixColX&, const MatrixColX&);
   GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void resize(int n);
   void ReSize(int n) { resize(n); }
   void resize(const GeneralMatrix& A);
   void ReSize(const GeneralMatrix& A) { resize(A); }
   MatrixBandWidth bandwidth() const;
//   ReturnMatrix Reverse() const;                // reverse order of elements
   void swap(IdentityMatrix& gm)
      { GeneralMatrix::swap((GeneralMatrix&)gm); }
   NEW_DELETE(IdentityMatrix)
};




// ************************** GenericMatrix class ************************/

class GenericMatrix : public BaseMatrix
{
   GeneralMatrix* gm;
   int search(const BaseMatrix* bm) const;
   friend class BaseMatrix;
public:
   GenericMatrix() : gm(0) {}
   GenericMatrix(const BaseMatrix& bm)
      { gm = ((BaseMatrix&)bm).Evaluate(); gm = gm->Image(); }
   GenericMatrix(const GenericMatrix& bm)
      { gm = bm.gm->Image(); }
   void operator=(const GenericMatrix&);
   void operator=(const BaseMatrix&);
   void operator+=(const BaseMatrix&);
   void operator-=(const BaseMatrix&);
   void operator*=(const BaseMatrix&);
   void operator|=(const BaseMatrix&);
   void operator&=(const BaseMatrix&);
   void operator+=(Real);
   void operator-=(Real r) { operator+=(-r); }
   void operator*=(Real);
   void operator/=(Real r) { operator*=(1.0/r); }
   ~GenericMatrix() { delete gm; }
   void cleanup() { delete gm; gm = 0; }
   void Release() { gm->Release(); }
   void release() { gm->release(); }
   GeneralMatrix* Evaluate(MatrixType = MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;
   void swap(GenericMatrix& x);
   NEW_DELETE(GenericMatrix)
};

// *************************** temporary classes *************************/

class MultipliedMatrix : public BaseMatrix
{
protected:
   // if these union statements cause problems, simply remove them
   // and declare the items individually
   union { const BaseMatrix* bm1; GeneralMatrix* gm1; };
                                                  // pointers to summands
   union { const BaseMatrix* bm2; GeneralMatrix* gm2; };
   MultipliedMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : bm1(bm1x),bm2(bm2x) {}
   int search(const BaseMatrix*) const;
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~MultipliedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;
   NEW_DELETE(MultipliedMatrix)
};

class AddedMatrix : public MultipliedMatrix
{
protected:
   AddedMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : MultipliedMatrix(bm1x,bm2x) {}

   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~AddedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;
   NEW_DELETE(AddedMatrix)
};

class SPMatrix : public AddedMatrix
{
protected:
   SPMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : AddedMatrix(bm1x,bm2x) {}

   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~SPMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;

   friend SPMatrix SP(const BaseMatrix&, const BaseMatrix&);

   NEW_DELETE(SPMatrix)
};

class KPMatrix : public MultipliedMatrix
{
protected:
   KPMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : MultipliedMatrix(bm1x,bm2x) {}

   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~KPMatrix() {}
   MatrixBandWidth bandwidth() const;
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   friend KPMatrix KP(const BaseMatrix&, const BaseMatrix&);
   NEW_DELETE(KPMatrix)
};

class ConcatenatedMatrix : public MultipliedMatrix
{
protected:
   ConcatenatedMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : MultipliedMatrix(bm1x,bm2x) {}

   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~ConcatenatedMatrix() {}
   MatrixBandWidth bandwidth() const;
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   NEW_DELETE(ConcatenatedMatrix)
};

class StackedMatrix : public ConcatenatedMatrix
{
protected:
   StackedMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : ConcatenatedMatrix(bm1x,bm2x) {}

   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~StackedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   NEW_DELETE(StackedMatrix)
};

class SolvedMatrix : public MultipliedMatrix
{
   SolvedMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : MultipliedMatrix(bm1x,bm2x) {}
   friend class BaseMatrix;
   friend class InvertedMatrix;                        // for operator*
public:
   ~SolvedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;
   NEW_DELETE(SolvedMatrix)
};

class SubtractedMatrix : public AddedMatrix
{
   SubtractedMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : AddedMatrix(bm1x,bm2x) {}
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~SubtractedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   NEW_DELETE(SubtractedMatrix)
};

class ShiftedMatrix : public BaseMatrix
{
protected:
   union { const BaseMatrix* bm; GeneralMatrix* gm; };
   Real f;
   ShiftedMatrix(const BaseMatrix* bmx, Real fx) : bm(bmx),f(fx) {}
   int search(const BaseMatrix*) const;
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~ShiftedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   friend ShiftedMatrix operator+(Real f, const BaseMatrix& BM);
   NEW_DELETE(ShiftedMatrix)
};

class NegShiftedMatrix : public ShiftedMatrix
{
protected:
   NegShiftedMatrix(Real fx, const BaseMatrix* bmx) : ShiftedMatrix(bmx,fx) {}
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~NegShiftedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   friend NegShiftedMatrix operator-(Real, const BaseMatrix&);
   NEW_DELETE(NegShiftedMatrix)
};

class ScaledMatrix : public ShiftedMatrix
{
   ScaledMatrix(const BaseMatrix* bmx, Real fx) : ShiftedMatrix(bmx,fx) {}
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~ScaledMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;
   friend ScaledMatrix operator*(Real f, const BaseMatrix& BM);
   NEW_DELETE(ScaledMatrix)
};

class NegatedMatrix : public BaseMatrix
{
protected:
   union { const BaseMatrix* bm; GeneralMatrix* gm; };
   NegatedMatrix(const BaseMatrix* bmx) : bm(bmx) {}
   int search(const BaseMatrix*) const;
private:
   friend class BaseMatrix;
public:
   ~NegatedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;
   NEW_DELETE(NegatedMatrix)
};

class TransposedMatrix : public NegatedMatrix
{
   TransposedMatrix(const BaseMatrix* bmx) : NegatedMatrix(bmx) {}
   friend class BaseMatrix;
public:
   ~TransposedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;
   NEW_DELETE(TransposedMatrix)
};

class ReversedMatrix : public NegatedMatrix
{
   ReversedMatrix(const BaseMatrix* bmx) : NegatedMatrix(bmx) {}
   friend class BaseMatrix;
public:
   ~ReversedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   NEW_DELETE(ReversedMatrix)
};

class InvertedMatrix : public NegatedMatrix
{
   InvertedMatrix(const BaseMatrix* bmx) : NegatedMatrix(bmx) {}
public:
   ~InvertedMatrix() {}
   SolvedMatrix operator*(const BaseMatrix&) const;       // inverse(A) * B
   ScaledMatrix operator*(Real tt) const { return BaseMatrix::operator*(tt); }
   friend class BaseMatrix;
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;
   NEW_DELETE(InvertedMatrix)
};

class RowedMatrix : public NegatedMatrix
{
   RowedMatrix(const BaseMatrix* bmx) : NegatedMatrix(bmx) {}
   friend class BaseMatrix;
public:
   ~RowedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;
   NEW_DELETE(RowedMatrix)
};

class ColedMatrix : public NegatedMatrix
{
   ColedMatrix(const BaseMatrix* bmx) : NegatedMatrix(bmx) {}
   friend class BaseMatrix;
public:
   ~ColedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;
   NEW_DELETE(ColedMatrix)
};

class DiagedMatrix : public NegatedMatrix
{
   DiagedMatrix(const BaseMatrix* bmx) : NegatedMatrix(bmx) {}
   friend class BaseMatrix;
public:
   ~DiagedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;
   NEW_DELETE(DiagedMatrix)
};

class MatedMatrix : public NegatedMatrix
{
   int nr, nc;
   MatedMatrix(const BaseMatrix* bmx, int nrx, int ncx)
      : NegatedMatrix(bmx), nr(nrx), nc(ncx) {}
   friend class BaseMatrix;
public:
   ~MatedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth bandwidth() const;
   NEW_DELETE(MatedMatrix)
};

class ReturnMatrix : public BaseMatrix    // for matrix return
{
   GeneralMatrix* gm;
   int search(const BaseMatrix*) const;
public:
   ~ReturnMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   friend class BaseMatrix;
   ReturnMatrix(const ReturnMatrix& tm) : gm(tm.gm) {}
   ReturnMatrix(const GeneralMatrix* gmx) : gm((GeneralMatrix*&)gmx) {}
//   ReturnMatrix(GeneralMatrix&);
   MatrixBandWidth bandwidth() const;
   NEW_DELETE(ReturnMatrix)
};


// ************************** submatrices ******************************/

class GetSubMatrix : public NegatedMatrix
{
   int row_skip;
   int row_number;
   int col_skip;
   int col_number;
   bool IsSym;

   GetSubMatrix
      (const BaseMatrix* bmx, int rs, int rn, int cs, int cn, bool is)
      : NegatedMatrix(bmx),
      row_skip(rs), row_number(rn), col_skip(cs), col_number(cn), IsSym(is) {}
   void SetUpLHS();
   friend class BaseMatrix;
public:
   GetSubMatrix(const GetSubMatrix& g)
      : NegatedMatrix(g.bm), row_skip(g.row_skip), row_number(g.row_number),
      col_skip(g.col_skip), col_number(g.col_number), IsSym(g.IsSym) {}
   ~GetSubMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   void operator=(const BaseMatrix&);
   void operator+=(const BaseMatrix&);
   void operator-=(const BaseMatrix&);
   void operator=(const GetSubMatrix& m) { operator=((const BaseMatrix&)m); }
   void operator<<(const BaseMatrix&);
   void operator<<(const double*);                // copy from array
   void operator<<(const float*);                // copy from array
   void operator<<(const int*);                 // copy from array
   MatrixInput operator<<(double);                // for loading a list
   MatrixInput operator<<(float);                // for loading a list
   MatrixInput operator<<(int f);
   void operator=(Real);                        // copy from constant
   void operator+=(Real);                       // add constant
   void operator-=(Real r) { operator+=(-r); }  // subtract constant
   void operator*=(Real);                       // multiply by constant
   void operator/=(Real r) { operator*=(1.0/r); } // divide by constant
   void inject(const GeneralMatrix&);           // copy stored els only
   void Inject(const GeneralMatrix& GM) { inject(GM); }
   MatrixBandWidth bandwidth() const;
   NEW_DELETE(GetSubMatrix)
};

// ******************** linear equation solving ****************************/

class LinearEquationSolver : public BaseMatrix
{
   GeneralMatrix* gm;
   int search(const BaseMatrix*) const { return 0; }
   friend class BaseMatrix;
public:
   LinearEquationSolver(const BaseMatrix& bm);
   ~LinearEquationSolver() { delete gm; }
   void cleanup() { delete gm; } 
   GeneralMatrix* Evaluate(MatrixType) { return gm; }
   // probably should have an error message if MatrixType != UnSp
   NEW_DELETE(LinearEquationSolver)
};

// ************************** matrix input *******************************/

class MatrixInput          // for reading a list of values into a matrix
                           // the difficult part is detecting a mismatch
                           // in the number of elements
{
   int n;                  // number values still to be read
   Real* r;                // pointer to next location to be read to
public:
   MatrixInput(const MatrixInput& mi) : n(mi.n), r(mi.r) {}
   MatrixInput(int nx, Real* rx) : n(nx), r(rx) {}
   ~MatrixInput();
   MatrixInput operator<<(double);
   MatrixInput operator<<(float);
   MatrixInput operator<<(int f);
   friend class GeneralMatrix;
};



// **************** a very simple integer array class ********************/

// A minimal array class to imitate a C style array but giving dynamic storage
// mostly intended for internal use by newmat

class SimpleIntArray : public Janitor
{
protected:
   int* a;                    // pointer to the array
   int n;                     // length of the array
public:
   SimpleIntArray(int xn);    // build an array length xn
   SimpleIntArray() : a(0), n(0) {}  // build an array length 0
   ~SimpleIntArray();         // return the space to memory
   int& operator[](int ii);    // access element of the array - start at 0
   int operator[](int ii) const;
                              // access element of constant array
   void operator=(int ai);    // set the array equal to a constant
   void operator=(const SimpleIntArray& b);
                              // copy the elements of an array
   SimpleIntArray(const SimpleIntArray& b);
                              // make a new array equal to an existing one
   int Size() const { return n; }
                              // return the size of the array
   int size() const { return n; }
                              // return the size of the array
   int* Data() { return a; }  // pointer to the data
   const int* Data() const { return a; }  // pointer to the data
   int* data() { return a; }  // pointer to the data
   const int* data() const { return a; }  // pointer to the data
   const int* const_data() const { return a; }  // pointer to the data
   void resize(int ii, bool keep = false);
                              // change length, keep data if keep = true
   void ReSize(int ii, bool keep = false) { resize(ii, keep); }
                              // change length, keep data if keep = true
   void resize_keep(int ii) { resize(ii, true); }
                              // change length, keep data
   void cleanup() { resize(0); }
   void CleanUp() { resize(0); }
   NEW_DELETE(SimpleIntArray)
};

// ********************** C subscript classes ****************************

class RealStarStar
{
   Real** a;
public:
   RealStarStar(Matrix& A);
   ~RealStarStar() { delete [] a; }
   operator Real**() { return a; }
};

class ConstRealStarStar
{
   const Real** a;
public:
   ConstRealStarStar(const Matrix& A);
   ~ConstRealStarStar() { delete [] a; }
   operator const Real**() { return a; }
};

// *************************** exceptions ********************************/

class NPDException : public Runtime_error     // Not positive definite
{
public:
   static unsigned long Select;          // for identifying exception
   NPDException(const GeneralMatrix&);
};

class ConvergenceException : public Runtime_error
{
public:
   static unsigned long Select;          // for identifying exception
   ConvergenceException(const GeneralMatrix& A);
   ConvergenceException(const char* c);
};

class SingularException : public Runtime_error
{
public:
   static unsigned long Select;          // for identifying exception
   SingularException(const GeneralMatrix& A);
};

class OverflowException : public Runtime_error
{
public:
   static unsigned long Select;          // for identifying exception
   OverflowException(const char* c);
};

class ProgramException : public Logic_error
{
protected:
   ProgramException();
public:
   static unsigned long Select;          // for identifying exception
   ProgramException(const char* c);
   ProgramException(const char* c, const GeneralMatrix&);
   ProgramException(const char* c, const GeneralMatrix&, const GeneralMatrix&);
   ProgramException(const char* c, MatrixType, MatrixType);
};

class IndexException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   IndexException(int ii, const GeneralMatrix& A);
   IndexException(int ii, int jj, const GeneralMatrix& A);
   // next two are for access via element function
   IndexException(int ii, const GeneralMatrix& A, bool);
   IndexException(int ii, int jj, const GeneralMatrix& A, bool);
};

class VectorException : public Logic_error    // cannot convert to vector
{
public:
   static unsigned long Select;          // for identifying exception
   VectorException();
   VectorException(const GeneralMatrix& A);
};

class NotSquareException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   NotSquareException(const GeneralMatrix& A);
   NotSquareException();
};

class SubMatrixDimensionException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   SubMatrixDimensionException();
};

class IncompatibleDimensionsException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   IncompatibleDimensionsException();
   IncompatibleDimensionsException(const GeneralMatrix&);
   IncompatibleDimensionsException(const GeneralMatrix&, const GeneralMatrix&);
};

class NotDefinedException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   NotDefinedException(const char* op, const char* matrix);
};

class CannotBuildException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   CannotBuildException(const char* matrix);
};


class InternalException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   InternalException(const char* c);
};

// ************************ functions ************************************ //

bool operator==(const GeneralMatrix& A, const GeneralMatrix& B);
bool operator==(const BaseMatrix& A, const BaseMatrix& B);
inline bool operator!=(const GeneralMatrix& A, const GeneralMatrix& B)
   { return ! (A==B); }
inline bool operator!=(const BaseMatrix& A, const BaseMatrix& B)
   { return ! (A==B); }

   // inequality operators are dummies included for compatibility
   // with STL. They throw an exception if actually called.
inline bool operator<=(const BaseMatrix& A, const BaseMatrix&)
   { A.IEQND(); return true; }
inline bool operator>=(const BaseMatrix& A, const BaseMatrix&)
   { A.IEQND(); return true; }
inline bool operator<(const BaseMatrix& A, const BaseMatrix&)
   { A.IEQND(); return true; }
inline bool operator>(const BaseMatrix& A, const BaseMatrix&)
   { A.IEQND(); return true; }

bool is_zero(const BaseMatrix& A);
inline bool IsZero(const BaseMatrix& A) { return is_zero(A); }

Real dotproduct(const Matrix& A, const Matrix& B);
Matrix crossproduct(const Matrix& A, const Matrix& B);
ReturnMatrix crossproduct_rows(const Matrix& A, const Matrix& B);
ReturnMatrix crossproduct_columns(const Matrix& A, const Matrix& B);

inline Real DotProduct(const Matrix& A, const Matrix& B)
   { return dotproduct(A, B); }
inline Matrix CrossProduct(const Matrix& A, const Matrix& B)
   { return crossproduct(A, B); }
inline ReturnMatrix CrossProductRows(const Matrix& A, const Matrix& B)
   { return crossproduct_rows(A, B); }
inline ReturnMatrix CrossProductColumns(const Matrix& A, const Matrix& B)
   { return crossproduct_columns(A, B); }
   
void newmat_block_copy(int n, Real* from, Real* to);



// ********************* inline functions ******************************** //


inline LogAndSign log_determinant(const BaseMatrix& B)
   { return B.log_determinant(); }
inline LogAndSign LogDeterminant(const BaseMatrix& B)
   { return B.log_determinant(); }
inline Real determinant(const BaseMatrix& B)
   { return B.determinant(); }
inline Real Determinant(const BaseMatrix& B)
   { return B.determinant(); }
inline Real sum_square(const BaseMatrix& B) { return B.sum_square(); }
inline Real SumSquare(const BaseMatrix& B) { return B.sum_square(); }
inline Real norm_Frobenius(const BaseMatrix& B) { return B.norm_Frobenius(); }
inline Real norm_frobenius(const BaseMatrix& B) { return B.norm_Frobenius(); }
inline Real NormFrobenius(const BaseMatrix& B) { return B.norm_Frobenius(); }
inline Real trace(const BaseMatrix& B) { return B.trace(); }
inline Real Trace(const BaseMatrix& B) { return B.trace(); }
inline Real sum_absolute_value(const BaseMatrix& B)
   { return B.sum_absolute_value(); }
inline Real SumAbsoluteValue(const BaseMatrix& B)
   { return B.sum_absolute_value(); }
inline Real sum(const BaseMatrix& B)
   { return B.sum(); }
inline Real Sum(const BaseMatrix& B)
   { return B.sum(); }
inline Real maximum_absolute_value(const BaseMatrix& B)
   { return B.maximum_absolute_value(); }
inline Real MaximumAbsoluteValue(const BaseMatrix& B)
   { return B.maximum_absolute_value(); }
inline Real minimum_absolute_value(const BaseMatrix& B)
   { return B.minimum_absolute_value(); }
inline Real MinimumAbsoluteValue(const BaseMatrix& B)
   { return B.minimum_absolute_value(); }
inline Real maximum(const BaseMatrix& B) { return B.maximum(); }
inline Real Maximum(const BaseMatrix& B) { return B.maximum(); }
inline Real minimum(const BaseMatrix& B) { return B.minimum(); }
inline Real Minimum(const BaseMatrix& B) { return B.minimum(); }
inline Real norm1(const BaseMatrix& B) { return B.norm1(); }
inline Real Norm1(const BaseMatrix& B) { return B.norm1(); }
inline Real norm1(RowVector& RV) { return RV.maximum_absolute_value(); }
inline Real Norm1(RowVector& RV) { return RV.maximum_absolute_value(); }
inline Real norm_infinity(const BaseMatrix& B) { return B.norm_infinity(); }
inline Real NormInfinity(const BaseMatrix& B) { return B.norm_infinity(); }
inline Real norm_infinity(ColumnVector& CV)
   { return CV.maximum_absolute_value(); }
inline Real NormInfinity(ColumnVector& CV)
   { return CV.maximum_absolute_value(); }
inline bool IsZero(const GeneralMatrix& A) { return A.IsZero(); }
inline bool is_zero(const GeneralMatrix& A) { return A.is_zero(); }


inline MatrixInput MatrixInput::operator<<(int f) { return *this << (Real)f; }
inline MatrixInput GeneralMatrix::operator<<(int f) { return *this << (Real)f; }
inline MatrixInput BandMatrix::operator<<(int f) { return *this << (Real)f; }
inline MatrixInput GetSubMatrix::operator<<(int f) { return *this << (Real)f; }

inline ReversedMatrix BaseMatrix::Reverse() const { return reverse(); }
inline RowedMatrix BaseMatrix::AsRow() const { return as_row(); }
inline ColedMatrix BaseMatrix::AsColumn() const { return as_column(); }
inline DiagedMatrix BaseMatrix::AsDiagonal() const { return as_diagonal(); }
inline MatedMatrix BaseMatrix::AsMatrix(int m, int n) const
   { return as_matrix(m, n); }
inline GetSubMatrix BaseMatrix::SubMatrix(int fr, int lr, int fc, int lc) const
   { return submatrix(fr, lr, fc, lc); }
inline GetSubMatrix BaseMatrix::SymSubMatrix(int f, int l) const
   { return sym_submatrix(f, l); }
inline GetSubMatrix BaseMatrix::Row(int f) const { return row(f); }
inline GetSubMatrix BaseMatrix::Rows(int f, int l) const { return rows(f, l); }
inline GetSubMatrix BaseMatrix::Column(int f) const { return column(f); }
inline GetSubMatrix BaseMatrix::Columns(int f, int l) const
   { return columns(f, l); }
inline Real BaseMatrix::AsScalar() const { return as_scalar(); }

inline ReturnMatrix GeneralMatrix::ForReturn() const { return for_return(); }

inline void swap(Matrix& A, Matrix& B) { A.swap(B); }
inline void swap(SquareMatrix& A, SquareMatrix& B) { A.swap(B); }
inline void swap(nricMatrix& A, nricMatrix& B) { A.swap(B); }
inline void swap(UpperTriangularMatrix& A, UpperTriangularMatrix& B)
   { A.swap(B); }
inline void swap(LowerTriangularMatrix& A, LowerTriangularMatrix& B)
   { A.swap(B); }
inline void swap(SymmetricMatrix& A, SymmetricMatrix& B) { A.swap(B); }
inline void swap(DiagonalMatrix& A, DiagonalMatrix& B) { A.swap(B); }
inline void swap(RowVector& A, RowVector& B) { A.swap(B); }
inline void swap(ColumnVector& A, ColumnVector& B) { A.swap(B); }
inline void swap(CroutMatrix& A, CroutMatrix& B) { A.swap(B); }
inline void swap(BandMatrix& A, BandMatrix& B) { A.swap(B); }
inline void swap(UpperBandMatrix& A, UpperBandMatrix& B) { A.swap(B); }
inline void swap(LowerBandMatrix& A, LowerBandMatrix& B) { A.swap(B); }
inline void swap(SymmetricBandMatrix& A, SymmetricBandMatrix& B) { A.swap(B); }
inline void swap(BandLUMatrix& A, BandLUMatrix& B) { A.swap(B); }
inline void swap(IdentityMatrix& A, IdentityMatrix& B) { A.swap(B); }
inline void swap(GenericMatrix& A, GenericMatrix& B) { A.swap(B); }

#ifdef OPT_COMPATIBLE                    // for compatibility with opt++

inline Real Norm2(const ColumnVector& CV) { return CV.norm_Frobenius(); }
inline Real Dot(ColumnVector& CV1, ColumnVector& CV2)
   { return dotproduct(CV1, CV2); }

#endif


#ifdef use_namespace
}
#endif


#endif

// body file: newmat1.cpp
// body file: newmat2.cpp
// body file: newmat3.cpp
// body file: newmat4.cpp
// body file: newmat5.cpp
// body file: newmat6.cpp
// body file: newmat7.cpp
// body file: newmat8.cpp
// body file: newmatex.cpp
// body file: bandmat.cpp
// body file: submat.cpp







