/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFreeSurferModelStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLFreeSurferModelStorageNode - MRML node for model storage on disk
// .SECTION Description
// Storage nodes has methods to read/write vtkPolyData to/from disk

#ifndef __vtkMRMLFreeSurferModelStorageNode_h
#define __vtkMRMLFreeSurferModelStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLModelNode.h" 


class vtkImageData;
class vtkCollection;
class VTK_MRML_EXPORT vtkMRMLFreeSurferModelStorageNode : public vtkMRMLModelStorageNode
{
  public:
  static vtkMRMLFreeSurferModelStorageNode *New();
  vtkTypeMacro(vtkMRMLFreeSurferModelStorageNode,vtkMRMLStorageNode);
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
  virtual const char* GetNodeTagName()  {return "FreeSurferModelStorage";};


  // Description:
  // Get the number of overlay files
  int GetNumberOfOverlayFiles();
  
  // Description:
  // Add an overlay file name to the collection of those associated with this
  // storage node, checks for duplicates first
  void AddOverlayFileName (const char *fileName);

  // Description:
  // Remove an overlay file name (used if remove an array)
  void RemoveOverlayFileName (const char *fileName);

  // Description:
  // Go through the current list of overlay files and load them.
  // Returns number of files sucessfully reloaded
  int ReloadOverlayFiles( vtkMRMLNode *refNode );
  
  // Description:
  // Get the ith overlay file name, 0 indexed, return NULL if out of range
  const char *GetOverlayFileName(int i);

  // Description:
  // Control use of the triangle stipper when reading the polydata
  vtkGetMacro(UseStripper, int);
  vtkSetMacro(UseStripper, int);
  
  // Description:
  // Set/Get the surface file name (the super class variable will be over
  // written if this storage node is used to load overlays)
  vtkSetStringMacro(SurfaceFileName);
  vtkGetStringMacro(SurfaceFileName);

//BTX
  // Description:
  // Add a known overlay file extension
  void AddOverlayFileExtension(std::string ext);
  // Description:
  // returns true if on the list, false otherwise
  bool IsKnownOverlayFileExtension(std::string ext);
//ETX
  
protected:
  vtkMRMLFreeSurferModelStorageNode();
  ~vtkMRMLFreeSurferModelStorageNode();
  vtkMRMLFreeSurferModelStorageNode(const vtkMRMLFreeSurferModelStorageNode&);
  void operator=(const vtkMRMLFreeSurferModelStorageNode&);

  // Description:
  // the overlay files that were applied to this model
  //BTX
  std::vector< std::string > OverlayFiles;
  // Description:
  // a list of valid overlay file extensions
  std::vector< std::string > KnownOverlayFileExtensions; 
  //ETX

  char *SurfaceFileName;

  int UseStripper;
};

#endif

