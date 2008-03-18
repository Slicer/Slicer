/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLStorageNode - a supercalss for other storage nodes
// .SECTION Description
// a supercalss for other storage nodes like volume and model

#ifndef __vtkMRMLStorageNode_h
#define __vtkMRMLStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkURIHandler.h"

class vtkURIHandler;
class VTK_MRML_EXPORT vtkMRMLStorageNode : public vtkMRMLNode
{
  public:
  static vtkMRMLStorageNode *New(){return NULL;};
  vtkTypeMacro(vtkMRMLStorageNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);
  
  // Description:
  // Read data and set it in the referenced node
  // NOTE: Subclasses should implement this method
  virtual int ReadData(vtkMRMLNode *refNode) = 0;

  // Description:
  // Write data from a  referenced node
  // NOTE: Subclasses should implement this method
  virtual int WriteData(vtkMRMLNode *refNode) = 0;

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName() = 0;

  // Description:
  // A file name or one name in a series
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Use compression on write
  vtkGetMacro(UseCompression, int);
  vtkSetMacro(UseCompression, int);

  // Description:
  // Location of the remote copy of this file.
  vtkSetStringMacro(URI);
  vtkGetStringMacro(URI);
  
  vtkGetObjectMacro (URIHandler, vtkURIHandler);
  vtkSetObjectMacro (URIHandler, vtkURIHandler);
  
  // Description:
  // Propagate Progress Event generated in ReadData
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  // Description:
  // If the URI is not null, fetch it and save it to the node's FileName location or
  // load directly into the reference node.
  void StageReadData ( vtkMRMLNode *refNode );
  // Description:
  // Copy data from the local file location (node->FileName) or node to the remote
  // location specified by the URI
  void StageWriteData ( vtkMRMLNode *refNode );

  // Description:
  // Possible Read and Write states 
  //BTX
  enum
  {
    Ready,
    Pending,
    Scheduled,
  };
  //ETX
  // Description:
  // Get/Set the state of reading 
  vtkGetMacro(ReadState,int);
  vtkSetMacro(ReadState,int);
  void SetReadStatePending() { this->SetReadState(this->Pending); };
  void SetReadStateReady() { this->SetReadState(this->Ready); };
  void SetReadStateScheduled() { this->SetReadState(this->Scheduled); };
  const char *GetStateAsString(int state);
  const char *GetReadStateAsString() { return this->GetStateAsString(this->ReadState); };
  
  // Description:
  // Get/Set the state of writing 
  vtkGetMacro(WriteState,int);
  vtkSetMacro(WriteState,int);
  void SetWriteStatePending() { this->SetWriteState(this->Pending); };
  void SetWriteStateReady() { this->SetWriteState(this->Ready); };
  void SetWriteStateScheduled() { this->SetWriteState(this->Scheduled); };
  const char *GetWriteStateAsString() { return this->GetStateAsString(this->WriteState); };

  // Description:
  // Get the file's absolute path from the file name and the mrml scene root
  // dir
  //BTX
  std::string GetFullNameFromFileName();
  //ETX

  // Description:
  // Check to see if this storage node can handle the file type in the input
  // string. If input string is null, check URI, then check FileName. Returns
  // 1 if is supported, 0 otherwise.
  // Subclasses should implement this method.
  virtual int SupportedFileType(const char *fileName);
  
protected:
  vtkMRMLStorageNode();
  ~vtkMRMLStorageNode();
  vtkMRMLStorageNode(const vtkMRMLStorageNode&);
  void operator=(const vtkMRMLStorageNode&);

  char *FileName;
  char *URI;
  vtkURIHandler *URIHandler;
  int UseCompression;
  int ReadState;
  int WriteState;
  
};

#endif

