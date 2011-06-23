/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerGPURayCastMultiVolumeMapper.h,v $

   Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkSlicerGPURayCastMultiVolumeMapper
  Module:    $HeadURL: http://www.na-mic.org/svn/Slicer3/ $
  Date:      $Date: 2009-01-07 09:26:53 -0500 (Tue, 30 Jan 2009) $
  Version:   $Revision:  $


=========================================================================*/
#include <string.h>
#include <iostream>
#include <sstream>

#include "vtkWindows.h"
#include "vtkSlicerGPURayCastMultiVolumeMapper.h"

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
vtkCxxRevisionMacro(vtkSlicerGPURayCastMultiVolumeMapper, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkSlicerGPURayCastMultiVolumeMapper);
//#endif

vtkSlicerGPURayCastMultiVolumeMapper::vtkSlicerGPURayCastMultiVolumeMapper()
{
  this->Initialized          =  0;
  this->RayCastInitialized       =  0;
  this->Technique            =  0;//by default composit shading
  this->TechniqueFg            =  0;//by default composit shading
  this->ColorOpacityFusion   =  0;
  this->FgBgRatio            =  0.0;

  this->Clipping             =  0;
  this->ReloadShaderFlag     =  0;
  this->InternalVolumeSize      =  256; //by default 256^3

  this->Volume1Index         =  0;

  this->ColorLookupIndex         =  0;
  this->ColorLookup2Index         =  0;
  this->RayCastVertexShader      =  0;
  this->RayCastFragmentShader    =  0;
  this->RayCastProgram           =  0;
  this->RayCastSupported         =  0;
  this->RenderWindow         = NULL;
  this->RaySteps             = 450.0f;

  this->GlobalAlpha          = 1.0f;
}

vtkSlicerGPURayCastMultiVolumeMapper::~vtkSlicerGPURayCastMultiVolumeMapper()
{
}

// Release the graphics resources used by this texture.
void vtkSlicerGPURayCastMultiVolumeMapper::ReleaseGraphicsResources(vtkWindow
                                *renWin)
{
  if (( this->Volume1Index || this->ColorLookupIndex || this->ColorLookup2Index) && renWin)
    {
    static_cast<vtkRenderWindow *>(renWin)->MakeCurrent();
#ifdef GL_VERSION_1_1
    // free any textures
    this->DeleteTextureIndex( &this->Volume1Index );
    this->DeleteTextureIndex( &this->ColorLookupIndex );
    this->DeleteTextureIndex( &this->ColorLookup2Index );
#endif
    }
  if ( this->RayCastVertexShader || this->RayCastFragmentShader || this->RayCastProgram)
  {
    vtkgl::DeleteShader(this->RayCastVertexShader);
    vtkgl::DeleteShader(this->RayCastFragmentShader);
    vtkgl::DeleteProgram(this->RayCastProgram);
  }

  this->Volume1Index     = 0;

  this->ColorLookupIndex = 0;
  this->ColorLookup2Index = 0;
  this->RayCastVertexShader   = 0;
  this->RayCastFragmentShader = 0;
  this->RayCastProgram    = 0;
  this->RenderWindow     = NULL;
  this->Modified();
}

void vtkSlicerGPURayCastMultiVolumeMapper::Render(vtkRenderer *ren, vtkVolume *vol)
{
  ren->GetRenderWindow()->MakeCurrent();

  if ( !this->Initialized )
    {
    this->Initialize(ren->GetRenderWindow());
    }

  if ( !this->RayCastInitialized || this->ReloadShaderFlag)
    {
    this->InitializeRayCast();
    }

  //adjust ray steps based on requrestd frame rate
  if ( this->TimeToDraw == 0.0 )
  {
    this->TimeToDraw = 0.0001;
  }
  
  this->AdaptivePerformanceControl();
  
  glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);

  //setup material based on volume property
  float ambient = vol->GetProperty()->GetAmbient() * 1.2f;
  float specular = vol->GetProperty()->GetSpecular();
  float diffuse = vol->GetProperty()->GetDiffuse() * 1.2f;
  float power = 0.75f*128*vol->GetProperty()->GetSpecularPower()/50;

  float ambientMaterial[4];
  float diffuseMaterial[4];
  float specularMaterial[4];

  ambientMaterial[0] = ambient; ambientMaterial[1] = ambient; ambientMaterial[2] = ambient; ambientMaterial[3] = 1.0;
  diffuseMaterial[0] = diffuse; diffuseMaterial[1] = diffuse; diffuseMaterial[2] = diffuse; diffuseMaterial[3] = 1.0;
  specularMaterial[0] = specular; specularMaterial[1] = specular; specularMaterial[2] = specular; specularMaterial[3] = 1.0;

  glMaterialfv(GL_FRONT, GL_AMBIENT, ambientMaterial);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);
  glMaterialfv(GL_FRONT, GL_SPECULAR, specularMaterial);
  glMaterialf(GL_FRONT, GL_SHININESS, power);

  glDisable(GL_LIGHTING);

  this->RenderGLSL(ren, vol);

  glPopAttrib();

  this->TimeToDraw = static_cast<float>(this->Timer->GetElapsedTime());

  //printf("ray step: %f, fps: %f\n", this->RaySteps, 1.0/this->TimeToDraw);

//  double progress = 1;
//  this->InvokeEvent(vtkCommand::VolumeMapperRenderProgressEvent, &progress);
}

