/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkOpenGLScriptedActor.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOpenGLScriptedActor.h"

#include "vtkMapper.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLRenderer.h"
#include "vtkProperty.h"

#include "vtkOpenGL.h"
#include <math.h>

//
// this class doesn't really need to be part of slicer,
// but we re-use slicer's python and tcl inferaces 
// and build options to be part of the module
//
#include "vtkScriptedModuleGUI.h"
#include "vtkSlicerConfigure.h" /* Slicer_USE_* */

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"

#ifdef Slicer_USE_PYTHON
#include "slicerPython.h"
#endif


#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkOpenGLScriptedActor, "$Revision: 1.31 $");
vtkStandardNewMacro(vtkOpenGLScriptedActor);
#endif

//----------------------------------------------------------------------------
vtkOpenGLScriptedActor::vtkOpenGLScriptedActor()
{
  this->Script = NULL;
  this->Language = vtkOpenGLScriptedActor::Python;
}

//----------------------------------------------------------------------------
vtkOpenGLScriptedActor::~vtkOpenGLScriptedActor()
{
  this->SetScript(NULL);
}

//----------------------------------------------------------------------------
// Actual scripted actor render method.
//
// This is essenitally the core of vtkOpenGLActor, but with the call
// to the mapper replaced by the invocation of a script.  In this case
// the mapper is optional, since it's rendering code is replaced by the script.
// But it can still be useful to have the mapper available to 
// access rendering parameters.
//
// Care must be taken in the render script not to re-enter the render pipeline
// (e.g. don't call Render on any render windows or update the GUI loop).
//
void vtkOpenGLScriptedActor::Render(vtkRenderer *ren, vtkMapper *vtkNotUsed(mapper))
{
  // bail out early if we aren't configured corretly
  if (this->Language != vtkOpenGLScriptedActor::Python)
    {
    vtkErrorMacro("Only python is currently supported.");
    return;
    }
  if (this->Script == NULL)
    {
    vtkErrorMacro("No script specified for scripted actor.");
    return;
    }

  double opacity;

  // get opacity
  opacity = this->GetProperty()->GetOpacity();
  if (opacity == 1.0)
    {
    glDepthMask (GL_TRUE);
    }
  else
    {
    // add this check here for GL_SELECT mode
    // If we are not picking, then don't write to the zbuffer
    // because we probably haven't sorted the polygons. If we
    // are picking, then translucency doesn't matter - we want to
    // pick the thing closest to us.
    GLint param[1];
    glGetIntegerv(GL_RENDER_MODE, param);
    if(param[0] == GL_SELECT )
      {
      glDepthMask(GL_TRUE);
      }
    else
      {
      if(ren->GetLastRenderingUsedDepthPeeling())
        {
        glDepthMask(GL_TRUE); // transparency with depth peeling
        }
      else
        {
        glDepthMask (GL_FALSE); // transparency with alpha blending
        }
      }
    }

  // build transformation 
  if (!this->IsIdentity)
    {
    double *mat = this->GetMatrix()->Element[0];
    double mat2[16];
    mat2[0] = mat[0];
    mat2[1] = mat[4];
    mat2[2] = mat[8];
    mat2[3] = mat[12];
    mat2[4] = mat[1];
    mat2[5] = mat[5];
    mat2[6] = mat[9];
    mat2[7] = mat[13];
    mat2[8] = mat[2];
    mat2[9] = mat[6];
    mat2[10] = mat[10];
    mat2[11] = mat[14];
    mat2[12] = mat[3];
    mat2[13] = mat[7];
    mat2[14] = mat[11];
    mat2[15] = mat[15];
    
    // insert model transformation 
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glMultMatrixd(mat2);
    }

  
  //
  // invoke the script here
  //
  
  if ( this->Script)
    {
    if (this->Language == vtkOpenGLScriptedActor::Python)
      {
#ifdef Slicer_USE_PYTHON
      // Script actor renderer mapper
      vtkSlicerApplication *slicerApp = vtkSlicerApplication::GetInstance();
      vtkSlicerApplicationGUI *slicerAppGUI = slicerApp->GetApplicationGUI();
      slicerAppGUI->PythonCommand(this->Script);
#else
      vtkErrorMacro("Only python is currently supported but it was not compiled into this build.");
#endif
      }
    }


  // pop transformation matrix
  if (!this->IsIdentity)
    {
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    }
  
  if (opacity != 1.0)
    {
    glDepthMask (GL_TRUE);
    }
}

//----------------------------------------------------------------------------
void vtkOpenGLScriptedActor::PrintSelf(ostream& os, vtkIndent indent)
{
  if ( this->Script )
    {
    os << indent << "Script: " << this->Script << "\n";
    }
  else
    {
    os << indent << "Script: (none)\n";
    }
  this->Superclass::PrintSelf(os,indent);
}
