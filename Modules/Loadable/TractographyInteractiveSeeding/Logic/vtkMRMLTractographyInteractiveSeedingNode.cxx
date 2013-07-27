/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTractographyInteractiveSeedingNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRMLTractographyInteractiveSeeding includes
#include "vtkMRMLTractographyInteractiveSeedingNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTractographyInteractiveSeedingNode);

//----------------------------------------------------------------------------
vtkMRMLTractographyInteractiveSeedingNode::vtkMRMLTractographyInteractiveSeedingNode()
{
   this->StoppingMode = 0;
   this->StoppingValue = 0.25;
   this->StoppingCurvature = 0.7;
   this->IntegrationStep = 0.5;
   this->SeedingRegionSize = 2.5;
   this->SeedingRegionStep = 1.0;
   this->MinimumPathLength = 20.0;
   this->MaximumPathLength = 800.0;
   this->MaxNumberOfSeeds = 100;
   this->SeedSelectedFiducials = 0;
   this->ROILabels = 0;
   this->RandomGrid = 0;
   this->UseIndexSpace = 0;
   this->LinearMeasureStart = 0.3;
   this->SeedSpacing = 2.0;
   this->DisplayMode = 1;
   this->EnableSeeding = 1;
  
   this->FilePrefix = NULL;
   this->FileDirectoryName = NULL;
   this->WriteToFile = 0;

   this->InputVolumeRef = NULL;
   this->InputFiducialRef = NULL;
   this->OutputFiberRef = NULL;
   this->HideFromEditors = true;
}

//----------------------------------------------------------------------------
vtkMRMLTractographyInteractiveSeedingNode::~vtkMRMLTractographyInteractiveSeedingNode()
{
  if (ROILabels)
    {
    this->ROILabels->Delete();
    }
  this->SetInputVolumeRef( NULL );
  this->SetInputFiducialRef( NULL );
  this->SetOutputFiberRef( NULL );
}

//----------------------------------------------------------------------------
void vtkMRMLTractographyInteractiveSeedingNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream

  vtkIndent indent(nIndent);

  {
    std::stringstream ss;
    ss << this->StoppingValue;
    of << indent << " StoppingValue=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->StoppingMode;
    of << indent << " StoppingMode=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->StoppingCurvature;
    of << indent << " StoppingCurvature=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->IntegrationStep;
    of << indent << " IntegrationStep=\"" << ss.str() << "\"";
  }
  { 
    std::stringstream ss;
    ss << this->MaximumPathLength;
    of << indent << " MaximumPathLength=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->MinimumPathLength;
    of << indent << " MinimumPathLength=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->SeedingRegionSize;
    of << indent << " SeedingRegionSize=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->SeedingRegionStep;
    of << indent << " SeedingRegionStep=\"" << ss.str() << "\"";
  }  
  {
    std::stringstream ss;
    ss << this->MaxNumberOfSeeds;
    of << indent << " MaxNumberOfSeeds=\"" << ss.str() << "\"";
  }
    
  of << indent << " seedSelectedFiducials=\"" << (this->SeedSelectedFiducials ? "true" : "false") << "\"";
  
  {
    std::stringstream ss;
    ss << this->DisplayMode;
    of << indent << " displayMode=\"" << ss.str() << "\"";
  }

  {
    std::stringstream ss;
    ss << this->ROILabelsToString();
    of << indent << " ROILabel=\"" << ss.str() << "\"";
  }
    
  of << indent << " randomGrid=\"" << (this->RandomGrid ? "true" : "false") << "\"";

  of << indent << " useIndexSpace=\"" << (this->UseIndexSpace ? "true" : "false") << "\"";

  {
    std::stringstream ss;
    ss << this->LinearMeasureStart;
    of << indent << " linearMeasureStart=\"" << ss.str() << "\"";
  }

  {
    std::stringstream ss;
    ss << this->SeedSpacing;
    of << indent << " seedSpacing=\"" << ss.str() << "\"";
  }

  of << indent << " enableSeeding=\"" << (this->EnableSeeding ? "true" : "false") << "\"";

  of << indent << " writeToFile=\"" << (this->WriteToFile ? "true" : "false") << "\"";

  {
    std::stringstream ss;
    if ( this->InputVolumeRef )
      {
      ss << this->InputVolumeRef;
      of << indent << " InputVolumeRef=\"" << ss.str() << "\"";
     }
  }
  {
    std::stringstream ss;
    if ( this->FileDirectoryName )
      {
      ss << this->FileDirectoryName;
      of << indent << " fileDirectoryName=\"" << ss.str() << "\"";
     }
  }
  {
    std::stringstream ss;
    if ( this->FilePrefix )
      {
      ss << this->FilePrefix;
      of << indent << " filePrefix=\"" << ss.str() << "\"";
     }
  }
  {
    std::stringstream ss;
    if ( this->InputFiducialRef )
      {
      ss << this->InputFiducialRef;
      of << indent << " InputFiducialRef=\"" << ss.str() << "\"";
     }
  }  
  {
    std::stringstream ss;
    if ( this->OutputFiberRef )
      {
      ss << this->OutputFiberRef;
      of << indent << " OutputFiberRef=\"" << ss.str() << "\"";
      }
  }
}

