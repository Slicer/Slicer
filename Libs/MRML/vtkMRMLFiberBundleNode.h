/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLFiberBundleNode - MRML node to represent a 3D surface model.
// .SECTION Description
// FiberBundle nodes describe polygonal data.  FiberBundles 
// are assumed to have been constructed with the orientation and voxel 
// dimensions of the original segmented volume.

#ifndef __vtkMRMLFiberBundleNode_h
#define __vtkMRMLFiberBundleNode_h

#include <string>

#include "vtkPolyData.h" 
#include "vtkMatrix4x4.h"

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
  virtual const char* GetNodeTagName() {return "FiberBundle";};

   // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Finds the storage node and reads the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // String ID of the storage MRML node
  vtkSetReferenceStringMacro(StorageNodeID);
  vtkGetStringMacro(StorageNodeID);

  // Description:
  // String ID of the display MRML node
  void SetAndObserveDisplayNodeID(const char *DisplayNodeID);
  vtkGetStringMacro(DisplayNodeID);


  // Description:
  // Get associated storage MRML node
  vtkMRMLStorageNode* GetStorageNode();

  // Description:
  // Get associated display MRML node
  vtkMRMLFiberBundleDisplayNode* GetDisplayNode();
    
  // Description:
  // Set and observe poly data for this fiber bundle
  vtkGetObjectMacro(PolyData, vtkPolyData);
  void SetAndObservePolyData(vtkPolyData *PolyData);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

//BTX
  // Description:
  // DisplayModifiedEvent is generated when display node parameters is changed
  // PolyDataModifiedEvent is generated when PolyData is changed
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


  // Data
  vtkPolyData *PolyData;

  char *StorageNodeID;
  char *DisplayNodeID;

  vtkMRMLFiberBundleDisplayNode *FiberBundleDisplayNode;
};

#endif
