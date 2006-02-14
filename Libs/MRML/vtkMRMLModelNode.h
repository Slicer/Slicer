/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelNode.h,v $
  Date:      $Date: 2006/02/11 17:20:11 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/
// .NAME vtkMRMLModelNode - MRML node to represent a 3D surface model.
// .SECTION Description
// Model nodes describe polygonal data.  They indicate where the model is 
// stored on disk, and how to render it (color, opacity, etc).  Models 
// are assumed to have been constructed with the orientation and voxel 
// dimensions of the original segmented volume.

#ifndef __vtkMRMLModelNode_h
#define __vtkMRMLModelNode_h

#include <string>

#include "vtkPolyData.h" 
#include "vtkMRMLNode.h"


class VTK_EXPORT vtkMRMLModelNode : public vtkMRMLNode
{
public:
  static vtkMRMLModelNode *New();
  vtkTypeMacro(vtkMRMLModelNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Read data for the node
  virtual void ReadData();

  // Write data for the node
  virtual void WriteData();

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual char* GetNodeTagName() {return "Model";};

  // Description:
  // Path of the data file, relative to the MRML file
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Name of the model's color, which is defined by a Color node in a MRML file
  vtkSetStringMacro(Color);
  vtkGetStringMacro(Color);

  // Description:
  // Opacity of the surface expressed as a number from 0 to 1
  vtkSetMacro(Opacity, float);
  vtkGetMacro(Opacity, float);

  // Description:
  // Indicates if the surface is visible
  vtkBooleanMacro(Visibility, int);
  vtkGetMacro(Visibility, int);
  vtkSetMacro(Visibility, int);

  // Description:
  // Specifies whether to clip the surface with the slice planes
  vtkBooleanMacro(Clipping, int);
  vtkGetMacro(Clipping, int);
  vtkSetMacro(Clipping, int);

  // Description:
  // Indicates whether to cull (not render) the backface of the surface
  vtkBooleanMacro(BackfaceCulling, int);
  vtkGetMacro(BackfaceCulling, int);
  vtkSetMacro(BackfaceCulling, int);

  // Description:
  // Indicates whether to render the scalar value associated with each polygon vertex
  vtkBooleanMacro(ScalarVisibility, int);
  vtkGetMacro(ScalarVisibility, int);
  vtkSetMacro(ScalarVisibility, int);

  // Description:
  // Indicates whether to render the vector value associated with each polygon vertex
  vtkBooleanMacro(VectorVisibility, int);
  vtkGetMacro(VectorVisibility, int);
  vtkSetMacro(VectorVisibility, int);

  // Description:
  // Indicates whether to render the tensor value associated with each polygon vertex
  vtkBooleanMacro(TensorVisibility, int);
  vtkGetMacro(TensorVisibility, int);
  vtkSetMacro(TensorVisibility, int);

  // Description:
  // Range of scalar values to render rather than the single color designated by colorName
  vtkSetVector2Macro(ScalarRange, vtkFloatingPointType);
  vtkGetVector2Macro(ScalarRange, vtkFloatingPointType);


  // Description:
  // Numerical ID of the color lookup table to use for rendering the overlay
  // for this model
  vtkGetMacro(LUTName,int);
  vtkSetMacro(LUTName,int);

    
  vtkGetObjectMacro(PolyData, vtkPolyData);
  vtkSetObjectMacro(PolyData, vtkPolyData);

protected:
  vtkMRMLModelNode();
  ~vtkMRMLModelNode();
  vtkMRMLModelNode(const vtkMRMLModelNode&) {};
  void operator=(const vtkMRMLModelNode&) {};

  // Data
  vtkPolyData *PolyData;

  // Strings
  char *FileName;
  char *Color;
  int LUTName;
    
  // Numbers
  float Opacity;

  // Booleans
  int Visibility;
  int Clipping;
  int BackfaceCulling;
  int ScalarVisibility;
  int VectorVisibility;
  int TensorVisibility;

  // Arrays
  vtkFloatingPointType ScalarRange[2];

};

#endif
