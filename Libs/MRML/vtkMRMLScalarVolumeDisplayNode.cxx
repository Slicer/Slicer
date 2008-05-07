/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLScalarVolumeDisplayNode* vtkMRMLScalarVolumeDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLScalarVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLScalarVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLScalarVolumeDisplayNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLScalarVolumeDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLScalarVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLScalarVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLScalarVolumeDisplayNode;
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeDisplayNode::vtkMRMLScalarVolumeDisplayNode()
{
  // Strings
  this->Interpolate = 1;
  this->AutoWindowLevel = 1;
  this->Window = 256;
  this->Level = 128;
  this->AutoThreshold = 0;
  this->ApplyThreshold = 0;
  this->LowerThreshold = VTK_SHORT_MIN;
  this->UpperThreshold = VTK_SHORT_MAX;

  // try setting a default greyscale color map
  //this->SetDefaultColorMap(0);

  // create and set visulaization pipeline
  this->ResliceAlphaCast = vtkImageCast::New();
  this->AlphaLogic = vtkImageLogic::New();
  this->MapToColors = vtkImageMapToColors::New();
  this->Threshold = vtkImageThreshold::New();
  this->AppendComponents = vtkImageAppendComponents::New();
  this->MapToWindowLevelColors = vtkImageMapToWindowLevelColors::New();

  this->MapToWindowLevelColors->SetOutputFormatToLuminance();
  this->MapToColors->SetOutputFormatToRGB();
  this->MapToColors->SetInput( this->MapToWindowLevelColors->GetOutput() );
  this->Threshold->SetOutputScalarTypeToUnsignedChar();
  this->ResliceAlphaCast->SetOutputScalarTypeToUnsignedChar();

  this->AlphaLogic->SetOperationToAnd();
  this->AlphaLogic->SetOutputTrueValue(255);
  this->AlphaLogic->SetInput1( this->Threshold->GetOutput() );
  //this->AlphaLogic->SetInput2( this->Threshold->GetOutput() );
  this->AlphaLogic->SetInput2( this->ResliceAlphaCast->GetOutput() );

  this->AppendComponents->RemoveAllInputs();
  this->AppendComponents->SetInputConnection(0, this->MapToColors->GetOutput()->GetProducerPort() );
  this->AppendComponents->AddInputConnection(0, this->AlphaLogic->GetOutput()->GetProducerPort() );
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetDefaultColorMap()
{
  this->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");


}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeDisplayNode::~vtkMRMLScalarVolumeDisplayNode()
{
  this->SetAndObserveColorNodeID( NULL);

  this->ResliceAlphaCast->Delete();
  this->AlphaLogic->Delete();
  this->MapToColors->Delete();
  this->Threshold->Delete();
  this->AppendComponents->Delete();
  this->MapToWindowLevelColors->Delete();

}

void vtkMRMLScalarVolumeDisplayNode::UpdateImageDataPipeline()
{
  Superclass::UpdateImageDataPipeline();

  vtkLookupTable *lookupTable = NULL;
  if (this->GetColorNode())
    {
    lookupTable = this->GetColorNode()->GetLookupTable();
    }

  if ( this->MapToColors && (lookupTable != this->MapToColors->GetLookupTable()) )
    {
    this->MapToColors->SetLookupTable(lookupTable);
    }

  this->MapToWindowLevelColors->SetWindow(this->GetWindow());
  this->MapToWindowLevelColors->SetLevel(this->GetLevel());

  if ( this->GetApplyThreshold() )
    {
    this->Threshold->ReplaceInOn();
    this->Threshold->SetInValue(255);
    this->Threshold->ReplaceOutOn();
    this->Threshold->SetOutValue(0);
    this->Threshold->ThresholdBetween( this->GetLowerThreshold(), 
                                       this->GetUpperThreshold() );
    }
  else
    {
    // don't apply threshold - alpha channel becomes 255 everywhere
    this->Threshold->ThresholdBetween( 1, 0 ); 
    this->Threshold->ReplaceInOn();
    this->Threshold->SetInValue(255);
    this->Threshold->ReplaceOutOn();
    this->Threshold->SetOutValue(255); 
    }
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  {
  std::stringstream ss;
  ss << this->Window;
  of << indent << " window=\"" << ss.str() << "\"";
  }
  {
  std::stringstream ss;
  ss << this->Level;
  of << indent << " level=\"" << ss.str() << "\"";
  }
  {
  std::stringstream ss;
  ss << this->UpperThreshold;
  of << indent << " upperThreshold=\"" << ss.str() << "\"";
  }
  {    
  std::stringstream ss;
  ss << this->LowerThreshold;
  of << indent << " lowerThreshold=\"" << ss.str() << "\"";
  }
  {   
  std::stringstream ss;
  ss << this->Interpolate;
  of << indent << " interpolate=\"" << ss.str() << "\"";
  }
  {
  std::stringstream ss;
  ss << this->AutoWindowLevel;
  of << indent << " autoWindowLevel=\"" << ss.str() << "\"";
  }
  {
  std::stringstream ss;
  ss << this->ApplyThreshold;
  of << indent << " applyThreshold=\"" << ss.str() << "\"";
  }
  {
  std::stringstream ss;
  ss << this->AutoThreshold;
  of << indent << " autoThreshold=\"" << ss.str() << "\"";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "window")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Window;
      }
    else if (!strcmp(attName, "level")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Level;
      }
    else if (!strcmp(attName, "upperThreshold")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->UpperThreshold;
      }
    else if (!strcmp(attName, "lowerThreshold")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LowerThreshold;
      }
    else if (!strcmp(attName, "interpolate")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Interpolate;
      }
    else if (!strcmp(attName, "autoWindowLevel")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->AutoWindowLevel;
      }
    else if (!strcmp(attName, "applyThreshold")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ApplyThreshold;
      }
    else if (!strcmp(attName, "autoThreshold")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->AutoThreshold;
      }
    }  
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLScalarVolumeDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLScalarVolumeDisplayNode *node = (vtkMRMLScalarVolumeDisplayNode *) anode;
  
  this->DisableModifiedEventOn();

  this->SetAutoWindowLevel(node->AutoWindowLevel);
  this->SetWindow(node->Window);
  this->SetLevel(node->Level);
  this->SetAutoThreshold(node->AutoThreshold);
  this->SetApplyThreshold(node->ApplyThreshold);
  this->SetUpperThreshold(node->UpperThreshold);
  this->SetLowerThreshold(node->LowerThreshold);
  this->SetInterpolate(node->Interpolate);
  
  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "AutoWindowLevel:   " << this->AutoWindowLevel << "\n";
  os << indent << "Window:            " << this->Window << "\n";
  os << indent << "Level:             " << this->Level << "\n";
  os << indent << "AutoThreshold:     " << this->AutoThreshold << "\n";
  os << indent << "ApplyThreshold:    " << this->ApplyThreshold << "\n";
  os << indent << "UpperThreshold:    " << this->UpperThreshold << "\n";
  os << indent << "LowerThreshold:    " << this->LowerThreshold << "\n";
  os << indent << "Interpolate:       " << this->Interpolate << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetAutoWindowLevel (int flag)
{
  if (this->AutoWindowLevel == flag)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting AutoWindowLevel to " << flag);
  this->AutoWindowLevel = flag;

  // invoke a modified event
  this->Modified();

  // TODO: make sure trigger update of the auto win/level
  
  return;
}
