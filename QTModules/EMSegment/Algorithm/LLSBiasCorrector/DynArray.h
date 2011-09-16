
////////////////////////////////////////////////////////////////////////////////
//
// Dynamically allocated array
//
////////////////////////////////////////////////////////////////////////////////

// prastawa@cs.unc.edu 11/2003

#ifndef _DynArray_h
#define _DynArray_h

template <class T>
class DynArray
{

public:
  DynArray();
  DynArray(const DynArray& l);
  ~DynArray();

  // Reference with bounds checking
  T& operator[](unsigned int i) const;

  DynArray<T>& operator=(const DynArray<T>& l);

  void Allocate(unsigned int size);
  void Clear();

  void Append(const T& e);
  void Remove(unsigned int i);

  void Initialize(unsigned int n, const T& e);

  DynArray<T> Slice(unsigned int i, unsigned int j);

  void Pack();

  void WrapArray(T* array, unsigned int n);

  inline unsigned int GetSize() const { return m_Size; }

  inline T* GetRawArray() const { return m_Array; }

  // Pointers for iterators and STL sort()
  inline T* GetFirst() const { return m_Array; }
  inline T* GetLast() const { return (m_Array+m_Size); }

  // Cast operator
  //inline operator T*() const { return m_Array; }

private:

  T* m_Array;

  unsigned int m_MaxSize;
  unsigned int m_Size;

};

#ifndef MU_MANUAL_INSTANTIATION
#include "DynArray.txx"
#endif

#endif
