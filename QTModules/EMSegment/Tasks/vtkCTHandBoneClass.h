#ifndef __vtkCTHandBoneClass_h
#define __vtkCTHandBoneClass_h
  
#include "vtkEMSegment.h"
#include "vtkImageData.h"
#include "vtkObject.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiscreteGaussianImageFilter.h"

#include "itkFlipImageFilter.h"
#include "itkImageSeriesReader.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkGDCMImageIO.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include "itkTernaryAddImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkIdentityTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkRecursiveGaussianImageFilter.h"
#include "itkCastImageFilter.h"

#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkLineConstIterator.h"
#include "itkLineIterator.h"

#include "itkSpatialOrientationAdapter.h"
#include "vtkDoubleArray.h"

class VTK_EMSEGMENT_EXPORT vtkCTHandBoneClass : public vtkObject
{
  public:
  static vtkCTHandBoneClass *New();
  vtkTypeMacro(vtkCTHandBoneClass,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  int blur(const char* inputImage, const char* outputImage, const char* gaussianVariance, const char* maxKernelWidth);
  int binary_threshold(const char* inputDirectory, const char* outputThresholdImage, const char* _lowerThreshold, const char* _upperThreshold);
  int flip( const char* inputFile, const char* outputFlippedImageFile, const char* flipAxisX, const char* flipAxisY, const char* flipAxisZ );
  //int fiducial_threshold(const char* inputImage, const char* outputImage, const char* fiducialfile, const char* logfile )
  int fiducial_threshold(const char* inputImage, const char* outputImage);
  int largest_component( const char* inputImage, const char* outputImage );


protected:
  vtkCTHandBoneClass();
//  vtkCTHandBoneClass(const vtkCTHandBoneClass&) {};
  ~vtkCTHandBoneClass(){};


//  void DeleteClassVariables();

//  void operator=(const vtkCTHandBoneClass&) {};
//  void ThreadedExecute(vtkImageData **inData, vtkImageData *outData,int outExt[6], int id) {};
//  void ExecuteData(vtkDataObject *);

};
#endif
