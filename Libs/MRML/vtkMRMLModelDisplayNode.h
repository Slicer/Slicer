/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
///  vtkMRMLModelDisplayNode - MRML node to represent a dispaly property of 3D surface model.
/// 
/// vtkMRMLModelDisplayNode nodes stores display property of a 3D surface model
/// including reference to ColorNode, texture, opacity, etc.

#ifndef __vtkMRMLModelDisplayNode_h
#define __vtkMRMLModelDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLColorNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

class VTK_MRML_EXPORT vtkMRMLModelDisplayNode : public vtkMRMLDisplayNode
{
public:
  static vtkMRMLModelDisplayNode *New();
  vtkTypeMacro(vtkMRMLModelDisplayNode,vtkMRMLDisplayNode);
  
  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "ModelDisplay";};

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Set and observe poly data for this model
  vtkGetObjectMacro(PolyData, vtkPolyData);
  virtual void SetPolyData(vtkPolyData* polydata);

protected:
  vtkMRMLModelDisplayNode();
  ~vtkMRMLModelDisplayNode();
  vtkMRMLModelDisplayNode(const vtkMRMLModelDisplayNode&);
  void operator=(const vtkMRMLModelDisplayNode&);

  vtkPolyData *PolyData;
};

#endif
