/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkFSSurfaceHelper_h
#define __vtkFSSurfaceHelper_h

#include "vtkFreeSurferExport.h"

// VTK includes
#include <vtkObject.h>

class vtkMatrix4x4;

/// \brief Provides tools.
///
/// Utility functions associated to FreeSurfer data.
class VTK_FreeSurfer_EXPORT vtkFSSurfaceHelper: public vtkObject
{
public:
  static vtkFSSurfaceHelper* New();

  /// Convenience method to compute a volume's Vox2RAS-tkreg Matrix
  static void ComputeTkRegVox2RASMatrix(double* spacing, int* dimensions, vtkMatrix4x4 *M );

  /// Computes matrix we need to register V1Node to V2Node given the
  /// "register.dat" matrix from tkregister2 (FreeSurfer)
  static void TranslateFreeSurferRegistrationMatrixIntoSlicerRASToRASMatrix(
    double* V1Spacing, int* V1Dim, vtkMatrix4x4* V1IJKToRASMatrix,
    double* V2Spacing, int* V2Dim, vtkMatrix4x4* V2RASToIJKMatrix,
    vtkMatrix4x4 *FSRegistrationMatrix, vtkMatrix4x4 *RAS2RASMatrix);

protected:
  vtkFSSurfaceHelper();

private:
  vtkFSSurfaceHelper(const vtkFSSurfaceHelper&) = delete;
  void operator=(const vtkFSSurfaceHelper&) = delete;
};

#endif
