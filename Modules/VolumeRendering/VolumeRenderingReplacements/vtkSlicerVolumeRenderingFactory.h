/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerVolumeRenderingFactory.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSlicerVolumeRenderingFactory - 
// .SECTION Description

#ifndef __vtkSlicerVolumeRenderingFactory_h
#define __vtkSlicerVolumeRenderingFactory_h

#include "vtkObject.h"
#include "vtkVolumeRenderingReplacements.h"

class VTK_VOLUMERENDERINGREPLACEMENTS_EXPORT vtkSlicerVolumeRenderingFactory : public vtkObject
{
public:
  static vtkSlicerVolumeRenderingFactory *New();
  vtkTypeRevisionMacro(vtkSlicerVolumeRenderingFactory,vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create and return an instance of the named vtk object.
  // This method first checks the vtkObjectFactory to support
  // dynamic loading. 
  static vtkObject* CreateInstance(const char* vtkclassname);

protected:
  vtkSlicerVolumeRenderingFactory() {};

private:
  vtkSlicerVolumeRenderingFactory(const vtkSlicerVolumeRenderingFactory&);  // Not implemented.
  void operator=(const vtkSlicerVolumeRenderingFactory&);  // Not implemented.
};

#endif
