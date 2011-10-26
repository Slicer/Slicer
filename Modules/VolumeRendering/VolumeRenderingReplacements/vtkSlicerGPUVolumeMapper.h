/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerGPUVolumeMapper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSlicerGPUVolumeMapper - GPU volume render with 3D texture mapping (GLSL)

// .SECTION Description
// vtkSlicerGPUVolumeMapper renders a volume using 3D texture mapping in fragment shader. 
// This class is actually an abstract superclass - with all the actual
// work done by vtkSlicerGPURayCastVolumeTextureMapper3D. 
// 
// This mappers currently supports:
//
// - any data type as input
// - one component, or two or four non-independent components
// - composite blending
// - intermixed opaque geometry
// - multiple volumes can be rendered if they can
//   be sorted into back-to-front order (use the vtkFrustumCoverageCuller)
// 
// This mapper does not support:
// - more than one independent component
// - maximum intensity projection
// 
// Internally, this mapper will potentially change the resolution of the
// input data. The data will be resampled to be a power of two in each
// direction, and also no greater than 128*256*256 voxels (any aspect) 
// for one or two component data, or 128*128*256 voxels (any aspect)
// for four component data. The limits are currently hardcoded after 
// a check using the GL_PROXY_TEXTURE3D because some graphics drivers 
// were always responding "yes" to the proxy call despite not being
// able to allocate that much texture memory.
//
// Currently, calculations are computed using 8 bits per RGBA channel.
// In the future this should be expanded to handle newer boards that
// can support 15 bit float compositing.
//

// .SECTION see also
// vtkVolumeMapper

#ifndef __vtkSlicerGPUVolumeMapper_h
#define __vtkSlicerGPUVolumeMapper_h

#include "vtkVolumeMapper.h"
#include "vtkVolumeRenderingReplacements.h"

class vtkMultiThreader;
class vtkImageData;
class vtkColorTransferFunction;
class vtkPiecewiseFunction;
class vtkVolumeProperty;

VTK_THREAD_RETURN_TYPE vtkSlicerGPUVolumeMapperComputeGradients(void *arg);

class vtkSlicerGPUVolumeMapper;
struct GradientsArgsType
{
  float *dataPtr;
  vtkSlicerGPUVolumeMapper *me;
  double scalarRange[2];
  unsigned char *volume1;
  unsigned char *volume2;
};

class VTK_VOLUMERENDERINGREPLACEMENTS_EXPORT vtkSlicerGPUVolumeMapper : public vtkVolumeMapper
{
public:
  vtkTypeRevisionMacro(vtkSlicerGPUVolumeMapper,vtkVolumeMapper);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkSlicerGPUVolumeMapper *New();

  // Description:
  // Desired frame rate
  vtkSetMacro(Framerate, float);
  vtkGetMacro(Framerate, float);
  
  // Description:
  // These are the dimensions of the 3D texture
  vtkGetVectorMacro( VolumeDimensions, int,   3 );
  
  // Description:
  // This is the spacing of the 3D texture
  vtkGetVectorMacro( VolumeSpacing,    float, 3 );

  // Description:
  // Based on hardware and properties, we may or may not be able to
  // render using 3D texture mapping. This indicates if 3D texture
  // mapping is supported by the hardware, and if the other extensions
  // necessary to support the specific properties are available.
  virtual int IsRenderSupported( vtkVolumeProperty * ) {return 0;};


  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  // Render the volume
  virtual void Render(vtkRenderer *, vtkVolume *) {};   

protected:
  vtkSlicerGPUVolumeMapper();
  ~vtkSlicerGPUVolumeMapper();

  float                     ScalarOffset;
  float                     ScalarScale;
  
  float                     Framerate;
  
  unsigned char            *Volume1;
  unsigned char            *Volume2;
  int                       VolumeSize;
  int                       VolumeComponents;
  int                       VolumeDimensions[3];
  float                     VolumeSpacing[3];
  
  vtkImageData             *SavedTextureInput;
  vtkImageData             *SavedParametersInput;
  
  vtkColorTransferFunction *SavedRGBFunction;
  vtkPiecewiseFunction     *SavedGrayFunction;
  vtkPiecewiseFunction     *SavedScalarOpacityFunction;
  vtkPiecewiseFunction     *SavedGradientOpacityFunction;
  int                       SavedColorChannels;
  float                     SavedScalarOpacityDistance;
  
  unsigned char             ColorLookup[65536*4];
  float                     TempArray1[3*4096];
  float                     TempArray2[4096];
  int                       ColorTableSize;
  
  vtkTimeStamp              SavedTextureMTime;
  vtkTimeStamp              SavedParametersMTime;
  
  vtkMultiThreader          *Threader;
  
  GradientsArgsType         *GradientsArgs;

  // Description:
  // Update the internal RGBA representation of the volume. Return 1 if
  // anything change, 0 if nothing changed.
  int    UpdateVolumes( vtkVolume * );
  int    UpdateColorLookup( vtkVolume * );

  void CopyToFloatBuffer(vtkImageData* input, float* floatDataPtr, int dataPtrSize);
  
  // Description:
  // Impemented in subclass - check is texture size is OK.
  virtual int IsTextureSizeSupported( int [3] ) {return 0;};
  
  friend VTK_THREAD_RETURN_TYPE vtkSlicerGPUVolumeMapperComputeGradients( void *arg );
  
private:
  vtkSlicerGPUVolumeMapper(const vtkSlicerGPUVolumeMapper&);  // Not implemented.
  void operator=(const vtkSlicerGPUVolumeMapper&);  // Not implemented.
};


#endif






