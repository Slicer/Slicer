/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLStorageNode - a supercalss for other storage nodes
/// 
/// a supercalss for other storage nodes like volume and model

#ifndef __vtkMRMLStorageNode_h
#define __vtkMRMLStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkURIHandler.h"

class vtkStringArray;
class vtkURIHandler;

class VTK_MRML_EXPORT vtkMRMLStorageNode : public vtkMRMLNode
{
  public:
  static vtkMRMLStorageNode *New(){return NULL;};
  vtkTypeMacro(vtkMRMLStorageNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);
  
  /// 
  /// Read data and set it in the referenced node. 
  /// Return 1 on success, 0 on failure.
  /// NOTE: Subclasses should implement this method
  virtual int ReadData(vtkMRMLNode *refNode) = 0;

  /// 
  /// Write data from a  referenced node
  /// Return 1 on success, 0 on failure.
  /// NOTE: Subclasses should implement this method
  virtual int WriteData(vtkMRMLNode *refNode) = 0;

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName() = 0;

  /// 
  /// A file name or the archetype file name for a series
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  /// 
  /// return the nth file name, null if doesn't exist
  const char *GetNthFileName(int n) const;

  /// 
  /// Use compression on write
  vtkBooleanMacro(UseCompression, int);
  vtkGetMacro(UseCompression, int);
  vtkSetMacro(UseCompression, int);

  /// 
  /// Location of the remote copy of this file.
  vtkSetStringMacro(URI);
  vtkGetStringMacro(URI);
  
  vtkGetObjectMacro (URIHandler, vtkURIHandler);
  vtkSetObjectMacro (URIHandler, vtkURIHandler);
  
  /// 
  /// Propagate Progress Event generated in ReadData
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  /// 
  /// If the URI is not null, fetch it and save it to the node's FileName location or
  /// load directly into the reference node.
  void StageReadData ( vtkMRMLNode *refNode );
  /// 
  /// Copy data from the local file location (node->FileName) or node to the remote
  /// location specified by the URI
  void StageWriteData ( vtkMRMLNode *refNode );

  /// 
  /// Possible Read and Write states
  /// Idle: not currently working on any data, ready for the next transfer
  /// Pending: the data is remote, waiting for a transfer to be scheduled
  /// Scheduled: the data is remote, and is scheduled for download
  /// Transferring: data is remote, and the transfer is working to completion
  /// TransferDone: the data is on disk and ready to be read
  /// Cancelled: the user cancelled the remote data transfer
  enum
  {
    Idle,
    Pending,
    Scheduled,
    Transferring,
    TransferDone,
    Cancelled
  };

  /// Get/Set the state of reading 
  vtkGetMacro(ReadState,int);
  vtkSetMacro(ReadState,int);
  void SetReadStatePending() { this->SetReadState(this->Pending); };
  void SetReadStateIdle() { this->SetReadState(this->Idle); }; 
  void SetReadStateScheduled() { this->SetReadState(this->Scheduled); };
  void SetReadStateTransferring() { this->SetReadState(this->Transferring); }; 
  void SetReadStateTransferDone() { this->SetReadState(this->TransferDone); }; 
  void SetReadStateCancelled() { this->SetReadState(this->Cancelled); }; 
  const char *GetStateAsString(int state);
  const char *GetReadStateAsString() { return this->GetStateAsString(this->ReadState); };
  
  /// 
  /// Get/Set the state of writing 
  vtkGetMacro(WriteState,int);
  vtkSetMacro(WriteState,int);
  void SetWriteStatePending() { this->SetWriteState(this->Pending); };
  void SetWriteStateIdle() { this->SetWriteState(this->Idle); };
  void SetWriteStateScheduled() { this->SetWriteState(this->Scheduled); };
  void SetWriteStateTransferring() { this->SetWriteState(this->Transferring); };
  void SetWriteStateTransferDone() { this->SetWriteState(this->TransferDone); };
  void SetWriteStateCancelled() { this->SetWriteState(this->Cancelled); }; 
  const char *GetWriteStateAsString() { return this->GetStateAsString(this->WriteState); };

