/*=auto=========================================================================

  Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/
// .NAME vtkProstateNavTargetDescriptor - describes the target, along with targeting paramteres and validation info
// .SECTION Description
// vtkProstateNavTargetDescriptor describes the target, along with targeting paramteres and validation info

#ifndef _vtkProstateNavTargetDescriptor_h
#define _vtkProstateNavTargetDescriptor_h
#pragma once

#include <stdio.h>
#include <string>

#include "vtkObject.h"
#include "vtkProstateNavWin32Header.h"



class VTK_PROSTATENAV_EXPORT vtkProstateNavTargetDescriptor :
  public vtkObject
{
public:

  static vtkProstateNavTargetDescriptor *New();
  vtkTypeRevisionMacro(vtkProstateNavTargetDescriptor,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

    //BTX
    // Description
    // Set/get validation info: Validation volume FoR string
    void SetName(std::string str){this->Name = str;}
    std::string GetName() const {return this->Name;};
    //ETX


    // Desciption
    // Save/load from ini file
    //virtual bool Save(wxFileConfig *ini, const char *iniCategory);
    //virtual bool Load(wxFileConfig *ini, const char *iniCategory);

    // Description
    // Set/get Targeting parameter: rotation
    vtkGetMacro(AxisRotation,double);
    vtkSetMacro(AxisRotation,double);

    // Description
    // Set/get Targeting parameter: needle angle
    vtkGetMacro(NeedleAngle,double);
    vtkSetMacro(NeedleAngle,double);

    // Description
    // Set/get Targeting parameter: depth in centimeters
    vtkGetMacro(DepthCM,double);
    vtkSetMacro(DepthCM,double);

    // Description
    // Set/get Needle overshoot
    vtkGetMacro(NeedleOvershoot,double);
    vtkSetMacro(NeedleOvershoot,double);


    // Description
    // Set/get Targeting parameter: is reachable or not 
    vtkGetMacro(IsOutsideReach,bool);
    vtkSetMacro(IsOutsideReach,bool);

    //BTX
    std::string GetReachableString() const
        { 
            if (this->IsOutsideReach) 
                return "No";
            else
                return "Yes";
        }
    //ETX

    //BTX
    // Description
    // Set/get Targeting parameter: needle type
    void SetNeedleType(std::string NeedleType, double Depth, double Overshoot) 
    { 
        this->NeedleType = NeedleType;
        this->NeedleLength = Depth;
        this->NeedleOvershoot = Overshoot;
    }
    std::string GetNeedleTypeString() const { return this->NeedleType;};
    //ETX
    // Description
    // Set/get Targeting parameter: needle type depth
    vtkGetMacro(NeedleLength,double);
    vtkSetMacro(NeedleLength,double);

    // Description
    // Set/get Targeting parameter: hinge position (in RAS coordinate system)
    vtkSetVector3Macro(HingePosition,double);
    vtkGetVectorMacro(HingePosition,double,3);

    // Description
    // Set/get : ras location
    vtkSetVector3Macro(RASLocation,double);
    vtkGetVectorMacro(RASLocation,double,3);

    // Description
    // Set/get : ras location
    vtkSetVector4Macro(RASOrientation,double);
    vtkGetVectorMacro(RASOrientation,double,4);

    //BTX
    std::string GetRASLocationString() const
    {
        char ras[50];
        sprintf(ras, "R %.1f, A %.1f, S %.1f", this->RASLocation[0], this->RASLocation[1], this->RASLocation[2]);
        return std::string(ras);
    }
    //ETX
    
    // Description
    // Set/get targeting info is valid (they are computed using a valid calibration data)
    vtkGetMacro(TargetingParametersValid,bool);
    vtkSetMacro(TargetingParametersValid,bool);

    // Description
    // Set/get Needle placement is validated
    vtkGetMacro(TargetValidated,bool);
    vtkSetMacro(TargetValidated,bool);

    // Description
    // Set/get validation info: needle tip point
    vtkSetVector3Macro(NeedleTipValidationPosition,double);
    vtkGetVectorMacro(NeedleTipValidationPosition,double,3);

    // Description
    // Set/get validation info: needle base point (any point along the needle)
    vtkSetVector3Macro(NeedleBaseValidationPosition,double);
    vtkGetVectorMacro(NeedleBaseValidationPosition,double,3);

    // Description
    // Set/get validation info: computed distance from the needle
    vtkGetMacro(OverallError,double);
    vtkSetMacro(OverallError,double);

    // Description
    // Set/get validation info: computed distance from the needle
    vtkGetMacro(APError,double);
    vtkSetMacro(APError,double);

    // Description
    // Set/get validation info: computed distance from the needle
    vtkGetMacro(LRError,double);
    vtkSetMacro(LRError,double);

    // Description
    // Set/get validation info: computed distance from the needle
    vtkGetMacro(ISError,double);
    vtkSetMacro(ISError,double);

    //BTX
    // Description
    // Set/get validation info: Validation volume FoR string
    void SetNeedleConfirmationVolumeFoRStr(std::string foRStr){this->ValidationVolumeFoR_STR = foRStr;}
    std::string GetNeedleConfirmationVolumeFoRStr() const {return this->ValidationVolumeFoR_STR;};

    // Description
    // Set/get FoR string of the targeting volume
    void SetTargetingFoRStr(std::string foRStr){this->TargetingFoR_STR = foRStr;}
    std::string GetTargetingFoRStr() const {return this->TargetingFoR_STR;};

    // Description
    // Set/get FoR string of the calibration volume
    void SetCalibrationFoRStr(std::string foRStr){this->CalibrationFoR_STR = foRStr;}
    std::string GetCalibrationFoRStr() const {return this->CalibrationFoR_STR;};

    void SetFiducialID(std::string fidId){this->FiducialID = fidId;}
    std::string GetFiducialID() const {return this->FiducialID; };

    void DeepCopy(vtkProstateNavTargetDescriptor& src);

    //ETX
        
private:
    //BTX
    std::string Name;
    //ETX
    double AxisRotation;    ///< Calculated value: Axis rotation in degree
    double NeedleAngle;     ///< Calculated value: Needle angle in degree
    double DepthCM;         ///< Calculated value: Insertion deepth in cm
    bool IsOutsideReach;    ///< Calculated value: Can be reaced?
    double NeedleLength; ///< Depth depending on needle type (biopsy, placement...)
    double NeedleOvershoot; ///< Overshooting
    double HingePosition[3];
    double RASLocation[3];
    double RASOrientation[4];
    //BTX
    std::string TargetingFoR_STR;
    std::string CalibrationFoR_STR; /// string identifier for the calibration volume
    std::string ValidationVolumeFoR_STR; /// string identifier for the volume used for needle confirmation
    std::string NeedleType;
    std::string FiducialID; // ID of the fiducial in the FiducialListNode
    //ETX
    bool TargetValidated;  ///< Indicates whether it was validated against needle insertion
    bool TargetingParametersValid;
    double NeedleTipValidationPosition[3];
    double NeedleBaseValidationPosition[3];
    double OverallError; /// < Calculated value, ComputedDistanceFromNeedle
    double APError;/// < Calculated value
    double LRError;/// < Calculated value
    double ISError;/// < Calculated value
    
    vtkProstateNavTargetDescriptor(void);
    ~vtkProstateNavTargetDescriptor(void);

    /*vtkProstateNavTargetDescriptor(const vtkProstateNavTargetDescriptor&);
    void operator=(const vtkProstateNavTargetDescriptor&);*/
};

#endif
