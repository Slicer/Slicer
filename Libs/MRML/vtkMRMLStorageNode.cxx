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
#include "vtkStringArray.h"
#include "vtkURIHandler.h"

#include <vtksys/stl/string>
#include <vtksys/SystemTools.hxx>

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
  this->FileNameList.clear();
  this->URIList.clear();

  this->SupportedWriteFileTypes = vtkStringArray::New();
  this->WriteFileFormat = NULL;
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

  if(this->SupportedWriteFileTypes)
    {
    this->SupportedWriteFileTypes->Delete();
    }
  if(this->WriteFileFormat)
    {
    delete [] this->WriteFileFormat;
    this->WriteFileFormat = NULL;
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
  for (int i = 0; i < this->GetNumberOfFileNames(); i++)
    {
    of << indent << " fileListMember" << i << "=\"" << vtkMRMLNode::URLEncodeString(this->GetNthFileName(i)) << "\"";
    }

  if (this->URI != NULL)
    {
    of << indent << " uri=\"" << vtkMRMLNode::URLEncodeString(this->URI) << "\"";
    }
  for (int i = 0; i < this->GetNumberOfURIs(); i++)
    {
    of << indent << " uriListMember" << i << "=\"" << vtkMRMLNode::URLEncodeString(this->GetNthURI(i)) << "\"";
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

  this->ResetFileNameList();
  this->ResetURIList();
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "fileName")) 
      {
      std::string filename = vtkMRMLNode::URLDecodeString(attValue);
      this->SetFileName(filename.c_str());
      }
    if (!strncmp(attName, "fileListMember", 14))
      {
      std::string filename = vtkMRMLNode::URLDecodeString(attValue);
      this->AddFileName(filename.c_str());
      }
    else if (!strcmp(attName, "uri"))
      {
      std::string uri = vtkMRMLNode::URLDecodeString(attValue);
      this->SetURI(uri.c_str());
      }
    else if (!strncmp(attName, "uriListMember", 13))
      {
      std::string uri = vtkMRMLNode::URLDecodeString(attValue);
      this->AddURI(uri.c_str());
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
  this->ResetFileNameList();
  for (int i = 0; i < node->GetNumberOfFileNames(); i++)
    {
    this->AddFileName(node->GetNthFileName(i));
    }
  this->SetURI(node->URI);
  this->ResetURIList();
  for (int i = 0; i < node->GetNumberOfURIs(); i++)
    {
    this->AddURI(node->GetNthURI(i));
    }
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

  for (int i = 0; i < this->GetNumberOfFileNames(); i++)
    {
    os << indent << "FileListMember: " << this->GetNthFileName(i) << "\n";
    }
  os << indent << "URI: " <<
    (this->URI ? this->URI : "(none)") << "\n";
  for (int i = 0; i < this->GetNumberOfURIs(); i++)
    {
    os << indent << "URIListMember: " << this->GetNthURI(i) << "\n";
    }
  os << indent << "UseCompression:   " << this->UseCompression << "\n";
  os << indent << "ReadState:  " << this->GetReadStateAsString() << "\n";
  os << indent << "WriteState: " << this->GetWriteStateAsString() << "\n";
  os << indent << "SupportedWriteFileTypes: \n";
  for(int i=0; i<this->SupportedWriteFileTypes->GetNumberOfTuples(); i++)
    {
    os << indent << "FileType: " << 
      this->SupportedWriteFileTypes->GetValue(i) << "\n";
    }
  os << indent << "WriteFileFormat: " <<
    (this->WriteFileFormat ? this->WriteFileFormat : "(none)") << "\n";

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
    
  // do not read if if we are not in the scene (for example inside snapshot)
  if ( !this->GetAddToScene() || !refNode->GetAddToScene() )
    {
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
std::string vtkMRMLStorageNode::GetFullNameFromNthFileName(int n)
{
  std::string fullName = std::string("");
  if (this->GetNumberOfFileNames() < n)
    {
    vtkDebugMacro("GetFullNameFromNthFileName: file name " << n << " not in list (size = " << this->GetNumberOfFileNames() << "), returning empty string");
    return fullName;
    }
  const char *fileName = this->GetNthFileName(n); //FileNameList[n].c_str();
  if (this->SceneRootDir != NULL && this->Scene->IsFilePathRelative(fileName)) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(fileName);
    }
  else 
    {
    fullName = std::string(fileName);
    }
  return fullName;
}

//----------------------------------------------------------------------------
int vtkMRMLStorageNode::SupportedFileType(const char *fileName)
{
  vtkErrorMacro("SupportedFileType: sub class didn't define this method! (fileName = '" << fileName << "')");
  return 0;
}


//----------------------------------------------------------------------------
unsigned int vtkMRMLStorageNode::AddFileName( const char* filename )
{
  std::string filenamestr (filename);
  this->FileNameList.push_back( filenamestr );
  return this->FileNameList.size();
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::ResetFileNameList( )
{
  this->FileNameList.resize( 0 );
}

//----------------------------------------------------------------------------
const char * vtkMRMLStorageNode::GetNthFileName(int n)
{
  if (this->GetNumberOfFileNames() < n)
    {
    return NULL;
    }
  else
    {
    return this->FileNameList[n].c_str();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::ResetNthFileName(int n, const char* fileName)
{
  if (fileName == NULL)
    {
    vtkErrorMacro("ResetNthFileName: given file name is null (n = " << n << ")");
    return;
    }
  if (this->GetNumberOfFileNames() >= n)
    {
    this->FileNameList[n] = std::string(fileName);
    }
  else
    {
    vtkErrorMacro("RestNthFileName: file name number " << n << " not already set, returning.");
    }
  
}

//----------------------------------------------------------------------------
unsigned int vtkMRMLStorageNode::AddURI( const char* filename )
{
  std::string filenamestr (filename);
  this->URIList.push_back( filenamestr );
  return this->URIList.size();
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::ResetURIList( )
{
  this->URIList.resize( 0 );
}

//----------------------------------------------------------------------------
const char * vtkMRMLStorageNode::GetNthURI(int n)
{
  if (this->GetNumberOfURIs() < n)
    {
    return NULL;
    }
  else
    {
    return this->URIList[n].c_str();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::ResetNthURI(int n, const char* fileName)
{
  if (fileName == NULL)
    {
    vtkErrorMacro("ResetNthURI: given URI is null (n = " << n << ")");
    return;
    }
  if (this->GetNumberOfURIs() >= n)
    {
    this->URIList[n] = std::string(fileName);
    }
  else
    {
    vtkErrorMacro("RestNthURI: URI number " << n << " not already set, returning.");
    }
  
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::SetDataDirectory(const char *dataDirName)
{
  if (dataDirName == NULL)
    {
    vtkErrorMacro("SetDataDirectory: input directory name is null, returning.");
    return;
    }
  // reset the filename
  vtksys_stl::string filePath = vtksys::SystemTools::GetFilenamePath(this->GetFileName());
  vtksys_stl::vector<vtksys_stl::string> pathComponents;
  vtksys::SystemTools::SplitPath(filePath.c_str(), pathComponents);
  vtksys_stl::string fileName, newFileName; 
  if (filePath != vtksys_stl::string(dataDirName))
    {
    fileName = vtksys::SystemTools::GetFilenameName(this->GetFileName());
    pathComponents.push_back(fileName);
    newFileName =  vtksys::SystemTools::JoinPath(pathComponents);
    vtkDebugMacro("SetDataDirectory: Resetting filename to " << newFileName.c_str());
    this->SetFileName(newFileName.c_str());
    pathComponents.pop_back();
    }
  // then reset all the files in the list
  for (int i = 0; i < this->GetNumberOfFileNames(); i++)
    {
    fileName = vtksys::SystemTools::GetFilenameName(this->GetNthFileName(i));
    pathComponents.push_back(fileName);
    newFileName =  vtksys::SystemTools::JoinPath(pathComponents);
    vtkDebugMacro("SetDataDirectory: Resetting " << i << "th filename to " << newFileName.c_str());
    this->ResetNthFileName(i, newFileName.c_str());
    pathComponents.pop_back();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::SetURIPrefix(const char *uriPrefix)
{
  if (uriPrefix == NULL)
    {
    vtkErrorMacro("SetURIPrefix: input prefix is null, returning.");
    return;
    }
  vtkWarningMacro("SetURIPrefix " << uriPrefix << " NOT IMPLEMENTED YET");
  // reset the uri

  // then reset all the uris in the list
}

//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLStorageNode::GetSupportedWriteFileTypes()
{
  if(this->SupportedWriteFileTypes->GetNumberOfTuples()==0)
    {
    this->InitializeSupportedWriteFileTypes();
    }
  return this->SupportedWriteFileTypes;
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->Reset();
  this->SupportedWriteFileTypes->SetNumberOfTuples(0);
}


