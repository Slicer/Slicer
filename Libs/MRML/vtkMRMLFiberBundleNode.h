/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLFiberBundleNode - MRML node to represent a fiber bundle from tractography in DTI data.
// .SECTION Description
// FiberBundle nodes contain trajectories ("fibers") from tractography, internally represented as vtkPolyData.
// A FiberBundle node contains many fibers and forms the smallest logical unit of tractography 
// that MRML will manage/read/write. Each fiber has accompanying tensor data.  
// Visualization parameters for these nodes are controlled by the vtkMRMLFiberBundleDisplayNode class.
//

#ifndef __vtkMRMLFiberBundleNode_h
#define __vtkMRMLFiberBundleNode_h

#include <string>

#include "vtkPolyData.h" 

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"


class vtkCallbackCommand;

class VTK_MRML_EXPORT vtkMRMLFiberBundleNode : public vtkMRMLModelNode
{
public:
  static vtkMRMLFiberBundleNode *New();
  vtkTypeMacro(vtkMRMLFiberBundleNode,vtkMRMLModelNode);
  //vtkTypeMacro(vtkMRMLFiberBundleNode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "FiberBundle";};

  // Description:
  // get associated line display node or NULL if not set
  vtkMRMLFiberBundleDisplayNode* GetLineDisplayNode();

  // Description:
  // get associated tube display node or NULL if not set
  vtkMRMLFiberBundleDisplayNode* GetTubeDisplayNode();

  // Description:
  // get associated glyph display node or NULL if not set
  vtkMRMLFiberBundleDisplayNode* GetGlyphDisplayNode();

  // Description:
  // add line display node if not already present and return it
  vtkMRMLFiberBundleDisplayNode* AddLineDisplayNode();

  // Description:
  // add tube display node if not already present and return it
  vtkMRMLFiberBundleDisplayNode* AddTubeDisplayNode();

  // Description:
  // add glyph display node if not already present and return it
  vtkMRMLFiberBundleDisplayNode* AddGlyphDisplayNode();

  
protected:
  vtkMRMLFiberBundleNode(){};
  ~vtkMRMLFiberBundleNode(){};
  vtkMRMLFiberBundleNode(const vtkMRMLFiberBundleNode&);
  void operator=(const vtkMRMLFiberBundleNode&);

};

#endif
