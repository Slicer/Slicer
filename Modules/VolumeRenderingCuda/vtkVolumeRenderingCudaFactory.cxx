/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVolumeRenderingCudaFactory.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

    This software is distributed WITHOUT ANY WARRANTY; without even
    the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
    PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkObjectFactory.h"

#include "vtkVolumeRenderingCudaFactory.h"
#include "vtkToolkits.h"
#include "vtkVolumeRenderingToolkit.h"
#include "vtkDebugLeaks.h"
#include "vtkGraphicsFactory.h"

#include "vtkCriticalSection.h"

#include "stdlib.h"
#include "vtkCudaVolumeMapper.h"
#include "vtkVolumeRenderingCudaModuleGUI.h"

vtkCxxRevisionMacro(vtkVolumeRenderingCudaFactory, "$Revision: 1.7 $");
vtkStandardNewMacro(vtkVolumeRenderingCudaFactory);


vtkObject* vtkVolumeRenderingCudaFactory::CreateInstance(const char* vtkclassname )
{
  // first check the object factory
  vtkObject *ret = vtkObjectFactory::CreateInstance(vtkclassname);
  if (ret)
  {
    return ret;
  }
  // if the factory failed to create the object,
  // then destroy it now, as vtkDebugLeaks::ConstructClass was called
  // with vtkclassname, and not the real name of the class
#ifdef VTK_DEBUG_LEAKS
  vtkDebugLeaks::DestructClass(vtkclassname);
#endif
  const char *rl = vtkGraphicsFactory::GetRenderLibrary();

  if (!strcmp("vtkCudaVolumeMapper", vtkclassname) ||
      !strcmp("vtkOpenGLVolumeTextureMapper2D", vtkclassname))
    return vtkCudaVolumeMapper::New();

  if (!strcmp("vtkVolumeRenderingCudaModuleGUI", vtkclassname))
    return vtkVolumeRenderingCudaModuleGUI::New();

  return 0;
}

//----------------------------------------------------------------------------
void vtkVolumeRenderingCudaFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
