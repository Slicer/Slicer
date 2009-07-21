/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAtlasCreatorNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $
  Author:    $Sylvain Jaume (MIT)$

=======================================================================auto=*/

#ifndef __vtkMRMLAtlasCreatorNode_h
#define __vtkMRMLAtlasCreatorNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkAtlasCreator.h"
#include "vtkImageData.h"

class VTK_AtlasCreator_EXPORT vtkMRMLAtlasCreatorNode :
  public vtkMRMLNode
{
  public:
  static vtkMRMLAtlasCreatorNode *New();
  vtkTypeMacro(vtkMRMLAtlasCreatorNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create instance of a GAD node.
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes from name/value pairs
  virtual void ReadXMLAttributes(const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "GADParameters";};

  // Description:
  // Get/Set the threshold to create the mask
  vtkGetMacro(SecondLabelMapThreshold, double);
  vtkSetMacro(SecondLabelMapThreshold, double);

  // Description:
  // Get/Set the factor to downsample the image
  vtkGetMacro(OutputSize, double);
  vtkSetMacro(OutputSize, double);

  // Description:
  // Get/Set input volume MRML id
  vtkGetStringMacro(FirstLabelMapRef);
  vtkSetStringMacro(FirstLabelMapRef);

  // Description:
  // Get/Set output volume MRML id
  vtkGetStringMacro(OutputVolumeRef);
  vtkSetStringMacro(OutputVolumeRef);

  // Description:
  // Get/Set preview volume MRML id
  vtkGetStringMacro(ThirdLabelMapVolumeRef);
  vtkSetStringMacro(ThirdLabelMapVolumeRef);

  // Description:
  // Get/Set mask volume MRML id
  vtkGetStringMacro(SecondLabelMapVolumeRef);
  vtkSetStringMacro(SecondLabelMapVolumeRef);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

protected:
  vtkMRMLAtlasCreatorNode();
  ~vtkMRMLAtlasCreatorNode();
  vtkMRMLAtlasCreatorNode(const vtkMRMLAtlasCreatorNode&);
  void operator=(const vtkMRMLAtlasCreatorNode&);

  char *FirstLabelMapRef;
  char *OutputVolumeRef;
  char *ThirdLabelMapVolumeRef;
  char *SecondLabelMapVolumeRef;

  double SecondLabelMapThreshold;
  double OutputSize;
};

#endif

