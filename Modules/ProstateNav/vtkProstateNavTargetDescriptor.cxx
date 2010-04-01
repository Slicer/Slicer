#include "vtkProstateNavTargetDescriptor.h"
#include "vtkObjectFactory.h"
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavTargetDescriptor);
vtkCxxRevisionMacro(vtkProstateNavTargetDescriptor, "$Revision: 1.0 $");
//----------------------------------------------------------------------------
/// Constructor
vtkProstateNavTargetDescriptor::vtkProstateNavTargetDescriptor()
{
  this->Name="";  
  this->AxisRotation=0;
  this->NeedleAngle=0;
  this->DepthCM=0;
  this->NeedleLength=0.0;
  this->NeedleType = "";
  this->FiducialID = "";
  this->NeedleOvershoot=0.0;
  this->TargetingParametersValid = false;
  this->TargetValidated = false;
  this->NeedleTipValidationPosition[0]=0;
  this->NeedleTipValidationPosition[1]=0;
  this->NeedleTipValidationPosition[2]=0;
  this->NeedleBaseValidationPosition[0]=0;
  this->NeedleBaseValidationPosition[1]=0;
  this->NeedleBaseValidationPosition[2]=0;
  this->CalibrationFoR_STR.clear();
  this->TargetingFoR_STR.clear();
  this->ValidationVolumeFoR_STR.clear();
  this->OverallError = 0.0;
  this->APError = 0.0;
  this->LRError = 0.0;
  this->ISError = 0.0;
  this->IsOutsideReach=true;
  this->RASLocation[0] = 0;
  this->RASLocation[1] = 0;
  this->RASLocation[2] = 0;
  this->RASOrientation[0] = 0;
  this->RASOrientation[1] = 0;
  this->RASOrientation[2] = 0;
  this->RASOrientation[3] = 0;
  this->HingePosition[0] = 0;
  this->HingePosition[1] = 0;
  this->HingePosition[2] = 0;
}


/// Destructor
vtkProstateNavTargetDescriptor::~vtkProstateNavTargetDescriptor(void)
{
}

//----------------------------------------------------------------------------
void vtkProstateNavTargetDescriptor::DeepCopy(vtkProstateNavTargetDescriptor& src)
{
  this->Name=src.Name;
  this->AxisRotation=src.AxisRotation;
  this->NeedleAngle=src.NeedleAngle;
  this->DepthCM=src.DepthCM;
  this->NeedleLength=src.NeedleLength;
  this->NeedleType =src.NeedleType;
  this->FiducialID = src.FiducialID;
  this->NeedleOvershoot=src.NeedleOvershoot;
  this->TargetValidated =src.TargetValidated;
  this->TargetingParametersValid = src.TargetingParametersValid;
  this->NeedleTipValidationPosition[0]=src.NeedleTipValidationPosition[0];
  this->NeedleTipValidationPosition[1]=src.NeedleTipValidationPosition[1];
  this->NeedleTipValidationPosition[2]=src.NeedleTipValidationPosition[2];
  this->NeedleBaseValidationPosition[0]=src.NeedleBaseValidationPosition[0];
  this->NeedleBaseValidationPosition[1]=src.NeedleBaseValidationPosition[1];
  this->NeedleBaseValidationPosition[2]=src.NeedleBaseValidationPosition[2];
  this->CalibrationFoR_STR =src.CalibrationFoR_STR;
  this->TargetingFoR_STR =src.TargetingFoR_STR;
  this->ValidationVolumeFoR_STR=src.ValidationVolumeFoR_STR;
  this->OverallError=src.OverallError;
  this->APError=src.APError;
  this->LRError=src.LRError;
  this->ISError=src.ISError;
  this->IsOutsideReach=src.IsOutsideReach;
  this->RASLocation[0]=src.RASLocation[0];
  this->RASLocation[1] =src.RASLocation[1];
  this->RASLocation[2] =src.RASLocation[2];
  this->RASOrientation[0] = src.RASOrientation[0];
  this->RASOrientation[1] = src.RASOrientation[1];
  this->RASOrientation[2] = src.RASOrientation[2];
  this->RASOrientation[3] = src.RASOrientation[3];
  this->HingePosition[0] =src.HingePosition[0];
  this->HingePosition[1] =src.HingePosition[1];
  this->HingePosition[2] =src.HingePosition[2];
}

