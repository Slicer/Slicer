/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: point.h,v $
  Date:      $Date: 2006/02/14 22:11:24 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
#ifndef __point_h
#define __point_h

#include "vtkSlicerBaseLogic.h"


class VTK_SLICER_BASE_LOGIC_EXPORT Point
{
public:
    Point() {
        this->x = 0;
        this->y = 0;
        this->selected = 0;
        this->next = NULL;
    }
    Point(int xa, int ya) {
        this->x = xa;
        this->y = ya;
        this->selected = 0;
        this->next = NULL;
    }
    ~Point() {
        this->next = NULL;
    }

    void Select()    {this->selected = 1;}
    void Deselect()  {this->selected = 0;}
    int IsSelected()   {return this->selected;}
    Point *GetNext() {return this->next;}
    
   //Karl - June 2005
    void Set3DCoord(float _x,float _y,float _z) {
      this->x0=_x;
      this->y0=_y;
      this->z0=_z;
    }

public:
    int x;
    int y;
    //jc - 4.7.05 - add x,y,z for 3D point 
    float x0, y0, z0;
    Point *next;

private:
    int selected;
};

#endif

