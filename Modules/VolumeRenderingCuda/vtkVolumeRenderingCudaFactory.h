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

#ifndef __vtkVolumeRenderingCudaFactory_h
#define __vtkVolumeRenderingCudaFactory_h

#include "vtkObject.h"
#include "vtkVolumeRenderingCudaModule.h"

class VTK_VOLUMERENDERINGCUDAMODULE_EXPORT vtkVolumeRenderingCudaFactory : public vtkObject
{
public:
  static vtkVolumeRenderingCudaFactory *New();
  vtkTypeRevisionMacro(vtkVolumeRenderingCudaFactory,vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create and return an instance of the named vtk object.
  // This method first checks the vtkObjectFactory to support
  // dynamic loading. 
  static vtkObject* CreateInstance(const char* vtkclassname);

protected:
  vtkVolumeRenderingCudaFactory() {};

private:
  vtkVolumeRenderingCudaFactory(const vtkVolumeRenderingCudaFactory&);  // Not implemented.
  void operator=(const vtkVolumeRenderingCudaFactory&);  // Not implemented.
};

#endif
