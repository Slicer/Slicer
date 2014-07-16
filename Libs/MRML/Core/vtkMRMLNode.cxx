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

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <iostream>
#include <sstream>


//------------------------------------------------------------------------------
vtkMRMLNode::vtkMRMLNode()
{
  this->ID = NULL;

  // By default nodes have no effect on indentation
  this->Indent = 0;

  // Strings
  this->Description = NULL;

  // By default all MRML nodes have a blank name
  // Must set name to NULL first so that the SetName
  // macro will not free memory.
  this->Name = NULL;

  this->SingletonTag = NULL;

  this->SceneRootDir = NULL;
  this->Scene = NULL;

  this->HideFromEditors = 0;
  this->Selectable = 1;
  this->Selected = 0;

  this->AddToScene = 1;

  this->DisableModifiedEvent = 0;
  this->ModifiedEventPending = 0;

  // Set up callbacks
  this->MRMLCallbackCommand = vtkCallbackCommand::New ( );
  this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->MRMLCallbackCommand->SetCallback( vtkMRMLNode::MRMLCallback );
  this->InMRMLCallbackFlag = 0;
  this->SaveWithScene = true;

  this->MRMLObserverManager = vtkObserverManager::New();
  this->MRMLObserverManager->AssignOwner( this );
  this->MRMLObserverManager->GetCallbackCommand()->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLObserverManager->GetCallbackCommand()->SetCallback(vtkMRMLNode::MRMLCallback);


  this->TempURLString = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLNode::~vtkMRMLNode()
{
  if (this->Description)
    {
    delete [] this->Description;
    this->Description = NULL;
    }
  if (this->Name)
    {
    delete [] this->Name;
    this->Name = NULL;
    }
  if (this->ID)
    {
    delete [] this->ID;
    this->ID = NULL;
    }
  if (this->SceneRootDir)
    {
    delete [] this->SceneRootDir;
    this->SceneRootDir = NULL;
    }
  if (this->MRMLObserverManager)
    {
    this->MRMLObserverManager->Delete();
    }

  // unregister and set null pointers.
  if ( this->MRMLCallbackCommand )
    {
    this->MRMLCallbackCommand->SetClientData( NULL );
    this->MRMLCallbackCommand->Delete ( );
    this->MRMLCallbackCommand = NULL;
    }

  if (this->TempURLString)
    {
    delete [] this->TempURLString;
    this->TempURLString = NULL;
    }

  if (this->SingletonTag)
    {
    this->SetSingletonTag(NULL);
    }

  this->DeleteAllReferences();
}

//----------------------------------------------------------------------------
void vtkMRMLNode::DeleteAllReferences(bool callOnNodeReferenceRemoved)
{
  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    NodeReferenceListType::iterator it1;
    for (it1 = it->second.begin(); it1 != it->second.end(); it1++)
      {
      vtkMRMLNodeReference* reference = *it1;
      if (callOnNodeReferenceRemoved)
        {
        this->OnNodeReferenceRemoved(reference);
        }
      }
    }
  this->NodeReferences.clear();
  this->NodeReferenceEvents.clear();
}

