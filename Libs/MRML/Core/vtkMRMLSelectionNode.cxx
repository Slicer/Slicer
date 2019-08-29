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

// STD includes

vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, SecondaryVolumeID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveLabelVolumeID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveFiducialListID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActivePlaceNodeID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveROIListID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveCameraID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveTableID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveViewID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveLayoutID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActiveVolumeID);
vtkCxxSetReferenceStringMacro(vtkMRMLSelectionNode, ActivePlotChartID);

const char* vtkMRMLSelectionNode::UnitNodeReferenceRole = "unit/";
const char* vtkMRMLSelectionNode::UnitNodeReferenceMRMLAttributeName = "UnitNodeRef";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSelectionNode);

//----------------------------------------------------------------------------
vtkMRMLSelectionNode::vtkMRMLSelectionNode()
{
  this->HideFromEditors = 1;

  this->SetSingletonTag("Singleton");

  this->ActiveVolumeID = nullptr;
  this->SecondaryVolumeID = nullptr;
  this->ActiveLabelVolumeID = nullptr;
  this->ActiveFiducialListID = nullptr;
  this->ActivePlaceNodeID = nullptr;
  this->ActivePlaceNodeClassName = nullptr;
  this->ActiveROIListID  =nullptr;
  this->ActiveCameraID = nullptr;
  this->ActiveTableID = nullptr;
  this->ActiveViewID = nullptr;
  this->ActiveLayoutID = nullptr;
  this->ActivePlotChartID = nullptr;

  this->AddNodeReferenceRole(this->GetUnitNodeReferenceRole(),
                             this->GetUnitNodeReferenceMRMLAttributeName());
}

