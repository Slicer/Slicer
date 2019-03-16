
#ifndef __vtkITKGrowCutSegmentationImageFilter_h
#define __vtkITKGrowCutSegmentationImageFilter_h

#include "vtkITK.h"

// VTK includes
#include <vtkImageAlgorithm.h>
#include <vtkVersion.h>

class vtkImageData;

/// \brief- Wrapper class around itk::GrowCutSegmentationImageFilter
///
/// GrowCutSegmentationImageFilter produces segmentation of regions on an image interactively.
/// It uses the positive and  negative gestures (strokes/ lines/points marked by the user on the region of interest (positive),
/// and outside the object (negative) to automatically generate the segmentation
///
/// Usage: SetInput1 is the input feature/intensity image (required)
/// SetInput2 takes the gesture image (the gestures) image (required)
/// SetInput3 takes the previous segmented image (optional)
///
/// GetOutput produces the output segmented image
///
/// This filter is implemented only for scalar images gray scale images.
/// The current implementation supports n-class segmentation.
class VTK_ITK_EXPORT vtkITKGrowCutSegmentationImageFilter : public vtkImageAlgorithm
{
public:

  static vtkITKGrowCutSegmentationImageFilter *New();
  vtkTypeMacro(vtkITKGrowCutSegmentationImageFilter,vtkImageAlgorithm );
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Methods to set/get objectSize
  vtkSetMacro(ObjectSize, double);
  vtkGetMacro(ObjectSize, double);

  /// Methods to set/get contrastNoiseRatio
  vtkSetMacro(ContrastNoiseRatio, double);
  vtkGetMacro(ContrastNoiseRatio, double);

  /// Methods to set/get priorSegmentConfidence
  vtkSetMacro(PriorSegmentConfidence, double);
  vtkGetMacro(PriorSegmentConfidence, double);

public:
  double ObjectSize;
  double PriorSegmentConfidence;
  double ContrastNoiseRatio;


protected:
  vtkITKGrowCutSegmentationImageFilter();
  ~vtkITKGrowCutSegmentationImageFilter() override = default;

  void ExecuteDataWithInformation(vtkDataObject *outData, vtkInformation *outInfo) override;
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

private:
  vtkITKGrowCutSegmentationImageFilter(const vtkITKGrowCutSegmentationImageFilter&) = delete;
  void operator=(const vtkITKGrowCutSegmentationImageFilter&) = delete;

};

#endif
