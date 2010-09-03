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
#include "vtkMRMLProceduralColorNode.h"

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

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::UpdateImageDataPipeline()
{
  Superclass::UpdateImageDataPipeline();

  vtkScalarsToColors *lookupTable = NULL;
  vtkMRMLColorNode* colorNode = this->GetColorNode();
  if (colorNode)
    {
    lookupTable = colorNode->GetLookupTable();
    if (lookupTable == NULL)
      {
      if (vtkMRMLProceduralColorNode::SafeDownCast(colorNode) != NULL)
        {
        vtkDebugMacro("UpdateImageDataPipeline: getting color transfer function");
        lookupTable = (vtkScalarsToColors*)(vtkMRMLProceduralColorNode::SafeDownCast(colorNode)->GetColorTransferFunction());
        }
      }
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
  if (this->WindowLevelPresets.size() > 0)
    {
    for (int p = 0; p < this->GetNumberOfWindowLevelPresets(); p++)
      {
      std::stringstream ss;
      ss << this->WindowLevelPresets[p].Window;
      ss << "|";
      ss << this->WindowLevelPresets[p].Level;
      of << indent << " windowLevelPreset" << p << "=\"" << ss.str() << "\"";
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

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
    else if (!strncmp(attName, "windowLevelPreset", 17)) 
      {
      this->AddWindowLevelPresetFromString(attValue);
      }
    }  
    
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLScalarVolumeDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLScalarVolumeDisplayNode *node = (vtkMRMLScalarVolumeDisplayNode *) anode;
  
  this->SetAutoWindowLevel(node->AutoWindowLevel);
  this->SetWindow(node->Window);
  this->SetLevel(node->Level);
  this->SetAutoThreshold(node->AutoThreshold);
  this->SetApplyThreshold(node->ApplyThreshold);
  this->SetUpperThreshold(node->UpperThreshold);
  this->SetLowerThreshold(node->LowerThreshold);
  this->SetInterpolate(node->Interpolate);
  for (int p = 0; p < node->GetNumberOfWindowLevelPresets(); p++)
    {
    this->AddWindowLevelPreset(node->GetWindowPreset(p), node->GetLevelPreset(p));
    }

  this->EndModify(disabledModify);
  
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "AutoWindowLevel:   " << this->AutoWindowLevel << "\n";
  os << indent << "Window:            " << this->Window << "\n";
  os << indent << "Level:             " << this->Level << "\n";
  os << indent << "Window Level Presets:\n";
  for (int p = 0; p < this->GetNumberOfWindowLevelPresets(); p++)
    {
    os << indent.GetNextIndent() << p << " Window: " << this->GetWindowPreset(p) << " | Level: " << this->GetLevelPreset(p) << "\n";
    }
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

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::AddWindowLevelPresetFromString(const char *preset)
{
  // the string is double|double
  double window = 0.0;
  double level = 0.0;

  if (preset == NULL)
    {
    vtkErrorMacro("AddWindowLevelPresetFromString: null input string!");
    return;
    }
  // parse the string
  std::string presetString = std::string(preset);
  char *presetChars = new char [presetString.size()+1];
  strcpy(presetChars, presetString.c_str());
  char *pos = strtok(presetChars, "|");
  if (pos != NULL)
    {
    window = atof(pos);
    }
  pos = strtok(NULL, "|");
  if (pos != NULL)
    {
    level = atof(pos);
    }
  delete[] presetChars;
  this->AddWindowLevelPreset(window, level);
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::AddWindowLevelPreset(double window, double level)
{
  vtkMRMLScalarVolumeDisplayNode::WindowLevelPreset preset;
  preset.Window = window;
  preset.Level = level;

  this->WindowLevelPresets.push_back(preset);
}

//---------------------------------------------------------------------------
int vtkMRMLScalarVolumeDisplayNode::GetNumberOfWindowLevelPresets()
{
  return this->WindowLevelPresets.size();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetWindowLevelFromPreset(int p)
{
  if (p < 0 || p >= this->GetNumberOfWindowLevelPresets())
    {
    vtkErrorMacro("SetWindowLevelFromPreset: index " << p << " out of range 0 to " << this->GetNumberOfWindowLevelPresets() - 1);
    return;
    }
  // TODO: change this flag to a regular int showing which preset is being used
  this->SetAutoWindowLevel(0);
  this->SetWindow(this->GetWindowPreset(p));
  this->SetLevel(this->GetLevelPreset(p));
}

//---------------------------------------------------------------------------
double vtkMRMLScalarVolumeDisplayNode::GetWindowPreset(int p)
{
  if (p < 0 || p >= this->GetNumberOfWindowLevelPresets())
    {
    vtkErrorMacro("GetWindowPreset: index " << p << " out of range 0 to " << this->GetNumberOfWindowLevelPresets() - 1);
    return 0.0;
    }

  return this->WindowLevelPresets[p].Window;
}

//---------------------------------------------------------------------------
double vtkMRMLScalarVolumeDisplayNode::GetLevelPreset(int p)
{
  if (p < 0 || p >= this->GetNumberOfWindowLevelPresets())
    {
    vtkErrorMacro("GetLevelPreset: index " << p << " out of range 0 to " << this->GetNumberOfWindowLevelPresets() - 1);
    return 0.0;
    }

  return this->WindowLevelPresets[p].Level;
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::ResetWindowLevelPresets()
{
  this->WindowLevelPresets.clear();
}
