/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFreeSurferModelOverlayStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLFreeSurferModelOverlayStorageNode - MRML node for model storage on disk
// .SECTION Description
// Storage nodes has methods to read/write vtkPolyData to/from disk

#ifndef __vtkMRMLFreeSurferModelOverlayStorageNode_h
#define __vtkMRMLFreeSurferModelOverlayStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLModelNode.h" 


class vtkImageData;
class vtkCollection;
class VTK_MRML_EXPORT vtkMRMLFreeSurferModelOverlayStorageNode : public vtkMRMLModelStorageNode
{
  public:
  static vtkMRMLFreeSurferModelOverlayStorageNode *New();
  vtkTypeMacro(vtkMRMLFreeSurferModelOverlayStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

   // Description:
  // Set dependencies between this node and the parent node
  // when parsing XML file
  virtual void ProcessParentNode(vtkMRMLNode *parentNode);
  
  // Description:
  // Read data and set it in the referenced node
  // NOTE: Subclasses should implement this method
  virtual int ReadData(vtkMRMLNode *refNode);
  
  // Description:
  // Write data from a  referenced node
  // NOTE: Subclasses should implement this method
  virtual int WriteData(vtkMRMLNode *refNode);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "FreeSurferModelOverlayStorage";};
  
//BTX
  // Description:
  // Add a known overlay file extension
  void AddFileExtension(std::string ext);
  // Description:
  // returns true if on the list, false otherwise
  bool IsKnownFileExtension(std::string ext);
//ETX

  // Description:
  // return the number of known file extensions
  int GetNumberOfKnownFileExtensions();
  
  // Description:
  // return the indexed file extension, null if i out of range
  const char *GetKnownFileExtension(int i);

  // Description:
  // can I load files with extensions like this filename has?
  int SupportedFileType(const char *fileName);
protected:
  vtkMRMLFreeSurferModelOverlayStorageNode();
  ~vtkMRMLFreeSurferModelOverlayStorageNode();
  vtkMRMLFreeSurferModelOverlayStorageNode(const vtkMRMLFreeSurferModelOverlayStorageNode&);
  void operator=(const vtkMRMLFreeSurferModelOverlayStorageNode&);

  // Description:
  // a list of valid overlay file extensions
  //BTX
  std::vector< std::string > KnownFileExtensions; 
  //ETX
};

#endif

