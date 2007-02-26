#ifndef __vtkSlicerImageCrossHair2D_h
#define __vtkSlicerImageCrossHair2D_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkImageInPlaceFilter.h"

class vtkImageData;
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerImageCrossHair2D : public vtkImageInPlaceFilter
{
public:
  static vtkSlicerImageCrossHair2D *New();
  vtkTypeMacro(vtkSlicerImageCrossHair2D,vtkImageInPlaceFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the RGB CursorColor
  vtkSetVector3Macro(CursorColor, vtkFloatingPointType);
  vtkGetVectorMacro(CursorColor, vtkFloatingPointType, 3);

  // Description:
  // Get/Set the Number of Hash marks on the Cross Hair
  vtkGetMacro(NumHashes, int);
  vtkSetMacro(NumHashes, int);

  // Description:
  // Get/Set the BullsEyeWidth in pixels.
  vtkGetMacro(BullsEyeWidth, int);
  vtkSetMacro(BullsEyeWidth, int);

  // Description:
  // Turn the BullsEye on and off
  vtkGetMacro(BullsEye, int);
  vtkSetMacro(BullsEye, int);
  vtkBooleanMacro(BullsEye, int);

  // Description:
  // Get/Set the Spacing between Hash Marks in mm.
  vtkGetMacro(HashGap, vtkFloatingPointType);
  vtkSetMacro(HashGap, vtkFloatingPointType);

  // Description:
  // Get/Set the Length of a hash mark in mm.
  vtkGetMacro(HashLength, vtkFloatingPointType);
  vtkSetMacro(HashLength, vtkFloatingPointType);

  // Description:
  // Get/Set the Magnification
  // NOTE: This should not be used.  Instead, specify the magnification
  // implicitly in the spacing.
  vtkGetMacro(Magnification, vtkFloatingPointType);
  vtkSetMacro(Magnification, vtkFloatingPointType);

  // Description:
  // Set whether or not the cursor should be shown
  // If not, this filter does nothing.
  vtkGetMacro(ShowCursor, int);
  vtkSetMacro(ShowCursor, int);
  vtkBooleanMacro(ShowCursor, int);

  // Description 
  // Set the cross to intersect or not. 
  // If not, the result is perpendicular lines
  // with their intersection removed.
  vtkGetMacro(IntersectCross, int);
  vtkSetMacro(IntersectCross, int);
  vtkBooleanMacro(IntersectCross, int);

  // Description 
  // Get/Set The Cursor Position.
  vtkSetVector2Macro(Cursor, int);
  vtkGetVectorMacro(Cursor, int, 2);

protected:
  vtkSlicerImageCrossHair2D();
  ~vtkSlicerImageCrossHair2D() {};

  int ShowCursor;
  int NumHashes;
  int IntersectCross;
  int Cursor[2];
  vtkFloatingPointType CursorColor[3];
  vtkFloatingPointType Magnification;
  vtkFloatingPointType HashGap;
  vtkFloatingPointType HashLength;
  int BullsEye;
  int BullsEyeWidth;

  virtual int RequestData(vtkInformation *request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);
  
  void DrawCursor(vtkImageData *outData, int outExt[6]);

private:
  vtkSlicerImageCrossHair2D(const vtkSlicerImageCrossHair2D&);
  void operator=(const vtkSlicerImageCrossHair2D&);
};

#endif

