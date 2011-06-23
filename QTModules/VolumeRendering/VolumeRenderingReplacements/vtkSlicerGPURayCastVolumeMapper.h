/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerGPURayCastVolumeMapper.h,v $

   Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   GPGPU Ray Cast Mapper
  Module:    $HeadURL: $
  Date:      $Date: 2009-01-07  $
  Version:   $Revision:  $


=========================================================================*/
// .NAME vtkSlicerGPURayCastVolumeMapper - concrete implementation of 3D volume texture mapping

// .SECTION Description
// vtkSlicerGPURayCastVolumeMapper renders a volume using ray casting on 3D texture.
// See vtkVolumeTextureMapper3D for full description.

// .SECTION see also
// vtkVolumeTextureMapper3D vtkVolumeMapper

#ifndef __vtkSlicerGPURayCastVolumeMapper_h
#define __vtkSlicerGPURayCastVolumeMapper_h

#include "vtkSlicerGPUVolumeMapper.h"
#include "VolumeRenderingReplacementsExport.h"


#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h" // GLfloat type is used in some method signatures.
#endif

class vtkMatrix4x4;
class vtkRenderWindow;
class vtkVolumeProperty;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_QTMODULES_VOLUMERENDERING_REPLACEMENTS_EXPORT vtkSlicerGPURayCastVolumeMapper : public vtkSlicerGPUVolumeMapper
{
public:
  vtkTypeRevisionMacro(vtkSlicerGPURayCastVolumeMapper,vtkSlicerGPUVolumeMapper);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkSlicerGPURayCastVolumeMapper *New();

  // Description:
  // Distance color blending (closer voxels brighter, more far voxels darker)
  // Default value: 0.0, no blending
  vtkSetMacro(DistanceColorBlending, float);
  vtkGetMacro(DistanceColorBlending, float);

  // Description:
  // Depth peeling threshold
  vtkSetMacro(DepthPeelingThreshold, float);
  vtkGetMacro(DepthPeelingThreshold, float);

  // Description:
  // ICPE kt and ks
  vtkSetMacro(ICPEScale, float);
  vtkSetMacro(ICPESmoothness, float);

  // Description:
  // set internal volume size
  void SetInternalVolumeSize(int size);

  // Description:
  // Enable/Disable clipping
  vtkSetMacro(Clipping, int);
  vtkGetMacro(Clipping,int);
  vtkBooleanMacro(Clipping,int);

  // Description:
  // Set technique
  void SetTechnique(int tech);

  // Description:
  // Is hardware rendering supported? No if the input data is
  // more than one independent component, or if the hardware does
  // not support the required extensions
  int IsRenderSupported(vtkRenderWindow* window, vtkVolumeProperty *);

//BTX

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  // Render the volume
  virtual void Render(vtkRenderer *ren, vtkVolume *vol);

//ETX

  // Desciption:
  // Initialize when we go to render, or go to answer the
  // IsRenderSupported question. Don't call unless we have
  // a valid OpenGL context!
  vtkGetMacro( Initialized, int );

  // Description:
  // Release any graphics resources that are being consumed by this texture.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

protected:
  vtkSlicerGPURayCastVolumeMapper();
  ~vtkSlicerGPURayCastVolumeMapper();

//ETX

  int              Initialized;
  int              RayCastInitialized;

  int              RayCastSupported;

  int              Technique;
  int              Clipping;
  int              Shading;
  int              InternalVolumeSize;

  GLuint           Volume1Index;
  GLuint           Volume2Index;
  GLuint           ColorLookupIndex;
  GLuint           RayCastVertexShader;
  GLuint           RayCastFragmentShader;
  GLuint           RayCastProgram;
  vtkRenderWindow *RenderWindow;

  double           VolumeBBoxVertices[8][3];
  double           VolumeBBoxVerticesColor[8][3];

  GLfloat          ParaMatrix[16];//4x4 matrix uniform for ray casting parameters
  GLfloat          ParaMatrix1[16];
  
  float            RaySteps;
  float            DistanceColorBlending;
  float            DepthPeelingThreshold;

  float            ICPEScale;
  float            ICPESmoothness;

  void Initialize(vtkRenderWindow* ren);
  void InitializeRayCast();

  void RenderGLSL(vtkRenderer *pRen, vtkVolume *pVol);
  void SetupTextures( vtkRenderer *ren, vtkVolume *vol );

  void DeleteTextureIndex( GLuint *index );
  void CreateTextureIndex( GLuint *index );

  void DrawVolumeBBox();

  void SetupRayCastParameters( vtkRenderer *pRen, vtkVolume *pVol);

  void LoadVertexShader();
  void LoadFragmentShaders();

  void LoadRayCastProgram();

  void AdaptivePerformanceControl();
  void PerformanceControl();

  // Description:
  // Check if we can support this texture size.
  int IsTextureSizeSupported( int size[3] );

  // Description:
  // Common code for setting up interpolation / clamping on 3D textures
  void Setup3DTextureParameters( vtkVolumeProperty *property );

private:
  vtkSlicerGPURayCastVolumeMapper(const vtkSlicerGPURayCastVolumeMapper&);  // Not implemented.
  void operator=(const vtkSlicerGPURayCastVolumeMapper&);  // Not implemented.

  void PrintGLErrorString();
  void PrintFragmentShaderInfoLog();

  int ReloadShaderFlag;
};


#endif