//----------------------------------------------------------------------------
void vtkMRMLTractographyInteractiveSeedingNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "StoppingValue")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->StoppingValue;
      }
    else if (!strcmp(attName, "StoppingMode")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->StoppingMode;
      }
    else if (!strcmp(attName, "StoppingCurvature")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->StoppingCurvature;
      }
    else if (!strcmp(attName, "IntegrationStep")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->IntegrationStep;
      }
    else if (!strcmp(attName, "MinimumPathLength")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MinimumPathLength;
      }
    else if (!strcmp(attName, "MaximumPathLength")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MaximumPathLength;
      }
    else if (!strcmp(attName, "SeedingRegionSize")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SeedingRegionSize;
      }
    else if (!strcmp(attName, "SeedingRegionStep")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SeedingRegionStep;
      }
    else if (!strcmp(attName, "MaxNumberOfSeeds")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MaxNumberOfSeeds;
      }
    else if (!strcmp(attName, "seedSelectedFiducials")) 
      {
     if (!strcmp(attValue,"true")) 
        {
        this->SeedSelectedFiducials = 1;
        }
      else
        {
        this->SeedSelectedFiducials = 0;
        }
      }
    else if (!strcmp(attName, "ROILabel")) 
      {
      std::stringstream ss;
      ss << attValue;
      this->StringToROILabels(ss.str());
      }
    else if (!strcmp(attName, "randomGrid")) 
      {
     if (!strcmp(attValue,"true")) 
        {
        this->RandomGrid = 1;
        }
      else
        {
        this->RandomGrid = 0;
        }
      }
    else if (!strcmp(attName, "useIndexSpace")) 
      {
     if (!strcmp(attValue,"true")) 
        {
        this->UseIndexSpace = 1;
        }
      else
        {
        this->UseIndexSpace = 0;
        }
      }
    else if (!strcmp(attName, "linearMeasureStart")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LinearMeasureStart;
      }
    else if (!strcmp(attName, "seedSpacing")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SeedSpacing;
      }
    else if (!strcmp(attName, "enableSeeding")) 
      {
     if (!strcmp(attValue,"true")) 
        {
        this->EnableSeeding = 1;
        }
      else
        {
        this->EnableSeeding = 0;
        }
      }
    else if (!strcmp(attName, "writeToFile")) 
      {
     if (!strcmp(attValue,"true")) 
        {
        this->WriteToFile = 1;
        }
      else
        {
        this->WriteToFile = 0;
        }
      }
    else if (!strcmp(attName, "DisplayMode")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->DisplayMode;
      }

    else if (!strcmp(attName, "fileDirectoryName"))
      {
      this->SetFileDirectoryName(attValue);
      }
    else if (!strcmp(attName, "filePrefix"))
      {
      this->SetFilePrefix(attValue);
      }
    else if (!strcmp(attName, "InputVolumeRef"))
      {
      this->SetInputVolumeRef(attValue);
      this->Scene->AddReferencedNodeID(this->InputVolumeRef, this);
      }
    else if (!strcmp(attName, "InputFiducialRef"))
      {
      this->SetInputFiducialRef(attValue);
      this->Scene->AddReferencedNodeID(this->InputFiducialRef, this);
      }    
    else if (!strcmp(attName, "OutputFiberRef"))
      {
      this->SetOutputFiberRef(attValue);
      this->Scene->AddReferencedNodeID(this->OutputFiberRef, this);
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLTractographyInteractiveSeedingNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  this->DisableModifiedEventOn();

  vtkMRMLTractographyInteractiveSeedingNode *node = (vtkMRMLTractographyInteractiveSeedingNode *) anode;

  this->SetStoppingValue(node->StoppingValue);
  this->SetStoppingMode(node->StoppingMode);
  this->SetStoppingCurvature(node->StoppingCurvature);
  this->SetIntegrationStep(node->IntegrationStep);
  this->SetMinimumPathLength(node->MinimumPathLength);
  this->SetMaximumPathLength(node->MaximumPathLength);
  this->SetSeedingRegionSize(node->SeedingRegionSize);
  this->SetSeedingRegionStep(node->SeedingRegionStep);
  this->SetMaxNumberOfSeeds(node->MaxNumberOfSeeds);
  this->SetSeedSelectedFiducials(node->SeedSelectedFiducials);
  this->SetROILabels(node->ROILabels);
  this->SetRandomGrid(node->RandomGrid);
  this->SetUseIndexSpace(node->UseIndexSpace);
  this->SetLinearMeasureStart(node->LinearMeasureStart);
  this->SetSeedSpacing(node->SeedSpacing);
  this->SetDisplayMode(node->DisplayMode);
  this->SetEnableSeeding(node->EnableSeeding);
  this->SetWriteToFile(node->WriteToFile);
  this->SetFileDirectoryName(node->FileDirectoryName);
  this->SetFilePrefix(node->FilePrefix);
  this->SetInputVolumeRef(node->InputVolumeRef);
  this->SetInputFiducialRef(node->InputFiducialRef);
  this->SetOutputFiberRef(node->OutputFiberRef);

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();

}

