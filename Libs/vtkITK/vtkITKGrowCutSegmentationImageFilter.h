// .NAME vtkITKGrowCutSegmentationImageFilter - Wrapper class around itk::GrowCutSegmentationImageFilter
// .SECTION Description
// itk::GrowCutSegmentationImageFilter
// GrowCutSegmentationImageFilter produces segmentation of regions on an image interactively. 
// It uses the postive and  negative gestures (strokes/ lines/points marked by the user on the region of interest (positive), 
// and outside the object (negative) to automatically generate the segmentation

// .SECTION Caveats
// This filter is implemented only for scalar images gray scale images. 
// The current implementation supports n-class segmentation

#ifndef __vtkITKGrowCutSegmentationImageFilter_h
#define __vtkITKGrowCutSegmentationImageFilter_h

#include "vtkITK.h" 
//#include "vtkImageTwoInputFilter.h"
#include "vtkImageMultipleInputFilter.h"
#include "vtkPoints.h"
#include "vtkImageData.h"


// Usage: SetInput1 is the input feature/intensity image (required)
// SetInput2 takes the gesture image (the gestures) image (required)
// SetInput3 takes the previous segmented image (optional)
// GetOutput produces the output segmented image

//class VTK_EXPORT vtkITKGrowCutSegmentationImageFilter : public vtkImageTwoInputFilter

class VTK_ITK_EXPORT vtkITKGrowCutSegmentationImageFilter : public vtkImageMultipleInputFilter 
{
public:

  static vtkITKGrowCutSegmentationImageFilter *New();
  vtkTypeRevisionMacro(vtkITKGrowCutSegmentationImageFilter,vtkImageMultipleInputFilter );
  void PrintSelf(ostream& os, vtkIndent indent);

  // Methods to set/get maxIterations
  //  vtkSetMacro(MaxIterations, double);
  //vtkGetMacro(MaxIterations, double);

  // Methods to set/get objectSize
  vtkSetMacro(ObjectSize, double);
  vtkGetMacro(ObjectSize, double);

  // Methods to set/get contrastNoiseRatio
  vtkSetMacro(ContrastNoiseRatio, double);
  vtkGetMacro(ContrastNoiseRatio, double);    


  // Methods to set/get contrastNoiseRatioThreshold 
  // cnrThreshold
  //vtkSetMacro(CnrThreshold, double);
  //vtkGetMacro(CnrThreshold, double);

  // Methods to set/get priorSegmentConfidence
  vtkSetMacro(PriorSegmentConfidence, double);
  vtkGetMacro(PriorSegmentConfidence, double);

  // Methods to set the gestureColors
  void SetGestureColors( vtkPoints *points )
 {
    GestureColors = points;
    this->Modified();
 }
 
  // Method for getting the gesture colors
  vtkPoints* GetGestureColors (){
    return GestureColors;
  }

  

public:
  // member variables
  //  double MaxIterations;
  double ObjectSize;
  double PriorSegmentConfidence;
  double ContrastNoiseRatio;
  vtkPoints *GestureColors;

  // optional : contrastNoiseRatio Threshold. Use this for producing 
  // output with values over the threshold in the final segmentation
  // not implemented yet 
  //double CnrThreshold; 


protected:
  vtkITKGrowCutSegmentationImageFilter();
  ~vtkITKGrowCutSegmentationImageFilter() {};

  virtual void ExecuteData(vtkDataObject *outData);
  
   
private:
  vtkITKGrowCutSegmentationImageFilter(const vtkITKGrowCutSegmentationImageFilter&);  // Not implemented.
  void operator=(const vtkITKGrowCutSegmentationImageFilter&);  // Not implemented.
  

};

#endif
