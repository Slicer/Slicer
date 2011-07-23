/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
///  vtkMRMLFiberBundleNode - MRML node to represent a fiber bundle from tractography in DTI data.
/// 
/// FiberBundle nodes contain trajectories ("fibers") from tractography, internally represented as vtkPolyData.
/// A FiberBundle node contains many fibers and forms the smallest logical unit of tractography 
/// that MRML will manage/read/write. Each fiber has accompanying tensor data.  
/// Visualization parameters for these nodes are controlled by the vtkMRMLFiberBundleDisplayNode class.
//

#ifndef __vtkMRMLFiberBundleNode_h
#define __vtkMRMLFiberBundleNode_h

#include "vtkMRMLModelNode.h"
class vtkMRMLFiberBundleDisplayNode;

//class vtkCallbackCommand;
//class vtkMRMLStorageNode;

class VTK_MRML_EXPORT vtkMRMLFiberBundleNode : public vtkMRMLModelNode
{
public:
  static vtkMRMLFiberBundleNode *New();
  vtkTypeMacro(vtkMRMLFiberBundleNode,vtkMRMLModelNode);
  //vtkTypeMacro(vtkMRMLFiberBundleNode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "FiberBundle";};

  /// 
  /// get associated line display node or NULL if not set
  vtkMRMLFiberBundleDisplayNode* GetLineDisplayNode();

  /// 
  /// get associated tube display node or NULL if not set
  vtkMRMLFiberBundleDisplayNode* GetTubeDisplayNode();

  /// 
  /// get associated glyph display node or NULL if not set
  vtkMRMLFiberBundleDisplayNode* GetGlyphDisplayNode();

  /// 
  /// add line display node if not already present and return it
  vtkMRMLFiberBundleDisplayNode* AddLineDisplayNode();

  /// 
  /// add tube display node if not already present and return it
  vtkMRMLFiberBundleDisplayNode* AddTubeDisplayNode();

  /// 
  /// add glyph display node if not already present and return it
  vtkMRMLFiberBundleDisplayNode* AddGlyphDisplayNode();

  /// 
  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode()
    {
    return Superclass::CreateDefaultStorageNode();
    };
  
protected:
  vtkMRMLFiberBundleNode(){};
  ~vtkMRMLFiberBundleNode(){};
  vtkMRMLFiberBundleNode(const vtkMRMLFiberBundleNode&);
  void operator=(const vtkMRMLFiberBundleNode&);

};

#endif
