/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLViewNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLViewNode - MRML node to represent view parameters
// .SECTION Description
// View node contains view parameters

#ifndef __vtkMRMLViewNode_h
#define __vtkMRMLViewNode_h

#include <string>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

class VTK_MRML_EXPORT vtkMRMLViewNode : public vtkMRMLNode
{
public:
  static vtkMRMLViewNode *New();
  vtkTypeMacro(vtkMRMLViewNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "View";};

  // Description:
  // Indicates if the box is visible
  vtkGetMacro(BoxVisible, int);
  vtkSetMacro(BoxVisible, int); 
  
  // Description:
  // Indicates if the axis labels are visible
  vtkGetMacro(AxisLabelsVisible, int);
  vtkSetMacro(AxisLabelsVisible, int);

  // Description:
  // Field of view size
  vtkGetMacro(FieldOfView, double);
  vtkSetMacro(FieldOfView, double);

  // Description:
  // Axis label size
  vtkGetMacro(LetterSize, double);
  vtkSetMacro(LetterSize, double);


protected:
  vtkMRMLViewNode();
  ~vtkMRMLViewNode();
  vtkMRMLViewNode(const vtkMRMLViewNode&);
  void operator=(const vtkMRMLViewNode&);


  int BoxVisible;
  int AxisLabelsVisible;
  double FieldOfView;
  double LetterSize;
  
  
};

#endif
