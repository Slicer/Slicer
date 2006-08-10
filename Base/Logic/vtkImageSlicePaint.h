/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageSlicePaint.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageSlicePaint - Extract or Replace an arbitrary bilinear
// subvolume of an image
// .SECTION Description
// vtkImageSlicePaint take the given TopLeft, TopRight, and BottomRight, 
// and BottomLeft points to form a bilinear surface within the volume.
// If the ExtractImage is set, then it will be filled with copies of all
// pixels from this surface.
// If the ReplaceImage is set, then it will be used to replace the pixels
// of this surface.
// The expected use is that the filter will be run once to get the ExtractImage
// which will then be processed (e.g. a paint brush applied) and then 
// the image will be put back with the ReplaceImage.  The unmodified version
// of the ExtractImage can be saved together with the original coordinates
// in order to implement undo.
//

#ifndef __vtkImageSlicePaint_h
#define __vtkImageSlicePaint_h

#include "vtkSlicerBaseLogic.h"

#include "vtkImageInPlaceFilter.h"

#include "vtkImageData.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkImageSlicePaint : public vtkObject
{
public:
  static vtkImageSlicePaint *New();
  vtkTypeRevisionMacro(vtkImageSlicePaint,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);


  // Description:
  // Sets/Gets the PaintRegion in IJK (pixel) coordinates
  vtkSetVector3Macro(TopLeft, int);
  vtkGetVector3Macro(TopLeft, int);
  vtkSetVector3Macro(TopRight, int);
  vtkGetVector3Macro(TopRight, int);
  vtkSetVector3Macro(BottomLeft, int);
  vtkGetVector3Macro(BottomLeft, int);
  vtkSetVector3Macro(BottomRight, int);
  vtkGetVector3Macro(BottomRight, int);

  // Description:
  // The place to store data pulled out
  vtkSetObjectMacro(WorkingImage, vtkImageData);
  vtkGetObjectMacro(WorkingImage, vtkImageData);

  // Description:
  // The place to store data pulled out
  vtkSetObjectMacro(ExtractImage, vtkImageData);
  vtkGetObjectMacro(ExtractImage, vtkImageData);

  // Description:
  // The place to get data to be replaced
  vtkSetObjectMacro(ReplaceImage, vtkImageData);
  vtkGetObjectMacro(ReplaceImage, vtkImageData);
  
  void Extract();
  void Replace();

protected:
  vtkImageSlicePaint();
  ~vtkImageSlicePaint() {};

  int TopLeft[3];
  int TopRight[3];
  int BottomLeft[3];
  int BottomRight[3];

  vtkImageData *WorkingImage;
  vtkImageData *ExtractImage;
  vtkImageData *ReplaceImage;

private:
  vtkImageSlicePaint(const vtkImageSlicePaint&);  // Not implemented.
  void operator=(const vtkImageSlicePaint&);  // Not implemented.
};



#endif



