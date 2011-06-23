/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerGPURayCastVolumeMapper.h,v $

   Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkSlicerGPURayCastVolumeMapper
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/ $
  Date:      $Date: 2009-01-07 09:26:53 -0500 (Tue, 30 Jan 2009) $
  Version:   $Revision:  $


=========================================================================*/
#include <string.h>
#include <iostream>
#include <sstream>

#include "vtkWindows.h"
#include "vtkSlicerGPURayCastVolumeMapper.h"

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
vtkCxxRevisionMacro(vtkSlicerGPURayCastVolumeMapper, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkSlicerGPURayCastVolumeMapper);
//#endif

vtkSlicerGPURayCastVolumeMapper::vtkSlicerGPURayCastVolumeMapper()
{
  this->Initialized          =  0;
  this->RayCastInitialized   =  0;
  this->Technique            =  0;//by default composit with shading

  this->Clipping             =  0;
  this->ReloadShaderFlag     =  0;
  this->InternalVolumeSize      =  256; //by default 256^3

  this->Volume1Index         =  0;
  this->Volume2Index         =  0;
  this->ColorLookupIndex         =  0;
  this->RayCastVertexShader      =  0;
  this->RayCastFragmentShader    =  0;
  this->RayCastProgram           =  0;
  this->RayCastSupported         =  0;
  this->RenderWindow         = NULL;
  this->RaySteps             = 500.0f;

  this->ICPEScale            = 1.0f;
  this->ICPESmoothness       = 0.5f;

  this->DistanceColorBlending   = 0.0f;
}

vtkSlicerGPURayCastVolumeMapper::~vtkSlicerGPURayCastVolumeMapper()
{
}

// Release the graphics resources used by this texture.
void vtkSlicerGPURayCastVolumeMapper::ReleaseGraphicsResources(vtkWindow
                                *renWin)
{
  if (( this->Volume1Index || this->Volume2Index || this->ColorLookupIndex ) && renWin)
    {
    static_cast<vtkRenderWindow *>(renWin)->MakeCurrent();
#ifdef GL_VERSION_1_1
    // free any textures
    this->DeleteTextureIndex( &this->Volume1Index );
    this->DeleteTextureIndex( &this->Volume2Index );
    this->DeleteTextureIndex( &this->ColorLookupIndex );
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
  this->ColorLookupIndex = 0;
  this->RayCastVertexShader   = 0;
  this->RayCastFragmentShader = 0;
  this->RayCastProgram    = 0;
  this->RenderWindow     = NULL;
  this->Modified();
}

void vtkSlicerGPURayCastVolumeMapper::Render(vtkRenderer *ren, vtkVolume *vol)
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

  
//  cout<<ambient<<" "<<diffuse<<" "<<specular<<endl;
//  cout.flush();

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

void vtkSlicerGPURayCastVolumeMapper::AdaptivePerformanceControl()
{
  //do automatic performance control
  if(this->Framerate <= 0.01f)
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

//  maxRaysteps = maxRaysteps < 512.0f ? 512.0f : maxRaysteps;//ensure high sampling rate on low resolution volumes
  
  // add clamp
  if (this->RaySteps > maxRaysteps) this->RaySteps = maxRaysteps;
  if (this->RaySteps < 200.0f)       this->RaySteps = 200.0f;

//  cout<<this->Framerate<<" "<<this->TimeToDraw<<" "<<this->RaySteps<<endl;
//  cout.flush();
}

//needs to be cleaned, 2008/10/20, Yanling Liu
void vtkSlicerGPURayCastVolumeMapper::SetupRayCastParameters(vtkRenderer *vtkNotUsed(pRen),
                                                                      vtkVolume *pVol)
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
  //VolBBoxHigh.y, VolBBoxHigh.z, MinDist,      DepthPeelingThreshold,
  //ICPE_s,        GlobalAlpha,   MinMaxLen,    ICPE_t

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
    eye[i] = (eye[i] - lowerBounds[i])/bboxLen[i];//eye pos in texture space
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

  float maxDist, minDist;

  {
    float x = VolumeBBoxVerticesColor[0][0] - eye[0];
    float y = VolumeBBoxVerticesColor[0][1] - eye[1];
    float z = VolumeBBoxVerticesColor[0][2] - eye[2];
    float dist = sqrtf(x*x + y*y + z*z);

    maxDist = dist;
    minDist = dist;
  }

  for (int i = 1; i < 8; i++)
  {
    float x = VolumeBBoxVerticesColor[i][0] - eye[0];
    float y = VolumeBBoxVerticesColor[i][1] - eye[1];
    float z = VolumeBBoxVerticesColor[i][2] - eye[2];
    float dist = sqrtf(x*x + y*y + z*z);

    maxDist = maxDist < dist ? dist : maxDist;
    minDist = minDist > dist ? dist : minDist;
  }

  this->ParaMatrix[10] = minDist;

  //scalar range is 0 ~ 255
  this->ParaMatrix[11] = ((this->DepthPeelingThreshold + this->ScalarOffset) * this->ScalarScale )/255.0f;
  this->ParaMatrix[12] = this->ICPESmoothness;

  this->ParaMatrix[13] = DistanceColorBlending;

  this->ParaMatrix[14] = maxDist - minDist;

//  cout<<eye[0]<<" "<<eye[1]<<" "<<eye[2]<<" "<<minDist<<" "<<maxDist<<endl;cout.flush();

  this->ParaMatrix[15] = this->ICPEScale;

  GLfloat volMat[16];
  for (int i = 0; i < 16; i++)
    volMat[i] = (GLfloat)(*(matrix->Element))[i];

  int dim[3];
  this->GetInput()->GetDimensions(dim);

  this->ParaMatrix1[0] = 1.0f / dim[0];
  this->ParaMatrix1[1] = 1.0f / dim[1];
  this->ParaMatrix1[2] = 1.0f / dim[2];
  
  GLint loc = vtkgl::GetUniformLocation(RayCastProgram, "ParaMatrix");
  if (loc >= 0)
    vtkgl::UniformMatrix4fv(loc, 1, false, this->ParaMatrix);
//  loc = vtkgl::GetUniformLocation(RayCastProgram, "ParaMatrix1");
//  if (loc >= 0)
//    vtkgl::UniformMatrix4fv(loc, 1, false, this->ParaMatrix1);
  loc = vtkgl::GetUniformLocation(RayCastProgram, "VolumeMatrix");
  if (loc >= 0)
    vtkgl::UniformMatrix4fv(loc, 1, false, volMat);
  matrix->Delete();
}

