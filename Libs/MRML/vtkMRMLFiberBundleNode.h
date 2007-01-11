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
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"


class vtkCallbackCommand;

class VTK_MRML_EXPORT vtkMRMLFiberBundleNode : public vtkMRMLModelNode
{
public:
  static vtkMRMLFiberBundleNode *New();
  //vtkTypeMacro(vtkMRMLFiberBundleNode,vtkMRMLModelNode);
  vtkTypeMacro(vtkMRMLFiberBundleNode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML (MRML) file.
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "FiberBundle";};

  // Description:
  // Update this node if it depends on other nodes 
  // when the node is deleted in the scene.
  virtual void UpdateReferences();

  // Description:
  // Find the storage node and read the data.
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Update the stored reference to another node in the scene.
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // Set string ID of the storage MRML node.
  vtkSetReferenceStringMacro(StorageNodeID);

  // Description:
  // Get string ID of the storage MRML node.
  vtkGetStringMacro(StorageNodeID);

  // Description:
  // Set string ID of the display MRML node, and observe it to generate DisplayModified events.
  void SetAndObserveDisplayNodeID(const char *DisplayNodeID);

  // Description:
  // Get string ID of the display MRML node.
  vtkGetStringMacro(DisplayNodeID);


  // Description:
  // Get associated storage MRML node.
  vtkMRMLStorageNode* GetStorageNode();

  // Description:
  // Get associated display MRML node.
  vtkMRMLFiberBundleDisplayNode* GetDisplayNode();
    
  // Description:
  // Set and observe poly data for this fiber bundle.
  vtkGetObjectMacro(PolyData, vtkPolyData);
  void SetAndObservePolyData(vtkPolyData *PolyData);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

//BTX
  // Description:
  // DisplayModifiedEvent is generated when display node parameters are changed.
  // PolyDataModifiedEvent is generated when PolyData is changed.
  enum
    {
      DisplayModifiedEvent = 17000,
      PolyDataModifiedEvent = 17001,
    };
//ETX

protected:
  vtkMRMLFiberBundleNode();
  ~vtkMRMLFiberBundleNode();
  vtkMRMLFiberBundleNode(const vtkMRMLFiberBundleNode&);
  void operator=(const vtkMRMLFiberBundleNode&);

  vtkSetReferenceStringMacro(DisplayNodeID);

  vtkSetObjectMacro(PolyData, vtkPolyData);

  // Description:
  // The fiber data itself, with tensors.
  vtkPolyData *PolyData;

  // Description:
  // The storage node for read/write fiber data.
  char *StorageNodeID;

  // Description:
  // The ID of the display node for display parameters of fiber data.
  char *DisplayNodeID;

  // Description:
  // Pointer to the display node for display parameters of fiber data.
  vtkMRMLFiberBundleDisplayNode *FiberBundleDisplayNode;
};

#endif
