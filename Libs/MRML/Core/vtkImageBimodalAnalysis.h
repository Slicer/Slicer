/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageBimodalAnalysis.h,v $
  Date:      $Date: 2006/06/14 20:44:13 $
  Version:   $Revision: 1.23 $

=========================================================================auto=*/

#ifndef __vtkImageBimodalAnalysis_h
#define __vtkImageBimodalAnalysis_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkImageAlgorithm.h>

#define VTK_BIMODAL_MODALITY_CT 0
#define VTK_BIMODAL_MODALITY_MR 1

/// \brief Analysis bimodal histograms
///
/// This filter assumes the input comes
/// from vtkImageAccumulate, so there.
/// \warning
/// FIXME: only works on output floating point
/// FIXME: should use vtkTemplateMacro
class VTK_MRML_EXPORT vtkImageBimodalAnalysis : public vtkImageAlgorithm
{
public:
  static vtkImageBimodalAnalysis *New();
  vtkTypeMacro(vtkImageBimodalAnalysis,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// Set the type of data, if known
  vtkSetMacro(Modality, int);
  vtkGetMacro(Modality, int);
  void SetModalityToMR() {this->SetModality(VTK_BIMODAL_MODALITY_MR);};
  void SetModalityToCT() {this->SetModality(VTK_BIMODAL_MODALITY_CT);};

  ///
  /// Get stats
  vtkGetMacro(Threshold, int);
  vtkGetMacro(Window, int);
  vtkGetMacro(Level, int);
  vtkGetMacro(Min, int);
  vtkGetMacro(Max, int);
  vtkGetVectorMacro(SignalRange, int, 2);
  vtkGetVectorMacro(ClipExtent, int, 6);

  vtkGetMacro(Offset, int);
  vtkSetMacro(Offset, int);

  ///
  /// Ideally this should not be public API
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

  virtual int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  void ExecuteDataWithInformation(vtkDataObject *, vtkInformation*);

private:
  vtkImageBimodalAnalysis(const vtkImageBimodalAnalysis&);
  void operator=(const vtkImageBimodalAnalysis&);
};

#endif

