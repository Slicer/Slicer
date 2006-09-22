/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLClipModelsNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLClipModelsNode - MRML node to represent a 3D surface model.
// .SECTION Description
// Model nodes describe polygonal data.  They indicate where the model is 
// stored on disk, and how to render it (color, opacity, etc).  Models 
// are assumed to have been constructed with the orientation and voxel 
// dimensions of the original segmented volume.

#ifndef __vtkMRMLClipModelsNode_h
#define __vtkMRMLClipModelsNode_h

#include <string>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

class VTK_MRML_EXPORT vtkMRMLClipModelsNode : public vtkMRMLNode
{
public:
  static vtkMRMLClipModelsNode *New();
  vtkTypeMacro(vtkMRMLClipModelsNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "ClipModels";};

  // Description:
  // Indicates the type of clipping
  // "Intersection" or "Union"
  vtkGetMacro(ClipType, int);
  vtkSetMacro(ClipType, int);

  //BTX
  enum
    {
      ClipIntersection = 0,
      ClipUnion = 1,
    };
  //ETX

  // Description:
  // Indicates if the Red slice clipping is Off, 
  // Positive space, or Negative space
  vtkGetMacro(RedSliceClipState, int);
  vtkSetMacro(RedSliceClipState, int);

  // Description:
  // Indicates if the Yellow slice clipping is Off, 
  // Positive space, or Negative space
  vtkGetMacro(YellowSliceClipState, int);
  vtkSetMacro(YellowSliceClipState, int);

  // Description:
  // Indicates if the Green slice clipping is Off, 
  // Positive space, or Negative space
  vtkGetMacro(GreenSliceClipState, int);
  vtkSetMacro(GreenSliceClipState, int);

  //BTX
  enum
    {
      ClipOff = 0,
      ClipPositiveSpace = 1,
      ClipNegativeSpace = 2,
    };
  //ETX
  
protected:
  vtkMRMLClipModelsNode();
  ~vtkMRMLClipModelsNode();
  vtkMRMLClipModelsNode(const vtkMRMLClipModelsNode&);
  void operator=(const vtkMRMLClipModelsNode&);

  int ClipType;

  int RedSliceClipState;
  int YellowSliceClipState;
  int GreenSliceClipState;


};

#endif
