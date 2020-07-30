/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLStorageNode.cxx,v $
Date:      $Date: 2007/01/17 20:09:05 $
Version:   $Revision: 1.1.1.1 $

=========================================================================auto=*/


// MRML includes
#include "vtkCacheManager.h"
#include "vtkDataFileFormatHelper.h"
#include "vtkDataIOManager.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorableNode.h"
#include "vtkMRMLStorageNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkStringArray.h>
#include <vtkURIHandler.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <algorithm>
#include <sstream>

//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkMRMLStorageNode, URIHandler, vtkURIHandler)

//----------------------------------------------------------------------------
vtkMRMLStorageNode::vtkMRMLStorageNode()
{
  this->HideFromEditors = 1;
  this->FileName = nullptr;
  this->TempFileName = nullptr;
  this->URI = nullptr;
  this->URIHandler = nullptr;
  this->UseCompression = 1;
  this->ReadState = this->Idle;
  this->WriteState = this->Idle;
  this->URIHandler = nullptr;
  this->FileNameList.clear();
  this->URIList.clear();

  this->SupportedReadFileTypes = vtkStringArray::New();
  this->SupportedWriteFileTypes = vtkStringArray::New();
  this->WriteFileFormat = nullptr;
  this->StoredTime = vtkTimeStamp::New();
  this->UserMessages = vtkMRMLMessageCollection::New();
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode::~vtkMRMLStorageNode()
{
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = nullptr;
    }
  if (this->TempFileName)
    {
    delete [] this->TempFileName;
    this->TempFileName = nullptr;
    }
  if (this->URI)
    {
    delete [] this->URI;
    this->URI = nullptr;
    }
  if ( this->URIHandler )
    {
    // don't delete it, it's obtained from the scene, it's just a pointer
    this->URIHandler = nullptr;
    }

  if(this->SupportedReadFileTypes)
    {
    this->SupportedReadFileTypes->Delete();
    this->SupportedReadFileTypes = nullptr;
    }
  if(this->SupportedWriteFileTypes)
    {
    this->SupportedWriteFileTypes->Delete();
    this->SupportedWriteFileTypes = nullptr;
    }
  if(this->WriteFileFormat)
    {
    delete [] this->WriteFileFormat;
    this->WriteFileFormat = nullptr;
    }
  if (this->StoredTime)
    {
    this->StoredTime->Delete();
    this->StoredTime = nullptr;
    }
  if (this->UserMessages)
    {
    this->UserMessages->Delete();
    this->UserMessages = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  if (this->FileName != nullptr)
    {
    // convert to relative filename
    std::string name = this->FileName;
    if (this->GetScene() && !this->IsFilePathRelative(this->FileName))
      {
      name = vtksys::SystemTools::RelativePath(this->GetScene()->GetRootDirectory(), this->FileName);
      }

    of << " fileName=\"" << vtkMRMLNode::URLEncodeString(name.c_str()) << "\"";

    // if there is a file list, add the archetype to it. add file will check
    // that it's not already there. currently needed for reading in multi
    // volume files with the vtk itk io factory 10/17/08. - NOT TESTED YET
    /*if (this->GetNumberOfFileNames() > 0)
      {
      this->AddFileName(this->FileName);
      }
    */
    if (this->GetScene() && this->IsFilePathRelative(this->FileName))
      {
      // now that we've written out the relative path, go back to keeping an
      // absolute one here so that any future saves in different scene root
      // directories will be able to compute the correct relative path.

      const char * absFilePath = this->GetAbsoluteFilePath(this->FileName);
      if (absFilePath)
        {
        vtkDebugMacro("WriteXML: going back to absolute path for file name " << this->FileName << ", using " << absFilePath);
        this->SetFileName(absFilePath);
        }
      else
        {
        vtkWarningMacro("WriteXML: unable to convert relative file path to absolute, still using " << this->FileName);
        this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent,
          std::string("WriteXML: unable to convert relative file path to absolute, still using ") + this->FileName);
        }
      }
    }
  for (int i = 0; i < this->GetNumberOfFileNames(); i++)
    {
    // convert to relative filename
    std::string name = this->GetNthFileName(i);
    if (this->GetScene() && !this->IsFilePathRelative(this->GetNthFileName(i)))
      {
      name = vtksys::SystemTools::RelativePath(this->GetScene()->GetRootDirectory(), this->GetNthFileName(i));
      }

    of << " fileListMember" << i << "=\"" << vtkMRMLNode::URLEncodeString(name.c_str()) << "\"";
    if (this->GetScene() && this->IsFilePathRelative(this->GetNthFileName(i)))
      {
      // go back to absolute
      const char *absFilePath = this->GetAbsoluteFilePath(this->GetNthFileName(i));
      if (absFilePath)
        {
        vtkDebugMacro("WriteXML: going back to absolute path for file name " << this->GetNthFileName(i) << ", using " << absFilePath);
        this->ResetNthFileName(i, absFilePath);
        }
      else
        {
        vtkWarningMacro("WriteXML: unable to convert relative file path to absolute, still using " << this->GetNthFileName(i));
        this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent,
          std::string("WriteXML: unable to convert relative file path to absolute, still using ") + this->GetNthFileName(i));
        }
      }
    }

  if (this->URI != nullptr)
    {
    of << " uri=\"" << vtkMRMLNode::URLEncodeString(this->URI) << "\"";
    }
  for (int i = 0; i < this->GetNumberOfURIs(); i++)
    {
    of << " uriListMember" << i << "=\"" << vtkMRMLNode::URLEncodeString(this->GetNthURI(i)) << "\"";
    }

  std::stringstream ss;
  ss << this->UseCompression;
  of << " useCompression=\"" << ss.str() << "\"";

  if (!this->CompressionParameter.empty())
    {
    of << " compressionParameter=\"" << this->XMLAttributeEncodeString(this->CompressionParameter) << "\"";
    }

  if (this->GetDefaultWriteFileExtension() != nullptr)
    {
    of << " defaultWriteFileExtension=\"" << this->GetDefaultWriteFileExtension() << "\"";
    }

  of << " readState=\"" << this->ReadState <<  "\"";
  of << " writeState=\"" << this->WriteState <<  "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  this->ResetFileNameList();
  this->ResetURIList();
  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "fileName"))
      {
      std::string filename = vtkMRMLNode::URLDecodeString(attValue);

      // convert to absolute filename
      std::string name;
      if (this->GetScene() &&
          this->GetScene()->GetRootDirectory() &&
          this->IsFilePathRelative(filename.c_str()))
        {
        name = this->GetScene()->GetRootDirectory();
        if (name[name.size()-1] != '/')
          {
          name = name + std::string("/");
          }
        }

      name += filename;
      // use collapse full path, since if there's a sym link somewhere in the
      // relative path, the readers will fail
      std::string collapsedFullPath = vtksys::SystemTools::CollapseFullPath(name.c_str());
      vtkDebugMacro("ReadXMLAttributes: collapsed path = " << collapsedFullPath.c_str());

      this->SetFileName(collapsedFullPath.c_str());
      }
    if (!strncmp(attName, "fileListMember", 14))
      {
      std::string filename = vtkMRMLNode::URLDecodeString(attValue);

      // convert to absolute filename
      std::string name;
      if (this->GetScene() &&
          this->GetScene()->GetRootDirectory() &&
          this->IsFilePathRelative(filename.c_str()))
        {
        name = this->GetScene()->GetRootDirectory();
        if (name[name.size()-1] != '/')
          {
          name = name + std::string("/");
          }
        }

      name += filename;
      std::string collapsedFullPath = vtksys::SystemTools::CollapseFullPath(name.c_str());
      vtkDebugMacro("ReadXMLAttributes: collapsed path for " << attName << " = " << collapsedFullPath.c_str());
      // check if this file exists
      if (vtksys::SystemTools::FileExists(collapsedFullPath.c_str(), true) == false &&
          this->GetFileName() != nullptr)
        {
        vtkDebugMacro("ReadXMLAttributes: File list member " << filename << " is relative, but is not found relative to the mrml root directory. " << collapsedFullPath << " was not found. Trying to find it relative to the archetype: " << this->GetFileName());
        // get the directory of the file name
        std::string fileNameDirectory = vtksys::SystemTools::GetParentDirectory(this->GetFileName());
        vtkDebugMacro("ReadXMLAttributes: Directory of archetype = " << fileNameDirectory);
        // add a trailing slash if missing
        if (fileNameDirectory[fileNameDirectory.size()-1]  != '/')
          {
          fileNameDirectory = fileNameDirectory + std::string("/");
          }
        fileNameDirectory += filename;
        vtkDebugMacro("ReadXMLAttributes: New uncollapsed path = " << fileNameDirectory);
        std::string secondCollapsedFullPath = vtksys::SystemTools::CollapseFullPath(fileNameDirectory.c_str());
        vtkDebugMacro("ReadXMLAttributes: New collapsed path for " << attName << " = " << secondCollapsedFullPath.c_str());
        if (vtksys::SystemTools::FileExists(secondCollapsedFullPath.c_str(), true) == false)
          {
          vtkWarningMacro("ReadXMLAttributes: file list member " << filename << " is relative, but not found relative to the mrml root directory. Trying to find the path relative to the archetype doesn't work either: " << secondCollapsedFullPath << " not found, going back to using path from mrml root dir of " << collapsedFullPath);
          }
        else
          {
          vtkWarningMacro("ReadXMLAttributes: file list member " << filename << " is relative, but not found relative to the mrml root directory: " << collapsedFullPath << " is invalid. Found the file relative to the archetype, so using path  " <<  secondCollapsedFullPath);
          collapsedFullPath = secondCollapsedFullPath;
          }
        }
      this->AddFileName(collapsedFullPath.c_str());
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
    else if (!strncmp(attName, "defaultWriteFileExtension", 25))
      {
      this->SetDefaultWriteFileExtension(attValue);
      }

    else if (!strcmp(attName, "useCompression"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->UseCompression;
      }
    else if (!strcmp(attName, "compressionParameter"))
      {
      this->CompressionParameter = attValue;
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

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLStorageNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLStorageNode *node = (vtkMRMLStorageNode *) anode;
  this->SetFileName(node->FileName);
  this->FileNameList = node->FileNameList; // a loop on AddFileName would be n log(n)
  this->SetURI(node->URI);
  this->ResetURIList();
  for (int i = 0; i < node->GetNumberOfURIs(); i++)
    {
    this->AddURI(node->GetNthURI(i));
    }
  this->SetUseCompression(node->UseCompression);
  this->SetCompressionParameter(node->CompressionParameter);
  this->SetReadState(node->ReadState);
  this->SetWriteState(node->WriteState);
  this->SetDefaultWriteFileExtension(node->GetDefaultWriteFileExtension());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "FileName: " <<
    (this->FileName ? this->FileName : "(none)") << "\n";

  for (int i = 0; i < this->GetNumberOfFileNames(); i++)
    {
    os << indent << "FileListMember: " << (this->GetNthFileName(i) == nullptr ? "NULL" : this->GetNthFileName(i)) << "\n";
    }
  os << indent << "URI: " <<
    (this->URI ? this->URI : "(none)") << "\n";
  for (int i = 0; i < this->GetNumberOfURIs(); i++)
    {
    os << indent << "URIListMember: " << this->GetNthURI(i) << "\n";
    }
  os << indent << "UseCompression:   " << this->UseCompression << "\n";
  if (!this->CompressionParameter.empty())
    {
    os << indent << "CompressionParameter:   " << this->CompressionParameter << "\n";
    }

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
  os << indent << "DefaultWriteFileExtension: " <<
    (this->GetDefaultWriteFileExtension() ? this->GetDefaultWriteFileExtension() : "(none)") << "\n";

  os << indent << "TempFileName: " << (this->TempFileName ? this->TempFileName : "(none)") << "\n";
  if (this->UserMessages)
    {
    this->UserMessages->PrintSelf(os, indent);
    }
  else
    {
    os << indent << "UserMessages: (none)\n";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::ProcessMRMLEvents ( vtkObject *vtkNotUsed(caller), unsigned long event, void *callData )
{
  if (event ==  vtkCommand::ProgressEvent)
    {
    this->InvokeEvent ( vtkCommand::ProgressEvent,callData );
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::StageReadData ( vtkMRMLNode *refNode )
{
  // if the URI is null, or empty assume the file name is set and return
  if ( this->Scene )
    {
    // this event is being detected by GUI to provide feedback during load
    // of data. But,
    // commented out for now because CLI modules are using MRML to write
    // data in another thread, causing GUI to crash.
//    this->Scene->InvokeEvent (vtkMRMLScene::ImportProgressFeedbackEvent );
    }

  if ( this->GetURI() == nullptr )
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

  if (refNode == nullptr)
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
  const char *fname = nullptr;
  if ( cacheManager != nullptr )
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
  if (this->Scene == nullptr)
    {
    vtkDebugMacro("StageReadData: Cannot get mrml scene, unable to get remote file handlers.");
    return;
    }

  // Get the data io manager
  vtkDataIOManager *iomanager = this->Scene->GetDataIOManager();
  if (iomanager != nullptr)
    {
    if (this->GetReadState() == this->Idle)
      {
      vtkDebugMacro("StageReadData: setting read state to pending, finding a URI handler and queuing read on the io manager");
      this->SetReadStatePending();
      // set up the data handler if it's not set already (may want to over
      // ride, esp. for the XND handler)
      if (this->URIHandler == nullptr)
        {
        this->URIHandler = this->Scene->FindURIHandler(this->URI);
        }
      if (this->URIHandler != nullptr)
        {
        vtkDebugMacro("StageReadData: got a URI Handler");
        }
      else
        {
        vtkErrorMacro("StageReadData: unable to get a URI handler for " << this->URI << ", resetting stage to idle");
        this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
          std::string("StageReadData: unable to get a URI handler for ") + this->URI + ", resetting stage to idle");
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
    this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent,
      std::string("StageReadData: No IO Manager on the scene, returning."));
    }
  vtkDebugMacro("StageReadData: done");
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::StageWriteData ( vtkMRMLNode *refNode )
{
  if ( this->Scene )
    {
    // this event is being detected by GUI to provide feedback during load
    // of data. But,
    // commented out for now because CLI modules are using MRML to write
    // data in another thread, causing GUI to crash.
//    this->Scene->InvokeEvent (vtkMRMLScene::SaveProgressFeedbackEvent );
    }

  if (this->WriteState == Cancelled || this->WriteState == SkippedNoData)
    {
    return;
    }
  if (this->URI == nullptr)
    {
    this->SetWriteStateTransferDone();
    vtkDebugMacro("StageWriteData: uri is null, setting state to transfer done");
    return;
    }
  if ( !(strcmp(this->GetURI(), "")) )
    {
    vtkDebugMacro("StageWriteData: uri is empty, setting state to transfer done");
    this->SetWriteStateTransferDone();
    return;
    }

  // need to get URI handlers from the scene
  if (this->Scene == nullptr)
    {
    vtkDebugMacro("StageWriteData: Cannot get mrml scene, unable to get remote file handlers.");
    return;
    }
  if (refNode == nullptr)
    {
    vtkDebugMacro("StageWriteData: input mrml node is null, returning.");
    return;
    }

   // Get the data io manager
   vtkDataIOManager *iomanager = this->Scene->GetDataIOManager();
   if (iomanager != nullptr)
     {
     if (this->GetWriteState() == this->Idle)
       {
       vtkDebugMacro("StageWriteData: finding a URI handler and queuing write on the io manager");
       // set up the data handler if it's not set already
       if (this->URIHandler == nullptr)
         {
         this->URIHandler = this->Scene->FindURIHandler(this->URI);
         }
       if (this->URIHandler == nullptr)
         {
         vtkErrorMacro("StageWriteData: unable to get a URI handler for " << this->URI << ", resetting stage to idle");
         this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
           std::string("StageWriteData: unable to get a URI handler for ") + this->URI + ", resetting stage to idle");
         return;
         }
       vtkDebugMacro("StageWriteData: got a URI Handler");
       this->SetWriteStatePending();
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
     this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent,
       std::string("StageWriteData: No IO Manager on the scene, returning."));
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
  if (state == this->SkippedNoData)
    {
    return "SkippedNoData";
    }
  return "(undefined)";
}

//----------------------------------------------------------------------------
std::string vtkMRMLStorageNode::GetFullNameFromFileName()
{
  return this->GetFullNameFromNthFileName(-1);
}

//----------------------------------------------------------------------------
std::string vtkMRMLStorageNode::GetFullNameFromNthFileName(int n)
{
  std::string fullName = std::string("");
  const char *fileName;
  if (n == -1)
    {
    // special case, use the archetype
    if (this->GetFileName() == nullptr)
      {
      vtkDebugMacro("GetFullNameFromFileName: filename is null, returning empty string");
      return fullName;
      }
    fileName = this->GetFileName();
    }
  else
    {
    if (n < 0 || this->GetNumberOfFileNames() < n)
      {
      vtkDebugMacro("GetFullNameFromNthFileName: file name " << n << " not in list (size = " << this->GetNumberOfFileNames() << "), returning empty string");
      return fullName;
      }
    fileName = this->GetNthFileName(n);
    }

  if (fileName == nullptr)
    {
    vtkDebugMacro("GetFullNameFromNthFileName: n = " << n << " have a null filename, returning empty string");
    return fullName;
    }

  vtkDebugMacro("GetFullNameFromNthFileName: n = " << n << ", using file name '" << fileName << "'");

  if (this->Scene != nullptr &&
      this->Scene->GetRootDirectory() != nullptr &&
      this->IsFilePathRelative(fileName))
    {
    vtkDebugMacro("GetFullNameFromNthFileName: n = " << n << ", scene root dir = '" << this->Scene->GetRootDirectory() << "'");
    // use the system tools to join the two paths and then collapse them
    if (strcmp(this->Scene->GetRootDirectory(), "") == 0)
      {
      vtkDebugMacro("GetFullNameFromNthFileName: scene root dir is empty, just collapsing the fileName " << fileName);
      fullName = vtksys::SystemTools::CollapseFullPath(fileName);
      }
    else
      {
      // if the root directory is ./, using it as a base dir for
      // CollapseFullPath doesn't work well, so collapse it into a full path
      std::string rootDirCollapsed = vtksys::SystemTools::CollapseFullPath(this->Scene->GetRootDirectory());
      vtkDebugMacro("GetFullNameFromNthFileName: using scene root dir to collapse file name: " << rootDirCollapsed);
      fullName = vtksys::SystemTools::CollapseFullPath(fileName, rootDirCollapsed.c_str());
      }
    }
  else
    {
    if (this->Scene == nullptr)
      {
      vtkDebugMacro("GetFullNameFromNthFileName: scene is null, returning " << fileName);
      }
    else
      {
      vtkDebugMacro("GetFullNameFromNthFileName: scene root dir = " << (this->Scene->GetRootDirectory() != nullptr ? this->Scene->GetRootDirectory() : "null") << ", relative = " << (this->IsFilePathRelative(fileName) ? "yes" : "no"));
      }
    fullName = std::string(fileName);
    }
  vtkDebugMacro("GetFullNameFromNthFileName: " << n << ", returning full name " << fullName);
  return fullName;
}

//----------------------------------------------------------------------------
int vtkMRMLStorageNode::SupportedFileType(const char *fileName)
{
  // check to see which file name we need to check
  std::string name;
  if (fileName)
    {
    name = std::string(fileName);
    }
  else if (this->FileName != nullptr)
    {
    name = std::string(this->FileName);
    }
  else if (this->URI != nullptr)
    {
    name = std::string(this->URI);
    }
  else
    {
    vtkWarningMacro("SupportedFileType: no file name to check");
    this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent,
      std::string("SupportedFileType: no file name to check"));
    return 0;
    }

  if (name.size() == 0)
    {
    return 0;
    }

  // Check if any extension is matched with readable file extensions
  std::string extension = this->GetSupportedFileExtension(name.c_str(), true, false);
  if (!extension.empty())
    {
    // exact match found
    return 1;
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLStorageNode::FileNameIsInList(const char *fileName)
{
  if (fileName == nullptr)
    {
    return 0;
    }
  const std::string fileNameString(fileName);
  const int fileNameIsRelative =  this->IsFilePathRelative(fileName);
  const char *rootDir = this->Scene ? this->Scene->GetRootDirectory() : ".";
  const std::string relativeFileName = fileNameIsRelative ?
    fileNameString : vtksys::SystemTools::RelativePath(rootDir, fileName);

  for (std::vector<std::string>::const_iterator it = this->FileNameList.begin();
       it != this->FileNameList.end(); ++it)
    {
    const int thisFileIsRelative = this->IsFilePathRelative(it->c_str());
    // make sure we're comparing apples to apples
    if (fileNameIsRelative != thisFileIsRelative)
      {
      vtkDebugMacro("WARNING: trying to determine if file " << fileName
        << " is already in the list and comparing against " << it->c_str()
        << ", they have mismatched absolute/relative paths. "
        << "Using scene root dir to disambiguate: " << rootDir);
      std::string thisRelativeFileName = thisFileIsRelative ?
        *it : vtksys::SystemTools::RelativePath(rootDir, it->c_str());
      vtkDebugMacro("\tComparing " << relativeFileName
        << " and " << thisRelativeFileName);
      if ( relativeFileName == thisRelativeFileName )
        {
        return 1;
        }
      }
    else if (*it == fileNameString)
      {
      return 1;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
unsigned int vtkMRMLStorageNode::AddFileName( const char* filename )
{
  if (!filename)
    {
    vtkErrorMacro("AddFileName: can't add a null file name");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("AddFileName: can't add a null file name"));
    return 0;
    }
  std::string filenamestr (filename);
  if (!this->FileNameIsInList(filename))
    {
    vtkDebugMacro("AddFileName: adding " << filename);
    this->FileNameList.push_back( filenamestr );
    }
  return (unsigned int)this->FileNameList.size();
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::ResetFileNameList( )
{
  this->FileNameList.resize( 0 );
}

//----------------------------------------------------------------------------
const char * vtkMRMLStorageNode::GetNthFileName(int n) const
{
  if (n < 0 || n >= this->GetNumberOfFileNames())
    {
    return nullptr;
    }
  return this->FileNameList[n].c_str();
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::ResetNthFileName(int n, const char* fileName)
{
  if (fileName == nullptr)
    {
    vtkErrorMacro("ResetNthFileName: given file name is null (n = " << n << ")");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("ResetNthFileName: given file name is null (n = ") + std::to_string(n) + ")");
    return;
    }
  if (n < 0)
    {
    vtkErrorMacro("ResetNthFileName: invalid file name number (n = " << n << ")");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("ResetNthFileName: invalid file name number (n = ") + std::to_string(n) + ")");
    return;
    }
  else if (n >= this->GetNumberOfFileNames())
    {
    vtkErrorMacro("ResetNthFileName: file name number " << n << " not already set, returning.");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("ResetNthFileName: file name number ") + std::to_string(n) + " not already set, returning.");
    return;
    }
  this->FileNameList[n] = fileName;
}

//----------------------------------------------------------------------------
unsigned int vtkMRMLStorageNode::AddURI( const char* uri )
{
  if (uri == nullptr)
    {
    vtkErrorMacro("AddURI: can't add a null URI");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("AddURI: can't add a null URI"));
    return 0;
    }
  std::string uristr (uri);
  this->URIList.push_back( uristr );
  return (unsigned int)this->URIList.size();
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
    return nullptr;
    }
  else
    {
    return this->URIList[n].c_str();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::ResetNthURI(int n, const char* uri)
{
  if (uri == nullptr)
    {
    vtkErrorMacro("ResetNthURI: given URI is null (n = " << n << ")");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("ResetNthURI: given URI is null (n = ") + std::to_string(n) + ")");
    return;
    }
  if (n >= 0 && this->GetNumberOfURIs() >= n)
    {
    this->URIList[n] = std::string(uri);
    }
  else
    {
    vtkErrorMacro("RestNthURI: URI number " << n << " not already set, returning.");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("RestNthURI: URI number ") + std::to_string(n) + " not already set, returning.");
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::SetDataDirectory(const char *dataDirName)
{
  if (dataDirName == nullptr)
    {
    vtkErrorMacro("SetDataDirectory: input directory name is null, returning.");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("SetDataDirectory: input directory name is null, returning."));
    return;
    }
  if (this->GetFileName() == nullptr)
    {
    vtkWarningMacro("SetDataDirectory: file name is null, no reason to reset data directory.");
    this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent,
      std::string("SetDataDirectory: file name is null, no reason to reset data directory."));
    return;
    }
  // reset the filename
  std::string filePath = vtksys::SystemTools::GetFilenamePath(this->GetFileName());
  vtkDebugMacro("SetDataDirectory: from FileName " << (this->GetFileName() ? this->GetFileName() : "NULL") << ", got filePath = " << filePath.c_str());
  std::vector<std::string> pathComponents;
  vtksys::SystemTools::SplitPath(dataDirName, pathComponents);
  std::string fileName, newFileName;
  if (filePath != std::string(dataDirName))
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
  if (uriPrefix == nullptr)
    {
    vtkErrorMacro("SetURIPrefix: input prefix is null, returning.");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("SetURIPrefix: input prefix is null, returning."));
    return;
    }
  vtkWarningMacro("SetURIPrefix " << uriPrefix << " NOT IMPLEMENTED YET");
  this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent,
    std::string("SetURIPrefix ") + uriPrefix + " NOT IMPLEMENTED YET");
  // reset the uri

  // then reset all the uris in the list
}

//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLStorageNode::GetSupportedReadFileTypes()
{
  if (this->SupportedReadFileTypes->GetNumberOfTuples() == 0)
    {
    // File types list has not been initialized yet
    this->InitializeSupportedReadFileTypes();
    if (this->SupportedReadFileTypes->GetNumberOfTuples() == 0)
      {
      // InitializeSupportedReadFileTypes has not added any specific file formats,
      // which means that all file types are supported.
      this->SupportedReadFileTypes->InsertNextValue("All files (.*)");
      }
    }
  return this->SupportedReadFileTypes;
}

//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLStorageNode::GetSupportedWriteFileTypes()
{
  if(this->SupportedWriteFileTypes->GetNumberOfTuples()==0)
    {
    // File types list has not been initialized yet
    this->InitializeSupportedWriteFileTypes();
    }
  return this->SupportedWriteFileTypes;
}

//------------------------------------------------------------------------------
const char* vtkMRMLStorageNode::GetDefaultWriteFileExtension()
{
  // for backward compatibility, we return nullptr by default
  if (this->DefaultWriteFileExtension.empty())
    {
    return nullptr;
    }
  return this->DefaultWriteFileExtension.c_str();
};

//------------------------------------------------------------------------------
void vtkMRMLStorageNode::GetFileExtensionsFromFileTypes(vtkStringArray* inputFileTypes, vtkStringArray* outputFileExtensions)
{
  if (inputFileTypes == nullptr || outputFileExtensions == nullptr)
    {
    vtkErrorMacro("vtkMRMLStorageNode::GetSupportedReadFileExtensions failed: invalid inputs");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("vtkMRMLStorageNode::GetSupportedReadFileExtensions failed: invalid inputs"));
    return;
    }
  outputFileExtensions->Reset();
  const int formatCount = inputFileTypes->GetNumberOfValues();
  for (int formatIt = 0; formatIt < formatCount; ++formatIt)
    {
    std::string format = inputFileTypes->GetValue(formatIt);
    std::string ext = vtkDataFileFormatHelper::GetFileExtensionFromFormatString(format.c_str());
    if (ext.empty())
      {
      ext = ".*";
      }
    if (outputFileExtensions->LookupValue(ext) >= 0)
      {
      // already in the list
      continue;
      }
    outputFileExtensions->InsertNextValue(ext);
    }
}

//------------------------------------------------------------------------------
std::string vtkMRMLStorageNode::GetSupportedFileExtension(const char* fileName /* =nullptr */, bool includeReadable /* =true */, bool includeWriteable /* =true */)
{
  std::string fileNameStr;
  if (fileName)
    {
    fileNameStr = fileName;
    }
  else
    {
    fileNameStr = this->GetFullNameFromFileName();
    }
  std::string longestFoundExtension;
  fileNameStr = vtksys::SystemTools::LowerCase(fileNameStr);
  vtkNew<vtkStringArray> supportedFileExtensions;
  if (includeReadable)
    {
    this->GetFileExtensionsFromFileTypes(this->GetSupportedReadFileTypes(), supportedFileExtensions.GetPointer());
    }
  if (includeWriteable)
    {
    vtkNew<vtkStringArray> supportedWriteFileExtensions;
    this->GetFileExtensionsFromFileTypes(this->GetSupportedWriteFileTypes(), supportedWriteFileExtensions.GetPointer());
    supportedFileExtensions->InsertTuples(0, supportedWriteFileExtensions->GetNumberOfValues(), 0, supportedWriteFileExtensions.GetPointer());
    }

  const int extCount = supportedFileExtensions->GetNumberOfValues();
  bool wildcardMatchEnabled = false;
  for (int extIt = 0; extIt < extCount; ++extIt)
    {
    std::string ext = vtksys::SystemTools::LowerCase(supportedFileExtensions->GetValue(extIt));
    std::string foundExt;
    if (ext != ".*")
      {
      if (fileNameStr.length() > ext.length() &&
        fileNameStr.compare(fileNameStr.length() - ext.length(), ext.length(), ext) == 0)
        {
        foundExt = ext;
        }
      }
    else
      {
      wildcardMatchEnabled = true;
      }
    if (foundExt.size() > longestFoundExtension.size())
      {
      longestFoundExtension = foundExt;
      }
    }
  // If any specific extension is matched then use that (ignore wildcard;
  // this allows using dots in the filename).
  if (!longestFoundExtension.empty())
  {
    return longestFoundExtension;
  }
  if (wildcardMatchEnabled)
  {
    // extension is *, match last extension
    longestFoundExtension = vtksys::SystemTools::GetFilenameExtension(fileNameStr); // include everything after the first . (. included)
    if (longestFoundExtension.empty())
    {
      longestFoundExtension = ".";
    }
  }
  return longestFoundExtension;
}

//------------------------------------------------------------------------------
void vtkMRMLStorageNode::SetDefaultWriteFileExtension(const char* ext)
{
  std::string extStr = (ext ? ext : "");
  if (extStr == this->DefaultWriteFileExtension)
    {
    return;
    }
  this->DefaultWriteFileExtension = extStr;
  this->Modified();
};

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedWriteFileTypes->Reset();
  this->SupportedWriteFileTypes->SetNumberOfTuples(0);
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->Reset();
  this->SupportedWriteFileTypes->SetNumberOfTuples(0);
}

//------------------------------------------------------------------------------
int vtkMRMLStorageNode::IsFilePathRelative(const char * filepath)
{
  if (filepath == nullptr)
    {
    vtkErrorMacro("IsFilePathRelative: input file path is null! Returning 0");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("IsFilePathRelative: input file path is null! Returning 0"));
    return 0;
    }
  if ( this->Scene )
    {
    return this->Scene->IsFilePathRelative(filepath);
    }
  else
    {
    const bool absoluteFilePath = vtksys::SystemTools::FileIsFullPath(filepath);
    return absoluteFilePath ? 0 : 1;
    }
}

//------------------------------------------------------------------------------
const char *vtkMRMLStorageNode::GetAbsoluteFilePath(const char *inputPath)
{
  if (inputPath == nullptr)
    {
    vtkErrorMacro("GetAbsoluteFilePath: input path is null.");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("GetAbsoluteFilePath: input path is null."));
    return nullptr;
    }
  if (!this->IsFilePathRelative(inputPath))
    {
    // the path is already absolute, return it
    return inputPath;
    }
  if (!this->GetScene() ||
      !this->GetScene()->GetRootDirectory())
    {
    vtkErrorMacro("GetAbsoluteFilePath: have a relative path " << inputPath << " but no scene or root directory to find it from!");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("GetAbsoluteFilePath: have a relative path ") + inputPath + " but no scene or root directory to find it from!");
    return nullptr;
    }

  std::string path = this->GetScene()->GetRootDirectory();
  if (path.size() > 0 &&
      path[path.size()-1] != '/')
    {
    path = path + std::string("/");
    }
  // now add the input relative path to the end
  path += inputPath;
  // collapse it
  std::string collapsedFullPath = vtksys::SystemTools::CollapseFullPath(path.c_str());
  vtkDebugMacro("GetAbsoluteFilePath: for relative path " << inputPath << ", collapsed full path = " << collapsedFullPath.c_str());
  this->SetTempFileName(collapsedFullPath.c_str());
  return this->GetTempFileName();
}

