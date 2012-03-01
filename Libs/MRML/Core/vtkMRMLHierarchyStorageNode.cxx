/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiducialListStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/


#include "vtkMRMLHierarchyStorageNode.h"
#include "vtkMRMLHierarchyNode.h"
#include "vtkMRMLScene.h"

#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLHierarchyStorageNode);

//----------------------------------------------------------------------------
vtkMRMLHierarchyStorageNode::vtkMRMLHierarchyStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLHierarchyStorageNode::~vtkMRMLHierarchyStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyStorageNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLHierarchyStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLHierarchyStorageNode::ReadData(vtkMRMLNode *refNode)
{
  // do not read if if we are not in the scene (for example inside snapshot)
  if ( !this->GetAddToScene() || !refNode->GetAddToScene() )
    {
    return 1;
    }
  
  if (this->GetScene() && this->GetScene()->GetReadDataOnLoad() == 0)
    {
    return 1;
    }
  
  vtkDebugMacro("Reading hierarchy data");
  // test whether refNode is a valid node to hold a hierarchy
  if ( !( refNode->IsA("vtkMRMLHierarchyNode"))
       ) 
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLHierarchyNode");
    return 0;         
    }
  
  if (this->GetFileName() == NULL && this->GetURI() == NULL) 
    {
    vtkErrorMacro("ReadData: file name and uri not set");
    return 0;
    }
  
  Superclass::StageReadData(refNode);
  if ( this->GetReadState() != this->TransferDone )
    {
    // remote file download hasn't finished
    vtkWarningMacro("ReadData: Read state is pending, returning.");
    return 0;
    }
  
  std::string fullName = this->GetFullNameFromFileName(); 
  
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLHierarchyStorageNode: File name not specified");
    return 0;
    }
  
  // cast the input node
  vtkMRMLHierarchyNode *hierarchyNode = NULL;
  if ( refNode->IsA("vtkMRMLHierarchyNode") )
    {
    hierarchyNode = dynamic_cast <vtkMRMLHierarchyNode *> (refNode);
    }
  
  if (hierarchyNode == NULL)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a hierarchy node");
    return 0;
    }

  // open the file for reading input
  fstream fstr;
  
  fstr.open(fullName.c_str(), fstream::in);
  
  if (fstr.is_open())
    {
    //turn off modified events
    int modFlag = hierarchyNode->GetDisableModifiedEvent(); 
    hierarchyNode->DisableModifiedEventOn();

    // do the reading here if necessary, but it's not right now
    
    hierarchyNode->SetDisableModifiedEvent(modFlag);
    hierarchyNode->InvokeEvent(vtkMRMLScene::NodeAddedEvent, hierarchyNode);
    fstr.close();
    }
  else
    {
    vtkErrorMacro("ERROR opening file " << this->FileName << endl);
    return 0;
    }
  
  this->SetReadStateIdle();

  // make sure that the list node points to this storage node
  //-------------------------> hierarchyNode->SetAndObserveStorageNodeID(this->GetID());
  

  // mark it unmodified since read
  hierarchyNode->ModifiedSinceReadOff();

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLHierarchyStorageNode::WriteData(vtkMRMLNode *refNode)
{

  // test whether refNode is a valid node to hold a volume
  if ( !( refNode->IsA("vtkMRMLHierarchyNode") ) )
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLHierarchyNode");
    return 0;         
    }
  
  if (this->GetFileName() == NULL) 
    {
    vtkErrorMacro("WriteData: file name is not set");
    return 0;
    }
  
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLHierarchyStorageNode: File name not specified");
    return 0;
    }
  
  // cast the input node
  vtkMRMLHierarchyNode *hierarchyNode = NULL;
  if ( refNode->IsA("vtkMRMLHierarchyNode") )
    {
    hierarchyNode = dynamic_cast <vtkMRMLHierarchyNode *> (refNode);
    }
  
  if (hierarchyNode == NULL)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known hierarchy node");
    return 0;
    }

  // open the file for writing
  fstream of;

  of.open(fullName.c_str(), fstream::out);
  
  if (!of.is_open())
  {
  vtkErrorMacro("WriteData: unable to open file " << fullName.c_str() << " for writing");
  return 0;
  }

  // put down a header
  of << "# hierarchy file " << (this->GetFileName() != NULL ? this->GetFileName() : "null") << endl;

  of.close();
  
  Superclass::StageWriteData(refNode);
  
  return 1;

}

//----------------------------------------------------------------------------
int vtkMRMLHierarchyStorageNode::SupportedFileType(const char *fileName)
{
  // check to see which file name we need to check
  std::string name;
  if (fileName)
    {
    name = std::string(fileName);
    }
  else if (this->FileName != NULL)
    {
    name = std::string(this->FileName);
    }
  else if (this->URI != NULL)
    {
    name = std::string(this->URI);
    }
  else
    {
    vtkWarningMacro("SupportedFileType: no file name to check");
    return 0;
    }
  
  std::string::size_type loc = name.find_last_of(".");
  if( loc == std::string::npos ) 
    {
    vtkErrorMacro("SupportedFileType: no file extension specified");
    return 0;
    }
  std::string extension = name.substr(loc);
  
  vtkDebugMacro("SupportedFileType: extension = " << extension.c_str());
  if (extension.compare(".txt") == 0)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}
