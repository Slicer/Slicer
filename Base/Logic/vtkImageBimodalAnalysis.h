/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageBimodalAnalysis.h,v $
  Date:      $Date: 2006/06/14 20:44:13 $
  Version:   $Revision: 1.23 $

=========================================================================auto=*/
// .NAME vtkImageBimodalAnalysis - Analysis bimodal histograms
// .SECTION Description
// vtkImageBimodalAnalysis - This filter assumes the input comes
// from vtkImageAccumulateDiscrete, so there.
// .SECTION Warning
// FIXME: only works on output floating point
// FIXME: should use vtkTemplateMacro

#ifndef __vtkImageBimodalAnalysis_h
#define __vtkImageBimodalAnalysis_h

#include "vtkImageToImageFilter.h"

#include "vtkSlicerBaseLogic.h"


#define VTK_BIMODAL_MODALITY_CT 0
#define VTK_BIMODAL_MODALITY_MR 1

class VTK_SLICER_BASE_LOGIC_EXPORT vtkImageBimodalAnalysis : public vtkImageToImageFilter
{
public:
  static vtkImageBimodalAnalysis *New();
  vtkTypeMacro(vtkImageBimodalAnalysis,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the type of data, if known
  vtkSetMacro(Modality, int);
  vtkGetMacro(Modality, int);
  void SetModalityToMR() {this->SetModality(VTK_BIMODAL_MODALITY_MR);};
  void SetModalityToCT() {this->SetModality(VTK_BIMODAL_MODALITY_CT);};
 
  // Description:
  // Get stats
  vtkGetMacro(Threshold, int);
  vtkGetMacro(Window, int);
  vtkGetMacro(Level, int);
  vtkGetMacro(Min, int);
  vtkGetMacro(Max, int);
  vtkGetVectorMacro(SignalRange, int, 2);
  vtkGetVectorMacro(ClipExtent, int, 6);

    vtkGetMacro(Offset, int);
    vtkSetMacro(Offset, int);
    
  // Description:
  // Ideally this should not be public API
  vtkSetMacro(Threshold, int);
  vtkSetMacro(Window, int);
  vtkSetMacro(Level, int);
  vtkSetMacro(Min, int);
  vtkSetMacro(Max, int);
  vtkSetVector2Macro(SignalRange, int);
  vtkSetVectorMacro(ClipExtent, int, 6);

protected:
  vtkImageBimodalAnalysis();
  ~vtkImageBimodalAnalysis() {};

  int Modality;

  int Threshold;
  int Window;
  int Level;
  int Min;
  int Max;
  int ClipExtent[6];
  int SignalRange[2];

    int Offset;

  void ExecuteInformation(vtkImageData *input, vtkImageData *output);
  void ExecuteInformation(){this->Superclass::ExecuteInformation();};
  void ExecuteData(vtkDataObject *);

private:
  vtkImageBimodalAnalysis(const vtkImageBimodalAnalysis&);
  void operator=(const vtkImageBimodalAnalysis&);
};

#endif

