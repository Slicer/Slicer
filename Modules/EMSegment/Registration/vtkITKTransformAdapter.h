#ifndef __vtkITKTransformAdapter_h
#define __vtkITKTransformAdapter_h

#include "vtkObject.h"
#include "vtkEMSegmentWin32Header.h"
#include "vtkWarpTransform.h"
#include "itkTransform.h"

//
// Helper class for converting from itk transforms to vtk transforms.
// This class is not comprehensive or ready for general use.
class VTK_EMSEGMENT_EXPORT vtkITKTransformAdapter : 
  public vtkWarpTransform
{
public:
  typedef itk::Transform<double, 3, 3> TransformType;
  
  static vtkITKTransformAdapter *New();
  vtkTypeRevisionMacro(vtkITKTransformAdapter,
                       vtkWarpTransform);
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  
  void SetITKTransform(TransformType* transform)
  {
    this->ITKTransform = transform;
  }
  
  vtkAbstractTransform* MakeTransform() 
  { return vtkITKTransformAdapter::New(); }
  
protected:
  vtkITKTransformAdapter() { this->ITKTransform = NULL; }
  ~vtkITKTransformAdapter() {}
  
  // Description:
  // Internal functions for calculating the transformation.
  void ForwardTransformPoint(const float in[3], float out[3]) 
  {
    this->ITKTMP_point[0] = in[0];
    this->ITKTMP_point[1] = in[1];
    this->ITKTMP_point[2] = in[2];
    
    this->ITKTMP_outputPoint = 
      this->ITKTransform->TransformPoint(this->ITKTMP_point);
    
    out[0] = this->ITKTMP_outputPoint[0];
    out[1] = this->ITKTMP_outputPoint[1];
    out[2] = this->ITKTMP_outputPoint[2];
  }
  void ForwardTransformPoint(const double in[3], double out[3]) 
  {
    this->ITKTMP_point[0] = in[0];
    this->ITKTMP_point[1] = in[1];
    this->ITKTMP_point[2] = in[2];
    
    this->ITKTMP_outputPoint = 
      this->ITKTransform->TransformPoint(this->ITKTMP_point);
    
    out[0] = this->ITKTMP_outputPoint[0];
    out[1] = this->ITKTMP_outputPoint[1];
    out[2] = this->ITKTMP_outputPoint[2];
  }
  
  void ForwardTransformDerivative(const float in[3], float out[3],
                                  float derivative[3][3]) {}
  void ForwardTransformDerivative(const double in[3], double out[3],
                                  double derivative[3][3]) {}
  
private:
  vtkITKTransformAdapter
  (const vtkITKTransformAdapter&);  // Not implemented.
// Not implemented.
  void operator=(const vtkITKTransformAdapter&);  

  TransformType::InputPointType           ITKTMP_point;
  TransformType::OutputPointType          ITKTMP_outputPoint;
  
  TransformType* ITKTransform;
};

#endif // __vtkITKTransformAdapter_h
