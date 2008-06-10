/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTumorGrowthNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLTumorGrowthNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLTumorGrowthNode* vtkMRMLTumorGrowthNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLTumorGrowthNode");
  if(ret)
    {
      return (vtkMRMLTumorGrowthNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTumorGrowthNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLTumorGrowthNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLTumorGrowthNode");
  if(ret)
    {
      return (vtkMRMLTumorGrowthNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTumorGrowthNode;
}

//----------------------------------------------------------------------------
vtkMRMLTumorGrowthNode::vtkMRMLTumorGrowthNode()
{
   // Only one node is created 
   this->SingletonTag = "vtkMRMLTumorGrowthNode";
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

   this->Analysis_Intensity_Flag = 0;
   this->Analysis_Intensity_Sensitivity = 0.5;
   this->Analysis_Intensity_Ref = NULL;

   this->Analysis_Deformable_Flag = 0;
   this->Analysis_Deformable_JacobianGrowth = 0.0;
   this->Analysis_Deformable_SegmentationGrowth = 0.0;
   this->Analysis_Deformable_Ref = NULL;

}

//----------------------------------------------------------------------------
vtkMRMLTumorGrowthNode::~vtkMRMLTumorGrowthNode()
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
}

//----------------------------------------------------------------------------
void vtkMRMLTumorGrowthNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  cout << "vtkMRMLTumorGrowthNode::WriteXML" << endl;
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

}

//----------------------------------------------------------------------------
void vtkMRMLTumorGrowthNode::ReadXMLAttributes(const char** atts)
{
  // cout << "vtkMRMLTumorGrowthNode::ReadXMLAttributes(const char** atts)" << endl;
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
    }
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLTumorGrowthNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLTumorGrowthNode *node = (vtkMRMLTumorGrowthNode *) anode;

  this->SetScan1_Ref(node->Scan1_Ref);
  this->SetScan2_Ref(node->Scan2_Ref);
  this->ROIMin = node->ROIMin; 
  this->ROIMax = node->ROIMax; 
  this->SegmentThresholdMin = node->SegmentThresholdMin; 
  this->SegmentThresholdMax = node->SegmentThresholdMax; 
  this->Analysis_Intensity_Flag = node->Analysis_Intensity_Flag; 
  this->Analysis_Intensity_Sensitivity = node->Analysis_Intensity_Sensitivity; 
  this->Analysis_Deformable_Flag = node->Analysis_Deformable_Flag; 
}

//----------------------------------------------------------------------------
void vtkMRMLTumorGrowthNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  // vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "ID:                   " << this->ID   << "\n";
  os << indent << "Name:                 " << this->Name << "\n";
  os << indent << "Scan1_Ref:            " << 
   (this->Scan1_Ref ? this->Scan1_Ref : "(none)") << "\n";
  os << indent << "Scan2_Ref:            " << 
   (this->Scan2_Ref ? this->Scan2_Ref : "(none)") << "\n";
  os << indent << "Scan1_SuperSampleRef: " << 
   (this->Scan1_SuperSampleRef ? this->Scan1_SuperSampleRef : "(none)") << "\n";
  os << indent << "Scan1_SegmentRef:     " << 
   (this->Scan1_SegmentRef ? this->Scan1_SegmentRef : "(none)") << "\n";
  os << indent << "ROIMin:               "<< this->ROIMin[0] << " "<< this->ROIMin[1] << " "<< this->ROIMin[2] <<"\n";
  os << indent << "ROIMax:               "<< this->ROIMax[0] << " "<< this->ROIMax[1] << " "<< this->ROIMax[2] <<"\n";
  os << indent << "SegmentThresholdMin:     "<< this->SegmentThresholdMin << "\n";
  os << indent << "SegmentThresholdMax:     "<< this->SegmentThresholdMax << "\n";
  os << indent << "Analysis_Intensity_Flag: "<< this->Analysis_Intensity_Flag << "\n";
  os << indent << "Analysis_Intensity_Sensitivity: "<< this->Analysis_Intensity_Sensitivity << "\n";
  os << indent << "Analysis_Deformable_Flag: "<< this->Analysis_Deformable_Flag << "\n";
  os << indent << "WorkingDir:           " <<  (this->WorkingDir ? this->WorkingDir : "(none)") << "\n";

}



