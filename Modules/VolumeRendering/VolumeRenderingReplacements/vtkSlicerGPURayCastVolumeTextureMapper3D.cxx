/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerGPURayCastVolumeTextureMapper3D.h,v $

   Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkSlicerGPURayCastVolumeTextureMapper3D
  Module:    $HeadURL: http://www.na-mic.org/svn/Slicer3/ $
  Date:      $Date: 2009-01-07 09:26:53 -0500 (Tue, 30 Jan 2009) $
  Version:   $Revision:  $


=========================================================================*/
#include <string.h>
#include <iostream>
#include <sstream>

#include "vtkWindows.h"
#include "vtkSlicerGPURayCastVolumeTextureMapper3D.h"

#include "vtkImageData.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPlaneCollection.h"
#include "vtkPointData.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkTimerLog.h"
#include "vtkVolumeProperty.h"
#include "vtkTransform.h"
#include "vtkLightCollection.h"
#include "vtkLight.h"
#include "vtkCamera.h"
#include "vtkMath.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkgl.h"

#include "vtkCommand.h"

//#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkSlicerGPURayCastVolumeTextureMapper3D, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkSlicerGPURayCastVolumeTextureMapper3D);
//#endif

vtkSlicerGPURayCastVolumeTextureMapper3D::vtkSlicerGPURayCastVolumeTextureMapper3D()
{
  this->Initialized          =  0;
  this->RayCastInitialized       =  0;
  this->MIPRendering         =  0;
  
  this->Clipping             =  0;
  this->Shading              =  0;
  this->ReloadShaderFlag     =  0;
  this->InternalVolumeSize      =  256; //by default 256^3

  this->Volume1Index         =  0;
  this->Volume2Index         =  0;
  this->Volume3Index         =  0;
  this->ColorLookupIndex         =  0;
  this->AlphaLookupIndex         =  0;
  this->RayCastVertexShader      =  0;
  this->RayCastFragmentShader    =  0;
  this->RayCastProgram           =  0;
  this->RayCastSupported         =  0;
  this->RenderWindow         = NULL;
  this->RaySteps             = 250.0f;

  this->GlobalAlpha          = 1.0f;
}

vtkSlicerGPURayCastVolumeTextureMapper3D::~vtkSlicerGPURayCastVolumeTextureMapper3D()
{
}

// Release the graphics resources used by this texture.  
void vtkSlicerGPURayCastVolumeTextureMapper3D::ReleaseGraphicsResources(vtkWindow 
                                *renWin)
{
  if (( this->Volume1Index || this->Volume2Index || 
    this->Volume3Index || this->ColorLookupIndex ||
    this->AlphaLookupIndex) && renWin)
    {
    static_cast<vtkRenderWindow *>(renWin)->MakeCurrent();
#ifdef GL_VERSION_1_1
    // free any textures
    this->DeleteTextureIndex( &this->Volume1Index );
    this->DeleteTextureIndex( &this->Volume2Index );
    this->DeleteTextureIndex( &this->Volume3Index );
    this->DeleteTextureIndex( &this->ColorLookupIndex );
    this->DeleteTextureIndex( &this->AlphaLookupIndex );    
#endif
    }
  if ( this->RayCastVertexShader || this->RayCastFragmentShader || this->RayCastProgram)
  {
    vtkgl::DeleteShader(this->RayCastVertexShader);
    vtkgl::DeleteShader(this->RayCastFragmentShader);
    vtkgl::DeleteProgram(this->RayCastProgram);
  }
    
  this->Volume1Index     = 0;
  this->Volume2Index     = 0;
  this->Volume3Index     = 0;
  this->ColorLookupIndex = 0;
  this->AlphaLookupIndex = 0;
  this->RayCastVertexShader   = 0;
  this->RayCastFragmentShader = 0;
  this->RayCastProgram    = 0;
  this->RenderWindow     = NULL;
  this->Modified();
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::Render(vtkRenderer *ren, vtkVolume *vol)
{  
  ren->GetRenderWindow()->MakeCurrent();
    
  if ( !this->Initialized )
    {
    this->Initialize();
    }
  
  if ( !this->RayCastInitialized || this->ReloadShaderFlag)
    {
    this->InitializeRayCast();
    }
    
  // Start the timer now
  this->Timer->StartTimer();

  glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_TEXTURE_BIT);
  
  glDisable(GL_LIGHTING);
  vtkGraphicErrorMacro(ren->GetRenderWindow(),"Before actual render method");
 
  this->RenderGLSL(ren, vol);
  
  glPopAttrib();

  glFlush();
  glFinish();
      
  this->Timer->StopTimer();      

  this->TimeToDraw = static_cast<float>(this->Timer->GetElapsedTime());

  // If the timer is not accurate enough, set it to a small
  // time so that it is not zero
  if ( this->TimeToDraw == 0.0 )
    {
    this->TimeToDraw = 0.0001;
    }
  
  //adjust ray steps based on requrestd frame rate
  this->AdaptivePerformanceControl();
  
  //printf("ray step: %f, fps: %f\n", this->RaySteps, 1.0/this->TimeToDraw);
  
  double progress = 1;
  this->InvokeEvent(vtkCommand::VolumeMapperRenderProgressEvent, &progress);
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::AdaptivePerformanceControl()
{
  //do automatic performance control
  if(this->Framerate <= 0.0f)
    this->Framerate = 1.0f;

  if (this->TimeToDraw <= 0.25/this->Framerate)//increase ray steps for better quality when possible
  {
    this->RaySteps *= 3.75f;
  }
  else if (this->TimeToDraw <= 0.5/this->Framerate)
  {
    this->RaySteps *= 1.75f;
  }
  else if (this->TimeToDraw <= 0.85/this->Framerate)
  {
    this->RaySteps += 25.0f; 
  }
  else if (this->TimeToDraw > 1.25/this->Framerate)//reduce ray steps to ensure performance
  {
    this->RaySteps *= 0.65f;
  }
  
  int dim[3];
  this->GetVolumeDimensions(dim);

  float maxRaysteps = dim[0];
  maxRaysteps = maxRaysteps > dim[1] ? maxRaysteps : dim[1];  
  maxRaysteps = maxRaysteps > dim[2] ? maxRaysteps : dim[2];  
  maxRaysteps *= 16.0f; //make sure we have enough sampling rate to recover details
  
  maxRaysteps = maxRaysteps < 1050.0f ? 1050.0f : maxRaysteps;//ensure high sampling rate on low resolution volumes
  
  // add clamp
  if (this->RaySteps > maxRaysteps) this->RaySteps = maxRaysteps;
  if (this->RaySteps < 150.0f)       this->RaySteps = 150.0f;
}

//needs to be cleaned, 2008/10/20, Yanling Liu
void vtkSlicerGPURayCastVolumeTextureMapper3D::SetupRayCastParameters(vtkRenderer *pRen, vtkVolume *pVol)
{
  double bounds[6];
  this->GetInput()->GetBounds(bounds);
  
  vtkMatrix4x4       *matrix = vtkMatrix4x4::New();

  // build transformation 
  pVol->GetMatrix(matrix);

  //transform volume bbox vertices
  //
  //Yanling, ABCC, NCI-Frederick, 20081124
  //
  //Problem:
  //Failed to implement the original GPU ray casting algorithm in Slicer: 
  // unable to get correct texture when rendering both volume and 3D slices
  // back and aux buffer tested
  //The original GPU ray casting algorithm incompatible with quad-buffer stereo rendering
  // which buffer to read in stereo rendering?
  
  //Solution:
  // draw front face of volume bbox only
  // one-pass GPU ray casting
  // no need for render-to-buffer and buffer reading
  
  //Known problem:
  // one-pass GPU ray casting assumes volume bbox is axis-aligned
  // when volume bbox is not axis-aligned, distorted image rendered when clipping enabled
  
  double vertices[8][4] = {
    {bounds[0], bounds[2], bounds[4], 1.0},
    {bounds[1], bounds[2], bounds[4], 1.0},
    {bounds[1], bounds[3], bounds[4], 1.0},
    {bounds[0], bounds[3], bounds[4], 1.0},
        
    {bounds[0], bounds[2], bounds[5], 1.0},
    {bounds[1], bounds[2], bounds[5], 1.0},
    {bounds[1], bounds[3], bounds[5], 1.0},
    {bounds[0], bounds[3], bounds[5], 1.0},
  };
    
  for (int i = 0; i < 8; i++)
  {
    vtkMatrix4x4::MultiplyPoint(*(matrix->Element), vertices[i], vertices[i]);
  }

  {//do clipping
    vtkPlaneCollection *clipPlanes;
    vtkPlane       *plane;
    int        numClipPlanes = 0;

    clipPlanes = this->ClippingPlanes;
    if ( clipPlanes && this->Clipping)
    {
        numClipPlanes = clipPlanes->GetNumberOfItems();
        if (numClipPlanes > 6)
        {
      vtkErrorMacro(<< "OpenGL guarantees only 6 additional clipping planes");
        }
        
        double lowerBounds[3];
        double upperBounds[3];
    
    double *pNormal = NULL;
    double *pOrigin = NULL;
    
    //find out clip box
    for (int i = 0; i < numClipPlanes; i++)
    {
        plane = static_cast<vtkPlane *>(clipPlanes->GetItemAsObject(i));
        pNormal = plane->GetNormal();
        pOrigin = plane->GetOrigin();
                
        if (pNormal[0] > 0.85 || pNormal[0] < -0.85)//x
        {
            if (pNormal[0] > 0.0)//+x: min
                lowerBounds[0] = pOrigin[0];
            else
                upperBounds[0] = pOrigin[0];
        }
        else if (pNormal[1] > 0.85 || pNormal[1] < -0.85)//y
        {
            if (pNormal[1] > 0.0)//+y: min
                lowerBounds[1] = pOrigin[1];
            else
                upperBounds[1] = pOrigin[1];
        }
        else //z
        {
            if (pNormal[2] > 0.0)//+z: min
                lowerBounds[2] = pOrigin[2];
            else
                upperBounds[2] = pOrigin[2];
        }
    }
    
    //clip vertices
    //correct when volume is axis-aligned
    //not correct when volume is rotated to be non-axis-aligned
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            vertices[i][j] = vertices[i][j] < lowerBounds[j] ? lowerBounds[j] : vertices[i][j];
            vertices[i][j] = vertices[i][j] > upperBounds[j] ? upperBounds[j] : vertices[i][j];
        }
    }
    }
  }
   
  {//volume bbox vertices coords
    for (int i = 0; i < 8; i++)
    {
    for (int j = 0; j < 3; j++)
    {
        VolumeBBoxVertices[i][j] = vertices[i][j];
    }
    }
  }
  
  {//volume bbox vertices colors
    //transfer clipped vertices back for color(texture coord)
    matrix->Invert();
    for (int i = 0; i < 8; i++)
    {
        vtkMatrix4x4::MultiplyPoint(*(matrix->Element), vertices[i], vertices[i]);
    }
    
    double verticesColor[8][3];
    double bboxLen[3] = {
        bounds[1] - bounds[0],
        bounds[3] - bounds[2],
        bounds[5] - bounds[4],
    };
        
    double lowerBounds[3] = {bounds[0], bounds[2], bounds[4]};
    
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 3; j++)
            verticesColor[i][j] = (vertices[i][j] - lowerBounds[j])/bboxLen[j];
    }
    
    memcpy(VolumeBBoxVerticesColor, verticesColor, sizeof(double)*24);
  }
  
  //ParaMatrix:                                                             
  //EyePos.x,      EyePos.y,      EyePos.z,     Step                        
  //VolBBoxLow.x,  VolBBoxLow.y,  VolBBoxLow.z, VolBBoxHigh.x               
  //VolBBoxHigh.y, VolBBoxHigh.z, RenderMethod, DepthPeelingThreshold,                
  //N/A,           GlobalAlpha,   Debug,
  
  double modelViewMat[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMat);
  
  double invModelViewMat[16];
  vtkMatrix4x4::Invert(modelViewMat, invModelViewMat);
  
  double zero[4] = {0.0, 0.0, 0.0, 1.0};
  double eye[4];
  vtkMatrix4x4::PointMultiply(invModelViewMat, zero, eye);

  vtkMatrix4x4::MultiplyPoint(*(matrix->Element), eye, eye);
  double bboxLen[3] = {
    bounds[1] - bounds[0],
    bounds[3] - bounds[2],
    bounds[5] - bounds[4],
  };
        
  double lowerBounds[3] = {bounds[0], bounds[2], bounds[4]};
  
  for (int i = 0; i < 3; i++)
  {
    eye[i] = (eye[i] - lowerBounds[i])/bboxLen[i];
  }
  
  this->ParaMatrix[0] = (GLfloat)eye[0];
  this->ParaMatrix[1] = (GLfloat)eye[1];
  this->ParaMatrix[2] = (GLfloat)eye[2];
  this->ParaMatrix[3] = (GLfloat)(1.0f/RaySteps);
  
  //recalculate texcoord/color bounds in case volume is not axis aligned
  double clrLowerBounds[3] = {VolumeBBoxVerticesColor[0][0], VolumeBBoxVerticesColor[0][1], VolumeBBoxVerticesColor[0][2]};
  double clrUpperBounds[3] = {VolumeBBoxVerticesColor[0][0], VolumeBBoxVerticesColor[0][1], VolumeBBoxVerticesColor[0][2]};
  
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 3; j++)
    {
        clrLowerBounds[j] = clrLowerBounds[j] > VolumeBBoxVerticesColor[i][j] ? VolumeBBoxVerticesColor[i][j] : clrLowerBounds[j];
        clrUpperBounds[j] = clrUpperBounds[j] < VolumeBBoxVerticesColor[i][j] ? VolumeBBoxVerticesColor[i][j] : clrUpperBounds[j];
    }
  }
  
  this->ParaMatrix[4] = (GLfloat)clrLowerBounds[0] < 0.0f ? 0.0f : (GLfloat)clrLowerBounds[0];
  this->ParaMatrix[5] = (GLfloat)clrLowerBounds[1] < 0.0f ? 0.0f : (GLfloat)clrLowerBounds[1];
  this->ParaMatrix[6] = (GLfloat)clrLowerBounds[2] < 0.0f ? 0.0f : (GLfloat)clrLowerBounds[2];
  
  this->ParaMatrix[7] = (GLfloat)clrUpperBounds[0] > 1.0f ? 1.0f : (GLfloat)clrUpperBounds[0];
  this->ParaMatrix[8] = (GLfloat)clrUpperBounds[1] > 1.0f ? 1.0f : (GLfloat)clrUpperBounds[1];
  this->ParaMatrix[9] = (GLfloat)clrUpperBounds[2] > 1.0f ? 1.0f : (GLfloat)clrUpperBounds[2];
  
