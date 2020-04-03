/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.11 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <iostream>
#include <sstream>
#include <algorithm> // for std::sort

//------------------------------------------------------------------------------
vtkMRMLNode::vtkMRMLNode()
{
  this->ContentModifiedEvents = vtkIntArray::New();
  this->ContentModifiedEvents->InsertNextValue(vtkCommand::ModifiedEvent);

  // Set up callbacks
  this->MRMLCallbackCommand = vtkCallbackCommand::New();
  this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->MRMLCallbackCommand->SetCallback( vtkMRMLNode::MRMLCallback );

  this->MRMLObserverManager = vtkObserverManager::New();
  this->MRMLObserverManager->AssignOwner( this );
  this->MRMLObserverManager->GetCallbackCommand()->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLObserverManager->GetCallbackCommand()->SetCallback(vtkMRMLNode::MRMLCallback);
}

//----------------------------------------------------------------------------
vtkMRMLNode::~vtkMRMLNode()
{
  // Delete all references in bulk, which is faster than RemoveNodeReferenceIDs
  // but it does not send individual NodeReferencesRemoved events (the whole class
  // is deleted, so references cannot be meaningful anymore) or updates node
  // references in the scene (the scene removes all node references anyway when
  // a node is deleted).
  // Need to remove all observers by calling InvalidateNodeReferences
  // before clearing this->NodeReferences to avoid memory leaks.
  this->InvalidateNodeReferences();
  this->NodeReferences.clear();
  this->NodeReferenceEvents.clear();

  this->SetID(nullptr);
  this->SetName(nullptr);
  this->SetDescription(nullptr);

  if (this->MRMLObserverManager)
    {
    this->MRMLObserverManager->Delete();
    this->MRMLObserverManager = nullptr;
    }

  // unregister and set null pointers.
  if ( this->MRMLCallbackCommand )
    {
    this->MRMLCallbackCommand->SetClientData( nullptr );
    this->MRMLCallbackCommand->Delete ( );
    this->MRMLCallbackCommand = nullptr;
    }

  if (this->ContentModifiedEvents)
    {
    this->ContentModifiedEvents->Delete();
    this->ContentModifiedEvents = nullptr;
    }

  this->SetTempURLString(nullptr);
  this->SetSingletonTag(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLNode::CopyWithScene(vtkMRMLNode *node)
{
  MRMLNodeModifyBlocker blocker(this);
  if (node->GetScene())
    {
    this->SetScene(node->GetScene());
    }
  if (node->GetID())
    {
    this->SetID( node->GetID() );
    }
  this->Copy(node);
}

//----------------------------------------------------------------------------
void vtkMRMLNode::Copy(vtkMRMLNode *node)
{
  MRMLNodeModifyBlocker blocker(this);
  vtkMRMLCopyBeginMacro(node);

  // Only copy the node name when in the source node it is not empty
  // this is used for singleton node updates and for resetting nodes.
  if (node->GetName() && strcmp(node->GetName(),""))
    {
    vtkMRMLCopyStringMacro(Name);
    }
  vtkMRMLCopyBooleanMacro(HideFromEditors);
  vtkMRMLCopyBooleanMacro(AddToScene);
  if (node->GetSingletonTag())
    {
    vtkMRMLCopyStringMacro(SingletonTag);
    }
  vtkMRMLCopyBooleanMacro(UndoEnabled);
  vtkMRMLCopyEndMacro();
  this->CopyContent(node);
  this->CopyReferences(node);
}

//----------------------------------------------------------------------------
void vtkMRMLNode::CopyContent(vtkMRMLNode* node, bool vtkNotUsed(deepCopy)/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyStringMacro(Description);
  vtkMRMLCopyBooleanMacro(Selectable);
  vtkMRMLCopyEndMacro();
  this->Attributes = node->Attributes;
}

//----------------------------------------------------------------------------
bool ArraysEqual(vtkIntArray* array1, vtkIntArray* array2)
{
  if (array1 == nullptr && array2 == nullptr)
    {
    return true;
    }
  if (array1 == nullptr || array2 == nullptr)
    {
    return false;
    }
  if (array1->GetNumberOfTuples() != array2->GetNumberOfTuples())
    {
    return false;
    }
  int arraySize = array1->GetNumberOfTuples();
  for (int i = 0; i<arraySize; i++)
    {
    if (array1->GetValue(i) != array2->GetValue(i))
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLNode::CopyReferences(vtkMRMLNode* node)
{
  this->NodeReferenceMRMLAttributeNames = node->NodeReferenceMRMLAttributeNames;
  this->NodeReferenceEvents = node->NodeReferenceEvents;

  // Only update references is they are different (to avoid unnecessary event invocations)
  std::vector<std::string> referenceRoles;
  this->GetNodeReferenceRoles(referenceRoles);
  std::vector<std::string> referenceRolesInSource;
  node->GetNodeReferenceRoles(referenceRolesInSource);
  bool referencesAreEqual = false;
  if (referenceRoles == referenceRolesInSource)
    {
    referencesAreEqual = true;
    for (NodeReferencesType::iterator sourceNodeReferencesIt = node->NodeReferences.begin();
      sourceNodeReferencesIt != node->NodeReferences.end() && referencesAreEqual; sourceNodeReferencesIt++)
      {
      std::string referenceRole = sourceNodeReferencesIt->first;
      NodeReferencesType::iterator targetNodeReferencesIt = this->NodeReferences.find(referenceRole);
      if (sourceNodeReferencesIt->second.size() != targetNodeReferencesIt->second.size())
        {
        referencesAreEqual = false;
        break;
        }
      int numberOfNodeReferences = sourceNodeReferencesIt->second.size();
      for (int i = 0; i < numberOfNodeReferences; i++)
        {
        vtkMRMLNodeReference* sourceReference = sourceNodeReferencesIt->second[i];
        vtkMRMLNodeReference* targetReference = targetNodeReferencesIt->second[i];
        if (!sourceReference || !targetReference)
          {
          vtkErrorMacro(<< "CopyReferences: invalid reference found.");
          referencesAreEqual = false;
          break;
          }
        if (strcmp(sourceReference->GetReferencedNodeID(), targetReference->GetReferencedNodeID()) != 0
          || !ArraysEqual(sourceReference->GetEvents(), targetReference->GetEvents()))
          {
          referencesAreEqual = false;
          break;
          }
        }
      }
    }

  if (referencesAreEqual)
    {
    // no need to copy, they are already the same
    return;
    }

  // Remove all existing references
  this->RemoveNodeReferenceIDs(nullptr);

  // Add node references
  for (NodeReferencesType::iterator it = node->NodeReferences.begin(); it != node->NodeReferences.end(); it++)
    {
    std::string referenceRole = it->first;
    NodeReferenceListType::iterator it1;
    for (it1 = it->second.begin(); it1 != it->second.end(); it1++)
      {
      vtkMRMLNodeReference* reference = *it1;
      if (!reference)
        {
        vtkErrorMacro(<< "CopyReferences: Reference is expected to be non nullptr.");
        return;
        }
      // We must not use SetAndObserveNthNodeReferenceID here, because referenced node IDs may change
      // due to conflict with node IDs existing in the scene.
      vtkNew<vtkMRMLNodeReference> copiedReference;
      copiedReference->SetReferenceRole(referenceRole.c_str());
      copiedReference->SetReferencedNodeID(reference->GetReferencedNodeID());
      copiedReference->SetReferencingNode(this);
      copiedReference->SetEvents(reference->GetEvents());
      this->NodeReferences[std::string(referenceRole)].push_back(copiedReference.GetPointer());
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNode::Reset(vtkMRMLNode* defaultNode)
{
  vtkSmartPointer<vtkMRMLNode> newNode;
  if (defaultNode)
    {
    newNode = defaultNode;
    }
  else
    {
    newNode = vtkSmartPointer<vtkMRMLNode>::Take(this->CreateNodeInstance());
    }

  // Preserve SaveWithScene, HideFromEditors, Selectable, and SingletonTag
  // properties during reset by saving the original values and then restoring them.

  // Save
  int save = this->GetSaveWithScene();
  int hide = this->GetHideFromEditors();
  int select = this->GetSelectable();
  char *tag = this->GetSingletonTag();

  int wasModifying = this->StartModify();

  // Copy
  this->CopyWithScene(newNode);

  // Restore
  this->SetSaveWithScene(save);
  this->SetHideFromEditors(hide);
  this->SetSelectable(select);
  this->SetSingletonTag(tag);

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintStringMacro(ID);
  vtkMRMLPrintStringMacro(ClassName);
  vtkMRMLPrintStringMacro(Name);

  // vtkObject's PrintSelf prints a long list of registered events, which
  // is too long and not useful, therefore we don't call vtkObject::PrintSelf
  // but print essential information on the vtkObject base.
  vtkMRMLPrintBooleanMacro(Debug);
  vtkMRMLPrintIntMacro(MTime);

  vtkMRMLPrintStringMacro(Description);
  vtkMRMLPrintStringMacro(SingletonTag);
  vtkMRMLPrintBooleanMacro(HideFromEditors);
  vtkMRMLPrintBooleanMacro(Selectable);
  vtkMRMLPrintBooleanMacro(Selected);
  vtkMRMLPrintBooleanMacro(UndoEnabled)
  vtkMRMLPrintEndMacro();

  if (!this->Attributes.empty())
    {
    os << indent << "Attributes:\n";
    AttributesType::const_iterator it;
    AttributesType::const_iterator begin = this->Attributes.begin();
    AttributesType::const_iterator end = this->Attributes.end();
    for (it = begin; it != end; ++it)
      {
      os << indent.GetNextIndent() << it->first << ':' << it->second << "\n";
      }
    }

  if (!this->NodeReferences.empty())
    {
    os << indent << "Node references:\n";
    NodeReferencesType::iterator it;
    for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
      {
      const std::string& referenceRole = it->first;
      os << indent.GetNextIndent() << referenceRole;
      const char* refAttribute = this->GetMRMLAttributeNameFromReferenceRole(referenceRole.c_str());
      if (refAttribute != nullptr)
        {
        os << " [" << refAttribute << "]";
        }
      os << ":";
      std::vector< const char* > referencedNodeIds;
      GetNodeReferenceIDs(referenceRole.c_str(), referencedNodeIds);
      if (referencedNodeIds.empty())
        {
        os << " (none)\n";
        }
      else
        {
        for (std::vector< const char* >::iterator referencedNodeIdsIt=referencedNodeIds.begin(); referencedNodeIdsIt!=referencedNodeIds.end(); ++referencedNodeIdsIt)
          {
          const char * id = *referencedNodeIdsIt;
          os << " " << (id ? id : "(nullptr)");
          }
        os << "\n";
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLStringMacro(id, ID);
  vtkMRMLReadXMLStringMacro(name, Name);
  vtkMRMLReadXMLStringMacro(description, Description);
  vtkMRMLReadXMLBooleanMacro(hideFromEditors, HideFromEditors);
  vtkMRMLReadXMLBooleanMacro(selectable, Selectable);
  vtkMRMLReadXMLBooleanMacro(selected, Selected);
  vtkMRMLReadXMLStringMacro(singletonTag, SingletonTag);
  vtkMRMLReadXMLBooleanMacro(undoEnabled, UndoEnabled)
  vtkMRMLReadXMLEndMacro();

  std::set<std::string> references;
  const char** xmlReadAtts = atts;
  const char* attName;
  const char* attValue;
  while (*xmlReadAtts != nullptr)
    {
    attName = *(xmlReadAtts++);
    attValue = *(xmlReadAtts++);
    if (!strcmp(attName, "attributes"))
      {
      std::stringstream attributes(attValue);
      std::string attribute;
      while (std::getline(attributes, attribute, ';'))
        {
        int colonIndex = attribute.find(':');
        std::string name = attribute.substr(0, colonIndex);
        std::string value = attribute.substr(colonIndex + 1);
        // decode percent sign and semicolon (semicolon is a special character because it separates attributes)
        vtksys::SystemTools::ReplaceString(value, "%3B", ";");
        vtksys::SystemTools::ReplaceString(value, "%25", "%");
        this->SetAttribute(name.c_str(), value.c_str());
        }
      }
    else if (!strcmp(attName, "references"))
      {
      this->ParseReferencesAttribute(attValue, references);
      }
    }

  // It is important that we look for legacy node references after the "reference" attribute.
  // If the reference has already been read from the "references" attribute, then we skip it here.
  xmlReadAtts = atts;
  while (*xmlReadAtts != nullptr)
    {
    attName = *(xmlReadAtts++);
    attValue = *(xmlReadAtts++);
    if ( const char* referenceRole =
                this->GetReferenceRoleFromMRMLAttributeName(attName) )
      {
      // Reference role has already been read
      if (references.find(referenceRole) != references.end())
        {
        continue;
        }

      std::stringstream ss(attValue);
      while (!ss.eof())
        {
        std::string id;
        ss >> id;
        if (!id.empty())
          {
          this->AddNodeReferenceID(referenceRole, id.c_str());
          references.insert(std::string(referenceRole));
          }
        }
      }
    }
  this->EndModify(disabledModify);

  return;
}


//----------------------------------------------------------------------------
void vtkMRMLNode::ParseReferencesAttribute(const char *attValue, std::set<std::string> &references)
{
  /// parse references in the form "role1:id1 id2;role2:id3;"
  std::string attribute(attValue);

  std::size_t start = 0;
  std::size_t end = attribute.find_first_of(';', start);
  std::size_t sep = attribute.find_first_of(':', start);
  while (start != std::string::npos && sep != std::string::npos && start != end && start != sep)
    {
    std::string role = attribute.substr(start, sep-start);
    // Only process this role if it has not been encountered already
    if (references.find(role) == references.end())
      {
      std::string ids = attribute.substr(sep + 1, end - sep - 1);
      std::stringstream ss(ids);
      while (!ss.eof())
        {
        std::string id;
        ss >> id;
        if (!id.empty())
          {
          this->AddNodeReferenceID(role.c_str(), id.c_str());
          references.insert(role);
          }
        }
      }
    start = (end == std::string::npos) ? std::string::npos : end+1;
    end = attribute.find_first_of(';', start);
    sep = attribute.find_first_of(':', start);
    }
}


//----------------------------------------------------------------------------
void vtkMRMLNode::WriteXML(ostream& of, int nIndent)
{
  vtkIndent indent(nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLStringMacro(id, ID);
  vtkMRMLWriteXMLStringMacro(name, Name);
  vtkMRMLWriteXMLStringMacro(description, Description);
  vtkMRMLWriteXMLBooleanMacro(hideFromEditors, HideFromEditors);
  vtkMRMLWriteXMLBooleanMacro(selectable, Selectable);
  vtkMRMLWriteXMLBooleanMacro(selected, Selected);
  vtkMRMLWriteXMLStringMacro(singletonTag, SingletonTag);
  if (this->UndoEnabled)
    {
    // Only write out UndoEnabled flag in case of non-default value is used,
    // to keep the written XML file cleaner.
    vtkMRMLWriteXMLBooleanMacro(undoEnabled, UndoEnabled);
    }
  vtkMRMLWriteXMLEndMacro();

  if (this->Attributes.size())
    {
    of << " attributes=\"";
    AttributesType::const_iterator it;
    AttributesType::const_iterator begin = this->Attributes.begin();
    AttributesType::const_iterator end = this->Attributes.end();
    for (it = begin; it != end; ++it)
      {
      if (it != begin)
        {
        of << ';';
        }
      std::string attributeValue = it->second;
      // encode percent sign and semicolon (semicolon is a special character because it separates attributes)
      vtksys::SystemTools::ReplaceString(attributeValue, "%", "%25");
      vtksys::SystemTools::ReplaceString(attributeValue, ";", "%3B");
      of << this->XMLAttributeEncodeString(it->first) << ':' << this->XMLAttributeEncodeString(attributeValue);
      }
    of << "\"";
    }

  //write node references
  std::stringstream ssRef;
  NodeReferencesType::iterator it;
  std::map< std::string, std::string>::iterator itName;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    const std::string& referenceRole = it->first;
    int numReferencedNodes = this->GetNumberOfNodeReferences(referenceRole.c_str());

    if (numReferencedNodes > 0)
      {
      ssRef << referenceRole << ":";
      }

    for (int n=0; n < numReferencedNodes; n++)
      {
      const char * id = this->GetNthNodeReferenceID(referenceRole.c_str(), n);
      ssRef << id;
      if (n < numReferencedNodes-1)
        {
        ssRef << " ";
        }
      }
    if (numReferencedNodes > 0)
      {
      ssRef << ";";
      }
    }

    if (!(ssRef.str().empty()))
      {
      of << " " << "references=\"" << ssRef.str().c_str() << "\"";
      }
}

//----------------------------------------------------------------------------
void vtkMRMLNode::WriteNodeBodyXML(ostream &, int )
{
}

//----------------------------------------------------------------------------
void vtkMRMLNode::ProcessMRMLEvents (vtkObject *caller,
                                     unsigned long event,
                                     void *vtkNotUsed(callData) )
{

  // Refreshes the reference (by calling GetNthNodeReference).
  // Also invokes vtkMRMLNode::ReferencedNodeModifiedEvent if any of the observed
  // referenced nodes are changed. Used by extensions.
  NodeReferencesType::iterator it;
  for (it=this->NodeReferences.begin(); it!=this->NodeReferences.end(); it++)
    {
    if (it->first.c_str())
      {
      NodeReferenceListType references = this->NodeReferences[it->first];
      for (unsigned int i=0; i<references.size(); i++)
        {
        vtkMRMLNode *node = this->GetNthNodeReference(it->first.c_str(), i);
        if (node != nullptr && node == vtkMRMLNode::SafeDownCast(caller) &&
          event ==  vtkCommand::ModifiedEvent)
          {
          this->InvokeEvent(vtkMRMLNode::ReferencedNodeModifiedEvent, node);
          }
        }
      }
    }
  return;
}

//----------------------------------------------------------------------------
vtkMRMLScene* vtkMRMLNode::GetScene()
{
  return this->Scene.GetPointer();
}

//----------------------------------------------------------------------------
void vtkMRMLNode::SetScene(vtkMRMLScene* scene)
{
  if (this->Scene == scene)
    {
    return;
    }
  if (this->Scene)
    {
    this->InvalidateNodeReferences();
    }

  this->Scene = scene;
  if (this->Scene)
    {
    this->SetSceneReferences();
    // We must not call UpdateNodeReferences() here yet, because referenced node IDs may change
    // due to conflict with node IDs existing in the scene.
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNode::AddNodeReferenceRole(const char *refRole, const char *mrmlAttributeName, vtkIntArray *events)
{
  if (!refRole)
    {
    return;
    }
  const std::string referenceRole(refRole);
  this->NodeReferenceMRMLAttributeNames[referenceRole] =
    mrmlAttributeName ? std::string(mrmlAttributeName) : referenceRole;
  if (!this->IsReferenceRoleGeneric(refRole))
    {
    this->NodeReferences[referenceRole] = NodeReferenceListType();
    }

  this->NodeReferenceEvents[referenceRole] = vtkSmartPointer<vtkIntArray>::New();
  if (events)
    {
    for (int i=0; i<events->GetNumberOfTuples(); i++)
      {
      this->NodeReferenceEvents[referenceRole]->InsertNextValue(events->GetValue(i));
      }
    }
}

//----------------------------------------------------------------------------
const char* vtkMRMLNode::GetReferenceRoleFromMRMLAttributeName(const char* attName)
{
  if (attName == nullptr)
    {
    return nullptr;
    }
  std::string attributeName(attName);
  // Search if the attribute name has been registered using AddNodeReferenceRole.
  std::map< std::string, std::string>::iterator it;
  for (it = this->NodeReferenceMRMLAttributeNames.begin();
       it != this->NodeReferenceMRMLAttributeNames.end();
       ++it)
    {
    const std::string& nodeReferenceRole = it->first;
    const std::string& nodeMRMLAttributeName = it->second;
    if (nodeMRMLAttributeName == attributeName)
      {
      return nodeReferenceRole.c_str();
      }
    else if (this->IsReferenceRoleGeneric(nodeReferenceRole.c_str()) &&
             (attributeName.length() >= nodeMRMLAttributeName.length()) &&
             attributeName.compare(
              attributeName.length() - nodeMRMLAttributeName.length(),
              nodeMRMLAttributeName.length(), nodeMRMLAttributeName) == 0)
      {
      // if attName = "lengthUnitRef" and  [refRole,attName] = ["unit/","UnitRef"]
      // then return "unit/length"
      static std::string referenceRole;
      referenceRole = nodeReferenceRole;
      referenceRole += attributeName.substr(0, attributeName.length() -
                                               nodeMRMLAttributeName.length());
      return referenceRole.c_str();
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
const char* vtkMRMLNode
::GetMRMLAttributeNameFromReferenceRole(const char* refRole)
{
  if (refRole == nullptr)
    {
    return nullptr;
    }
  std::string referenceRole(refRole);
  // Try first to see if the reference role is registered as is.
  std::map< std::string, std::string>::const_iterator it =
    this->NodeReferenceMRMLAttributeNames.find(referenceRole);
  if (it != this->NodeReferenceMRMLAttributeNames.end())
    {
    return it->second.c_str();
    }
  // Otherwise, it might be a generic reference role.
  for (it = this->NodeReferenceMRMLAttributeNames.begin();
       it != this->NodeReferenceMRMLAttributeNames.end();
       ++it)
    {
    const std::string& nodeReferenceRole = it->first;
    const std::string& nodeMRMLAttributeName = it->second;
    if (this->IsReferenceRoleGeneric(nodeReferenceRole.c_str()) &&
        referenceRole.compare(0, nodeReferenceRole.length(),
                              nodeReferenceRole) == 0)
      {
      // if refRole = "unit/length" and  [refRole,attName] = ["unit/","UnitRef"]
      // then return "lengthUnitRef"
      static std::string mrmlAttributeName;
      mrmlAttributeName = referenceRole.substr(nodeReferenceRole.length());
      mrmlAttributeName += nodeMRMLAttributeName;
      return mrmlAttributeName.c_str();
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
bool vtkMRMLNode::IsReferenceRoleGeneric(const char* refRole)
{
  if (refRole == nullptr)
    {
    return false;
    }
  std::string nodeReferenceRole(refRole);
  return nodeReferenceRole.at(nodeReferenceRole.length()-1) == '/';
}

//----------------------------------------------------------------------------
void vtkMRMLNode::SetSceneReferences()
{
  if (!this->Scene)
    {
    vtkErrorMacro(<< "SetSceneReferences: Scene is expected to be non nullptr.");
    return;
    }

  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    for (unsigned int i=0; i<it->second.size(); i++)
      {
      vtkMRMLNodeReference* reference = it->second[i];
      if (!reference)
        {
        vtkErrorMacro(<< "SetSceneReferences: Reference " << i << " is expected to be non nullptr.");
        return;
        }
      this->Scene->AddReferencedNodeID(reference->GetReferencedNodeID(), this);
      }
    }
}


//----------------------------------------------------------------------------
void vtkMRMLNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  int wasModifying = this->StartModify();

  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    for (unsigned int i=0; i<it->second.size(); i++)
      {
      vtkMRMLNodeReference* reference = it->second[i];
      if (!reference)
        {
        vtkErrorMacro(<< "UpdateReferenceID: Reference " << i << " is expected to be non nullptr.");
        continue;
        }
      if (std::string(oldID) == std::string(reference->GetReferencedNodeID()))
        {
        this->SetAndObserveNthNodeReferenceID(reference->GetReferenceRole(), i, newID, reference->GetEvents());
        }
      }
    }
  this->EndModify(wasModifying);
}


//----------------------------------------------------------------------------
void vtkMRMLNode::SetAttribute(const char* name, const char* value)
{
  if (!name)
    {
    vtkErrorMacro(<< "SetAttribute: Name parameter is expected to be non nullptr.");
    return;
    }
  if (strlen(name) == 0)
    {
    vtkErrorMacro(<< "SetAttribute: Name parameter is expected to have at least one character.");
    return;
    }
  const char* oldValue = this->GetAttribute(name);
  if ((!oldValue && !value) ||
      (oldValue && value && !strcmp(oldValue, value)))
    {
    return;
    }
  if (value != nullptr)
    {
    this->Attributes[std::string(name)] = std::string(value);
    }
  else
    {
    this->Attributes.erase(std::string(name));
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLNode::RemoveAttribute(const char* name)
{
  if (!name)
    {
    vtkErrorMacro(<< "RemoveAttribute: Name parameter is expected to be non nullptr.");
    return;
    }
  if (strlen(name) == 0)
    {
    vtkErrorMacro(<< "RemoveAttribute: Name parameter is expected to have at least one character.");
    return;
    }
  this->SetAttribute(name, nullptr);
}

//----------------------------------------------------------------------------
const char* vtkMRMLNode::GetAttribute(const char* name)
{
  if (!name)
    {
    vtkErrorMacro(<< "GetAttribute: Name parameter is expected to be non nullptr.");
    return nullptr;
    }
  if (strlen(name) == 0)
    {
    vtkErrorMacro(<< "GetAttribute: Name parameter is expected to have at least one character.");
    return nullptr;
    }
  AttributesType::const_iterator iter =
    this->Attributes.find(std::string(name));
  if (iter == Attributes.end())
    {
    return nullptr;
    }
  else
    {
    return iter->second.c_str();
    }
}

//----------------------------------------------------------------------------
std::vector< std::string > vtkMRMLNode::GetAttributeNames()
{
  std::vector< std::string > attributeNamesVector;
  for ( AttributesType::iterator iter = this->Attributes.begin(); iter != this->Attributes.end(); ++iter )
    {
    attributeNamesVector.push_back(iter->first);
    }
  return attributeNamesVector;
}

//----------------------------------------------------------------------------
void vtkMRMLNode::GetAttributeNames(vtkStringArray* attributeNames)
{
  if (attributeNames == nullptr)
    {
    vtkErrorMacro("vtkMRMLNode::GetAttributeNames: attributeNames is invalid");
    return;
    }
  attributeNames->Reset();
  for (AttributesType::iterator iter = this->Attributes.begin(); iter != this->Attributes.end(); ++iter)
    {
    attributeNames->InsertNextValue(iter->first);
    }
}

//----------------------------------------------------------------------------
// Description:
// the MRMLCallback is a static function to relay modified events from the
// observed mrml node back into the gui layer for further processing
//
void vtkMRMLNode::MRMLCallback(vtkObject *caller,
                               unsigned long eid,
                               void *clientData,
                               void *callData)
{
  vtkMRMLNode *self = reinterpret_cast<vtkMRMLNode *>(clientData);

  if ( self == nullptr )
    {
    //vtkDebugMacro(self, "In vtkMRMLNode *********MRMLCallback called after delete!");
    return;
    }


  if (self->GetInMRMLCallbackFlag())
    {
    vtkDebugWithObjectMacro(self, "In vtkMRMLNode *********MRMLCallback called recursively?");
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkMRMLNode MRMLCallback");

  self->SetInMRMLCallbackFlag(1);
  self->ProcessMRMLEvents(caller, eid, callData);
  self->SetInMRMLCallbackFlag(0);
}

//----------------------------------------------------------------------------
void vtkMRMLNode::SetAddToSceneNoModify(int value)
{
   this->AddToScene = value;
}

//----------------------------------------------------------------------------
void vtkMRMLNode::SetID (const char* _arg)
{
  // Mostly copied from vtkSetStringMacro() in vtkSetGet.cxx
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting ID to " << (_arg?_arg:"(null)") );
  if ( this->ID == nullptr && _arg == nullptr) { return;}
  if ( this->ID && _arg && (!strcmp(this->ID,_arg))) { return;}
  char* oldID = this->ID;
  if (_arg)
    {
    size_t n = strlen(_arg) + 1;
    char *cp1 =  new char[n];
    const char *cp2 = (_arg);
    this->ID = cp1;
    do { *cp1++ = *cp2++; } while ( --n );
    }
   else
    {
    this->ID = nullptr;
    }
  this->InvokeEvent(vtkMRMLNode::IDChangedEvent, oldID);
  if (oldID) { delete [] oldID; }
  this->Modified();
}

//----------------------------------------------------------------------------
const char * vtkMRMLNode::URLEncodeString(const char *inString)
{
  if (inString == nullptr)
    {
    return "(null)";
    }
  if (strcmp(inString, "") == 0)
    {
    return "";
    }

  std::string kwInString = std::string(inString);
  // encode %
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%", "%25");
  // encode space
  vtksys::SystemTools::ReplaceString(kwInString,
                                     " ", "%20");
  // encode single quote
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "'", "%27");
  // encode greater than
  vtksys::SystemTools::ReplaceString(kwInString,
                                     ">", "%3E");
  // encode less than
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "<", "%3C");
  // encode double quote
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "\"", "%22");

  this->DisableModifiedEventOn();
  this->SetTempURLString(kwInString.c_str());
  this->DisableModifiedEventOff();
  return (this->GetTempURLString());
}

//----------------------------------------------------------------------------
const char * vtkMRMLNode::URLDecodeString(const char *inString)
{
  if (inString == nullptr)
    {
    return "(null)";
    }
  if (strcmp(inString, "") == 0)
    {
    return "";
    }
  std::string kwInString = std::string(inString);

  // decode in the opposite order they were encoded in

  // decode double quote
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%22", "\"");
  // decode less than
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%3C", "<");
  // decode greater than
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%3E", ">");
  // decode single quote
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%27", "'");
  // decode space
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%20", " ");
  // decode %
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%25", "%");

  this->DisableModifiedEventOn();
  this->SetTempURLString(kwInString.c_str());
  this->DisableModifiedEventOff();
  return (this->GetTempURLString());
}

//----------------------------------------------------------------------------
std::string vtkMRMLNode::XMLAttributeEncodeString(const std::string& inString)
{
  std::string outString = inString;
  vtksys::SystemTools::ReplaceString(outString, "&", "&amp;");
  vtksys::SystemTools::ReplaceString(outString, "\"", "&quot;");
  vtksys::SystemTools::ReplaceString(outString, "'", "&apos;");
  vtksys::SystemTools::ReplaceString(outString, "<", "&lt;");
  vtksys::SystemTools::ReplaceString(outString, ">", "&gt;");
  return outString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLNode::XMLAttributeDecodeString(const std::string& inString)
{
  std::string outString = inString;
  vtksys::SystemTools::ReplaceString(outString, "&quot;", "\"");
  vtksys::SystemTools::ReplaceString(outString, "&apos;", "'");
  vtksys::SystemTools::ReplaceString(outString, "&lt;", "<");
  vtksys::SystemTools::ReplaceString(outString, "&gt;", ">");
  vtksys::SystemTools::ReplaceString(outString, "&amp;", "&");
  return outString;
}

//// Reference API

//-----------------------------------------------------------
void vtkMRMLNode::UpdateReferences()
{
  if (!this->Scene)
    {
    return;
    }
  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    for (unsigned int i=0; i<it->second.size();)
      {
      vtkMRMLNodeReference* reference = it->second[i];
      if (reference->GetReferencedNodeID() &&
          std::string(reference->GetReferencedNodeID()) != "" &&
          this->Scene->GetNodeByID(reference->GetReferencedNodeID()) == nullptr)
        {
        this->RemoveNthNodeReferenceID(reference->GetReferenceRole(), i);
        }
      else
        {
        ++i;
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNode::RemoveNodeReferenceIDs(const char* referenceRole)
{
  if (!referenceRole)
    {
    int wasModifying = this->StartModify();
    NodeReferencesType::iterator it;
    for (it=this->NodeReferences.begin(); it!=this->NodeReferences.end(); it++)
      {
      if (it->first.c_str())
        {
        this->RemoveNodeReferenceIDs(it->first.c_str());
        }
      }
    this->EndModify(wasModifying);
    return;
    }

  int wasModifying = this->StartModify();
  while(this->GetNumberOfNodeReferences(referenceRole) > 0)
    {
    this->RemoveNthNodeReferenceID(referenceRole, 0);
    }
  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLNode::GetNodeReferenceRoles(std::vector<std::string> &roles)
{
  roles.clear();
  for (NodeReferencesType::iterator roleIt = this->NodeReferences.begin(); roleIt != this->NodeReferences.end(); roleIt++)
    {
    roles.push_back(roleIt->first);
    }
}

//----------------------------------------------------------------------------
int vtkMRMLNode::GetNumberOfNodeReferenceRoles()
{
  return this->NodeReferences.size();
}

//----------------------------------------------------------------------------
const char* vtkMRMLNode::GetNthNodeReferenceRole(int n)
{
  if (n < 0 || n >= static_cast<int>(this->NodeReferences.size()))
    {
    vtkErrorMacro("vtkMRMLNode::GetNthNodeReferenceRole failed: n=" << n << " is out of range");
    return nullptr;
    }
  NodeReferencesType::iterator roleIt( this->NodeReferences.begin() );
  std::advance( roleIt, n );
  return roleIt->first.c_str();
}

//----------------------------------------------------------------------------
void vtkMRMLNode::GetNodeReferences(const char* referenceRole, std::vector<vtkMRMLNode*> &nodes)
{
  if (referenceRole)
    {
    this->UpdateNodeReferences(referenceRole);
    NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
    for (unsigned int i=0; i<references.size(); i++)
      {
      nodes.push_back(references[i]->GetReferencedNode());
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNode::GetNodeReferenceIDs(const char* referenceRole,
                                      std::vector<const char*> &referencedNodeIDs)
{
  if (!referenceRole)
    {
    return;
    }

  NodeReferenceListType &references =
    this->NodeReferences[std::string(referenceRole)];
  for (unsigned int i=0; i<references.size(); ++i)
    {
    referencedNodeIDs.push_back(references[i] ? references[i]->GetReferencedNodeID() : nullptr);
    }
}


//----------------------------------------------------------------------------
const char * vtkMRMLNode::GetNthNodeReferenceID(const char* referenceRole, int n)
{
  if (!referenceRole || n < 0)
    {
    return nullptr;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
  if (n >= static_cast<int>(references.size()))
    {
    return nullptr;
    }
  if (!references[n])
    {
    vtkErrorMacro(<< "GetNthNodeReferenceID: Reference " << n << "should NOT be nullptr.");
    return nullptr;
    }
  return references[n]->GetReferencedNodeID();
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::GetNthNodeReference(const char* referenceRole, int n)
{

  if (!referenceRole || n < 0 )
    {
    return nullptr;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
  if (n >= static_cast<int>(references.size()))
    {
    return nullptr;
    }

  vtkMRMLNode* node = references[n]->GetReferencedNode();
  // Maybe the node was not yet in the scene when the node ID was set.
  // Check to see if it's now there.
  // Similarly, if the scene is 0, clear the node if not already null.
  if ((!node || node->GetScene() != this->GetScene()) ||
      (node && this->GetScene() == nullptr))
    {
    this->UpdateNthNodeReference(referenceRole, n);
    NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
    node = references[n]->GetReferencedNode();
    }
  return node;
}

//-----------------------------------------------------------
void vtkMRMLNode::UpdateNodeReferences(const char* referenceRole)
{
  if (referenceRole==nullptr)
    {
    // update all roles
    int wasModifying = this->StartModify();
    NodeReferencesType::iterator it;
    for (it=this->NodeReferences.begin(); it!=this->NodeReferences.end(); it++)
      {
      if (it->first.c_str())
        {
        this->UpdateNodeReferences(it->first.c_str());
        }
      }
    this->EndModify(wasModifying);
    return;
    }

  int wasModifying = this->StartModify();
  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
  for (unsigned int i=0; i<references.size(); i++)
    {
    this->UpdateNthNodeReference(referenceRole, i);
    }
  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLNode::UpdateNthNodeReference(const char* referenceRole, int n)
{
  if (!referenceRole)
    {
    vtkErrorMacro(<< "UpdateNthNodeReference: Non-null role is expected.");
    return;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];

  if (n >= static_cast<int>(references.size()))
    {
    vtkErrorMacro(<< "UpdateNthNodeReference: n is " << n << "."
        << " Value is expected to be smaller than " << references.size());
    return;
    }

  this->SetAndObserveNthNodeReferenceID(references[n]->GetReferenceRole(), n, references[n]->GetReferencedNodeID(), references[n]->GetEvents());
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::SetNthNodeReferenceID(const char* referenceRole,
                                                 int n,
                                                 const char* referencedNodeID)
{
  return SetAndObserveNthNodeReferenceID(referenceRole, n, referencedNodeID, nullptr);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::SetAndObserveNthNodeReferenceID(const char* referenceRole,
                                                           int n,
                                                           const char* referencedNodeID,
                                                           vtkIntArray *events)
{
  if (!referenceRole)
    {
    vtkErrorMacro(<< "SetAndObserveNthNodeReferenceID: Non-null role is expected.");
    return nullptr;
    }
  if (n < 0)
    {
    vtkErrorMacro(<< "SetAndObserveNthNodeReferenceID: Non-negative reference index is expected.");
    return nullptr;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];

  vtkMRMLNodeReference* oldReference = nullptr;
  vtkMRMLNode* oldReferencedNode = nullptr;
  std::string oldReferencedNodeID;
  NodeReferenceListType::iterator referenceIt=references.end();
  if (n < static_cast<int>(references.size()))
    {
    referenceIt = references.begin()+n;
    oldReference = referenceIt->GetPointer();
    oldReferencedNode = oldReference->GetReferencedNode();
    oldReferencedNodeID = oldReference->GetReferencedNodeID();
    }

  vtkMRMLNode* referencedNode = nullptr;
  if (referencedNodeID && strlen(referencedNodeID)>0)
    {
    // Add/update reference
    if (this->Scene)
      {
      referencedNode = this->Scene->GetNodeByID(referencedNodeID);
      }

    if (referenceIt==references.end())
      {
      vtkNew<vtkMRMLNodeReference> reference;
      reference->SetReferencingNode(this);
      reference->SetReferenceRole(referenceRole);
      references.push_back(reference.GetPointer());
      referenceIt = references.begin() + (references.size()-1);
      }

    // Update node observations
    // If no events are specified then use the default events specified for the role
    if (events == nullptr && this->NodeReferenceEvents[referenceRole] && this->NodeReferenceEvents[referenceRole]->GetNumberOfTuples() > 0)
      {
      events = this->NodeReferenceEvents[referenceRole];
      }
    this->UpdateNodeReferenceEventObserver(oldReferencedNode, referencedNode, events, referenceIt->GetPointer());

    (*referenceIt)->SetReferencedNodeID(referencedNodeID);
    (*referenceIt)->SetReferencedNode(referencedNode);
    (*referenceIt)->SetEvents(events);

    if (oldReferencedNode==nullptr && referencedNode != nullptr)
      {
      this->OnNodeReferenceAdded(referenceIt->GetPointer());
      }
    else if (oldReferencedNode!=nullptr && referencedNode == nullptr)
      {
      this->OnNodeReferenceRemoved(referenceIt->GetPointer());
      }
    else if (oldReferencedNode!=referencedNode)
      {
      this->OnNodeReferenceModified(referenceIt->GetPointer());
      }

    }
  else
    {
    // Delete reference
    if (referenceIt!=references.end())
      {
      this->UpdateNodeReferenceEventObserver(oldReferencedNode, nullptr, nullptr, oldReference);
      vtkSmartPointer<vtkMRMLNodeReference> nodeRefToDelete = referenceIt->GetPointer();
      references.erase(referenceIt);

      if (oldReferencedNode != nullptr)
        {
        this->OnNodeReferenceRemoved(nodeRefToDelete);
        }

      // Already removed the ReferencedNode reference by calling UpdateNodeReferenceEventObserver,
      // so we have to set ReferencedNode to nullptr to avoid removing the reference again in the
      // reference's destructor.
      nodeRefToDelete->SetReferencedNode(nullptr);
      }
    }

  // Update scene node references
  std::string newReferencedNodeID = (referencedNodeID ? referencedNodeID : "");
  if (oldReferencedNodeID!=newReferencedNodeID)
    {
    this->Modified();
    }
  if (this->Scene && oldReferencedNodeID!=newReferencedNodeID)
    {
    if (!oldReferencedNodeID.empty() && !this->HasNodeReferenceID(nullptr, oldReferencedNodeID.c_str()))
      {
      // the old referenced node ID is not used anymore by any node references
      this->Scene->RemoveReferencedNodeID(oldReferencedNodeID.c_str(), this);
      }
    if (!newReferencedNodeID.empty())
      {
      // The new node may or may not be referenced by the scene.
      // Let the scene know that it is not referenced.
      // It is not a problem if the referenced node ID is already added (the scene then ignores the call).
      this->Scene->AddReferencedNodeID(newReferencedNodeID.c_str(), this);
      }
    }

  return referencedNode;
}

//----------------------------------------------------------------------------
void vtkMRMLNode::GetUpdatedReferencedNodeEventList(int& oldReferencedNodeUseCount, int& newReferencedNodeUseCount,
  vtkIntArray* oldConsolidatedEventList, vtkIntArray* newConsolidatedEventList,
  vtkMRMLNode* oldReferencedNode, vtkMRMLNode* newReferencedNode, vtkMRMLNodeReference* referenceToIgnore, vtkIntArray* newEvents)
{
  oldReferencedNodeUseCount = 0; // only computed if referencedNode is not the same as the old one
  newReferencedNodeUseCount = 0;
  oldConsolidatedEventList->SetNumberOfTuples(0);
  newConsolidatedEventList->SetNumberOfTuples(0);
  if (oldReferencedNode == nullptr && newReferencedNode == nullptr)
    {
    return;
    }

  std::vector<int> oldReferencedNodeEvents; // only computed if referencedNode is not the same as the old one
  std::vector<int> newReferencedNodeEvents;
  for (NodeReferencesType::iterator roleIt = this->NodeReferences.begin(); roleIt != this->NodeReferences.end(); roleIt++)
    {
    for (NodeReferenceListType::iterator it = roleIt->second.begin(); it != roleIt->second.end(); it++)
      {
      vtkMRMLNodeReference* reference = it->GetPointer();
      if (!reference->GetReferencedNodeID() || strlen(reference->GetReferencedNodeID()) == 0)
        {
        // deleted reference
        continue;
        }
      if (reference == referenceToIgnore)
        {
        // ignore the reference that we are processing right now
        continue;
        }
      if (newReferencedNode && reference->GetReferencedNode() == newReferencedNode)
        {
        newReferencedNodeUseCount++;
        vtkIntArray* events = reference->GetEvents();
        if (events)
          {
          int eventCount = events->GetNumberOfTuples();
          for (int i=0; i<eventCount; i++)
            {
            newReferencedNodeEvents.push_back(events->GetValue(i));
            }
          }
        }
      if (oldReferencedNode == newReferencedNode)
        {
        // don't compute events and uses of old and new referenced nodes separately
        // if they are the same
        continue;
        }
      if (oldReferencedNode && reference->GetReferencedNode() == oldReferencedNode)
        {
        oldReferencedNodeUseCount++;
        vtkIntArray* events = reference->GetEvents();
        if (events)
          {
          int eventCount = events->GetNumberOfTuples();
          for (int i=0; i<eventCount; i++)
            {
            oldReferencedNodeEvents.push_back(events->GetValue(i));
            }
          }
        }
      } // references
    } // roles

  // Determine the consolidated new event list
  if (newReferencedNode != nullptr)
    {
    // Add new events
    if (newEvents)
      {
      int eventCount = newEvents->GetNumberOfTuples();
      for (int i=0; i<eventCount; i++)
        {
        newReferencedNodeEvents.push_back(newEvents->GetValue(i));
        }
      }
    // Standardize the event list (sort it and remove duplicates)
    std::sort(newReferencedNodeEvents.begin(),newReferencedNodeEvents.end());
    std::vector<int>::iterator lastUnique = std::unique(newReferencedNodeEvents.begin(), newReferencedNodeEvents.end());
    newReferencedNodeEvents.erase(lastUnique, newReferencedNodeEvents.end());
    // Convert consolidated event list to vtkIntArray
    for (std::vector<int>::iterator it=newReferencedNodeEvents.begin(); it!=newReferencedNodeEvents.end(); ++it)
      {
      newConsolidatedEventList->InsertNextValue(*it);
      }
    }

  // Determine the consolidated old event list
  if (oldReferencedNode != nullptr && oldReferencedNode != newReferencedNode)
    {
    // Standardize the event list (sort it and remove duplicates)
    std::sort(oldReferencedNodeEvents.begin(),oldReferencedNodeEvents.end());
    std::vector<int>::iterator lastUnique = std::unique(oldReferencedNodeEvents.begin(), oldReferencedNodeEvents.end());
    oldReferencedNodeEvents.erase(lastUnique, oldReferencedNodeEvents.end());
    for (std::vector<int>::iterator it=oldReferencedNodeEvents.begin(); it!=oldReferencedNodeEvents.end(); ++it)
      {
      oldConsolidatedEventList->InsertNextValue(*it);
      }
    }
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::UpdateNodeReferenceEventObserver(vtkMRMLNode *oldReferencedNode, vtkMRMLNode *newReferencedNode, vtkIntArray *newEvents, vtkMRMLNodeReference* referenceToIgnore)
{
  if (oldReferencedNode == nullptr && newReferencedNode == nullptr)
    {
    // both old and new references are nullptr, there is nothing to do
    return nullptr;
    }

  std::string oldReferencedNodeID;
  if (oldReferencedNode != nullptr && oldReferencedNode->GetID() != nullptr)
    {
    oldReferencedNodeID = oldReferencedNode->GetID();
    }

  std::string newReferencedNodeID;
  if (newReferencedNode != nullptr && newReferencedNode->GetID() != nullptr)
    {
    newReferencedNodeID = newReferencedNode->GetID();
    }

  // Get consolidated event counts and references to the old and new referenced node
  int oldReferencedNodeUseCount = 0; // only computed if referencedNode is not the same as the old one
  int newReferencedNodeUseCount = 0;
  vtkNew<vtkIntArray> oldConsolidatedEventList; // only computed if referencedNode is not the same as the old one
  vtkNew<vtkIntArray> newConsolidatedEventList;
  this->GetUpdatedReferencedNodeEventList(oldReferencedNodeUseCount, newReferencedNodeUseCount,
    oldConsolidatedEventList.GetPointer(), newConsolidatedEventList.GetPointer(),
    oldReferencedNode, newReferencedNode, referenceToIgnore, newEvents);

  // Update events
  if (oldReferencedNode==newReferencedNode)
    {
    // Referenced node not changed: only update the events
    // The same events may be already observed by other nodes, so suppress the warning if there are no changes in the event list.
    vtkSetAndObserveMRMLObjectEventsMacroNoWarning(newReferencedNode, newReferencedNode, newConsolidatedEventList.GetPointer()); // update the event list
    }
  else
    {
    // Update events of the old node
    if (oldReferencedNode != nullptr)
      {
      if (oldReferencedNodeUseCount==0)
        {
        // This was the last reference that used the oldReferencedNode node: remove all event observers and unregister
        vtkMRMLNode* oldReferencedNodeCopy = oldReferencedNode; // make a copy of oldReferencedNode (vtkSetAndObserveMRMLObjectMacro would overwrite the value with nullptr)
        vtkSetAndObserveMRMLObjectMacro(oldReferencedNodeCopy, nullptr); // unregister & remove events
        }
      else
        {
        // This reference does not use oldReferencedNode anymore but other references still use it:
        // update the event list and unregister
        vtkSetAndObserveMRMLObjectEventsMacroNoWarning(oldReferencedNode, oldReferencedNode, oldConsolidatedEventList.GetPointer()); // update the event list
        oldReferencedNode->UnRegister(this->MRMLObserverManager); // unregister
        }
      }
    // Update events of the new node
    if (newReferencedNode != nullptr)
      {
      // This reference is now using a new node: register with an updated event list
      vtkMRMLNode* dummyNullReferencedNode = nullptr; // forces registration
      // the same events may be already observed by other nodes, so suppress the warning if there are no changes in the event list
      vtkSetAndObserveMRMLObjectEventsMacroNoWarning(dummyNullReferencedNode, newReferencedNode, newConsolidatedEventList.GetPointer()); // update the event list & register
      }
    }

  return newReferencedNode;
}

//----------------------------------------------------------------------------
bool vtkMRMLNode::HasNodeReferenceID(const char* referenceRole, const char* referencedNodeID)
{
  if (!referencedNodeID)
    {
    return false;
    }

  if (!referenceRole)
    {
    NodeReferencesType::iterator it;
    for (it=this->NodeReferences.begin(); it!=this->NodeReferences.end(); it++)
      {
      if (it->first.c_str())
        {
        if (this->HasNodeReferenceID(it->first.c_str(), referencedNodeID))
          {
          return true;
          }
        }
      }
    return false;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
  NodeReferenceListType::iterator it;
  std::string sID(referencedNodeID);
  for (it=references.begin(); it!=references.end(); it++)
    {
    vtkMRMLNodeReference* reference = *it;
    if (sID == std::string(reference->GetReferencedNodeID()))
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void vtkMRMLNode::InvalidateNodeReferences()
{
  // Remove the referenced node pointers, but keep the IDs and events
  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    NodeReferenceListType::iterator it1;
    for (it1 = it->second.begin(); it1 != it->second.end(); it1++)
      {
      vtkMRMLNodeReference* reference = *it1;
      if (reference->GetReferencedNode())
        {
        vtkMRMLNode *nodePtr = reference->GetReferencedNode(); // vtkSetAndObserveMRMLObjectMacro overwrites the argument, so we need to make a copy
        vtkSetAndObserveMRMLObjectMacro(nodePtr, nullptr);
        reference->SetReferencedNode(nullptr);
        }
      }
    }
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::SetNodeReferenceID(const char* referenceRole, const char *referencedNodeID)
{
  return this->SetNthNodeReferenceID(referenceRole, 0, referencedNodeID);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::AddNodeReferenceID(const char* referenceRole, const char *referencedNodeID)
{
  return this->SetNthNodeReferenceID(referenceRole, this->GetNumberOfNodeReferences(referenceRole), referencedNodeID);
}

//----------------------------------------------------------------------------
const char * vtkMRMLNode::GetNodeReferenceID(const char* referenceRole)
{
  return this->GetNthNodeReferenceID(referenceRole, 0);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::GetNodeReference(const char* referenceRole)
{
  return this->GetNthNodeReference(referenceRole, 0);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::SetAndObserveNodeReferenceID(const char* referenceRole, const char *referencedNodeID, vtkIntArray *events)
{
  return this->SetAndObserveNthNodeReferenceID(referenceRole, 0, referencedNodeID, events);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::AddAndObserveNodeReferenceID(const char* referenceRole, const char *referencedNodeID, vtkIntArray *events)
{
  return this->SetAndObserveNthNodeReferenceID(referenceRole, this->GetNumberOfNodeReferences(referenceRole), referencedNodeID, events);
}

//----------------------------------------------------------------------------
void vtkMRMLNode::RemoveNthNodeReferenceID(const char* referenceRole, int n)
{
  this->SetAndObserveNthNodeReferenceID(referenceRole, n, nullptr);
}

//----------------------------------------------------------------------------
int vtkMRMLNode::GetNumberOfNodeReferences(const char* referenceRole)
{
  int n=0;
  if (referenceRole)
    {
    NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
    NodeReferenceListType::iterator it;
    for (it = references.begin(); it != references.end(); it++)
      {
      vtkMRMLNodeReference* reference = *it;
      if (reference->GetReferencedNodeID() && strlen(reference->GetReferencedNodeID()) > 0)
        {
        n++;
        }
      }
    }
  return n;
}


//----------------------------------------------------------------------------
vtkMRMLNode::vtkMRMLNodeReference* vtkMRMLNode::vtkMRMLNodeReference::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLNodeReference");
  if(ret)
    {
    return static_cast<vtkMRMLNode::vtkMRMLNodeReference*>(ret);
    }

  vtkMRMLNode::vtkMRMLNodeReference* result = new vtkMRMLNode::vtkMRMLNodeReference;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
  result->InitializeObjectBase();
#endif
  return result;
}

//----------------------------------------------------------------------------
vtkMRMLNode::vtkMRMLNodeReference::vtkMRMLNodeReference()
{
  this->ReferencedNodeID = nullptr;
  this->ReferenceRole = nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLNode::vtkMRMLNodeReference::~vtkMRMLNodeReference()
{
  if (this->GetReferencedNode() != nullptr)
    {
    // The referenced node has to be nullptr before getting to this destructor.
    // We cannot properly clean up the referenced node in the destructor, because the referencing node may be already nullptr and
    // we don't know which event observers we would need to remove. Therefore, we just report the error to allow easier debugging.
    const char* referencedNodeId = this->ReferencedNode->GetID() ? this->ReferencedNode->GetID() : "(unknown)";
    const char* referencingNodeId = (this->ReferencingNode.GetPointer() && this->ReferencingNode->GetID()) ? this->ReferencingNode->GetID() : "(unknown)";
    vtkWarningMacro("While deleting a reference object an active node reference is found to node "<<referencedNodeId<<" from node "<<referencingNodeId<<". Remaining references and observations may cause memory leaks.");
    }
  SetReferencedNodeID(nullptr);
  SetReferenceRole(nullptr);
}

//----------------------------------------------------------------------------
vtkIntArray* vtkMRMLNode::vtkMRMLNodeReference::GetEvents() const
{
  return this->Events.GetPointer();
}

//----------------------------------------------------------------------------
void vtkMRMLNode::vtkMRMLNodeReference::SetEvents(vtkIntArray* events)
{
  // The events are stored and used sometime later (when the references are updated).
  // Make a copy of the events to make sure the current values are used (and not the values that are current at the time of node reference update).
  if (events==this->Events)
    {
    // no change
    return;
    }
  if (events)
    {
    if (!this->Events.GetPointer())
      {
      this->Events = vtkSmartPointer<vtkIntArray>::New();
      }
    this->Events->DeepCopy(events);
    }
  else
    {
    this->Events=nullptr;
    }
  Modified();
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::vtkMRMLNodeReference::GetReferencingNode() const
{
  return this->ReferencingNode.GetPointer();
}

//----------------------------------------------------------------------------
void vtkMRMLNode::vtkMRMLNodeReference::SetReferencingNode(vtkMRMLNode* node)
{
  if (this->ReferencingNode.GetPointer() == node)
  {
    return;
  }
  this->ReferencingNode = node;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::vtkMRMLNodeReference::GetReferencedNode() const
{
  return this->ReferencedNode.GetPointer();
}

//----------------------------------------------------------------------------
void vtkMRMLNode::vtkMRMLNodeReference::SetReferencedNode(vtkMRMLNode* node)
{
  if (this->ReferencedNode.GetPointer() == node)
  {
    return;
  }
  this->ReferencedNode = node;
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkMRMLNode::HasCopyContent() const
{
  // This may be overridden in derived classes by placing vtkMRMLNodeHasCopyContentMacro
  // in the header.
  return strcmp("vtkMRMLNode", this->GetClassNameInternal()) == 0;
}
