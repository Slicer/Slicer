
#ifndef __vtkITKGrowCutSegmentationImageFilter_h
#define __vtkITKGrowCutSegmentationImageFilter_h

#include "vtkITK.h"

// VTK includes
#include <vtkImageMultipleInputFilter.h>

class vtkImageData;

/// \brief- Wrapper class around itk::GrowCutSegmentationImageFilter
///
/// GrowCutSegmentationImageFilter produces segmentation of regions on an image interactively. 
/// It uses the postive and  negative gestures (strokes/ lines/points marked by the user on the region of interest (positive), 
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
class VTK_ITK_EXPORT vtkITKGrowCutSegmentationImageFilter : public vtkImageMultipleInputFilter 
{
public:

  static vtkITKGrowCutSegmentationImageFilter *New();
  vtkTypeRevisionMacro(vtkITKGrowCutSegmentationImageFilter,vtkImageMultipleInputFilter );
  void PrintSelf(ostream& os, vtkIndent indent);

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
  ~vtkITKGrowCutSegmentationImageFilter(){}

  virtual void ExecuteData(vtkDataObject *outData);

  /// Override ExecuteInformation so that the second input is used to
  /// define the output information (input gestures and output
  /// segmentation images should be same image type)
  virtual void ExecuteInformation(vtkImageData **, vtkImageData *);

  /// Need to provide ExecuteInformation() or it will be hidden by the
  /// override to ExecuteInformation(vtkImageData**, vtkImageData**)
  virtual void ExecuteInformation();

private:
  vtkITKGrowCutSegmentationImageFilter(const vtkITKGrowCutSegmentationImageFilter&);  // Not implemented.
  void operator=(const vtkITKGrowCutSegmentationImageFilter&);  // Not implemented.

};

#endif