//  printf("%f %f %f %f %f %f\n", this->ParaMatrix[4], this->ParaMatrix[5], this->ParaMatrix[6], this->ParaMatrix[7], this->ParaMatrix[8], this->ParaMatrix[9]);
  int components = this->GetInput()->GetNumberOfScalarComponents();   
  switch ( components )
    {
    case 1:
    this->ParaMatrix[10] = 0.0f;
      break;
      
    case 2:
    this->ParaMatrix[10] = 1.0f;
      break;
      
    case 3:
    case 4:
    this->ParaMatrix[10] = 2.0f;
      break;
    }
  
  //scalar range is 0 ~ 2048, mapping to 0 ~ 65535 since we are using unsigned short
  this->ParaMatrix[11] = ((this->DepthPeelingThreshold + this->ScalarOffset) * this->ScalarScale )/255.0f;
  this->ParaMatrix[12] = 0.0f;
  
  this->ParaMatrix[13] = GlobalAlpha;
  
  this->ParaMatrix[14] = 0.0f;//debug, 0: no debug, 1: show ray origin, 2: show ray end
  
  GLfloat volMat[16];
  for (int i = 0; i < 16; i++)
    volMat[i] = (GLfloat)(*(matrix->Element))[i];
    
  GLint loc = vtkgl::GetUniformLocation(RayCastProgram, "ParaMatrix");
  if (loc >= 0)
    vtkgl::UniformMatrix4fv(loc, 1, false, this->ParaMatrix);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "VolumeMatrix");
  if (loc >= 0)
    vtkgl::UniformMatrix4fv(loc, 1, false, volMat);  
  matrix->Delete();
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::RenderGLSL( vtkRenderer *ren, vtkVolume *vol )
{
  int components = this->GetInput()->GetNumberOfScalarComponents();   
  switch ( components )
    {
    case 1:
    this->RenderOneIndependentGLSL(ren,vol);
      break;
      
    case 2:
    this->RenderTwoDependentGLSL(ren,vol);
      break;
      
    case 3:
    case 4:
    this->RenderFourDependentGLSL(ren,vol);
      break;
    }
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::RenderOneIndependentGLSL( vtkRenderer *ren, vtkVolume *vol )
{
  vtkgl::UseProgram(RayCastProgram);    
  
  this->SetupOneIndependentTextures( ren, vol );
  this->SetupRayCastParameters(ren, vol);
  
//  glFrontFace(GL_CW);
  glEnable(GL_CULL_FACE);

//  glDisable(GL_DEPTH_TEST);
  this->DrawVolumeBBox();
   
  vtkgl::UseProgram(0);
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::RenderTwoDependentGLSL( vtkRenderer *ren, vtkVolume *vol )
{
  vtkgl::UseProgram(RayCastProgram);    
  
  this->SetupTwoDependentTextures( ren, vol );
  this->SetupRayCastParameters(ren, vol);
  
//  glFrontFace(GL_CW);
  glEnable(GL_CULL_FACE);

//  glDisable(GL_DEPTH_TEST);
  this->DrawVolumeBBox();
  
  vtkgl::UseProgram(0);
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::RenderFourDependentGLSL( vtkRenderer *ren, vtkVolume *vol )
{
  vtkgl::UseProgram(RayCastProgram);    
  
  this->SetupFourDependentTextures( ren, vol );
  this->SetupRayCastParameters(ren, vol);
  
//  glFrontFace(GL_CW);
  glEnable(GL_CULL_FACE);

//  glDisable(GL_DEPTH_TEST);
  this->DrawVolumeBBox();
  
  vtkgl::UseProgram(0);
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::DeleteTextureIndex( GLuint *index )
{
  if (glIsTexture(*index))
    {
    GLuint tempIndex;
    tempIndex = *index;
    glDeleteTextures(1, &tempIndex);
    *index = 0;
    }
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::CreateTextureIndex( GLuint *index )
{
  GLuint tempIndex=0;    
  glGenTextures(1, &tempIndex);
  *index = static_cast<long>(tempIndex);
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::Setup3DTextureParameters( vtkVolumeProperty *property )
{
  if ( property->GetInterpolationType() == VTK_NEAREST_INTERPOLATION )
    {
    glTexParameterf( vtkgl::TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( vtkgl::TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    }
  else
    {
    glTexParameterf( vtkgl::TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( vtkgl::TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    }
  glTexParameterf( vtkgl::TEXTURE_3D, vtkgl::TEXTURE_WRAP_R, GL_CLAMP );
  glTexParameterf( vtkgl::TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameterf( vtkgl::TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP );
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::SetupOneIndependentTextures( vtkRenderer *vtkNotUsed(ren),
            vtkVolume *vol )
{
  //0, 1, 2, 3
  //7, 6, 5, 4
  // Update the volume containing the 2 byte scalar / gradient magnitude
  if ( this->UpdateVolumes( vol ) || !this->Volume1Index || !this->Volume2Index )
    {    
    int dim[3];
    this->GetVolumeDimensions(dim);
    this->DeleteTextureIndex(&this->Volume3Index);
    
    vtkgl::ActiveTexture( vtkgl::TEXTURE7 );
    this->DeleteTextureIndex(&this->Volume1Index);
    this->CreateTextureIndex(&this->Volume1Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);
    vtkgl::TexImage3D( vtkgl::TEXTURE_3D, 0, GL_LUMINANCE8_ALPHA8, dim[0], dim[1], dim[2], 0,
               GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, this->Volume1 );
    

    vtkgl::ActiveTexture( vtkgl::TEXTURE5 );
    this->DeleteTextureIndex(&this->Volume2Index);
    this->CreateTextureIndex(&this->Volume2Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);
    vtkgl::TexImage3D( vtkgl::TEXTURE_3D, 0, GL_RGBA8, dim[0], dim[1], dim[2], 0,
               GL_RGB, GL_UNSIGNED_BYTE, this->Volume2 );
    }
  
  vtkgl::ActiveTexture( vtkgl::TEXTURE7 );
  glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);   
  this->Setup3DTextureParameters( vol->GetProperty() );

  vtkgl::ActiveTexture( vtkgl::TEXTURE5 );
  glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);   
  this->Setup3DTextureParameters( vol->GetProperty() );

  vtkgl::ActiveTexture( vtkgl::TEXTURE6 );
 
  // Update the dependent 2D color table mapping scalar value and
  // gradient magnitude to RGBA
  if ( this->UpdateColorLookup( vol ) || !this->ColorLookupIndex )
    {
    this->DeleteTextureIndex( &this->ColorLookupIndex );
    this->DeleteTextureIndex( &this->AlphaLookupIndex );
    
    this->CreateTextureIndex( &this->ColorLookupIndex );
    glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);   

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );    
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0,
          GL_RGBA, GL_UNSIGNED_BYTE, this->ColorLookup );    
    }
  
  vtkgl::ActiveTexture( vtkgl::TEXTURE6 );
  glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);
  
  GLint loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureVol");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 7);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureVol1");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 5);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureColorLookup");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 6);
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::SetupTwoDependentTextures(
  vtkRenderer *vtkNotUsed(ren),
  vtkVolume *vol )
{
  //0, 1, 2, 3
  //7, 6, 5, 4
  // Update the volume containing the 3 byte scalars / gradient magnitude
  if ( this->UpdateVolumes( vol ) || !this->Volume1Index || !this->Volume2Index )
    {    
    int dim[3];
    this->GetVolumeDimensions(dim);
    this->DeleteTextureIndex(&this->Volume3Index);
    
    vtkgl::ActiveTexture( vtkgl::TEXTURE7 );
    glBindTexture(vtkgl::TEXTURE_3D,0);
    this->DeleteTextureIndex(&this->Volume1Index);
    this->CreateTextureIndex(&this->Volume1Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D, 0, GL_RGB8, dim[0], dim[1], dim[2], 0,
              GL_RGB, GL_UNSIGNED_BYTE, this->Volume1 );
    
    vtkgl::ActiveTexture( vtkgl::TEXTURE5 );
    glBindTexture(vtkgl::TEXTURE_3D,0);
    this->DeleteTextureIndex(&this->Volume2Index);
    this->CreateTextureIndex(&this->Volume2Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0, GL_RGBA8, dim[0], dim[1], dim[2], 0,
               GL_RGB, GL_UNSIGNED_BYTE, this->Volume2 );
    }
  
  vtkgl::ActiveTexture( vtkgl::TEXTURE7 );
  glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);   
  this->Setup3DTextureParameters( vol->GetProperty() );

  vtkgl::ActiveTexture( vtkgl::TEXTURE5 );
  glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);   
  this->Setup3DTextureParameters( vol->GetProperty() );

  // Update the dependent 2D color table mapping scalar value and
  // gradient magnitude to RGBA
  if ( this->UpdateColorLookup( vol ) || 
       !this->ColorLookupIndex || !this->AlphaLookupIndex )
    {    
    vtkgl::ActiveTexture( vtkgl::TEXTURE6 );
    glBindTexture(GL_TEXTURE_2D,0);
    this->DeleteTextureIndex(&this->ColorLookupIndex);
    this->CreateTextureIndex(&this->ColorLookupIndex);
    glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);   

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, 256, 256, 0,
          GL_RGB, GL_UNSIGNED_BYTE, this->ColorLookup );    
      
    vtkgl::ActiveTexture( vtkgl::TEXTURE4 );
    glBindTexture(GL_TEXTURE_2D,0);
    this->DeleteTextureIndex(&this->AlphaLookupIndex);
    this->CreateTextureIndex(&this->AlphaLookupIndex);
    glBindTexture(GL_TEXTURE_2D, this->AlphaLookupIndex);   

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA8, 256, 256, 0,
          GL_ALPHA, GL_UNSIGNED_BYTE, this->AlphaLookup );      
    }
  
  vtkgl::ActiveTexture( vtkgl::TEXTURE6 );
  glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);   

  vtkgl::ActiveTexture( vtkgl::TEXTURE4 );
  glBindTexture(GL_TEXTURE_2D, this->AlphaLookupIndex);   
  
  GLint loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureVol");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 7);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureVol1");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 5);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureColorLookup");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 6);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureAlphaLookup");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 4);
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::SetupFourDependentTextures(
  vtkRenderer *vtkNotUsed(ren),
  vtkVolume *vol )
{
  //0, 1, 2, 3
  //7, 6, 5, 4
  // Update the volume containing the 3 byte scalars / gradient magnitude
  if ( this->UpdateVolumes( vol ) || !this->Volume1Index || 
       !this->Volume2Index || !this->Volume3Index )
    {    
    int dim[3];
    this->GetVolumeDimensions(dim);
    
    vtkgl::ActiveTexture( vtkgl::TEXTURE7 );
    glBindTexture(vtkgl::TEXTURE_3D,0);
    this->DeleteTextureIndex(&this->Volume1Index);
    this->CreateTextureIndex(&this->Volume1Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D, 0, GL_RGB8, dim[0], dim[1], dim[2], 0,
              GL_RGB, GL_UNSIGNED_BYTE, this->Volume1 );

    vtkgl::ActiveTexture( vtkgl::TEXTURE6 );
    glBindTexture(vtkgl::TEXTURE_3D,0);
    this->DeleteTextureIndex(&this->Volume2Index);
    this->CreateTextureIndex(&this->Volume2Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);   
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,GL_LUMINANCE8_ALPHA8,dim[0],dim[1],
              dim[2], 0,GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
              this->Volume2 );

    vtkgl::ActiveTexture( vtkgl::TEXTURE5 );
    glBindTexture(vtkgl::TEXTURE_3D,0);
    this->DeleteTextureIndex(&this->Volume3Index);
    this->CreateTextureIndex(&this->Volume3Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume3Index);
    vtkgl::TexImage3D( vtkgl::TEXTURE_3D,0, GL_RGB8, dim[0], dim[1], dim[2], 0,
               GL_RGB, GL_UNSIGNED_BYTE, this->Volume3 );
    }
  
  vtkgl::ActiveTexture( vtkgl::TEXTURE7 );
  glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);
  this->Setup3DTextureParameters( vol->GetProperty() );

  vtkgl::ActiveTexture( vtkgl::TEXTURE6 );
  glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);   
  this->Setup3DTextureParameters( vol->GetProperty() );

  vtkgl::ActiveTexture( vtkgl::TEXTURE5 );
  glBindTexture(vtkgl::TEXTURE_3D_EXT, this->Volume3Index);   
  this->Setup3DTextureParameters( vol->GetProperty() );

  // Update the dependent 2D table mapping scalar value and
  // gradient magnitude to opacity
  if ( this->UpdateColorLookup( vol ) || !this->AlphaLookupIndex )
    {    
    this->DeleteTextureIndex(&this->ColorLookupIndex);
    
    vtkgl::ActiveTexture( vtkgl::TEXTURE4 );
    glBindTexture(GL_TEXTURE_2D,0);
    this->DeleteTextureIndex(&this->AlphaLookupIndex);
    this->CreateTextureIndex(&this->AlphaLookupIndex);
    glBindTexture(GL_TEXTURE_2D, this->AlphaLookupIndex);   

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA8, 256, 256, 0,
          GL_ALPHA, GL_UNSIGNED_BYTE, this->AlphaLookup );      
    }

  vtkgl::ActiveTexture( vtkgl::TEXTURE4 );
  glBindTexture(GL_TEXTURE_2D, this->AlphaLookupIndex);   
  
  GLint loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureVol");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 7);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureVol1");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 6);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureVol2");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 5);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureAlphaLookup");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 4);
}