//------------------------------------------------------------------------------
void vtkMRMLStorageNode::InvalidateFile()
{
  this->StoredTime->Delete();
  this->StoredTime = vtkTimeStamp::New();
}

//------------------------------------------------------------------------------
vtkTimeStamp vtkMRMLStorageNode::GetStoredTime()
{
  return *this->StoredTime;
}

//----------------------------------------------------------------------------
bool vtkMRMLStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  // Must be reimplemented in subclass;
  return refNode->IsA("vtkMRMLStorableNode");
}

//----------------------------------------------------------------------------
bool vtkMRMLStorageNode::CanWriteFromReferenceNode(vtkMRMLNode *refNode)
{
  return this->CanReadInReferenceNode(refNode);
}

//------------------------------------------------------------------------------
int vtkMRMLStorageNode::ReadData(vtkMRMLNode* refNode, bool temporary)
{
  if (refNode == nullptr)
    {
    vtkErrorMacro("ReadData: can't read into a null node");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("ReadData: can't read into a null node"));
    return 0;
    }

  if ( !this->CanReadInReferenceNode(refNode) )
    {
    return 0;
    }

  // do not read if if we are not in the scene (for example inside snapshot)
  if ( !refNode->GetAddToScene() )
    {
    return 0;
    }

  if (this->GetScene() && this->GetScene()->GetReadDataOnLoad() == 0)
    {
    return 0;
    }

  if (this->GetFileName() == nullptr && this->GetURI() == nullptr)
    {
    vtkErrorMacro("ReadData: both filename and uri are null.");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("ReadData: both filename and uri are null."));
    return 0;
    }

  this->StageReadData(refNode);
  if ( this->GetReadState() != this->TransferDone )
    {
    // remote file download hasn't finished
    vtkWarningMacro("ReadData: read state is pending, remote download hasn't finished yet");
    this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent,
      std::string("ReadData: read state is pending, remote download hasn't finished yet"));
    return 0;
    }
  vtkDebugMacro("ReadData: read state is ready, "
    <<  "URI = " << (this->GetURI() == nullptr ? "null" : this->GetURI()) << ", "
    << "filename = " << (this->GetFileName() == nullptr ? "null" : this->GetFileName()));
  int res = this->ReadDataInternal(refNode);
  if (res)
    {
    vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(refNode);
    if (storableNode)
      {
      storableNode->SetAndObserveStorageNodeID(this->GetID());
      }
    this->SetReadStateIdle();
    if (!temporary)
      {
      this->StoredTime->Modified();
      }
    }
  return res;
}

