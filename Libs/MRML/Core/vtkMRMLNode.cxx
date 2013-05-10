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
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <cassert>
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

  this->HideFromEditors = 1;
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
    }
  if (this->Name)
    {
    delete [] this->Name;
    }
  if (this->ID)
    {
    delete [] this->ID;
    }
  if (this->SceneRootDir)
    {
    delete [] this->SceneRootDir;
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
  for (it = this->NodeReferences.begin(); it != NodeReferences.end(); it++)
    {
    std::vector< vtkMRMLNodeReference *>::iterator it1;
    for (it1 = it->second.begin(); it1 != it->second.end(); it1++)
      {
      if ((*it1)->Events == 0 && (*it1)->ReferencedNode != 0)
        {
        // node is registered here, with no events
        (*it1)->ReferencedNode->UnRegister(this);
        }
      else if ((*it1)->Events != 0 && (*it1)->ReferencedNode != 0)
        {
        // node is registered with ObsereverManager, events is registered here
        (*it1)->Events->UnRegister(this);
        }
      if (callOnNodeReferenceRemoved)
        {
        this->OnNodeReferenceRemoved((*it1));
        }
      (*it1)->Delete();
      }
    }
    this->NodeReferences.clear();

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
  this->HideFromEditors = node->HideFromEditors;
  this->SaveWithScene = node->SaveWithScene ;
  this->Selectable = node->Selectable;
  this->AddToScene = node->AddToScene;

  if (node->GetSingletonTag())
    {
    this->SetSingletonTag( node->GetSingletonTag() );
    }
  this->SetDescription(node->GetDescription());
  this->Attributes = node->Attributes;

  // copy references
  this->DeleteAllReferences();

  NodeReferencesType::iterator it;
  for (it = node->NodeReferences.begin(); it != node->NodeReferences.end(); it++)
    {
    std::string referenceRole = it->first;
    std::vector< vtkMRMLNodeReference *> referencedNodes = std::vector< vtkMRMLNodeReference *>();
    std::vector< vtkMRMLNodeReference *>::iterator it1;
    for (it1 = it->second.begin(); it1 != it->second.end(); it1++)
      {
      vtkMRMLNodeReference *referencedNode = vtkMRMLNodeReference::New();
      referencedNode->ReferencingNode = this;
      referencedNode->ReferencedNode = 0;
      referencedNode->SetReferencedNodeID((*it1)->GetReferencedNodeID());
      referencedNode->SetReferenceRole(referenceRole.c_str());
      referencedNodes.push_back(referencedNode);
      this->NodeReferences[referenceRole] = referencedNodes;
      this->OnNodeReferenceAdded(referencedNode);
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLNode::Reset()
{    
  vtkMRMLNode *newNode = this->CreateNodeInstance();
    
  int save = this->GetSaveWithScene();
  int hide = this->GetHideFromEditors();
  int select = this->GetSelectable();
  char *tag = this->GetSingletonTag();

  this->DisableModifiedEventOn();
  this->CopyWithSceneWithoutModifiedEvent(newNode);
  
  this->SetSaveWithScene(save);
  this->SetHideFromEditors(hide);
  this->SetSelectable(select);
  this->SetSingletonTag(tag);
  this->DisableModifiedEventOff(); // does not invoke Modified()
  
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
  std::map< std::string, std::string>::iterator itName;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    std::string referenceRole = it->first;
    std::string referenceMRMLAttributeName = NodeReferenceMRMLAttributeNames[referenceRole];

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
      os << indent << " " << referenceMRMLAttributeName << "=\"" << ss.str().c_str() << "\"";
      }
    }

}

//----------------------------------------------------------------------------
void vtkMRMLNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    // read node refereences
    std::map< std::string, std::string>::iterator it;
    for (it = this->NodeReferenceMRMLAttributeNames.begin(); it != this->NodeReferenceMRMLAttributeNames.end(); it++)
      {
      std::string referenceRole = it->first;
      std::string referenceMRMLAttributeName = it->second;

      if (!strcmp(attName, referenceMRMLAttributeName.c_str() ) )
        {
        std::stringstream ss(attValue);
        while (!ss.eof())
          {
          std::string id;
          ss >> id;
          if (!id.empty())
            {
            this->AddNodeReferenceID(referenceRole.c_str(), id.c_str());
            }
          }
        }
      }
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
    }
  this->EndModify(disabledModify);

  return;
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
  NodeReferencesType::iterator it;
  std::map< std::string, std::string>::iterator itName;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    std::string referenceRole = it->first;
    std::string referenceMRMLAttributeName = NodeReferenceMRMLAttributeNames[referenceRole];

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
      of << indent << " " << referenceMRMLAttributeName << "=\"" << ss.str().c_str() << "\"";
      }
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
      std::vector< vtkMRMLNodeReference *> references = this->NodeReferences[it->first];
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
void vtkMRMLNode::AddNodeReferenceRole(const char *referenceRole, const char *mrmlAttributeName)
{
  if (referenceRole)
    {
    this->NodeReferences[std::string(referenceRole)] = std::vector< vtkMRMLNodeReference *>();
    this->NodeReferenceMRMLAttributeNames[std::string(referenceRole)] = mrmlAttributeName ? 
      std::string(mrmlAttributeName) : std::string(referenceRole);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNode::SetSceneReferences()
{
  assert(this->Scene); // must always have a valid scene when called.

  NodeReferencesType::iterator it;
  for (it = this->NodeReferences.begin(); it != this->NodeReferences.end(); it++)
    {
    for (unsigned int i=0; i<it->second.size(); i++)
      {
      this->Scene->AddReferencedNodeID(it->second[i]->GetReferencedNodeID(), this);
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
      if (std::string(oldID) == std::string( (it->second[i]->GetReferencedNodeID()) ) )
        {
        this->SetAndObserveNthNodeReferenceID(it->second[i]->GetReferenceRole(), i, newID);
        }
      }
    }
  this->EndModify(wasModifying);
}


//----------------------------------------------------------------------------
void vtkMRMLNode::SetAttribute(const char* name, const char* value)
{
  const char* oldValue = this->GetAttribute(name);
  if (name == 0 ||
      strlen(name) == 0 ||
      (!oldValue && !value) ||
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
  assert(name != 0);
  this->SetAttribute(name, 0);
}

//----------------------------------------------------------------------------
const char* vtkMRMLNode::GetAttribute(const char* name)
{
  if ( name == NULL )
    {
    return NULL;
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
      if (it->second[i]->GetReferencedNodeID() &&
          std::string(it->second[i]->GetReferencedNodeID()) != "" &&
          this->Scene->GetNodeByID(it->second[i]->GetReferencedNodeID()) == NULL)
        {
        this->RemoveNthNodeReferenceID(it->second[i]->GetReferenceRole(), i);
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
    std::vector< vtkMRMLNodeReference *> &references = this->NodeReferences[std::string(referenceRole)];
    for (unsigned int i=0; i<references.size(); i++)
      {
        nodes.push_back(references[i]->ReferencedNode);
      }
    }
}

//----------------------------------------------------------------------------
const char * vtkMRMLNode::GetNthNodeReferenceID(const char* referenceRole, int n)
{
  if (!referenceRole || n < 0)
    {
    return NULL;
    }
  
  std::vector< vtkMRMLNodeReference *> &references = this->NodeReferences[std::string(referenceRole)];

  if (n >= static_cast<int>(references.size()) )
    {
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
  
  std::vector< vtkMRMLNodeReference *> &references = this->NodeReferences[std::string(referenceRole)];

  if (n >= static_cast<int>(references.size()) )
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
    std::vector< vtkMRMLNodeReference *> &references = this->NodeReferences[std::string(referenceRole)];
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

  std::vector< vtkMRMLNodeReference *> &references = this->NodeReferences[std::string(referenceRole)];
  for (unsigned int i=0; i<references.size(); i++)
    {
    this->UpdateNthNodeReference(referenceRole, i);
    }

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLNode::UpdateNthNodeReference(vtkMRMLNodeReference *reference, int n)
{
  if (reference->GetReferenceRole() == 0 || std::string(reference->GetReferenceRole()) == "")
    {
    return;
    }

  vtkMRMLNode* node = this->GetScene() ?
    this->GetScene()->GetNodeByID(reference->GetReferencedNodeID()) : 0;

  this->SetAndObserveNthNodeReference(reference->GetReferenceRole(), n, node, reference->Events);
  reference->ReferencedNode = node;
  reference->ReferencingNode = this;
}


//----------------------------------------------------------------------------
void vtkMRMLNode::UpdateNthNodeReference(const char* referenceRole, int n)
{
  if (!referenceRole)
    {
    return;
    }

  std::vector< vtkMRMLNodeReference *> &references = this->NodeReferences[std::string(referenceRole)];

  assert( references.size() > (unsigned int)(n));

  this->UpdateNthNodeReference(references[n], n);
}


//----------------------------------------------------------------------------
 vtkMRMLNode* vtkMRMLNode::SetNthNodeReferenceID(const char* referenceRole, int n, const char* referencedNodeID)
{
  vtkMRMLNode* referencedNode = NULL;

  std::vector< vtkMRMLNodeReference *> referencedNodes;
  if (referenceRole)
    {
    referencedNodes = this->NodeReferences[std::string(referenceRole)];
    }
  else
    {
    // TODO maybe create a dummy role?
    vtkWarningMacro("referenceRole is NULL");
    return 0;
    }

  std::string newReferencedNodeID;

  if (referencedNodeID)
    {
    newReferencedNodeID = referencedNodeID;
    }

  // Extend the list if needed. But don't do it if the node ID to add is null
  if (n >= static_cast<int>(referencedNodes.size()) &&
      referencedNodeID != 0)
    {
    assert(n == static_cast<int>(referencedNodes.size()));
    vtkMRMLNodeReference *referencedNode = vtkMRMLNodeReference::New();
    referencedNode->ReferencingNode = this;
    referencedNode->ReferencedNode = 0;
    referencedNode->SetReferencedNodeID("");
    referencedNode->SetReferenceRole(referenceRole);
    referencedNodes.push_back(referencedNode);
    n = referencedNodes.size() - 1;
    this->NodeReferences[std::string(referenceRole)] = referencedNodes;
    }

  std::vector< vtkMRMLNodeReference *>::iterator referencedNodesIt = 
                                             referencedNodes.begin() + n;

  // if we dont find the node or they have the same id
  // just return null
  if (referencedNodesIt == referencedNodes.end())
    {
    return 0;
    }
  if ((*referencedNodesIt)->GetReferencedNodeID() &&
      std::string((*referencedNodesIt)->GetReferencedNodeID()) == newReferencedNodeID)
    {
    return (*referencedNodesIt)->ReferencedNode;
    }

  bool oldReferenceRemoved = false;
  // node id is different, remove reference from scene
  if ( (*referencedNodesIt)->GetReferencedNodeID() != 0 
        && std::string((*referencedNodesIt)->GetReferencedNodeID()) != ""
        && this->Scene)
    {
    oldReferenceRemoved = true;
    this->Scene->RemoveReferencedNodeID(
      (*referencedNodesIt)->GetReferencedNodeID(), this);
    if ((*referencedNodesIt)->ReferencedNode)
      {
      (*referencedNodesIt)->ReferencedNode->UnRegister(this);
      }
    }

  // Delete the  node ID if the new value is 0.
  if (newReferencedNodeID.empty())
    {
    this->OnNodeReferenceRemoved(*referencedNodesIt);
    referencedNode->UnRegister(this);
    referencedNodes.erase(referencedNodesIt);
    this->NodeReferences[std::string(referenceRole)] = referencedNodes;
    }
  else
    {
    (*referencedNodesIt)->SetReferencedNodeID(newReferencedNodeID.c_str());
    if (this->Scene)
      {
      this->Scene->AddReferencedNodeID(newReferencedNodeID.c_str(), this);
      referencedNode = this->Scene->GetNodeByID(newReferencedNodeID.c_str());
      if (//!oldReferenceRemoved && 
        referencedNode)
        {
        referencedNode->Register(this);
        }
      }
    (*referencedNodesIt)->ReferencedNode = referencedNode;
    (*referencedNodesIt)->ReferencingNode = this;
    if (oldReferenceRemoved)
      {
      this->OnNodeReferenceModified(*referencedNodesIt);
      }
    else
      {
      this->OnNodeReferenceAdded(*referencedNodesIt);
      }
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
  vtkMRMLNode* referencedNode = NULL;

  std::vector< vtkMRMLNodeReference *> referencedNodes;
  if (referenceRole)
    {
    referencedNodes = this->NodeReferences[std::string(referenceRole)];
    }
  else
    {
    // TODO maybe create a dummy role?
    vtkWarningMacro("referenceRole is NULL");
    return referencedNode;
    }

  std::string newReferencedNodeID;

  if (referencedNodeID)
    {
    newReferencedNodeID = referencedNodeID;
    }

  // Extend the list if needed. But don't do it if the node ID to add is null
  if (n >= static_cast<int>(referencedNodes.size()) &&
      referencedNodeID != 0)
    {
    assert(n == static_cast<int>(referencedNodes.size()));
    vtkMRMLNodeReference *reference = vtkMRMLNodeReference::New();
    reference->ReferencingNode = this;
    reference->ReferencedNode = 0;
    //reference->SetReferencedNodeID = std::string();
    reference->SetReferenceRole(referenceRole);
    reference->Events = events;
    if (events)
    {
      events->Register(this);
    }
    referencedNodes.push_back(reference);
    n = referencedNodes.size() - 1;
    this->NodeReferences[std::string(referenceRole)] = referencedNodes;

    }

  std::vector< vtkMRMLNodeReference *>::iterator referencedNodesIt = 
                                             referencedNodes.begin() + n;

  // if we dont find the node or they have the same id adn events
  // just update the references
  if (referencedNodesIt == referencedNodes.end() || 
     ((*referencedNodesIt)->GetReferencedNodeID() &&
       std::string((*referencedNodesIt)->GetReferencedNodeID()) == newReferencedNodeID &&
       (*referencedNodesIt)->Events == events) )
    {
    if (n < static_cast<int>(referencedNodes.size()))
      {
      this->UpdateNthNodeReference(*referencedNodesIt, n);
      }
    return referencedNodesIt == referencedNodes.end() ?
      0 : (*referencedNodesIt)->ReferencedNode;
    }

  // node id is different, unobserve the old node
  if ((*referencedNodesIt)->GetReferencedNodeID() != 0 
      && std::string((*referencedNodesIt)->GetReferencedNodeID()) != ""
      && this->Scene)
    {
    this->Scene->RemoveReferencedNodeID(
      (*referencedNodesIt)->GetReferencedNodeID(), this);
    }

  // Delete the node ID if the new value is 0.
  if (newReferencedNodeID.empty())
    {
    /// Need to unobserve
    this->SetAndObserveNthNodeReference(referenceRole, n, 0, (*referencedNodesIt)->Events);
    vtkMRMLNodeReference *tmp = (*referencedNodesIt);
    referencedNodes.erase(referencedNodesIt);
    this->NodeReferences[std::string(referenceRole)] = referencedNodes;
    tmp->Delete();
    }
  else
    {
    // update ID in the NodeReferences
    (*referencedNodesIt)->SetReferencedNodeID(newReferencedNodeID.c_str());

    // add reference to the scene
    if (this->Scene)
      {
      this->Scene->AddReferencedNodeID(newReferencedNodeID.c_str(), this);
      }

    (*referencedNodesIt)->ReferencingNode = this;

    // Update events
    if ((*referencedNodesIt)->Events != events) // events are different
      {
      if ((*referencedNodesIt)->Events)
        {
        (*referencedNodesIt)->Events->UnRegister(this);
        }
      if (events)
        {
          events->Register(this);
        }
      }
    (*referencedNodesIt)->Events = events;

    this->UpdateNthNodeReference(*referencedNodesIt, n);
    referencedNode =  (*referencedNodesIt)->ReferencedNode;
    }

  this->Modified();

  return referencedNode;
}




 //----------------------------------------------------------------------------
void vtkMRMLNode::SetAndObserveNthNodeReference(const char* referenceRole, int n, vtkMRMLNode *referencedNode, vtkIntArray *events)
{
  std::vector< vtkMRMLNodeReference *> &references = this->NodeReferences[std::string(referenceRole)];
    
  
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
bool vtkMRMLNode::HasNodeReferenceID(const char* referenceRole, const char* NodeReferenceID)
{
  if (NodeReferenceID == 0)
    {
    return false;
    }

  std::vector< vtkMRMLNodeReference *> &references = this->NodeReferences[std::string(referenceRole)];
  std::vector< vtkMRMLNodeReference *>::iterator it;
  std::string sID(NodeReferenceID);
  for (it=references.begin(); it!=references.end(); it++)
    {
    if (sID == std::string((*it)->GetReferencedNodeID()) )
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
    std::vector< vtkMRMLNodeReference *>::iterator it1;
    for (it1 = it->second.begin(); it1 != it->second.end(); it1++)
      {
      if ((*it1)->Events == 0 && (*it1)->ReferencedNode != 0)
        {
        // node is registered here, with no events
        (*it1)->ReferencedNode->UnRegister(this);
        }
      else if ((*it1)->Events != 0 && (*it1)->ReferencedNode != 0)
        {
        // node is registered with ObsereverManager, events is registered here
        (*it1)->Events->UnRegister(this);
        (*it1)->ReferencedNode->UnRegister(this);
        }
      (*it1)->Events = 0;
      (*it1)->ReferencedNode = 0;
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
vtkMRMLNode* vtkMRMLNode::SetAndObserveNodeReferenceID(const char* referenceRole, const char *NodeReferenceID, vtkIntArray *events)
{
  return this->SetAndObserveNthNodeReferenceID(referenceRole, 0, NodeReferenceID, events);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::AddAndObserveNodeReferenceID(const char* referenceRole, const char *NodeReferenceID, vtkIntArray *events)
{
  return this->SetAndObserveNthNodeReferenceID(referenceRole, this->GetNumberOfNodeReferences(referenceRole), NodeReferenceID, events);
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
    n = static_cast<int>(this->NodeReferences[std::string(referenceRole)].size());
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
