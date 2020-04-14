/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLSelectionNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/
#include <sstream>

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLUnitNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkStdString.h>

static const char* UNIT_NODE_REFERENCE_ROLE = "unit/";
static const char* ACTIVE_VOLUME_REFERENCE_ROLE = "ActiveVolume";
static const char* SECONDARY_VOLUME_REFERENCE_ROLE = "SecondaryVolume";
static const char* ACTIVE_LABEL_VOLUME_REFERENCE_ROLE = "ActiveLabelVolume";
static const char* ACTIVE_FIDUCIAL_LIST_REFERENCE_ROLE = "ActiveFiducialList";
static const char* ACTIVE_PLACE_NODE_REFERENCE_ROLE = "ActivePlaceNode";
static const char* ACTIVE_ROI_LIST_REFERENCE_ROLE = "ActiveROIList";
static const char* ACTIVE_CAMERA_REFERENCE_ROLE = "ActiveCamera";
static const char* ACTIVE_TABLE_REFERENCE_ROLE = "ActiveTable";
static const char* ACTIVE_VIEW_REFERENCE_ROLE = "ActiveView";
static const char* ACTIVE_LAYOUT_REFERENCE_ROLE = "ActiveLayout";
static const char* ACTIVE_PLOT_CHART_REFERENCE_ROLE = "ActivePlotChart";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSelectionNode);

//----------------------------------------------------------------------------
vtkMRMLSelectionNode::vtkMRMLSelectionNode()
{
  this->HideFromEditors = 1;

  this->SetSingletonTag("Singleton");
  this->ActivePlaceNodeClassName = nullptr;

  this->AddNodeReferenceRole(UNIT_NODE_REFERENCE_ROLE, "UnitNodeRef");
  this->AddNodeReferenceRole(ACTIVE_VOLUME_REFERENCE_ROLE, "activeVolumeID");
  this->AddNodeReferenceRole(SECONDARY_VOLUME_REFERENCE_ROLE, "secondaryVolumeID");
  this->AddNodeReferenceRole(ACTIVE_LABEL_VOLUME_REFERENCE_ROLE, "ActiveLabelVolumeID");
  this->AddNodeReferenceRole(ACTIVE_FIDUCIAL_LIST_REFERENCE_ROLE, "ActiveFiducialListID");
  this->AddNodeReferenceRole(ACTIVE_PLACE_NODE_REFERENCE_ROLE, "ActivePlaceNodeID");
  this->AddNodeReferenceRole(ACTIVE_ROI_LIST_REFERENCE_ROLE, "ActiveROIListID");
  this->AddNodeReferenceRole(ACTIVE_CAMERA_REFERENCE_ROLE, "ActiveCameraID");
  this->AddNodeReferenceRole(ACTIVE_TABLE_REFERENCE_ROLE, "ActiveTableID");
  this->AddNodeReferenceRole(ACTIVE_VIEW_REFERENCE_ROLE, "ActiveViewID");
  this->AddNodeReferenceRole(ACTIVE_LAYOUT_REFERENCE_ROLE, "ActiveLayoutID");
  this->AddNodeReferenceRole(ACTIVE_PLOT_CHART_REFERENCE_ROLE, "ActivePlotChartID");
}

