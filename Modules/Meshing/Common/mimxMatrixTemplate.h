// **************************************************
// Matrix Template Class
// Copyright(c), 2000, S. D. Rajan
// 
// Implements a two-dimensional array
// Row and Column indexing start at 1
//
// **************************************************
#ifndef __mimxMatrixTemplate_h
#define __mimxMatrixTemplate_h

#include "vtkCommon.h"

#pragma once

#include <iostream>
/*using std::cout;
using std::cin;*/

//using std::cerr;

#include <cassert>
#include <cmath>

#ifndef FATAL
#define FATAL    0
#endif
#ifndef NOTFATAL
#define NOTFATAL  1
#endif

// defines the Matrix class
template <class T>
class CMatrix {
  protected:            
    T  **cells;      // address where the matrix of type T is stored
    int  nRows;        // number of rows in the matrix
    int nColumns;      // number of columns in the matrix
    void ErrorHandler (int,int nR=0, int nC=0) const;
                // handles error conditions
    void Release ();    // similar to destructor

  public:            
    CMatrix (int, int);    // constructor
    CMatrix ();        // default constructor
    ~CMatrix ();      // destructor
    void SetSize (int, int);  // sets the size of the matrix
                  // used with the default constructor

    // matrix size manipulation functions
    // including memory allocations and deallocations
    int GetRows () const;    // gets the current number of rows
    int GetColumns () const;  // gets the current number of columns

    // matrix manipulations (mutator)
    void Set (T);        // sets the value of all elements
                  // of a matrix
    T& operator() (int, int);        // row-col access
    const T& operator() (int, int) const;  // row-col access
    T& operator= (const CMatrix&);  // overloaded = operator
};

// =============== definitions ===========================================
template <class T>
CMatrix<T>::CMatrix ()          // default constructor
{
  cells = NULL; 
  nRows = 0;
  nColumns = 0;
}

template <class T>
CMatrix<T>::CMatrix (int nR, int nC)  // constructor
{
  int i;
  // check whether nR and nC are legal
  if (nR <= 0 || nC <= 0)
    ErrorHandler (3);
  int size = nR*nC + 1;
  cells = new T *[nR + 1]; 
  if (cells == NULL) ErrorHandler (1);
  cells[0] = new T[size]; 
  if (cells == NULL) ErrorHandler (1);
  cells[1] = cells[0];
  for (i=2; i <= nR; i++)
     cells[i] = cells[i-1]+nC;
  nRows = nR;
  nColumns = nC;
}

template <class T>
void CMatrix<T>::SetSize (int nR, int nC)  // sets the size of the matrix
{
  int i;
  // check whether nR and nC are legal
  if (nR <= 0 || nC <= 0)
    ErrorHandler (3);
  Release ();
  int size = nR*nC + 1;
  cells = new T *[nR + 1]; 
  if (cells == NULL) ErrorHandler (1);
  cells[0]=new T[size]; 
  if (cells == NULL) ErrorHandler (1);
  cells[1] = cells[0];
  for (i=2; i <= nR; i++)
     cells[i] = cells[i-1]+nC;
  nRows = nR;
  nColumns = nC;
}

template <class T>
CMatrix<T>::~CMatrix ()    // destructor
{
  // deallocate storage
  if (cells != NULL) delete [] cells[0];
  if (cells != NULL) delete [] cells;
}

template <class T>
void CMatrix<T>::Release ()    // similar to the destructor
{
  // deallocate storage
  if (cells != NULL) delete [] cells[0];
  if (cells != NULL) delete [] cells;
  cells = NULL; 
  nRows = 0;
  nColumns = 0;
}

// =============== member functions ===========================================
template <class T>
int CMatrix<T>::GetRows () const
{
  return (nRows);
}

template <class T>
int CMatrix<T>::GetColumns () const
{
  return (nColumns);
}

template <class T>
void CMatrix<T>::Set (T dV)  // sets the value of all the elements of the matrix
              // to dV
{
  for (int i=1; i <= nRows; i++) {
    for (int j=1; j <= nColumns; j++) {
      cells[i][j] = dV; // or, (*this)(i,j) = dV;
    }
  }
}

// ==================== Overloaded Operators ========================
// overload () for use as bounds-checking
template <class T>
T& CMatrix<T>::operator() (int nR, int nC)  // T& is reference
{
  // row-column index in bounds?
#ifdef _DEBUG
  if (nR <= 0 || nR > nRows || nC <= 0 || nC > nColumns) {
    ErrorHandler (2,nR,nC);
    return cells[1][1];
  }
  else
    return cells[nR][nC];
#else
    return cells[nR][nC];
#endif
}

template <class T>
const T& CMatrix<T>::operator() (int nR, int nC) const  // T& is reference
{
  // row-column index in bounds?
#ifdef _DEBUG
  if (nR <= 0 || nR > nRows || nC <= 0 || nC > nColumns) {
    ErrorHandler (2,nR,nC);
    return cells[1][1];
  }
  else
    return cells[nR][nC];
#else
    return cells[nR][nC];
#endif
}

// overload = for matrix equality
template <class T>
T& CMatrix<T>::operator= (const CMatrix& matarg)
{
  // check whether matrix is assigned to itself
  if (this != &matarg)
  {
    // compatible matrices?
    if (nRows != matarg.nRows || nColumns != matarg.nColumns) {
      ErrorHandler (4);
      return (T&)(*this);
    }
    // now copy
    for (int i=1; i <= matarg.nRows; i++)
      for (int j=1; j <= matarg.nColumns; j++) {
        cells[i][j]= matarg.cells[i][j];
      }
  }

  return (T&)(*this);
}

// ==================== Error Handler ========================
template <class T>
void CMatrix<T>::ErrorHandler (int nErrorCode, int nR, int nC) const
{
  int errorLevel = FATAL;

#if _DEBUG
  errorLevel = NOTFATAL;
  switch (nErrorCode)
  {
    case 1:
      cerr << "Matrix:: Memory allocation failure.\n";
      errorLevel = FATAL;
    break;
    case 2:
      cerr << "Matrix::Row-Column reference is out of bounds.\n";
      errorLevel = FATAL;
    break;
    case 3:
      cerr << "Matrix::Constructor.Invalid number of rows or columns.\n";
      errorLevel = FATAL;
    break;
    case 4:
      cerr << "Matrix::Incompatible matrices.\n";
    break;
  }
#endif
  assert (errorLevel == NOTFATAL);
}


#endif
