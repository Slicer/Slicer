/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLStorageNode.cxx,v $
Date:      $Date: 2007/01/17 20:09:05 $
Version:   $Revision: 1.1.1.1 $

=========================================================================auto=*/

#include <sstream>

#include "vtkCacheManager.h"
#include "vtkDataIOManager.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLScene.h"

#include "vtkCommand.h"
#include "vtkStringArray.h"
#include "vtkURIHandler.h"

#include <vtksys/SystemTools.hxx>

#include <itksys/SystemTools.hxx>

vtkCxxSetObjectMacro(vtkMRMLStorageNode, URIHandler, vtkURIHandler)

//----------------------------------------------------------------------------
vtkMRMLStorageNode::vtkMRMLStorageNode()
{
  this->FileName = NULL;
  this->TempFileName = NULL;
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
  if (this->TempFileName) 
    {
    delete [] this->TempFileName;
    this->TempFileName = NULL;
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
    this->SupportedWriteFileTypes->Resize(0); 
    this->SupportedWriteFileTypes->Delete();
    this->SupportedWriteFileTypes = NULL;
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
    // convert to relative filename
    std::string name = this->FileName;
    if (this->GetScene() && !this->IsFilePathRelative(this->FileName))
      {
      name = itksys::SystemTools::RelativePath(this->GetScene()->GetRootDirectory(), this->FileName);
      }
    
    of << indent << " fileName=\"" << vtkMRMLNode::URLEncodeString(name.c_str()) << "\"";
    
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
        }
      }
    }
  for (int i = 0; i < this->GetNumberOfFileNames(); i++)
    {
    // convert to relative filename
    std::string name = this->GetNthFileName(i);
    if (this->GetScene() && !this->IsFilePathRelative(this->GetNthFileName(i)))
      {
      name = itksys::SystemTools::RelativePath(this->GetScene()->GetRootDirectory(), this->GetNthFileName(i));
      }


    of << indent << " fileListMember" << i << "=\"" << vtkMRMLNode::URLEncodeString(name.c_str()) << "\"";
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
        }
      }
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
  int disabledModify = this->StartModify();

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

      // convert to absolute filename
      std::string name;
      if (this->GetScene() && this->IsFilePathRelative(filename.c_str()))
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
      if (this->GetScene() && this->IsFilePathRelative(filename.c_str()))
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
          this->GetFileName() != NULL)
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
    os << indent << "FileListMember: " << (this->GetNthFileName(i) == NULL ? "NULL" : this->GetNthFileName(i)) << "\n";
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

  os << indent << "TempFileName: " << (this->TempFileName ? this->TempFileName : "(none)") << "\n";
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
  // if the URI is null, or emtpy assume the file name is set and return
  if ( this->Scene )
    {
    // this event is being detected by GUI to provide feedback during load
    // of data. But,
    // commented out for now because CLI modules are using MRML to write
    // data in another thread, causing GUI to crash.
//    this->Scene->InvokeEvent (vtkMRMLScene::ImportProgressFeedbackEvent );
    }

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
      // set up the data handler if it's not set already (may want to over
      // ride, esp. for the XND handler)
      if (this->URIHandler == NULL)
        {
        this->URIHandler = this->Scene->FindURIHandler(this->URI);
        }
      if (this->URIHandler != NULL)
        {
        vtkDebugMacro("StageReadData: got a URI Handler");
        }
      else
        {
        vtkErrorMacro("StageReadData: unable to get a URI handler for " << this->URI << ", resetting stage to idle");
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
  if ( this->Scene )
    {
    // this event is being detected by GUI to provide feedback during load
    // of data. But,
    // commented out for now because CLI modules are using MRML to write
    // data in another thread, causing GUI to crash.
//    this->Scene->InvokeEvent (vtkMRMLScene::SaveProgressFeedbackEvent );
    }

  if (this->URI == NULL)
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
       // set up the data handler if it's not set already
       if (this->URIHandler == NULL)
         {
         this->URIHandler = this->Scene->FindURIHandler(this->URI);
         }
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
    if (this->GetFileName() == NULL)
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

  if (fileName == NULL)
    {
    vtkDebugMacro("GetFullNameFromNthFileName: n = " << n << " have a null filename, returning empty string");
    return fullName;
    }
  
  vtkDebugMacro("GetFullNameFromNthFileName: n = " << n << ", using file name '" << fileName << "'");
  
  if (this->Scene != NULL &&
      this->Scene->GetRootDirectory() != NULL &&
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
    if (this->Scene == NULL)
      {
      vtkDebugMacro("GetFullNameFromNthFileName: scene is null, returning " << fileName);
      }
    else
      {
      vtkDebugMacro("GetFullNameFromNthFileName: scene root dir = " << (this->Scene->GetRootDirectory() != NULL ? this->Scene->GetRootDirectory() : "null") << ", relative = " << (this->IsFilePathRelative(fileName) ? "yes" : "no"));
      }
    fullName = std::string(fileName);
    }
  vtkDebugMacro("GetFullNameFromNthFileName: " << n << ", returning full name " << fullName);
  return fullName;
}

