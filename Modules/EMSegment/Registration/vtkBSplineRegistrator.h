#ifndef __vtkBSplineRegistrator_h
#define __vtkBSplineRegistrator_h

#include "vtkObject.h"
#include "vtkImageData.h"
#include "vtkGridTransform.h"
#include "vtkEMSegmentWin32Header.h"

class VTK_EMSEGMENT_EXPORT vtkBSplineRegistrator : 
  public vtkObject
{
public:
  static vtkBSplineRegistrator *New();
  vtkTypeRevisionMacro(vtkBSplineRegistrator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetObjectMacro(FixedImage, vtkImageData);
  vtkGetObjectMacro(FixedImage, vtkImageData);

  vtkSetObjectMacro(MovingImage, vtkImageData);
  vtkGetObjectMacro(MovingImage, vtkImageData);

  //BTX
  typedef enum 
  {
    MutualInformation,
    CrossCorrelation,
    MeanSquaredError
  } MetricType;
  //ETX
  static const char* GetStringFromMetricType(MetricType);

  vtkSetMacro(ImageToImageMetric, MetricType);
  vtkGetMacro(ImageToImageMetric, MetricType);
  void SetImageToImageMetricToMutualInformation()
  { this->SetImageToImageMetric(MutualInformation); }
  void SetImageToImageMetricToCrossCorrelation()
  { this->SetImageToImageMetric(CrossCorrelation); }
  void SetImageToImageMetricToMeanSquaredError()
  { this->SetImageToImageMetric(MeanSquaredError); }

  //BTX
  typedef enum 
  {
    NearestNeighbor,
    Linear
  } InterpolationType;
  //ETX
  static const char* GetStringFromInterpolationType(InterpolationType);

  vtkSetMacro(IntensityInterpolationType, InterpolationType);
  vtkGetMacro(IntensityInterpolationType, InterpolationType);
  void SetIntensityInterpolationTypeToNearestNeighbor()
  { this->SetIntensityInterpolationType(NearestNeighbor); }
  void SetIntensityInterpolationTypeToLinear()
  { this->SetIntensityInterpolationType(Linear); }

  //BTX
  typedef enum 
  {
    Identity,
    CentersOfMass,
    ImageCenters
  } InitializationType;
  //ETX
  static const char* 
  GetStringFromTransformInitializationType(InitializationType);

  vtkSetMacro(TransformInitializationType, InitializationType);
  vtkGetMacro(TransformInitializationType, InitializationType);  
  void SetTransformInitializationTypeToIdentity()
  { this->SetTransformInitializationType(Identity); }
  void SetTransformInitializationTypeToCentersOfMass()
  { this->SetTransformInitializationType(CentersOfMass); }
  void SetTransformInitializationTypeToImageCenters()
  { this->SetTransformInitializationType(ImageCenters); }

  vtkSetMacro(NumberOfIterations, int);
  vtkGetMacro(NumberOfIterations, int);

  vtkSetClampMacro(MetricComputationSamplingRatio, double, 0, 1);
  vtkGetMacro(MetricComputationSamplingRatio, double);

  vtkGetObjectMacro(Transform, vtkGridTransform);

  void RegisterImages();

protected:
  vtkBSplineRegistrator();
  virtual ~vtkBSplineRegistrator();

  //
  // these functions are used to deal with combining the voxel types
  //of the fixed and moving images into a single type for registration
  //BTX
  template <class TFixedImageVoxel>
  void RegisterImagesInternal1();
  template <class TFixedImageVoxel, class TMovingImageVoxel>
  void RegisterImagesInternal2();
  template <class CommonVoxelType>
  void RegisterImagesInternal3();
  //ETX

private:
  vtkBSplineRegistrator(const vtkBSplineRegistrator&);  // not implemented
  void operator=(const vtkBSplineRegistrator&);        // not implemented

  vtkImageData*                   FixedImage;
  vtkImageData*                   MovingImage;

  vtkGridTransform*               Transform;

  int                             NumberOfIterations;
  MetricType                      ImageToImageMetric;
  InterpolationType               IntensityInterpolationType;
  InitializationType              TransformInitializationType;
  double                          MetricComputationSamplingRatio;
};

#endif // __vtkBSplineRegistrator_h
