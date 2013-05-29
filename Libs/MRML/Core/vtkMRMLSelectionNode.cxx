/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLSelectionNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLUnitNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkStdString.h>

// STD includes

vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, SecondaryVolumeID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveLabelVolumeID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveFiducialListID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveAnnotationID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveROIListID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveCameraID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveViewID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveLayoutID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveVolumeID);

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSelectionNode);

//----------------------------------------------------------------------------
vtkMRMLSelectionNode::vtkMRMLSelectionNode()
{
  this->SetSingletonTag("Singleton");

  this->ActiveVolumeID = NULL;
  this->SecondaryVolumeID = NULL;
  this->ActiveLabelVolumeID = NULL;
  this->ActiveFiducialListID = NULL;
  this->ActiveAnnotationID = NULL;
  this->ActiveROIListID  =NULL;
  this->ActiveCameraID = NULL;
  this->ActiveViewID = NULL;
  this->ActiveLayoutID = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLSelectionNode::~vtkMRMLSelectionNode()
{
  if (this->ActiveVolumeID)
    {
    delete [] this->ActiveVolumeID;
    this->ActiveVolumeID = NULL;
    }
  if (this->SecondaryVolumeID)
    {
    delete [] this->SecondaryVolumeID;
    this->SecondaryVolumeID = NULL;
    }
  if (this->ActiveLabelVolumeID)
    {
    delete [] this->ActiveLabelVolumeID;
    this->ActiveLabelVolumeID = NULL;
    }
  if (this->ActiveFiducialListID)
    {
    delete [] this->ActiveFiducialListID;
    this->ActiveFiducialListID = NULL;
    }
  if (this->ActiveAnnotationID)
    {
    delete [] this->ActiveAnnotationID;
    this->ActiveAnnotationID = NULL;
    }
  if (this->ActiveROIListID)
    {
    delete [] this->ActiveROIListID;
    this->ActiveROIListID = NULL;
    }
  if ( this->ActiveCameraID )
    {
    delete [] this->ActiveCameraID;
    this->ActiveCameraID = NULL;
    }
  if ( this->ActiveViewID)
    {
    delete []  this->ActiveViewID;
    this->ActiveViewID = NULL;
    }
  if ( this->ActiveLayoutID)
    {
    delete [] this->ActiveLayoutID;
    this->ActiveLayoutID = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " activeVolumeID=\"" << (this->ActiveVolumeID ? this->ActiveVolumeID : "NULL") << "\"";
  of << indent << " secondaryVolumeID=\"" << (this->SecondaryVolumeID ? this->SecondaryVolumeID : "NULL") << "\"";
  of << indent << " activeLabelVolumeID=\"" << (this->ActiveLabelVolumeID ? this->ActiveLabelVolumeID : "NULL") << "\"";
  of << indent << " activeFiducialListID=\"" << (this->ActiveFiducialListID ? this->ActiveFiducialListID : "NULL") << "\"";
  of << indent << " activeAnnotationID=\"" << (this->ActiveAnnotationID ? this->ActiveAnnotationID : "NULL") << "\"";
  of << indent << " activeROIListID=\"" << (this->ActiveROIListID ? this->ActiveROIListID : "NULL") << "\"";
  of << indent << " activeCameraID=\"" << (this->ActiveCameraID ? this->ActiveCameraID : "NULL") << "\"";
  of << indent << " activeViewID=\"" << (this->ActiveViewID ? this->ActiveViewID : "NULL") << "\"";
  of << indent << " activeLayoutID=\"" << (this->ActiveLayoutID ? this->ActiveLayoutID : "NULL") << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  this->Scene->AddReferencedNodeID(this->ActiveVolumeID, this);
  this->Scene->AddReferencedNodeID(this->ActiveLabelVolumeID, this);
  this->Scene->AddReferencedNodeID(this->ActiveFiducialListID, this);
  this->Scene->AddReferencedNodeID(this->ActiveCameraID, this);
  this->Scene->AddReferencedNodeID(this->ActiveViewID, this);
  this->Scene->AddReferencedNodeID(this->ActiveLayoutID, this);
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->ActiveVolumeID && !strcmp(oldID, this->ActiveVolumeID))
    {
    this->SetActiveVolumeID(newID);
    }
  if (this->SecondaryVolumeID && !strcmp(oldID, this->SecondaryVolumeID))
    {
    this->SetSecondaryVolumeID(newID);
    }
  if (this->ActiveLabelVolumeID && !strcmp(oldID, this->ActiveLabelVolumeID))
    {
    this->SetActiveLabelVolumeID(newID);
    }
  if (this->ActiveFiducialListID && !strcmp(oldID, this->ActiveFiducialListID))
    {
    this->SetActiveFiducialListID(newID);
    }
  if (this->ActiveAnnotationID && !strcmp(oldID, this->ActiveAnnotationID))
    {
    this->SetActiveAnnotationID(newID);
    }
  if ( this->ActiveCameraID && !strcmp (oldID, this->ActiveCameraID ))
    {
    this->SetActiveCameraID (newID);
    }
  if ( this->ActiveViewID && !strcmp ( oldID, this->ActiveViewID ))
    {
    this->SetActiveViewID (newID );
    }
  if ( this->ActiveLayoutID && !strcmp ( oldID, this->ActiveLayoutID ))
    {
    this->SetActiveLayoutID (newID );
    }

}

//-----------------------------------------------------------
void vtkMRMLSelectionNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->ActiveVolumeID != NULL && this->Scene->GetNodeByID(this->ActiveVolumeID) == NULL)
    {
    this->SetActiveVolumeID(NULL);
    }
  if (this->SecondaryVolumeID != NULL && this->Scene->GetNodeByID(this->SecondaryVolumeID) == NULL)
    {
    this->SetSecondaryVolumeID(NULL);
    }
  if (this->ActiveLabelVolumeID != NULL && this->Scene->GetNodeByID(this->ActiveLabelVolumeID) == NULL)
    {
    this->SetActiveLabelVolumeID(NULL);
    }
  if (this->ActiveFiducialListID != NULL && this->Scene->GetNodeByID(this->ActiveFiducialListID) == NULL)
    {
    this->SetActiveFiducialListID(NULL);
    }
  if (this->ActiveAnnotationID != NULL && this->Scene->GetNodeByID(this->ActiveAnnotationID) == NULL)
    {
    this->SetActiveAnnotationID(NULL);
    }
  if (this->ActiveViewID != NULL && this->Scene->GetNodeByID(this->ActiveViewID) == NULL)
    {
    this->SetActiveViewID(NULL);
    }
  if (this->ActiveLayoutID != NULL && this->Scene->GetNodeByID(this->ActiveLayoutID) == NULL)
    {
    this->SetActiveLayoutID(NULL);
    }
  if (this->ActiveCameraID != NULL && this->Scene->GetNodeByID(this->ActiveCameraID) == NULL)
    {
    this->SetActiveCameraID(NULL);
    }

}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "activeVolumeID")) 
      {
      this->SetActiveVolumeID(attValue);
      //this->Scene->AddReferencedNodeID(this->ActiveVolumeID, this);
      }
    if (!strcmp(attName, "secondaryVolumeID")) 
      {
      this->SetSecondaryVolumeID(attValue);
      //this->Scene->AddReferencedNodeID(this->ActiveVolumeID, this);
      }
    if (!strcmp(attName, "activeLabelVolumeID")) 
      {
      this->SetActiveLabelVolumeID(attValue);
      //this->Scene->AddReferencedNodeID(this->ActiveLabelVolumeID, this);
      }
    if (!strcmp(attName, "activeFiducialListID")) 
      {
      this->SetActiveFiducialListID(attValue);
      //this->Scene->AddReferencedNodeID(this->ActiveFiducialListID, this);
      }
    if (!strcmp(attName, "activeAnnotationID"))
      {
      this->SetActiveAnnotationID(attValue);
      //this->Scene->AddReferencedNodeID(this->ActiveFiducialListID, this);
      }
    if (!strcmp (attName, "activeCameraID"))
      {
      this->SetActiveCameraID (attValue );
      //this->Scene->AddReferencedNodeID (this->ActiveCameraID, this);
      }
    if (!strcmp (attName, "activeViewID"))
      {
      this->SetActiveViewID (attValue);
      //this->Scene->AddReferencedNodeID ( this->ActiveViewID, this);
      }
    if (!strcmp (attName, "activeLayoutID"))
      {
      this->SetActiveLayoutID (attValue);
      //this->Scene->AddReferencedNodeID ( this->ActiveLayoutID, this);
      }

    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLSelectionNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLSelectionNode *node = vtkMRMLSelectionNode::SafeDownCast(anode);

  this->SetActiveVolumeID(node->GetActiveVolumeID());
  this->SetSecondaryVolumeID(node->GetActiveVolumeID());
  this->SetActiveLabelVolumeID(node->GetActiveLabelVolumeID());
  this->SetActiveFiducialListID(node->GetActiveFiducialListID());
  this->SetActiveAnnotationID(node->GetActiveAnnotationID());
  this->SetActiveCameraID (node->GetActiveCameraID());
  this->SetActiveViewID (node->GetActiveViewID() );
  this->SetActiveLayoutID (node->GetActiveLayoutID() );

  this->EndModify(disabledModify);
  
  }

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << "ActiveVolumeID: " << ( (this->ActiveVolumeID) ? this->ActiveVolumeID : "None" ) << "\n";
  os << "SecondaryVolumeID: " << ( (this->SecondaryVolumeID) ? this->SecondaryVolumeID : "None" ) << "\n";
  os << "ActiveLabelVolumeID: " << ( (this->ActiveLabelVolumeID) ? this->ActiveLabelVolumeID : "None" ) << "\n";
  os << "ActiveFiducialListID: " << ( (this->ActiveFiducialListID) ? this->ActiveFiducialListID : "None" ) << "\n";
  os << "ActiveAnnotationID: " << ( (this->ActiveAnnotationID) ? this->ActiveAnnotationID : "None" ) << "\n";
  if (this->AnnotationIDList.size() > 0)
    {
    os << "Valid Annotation IDs: \n";
    for (unsigned int i = 0; i < this->AnnotationIDList.size(); ++i)
      {
      os << indent.GetNextIndent() << i << ": " << this->AnnotationIDList[i]<< "\n";
      }
    }
  if (this->AnnotationResourceList.size() > 0)
    {
    os << "Annotation Resources: \n";
    for (unsigned int i = 0; i < this->AnnotationResourceList.size(); ++i)
      {
      os << indent.GetNextIndent() << i << ": " << this->AnnotationResourceList[i] << "\n";
      }
    }
  os << "ActiveCameraID: " << ( (this->ActiveCameraID) ? this->ActiveCameraID : "None" ) << "\n";
  os << "ActiveViewID: " << ( (this->ActiveViewID) ? this->ActiveViewID : "None" ) << "\n";
  os << "ActiveLayoutID: " << ( (this->ActiveLayoutID) ? this->ActiveLayoutID : "None" ) << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::AddNewAnnotationIDToList(const char *newID, const char *resource)
{
  if (newID == NULL)
    {
    return;
    }
  vtkDebugMacro("AddNewAnnotationIDToList: newID = " << newID);

  std::string idString = std::string(newID);
  std::string resourceString;
  if (resource)
    {
    resourceString = std::string(resource);
    }
  int index = this->AnnotationIDInList(idString);
  if (index == -1)
    {
    vtkDebugMacro("Annotation id " << idString << " not in list, adding it and invoking annotation id list modified event");
    this->AnnotationIDList.push_back(idString);
    this->AnnotationResourceList.push_back(resourceString);
    this->InvokeEvent(vtkMRMLSelectionNode::AnnotationIDListModifiedEvent);
    }
  else
    {
    // check if the resource needs to be updated
    if (resourceString.compare(this->GetAnnotationResourceByIndex(index)) != 0)
      {
      vtkDebugMacro("Updating resource for id " << idString << ", at index " << index << " to " << resourceString);
      this->AnnotationResourceList[index] = resourceString;
      this->InvokeEvent(vtkMRMLSelectionNode::AnnotationIDListModifiedEvent);
      }
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::RemoveAnnotationIDFromList(const char *id)
{
  if (id == NULL)
    {
    return;
    }
  vtkDebugMacro("RemoveAnnotationIDFromList: id = " << id);

  std::string idString = std::string(id);

  int index = this->AnnotationIDInList(idString);
  if (index == -1)
    {
    return;
    }
  vtkDebugMacro("Removing annotation id " << id << ", found at index " << index);
  // erase the id and resource
  this->AnnotationIDList.erase(this->AnnotationIDList.begin()+index);
  this->AnnotationResourceList.erase(this->AnnotationResourceList.begin()+index);

  // was it the active one?
  if (this->GetActiveAnnotationID() &&
      idString.compare(this->GetActiveAnnotationID()) == 0)
    {
    // make it inactive
    this->SetActiveAnnotationID(NULL);
    }
  this->InvokeEvent(vtkMRMLSelectionNode::AnnotationIDListModifiedEvent);
  this->Modified();
}

//----------------------------------------------------------------------------
std::string vtkMRMLSelectionNode::GetAnnotationIDByIndex(int n)
{
  std::string id;
  if (this->AnnotationIDList.size() > (unsigned int)n && n >= 0)
    {
    id = this->AnnotationIDList[n];
    }
  else
    {
    vtkWarningMacro("GetAnnotationIDByIndex: index " << n << " is out of bounds of 0-" << this->AnnotationIDList.size());
    }
  return id;
}

//----------------------------------------------------------------------------
std::string vtkMRMLSelectionNode::GetAnnotationResourceByIndex(int n)
{
  std::string resource;
  if (this->AnnotationResourceList.size() > (unsigned int)n && n >= 0)
    {
    resource = this->AnnotationResourceList[n];
    }
  else
    {
    vtkWarningMacro("GetAnnotationResourceByIndex: index " << n << " is out of bounds of 0-" << this->AnnotationResourceList.size());
    }
  return resource;
}

//----------------------------------------------------------------------------
int vtkMRMLSelectionNode::AnnotationIDInList(std::string id)
{
  for (unsigned int i = 0; i < this->AnnotationIDList.size(); ++i)
    {
    if (this->AnnotationIDList[i].compare(id) == 0)
      {
      return i;
      }
    }
  return -1;
}

//----------------------------------------------------------------------------
std::string vtkMRMLSelectionNode::GetAnnotationResourceByID(std::string id)
{
  std::string resource;

  int annotationIndex = this->AnnotationIDInList(id);
  if (annotationIndex != -1)
    {
    resource = this->GetAnnotationResourceByIndex(annotationIndex);
    }
  return resource;
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetReferenceActiveAnnotationID (const char *id)
{
  this->SetActiveAnnotationID(id);
  this->InvokeEvent(vtkMRMLSelectionNode::ActiveAnnotationIDChangedEvent);
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::GetUnitNodes(std::vector<vtkMRMLUnitNode*>& units)
{
  std::string unit = "Unit/";
  for (NodeReferencesType::const_iterator it = this->NodeReferences.begin();
    it != this->NodeReferences.end(); ++it)
    {
    if (it->first.compare(0, unit.size(), unit) == 0)
      {
      // there is only one referenced node per reference role
      units.push_back(
        vtkMRMLUnitNode::SafeDownCast(it->second[0]->ReferencedNode));
      }
    }
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetUnitNodeID(const char* quantity)
{
  std::string referenceRole = "Unit/";
  referenceRole += quantity ? quantity : "";
  return this->GetNodeReferenceID(referenceRole.c_str());
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetUnitNodeID(const char* quantity, const char* id)
{
  std::string safeQuantity = quantity ? quantity : "";
  std::string referenceRole = "Unit/" + safeQuantity;

  unsigned long mTime = this->GetMTime();
  this->SetAndObserveNodeReferenceID(referenceRole.c_str(), id);

  if (this->GetMTime() > mTime)
    {
    // Node changed, propaged unit modified event
    this->InvokeEvent(vtkMRMLSelectionNode::UnitModifiedEvent, &safeQuantity);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSelectionNode::ProcessMRMLEvents(vtkObject *caller,
                                             unsigned long event,
                                             void *callData)
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLUnitNode* unitNode = vtkMRMLUnitNode::SafeDownCast(caller);
  if (unitNode && event == vtkCommand::ModifiedEvent)
    {
    std::string quantity =
      unitNode->GetQuantity() ? unitNode->GetQuantity() : "";
    this->InvokeEvent(
      vtkMRMLSelectionNode::UnitModifiedEvent, &quantity);
    }
}
