/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerOpenGLRayCastImageDisplayHelper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME vtkSlicerOpenGLRayCastImageDisplayHelper - OpenGL subclass that draws the image to the screen
// .SECTION Description
// This is the concrete implementation of a ray cast image display helper -
// a helper class responsible for drawing the image to the screen.

// .SECTION see also
// vtkSlicerRayCastImageDisplayHelper

#ifndef __vtkSlicerOpenGLRayCastImageDisplayHelper_h
#define __vtkSlicerOpenGLRayCastImageDisplayHelper_h

#include "vtkSlicerRayCastImageDisplayHelper.h"
#include "vtkVolumeRenderingModule.h"

class vtkVolume;
class vtkRenderer;
class vtkSlicerFixedPointRayCastImage;

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerOpenGLRayCastImageDisplayHelper : public vtkSlicerRayCastImageDisplayHelper
{
public:
  static vtkSlicerOpenGLRayCastImageDisplayHelper *New();
  vtkTypeRevisionMacro(vtkSlicerOpenGLRayCastImageDisplayHelper,vtkSlicerRayCastImageDisplayHelper);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  void RenderTexture( vtkVolume *vol, vtkRenderer *ren,
                      int imageMemorySize[2],
                      int imageViewportSize[2],
                      int imageInUseSize[2],
                      int imageOrigin[2],
                      float requestedDepth,
                      unsigned char *image );

  void RenderTexture( vtkVolume *vol, vtkRenderer *ren,
                      int imageMemorySize[2],
                      int imageViewportSize[2],
                      int imageInUseSize[2],
                      int imageOrigin[2],
                      float requestedDepth,
                      unsigned short *image );

  void RenderTexture( vtkVolume *vol, vtkRenderer *ren,
                      vtkSlicerFixedPointRayCastImage *image,
                      float requestedDepth );

protected:
  vtkSlicerOpenGLRayCastImageDisplayHelper();
  ~vtkSlicerOpenGLRayCastImageDisplayHelper();
  
  void RenderTextureInternal( vtkVolume *vol, vtkRenderer *ren,
                              int imageMemorySize[2],
                              int imageViewportSize[2],
                              int imageInUseSize[2],
                              int imageOrigin[2],
                              float requestedDepth,
                              int imageScalarType,
                              void *image );

private:

  void *PreviousImage;
  vtkSlicerOpenGLRayCastImageDisplayHelper(const vtkSlicerOpenGLRayCastImageDisplayHelper&);  // Not implemented.
  void operator=(const vtkSlicerOpenGLRayCastImageDisplayHelper&);  // Not implemented.
};

#endif

