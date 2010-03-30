/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLChangeTrackerNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLChangeTrackerNode.h"
#include "vtkMRMLScene.h"

#include <vtksys/ios/sstream>

//------------------------------------------------------------------------------
vtkMRMLChangeTrackerNode* vtkMRMLChangeTrackerNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLChangeTrackerNode");
  if(ret)
    {
      return (vtkMRMLChangeTrackerNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLChangeTrackerNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLChangeTrackerNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLChangeTrackerNode");
  if(ret)
    {
      return (vtkMRMLChangeTrackerNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLChangeTrackerNode;
}

//----------------------------------------------------------------------------
vtkMRMLChangeTrackerNode::vtkMRMLChangeTrackerNode()
{
   // Only one node is created 
  this->SetSingletonTag("vtkMRMLChangeTrackerNode");
   this->HideFromEditors = true;

   this->Scan1_Ref = NULL;
   this->Scan2_Ref = NULL;
   this->Scan1_SuperSampleRef = NULL;
   this->Scan1_SegmentRef = NULL;
   this->WorkingDir= NULL;

   // this->ROIMin[0] = this->ROIMin[1] = this->ROIMin[2] = this->ROIMax[0] = this->ROIMax[1] = this->ROIMax[2] = -1;
   this->ROIMin.resize(3,-1); 
   this->ROIMax.resize(3,-1); 

   this->SuperSampled_Spacing = -1;
   this->SuperSampled_VoxelVolume = -1;
   this->SuperSampled_RatioNewOldSpacing = -1;
   this->Scan1_VoxelVolume = -1;

   this->SegmentThresholdMin=-1;
   this->SegmentThresholdMax=-1;

   this->Scan2_GlobalRef = NULL;
   this->Scan2_SuperSampleRef = NULL;

   this->Scan2_LocalRef = NULL;
   this->Scan2_NormedRef = NULL;

   this->Scan1_ThreshRef = NULL;
   this->Scan2_ThreshRef = NULL;

   this->Grid_Ref = NULL;

   this->Scan2_RegisteredRef = NULL;

   this->Analysis_Intensity_Flag = 0;
   this->Analysis_Intensity_Sensitivity = 0.96;
   this->Analysis_Intensity_Ref = NULL;

   this->Analysis_Deformable_Flag = 0;
   this->Analysis_Deformable_JacobianGrowth = 0.0;
   this->Analysis_Deformable_SegmentationGrowth = 0.0;
   this->Analysis_Deformable_Ref = NULL;

   // AF: by default, use ITK functionality -- it is faster, and appears to be
   // more reliable
   this->UseITK = true;
   this->Scan2_RegisteredReady = false;

   this->RegistrationChoice = REGCHOICE_REGISTER;
   this->ROIRegistration = true;
   this->Scan2_TransformRef = NULL;

   this->ResampleChoice = RESCHOICE_ISO;
   this->ResampleConst = 0.5;

   this->Scan1_InputSegmentRef = NULL;
   this->Scan1_InputSegmentSuperSampleRef = NULL;
   this->Scan2_InputSegmentRef = NULL;
   this->Scan2_InputSegmentSuperSampleRef = NULL;

   this->ROI_Ref = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLChangeTrackerNode::~vtkMRMLChangeTrackerNode()
{
   this->SetScan1_Ref( NULL );
   this->SetScan2_Ref( NULL );
   this->SetScan1_SuperSampleRef( NULL);
   this->SetScan1_SegmentRef(NULL);
   this->SetWorkingDir(NULL);
   this->SetScan2_GlobalRef(NULL);
   this->SetScan2_SuperSampleRef(NULL);
   this->SetScan2_LocalRef(NULL);
   this->SetScan2_NormedRef(NULL);
   this->SetScan1_ThreshRef(NULL);
   this->SetScan2_ThreshRef(NULL);
   this->SetAnalysis_Intensity_Ref(NULL);
   this->SetAnalysis_Deformable_Ref(NULL);
   this->SetGrid_Ref(NULL);
   this->SetScan2_TransformRef(NULL);
   
   this->SetScan1_InputSegmentRef(NULL);
   this->SetScan1_InputSegmentSuperSampleRef(NULL);
   this->SetScan2_InputSegmentRef(NULL);
   this->SetScan2_InputSegmentSuperSampleRef(NULL);

   this->SetROI_Ref(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLChangeTrackerNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkIndent indent(nIndent);

  {
    std::stringstream ss;
    if ( this->Scan1_Ref )
      {
      ss << this->Scan1_Ref;
      of << indent << " Scan1_Ref=\"" << ss.str() << "\"";
     }
  }
  {
    std::stringstream ss;
    if ( this->Scan2_Ref )
      {
      ss << this->Scan2_Ref;
      of << indent << " Scan2_Ref=\"" << ss.str() << "\"";
      }
  }

  of << indent << " ROIMin=\""<< this->ROIMin[0] << " "<< this->ROIMin[1] << " "<< this->ROIMin[2] <<"\"";
  of << indent << " ROIMax=\""<< this->ROIMax[0] << " "<< this->ROIMax[1] << " "<< this->ROIMax[2] <<"\"";

  // Do not write out the following parameters bc they are defined by rest
  // of << indent << " SuperSampled_Spacing=\""<< this->SuperSampled_Spacing  << "\"";
  // of << indent << " SuperSampled_VoxelVolume=\""<< this->SuperSampled_VoxelVolume  << "\"";
  // of << indent << " SuperSampled_RatioNewOldSpacing=\""<< this->SuperSampled_RatioNewOldSpacing  << "\"";

  of << indent << " SegmentThresholdMin=\""<< this->SegmentThresholdMin  << "\"";
  of << indent << " SegmentThresholdMax=\""<< this->SegmentThresholdMax  << "\"";
  of << indent << " Analysis_Intensity_Flag=\""<< this->Analysis_Intensity_Flag  << "\"";
  if (this->Analysis_Intensity_Flag) {
    of << indent << " Analysis_Intensity_Sensitivity=\""<< this->Analysis_Intensity_Sensitivity  << "\"";
  }
  of << indent << " Analysis_Deformable_Flag=\""<< this->Analysis_Deformable_Flag  << "\"";

  of << indent << " UseITK=\"" << this->UseITK << "\"";
  of << indent << " RegistrationChoice=\"" << this->RegistrationChoice << "\"";
  of << indent << " ROIRegistration=\"" << this->ROIRegistration << "\"";

  of << indent << " ResampleChoice=\"" << this->ResampleChoice << "\"";
  of << indent << " ResampleConst=\"" << this->ResampleConst << "\"";
  
  if ( this->Scan1_InputSegmentRef )
      {
      std::stringstream ss;
      ss << this->Scan1_InputSegmentRef;
      of << indent << " Scan1_InputSegmentRef=\"" << ss.str() << "\"";
     }
  if ( this->Scan1_InputSegmentSuperSampleRef )
      {
      std::stringstream ss;
      ss << this->Scan1_InputSegmentSuperSampleRef;
      of << indent << " Scan1_InputSegmentSuperSampleRef=\"" << ss.str() << "\"";
     }
  if ( this->Scan2_InputSegmentRef )
      {
      std::stringstream ss;
      ss << this->Scan2_InputSegmentRef;
      of << indent << " Scan2_InputSegmentRef=\"" << ss.str() << "\"";
     }
  if ( this->Scan2_InputSegmentSuperSampleRef )
      {
      std::stringstream ss;
      ss << this->Scan2_InputSegmentSuperSampleRef;
      of << indent << " Scan2_InputSegmentSuperSampleRef=\"" << ss.str() << "\"";
     }
  if (this->ROI_Ref)
    {
    std::stringstream ss;
    ss << this->ROI_Ref;
    of << indent << " ROI_Ref=\"" << ss.str() << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLChangeTrackerNode::ReadXMLAttributes(const char** atts)
{
  // cout << "vtkMRMLChangeTrackerNode::ReadXMLAttributes(const char** atts)" << endl;
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "Scan1_Ref"))
      {
      this->SetScan1_Ref(attValue);
      this->Scene->AddReferencedNodeID(this->Scan1_Ref, this);
      }
    else if (!strcmp(attName, "Scan2_Ref"))
      {
      this->SetScan2_Ref(attValue);
      this->Scene->AddReferencedNodeID(this->Scan2_Ref, this);
      }
    else if (!strcmp(attName, "ROIMin"))
      {
      // read data into a temporary vector
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >> this->ROIMin[0] >> this->ROIMin[1] >> this->ROIMin[2];
      }
    else if (!strcmp(attName, "ROIMax"))
      {
      // read data into a temporary vector
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >> this->ROIMax[0] >> this->ROIMax[1] >> this->ROIMax[2];
      }
    else if (!strcmp(attName, "SegmentThresholdMin"))
      {
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >>  this->SegmentThresholdMin; 
      }
    else if (!strcmp(attName, "SegmentThresholdMax"))
      {
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >>  this->SegmentThresholdMax; 
      }
    else if (!strcmp(attName, "Analysis_Intensity_Flag"))
      {
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >>  this->Analysis_Intensity_Flag; 
      }
    else if (!strcmp(attName, "Analysis_Intensity_Sensitivity"))
      {
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >>  this->Analysis_Intensity_Sensitivity; 
      }
    else if (!strcmp(attName, "Analysis_Deformable_Flag"))
      {
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >>  this->Analysis_Deformable_Flag; 
      }
    else if(!strcmp(attName, "UseITK"))
      {
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >>  this->UseITK; 
      }
    else if(!strcmp(attName, "RegistrationChoice"))
      {
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >>  this->RegistrationChoice; 
      }
    else if(!strcmp(attName, "ROIRegistration"))
      {
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >>  this->ROIRegistration; 
      }
    else if(!strcmp(attName, "ResampleChoice"))
      {
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >>  this->ResampleChoice; 
      }
    else if(!strcmp(attName, "ResampleConst"))
      {
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >>  this->ResampleConst; 
      }
    else if(!strcmp(attName, "Scan1_InputSegmentRef"))
      {
      this->SetScan1_InputSegmentRef(attValue);
      this->Scene->AddReferencedNodeID(this->Scan1_InputSegmentRef, this);
      }
    else if(!strcmp(attName, "Scan1_InputSegmentSuperSampleRef"))
      {
      this->SetScan1_InputSegmentSuperSampleRef(attValue);
      this->Scene->AddReferencedNodeID(this->Scan1_InputSegmentSuperSampleRef, this);
      }
    else if(!strcmp(attName, "Scan2_InputSegmentRef"))
      {
      this->SetScan2_InputSegmentRef(attValue);
      this->Scene->AddReferencedNodeID(this->Scan2_InputSegmentRef, this);
      }
    else if(!strcmp(attName, "Scan2_InputSegmentSuperSampleRef"))
      {
      this->SetScan2_InputSegmentSuperSampleRef(attValue);
      this->Scene->AddReferencedNodeID(this->Scan2_InputSegmentSuperSampleRef, this);
      }
    else if(!strcmp(attName, "ROI_Ref"))
      {
      this->SetROI_Ref(attValue);
      this->Scene->AddReferencedNodeID(this->ROI_Ref, this);
      }

    // AF: should the intermediate volumes be stored here?
    /*
    else if(!strcmp(attName, "Scan2_RegisteredRef"))
      {
      vtksys_stl::stringstream ss;
      ss << attValue;
      ss >>  this->UseITK; 
      }
    */
    }
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLChangeTrackerNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLChangeTrackerNode *node = (vtkMRMLChangeTrackerNode *) anode;

  this->SetScan1_Ref(node->Scan1_Ref);
  this->SetScan2_Ref(node->Scan2_Ref);
  this->ROIMin = node->ROIMin; 
  this->ROIMax = node->ROIMax; 
  this->SegmentThresholdMin = node->SegmentThresholdMin; 
  this->SegmentThresholdMax = node->SegmentThresholdMax; 
  this->Analysis_Intensity_Flag = node->Analysis_Intensity_Flag; 
  this->Analysis_Intensity_Sensitivity = node->Analysis_Intensity_Sensitivity; 
  this->Analysis_Deformable_Flag = node->Analysis_Deformable_Flag; 
  this->UseITK = node->UseITK;
  this->RegistrationChoice = node->RegistrationChoice;
  this->ROIRegistration = node->ROIRegistration;
  this->ResampleChoice = node->ResampleChoice;
  this->ResampleConst = node->ResampleConst;
  this->SetScan1_InputSegmentRef(node->Scan1_InputSegmentRef);
  this->SetScan2_InputSegmentRef(node->Scan2_InputSegmentRef);
  this->SetScan1_InputSegmentSuperSampleRef(node->Scan1_InputSegmentSuperSampleRef);
  this->SetScan2_InputSegmentSuperSampleRef(node->Scan2_InputSegmentSuperSampleRef);
  this->SetROI_Ref(node->ROI_Ref);
  // AF: why not all of the volume references are copied?
}

//----------------------------------------------------------------------------
void vtkMRMLChangeTrackerNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  // vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "ID:                   " << (this->ID ? this->ID:"(none)") << "\n";
  os << indent << "Name:                 " << (this->Name ? this->Name:"(none)") << "\n";
  os << indent << "Scan1_Ref:            " << 
   (this->Scan1_Ref ? this->Scan1_Ref : "(none)") << "\n";
  os << indent << "Scan2_Ref:            " << 
   (this->Scan2_Ref ? this->Scan2_Ref : "(none)") << "\n";
  os << indent << "Scan1_SuperSampleRef: " << 
   (this->Scan1_SuperSampleRef ? this->Scan1_SuperSampleRef : "(none)") << "\n";
  os << indent << "Scan1_SegmentRef:     " << 
   (this->Scan1_SegmentRef ? this->Scan1_SegmentRef : "(none)") << "\n";
  os << indent << "Scan2_RegisteredRef:  " <<
    (this->Scan2_RegisteredRef ? this->Scan2_RegisteredRef : "(none)") << "\n";
  os << indent << "ROIMin:               "<< this->ROIMin[0] << " "<< this->ROIMin[1] << " "<< this->ROIMin[2] <<"\n";
  os << indent << "ROIMax:               "<< this->ROIMax[0] << " "<< this->ROIMax[1] << " "<< this->ROIMax[2] <<"\n";
  os << indent << "SegmentThresholdMin:     "<< this->SegmentThresholdMin << "\n";
  os << indent << "SegmentThresholdMax:     "<< this->SegmentThresholdMax << "\n";
  os << indent << "Analysis_Intensity_Flag: "<< this->Analysis_Intensity_Flag << "\n";
  os << indent << "Analysis_Intensity_Sensitivity: "<< this->Analysis_Intensity_Sensitivity << "\n";
  os << indent << "Analysis_Deformable_Flag: "<< this->Analysis_Deformable_Flag << "\n";
  os << indent << "WorkingDir:           " <<  (this->WorkingDir ? this->WorkingDir : "(none)") << "\n";
  os << indent << "UseITK:  " << this->UseITK << "\n";
  os << indent << "Scan2_RegisteredReady: " << this->Scan2_RegisteredReady << "\n";
  os << indent << "RegistrationChoice: ";
  switch(this->RegistrationChoice){
  case REGCHOICE_ALIGNED: os << "REGCHOICE_ALIGNED"; break;
  case REGCHOICE_RESAMPLE: os << "REGCHOICE_RESAMPLE"; break;
  case REGCHOICE_REGISTER: os << "REGCHOICE_REGISTER"; break;
  default: os << "Invalid value";
  }
  os << "\n";
  os << indent << "ROIRegistration: " << this->ROIRegistration << "\n";
  
  switch(this->ResampleChoice){
  case RESCHOICE_NONE: os << "REGCHOICE_NONE"; break;
  case RESCHOICE_LEGACY: os << "RESCHOICE_LEGACY"; break;
  case RESCHOICE_ISO: os << "RESCHOICE_ISO"; break;
  default: os << "Invalid value";
  }
  os << "\n";
  os << indent << "ResampleConst: " << this->ResampleConst << "\n";

  if (this->GetScan1_InputSegmentRef())
    {
    os << indent << "Scan1_InputSegmentRef: " << GetScan1_InputSegmentRef() << "\n";
    }
  else
    {
    os << indent << "Scan1_InputSegmentRef: NULL\n";
    }
  if (this->GetScan2_InputSegmentRef())
    {
    os << indent << "Scan2_InputSegmentRef: " << GetScan2_InputSegmentRef() << "\n";
    }
  else
    {
    os << indent << "Scan2_InputSegmentRef: NULL\n";
    }

  if (this->GetScan1_InputSegmentSuperSampleRef())
    {
    os << indent << "Scan1_InputSegmentSuperSampleRef: " << GetScan1_InputSegmentSuperSampleRef() << "\n";
    }
  else
    {
    os << indent << "Scan1_InputSegmentSuperSampleRef: NULL\n";
    }

  if (this->GetScan2_InputSegmentSuperSampleRef())
    {
    os << indent << "Scan2_InputSegmentSuperSampleRef: " << GetScan2_InputSegmentSuperSampleRef() << "\n";
    }
  else
    {
    os << indent << "Scan2_InputSegmentSuperSampleRef: NULL\n";
    }

  os << "\n";
  if (this->GetROI_Ref())
    {
    os << indent << "ROI_Ref: " << GetROI_Ref() << "\n";
    }
  else
    {
    os << indent << "ROI_Ref: NULL\n";
    }
  os << "\n";
}

