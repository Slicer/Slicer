/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLParser.cxx,v $
Date:      $Date: 2006/02/07 19:29:34 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/
#include "vtkObjectFactory.h"
#include "vtkMRMLParser.h"


//------------------------------------------------------------------------------
vtkMRMLParser* vtkMRMLParser::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLParser");
  if(ret) {
    return (vtkMRMLParser*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLParser;
}


//------------------------------------------------------------------------------
void vtkMRMLParser::StartElement(const char* tagName, const char** atts)
{
  if (!strcmp(tagName, "MRML")) {
    return;
  }
  const char* className = this->MRMLScene->GetClassNameByTag(tagName);

  if (className == NULL) {
    return;
  }

  vtkMRMLNode* node = this->MRMLScene->CreateNodeByClass( className );

  node->SetID(this->MRMLScene->GetUniqueIdByClass(className));

  node->ReadXMLAttributes(atts);

  node->ReadData();

  this->MRMLScene->AddNode(node);

  node->Delete();
}
