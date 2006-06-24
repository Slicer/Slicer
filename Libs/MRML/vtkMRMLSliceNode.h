/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLSliceNode - MRML node for storing a slice through RAS space
// .SECTION Description
// This node stores the information about how to map from RAS space to 
// the desired slice plane.
// -- SliceToRAS is the matrix that rotates and translates the slice plane
// -- FieldOfView tells the size of  slice plane
//

#ifndef __vtkMRMLSliceNode_h
#define __vtkMRMLSliceNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkMatrix4x4.h"

class VTK_MRML_EXPORT vtkMRMLSliceNode : public vtkMRMLNode
{
  public:
  static vtkMRMLSliceNode *New();
  vtkTypeMacro(vtkMRMLSliceNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "Slice";};

  // Description:
  // Mapping from RAS space onto the slice plane
  // TODO: maybe this should be a quaternion and a translate to avoid shears/scales
  vtkGetObjectMacro (SliceToRAS, vtkMatrix4x4);
  vtkSetObjectMacro (SliceToRAS, vtkMatrix4x4);

 // Description:
  // The visibility of the slice in the 3DViewer.
  vtkGetMacro ( SliceVisible, int );
  vtkSetMacro ( SliceVisible, int );

  // Description:
  // 'standard' radiological convention views of patient space
  // these calls adjust the SliceToRAS matrix to position the slice
  // cutting plane 
  void SetOrientationToAxial();
  void SetOrientationToSagittal();
  void SetOrientationToCoronal();

  // Description
  // A description of the current orientation
  vtkGetStringMacro (OrientationString);
  vtkSetStringMacro (OrientationString);

  // Description:
  // Size of the slice plane in millimeters
  vtkGetVector3Macro (FieldOfView, double);
  void SetFieldOfView (double x, double y, double z) 
  {
    if ( x != this->FieldOfView[0] || y != this->FieldOfView[1] || z != this->FieldOfView[2] )
      {
      this->FieldOfView[0] = x;
      this->FieldOfView[1] = y;
      this->FieldOfView[2] = z;
      this->UpdateMatrices();
      }
  }


  // Description:
  // Number of samples in each direction
  // -- note that the spacing is implicitly FieldOfView / Dimensions
  vtkGetVector3Macro (Dimensions, unsigned int);
  void SetDimensions (unsigned int x, unsigned int y, unsigned int z) 
  {
    if ( x != this->Dimensions[0] || y != this->Dimensions[1] || z != this->Dimensions[2] )
      {
      this->Dimensions[0] = x;
      this->Dimensions[1] = y;
      this->Dimensions[2] = z;
      this->UpdateMatrices();
      }
  }

  // Description:
  // Matrix mapping from XY pixel coordinates on an image window 
  // into slice coordinates in mm
  vtkGetObjectMacro (XYToSlice, vtkMatrix4x4);

  // Description:
  // Matrix mapping from XY pixel coordinates on an image window 
  // into RAS world coordinates
  vtkGetObjectMacro (XYToRAS, vtkMatrix4x4);

  // Description:
  // helper for comparing to matrices
  // TODO: is there a standard VTK method?
  int Matrix4x4AreEqual(vtkMatrix4x4 *m1, vtkMatrix4x4 *m2);

  // Description:
  // Recalculate XYToSlice and XYToRAS in terms or fov, dim, SliceToRAS
  // - called when any of the inputs change
  void UpdateMatrices();
  
protected:


  vtkMRMLSliceNode();
  ~vtkMRMLSliceNode();
  vtkMRMLSliceNode(const vtkMRMLSliceNode&);
  void operator=(const vtkMRMLSliceNode&);


  vtkMatrix4x4 *SliceToRAS;
  vtkMatrix4x4 *XYToSlice;
  vtkMatrix4x4 *XYToRAS;

  int SliceVisible;
  double FieldOfView[3];
  unsigned int Dimensions[3];
  char *OrientationString;

};

#endif

