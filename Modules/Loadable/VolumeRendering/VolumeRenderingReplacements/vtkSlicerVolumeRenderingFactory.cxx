/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerVolumeRenderingFactory.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkSlicerVolumeRenderingFactory.h"

#include <vtkDebugLeaks.h>
#include <vtkGraphicsFactory.h>
#include <vtkObjectFactory.h>
#include <vtkVersion.h>

// if using some sort of opengl, then include these files
#if defined(VTK_USE_OGLR) || defined(_WIN32) || defined(VTK_USE_COCOA) || defined(VTK_USE_CARBON)
#include "vtkSlicerOpenGLRayCastImageDisplayHelper.h"
#endif

vtkStandardNewMacro(vtkSlicerVolumeRenderingFactory);


vtkObject* vtkSlicerVolumeRenderingFactory::CreateInstance(const char* vtkclassname )
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

#if defined(VTK_USE_OGLR) || defined(_WIN32) || defined(VTK_USE_COCOA) || defined(VTK_USE_CARBON)
  const char *rl = vtkGraphicsFactory::GetRenderLibrary();
  if (!strcmp("OpenGL",rl) || !strcmp("Win32OpenGL",rl) || !strcmp("CarbonOpenGL",rl) || !strcmp("CocoaOpenGL",rl))
    {
    // Ray Cast Image Display Helper
    if(strcmp(vtkclassname, "vtkSlicerRayCastImageDisplayHelper") == 0)
      {
      return vtkSlicerOpenGLRayCastImageDisplayHelper::New();
      }
    }
#endif

  return 0;
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
