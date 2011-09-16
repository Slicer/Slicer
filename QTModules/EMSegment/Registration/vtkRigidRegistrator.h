#ifndef __vtkRigidRegistrator_h
#define __vtkRigidRegistrator_h

#include "vtkObject.h"
#include "vtkImageData.h"
#include "vtkTransform.h"
#include "vtkEMSegment.h"

class VTK_EMSEGMENT_EXPORT vtkRigidRegistrator : 
  public vtkObject
{
public:
  static vtkRigidRegistrator *New();
  vtkTypeRevisionMacro(vtkRigidRegistrator, vtkObject);
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
    Linear,
    Cubic
  } InterpolationType;
  //ETX
  static const char* GetStringFromInterpolationType(InterpolationType);

  vtkSetMacro(IntensityInterpolationType, InterpolationType);
  vtkGetMacro(IntensityInterpolationType, InterpolationType);
  void SetIntensityInterpolationTypeToNearestNeighbor()
  { this->SetIntensityInterpolationType(NearestNeighbor); }
  void SetIntensityInterpolationTypeToLinear()
  { this->SetIntensityInterpolationType(Linear); }
  void SetIntensityInterpolationTypeToCubic()
  { this->SetIntensityInterpolationType(Cubic); }

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

  vtkGetObjectMacro(Transform, vtkTransform);

  vtkSetObjectMacro(FixedIJKToXYZ, vtkMatrix4x4);
  vtkSetObjectMacro(MovingIJKToXYZ, vtkMatrix4x4);

  void RegisterImages();

protected:
  vtkRigidRegistrator();
  virtual ~vtkRigidRegistrator();

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
  vtkRigidRegistrator(const vtkRigidRegistrator&);  // not implemented
  void operator=(const vtkRigidRegistrator&);        // not implemented


  //
  // Deal with orientation.  Permute images and setup origin and
  // spacing so that both images are measured in XYZ basis vectors
  // with only spacing and origin information (no need for direction
  // matrix or cosines).  This way ITK will do registration in XYZ
  // coordinates.
  static void
    ComputeReorientationInformation(const vtkMatrix4x4* IJKToXYZ,
                                    int*    filteredAxesForPermuteFilter,
                                    double* originForChangeInformationFilter,
                                    double* spacingForChangeInformationFilter);

  vtkImageData*                   FixedImage;
  vtkImageData*                   MovingImage;

  vtkMatrix4x4*                   FixedIJKToXYZ;
  vtkMatrix4x4*                   MovingIJKToXYZ;

  vtkTransform*                   Transform;

  int                             NumberOfIterations;
  MetricType                      ImageToImageMetric;
  InterpolationType               IntensityInterpolationType;
  InitializationType              TransformInitializationType;
  double                          MetricComputationSamplingRatio;
};

#endif // __vtkRigidRegistrator_h
