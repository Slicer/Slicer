/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceCompositeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLSliceCompositeNode - MRML node for storing a slice through RAS space
// .SECTION Description
// This node stores the information about how to composite two
// vtkMRMLVolumes into a single display image
//

#ifndef __vtkMRMLSliceCompositeNode_h
#define __vtkMRMLSliceCompositeNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkMatrix4x4.h"

class VTK_MRML_EXPORT vtkMRMLSliceCompositeNode : public vtkMRMLNode
{
  public:
  static vtkMRMLSliceCompositeNode *New();
  vtkTypeMacro(vtkMRMLSliceCompositeNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "SliceComposite";};

  // Description:
  // the ID of a MRMLVolumeNode
  vtkGetStringMacro (BackgroundVolumeID);
  vtkSetStringMacro (BackgroundVolumeID);

  // Description:
  // the ID of a MRMLVolumeNode
  // TODO: make this an arbitrary list of layers
  vtkGetStringMacro (ForegroundVolumeID);
  vtkSetStringMacro (ForegroundVolumeID);

  // Description:
  // the ID of a MRMLVolumeNode
  // TODO: make this an arbitrary list of layers
  vtkGetStringMacro (LabelVolumeID);
  vtkSetStringMacro (LabelVolumeID);

  // Description:
  // opacity of the Foreground for rendering over background
  // TODO: make this an arbitrary list of layers
  // TODO: make different composite types (checkerboard, etc)
  vtkGetMacro (ForegroundOpacity, double);
  vtkSetMacro (ForegroundOpacity, double);

  // Description:
  // opacity of the Label for rendering over background
  // TODO: make this an arbitrary list of layers
  // TODO: make different composite types (checkerboard, etc)
  vtkGetMacro (LabelOpacity, double);
  vtkSetMacro (LabelOpacity, double);


protected:
  vtkMRMLSliceCompositeNode();
  ~vtkMRMLSliceCompositeNode();
  vtkMRMLSliceCompositeNode(const vtkMRMLSliceCompositeNode&);
  void operator=(const vtkMRMLSliceCompositeNode&);

  char *BackgroundVolumeID;
  char *ForegroundVolumeID;
  char *LabelVolumeID;
  double ForegroundOpacity;
  double LabelOpacity;

};

#endif

