/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLStorageNode.cxx,v $
Date:      $Date: 2007/01/17 20:09:05 $
Version:   $Revision: 1.1.1.1 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLScene.h"


//----------------------------------------------------------------------------
vtkMRMLStorageNode::vtkMRMLStorageNode()
{
  this->FileName = NULL;
  this->URI = NULL;
  this->UseCompression = 1;
  this->ReadState = this->Done;
  this->WriteState = this->Done;
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode::~vtkMRMLStorageNode()
{
  if (this->FileName) 
    {
    delete [] this->FileName;
    this->FileName = NULL;
    }
  if (this->URI)
    {
    delete [] this->URI;
    this->URI = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  if (this->FileName != NULL) 
    {
    of << indent << " fileName=\"" << vtkMRMLNode::URLEncodeString(this->FileName) << "\"";
    }
  if (this->URI != NULL)
    {
    of << indent << " uri=\"" << vtkMRMLNode::URLEncodeString(this->URI) << "\"";
    }
  
  std::stringstream ss;
  ss << this->UseCompression;
  of << indent << " useCompression=\"" << ss.str() << "\"";

  of << indent << " readState=\"" << this->ReadState <<  "\"";
  of << indent << " writeState=\"" << this->WriteState <<  "\"";
  
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "fileName")) 
      {
      // URLDeodeString returns a buffer that was created using new[].
      // It is up to the client to delete it.
      const char* filename = vtkMRMLNode::URLDecodeString(attValue);
      this->SetFileName(filename);
      delete [] filename;
      }
    else if (!strcmp(attName, "uri"))
      {
      // URLDeodeString returns a buffer that was created using new[].
      // It is up to the client to delete it.
      const char* uri = vtkMRMLNode::URLDecodeString(attValue);
      this->SetURI(uri);
      delete [] uri;
      }
    else if (!strcmp(attName, "useCompression")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->UseCompression;
      }
    else if (!strcmp(attName, "readState"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ReadState;
      }
    else if (!strcmp(attName, "writeState"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->WriteState;
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLStorageNode *node = (vtkMRMLStorageNode *) anode;
  this->SetFileName(node->FileName);
  this->SetURI(node->URI);
  this->SetUseCompression(node->UseCompression);
  this->SetReadState(node->ReadState);
  this->SetWriteState(node->WriteState);
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " <<
    (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "URI: " <<
    (this->URI ? this->URI : "(none)") << "\n";
  os << indent << "UseCompression:   " << this->UseCompression << "\n";
  os << indent << "ReadState:  " << this->GetReadStateAsString() << "\n";
  os << indent << "WriteState: " << this->GetWriteStateAsString() << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{
  if (event ==  vtkCommand::ProgressEvent) 
    {
    this->InvokeEvent ( vtkCommand::ProgressEvent,callData );
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::StageReadData ( vtkMRMLNode *refNode )
{
  // for now, just set the read state to done
  this->SetReadStateDone();
  return;
   
  if (this->URI == NULL)
    {
    vtkWarningMacro("Cannot stage data for reading, URI is not set.");
    return;
    }
  // need to get URI handlers from the scene
  if (this->Scene == NULL)
    {
    vtkWarningMacro("StageReadData: Cannot get mrml scene, unable to get remote file handlers.");
    return;
    }
  if (refNode == NULL)
    {
    vtkWarningMacro("StageReadData: input mrml node is null, returning.");
    return;
    }
  
  int asynch = 0;
  
  /* To be finalised
  // Get the data io manager
   vtkDataIOManager *iomanager = this->Scene->GetDataIOManager();
   if (iomanager != NULL)
     {
     asynch = iomanger->GetAynchronousEnabled();
     }
   
   if (iomanager != NULL &&
       asynch &&
       this->GetReadState() != this->Pending)
     {
     this->SetReadStatePending();
     // set up the data handler
     this->URIHandler = this->Scene->FindURIHandler(this->URI);
     iomanager->Queue(node);
     }
   else
     {
     std::string cacheFileName;
     if (this->Scene->GetCacheManager() != NULL)
       {
       cacheFileName = this->Scene->GetCacheManager()->GetFilenameFromURI(this->URI);
       }
     else
       {
       vtkWarningMacro("Couldn't get the cache manager, using filename = " << this->GetFileName());
       cacheFileName = this->GetFileName();
       }
     if (this->URIHandler)
       {
       this->URIHandler->StageFile(this->URI, cacheFileName);
       this->SetReadState(this->Done);
       }
     else
       {
       vtkWarningMacro("No URI Handler!");
       return;
       }
     }
   */
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::StageWriteData ( vtkMRMLNode *refNode )
{
  // for now, just set the write state to done
  this->SetWriteStateDone();
  return;
  
  if (this->URI == NULL)
    {
    vtkWarningMacro("Cannot stage data for writing, URI is not set.");
    return;
    }
  // need to get URI handlers from the scene
  if (this->Scene == NULL)
    {
    vtkWarningMacro("StageWriteData: Cannot get mrml scene, unable to get remote file handlers.");
    return;
    }
  if (refNode == NULL)
    {
    vtkWarningMacro("StageWriteData: input mrml node is null, returning.");
    return;
    }
}

//----------------------------------------------------------------------------
const char * vtkMRMLStorageNode::GetStateAsString(int state)
{
  if (state == this->Pending)
    {
    return "Pending";
    }
  if (state = this->Done)
    {
    return "Done";
    }
  return "(undefined)";
}

//----------------------------------------------------------------------------
std::string vtkMRMLStorageNode::GetFullNameFromFileName()
{
  std::string fullName = std::string("");
  if (this->SceneRootDir != NULL && this->Scene->IsFilePathRelative(this->GetFileName())) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileName());
    }
  else 
    {
    fullName = std::string(this->GetFileName());
    }
  return fullName;
}
