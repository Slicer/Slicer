/*=auto=========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerTractographyDisplayLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerTractographyDisplayLogic - slicer logic class for tractography display
// .SECTION Description
// This class wraps vtkSlicerFiberBundleDisplayLogic so the loadable module
// code can access this using the naming convention.
// .SEE  vtkSlicerFiberBundleDisplayLogic
#ifndef __vtkSlicerTractographyDisplayLogic_h
#define __vtkSlicerTractographyDisplayLogic_h

#include "vtkSlicerFiberBundleDisplayLogic.h"

class VTK_SLICERTRACTOGRAPHYDISPLAY_EXPORT vtkSlicerTractographyDisplayLogic : public vtkSlicerFiberBundleDisplayLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerTractographyDisplayLogic *New();
  vtkTypeRevisionMacro(vtkSlicerTractographyDisplayLogic,vtkSlicerFiberBundleDisplayLogic);

  protected:
  vtkSlicerTractographyDisplayLogic();
  vtkSlicerTractographyDisplayLogic(const vtkSlicerTractographyDisplayLogic&);
  void operator=(const vtkSlicerTractographyDisplayLogic&);

};

#endif

