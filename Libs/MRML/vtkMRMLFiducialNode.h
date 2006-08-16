/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiducialNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLFiducialNode - MRML node to represent a 3D surface model.
// .SECTION Description
// Model nodes describe polygonal data.  They indicate where the model is 
// stored on disk, and how to render it (color, opacity, etc).  Models 
// are assumed to have been constructed with the orientation and voxel 
// dimensions of the original segmented volume.

#ifndef __vtkMRMLFiducialNode_h
#define __vtkMRMLFiducialNode_h


#include "vtkMatrix4x4.h"

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

class VTK_MRML_EXPORT vtkMRMLFiducialNode : public vtkMRMLNode
{
public:
  static vtkMRMLFiducialNode *New();
  vtkTypeMacro(vtkMRMLFiducialNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

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
  virtual const char* GetNodeTagName() {return "Fiducial";};

  // Description:
  // Set dependencies between this node and the parent node
  // when parsing XML file
  virtual void ProcessParentNode(vtkMRMLNode *parentNode);

  // Description:
  // Get/Set for Point
  vtkSetVector3Macro(XYZ,float);
  vtkGetVectorMacro(XYZ,float,3);
  
  // Get/Set for orientation 
  vtkSetVector4Macro(OrientationWXYZ,float);
  vtkGetVectorMacro(OrientationWXYZ,float,4);

  void SetOrientationWXYZFromMatrix4x4(vtkMatrix4x4 *mat);

  // Get/Set for Name
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);

  // Get/Set for Selected
  vtkGetMacro(Selected, bool);
  vtkSetMacro(Selected, bool);

protected:
  vtkMRMLFiducialNode();
  ~vtkMRMLFiducialNode();
  vtkMRMLFiducialNode(const vtkMRMLFiducialNode&);
  void operator=(const vtkMRMLFiducialNode&);

  // Data
  float XYZ[3];
  float OrientationWXYZ[4];
  char *Name;
  bool Selected;  
};

#endif
