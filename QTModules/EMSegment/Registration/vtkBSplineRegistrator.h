#ifndef __vtkBSplineRegistrator_h
#define __vtkBSplineRegistrator_h

#include "vtkObject.h"

#include "vtkImageData.h"
#include "vtkMatrix4x4.h"
#include "vtkGridTransform.h"
#include "vtkTransform.h"
#include "vtkEMSegmentWin32Header.h"

class VTK_EMSEGMENT_EXPORT vtkBSplineRegistrator : 
  public vtkObject
{
public:
  static vtkBSplineRegistrator *New();
  vtkTypeRevisionMacro(vtkBSplineRegistrator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //BTX
  typedef enum 
  {
    MutualInformation,
    CrossCorrelation,
    MeanSquaredError
  } MetricType;
  //ETX
  static const char* GetStringFromMetricType(MetricType);

  vtkSetObjectMacro(FixedImage, vtkImageData);
  vtkGetObjectMacro(FixedImage, vtkImageData);
  
  vtkSetObjectMacro(MovingImage, vtkImageData);
  vtkGetObjectMacro(MovingImage, vtkImageData);

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

  vtkSetMacro(NumberOfIterations, int);
  vtkGetMacro(NumberOfIterations, int);

  vtkSetClampMacro(MetricComputationSamplingRatio, double, 0, 1);
  vtkGetMacro(MetricComputationSamplingRatio, double);

  vtkSetMacro(NumberOfKnotPoints, int);
  vtkGetMacro(NumberOfKnotPoints, int);

  vtkSetObjectMacro(BulkTransform, vtkTransform);
  vtkGetObjectMacro(BulkTransform, vtkTransform);
  
  vtkGetObjectMacro(Transform, vtkGridTransform);

  vtkSetObjectMacro(FixedIJKToXYZ, vtkMatrix4x4);
  vtkSetObjectMacro(MovingIJKToXYZ, vtkMatrix4x4);

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

  vtkTransform*                   BulkTransform;
  vtkGridTransform*               Transform;

  int                             NumberOfIterations;
  MetricType                      ImageToImageMetric;
  InterpolationType               IntensityInterpolationType;
  double                          MetricComputationSamplingRatio;

  int                             NumberOfKnotPoints;
};

#endif // __vtkBSplineRegistrator_h
