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


typedef itk::Versor<double> QuaternionType;
typedef itk::Vector<double> TranslationType;

//class VTK_PatientToImageRegistration_EXPORT PivotCalibration

class PivotCalibration
{
public:
    PivotCalibration();
    ~PivotCalibration();

    unsigned int GetNumberOfSamples() const;

    /// Adds a sample
    /// @param quat orientation (as a quaternion x,y,z,w)
    /// @param trans position
    void AddSample(double *quat, double *trans);
    /// Calculates the pivot calibration
    void CalculateCalibration();

    /// Gets the position of the pivot point
    void GetPivotPosition(double pos[3]);
    /// Gets the translation from the tracked marker to the pivot point
    void GetTranslation(double trans[3]);
    /// Gets the root mean square error (RMSE)
    double GetRMSE();

    void Clear();

private:

    double pivotPosition[3];
    double translation[3];
    double RMSE;
    bool validPivotCalibration;

    std::vector<QuaternionType> quaternionSampleCollection;
    std::vector<TranslationType> translationSampleCollection;
};

#endif