//----------------------------------------------------------------------------
vtkMRMLSelectionNode::~vtkMRMLSelectionNode()
{
  if (this->ActiveVolumeID)
    {
    delete [] this->ActiveVolumeID;
    this->ActiveVolumeID = nullptr;
    }
  if (this->SecondaryVolumeID)
    {
    delete [] this->SecondaryVolumeID;
    this->SecondaryVolumeID = nullptr;
    }
  if (this->ActiveLabelVolumeID)
    {
    delete [] this->ActiveLabelVolumeID;
    this->ActiveLabelVolumeID = nullptr;
    }
  if (this->ActiveFiducialListID)
    {
    delete [] this->ActiveFiducialListID;
    this->ActiveFiducialListID = nullptr;
    }
  if (this->ActivePlaceNodeID)
    {
    delete [] this->ActivePlaceNodeID;
    this->ActivePlaceNodeID = nullptr;
    }
  this->SetActivePlaceNodeClassName(nullptr);
  if (this->ActiveROIListID)
    {
    delete [] this->ActiveROIListID;
    this->ActiveROIListID = nullptr;
    }
  if ( this->ActiveCameraID )
    {
    delete [] this->ActiveCameraID;
    this->ActiveCameraID = nullptr;
    }
  if ( this->ActiveTableID )
    {
    delete [] this->ActiveTableID;
    this->ActiveTableID = nullptr;
    }
  if ( this->ActiveViewID)
    {
    delete []  this->ActiveViewID;
    this->ActiveViewID = nullptr;
    }
  if ( this->ActiveLayoutID)
    {
    delete [] this->ActiveLayoutID;
    this->ActiveLayoutID = nullptr;
    }
  if ( this->ActivePlotChartID)
    {
    delete [] this->ActivePlotChartID;
    this->ActivePlotChartID = nullptr;
    }
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetUnitNodeReferenceRole()
{
  return vtkMRMLSelectionNode::UnitNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkMRMLSelectionNode::GetUnitNodeReferenceMRMLAttributeName()
{
  return vtkMRMLSelectionNode::UnitNodeReferenceMRMLAttributeName;
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " activeVolumeID=\"" << (this->ActiveVolumeID ? this->ActiveVolumeID : "NULL") << "\"";
  of << " secondaryVolumeID=\"" << (this->SecondaryVolumeID ? this->SecondaryVolumeID : "NULL") << "\"";
  of << " activeLabelVolumeID=\"" << (this->ActiveLabelVolumeID ? this->ActiveLabelVolumeID : "NULL") << "\"";
  of << " activeFiducialListID=\"" << (this->ActiveFiducialListID ? this->ActiveFiducialListID : "NULL") << "\"";
  of << " activePlaceNodeID=\"" << (this->ActivePlaceNodeID ? this->ActivePlaceNodeID : "NULL") << "\"";
  of << " activePlaceNodeClassName=\"" << (this->ActivePlaceNodeClassName ? this->ActivePlaceNodeClassName : "NULL") << "\"";
  of << " activeROIListID=\"" << (this->ActiveROIListID ? this->ActiveROIListID : "NULL") << "\"";
  of << " activeCameraID=\"" << (this->ActiveCameraID ? this->ActiveCameraID : "NULL") << "\"";
  of << " activeTableID=\"" << (this->ActiveTableID ? this->ActiveTableID : "NULL") << "\"";
  of << " activeViewID=\"" << (this->ActiveViewID ? this->ActiveViewID : "NULL") << "\"";
  of << " activeLayoutID=\"" << (this->ActiveLayoutID ? this->ActiveLayoutID : "NULL") << "\"";
  of << " activePlotChartID=\"" << (this->ActivePlotChartID ? this->ActivePlotChartID : "NULL") << "\"";

  if (this->ModelHierarchyDisplayNodeClassName.size() > 0)
    {
    of << " modelHierarchyDisplayableNodeClassName=\"";

    for (std::map<std::string, std::string>::const_iterator it = this->ModelHierarchyDisplayNodeClassName.begin();
         it != this->ModelHierarchyDisplayNodeClassName.end(); it++)
      {
      of << " " << it->first;
      }
    of << "\"";

    of << " modelHierarchyDisplayNodeClassName=\"";

    for (std::map<std::string, std::string>::const_iterator it = this->ModelHierarchyDisplayNodeClassName.begin();
         it != this->ModelHierarchyDisplayNodeClassName.end(); it++)
      {
      of << " " << it->second;
      }
    of << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  this->Scene->AddReferencedNodeID(this->ActiveVolumeID, this);
  this->Scene->AddReferencedNodeID(this->ActiveLabelVolumeID, this);
  this->Scene->AddReferencedNodeID(this->ActiveFiducialListID, this);
  this->Scene->AddReferencedNodeID(this->ActivePlaceNodeID, this);
  this->Scene->AddReferencedNodeID(this->ActiveCameraID, this);
  this->Scene->AddReferencedNodeID(this->ActiveTableID, this);
  this->Scene->AddReferencedNodeID(this->ActiveViewID, this);
  this->Scene->AddReferencedNodeID(this->ActiveLayoutID, this);
  this->Scene->AddReferencedNodeID(this->ActivePlotChartID, this);
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
  if (this->ActivePlaceNodeID && !strcmp(oldID, this->ActivePlaceNodeID))
    {
    this->SetActivePlaceNodeID(newID);
    }
  if ( this->ActiveCameraID && !strcmp (oldID, this->ActiveCameraID ))
    {
    this->SetActiveCameraID (newID);
    }
if ( this->ActiveTableID && !strcmp (oldID, this->ActiveTableID ))
    {
    this->SetActiveTableID (newID);
    }
  if ( this->ActiveViewID && !strcmp ( oldID, this->ActiveViewID ))
    {
    this->SetActiveViewID (newID );
    }
  if ( this->ActiveLayoutID && !strcmp ( oldID, this->ActiveLayoutID ))
    {
    this->SetActiveLayoutID (newID );
    }
  if ( this->ActivePlotChartID && !strcmp ( oldID, this->ActivePlotChartID ))
    {
    this->SetActivePlotChartID (newID );
    }
}

//-----------------------------------------------------------
void vtkMRMLSelectionNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->ActiveVolumeID != nullptr && this->Scene->GetNodeByID(this->ActiveVolumeID) == nullptr)
    {
    this->SetActiveVolumeID(nullptr);
    }
  if (this->SecondaryVolumeID != nullptr && this->Scene->GetNodeByID(this->SecondaryVolumeID) == nullptr)
    {
    this->SetSecondaryVolumeID(nullptr);
    }
  if (this->ActiveLabelVolumeID != nullptr && this->Scene->GetNodeByID(this->ActiveLabelVolumeID) == nullptr)
    {
    this->SetActiveLabelVolumeID(nullptr);
    }
  if (this->ActiveFiducialListID != nullptr && this->Scene->GetNodeByID(this->ActiveFiducialListID) == nullptr)
    {
    this->SetActiveFiducialListID(nullptr);
    }
  if (this->ActivePlaceNodeID != nullptr && this->Scene->GetNodeByID(this->ActivePlaceNodeID) == nullptr)
    {
    this->SetActivePlaceNodeID(nullptr);
    }
  if (this->ActiveViewID != nullptr && this->Scene->GetNodeByID(this->ActiveViewID) == nullptr)
    {
    this->SetActiveViewID(nullptr);
    }
  if (this->ActiveLayoutID != nullptr && this->Scene->GetNodeByID(this->ActiveLayoutID) == nullptr)
    {
    this->SetActiveLayoutID(nullptr);
    }
  if (this->ActiveCameraID != nullptr && this->Scene->GetNodeByID(this->ActiveCameraID) == nullptr)
    {
    this->SetActiveCameraID(nullptr);
    }
  if (this->ActiveTableID != nullptr && this->Scene->GetNodeByID(this->ActiveTableID) == nullptr)
    {
    this->SetActiveTableID(nullptr);
    }
  if (this->ActivePlotChartID != nullptr && this->Scene->GetNodeByID(this->ActivePlotChartID) == nullptr)
    {
    this->SetActivePlotChartID(nullptr);
    }
}
//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  std::vector<std::string> modelHierarchyDisplayableNodeClassName;
  std::vector<std::string> modelHierarchyDisplayNodeClassName;

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
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
    if (!strcmp(attName, "activePlaceNodeID"))
      {
      this->SetActivePlaceNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->ActivePlaceNodeID, this);
      }
    if (!strcmp(attName, "activePlaceNodeClassName"))
      {
      this->SetActivePlaceNodeClassName(attValue);
      }
    if (!strcmp (attName, "activeCameraID"))
      {
      this->SetActiveCameraID (attValue );
      //this->Scene->AddReferencedNodeID (this->ActiveCameraID, this);
      }
    if (!strcmp (attName, "activeTableID"))
      {
      this->SetActiveTableID (attValue );
      //this->Scene->AddReferencedNodeID (this->ActiveTableID, this);
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
    if (!strcmp (attName, "activePlotChartID"))
      {
      this->SetActivePlotChartID (attValue);
      //this->Scene->AddReferencedNodeID ( this->ActivePlotChartID, this);
      }
    if (!strcmp (attName, "modelHierarchyDisplayableNodeClassName"))
      {
      std::stringstream ss(attValue);
      while (!ss.eof())
        {
        std::string name;
        ss >> name;
        if (!name.empty())
          {
          modelHierarchyDisplayableNodeClassName.push_back(name);
          }
        }
      }
    if (!strcmp (attName, "modelHierarchyDisplayNodeClassName"))
      {
      std::stringstream ss(attValue);
      while (!ss.eof())
        {
        std::string name;
        ss >> name;
        if (!name.empty())
          {
          modelHierarchyDisplayNodeClassName.push_back(name);
          }
        }
      }
    }

  if (modelHierarchyDisplayableNodeClassName.size() != modelHierarchyDisplayNodeClassName.size() )
    {
    vtkErrorMacro(<< "ReadXMLAttributes: Number of modelHierarchyDisplayableNodeClassName is not equal to modelHierarchyDisplayNodeClassName.");
    }
  else
    {
    std::vector<std::string>::const_iterator it1 = modelHierarchyDisplayableNodeClassName.begin();
    for (std::vector<std::string>::const_iterator it = modelHierarchyDisplayNodeClassName.begin();
        it != modelHierarchyDisplayNodeClassName.end(); it++)
      {
      this->AddModelHierarchyDisplayNodeClassName(*it1, *it);
      it1++;
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
  this->SetActivePlaceNodeID(node->GetActivePlaceNodeID());
  this->SetActivePlaceNodeClassName(node->GetActivePlaceNodeClassName());
  this->SetActiveCameraID (node->GetActiveCameraID());
  this->SetActiveTableID (node->GetActiveTableID());
  this->SetActiveViewID (node->GetActiveViewID() );
  this->SetActiveLayoutID (node->GetActiveLayoutID() );
  this->SetActivePlotChartID (node->GetActivePlotChartID());
  this->ModelHierarchyDisplayNodeClassName = node->ModelHierarchyDisplayNodeClassName;
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
  os << "ActivePlaceNodeID: " << ( (this->ActivePlaceNodeID) ? this->ActivePlaceNodeID : "None" ) << "\n";
  os << "ActivePlaceNodeClassName: " << ( (this->ActivePlaceNodeClassName) ? this->ActivePlaceNodeClassName : "None" ) << "\n";
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
  os << "ActiveCameraID: " << ( (this->ActiveCameraID) ? this->ActiveCameraID : "None" ) << "\n";
  os << "ActiveTableID: " << ( (this->ActiveTableID) ? this->ActiveTableID : "None" ) << "\n";
  os << "ActiveViewID: " << ( (this->ActiveViewID) ? this->ActiveViewID : "None" ) << "\n";
  os << "ActiveLayoutID: " << ( (this->ActiveLayoutID) ? this->ActiveLayoutID : "None" ) << "\n";
  os << "ActivePlotChartID: " << ( (this->ActivePlotChartID) ? this->ActivePlotChartID : "None" ) << "\n";

  if (this->ModelHierarchyDisplayNodeClassName.size() > 0)
    {
    os << "Mode lHierarchy Display Node Class Name: \n";
    for (std::map<std::string, std::string>::const_iterator it = this->ModelHierarchyDisplayNodeClassName.begin();
         it != this->ModelHierarchyDisplayNodeClassName.end(); it++)
      {
      os << indent.GetNextIndent() << it->first << " " << it->second << "\n";
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
    if (it->first.compare(0, strlen(this->GetUnitNodeReferenceRole()),
                          this->GetUnitNodeReferenceRole()) == 0 &&
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
    if (it->first.compare(0, strlen(this->GetUnitNodeReferenceRole()),
                          this->GetUnitNodeReferenceRole()) == 0 &&
        it->second.size() > 0)
      {
      // there is only one referenced node per reference role
      vtkMRMLNodeReference * reference = it->second[0];
      if (reference)
        {
        quantities.push_back(&reference->GetReferenceRole()[strlen(this->GetUnitNodeReferenceRole())]);
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
  std::string referenceRole = this->GetUnitNodeReferenceRole() + safeQuantity;
  return this->GetNodeReferenceID(referenceRole.c_str());
}

//----------------------------------------------------------------------------
vtkMRMLUnitNode* vtkMRMLSelectionNode::GetUnitNode(const char* quantity)
{
  std::string safeQuantity = quantity ? quantity : "";
  std::string referenceRole = this->GetUnitNodeReferenceRole() + safeQuantity;
  return vtkMRMLUnitNode::SafeDownCast(this->GetNodeReference(referenceRole.c_str()));
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetUnitNodeID(const char* quantity, const char* id)
{
  std::string safeQuantity = quantity ? quantity : "";
  std::string referenceRole = this->GetUnitNodeReferenceRole() + safeQuantity;

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
std::string
vtkMRMLSelectionNode::GetModelHierarchyDisplayNodeClassName(const std::string& dispayableNodeClass)const
{
  std::map<std::string, std::string>::const_iterator it = this->ModelHierarchyDisplayNodeClassName.find(dispayableNodeClass);
  return (it == this->ModelHierarchyDisplayNodeClassName.end() ? std::string() : it->second);
}

//----------------------------------------------------------------------------
std::map<std::string, std::string>
vtkMRMLSelectionNode::GetModelHierarchyDisplayNodeClassNames()const
{
  return this->ModelHierarchyDisplayNodeClassName;
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::AddModelHierarchyDisplayNodeClassName(const std::string& dispayableNodeClass,
                                                                 const std::string& displayNodeClass)
{
  this->ModelHierarchyDisplayNodeClassName[dispayableNodeClass] = displayNodeClass;
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::ClearModelHierarchyDisplayNodeClassNames()
{
  this->ModelHierarchyDisplayNodeClassName.clear();
}
