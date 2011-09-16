/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMinHeap.h,v $
  Date:      $Date: 2006/01/06 17:57:55 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
/*  ==================================================
    Module : vtkLevelSets
    Authors: Karl Krissian
    Email  : karl@bwh.harvard.edu

    This module implements a Active Contour evolution
    for segmentation of 2D and 3D images.
    It implements a 'codimension 2' levelsets as an
    option for the smoothing term.
    It comes with a Tcl/Tk interface for the '3D Slicer'.
    ==================================================
    Copyright (C) 2003  LMI, Laboratory of Mathematics in Imaging, 
    Brigham and Women's Hospital, Boston MA USA

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    ================================================== 
   The full GNU Lesser General Public License file is in vtkLevelSets/LesserGPL_license.txt
*/

//
//  Karl Krissian
//  Brigham and Women's Hospital
//  Harvard Medical School
//  10/31/2001
//


#ifndef _MINHEAP_HPP
#define _MINHEAP_HPP


//#include "style.hpp"
#include "vtkEMSegment.h"
#include <iostream>
#include <stdio.h>

#if defined(_WIN32) || defined(WIN32)
# define IS_WIN32
# if defined(_MSC_VER)
#  define IS_VC
#  if _MSC_VER >= 1300
#   define IS_VC_DOTNET 1 // VC is at least version >= 7.0
#  elif _MSC_VER >= 1200   // last version before advent of .NET = Version 6.0
#   define IS_VC60 1
#  else
#   define IS_VC50 1
#  endif
# endif
#endif

//BTX

template < class T >  
class vtkMinHeap;

// Template ??
//
template<class T>
ostream& operator << (ostream& os, const vtkMinHeap<T>& p);

//----------------------------------------------------------------------
template < class T > class /* VTK_EMSEGMENT_EXPORT */ vtkMinHeap
//                                              ----------
{

protected:

  int num_elts;
  int array_size;

  // elements from 1 for num_elts
  T*  array;

  void Resize(int size);

  int UpHeap(int pos);

  int DownHeap(int pos);

  void (*move_func)(const T&, int pos, void* data);
  void* move_data;

public:

  vtkMinHeap( int arraysize = 2)
  {
    num_elts = 0;
    array_size = arraysize;
    if (array_size<1) array_size = 1;
    array = new T[array_size];
    move_func = NULL;
  }

  ~vtkMinHeap()
  {
    delete [] array;
  }

  // This function acts when a value is moved or added in
  // the tree, and can allow to keep trace of the position
  // in the tree of each value.
  void SetMoveFunction(void (*func)(const T&, int pos, void* data),
               void* movedata) 
  {
    move_func = func;
    move_data = movedata;
  }

  void RemoveAll() { num_elts = 0; }

  void SetValue( const T& t, int pos)
  {
    array[pos] = t;
    if (move_func != NULL) move_func(t,pos,move_data);
  }

  int Size() { return num_elts; }

  vtkMinHeap<T>&  operator+=( T elt);

  const T GetMin();

  T& operator[](int n);
  
  void ChangeValue(int n, const T& elt);

  //#if !(defined(_sgi_)) 
  //  friend ostream& operator << <>(ostream&, const vtkMinHeap<T>& p);
  //#else
  //  friend ostream& operator << (ostream&, const vtkMinHeap<T>& p);
  //#endif

  //#ifdef _WIN32

#if defined(IS_VC60) || defined(IS_VC50) 
  friend ostream& operator << (ostream&, const vtkMinHeap<T>& p);
#else
  friend ostream& operator << <>(ostream&, const vtkMinHeap<T>& p);
#endif

};


//----------------------------------------------------------------------
// PROTECTED MEMBERS
//----------------------------------------------------------------------