void vtkSlicerGPURayCastMultiVolumeMapper::AdaptivePerformanceControl()
{
  //do automatic performance control
  if(this->Framerate <= 0.0f)
    this->Framerate = 1.0f;

  float targetTime = 1.0/this->Framerate;

  if (fabs(targetTime - this->TimeToDraw) < 0.1*targetTime)
    return;
    
  this->RaySteps *= targetTime/(this->TimeToDraw*1.5);

  int dim[3];
  this->GetVolumeDimensions(dim);

  float maxRaysteps = dim[0];
  maxRaysteps = maxRaysteps > dim[1] ? maxRaysteps : dim[1];
  maxRaysteps = maxRaysteps > dim[2] ? maxRaysteps : dim[2];
  maxRaysteps *= 1.8f; //make sure we have enough sampling rate to recover details

//  maxRaysteps = maxRaysteps < 1050.0f ? 1050.0f : maxRaysteps;//ensure high sampling rate on low resolution volumes

  // add clamp
  if (this->RaySteps > maxRaysteps) this->RaySteps = maxRaysteps;
  if (this->RaySteps < 200.0f)       this->RaySteps = 200.0f;
}

//needs to be cleaned, 2008/10/20, Yanling Liu
void vtkSlicerGPURayCastMultiVolumeMapper::SetupRayCastParameters(vtkRenderer *vtkNotUsed(pRen),
                                                             vtkVolume *pVol)
{
  double bounds[6];
  this->GetNthInput(0)->GetBounds(bounds);

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

      plane = static_cast<vtkPlane *>(clipPlanes->GetItemAsObject(0));
      pNormal = plane->GetNormal();
      pOrigin = plane->GetOrigin();

      lowerBounds[0] = pOrigin[0];

      plane = static_cast<vtkPlane *>(clipPlanes->GetItemAsObject(1));
      pNormal = plane->GetNormal();
      pOrigin = plane->GetOrigin();

      upperBounds[0] = pOrigin[0];

      plane = static_cast<vtkPlane *>(clipPlanes->GetItemAsObject(2));
      pNormal = plane->GetNormal();
      pOrigin = plane->GetOrigin();

      lowerBounds[1] = pOrigin[1];

      plane = static_cast<vtkPlane *>(clipPlanes->GetItemAsObject(3));
      pNormal = plane->GetNormal();
      pOrigin = plane->GetOrigin();

      upperBounds[1] = pOrigin[1];

      plane = static_cast<vtkPlane *>(clipPlanes->GetItemAsObject(4));
      pNormal = plane->GetNormal();
      pOrigin = plane->GetOrigin();

      lowerBounds[2] = pOrigin[2];

      plane = static_cast<vtkPlane *>(clipPlanes->GetItemAsObject(5));
      pNormal = plane->GetNormal();
      pOrigin = plane->GetOrigin();

      upperBounds[2] = pOrigin[2];

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
  //VolBBoxHigh.y, VolBBoxHigh.z, FgBgRatio, DepthPeelingThreshold,
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
  this->ParaMatrix[10] = this->FgBgRatio;

  //scalar range is 0 ~ 255
  this->ParaMatrix[11] = ((this->DepthPeelingThreshold + this->ScalarOffset) * this->ScalarScale )/255.0f;
  this->ParaMatrix[12] = 0.0f;

  this->ParaMatrix[13] = GlobalAlpha;

  this->ParaMatrix[14] = 0.0f;//debug, 0: no debug, 1: show ray origin, 2: show ray end

  GLfloat volMat[16];
  for (int i = 0; i < 16; i++)
    volMat[i] = (GLfloat)(*(matrix->Element))[i];

  int dim[3];
  this->GetVolumeDimensions(dim);
  
  this->ParaMatrix1[0] = 1.0f / dim[0];
  this->ParaMatrix1[1] = 1.0f / dim[1];
  this->ParaMatrix1[2] = 1.0f / dim[2];
  
  GLint loc = vtkgl::GetUniformLocation(RayCastProgram, "ParaMatrix");
  if (loc >= 0)
    vtkgl::UniformMatrix4fv(loc, 1, false, this->ParaMatrix);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "ParaMatrix1");
  if (loc >= 0)
    vtkgl::UniformMatrix4fv(loc, 1, false, this->ParaMatrix1);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "VolumeMatrix");
  if (loc >= 0)
    vtkgl::UniformMatrix4fv(loc, 1, false, volMat);
  matrix->Delete();
}

void vtkSlicerGPURayCastMultiVolumeMapper::RenderGLSL( vtkRenderer *ren, vtkVolume *vol )
{
  //force shader program reinit in dual 3D view mode
  if (vol->GetNumberOfConsumers() > 1)
    this->InitializeRayCast(); 

  vtkgl::UseProgram(RayCastProgram);

  this->SetupTextures( ren, vol );
  this->SetupRayCastParameters(ren, vol);

  glEnable(GL_CULL_FACE);

  // Start the timer now
  this->Timer->StartTimer();
  
  this->DrawVolumeBBox();
  glFinish();

  this->Timer->StopTimer();
  
  vtkgl::UseProgram(0);
}

void vtkSlicerGPURayCastMultiVolumeMapper::DeleteTextureIndex( GLuint *index )
{
  if (glIsTexture(*index))
  {
    GLuint tempIndex;
    tempIndex = *index;
    glDeleteTextures(1, &tempIndex);
    *index = 0;
  }
}

void vtkSlicerGPURayCastMultiVolumeMapper::CreateTextureIndex( GLuint *index )
{
  GLuint tempIndex=0;
  glGenTextures(1, &tempIndex);
  *index = static_cast<long>(tempIndex);
}

void vtkSlicerGPURayCastMultiVolumeMapper::Setup3DTextureParameters( vtkVolumeProperty *property )
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