  /// 
  /// Get the file's absolute path from the file name and the mrml scene root
  /// dir. GetFullnameFromFileName calls GetFullNameFromNthFileName with -1.
  //BTX
  std::string GetFullNameFromFileName();
  std::string GetFullNameFromNthFileName(int n);
  //ETX

  /// 
  /// Check to see if this storage node can handle the file type in the input
  /// string. If input string is null, check URI, then check FileName. Returns
  /// 1 if is supported, 0 otherwise.
  /// Subclasses should implement this method.
  virtual int SupportedFileType(const char *fileName);

  /// 
  /// Get all the supported write file types
  /// Subclasses should overwrite InitializeSupportedWriteFileTypes().
  virtual vtkStringArray* GetSupportedWriteFileTypes();

  /// 
  /// Allow to set specific file format that this node will write output.
  vtkSetStringMacro(WriteFileFormat);
  vtkGetStringMacro(WriteFileFormat);

  /// 
  /// Add in another file name to the list of file names
  unsigned int AddFileName (const char *fileName);
  /// 
  /// Clear the array of file names
  void ResetFileNameList();
  
  /// 
  /// See how many file names were generated during ExecuteInformation
  int GetNumberOfFileNames() const
  {
    return (int)this->FileNameList.size();
  };

  /// 
  /// is filename in the filename list already?
  /// returns 1 if yes, 0 if no
  int FileNameIsInList(const char *fileName);
  
  /// 
  /// Add in another URI to the list of URI's
  unsigned int AddURI(const char *uri);

  /// 
  /// Get the nth URI from the list of URI's
  const char *GetNthURI(int n);
  
  /// 
  /// Clear the array of URIs
  void ResetURIList();
  
  /// 
  /// Return how many uri names this storage node holds in it's list
  int GetNumberOfURIs()
  {
    return (int)this->URIList.size();
  }

  /// 
  /// Set a new data directory for all files
  void SetDataDirectory(const char* dataDirName);
  /// 
  /// Set a new URI base for all URI's
  void SetURIPrefix(const char *uriPrefix);
  
  /// 
  /// Return a default file extension for writting
  virtual const char* GetDefaultWriteFileExtension()
    {
    return NULL;
    };

  /// 
  /// Set the nth file in FileNameList, checks that it is already defined
  void ResetNthFileName(int n, const char *fileName);
  /// 
  /// Set the nth uri in URIList, checks that it is already defined
  void ResetNthURI(int n, const char *uri);

  /// 
  /// Checks is file path is a relative path by calling appropriate
  /// method on the scene depending on whether the scene pointer is valid.
  /// returns 0 if it's not relative or the input is null, 1 if it is relative
  int IsFilePathRelative(const char * filepath);

  ///
  /// Calcualtes and the absolute path to the input file if the input path is
  /// relative and the scene is defined with a root directory. Sets and then
  /// return TempFileName. Returns null if the input path is null or the path
  /// is relative and the scene is not defined. Returns inputPath if it's absolute.
  const char *GetAbsoluteFilePath(const char *inputPath);
  
  /// 
  /// A temporary file name used to calculate absolute paths
  vtkSetStringMacro(TempFileName);
  vtkGetStringMacro(TempFileName);
  
protected:
  vtkMRMLStorageNode();
  ~vtkMRMLStorageNode();
  vtkMRMLStorageNode(const vtkMRMLStorageNode&);
  void operator=(const vtkMRMLStorageNode&);
  
  char *FileName;
  char *TempFileName;
  char *URI;
  vtkURIHandler *URIHandler;
  int UseCompression;
  int ReadState;
  int WriteState;

  //BTX
  /// 
  /// An array of file names, should contain the FileName but may not
  std::vector<std::string> FileNameList;
  /// 
  /// An array of URI's, should contain the URI but may not
  std::vector<std::string> URIList;
  //ETX
  vtkStringArray* SupportedWriteFileTypes;
  char* WriteFileFormat;

  /// 
  /// Initialize all the supported write file types
  /// Subclasses should use this method to initialize SupportedWriteFileTypes.
  virtual void InitializeSupportedWriteFileTypes();

};

#endif



