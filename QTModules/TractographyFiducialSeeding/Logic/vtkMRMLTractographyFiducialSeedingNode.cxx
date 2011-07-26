/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTractographyFiducialSeedingNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLTractographyFiducialSeedingNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLTractographyFiducialSeedingNode* vtkMRMLTractographyFiducialSeedingNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLTractographyFiducialSeedingNode");
  if(ret)
    {
      return (vtkMRMLTractographyFiducialSeedingNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTractographyFiducialSeedingNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLTractographyFiducialSeedingNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLTractographyFiducialSeedingNode");
  if(ret)
    {
      return (vtkMRMLTractographyFiducialSeedingNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTractographyFiducialSeedingNode;
}

//----------------------------------------------------------------------------
vtkMRMLTractographyFiducialSeedingNode::vtkMRMLTractographyFiducialSeedingNode()
{
   this->StoppingMode = 0;
   this->StoppingValue = 0.25;
   this->StoppingCurvature = 0.7;
   this->IntegrationStep = 0.5;
   this->SeedingRegionSize = 2.5;
   this->SeedingRegionStep = 1.0;
   this->MinimumPathLength = 20.0;
   this->MaxNumberOfSeeds = 100;
   this->SeedSelectedFiducials = 0;
   this->DisplayMode = 1;
   this->EnableSeeding = 1;

   this->InputVolumeRef = NULL;
   this->InputFiducialRef = NULL;
   this->OutputFiberRef = NULL;
   this->HideFromEditors = true;
}

//----------------------------------------------------------------------------
vtkMRMLTractographyFiducialSeedingNode::~vtkMRMLTractographyFiducialSeedingNode()
{
   this->SetInputVolumeRef( NULL );
   this->SetInputFiducialRef( NULL );
   this->SetOutputFiberRef( NULL );
}

//----------------------------------------------------------------------------
void vtkMRMLTractographyFiducialSeedingNode::WriteXML(ostream& of, int nIndent)
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

  of << indent << " enableSeeding=\"" << (this->EnableSeeding ? "true" : "false") << "\"";

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
void vtkMRMLTractographyFiducialSeedingNode::ReadXMLAttributes(const char** atts)
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
    else if (!strcmp(attName, "DisplayMode")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->DisplayMode;
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
void vtkMRMLTractographyFiducialSeedingNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  this->DisableModifiedEventOn();

  vtkMRMLTractographyFiducialSeedingNode *node = (vtkMRMLTractographyFiducialSeedingNode *) anode;

  this->SetStoppingValue(node->StoppingValue);
  this->SetStoppingMode(node->StoppingMode);
  this->SetStoppingCurvature(node->StoppingCurvature);
  this->SetIntegrationStep(node->IntegrationStep);
  this->SetMinimumPathLength(node->MinimumPathLength);
  this->SetSeedingRegionSize(node->SeedingRegionSize);
  this->SetSeedingRegionStep(node->SeedingRegionStep);
  this->SetMaxNumberOfSeeds(node->MaxNumberOfSeeds);
  this->SetSeedSelectedFiducials(node->SeedSelectedFiducials);
  this->SetDisplayMode(node->DisplayMode);
  this->SetEnableSeeding(node->EnableSeeding);
  this->SetInputVolumeRef(node->InputVolumeRef);
  this->SetInputFiducialRef(node->InputFiducialRef);
  this->SetOutputFiberRef(node->OutputFiberRef);

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();

}

//----------------------------------------------------------------------------
void vtkMRMLTractographyFiducialSeedingNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "StoppingValue:   " << this->StoppingValue << "\n";
  os << indent << "StoppingMode:   " << this->StoppingMode << "\n";
  os << indent << "StoppingCurvature:   " << this->StoppingCurvature << "\n";
  os << indent << "IntegrationStep:   " << this->IntegrationStep << "\n";
  os << indent << "MinimumPathLength:   " << this->MinimumPathLength << "\n";
  os << indent << "SeedingRegionSize:   " << this->SeedingRegionSize << "\n";
  os << indent << "SeedingRegionStep:   " << this->SeedingRegionStep << "\n";
  os << indent << "MaxNumberOfSeeds:   " << this->MaxNumberOfSeeds << "\n";
  os << indent << "SeedSelectedFiducials:   " << this->SeedSelectedFiducials << "\n";
  os << indent << "DisplayMode:   " << this->DisplayMode << "\n";
  os << indent << "EnableSeeding:   " << this->EnableSeeding << "\n";
  os << indent << "InputVolumeRef:   " << 
   (this->InputVolumeRef ? this->InputVolumeRef : "(none)") << "\n";
  os << indent << "InputFiducialRef:   " << 
   (this->InputFiducialRef ? this->InputFiducialRef : "(none)") << "\n";  
  os << indent << "OutputFiberRef:   " << 
   (this->OutputFiberRef ? this->OutputFiberRef : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLTractographyFiducialSeedingNode::UpdateReferenceID(const char *oldID, const char *newID)
{
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
