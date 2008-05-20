/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

#include "vtkPointData.h"
#include "vtkDataArray.h"

// for calculating auto win/level
#include "vtkImageAccumulateDiscrete.h"
#include "vtkImageBimodalAnalysis.h"
#include "vtkImageExtractComponents.h"

// temporary fix for finding stats volumes
#include "vtkMRMLStorageNode.h"

//------------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLScalarVolumeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLScalarVolumeNode");
  if(ret)
    {
    return (vtkMRMLScalarVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLScalarVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScalarVolumeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLScalarVolumeNode");
  if(ret)
    {
    return (vtkMRMLScalarVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLScalarVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode::vtkMRMLScalarVolumeNode()
{
  this->Bimodal = vtkImageBimodalAnalysis::New();
  this->Accumulate = vtkImageAccumulateDiscrete::New();
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode::~vtkMRMLScalarVolumeNode()
{
  if (this->Bimodal)
    {
    this->Bimodal->Delete();
    this->Bimodal = NULL;
    }
  if (this->Accumulate)
    {
    this->Accumulate->Delete();
    this->Accumulate = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  std::stringstream ss;
  ss << this->GetLabelMap();
  of << indent << " labelMap=\"" << ss.str() << "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "labelMap")) 
      {
      std::stringstream ss;
      int val;
      ss << attValue;
      ss >> val;
      this->SetLabelMap(val);
      }
    }  
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLScalarVolumeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLScalarVolumeNode *node = (vtkMRMLScalarVolumeNode *) anode;

  this->SetLabelMap(node->GetLabelMap());
}

//-----------------------------------------------------------
void vtkMRMLScalarVolumeNode::CreateNoneNode(vtkMRMLScene *scene)
{
  vtkMRMLScalarVolumeNode *n = vtkMRMLScalarVolumeNode::New();
  n->SetName("None");
  // the scene will set the id
//  n->SetID("None");

  // Create a None volume RGBA of 0, 0, 0 so the filters won't complain
  // about missing input
  vtkImageData *id;
  id = vtkImageData::New();
  id->SetDimensions(1, 1, 1);
  id->SetNumberOfScalarComponents(4);
  id->AllocateScalars();
  id->GetPointData()->GetScalars()->FillComponent(0, 0.0);
  id->GetPointData()->GetScalars()->FillComponent(1, 125.0);
  id->GetPointData()->GetScalars()->FillComponent(2, 0.0);
  id->GetPointData()->GetScalars()->FillComponent(3, 0.0);

  n->SetImageData(id);
  scene->AddNode(n);

  n->Delete();
  id->Delete();
}


//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
}

int vtkMRMLScalarVolumeNode::GetLabelMap()
{
  if (!this->GetAttribute("LabelMap"))
    {
    return 0;
    }
  
  std::string value = this->GetAttribute("LabelMap");
  if (value == "0")
    {
    return 0;
    }
  else
    {
    return 1;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::LabelMapOn()
{
  this->SetLabelMap(1);
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::LabelMapOff()
{
  this->SetLabelMap(0);
}


//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::SetLabelMap(int flag)
{
  std::string value;
  if (flag)
    {
    value = "1";
    }
  else
    {
    value = "0";
    }

  const char *attr = this->GetAttribute("LabelMap");
  if (attr && (value == attr))
    {
    return;
    }
  
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting LabelMap to " << flag);

  this->SetAttribute("LabelMap", value.c_str());

/*
    if (this->GetDisplayNode() != NULL)
      {
      if (this->LabelMap == 1)
        {
        // set the display node's color node to be Labels
        vtkDebugMacro("Label map is 1, need to update the display node to be labels\n");
        this->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorNodeLabels");
        }
      else
        {
        vtkDebugMacro("Label map is not 1, updating color node in display to be grey (this is too restrictive)\n");
        this->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorNodeGrey");
        }
      }
    else
      {
      vtkErrorMacro("ERROR: no display node associated with this scalar volume, not changing color node\n");
      }
*/
     // invoke a modified event
    this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::UpdateFromMRML()
{
  vtkDebugMacro("UpdateFromMRML: calling calculate auto levels");
  // check for a stats volume - name or storage node's file name
  std::string nodeName = "";
  std::string fileName = "";
  if ( this->GetName() != NULL )
    {
    nodeName = std::string(this->GetName());
    }
  if (this->GetStorageNode() != NULL)
    {
    if ( this->GetStorageNode()->GetFileName() != NULL )
      {
      fileName = std::string(this->GetStorageNode()->GetFileName());
      }
    else
      {
      fileName = std::string("");
      }
    }
  // TODO: checking the name for 'stat' -- should be an attrbute like labelMap.
  if ((nodeName != "" && nodeName.find("stat",0) != std::string::npos) ||
      (fileName != "" && fileName.find("stat",0) != std::string::npos))
    {
    vtkMRMLScalarVolumeDisplayNode *sdNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
    this->CalculateStatisticsAutoLevels(sdNode, this->GetImageData());
    }
  else
    {
    this->CalculateScalarAutoLevels(NULL, NULL);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::CalculateScalarAutoLevels(vtkMRMLScalarVolumeDisplayNode *refNode, vtkImageData *refData)
{
  if (refNode == NULL &&  !this->GetScalarVolumeDisplayNode())
    {
    vtkDebugMacro("CalculateScalarAutoLevels: input display node is null and can't get local display node");
    return;
    }
  if (refData == NULL && !this->GetImageData())
    {
    vtkDebugMacro("CalculateScalarAutoLevels: input image data is null, and can't get local image data");
    return;
    }
  
  vtkMRMLScalarVolumeDisplayNode *displayNode;
  if (refNode == NULL)
    {
    displayNode = this->GetScalarVolumeDisplayNode();
    }
  else
    {
    displayNode = refNode;
    }
  
  vtkImageData *imageDataScalar;
  if (refData == NULL)
    {
    imageDataScalar = this->GetImageData();
    }
  else
    {
    imageDataScalar = refData;
    }

  if ( !imageDataScalar )
    {
    vtkDebugMacro("CalculateScalarAutoLevels: image data is null");
    return;
    }
  
  if (!displayNode->GetAutoWindowLevel() && !displayNode->GetAutoThreshold())
    {
    vtkDebugMacro("CalculateScalarAutoLevels: " << (this->GetID() == NULL ? "nullid" : this->GetID()) << ": Auto window level not turned on, returning.");
    return;
    }

   displayNode->DisableModifiedEventOn();

  if (imageDataScalar && imageDataScalar->GetNumberOfScalarComponents() == 1) 
    {
    // check the scalar type, bimodal analysis only works on int
    if (imageDataScalar->GetScalarType() != VTK_INT)
      {
      vtkDebugMacro("CalculateScalarAutoLevels: image data scalar type is not integer, doing ad hoc calc of window/level.");
      }

    if (this->Bimodal == NULL)
      {
      this->Bimodal = vtkImageBimodalAnalysis::New();
      }
    if (this->Accumulate == NULL)
      {
      this->Accumulate = vtkImageAccumulateDiscrete::New();
      }
    this->Accumulate->SetInput(imageDataScalar);
    this->Bimodal->SetInput(this->Accumulate->GetOutput());
    this->Bimodal->Update();
    double range[2];
    imageDataScalar->GetScalarRange(range);
    // Workaround for image data where all accumulate samples fall
    // within the same histogram bin
    if (this->Bimodal->GetWindow() == 0.0 && this->Bimodal->GetLevel() == 0.0 ||
        (imageDataScalar->GetScalarType() != VTK_INT && range[1]-range[0] < 1.00001) ) 
      {
 
      double min = range[0];
      double max = range[1];
      //std::cout << "CalculateScalarAutoLevels: Window and Level are 0, or type is not int, using image scalar range, " << min << ", " << max << std::endl);
      if (displayNode->GetAutoWindowLevel())
        {    
        displayNode->SetWindow (max-min);
        displayNode->SetLevel (0.5*(max+min));
        }
      if (displayNode->GetAutoThreshold())
        {
        displayNode->SetLowerThreshold (displayNode->GetLevel());
        displayNode->SetUpperThreshold (range[1]);
        }
      vtkDebugMacro("CalculateScalarAutoLevels: set display node window to " << displayNode->GetWindow() << ", level to " << displayNode->GetLevel() << ", lower threshold to " << displayNode->GetLowerThreshold() << ", upper threshold to " << displayNode->GetUpperThreshold() << " (scalar range " << range[0] << ", " << range[1] << "), displayNode id = " << displayNode->GetID());
      }
    else
      {
      if (displayNode->GetAutoWindowLevel()) 
        {
        displayNode->SetWindow (this->Bimodal->GetWindow());
        displayNode->SetLevel (this->Bimodal->GetLevel());
        }
      if (displayNode->GetAutoThreshold())
        {
        displayNode->SetLowerThreshold (this->Bimodal->GetThreshold());
        displayNode->SetUpperThreshold (this->Bimodal->GetMax());
        }
      vtkDebugMacro("CalculateScalarAutoLevels: set display node window to " << this->Bimodal->GetWindow() << ", level to " << this->Bimodal->GetLevel() << ", lower threshold to " << displayNode->GetLowerThreshold() << ", upper threshold to " << displayNode->GetUpperThreshold() << ", displayNode id = " << displayNode->GetID());
      }
    }
    displayNode->DisableModifiedEventOff();
    displayNode->InvokePendingModifiedEvent();

}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::CalculateStatisticsAutoLevels(vtkMRMLScalarVolumeDisplayNode *refNode, vtkImageData *refData)
{
  if (refNode == NULL && !this->GetScalarVolumeDisplayNode())
    {
    vtkDebugMacro("CalculateStatisticsAutoLevels: input display node is null and can't get local display node");
    return;
    }

  vtkMRMLScalarVolumeDisplayNode *displayNode;
  if (refNode == NULL)
    {
    displayNode = this->GetScalarVolumeDisplayNode();
    }
  else
    {
    displayNode = refNode;
    }

  if (!displayNode)
    {
    vtkDebugMacro("CalculateStatisticsAutoLevels: can't get display node.");
    return;
    }
                  
  vtkImageData *imageDataScalar;
  if (refData == NULL)
    {
    imageDataScalar = this->GetImageData();
    }
  else
    {
    imageDataScalar = refData;
    }

  if ( !imageDataScalar )
    {
    vtkDebugMacro("CalculateStatisticsAutoLevels: image data is null");
    return;
    }
  
  if (!displayNode->GetAutoWindowLevel())
    {
    vtkDebugMacro("CalculateStatisticsAutoLevels: " << (this->GetID() == NULL ? "nullid" : this->GetID()) << ": Auto window level not turned on, returning.");
    return;
    }

  if ( displayNode != NULL ) 
    {
    displayNode->DisableModifiedEventOn();

    double win, level, upT, lowT;
    displayNode->SetAutoThreshold (0);
    displayNode->SetAutoWindowLevel (0);
    win = displayNode->GetWindow();
//    if (win == 256)
      {
      // it probably was not set yet. just do it
      if (this->Bimodal == NULL)
        {
        this->Bimodal = vtkImageBimodalAnalysis::New();
        }
      if (this->Accumulate == NULL)
        {
        this->Accumulate = vtkImageAccumulateDiscrete::New();
        }
      this->Accumulate->SetInput(imageDataScalar);
      this->Bimodal->SetInput(this->Accumulate->GetOutput());
      this->Bimodal->Update();
            
      displayNode->SetWindow (this->Bimodal->GetWindow());
      displayNode->SetLevel (this->Bimodal->GetLevel());
      displayNode->SetLowerThreshold (this->Bimodal->GetThreshold());
      displayNode->SetUpperThreshold (this->Bimodal->GetMax());
            
      win = displayNode->GetWindow();
      vtkDebugMacro("CalculateStatisticsAutoLevels: rehacked stuff");
      }
    level = displayNode->GetLevel();
    upT = displayNode->GetUpperThreshold();
    lowT = displayNode->GetLowerThreshold();
    
    //--- set window... a guess
    displayNode->SetWindow ( win/2.6 );
    win = displayNode->GetWindow();
    displayNode->SetLevel ( upT - (win/2.0) );
    
    //--- set lower threshold
    displayNode->SetLowerThreshold ( upT - ( (upT-lowT)/2.5));
    displayNode->SetUpperThreshold ( upT );
    
    //-- apply the settings
    displayNode->SetApplyThreshold(1);
    displayNode->SetAutoThreshold( 0 );
    displayNode->SetAutoWindowLevel(0);
    
    vtkDebugMacro("CalculateStatisticsAutoLevels: reset display node win/level/thresh! window = " << displayNode->GetWindow() << ", level = " << displayNode->GetLevel() << ", upper thresh = " << displayNode->GetUpperThreshold() << ", lower thresh = " << displayNode->GetLowerThreshold());
    
    displayNode->DisableModifiedEventOff();
    displayNode->InvokePendingModifiedEvent();
    }

}
