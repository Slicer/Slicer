/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerGPUMultiVolumeMapper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSlicerGPUMultiVolumeMapper - GPU volume render with 3D texture mapping (GLSL)

// .SECTION Description
// vtkSlicerGPUMultiVolumeMapper renders two volume using 3D texture mapping in fragment shader. 
// This class is actually an abstract superclass - with all the actual
// work done by vtkSlicerGPURayCastVolumeMapper. 
// 
// This mappers currently supports:
//
// - any data type as input
// - one component
// - composite blending
// - mip and minip
// - two input volumes
//
// This mapper does not support:
// - more than one component
// 
// Two input volumes must have same dimension, spacing and number of components
//
// .SECTION see also
// vtkVolumeMapper

#ifndef __vtkSlicerGPUMultiVolumeMapper_h
#define __vtkSlicerGPUMultiVolumeMapper_h

#include "vtkVolumeMapper.h"
#include "qSlicerVolumeRenderingModuleExport.h"


class vtkMultiThreader;
class vtkImageData;
class vtkColorTransferFunction;
class vtkPiecewiseFunction;
class vtkVolumeProperty;

class vtkSlicerGPUMultiVolumeMapper;
typedef struct{
    float *dataPtr;
    vtkSlicerGPUMultiVolumeMapper *me;
    double scalarRange[2];
    unsigned char *volume;
}GPUGradientsArgsType;

class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT vtkSlicerGPUMultiVolumeMapper : public vtkVolumeMapper
{
public:
  vtkTypeRevisionMacro(vtkSlicerGPUMultiVolumeMapper,vtkVolumeMapper);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkSlicerGPUMultiVolumeMapper *New();

  // Description:
  // Desired frame rate
  vtkSetMacro(Framerate, float);
  vtkGetMacro(Framerate, float);
  
  // Description:
  // Set/Get the nth input data, could be label map or just another volume
  virtual void SetNthInput( int index, vtkImageData *);
  virtual void SetNthInput( int index, vtkDataSet *);
  vtkImageData *GetNthInput(int index);
  
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

//BTX

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  // Render the volume
  virtual void Render(vtkRenderer *, vtkVolume *) {};   

protected:
  vtkSlicerGPUMultiVolumeMapper();
  ~vtkSlicerGPUMultiVolumeMapper();

  float                     ScalarOffset;
  float                     ScalarScale;
  
  float                     ScalarOffset2nd;
  float                     ScalarScale2nd;
  
  float                     Framerate;
  
  unsigned char            *Volume1;
  unsigned char            *Volume2;
  unsigned char            *Volume3;
  int                       VolumeSize;
  int                       VolumeDimensions[3];
  float                     VolumeSpacing[3];
  
  vtkImageData             *SavedTextureInput;
  vtkImageData             *SavedTextureInput2nd;
  
  vtkColorTransferFunction *SavedRGBFunction;
  vtkPiecewiseFunction     *SavedGrayFunction;
  vtkPiecewiseFunction     *SavedScalarOpacityFunction;
  vtkPiecewiseFunction     *SavedGradientOpacityFunction;
  int                       SavedColorChannels;
  float                     SavedScalarOpacityDistance;
  
  vtkColorTransferFunction *SavedRGBFunction2nd;
  vtkPiecewiseFunction     *SavedGrayFunction2nd;
  vtkPiecewiseFunction     *SavedScalarOpacityFunction2nd;
  vtkPiecewiseFunction     *SavedGradientOpacityFunction2nd;
  int                       SavedColorChannels2nd;
  float                     SavedScalarOpacityDistance2nd;
  
  unsigned char             ColorLookup[256*256*4];
  float                     TempArray1[3*4096];
  float                     TempArray2[4096];
  int                       ColorTableSize;
  
  unsigned char             ColorLookup2nd[256*256*4];
  float                     TempArray11[3*4096];
  float                     TempArray21[4096];
  int                       ColorTableSize2nd;
  
  vtkTimeStamp              SavedTextureMTime;
  vtkTimeStamp              SavedTextureMTime2nd;
  
  vtkTimeStamp              SavedColorOpacityMTime;
  vtkTimeStamp              SavedColorOpacityMTime2nd;
  
  vtkMultiThreader          *Threader;
  
  GPUGradientsArgsType         *GradientsArgs;

  // Description:
  // Update the internal RGBA representation of the volume. Return 1 if
  // anything change, 0 if nothing changed.
  
  int    UpdateVolumes( vtkVolume * );
  int    UpdateColorLookup( vtkVolume * );
  
  void   CopyToFloatBuffer(vtkImageData* input, float* floatDataPtr, int dataPtrSize);

  // Description:
  // Impemented in subclass - check is texture size is OK.
  //BTX
  virtual int IsTextureSizeSupported( int [3] ) {return 0;};
  //ETX
  
  friend VTK_THREAD_RETURN_TYPE vtkSlicerGPUMultiVolumeMapperComputeGradients( void *arg );
  
private:
  vtkSlicerGPUMultiVolumeMapper(const vtkSlicerGPUMultiVolumeMapper&);  // Not implemented.
  void operator=(const vtkSlicerGPUMultiVolumeMapper&);  // Not implemented.
};


#endif






