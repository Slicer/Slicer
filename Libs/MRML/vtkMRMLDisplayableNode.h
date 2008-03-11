/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayableNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLDisplayableNode - MRML node to represent a 3D surface model.
// .SECTION Description
// Model nodes describe polygonal data.  Models 
// are assumed to have been constructed with the orientation and voxel 
// dimensions of the original segmented volume.

#ifndef __vtkMRMLDisplayableNode_h
#define __vtkMRMLDisplayableNode_h

#include <string>

#include "vtkPolyData.h" 
#include "vtkMatrix4x4.h"

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorableNode.h"
#include "vtkMRMLDisplayNode.h"

class vtkCallbackCommand;
class vtkFloatArray;
class VTK_MRML_EXPORT vtkMRMLDisplayableNode : public vtkMRMLStorableNode
{
public:
  static vtkMRMLDisplayableNode *New(){return NULL;};
  vtkTypeMacro(vtkMRMLDisplayableNode,vtkMRMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  virtual const char* GetNodeTagName() = 0;

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
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Clears out the list of display nodes, and updates them from teh lsit of
  // display node ids
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // String ID of the display MRML node
  void SetAndObserveDisplayNodeID(const char *DisplayNodeID);
  void AddAndObserveDisplayNodeID(const char *DisplayNodeID);
  void SetAndObserveNthDisplayNodeID(int n, const char *DisplayNodeID);

  int GetNumberOfDisplayNodes()
    {
    return this->DisplayNodeIDs.size();
    };

  const char *GetNthDisplayNodeID(int n)
  {
      if (n < 0 || n >= (int)this->DisplayNodeIDs.size())
      {
          return NULL;
      }
      return this->DisplayNodeIDs[n].c_str();
  };

  const char *GetDisplayNodeID()
    {
    return this->GetNthDisplayNodeID(0);
    };

  // Description:
  // Get associated display MRML node
  vtkMRMLDisplayNode* GetNthDisplayNode(int n);

  vtkMRMLDisplayNode* GetDisplayNode()
    {
    return this->GetNthDisplayNode(0);
    };
    
  // Description:
  // Set and observe poly data for this model
  vtkGetObjectMacro(PolyData, vtkPolyData);
  virtual void SetAndObservePolyData(vtkPolyData *PolyData);


  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
  
//BTX
  // Description:
  // DisplayModifiedEvent is generated when display node parameters is changed
  // PolyDataModifiedEvent is generated when PloyData is changed
  enum
    {
      DisplayModifiedEvent = 17000,
      PolyDataModifiedEvent = 17001,
    };
//ETX

 protected:
  vtkMRMLDisplayableNode();
  ~vtkMRMLDisplayableNode();
  vtkMRMLDisplayableNode(const vtkMRMLDisplayableNode&);
  void operator=(const vtkMRMLDisplayableNode&);

  void SetDisplayNodeID(const char* id) ;
  void SetNthDisplayNodeID(int n, const char* id);
  void AddDisplayNodeID(const char* id);
  void AddAndObserveDisplayNode(vtkMRMLDisplayNode *dnode);

  vtkSetObjectMacro(PolyData, vtkPolyData);


  // Data
  vtkPolyData *PolyData;

//BTX
  std::vector<std::string> DisplayNodeIDs;
 
  std::vector<vtkMRMLDisplayNode *> DisplayNodes;
//ETX
};

#endif