//----------------------------------------------------------------------------
vtkMRMLSelectionNode::~vtkMRMLSelectionNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLStringMacro(activePlaceNodeClassName, ActivePlaceNodeClassName);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLStringMacro(activePlaceNodeClassName, ActivePlaceNodeClassName);
  vtkMRMLReadXMLEndMacro();

  // In legacy scenes, NULL value is written for empty/nullptr values - replace those now
  if (this->ActivePlaceNodeClassName && strcmp(this->ActivePlaceNodeClassName, "NULL") == 0)
    {
    SetActivePlaceNodeClassName("");
    }
  if (this->GetActiveVolumeID() && strcmp(this->GetActiveVolumeID(), "NULL") == 0)
    {
    this->SetActiveVolumeID(nullptr);
    }
  if (this->GetSecondaryVolumeID() && strcmp(this->GetSecondaryVolumeID(), "NULL") == 0)
    {
    this->SetSecondaryVolumeID(nullptr);
    }
  if (this->GetActiveLabelVolumeID() && strcmp(this->GetActiveLabelVolumeID(), "NULL") == 0)
    {
    this->SetActiveLabelVolumeID(nullptr);
    }
  if (this->GetActiveFiducialListID() && strcmp(this->GetActiveFiducialListID(), "NULL") == 0)
    {
    this->SetActiveFiducialListID(nullptr);
    }
  if (this->GetActivePlaceNodeID() && strcmp(this->GetActivePlaceNodeID(), "NULL") == 0)
    {
    this->SetActivePlaceNodeID(nullptr);
    }
  if (this->GetActiveROIListID() && strcmp(this->GetActiveROIListID(), "NULL") == 0)
    {
    this->SetActiveROIListID(nullptr);
    }
  if (this->GetActiveCameraID() && strcmp(this->GetActiveCameraID(), "NULL") == 0)
    {
    this->SetActiveCameraID(nullptr);
    }
  if (this->GetActiveTableID() && strcmp(this->GetActiveTableID(), "NULL") == 0)
    {
    this->SetActiveTableID(nullptr);
    }
  if (this->GetActiveViewID() && strcmp(this->GetActiveViewID(), "NULL") == 0)
    {
    this->SetActiveViewID(nullptr);
    }
  if (this->GetActiveLayoutID() && strcmp(this->GetActiveLayoutID(), "NULL") == 0)
    {
    this->SetActiveLayoutID(nullptr);
    }
  if (this->GetActivePlotChartID() && strcmp(this->GetActivePlotChartID(), "NULL") == 0)
    {
    this->SetActivePlotChartID(nullptr);
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

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyStringMacro(ActivePlaceNodeClassName);
  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintStringMacro(ActivePlaceNodeClassName);
  vtkMRMLPrintEndMacro();

  if (this->PlaceNodeClassNameList.size() > 0)
    {
    os << "Valid PlaceNode IDs: \n";
    for (unsigned int i = 0; i < this->PlaceNodeClassNameList.size(); ++i)
      {
      os << indent.GetNextIndent() << i << ": " << this->PlaceNodeClassNameList[i]<< "\n";
      }
    }
  if (this->PlaceNodeResourceList.size() > 0)
    {
    os << "PlaceNode Resources: \n";
    for (unsigned int i = 0; i < this->PlaceNodeResourceList.size(); ++i)
      {
      os << indent.GetNextIndent() << i << ": " << this->PlaceNodeResourceList[i] << "\n";
      }
    }
  if (this->PlaceNodeIconNameList.size() > 0)
    {
    os << "PlaceNode Icon Names: \n";
    for (unsigned int i = 0; i < this->PlaceNodeIconNameList.size(); ++i)
      {
      os << indent.GetNextIndent() << i << ": " << this->PlaceNodeIconNameList[i] << "\n";
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::AddNewPlaceNodeClassNameToList(const char *newClassName, const char *resource, const char *iconName)
{
  if (newClassName == nullptr)
    {
    return;
    }
  vtkDebugMacro("AddNewPlaceNodeClassNameToList: newClassName = " << newClassName);

  std::string classNameString = std::string(newClassName);
  std::string resourceString;
  if (resource)
    {
    resourceString = std::string(resource);
    }
  std::string iconNameString;
  if (iconName != nullptr)
    {
    iconNameString = std::string(iconName);
    }
  int index = this->PlaceNodeClassNameInList(classNameString);
  if (index == -1)
    {
    vtkDebugMacro("PlaceNode class name " << classNameString << " not in list, adding it and invoking placeNode class name list modified event");
    this->PlaceNodeClassNameList.push_back(classNameString);
    this->PlaceNodeIconNameList.push_back(iconNameString);
    this->PlaceNodeResourceList.push_back(resourceString);
    this->InvokeEvent(vtkMRMLSelectionNode::PlaceNodeClassNameListModifiedEvent);
    }
  else
    {
    // check if the resource needs to be updated
    if (resourceString.compare(this->GetPlaceNodeResourceByIndex(index)) != 0)
      {
      vtkDebugMacro("Updating resource for class name " << classNameString << ", at index " << index << " to " << resourceString);
      this->PlaceNodeResourceList[index] = resourceString;
      this->InvokeEvent(vtkMRMLSelectionNode::PlaceNodeClassNameListModifiedEvent);
      }
    // check if the icon name needs to be updated
    if (iconNameString.compare(this->GetPlaceNodeIconNameByIndex(index)) != 0)
      {
      vtkDebugMacro("Updating icon name for place node class name " << classNameString << ", at index " << index << " to " << iconNameString);
      this->PlaceNodeIconNameList[index] = iconNameString;
      this->InvokeEvent(vtkMRMLSelectionNode::PlaceNodeClassNameListModifiedEvent);
      }
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::RemovePlaceNodeClassNameFromList(const char *className)
{
  if (className == nullptr)
    {
    return;
    }
  vtkDebugMacro("RemovePlaceNodeClassNameFromList: className = " << className);

  std::string classNameString = std::string(className);

  int index = this->PlaceNodeClassNameInList(classNameString);
  if (index == -1)
    {
    return;
    }
  vtkDebugMacro("Removing placeNode className " << className << ", found at index " << index);
  // erase the className and resource
  this->PlaceNodeClassNameList.erase(this->PlaceNodeClassNameList.begin()+index);
  this->PlaceNodeResourceList.erase(this->PlaceNodeResourceList.begin()+index);
  this->PlaceNodeIconNameList.erase(this->PlaceNodeIconNameList.begin()+index);

  // was it the active one?
  if (this->GetActivePlaceNodeClassName() &&
      classNameString.compare(this->GetActivePlaceNodeClassName()) == 0)
    {
    // make it inactive
    this->SetActivePlaceNodeClassName(nullptr);
    }
  this->InvokeEvent(vtkMRMLSelectionNode::PlaceNodeClassNameListModifiedEvent);
  this->Modified();
}

//----------------------------------------------------------------------------
std::string vtkMRMLSelectionNode::GetPlaceNodeClassNameByIndex(int n)
{
  std::string className;
  if (this->PlaceNodeClassNameList.size() > (unsigned int)n && n >= 0)
    {
    className = this->PlaceNodeClassNameList[n];
    }
  else
    {
    vtkWarningMacro("GetPlaceNodeClassNameByIndex: index " << n << " is out of bounds of 0-" << this->PlaceNodeClassNameList.size());
    }
  return className;
}

//----------------------------------------------------------------------------
std::string vtkMRMLSelectionNode::GetPlaceNodeResourceByIndex(int n)
{
  std::string resource;
  if (this->PlaceNodeResourceList.size() > (unsigned int)n && n >= 0)
    {
    resource = this->PlaceNodeResourceList[n];
    }
  else
    {
    vtkWarningMacro("GetPlaceNodeResourceByIndex: index " << n << " is out of bounds of 0-" << this->PlaceNodeResourceList.size());
    }
  return resource;
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::GetUnitNodes(std::vector<vtkMRMLUnitNode*>& units)
{
  this->UpdateNodeReferences();
  for (NodeReferencesType::const_iterator it = this->NodeReferences.begin();
    it != this->NodeReferences.end(); ++it)
    {
    if (it->first.compare(0, strlen(UNIT_NODE_REFERENCE_ROLE), UNIT_NODE_REFERENCE_ROLE) == 0 &&
        it->second.size() > 0)
      {
      // there is only one referenced node per reference role
      vtkMRMLNodeReference * reference = it->second[0];
      if (reference)
        {
        units.push_back(
          vtkMRMLUnitNode::SafeDownCast(reference->GetReferencedNode()));
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::GetUnitNodeIDs(std::vector<const char*>& quantities,
                                          std::vector<const char*>& units)
{
  for (NodeReferencesType::const_iterator it = this->NodeReferences.begin();
    it != this->NodeReferences.end(); ++it)
    {
    if (it->first.compare(0, strlen(UNIT_NODE_REFERENCE_ROLE), UNIT_NODE_REFERENCE_ROLE) == 0 &&
        it->second.size() > 0)
      {
      // there is only one referenced node per reference role
      vtkMRMLNodeReference * reference = it->second[0];
      if (reference)
        {
        quantities.push_back(&reference->GetReferenceRole()[strlen(UNIT_NODE_REFERENCE_ROLE)]);
        units.push_back(reference->GetReferencedNodeID());
        }
      }
    }
}

//----------------------------------------------------------------------------
std::string vtkMRMLSelectionNode::GetPlaceNodeIconNameByIndex(int n)
{
  std::string iconName;
  if (this->PlaceNodeIconNameList.size() > (unsigned int)n && n >= 0)
    {
    iconName = this->PlaceNodeIconNameList[n];
    }
  else
    {
    vtkWarningMacro("GetPlaceNodeIconNameByIndex: index " << n << " is out of bounds of 0-" << this->PlaceNodeIconNameList.size());
    }
  return iconName;
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetUnitNodeID(const char* quantity)
{
  std::string safeQuantity = quantity ? quantity : "";
  std::string referenceRole = UNIT_NODE_REFERENCE_ROLE + safeQuantity;
  return this->GetNodeReferenceID(referenceRole.c_str());
}

//----------------------------------------------------------------------------
vtkMRMLUnitNode* vtkMRMLSelectionNode::GetUnitNode(const char* quantity)
{
  std::string safeQuantity = quantity ? quantity : "";
  std::string referenceRole = UNIT_NODE_REFERENCE_ROLE + safeQuantity;
  return vtkMRMLUnitNode::SafeDownCast(this->GetNodeReference(referenceRole.c_str()));
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetUnitNodeID(const char* quantity, const char* id)
{
  std::string safeQuantity = quantity ? quantity : "";
  std::string referenceRole = UNIT_NODE_REFERENCE_ROLE + safeQuantity;

  vtkMTimeType mTime = this->GetMTime();
  this->SetAndObserveNodeReferenceID(referenceRole.c_str(), id);
  // \todo a bit too much hackish...
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

//----------------------------------------------------------------------------
int vtkMRMLSelectionNode::PlaceNodeClassNameInList(std::string className)
{
  for (unsigned int i = 0; i < this->PlaceNodeClassNameList.size(); ++i)
    {
    if (this->PlaceNodeClassNameList[i].compare(className) == 0)
      {
      return i;
      }
    }
  return -1;
}

//----------------------------------------------------------------------------
std::string vtkMRMLSelectionNode::GetPlaceNodeResourceByClassName(std::string className)
{
  std::string resource;

  int placeNodeIndex = this->PlaceNodeClassNameInList(className);
  if (placeNodeIndex != -1)
    {
    resource = this->GetPlaceNodeResourceByIndex(placeNodeIndex);
    }
  return resource;
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetReferenceActivePlaceNodeClassName (const char *className)
{
  this->SetActivePlaceNodeClassName(className);
  this->InvokeEvent(vtkMRMLSelectionNode::ActivePlaceNodeClassNameChangedEvent);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetActiveVolumeID()
{
  return this->GetNodeReferenceID(ACTIVE_VOLUME_REFERENCE_ROLE);
}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetActiveVolumeID(const char* id)
{
  this->SetNodeReferenceID(ACTIVE_VOLUME_REFERENCE_ROLE, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetSecondaryVolumeID()
{
  return this->GetNodeReferenceID(SECONDARY_VOLUME_REFERENCE_ROLE);
}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetSecondaryVolumeID(const char* id)
{
  this->SetNodeReferenceID(SECONDARY_VOLUME_REFERENCE_ROLE, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetActiveLabelVolumeID()
{
  return this->GetNodeReferenceID(ACTIVE_LABEL_VOLUME_REFERENCE_ROLE);
}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetActiveLabelVolumeID(const char* id)
{
  this->SetNodeReferenceID(ACTIVE_LABEL_VOLUME_REFERENCE_ROLE, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetActiveFiducialListID()
{
  return this->GetNodeReferenceID(ACTIVE_FIDUCIAL_LIST_REFERENCE_ROLE);
}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetActiveFiducialListID(const char* id)
{
  this->SetNodeReferenceID(ACTIVE_FIDUCIAL_LIST_REFERENCE_ROLE, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetActivePlaceNodeID()
{
  return this->GetNodeReferenceID(ACTIVE_PLACE_NODE_REFERENCE_ROLE);
}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetActivePlaceNodeID(const char* id)
{
  this->SetNodeReferenceID(ACTIVE_PLACE_NODE_REFERENCE_ROLE, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetActiveROIListID()
{
  return this->GetNodeReferenceID(ACTIVE_ROI_LIST_REFERENCE_ROLE);
}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetActiveROIListID(const char* id)
{
  this->SetNodeReferenceID(ACTIVE_ROI_LIST_REFERENCE_ROLE, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetActiveCameraID()
{
  return this->GetNodeReferenceID(ACTIVE_CAMERA_REFERENCE_ROLE);
}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetActiveCameraID(const char* id)
{
  this->SetNodeReferenceID(ACTIVE_CAMERA_REFERENCE_ROLE, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetActiveTableID()
{
  return this->GetNodeReferenceID(ACTIVE_TABLE_REFERENCE_ROLE);
}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetActiveTableID(const char* id)
{
  this->SetNodeReferenceID(ACTIVE_TABLE_REFERENCE_ROLE, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetActiveViewID()
{
  return this->GetNodeReferenceID(ACTIVE_VIEW_REFERENCE_ROLE);
}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetActiveViewID(const char* id)
{
  this->SetNodeReferenceID(ACTIVE_VIEW_REFERENCE_ROLE, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetActiveLayoutID()
{
  return this->GetNodeReferenceID(ACTIVE_LAYOUT_REFERENCE_ROLE);
}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetActiveLayoutID(const char* id)
{
  this->SetNodeReferenceID(ACTIVE_LAYOUT_REFERENCE_ROLE, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetActivePlotChartID()
{
  return this->GetNodeReferenceID(ACTIVE_PLOT_CHART_REFERENCE_ROLE);
}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetActivePlotChartID(const char* id)
{
  this->SetNodeReferenceID(ACTIVE_PLOT_CHART_REFERENCE_ROLE, id);
}
