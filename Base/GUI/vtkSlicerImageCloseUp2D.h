#ifndef __vtkSlicerImageCloseUp2D_h
#define __vtkSlicerImageCloseUp2D_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkImageToImageFilter.h"

class vtkImageData;
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerImageCloseUp2D : public vtkImageToImageFilter
{
public:
  static vtkSlicerImageCloseUp2D *New();
  vtkTypeMacro(vtkSlicerImageCloseUp2D,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the Center of the window (X,Y) that we zoom in on.
  // Set the Magnification
  // Set the half-width of the region to zoom in on (HalfWidth)
  // The half-length is set to the same value.
  vtkSetMacro(X, int);
  vtkSetMacro(Y, int);
  vtkSetMacro(Z, int);
  vtkSetMacro(HalfWidth, int);
  vtkSetMacro (HalfHeight, int);
  vtkSetMacro(Magnification, int);

  int Magnification;
  int HalfWidth;
  int HalfHeight;
  int X;
  int Y;
  int Z;

protected:
  vtkSlicerImageCloseUp2D();
  ~vtkSlicerImageCloseUp2D() {};

  void ExecuteInformation(vtkImageData *inData, 
                          vtkImageData *outData);
  void ExecuteInformation(){this->Superclass::ExecuteInformation();};

  // Override this function since inExt != outExt
  void ComputeInputUpdateExtent(int inExt[6], int outExt[6]);
  
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
                       int extent[6], int id);
private:
  vtkSlicerImageCloseUp2D(const vtkSlicerImageCloseUp2D&);
  void operator=(const vtkSlicerImageCloseUp2D&);
};

#endif



