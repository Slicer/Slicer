/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageLabelOutline.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

#ifndef __vtkImageLabelOutline_h
#define __vtkImageLabelOutline_h

#include "vtkImageNeighborhoodFilter.h"

#include "vtkMRMLLogicExport.h"

class vtkImageData;

/// \brief Display labelmap outlines.
///
/// Used  in slicer for the Label layer to outline the segmented
/// structures (instead of showing them filled-in).
class VTK_MRML_LOGIC_EXPORT vtkImageLabelOutline : public vtkImageNeighborhoodFilter
{
public:
  static vtkImageLabelOutline *New();
  vtkTypeMacro(vtkImageLabelOutline,vtkImageNeighborhoodFilter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// background pixel value in the image (usually 0)
  vtkSetMacro(Background, float);
  vtkGetMacro(Background, float);

  ///
  /// Thickness of the outline, used to set the kernel size
  void SetOutline(int outline);
  vtkGetMacro(Outline, int);

protected:
  vtkImageLabelOutline();
  ~vtkImageLabelOutline() override;

  float Background;
  int Outline;

  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
                       int extent[6], int id) override;

private:
  vtkImageLabelOutline(const vtkImageLabelOutline&) = delete;
  void operator=(const vtkImageLabelOutline&) = delete;
};

#endif
