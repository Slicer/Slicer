/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSorter.h,v $
  Date:      $Date: 2006/05/26 19:59:44 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
/*===========================================================

Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#ifndef __vtkSorter_h
#define __vtkSorter_h


#include "vtkObject.h"
#include "vtkCollection.h"
#include "vtkRenderer.h"


#include <vtkQueryAtlasConfigure.h>

#include <vtkTextureText.h>
#include <vtkCard.h>

class vtkCamera;


// orig _vtkSortValues from vtkDepthSortPolyData:

typedef struct _vtkSorterValues {
  vtkFloatingPointType z;
  vtkIdType cellId;
  int cellType;
} vtkSorterValues;


// strange issue /w vtkCommand not being a vtkObject - so we'll make the command a member - 2003-03-11

class VTK_QUERYATLAS_EXPORT vtkSorter : public vtkObject
{
public:
    void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkSorter, vtkObject);

    static vtkSorter *New();

    vtkSorterValues *LastDepth;
    int LastDepthNum;

    void SetSelectedItem(vtkObject *o);

    vtkGetMacro(AlwaysResort, int);  
    vtkSetMacro(AlwaysResort, int);  

    vtkSetObjectMacro(Renderer, vtkRenderer);
    vtkGetObjectMacro(Renderer, vtkRenderer);

    vtkGetObjectMacro(Cards, vtkCollection);  
    vtkGetObjectMacro(TextureTexts, vtkCollection);  

    void DepthSort();


protected:
    vtkSorter();
    ~vtkSorter(); 


    int AlwaysResort;

//BTX
    void ComputeProjectionVector(vtkCamera *c, vtkFloatingPointType vector[3], vtkFloatingPointType origin[3]);
//ETX

    vtkCollection *Cards;
    vtkCollection *TextureTexts;

    vtkRenderer *Renderer;

    vtkObject *SelectedItem;
};

#endif