void vtkSlicerGPURayCastVolumeMapper::RenderGLSL( vtkRenderer *ren, vtkVolume *vol )
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

void vtkSlicerGPURayCastVolumeMapper::DeleteTextureIndex( GLuint *index )
{
  if (glIsTexture(*index))
  {
    GLuint tempIndex;
    tempIndex = *index;
    glDeleteTextures(1, &tempIndex);
    *index = 0;
  }
}

void vtkSlicerGPURayCastVolumeMapper::CreateTextureIndex( GLuint *index )
{
  GLuint tempIndex=0;
  glGenTextures(1, &tempIndex);
  *index = static_cast<long>(tempIndex);
}

void vtkSlicerGPURayCastVolumeMapper::Setup3DTextureParameters( vtkVolumeProperty *property )
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

void vtkSlicerGPURayCastVolumeMapper::SetupTextures(vtkRenderer *vtkNotUsed(ren),
                                                             vtkVolume *vol )
{
  //0, 1, 2, 3
  //7, 6, 5, 4
  // Update the volume containing the 2 byte scalar / gradient magnitude
  // copy texture into GPU memory every frame for Dual 3D view mode
  if ( this->UpdateVolumes( vol ) || !this->Volume1Index || !this->Volume2Index || vol->GetNumberOfConsumers() > 1)
    {
    int dim[3];
    this->GetVolumeDimensions(dim);

    vtkgl::ActiveTexture( vtkgl::TEXTURE7 );
    this->DeleteTextureIndex(&this->Volume1Index);
    this->CreateTextureIndex(&this->Volume1Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);
    vtkgl::TexImage3D( vtkgl::TEXTURE_3D, 0, GL_RGBA8, dim[0], dim[1], dim[2], 0,
               GL_RGBA, GL_UNSIGNED_BYTE, this->Volume1 );


    vtkgl::ActiveTexture( vtkgl::TEXTURE5 );
    this->DeleteTextureIndex(&this->Volume2Index);
    this->CreateTextureIndex(&this->Volume2Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);
    vtkgl::TexImage3D( vtkgl::TEXTURE_3D, 0, GL_RGBA8, dim[0], dim[1], dim[2], 0,
               GL_RGBA, GL_UNSIGNED_BYTE, this->Volume2 );
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
  if ( this->UpdateColorLookup( vol ) || !this->ColorLookupIndex || vol ->GetNumberOfConsumers() > 1)
    {
    this->DeleteTextureIndex( &this->ColorLookupIndex );

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

int  vtkSlicerGPURayCastVolumeMapper::IsRenderSupported(vtkRenderWindow* window, vtkVolumeProperty *property )
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
  if (num < 8)//we use texture unit 4,5,6,7 to avoid conflict with slice planes
    return 0;

  num = 0;
  glGetIntegerv(vtkgl::MAX_FRAGMENT_UNIFORM_COMPONENTS, &num);
  if (num < 32)
    return 0; //at least we need 32 floats (two 4x4 matrices) in fragment shader
  return 1;
}

void vtkSlicerGPURayCastVolumeMapper::Initialize(vtkRenderWindow* window)
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

int vtkSlicerGPURayCastVolumeMapper::IsTextureSizeSupported( int size[3] )
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

// Print the vtkSlicerGPURayCastVolumeMapper
void vtkSlicerGPURayCastVolumeMapper::PrintSelf(ostream& os, vtkIndent indent)
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

void vtkSlicerGPURayCastVolumeMapper::DrawVolumeBBox()
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

void vtkSlicerGPURayCastVolumeMapper::InitializeRayCast()
{
    RayCastInitialized = 1;

    vtkgl::DeleteShader(this->RayCastVertexShader);
    vtkgl::DeleteShader(this->RayCastFragmentShader);
    vtkgl::DeleteProgram(this->RayCastProgram);

    this->RayCastVertexShader = vtkgl::CreateShader(vtkgl::VERTEX_SHADER);
    this->RayCastFragmentShader = vtkgl::CreateShader(vtkgl::FRAGMENT_SHADER);
    this->RayCastProgram = vtkgl::CreateProgram();

    LoadVertexShader();
    LoadFragmentShaders();

    LoadRayCastProgram();
}

void vtkSlicerGPURayCastVolumeMapper::LoadRayCastProgram()
{
  vtkgl::AttachShader(RayCastProgram, RayCastVertexShader);
  vtkgl::AttachShader(RayCastProgram, RayCastFragmentShader);
  vtkgl::LinkProgram(RayCastProgram);

  GLint result;
  vtkgl::GetProgramiv(RayCastProgram, vtkgl::LINK_STATUS, &result);

  if (!result)
    printf("Program Link Status: FALSE\n");

}

void vtkSlicerGPURayCastVolumeMapper::LoadVertexShader()
{
    std::ostringstream vp_oss;
    vp_oss <<
        "varying vec3 ViewDir;                                                                     \n"
        "void main()                                                                               \n"
        "{                                                                                         \n"
        "    gl_Position = ftransform();                                                           \n"
        "    gl_TexCoord[0] = gl_Color;                                                            \n"
        "    ViewDir = vec3(gl_ModelViewMatrix * gl_Vertex);                                       \n"
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

void vtkSlicerGPURayCastVolumeMapper::LoadFragmentShaders()
{
  std::ostringstream fp_oss;
    fp_oss <<
      "varying vec3 ViewDir;                                                                 \n"
      "uniform sampler3D TextureVol;                                                         \n"
      "uniform sampler3D TextureVol1;                                                        \n"
      "uniform sampler2D TextureColorLookup;                                                 \n"
      "uniform mat4 ParaMatrix;                                                              \n"
      "uniform mat4 VolumeMatrix;                                                            \n"
      "//uniform mat4 ParaMatrix1;                                                             \n"
      "                                                                                      \n"
      "//ParaMatrix:                                                                         \n"
      "//EyePos.x,      EyePos.y,      EyePos.z,     Step                                    \n"
      "//VolBBoxLow.x,  VolBBoxLow.y,  VolBBoxLow.z, VolBBoxHigh.x                           \n"
      "//VolBBoxHigh.y, VolBBoxHigh.z, MinDist,      DepthPeelingThreshold,                  \n"
      "//ICPE_s,        GlobalAlpha,   MinMaxLen,    ICPE_t                                  \n"
      "                                                                                      \n"
      "vec4 computeRayEnd()                                                                  \n"
      "{                                                                                     \n"
      "  vec3 o = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);                \n"
      "  vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);              \n"
      "  vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);              \n"
      "  mmn = clamp(mmn, 0.0, 1.0);                                                         \n"
      "  mmx = clamp(mmx, 0.0, 1.0);                                                         \n"
      "                                                                                      \n"
      "  if (all(greaterThanEqual(o, mmn)) && all(lessThanEqual(o, mmx)) )                   \n"
      "    return gl_TexCoord[0];                                                            \n"
      "                                                                                      \n"
      "  vec3 a1 = gl_TexCoord[0].xyz;                                                       \n"
      "  vec3 a2 = a1 + normalize(a1 - o) * length(mmx - mmn);                               \n"
      "  vec3 a3;                                                                            \n"
      "                                                                                      \n"
      "  float halfStep = ParaMatrix[0][3]*0.5;                                              \n"
      "  for (int i = 0; i < 10; i++)                                                        \n"
      "  {                                                                                   \n"
      "    a3 = (a1 + a2) * 0.5;                                                             \n"
      "    if (length(a2 - a1) <= halfStep)                                                  \n"
      "      return vec4(clamp(a3, mmn, mmx), 1.0);                                          \n"
      "    if ( all(greaterThanEqual(a3, mmn)) && all(lessThanEqual(a3, mmx)) )              \n"
      "      a1 = a3;                                                                        \n"
      "    else                                                                              \n"
      "      a2 = a3;                                                                        \n"
      "  }                                                                                   \n"
      "  return vec4(clamp(a3, mmn, mmx), 1.0);                                              \n"
      "}                                                                                     \n"
      "                                                                                      \n"
      "vec4 computeRayOrigin()                                                               \n"
      "{                                                                                     \n"
      "  vec3 o = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);                \n"
      "  vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);              \n"
      "  vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);              \n"
      "  mmn = clamp(mmn, 0.0, 1.0);                                                         \n"
      "  mmx = clamp(mmx, 0.0, 1.0);                                                         \n"
      "                                                                                      \n"
      "  if (all(greaterThanEqual(o, mmn)) && all(lessThanEqual(o, mmx)) )                   \n"
      "    return vec4(o, 1.0);                                                              \n"
      "  else                                                                                \n"
      "    return gl_TexCoord[0];                                                            \n"
      "}                                                                                     \n"
      "                                                                                      \n";
  //color lookup    
  switch(this->GetInput()->GetNumberOfScalarComponents())
  {
  case 1:
    fp_oss <<
      "vec4 voxelColor(vec3 coord)                                                          \n"
      "{                                                                                    \n"
      "  vec4 scalar = texture3D(TextureVol, coord);                                        \n"
      "  return texture2D(TextureColorLookup, vec2(scalar.x, scalar.w));                    \n"
      "}                                                                                    \n"
      "                                                                                     \n";
    break;
  case 2:
    fp_oss <<
      "vec4 voxelColor(vec3 coord)                                                          \n"
      "{                                                                                    \n"
      "  vec4 color = vec4(0);                                                              \n"
      "  vec4 scalar = texture3D(TextureVol, coord);                                        \n"
      "  color = texture2D(TextureColorLookup, vec2(scalar.x, scalar.w));                   \n"
      "  vec4 opacity = texture2D(TextureAlphaLookup, vec2(scalar.y, scalar.w));            \n"
      "  color.w = opacity.w;                                                               \n"
      "  return color;                                                                      \n"
      "}                                                                                    \n";
    break;
  case 3:
  case 4:
    fp_oss <<
      "vec4 voxelColor(vec3 coord)                                                          \n"
      "{                                                                                    \n"
      "  vec4 color = vec4(0);                                                              \n"
      "  color = texture3D(TextureVol, coord);                                              \n"
      "  vec4 scalar = texture3D(TextureVol1, coord);                                       \n"
      "  vec4 opacity = texture2D(TextureAlphaLookup, vec2(color.w, scalar.w));             \n"
      "  color.w = opacity.w;                                                               \n"
      "  return color;                                                                      \n"
      "}                                                                                    \n";
    break;
  }

  //scalar lookup
  switch(this->GetInput()->GetNumberOfScalarComponents())
  {
    case 1:
    case 2:
      fp_oss <<
        "float voxelScalar(vec3 coord)                                                        \n"
        "{                                                                                    \n"
        "  return texture3D(TextureVol, coord).x;                                             \n"
        "}                                                                                    \n";
      break;
    case 3:
    case 4:
      fp_oss <<
        "float voxelScalar(vec3 coord)                                                        \n"
        "{                                                                                    \n"
        "  return texture3D(TextureVol, coord).w;                                             \n"
        "}                                                                                    \n";
      break;
  }
  
  if (this->Technique != 2 && this->Technique != 3) //normal lookup (not needed for MIP and MINIP)
  {
    fp_oss <<
      "vec3 voxelNormal(vec3 coord)                                                           \n"
        "{                                                                                    \n"
        "  {//on-the-fly normal estimation                                                    \n"
        "   //vec4 sample1, sample2;                                                            \n"
        "   //sample1.x = texture3D(TextureVol, coord + vec3(-ParaMatrix1[0][0], 0.0, 0.0)).x;  \n"
        "   //sample1.y = texture3D(TextureVol, coord + vec3(0.0, -ParaMatrix1[0][1], 0.0)).x;  \n"
        "   //sample1.z = texture3D(TextureVol, coord + vec3(0.0, 0.0, -ParaMatrix1[0][2])).x;  \n"
        "   //sample2.x = texture3D(TextureVol, coord + vec3(ParaMatrix1[0][0], 0.0, 0.0)).x;   \n"
        "   //sample2.y = texture3D(TextureVol, coord + vec3(0.0, ParaMatrix1[0][1], 0.0)).x;   \n"
        "   //sample2.z = texture3D(TextureVol, coord + vec3(0.0, 0.0, ParaMatrix1[0][2])).x;   \n"
        "   //vec4 normal = sample1 - sample2;                                                  \n"
        "   //if (length(normal) < 0.001) return vec3(0);                                     \n"
        "   //normal = VolumeMatrix * normalize(normal);                                      \n"
        "   //return gl_NormalMatrix * normal.xyz;                                            \n"
        "  }                                                                                  \n"
        "  {                                                                                  \n"
        "   vec4 normal = texture3D(TextureVol1, coord);                                      \n"
        "   normal = normal * 2.0 - 1.0;                                                      \n"
        "   normal = VolumeMatrix * normal;                                                   \n"
        "   return gl_NormalMatrix * normal.xyz;                                              \n"
        "  }                                                                                  \n"
        "}                                                                                    \n";
  }

  // shading
  if (this->Technique == 0 || this->Technique == 4 || this->Technique == 5)
  {
    fp_oss <<
      "vec4 directionalLight(vec3 coord, vec3 lightDir, vec4 color)                              \n"
      "{                                                                                         \n"
      "  vec3    normal = voxelNormal(coord);                                                    \n"
      "  //if (length(normal) < 0.001)                                                             \n"
      "  //  return gl_FrontMaterial.ambient * color;                                              \n"
      "  normal = normalize(normal);                                                             \n"
      "  float   NdotL = abs(dot(normal, lightDir));                                             \n"
      "  vec4    specular = vec4(0);                                                             \n"
      "  if (NdotL > 0.0)                                                                        \n"
      "  {                                                                                       \n"
      "    float   NdotHV = max( dot( normal, gl_LightSource[0].halfVector.xyz), 0.0);           \n"
      "    specular = (gl_FrontMaterial.specular) * pow(NdotHV, gl_FrontMaterial.shininess)*color;   \n"
      "  }                                                                                           \n"
      "  vec4    diffuse = (gl_FrontMaterial.ambient + gl_FrontMaterial.diffuse * NdotL) * color;    \n"
      "  return (specular + diffuse);                                                                \n"
      "}                                                                                             \n";
  }
  else if (this->Technique == 1)
  {
    fp_oss <<
      "vec4 edgeColoring(vec3 coord, vec4 diffuse)                                              \n"
      "{                                                                                        \n"
      "  vec3    normal = normalize(voxelNormal(coord));                                        \n"
      "  float   NdotV = abs(dot(normal, normalize(-ViewDir)));                                 \n"
      "  return diffuse*NdotV;                                                                  \n"
      "}                                                                                        \n";
  }

  if (this->Technique == 5)
  {
    fp_oss <<
      "float ICPE(vec3 coord, float shading, float alpha, float dist)                           \n"
      "{                                                                                        \n"
      "   float gradMag = texture3D(TextureVol, coord).w;                                       \n"
      "   float base = shading*ParaMatrix[3][3]*(1.0-dist)*(1.0-alpha);                         \n"
      "   if (base > 0.0)                                                                       \n"
      "     return pow(gradMag, pow(base, ParaMatrix[3][0]));                                   \n"
      "   else                                                                                  \n"
      "     return 1.0;                                                                         \n"
      "}                                                                                        \n";
  }
      
  fp_oss <<      
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "  vec4 rayOrigin = computeRayOrigin();                                                 \n"
    "  vec4 rayEnd = computeRayEnd();                                                       \n"
    "  vec3 rayDir = rayEnd.xyz - rayOrigin.xyz;                                            \n"
    "  float rayLen = length(rayDir);                                                       \n"
    "                                                                                       \n"
    " if (rayLen > 1.732)                                                                   \n"
    "   rayLen = 1.732;                                                                     \n"
    "                                                                                       \n"
    "  rayDir = normalize(rayDir);                                                          \n"
    "                                                                                       \n"
    "  //do ray casting                                                                     \n"
    "  vec3 rayStep = rayDir*ParaMatrix[0][3];                                              \n"
    "  vec3 nextRayOrigin = rayOrigin.xyz;                                                  \n"
    "                                                                                       \n"
    "  float fading = 1.0;                                                                  \n"
    "  if (ParaMatrix[3][1] > 0.0001)                                                       \n"
    "  {                                                                                    \n"
    "    vec3 eyePos = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);          \n"
    "    vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);             \n"
    "    vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);             \n"
    "    mmn = clamp(mmn, 0.0, 1.0);                                                        \n"
    "    mmx = clamp(mmx, 0.0, 1.0);                                                        \n"
    "                                                                                       \n"
    "    if ( any(lessThanEqual(eyePos, mmn)) || any(greaterThanEqual(eyePos, mmx)) )       \n"
    "    {                                                                                  \n"
    "      float toEyeDist = length(nextRayOrigin - eyePos);                                \n"
    "      fading = 1.0 - (toEyeDist - ParaMatrix[2][2])/ParaMatrix[3][2];                  \n"
    "    }                                                                                  \n"
    "  }                                                                                    \n"
    "                                                                                       \n"
    "  vec4 pixelColor = vec4(0);                                                           \n"
    "  float alpha = 0.0;                                                                   \n"
    "  float t = 0.0;                                                                       \n"
    "  vec3  lightDir = normalize( gl_LightSource[0].position.xyz );                        \n"
    "  float depthPeeling = ParaMatrix[2][3];                                               \n"
    "                                                                                       \n"
    "  while( t < rayLen)                                                                   \n"
    "  {                                                                                    \n"
    "    if ( voxelScalar(nextRayOrigin) >= depthPeeling )                                  \n"
    "      break;                                                                           \n"
    "    t += ParaMatrix[0][3];                                                             \n"
    "    nextRayOrigin += rayStep;                                                          \n"
    "    fading -= ParaMatrix[3][1]*ParaMatrix[0][3];                                       \n"
    "  }                                                                                    \n";

  switch(this->Technique)
  {
    case 0:
      fp_oss <<
        "  while( (t < rayLen) && (alpha < 0.985) )                                          \n"
        "  {                                                                                 \n"
        "    vec4 nextColor = voxelColor(nextRayOrigin);                                     \n"
        "    float tempAlpha = nextColor.w;                                                  \n"
        "    nextColor *= vec4(fading, fading, fading, 1.0);                                 \n"
        "                                                                                    \n"
        "    if (tempAlpha > 0.0)                                                            \n"
        "    {                                                                               \n"
        "      nextColor = directionalLight(nextRayOrigin, lightDir, nextColor);             \n"
        "                                                                                    \n"
        "      tempAlpha = (1.0-alpha)*tempAlpha;                                            \n"
        "      pixelColor += nextColor*tempAlpha;                                            \n"
        "      alpha += tempAlpha;                                                           \n"
        "    }                                                                               \n"
        "                                                                                    \n"
        "    t += ParaMatrix[0][3];                                                          \n"
        "    nextRayOrigin += rayStep;                                                       \n"
        "    fading -= ParaMatrix[3][1]*ParaMatrix[0][3];                                    \n"
        "  }                                                                                 \n"
        "    gl_FragColor = vec4(pixelColor.xyz, alpha);                                     \n"
        "}                                                                                   \n";
      break;
    case 1:
      fp_oss <<
        "  while( (t < rayLen) && (alpha < 0.985) )                                          \n"
        "  {                                                                                 \n"
        "    vec4 nextColor = voxelColor(nextRayOrigin);                                     \n"
        "    float tempAlpha = nextColor.w;                                                  \n"
        "    nextColor *= vec4(fading, fading, fading, 1.0);                                 \n"
        "                                                                                    \n"
        "    if (tempAlpha > 0.0)                                                            \n"
        "    {                                                                               \n"
        "      nextColor = edgeColoring(nextRayOrigin, nextColor);                           \n"
        "      tempAlpha = (1.0-alpha)*tempAlpha;                                            \n"
        "      pixelColor += nextColor*tempAlpha;                                            \n"
        "      alpha += tempAlpha;                                                           \n"
        "    }                                                                               \n"
        "                                                                                    \n"
        "    t += ParaMatrix[0][3];                                                          \n"
        "    nextRayOrigin += rayStep;                                                       \n"
        "    fading -= ParaMatrix[3][1]*ParaMatrix[0][3];                                    \n"
        "  }                                                                                 \n"
        "  gl_FragColor = vec4(pixelColor.xyz, alpha);                                       \n"
        "}                                                                                   \n";
      break;
    case 2:
      fp_oss <<
        "  float maxScalar = voxelScalar(nextRayOrigin);                                     \n"
        "  vec3 maxScalarCoord = nextRayOrigin;                                              \n"
        "  float maxFading = fading;                                                         \n"
        "                                                                                    \n"
        "  while( t < rayLen )                                                               \n"
        "  {                                                                                 \n"
        "    float scalar = voxelScalar(nextRayOrigin);                                      \n"
        "    if (maxScalar < scalar)                                                         \n"
        "    {                                                                               \n"
        "      maxScalar = scalar;                                                           \n"
        "      maxScalarCoord = nextRayOrigin;                                               \n"
        "      maxFading = fading;                                                           \n"
        "    }                                                                               \n"
        "                                                                                    \n"
        "    fading -= ParaMatrix[3][1]*ParaMatrix[0][3];                                    \n"
        "    t += ParaMatrix[0][3];                                                          \n"
        "    nextRayOrigin += rayStep;                                                       \n"
        "  }                                                                                 \n"
        "                                                                                    \n"
        "  pixelColor = voxelColor(maxScalarCoord) * vec4(maxFading, maxFading, maxFading, 1.0);      \n"
        "  alpha = pixelColor.w;                                                             \n"
        "  gl_FragColor = vec4(pixelColor.xyz, alpha);                                       \n"
        "}                                                                                   \n";
      break;
    case 3:
      fp_oss <<
        "  float minScalar = voxelScalar(nextRayOrigin);                                     \n"
        "  vec3 minScalarCoord = nextRayOrigin;                                              \n"
        "  while( t < rayLen )                                                               \n"
        "  {                                                                                 \n"
        "    float scalar = voxelScalar(nextRayOrigin);                                      \n"
        "    if (minScalar > scalar)                                                         \n"
        "    {                                                                               \n"
        "      minScalar = scalar;                                                           \n"
        "      minScalarCoord = nextRayOrigin;                                               \n"
        "    }                                                                               \n"
        "                                                                                    \n"
        "    t += ParaMatrix[0][3];                                                          \n"
        "    nextRayOrigin += rayStep;                                                       \n"
        "  }                                                                                 \n"
        "                                                                                    \n"
        "  pixelColor = voxelColor(minScalarCoord);                                          \n"
        "  alpha = pixelColor.w;                                                             \n"
        "  gl_FragColor = vec4(pixelColor.xyz, alpha);                                       \n"
        "}                                                                                   \n";
      break;
    case 4:
      fp_oss <<
        "  while( (t < rayLen) && (alpha < 0.985) )                                          \n"
        "  {                                                                                 \n"
        "    vec4 nextColor = voxelColor(nextRayOrigin);                                     \n"
        "    float tempAlpha = nextColor.w;                                                  \n"
        "    nextColor *= vec4(fading, fading, fading, 1.0);                                 \n"
        "                                                                                    \n"
        "    if (tempAlpha > 0.0)                                                            \n"
        "    {                                                                               \n"
        "      nextColor = directionalLight(nextRayOrigin, lightDir, nextColor);             \n"
        "                                                                                    \n"
        "      tempAlpha = (1.0-alpha)*tempAlpha*texture3D(TextureVol, nextRayOrigin).w;     \n"
        "      pixelColor += nextColor*tempAlpha;                                            \n"
        "      alpha += tempAlpha;                                                           \n"
        "    }                                                                               \n"
        "                                                                                    \n"
        "    t += ParaMatrix[0][3];                                                          \n"
        "    nextRayOrigin += rayStep;                                                       \n"
        "    fading -= ParaMatrix[3][1]*ParaMatrix[0][3];                                    \n"
        "  }                                                                                 \n"
        "    gl_FragColor = vec4(pixelColor.xyz, alpha);                                     \n"
        "}                                                                                   \n";
      break;
    case 5:
      fp_oss <<
        "  vec3 eyePos = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);         \n"
        "  while( (t < rayLen) && (alpha < 0.985) )                                          \n"
        "  {                                                                                 \n"
        "    vec4 nextColor = voxelColor(nextRayOrigin);                                     \n"
        "    float tempAlpha = nextColor.w;                                                  \n"
        "                                                                                    \n"
        "    if (tempAlpha > 0.0)                                                            \n"
        "    {                                                                               \n"
        "      nextColor = directionalLight(nextRayOrigin, lightDir, nextColor);             \n"
        "      float toEyeDist = (length(nextRayOrigin - eyePos) - ParaMatrix[2][2])/ParaMatrix[3][2];\n"
        "      float icpe = ICPE(nextRayOrigin, length(nextColor), alpha, toEyeDist);        \n"
        "                                                                                    \n"
        "      tempAlpha = (1.0-alpha)*tempAlpha*icpe;                                       \n"
        "      pixelColor += nextColor*tempAlpha;                                            \n"
        "      alpha += tempAlpha;                                                           \n"
        "    }                                                                               \n"
        "                                                                                    \n"
        "    t += ParaMatrix[0][3];                                                          \n"
        "    nextRayOrigin += rayStep;                                                       \n"
        "  }                                                                                 \n"
        "  gl_FragColor = vec4(pixelColor.xyz, alpha);                                       \n"
        "}                                                                                   \n";
      break;
  }

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

void vtkSlicerGPURayCastVolumeMapper::PrintGLErrorString()
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

void vtkSlicerGPURayCastVolumeMapper::PrintFragmentShaderInfoLog()
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

void vtkSlicerGPURayCastVolumeMapper::SetTechnique(int tech)
{
    this->Technique = tech;
    this->ReloadShaderFlag = 1;
}

void vtkSlicerGPURayCastVolumeMapper::SetInternalVolumeSize(int size)
{
    if (this->InternalVolumeSize != size)
    {
        this->InternalVolumeSize = size;
        this->SavedTextureInput = NULL;//dirty input, force reprocess input
    }
}


