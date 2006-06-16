/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiducialListNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLFiducialListNode - MRML node to represent a 3D surface model.
// .SECTION Description
// Model nodes describe polygonal data.  They indicate where the model is 
// stored on disk, and how to render it (color, opacity, etc).  Models 
// are assumed to have been constructed with the orientation and voxel 
// dimensions of the original segmented volume.

#ifndef __vtkMRMLFiducialListNode_h
#define __vtkMRMLFiducialListNode_h

#include <string>

#include "vtkMatrix4x4.h"
#include "vtkCollection.h"

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLFiducialNode.h"
#include "vtkMRMLTransformableNode.h"

class VTK_MRML_EXPORT vtkMRMLFiducialListNode : public vtkMRMLTransformableNode
{
public:
  static vtkMRMLFiducialListNode *New();
  vtkTypeMacro(vtkMRMLFiducialListNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "FiducialList";};

  // Description:
  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Get/Set for Symbol size
  vtkSetMacro(SymbolSize,float);
  vtkGetMacro(SymbolSize,float);


  // Description:
  // Get/Set for Symbol size
  vtkSetMacro(Visibility,int);
  vtkGetMacro(Visibility,int);
  
  // Description:
  // Get/Set for Text size
  vtkSetMacro(TextSize,float);
  vtkGetMacro(TextSize,float);
  
  // Description:
  // Get/Set for Glyph and Text color
  vtkSetVector3Macro(Color,float);
  vtkGetVectorMacro(Color,float,3);
  
  vtkSetStringMacro(Type);
  vtkGetStringMacro(Type);
  
  int GetNumberOfFiducialNodes() { return this->FiducialList->vtkCollection::GetNumberOfItems(); };
  vtkMRMLFiducialNode* GetNthFiducialNode(int n);
  void AddFiducialNode(vtkMRMLFiducialNode *o) { this->FiducialList->vtkCollection::AddItem(o); this->Modified();};
  void RemoveFiducialNode(vtkMRMLFiducialNode *o) { this->FiducialList->vtkCollection::RemoveItem(o); this->Modified();};
  void RemoveFiducialNode(int i) { this->FiducialList->vtkCollection::RemoveItem(i); this->Modified();};
  int  IsFiducialNodePresent(vtkMRMLFiducialNode *o) { return this->FiducialList->vtkCollection::IsItemPresent(o);};

protected:
  vtkMRMLFiducialListNode();
  ~vtkMRMLFiducialListNode();
  vtkMRMLFiducialListNode(const vtkMRMLFiducialListNode&);
  void operator=(const vtkMRMLFiducialListNode&);

  float SymbolSize;
  float TextSize;
  int Visibility;
  float Color[3];
  char  *Type;

  vtkCollection *FiducialList;

};

#endif
