 /*=auto=========================================================================

  Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: PivotCalibration.h,v $
  Date:      $Date: 2008/02/15 23:15:19 $
  Version:   $Revision: 1.1.2.3 $

=========================================================================auto=*/

/**
 *  class PivotCalibration from Slicer Image-Guided Navigator (The SIGN) 
 *
 *  Pivot calibration for tracked tools.
 *
 *  This class performs a pivot calibration and computes the transform from the tracked marker to the pivot point
 *  of the surgical tool using a set of position/orientation samples. The pivot position, the transform, and the
 *  root mean square error can be obtained.
 *
 *  Author Arne Hans
 **/

#ifndef __PivotCalibration_h
#define __PivotCalibration_h

#include <vector>
#include "itkVersorRigid3DTransform.h"
#include "vtkPatientToImageRegistrationWin32Header.h"
#include "vtkMatrix4x4.h"


class VTK_PatientToImageRegistration_EXPORT PivotCalibration
{
public:
    PivotCalibration();
    ~PivotCalibration();

    unsigned int GetNumberOfSamples() const;

    /// Adds a sample
    /// @param quat orientation (as a quaternion x,y,z,w)
    /// @param trans position
    void AddSample(vtkMatrix4x4 *);
    /// Calculates the pivot calibration
    void ComputeCalibration();

    /// Gets the position of the pivot point
    void GetPivotPosition(double pos[3]);
    /// Gets the translation from the tracked marker to the pivot point
    void GetTranslation(double trans[3]);
    /// Gets the root mean square error (RMSE)
    double GetRMSE();

    void Clear();

private:

  double PivotPosition[3];
  double Translation[3];
  double RMSE;
  bool ValidPivotCalibration;

   // readings from tracking device
   std::vector<vtkMatrix4x4 *> Transforms;        

};

#endif