int  vtkSlicerGPURayCastVolumeTextureMapper3D::IsRenderSupported(vtkVolumeProperty *property )
{
  if ( !this->Initialized )
    {
    this->Initialize();
    }
  
  if ( !this->RayCastSupported )
    {
    return 0;
    }
  
  if ( !this->GetInput() )
    {
    return 0;
    }
  
  if ( this->GetInput()->GetNumberOfScalarComponents() > 1 &&
       property->GetIndependentComponents() )
    {
    return 0;
    }
  
  GLint num = 0;
  glGetIntegerv(vtkgl::MAX_TEXTURE_IMAGE_UNITS, &num);
  if (num < 8)//we use texture unit 4,5,6,7 to avoid conflict with slice planes
    return 0;
  
  num = 0;
  glGetIntegerv(vtkgl::MAX_FRAGMENT_UNIFORM_COMPONENTS, &num);
  if (num < 32)
    return 0; //at least we need 32 floats (two 4x4 matrices) in fragment shader
  return 1;
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::Initialize()
{
  this->Initialized = 1;
  vtkOpenGLExtensionManager * extensions = vtkOpenGLExtensionManager::New();
  extensions->SetRenderWindow(NULL); // set render window to the current one.
  
  int supports_2_0=extensions->ExtensionSupported( "GL_VERSION_2_0" );
  if(supports_2_0)
    {   
    extensions->LoadExtension("GL_VERSION_2_0");//printf("GL_2_0\n");
    }
  
  int supports_2_1=extensions->ExtensionSupported( "GL_VERSION_2_1" );
  if(supports_2_1)
    {   
    extensions->LoadExtension("GL_VERSION_2_1");//printf("GL_2_1\n");
    }
    
  int supports_3_0=extensions->ExtensionSupported( "GL_VERSION_3_0" );
  if(supports_3_0)
    {   
    extensions->LoadExtension("GL_VERSION_3_0");//printf("GL_3_0\n");
    }
    
    if (supports_2_0 || supports_2_1 || supports_3_0)
        RayCastSupported = 1;
    else
        RayCastSupported = 0;
    
    extensions->Delete();
    
//  GLint num;
//  glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &num);
//  printf("%d \n", num);
//  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &num);
//  printf("%d \n", num);
}

int vtkSlicerGPURayCastVolumeTextureMapper3D::IsTextureSizeSupported( int size[3] )
{
  if ( this->GetInput()->GetNumberOfScalarComponents() < 4 )
    {
    long maxSize = this->InternalVolumeSize * this->InternalVolumeSize * this->InternalVolumeSize;
            
    if ( size[0]*size[1]*size[2] > maxSize )//need to test graphics memory to determine volume size
      {
      return 0;
      }
    }
  else
    {
    if ( size[0]*size[1]*size[2] > 128*128*128 )
      {
      return 0;
      }
    }
    
  return 1;
}

// Print the vtkSlicerGPURayCastVolumeTextureMapper3D
void vtkSlicerGPURayCastVolumeTextureMapper3D::PrintSelf(ostream& os, vtkIndent indent)
{

  vtkOpenGLExtensionManager * extensions = vtkOpenGLExtensionManager::New();
  extensions->SetRenderWindow(NULL); // set render window to current render window
  
  os << indent << "Initialized " << this->Initialized << endl;
  if ( this->Initialized )
    {
    os << indent << "Supports GL_VERSION_1_2:" 
       << extensions->ExtensionSupported( "GL_VERSION_1_2" ) << endl;
    os << indent << "Supports GL_EXT_texture3D:" 
       << extensions->ExtensionSupported( "GL_EXT_texture3D" ) << endl;
    os << indent << "Supports GL_VERSION_1_3:" 
       << extensions->ExtensionSupported( "GL_VERSION_1_3" ) << endl;
    os << indent << "Supports GL_VERSION_1_5:" 
       << extensions->ExtensionSupported( "GL_VERSION_1_5" ) << endl;
    os << indent << "Supports GL_VERSION_2_0:" 
       << extensions->ExtensionSupported( "GL_VERSION_2_0" ) << endl;
    os << indent << "Supports GL_ARB_multitexture: " 
       << extensions->ExtensionSupported( "GL_ARB_multitexture" ) << endl;
    os << indent << "Supports GL_NV_texture_shader2: " 
       << extensions->ExtensionSupported( "GL_NV_texture_shader2" ) << endl;
    os << indent << "Supports GL_NV_register_combiners2: " 
       << extensions->ExtensionSupported( "GL_NV_register_combiners2" ) << endl;
    os << indent << "Supports GL_ATI_fragment_shader: " 
       << extensions->ExtensionSupported( "GL_ATI_fragment_shader" ) << endl;
    os << indent << "Supports GL_ARB_fragment_program: "
       << extensions->ExtensionSupported( "GL_ARB_fragment_program" ) << endl;
    }
  extensions->Delete();
  
  this->Superclass::PrintSelf(os,indent);
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::DrawVolumeBBox()
{       
    glBegin(GL_QUADS);      
        glColor3dv(VolumeBBoxVerticesColor[4]);
        glVertex3dv(VolumeBBoxVertices[4]);
        glColor3dv(VolumeBBoxVerticesColor[5]);
        glVertex3dv(VolumeBBoxVertices[5]);
        glColor3dv(VolumeBBoxVerticesColor[6]);
        glVertex3dv(VolumeBBoxVertices[6]);
        glColor3dv(VolumeBBoxVerticesColor[7]);
        glVertex3dv(VolumeBBoxVertices[7]);
        
        glColor3dv(VolumeBBoxVerticesColor[3]);
        glVertex3dv(VolumeBBoxVertices[3]);
        glColor3dv(VolumeBBoxVerticesColor[2]);
        glVertex3dv(VolumeBBoxVertices[2]);
        glColor3dv(VolumeBBoxVerticesColor[1]);
        glVertex3dv(VolumeBBoxVertices[1]);
        glColor3dv(VolumeBBoxVerticesColor[0]);
        glVertex3dv(VolumeBBoxVertices[0]);
        
        glColor3dv(VolumeBBoxVerticesColor[5]);
        glVertex3dv(VolumeBBoxVertices[5]);
        glColor3dv(VolumeBBoxVerticesColor[1]);
        glVertex3dv(VolumeBBoxVertices[1]);
        glColor3dv(VolumeBBoxVerticesColor[2]);
        glVertex3dv(VolumeBBoxVertices[2]);
        glColor3dv(VolumeBBoxVerticesColor[6]);
        glVertex3dv(VolumeBBoxVertices[6]);
        
        glColor3dv(VolumeBBoxVerticesColor[7]);
        glVertex3dv(VolumeBBoxVertices[7]);
        glColor3dv(VolumeBBoxVerticesColor[3]);
        glVertex3dv(VolumeBBoxVertices[3]);
        glColor3dv(VolumeBBoxVerticesColor[0]);
        glVertex3dv(VolumeBBoxVertices[0]);
        glColor3dv(VolumeBBoxVerticesColor[4]);
        glVertex3dv(VolumeBBoxVertices[4]);
        
        glColor3dv(VolumeBBoxVerticesColor[7]);
        glVertex3dv(VolumeBBoxVertices[7]);
        glColor3dv(VolumeBBoxVerticesColor[6]);
        glVertex3dv(VolumeBBoxVertices[6]);
        glColor3dv(VolumeBBoxVerticesColor[2]);
        glVertex3dv(VolumeBBoxVertices[2]);
        glColor3dv(VolumeBBoxVerticesColor[3]);
        glVertex3dv(VolumeBBoxVertices[3]);
        
        glColor3dv(VolumeBBoxVerticesColor[5]);
        glVertex3dv(VolumeBBoxVertices[5]);
        glColor3dv(VolumeBBoxVerticesColor[4]);
        glVertex3dv(VolumeBBoxVertices[4]);
        glColor3dv(VolumeBBoxVerticesColor[0]);
        glVertex3dv(VolumeBBoxVertices[0]);
        glColor3dv(VolumeBBoxVerticesColor[1]);
        glVertex3dv(VolumeBBoxVertices[1]);
  
        //also draw backface in case of negative scaling matrix
        glColor3dv(VolumeBBoxVerticesColor[2]);
        glVertex3dv(VolumeBBoxVertices[2]);
        glColor3dv(VolumeBBoxVerticesColor[3]);
        glVertex3dv(VolumeBBoxVertices[3]);
        glColor3dv(VolumeBBoxVerticesColor[0]);
        glVertex3dv(VolumeBBoxVertices[0]);
        glColor3dv(VolumeBBoxVerticesColor[1]);
        glVertex3dv(VolumeBBoxVertices[1]);
        
        glColor3dv(VolumeBBoxVerticesColor[6]);
        glVertex3dv(VolumeBBoxVertices[6]);
        glColor3dv(VolumeBBoxVerticesColor[7]);
        glVertex3dv(VolumeBBoxVertices[7]);
        glColor3dv(VolumeBBoxVerticesColor[3]);
        glVertex3dv(VolumeBBoxVertices[3]);
        glColor3dv(VolumeBBoxVerticesColor[2]);
        glVertex3dv(VolumeBBoxVertices[2]);
        
        glColor3dv(VolumeBBoxVerticesColor[7]);
        glVertex3dv(VolumeBBoxVertices[7]);
        glColor3dv(VolumeBBoxVerticesColor[4]);
        glVertex3dv(VolumeBBoxVertices[4]);
        glColor3dv(VolumeBBoxVerticesColor[0]);
        glVertex3dv(VolumeBBoxVertices[0]);
        glColor3dv(VolumeBBoxVerticesColor[3]);
        glVertex3dv(VolumeBBoxVertices[3]);
        
        glColor3dv(VolumeBBoxVerticesColor[5]);
        glVertex3dv(VolumeBBoxVertices[5]);
        glColor3dv(VolumeBBoxVerticesColor[6]);
        glVertex3dv(VolumeBBoxVertices[6]);
        glColor3dv(VolumeBBoxVerticesColor[2]);
        glVertex3dv(VolumeBBoxVertices[2]);
        glColor3dv(VolumeBBoxVerticesColor[1]);
        glVertex3dv(VolumeBBoxVertices[1]);
        
        glColor3dv(VolumeBBoxVerticesColor[6]);
        glVertex3dv(VolumeBBoxVertices[6]);
        glColor3dv(VolumeBBoxVerticesColor[5]);
        glVertex3dv(VolumeBBoxVertices[5]);
        glColor3dv(VolumeBBoxVerticesColor[4]);
        glVertex3dv(VolumeBBoxVertices[4]);
        glColor3dv(VolumeBBoxVerticesColor[7]);
        glVertex3dv(VolumeBBoxVertices[7]);
        
        glColor3dv(VolumeBBoxVerticesColor[5]);
        glVertex3dv(VolumeBBoxVertices[5]);
        glColor3dv(VolumeBBoxVerticesColor[1]);
        glVertex3dv(VolumeBBoxVertices[1]);
        glColor3dv(VolumeBBoxVerticesColor[0]);
        glVertex3dv(VolumeBBoxVertices[0]);
        glColor3dv(VolumeBBoxVerticesColor[4]);
        glVertex3dv(VolumeBBoxVertices[4]);
    glEnd();
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::InitializeRayCast()
{
    RayCastInitialized = 1;
    
    vtkgl::DeleteShader(this->RayCastVertexShader);
    vtkgl::DeleteShader(this->RayCastFragmentShader);
    vtkgl::DeleteProgram(this->RayCastProgram);
    
    this->RayCastVertexShader = vtkgl::CreateShader(vtkgl::VERTEX_SHADER);
    this->RayCastFragmentShader = vtkgl::CreateShader(vtkgl::FRAGMENT_SHADER);
    this->RayCastProgram = vtkgl::CreateProgram();
    
    LoadVertexShader();

    if (this->MIPRendering)
        LoadNoShadingFragmentShaderMIP();
    else
    {
        if (this->Shading)
            LoadFragmentShader();
        else
            LoadNoShadingFragmentShader();
    }
        
    LoadRayCastProgram();
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::LoadRayCastProgram()
{
  vtkgl::AttachShader(RayCastProgram, RayCastVertexShader);
  vtkgl::AttachShader(RayCastProgram, RayCastFragmentShader);
  vtkgl::LinkProgram(RayCastProgram);
  
  GLint result;
  vtkgl::GetProgramiv(RayCastProgram, vtkgl::LINK_STATUS, &result);
  
  if (!result)
    printf("Program Link Status: FALSE\n");
    
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::LoadVertexShader()
{
    std::ostringstream vp_oss;
    vp_oss <<
        "varying vec4 Position;                                                                     \n"
        "void main()                                                                            \n"
        "{                                                                                          \n"
        "    gl_Position = ftransform();                                                             \n"
        "    gl_TexCoord[0] = gl_Color;                                                              \n"
        "    Position = gl_Position;                                                             \n"
        "}                                                                                          \n";

        
    std::string source = vp_oss.str();
    const char* pSourceText = source.c_str();
    
    vtkgl::ShaderSource(RayCastVertexShader, 1, &pSourceText, NULL);
    vtkgl::CompileShader(RayCastVertexShader);
    
    GLint result;
    vtkgl::GetShaderiv(RayCastVertexShader, vtkgl::COMPILE_STATUS, &result);
    
    if (!result )
        printf("Vertex Shader Compile Status: FALSE\n");
        
    GLint infoLogLen;
    vtkgl::GetShaderiv(RayCastVertexShader, vtkgl::INFO_LOG_LENGTH, &infoLogLen);
    try
    {
        vtkgl::GLchar *pInfoLog = (vtkgl::GLchar*)malloc(sizeof(vtkgl::GLchar)*(infoLogLen+1));
        vtkgl::GetShaderInfoLog(RayCastVertexShader, infoLogLen, NULL, pInfoLog);
        printf("%s", pInfoLog);
    }catch(...)
    {
    }
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::LoadNoShadingFragmentShaderMIP()
{
    std::ostringstream fp_oss;
    fp_oss <<
        "varying vec4 Position;                                                                 \n"
        "uniform sampler3D TextureVol;                                                          \n"
        "uniform sampler3D TextureVol1;                                                         \n"
        "uniform sampler3D TextureVol2;                                                         \n"
        "uniform sampler2D TextureColorLookup;                                                  \n"
        "uniform sampler2D TextureAlphaLookup;                                                  \n"
        "uniform mat4 ParaMatrix;                                                                   \n"
        "uniform mat4 VolumeMatrix;                                                                 \n"
        "                                                                                        \n"
        "//ParaMatrix:                                                                          \n"
        "//EyePos.x,      EyePos.y,      EyePos.z,     Step                                         \n"
        "//VolBBoxLow.x,  VolBBoxLow.y,  VolBBoxLow.z, VolBBoxHigh.x                            \n"
        "//VolBBoxHigh.y, VolBBoxHigh.z, RenderMethod, N/A,                                 \n"
        "//N/A,           GlobalAlpha,   Debug,                                                 \n"
        "                                                                                        \n"
        "vec4 computeRayEnd()                                                                   \n"
        "{                                                                                          \n"
        "   vec3 o = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);                \n"
        "    vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);              \n"
        "    vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);              \n"
        "    mmn = clamp(mmn, 0.0, 1.0);                                                             \n"
        "    mmx = clamp(mmx, 0.0, 1.0);                                                             \n"     
        "                                                        \n"
        "    vec3 a1 = gl_TexCoord[0].xyz;                                                       \n"
        "    vec3 a2 = a1 + normalize(a1 - o) * length(mmx - mmn);                               \n"
        "    vec3 a3;                                                                            \n"
        "                                                                                        \n"
        "    float halfStep = ParaMatrix[0][3]*0.5;                                              \n"
        "    for (int i = 0; i < 10; i++)                                                        \n"
        "    {                                                                                       \n"
        "        a3 = (a1 + a2) * 0.5;                                                           \n"
        " //       if (length(a2 - a1) <= halfStep)                                                \n"
        " //           return vec4(clamp(a3, mmn, mmx), 1.0);                                      \n"
        "        if ( all(greaterThanEqual(a3, mmn)) && all(lessThanEqual(a3, mmx)) )            \n"
        "            a1 = a3;                                                                    \n"
        "        else                                                                            \n"
        "            a2 = a3;                                                                    \n"
        "    }                                                                                       \n"
        "    return vec4(clamp(a3, mmn, mmx), 1.0);                                              \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "vec4 computeRayOrigin()                                                                \n"
        "{                                                                                          \n"
        "    vec3 o = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);                \n"
        "    vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);              \n"
        "    vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);              \n"
        "    mmn = clamp(mmn, 0.0, 1.0);                                                             \n"
        "    mmx = clamp(mmx, 0.0, 1.0);                                                             \n"     
        "                                                        \n"
        "    if (all(greaterThanEqual(o, mmn)) && all(lessThanEqual(o, mmx)) )                       \n"
        "        return vec4(o, 1.0);                                                            \n"
        "    else                                                                                \n"
        "        return gl_TexCoord[0];                                                          \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "//perform 3D texture lookup based on RenderMethod                                          \n"
        "vec4 voxelColor(vec3 coord)                                                            \n"
        "{                                                                                          \n"
        "    vec4 color = vec4(0);                                                               \n"
        "    float renderMethod = ParaMatrix[2][2];                                              \n"
        "    if (renderMethod < 0.5)                                                             \n"
        "    {                                                                                       \n"
        "        vec4 scalar = texture3D(TextureVol, coord);                                         \n"
        "        color = texture2D(TextureColorLookup, vec2(scalar.w, scalar.x));                \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 1.5)                                                        \n"
        "    {                                                                                       \n"
        "        vec4 scalar = texture3D(TextureVol, coord);                                         \n"
        "        color = texture2D(TextureColorLookup, vec2(scalar.w, scalar.x));                \n"
        "        vec4 opacity = texture2D(TextureAlphaLookup, vec2(scalar.y, scalar.z));         \n"
        "        color.w = opacity.w;                                                            \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 2.5)                                                        \n"
        "    {                                                                                       \n"
        "        color = texture3D(TextureVol, coord);                                           \n"
        "        vec4 scalar = texture3D(TextureVol1, coord);                                    \n"
        "        vec4 opacity = texture2D(TextureAlphaLookup, vec2(scalar.w, scalar.x));         \n"
        "        color.w = opacity.w;                                                            \n"
        "    }                                                                                       \n"
        "    return color;                                                                       \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "float voxelScalar(vec3 coord)                                                          \n"
        "{                                                                                          \n"
        "    float renderMethod = ParaMatrix[2][2];                                              \n"
        "    if (renderMethod < 0.5)                                                             \n"
        "    {                                                                                       \n"
        "        vec4 scalar = texture3D(TextureVol, coord);                                         \n"
        "        return scalar.w;                                                                \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 1.5)                                                        \n"
        "    {                                                                                       \n"
        "        vec4 scalar = texture3D(TextureVol, coord);                                         \n"
        "        return scalar.w;                                                                \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 2.5)                                                        \n"
        "    {                                                                                       \n"
        "        vec4 color = texture3D(TextureVol, coord);                                          \n"
        "        vec4 scalar = texture3D(TextureVol1, coord);                                    \n"
        "        return scalar.w;                                                                \n"
        "    }                                                                                       \n"
        "    return 0.0;                                                                             \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "void main()                                                                            \n"
        "{                                                                      \n"
        "    vec4 rayOrigin = computeRayOrigin();                                                \n"
        "    vec4 rayEnd = computeRayEnd();                                                      \n"
        "    vec3 rayDir = rayEnd.xyz - rayOrigin.xyz;                                               \n"
        "    float rayLen = length(rayDir);                                                      \n"
        "    rayDir = normalize(rayDir);                                                             \n"
        "                                                                                        \n"
        "    //debug mode                                                                        \n"
        "    if (ParaMatrix[3][2] > 1.5)                                                             \n"                                      
        "    {                                                                                       \n"
        "        gl_FragColor = rayEnd;                                                          \n"
        "        return;                                                                         \n"
        "    }                                                                                       \n"
        "    else if (ParaMatrix[3][2] > 0.5)                                                    \n"
        "    {                                                                                       \n"
        "        gl_FragColor = rayOrigin;                                                           \n"
        "        return;                                                                         \n"
        "    }                                                                                       \n"
        "                                                                                        \n"
        "    //do ray casting                                                                    \n"
        "    vec3 rayStep = rayDir*ParaMatrix[0][3];                                             \n"
        "    vec3 nextRayOrigin = rayOrigin.xyz;                                                     \n"
        "                                                                                        \n"
        "    vec4 pixelColor = vec4(0);                                                          \n"
        "    float alpha = 0.0;                                                                      \n"
        "    float t = 0.0;                                                                      \n"
        "                                                                                        \n"
        "    {                                                                                       \n"
        "        float maxScalar = voxelScalar(nextRayOrigin);                                   \n"
        "        vec3 maxScalarCoord = nextRayOrigin;                                            \n"
        "        while( t < rayLen )                                                                 \n"
        "        {                                                                                   \n"
        "            float scalar = voxelScalar(nextRayOrigin);                                      \n"
        "            if (maxScalar < scalar)                                                     \n"
        "            {                                                                               \n"
        "                maxScalar = scalar;                                                         \n"
        "                maxScalarCoord = nextRayOrigin;                                         \n"
        "            }                                                                               \n"
        "                                                                                        \n"
        "            t += ParaMatrix[0][3];                                                      \n"
        "            nextRayOrigin += rayStep;                                                       \n"
        "        }                                                                                   \n"
        "                                                                                        \n"
        "        pixelColor = voxelColor(maxScalarCoord);                                           \n"
        "        alpha = pixelColor.w;                                                                \n"
        "    }                                                                                       \n"
        "    gl_FragColor = vec4(pixelColor.xyz, alpha*ParaMatrix[3][1]);                        \n"
        "                                                                                        \n"
        "}                                                                                          \n";
        
    std::string source = fp_oss.str();
    const char* pSourceText = source.c_str();
    
    vtkgl::ShaderSource(RayCastFragmentShader, 1, &pSourceText, NULL);
    vtkgl::CompileShader(RayCastFragmentShader);
    
    GLint result;
    vtkgl::GetShaderiv(RayCastFragmentShader, vtkgl::COMPILE_STATUS, &result);
    
    if (!result)
        printf("Fragment Shader Compile Status: FALSE\n");
        
    GLint infoLogLen;
    vtkgl::GetShaderiv(RayCastFragmentShader, vtkgl::INFO_LOG_LENGTH, &infoLogLen);
    try
    {
        vtkgl::GLchar *pInfoLog = (vtkgl::GLchar*)malloc(sizeof(vtkgl::GLchar)*(infoLogLen+1));
        vtkgl::GetShaderInfoLog(RayCastFragmentShader, infoLogLen, NULL, pInfoLog);
        printf("%s", pInfoLog);
    }catch(...)
    {
    }
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::LoadNoShadingFragmentShader()
{
    std::ostringstream fp_oss;
    fp_oss <<
        "varying vec4 Position;                                                                 \n"
        "uniform sampler3D TextureVol;                                                          \n"
        "uniform sampler3D TextureVol1;                                                         \n"
        "uniform sampler3D TextureVol2;                                                         \n"
        "uniform sampler2D TextureColorLookup;                                                  \n"
        "uniform sampler2D TextureAlphaLookup;                                                  \n"
        "uniform mat4 ParaMatrix;                                                                   \n"
        "uniform mat4 VolumeMatrix;                                                                 \n"
        "                                                                                        \n"
        "//ParaMatrix:                                                                          \n"
        "//EyePos.x,      EyePos.y,      EyePos.z,     Step                                         \n"
        "//VolBBoxLow.x,  VolBBoxLow.y,  VolBBoxLow.z, VolBBoxHigh.x                            \n"
        "//VolBBoxHigh.y, VolBBoxHigh.z, RenderMethod, DepthPeelingThreshold,                   \n"
        "//N/A,           GlobalAlpha,   Debug,                                                 \n"
        "                                                                                        \n"
        "vec4 computeRayEnd()                                                                   \n"
        "{                                                                                          \n"
        "   vec3 o = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);                \n"
        "    vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);              \n"
        "    vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);              \n"
        "    mmn = clamp(mmn, 0.0, 1.0);                                                             \n"
        "    mmx = clamp(mmx, 0.0, 1.0);                                                             \n"     
        "                                                        \n"
        "    vec3 a1 = gl_TexCoord[0].xyz;                                                       \n"
        "    vec3 a2 = a1 + normalize(a1 - o) * length(mmx - mmn);                               \n"
        "    vec3 a3;                                                                            \n"
        "                                                                                        \n"
        "    float halfStep = ParaMatrix[0][3]*0.5;                                              \n"
        "    for (int i = 0; i < 10; i++)                                                        \n"
        "    {                                                                                       \n"
        "        a3 = (a1 + a2) * 0.5;                                                           \n"
        "//        if (length(a2 - a1) <= halfStep)                                                \n"
        "//            return vec4(clamp(a3, mmn, mmx), 1.0);                                      \n"
        "        if ( all(greaterThanEqual(a3, mmn)) && all(lessThanEqual(a3, mmx)) )            \n"
        "            a1 = a3;                                                                    \n"
        "        else                                                                            \n"
        "            a2 = a3;                                                                    \n"
        "    }                                                                                       \n"
        "    return vec4(clamp(a3, mmn, mmx), 1.0);                                              \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "vec4 computeRayOrigin()                                                                \n"
        "{                                                                                          \n"
        "    vec3 o = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);                \n"
        "    vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);              \n"
        "    vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);              \n"
        "    mmn = clamp(mmn, 0.0, 1.0);                                                             \n"
        "    mmx = clamp(mmx, 0.0, 1.0);                                                             \n"     
        "                                                        \n"
        "    if (all(greaterThanEqual(o, mmn)) && all(lessThanEqual(o, mmx)) )                       \n"
        "        return vec4(o, 1.0);                                                            \n"
        "    else                                                                                \n"
        "        return gl_TexCoord[0];                                                          \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "//perform 3D texture lookup based on RenderMethod                                          \n"
        "vec4 voxelColor(vec3 coord)                                                            \n"
        "{                                                                                          \n"
        "    vec4 color = vec4(0);                                                               \n"
        "    float renderMethod = ParaMatrix[2][2];                                              \n"
        "    if (renderMethod < 0.5)                                                             \n"
        "    {                                                                                       \n"
        "        vec4 scalar = texture3D(TextureVol, coord);                                         \n"
        "        color = texture2D(TextureColorLookup, vec2(scalar.w, scalar.x));                \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 1.5)                                                        \n"
        "    {                                                                                       \n"
        "        vec4 scalar = texture3D(TextureVol, coord);                                         \n"
        "        color = texture2D(TextureColorLookup, vec2(scalar.w, scalar.x));                \n"
        "        vec4 opacity = texture2D(TextureAlphaLookup, vec2(scalar.y, scalar.z));         \n"
        "        color.w = opacity.w;                                                            \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 2.5)                                                        \n"
        "    {                                                                                       \n"
        "        color = texture3D(TextureVol, coord);                                           \n"
        "        vec4 scalar = texture3D(TextureVol1, coord);                                    \n"
        "        vec4 opacity = texture2D(TextureAlphaLookup, vec2(scalar.w, scalar.x));         \n"
        "        color.w = opacity.w;                                                            \n"
        "    }                                                                                       \n"
        "    return color;                                                                       \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "float voxelScalar(vec3 coord)                                                          \n"
        "{                                                                                          \n"
        "    float renderMethod = ParaMatrix[2][2];                                              \n"
        "    if (renderMethod < 0.5)                                                             \n"
        "    {                                                                                       \n"
        "        vec4 scalar = texture3D(TextureVol, coord);                                         \n"
        "        return scalar.w;                                                                \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 1.5)//use the first one                                     \n"
        "    {                                                                                       \n"
        "        vec4 scalar = texture3D(TextureVol, coord);                                         \n"
        "        return scalar.w;                                                                \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 2.5)//use the first one                                     \n"
        "    {                                                                                       \n"
        "        vec4 color = texture3D(TextureVol, coord);                                          \n"
        "        vec4 scalar = texture3D(TextureVol1, coord);                                    \n"
        "        return scalar.w;                                                                \n"
        "    }                                                                                       \n"
        "    return 0.0;                                                                             \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "void main()                                                                            \n"
        "{                                                                      \n"
        "    vec4 rayOrigin = computeRayOrigin();                                                \n"
        "    vec4 rayEnd = computeRayEnd();                                                      \n"
        "    vec3 rayDir = rayEnd.xyz - rayOrigin.xyz;                                               \n"
        "    float rayLen = length(rayDir);                                                      \n"
        "    rayDir = normalize(rayDir);                                                             \n"
        "                                                                                        \n"
        "    //debug mode                                                                        \n"
        "    if (ParaMatrix[3][2] > 1.5)                                                             \n"                                      
        "    {                                                                                       \n"
        "        gl_FragColor = rayEnd;                                                          \n"
        "        return;                                                                         \n"
        "    }                                                                                       \n"
        "    else if (ParaMatrix[3][2] > 0.5)                                                    \n"
        "    {                                                                                       \n"
        "        gl_FragColor = rayOrigin;                                                           \n"
        "        return;                                                                         \n"
        "    }                                                                                       \n"
        "                                                                                        \n"
        "    //do ray casting                                                                    \n"
        "    vec3 rayStep = rayDir*ParaMatrix[0][3];                                             \n"
        "    vec3 nextRayOrigin = rayOrigin.xyz;                                                     \n"
        "                                                                                        \n"
        "    vec4 pixelColor = vec4(0);                                                          \n"
        "    float alpha = 0.0;                                                                      \n"
        "    float t = 0.0;                                                                      \n"
        "    float depthPeeling = ParaMatrix[2][3];                                                  \n"
        "    {                                                                                       \n"
        "        while( t < rayLen)                                                                \n"
        "        {                                                                                  \n"
        "           vec4 nextColor = voxelColor(nextRayOrigin);                                    \n"
        "           if ( nextColor.w >= depthPeeling )                                                  \n"
        "               break;                                                                        \n"
        "           t += ParaMatrix[0][3];                                                      \n"
        "           nextRayOrigin += rayStep;                                                       \n"
        "        }                                                                                 \n"
        "        while( (t < rayLen) && (alpha < 1.0) )                                          \n"
        "        {                                                                                   \n"
        "            vec4 nextColor = voxelColor(nextRayOrigin);                                     \n"
        "            float tempAlpha = nextColor.w;                                              \n"
        "                                                                                           \n"
        "            tempAlpha = (1.0-alpha)*tempAlpha;                                              \n"
        "            pixelColor += nextColor*tempAlpha;                                              \n"
        "            alpha += tempAlpha;                                                             \n"
        "                                                                                     \n"
        "            t += ParaMatrix[0][3];                                                      \n"
        "            nextRayOrigin += rayStep;                                                       \n"
        "        }                                                                                   \n"
        "    }                                                                                       \n"
        "    gl_FragColor = vec4(pixelColor.xyz, alpha*ParaMatrix[3][1]);                        \n"
        "                                                                                        \n"
        "}                                                                                          \n";
        
    std::string source = fp_oss.str();
    const char* pSourceText = source.c_str();
    
    vtkgl::ShaderSource(RayCastFragmentShader, 1, &pSourceText, NULL);
    vtkgl::CompileShader(RayCastFragmentShader);
    
    GLint result;
    vtkgl::GetShaderiv(RayCastFragmentShader, vtkgl::COMPILE_STATUS, &result);
    
    if (!result)
        printf("Fragment Shader Compile Status: FALSE\n");
        
    GLint infoLogLen;
    vtkgl::GetShaderiv(RayCastFragmentShader, vtkgl::INFO_LOG_LENGTH, &infoLogLen);
    try
    {
        vtkgl::GLchar *pInfoLog = (vtkgl::GLchar*)malloc(sizeof(vtkgl::GLchar)*(infoLogLen+1));
        vtkgl::GetShaderInfoLog(RayCastFragmentShader, infoLogLen, NULL, pInfoLog);
        printf("%s", pInfoLog);
    }catch(...)
    {
    }
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::LoadFragmentShader()
{
    std::ostringstream fp_oss;
    fp_oss <<
        "varying vec4 Position;                                                                 \n"
        "uniform sampler3D TextureVol;                                                          \n"
        "uniform sampler3D TextureVol1;                                                         \n"
        "uniform sampler3D TextureVol2;                                                         \n"
        "uniform sampler2D TextureColorLookup;                                                  \n"
        "uniform sampler2D TextureAlphaLookup;                                                  \n"
        "uniform mat4 ParaMatrix;                                                                   \n"
        "uniform mat4 VolumeMatrix;                                                                 \n"
        "                                                                                        \n"
        "//ParaMatrix:                                                                          \n"
        "//EyePos.x,      EyePos.y,      EyePos.z,     Step                                         \n"
        "//VolBBoxLow.x,  VolBBoxLow.y,  VolBBoxLow.z, VolBBoxHigh.x                            \n"
        "//VolBBoxHigh.y, VolBBoxHigh.z, RenderMethod, DepthPeelingThreshold,                     \n"
        "//MIP,           GlobalAlpha,   Debug,                                                 \n"
        "                                                                                        \n"
        "vec4 computeRayEnd()                                                                   \n"
        "{                                                                                          \n"
        "   vec3 o = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);                \n"
        "    vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);              \n"
        "    vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);              \n"
        "    mmn = clamp(mmn, 0.0, 1.0);                                                             \n"
        "    mmx = clamp(mmx, 0.0, 1.0);                                                             \n"     
        "                                                        \n"
        "    vec3 a1 = gl_TexCoord[0].xyz;                                                       \n"
        "    vec3 a2 = a1 + normalize(a1 - o) * length(mmx - mmn);                               \n"
        "    vec3 a3;                                                                            \n"
        "                                                                                        \n"
        "    float halfStep = ParaMatrix[0][3]*0.5;                                              \n"
        "    for (int i = 0; i < 10; i++)                                                        \n"
        "    {                                                                                       \n"
        "        a3 = (a1 + a2) * 0.5;                                                           \n"
        " //       if (length(a2 - a1) <= halfStep)                                                \n"
        " //           return vec4(clamp(a3, mmn, mmx), 1.0);                                      \n"
        "        if ( all(greaterThanEqual(a3, mmn)) && all(lessThanEqual(a3, mmx)) )            \n"
        "            a1 = a3;                                                                    \n"
        "        else                                                                            \n"
        "            a2 = a3;                                                                    \n"
        "    }                                                                                       \n"
        "    return vec4(clamp(a3, mmn, mmx), 1.0);                                              \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "vec4 computeRayOrigin()                                                                \n"
        "{                                                                                          \n"
        "    vec3 o = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);                \n"
        "    vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);              \n"
        "    vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);              \n"
        "    mmn = clamp(mmn, 0.0, 1.0);                                                             \n"
        "    mmx = clamp(mmx, 0.0, 1.0);                                                             \n"     
        "                                                        \n"
        "    if (all(greaterThanEqual(o, mmn)) && all(lessThanEqual(o, mmx)) )                       \n"
        "        return vec4(o, 1.0);                                                            \n"
        "    else                                                                                \n"
        "        return gl_TexCoord[0];                                                          \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "//perform 3D texture lookup based on RenderMethod                                          \n"
        "vec4 voxelColor(vec3 coord)                                                            \n"
        "{                                                                                          \n"
        "    vec4 color = vec4(0);                                                               \n"
        "    float renderMethod = ParaMatrix[2][2];                                              \n"
        "    if (renderMethod < 0.5)                                                             \n"
        "    {                                                                                       \n"
        "        vec4 scalar = texture3D(TextureVol, coord);                                         \n"
        "        color = texture2D(TextureColorLookup, vec2(scalar.w, scalar.x));                \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 1.5)                                                        \n"
        "    {                                                                                       \n"
        "        vec4 scalar = texture3D(TextureVol, coord);                                         \n"
        "        color = texture2D(TextureColorLookup, vec2(scalar.w, scalar.x));                \n"
        "        vec4 opacity = texture2D(TextureAlphaLookup, vec2(scalar.y, scalar.z));         \n"
        "        color.w = opacity.w;                                                            \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 2.5)                                                        \n"
        "    {                                                                                       \n"
        "        color = texture3D(TextureVol, coord);                                           \n"
        "        vec4 scalar = texture3D(TextureVol1, coord);                                    \n"
        "        vec4 opacity = texture2D(TextureAlphaLookup, vec2(scalar.w, scalar.x));         \n"
        "        color.w = opacity.w;                                                            \n"
        "    }                                                                                       \n"
        "    return color;                                                                       \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "float voxelScalar(vec3 coord)                                                          \n"
        "{                                                                                          \n"
        "    float renderMethod = ParaMatrix[2][2];                                              \n"
        "    if (renderMethod < 0.5)                                                             \n"
        "    {                                                                                       \n"
        "        vec4 scalar = texture3D(TextureVol, coord);                                         \n"
        "        return scalar.w;                                                                \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 1.5)//use the first one                                     \n"
        "    {                                                                                       \n"
        "        vec4 scalar = texture3D(TextureVol, coord);                                         \n"
        "        return scalar.w;                                                                \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 2.5)//use the first one                                     \n"
        "    {                                                                                       \n"
        "        vec4 color = texture3D(TextureVol, coord);                                          \n"
        "        vec4 scalar = texture3D(TextureVol1, coord);                                    \n"
        "        return scalar.w;                                                                \n"
        "    }                                                                                       \n"
        "    return 0.0;                                                                             \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "vec3 voxelNormal(vec3 coord)                                                           \n"
        "{                                                                                          \n"
        "    vec4 normal = vec4(0);                                                              \n"
        "    float renderMethod = ParaMatrix[2][2];                                              \n"
        "    if (renderMethod < 0.5)                                                             \n"
        "    {                                                                                       \n"
        "        normal = texture3D(TextureVol1, coord);                                         \n"
        "        normal = normal * 2.0 - 1.0;                                                    \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 1.5)                                                        \n"
        "    {                                                                                       \n"
        "        normal = texture3D(TextureVol1, coord);                                         \n"
        "        normal = normal * 2.0 - 1.0;                                                    \n"
        "    }                                                                                       \n"
        "    else if (renderMethod < 2.5)                                                        \n"
        "    {                                                                                       \n"
        "        normal = texture3D(TextureVol2, coord);                                         \n"
        "        normal = normal * 2.0 - 1.0;                                                    \n"
        "    }                                                                                       \n"
        "    normal = normalize(VolumeMatrix * normal);                                              \n"
        "    return gl_NormalMatrix * normal.xyz;                                                \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "vec4 directionalLight(vec3 coord, vec3 lightDir)                                                          \n"
        "{                                                                                          \n"
        "    vec3    normal = normalize(voxelNormal(coord));                                     \n"
        "    float   NdotL = max( dot( normal, lightDir ), 0.0 );                                \n"
        "    if ( NdotL > 0.0 )                                                                      \n"
        "    {                                                                                       \n"
        "        float   NdotHV = max( dot( normal, gl_LightSource[0].halfVector.xyz), 0.0);             \n" 
        "        vec4 specular = gl_LightSource[0].specular * pow(NdotHV, 40.0); \n"
        "        return (specular + gl_LightSource[0].diffuse * NdotL + gl_LightSource[0].ambient);          \n"
        "    }                                                                                       \n"
        "    else                                                                                \n"
        "        return vec4(1.0);                                                                   \n"
        "}                                                                                          \n"
        "                                                                                        \n"
        "void main()                                                                            \n"
        "{                                                                      \n"
        "    vec4 rayOrigin = computeRayOrigin();                                                \n"
        "    vec4 rayEnd = computeRayEnd();                                                      \n"
        "    vec3 rayDir = rayEnd.xyz - rayOrigin.xyz;                                               \n"
        "    float rayLen = length(rayDir);                                                      \n"
        "    rayDir = normalize(rayDir);                                                             \n"
        "                                                                                        \n"
        "    //debug mode                                                                        \n"
        "    if (ParaMatrix[3][2] > 1.5)                                                             \n"                                      
        "    {                                                                                       \n"
        "        gl_FragColor = rayEnd;                                                          \n"
        "        return;                                                                         \n"
        "    }                                                                                       \n"
        "    else if (ParaMatrix[3][2] > 0.5)                                                    \n"
        "    {                                                                                       \n"
        "        gl_FragColor = rayOrigin;                                                           \n"
        "        return;                                                                         \n"
        "    }                                                                                       \n"
        "                                                                                        \n"
        "    //do ray casting                                                                    \n"
        "    vec3 rayStep = rayDir*ParaMatrix[0][3];                                             \n"
        "    vec3 nextRayOrigin = rayOrigin.xyz;                                                     \n"
        "                                                                                        \n"
        "    vec4 pixelColor = vec4(0);                                                          \n"
        "    float alpha = 0.0;                                                                      \n"
        "    float t = 0.0;                                                                      \n"
        "    vec3  lightDir = normalize( gl_LightSource[0].position.xyz );                     \n"
        "    float depthPeeling = ParaMatrix[2][3];                                                  \n"
        "                                                                                        \n"
        "    {                                                                                       \n"
        "        while( t < rayLen)                                                                \n"
        "        {                                                                                  \n"
        "           vec4 nextColor = voxelColor(nextRayOrigin);                                    \n"
        "           if ( nextColor.w >= depthPeeling )                                                  \n"
        "               break;                                                                        \n"
        "           t += ParaMatrix[0][3];                                                      \n"
        "           nextRayOrigin += rayStep;                                                       \n"
        "        }                                                                                 \n"
        "        while( (t < rayLen) && (alpha < 1.0) )                                          \n"
        "        {                                                                                   \n"
        "            vec4 nextColor = voxelColor(nextRayOrigin);                                     \n"
        "            float tempAlpha = nextColor.w;                                              \n"
        "                                                                                        \n"
        "            nextColor *= directionalLight(nextRayOrigin, lightDir);                                \n"
        "                                                                                        \n"      
        "            tempAlpha = (1.0-alpha)*tempAlpha;                                              \n"
        "            pixelColor += nextColor*tempAlpha;                                              \n"
        "            alpha += tempAlpha;                                                             \n"
        "                                                                                     \n"
        "            t += ParaMatrix[0][3];                                                      \n"
        "            nextRayOrigin += rayStep;                                                       \n"
        "        }                                                                                   \n"
        "    }                                                                                       \n"
        "    gl_FragColor = vec4(pixelColor.xyz, alpha*ParaMatrix[3][1]);                        \n"
        "                                                                                        \n"
        "}                                                                                          \n";
        
    std::string source = fp_oss.str();
    const char* pSourceText = source.c_str();
    
    vtkgl::ShaderSource(RayCastFragmentShader, 1, &pSourceText, NULL);
    vtkgl::CompileShader(RayCastFragmentShader);
    
    GLint result;
    vtkgl::GetShaderiv(RayCastFragmentShader, vtkgl::COMPILE_STATUS, &result);
    
    if (!result)
        printf("Fragment Shader Compile Status: FALSE\n");
        
    GLint infoLogLen;
    vtkgl::GetShaderiv(RayCastFragmentShader, vtkgl::INFO_LOG_LENGTH, &infoLogLen);
    try
    {
        vtkgl::GLchar *pInfoLog = (vtkgl::GLchar*)malloc(sizeof(vtkgl::GLchar)*(infoLogLen+1));
        vtkgl::GetShaderInfoLog(RayCastFragmentShader, infoLogLen, NULL, pInfoLog);
        printf("%s", pInfoLog);
    }catch(...)
    {
    }
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::PrintGLErrorString()
{
    GLenum error = glGetError();
        
    switch(error)
    {
    case GL_NO_ERROR: printf("GL_NO_ERROR\n");break;
    case GL_INVALID_ENUM: printf("GL_INVALID_ENUM\n");break;
    case GL_INVALID_VALUE: printf("GL_INVALID_VALUE\n");break;
    case GL_INVALID_OPERATION: printf("GL_INVALID_OPERATION\n");break;
        
    case GL_STACK_OVERFLOW: printf("GL_STACK_OVERFLOW\n");break;
    case GL_STACK_UNDERFLOW: printf("GL_STACK_UNDERFLOW\n");break;
    case GL_OUT_OF_MEMORY: printf("GL_OUT_OF_MEMORY\n");break;
    }
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::PrintFragmentShaderInfoLog()
{
    GLint infoLogLen;
    vtkgl::GetShaderiv(RayCastFragmentShader, vtkgl::INFO_LOG_LENGTH, &infoLogLen);
    try
    {
        vtkgl::GLchar *pInfoLog = (vtkgl::GLchar*)malloc(sizeof(vtkgl::GLchar)*(infoLogLen+1));
        vtkgl::GetShaderInfoLog(RayCastFragmentShader, infoLogLen, NULL, pInfoLog);
        printf("%s", pInfoLog);
    }catch(...)
    {
    }
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::MIPRenderingOff()
{
    this->MIPRendering = 0;
    this->ReloadShaderFlag = 1;
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::MIPRenderingOn()
{
    this->MIPRendering = 1;
    this->ReloadShaderFlag = 1;
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::ShadingOff()
{
    this->Shading = 0;
    this->ReloadShaderFlag = 1;
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::ShadingOn()
{
    this->Shading = 1;
    this->ReloadShaderFlag = 1;
}

void vtkSlicerGPURayCastVolumeTextureMapper3D::SetInternalVolumeSize(int size)
{
    if (this->InternalVolumeSize != size)
    {
        this->InternalVolumeSize = size;
        this->SavedTextureInput = NULL;//dirty input, force reprocess input
        this->RayCastInitialized = 0;
    }
}