void vtkSlicerGPURayCastMultiVolumeMapper::SetupTextures(vtkRenderer *vtkNotUsed(ren),
                                                    vtkVolume *vol )
{
  //0, 1, 2, 3
  //7, 6, 5, 4
  // Update the volume containing the 2 byte scalar / gradient magnitude
  if ( this->UpdateVolumes( vol ) || !this->Volume1Index || vol->GetNumberOfConsumers() > 1)
  {
    int dim[3];
    this->GetVolumeDimensions(dim);

    vtkgl::ActiveTexture( vtkgl::TEXTURE7 );
    this->DeleteTextureIndex(&this->Volume1Index);
    this->CreateTextureIndex(&this->Volume1Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);
    vtkgl::TexImage3D( vtkgl::TEXTURE_3D, 0, GL_RGBA8, dim[0], dim[1], dim[2], 0,
               GL_RGBA, GL_UNSIGNED_BYTE, this->Volume1 );
  }

  vtkgl::ActiveTexture( vtkgl::TEXTURE7 );
  glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);
  this->Setup3DTextureParameters( vol->GetProperty() );

  // Update the dependent 2D color table mapping scalar value and
  // gradient magnitude to RGBA
  if ( this->UpdateColorLookup( vol ) || !this->ColorLookupIndex
  || !this->ColorLookup2Index || vol->GetNumberOfConsumers() > 1)
  {
    vtkgl::ActiveTexture( vtkgl::TEXTURE6 );
    this->DeleteTextureIndex( &this->ColorLookupIndex );

    this->CreateTextureIndex( &this->ColorLookupIndex );
    glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0,
          GL_RGBA, GL_UNSIGNED_BYTE, this->ColorLookup );

    vtkgl::ActiveTexture( vtkgl::TEXTURE5 );
    this->DeleteTextureIndex( &this->ColorLookup2Index );

    this->CreateTextureIndex( &this->ColorLookup2Index );
    glBindTexture(GL_TEXTURE_2D, this->ColorLookup2Index);

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0,
          GL_RGBA, GL_UNSIGNED_BYTE, this->ColorLookup2nd );
  }

  vtkgl::ActiveTexture( vtkgl::TEXTURE6 );
  glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);

  vtkgl::ActiveTexture( vtkgl::TEXTURE5 );
  glBindTexture(GL_TEXTURE_2D, this->ColorLookup2Index);

  GLint loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureVol");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 7);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureColorLookupA");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 6);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "TextureColorLookupB");
  if (loc >= 0)
    vtkgl::Uniform1i(loc, 5);
}