//------------------------------------------------------------------------------
int vtkMRMLStorageNode::WriteData(vtkMRMLNode* refNode)
{
  this->WriteState = this->Idle;
  if (refNode == nullptr)
    {
    vtkErrorMacro("WriteData: can't write, input node is null");
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
      std::string("WriteData: can't write, input node is null"));
    return 0;
    }

  // test whether refNode is a valid node to hold a volume
  if (!this->CanWriteFromReferenceNode(refNode) )
    {
    return 0;
    }

  int res = this->WriteDataInternal(refNode);

  if (res)
    {
    this->StageWriteData(refNode);
    this->StoredTime->Modified();
    }

  return res;
}

//------------------------------------------------------------------------------
int vtkMRMLStorageNode::ReadDataInternal(vtkMRMLNode* vtkNotUsed(refNode))
{
  return 0;
}

//------------------------------------------------------------------------------
int vtkMRMLStorageNode::WriteDataInternal(vtkMRMLNode* vtkNotUsed(refNode))
{
  return 0;
}

//------------------------------------------------------------------------------
std::string vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(const std::string& filename)
{
  std::string extension = vtksys::SystemTools::GetFilenameLastExtension(filename);
  if (extension.compare(".gz") == 0)
    {
    // some file formats have a compressed version ending with gz, return
    // the full extension
    extension = vtksys::SystemTools::GetFilenameLastExtension(vtksys::SystemTools::GetFilenameWithoutLastExtension(filename)) +
                vtksys::SystemTools::GetFilenameLastExtension(filename);
    }
  return vtksys::SystemTools::LowerCase(extension);
}