//----------------------------------------------------------------------------
void vtkMRMLNode::CopyWithScene(vtkMRMLNode *node)
{
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
  int disabledModify = this->StartModify();

  if (node->GetName() && strcmp(node->GetName(),""))
    {
    this->SetName(node->GetName());
    }
  this->SetHideFromEditors( node->HideFromEditors );
  this->SetSaveWithScene( node->SaveWithScene );
  this->SetSelectable( node->Selectable );
  this->SetAddToScene( node->AddToScene );

  if (node->GetSingletonTag())
    {
    this->SetSingletonTag( node->GetSingletonTag() );
    }
  this->SetDescription(node->GetDescription());
  this->Attributes = node->Attributes;

  this->CopyReferences(node);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLNode::CopyReferences(vtkMRMLNode* node)
{
  // Remove references if not existing in the node to copy
  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end();)
    {
    if (node->NodeReferences.find(it->first) == node->NodeReferences.end())
      {
      const std::string referenceRoleToRemove = it->first;
      this->RemoveAllNodeReferenceIDs(referenceRoleToRemove.c_str());
      NodeReferencesType::iterator nextIt = it;
      ++nextIt;
      this->NodeReferences.erase(it);
      it = nextIt;
      }
    else
      {
      ++it;
      }
    }

  this->NodeReferenceMRMLAttributeNames = node->NodeReferenceMRMLAttributeNames;

  // Add or replace node references
  for (it = node->NodeReferences.begin(); it != node->NodeReferences.end(); it++)
    {
    std::string referenceRole = it->first;
    NodeReferenceListType::iterator it1;
    for (it1 = it->second.begin(); it1 != it->second.end(); it1++)
      {
      vtkMRMLNodeReference* reference = *it1;
      if (!reference)
        {
        vtkErrorMacro(<< "CopyReferences: Reference is expected to be non NULL.");
        return;
        }
      this->SetNthNodeReferenceID(referenceRole.c_str(),
                                  std::distance(it->second.begin(), it1),
                                  reference->GetReferencedNodeID());
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNode::Reset()
{
  vtkMRMLNode *newNode = this->CreateNodeInstance();

  int save = this->GetSaveWithScene();
  int hide = this->GetHideFromEditors();
  int select = this->GetSelectable();
  char *tag = this->GetSingletonTag();

  int wasModifying = this->StartModify();
  this->CopyWithScene(newNode);

  this->SetSaveWithScene(save);
  this->SetHideFromEditors(hide);
  this->SetSelectable(select);
  this->SetSingletonTag(tag);
  this->EndModify(wasModifying);

  newNode->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os,indent);

  os << indent << "ID: " <<
    (this->ID ? this->ID : "(none)") << "\n";

  os << indent << "Indent:      " << this->Indent << "\n";

  os << indent << "Name: " <<
    (this->Name ? this->Name : "(none)") << "\n";

  os << indent << "Description: " <<
    (this->Description ? this->Description : "(none)") << "\n";

  os << indent << "SingletonTag: " <<
    (this->SingletonTag ? this->SingletonTag : "(none)") << "\n";

  os << indent << "HideFromEditors: " << this->HideFromEditors << "\n";

  os << indent << "Selectable: " << this->Selectable << "\n";
  os << indent << "Selected: " << this->Selected << "\n";
  if (this->Attributes.size())
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

  //print node references
  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    const std::string& referenceRole = it->first;
    const char* refAttribute =
      this->GetMRMLAttributeNameFromReferenceRole(referenceRole.c_str());
    if (refAttribute == 0)
      {
      continue;
      }
    const std::string referenceMRMLAttributeName(refAttribute);
    std::stringstream ss;
    int numReferencedNodes = this->GetNumberOfNodeReferences(referenceRole.c_str());

    for (int n=0; n < numReferencedNodes; n++)
      {
      const char * id = this->GetNthNodeReferenceID(referenceRole.c_str(), n);

      ss << id;
      if (n < numReferencedNodes-1)
        {
        ss << " ";
        }
      }
    if (numReferencedNodes > 0)
      {
      os << indent << referenceMRMLAttributeName << "=\""
         << ss.str().c_str() << "\"" << "\n";
      }
    }

}

//----------------------------------------------------------------------------
void vtkMRMLNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  std::map<std::string, std::string> references;

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "id"))
      {
      this->SetID(attValue);
      }
    else if (!strcmp(attName, "name"))
      {
      this->SetName(attValue);
      }
    else if (!strcmp(attName, "description"))
      {
      this->SetDescription(attValue);
      }
    else if (!strcmp(attName, "hideFromEditors"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->HideFromEditors = 1;
        }
      else
        {
        this->HideFromEditors = 0;
        }
      }
    else if (!strcmp(attName, "selectable"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Selectable = 1;
        }
      else
        {
        this->Selectable = 0;
        }
      }
     else if (!strcmp(attName, "selected"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Selected = 1;
        }
      else
        {
        this->Selected = 0;
        }
      }
     else if (!strcmp(attName, "attributes"))
       {
       std::stringstream attributes(attValue);
       std::string attribute;
       while (std::getline(attributes, attribute, ';'))
         {
         int colonIndex = attribute.find(':');
         std::string name = attribute.substr(0, colonIndex);
         std::string value = attribute.substr(colonIndex + 1);
         this->SetAttribute(name.c_str(), value.c_str());
         }
       }
     else if (!strcmp(attName, "references"))
       {
       this->ParseReferencesAttribute(attValue, references);
       }
     else if ( const char* referenceRole =
                 this->GetReferenceRoleFromMRMLAttributeName(attName) )
       {
       std::stringstream ss(attValue);
       while (!ss.eof())
         {
         std::string id;
         ss >> id;
         if (!id.empty())
           {
           if (references.find(id) == references.end() ||
             references.find(id)->second != referenceRole)
             {
             this->AddNodeReferenceID(referenceRole, id.c_str());
             references[id] = std::string(referenceRole);
             }
           }
         }
       }
    }
  this->EndModify(disabledModify);

  return;
}