//----------------------------------------------------------------------------
void vtkMRMLTractographyInteractiveSeedingNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "StoppingValue:   " << this->StoppingValue << "\n";
  os << indent << "StoppingMode:   " << this->StoppingMode << "\n";
  os << indent << "StoppingCurvature:   " << this->StoppingCurvature << "\n";
  os << indent << "IntegrationStep:   " << this->IntegrationStep << "\n";
  os << indent << "MinimumPathLength:   " << this->MinimumPathLength << "\n";
  os << indent << "MaximumPathLength:   " << this->MaximumPathLength << "\n";
  os << indent << "SeedingRegionSize:   " << this->SeedingRegionSize << "\n";
  os << indent << "SeedingRegionStep:   " << this->SeedingRegionStep << "\n";
  os << indent << "MaxNumberOfSeeds:   " << this->MaxNumberOfSeeds << "\n";
  os << indent << "SeedSelectedFiducials:   " << this->SeedSelectedFiducials << "\n";
  os << indent << "ROILabels:   " << this->ROILabelsToString() << "\n";
  os << indent << "RandomGrid:   " << this->RandomGrid << "\n";
  os << indent << "UseIndexSpace:   " << this->UseIndexSpace << "\n";
  os << indent << "LinearMeasureStart:   " << this->LinearMeasureStart << "\n";
  os << indent << "SeedSpacing:   " << this->SeedSpacing << "\n";
  os << indent << "DisplayMode:   " << this->DisplayMode << "\n";
  os << indent << "EnableSeeding:   " << this->EnableSeeding << "\n";
  os << indent << "WriteToFile:   " << this->WriteToFile << "\n";

  os << indent << "FileDirectoryName:   " << 
   (this->FileDirectoryName ? this->FileDirectoryName : "(none)") << "\n";
  os << indent << "FilePrefix:   " << 
   (this->FilePrefix ? this->FilePrefix : "(none)") << "\n";

  os << indent << "InputVolumeRef:   " << 
   (this->InputVolumeRef ? this->InputVolumeRef : "(none)") << "\n";
  os << indent << "InputFiducialRef:   " << 
   (this->InputFiducialRef ? this->InputFiducialRef : "(none)") << "\n";  
  os << indent << "OutputFiberRef:   " << 
   (this->OutputFiberRef ? this->OutputFiberRef : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLTractographyInteractiveSeedingNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
  if (this->InputVolumeRef && !strcmp(oldID, this->InputVolumeRef))
    {
    this->SetInputVolumeRef(newID);
    }
  if (this->InputFiducialRef && !strcmp(oldID, this->InputFiducialRef))
    {
    this->SetInputFiducialRef(newID);
    }
  if (this->OutputFiberRef && !strcmp(oldID, this->OutputFiberRef))
    {
    this->SetOutputFiberRef(newID);
    }
}

void vtkMRMLTractographyInteractiveSeedingNode::StringToROILabels(std::string labels)
{
  if (ROILabels)
    {
    this->ROILabels->Delete();
    }
  this->ROILabels = vtkIntArray::New();

  // get lables range in the volume
  vtkMRMLScalarVolumeNode *labelsVolume = NULL;
  double range[2];
  range[0]=-1;
  range[1]=1e10;
  if (this->GetScene())
    {
    labelsVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->GetScene()->GetNodeByID(this->GetInputFiducialRef()) );
    if (labelsVolume && labelsVolume->GetImageData())
      {
      labelsVolume->GetImageData()->GetScalarRange(range);
      }
    }

  // parse the string
  std::stringstream ss(labels);
  int label;
  bool validLabels = false;
  while ( !ss.eof() )
    {
    int c =  ss.peek() ;
    if ( c < '0' || c > '9' )
      {
      validLabels = false;
      ss.ignore(1);
      continue;
      }
    if (ss >> label)
      {
      // check range
      if (range[0] <= label && label <= range[1])
        {
        this->ROILabels->InsertNextValue(label);
        validLabels = true;
        }
      }
    } //while ( !ss.eof() )
  if (validLabels)
    {
    this->Modified();
    }
}

std::string vtkMRMLTractographyInteractiveSeedingNode::ROILabelsToString()
{
  if (!this->ROILabels)
    {
      return std::string();
    }
  std::stringstream ss;
  for (int i=0; i<this->ROILabels->GetNumberOfTuples(); i++)
    {
    if (i)
      {
      ss << ",";
      }
    ss << this->ROILabels->GetValue(i);
    }
  return ss.str();
}