//------------------------------------------------------------------------------
std::string vtkMRMLStorageNode::GetFileNameWithoutExtension(const char* filePath /* =nullptr */)
{
  std::string filePathStd = (filePath ? filePath : "");
  if (filePathStd.empty())
    {
    // If filePath is not specified then use current filename
    filePathStd = (this->GetFileName() ? this->GetFileName() : "");
    }
  if (filePathStd.empty())
    {
    return "";
    }
  std::string fileName = vtksys::SystemTools::GetFilenameName(filePathStd);
  std::string extension = this->GetSupportedFileExtension(fileName.c_str());

  if (fileName.length() < extension.length() ||
    fileName.compare(fileName.length() - extension.length(), extension.length(), extension) != 0)
    {
    // extension not matched to the end of filename
    return fileName;
    }
  return fileName.substr(0, fileName.length() - extension.length());
}

//------------------------------------------------------------------------------
void vtkMRMLStorageNode::UpdateCompressionPresets()
{
}

//------------------------------------------------------------------------------
int vtkMRMLStorageNode::GetNumberOfCompressionPresets()
{
  this->UpdateCompressionPresets();
  return this->CompressionPresets.size();
}

//------------------------------------------------------------------------------
std::vector<std::string> vtkMRMLStorageNode::GetCompressionPresetDisplayNames()
{
  this->UpdateCompressionPresets();
  std::vector<std::string> names;
  std::vector<CompressionPreset>::iterator compressionPresetIt;
  for (compressionPresetIt = this->CompressionPresets.begin(); compressionPresetIt != this->CompressionPresets.end(); ++compressionPresetIt)
    {
    names.push_back(compressionPresetIt->DisplayName);
    }
  return names;
}