//----------------------------------------------------------------------------
void vtkMRMLNode::ParseReferencesAttribute(const char *attValue,
                                           std::map<std::string, std::string> &references)
{
  /// parse references in the form "role1:id1 id2;role2:id3;"
  std::string attribute(attValue);

  std::size_t start = 0;
  std::size_t end = attribute.find_first_of(';', start);
  std::size_t sep = attribute.find_first_of(':', start);
  while (start != std::string::npos && sep != std::string::npos && start != end && start != sep)
    {
    std::string ref = attribute.substr(start, end-start);
    std::string role = attribute.substr(start, sep-start);
    std::string ids = attribute.substr(sep+1, end-sep-1);
    std::stringstream ss(ids);
    while (!ss.eof())
      {
      std::string id;
      ss >> id;
      if (!id.empty())
        {
        if (references.find(id) == references.end() ||
          references.find(id)->second != role )
          {
          this->AddNodeReferenceID(role.c_str(), id.c_str());
          references[id] = role;
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
  if (this->ID != NULL)
    {
    of << indent << " id=\"" << this->ID << "\"";
    }
  if (this->Name != NULL)
    {
    of << indent << " name=\"" << this->Name << "\"";
    }
  if (this->Description != NULL)
    {
    of << indent << " description=\"" << this->Description << "\"";
    }
  of << indent << " hideFromEditors=\"" << (this->HideFromEditors ? "true" : "false") << "\"";

  of << indent << " selectable=\"" << (this->Selectable ? "true" : "false") << "\"";
  of << indent << " selected=\"" << (this->Selected ? "true" : "false") << "\"";
  if (this->Attributes.size())
    {
    of << indent << " attributes=\"";
    AttributesType::const_iterator it;
    AttributesType::const_iterator begin = this->Attributes.begin();
    AttributesType::const_iterator end = this->Attributes.end();
    for (it = begin; it != end; ++it)
      {
      if (it != begin)
        {
        of << ';';
        }
      of << it->first << ':' << it->second;
      }
    of << "\"";
    }

  //write node references
  std::stringstream ssRef;
  NodeReferencesType::iterator it;
  std::map< std::string, std::string>::iterator itName;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    std::string referenceMRMLAttributeName;
    const std::string& referenceRole = it->first;
    const char *attName = this->GetMRMLAttributeNameFromReferenceRole(referenceRole.c_str());
    if (attName != 0)
      {
      referenceMRMLAttributeName = std::string(attName);
      }
    std::stringstream ss;

    int numReferencedNodes = this->GetNumberOfNodeReferences(referenceRole.c_str());

    if (numReferencedNodes > 0)
      {
      ssRef << referenceRole << ":";
      }

    for (int n=0; n < numReferencedNodes; n++)
      {
      const char * id = this->GetNthNodeReferenceID(referenceRole.c_str(), n);

      ss << id;
      ssRef << id;
      if (n < numReferencedNodes-1)
        {
        ss << " ";
        ssRef << " ";
        }
      }
    if (numReferencedNodes > 0)
      {
      if (referenceMRMLAttributeName.length() > 0)
        {
        of << indent << " " << referenceMRMLAttributeName << "=\"" << ss.str().c_str() << "\"";
        }
      ssRef << ";";
      }
    }//for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)

    if (!(ssRef.str().empty()))
      {
      of << indent << " " << "references=\"" << ssRef.str().c_str() << "\"";
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

  NodeReferencesType::iterator it;
  for (it=this->NodeReferences.begin(); it!=this->NodeReferences.end(); it++)
    {
    if (it->first.c_str())
      {
      NodeReferenceListType references = this->NodeReferences[it->first];
      for (unsigned int i=0; i<references.size(); i++)
        {
        vtkMRMLNode *node = this->GetNthNodeReference(it->first.c_str(), i);
        if (node != NULL && node == vtkMRMLNode::SafeDownCast(caller) &&
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
void vtkMRMLNode::SetScene(vtkMRMLScene* scene)
{
  if (this->Scene == scene)
    {
    return;
    }
  if (this->Scene)
    {
    this->RemoveAllReferencedNodes();
    }

  this->Scene = scene;
  if (this->Scene)
    {
    this->SetSceneReferences();
    this->SetSceneRootDir(scene->GetRootDirectory());
    //this->UpdateNodeReferences();
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
  if (attName == 0)
    {
    return 0;
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
  return 0;
}

//----------------------------------------------------------------------------
const char* vtkMRMLNode
::GetMRMLAttributeNameFromReferenceRole(const char* refRole)
{
  if (refRole == 0)
    {
    return 0;
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
  return 0;
}

//----------------------------------------------------------------------------
bool vtkMRMLNode::IsReferenceRoleGeneric(const char* refRole)
{
  if (refRole == 0)
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
    vtkErrorMacro(<< "SetSceneReferences: Scene is expected to be non NULL.");
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
        vtkErrorMacro(<< "SetSceneReferences: Reference " << i << " is expected to be non NULL.");
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
        vtkErrorMacro(<< "UpdateReferenceID: Reference " << i << " is expected to be non NULL.");
        return;
        }
      if (std::string(oldID) == std::string( (reference->GetReferencedNodeID()) ) )
        {
        this->SetAndObserveNthNodeReferenceID(reference->GetReferenceRole(), i, newID);
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
    vtkErrorMacro(<< "SetAttribute: Name parameter is expected to be non NULL.");
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
  if (value != 0)
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
    vtkErrorMacro(<< "RemoveAttribute: Name parameter is expected to be non NULL.");
    return;
    }
  if (strlen(name) == 0)
    {
    vtkErrorMacro(<< "RemoveAttribute: Name parameter is expected to have at least one character.");
    return;
    }
  this->SetAttribute(name, 0);
}

//----------------------------------------------------------------------------
const char* vtkMRMLNode::GetAttribute(const char* name)
{
  if (!name)
    {
    vtkErrorMacro(<< "GetAttribute: Name parameter is expected to be non NULL.");
    return 0;
    }
  if (strlen(name) == 0)
    {
    vtkErrorMacro(<< "GetAttribute: Name parameter is expected to have at least one character.");
    return 0;
    }
  AttributesType::const_iterator iter =
    this->Attributes.find(std::string(name));
  if (iter == Attributes.end())
    {
    return NULL;
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

  if ( self == NULL )
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
  if ( this->ID == NULL && _arg == NULL) { return;}
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
    this->ID = NULL;
    }
  this->InvokeEvent(vtkMRMLNode::IDChangedEvent, oldID);
  if (oldID) { delete [] oldID; }
  this->Modified();
}

//----------------------------------------------------------------------------
const char * vtkMRMLNode::URLEncodeString(const char *inString)
{
  if (inString == NULL)
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
  if (inString == NULL)
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

//// Reference API

//-----------------------------------------------------------
void vtkMRMLNode::UpdateReferences()
{
  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    for (unsigned int i=0; i<it->second.size();)
      {
      vtkMRMLNodeReference* reference = it->second[i];
      if (reference->GetReferencedNodeID() &&
          std::string(reference->GetReferencedNodeID()) != "" &&
          this->Scene->GetNodeByID(reference->GetReferencedNodeID()) == NULL)
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
void vtkMRMLNode::RemoveAllNodeReferenceIDs(const char* referenceRole)
{
  if (!referenceRole)
    {
    NodeReferencesType::iterator it;
    for (it=this->NodeReferences.begin(); it!=this->NodeReferences.end(); it++)
      {
      if (it->first.c_str())
        {
        this->RemoveAllNodeReferenceIDs(it->first.c_str());
        }
      }
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
void vtkMRMLNode::GetNodeReferences(const char* referenceRole, std::vector<vtkMRMLNode*> &nodes)
{
  if (referenceRole)
    {
    this->UpdateNodeReferences(referenceRole);
    NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
    for (unsigned int i=0; i<references.size(); i++)
      {
      nodes.push_back(references[i]->ReferencedNode);
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
    referencedNodeIDs.push_back(references[i] ? references[i]->GetReferencedNodeID() : 0);
    }
}


//----------------------------------------------------------------------------
const char * vtkMRMLNode::GetNthNodeReferenceID(const char* referenceRole, int n)
{
  if (!referenceRole || n < 0)
    {
    return NULL;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
  if (n >= static_cast<int>(references.size()))
    {
    return NULL;
    }
  if (!references[n])
    {
    vtkErrorMacro(<< "GetNthNodeReferenceID: Reference " << n << "shoult NOT be NULL.");
    return NULL;
    }
  return references[n]->GetReferencedNodeID();
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::GetNthNodeReference(const char* referenceRole, int n)
{

  if (!referenceRole || n < 0 )
    {
    return NULL;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
  if (n >= static_cast<int>(references.size()))
    {
    return NULL;
    }

  vtkMRMLNode* node = references[n]->ReferencedNode;
  // Maybe the node was not yet in the scene when the node ID was set.
  // Check to see if it's now there.
  // Similarly, if the scene is 0, clear the node if not already null.
  if ((!node || node->GetScene() != this->GetScene()) ||
      (node && this->GetScene() == 0))
    {
    this->UpdateNthNodeReference(referenceRole, n);
    NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];
    node = references[n]->ReferencedNode;
    }
  return node;
}

//-----------------------------------------------------------
void vtkMRMLNode::UpdateNodeReferences()
{
  NodeReferencesType::iterator it;
  for (it=this->NodeReferences.begin(); it!=this->NodeReferences.end(); it++)
    {
    if (it->first.c_str())
      {
      this->UpdateNodeReferences(it->first.c_str());
      }
    }
  return;
}

//-----------------------------------------------------------
void vtkMRMLNode::UpdateNodeReferences(const char* referenceRole)
{
  if (!referenceRole)
    {
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
void vtkMRMLNode::UpdateNthNodeReference(vtkMRMLNodeReference *reference, int n)
{
  if (!reference)
    {
    vtkErrorMacro(<< "UpdateNthNodeReference: Non-null reference is expected.");
    return;
    }
  if (reference->GetReferenceRole() == 0 ||
      std::string(reference->GetReferenceRole()) == "")
    {
    return;
    }

  vtkMRMLNode* node = this->GetScene() ?
    this->GetScene()->GetNodeByID(reference->GetReferencedNodeID()) : 0;

  vtkIntArray *events = reference->Events;
  const char* referenceRole = reference->GetReferenceRole();
  if (events == 0 && this->NodeReferenceEvents[referenceRole] && this->NodeReferenceEvents[referenceRole]->GetNumberOfTuples() > 0)
    {
    events = this->NodeReferenceEvents[referenceRole];
    }

  this->SetAndObserveNthNodeReference(reference->GetReferenceRole(), n, node, events);
  reference->ReferencedNode = node;
}


//----------------------------------------------------------------------------
void vtkMRMLNode::UpdateNthNodeReference(const char* referenceRole, int n)
{
  if (!referenceRole)
    {
    return;
    }

  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];

  if (n >= static_cast<int>(references.size()))
    {
    vtkErrorMacro(<< "UpdateNthNodeReference: n is " << n << "."
        << " Value is expected to be smaller than " << references.size());
    return;
    }

  this->UpdateNthNodeReference(references[n], n);
}

//----------------------------------------------------------------------------
 vtkMRMLNode* vtkMRMLNode::SetNthNodeReferenceID(const char* referenceRole,
                                                 int n,
                                                 const char* referencedNodeID)
{
  if (referenceRole == 0 ||
      n < 0)
    {
    return 0;
    }

  //
  // No event
  //

  NodeReferenceListType &referencedNodes = this->NodeReferences[std::string(referenceRole)];

  std::string newReferencedNodeID;

  if (referencedNodeID)
    {
    newReferencedNodeID = referencedNodeID;
    }

  // Extend the list if needed. But don't do it if the node ID to add is null.
  if (n >= static_cast<int>(referencedNodes.size()) &&
      referencedNodeID != 0)
    {
    vtkNew<vtkMRMLNodeReference> reference;
    reference->ReferencingNode = this;
    reference->ReferencedNode = 0;
    reference->SetReferencedNodeID("");
    reference->SetReferenceRole(referenceRole);
    referencedNodes.push_back(reference.GetPointer());
    n = referencedNodes.size() - 1;
    this->NodeReferences[std::string(referenceRole)] = referencedNodes;
    }

  NodeReferenceListType::iterator referencedNodesIt = referencedNodes.begin() + n;
  // if we dont find the node or they have the same id
  // just return null
  if (referencedNodesIt >= referencedNodes.end())
    {
    return 0;
    }
  vtkMRMLNodeReference * reference = (*referencedNodesIt);

  // if the node has the same id and events
  if (reference->GetReferencedNodeID() &&
      std::string(reference->GetReferencedNodeID()) == newReferencedNodeID)
    {
    return reference->ReferencedNode;
    }

  // node id is different, remove reference from scene
  if ( reference->GetReferencedNodeID() != 0
        && std::string(reference->GetReferencedNodeID()) != ""
        && this->Scene)
    {
    this->Scene->RemoveReferencedNodeID(
      reference->GetReferencedNodeID(), this);
    }

  vtkMRMLNode* referencedNode = 0;

  // Delete the reference if the new value is 0.
  if (newReferencedNodeID.empty())
    {
    reference->SetReferencedNodeID(0); // Need to unobserve
    this->OnNodeReferenceRemoved(reference);
    if (!reference)
      {
      vtkErrorMacro(<< "SetNthNodeReferenceID: Reference " << n << " is expected to be non NULL.");
      return 0;
      }
    referencedNodes.erase(referencedNodesIt); // Delete the reference
    this->NodeReferences[std::string(referenceRole)] = referencedNodes;
    }
  else
    {
    // update ID in the NodeReferences
    reference->SetReferencedNodeID(newReferencedNodeID.c_str());

    // add reference to the scene
    if (this->Scene)
      {
      this->Scene->AddReferencedNodeID(newReferencedNodeID.c_str(), this);
      referencedNode = this->Scene->GetNodeByID(newReferencedNodeID.c_str());
      }


    // No event update

    // Adapted from UpdateNthNodeReference
    vtkMRMLNode *oldReferencedNode = reference->ReferencedNode;
    if (oldReferencedNode != 0 && referencedNode == 0)
      {
      this->OnNodeReferenceRemoved(reference);
      }
    else if (oldReferencedNode == 0 && referencedNode != 0)
      {
      this->OnNodeReferenceAdded(reference);
      }

    if (!reference)
      {
      vtkErrorMacro(<< "SetNthNodeReferenceID: Reference " << n << " is expected to be non NULL.");
      return 0;
      }
    reference->ReferencedNode = referencedNode;
    }

  this->Modified();

  return referencedNode;
}

//----------------------------------------------------------------------------
 vtkMRMLNode* vtkMRMLNode::SetAndObserveNthNodeReferenceID(const char* referenceRole,
                                                           int n,
                                                           const char* referencedNodeID,
                                                           vtkIntArray *events)
{
  if (referenceRole == 0 ||
      n < 0)
    {
    return 0;
    }

  if (events == 0 && this->NodeReferenceEvents[referenceRole] && this->NodeReferenceEvents[referenceRole]->GetNumberOfTuples() > 0)
    {
    events = NodeReferenceEvents[referenceRole];
    }

  NodeReferenceListType &referencedNodes = this->NodeReferences[std::string(referenceRole)];

  std::string newReferencedNodeID;

  if (referencedNodeID)
    {
    newReferencedNodeID = referencedNodeID;
    }

  // Extend the list if needed. But don't do it if the node ID to add is null.
  if (n >= static_cast<int>(referencedNodes.size()) &&
      referencedNodeID != 0)
    {
    vtkNew<vtkMRMLNodeReference> reference;
    reference->ReferencingNode = this;
    reference->ReferencedNode = 0;
    reference->SetReferencedNodeID("");
    reference->SetReferenceRole(referenceRole);
    referencedNodes.push_back(reference.GetPointer());
    n = referencedNodes.size() - 1;
    this->NodeReferences[std::string(referenceRole)] = referencedNodes;
    reference->Events = events;
    }

  NodeReferenceListType::iterator referencedNodesIt = referencedNodes.begin() + n;

  // if we dont find the node just return null
  if (referencedNodesIt >= referencedNodes.end())
    {
    return 0;
    }

  vtkMRMLNodeReference * reference = (*referencedNodesIt);

  // if the node has the same id and events
  // just update the references
  if (reference->GetReferencedNodeID() &&
       std::string(reference->GetReferencedNodeID()) == newReferencedNodeID &&
       reference->Events == events)
    {
    this->UpdateNthNodeReference(reference, n);
    if (!reference)
      {
      vtkErrorMacro(<< "SetAndObserveNthNodeReferenceID: Reference " << n << " is expected to be non NULL.");
      return 0;
      }
    return reference->ReferencedNode;
    }

  // node id is different, unobserve the old node
  if (reference->GetReferencedNodeID() != 0
      && std::string(reference->GetReferencedNodeID()) != ""
      && this->Scene)
    {
    this->Scene->RemoveReferencedNodeID(
      reference->GetReferencedNodeID(), this);
    }

  vtkMRMLNode* referencedNode = NULL;

  // Delete the reference if the new value is 0.
  if (newReferencedNodeID.empty())
    {
    reference->SetReferencedNodeID(0); // Need to unobserve
    this->SetAndObserveNthNodeReference(referenceRole, n, 0, reference->Events);
    referencedNodes.erase(referencedNodesIt); // Delete the reference
    this->NodeReferences[std::string(referenceRole)] = referencedNodes;
    }
  else
    {
    // update ID in the NodeReferences
    reference->SetReferencedNodeID(newReferencedNodeID.c_str());

    // add reference to the scene
    if (this->Scene)
      {
      this->Scene->AddReferencedNodeID(newReferencedNodeID.c_str(), this);
      }

    // Update events
    reference->Events = events;

    this->UpdateNthNodeReference(reference, n);
    if (!reference)
      {
      vtkErrorMacro(<< "SetAndObserveNthNodeReferenceID: Reference " << n << " is expected to be non NULL.");
      return 0;
      }
    referencedNode =  reference->ReferencedNode;
    }

  this->Modified();

  return referencedNode;
}

//----------------------------------------------------------------------------
void vtkMRMLNode::SetAndObserveNthNodeReference(const char* referenceRole, int n, vtkMRMLNode *referencedNode, vtkIntArray *events)
{
  NodeReferenceListType &references = this->NodeReferences[std::string(referenceRole)];


  vtkMRMLNode *oldReferencedNode = references[n]->ReferencedNode;

  if (events)
    {
    vtkSetAndObserveMRMLObjectEventsMacro(references[n]->ReferencedNode, referencedNode, events);
    }
  else
    {
    vtkSetAndObserveMRMLObjectMacro(references[n]->ReferencedNode, referencedNode);
    }

  if (oldReferencedNode != 0 && referencedNode == 0)
    {
    this->OnNodeReferenceRemoved(references[n]);
    }
  else if (oldReferencedNode == 0 && referencedNode != 0)
    {
    this->OnNodeReferenceAdded(references[n]);
    }
  else if (oldReferencedNode != referencedNode)
    {
    this->OnNodeReferenceModified(references[n]);
    }
}


//----------------------------------------------------------------------------
bool vtkMRMLNode::HasNodeReferenceID(const char* referenceRole, const char* referencedNodeID)
{
  if (referencedNodeID == 0)
    {
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
void vtkMRMLNode::RemoveAllReferencedNodes()
{
  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != NodeReferences.end(); it++)
    {
    NodeReferenceListType::iterator it1;
    for (it1 = it->second.begin(); it1 != it->second.end(); it1++)
      {
      vtkMRMLNodeReference* reference = *it1;
      reference->Events = 0;
      reference->ReferencedNode = 0;
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
  this->SetAndObserveNthNodeReferenceID(referenceRole, n, 0);
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
  return new vtkMRMLNode::vtkMRMLNodeReference;
}