int  vtkSlicerGPURayCastMultiVolumeMapper::IsRenderSupported(vtkRenderWindow* window, vtkVolumeProperty *property )
{
  if (window)
    {
    window->MakeCurrent();
    }
  if ( !this->Initialized )
    {
    this->Initialize(window);
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
  if (num < 10)
    return 0;

  num = 0;
  glGetIntegerv(vtkgl::MAX_FRAGMENT_UNIFORM_COMPONENTS, &num);
  if (num < 32)
    return 0; //at least we need 32 floats (two 4x4 matrices) in fragment shader
  return 1;
}

void vtkSlicerGPURayCastMultiVolumeMapper::Initialize(vtkRenderWindow* window)
{
  this->Initialized = 1;
  vtkOpenGLExtensionManager * extensions = vtkOpenGLExtensionManager::New();
  if (window)
    {
    extensions->SetRenderWindow(window); // set render window to the current one.
    }

  int supports_2_0=extensions->ExtensionSupported( "GL_VERSION_2_0" );
  if(supports_2_0)
  {
    extensions->LoadExtension("GL_VERSION_1_2");
    extensions->LoadExtension("GL_VERSION_1_3");
    extensions->LoadExtension("GL_VERSION_1_4");
    extensions->LoadExtension("GL_VERSION_1_5");
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

int vtkSlicerGPURayCastMultiVolumeMapper::IsTextureSizeSupported( int size[3] )
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

// Print the vtkSlicerGPURayCastMultiVolumeMapper
void vtkSlicerGPURayCastMultiVolumeMapper::PrintSelf(ostream& os, vtkIndent indent)
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

void vtkSlicerGPURayCastMultiVolumeMapper::DrawVolumeBBox()
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

void vtkSlicerGPURayCastMultiVolumeMapper::InitializeRayCast()
{
    RayCastInitialized = 1;

    vtkgl::DeleteShader(this->RayCastVertexShader);
    vtkgl::DeleteShader(this->RayCastFragmentShader);
    vtkgl::DeleteProgram(this->RayCastProgram);

    this->RayCastVertexShader = vtkgl::CreateShader(vtkgl::VERTEX_SHADER);
    this->RayCastFragmentShader = vtkgl::CreateShader(vtkgl::FRAGMENT_SHADER);
    this->RayCastProgram = vtkgl::CreateProgram();

    LoadVertexShader();
    LoadBgFgFragmentShader();
    LoadRayCastProgram();
}

void vtkSlicerGPURayCastMultiVolumeMapper::LoadBgFgFragmentShader()
{
  /* techniques in GPU ray cast II
   * 0: composite with directional lighting (default)
   * 1: composite with fake lighting (edge coloring, faster)
   * 2: MIP
   * 3: MINIP
   * 4: Gradient Magnitude Opacity Modulation
   * 5: Illustrative Context Preserving Exploration
   * */
  std::ostringstream fp_oss;
  fp_oss <<
    "varying vec3 ViewDir;                                                                  \n"
    "uniform sampler3D TextureVol;                                                          \n"
    "uniform sampler2D TextureColorLookupA;                                                 \n"
    "uniform sampler2D TextureColorLookupB;                                                 \n"
    "uniform mat4 ParaMatrix;                                                               \n"
    "uniform mat4 ParaMatrix1;                                                              \n"
    "uniform mat4 VolumeMatrix;                                                             \n"
    "uniform mat4 VolumeMatrix1;                                                            \n"
    "                                                                                       \n"
    "//ParaMatrix:                                                                          \n"
    "//EyePos.x,      EyePos.y,      EyePos.z,     Step                                     \n"
    "//VolBBoxLow.x,  VolBBoxLow.y,  VolBBoxLow.z, VolBBoxHigh.x                            \n"
    "//VolBBoxHigh.y, VolBBoxHigh.z, FgBgRatio,    DepthPeelingThreshold,                   \n"
    "//N/A,           GlobalAlpha,   Debug,                                                 \n"
    "                                                                                       \n"
    "vec4 computeRayEnd()                                                                   \n"
    "{                                                                                      \n"
    "   vec3 o = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);                \n"
    "    vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);             \n"
    "    vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);             \n"
    "    mmn = clamp(mmn, 0.0, 1.0);                                                        \n"
    "    mmx = clamp(mmx, 0.0, 1.0);                                                        \n"
    "                                                                                       \n"
    "    if (all(greaterThanEqual(o, mmn)) && all(lessThanEqual(o, mmx)) )                  \n"
    "        return gl_TexCoord[0];                                                         \n"
    "                                                                                       \n"
    "    vec3 a1 = gl_TexCoord[0].xyz;                                                      \n"
    "    vec3 a2 = a1 + normalize(a1 - o) * length(mmx - mmn);                              \n"
    "    vec3 a3;                                                                           \n"
    "                                                                                       \n"
    "    float halfStep = ParaMatrix[0][3]*0.5;                                             \n"
    "    for (int i = 0; i < 10; i++)                                                       \n"
    "    {                                                                                  \n"
    "        a3 = (a1 + a2) * 0.5;                                                          \n"
    "        if (length(a2 - a1) <= halfStep)                                               \n"
    "            return vec4(clamp(a3, mmn, mmx), 1.0);                                     \n"
    "        if ( all(greaterThanEqual(a3, mmn)) && all(lessThanEqual(a3, mmx)) )           \n"
    "            a1 = a3;                                                                   \n"
    "        else                                                                           \n"
    "            a2 = a3;                                                                   \n"
    "    }                                                                                  \n"
    "    return vec4(clamp(a3, mmn, mmx), 1.0);                                             \n"
    "}                                                                                      \n"
    "                                                                                       \n"
    "vec4 computeRayOrigin()                                                                \n"
    "{                                                                                      \n"
    "    vec3 o = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);               \n"
    "    vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);             \n"
    "    vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);             \n"
    "    mmn = clamp(mmn, 0.0, 1.0);                                                        \n"
    "    mmx = clamp(mmx, 0.0, 1.0);                                                        \n"
    "                                                                                       \n"
    "    if (all(greaterThanEqual(o, mmn)) && all(lessThanEqual(o, mmx)) )                  \n"
    "        return vec4(o, 1.0);                                                           \n"
    "    else                                                                               \n"
    "        return gl_TexCoord[0];                                                         \n"
    "}                                                                                      \n"
    "                                                                                       \n"
    "//perform 3D texture lookup based on RenderMethod                                      \n"
    "vec4 voxelColorA(vec3 coord)                                                           \n"
    "{                                                                                      \n"
    "    vec4 scalar = texture3D(TextureVol, coord);                                        \n"
    "    return texture2D(TextureColorLookupA, vec2(scalar.x, 1.0));                        \n"
    "}                                                                                      \n"
    "                                                                                       \n"
    "vec4 voxelColorB(vec3 coord)                                                           \n"
    "{                                                                                      \n"
    "    vec4 scalar = texture3D(TextureVol, coord);                                        \n"
    "    return texture2D(TextureColorLookupB, vec2(scalar.y, 1.0));                        \n"
    "}                                                                                      \n"
    "                                                                                       \n"
    "vec4 voxelNormalA(vec3 coord)                                                          \n"
    "{                                                                                      \n"
    "   vec4 sample1, sample2;                                                              \n"
    "   sample1.x = texture3D(TextureVol, coord + vec3(-ParaMatrix1[0][0], 0.0, 0.0)).x;    \n"
    "   sample1.y = texture3D(TextureVol, coord + vec3(0.0, -ParaMatrix1[0][1], 0.0)).x;    \n"
    "   sample1.z = texture3D(TextureVol, coord + vec3(0.0, 0.0, -ParaMatrix1[0][2])).x;    \n"
    "   sample2.x = texture3D(TextureVol, coord + vec3(ParaMatrix1[0][0], 0.0, 0.0)).x;     \n"
    "   sample2.y = texture3D(TextureVol, coord + vec3(0.0, ParaMatrix1[0][1], 0.0)).x;     \n"
    "   sample2.z = texture3D(TextureVol, coord + vec3(0.0, 0.0, ParaMatrix1[0][2])).x;     \n"
    "   vec4 normal = sample1 - sample2;                                                    \n"
    "   float length = length(normal.xyz);                                                  \n"
    "   if (length < 0.001) return vec4(0);                                                 \n"
    "   normal = VolumeMatrix * normalize(normal);                                          \n"
    "   return vec4(gl_NormalMatrix * normal.xyz, length);                                  \n"
    "}                                                                                      \n"
    "                                                                                       \n"
    "vec4 voxelNormalB(vec3 coord)                                                          \n"
    "{                                                                                      \n"
    "   vec4 sample1, sample2;                                                              \n"
    "   sample1.x = texture3D(TextureVol, coord + vec3(-ParaMatrix1[0][0], 0.0, 0.0)).y;    \n"
    "   sample1.y = texture3D(TextureVol, coord + vec3(0.0, -ParaMatrix1[0][1], 0.0)).y;    \n"
    "   sample1.z = texture3D(TextureVol, coord + vec3(0.0, 0.0, -ParaMatrix1[0][2])).y;    \n"
    "   sample2.x = texture3D(TextureVol, coord + vec3(ParaMatrix1[0][0], 0.0, 0.0)).y;     \n"
    "   sample2.y = texture3D(TextureVol, coord + vec3(0.0, ParaMatrix1[0][1], 0.0)).y;     \n"
    "   sample2.z = texture3D(TextureVol, coord + vec3(0.0, 0.0, ParaMatrix1[0][2])).y;     \n"
    "   vec4 normal = sample1 - sample2;                                                    \n"
    "   float length = length(normal.xyz);                                                  \n"
    "   if (length < 0.001) return vec4(0);                                                 \n"
    "   normal = VolumeMatrix * normalize(normal);                                          \n"
    "   return vec4(gl_NormalMatrix * normal.xyz, length);                                  \n"
    "}                                                                                      \n";
    
  //shading A
  if (this->Technique == 0 || this->Technique == 4 || this->Technique == 5)
  {
    fp_oss <<
      "vec4 directionalLightA(vec3 coord, vec3 lightDir, vec4 color, vec4 normalIn)            \n"
      "{                                                                                       \n"
      "  if (length(normalIn.xyz) <= 0.001)                                                    \n"
      "    return gl_FrontMaterial.ambient * color;                                            \n" 
      "  vec3    normal = normalize(normalIn.xyz);                                             \n"
      "  float   NdotL = abs( dot( normal, lightDir ) );                                       \n"
      "  vec4    specular = vec4(0);                                                           \n"
      "  if (NdotL > 0.0)                                                                      \n"
      "  {                                                                                     \n"
      "    float   NdotHV = max( dot( normal, gl_LightSource[0].halfVector.xyz), 0.0);         \n"
      "    specular = (gl_FrontMaterial.specular) * pow(NdotHV, gl_FrontMaterial.shininess);   \n"
      "  }                                                                                     \n"
      "  vec4 diffuse = (gl_FrontMaterial.ambient + gl_FrontMaterial.diffuse * NdotL) * color; \n"
      "  return (specular + diffuse);                                                          \n"
      "}                                                                                       \n";
  }
  else if (this->Technique == 1)
  {
    fp_oss <<
      "vec4 edgeColoringA(vec3 coord, vec4 diffuse, vec4 normalIn)                             \n"
      "{                                                                                       \n"
      "  if (normalIn.w <= 0.001)                                                              \n"
      "    return gl_FrontMaterial.ambient * color;                                            \n" 
      "  vec3    normal = normalize(normalIn.xyz);                                             \n"
      "  float   NdotV = abs( dot( normal, normalize(-ViewDir) ) );                            \n"
      "  return diffuse*NdotV;                                                                 \n"
      "}                                                                                       \n";
  }

  //shading B
  if (this->TechniqueFg == 0 || this->TechniqueFg == 4 || this->TechniqueFg == 5)
  {
    fp_oss <<
      "vec4 directionalLightB(vec3 coord, vec3 lightDir, vec4 color, vec4 normalIn)            \n"
      "{                                                                                       \n"
      "  if (length(normalIn.xyz) <= 0.001)                                                    \n"
      "    return gl_FrontMaterial.ambient * color;                                            \n" 
      "  vec3    normal = normalize(normalIn.xyz);                                             \n"
      "  float   NdotL = abs( dot( normal, lightDir ));                                        \n"
      "  vec4    specular = vec4(0);                                                           \n"
      "  if (NdotL > 0.0)                                                                      \n"
      "  {                                                                                     \n"
      "    float   NdotHV = max( dot( normal, gl_LightSource[0].halfVector.xyz), 0.0);         \n"
      "    specular = (gl_FrontMaterial.specular) * pow(NdotHV, gl_FrontMaterial.shininess);   \n"
      "  }                                                                                     \n"
      "  vec4 diffuse = (gl_FrontMaterial.ambient + gl_FrontMaterial.diffuse * NdotL) * color; \n"
      "  return (specular + diffuse);                                                          \n"
      "}                                                                                       \n";
  }
  else if (this->TechniqueFg == 1)
  {
    fp_oss <<
      "vec4 edgeColoringB(vec3 coord, vec4 diffuse, vec4 normalIn)                             \n"
      "{                                                                                       \n"
      "  if (normalIn.w <= 0.001)                                                               \n"
      "    return gl_FrontMaterial.ambient * color;                                            \n" 
      "  vec3    normal = normalize(normalIn.xyz);                                             \n"
      "  float   NdotV = abs( dot( normal, normalize(-ViewDir) ) );                            \n"
      "  return diffuse*NdotV;                                                                 \n"
      "}                                                                                       \n";
  }

  fp_oss <<
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 rayOrigin = computeRayOrigin();                                               \n"
    "    vec4 rayEnd = computeRayEnd();                                                     \n"
    "    vec3 rayDir = rayEnd.xyz - rayOrigin.xyz;                                          \n"
    "    float rayLen = length(rayDir);                                                     \n"
    "                                                                                       \n"
    "    if (rayLen > 1.732)                                                                \n"
    "      rayLen = 1.732;                                                                  \n"
    "                                                                                       \n"
    "    rayDir = normalize(rayDir);                                                        \n"
    "                                                                                       \n"
    "    //do ray casting                                                                   \n"
    "    vec3 rayStep = rayDir*ParaMatrix[0][3];                                            \n"
    "    vec3 nextRayOrigin = rayOrigin.xyz;                                                \n"
    "                                                                                       \n"
    "    vec4 pixelColor = vec4(0);                                                         \n"
    "    float alpha = 0.0;                                                                 \n";

  if (this->Technique != 2 || this->Technique != 3)
  {
    fp_oss <<
    "    float alphaA = 0.0;                                                                  \n";
  }

  if (this->TechniqueFg != 2 || this->TechniqueFg != 3)
  {
    fp_oss <<
    "    float alphaB = 0.0;                                                                  \n";
  }

  fp_oss <<
    "    float t = 0.0;                                                                       \n"
    "    vec3  lightDir = normalize( gl_LightSource[0].position.xyz );                        \n"
    "    float fgRatio = ParaMatrix[2][2];                                                    \n";

  switch(this->Technique)
  {
    case 0:
      switch(this->TechniqueFg)
      {
        case 0:
          switch(this->ColorOpacityFusion)
          {
            case 0://alpha blending or
              fp_oss <<
                "{                                                                                           \n"
                "  while( (t < rayLen) && (alpha < 0.975) )                                                  \n"
                "  {                                                                                         \n"
                "    vec4 nextColorA = voxelColorA(nextRayOrigin);                                           \n"
                "    vec4 nextColorB = voxelColorB(nextRayOrigin);                                           \n"
                "    float tempAlphaA = nextColorA.w;                                                        \n"
                "    float tempAlphaB = nextColorB.w;                                                        \n"
                "                                                                                            \n"
                "    if (tempAlphaA > 0.0)                                                                   \n"
                "    {                                                                                       \n"
                "      vec4 normalA = voxelNormalA(nextRayOrigin);                                           \n"
                "      nextColorA = directionalLightA(nextRayOrigin, lightDir, nextColorA, normalA);         \n"
                "                                                                                            \n"
                "      tempAlphaA *= (1.0 - alpha);                                                          \n"
                "      pixelColor += nextColorA * tempAlphaA * (1.0 - fgRatio);                              \n"
                "      alphaA += tempAlphaA;                                                                 \n"
                "    }                                                                                       \n"
                "    alpha = alphaA * (1.0 - fgRatio) + alphaB * fgRatio;                                    \n"
                "    if (tempAlphaB > 0.0)                                                                   \n"
                "    {                                                                                       \n"
                "      vec4 normalB = voxelNormalB(nextRayOrigin);                                           \n"
                "      nextColorB = directionalLightB(nextRayOrigin, lightDir, nextColorB, normalB);         \n"
                "                                                                                            \n"
                "      tempAlphaB *= (1.0 - alpha);                                                          \n"
                "      pixelColor += nextColorB * tempAlphaB * fgRatio;                                      \n"
                "      alphaB += tempAlphaB;                                                                 \n"
                "    }                                                                                       \n"
                "                                                                                            \n"
                "    alpha = alphaA * (1.0 - fgRatio) + alphaB * fgRatio;                                    \n"
                "    t += ParaMatrix[0][3];                                                                  \n"
                "    nextRayOrigin += rayStep;                                                               \n"
                "  }                                                                                         \n"
                "}                                                                                           \n"
                "gl_FragColor = vec4(pixelColor.xyz, alpha);                                                 \n";
              break;
            case 1://alpha blending and
              fp_oss <<
                "{                                                                                           \n"
                "  while( (t < rayLen) && (alpha < 0.975) )                                                  \n"
                "  {                                                                                         \n"
                "    vec4 nextColorA = voxelColorA(nextRayOrigin);                                           \n"
                "    vec4 nextColorB = voxelColorB(nextRayOrigin);                                           \n"
                "    float tempAlphaA = nextColorA.w;                                                        \n"
                "    float tempAlphaB = nextColorB.w;                                                        \n"
                "                                                                                            \n"
                "    if (tempAlphaA > 0.0 && tempAlphaB > 0.0)                                               \n"
                "    {                                                                                       \n"
                "      vec4 normalA = voxelNormalA(nextRayOrigin);                                           \n"
                "      nextColorA = directionalLightA(nextRayOrigin, lightDir, nextColorA, normalA);         \n"
                "                                                                                            \n"
                "      tempAlphaA *= (1.0 - alpha);                                                          \n"
                "      if (tempAlphaB > 0.0)                                                                 \n"
                "        pixelColor += nextColorA * tempAlphaA * (1.0 - fgRatio);                            \n"
                "      alphaA += tempAlphaA;                                                                 \n"
                "                                                                                            \n"
                "      alpha = alphaA * (1.0 - fgRatio) + alphaB * fgRatio;                                  \n"
                "                                                                                            \n"
                "      vec4 normalB = voxelNormalB(nextRayOrigin);                                           \n"
                "      nextColorB = directionalLightB(nextRayOrigin, lightDir, nextColorB, normalB);         \n"
                "                                                                                            \n"
                "      tempAlphaB *= (1.0 - alpha);                                                          \n"
                "      if (tempAlphaA > 0.0)                                                                 \n"
                "        pixelColor += nextColorB * tempAlphaB * fgRatio;                                    \n"
                "      alphaB += tempAlphaB;                                                                 \n"
                "                                                                                            \n"
                "      alpha = alphaA * (1.0 - fgRatio) + alphaB * fgRatio;                                  \n"
                "    }                                                                                       \n"
                "                                                                                            \n"
                "    t += ParaMatrix[0][3];                                                                  \n"
                "    nextRayOrigin += rayStep;                                                               \n"
                "  }                                                                                         \n"
                "}                                                                                           \n"
                "gl_FragColor = vec4(pixelColor.xyz, alpha);                                                 \n";
              break;
          }
          break;
        case 1:
          break;
        case 2:
          fp_oss <<
            "{                                                                                           \n"
            "  while( (t < rayLen) && (alphaA < 0.975) )                                                 \n"
            "  {                                                                                         \n"
            "    vec4 nextColorA = voxelColorA(nextRayOrigin);                                           \n"
            "    float tempAlphaA = nextColorA.w;                                                        \n"
            "                                                                                            \n"
            "    if (tempAlphaA > 0.0)                                                                   \n"
            "    {                                                                                       \n"
            "      vec4 normalA = voxelNormalA(nextRayOrigin);                                           \n"
            "      nextColorA = directionalLightA(nextRayOrigin, lightDir, nextColorA, normalA);         \n"
            "                                                                                            \n"
            "      tempAlphaA *= (1.0 - alphaA);                                                         \n"
            "      pixelColor += nextColorA * tempAlphaA * (1.0 - fgRatio);                              \n"
            "      alphaA += tempAlphaA;                                                                 \n"
            "    }                                                                                       \n"
            "                                                                                            \n"
            "    t += ParaMatrix[0][3];                                                                  \n"
            "    nextRayOrigin += rayStep;                                                               \n"
            "  }                                                                                         \n"
            "                                                                                            \n"
            "  t = 0.0;//reset parameters for mip traversal                                              \n"
            "  nextRayOrigin = rayOrigin.xyz;                                                            \n"
            "                                                                                            \n"
            "  float maxScalarB = texture3D(TextureVol, nextRayOrigin).y;                                \n"
            "  vec3 maxScalarCoordB = nextRayOrigin;                                                     \n"
            "                                                                                            \n"
            "  while( t < rayLen )                                                                       \n"
            "  {                                                                                         \n"
            "    float scalar = texture3D(TextureVol, nextRayOrigin).y;                                  \n"
            "    if (maxScalarB < scalar)                                                                \n"
            "    {                                                                                       \n"
            "       maxScalarB = scalar;                                                                 \n"
            "       maxScalarCoordB = nextRayOrigin;                                                     \n"
            "    }                                                                                       \n"
            "                                                                                            \n"
            "    t += ParaMatrix[0][3];                                                                  \n"
            "    nextRayOrigin += rayStep;                                                               \n"
            "  }                                                                                         \n"
            "                                                                                            \n"
            "  vec4 mipColor = voxelColorB(maxScalarCoordB);                                             \n"
            "  pixelColor += mipColor * fgRatio;                                                         \n"
            "  alpha = alphaA * (1.0 - fgRatio) +  mipColor.w * fgRatio;                                 \n"
            "}                                                                                           \n"
            "gl_FragColor = vec4(pixelColor.xyz, alpha);                                                 \n";
          break;
        case 3:
          break;
      }
      break;
    case 1:
      switch(this->TechniqueFg)
      {
        case 0:
          break;
        case 1:
          break;
        case 2:
          break;
        case 3:
          break;
      }
      break;
    case 2:
      switch(this->TechniqueFg)
      {
        case 0:
          break;
        case 1:
          break;
        case 2:
          fp_oss <<
            "{                                                                                           \n"
            "  float maxScalarA = texture3D(TextureVol, nextRayOrigin).x;                                \n"
            "  vec3 maxScalarCoordA = nextRayOrigin;                                                     \n"
            "  while( t < rayLen )                                                                       \n"
            "  {                                                                                         \n"
            "    float scalar = texture3D(TextureVol, nextRayOrigin).x;                                  \n"
            "    if (maxScalarA < scalar)                                                                \n"
            "    {                                                                                       \n"
            "       maxScalarA = scalar;                                                                 \n"
            "       maxScalarCoordA = nextRayOrigin;                                                     \n"
            "    }                                                                                       \n"
            "                                                                                            \n"
            "    t += ParaMatrix[0][3];                                                                  \n"
            "    nextRayOrigin += rayStep;                                                               \n"
            "  }                                                                                         \n"
            "                                                                                            \n"
            "  t = 0.0;//reset parameters for mip traversal                                              \n"
            "  nextRayOrigin = rayOrigin.xyz;                                                            \n"
            "                                                                                            \n"
            "  float maxScalarB = texture3D(TextureVol, nextRayOrigin).y;                                \n"
            "  vec3 maxScalarCoordB = nextRayOrigin;                                                     \n"
            "                                                                                            \n"
            "  while( t < rayLen )                                                                       \n"
            "  {                                                                                         \n"
            "    float scalar = texture3D(TextureVol, nextRayOrigin).y;                                  \n"
            "    if (maxScalarB < scalar)                                                                \n"
            "    {                                                                                       \n"
            "       maxScalarB = scalar;                                                                 \n"
            "       maxScalarCoordB = nextRayOrigin;                                                     \n"
            "    }                                                                                       \n"
            "                                                                                            \n"
            "    t += ParaMatrix[0][3];                                                                  \n"
            "    nextRayOrigin += rayStep;                                                               \n"
            "  }                                                                                         \n"
            "                                                                                            \n"
            "  vec4 mipColorA = voxelColorA(maxScalarCoordA);                                            \n"
            "  vec4 mipColorB = voxelColorB(maxScalarCoordB);                                            \n"
            "  pixelColor = mipColorA * (1.0 - fgRatio) + mipColorB * fgRatio;                           \n"
            "  alpha = mipColorA.w * (1.0 - fgRatio) +  mipColorB.w * fgRatio;                           \n"
            "}                                                                                           \n"
            "gl_FragColor = vec4(pixelColor.xyz, alpha);                                                 \n";
          break;
        case 3:
          break;
      }
      break;
    case 3:
      switch(this->TechniqueFg)
      {
        case 0:
          break;
        case 1:
          break;
        case 2:
          break;
        case 3:
          break;
      }
      break;
    case 4:
      switch(this->TechniqueFg)
      {
        case 4:
          switch(this->ColorOpacityFusion)
          {
            case 0://alpha blending or
              fp_oss <<
                "{                                                                                           \n"
                "  while( (t < rayLen) && (alpha < 0.975) )                                                  \n"
                "  {                                                                                         \n"
                "    vec4 nextColorA = voxelColorA(nextRayOrigin);                                           \n"
                "    vec4 nextColorB = voxelColorB(nextRayOrigin);                                           \n"
                "    float tempAlphaA = nextColorA.w;                                                        \n"
                "    float tempAlphaB = nextColorB.w;                                                        \n"
                "                                                                                            \n"
                "    if (tempAlphaA > 0.0)                                                                   \n"
                "    {                                                                                       \n"
                "      vec4 normalA = voxelNormalA(nextRayOrigin);                                           \n"
                "      if (length(normalA.xyz) > 0.001)                                                      \n"
                "      {                                                                                     \n"
                "        nextColorA = directionalLightA(nextRayOrigin, lightDir, nextColorA, normalA);       \n"
                "                                                                                            \n"
                "        tempAlphaA *= (1.0 - alpha) * normalA.w;                                            \n"
                "        pixelColor += nextColorA * tempAlphaA * (1.0 - fgRatio);                            \n"
                "        alphaA += tempAlphaA;                                                               \n"
                "      }                                                                                     \n"
                "    }                                                                                       \n"
                "    alpha = alphaA * (1.0 - fgRatio) + alphaB * fgRatio;                                    \n"
                "    if (tempAlphaB > 0.0)                                                                   \n"
                "    {                                                                                       \n"
                "      vec4 normalB = voxelNormalB(nextRayOrigin);                                           \n"
                "      if (length(normalB.xyz) > 0.001)                                                      \n"
                "      {                                                                                     \n"
                "        nextColorB = directionalLightB(nextRayOrigin, lightDir, nextColorB, normalB);       \n"
                "                                                                                            \n"
                "        tempAlphaB *= (1.0 - alpha) * normalB.w;                                            \n"
                "        pixelColor += nextColorB * tempAlphaB * fgRatio;                                    \n"
                "        alphaB += tempAlphaB;                                                               \n"
                "      }                                                                                     \n"
                "    }                                                                                       \n"
                "                                                                                            \n"
                "    alpha = alphaA * (1.0 - fgRatio) + alphaB * fgRatio;                                    \n"
                "    t += ParaMatrix[0][3];                                                                  \n"
                "    nextRayOrigin += rayStep;                                                               \n"
                "  }                                                                                         \n"
                "}                                                                                           \n"
                "gl_FragColor = vec4(pixelColor.xyz, alpha);                                                 \n";
              break;
          }
          break;
      }
      break;
  }

  fp_oss <<
    "}                                                                                        \n";

  std::string source = fp_oss.str();
  const char* pSourceText = source.c_str();

//  std::cerr << source;

  vtkgl::ShaderSource(RayCastFragmentShader, 1, &pSourceText, NULL);
  vtkgl::CompileShader(RayCastFragmentShader);

  GLint result;
  vtkgl::GetShaderiv(RayCastFragmentShader, vtkgl::COMPILE_STATUS, &result);

  if (!result)
    std::cerr << "Fragment Shader Compile Status: FALSE" << std::endl;

  GLint infoLogLen;
  vtkgl::GetShaderiv(RayCastFragmentShader, vtkgl::INFO_LOG_LENGTH, &infoLogLen);
  vtkgl::GLchar *pInfoLog = (vtkgl::GLchar*)malloc(sizeof(vtkgl::GLchar)*(infoLogLen+1));

  try
  {
    vtkgl::GetShaderInfoLog(RayCastFragmentShader, infoLogLen, NULL, pInfoLog);
    std::cerr << pInfoLog;
  }catch(...)
  {
  }

  free(pInfoLog);
}

void vtkSlicerGPURayCastMultiVolumeMapper::LoadRayCastProgram()
{
  vtkgl::AttachShader(RayCastProgram, RayCastVertexShader);
  vtkgl::AttachShader(RayCastProgram, RayCastFragmentShader);
  vtkgl::LinkProgram(RayCastProgram);

  GLint result;
  vtkgl::GetProgramiv(RayCastProgram, vtkgl::LINK_STATUS, &result);

  if (!result)
    printf("Program Link Status: FALSE\n");

}

void vtkSlicerGPURayCastMultiVolumeMapper::LoadVertexShader()
{
  std::ostringstream vp_oss;
  vp_oss <<
    "varying vec3 ViewDir;                                                                     \n"
    "void main()                                                                               \n"
    "{                                                                                         \n"
    "  gl_Position = ftransform();                                                             \n"
    "  gl_TexCoord[0] = gl_Color;                                                              \n"
    "  ViewDir = vec3(gl_ModelViewMatrix * gl_Vertex);                                         \n"
    "}                                                                                         \n";

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

void vtkSlicerGPURayCastMultiVolumeMapper::PrintGLErrorString()
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

void vtkSlicerGPURayCastMultiVolumeMapper::PrintFragmentShaderInfoLog()
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

void vtkSlicerGPURayCastMultiVolumeMapper::SetTechniques(int tech, int techFg)
{
  this->Technique = tech;
  this->TechniqueFg = techFg;
  this->ReloadShaderFlag = 1;
}

void vtkSlicerGPURayCastMultiVolumeMapper::SetColorOpacityFusion(int fusion)
{
  this->ColorOpacityFusion = fusion;
  this->ReloadShaderFlag = 1;
}

void vtkSlicerGPURayCastMultiVolumeMapper::SetInternalVolumeSize(int size)
{
  if (this->InternalVolumeSize != size)
  {
    this->InternalVolumeSize = size;
    this->SavedTextureInput = NULL;//dirty input, force reprocess input
  }
}


