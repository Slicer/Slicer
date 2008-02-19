/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLUnstructuredGridNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLUnstructuredGridNode - MRML node to represent a fiber bundle from tractography in DTI data.
// .SECTION Description
// UnstructuredGrid nodes contain trajectories ("fibers") from tractography, internally represented as vtkPolyData.
// A UnstructuredGrid node contains many fibers and forms the smallest logical unit of tractography 
// that MRML will manage/read/write. Each fiber has accompanying tensor data.  
// Visualization parameters for these nodes are controlled by the vtkMRMLUnstructuredGridDisplayNode class.
//

#ifndef __vtkMRMLUnstructuredGridNode_h
#define __vtkMRMLUnstructuredGridNode_h

#include <string>

#include "vtkPolyData.h" 
#include "vtkUnstructuredGrid.h" 

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLUnstructuredGridDisplayNode.h"

class vtkCallbackCommand;

class VTK_MRML_EXPORT vtkMRMLUnstructuredGridNode : public vtkMRMLDisplayableNode
{
public:
  static vtkMRMLUnstructuredGridNode *New();
  vtkTypeMacro(vtkMRMLUnstructuredGridNode,vtkMRMLDisplayableNode);

  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();


  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "UnstructuredGrid";};

  // Description:
  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);


  // Description:
  // Get associated model display MRML node
  /**
  vtkMRMLUnstructuredGridDisplayNode* GetUnstructuredGridDisplayNode() 
  {
    return vtkMRMLUnstructuredGridDisplayNode::SafeDownCast(this->DisplayNode);
  };
  **/

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
    // Description:
  // Set and observe UnstructuredGrid  for this model
  vtkGetObjectMacro(UnstructuredGrid, vtkUnstructuredGrid);
  void SetAndObserveUnstructuredGrid(vtkUnstructuredGrid *UnstructuredGrid);

  virtual bool CanApplyNonLinearTransforms() { return true; }
  virtual void ApplyTransform(vtkAbstractTransform* transform);

protected:
  vtkMRMLUnstructuredGridNode();
  ~vtkMRMLUnstructuredGridNode();
  vtkMRMLUnstructuredGridNode(const vtkMRMLUnstructuredGridNode&);
  void operator=(const vtkMRMLUnstructuredGridNode&);

  vtkSetObjectMacro(UnstructuredGrid, vtkUnstructuredGrid);


  // Data
  vtkUnstructuredGrid *UnstructuredGrid;
  
};

#endif
