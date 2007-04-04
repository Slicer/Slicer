/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: simpleVectors.h,v $
  Date:      $Date: 2006/05/26 19:59:40 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*===========================================================
  
Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#ifndef __simpleVectors_h
#define __simpleVectors_h

#include "stdio.h"

template <typename T>
struct Vector2D
{
public:
    Vector2D(T x, T y) {
        set(x, y); 
    }

    Vector2D() {};

    void set(T x, T y) { 
        values[0] = x; 
        values[1] = y;
    }

    void print() {
        printf("Vector2D: %g %g\n", values[0], values[1]);
    }

    T&  operator [](int i) { 
        return values[i];
    }

    const T&  operator [](int i) const {
        return values[i];
    }

    T values[2];
};


template <typename T>
struct Vector3D
{
public:
    Vector3D(T x, T y, T z) {
        set(x, y, z);
    }

    Vector3D() {};

    void set(T x, T y, T z) {
        values[0] = x;
        values[1] = y;
        values[2] = z; 
    }

    void print() {
        printf("Vector3D: %g %g %g\n", values[0], values[1], values[2]);
    }

    T&  operator [](int i) {
        return values[i];
    }

    const T&  operator [](int i) const {
        return values[i];
    }

    T values[3];
};

#endif
