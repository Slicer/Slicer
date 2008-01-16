/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper.h,v $
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

// .NAME vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper - A helper that generates composite images for the volume ray cast mapper
// .SECTION Description
// This is one of the helper classes for the vtkSlicerFixedPointVolumeRayCastMapper. 
// It will generate composite images using an alpha blending operation.
// This class should not be used directly, it is a helper class for
// the mapper and has no user-level API.
//
// .SECTION see also
// vtkSlicerFixedPointVolumeRayCastMapper

#ifndef __vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper_h
#define __vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper_h

#include "vtkSlicerFixedPointVolumeRayCastHelper.h"
#include "vtkVolumeRenderingReplacements.h"

class vtkSlicerFixedPointVolumeRayCastMapper;
class vtkVolume;

class VTK_VOLUMERENDERINGREPLACEMENTS_EXPORT vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper : public vtkSlicerFixedPointVolumeRayCastHelper
{
public:
  static vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper *New();
  vtkTypeRevisionMacro(vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper,vtkSlicerFixedPointVolumeRayCastHelper);
  void PrintSelf( ostream& os, vtkIndent indent );

  virtual void  GenerateImage( int threadID, 
                               int threadCount,
                               vtkVolume *vol,
                               vtkSlicerFixedPointVolumeRayCastMapper *mapper);

protected:
  vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper();
  ~vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper();

private:
  vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper(const vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper&);  // Not implemented.
  void operator=(const vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper&);  // Not implemented.
};

#endif


