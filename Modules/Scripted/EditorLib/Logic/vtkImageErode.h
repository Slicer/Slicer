/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
///  vtkImageErode -  Performs erosion
///
/// Erodes pixels of specified Foreground value by setting them
/// to the Background value. Variable 3D connectivity (4- or 8-neighbor).

#ifndef __vtkImageErode_h
#define __vtkImageErode_h

#include "vtkSlicerEditorLibModuleLogicExport.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkImageNeighborhoodFilter.h>

class VTK_SLICER_EDITORLIB_MODULE_LOGIC_EXPORT vtkImageErode : public vtkImageNeighborhoodFilter
{
public:
  static vtkImageErode *New();
  vtkTypeMacro(vtkImageErode,vtkImageNeighborhoodFilter);

  ///
  /// Background and foreground pixel values in the image.
  /// Usually 0 and some label value, respectively.
  vtkSetMacro(Background, float);
  vtkGetMacro(Background, float);
  vtkSetMacro(Foreground, float);
  vtkGetMacro(Foreground, float);

protected:
  vtkImageErode();
  ~vtkImageErode() override;

  float Background;
  float Foreground;

  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
    int extent[6], int id) override;

private:
  vtkImageErode(const vtkImageErode&) = delete;
  void operator=(const vtkImageErode&) = delete;
};

#endif

