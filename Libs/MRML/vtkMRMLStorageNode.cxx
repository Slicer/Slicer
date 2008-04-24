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
#include "vtkURIHandler.h"

//----------------------------------------------------------------------------
vtkMRMLStorageNode::vtkMRMLStorageNode()
{
  this->FileName = NULL;
  this->URI = NULL;
  this->URIHandler = NULL;
  this->UseCompression = 1;
  this->ReadState = this->Idle;
  this->WriteState = this->Idle;
  this->URIHandler = NULL;
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
  if ( this->URIHandler )
    {
    // don't delete it, it's obtained from the scene, it's just a pointer
    this->URIHandler = NULL;
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
  // if the URI is null, or emtpy assume the file name is set and return
  if ( this->GetURI() == NULL )
    {
    vtkDebugMacro("StageReadData: uri is null, setting state to transfer done");
    this->SetReadStateTransferDone();
    return;
    }
  if ( !(strcmp(this->GetURI(), "")) )
    {
    vtkDebugMacro("StageReadData: uri is empty, setting state to transfer done");
    this->SetReadStateTransferDone();
    return;
    }
  
  if (refNode == NULL)
    {
    vtkDebugMacro("StageReadData: input mrml node is null, returning.");
    return;
    }
  
  vtkCacheManager *cacheManager = this->Scene->GetCacheManager();
  const char *fname = NULL;
  if ( cacheManager != NULL )
    {
    fname = cacheManager->GetFilenameFromURI( this->GetURI() );
    }

  if (!this->SupportedFileType(fname))
    {
    // can't read this kind of file, so return
    this->SetReadStateIdle();
    vtkDebugMacro("StageReadData: can't read file type for URI : " << fname);
    return;
    }

  // need to get URI handlers from the scene
  if (this->Scene == NULL)
    {
    vtkDebugMacro("StageReadData: Cannot get mrml scene, unable to get remote file handlers.");
    return;
    }

  // Get the data io manager
  vtkDataIOManager *iomanager = this->Scene->GetDataIOManager();
  if (iomanager != NULL)
    {
    if (this->GetReadState() == this->Idle)
      {
      vtkDebugMacro("StageReadData: setting read state to pending, finding a URI handler and queuing read on the io manager");
      this->SetReadStatePending();
      // set up the data handler
      this->URIHandler = this->Scene->FindURIHandler(this->URI);
      if (this->URIHandler != NULL)
        {
        vtkDebugMacro("StageReadData: got a URI Handler");
        }
      else
        {
        vtkErrorMacro("StageReadData: unable to get a URI handler for " << this->URI << ", resetting stage to ready");
        this->SetReadStateIdle();
        return;
        }
      vtkDebugMacro("StageReadData: calling QueueRead on the io manager.");
      iomanager->QueueRead(refNode);
      }
    else
      {
      vtkDebugMacro("StageReadData: Read state is not pending, returning.");
      }
    }
  else
    {
    vtkWarningMacro("StageReadData: No IO Manager on the scene, returning.");
    }
  vtkDebugMacro("StageReadData: done");
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::StageWriteData ( vtkMRMLNode *refNode )
{
  if (this->URI == NULL)
    {
    this->SetWriteStateTransferDone();
    vtkDebugMacro("StageWriteData: uri is null, setting state to transfer done");
    return;
    }
  if ( !(strcmp(this->GetURI(), "")) )
    {
    vtkDebugMacro("StageWriteData: uri is empty, setting state to transfer done");
    this->SetReadStateTransferDone();
    return;
    }

  // need to get URI handlers from the scene
  if (this->Scene == NULL)
    {
    vtkDebugMacro("StageWriteData: Cannot get mrml scene, unable to get remote file handlers.");
    return;
    }
  if (refNode == NULL)
    {
    vtkDebugMacro("StageWriteData: input mrml node is null, returning.");
    return;
    }

   // Get the data io manager
   vtkDataIOManager *iomanager = this->Scene->GetDataIOManager();
   if (iomanager != NULL)
     {
     if (this->GetWriteState() == this->Idle)
       {
       vtkDebugMacro("StageWriteData: setting write state to pending, finding a URI handler and queuing write on the io manager");
       this->SetWriteStatePending();
       // set up the data handler
       this->URIHandler = this->Scene->FindURIHandler(this->URI);
       if (this->URIHandler != NULL)
         {
         vtkDebugMacro("StageWriteData: got a URI Handler");
         }
       else
         {
         vtkErrorMacro("StageWriteData: unable to get a URI handler for " << this->URI << ", resetting stage to idle");
         this->SetWriteStateIdle();
         return;
         }
       iomanager->QueueWrite(refNode);
       }
     else
       {
       vtkDebugMacro("StageWriteData: Write state is not pending, returning.");
       }
     }
   else
     {
     vtkWarningMacro("StageWriteData: No IO Manager on the scene, returning.");
     }
}

//----------------------------------------------------------------------------
const char * vtkMRMLStorageNode::GetStateAsString(int state)
{
  if (state == this->Pending)
    {
    return "Pending";
    }
  if (state == this->Idle)
    {
    return "Idle";
    }
  if (state == this->Scheduled)
    {
    return "Scheduled";
    }
  if (state == this->Transferring)
    {
    return  "Transferring";
    }
  if (state == this->TransferDone)
    {
    return "TransferDone";
    }
  if (state == this->Cancelled)
    {
    return "Cancelled";
    }
  return "(undefined)";
}

//----------------------------------------------------------------------------
std::string vtkMRMLStorageNode::GetFullNameFromFileName()
{
  std::string fullName = std::string("");
  if (this->GetFileName() == NULL)
    {
    vtkDebugMacro("GetFullNameFromFileName: filename is null, returning empty string");
    return fullName;
    }
  
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

//----------------------------------------------------------------------------
int vtkMRMLStorageNode::SupportedFileType(const char *fileName)
{
  vtkErrorMacro("SupportedFileType: sub class didn't define this method! (fileName = '" << fileName << "')");
  return 0;
}