//------------------------------------------------------------------------------
std::string vtkMRMLStorageNode::GetCompressionParameterFromDisplayName(const std::string& name)
{
  this->UpdateCompressionPresets();
  std::vector<CompressionPreset>::iterator compressionPresetIt;
  for (compressionPresetIt = this->CompressionPresets.begin(); compressionPresetIt != this->CompressionPresets.end(); ++compressionPresetIt)
    {
    if (compressionPresetIt->DisplayName == name)
      {
      break;
      }
    }
  if (compressionPresetIt == this->CompressionPresets.end())
    {
    return "";
    }
  return compressionPresetIt->CompressionParameter;
}

//------------------------------------------------------------------------------
std::string vtkMRMLStorageNode::GetDisplayNameFromCompressionParameter(const std::string& parameter)
{
  this->UpdateCompressionPresets();
  std::vector<CompressionPreset>::iterator compressionPresetIt;
  for (compressionPresetIt = this->CompressionPresets.begin(); compressionPresetIt != this->CompressionPresets.end(); ++compressionPresetIt)
    {
    if (compressionPresetIt->CompressionParameter == parameter)
      {
      break;
      }
    }
  if (compressionPresetIt == this->CompressionPresets.end())
    {
    return "";
    }
  return compressionPresetIt->DisplayName;
}

