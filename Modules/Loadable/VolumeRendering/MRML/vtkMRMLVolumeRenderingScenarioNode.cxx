/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeRenderingScenarioNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLVolumeRenderingScenarioNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

// STD includes

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVolumeRenderingScenarioNode);

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingScenarioNode::vtkMRMLVolumeRenderingScenarioNode()
{
  this->ParametersNodeID = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingScenarioNode::~vtkMRMLVolumeRenderingScenarioNode()
{
  if (this->ParametersNodeID)
    delete [] this->ParametersNodeID;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingScenarioNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
  {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "parametersNodeID"))
    {
      this->SetParametersNodeID(attValue);
      continue;
    }
  }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingScenarioNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " parametersNodeID=\"" << (this->ParametersNodeID ? this->ParametersNodeID : "NULL") << "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingScenarioNode::UpdateReferenceID(const char *oldID,
                                                           const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
}

//-----------------------------------------------------------
void vtkMRMLVolumeRenderingScenarioNode::UpdateReferences()
{
  this->Superclass::UpdateReferences();
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLVolumeRenderingScenarioNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify(); 

  Superclass::Copy(anode);
  vtkMRMLVolumeRenderingScenarioNode *node = vtkMRMLVolumeRenderingScenarioNode::SafeDownCast(anode);

  this->SetParametersNodeID(node->GetParametersNodeID());

  this->EndModify(disabledModify);
}

//-----------------------------------------------------------
void vtkMRMLVolumeRenderingScenarioNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingScenarioNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData )
{
    Superclass::ProcessMRMLEvents(caller, event, callData);
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    return;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingScenarioNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << "ParametersNodeID: " << ( (this->ParametersNodeID) ? this->ParametersNodeID : "None" ) << "\n";
}

// End