//----------------------------------------------------------------------------
int vtkMRMLStorageNode::SupportedFileType(const char *fileName)
{
  vtkErrorMacro("SupportedFileType: sub class didn't define this method! (fileName = '" << (fileName == NULL ? "NULL" : fileName) << "')");
  return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLStorageNode::FileNameIsInList(const char *fileName)
{
  if (fileName == NULL)
    {
    return 0;
    }
  std::string fname = std::string(fileName);
  int fileNameIsRelative =  this->IsFilePathRelative(fileName);
  for (unsigned int i = 0; i < this->FileNameList.size(); i++)
    {
    std::string thisFile = this->FileNameList[i];
    int thisFileIsRelative = this->IsFilePathRelative(thisFile.c_str());
    // make sure we're comparing apples to apples
    if (fileNameIsRelative != thisFileIsRelative)
      {
      std::string rel1, rel2;
      const char *rootDir;
      if ( this->Scene )
        {
        rootDir = this->Scene->GetRootDirectory();
        }
      else
        {
        rootDir = ".";
        }
      vtkDebugMacro("WARNING: trying to determine if file " << fileName << " is already in the list and comparing against " << thisFile.c_str() << ", they have mismatched absolute/relative paths. Using scene root dir to disambiguate: " << rootDir);
      if (fileNameIsRelative)
        {
        rel1 = std::string(fileName);
        }
      else
        {
        rel1 = vtksys::SystemTools::RelativePath(rootDir, fileName);
        }
      if (thisFileIsRelative)
        {
        rel2 = thisFile;
        }
      else
        {
        rel2 = vtksys::SystemTools::RelativePath(rootDir, thisFile.c_str());
        }
        
      vtkDebugMacro("\tComparing " << rel1 << " and " << rel2);
      if (rel1.compare(rel2) == 0)
        {
        return 1;
        }
      }
    else
      {
      if (fname.compare(this->FileNameList[i]) == 0)
        {
        return 1;
        }
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
  if ((this->GetNumberOfFileNames()-1) < n)
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
  if (n >= 0 && this->GetNumberOfFileNames() >= n)
    {
    this->FileNameList[n] = std::string(fileName);
    }
  else
    {
    vtkErrorMacro("RestNthFileName: file name number " << n << " not already set, returning.");
    }
  
}

//----------------------------------------------------------------------------
unsigned int vtkMRMLStorageNode::AddURI( const char* uri )
{
  if (uri == NULL)
    {
    vtkErrorMacro("AddURI: can't add a null URI");
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
    return NULL;
    }
  else
    {
    return this->URIList[n].c_str();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorageNode::ResetNthURI(int n, const char* uri)
{
  if (uri == NULL)
    {
    vtkErrorMacro("ResetNthURI: given URI is null (n = " << n << ")");
    return;
    }
  if (n >= 0 && this->GetNumberOfURIs() >= n)
    {
    this->URIList[n] = std::string(uri);
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
  if (this->GetFileName() == NULL)
    {
    vtkWarningMacro("SetDataDirectory: file name is null, no reason to reset data directory.");
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

//------------------------------------------------------------------------------
int vtkMRMLStorageNode::IsFilePathRelative(const char * filepath)
{
  if (filepath == NULL)
    {
    vtkErrorMacro("IsFilePathRelative: input file path is null! Returning 0");
    return 0;
    }
  if ( this->Scene )
    {
    return this->Scene->IsFilePathRelative(filepath);
    }
  else
    {
    vtksys_stl::vector<vtksys_stl::string> components;
    vtksys::SystemTools::SplitPath((const char*)filepath, components);
    if (components[0] == "") 
      {
      return 1;
      }
    else
      {
      return 0;
      }
    }
}

//------------------------------------------------------------------------------
const char *vtkMRMLStorageNode::GetAbsoluteFilePath(const char *inputPath)
{
  if (inputPath == NULL)
    {
    vtkErrorMacro("GetAbsoluteFilePath: input path is null.");
    return NULL;
    }
  if (!this->IsFilePathRelative(inputPath))
    {
    // the path is already absolute, return it
    return inputPath;
    }
  if (!this->GetScene())
    {
    vtkErrorMacro("GetAbsoluteFilePath: have a relative path " << inputPath << " but no scene to find it from!");
    return NULL;
    }

  std::string path = this->GetScene()->GetRootDirectory();
  if (path[path.size()-1] != '/')
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