//----------------------------------------------------------------------------
void vtkProstateNavTargetDescriptor::PrintSelf(ostream& os, vtkIndent indent)
{
  
  this->Superclass::PrintSelf(os,indent);

  /*
  os << indent << "PlanningVolumeRef:   " << 
   (this->PlanningVolumeRef ? this->PlanningVolumeRef : "(none)") << "\n";
 */
}

/*
/// Serialize to file
bool vtkProstateNavTargetDescriptor::Save(wxFileConfig *ini, const char *iniCategory)
{
    //cPrimitivePoint::Save(ini,iniCategory);

    wxString iniEntry;

    iniEntry=iniCategory;iniEntry+="NeedleType";
    ini->Write(iniEntry,this->NeedleType);

    iniEntry=iniCategory;iniEntry+="Info_NeedleDeepth";
    ini->Write(iniEntry,this->NeedleDeepth);

    iniEntry=iniCategory;iniEntry+="Info_AxisRotationDeg";
    ini->Write(iniEntry,this->AxisRotation);

    iniEntry=iniCategory;iniEntry+="Info_NeedleAngleDeg";
    ini->Write(iniEntry,this->NeedleAngle);

    iniEntry=iniCategory;iniEntry+="Info_DepthCM";
    ini->Write(iniEntry,this->DepthCM);

    iniEntry=iniCategory;iniEntry+="Info_Reacheable";
    if (this->IsOutsideReach) {
        ini->Write(iniEntry,"false");
    } else {
        ini->Write(iniEntry,"true");
    }
  
    iniEntry=iniCategory;iniEntry+="Target_Validated";
    if (this->TargetValidated) {
        ini->Write(iniEntry,"true");
    } else {
        ini->Write(iniEntry,"false");
    }

    iniEntry=iniCategory;iniEntry+="NeedleConfirmationVolume_SeriesIdentifier";
    ini->Write(iniEntry,this->ValidationVolumeFoR_STR);

    iniEntry=iniCategory;iniEntry+="Distance_From_Needle";
    ini->Write(iniEntry,this->ComputedDistanceFromNeedle);



    return true;
}


/// Restore from file
bool vtkProstateNavTargetDescriptor::Load(wxFileConfig *ini, const char *iniCategory)
{
    cPrimitivePoint::Load(ini,iniCategory);

    wxString iniEntry;

    iniEntry=iniCategory;iniEntry+="NeedleType";
    ini->Read(iniEntry,&this->NeedleType);

    iniEntry=iniCategory;iniEntry+="Info_NeedleDeepth";
    ini->Read(iniEntry,&this->NeedleDeepth);

    iniEntry=iniCategory;iniEntry+="Info_AxisRotationDeg";
    ini->Read(iniEntry,&this->AxisRotation);

    iniEntry=iniCategory;iniEntry+="Info_NeedleAngleDeg";
    ini->Read(iniEntry,&this->NeedleAngle);

    iniEntry=iniCategory;iniEntry+="Info_DepthCM";
    ini->Read(iniEntry,&this->DepthCM);

    iniEntry=iniCategory;iniEntry+="Info_Reacheable";
    wxString sReacheable;
    ini->Read(iniEntry,&sReacheable,"false");
    this->IsOutsideReach=true;
    if (sReacheable=="true") this->IsOutsideReach=false;

    iniEntry=iniCategory;iniEntry+="Target_Validated";
    wxString sTargetValidated;
    ini->Read(iniEntry, &sTargetValidated); 
    if (sTargetValidated == "true")
      {
      this->TargetValidated=true;
      }
    else
      {
      this->TargetValidated = false;
      }
    
    iniEntry=iniCategory;iniEntry+="NeedleConfirmationVolume_SeriesIdentifier"; 
    wxString str;
    ini->Read(iniEntry,&str);
    this->ValidationVolumeFoR_STR = std::string(str);

    iniEntry=iniCategory;iniEntry+="Distance_From_Needle";
    ini->Read(iniEntry,&this->ComputedDistanceFromNeedle);


    return true;
}
*/