//------------------------------------------------------------------------------
const std::vector<vtkMRMLStorageNode::CompressionPreset> vtkMRMLStorageNode::GetCompressionPresets()
{
  this->UpdateCompressionPresets();
  return this->CompressionPresets;
}

//----------------------------------------------------------------------------
vtkMRMLStorableNode* vtkMRMLStorageNode::GetStorableNode()
{
  if (this->Scene == nullptr)
    {
    return nullptr;
    }
  // It is an expensive operation to determine the storable node
  // (need to iterate through the scene), so the last found value
  // is cached. If it is still valid then we use it.
  if (this->LastFoundStorableNode != nullptr)
    {
    if (this->LastFoundStorableNode->GetScene() == this->Scene
      && this->LastFoundStorableNode->HasStorageNodeID(this->GetID()))
      {
      return this->LastFoundStorableNode;
      }
    }
  vtkMRMLNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* sceneNodes = this->Scene->GetNodes();
  for (sceneNodes->InitTraversal(it);
    (node = vtkMRMLNode::SafeDownCast(sceneNodes->GetNextItemAsObject(it)));)
    {
    vtkMRMLStorableNode* storableNode =
      vtkMRMLStorableNode::SafeDownCast(node);
    if (storableNode && storableNode->HasStorageNodeID(this->GetID()))
      {
      this->LastFoundStorableNode = storableNode;
      return storableNode;
      }
    }
  this->LastFoundStorableNode = nullptr;
  return nullptr;
}

//-----------------------------------------------------------
const char* vtkMRMLStorageNode::GetCoordinateSystemTypeAsString(int id)
{
  switch (id)
    {
    case CoordinateSystemRAS: return "RAS";
    case CoordinateSystemLPS: return "LPS";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLStorageNode::GetCoordinateSystemTypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < CoordinateSystemType_Last; ii++)
    {
    if (strcmp(name, GetCoordinateSystemTypeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}
