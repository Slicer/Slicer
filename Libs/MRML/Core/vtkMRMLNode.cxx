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
}

//----------------------------------------------------------------------------
void vtkMRMLNode::WriteNodeBodyXML(ostream &, int )
{
}

//----------------------------------------------------------------------------
void vtkMRMLNode::ProcessMRMLEvents ( vtkObject *, unsigned long , void *)
{
}

//----------------------------------------------------------------------------
void vtkMRMLNode::SetScene(vtkMRMLScene* scene)
{
  this->Scene = scene;
  if (this->Scene)
    {
    this->SetSceneReferences();
    this->SetSceneRootDir(scene->GetRootDirectory());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNode::SetSceneReferences()
{
  assert(this->Scene); // must always have a valid scene when called.
}

//----------------------------------------------------------------------------
void vtkMRMLNode::UpdateReferenceID(const char *, const char *)
{
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
