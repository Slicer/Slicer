/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/

// .NAME vtkNeuroNavLogic - slicer logic class for Locator module 
// .SECTION Description
// This class manages the logic associated with tracking device for
// IGT. 


#ifndef __vtkNeuroNavLogic_h
#define __vtkNeuroNavLogic_h

#include "vtkNeuroNavWin32Header.h"
#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerLogic.h"


class VTK_NEURONAV_EXPORT vtkNeuroNavLogic : public vtkSlicerLogic 
{
public:

    // The Usual vtk class functions
    static vtkNeuroNavLogic *New();
    vtkTypeRevisionMacro(vtkNeuroNavLogic,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);


protected:

    vtkNeuroNavLogic();
    ~vtkNeuroNavLogic();
    vtkNeuroNavLogic(const vtkNeuroNavLogic&);
    void operator=(const vtkNeuroNavLogic&);

};

#endif


  
