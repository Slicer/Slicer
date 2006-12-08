/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLParser.cxx,v $
Date:      $Date: 2006/03/11 19:51:14 $
Version:   $Revision: 1.8 $

=========================================================================auto=*/
#include "vtkObjectFactory.h"
#include "vtkMRMLParser.h"


//------------------------------------------------------------------------------
vtkMRMLParser* vtkMRMLParser::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLParser");
  if(ret) 
    {
    return (vtkMRMLParser*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLParser;
}


//------------------------------------------------------------------------------
void vtkMRMLParser::StartElement(const char* tagName, const char** atts)
{
  if (!strcmp(tagName, "MRML")) 
    {
    return;
    }
  const char* className = this->MRMLScene->GetClassNameByTag(tagName);

  if (className == NULL) 
    {
    return;
    }

  vtkMRMLNode* node = this->MRMLScene->CreateNodeByClass( className );

  node->SetScene(this->MRMLScene);
  node->ReadXMLAttributes(atts);

  // ID will be set by AddNodeNoNotify
  /*
  if (node->GetID() == NULL) 
    {
    node->SetID(this->MRMLScene->GetUniqueIDByClass(className));
    }
  */
  if (!this->NodeStack.empty()) 
    {
    vtkMRMLNode* parentNode = this->NodeStack.top();
    parentNode->ProcessChildNode(node);

    node->ProcessParentNode(parentNode);
    }

  this->NodeStack.push(node);

  if (this->NodeCollection)
    {
    this->NodeCollection->vtkCollection::AddItem((vtkObject *)node);
    }
  else
    {
    this->MRMLScene->AddNodeNoNotify(node);
    }
  node->Delete();
}

//-----------------------------------------------------------------------------

void vtkMRMLParser::EndElement (const char *name)
{
  if ( !strcmp(name, "MRML") || this->NodeStack.empty() ) 
    {
    return;
    }

  const char* className = this->MRMLScene->GetClassNameByTag(name);
  if (className == NULL) 
    {
    return;
    }

  this->NodeStack.pop();
}
