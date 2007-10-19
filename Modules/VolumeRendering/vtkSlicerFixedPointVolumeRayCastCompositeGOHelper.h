/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerFixedPointVolumeRayCastCompositeGOHelper.h,v $
  Language:  C++
  Date:      $Date: 2005/05/04 14:13:58 $
  Version:   $Revision: 1.1 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME vtkSlicerFixedPointVolumeRayCastCompositeGOHelper - A helper that generates composite images for the volume ray cast mapper
// .SECTION Description
// This is one of the helper classes for the vtkSlicerFixedPointVolumeRayCastMapper. 
// It will generate composite images using an alpha blending operation.
// This class should not be used directly, it is a helper class for
// the mapper and has no user-level API.
//
// .SECTION see also
// vtkSlicerFixedPointVolumeRayCastMapper

#ifndef __vtkSlicerFixedPointVolumeRayCastCompositeGOHelper_h
#define __vtkSlicerFixedPointVolumeRayCastCompositeGOHelper_h

#include "vtkVolumeRenderingModule.h"
#include "vtkSlicerFixedPointVolumeRayCastHelper.h"

class vtkSlicerFixedPointVolumeRayCastMapper;
class vtkVolume;

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerFixedPointVolumeRayCastCompositeGOHelper : public vtkSlicerFixedPointVolumeRayCastHelper
{
public:
  static vtkSlicerFixedPointVolumeRayCastCompositeGOHelper *New();
  vtkTypeRevisionMacro(vtkSlicerFixedPointVolumeRayCastCompositeGOHelper,vtkSlicerFixedPointVolumeRayCastHelper);
  void PrintSelf( ostream& os, vtkIndent indent );

  virtual void  GenerateImage( int threadID, 
                               int threadCount,
                               vtkVolume *vol,
                               vtkSlicerFixedPointVolumeRayCastMapper *mapper);

protected:
  vtkSlicerFixedPointVolumeRayCastCompositeGOHelper();
  ~vtkSlicerFixedPointVolumeRayCastCompositeGOHelper();

private:
  vtkSlicerFixedPointVolumeRayCastCompositeGOHelper(const vtkSlicerFixedPointVolumeRayCastCompositeGOHelper&);  // Not implemented.
  void operator=(const vtkSlicerFixedPointVolumeRayCastCompositeGOHelper&);  // Not implemented.
};

#endif