template<class T> void vtkMinHeap<T>::Resize( int size)
//                                 ------
{

    T*     new_array;
    int    new_nbelts,i=0;


    //  fprintf(stderr,"vtkMinHeap() resize %d \n",size);

  if (size < array_size) return;

  new_nbelts = size;

  //  fprintf(stderr,"vtkMinHeap::Resize() \t Allocation of size %f Mb \n",
  //      (new_nbelts+1)*sizeof(T)/1000000.0);
  new_array = new T [new_nbelts+1];

  for(i=0;i<=num_elts;i++)  new_array[i] = array[i];

  delete [] array;

  array = new_array;
  array_size = new_nbelts;

} // Resize()

//--------------------------------------------------
template<class T> int vtkMinHeap<T>::UpHeap( int pos)
//                                ------
{

    register int  up;
    T val;

    //    fprintf(stderr,"UpHeap(%d)\n",pos);

    if (pos<=1) return 1;

    val  = array[pos];
    up   = pos>>1;

    while ((up>0)&&(val<array[up])) {
      SetValue(array[up],pos);
      pos = up;
      up >>= 1;
    }

    SetValue(val,pos);

    return pos;


} // UpHeap()


//--------------------------------------------------
template<class T> int vtkMinHeap<T>::DownHeap( int pos)
//                                --------
{

    register int  down;
    T val;

    //    fprintf(stderr,"DownHeap(%d)\n",pos);

    val    = array[pos];
    down = pos<<1;

    if (down>num_elts) return pos;

    while (down<=num_elts) {

      if ((down<num_elts)&&(array[down+1]<array[down])) down++;

      if (val>array[down]) {
    SetValue(array[down],pos);
    pos = down;
    down <<= 1;
      }
      else break;

    }
    
    SetValue(val,pos);

    return pos;


} // DownHeap()

//----------------------------------------------------------------------
// PUBLIC MEMBERS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//
//   Add an element to the heap
//  
template<class T>
vtkMinHeap<T>& vtkMinHeap<T> :: operator+=( T elt)
//                        ----------
{

    if ( num_elts >= array_size-1) Resize( 2*array_size );

    num_elts++;
    SetValue(elt,num_elts);

    // Put the new value at a correct emplacement
    //    DownHeap( UpHeap(num_elts));
    UpHeap(num_elts);

    return *this;

} // operator +=

//----------------------------------------------------------------------
//
//   Retreive the min (top) of the heap
//  
template<class T>
const T vtkMinHeap<T> :: GetMin()
//              ------
{
    T valmin;

    if (num_elts == 1) { num_elts=0; return array[1];}

    valmin = array[1];
    SetValue(array[num_elts],1);
    num_elts--;

    // Put the value at a correct emplacement
    DownHeap( 1);

    return valmin;

} // GetMin()


//--------------------------------------------------
template<class T>
T& vtkMinHeap<T> :: operator[](int n)
//               ----------
{

  if ((n<1) || (n>num_elts))
  {     fprintf(stderr,
           "vtkMinHeap<T> operator[]\t Invalid Index... %d [1 %d]\n",
           n, num_elts);
      return array[1];
  } else
      return array[n];
      
} // operator[]


//--------------------------------------------------
template<class T> void vtkMinHeap<T>::ChangeValue( int pos, const T& elt)
//                                 -----------
{

  if (elt < array[pos]) {
    SetValue(elt,pos);
    UpHeap(pos);
  }
  else
    {
      SetValue(elt,pos);
      DownHeap(pos);
    }

  //  if (pos == UpHeap(pos)) DownHeap(pos);

} // UpDateValue()


//--------------------------------------------------
template<class T>
ostream& operator << (ostream& os, const vtkMinHeap<T>& p)
//       -----------
{
  int i,j,n;

  os << "Heap =  \n ";
  i = 1;
  j = 1;
  while (j<=p.num_elts) {
    for(n=1;n<=i;n++) {
      if (j<=p.num_elts) {
        os << p.array[j];
        j++;
        if (n < i)
          os  << " - ";
        else
          os   << "\n";
    
      } // end if
    } // end for
    i <<=1;
  }

  os   << "\n";
  return os;

} // operator << ( , const vtkMinHeap<T>&)


//ETX

#endif
