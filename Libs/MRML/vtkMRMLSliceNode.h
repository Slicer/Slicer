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
  // Updates other nodes in the scene depending on this node
  // or updates this node if it depends on other nodes when the scene is read in
  // This method is called automatically by XML parser after all nodes are created
  virtual void UpdateScene(vtkMRMLScene *);

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
  // The visibility of the slice plane widget in the 3DViewer.
  vtkGetMacro ( WidgetVisible, int );
  vtkSetMacro ( WidgetVisible, int );

  // Description:
  // 'standard' radiological convention views of patient space
  // these calls adjust the SliceToRAS matrix to position the slice
  // cutting plane 
  void SetOrientationToAxial();
  void SetOrientationToSagittal();
  void SetOrientationToCoronal();

  // Description:
  // General 'reformat' view that allows for multiplanar reformat
  void SetOrientationToReformat();
  
  // Description
  // A description of the current orientation
  vtkGetStringMacro (OrientationString);
  vtkSetStringMacro (OrientationString);

  // Description:
  // Size of the slice plane in millimeters
  vtkGetVector3Macro (FieldOfView, double);
  void SetFieldOfView (double x, double y, double z);

  // Description:
  // Number of samples in each direction
  // -- note that the spacing is implicitly FieldOfView / Dimensions
  vtkGetVector3Macro (Dimensions, unsigned int);
  void SetDimensions (unsigned int x, unsigned int y, unsigned int z);

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

  // Description:
  // Name of the layout
  void SetLayoutName(const char *layoutName) {
    this->SetSingletonTag(layoutName);
  }
  char *GetLayoutName() {
    return this->GetSingletonTag();
  }

  // Description:
  // Set the number of rows and columns to use in a LightBox display
  // of the node
  void SetLayoutGrid( int rows, int columns );

  // Description:
  // Set/Get the number of rows to use ina LightBox display
  vtkGetMacro (LayoutGridRows, int);
  virtual void SetLayoutGridRows(int rows);
  
  // Description:
  // Set/Get the number of columns to use ina LightBox display
  vtkGetMacro (LayoutGridColumns, int);
  virtual void SetLayoutGridColumns(int cols);
  
  // Description:
  // Set the SliceToRAS matrix according to the position and orientation of the locator:
  // N(x, y, z) - the direction vector of the locator
  // T(x, y, z) - the transverse direction vector of the locator
  // P(x, y, z) - the tip location of the locator
  // All the above values are in RAS space. 
  void SetSliceToRASByNTP (double Nx, double Ny, double Nz,
                           double Tx, double Ty, double Tz,
                           double Px, double Py, double Pz,
                           int Orientation);

  // Description:
  // Set the RAS offset of the Slice to the passed values
  void JumpSlice(double r, double a, double s);
  void JumpAllSlices(double r, double a, double s);

protected:


  vtkMRMLSliceNode();
  ~vtkMRMLSliceNode();
  vtkMRMLSliceNode(const vtkMRMLSliceNode&);
  void operator=(const vtkMRMLSliceNode&);


  vtkMatrix4x4 *SliceToRAS;
  vtkMatrix4x4 *XYToSlice;
  vtkMatrix4x4 *XYToRAS;

  int SliceVisible;
  int WidgetVisible;
  double FieldOfView[3];
  unsigned int Dimensions[3];
  char *OrientationString;

  int LayoutGridRows;
  int LayoutGridColumns;

};

#endif

