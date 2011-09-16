
#ifndef _DynArray_txx
#define _DynArray_txx

#include "DynArray.h"

#define muExceptionMacro(x) \
  { \
    std::cerr << "ERROR: " x << std::endl; \
  }

template <class T>
T*
_DynArray_safeAlloc(unsigned int n)
{ 
  T* array = new T[n];
  if (array == NULL)
    muExceptionMacro(<< "DynArray: Failed allocating " << n << " elements");
  return array;
}

template <class T>
DynArray<T>
::DynArray()
{

  m_Size = 0;
  m_MaxSize = 5;

  //m_Array = new T[m_MaxSize];
  m_Array = _DynArray_safeAlloc<T>(m_MaxSize);

}

template <class T>
DynArray<T>
::DynArray(const DynArray<T>& l)
{
  m_Size = l.m_Size;
  m_MaxSize = l.m_MaxSize;

  //m_Array = new T[m_MaxSize];
  m_Array = _DynArray_safeAlloc<T>(m_MaxSize);
  for (unsigned int i = 0; i < m_Size; i++)
    m_Array[i] = l.m_Array[i];
}

template <class T>
DynArray<T>
::~DynArray()
{
  delete [] m_Array;
}

template <class T>
T&
DynArray<T>
::operator[](unsigned int i) const
{
  if (i >= m_Size)
  {
    muExceptionMacro(
      << "DynArray[i] access out of bounds, index = " << i
      << ", size = " << m_Size);
  }

  return m_Array[i];
}

template <class T>
DynArray<T>&
DynArray<T>
::operator=(const DynArray& l)
{
  delete [] m_Array;

  m_MaxSize = l.m_MaxSize;
  m_Size = l.m_Size;

  //m_Array = new T[m_MaxSize];
  m_Array = _DynArray_safeAlloc<T>(m_MaxSize);
  for (unsigned int i = 0; i < m_Size; i++)
    m_Array[i] = l.m_Array[i];

  return *this;
}

template <class T>
void
DynArray<T>
::Allocate(unsigned int size)
{
  if (size == 0 || size <= m_Size)
    return;

  m_MaxSize = size + 1;

  //T* newArray = new T[m_MaxSize];
  T* newArray = _DynArray_safeAlloc<T>(m_MaxSize);

  for (unsigned int i = 0; i < m_Size; i++)
    newArray[i] = m_Array[i];

  delete [] m_Array;

  m_Array = newArray;
}

template <class T>
void
DynArray<T>
::Clear()
{

  m_MaxSize = 5;
  m_Size = 0;

  delete [] m_Array;

  //m_Array = new T[m_MaxSize];
  m_Array = _DynArray_safeAlloc<T>(m_MaxSize);

}

template <class T>
void
DynArray<T>
::Append(const T& e)
{
  if (m_Size == m_MaxSize)
  {
    m_MaxSize = 2*m_MaxSize + 1;
    //T* newArray = new T[m_MaxSize];
    T* newArray = _DynArray_safeAlloc<T>(m_MaxSize);
    for (unsigned int i = 0; i < m_Size; i++)
      newArray[i] = m_Array[i];
    delete [] m_Array;
    m_Array = newArray;
  }

  m_Array[m_Size] = e;
  m_Size++;
}

template <class T>
void
DynArray<T>
::Remove(unsigned int i)
{
  if (i >= m_Size)
  {
    muExceptionMacro(
      << "DynArray::Remove access out of bounds, index = " << i
      << ", size = " << m_Size);
  }

  for (unsigned int j = i; j < (m_Size-1); j++)
    m_Array[j] = m_Array[j+1];
  m_Size--;

/* Skip this part for speed
  // Reclaim memory when wasted space gets too large
  if (m_Size < (m_MaxSize/8))
  {
    m_MaxSize /= 8;
    //T* newArray = new T[m_MaxSize];
    T* newArray = _DynArray_safeAlloc<T>(m_MaxSize);
    for (unsigned int i = 0; i < m_Size; i++)
      newArray[i] = m_Array[i];
    delete [] m_Array;
    m_Array = newArray;
  }
*/
}

template <class T>
void
DynArray<T>
::Initialize(unsigned int n, const T& e)
{
  delete [] m_Array;

  m_Array = _DynArray_safeAlloc<T>(n+1);
  for (unsigned int k = 0; k < n; k++)
    m_Array[k] = e;

  m_Size = n;
  m_MaxSize = n+1;
}

template <class T>
DynArray<T>
DynArray<T>
::Slice(unsigned int i, unsigned int j)
{
  if (i >= m_Size)
  {
    muExceptionMacro(
      << "DynArray::Slice access out of bounds, index = " << i
      << ", size = " << m_Size);
  }

  if (j >= m_Size)
  {
    muExceptionMacro(
      << "DynArray::Slice access out of bounds, index = " << j
      << ", size = " << m_Size);
  }

  DynArray<T> temp;

  // Return empty array if range is invalid
  if (j <= i)
    return temp;

  unsigned int size = j - i + 1;

  temp.Allocate(size);

  for (unsigned int k = 0; k < size; k++)
    temp.Append(this->m_Array[i+k]);

  return temp;
}

template <class T>
void
DynArray<T>
::Pack()
{
  this->Allocate(m_Size+1);
}

template <class T>
void
DynArray<T>
::WrapArray(T* array, unsigned int n)
{
  if (array == 0)
    muExceptionMacro(<< "Cannot wrap a NULL array");

  delete [] m_Array;

  m_Array = array;
  m_Size = n;
  m_MaxSize = n;
}

#endif
