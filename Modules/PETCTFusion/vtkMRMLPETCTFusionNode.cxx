#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLPETCTFusionNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLPETCTFusionNode* vtkMRMLPETCTFusionNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLPETCTFusionNode");
  if(ret)
    {
      return (vtkMRMLPETCTFusionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLPETCTFusionNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLPETCTFusionNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLPETCTFusionNode");
  if(ret)
    {
      return (vtkMRMLPETCTFusionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLPETCTFusionNode;
}

//----------------------------------------------------------------------------
vtkMRMLPETCTFusionNode::vtkMRMLPETCTFusionNode()
{
   this->InputCTReference = NULL;
   this->InputPETReference = NULL;
   this->InputMask = NULL;
   this->PETLUT = NULL;
   this->CTLUT = NULL;
   this->MessageText = NULL;
   this->Layout = NULL;
   this->HideFromEditors = true;
   this->WeightUnits= NULL;
   this->VolumeUnits = NULL;
   this->DoseRadioactivityUnits = NULL;
   this->TissueRadioactivityUnits = NULL;
   this->VolumeRendering = 0;
   this->NumberOfTemporalPositions = 0;
   this->InjectedDose = 0.0;
   this->PatientName = NULL;
   this->StudyDate = NULL;
   this->PatientWeight = 0.0;
   this->SUV_t1 = 0.0;
   this->SUV_t2 = 0.0;
   this->SUVmax_t1 = 0.0;
   this->SUVmax_t2 = 0.0;
   this->SUVmin_t1 = 0.0;
   this->SUVmin_t2 = 0.0;
   this->SUVmean_t1 = 0.0;
   this->SUVmean_t2 = 0.0;
   this->SUVmaxmean_t1 = 0.0;
   this->SUVmaxmean_t2 = 0.0;
   this->RadiopharmaceuticalStartTime = NULL;
  this->DecayCorrection = NULL;
  this->DecayFactor = NULL;
  this->CalibrationFactor = NULL;
  this->FrameReferenceTime = NULL;
  this->RadionuclideHalfLife =NULL;
  this->SeriesTime = NULL;
  this->PhilipsSUVFactor = NULL;
  this->CTRangeMin = 0.0;
  this->CTRangeMax = 255.0;
  this->CTMin = 0.0;
  this->CTMax = 255.0;
  this->ColorRangeMin = 0.0;
  this->ColorRangeMax = 255.0;
  this->PETMin = 0.0;
  this->PETMax = 255.0;
  this->PETSUVmax = 255.0;
}

//----------------------------------------------------------------------------
vtkMRMLPETCTFusionNode::~vtkMRMLPETCTFusionNode()
{
  if ( this->InputCTReference )
    {
    this->SetInputCTReference( NULL );
    }
  if ( this->InputPETReference )
    {
    this->SetInputPETReference( NULL );
    }
  if ( this->InputMask )
    {
    this->SetInputMask ( NULL );
    }
  if ( this->Layout )
    {
    this->SetLayout ( NULL );
    }
  if ( this->PETLUT)
    {
    this->SetPETLUT ( NULL );
    }
  if ( this->CTLUT)
    {
    this->SetCTLUT ( NULL );
    }
  if ( this->MessageText )
    {
    this->SetMessageText( NULL );
    }
  if (this->VolumeUnits )
    {
    this->SetVolumeUnits ( NULL );
    }
  if ( this->WeightUnits )
    {
    this->SetWeightUnits( NULL );
    }
  if ( this->DoseRadioactivityUnits )
    {
    this->SetDoseRadioactivityUnits( NULL );
    }
  if ( this->PatientName )
    {
    this->SetPatientName ( NULL );
    }
  if ( this->StudyDate )
    {
    this->SetStudyDate ( NULL );
    }
  if ( this->TissueRadioactivityUnits )
    {
    this->SetTissueRadioactivityUnits( NULL );
    }
  if ( this->RadiopharmaceuticalStartTime )
    {
    this->RadiopharmaceuticalStartTime = NULL;
    }
  if ( this->DecayCorrection )
    {
    this->SetDecayCorrection( NULL);
    }
  if ( this->DecayFactor )
    {    
    this->DecayFactor = NULL;
    }
  if ( this->CalibrationFactor )
    {    
    this->CalibrationFactor = NULL;
    }
  if ( this->FrameReferenceTime )
    {
    this->FrameReferenceTime = NULL;
    }
  if ( this->RadionuclideHalfLife )
    {
    this->RadionuclideHalfLife = NULL;
    }
  if ( this->SeriesTime )
    {
    this->SeriesTime = NULL;
    }
  if ( this->PhilipsSUVFactor )
    {
    this->PhilipsSUVFactor = NULL;
    }

  this->VolumeRendering = 0;
  this->NumberOfTemporalPositions = 0;
  this->PatientWeight = 0.0;
  this->InjectedDose = 0.0;
  this->SUV_t1 = 0.0;
  this->SUV_t2 = 0.0;
  this->SUVmax_t1 = 0.0;
  this->SUVmax_t2 = 0.0;
  this->SUVmin_t1 = 0.0;
  this->SUVmin_t2 = 0.0;  
  this->SUVmean_t1 = 0.0;
  this->SUVmean_t2 = 0.0;
  this->SUVmaxmean_t1 = 0.0;
  this->SUVmaxmean_t2 = 0.0;
  this->CTRangeMin = 0.0;
  this->CTRangeMax = 255.0;
  this->CTMin = 0.0;
  this->CTMax = 255.0;
  this->ColorRangeMin = 0.0;
  this->ColorRangeMax = 255.0;  
  this->PETMin = 0.0;
  this->PETMax = 255.0;  
  this->PETSUVmax = 255.0;  
}

//----------------------------------------------------------------------------
void vtkMRMLPETCTFusionNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream

  vtkIndent indent(nIndent);


  std::stringstream ss;
  if ( this->InputCTReference )
    {
    ss << this->InputCTReference;
    of << indent << " InputCTReference=\"" << ss.str() << "\"";
    }

  ss.str("");
  ss.clear();
  if ( this->InputPETReference )
    {
    ss << this->InputPETReference;
    of << indent << " InputPETReference=\"" << ss.str() << "\"";
    }

  ss.str("");
  ss.clear();
  if ( this->InputMask )
    {
    ss << this->InputMask;
    of << indent << " InputMask=\"" << ss.str() << "\"";
    }

  ss.str("");
  ss.clear();
  if ( this->MessageText )
    {
    ss << this->MessageText;
    of << indent << " MessageText=\"" << ss.str() << "\"";
    }

  if ( this->PatientName )
    {
    ss.str("");
    ss.clear();
    ss << this->PatientName;
    of << indent << " PatientName=\"" << ss.str() << "\"";
    }

  if ( this->StudyDate )
    {
    ss.str("");
    ss.clear();
    ss << this->StudyDate;
    of << indent << " StudyDate=\"" << ss.str() << "\"";
    }

  ss.str("");
  ss.clear();
  ss << this->WeightUnits;
  of << indent << " PatientWeightUnits=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->VolumeUnits;
  of << indent << " VolumeUnits=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->DoseRadioactivityUnits;
  of << indent << " DoseRadioactivityUnits=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->TissueRadioactivityUnits;
  of << indent << " TissueRadioactivityUnits=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->VolumeRendering;
  of << indent << " VolumeRenderingOn=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->NumberOfTemporalPositions;
  of << indent << " NumberOfTemporalPositions=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->PatientWeight;
  of << indent << " PatientWeightOn=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->InjectedDose;
  of << indent << " InjectedDoseOn=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->SUV_t1;
  of << indent << " SUV_t1=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->SUV_t2;
  of << indent << " SUV_t2=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->SUVmax_t1;
  of << indent << " SUVmax_t1=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->SUVmax_t2;
  of << indent << " SUVmax_t2=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->SUVmin_t1;
  of << indent << " SUVmin_t1=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->SUVmin_t2;
  of << indent << " SUVmin_t2=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->SUVmean_t1;
  of << indent << " SUVmean_t1=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->SUVmean_t2;
  of << indent << " SUVmean_t2=\"" << ss.str() << "\"";
  
  ss.str("");
  ss.clear();
  ss << this->SUVmaxmean_t1;
  of << indent << " SUVmaxmean_t1=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->SUVmaxmean_t2;
  of << indent << " SUVmaxmean_t2=\"" << ss.str() << "\"";
  
  ss.str("");
  ss.clear();
  ss << this->RadiopharmaceuticalStartTime;
  of << indent << " RadiopharmaceuticalStartTime=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->DecayCorrection;
  of << indent << " DecayCorrection=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->CalibrationFactor;
  of << indent << " CalibrationFactor=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->DecayFactor;
  of << indent << " DecayFactor=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->FrameReferenceTime;
  of << indent << " FrameReferenceTime=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->RadionuclideHalfLife;
  of << indent << " RadionuclideHalfLife=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->SeriesTime;
  of << indent << " SeriesTime=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->PhilipsSUVFactor;
  of << indent << " PhilipsSUVFactor=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->CTRangeMin;
  of << indent << " CTRangeMin=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->CTRangeMax;
  of << indent << " CTRangeMax=\"" << ss.str() << "\"";
  
  ss.str("");
  ss.clear();
  ss << this->CTMin;
  of << indent << " CTMin=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->CTMax;
  of << indent << " CTMax=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->ColorRangeMin;
  of << indent << " ColorRangeMin=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->ColorRangeMax;
  of << indent << " ColorRangeMax=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->PETMin;
  of << indent << " PETMin=\"" << ss.str() << "\"";

  ss.str("");
  ss.clear();
  ss << this->PETMax;
  of << indent << " PETMax=\"" << ss.str() << "\"";
  
  ss.str("");
  ss.clear();
  ss << this->PETSUVmax;
  of << indent << " PETSUVmax=\"" << ss.str() << "\"";

}


//----------------------------------------------------------------------------
void vtkMRMLPETCTFusionNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
  
    if (!strcmp(attName, "InputCTReference"))
      {
      this->SetInputCTReference(attValue);
      this->Scene->AddReferencedNodeID(this->InputCTReference, this);
      }
    else if ( !strcmp (attName, "PatientName"))
      {
      this->SetPatientName ( attValue);
      }
    else if ( !strcmp (attName, "StudyDate"))
      {
      this->SetStudyDate ( attValue );
      }
    else if (!strcmp(attName, "PatientWeightUnits"))
      {
      this->SetWeightUnits (attValue );
      }
    else if (!strcmp(attName, "DoseRadioactivityUnits"))
      {
      this->SetDoseRadioactivityUnits (attValue );
      }
    else if (!strcmp(attName, "TissueRadioactivityUnits"))
      {
      this->SetTissueRadioactivityUnits (attValue );
      }
    else if (!strcmp(attName, "VolumeUnits"))
      {
      this->SetVolumeUnits (attValue );
      }
    else if (!strcmp(attName, "NumberOfTemporalPositions"))
      {
      this->SetNumberOfTemporalPositions ( atoi (attValue ));
      }
    else if (!strcmp(attName, "VolumeRenderingOn"))
      {
      this->SetVolumeRendering ( atoi(attValue) );
      }
    else if (!strcmp(attName, "PatientWeight"))
      {
      this->SetPatientWeight ( atof(attValue) );
      }
    else if (!strcmp(attName, "InjectedDose"))
      {
      this->SetInjectedDose ( atof(attValue) );
      }
    else if (!strcmp(attName, "SUV_t1"))
      {
      this->SetSUV_t1( atof (attValue ));
      }
    else if (!strcmp(attName, "SUV_t2"))
      {
      this->SetSUV_t2( atof (attValue ));
      }    
    else if (!strcmp(attName, "SUVmax_t1"))
      {
      this->SetSUVmax_t1( atof (attValue ));
      }
    else if (!strcmp(attName, "SUVmax_t2"))
      {
      this->SetSUVmax_t2( atof (attValue ));
      }
    else if (!strcmp(attName, "SUVmin_t1"))
      {
      this->SetSUVmin_t1( atof (attValue ));
      }
    else if (!strcmp(attName, "SUVmin_t2"))
      {
      this->SetSUVmin_t2( atof (attValue ));
      }
    else if (!strcmp(attName, "SUVmean_t1"))
      {
      this->SetSUVmean_t1( atof (attValue ));
      }
    else if (!strcmp(attName, "SUVmean_t2"))
      {
      this->SetSUVmean_t2( atof (attValue ));
      }
    else if (!strcmp(attName, "SUVmaxmean_t1"))
      {
      this->SetSUVmaxmean_t1( atof (attValue ));
      }
    else if (!strcmp(attName, "SUVmaxmean_t2"))
      {
      this->SetSUVmaxmean_t2( atof (attValue ));
      }
    else if (!strcmp(attName, "RadiopharmaceuticalStartTime"))
      {
      this->SetRadiopharmaceuticalStartTime ( attValue) ;
      }
    else if (!strcmp(attName, "DecayCorrection"))
      {
      this->SetDecayCorrection (attValue );
      }
    else if (!strcmp(attName, "CalibrationFactor"))
      {
      this->SetCalibrationFactor (attValue );
      }
    else if (!strcmp (attName, "CTRangeMin"))
      {
      this->SetCTRangeMin (atof (attValue));
      }
    else if (!strcmp (attName, "CTRangeMax"))
      {
      this->SetCTRangeMax (atof (attValue));
      }
    else if (!strcmp (attName, "CTMin"))
      {
      this->SetCTMin (atof (attValue));
      }
    else if (!strcmp (attName, "CTMax"))
      {
      this->SetCTMax (atof (attValue));
      }
    else if (!strcmp(attName, "ColorRangeMin"))
      {
      this->SetColorRangeMin ( atof (attValue ));
      }
    else if (!strcmp(attName, "ColorRangeMax"))
      {
      this->SetColorRangeMax ( atof (attValue ));
      }
    else if (!strcmp(attName, "PETMin"))
      {
      this->SetPETMin ( atof (attValue ));
      }
    else if (!strcmp(attName, "PETMax"))
      {
      this->SetPETMax ( atof (attValue ));
      }
    else if (!strcmp(attName, "PETSUVmax"))
      {
      this->SetPETSUVmax ( atof (attValue ));
      }
    else if (!strcmp(attName, "DecayFactor"))
      {
      this->SetDecayFactor ( attValue);
      }
    else if (!strcmp(attName, "FrameReferenceTime"))
      {
      this->SetFrameReferenceTime ( attValue );
      }
    else if (!strcmp(attName, "RadionuclideHalfLife"))
      {
      this->SetRadionuclideHalfLife ( attValue );
      }
    else if (!strcmp(attName, "SeriesTime"))
      {
      this->SetSeriesTime ( attValue );
      }
    else if (!strcmp(attName, "PhilipsSUVFactor"))
      {
      this->SetPhilipsSUVFactor ( attValue );
      }
    else if (!strcmp(attName, "InputPETReference"))
      {
      this->SetInputPETReference(attValue);
      this->Scene->AddReferencedNodeID(this->InputPETReference, this);
      }
    else if (!strcmp(attName, "InputMask"))
      {
      this->SetInputMask(attValue);
      this->Scene->AddReferencedNodeID(this->InputMask, this);
      }
    else if ( !strcmp (attName, "PETLUT"))
      {
      this->SetPETLUT(attValue);
      this->Scene->AddReferencedNodeID ( this->PETLUT, this);
      }
    else if ( !strcmp (attName, "CTLUT"))
      {
      this->SetCTLUT(attValue);
      this->Scene->AddReferencedNodeID ( this->CTLUT, this);
      }
    else if (!strcmp(attName, "MessageText"))
      {
      this->SetMessageText(attValue);
      this->Scene->AddReferencedNodeID(this->MessageText, this);
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLPETCTFusionNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLPETCTFusionNode *node = (vtkMRMLPETCTFusionNode *) anode;

  this->SetInputCTReference(node->InputCTReference);
  this->SetInputPETReference(node->InputPETReference);
  this->SetInputMask(node->InputMask);
  this->SetPETLUT(node->PETLUT);
  this->SetCTLUT(node->CTLUT);
  this->SetMessageText(node->MessageText);
  this->SetWeightUnits ( node->GetWeightUnits() );
  this->SetVolumeUnits (node->GetVolumeUnits() );
  this->SetDoseRadioactivityUnits (node->GetDoseRadioactivityUnits() );
  this->SetTissueRadioactivityUnits (node->GetTissueRadioactivityUnits() );
  this->SetVolumeRendering ( node->GetVolumeRendering() );
  this->SetNumberOfTemporalPositions ( node->GetNumberOfTemporalPositions() );
  this->SetInjectedDose ( node->GetInjectedDose () );
  this->SetPatientWeight ( node->GetPatientWeight () );
  this->SetPatientName ( node->GetPatientName() );
  this->SetStudyDate ( node->GetStudyDate () );
  this->SetSUV_t1 (node->GetSUV_t1() );
  this->SetSUV_t2 (node->GetSUV_t2() );
  this->SetSUVmax_t1 (node->GetSUVmax_t1() );
  this->SetSUVmax_t2 (node->GetSUVmax_t2() );
  this->SetSUVmin_t1 (node->GetSUVmin_t1() );
  this->SetSUVmin_t2 (node->GetSUVmin_t2() );
  this->SetSUVmean_t1 (node->GetSUVmean_t1() );
  this->SetSUVmean_t2 (node->GetSUVmean_t2() );
  this->SetSUVmaxmean_t1 (node->GetSUVmaxmean_t1() );
  this->SetSUVmaxmean_t2 (node->GetSUVmaxmean_t2() );
  this->SetRadiopharmaceuticalStartTime (node->GetRadiopharmaceuticalStartTime ( ) );
  this->SetDecayCorrection (node->GetDecayCorrection ( ) );
  this->SetDecayFactor (node->GetDecayFactor ( ) );
  this->SetCalibrationFactor (node->GetCalibrationFactor ( ) );
  this->SetFrameReferenceTime (node->GetFrameReferenceTime ( ) );
  this->SetRadionuclideHalfLife (node->GetRadionuclideHalfLife ( ) );
  this->SetSeriesTime (node->GetSeriesTime ( ) );
  this->SetPhilipsSUVFactor (node->GetPhilipsSUVFactor ( ) );
  this->SetCTRangeMin ( node->GetCTRangeMin() );
  this->SetCTRangeMax ( node->GetCTRangeMax() );
  this->SetCTMin ( node->GetCTMin() );
  this->SetCTMax ( node->GetCTMax() );
  this->SetColorRangeMin ( node->GetColorRangeMin () );
  this->SetColorRangeMax ( node->GetColorRangeMax () );
  this->SetPETMin ( node->GetPETMin () );
  this->SetPETMax ( node->GetPETMax () );
  this->SetPETSUVmax ( node->GetPETSUVmax () );
}

//----------------------------------------------------------------------------
void vtkMRMLPETCTFusionNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "InputCTReference:   " << 
   (this->InputCTReference ? this->InputCTReference : "(none)") << "\n";
  os << indent << "InputPETReference:   " << 
   (this->InputPETReference ? this->InputPETReference : "(none)") << "\n";
  os << indent << "InputMask:   " << 
   (this->InputMask ? this->InputMask : "(none)") << "\n";
  os << indent << "PETLUT:   " << 
   (this->PETLUT ? this->PETLUT : "(none)") << "\n";
  os << indent << "CTLUT:   " << 
   (this->CTLUT ? this->CTLUT : "(none)") << "\n";
  os << indent << "MessageText:   " << 
    (this->MessageText ? this->MessageText : "(none)") << "\n";
  os << indent << "WeightUnits: " << 
    (this->WeightUnits ? this->WeightUnits : "(none)") << "\n";
  os << indent << "VolumeUnits: " << 
    (this->VolumeUnits ? this->VolumeUnits : "(none)") << "\n";
  os << indent << "DoseRadioactivityUnits: " << 
    (this->DoseRadioactivityUnits ? this->DoseRadioactivityUnits : "(none)") << "\n";
  os << indent << "TissueRadioactivityUnits: " << 
    (this->TissueRadioactivityUnits ? this->TissueRadioactivityUnits : "(none)") << "\n";
  os << indent << "InjectedDose: " << this->InjectedDose << "\n";
  os << indent << "PatientName: " << this->PatientName << "\n";
  os << indent << "StudyDate: " << this->StudyDate << "\n";
  os << indent << "PatientWeight: " << this->PatientWeight << "\n";
  os << indent << "VolumeRendering: " << this->VolumeRendering << "\n";
  os << indent << "NumberOfTemporalPositions: " << this->NumberOfTemporalPositions << "\n";
  os << indent << "SUV_t1: " << this->SUV_t1 << "\n";
  os << indent << "SUV_t2: " << this->SUV_t2 << "\n";
  os << indent << "SUVmax_t1: " << this->SUVmax_t1 << "\n";
  os << indent << "SUVmax_t2: " << this->SUVmax_t2 << "\n";
  os << indent << "SUVmin_t1: " << this->SUVmin_t1 << "\n";
  os << indent << "SUVmin_t2: " << this->SUVmin_t2 << "\n";  
  os << indent << "SUVmean_t1: " << this->SUVmean_t1 << "\n";
  os << indent << "SUVmean_t2: " << this->SUVmean_t2 << "\n";
  os << indent << "SUVmaxmean_t1: " << this->SUVmaxmean_t1 << "\n";
  os << indent << "SUVmaxmean_t2: " << this->SUVmaxmean_t2 << "\n";
  os << indent << "RadiopharmaceuticalStartTime: " << this->RadiopharmaceuticalStartTime << "\n";
  os << indent << "DecayCorrection: " << this->DecayCorrection << "\n";
  os << indent << "DecayFactor: " << this->DecayFactor << "\n";
  os << indent << "CalibrationFactor: " << this->CalibrationFactor << "\n";
  os << indent << "FrameReferenceTime: " << this->FrameReferenceTime << "\n";
  os << indent << "RadionuclideHalfLife: " << this->RadionuclideHalfLife << "\n";
  os << indent << "SeriesTime: " << this->SeriesTime << "\n";
  os << indent << "PhilipsSUVFactor: " << this->PhilipsSUVFactor << "\n";
  os << indent << "CTRangeMin: " << this->CTRangeMin << "\n";
  os << indent << "CTRangeMax: " << this->CTRangeMax << "\n";
  os << indent << "CTMin: " << this->CTMin << "\n";
  os << indent << "CTMax: " << this->CTMax << "\n";
  os << indent << "ColorRangeMin: " << this->ColorRangeMin << "\n";
  os << indent << "ColorRangeMax: " << this->ColorRangeMax << "\n";
  os << indent << "PETMin: " << this->PETMin << "\n";
  os << indent << "PETMax: " << this->PETMax << "\n";
  os << indent << "PETSUVmax: " << this->PETSUVmax << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLPETCTFusionNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (!strcmp(oldID, this->InputCTReference))
    {
    this->SetInputCTReference(newID);
    }
  if (!strcmp(oldID, this->InputPETReference))
    {
    this->SetInputPETReference(newID);
    }
  if (!strcmp(oldID, this->InputMask))
    {
    this->SetInputMask(newID);
    }
  if (!strcmp(oldID, this->PETLUT))
    {
    this->SetPETLUT(newID);
    }
  if (!strcmp(oldID, this->CTLUT))
    {
    this->SetCTLUT(newID);
    }
}


//----------------------------------------------------------------------------
void vtkMRMLPETCTFusionNode::SetCTRange ( double min, double max )
{
  this->CTRangeMin = min;
  this->CTRangeMax = max;
  this->InvokeEvent (UpdateCTDisplayEvent );
}


//----------------------------------------------------------------------------
void vtkMRMLPETCTFusionNode::SetColorRange ( double min, double max )
{
  this->ColorRangeMin = min;
  this->ColorRangeMax = max;
  this->InvokeEvent (UpdatePETDisplayEvent );
}

//----------------------------------------------------------------------------
void vtkMRMLPETCTFusionNode::SaveResultToTextFile( const char *fileName )
{
  std::ofstream myfile;
  myfile.open (fileName);
  myfile << this->SUVmax_t1 << ",";
  myfile << this->SUVmax_t2 << ",";
  myfile << this->SUVmin_t1 << ",";
  myfile << this->SUVmin_t2 << ",";  
  myfile << this->SUVmean_t1 << ",";
  myfile << this->SUVmean_t2 << ",";
  myfile << this->SUVmaxmean_t1 << ",";
  myfile << this->SUVmaxmean_t2;
  myfile.close();

}
