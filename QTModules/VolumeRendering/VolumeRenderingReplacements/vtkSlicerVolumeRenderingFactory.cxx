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
#include "vtkObjectFactory.h"

#include "vtkSlicerVolumeRenderingFactory.h"
#include "vtkGraphicsFactory.h"


// if using some sort of opengl, then include these files
#if defined(VTK_USE_OGLR) || defined(_WIN32) || defined(VTK_USE_COCOA) || defined(VTK_USE_CARBON)
#endif

#if defined(VTK_USE_MANGLED_MESA)
#endif



vtkCxxRevisionMacro(vtkSlicerVolumeRenderingFactory, "$Revision: 1.7 $");
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
  const char *rl = vtkGraphicsFactory::GetRenderLibrary();
  

#if defined(VTK_USE_OGLR) || defined(_WIN32) || defined(VTK_USE_COCOA) || defined(VTK_USE_CARBON)
  if (!strcmp("OpenGL",rl) || !strcmp("Win32OpenGL",rl) || !strcmp("CarbonOpenGL",rl) || !strcmp("CocoaOpenGL",rl))
    {
    // 2D Volume Texture Mapper
    if(strcmp(vtkclassname, "vtkVolumeTextureMapper2D") == 0)
      {
#if defined(VTK_USE_MANGLED_MESA)
      if ( vtkGraphicsFactory::GetUseMesaClasses() )
        {
        return vtkMesaVolumeTextureMapper2D::New();
        }
#endif
      return vtkOpenGLVolumeTextureMapper2D::New();
      }
    
    // 3D Volume Texture Mapper
    if(strcmp(vtkclassname, "vtkSlicerVolumeTextureMapper3D") == 0)
      {
#if defined(VTK_USE_MANGLED_MESA)
      if ( vtkGraphicsFactory::GetUseMesaClasses() )
        {
        vtkGenericWarningMacro("No support for mesa in vtkVolumeTextureMapper3D");
        return 0;
        }
#endif
      return vtkSlicerOpenGLVolumeTextureMapper3D::New();
      }

        // 3D Volume GPU RayCast Mapper
    if(strcmp(vtkclassname, "vtkSlicerGPURayCastVolumeMapper") == 0)
      {
#if defined(VTK_USE_MANGLED_MESA)
      if ( vtkGraphicsFactory::GetUseMesaClasses() )
        {
        vtkGenericWarningMacro("No support for mesa in vtkSlicerGPURayCastVolumeMapper");
        return 0;
        }
#endif
        return vtkSlicerGPURayCastVolumeMapper::New();
      }

      // 3D Multi Volume GPU RayCast Mapper
    if(strcmp(vtkclassname, "vtkSlicerGPURayCastMultiVolumeMapper") == 0)
      {
#if defined(VTK_USE_MANGLED_MESA)
      if ( vtkGraphicsFactory::GetUseMesaClasses() )
        {
        vtkGenericWarningMacro("No support for mesa in vtkSlicerGPURayCastMultiVolumeMapper");
        return 0;
        }
#endif
        return vtkSlicerGPURayCastMultiVolumeMapper::New();
      }
      
    // Ray Cast Image Display Helper
    if(strcmp(vtkclassname, "vtkSlicerRayCastImageDisplayHelper") == 0)
      {
#if defined(VTK_USE_MANGLED_MESA)
      if ( vtkGraphicsFactory::GetUseMesaClasses() )
        {
        return vtkMesaRayCastImageDisplayHelper::New();
        }
#endif
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
